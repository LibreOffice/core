/*************************************************************************
 *
 *  $RCSfile: hutil.cpp,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2003-10-15 14:41:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2001 by Mizi Research Inc.
 *  Copyright 2003 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Mizi Research Inc.
 *
 *  Copyright: 2001 by Mizi Research Inc.
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/* $Id: hutil.cpp,v 1.1 2003-10-15 14:41:00 dvo Exp $ */

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
