/*************************************************************************
 *
 *  $RCSfile: numtochar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2002-03-26 17:13:19 $
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

// prevent internal compiler error with MSVC6SP3
#include <stl/utility>

#define TRANSLITERATION_ALL
#include <numtochar.hxx>
#include <data/numberchar.h>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL NumToChar::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset ) throw(RuntimeException) {

    // Create a string buffer which can hold nCount + 1 characters.
    rtl_uString *newStr;
    x_rtl_uString_new_WithLength( &newStr, nCount ); // defined in x_rtl_ustring.h  The reference count is 0 now.

    // Prepare pointers of unicode character arrays.
    const sal_Unicode *src = inStr.getStr() + startPos;
    sal_Unicode *dst = newStr->buffer;

    // Allocate the same length as inStr to offset argument.
    offset.realloc(inStr.getLength());
    sal_Int32 *p = offset.getArray();
    sal_Int32 position = startPos;

    for (sal_Int32 index = 0; index < nCount; index++) {
        sal_Unicode ch = src[index];
        dst[index] = (0x0030 <= ch && ch <= 0x0039) ? num2char[ ch - 0x0030 ] : ch;
        *p++ = position++;
    }

    return OUString( newStr ); // defined in rtl/usrting. The reference count is increased from 0 to 1.
}

#define TRANSLITERATION_NUMTOCHAR( number, name ) \
NumToChar##name::NumToChar##name() \
{ \
    num2char = NumberChar[number]; \
    transliterationName = "NumToChar"#name; \
    implementationName = "com.sun.star.i18n.Transliteration.NumToChar"#name; \
}

TRANSLITERATION_NUMTOCHAR( NumberChar_HalfWidth, )
TRANSLITERATION_NUMTOCHAR( NumberChar_FullWidth, Fullwidth)
TRANSLITERATION_NUMTOCHAR( NumberChar_Lower_zh, Lower_zh_CN)
TRANSLITERATION_NUMTOCHAR( NumberChar_Lower_zh, Lower_zh_TW)
TRANSLITERATION_NUMTOCHAR( NumberChar_Upper_zh, Upper_zh_CN)
TRANSLITERATION_NUMTOCHAR( NumberChar_Upper_zh_TW, Upper_zh_TW)
TRANSLITERATION_NUMTOCHAR( NumberChar_Modern_ja, KanjiShort_ja_JP)
TRANSLITERATION_NUMTOCHAR( NumberChar_Lower_ko, Lower_ko)
TRANSLITERATION_NUMTOCHAR( NumberChar_Upper_ko, Upper_ko)
TRANSLITERATION_NUMTOCHAR( NumberChar_Hangul_ko, Hangul_ko)
TRANSLITERATION_NUMTOCHAR( NumberChar_Indic_ar, Indic_ar)
TRANSLITERATION_NUMTOCHAR( NumberChar_EastIndic_ar, EastIndic_ar)
TRANSLITERATION_NUMTOCHAR( NumberChar_Indic_hi, Indic_hi)
TRANSLITERATION_NUMTOCHAR( NumberChar_th, _th)
#undef TRANSLITERATION_NUMTOCHAR

} } } }
