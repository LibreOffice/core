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

#include <assert.h>
#include <textconversion.hxx>
#include <com/sun/star/i18n/TextConversionType.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/linguistic2/ConversionDirection.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <rtl/ustrbuf.hxx>
#include <unicode/uchar.h>
#include <memory>

using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;


namespace i18npool {

#define SCRIPT_OTHERS   0
#define SCRIPT_HANJA    1
#define SCRIPT_HANGUL   2

TextConversion_ko::TextConversion_ko( const Reference < XComponentContext >& xContext )
    : TextConversionService("com.sun.star.i18n.TextConversion_ko")
{
    Reference < XInterface > xI;

    xI = xContext->getServiceManager()->createInstanceWithContext(
        "com.sun.star.i18n.ConversionDictionary_ko", xContext);

    if ( xI.is() )
        xCD.set( xI, UNO_QUERY );

    xCDL = ConversionDictionaryList::create(xContext);

    maxLeftLength = maxRightLength = 1;

    // get maximum length of word in dictionary
    if (xCDL.is()) {
        Locale loc("ko", "KR", OUString());
        maxLeftLength = xCDL->queryMaxCharCount(loc,
                        ConversionDictionaryType::HANGUL_HANJA,
                        ConversionDirection_FROM_LEFT);
        maxRightLength = xCDL->queryMaxCharCount(loc,
                        ConversionDictionaryType::HANGUL_HANJA,
                        ConversionDirection_FROM_RIGHT);
        if (xCD.is()) {
            sal_Int32 tmp = xCD->getMaxCharCount(ConversionDirection_FROM_LEFT);
            if (tmp > maxLeftLength)
                maxLeftLength = tmp;
            tmp = xCD->getMaxCharCount(ConversionDirection_FROM_RIGHT);
            if (tmp > maxRightLength)
                maxRightLength = tmp;
        }
    } else if (xCD.is()) {
        maxLeftLength = xCD->getMaxCharCount(ConversionDirection_FROM_LEFT);
        maxRightLength = xCD->getMaxCharCount(ConversionDirection_FROM_RIGHT);
    }
}

static sal_Int16 checkScriptType(sal_Unicode c)
{
    struct UBlock2Script {
        UBlockCode from;
        UBlockCode to;
        sal_Int16 script;
    };

    static const UBlock2Script scriptList[] = {
        {UBLOCK_HANGUL_JAMO, UBLOCK_HANGUL_JAMO, SCRIPT_HANGUL},
        {UBLOCK_CJK_RADICALS_SUPPLEMENT, UBLOCK_BOPOMOFO, SCRIPT_HANJA},
        {UBLOCK_HANGUL_COMPATIBILITY_JAMO, UBLOCK_HANGUL_COMPATIBILITY_JAMO, SCRIPT_HANGUL},
        {UBLOCK_KANBUN, UBLOCK_YI_RADICALS, SCRIPT_HANJA},
        {UBLOCK_HANGUL_SYLLABLES, UBLOCK_HANGUL_SYLLABLES, SCRIPT_HANGUL},
        {UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS, UBLOCK_CJK_COMPATIBILITY_IDEOGRAPHS, SCRIPT_HANJA},
        {UBLOCK_COMBINING_HALF_MARKS, UBLOCK_SMALL_FORM_VARIANTS, SCRIPT_HANJA},
        {UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS, UBLOCK_HALFWIDTH_AND_FULLWIDTH_FORMS, SCRIPT_HANJA},
    };

    UBlockCode block=ublock_getCode(static_cast<sal_uInt32>(c));
    size_t i;
    for ( i = 0; i < SAL_N_ELEMENTS(scriptList); i++) {
        if (block <= scriptList[i].to) break;
    }
    return (i < SAL_N_ELEMENTS(scriptList) && block >= scriptList[i].from) ? scriptList[i].script : SCRIPT_OTHERS;
}

#ifdef DISABLE_DYNLOADING

extern "C" {

const sal_Unicode* getHangul2HanjaData();
const Hangul_Index* getHangul2HanjaIndex();
sal_Int16 getHangul2HanjaIndexCount();
const sal_uInt16* getHanja2HangulIndex();
const sal_Unicode* getHanja2HangulData();

}

#endif

Sequence< OUString >
TextConversion_ko::getCharConversions(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, bool toHanja)
{
    sal_Unicode ch;
    Sequence< OUString > output;
#ifndef DISABLE_DYNLOADING
    const sal_Unicode* (*getHangul2HanjaData)() = reinterpret_cast<const sal_Unicode* (*)()>(getFunctionBySymbol("getHangul2HanjaData"));
    const Hangul_Index* (*getHangul2HanjaIndex)() = reinterpret_cast<const Hangul_Index* (*)()>(getFunctionBySymbol("getHangul2HanjaIndex"));
    sal_Int16 (*getHangul2HanjaIndexCount)() = reinterpret_cast<sal_Int16 (*)()>(getFunctionBySymbol("getHangul2HanjaIndexCount"));
    const sal_uInt16* (*getHanja2HangulIndex)() = reinterpret_cast<const sal_uInt16* (*)()>(getFunctionBySymbol("getHanja2HangulIndex"));
    const sal_Unicode* (*getHanja2HangulData)() = reinterpret_cast<const sal_Unicode* (*)()>(getFunctionBySymbol("getHanja2HangulData"));
#else
#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic warning "-Wbool-conversions"
#else
#pragma GCC diagnostic warning "-Waddress"
#endif
#endif
    if (toHanja && getHangul2HanjaIndex && getHangul2HanjaIndexCount && getHangul2HanjaData) {
        ch = aText[nStartPos];
        const Hangul_Index *Hangul_ko = getHangul2HanjaIndex();
        sal_Int16 top =  getHangul2HanjaIndexCount();
        --top;
        sal_Int16 bottom = 0;

        while (bottom <= top) {
            sal_Int16 current = (top + bottom) / 2;
            sal_Unicode current_ch = Hangul_ko[current].code;
            if (ch < current_ch)
                top = current - 1;
            else if (ch > current_ch)
                bottom = current + 1;
            else {
                const sal_Unicode *ptr = getHangul2HanjaData() + Hangul_ko[current].address;
                sal_Int16 count = Hangul_ko[current].count;
                output.realloc(count);
                for (sal_Int16 i = 0; i < count; i++)
                    output[i] = OUString(ptr + i, 1);
                break;
            }
        }
    } else if (! toHanja && getHanja2HangulIndex && getHanja2HangulData)
    {
        std::unique_ptr<sal_Unicode[]> newStr(new sal_Unicode[nLength+1]);
        sal_Int32 count = 0;
        while (count < nLength)
        {
            ch = aText[nStartPos + count];
            sal_Unicode address = getHanja2HangulIndex()[ch>>8];
            if (address != 0xFFFF)
                address = getHanja2HangulData()[address + (ch & 0xFF)];

            if (address != 0xFFFF)
                newStr[count++] = address;
            else
                break;
        }
        if (count > 0)
        {
            output.realloc(1);
            output[0] = OUString(newStr.get(), count);
        }
    }
#if defined(DISABLE_DYNLOADING)
#pragma GCC diagnostic pop
#endif
    return output;
}

static Sequence< OUString >& operator += (Sequence< OUString > &rSeq1, Sequence< OUString > &rSeq2 )
{
    if (! rSeq1.hasElements() && rSeq2.hasElements())
        rSeq1 = rSeq2;
    else if (rSeq2.hasElements()) {
        sal_Int32 i, j, k, l;
        k = l = rSeq1.getLength();
        rSeq1.realloc(l + rSeq2.getLength());

        for (i = 0; i < rSeq2.getLength(); i++) {
            for (j = 0; j < l; j++)
                if (rSeq1[j] == rSeq2[i])
                    break;
            if (j == l)
                rSeq1[k++] = rSeq2[i];
        }
        if (rSeq1.getLength() > k)
            rSeq1.realloc(k);
    }
    return rSeq1;
}

TextConversionResult SAL_CALL
TextConversion_ko::getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& aLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
{
    TextConversionResult result;
    Sequence <OUString> candidates;
    result.Boundary.startPos = result.Boundary.endPos = 0;

    // do conversion only when there are right conversion type and dictionary services.
    if (nConversionType != TextConversionType::TO_HANGUL &&
        nConversionType != TextConversionType::TO_HANJA)
        throw NoSupportException(); // Conversion type is not supported in this service.
    sal_Int32 start, end, length = aText.getLength() - nStartPos;

    if (length < 0 || nStartPos < 0)
        length = 0;
    else if (length > nLength)
        length = nLength;

    sal_Int16 scriptType = SCRIPT_OTHERS;
    sal_Int32 len = 1;
    bool toHanja = (nConversionType == TextConversionType::TO_HANJA);
    // FROM_LEFT:  Hangul -> Hanja
    // FROM_RIGHT: Hanja  -> Hangul
    ConversionDirection eDirection = toHanja ? ConversionDirection_FROM_LEFT : ConversionDirection_FROM_RIGHT;
    sal_Int32 maxLength = toHanja ? maxLeftLength : maxRightLength;
    if (maxLength == 0) maxLength = 1;

    // search for a max length of convertible text
    for (start = 0, end = 0; start < length; start++) {
        if (end <= start) {
            scriptType = checkScriptType(aText[nStartPos + start]);
            if (nConversionType == TextConversionType::TO_HANJA) {
                if (scriptType != SCRIPT_HANGUL) // skip non-Hangul characters
                    continue;
            } else {
                if (scriptType != SCRIPT_HANJA) // skip non-Hanja characters
                    continue;
            }
            end = start + 1;
        }
        if (nConversionOptions & TextConversionOption::CHARACTER_BY_CHARACTER) {
            result.Candidates = getCharConversions(aText, nStartPos + start, len, toHanja); // char2char conversion
        } else {
            for (; end < length && end - start < maxLength; end++)
                if (checkScriptType(aText[nStartPos + end]) != scriptType)
                    break;

            for (len = end - start; len > 0; len--) {
                if (len > 1) {
                    try {
                        if (xCDL.is())
                            result.Candidates = xCDL->queryConversions(aText, start + nStartPos, len,
                                aLocale, ConversionDictionaryType::HANGUL_HANJA, eDirection, nConversionOptions); // user dictionary
                    }
                    catch ( NoSupportException & ) {
                        // clear reference (when there is no user dictionary) in order
                        // to not always have to catch this exception again
                        // in further calls. (save time)
                        xCDL = nullptr;
                    }
                    catch (...) {
                        // catch all other exceptions to allow
                        // querying the system dictionary in the next line
                    }
                    if (xCD.is() && toHanja) { // System dictionary would not do Hanja_to_Hangul conversion.
                        candidates = xCD->getConversions(aText, start + nStartPos, len, eDirection, nConversionOptions);
                        result.Candidates += candidates;
                    }
                } else if (! toHanja) { // do whole word character 2 character conversion for Hanja to Hangul conversion
                    result.Candidates = getCharConversions(aText, nStartPos + start, length - start, toHanja);
                    if (result.Candidates.hasElements())
                        len = result.Candidates[0].getLength();
                }
                if (result.Candidates.hasElements())
                    break;
            }
        }
        // found match
        if (result.Candidates.hasElements()) {
            result.Boundary.startPos = start + nStartPos;
            result.Boundary.endPos = start + len + nStartPos;
            return result;
        }
    }

    return result;
}

OUString SAL_CALL
TextConversion_ko::getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& aLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
{
    sal_Int32 length = aText.getLength() - nStartPos;

    if (length <= 0 || nStartPos < 0)
        return OUString();
    else if (length > nLength)
        length = nLength;

    OUStringBuffer aBuf(length + 1);
    TextConversionResult result;
    const sal_Unicode *str = aText.getStr();

    for (sal_Int32 start = nStartPos; length + nStartPos > start; start = result.Boundary.endPos) {

        result = getConversions(aText, start, length + nStartPos - start, aLocale, nConversionType, nConversionOptions);

        if (result.Boundary.endPos > 0) {
            if (result.Boundary.startPos > start)
                aBuf.append(str + start, result.Boundary.startPos - start); // append skip portion
            aBuf.append(result.Candidates[0]); // append converted portion
        } else {
            aBuf.append(str + start, length + nStartPos - start); // append last portion
            break;
        }
    }

    return aBuf.makeStringAndClear();
}

OUString SAL_CALL
TextConversion_ko::getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions, Sequence<sal_Int32>& offset)
{
    offset.realloc(0);
    return getConversion(aText, nStartPos, nLength, rLocale, nConversionType, nConversionOptions);
}

sal_Bool SAL_CALL
TextConversion_ko::interactiveConversion( const Locale& /*rLocale*/, sal_Int16 /*nTextConversionType*/, sal_Int32 /*nTextConversionOptions*/ )
{
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
