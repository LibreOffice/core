/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
