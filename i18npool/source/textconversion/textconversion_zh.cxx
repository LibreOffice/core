/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <assert.h>
#include <textconversion.hxx>
#include <com/sun/star/i18n/TextConversionType.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/linguistic2/ConversionDirection.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryList.hpp>
#include <comphelper/string.hxx>

using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;


namespace com { namespace sun { namespace star { namespace i18n {

TextConversion_zh::TextConversion_zh( const Reference < XComponentContext >& xContext )
    : TextConversion("com.sun.star.i18n.TextConversion_zh")
{
    xCDL = ConversionDictionaryList::create(xContext);
}

sal_Unicode SAL_CALL getOneCharConversion(sal_Unicode ch, const sal_Unicode* Data, const sal_uInt16* Index)
{
    if (Data && Index) {
        sal_Unicode address = Index[ch>>8];
        if (address != 0xFFFF)
            address = Data[address + (ch & 0xFF)];
        return (address != 0xFFFF) ? address : ch;
    } else {
        return ch;
    }
}

#ifdef DISABLE_DYNLOADING

extern "C" {

const sal_Unicode* getSTC_CharData_T2S();
const sal_uInt16* getSTC_CharIndex_T2S();
const sal_Unicode* getSTC_CharData_S2V();
const sal_uInt16* getSTC_CharIndex_S2V();
const sal_Unicode* getSTC_CharData_S2T();
const sal_uInt16* getSTC_CharIndex_S2T();

const sal_Unicode *getSTC_WordData(sal_Int32&);

const sal_uInt16 *getSTC_WordIndex_T2S(sal_Int32&);
const sal_uInt16 *getSTC_WordEntry_T2S();
const sal_uInt16 *getSTC_WordIndex_S2T(sal_Int32&);
const sal_uInt16 *getSTC_WordEntry_S2T();

}

#endif

OUString SAL_CALL
TextConversion_zh::getCharConversion(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toSChinese, sal_Int32 nConversionOptions)
{
    const sal_Unicode *Data;
    const sal_uInt16 *Index;

#ifndef DISABLE_DYNLOADING
    if (toSChinese) {
        Data = ((const sal_Unicode* (*)())getFunctionBySymbol("getSTC_CharData_T2S"))();
        Index = ((const sal_uInt16* (*)())getFunctionBySymbol("getSTC_CharIndex_T2S"))();
    } else if (nConversionOptions & TextConversionOption::USE_CHARACTER_VARIANTS) {
        Data = ((const sal_Unicode* (*)())getFunctionBySymbol("getSTC_CharData_S2V"))();
        Index = ((const sal_uInt16* (*)())getFunctionBySymbol("getSTC_CharIndex_S2V"))();
    } else {
        Data = ((const sal_Unicode* (*)())getFunctionBySymbol("getSTC_CharData_S2T"))();
        Index = ((const sal_uInt16* (*)())getFunctionBySymbol("getSTC_CharIndex_S2T"))();
    }
#else
    if (toSChinese) {
        Data = getSTC_CharData_T2S();
        Index = getSTC_CharIndex_T2S();
    } else if (nConversionOptions & TextConversionOption::USE_CHARACTER_VARIANTS) {
        Data = getSTC_CharData_S2V();
        Index = getSTC_CharIndex_S2V();
    } else {
        Data = getSTC_CharData_S2T();
        Index = getSTC_CharIndex_S2T();
    }
#endif

    rtl_uString * newStr = rtl_uString_alloc(nLength);
    for (sal_Int32 i = 0; i < nLength; i++)
        newStr->buffer[i] =
            getOneCharConversion(aText[nStartPos+i], Data, Index);
    return OUString(newStr, SAL_NO_ACQUIRE); 
}

OUString SAL_CALL
TextConversion_zh::getWordConversion(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toSChinese, sal_Int32 nConversionOptions, Sequence<sal_Int32>& offset)
{
    sal_Int32 dictLen = 0;
    sal_Int32 maxLen = 0;
    const sal_uInt16 *index;
    const sal_uInt16 *entry;
    const sal_Unicode *charData;
    const sal_uInt16 *charIndex;
    sal_Bool one2one=sal_True;

#ifndef DISABLE_DYNLOADING
    const sal_Unicode *wordData = ((const sal_Unicode* (*)(sal_Int32&)) getFunctionBySymbol("getSTC_WordData"))(dictLen);
    if (toSChinese) {
        index = ((const sal_uInt16* (*)(sal_Int32&)) getFunctionBySymbol("getSTC_WordIndex_T2S"))(maxLen);
        entry = ((const sal_uInt16* (*)()) getFunctionBySymbol("getSTC_WordEntry_T2S"))();
        charData = ((const sal_Unicode* (*)()) getFunctionBySymbol("getSTC_CharData_T2S"))();
        charIndex = ((const sal_uInt16* (*)()) getFunctionBySymbol("getSTC_CharIndex_T2S"))();
    } else {
        index = ((const sal_uInt16* (*)(sal_Int32&)) getFunctionBySymbol("getSTC_WordIndex_S2T"))(maxLen);
        entry = ((const sal_uInt16* (*)()) getFunctionBySymbol("getSTC_WordEntry_S2T"))();
        if (nConversionOptions & TextConversionOption::USE_CHARACTER_VARIANTS) {
            charData = ((const sal_Unicode* (*)()) getFunctionBySymbol("getSTC_CharData_S2V"))();
            charIndex = ((const sal_uInt16* (*)()) getFunctionBySymbol("getSTC_CharIndex_S2V"))();
        } else {
            charData = ((const sal_Unicode* (*)()) getFunctionBySymbol("getSTC_CharData_S2T"))();
            charIndex = ((const sal_uInt16* (*)()) getFunctionBySymbol("getSTC_CharIndex_S2T"))();
        }
    }
#else
    const sal_Unicode *wordData = getSTC_WordData(dictLen);
    if (toSChinese) {
        index = getSTC_WordIndex_T2S(maxLen);
        entry = getSTC_WordEntry_T2S();
        charData = getSTC_CharData_T2S();
        charIndex = getSTC_CharIndex_T2S();
    } else {
        index = getSTC_WordIndex_S2T(maxLen);
        entry = getSTC_WordEntry_S2T();
        if (nConversionOptions & TextConversionOption::USE_CHARACTER_VARIANTS) {
            charData = getSTC_CharData_S2V();
            charIndex = getSTC_CharIndex_S2V();
        } else {
            charData = getSTC_CharData_S2T();
            charIndex = getSTC_CharIndex_S2T();
        }
    }
#endif

    if ((!wordData || !index || !entry) && !xCDL.is()) 
        return getCharConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions);

