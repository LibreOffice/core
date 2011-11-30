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



/* $Id: hutil.cpp,v 1.5 2008-06-04 10:00:16 vg Exp $ */

#include "precompile.h"

#include <ctype.h>
#include "hwpfile.h"
#include "hbox.h"

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


int hstrlen(const hchar * s)
{
    register int n = 0;

    while (*s++)
        n++;
    return (n);
}
