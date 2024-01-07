#include <stdio.h>
//

int substring(const char *input, int start, int end, char *output) {
    // copystring from from start to end but starting from index 0 in output string
    for (int i = start, j = 0; i < end; ++i, ++j) {
        output[j] = input[i];
    }
    // last item in output string is \0 for null: end - start = length of string
    output[end - start] = '\0';
    return 0;
}


int my_utf8_encode(const char *input, char *output) {
    for (int i = 0, j = 0; input[i] != '\0'; ++i, ++j) {
        // if start of code point
        if (input[i] == '\\' && input[i + 1] == 'u') {
            char slice[10];
            substring(input, i + 2, i + 6, slice);

            // Convert hexadecimal string
            unsigned int slice_hex;
            sscanf(slice, "%x", &slice_hex);

            // Check the size and do corresponding encoding
            if (slice_hex >= 0x80 && slice_hex <= 0x7FF) {
                // output[j] two ones and 0 followed by first 5 bits
                output[j] = (slice_hex >> 6) | 0b11000000;
                // output[j + 1] 10 followed by last 6 bits
                output[j + 1] = (slice_hex & 0b00111111) | 0b10000000;
                j += 1;
            } else if (slice_hex <= 0xFFFF) {
                // 3 ones followed 0 and by first 4 bits
                output[j] = (slice_hex >> 12) | 0b11100000;
                //10 followed by 6 bits
                output[j + 1] = ((slice_hex >> 6) & 0b00111111) | 0b10000000;
                // 10 followed by 6 bits
                output[j + 2] = (slice_hex & 0b00111111) | 0b10000000;
                j += 2;
            } else if (slice_hex <= 0x10FFFF) {
                // four ones followed by 0 annd 3 bits
                output[j] = (slice_hex >> 18) | 0b11110000;
                // 10 followed by 6 bits...
                output[j + 1] = ((slice_hex >> 12) & 0b00111111) | 0b10000000;
                output[j + 2] = ((slice_hex >> 6) & 0b00111111) | 0b10000000;
                output[j + 3] = (slice_hex & 0b00111111) | 0b10000000;
                j += 3;
            }
            i += 5;  // Skip the processed Unicode escape sequence - to make work fo U make dynamic
        } else {
            // ascii latters go unchanged
            output[j] = input[i];
        }
        if (input[i+1] == '\0')
            output[j] = '\0';  // add the null to the output string
    }
    return 0;
}

int my_utf8_numBytes(const char *input, int *length) {
    *length = 0;
    for (int i = 0, j = 0; input[i] != '\0'; ++i, ++j) {
        if (input[i] == '\\' && input[i + 1] == 'u') {
            char slice[10];
            substring(input, i + 2, i + 6, slice);

            // Convert hexadecimal string to integer
            unsigned int slice_hex;
            sscanf(slice, "%x", &slice_hex);

            // Check the range and perform encoding
            if (slice_hex >= 0x80 && slice_hex <= 0x7FF) {
                *length += 1;
            } else if (slice_hex <= 0xFFFF) {
                *length += 2;
            } else if (slice_hex <= 0x10FFFF) {
                *length += 3;
            }
            i += 5;  // Skip the processed Unicode escape sequence - to make work fo U make dynamic
        } else {
            *length += 1;
        }
    }
    return 0;
}


int my_utf8_decode(unsigned char *input, unsigned char *output) {
    for (int i = 0, j = 0; input[i] != '\0'; i++, j++) {
        if ((input[i] & 0b10000000) == 0x00) {
            output[j] = input[i];
        }
            // if first bits are 110 (2 byte utf8)
        else if ((0b11100000 & input[i]) == 0b11000000) {
            output[j] = '\\';
            output[j + 1] = 'u';
            int part1 = (input[i] & 0b00011111) << 6;
            int part2 = (input[i + 1] & 0b00111111);
            int part3 = part1 | part2;
            snprintf((char *)&output[j + 2], 6, "%04X", part3);
            i += 1;
            j += 4;
            // if first bits are 1110 (3 byte utf8)
        } else if ((0b11110000 & input[i]) == 0b11100000) {
            output[j] = '\\';
            output[j + 1] = 'u';
            int part1 = (input[i] & 0b00001111) << 12;
            int part2 = (input[i + 1] & 0b00111111) << 6;
            int part3 = (input[i + 2] & 0b00111111);
            int unicodeValue = part1 | part2 | part3;
            snprintf((char *)&output[j + 2], 6, "%04X", unicodeValue);
            i += 2;
            j += 4;
            // if first bits are 11110 (4 byte utf8)
        } else if ((0b11111000 & input[i]) == 0b11110000) {
            output[j] = '\\';
            output[j + 1] = 'u';
            int part1 = (input[i] & 0b00000111) << 18;
            int part2 = (input[i + 1] & 0b00111111) << 12;
            int part3 = (input[i + 2] & 0b00111111) << 6;
            int part4 = (input[i + 3] & 0b00111111);
            int unicodeValue = part1 | part2 | part3 | part4;
            snprintf((char *)&output[j + 2], 6, "%04X", unicodeValue);
            i += 3;
            j += 4;
        }
        if (input[i] == '\0')
            output[j] = '\0';  // add the null to the output string
    }
    return 0;
}



