/*************************************************************************
 *
 *  $RCSfile: tencinfo.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: th $ $Date: 2001-07-16 11:44:08 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_TENCINFO_H
#define _RTL_TENCINFO_H

#ifndef _SAL_TYPES_H
#include <sal/types.h>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------- */
/* - TextEncoding - InfoFlags - */
/* ---------------------------- */

#define RTL_TEXTENCODING_INFO_CONTEXT   ((sal_uInt32)0x00000001)
#define RTL_TEXTENCODING_INFO_ASCII     ((sal_uInt32)0x00000002)
#define RTL_TEXTENCODING_INFO_UNICODE   ((sal_uInt32)0x00000004)
#define RTL_TEXTENCODING_INFO_MULTIBYTE ((sal_uInt32)0x00000008)
#define RTL_TEXTENCODING_INFO_R2L       ((sal_uInt32)0x00000010)
#define RTL_TEXTENCODING_INFO_7BIT      ((sal_uInt32)0x00000020)
#define RTL_TEXTENCODING_INFO_SYMBOL    ((sal_uInt32)0x00000040)
#define RTL_TEXTENCODING_INFO_MIME      ((sal_uInt32)0x00000080)

/* ----------------------- */
/* - TextEncoding - Info - */
/* ----------------------- */

typedef struct _rtl_TextEncodingInfo
{
    sal_uInt32          StructSize;
    sal_uInt8           MinimumCharSize;
    sal_uInt8           MaximumCharSize;
    sal_uInt8           AverageCharSize;
    sal_uInt8           Reserved;
    sal_uInt32          Flags;
} rtl_TextEncodingInfo;

sal_Bool SAL_CALL rtl_getTextEncodingInfo( rtl_TextEncoding eTextEncoding, rtl_TextEncodingInfo* pEncInfo );

/* -------------------------------------- */
/* - Charset and TextEncoding - Convert - */
/* -------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromWindowsCharset( sal_uInt8 nWinCharset );
rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMimeCharset( const sal_Char* pMimeCharset );
rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromUnixCharset( const sal_Char* pUnixCharset );

sal_uInt8       SAL_CALL rtl_getBestWindowsCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding );
const sal_Char* SAL_CALL rtl_getBestMimeCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding );
const sal_Char* SAL_CALL rtl_getBestUnixCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding  );

#ifdef __cplusplus
}
#endif

#endif /* _RTL_TENCINFO_H */