    sal_Unicode *newStr = new sal_Unicode[nLength * 2 + 1];
    sal_Int32 currPos = 0, count = 0;
    while (currPos < nLength) {
        sal_Int32 len = nLength - currPos;
        sal_Bool found = sal_False;
        if (len > maxLen)
            len = maxLen;
        for (; len > 0 && ! found; len--) {
            OUString word = aText.copy(nStartPos + currPos, len);
            sal_Int32 current = 0;
            
            if (xCDL.is()) {
                Sequence < OUString > conversions;
                try {
                    conversions = xCDL->queryConversions(word, 0, len,
                            aLocale, ConversionDictionaryType::SCHINESE_TCHINESE,
                            /*toSChinese ?*/ ConversionDirection_FROM_LEFT /*: ConversionDirection_FROM_RIGHT*/,
                            nConversionOptions);
                }
                catch ( NoSupportException & ) {
                    
                    
                    
                    xCDL = 0;
                }
                catch (...) {
                    
                    
                }
                if (conversions.getLength() > 0) {
                    if (offset.getLength() > 0) {
                        if (word.getLength() != conversions[0].getLength())
                            one2one=sal_False;
                        while (current < conversions[0].getLength()) {
                            offset[count] = nStartPos + currPos + (current *
                                    word.getLength() / conversions[0].getLength());
                            newStr[count++] = conversions[0][current++];
                        }
                        
                    } else {
                        while (current < conversions[0].getLength())
                            newStr[count++] = conversions[0][current++];
                    }
                    currPos += word.getLength();
                    found = sal_True;
                }
            }

            if (!found && index[len+1] - index[len] > 0) {
                sal_Int32 bottom = (sal_Int32) index[len];
                sal_Int32 top = (sal_Int32) index[len+1] - 1;

                while (bottom <= top && !found) {
                    current = (top + bottom) / 2;
                    const sal_Int32 result = word.compareTo(wordData + entry[current]);
                    if (result < 0)
                        top = current - 1;
                    else if (result > 0)
                        bottom = current + 1;
                    else {
                        if (toSChinese)   
                            for (current = entry[current]-1; current > 0 && wordData[current-1]; current--) ;
                        else  
                            current = entry[current] + word.getLength() + 1;
                        sal_Int32 start=current;
                        if (offset.getLength() > 0) {
                            if (word.getLength() != OUString(&wordData[current]).getLength())
                                one2one=sal_False;
                            sal_Int32 convertedLength=OUString(&wordData[current]).getLength();
                            while (wordData[current]) {
                                offset[count]=nStartPos + currPos + ((current-start) *
                                    word.getLength() / convertedLength);
                                newStr[count++] = wordData[current++];
                            }
                            
                        } else {
                            while (wordData[current])
                                newStr[count++] = wordData[current++];
                        }
                        currPos += word.getLength();
                        found = sal_True;
                    }
                }
            }
        }
        if (!found) {
            if (offset.getLength() > 0)
                offset[count]=nStartPos+currPos;
            newStr[count++] =
                getOneCharConversion(aText[nStartPos+currPos], charData, charIndex);
            currPos++;
        }
    }
    if (offset.getLength() > 0)
        offset.realloc(one2one ? 0 : count);
    OUString aRet(newStr, count);
    delete[] newStr;
    return aRet;
}

TextConversionResult SAL_CALL
TextConversion_zh::getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    TextConversionResult result;

