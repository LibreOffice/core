/*************************************************************************
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define TRANSLITERATION_ALL
#include <chartonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL CharToNum::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    const sal_Unicode *str = inStr.getStr() + startPos;
    rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount + 1);
    offset.realloc(nCount);
    sal_Int16 index;

    OUString numberChar, decimalChar, minusChar;
    if (number == 0) {
        OUStringBuffer aBuf(NumberChar_Count * 10 + 1);
        for (sal_Int32 i = 0; i < NumberChar_Count; i++)
        aBuf.append(NumberChar[i], 10);
        numberChar = aBuf.makeStringAndClear();
        decimalChar = OUString(DecimalChar);
        minusChar = OUString(MinusChar);
    } else {
        numberChar = OUString(NumberChar[number], 10);
        decimalChar = OUString::valueOf(DecimalChar[number]);
        minusChar = OUString::valueOf(MinusChar[number]);
    }

    for (sal_Int32 i = 0; i < nCount; i++) {
        if ((index = numberChar.indexOf(str[i])) >= 0)
        newStr->buffer[i] = (NUMBER_ZERO + (index % 10));
        else if ((index = decimalChar.indexOf(str[i]) >= 0) &&
                i < nCount-1 && numberChar.indexOf(str[i+1]) >= 0)
        newStr->buffer[i] = NUMBER_DECIMAL;
        else if ((index = minusChar.indexOf(str[i]) >= 0) &&
                i < nCount-1 && numberChar.indexOf(str[i+1]) >= 0)
        newStr->buffer[i] = NUMBER_MINUS;
        else
        newStr->buffer[i] = str[i];
        offset[i] = startPos + i;
    }
    return OUString(newStr->buffer, nCount);
}

CharToNum::CharToNum()
{
    number = 0;
    transliterationName = "CharToNum";
    implementationName = "com.sun.star.i18n.Transliteration.CharToNum";
}

#define TRANSLITERATION_CHARTONUM( name, _number ) \
CharToNum##name::CharToNum##name() \
{ \
    number = NumberChar_##_number; \
    transliterationName = "CharToNum"#name; \
    implementationName = "com.sun.star.i18n.Transliteration.CharToNum"#name; \
}
TRANSLITERATION_CHARTONUM( Fullwidth, FullWidth)
TRANSLITERATION_CHARTONUM( Lower_zh_CN, Lower_zh)
TRANSLITERATION_CHARTONUM( Lower_zh_TW, Lower_zh)
TRANSLITERATION_CHARTONUM( Upper_zh_CN, Upper_zh)
TRANSLITERATION_CHARTONUM( Upper_zh_TW, Upper_zh_TW)
TRANSLITERATION_CHARTONUM( KanjiShort_ja_JP, Modern_ja)
TRANSLITERATION_CHARTONUM( KanjiTraditional_ja_JP, Traditional_ja)
TRANSLITERATION_CHARTONUM( Lower_ko, Lower_ko)
TRANSLITERATION_CHARTONUM( Upper_ko, Upper_ko)
TRANSLITERATION_CHARTONUM( Hangul_ko, Hangul_ko)
TRANSLITERATION_CHARTONUM( Indic_ar, Indic_ar)
TRANSLITERATION_CHARTONUM( EastIndic_ar, EastIndic_ar)
TRANSLITERATION_CHARTONUM( Indic_hi, Indic_hi)
TRANSLITERATION_CHARTONUM( _th, th)
#undef TRANSLITERATION_CHARTONUM

} } } }
