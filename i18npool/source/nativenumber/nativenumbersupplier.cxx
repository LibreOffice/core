/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <i18npool/mslangid.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/macros.h>
#include <nativenumbersupplier.hxx>
#include <localedata.hxx>
#include <data/numberchar.h>
#include <comphelper/string.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::lang;
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


#define MAX_SAL_UINT32  0xFFFFFFFF
#define MAX_VALUE       (MAX_SAL_UINT32 - 9) / 10

namespace com { namespace sun { namespace star { namespace i18n {

OUString SAL_CALL getHebrewNativeNumberString(const OUString& aNumberString, sal_Bool useGeresh);

OUString SAL_CALL AsciiToNativeChar( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset, sal_Bool useOffset, sal_Int16 number ) throw(RuntimeException)
{
        const sal_Unicode *src = inStr.getStr() + startPos;
        rtl_uString *newStr = comphelper::string::rtl_uString_alloc(nCount);
        if (useOffset)
            offset.realloc(nCount);

        for (sal_Int32 i = 0; i < nCount; i++)
        {
            sal_Unicode ch = src[i];
            if (isNumber(ch))
                newStr->buffer[i] = NumberChar[number][ ch - NUMBER_ZERO ];
            else if (i+1 < nCount && isNumber(src[i+1])) {
                if (i > 0 && isNumber(src[i-1]) && isSeparator(ch))
                    newStr->buffer[i] = SeparatorChar[number] ? SeparatorChar[number] : ch;
                else
                    newStr->buffer[i] = isDecimal(ch) ? (DecimalChar[number] ? DecimalChar[number] : ch) :
                            isMinus(ch) ? (MinusChar[number] ? MinusChar[number] : ch) : ch;
            }
            else
                newStr->buffer[i] = ch;
            if (useOffset)
                offset[i] = startPos + i;
        }
        return OUString(newStr, SAL_NO_ACQUIRE); // take ownership
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
                            offset[count] = begin + startPos;
                        count++;
                        notZero = sal_True;
                    }
                }
                if (notZero && multiChar > 0) {
                    dst[count] = multiChar;
                    if (useOffset)
                        offset[count] = begin + startPos;
                    count++;
                }
                return notZero;
            } else if (str[begin] != NUMBER_ZERO) {
                if (!(number->numberFlag & (multiChar_index < 0 ? 0 : NUMBER_OMIT_ONE_CHECK(multiChar_index))) || str[begin] != NUMBER_ONE) {
                    dst[count] = numberChar[str[begin] - NUMBER_ZERO];
                    if (useOffset)
                        offset[count] = begin + startPos;
                    count++;
                }
                if (multiChar > 0) {
                    dst[count] = multiChar;
                    if (useOffset)
                        offset[count] = begin + startPos;
                    count++;
                }
            } else if (!(number->numberFlag & NUMBER_OMIT_ZERO) && count > 0 && dst[count-1] != numberChar[0]) {
                dst[count] = numberChar[0];
                if (useOffset)
                    offset[count] = begin + startPos;
                count++;
            }
            return str[begin] != NUMBER_ZERO;
        } else {
            sal_Bool printPower = sal_False;
            // sal_Int16 last = 0;
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
                        offset[count] = begin + startPos;
                    count++;
                }
            }
            return printPower;
        }
}

