/*************************************************************************
 *
 *  $RCSfile: numtotext_cjk.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: khong $ $Date: 2002-05-31 06:40:46 $
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

#define TRANSLITERATION_ALL
#include <numtotext_cjk.hxx>
#include <data/numberchar.h>
#include <data/bullet.h>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

#define NUMBER_OMIT_ZERO (1 << 0)
#define NUMBER_OMIT_ONE  (1 << 1)
#define NUMBER_OMIT_ONLY_ZERO  (1 << 2)

#define NUMBER_COMMA    0x002C
#define isComma(ch) (ch == NUMBER_COMMA)
#define MAX_SAL_UINT32  0xFFFFFFFF
#define MAX_VALUE   (MAX_SAL_UINT32 - 9) / 10

NumToText_CJK::NumToText_CJK() {
    numberChar = NULL;
    bulletCount = 0;
    number = 0;
}

OUString SAL_CALL NumToText_CJK::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    if (! numberChar) throw RuntimeException();

    sal_Int32 strLen = inStr.getLength() - startPos;

    if (nCount > strLen)
        nCount = strLen;

    if (nCount > 0) {
        const sal_Unicode *str = inStr.getStr() + startPos;
        rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount * 2);
        rtl_uString *srcStr = x_rtl_uString_new_WithLength(nCount); // for keeping number without comma
        sal_Int32 i, len = 0, count = 0;

        offset.realloc( nCount * 2 );
        sal_Bool doDecimal = sal_False;
        sal_Bool makeBullet = bulletCount > 0;

        for (i = 0; i <= nCount; i++) {
        if (i < nCount && isNumber(str[i])) {
            if (doDecimal) {
            newStr->buffer[count] = numberChar[str[i] - NUMBER_ZERO];
            offset[count++] = i + startPos;
            }
            else
            srcStr->buffer[len++] = str[i];
        } else {
            if (len > 0) {
            if (isComma(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                continue; // skip comma inside number string
            if (makeBullet) {
                sal_uInt32 value = 0;
                for (sal_Int32 j = 0; j < len; j++) {
                if (value < MAX_VALUE)
                    value = (value * 10) + (str[j] - NUMBER_ZERO);
                else
                    throw RuntimeException();   // overfollow, number is too big
                }
                newStr->buffer[count] =  value ? numberChar[(value-1) % bulletCount] : NUMBER_ZERO;
                offset[count++] = i - len + startPos;
            }
            else {
                sal_Bool notZero = sal_False;
                for (sal_Int32 begin = 0, end = len % multiplierExponent[0];
                    end <= len; begin = end, end += multiplierExponent[0]) {
                sal_Int32 _count = count;
                notZero |= numberMaker(srcStr->buffer, begin, end - begin, newStr->buffer, count,
                        end == len ? 0 : multiplierChar[0], offset, i - len + startPos);
                if (count > 0 && newStr->buffer[count-1] == numberChar[0])
                    count--;
                if (notZero && _count == count) {
                    if (end != len) {
                    newStr->buffer[count] = multiplierChar[0];
                    offset[count++] = i - len + startPos;
                    }
                }
                }
                if (! notZero && ! (numberFlag & NUMBER_OMIT_ONLY_ZERO)) {
                newStr->buffer[count] = numberChar[0];
                offset[count++] = i - len + startPos;
                }
            }
            len = 0;
            }
            if (i < nCount) {
            if (doDecimal = (!makeBullet && !doDecimal &&
                    isDecimal(str[i]) && i < nCount-1 && isNumber(str[i+1])))
                newStr->buffer[count] = DecimalChar[number];
            else if (!makeBullet && isMinus(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                newStr->buffer[count] = MinusChar[number];
            else
                newStr->buffer[count] = str[i];
            offset[count++] = i + startPos;
            }
        }
        }

        offset.realloc(count);
        return OUString(newStr->buffer, count);
    }
    return OUString();
}

sal_Bool SAL_CALL NumToText_CJK::numberMaker(const sal_Unicode *str, sal_Int32 begin, sal_Int32 len,
    sal_Unicode *dst, sal_Int32& count, sal_Unicode multiChar, Sequence< sal_Int32 >& offset, sal_Int32 startPos)
{
    if ( len <= multiplierExponent[exponentCount-1] ) {
        if (multiplierExponent[exponentCount-1] > 1) {
        sal_Int16 i;
        sal_Bool notZero = false;
        for (i = 0; i < len; i++, begin++) {
            if (notZero || str[begin] != NUMBER_ZERO) {
            dst[count] = numberChar[str[begin] - NUMBER_ZERO];
            offset[count++] = begin + startPos;
            notZero = sal_True;
            }
        }
        if (notZero && multiChar > 0) {
            dst[count] = multiChar;
            offset[count++] = begin + startPos;
        }
        return notZero;
        } else if (str[begin] != NUMBER_ZERO) {
        if (!(numberFlag & NUMBER_OMIT_ONE) || multiChar == 0 || str[begin] != NUMBER_ONE) {
            dst[count] = numberChar[str[begin] - NUMBER_ZERO];
            offset[count++] = begin + startPos;
        }
        if (multiChar > 0) {
            dst[count] = multiChar;
            offset[count++] = begin + startPos;
        }
        } else if (!(numberFlag & NUMBER_OMIT_ZERO) && count > 0 && dst[count-1] != numberChar[0]) {
        dst[count] = numberChar[0];
        offset[count++] = begin + startPos;
        }
        return str[begin] != NUMBER_ZERO;
    } else {
        sal_Bool printPower = sal_False;
        sal_Int16 last = 0;
        for (sal_Int16 i = 1; i <= exponentCount; i++) {
        sal_Int32 tmp = len - (i == exponentCount ? 0 : multiplierExponent[i]);
        if (tmp > 0) {
            printPower |= numberMaker(str, begin, tmp, dst, count,
                (i == exponentCount ? 0 : multiplierChar[i]), offset, startPos);
            begin += tmp;
            len -= tmp;
        }
        }
        if (printPower) {
        if (count > 0 && dst[count-1] == numberChar[0])
            count--;
        if (multiChar > 0) {
            dst[count] = multiChar;
            offset[count++] = begin + startPos;
        }
        }
        return printPower;
    }
}

#define TRANSLITERATION_NUMTOTEXT( name, _number, _mult, _short, flag ) \
NumToText##name::NumToText##name() \
{ \
    number = NumberChar_##_number; \
    numberChar = NumberChar[NumberChar_##_number]; \
    multiplierChar = MultiplierChar_##_short[Multiplier_##_mult]; \
    exponentCount = ExponentCount_##_short; \
    multiplierExponent = MultiplierExponent_##_short; \
    numberFlag = flag; \
    transliterationName = "NumToText"#name; \
    implementationName = "com.sun.star.i18n.Transliteration.NumToText"#name; \
}
TRANSLITERATION_NUMTOTEXT( Lower_zh_CN, Lower_zh, Lower_zh, CJK, 0 )
TRANSLITERATION_NUMTOTEXT( Upper_zh_CN, Upper_zh, Upper_zh, CJK, 0 )
TRANSLITERATION_NUMTOTEXT( Lower_zh_TW, Lower_zh, Lower_zh, CJK, 0 )
TRANSLITERATION_NUMTOTEXT( Upper_zh_TW, Upper_zh_TW, Upper_zh_TW, CJK, 0 )
TRANSLITERATION_NUMTOTEXT( Fullwidth_zh, FullWidth, Lower_zh, CJK, 0 )
TRANSLITERATION_NUMTOTEXT( Fullwidth_ja, FullWidth, Modern_ja, CJK, 0 )
TRANSLITERATION_NUMTOTEXT( Fullwidth_ko, FullWidth, Hangul_ko, CJK, 0 )
#define Multiplier_Lower_ko Multiplier_Upper_zh_TW
#define Multiplier_Upper_ko Multiplier_Upper_zh_TW
TRANSLITERATION_NUMTOTEXT( FormalLower_ko, Lower_ko, Lower_ko, CJK, NUMBER_OMIT_ZERO )
TRANSLITERATION_NUMTOTEXT( FormalUpper_ko, Upper_ko, Upper_ko, CJK, NUMBER_OMIT_ZERO )
TRANSLITERATION_NUMTOTEXT( FormalHangul_ko, Hangul_ko, Hangul_ko, CJK, NUMBER_OMIT_ZERO )
#define NUMBER_OMIT_ALL ( NUMBER_OMIT_ZERO|NUMBER_OMIT_ONE|NUMBER_OMIT_ONLY_ZERO )
TRANSLITERATION_NUMTOTEXT( InformalLower_ko, Lower_ko, Lower_ko, CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( InformalUpper_ko, Upper_ko, Upper_ko, CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( InformalHangul_ko, Hangul_ko, Hangul_ko, CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( KanjiLongTraditional_ja_JP, Traditional_ja, Traditional_ja, CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( KanjiLongModern_ja_JP, Modern_ja, Modern_ja, CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( Date_zh, Lower_zh, Lower_zh, CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( KanjiShortTraditional_ja_JP, Traditional_ja, Traditional_ja, short_CJK, NUMBER_OMIT_ALL )
TRANSLITERATION_NUMTOTEXT( KanjiShortModern_ja_JP, Modern_ja, Modern_ja,short_CJK, NUMBER_OMIT_ALL )
#undef TRANSLITERATION_NUMTOTEXT

#define TRANSLITERATION_NUMTOTEXT( name ) \
NumToText##name::NumToText##name() \
{ \
    numberChar = table_##name; \
    bulletCount = sizeof(table_##name) / sizeof(sal_Unicode); \
    transliterationName = "NumToText"#name; \
    implementationName = "com.sun.star.i18n.Transliteration.NumToText"#name; \
}
TRANSLITERATION_NUMTOTEXT( AIUFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT( AIUHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT( IROHAFullWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT( IROHAHalfWidth_ja_JP )
TRANSLITERATION_NUMTOTEXT( CircledNumber )
#undef TRANSLITERATION_NUMTOTEXT

} } } }