    result.Candidates.realloc(1);
    result.Candidates[0] = getConversion( aText, nStartPos, nLength, rLocale, nConversionType, nConversionOptions);
    result.Boundary.startPos = nStartPos;
    result.Boundary.endPos = nStartPos + nLength;

    return result;
}

OUString SAL_CALL
TextConversion_zh::getConversion( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    if (rLocale.Language == "zh" && ( nConversionType == TextConversionType::TO_SCHINESE || nConversionType == TextConversionType::TO_TCHINESE) ) {

        aLocale=rLocale;
        sal_Bool toSChinese = nConversionType == TextConversionType::TO_SCHINESE;

        if (nConversionOptions & TextConversionOption::CHARACTER_BY_CHARACTER)
            
            return getCharConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions);
        else {
            Sequence <sal_Int32> offset;
            
            return  getWordConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions, offset);
        }
    } else
        throw NoSupportException(); 
}

OUString SAL_CALL
TextConversion_zh::getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions, Sequence<sal_Int32>& offset)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    if (rLocale.Language == "zh" && ( nConversionType == TextConversionType::TO_SCHINESE || nConversionType == TextConversionType::TO_TCHINESE) ) {

        aLocale=rLocale;
        sal_Bool toSChinese = nConversionType == TextConversionType::TO_SCHINESE;

        if (nConversionOptions & TextConversionOption::CHARACTER_BY_CHARACTER) {
            offset.realloc(0);
            
            return getCharConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions);
        } else {
            if (offset.getLength() < 2*nLength)
                offset.realloc(2*nLength);
            
            return  getWordConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions, offset);
        }
    } else
        throw NoSupportException(); 
}

sal_Bool SAL_CALL
TextConversion_zh::interactiveConversion( const Locale& /*rLocale*/, sal_Int16 /*nTextConversionType*/, sal_Int32 /*nTextConversionOptions*/ )
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    return sal_False;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
