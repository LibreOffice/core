/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