OUString SAL_CALL AsciiToNative( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        Sequence< sal_Int32 >& offset, sal_Bool useOffset, Number* number ) throw(RuntimeException)
{
        rtl::OUString aRet;

        sal_Int32 strLen = inStr.getLength() - startPos;
        sal_Unicode *numberChar = NumberChar[number->number];

        if (nCount > strLen)
            nCount = strLen;

        if (nCount > 0)
        {
            const sal_Unicode *str = inStr.getStr() + startPos;
            sal_Unicode *newStr = new sal_Unicode[nCount * 2 + 1];
            sal_Unicode *srcStr = new sal_Unicode[nCount + 1]; // for keeping number without comma
            sal_Int32 i, len = 0, count = 0;

            if (useOffset)
                offset.realloc( nCount * 2 );
            sal_Bool doDecimal = sal_False;

            for (i = 0; i <= nCount; i++)
            {
                if (i < nCount && isNumber(str[i])) {
                    if (doDecimal) {
                        newStr[count] = numberChar[str[i] - NUMBER_ZERO];
                        if (useOffset)
                            offset[count] = i + startPos;
                        count++;
                    }
                    else
                        srcStr[len++] = str[i];
                } else {
                    if (len > 0) {
                        if (isSeparator(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                            continue; // skip comma inside number string
                        sal_Bool notZero = sal_False;
                        for (sal_Int32 begin = 0, end = len % number->multiplierExponent[0];
                                end <= len; begin = end, end += number->multiplierExponent[0]) {
                            if (end == 0) continue;
                            sal_Int32 _count = count;
                            notZero |= AsciiToNative_numberMaker(srcStr, begin, end - begin, newStr, count,
                                        end == len ? -1 : 0, offset, useOffset, i - len + startPos, number, numberChar);
                            if (count > 0 && number->multiplierExponent[number->exponentCount-1] == 1 &&
                                        newStr[count-1] == numberChar[0])
                                count--;
                            if (notZero && _count == count) {
                                if (end != len) {
                                    newStr[count] = number->multiplierChar[0];
                                    if (useOffset)
                                        offset[count] = i - len + startPos;
                                    count++;
                                }
                            }
                        }
                        if (! notZero && ! (number->numberFlag & NUMBER_OMIT_ONLY_ZERO)) {
                            newStr[count] = numberChar[0];
                            if (useOffset)
                                offset[count] = i - len + startPos;
                            count++;
                        }
                        len = 0;
                    }
                    if (i < nCount) {
                        if ((doDecimal = (!doDecimal && isDecimal(str[i]) && i < nCount-1 && isNumber(str[i+1]))) != sal_False)
                            newStr[count] = (DecimalChar[number->number] ? DecimalChar[number->number] : str[i]);
                        else if (isMinus(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                            newStr[count] = (MinusChar[number->number] ? MinusChar[number->number] : str[i]);
                        else if (isSeparator(str[i]) && i < nCount-1 && isNumber(str[i+1]))
                            newStr[count] = (SeparatorChar[number->number] ? SeparatorChar[number->number] : str[i]);
                        else
                            newStr[count] = str[i];
                        if (useOffset)
                            offset[count] = i + startPos;
                        count++;
                    }
                }
            }

            delete[] srcStr;

            if (useOffset)
                offset.realloc(count);
            aRet = OUString(newStr, count);
            delete[] newStr;
        }
        return aRet;
}
static void SAL_CALL NativeToAscii_numberMaker(sal_Int16 max, sal_Int16 prev, const sal_Unicode *str,
        sal_Int32& i, sal_Int32 nCount, sal_Unicode *dst, sal_Int32& count, Sequence< sal_Int32 >& offset, sal_Bool useOffset,
        OUString& numberChar, OUString& multiplierChar)
{
        sal_Int16 curr = 0, num = 0, end = 0, shift = 0;
        while (++i < nCount) {
            if ((curr = sal::static_int_cast<sal_Int16>( numberChar.indexOf(str[i]) )) >= 0) {
                if (num > 0)
                    break;
                num = curr % 10;
            } else if ((curr = sal::static_int_cast<sal_Int16>( multiplierChar.indexOf(str[i]) )) >= 0) {
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
                        offset[count] = i;
                    count++;
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
                offset[count] = i - 1;
            count++;
        }
}

static OUString SAL_CALL NativeToAscii(const OUString& inStr,
        sal_Int32 startPos, sal_Int32 nCount, Sequence< sal_Int32 >& offset, sal_Bool useOffset ) throw(RuntimeException)
{
        OUString aRet;

        sal_Int32 strLen = inStr.getLength() - startPos;

        if (nCount > strLen)
            nCount = strLen;

        if (nCount > 0) {
            const sal_Unicode *str = inStr.getStr() + startPos;
            sal_Unicode *newStr = new sal_Unicode[nCount * MultiplierExponent_7_CJK[0] + 2];
            if (useOffset)
                offset.realloc( nCount * MultiplierExponent_7_CJK[0] + 1 );
            sal_Int32 count = 0, index;
            sal_Int32 i;

            OUString numberChar, multiplierChar, decimalChar, minusChar, separatorChar;
            numberChar = OUString((sal_Unicode*)NumberChar, 10*NumberChar_Count);
            multiplierChar = OUString((sal_Unicode*) MultiplierChar_7_CJK, ExponentCount_7_CJK*Multiplier_Count);
            decimalChar = OUString(DecimalChar, NumberChar_Count);
            minusChar = OUString(MinusChar, NumberChar_Count);
            separatorChar = OUString(SeparatorChar, NumberChar_Count);

            for ( i = 0; i < nCount; i++) {
                if ((index = multiplierChar.indexOf(str[i])) >= 0) {
                    if (count == 0 || !isNumber(newStr[count-1])) { // add 1 in front of multiplier
                        newStr[count] = NUMBER_ONE;
                        if (useOffset)
                            offset[count] = i;
                        count++;
                    }
                    index = MultiplierExponent_7_CJK[index % ExponentCount_7_CJK];
                    NativeToAscii_numberMaker(
                                sal::static_int_cast<sal_Int16>( index ), sal::static_int_cast<sal_Int16>( index ),
                                str, i, nCount, newStr, count, offset, useOffset,
                                numberChar, multiplierChar);
                } else {
                    if ((index = numberChar.indexOf(str[i])) >= 0)
                        newStr[count] = sal::static_int_cast<sal_Unicode>( (index % 10) + NUMBER_ZERO );
                    else if ((index = separatorChar.indexOf(str[i])) >= 0 &&
                            (i < nCount-1 && (numberChar.indexOf(str[i+1]) >= 0 ||
                                            multiplierChar.indexOf(str[i+1]) >= 0)))
                        newStr[count] = SeparatorChar[NumberChar_HalfWidth];
                    else if ((index = decimalChar.indexOf(str[i])) >= 0 &&
                            (i < nCount-1 && (numberChar.indexOf(str[i+1]) >= 0 ||
                                            multiplierChar.indexOf(str[i+1]) >= 0)))
                        // Only when decimal point is followed by numbers,
                        // it will be convert to ASCII decimal point
                        newStr[count] = DecimalChar[NumberChar_HalfWidth];
                    else if ((index = minusChar.indexOf(str[i])) >= 0 &&
                            (i < nCount-1 && (numberChar.indexOf(str[i+1]) >= 0 ||
                                            multiplierChar.indexOf(str[i+1]) >= 0)))
                        // Only when minus is followed by numbers,
                        // it will be convert to ASCII minus sign
                        newStr[count] = MinusChar[NumberChar_HalfWidth];
                    else
                        newStr[count] = str[i];
                    if (useOffset)
                        offset[count] = i;
                    count++;
                }
            }

            if (useOffset) {
                offset.realloc(count);
                for (i = 0; i < count; i++)
                    offset[i] += startPos;
            }
            aRet = OUString(newStr, count);
            delete[] newStr;
        }
        return aRet;
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

//! ATTENTION: Do not change order of elements!
//! Append new languages to the end of the list!
static const sal_Char *natnum1Locales[] = {
    "zh_CN",
    "zh_TW",
    "ja",
    "ko",
    "he",
    "ar",
    "th",
    "hi",
    "or",
    "mr",
    "bn",
    "pa",
    "gu",
    "ta",
    "te",
    "kn",
    "ml",
    "lo",
    "bo",
    "my",
    "km",
    "mn",
    "ne",
    "dz",
    "fa"
};
static sal_Int16 nbOfLocale = SAL_N_ELEMENTS(natnum1Locales);

//! ATTENTION: Do not change order of elements!
//! Number and order must match elements of natnum1Locales!
static sal_Int16 natnum1[] = {
    NumberChar_Lower_zh,
    NumberChar_Lower_zh,
    NumberChar_Modern_ja,
    NumberChar_Lower_ko,
    NumberChar_he,
    NumberChar_Indic_ar,
    NumberChar_th,
    NumberChar_hi,
    NumberChar_or,
    NumberChar_mr,
    NumberChar_bn,
    NumberChar_pa,
    NumberChar_gu,
    NumberChar_ta,
    NumberChar_te,
    NumberChar_kn,
    NumberChar_ml,
    NumberChar_lo,
    NumberChar_bo,
    NumberChar_my,
    NumberChar_km,
    NumberChar_mn,
    NumberChar_ne,
    NumberChar_dz,
    NumberChar_EastIndic_ar
};
static sal_Int16 sizeof_natnum1 = SAL_N_ELEMENTS(natnum1);

//! ATTENTION: Do not change order of elements!
//! Order must match first elements of natnum1Locales!
static sal_Int16 natnum2[] = {
    NumberChar_Upper_zh,
    NumberChar_Upper_zh_TW,
    NumberChar_Traditional_ja,
    NumberChar_Upper_ko,
    NumberChar_he
};
static sal_Int16 sizeof_natnum2 = SAL_N_ELEMENTS(natnum2);

#define isLang(lang) rLocale.Language.equalsAsciiL(lang, 2)

static sal_Int16 SAL_CALL getLanguageNumber( const Locale& rLocale)
{
    // return zh_TW for TW, HK and MO, return zh_CN for other zh locales.
    if (isLang("zh")) return MsLangId::isTraditionalChinese(rLocale) ? 1 : 0;

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

        if (isValidNatNum(rLocale, nNativeNumberMode)) {
            sal_Int16 langnum = getLanguageNumber(rLocale);
            switch (nNativeNumberMode) {
                case NativeNumberMode::NATNUM0: // Ascii
                    return NativeToAscii(aNumberString,  0, aNumberString.getLength(), offset, useOffset);
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

        if (number || num >= 0) {
            if (!aLocale.Language.equals(rLocale.Language) ||
                    !aLocale.Country.equals(rLocale.Country) ||
                    !aLocale.Variant.equals(rLocale.Variant)) {
                LocaleDataItem item = LocaleData().getLocaleItem( rLocale );
                aLocale = rLocale;
                DecimalChar[NumberChar_HalfWidth]=item.decimalSeparator.toChar();
                if (DecimalChar[NumberChar_HalfWidth] > 0x7E || DecimalChar[NumberChar_HalfWidth] < 0x21)
                    DecimalChar[NumberChar_FullWidth]=0xFF0E;
                else
                    DecimalChar[NumberChar_FullWidth]=DecimalChar[NumberChar_HalfWidth]+0xFEE0;
                SeparatorChar[NumberChar_HalfWidth]=item.thousandSeparator.toChar();
                if (SeparatorChar[NumberChar_HalfWidth] > 0x7E || SeparatorChar[NumberChar_HalfWidth] < 0x21)
                    SeparatorChar[NumberChar_FullWidth]=0xFF0C;
                else
                    SeparatorChar[NumberChar_FullWidth]=SeparatorChar[NumberChar_HalfWidth]+0xFEE0;
            }
            if (number)
                return AsciiToNative( aNumberString, 0, aNumberString.getLength(), offset, useOffset, number );
            else if (num == NumberChar_he)
                return getHebrewNativeNumberString(aNumberString,
                                nNativeNumberMode == NativeNumberMode::NATNUM2);
            else
                return AsciiToNativeChar(aNumberString, 0, aNumberString.getLength(), offset, useOffset, num);
        }
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

sal_Bool SAL_CALL NativeNumberSupplier::isValidNatNum( const Locale& rLocale, sal_Int16 nNativeNumberMode ) throw (RuntimeException)
{
        sal_Int16 langnum = getLanguageNumber(rLocale);

        switch (nNativeNumberMode) {
            case NativeNumberMode::NATNUM0:     // Ascii
            case NativeNumberMode::NATNUM3:     // Char, FullWidth
                return sal_True;
            case NativeNumberMode::NATNUM1:     // Char, Lower
                return (langnum >= 0);
            case NativeNumberMode::NATNUM2:     // Char, Upper
                if (langnum == 4) // Hebrew numbering
                    return sal_True;
            case NativeNumberMode::NATNUM4:     // Text, Lower, Long
            case NativeNumberMode::NATNUM5:     // Text, Upper, Long
            case NativeNumberMode::NATNUM6:     // Text, FullWidth
            case NativeNumberMode::NATNUM7:     // Text. Lower, Short
            case NativeNumberMode::NATNUM8:     // Text, Upper, Short
                return (langnum >= 0 && langnum < 4); // CJK numbering
            case NativeNumberMode::NATNUM9:     // Char, Hangul
            case NativeNumberMode::NATNUM10:    // Text, Hangul, Long
            case NativeNumberMode::NATNUM11:    // Text, Hangul, Short
                return (langnum == 3); // Korean numbering
        }
        return sal_False;
}

NativeNumberXmlAttributes SAL_CALL NativeNumberSupplier::convertToXmlAttributes( const Locale& rLocale, sal_Int16 nNativeNumberMode ) throw (RuntimeException)
{
        static const sal_Int16 attShort         = 0;
        static const sal_Int16 attMedium        = 1;
        static const sal_Int16 attLong          = 2;
        static const sal_Char *attType[] = { "short", "medium", "long" };

        sal_Int16 number = NumberChar_HalfWidth, type = attShort;

        if (isValidNatNum(rLocale, nNativeNumberMode)) {
            sal_Int16 langnum = getLanguageNumber(rLocale);
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
                    type = number == NumberChar_he ? attMedium : attShort;
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
        return NativeNumberXmlAttributes(rLocale, OUString(&NumberChar[number][1], 1),
                                            OUString::createFromAscii(attType[type]));
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

        sal_Int16 num = sal::static_int_cast<sal_Int16>( number.indexOf(aAttr.Format) );

        if ( aAttr.Style == "short" ) {
            if (num == NumberChar_FullWidth)
                return NativeNumberMode::NATNUM3;
            else if (num == NumberChar_Hangul_ko)
                return NativeNumberMode::NATNUM9;
            else if (natNumIn(num, natnum1, sizeof_natnum1))
                return NativeNumberMode::NATNUM1;
            else if (natNumIn(num, natnum2, sizeof_natnum2))
                return NativeNumberMode::NATNUM2;
        } else if ( aAttr.Style == "medium" ) {
            if (num == NumberChar_Hangul_ko)
                return NativeNumberMode::NATNUM11;
            else if (num == NumberChar_he)
                return NativeNumberMode::NATNUM2;
            else if (natNumIn(num, natnum1, sizeof_natnum1))
                return NativeNumberMode::NATNUM7;
            else if (natNumIn(num, natnum2, sizeof_natnum2))
                return NativeNumberMode::NATNUM8;
        } else if ( aAttr.Style == "long" ) {
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


// Following code generates Hebrew Number,
// see numerical system in the Hebrew Numbering System in following link for details,
// http://smontagu.org/writings/HebrewNumbers.html

struct HebrewNumberChar {
    sal_Unicode code;
    sal_Int16 value;
} HebrewNumberCharArray[] = {
    { 0x05ea, 400 },
    { 0x05ea, 400 },
    { 0x05e9, 300 },
    { 0x05e8, 200 },
    { 0x05e7, 100 },
    { 0x05e6, 90 },
    { 0x05e4, 80 },
    { 0x05e2, 70 },
    { 0x05e1, 60 },
    { 0x05e0, 50 },
    { 0x05de, 40 },
    { 0x05dc, 30 },
    { 0x05db, 20 },
    { 0x05d9, 10 },
    { 0x05d8, 9 },
    { 0x05d7, 8 },
    { 0x05d6, 7 },
    { 0x05d5, 6 },
    { 0x05d4, 5 },
    { 0x05d3, 4 },
    { 0x05d2, 3 },
    { 0x05d1, 2 },
    { 0x05d0, 1 }
};

static sal_Int16 nbOfHebrewNumberChar = sizeof(HebrewNumberCharArray)/sizeof(HebrewNumberChar);

static sal_Unicode thousand[] = {0x05d0, 0x05dc, 0x05e3, 0x0};
static sal_Unicode thousands[] = {0x05d0, 0x05dc, 0x05e4, 0x05d9, 0x0};
static sal_Unicode thousands_last[] = {0x05d0, 0x05dc, 0x05e4, 0x05d9, 0x05dd, 0x0};
static sal_Unicode geresh = 0x05f3;
static sal_Unicode gershayim = 0x05f4;

void makeHebrewNumber(sal_Int64 value, OUStringBuffer& output, sal_Bool isLast, sal_Bool useGeresh)
{
    sal_Int16 num = sal::static_int_cast<sal_Int16>(value % 1000);

    if (value > 1000) {
        makeHebrewNumber(value / 1000, output, num != 0, useGeresh);
        output.appendAscii(" ");
    }
    if (num == 0) {
        output.append(value == 1000 ? thousand : isLast ? thousands_last : thousands);
    } else {
        sal_Int16 nbOfChar = 0;
        for (sal_Int32 j = 0; num > 0 && j < nbOfHebrewNumberChar; j++) {
            if (num - HebrewNumberCharArray[j].value >= 0) {
                nbOfChar++;
                if (num == 15 || num == 16) // substitution for 15 and 16
                    j++;
                num = sal::static_int_cast<sal_Int16>( num - HebrewNumberCharArray[j].value );
                output.append(HebrewNumberCharArray[j].code);
            }
        }
        if (useGeresh) {
            if (nbOfChar > 1)   // a number is written as more than one character
                output.insert(output.getLength() - 1, gershayim);
            else if (nbOfChar == 1) // a number is written as a single character
                output.append(geresh);
        }
    }
}

OUString SAL_CALL getHebrewNativeNumberString(const OUString& aNumberString, sal_Bool useGeresh)
{
    sal_Int64 value = 0;
    sal_Int32 i, count = 0, len = aNumberString.getLength();
    const sal_Unicode *src = aNumberString.getStr();

    for (i = 0; i < len; i++) {
        sal_Unicode ch = src[i];
        if (isNumber(ch)) {
            if (++count >= 20) // Number is too long, could not be handled.
                return aNumberString;
            value = value * 10 + (ch - NUMBER_ZERO);
        }
        else if (isSeparator(ch) && count > 0) continue;
        else if (isMinus(ch) && count == 0) continue;
        else break;
    }

    if (value > 0) {
        OUStringBuffer output(count*2 + 2 + len - i);

        makeHebrewNumber(value, output, sal_True, useGeresh);

        if (i < len)
            output.append(aNumberString.copy(i));

        return output.makeStringAndClear();
    }
    else
        return aNumberString;
}

static const sal_Char* implementationName = "com.sun.star.i18n.NativeNumberSupplier";

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
