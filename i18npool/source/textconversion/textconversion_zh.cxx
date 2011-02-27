/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#include <assert.h>
#include <textconversion.hxx>
#include <com/sun/star/i18n/TextConversionType.hpp>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/linguistic2/ConversionDirection.hpp>
#include <com/sun/star/linguistic2/ConversionDictionaryType.hpp>
#include <i18nutil/x_rtl_ustring.h>

using namespace com::sun::star::lang;
using namespace com::sun::star::i18n;
using namespace com::sun::star::linguistic2;
using namespace com::sun::star::uno;

using ::rtl::OUString;

namespace com { namespace sun { namespace star { namespace i18n {

TextConversion_zh::TextConversion_zh( const Reference < XMultiServiceFactory >& xMSF )
{
    Reference < XInterface > xI;
    xI = xMSF->createInstance(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.linguistic2.ConversionDictionaryList")));
    if ( xI.is() )
        xI->queryInterface( getCppuType((const Reference< XConversionDictionaryList>*)0) ) >>= xCDL;

    implementationName = "com.sun.star.i18n.TextConversion_zh";
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

OUString SAL_CALL
TextConversion_zh::getCharConversion(const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, sal_Bool toSChinese, sal_Int32 nConversionOptions)
{
    const sal_Unicode *Data;
    const sal_uInt16 *Index;

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

    rtl_uString * newStr = x_rtl_uString_new_WithLength( nLength ); // defined in x_rtl_ustring.h
    for (sal_Int32 i = 0; i < nLength; i++)
        newStr->buffer[i] =
            getOneCharConversion(aText[nStartPos+i], Data, Index);
    return OUString( newStr->buffer, nLength);
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

    if ((!wordData || !index || !entry) && !xCDL.is()) // no word mapping defined, do char2char conversion.
        return getCharConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions);

    rtl_uString * newStr = x_rtl_uString_new_WithLength( nLength * 2 ); // defined in x_rtl_ustring.h
    sal_Int32 currPos = 0, count = 0;
    while (currPos < nLength) {
        sal_Int32 len = nLength - currPos;
        sal_Bool found = sal_False;
        if (len > maxLen)
            len = maxLen;
        for (; len > 0 && ! found; len--) {
            OUString word = aText.copy(nStartPos + currPos, len);
            sal_Int32 current = 0;
            // user dictionary
            if (xCDL.is()) {
                Sequence < OUString > conversions;
                try {
                    conversions = xCDL->queryConversions(word, 0, len,
                            aLocale, ConversionDictionaryType::SCHINESE_TCHINESE,
                            /*toSChinese ?*/ ConversionDirection_FROM_LEFT /*: ConversionDirection_FROM_RIGHT*/,
                            nConversionOptions);
                }
                catch ( NoSupportException & ) {
                    // clear reference (when there is no user dictionary) in order
                    // to not always have to catch this exception again
                    // in further calls. (save time)
                    xCDL = 0;
                }
                catch (...) {
                    // catch all other exceptions to allow
                    // querying the system dictionary in the next line
                }
                if (conversions.getLength() > 0) {
                    if (offset.getLength() > 0) {
                        if (word.getLength() != conversions[0].getLength())
                            one2one=sal_False;
                        while (current < conversions[0].getLength()) {
                            offset[count] = nStartPos + currPos + (current *
                                    word.getLength() / conversions[0].getLength());
                            newStr->buffer[count++] = conversions[0][current++];
                        }
                        // offset[count-1] = nStartPos + currPos + word.getLength() - 1;
                    } else {
                        while (current < conversions[0].getLength())
                            newStr->buffer[count++] = conversions[0][current++];
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
                        if (toSChinese)   // Traditionary/Simplified conversion,
                            for (current = entry[current]-1; current > 0 && wordData[current-1]; current--) ;
                        else  // Simplified/Traditionary conversion, forwards search for next word
                            current = entry[current] + word.getLength() + 1;
                        sal_Int32 start=current;
                        if (offset.getLength() > 0) {
                            if (word.getLength() != OUString(&wordData[current]).getLength())
                                one2one=sal_False;
                            sal_Int32 convertedLength=OUString(&wordData[current]).getLength();
                            while (wordData[current]) {
                                offset[count]=nStartPos + currPos + ((current-start) *
                                    word.getLength() / convertedLength);
                                newStr->buffer[count++] = wordData[current++];
                            }
                            // offset[count-1]=nStartPos + currPos + word.getLength() - 1;
                        } else {
                            while (wordData[current])
                                newStr->buffer[count++] = wordData[current++];
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
            newStr->buffer[count++] =
                getOneCharConversion(aText[nStartPos+currPos], charData, charIndex);
            currPos++;
        }
    }
    if (offset.getLength() > 0)
        offset.realloc(one2one ? 0 : count);
    return OUString( newStr->buffer, count);
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
    if (rLocale.Language.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("zh")) &&
            ( nConversionType == TextConversionType::TO_SCHINESE ||
            nConversionType == TextConversionType::TO_TCHINESE) ) {

        aLocale=rLocale;
        sal_Bool toSChinese = nConversionType == TextConversionType::TO_SCHINESE;

        if (nConversionOptions & TextConversionOption::CHARACTER_BY_CHARACTER)
            // char to char dictionary
            return getCharConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions);
        else {
            Sequence <sal_Int32> offset;
            // word to word dictionary
            return  getWordConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions, offset);
        }
    } else
        throw NoSupportException(); // Conversion type is not supported in this service.
}

OUString SAL_CALL
TextConversion_zh::getConversionWithOffset( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength,
    const Locale& rLocale, sal_Int16 nConversionType, sal_Int32 nConversionOptions, Sequence<sal_Int32>& offset)
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    if (rLocale.Language.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("zh")) &&
            ( nConversionType == TextConversionType::TO_SCHINESE ||
            nConversionType == TextConversionType::TO_TCHINESE) ) {

        aLocale=rLocale;
        sal_Bool toSChinese = nConversionType == TextConversionType::TO_SCHINESE;

        if (nConversionOptions & TextConversionOption::CHARACTER_BY_CHARACTER) {
            offset.realloc(0);
            // char to char dictionary
            return getCharConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions);
        } else {
            if (offset.getLength() < 2*nLength)
                offset.realloc(2*nLength);
            // word to word dictionary
            return  getWordConversion(aText, nStartPos, nLength, toSChinese, nConversionOptions, offset);
        }
    } else
        throw NoSupportException(); // Conversion type is not supported in this service.
}

sal_Bool SAL_CALL
TextConversion_zh::interactiveConversion( const Locale& /*rLocale*/, sal_Int16 /*nTextConversionType*/, sal_Int32 /*nTextConversionOptions*/ )
    throw(  RuntimeException, IllegalArgumentException, NoSupportException )
{
    return sal_False;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
