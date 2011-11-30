/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    for (;;) {
        int c;
        errno = 0;
        c = getchar();
        if (c == EOF) {
            exit(errno == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
        } else if (isalnum(c) || strchr("!$'()*+,-.:=@_~/\n", c) != NULL) {
            /* valid RFC 2396 pchar characters + '/' + newline */
            if (putchar(c) == EOF) {
                exit(EXIT_FAILURE);
            }
        } else if (printf("%%%02X", (unsigned char) (char) c) < 0) {
            exit(EXIT_FAILURE);
        }
    }
}
