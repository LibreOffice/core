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
#include <texttonum.hxx>
#include <data/numberchar.h>
#include <rtl/ustrbuf.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL TextToNum::transliterate( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset ) throw(RuntimeException)
{
    sal_Int32 strLen = inStr.getLength() - startPos;

    if (nCount > strLen)
        nCount = strLen;

    if (nCount > 0) {
        const sal_Unicode *str = inStr.getStr() + startPos;
        rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount * MultiplierExponent_CJK[0] + 1);
        offset.realloc( nCount * MultiplierExponent_CJK[0] + 1 );
        sal_Int32 i = 0, count = 0, index;

        OUString numberChar, multiplierChar, decimalChar, minusChar;
        if (number == 0) {
        OUStringBuffer aBuf(NumberChar_Count * 10 + 1);
        for (i = 0; i < NumberChar_Count; i++)
            aBuf.append(NumberChar[i], 10);
        numberChar = aBuf.makeStringAndClear();
        aBuf.ensureCapacity(Multiplier_Count * ExponentCount_CJK + 1);
        for (i = 0; i < Multiplier_Count; i++)
            aBuf.append(MultiplierChar_CJK[i], ExponentCount_CJK);
        multiplierChar = aBuf.makeStringAndClear();
        decimalChar = OUString(DecimalChar);
        minusChar = OUString(MinusChar);
        } else {
        numberChar = OUString(NumberChar[number], 10);
        decimalChar = OUString::valueOf(DecimalChar[number]);
        minusChar = OUString::valueOf(MinusChar[number]);
        multiplierChar = OUString(MultiplierChar_CJK[multiplier], ExponentCount_CJK); \
        }

        while (i < nCount) {
        if ((index = multiplierChar.indexOf(str[i])) >= 0) {
            if (count == 0 || !isNumber(newStr->buffer[count-1])) { // add 1 in front of multiplier
            newStr->buffer[count] = NUMBER_ONE;
            offset[count++] = i;
            }
            index = MultiplierExponent_CJK[index % ExponentCount_CJK];
            numberMaker(index, index, str, i, nCount, newStr->buffer, count, offset,
                numberChar, multiplierChar);
        } else {
            if ((index = numberChar.indexOf(str[i])) >= 0)
            newStr->buffer[count] = (index % 10) + NUMBER_ZERO;
            else if ((index = decimalChar.indexOf(str[i])) >= 0 &&
                (i < nCount-1 && (numberChar.indexOf(str[i+1]) >= 0 ||
                        multiplierChar.indexOf(str[i+1]) >= 0)))
            // Only when decimal point is followed by numbers,
            // it will be convert to ASCII decimal point
            newStr->buffer[count] = NUMBER_DECIMAL;
            else if ((index = minusChar.indexOf(str[i])) >= 0 &&
                (i < nCount-1 && (numberChar.indexOf(str[i+1]) >= 0 ||
                        multiplierChar.indexOf(str[i+1]) >= 0)))
            // Only when minus is followed by numbers,
            // it will be convert to ASCII minus sign
            newStr->buffer[count] = NUMBER_MINUS;
            else
            newStr->buffer[count] = str[i];
            offset[count++] = i++;
        }
        }

        offset.realloc(count);
        for (i = 0; i < count; i++)
        offset[i] += startPos;
        return OUString(newStr->buffer, count);
    }
    return OUString();
}

void SAL_CALL TextToNum::numberMaker(sal_Int16 max, sal_Int16 prev, const sal_Unicode *str, sal_Int32& i,
    sal_Int32 nCount, sal_Unicode *dst, sal_Int32& count, Sequence< sal_Int32 >& offset,
    OUString& numberChar, OUString& multiplierChar)
{
    sal_Int16 curr = 0, num = 0, end = 0, shift = 0;
    while (++i < nCount) {
        if ((curr = numberChar.indexOf(str[i])) >= 0) {
        if (num > 0)
            break;
        num = curr % 10;
        } else if ((curr = multiplierChar.indexOf(str[i])) >= 0) {
        curr = MultiplierExponent_CJK[curr % ExponentCount_CJK];
        if (prev > curr && num == 0) num = 1; // One may be omitted in informal format
        shift = end = 0;
        if (curr >= max)
            max = curr;
        else if (curr > prev)
            shift = max - curr;
        else
            end = curr;
        while (end++ < prev) {
            dst[count] = NUMBER_ZERO + (end == prev ? num : 0);
            offset[count++] = i;
        }
        if (shift) {
            count -= max;
            for (sal_Int16 j = 0; j < shift; j++, count++) {
            dst[count] = dst[count + curr];
            offset[count] = offset[count + curr];
            }
            max = curr;
        }
        numberMaker(max, curr, str, i, nCount, dst, count, offset, numberChar, multiplierChar);
        return;
        } else
        break;
    }
    while (end++ < prev) {
        dst[count] = NUMBER_ZERO + (end == prev ? num : 0);
        offset[count++] = i - 1;
    }
}

TextToNum::TextToNum()
{
    number = multiplier = 0;
    transliterationName = "TextToNum";
    implementationName = "com.sun.star.i18n.Transliteration.TextToNum";
}

#define TRANSLITERATION_TEXTTONUM( name, _number ) \
TextToNum##name::TextToNum##name() \
{ \
    number = NumberChar_##_number; \
    multiplier = Multiplier_##_number; \
    transliterationName = "TextToNum"#name; \
    implementationName = "com.sun.star.i18n.Transliteration.TextToNum"#name; \
}

TRANSLITERATION_TEXTTONUM( Lower_zh_CN, Lower_zh )
TRANSLITERATION_TEXTTONUM( Upper_zh_CN, Upper_zh )
TRANSLITERATION_TEXTTONUM( Lower_zh_TW, Lower_zh )
TRANSLITERATION_TEXTTONUM( Upper_zh_TW, Upper_zh_TW )
#define Multiplier_Lower_ko Multiplier_Upper_zh_TW
#define Multiplier_Upper_ko Multiplier_Upper_zh_TW
TRANSLITERATION_TEXTTONUM( FormalLower_ko, Lower_ko )
TRANSLITERATION_TEXTTONUM( FormalUpper_ko, Upper_ko )
TRANSLITERATION_TEXTTONUM( FormalHangul_ko, Hangul_ko )
TRANSLITERATION_TEXTTONUM( InformalLower_ko, Lower_ko )
TRANSLITERATION_TEXTTONUM( InformalUpper_ko, Upper_ko )
TRANSLITERATION_TEXTTONUM( InformalHangul_ko, Hangul_ko )
TRANSLITERATION_TEXTTONUM( KanjiLongTraditional_ja_JP, Traditional_ja )
TRANSLITERATION_TEXTTONUM( KanjiLongModern_ja_JP, Modern_ja )
#undef TRANSLITERATION_TEXTTONUM

} } } }
