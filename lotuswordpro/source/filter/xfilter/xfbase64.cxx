/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * Base64 tool.
 ************************************************************************/
#include <string.h>
#include "xfbase64.hxx"

const  sal_Char aBase64EncodeTable[] =
{ 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

/**
 * @descr   Encode 3 byte to 4 byte.
 *          Please refer to RFC to get the base64 algorithm.
 */
inline void Encode_(sal_uInt8 *src, sal_Char* dest)
{
    sal_Int32 nBinaer = (((sal_uInt8)src[ 0]) << 16) +
        (((sal_uInt8)src[1]) <<  8) +
        ((sal_uInt8)src[2]);

    sal_uInt8 nIndex = ((nBinaer & 0xFC0000) >> 18);
    dest[0] = aBase64EncodeTable [nIndex];

    nIndex = (nBinaer & 0x3F000) >> 12;
    dest[1] =aBase64EncodeTable [nIndex];

    nIndex = (nBinaer & 0xFC0) >> 6;
    dest[2] = aBase64EncodeTable [nIndex];

    nIndex = (nBinaer & 0x3F);
    dest[3] = aBase64EncodeTable [nIndex];
}

/**
 * @descr   Base64 encode.
 */
rtl::OUString XFBase64::Encode(sal_uInt8 *buf, sal_Int32 len)
{
    sal_Char    *buffer;
    sal_Int32   nNeeded;
    sal_Int32   cycles = len/3;
    sal_Int32   remain = len%3;

    if( remain == 0 )
        nNeeded = cycles*4;
    else
        nNeeded = (cycles+1)*4;
    buffer = new sal_Char[nNeeded+1];

    memset(buffer, 0, nNeeded+1);

    for( sal_Int32 i=0; i<cycles; i++ )
        Encode_(buf+i*3,buffer+i*4);

    sal_uInt8 last[3];
    if( remain == 1 )
    {
        last[0] = buf[len-1];
        last[1] = last[2] = 0;
        Encode_(last,buffer+nNeeded+1-5);
    }
    else if( remain == 2 )
    {
        last[0] = buf[len-2];
        last[1] = buf[len-1];
        last[2] = 0;
        Encode_(last,buffer+nNeeded+1-5);
    }

    rtl::OUString str = rtl::OUString::createFromAscii(buffer);
    delete[] buffer;

    return str;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
