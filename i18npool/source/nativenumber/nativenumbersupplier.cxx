/*************************************************************************
 *
 *  $RCSfile: nativenumbersupplier.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 16:00:07 $
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

#include <rtl/ustrbuf.hxx>
#include <tools/string.hxx>
#include <nativenumbersupplier.hxx>
#include <localedata.hxx>
#include <data/numberchar.h>
#include <x_rtl_ustring.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::lang;
using namespace ::drafts::com::sun::star::i18n;
using namespace ::rtl;


typedef struct {
    sal_Int16 number;
    sal_Unicode *multiplierChar;
    sal_Int16 numberFlag;
    sal_Int16 exponentCount;
    sal_Int16 *multiplierExponent;
} Number;


#define NUMBER_OMIT_ZERO (1 << 0)
#define NUMBER_OMIT_ONLY_ZERO  (1 << 1)
#define NUMBER_OMIT_ONE_1  (1 << 2)
#define NUMBER_OMIT_ONE_2  (1 << 3)
#define NUMBER_OMIT_ONE_3  (1 << 4)
#define NUMBER_OMIT_ONE_4  (1 << 5)
#define NUMBER_OMIT_ONE_5  (1 << 6)
#define NUMBER_OMIT_ONE_6  (1 << 7)
#define NUMBER_OMIT_ONE_7  (1 << 8)
#define NUMBER_OMIT_ONE  (NUMBER_OMIT_ONE_1|NUMBER_OMIT_ONE_2|NUMBER_OMIT_ONE_3|NUMBER_OMIT_ONE_4|NUMBER_OMIT_ONE_5|NUMBER_OMIT_ONE_6|NUMBER_OMIT_ONE_7)
#define NUMBER_OMIT_ONE_CHECK(bit)  (1 << (2 + bit))
#define NUMBER_OMIT_ALL ( NUMBER_OMIT_ZERO|NUMBER_OMIT_ONE|NUMBER_OMIT_ONLY_ZERO )
#define NUMBER_OMIT_ZERO_ONE ( NUMBER_OMIT_ZERO|NUMBER_OMIT_ONE )
#define NUMBER_OMIT_ONE_67 (NUMBER_OMIT_ONE_6|NUMBER_OMIT_ONE_7)
#define NUMBER_OMIT_ZERO_ONE_67 ( NUMBER_OMIT_ZERO|NUMBER_OMIT_ONE_67 )


#define NUMBER_COMMA    0x002C
#define isSeparator(ch)     (ch == thousandSeparator || ch == (thousandSeparator + 0xFEE0))
#define MAX_SAL_UINT32  0xFFFFFFFF
#define MAX_VALUE       (MAX_SAL_UINT32 - 9) / 10

namespace com { namespace sun { namespace star { namespace i18n {

static sal_Unicode thousandSeparator = NUMBER_COMMA;    // default separator

OUString SAL_CALL AsciiToNativeChar( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset, sal_Bool useOffset, sal_Int16 number ) throw(RuntimeException)
{
        const sal_Unicode *src = inStr.getStr() + startPos;
        rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount);
        if (useOffset)
            offset.realloc(nCount);

        for (sal_Int32 i = 0; i < nCount; i++) {
            sal_Unicode ch = src[i];
            newStr->buffer[i] = (isNumber(ch) ? NumberChar[number][ ch - NUMBER_ZERO ] :
                isDecimal(ch) ? DecimalChar[number] : isMinus(ch) ? MinusChar[number] :
                ! isSeparator(ch) ? ch : number == NumberChar_FullWidth ? thousandSeparator + 0xFEE0 : SeparatorChar[number]);
            if (useOffset)
                offset[i] = startPos + i;
        }
        return OUString(newStr->buffer, nCount);
}

sal_Bool SAL_CALL AsciiToNative_numberMaker(const sal_Unicode *str, sal_Int32 begin, sal_Int32 len,
        sal_Unicode *dst, sal_Int32& count, sal_Int16 multiChar_index, Sequence< sal_Int32 >& offset, sal_Bool useOffset, sal_Int32 startPos,
 Number *number, sal_Unicode* numberChar)
{
        sal_Unicode multiChar = (multiChar_index == -1 ? 0 : number->multiplierChar[multiChar_index]);
        if ( len <= number->multiplierExponent[number->exponentCount-1] ) {
            if (number->multiplierExponent[number->exponentCount-1] > 1) {
                sal_Int16 i;
                sal_Bool notZero = false;
                for (i = 0; i < len; i++, begin++) {
                    if (notZero || str[begin] != NUMBER_ZERO) {
                        dst[count] = numberChar[str[begin] - NUMBER_ZERO];
                        if (useOffset)
                            offset[count++] = begin + startPos;
                        notZero = sal_True;
                    }
                }
                if (notZero && multiChar > 0) {
                    dst[count] = multiChar;
                    if (useOffset)
                        offset[count++] = begin + startPos;
                }
                return notZero;
            } else if (str[begin] != NUMBER_ZERO) {
                if (!(number->numberFlag & (multiChar_index < 0 ? 0 : NUMBER_OMIT_ONE_CHECK(multiChar_index))) || str[begin] != NUMBER_ONE) {
                    dst[count] = numberChar[str[begin] - NUMBER_ZERO];
                    if (useOffset)
                        offset[count++] = begin + startPos;
                }
                if (multiChar > 0) {
                    dst[count] = multiChar;
                    if (useOffset)
                        offset[count++] = begin + startPos;
                }
            } else if (!(number->numberFlag & NUMBER_OMIT_ZERO) && count > 0 && dst[count-1] != numberChar[0]) {
                dst[count] = numberChar[0];
                if (useOffset)
                    offset[count++] = begin + startPos;
            }
            return str[begin] != NUMBER_ZERO;
        } else {
            sal_Bool printPower = sal_False;
            sal_Int16 last = 0;
            for (sal_Int16 i = 1; i <= number->exponentCount; i++) {
                sal_Int32 tmp = len - (i == number->exponentCount ? 0 : number->multiplierExponent[i]);
                if (tmp > 0) {
                    printPower |= AsciiToNative_numberMaker(str, begin, tmp, dst, count,
                        (i == number->exponentCount ? -1 : i), offset, useOffset, startPos, number, numberChar);
                    begin += tmp;
                    len -= tmp;
                }
            }
            if (printPower) {
                if (count > 0 && number->multiplierExponent[number->exponentCount-1] == 1 &&
                            dst[count-1] == numberChar[0])
                    count--;
                if (multiChar > 0) {
                    dst[count] = multiChar;
                    if (useOffset)
                        offset[count++] = begin + startPos;
                }
            }
            return printPower;
        }
}

OUString SAL_CALL AsciiToNative( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset, sal_Bool useOffset, Number* number ) throw(RuntimeException)
{
        sal_Int32 strLen = inStr.getLength() - startPos;
        sal_Unicode *numberChar = NumberChar[number->number];

        if (nCount > strLen)
            nCount = strLen;

        if (nCount > 0) {
            const sal_Unicode *str = inStr.getStr() + startPos;
            rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount * 2);
            rtl_uString *srcStr = x_rtl_uString_new_WithLength(nCount); // for keeping number without comma
            sal_Int32 i, len = 0, count = 0;

            if (useOffset)
                offset.realloc( nCount * 2 );
            sal_Bool doDecimal = sal_False;

            for (i = 0; i <= nCount; i++) {
                if (i < nCount && isNumber(str[i])) {
                    if (doDecimal) {
                        newStr->buffer[count] = numberChar[str[i] - NUMBER_ZERO];
                        if (useOffset)
                            offset[count++] = i + startPos;
                    }
                    else
                        srcStr->buffer[len++] = str[i];
                } else {
                    if (len > 0) {
                        if (isSeparator(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                            continue; // skip comma inside number string
                        sal_Bool notZero = sal_False;
                        for (sal_Int32 begin = 0, end = len % number->multiplierExponent[0];
                                end <= len; begin = end, end += number->multiplierExponent[0]) {
                            if (end == 0) continue;
                            sal_Int32 _count = count;
                            notZero |= AsciiToNative_numberMaker(srcStr->buffer, begin, end - begin, newStr->buffer, count,
                                        end == len ? -1 : 0, offset, useOffset, i - len + startPos, number, numberChar);
                            if (count > 0 && number->multiplierExponent[number->exponentCount-1] == 1 &&
                                        newStr->buffer[count-1] == numberChar[0])
                                count--;
                            if (notZero && _count == count) {
                                if (end != len) {
                                    newStr->buffer[count] = number->multiplierChar[0];
                                    if (useOffset)
                                        offset[count++] = i - len + startPos;
                                }
                            }
                        }
                        if (! notZero && ! (number->numberFlag & NUMBER_OMIT_ONLY_ZERO)) {
                            newStr->buffer[count] = numberChar[0];
                            if (useOffset)
                                offset[count++] = i - len + startPos;
                        }
                        len = 0;
                    }
                    if (i < nCount) {
                        if (doDecimal = (!doDecimal && isDecimal(str[i]) && i < nCount-1 && isNumber(str[i+1])))
                            newStr->buffer[count] = DecimalChar[number->number];
                        else if (isMinus(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                            newStr->buffer[count] = MinusChar[number->number];
                        else
                            newStr->buffer[count] = str[i];
                        if (useOffset)
                            offset[count++] = i + startPos;
                    }
                }
            }

            if (useOffset)
                offset.realloc(count);
            return OUString(newStr->buffer, count);
        }
        return OUString();
}
static void SAL_CALL NativeToAscii_numberMaker(sal_Int16 max, sal_Int16 prev, const sal_Unicode *str,
        sal_Int32& i, sal_Int32 nCount, sal_Unicode *dst, sal_Int32& count, Sequence< sal_Int32 >& offset, sal_Bool useOffset,
        OUString& numberChar, OUString& multiplierChar)
{
        sal_Int16 curr = 0, num = 0, end = 0, shift = 0;
        while (++i < nCount) {
            if ((curr = numberChar.indexOf(str[i])) >= 0) {
                if (num > 0)
                    break;
                num = curr % 10;
            } else if ((curr = multiplierChar.indexOf(str[i])) >= 0) {
                curr = MultiplierExponent_7_CJK[curr % ExponentCount_7_CJK];
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
                    if (useOffset)
                        offset[count++] = i;
                }
                if (shift) {
                    count -= max;
                    for (sal_Int16 j = 0; j < shift; j++, count++) {
                        dst[count] = dst[count + curr];
                        if (useOffset)
                            offset[count] = offset[count + curr];
                    }
                    max = curr;
                }
                NativeToAscii_numberMaker(max, curr, str, i, nCount, dst,
                        count, offset, useOffset, numberChar, multiplierChar);
                return;
            } else
                break;
        }
        while (end++ < prev) {
            dst[count] = NUMBER_ZERO + (end == prev ? num : 0);
            if (useOffset)
                offset[count++] = i - 1;
        }
}

static OUString SAL_CALL NativeToAscii(const OUString& inStr,
        sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, sal_Bool useOffset ) throw(RuntimeException)
{
        sal_Int32 strLen = inStr.getLength() - startPos;

        if (nCount > strLen)
            nCount = strLen;

        if (nCount > 0) {
            const sal_Unicode *str = inStr.getStr() + startPos;
            rtl_uString *newStr = x_rtl_uString_new_WithLength(nCount * MultiplierExponent_7_CJK[0] + 1);
            if (useOffset)
                offset.realloc( nCount * MultiplierExponent_7_CJK[0] + 1 );
            sal_Int32 count = 0, index;

            OUString numberChar, multiplierChar, decimalChar, minusChar;
            numberChar = OUString((sal_Unicode*)NumberChar, 10*NumberChar_Count);
            multiplierChar = OUString((sal_Unicode*) MultiplierChar_7_CJK, ExponentCount_7_CJK*Multiplier_Count);
            decimalChar = OUString(DecimalChar, NumberChar_Count);
            minusChar = OUString(MinusChar, NumberChar_Count);

            for (sal_Int32 i = 0; i < nCount; i++) {
                if ((index = multiplierChar.indexOf(str[i])) >= 0) {
                    if (count == 0 || !isNumber(newStr->buffer[count-1])) { // add 1 in front of multiplier
                        newStr->buffer[count] = NUMBER_ONE;
                        if (useOffset)
                            offset[count++] = i;
                    }
                    index = MultiplierExponent_7_CJK[index % ExponentCount_7_CJK];
                    NativeToAscii_numberMaker(index, index, str, i, nCount, newStr->buffer, count, offset, useOffset,
                                numberChar, multiplierChar);
                } else {
                    if ((index = numberChar.indexOf(str[i])) >= 0)
                        newStr->buffer[count] = (index % 10) + NUMBER_ZERO;
                    else if ((isSeparator(str[i]) || str[i] == 0x3001) &&
                            (i < nCount-1 && (numberChar.indexOf(str[i+1]) >= 0 ||
                                            multiplierChar.indexOf(str[i+1]) >= 0)))
                        newStr->buffer[count] = thousandSeparator;
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
                    if (useOffset)
                        offset[count++] = i;
                }
            }

            if (useOffset) {
                offset.realloc(count);
                for (i = 0; i < count; i++)
                    offset[i] += startPos;
            }
            return OUString(newStr->buffer, count);
        }
        return OUString();
}

static Number natnum4[4] = {
        { NumberChar_Lower_zh, MultiplierChar_6_CJK[Multiplier_Lower_zh], 0,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Lower_zh, MultiplierChar_6_CJK[Multiplier_Lower_zh_TW], 0,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Modern_ja, MultiplierChar_7_CJK[Multiplier_Modern_ja], NUMBER_OMIT_ZERO_ONE_67,
                ExponentCount_7_CJK, MultiplierExponent_7_CJK },
        { NumberChar_Lower_ko, MultiplierChar_6_CJK[Multiplier_Lower_ko], NUMBER_OMIT_ZERO,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
};

static Number natnum5[4] = {
        { NumberChar_Upper_zh, MultiplierChar_6_CJK[Multiplier_Upper_zh], 0,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Upper_zh_TW, MultiplierChar_6_CJK[Multiplier_Upper_zh_TW], 0,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Traditional_ja, MultiplierChar_7_CJK[Multiplier_Traditional_ja], NUMBER_OMIT_ZERO_ONE_67,
                ExponentCount_7_CJK, MultiplierExponent_7_CJK },
        { NumberChar_Upper_ko, MultiplierChar_6_CJK[Multiplier_Upper_zh_TW], NUMBER_OMIT_ZERO,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
};

static Number natnum6[4] = {
        { NumberChar_FullWidth, MultiplierChar_6_CJK[Multiplier_Lower_zh], 0,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_FullWidth, MultiplierChar_6_CJK[Multiplier_Lower_zh_TW], 0,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_FullWidth, MultiplierChar_7_CJK[Multiplier_Modern_ja], NUMBER_OMIT_ZERO_ONE_67,
                ExponentCount_7_CJK, MultiplierExponent_7_CJK },
        { NumberChar_FullWidth, MultiplierChar_6_CJK[Multiplier_Hangul_ko], NUMBER_OMIT_ZERO,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
};

static Number natnum7[4] = {
        { NumberChar_Lower_zh, MultiplierChar_6_CJK[Multiplier_Lower_zh], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Lower_zh, MultiplierChar_6_CJK[Multiplier_Lower_zh_TW], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Modern_ja, MultiplierChar_2_CJK[Multiplier_Modern_ja], NUMBER_OMIT_ZERO_ONE,
                ExponentCount_2_CJK, MultiplierExponent_2_CJK },
        { NumberChar_Lower_ko, MultiplierChar_6_CJK[Multiplier_Lower_ko], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
};

static Number natnum8[4] = {
        { NumberChar_Upper_zh, MultiplierChar_6_CJK[Multiplier_Upper_zh], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Upper_zh_TW, MultiplierChar_6_CJK[Multiplier_Upper_zh_TW], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
        { NumberChar_Traditional_ja, MultiplierChar_2_CJK[Multiplier_Traditional_ja], NUMBER_OMIT_ZERO_ONE,
                ExponentCount_2_CJK, MultiplierExponent_2_CJK },
        { NumberChar_Upper_ko, MultiplierChar_6_CJK[Multiplier_Upper_zh_TW], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK },
};

static Number natnum10 = { NumberChar_Hangul_ko, MultiplierChar_6_CJK[Multiplier_Hangul_ko], NUMBER_OMIT_ZERO,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK };
static Number natnum11 = { NumberChar_Hangul_ko, MultiplierChar_6_CJK[Multiplier_Hangul_ko], NUMBER_OMIT_ALL,
                ExponentCount_6_CJK, MultiplierExponent_6_CJK };

static sal_Char *natnum1Locales[] = { "zh_CN", "zh_TW", "ja", "ko", "ar", "th", "hi", "or", "mr", "bn", "pa", "gu", "ta", "te", "kn", "ml", "lo", "bo", "my", "km", "mn" };
static sal_Int16 nbOfLocale = sizeof(natnum1Locales)/sizeof(natnum1Locales[0]);

static sal_Int16 natnum1[] = { NumberChar_Lower_zh, NumberChar_Lower_zh, NumberChar_Modern_ja, NumberChar_Lower_ko,
                NumberChar_Indic_ar, NumberChar_th, NumberChar_hi, NumberChar_or, NumberChar_mr, NumberChar_bn,
                NumberChar_pa, NumberChar_gu, NumberChar_ta, NumberChar_te, NumberChar_kn, NumberChar_ml,
                NumberChar_lo, NumberChar_bo, NumberChar_my, NumberChar_km, NumberChar_mn };
static sal_Int16 sizeof_natnum1 = sizeof(natnum1)/sizeof(natnum1[0]);
static sal_Int16 natnum2[] = { NumberChar_Upper_zh, NumberChar_Upper_zh_TW, NumberChar_Traditional_ja,
                NumberChar_Upper_ko };
static sal_Int16 sizeof_natnum2 = sizeof(natnum2)/sizeof(natnum2[0]);

#define isLang(lang) rLocale.Language.equalsAsciiL(lang, 2)
#define isCtry(ctry) rLocale.Country.equalsAsciiL(ctry, 2)

static sal_Int16 SAL_CALL getLanguageNumber( const Locale& rLocale)
{
    // return zh_TW for TW, HK and MO, return zh_CN for other zh locales.
    if (isLang("zh")) return (isCtry("TW") || isCtry("HK") || isCtry("MO")) ? 1 : 0;

    for (sal_Int16 i = 2; i < nbOfLocale; i++)
        if (isLang(natnum1Locales[i]))
            return i;

    return -1;
}

OUString SAL_CALL NativeNumberSupplier::getNativeNumberString(const OUString& aNumberString, const Locale& rLocale,
                sal_Int16 nNativeNumberMode, Sequence< sal_Int32 >& offset) throw (RuntimeException)
{
        Number *number = 0;
        sal_Int16 num = -1;
        if (!aLocale.Language.equals(rLocale.Language) ||
                !aLocale.Country.equals(rLocale.Country) ||
                !aLocale.Variant.equals(rLocale.Variant)) {
            LocaleDataItem item = LocaleData().getLocaleItem( rLocale );
            aLocale = rLocale;
            DecimalChar[NumberChar_HalfWidth] = item.decimalSeparator.toChar();
            if (DecimalChar[NumberChar_HalfWidth] > 0x7E || DecimalChar[NumberChar_HalfWidth] < 0x21)
                DecimalChar[NumberChar_HalfWidth] = 0x002C;
            DecimalChar[NumberChar_FullWidth] = DecimalChar[NumberChar_HalfWidth] + 0xFEE0;
            thousandSeparator = item.thousandSeparator.toChar();
        }

        if (isValidNatNum(rLocale, nNativeNumberMode)) {
            sal_Int16 langnum = getLanguageNumber(rLocale);
            switch (nNativeNumberMode) {
                case NativeNumberMode::NATNUM0: // Ascii
                    return NativeToAscii(aNumberString,  0, aNumberString.getLength(), offset, useOffset);
                break;
                case NativeNumberMode::NATNUM1: // Char, Lower
                    num = natnum1[langnum];
                break;
                case NativeNumberMode::NATNUM2: // Char, Upper
                    num = natnum2[langnum];
                break;
                case NativeNumberMode::NATNUM3: // Char, FullWidth
                    num = NumberChar_FullWidth;
                break;
                case NativeNumberMode::NATNUM4: // Text, Lower, Long
                    number = &natnum4[langnum];
                break;
                case NativeNumberMode::NATNUM5: // Text, Upper, Long
                    number = &natnum5[langnum];
                break;
                case NativeNumberMode::NATNUM6: // Text, FullWidth
                    number = &natnum6[langnum];
                break;
                case NativeNumberMode::NATNUM7: // Text. Lower, Short
                    number = &natnum7[langnum];
                break;
                case NativeNumberMode::NATNUM8: // Text, Upper, Short
                    number = &natnum8[langnum];
                break;
                case NativeNumberMode::NATNUM9: // Char, Hangul
                    num = NumberChar_Hangul_ko;
                break;
                case NativeNumberMode::NATNUM10:        // Text, Hangul, Long
                    number = &natnum10;
                break;
                case NativeNumberMode::NATNUM11:        // Text, Hangul, Short
                    number = &natnum11;
                break;
                default:
                break;
            }
        }
        if (number)
            return AsciiToNative( aNumberString, 0, aNumberString.getLength(), offset, useOffset, number );
        else if (num >= 0)
            return AsciiToNativeChar(aNumberString, 0, aNumberString.getLength(), offset, useOffset, num);
        else
            return aNumberString;
}

OUString SAL_CALL NativeNumberSupplier::getNativeNumberString(const OUString& aNumberString, const Locale& rLocale,
                sal_Int16 nNativeNumberMode) throw (RuntimeException)
{
    Sequence< sal_Int32 > offset;
    return getNativeNumberString(aNumberString, rLocale, nNativeNumberMode, offset);
}

sal_Unicode SAL_CALL NativeNumberSupplier::getNativeNumberChar( const sal_Unicode inChar, const Locale& rLocale, sal_Int16 nNativeNumberMode ) throw(com::sun::star::uno::RuntimeException)
{
        if (nNativeNumberMode == NativeNumberMode::NATNUM0) { // Ascii
            for (sal_Int16 i = 0; i < NumberChar_Count; i++)
                for (sal_Int16 j = 0; j < 10; j++)
                    if (inChar == NumberChar[i][j])
                        return j;
            return inChar;
        }
        else if (isNumber(inChar) && isValidNatNum(rLocale, nNativeNumberMode)) {
            sal_Int16 langnum = getLanguageNumber(rLocale);
            switch (nNativeNumberMode) {
                case NativeNumberMode::NATNUM1: // Char, Lower
                case NativeNumberMode::NATNUM4: // Text, Lower, Long
                case NativeNumberMode::NATNUM7: // Text. Lower, Short
                    return NumberChar[natnum1[langnum]][inChar - NUMBER_ZERO];
                case NativeNumberMode::NATNUM2: // Char, Upper
                case NativeNumberMode::NATNUM5: // Text, Upper, Long
                case NativeNumberMode::NATNUM8: // Text, Upper, Short
                    return NumberChar[natnum2[langnum]][inChar - NUMBER_ZERO];
                case NativeNumberMode::NATNUM3: // Char, FullWidth
                case NativeNumberMode::NATNUM6: // Text, FullWidth
                    return NumberChar[NumberChar_FullWidth][inChar - NUMBER_ZERO];
                break;
                case NativeNumberMode::NATNUM9: // Char, Hangul
                case NativeNumberMode::NATNUM10:        // Text, Hangul, Long
                case NativeNumberMode::NATNUM11:        // Text, Hangul, Short
                    return NumberChar[NumberChar_Hangul_ko][inChar - NUMBER_ZERO];
                default:
                break;
            }
        }
        return inChar;
}

sal_Bool SAL_CALL NativeNumberSupplier::isValidNatNum( const Locale& aLocale, sal_Int16 nNativeNumberMode ) throw (RuntimeException)
{
        sal_Int16 langnum = getLanguageNumber(aLocale);

        switch (nNativeNumberMode) {
            case NativeNumberMode::NATNUM0:     // Ascii
            case NativeNumberMode::NATNUM3:     // Char, FullWidth
                return sal_True;
            break;
            case NativeNumberMode::NATNUM1:     // Char, Lower
                return (langnum >= 0);
            break;
            case NativeNumberMode::NATNUM2:     // Char, Upper
            case NativeNumberMode::NATNUM4:     // Text, Lower, Long
            case NativeNumberMode::NATNUM5:     // Text, Upper, Long
            case NativeNumberMode::NATNUM6:     // Text, FullWidth
            case NativeNumberMode::NATNUM7:     // Text. Lower, Short
            case NativeNumberMode::NATNUM8:     // Text, Upper, Short
                return (langnum >= 0 && langnum < 4);
            break;
            case NativeNumberMode::NATNUM9:     // Char, Hangul
            case NativeNumberMode::NATNUM10:    // Text, Hangul, Long
            case NativeNumberMode::NATNUM11:    // Text, Hangul, Short
                return (langnum == 3);
            break;
        }
        return sal_False;
}

NativeNumberXmlAttributes SAL_CALL NativeNumberSupplier::convertToXmlAttributes( const Locale& aLocale, sal_Int16 nNativeNumberMode ) throw (RuntimeException)
{
        static const sal_Int16 attShort         = 0;
        static const sal_Int16 attMedium        = 1;
        static const sal_Int16 attLong          = 2;
        static sal_Char *attType[] = { "short", "medium", "long" };

        sal_Int16 number = NumberChar_HalfWidth, type = attShort;

        if (isValidNatNum(aLocale, nNativeNumberMode)) {
            sal_Int16 langnum = getLanguageNumber(aLocale);
            switch (nNativeNumberMode) {
                case NativeNumberMode::NATNUM0: // Ascii
                    number = NumberChar_HalfWidth;
                    type = attShort;
                break;
                case NativeNumberMode::NATNUM1: // Char, Lower
                    number = natnum1[langnum];
                    type = attShort;
                break;
                case NativeNumberMode::NATNUM2: // Char, Upper
                    number = natnum2[langnum];
                    type = attShort;
                break;
                case NativeNumberMode::NATNUM3: // Char, FullWidth
                    number = NumberChar_FullWidth;
                    type = attShort;
                break;
                case NativeNumberMode::NATNUM4: // Text, Lower, Long
                    number = natnum1[langnum];
                    type = attLong;
                break;
                case NativeNumberMode::NATNUM5: // Text, Upper, Long
                    number = natnum2[langnum];
                    type = attLong;
                break;
                case NativeNumberMode::NATNUM6: // Text, FullWidth
                    number = NumberChar_FullWidth;
                    type = attLong;
                break;
                case NativeNumberMode::NATNUM7: // Text. Lower, Short
                    number = natnum1[langnum];
                    type = attMedium;
                break;
                case NativeNumberMode::NATNUM8: // Text, Upper, Short
                    number = natnum2[langnum];
                    type = attMedium;
                break;
                case NativeNumberMode::NATNUM9: // Char, Hangul
                    number = NumberChar_Hangul_ko;
                    type = attShort;
                break;
                case NativeNumberMode::NATNUM10:        // Text, Hangul, Long
                    number = NumberChar_Hangul_ko;
                    type = attLong;
                break;
                case NativeNumberMode::NATNUM11:        // Text, Hangul, Short
                    number = NumberChar_Hangul_ko;
                    type = attMedium;
                break;
                default:
                break;
            }
        }
        NativeNumberXmlAttributes att(aLocale, OUString(NumberChar[number] + 1, 1),
                                            OUString::createFromAscii(attType[type]));
        return att;
}

static sal_Bool natNumIn(sal_Int16 num, sal_Int16 natnum[], sal_Int16 len)
{
        for (sal_Int16 i = 0; i < len; i++)
            if (natnum[i] == num)
                return sal_True;
        return sal_False;
}

sal_Int16 SAL_CALL NativeNumberSupplier::convertFromXmlAttributes( const NativeNumberXmlAttributes& aAttr ) throw (RuntimeException)
{
        sal_Unicode numberChar[NumberChar_Count];
        for (sal_Int16 i = 0; i < NumberChar_Count; i++)
            numberChar[i] = NumberChar[i][1];
        OUString number(numberChar, NumberChar_Count);

        sal_Int16 num = number.indexOf(aAttr.Format);

        if (aAttr.Style.equalsAscii("short")) {
            if (num == NumberChar_FullWidth)
                return NativeNumberMode::NATNUM3;
            else if (num == NumberChar_Hangul_ko)
                return NativeNumberMode::NATNUM9;
            else if (natNumIn(num, natnum1, sizeof_natnum1))
                return NativeNumberMode::NATNUM1;
            else if (natNumIn(num, natnum2, sizeof_natnum2))
                return NativeNumberMode::NATNUM2;
        } else if (aAttr.Style.equalsAscii("medium")) {
            if (num == NumberChar_Hangul_ko)
                return NativeNumberMode::NATNUM11;
            else if (natNumIn(num, natnum1, sizeof_natnum1))
                return NativeNumberMode::NATNUM7;
            else if (natNumIn(num, natnum2, sizeof_natnum2))
                return NativeNumberMode::NATNUM8;
        } else if (aAttr.Style.equalsAscii("long")) {
            if (num == NumberChar_FullWidth)
                return NativeNumberMode::NATNUM6;
            else if (num == NumberChar_Hangul_ko)
                return NativeNumberMode::NATNUM10;
            else if (natNumIn(num, natnum1, sizeof_natnum1))
                return NativeNumberMode::NATNUM4;
            else if (natNumIn(num, natnum2, sizeof_natnum2))
                return NativeNumberMode::NATNUM5;
        } else {
            throw RuntimeException();
        }
        return NativeNumberMode::NATNUM0;
}

static sal_Char* implementationName = "com.sun.star.i18n.NativeNumberSupplier";

OUString SAL_CALL NativeNumberSupplier::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii( implementationName );
}

sal_Bool SAL_CALL
NativeNumberSupplier::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return rServiceName.compareToAscii(implementationName) == 0;
}

Sequence< OUString > SAL_CALL
NativeNumberSupplier::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii( implementationName );
    return aRet;
}

} } } }
