/*************************************************************************
 *
 *  $RCSfile: converter.c,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2001-10-12 10:44:53 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef INCLUDED_RTL_TEXTENC_CONVERTER_H
#include "converter.h"
#endif

#ifndef INCLUDED_RTL_TEXTENC_TENCHELP_H
#include "tenchelp.h"
#endif

#ifndef _RTL_TEXTCVT_H
#include "rtl/textcvt.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

ImplBadInputConversionAction
ImplHandleBadInputMbTextToUnicodeConversion(sal_Bool bUndefined,
                                            sal_uInt32 nFlags,
                                            sal_Unicode ** pDestBufPtr,
                                            sal_Unicode * pDestBufEnd,
                                            sal_uInt32 * pInfo)
{
    *pInfo |= bUndefined ? RTL_TEXTTOUNICODE_INFO_MBUNDEFINED :
                           RTL_TEXTTOUNICODE_INFO_INVALID;
    switch (nFlags & (bUndefined ? RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_MASK :
                                   RTL_TEXTTOUNICODE_FLAGS_INVALID_MASK))
    {
    case RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR:
    case RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR:
        *pInfo |= RTL_TEXTTOUNICODE_INFO_ERROR;
        return IMPL_BAD_INPUT_STOP;

    case RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_IGNORE:
    case RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE:
        return IMPL_BAD_INPUT_CONTINUE;

    default: /* RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT,
                RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT */
        if (*pDestBufPtr != pDestBufEnd)
        {
            *(*pDestBufPtr)++ = RTL_UNICODE_CHAR_DEFAULT;
            return IMPL_BAD_INPUT_CONTINUE;
        }
        else
            return IMPL_BAD_INPUT_NO_OUTPUT;
    }
}

ImplBadInputConversionAction
ImplHandleBadInputUnicodeToTextConversion(sal_Bool bUndefined,
                                          sal_uInt32 nFlags,
                                          sal_Char ** pDestBufPtr,
                                          sal_Char * pDestBufEnd,
                                          sal_uInt32 * pInfo)
{
    *pInfo |= bUndefined ? RTL_UNICODETOTEXT_INFO_UNDEFINED :
                           RTL_UNICODETOTEXT_INFO_INVALID;
    switch (nFlags & (bUndefined ? RTL_UNICODETOTEXT_FLAGS_UNDEFINED_MASK :
                                   RTL_UNICODETOTEXT_FLAGS_INVALID_MASK))
    {
    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR:
        *pInfo |= RTL_UNICODETOTEXT_INFO_ERROR;
        return IMPL_BAD_INPUT_STOP;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_IGNORE:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_IGNORE:
        return IMPL_BAD_INPUT_CONTINUE;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_0:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_0:
        if (*pDestBufPtr != pDestBufEnd)
        {
            *(*pDestBufPtr)++ = 0;
            return IMPL_BAD_INPUT_CONTINUE;
        }
        else
            return IMPL_BAD_INPUT_NO_OUTPUT;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_QUESTIONMARK:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_QUESTIONMARK:
    default: /* RTL_UNICODETOTEXT_FLAGS_UNDEFINED_DEFAULT,
                RTL_UNICODETOTEXT_FLAGS_INVALID_DEFAULT */
        if (*pDestBufPtr != pDestBufEnd)
        {
            *(*pDestBufPtr)++ = '?';
            return IMPL_BAD_INPUT_CONTINUE;
        }
        else
            return IMPL_BAD_INPUT_NO_OUTPUT;

    case RTL_UNICODETOTEXT_FLAGS_UNDEFINED_UNDERLINE:
    case RTL_UNICODETOTEXT_FLAGS_INVALID_UNDERLINE:
        if (*pDestBufPtr != pDestBufEnd)
        {
            *(*pDestBufPtr)++ = '_';
            return IMPL_BAD_INPUT_CONTINUE;
        }
        else
            return IMPL_BAD_INPUT_NO_OUTPUT;
    }
}