int my_utf8_strLen(const char *input, int *length) {
    *length = 0;
    for (int i = 0, j = 0; input[i] != '\0'; i++, j++) {
        if ((input[i] & 0b10000000) == 0x00) {
            *length += 1;
        }
            // if first 2 bits are 110
        else if ((0b11100000 & input[i]) == 0b11000000) {
            i += 1;
            *length += 1;
        } else if ((0b11110000 & input[i]) == 0b11100000) {
            i += 2;
            *length += 1;
        } else if ((0b11111000 & input[i]) == 0b11110000) {
            i += 3;
            *length += 1;
        }
    }
    return 0;
}




int my_utf8_check(char *input) {
    int len;
    my_utf8_strLen(input, &len);

    for (int i = 0; input[i] != '\0'; i += 8) {
        char slice[9];  // Change size to 9 to include the null terminator
        substring(input, i, i + 8, slice);
        // Convert hexadecimal string to integer
        unsigned int slice_num;
        sscanf(slice, "%x", &slice_num);

        int count = 0;

        // if first bit is set to 0
        if ((slice_num & 0b10000000) == 0x00) {
            continue;
        }
            // if first 2 bits are set to 110
        else if ((0b11100000 & slice_num) == 0b11000000)
            count = 1;
            // if first 3 bits are set to 1110
        else if ((0b11110000 & slice_num) == 0b11100000)
            count = 2;
        else if ((0b11111000 & slice_num) == 0b11110000)
            count = 3;
        else
            return 0;


        // if there are not enough bytes left
        if (i + (8 * count) >= len)
            return 0;

        while (count > 0) {
            i += 8;
            substring(input, i, i + 8, slice);

            // Convert hexadecimal string to integer
            sscanf(slice, "%x", &slice_num);

            // if first 2 bits are not set to 10
            if ((0b11000000 & slice_num) != 0b10000000)
                return 0;

            count--;
        }
    }

    return 1;
}


char *my_utf8_charat(unsigned char *string, int index){
    int len;
    if (my_utf8_check(string) == 0)
        return NULL;
    my_utf8_strLen(string, &len);
    if (index >= len)
        return NULL;
    char output[9];
    substring(string, index, index + 8, output);
    return output;
}


int my_utf8_strcmp(unsigned char *string1, unsigned char *string2){
    for(int i = 0; string1[i] != '\0'; i++){
        if(string1[i] != string2[i])
            return 0;
    }
    return 1;
}

int my_utf8_strGreater(unsigned char *string1, unsigned char *string2){
    if (my_utf8_strcmp(string1, string2)) return 0;
    for(int i = 0; string1[i] != '\0'; i++){
        if(string1[i] > string2[i])
            printf("%c > %c\n", string1[i], string2[i]);
        return 1;
    }
    return 2;
}

int test_my_utf8_encode(char *input, char *expected_output) {
    int bytes = 0;
    my_utf8_numBytes(input, &bytes);
    char output[bytes];
    my_utf8_encode(input, output);
    int result = my_utf8_strcmp(output, expected_output);
    if (!result) {
        printf("encode Test failed: %s\n", input);
        printf("Expected: %s\n", expected_output);
        printf("Got: %s\n", output);
        return 1;
    }
    printf("encode Test passed: %s\n", input);
    return 0;
}

int test_my_utf8_decode(char *expected_output, char *input) {
    int bytes = 0;
    my_utf8_numBytes(input, &bytes);
    //printf("numbytes %d", bytes);
    char output[20];
    printf("%s\n", output);
    my_utf8_decode(input, output);
    int result = my_utf8_strcmp(output, expected_output);
    if (!result) {
        printf("decode Test failed: %s\n", input);
        printf("Expected: %s\n", expected_output);
        printf("Got: %s\n", output);
        return 1;
    }
    printf("decode Test passed: %s\n", input);
    return 0;
}

int test_my_utf8_check(char *input, int expected_output) {
    int result = my_utf8_check(input);
    if (!(result == expected_output)) {
        printf("check Test failed: %s\n", input);
        printf("Expected: %d\n", expected_output);
        printf("Got: %d\n", result);
        return 1;
    }
    printf("check Test passed: %s\n", input);
    return 0;
}

int test_my_utf8_strcomp(char *input1, char *input2 ,int expected_output) {
    int result = my_utf8_strcmp(input1, input2);
    if (!(result == expected_output)) {
        printf("strcomp Test failed: %s, %s\n", input1, input2);
        printf("Expected: %d\n", expected_output);
        printf("Got: %d\n", result);
        return 1;
    }
    printf("strcomp Test passed: %s, %s\n", input1, input2);
    return 0;
}

