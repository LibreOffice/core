/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hutil.cpp,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:39:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* $Id: hutil.cpp,v 1.2 2005-09-07 16:39:34 rt Exp $ */

#include "precompile.h"

#include <ctype.h>
#include "hwpfile.h"
#include "hbox.h"

static char *get_one_roman(int num, char one, char five, char ten, char *str)
{
    static char *one_strs[] =
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

    while ((ch = *c++))
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


#ifndef _WIN32

char *strupr(char *str)
{
    while (*str)
    {
        *str = toupper(*str);
        str++;
    }
    return str;
}
#endif                                            // _WIN32
