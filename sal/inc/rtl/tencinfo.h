/*************************************************************************
 *
 *  $RCSfile: tencinfo.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:15 $
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

/* ----------- */
/* - Scripts - */
/* ----------- */

typedef sal_uInt16 rtl_Script;
#define SCRIPT_DONTKNOW             ((rtl_Script)0)
#define SCRIPT_UNICODE              ((rtl_Script)1)
#define SCRIPT_SYMBOL               ((rtl_Script)2)
#define SCRIPT_LATIN                ((rtl_Script)3)
#define SCRIPT_EASTEUROPE           ((rtl_Script)4)
#define SCRIPT_CYRILLIC             ((rtl_Script)5)
#define SCRIPT_BALTIC               ((rtl_Script)6)
#define SCRIPT_TURKISH              ((rtl_Script)7)
#define SCRIPT_GREEK                ((rtl_Script)8)
#define SCRIPT_JAPANESE             ((rtl_Script)9)
#define SCRIPT_CHINESE_SIMPLIFIED   ((rtl_Script)10)
#define SCRIPT_CHINESE_TRADITIONAL  ((rtl_Script)11)
#define SCRIPT_KOREAN               ((rtl_Script)12)
#define SCRIPT_ARABIC               ((rtl_Script)13)
#define SCRIPT_HEBREW               ((rtl_Script)14)
#define SCRIPT_ARMENIAN             ((rtl_Script)15)
#define SCRIPT_DEVANAGARI           ((rtl_Script)16)
#define SCRIPT_BENGALI              ((rtl_Script)17)
#define SCRIPT_GURMUKHI             ((rtl_Script)18)
#define SCRIPT_GUJARATI             ((rtl_Script)19)
#define SCRIPT_ORIYA                ((rtl_Script)20)
#define SCRIPT_TAMIL                ((rtl_Script)21)
#define SCRIPT_TELUGU               ((rtl_Script)22)
#define SCRIPT_KANNADA              ((rtl_Script)23)
#define SCRIPT_MALAYALAM            ((rtl_Script)24)
#define SCRIPT_THAI                 ((rtl_Script)25)
#define SCRIPT_VIETNAMESE           ((rtl_Script)26)
#define SCRIPT_LAO                  ((rtl_Script)27)
#define SCRIPT_GEORGIEN             ((rtl_Script)28)

/* ---------------------------- */
/* - TextEncoding - InfoFlags - */
/* ---------------------------- */

#define RTL_TEXTENCODING_INFO_CONTEXT   ((sal_uInt32)0x0001)
#define RTL_TEXTENCODING_INFO_ASCII     ((sal_uInt32)0x0002)
#define RTL_TEXTENCODING_INFO_UNICODE   ((sal_uInt32)0x0004)
#define RTL_TEXTENCODING_INFO_MULTIBYTE ((sal_uInt32)0x0008)
#define RTL_TEXTENCODING_INFO_R2L       ((sal_uInt32)0x0010)
#define RTL_TEXTENCODING_INFO_7BIT      ((sal_uInt32)0x0020)
#define RTL_TEXTENCODING_INFO_SYMBOL    ((sal_uInt32)0x0040)
#define RTL_TEXTENCODING_INFO_MIME      ((sal_uInt32)0x0080)

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
    rtl_Script          Script;
} rtl_TextEncodingInfo;

sal_Bool SAL_CALL rtl_getTextEncodingInfo( rtl_TextEncoding eTextEncoding, rtl_TextEncodingInfo* pEncInfo );

/* -------------------------------------- */
/* - Charset and TextEncoding - Convert - */
/* -------------------------------------- */

rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromWindowsCharset( sal_uInt8 nWinCharset );
rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromPCCodePage( sal_uInt32 nCodePage );
rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMacTextEncoding( sal_uInt32 nMacTextEncoding );
rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromUnixCharset( const sal_Char* pUnixCharset );
rtl_TextEncoding SAL_CALL rtl_getTextEncodingFromMimeCharset( const sal_Char* pMimeCharset );

/* Only for internal use. Because this functions are removed in future versions */
sal_uInt8       SAL_CALL rtl_getBestWindowsCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding );
sal_uInt32      SAL_CALL rtl_getBestPCCodePageFromTextEncoding( rtl_TextEncoding eTextEncoding  );
sal_uInt32      SAL_CALL rtl_getBestMacTextEncodingFromTextEncoding( rtl_TextEncoding eTextEncoding );
const sal_Char* SAL_CALL rtl_getBestUnixCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding  );
const sal_Char* SAL_CALL rtl_getBestMimeCharsetFromTextEncoding( rtl_TextEncoding eTextEncoding );

#ifdef __cplusplus
}
#endif

#endif /* _RTL_TENCINFO_H */