int test_my_utf8_strGreater(char *input1, char *input2 ,int expected_output) {
    int result = my_utf8_strGreater(input1, input2);
    if (!(result == expected_output)) {
        printf("strGreater Test failed: %s, %s\n", input1, input2);
        printf("Expected: %d\n", expected_output);
        printf("Got: %d\n", result);
        return 1;
    }
    printf("strGreater Test passed: %s, %s\n", input1, input2);
    return 0;
}

int test_my_utf8_strlen(char *input ,int expected_output) {
    int length = 0;
    my_utf8_strLen(input, &length);
    if (!(length == expected_output)) {
        printf("strlen Test failed: %s\n", input);
        printf("Expected: %d\n", expected_output);
        printf("Got: %d\n", length);
        return 1;
    }
    printf("strlen Test passed: %s\n", input);
    return 0;
}


int test_my_utf8_charat(char *input, char *expected_output, int index) {
    char *ans = my_utf8_charat(input, index);
    int result = my_utf8_strcmp(ans, expected_output);
    if (!result) {
        printf("charat Test failed: %s\n", input);
        printf("Expected: %d\n", expected_output);
        printf("Got: %d\n", result);
        return 1;
    }
    printf("charat Test passed: %s\n", input);
    return 0;
}

int test_my_utf8_numBytes(char *input ,int expected_output) {
    int length = 0;
    my_utf8_numBytes(input, &length);
    if (!(length == expected_output)) {
        printf("numBytes Test failed: %s\n", input);
        printf("Expected: %d\n", expected_output);
        printf("Got: %d\n", length);
        return 1;
    }
    printf("numBytes Test passed: %s\n", input);
    return 0;
}



int main() {
//    char input3[] = "Hello אריה"; // Example UTF-8 encoded string
//    char output3[12]; // Adjust the size accordingly
//    my_utf8_decode(input3, output3);
//    printf("UTF-8 decoded string: %s\n", output3);

    // Test my_utf8_encode
    test_my_utf8_encode("a", "a");
    test_my_utf8_encode("\\u05D0\\u05E8\\u05D9\\u05D4", "אריה");
    test_my_utf8_encode("Hello \\u05D0\\u05E8\\u05D9\\u05D4", "Hello אריה");


    //testing check
    test_my_utf8_check("אריה", 1);
    test_my_utf8_check("111110", 0);

    //testing strcomp
    test_my_utf8_strcomp("אריה", "אריה",1);
    test_my_utf8_strcomp("111110", "אריה",0);

    //testing strlen
    test_my_utf8_strlen("אריה",4);
    test_my_utf8_strlen("Hello אריה",10);

    //testing charat
    //test_my_utf8_charat("אריה","ה", 4);
    //test_my_utf8_charat("Hello אריה", "l", 4);

    //testing numBytes
    test_my_utf8_numBytes("\\u05D0\\u05E8\\u05D9\\u05D4",4);
    test_my_utf8_numBytes("Hell0", 5);

    test_my_utf8_strGreater("אריה", "אריה",0);
    test_my_utf8_strGreater("Racheli", "Leah", 1);
    test_my_utf8_strGreater("Gottesman", "Racheli", 2);

    //test my_utf8_decode
    test_my_utf8_decode("a", "a");
    test_my_utf8_decode("\\u05D0\\u05E8\\u05D9\\u05D4", "אריה");
    test_my_utf8_decode("Hello \\u05D0\\u05E8\\u05D9\\u05D4", "Hello אריה");

    char string1[] = "אריה";
    char string2[] = "אריה";
    int result = my_utf8_strcmp(string1, string2);

//    char input[] = "Hello \\u05D0\\u05E8\\u05D9\\u05D4"; // Example UTF-8 encoded string
//    printf("Original string: %s\n", input);
//    char output[17]; // Adjust the size accordingly
//    int length = 0;
//    my_utf8_encode(input, output);
//
//    // Print the resulting UTF-8 encoded string
//    printf("UTF-8 encoded string: %s\n", output);
//    my_utf8_numBytes(input, &length);
//    printf("Bytelength: %d\n", length);
//    my_utf8_strLen(input, &length);
//    printf("strlength: %d\n", length);
//
//    char input2[] = "\\u05D0\\u05E8\\u05D9\\u05D4"; // Example UTF-8 encoded string
//    char output2[9]; // Adjust the size accordingly
//
//    my_utf8_encode(input2, output2);
//    my_utf8_numBytes(input2, &length);
//    printf("Bytelength: %d\n", length);
//    // Print the resulting UTF-8 encoded string
//    printf("UTF-8 encoded string: %s\n", output2);
//
//    my_utf8_strLen(input2, &length);
//    printf("strlength: %d\n", length);
//

//
//

    return 0;
}
