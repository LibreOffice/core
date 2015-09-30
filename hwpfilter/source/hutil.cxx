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

#include "precompile.h"

#include <ctype.h>
#include "hwpfile.h"
#include "hbox.h"
#include "hutil.h"

static char *get_one_roman(int num, char one, char five, char ten, char *str)
{
    static const char *one_strs[] =
    {
        "", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix"
    };

    strcpy(str, one_strs[num]);
    while (*str)
    {
        switch (*str)
        {
            case 'i':
                *str = one;
                break;
            case 'v':
                *str = five;
                break;
            case 'x':
                *str = ten;
        }
        str++;
    }
    return str;
}


void num2roman(int num, char *buf)
{
    char *pt;

    pt = get_one_roman((num / 100) % 10, 'c', 'd', 'm', buf);
    pt = get_one_roman((num / 10) % 10, 'x', 'l', 'c', pt);
    get_one_roman(num % 10, 'i', 'v', 'x', pt);
}


void str2hstr(const char *c, hchar * i)
{
    hchar ch;

    while( 0 != (ch = *c++))
    {
        if (ch & 0x80)
        {
            if (*c > 32)
            {
                *i++ = (ch << 8) | *c;
                c++;
            }
        }
        else
            *i++ = ch;
    }
    *i = 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
