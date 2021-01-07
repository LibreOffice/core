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

#include <rtl/ref.hxx>
#include <i18nutil/casefolding.hxx>
#include <i18nutil/unicode.hxx>
#include <com/sun/star/i18n/MultipleCharsOutputException.hpp>
#include <com/sun/star/i18n/TransliterationType.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <o3tl/temporary.hxx>

#include <characterclassificationImpl.hxx>

#include <transliteration_body.hxx>
#include <memory>
#include <numeric>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

Transliteration_body::Transliteration_body()
{
    nMappingType = MappingType::NONE;
    transliterationName = "Transliteration_body";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_body";
}

sal_Int16 SAL_CALL Transliteration_body::getType()
{
    return TransliterationType::ONE_TO_ONE;
}

sal_Bool SAL_CALL Transliteration_body::equals(
    const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/, sal_Int32& /*nMatch1*/,
    const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
Transliteration_body::transliterateRange( const OUString& str1, const OUString& str2 )
{
    Sequence< OUString > ostr(2);
    ostr[0] = str1;
    ostr[1] = str2;
    return ostr;
}

static MappingType lcl_getMappingTypeForToggleCase( MappingType nMappingType, sal_Unicode cChar )
{
    MappingType nRes = nMappingType;

    // take care of TOGGLE_CASE transliteration:
    // nMappingType should not be a combination of flags, thuse we decide now
    // which one to use.
    if (nMappingType == (MappingType::LowerToUpper | MappingType::UpperToLower))
    {
        const sal_Int16 nType = unicode::getUnicodeType( cChar );
        if (nType & 0x02 /* lower case*/)
            nRes = MappingType::LowerToUpper;
        else
        {
            // should also work properly for non-upper characters like white spaces, numbers, ...
            nRes = MappingType::UpperToLower;
        }
    }

    return nRes;
}

OUString
Transliteration_body::transliterateImpl(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset, bool useOffset)
{
    const sal_Unicode *in = inStr.getStr() + startPos;

    // We could assume that most calls result in identical string lengths,
    // thus using a preallocated OUStringBuffer could be an easy way
    // to assemble the return string without too much hassle. However,
    // for single characters the OUStringBuffer::append() method is quite
    // expensive compared to a simple array operation, so it pays here
    // to copy the final result instead.

    // Allocate the max possible buffer. Try to use stack instead of heap,
    // which would have to be reallocated most times anyways.
    constexpr sal_Int32 nLocalBuf = 2048;
    sal_Unicode aLocalBuf[ nLocalBuf * NMAPPINGMAX ], *out = aLocalBuf;
    std::unique_ptr<sal_Unicode[]> pHeapBuf;
    if (nCount > nLocalBuf)
    {
        pHeapBuf.reset(new sal_Unicode[ nCount * NMAPPINGMAX ]);
        out = pHeapBuf.get();
    }

    sal_Int32 j = 0;
    // Two different blocks to eliminate the if(useOffset) condition inside the loop.
    // Yes, on massive use even such small things do count.
    if ( useOffset )
    {
        std::vector<sal_Int32> aVec;
        aVec.reserve(std::max<sal_Int32>(nLocalBuf, nCount) * NMAPPINGMAX);

        for (sal_Int32 i = 0; i < nCount; i++)
        {
            // take care of TOGGLE_CASE transliteration:
            MappingType nTmpMappingType = lcl_getMappingTypeForToggleCase( nMappingType, in[i] );

            const i18nutil::Mapping &map = i18nutil::casefolding::getValue( in, i, nCount, aLocale, nTmpMappingType );
            std::fill_n(std::back_inserter(aVec), map.nmap, i + startPos);
            std::copy_n(map.map, map.nmap, out + j);
            j += map.nmap;
        }

        offset = comphelper::containerToSequence(aVec);
    }
    else
    {
        for ( sal_Int32 i = 0; i < nCount; i++)
        {
            // take care of TOGGLE_CASE transliteration:
            MappingType nTmpMappingType = lcl_getMappingTypeForToggleCase( nMappingType, in[i] );

            const i18nutil::Mapping &map = i18nutil::casefolding::getValue( in, i, nCount, aLocale, nTmpMappingType );
            std::copy_n(map.map, map.nmap, out + j);
            j += map.nmap;
        }
    }

    return OUString(out, j);
}

OUString SAL_CALL
Transliteration_body::transliterateChar2String( sal_Unicode inChar )
{
    const i18nutil::Mapping &map = i18nutil::casefolding::getValue(&inChar, 0, 1, aLocale, nMappingType);
    rtl_uString* pStr = rtl_uString_alloc(map.nmap);
    sal_Unicode* out = pStr->buffer;
    sal_Int32 i;

    for (i = 0; i < map.nmap; i++)
        out[i] = map.map[i];
    out[i] = 0;

    return OUString( pStr, SAL_NO_ACQUIRE );
}

sal_Unicode SAL_CALL
Transliteration_body::transliterateChar2Char( sal_Unicode inChar )
{
    const i18nutil::Mapping &map = i18nutil::casefolding::getValue(&inChar, 0, 1, aLocale, nMappingType);
    if (map.nmap > 1)
        throw MultipleCharsOutputException();
    return map.map[0];
}

OUString
Transliteration_body::foldingImpl( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset, bool useOffset)
{
    return transliterateImpl(inStr, startPos, nCount, offset, useOffset);
}

Transliteration_casemapping::Transliteration_casemapping()
{
    nMappingType = MappingType::NONE;
    transliterationName = "casemapping(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_casemapping";
}

void
Transliteration_casemapping::setMappingType( const MappingType rMappingType, const Locale& rLocale )
{
    nMappingType = rMappingType;
    aLocale = rLocale;
}

Transliteration_u2l::Transliteration_u2l()
{
    nMappingType = MappingType::UpperToLower;
    transliterationName = "upper_to_lower(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_u2l";
}

Transliteration_l2u::Transliteration_l2u()
{
    nMappingType = MappingType::LowerToUpper;
    transliterationName = "lower_to_upper(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_l2u";
}

Transliteration_togglecase::Transliteration_togglecase()
{
    // usually nMappingType must NOT be a combination of different flags here,
    // but we take care of that problem in Transliteration_body::transliterate above
    // before that value is used. There we will decide which of both is to be used on
    // a per character basis.
    nMappingType = MappingType::LowerToUpper | MappingType::UpperToLower;
    transliterationName = "toggle(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_togglecase";
}

Transliteration_titlecase::Transliteration_titlecase()
{
    nMappingType = MappingType::ToTitle;
    transliterationName = "title(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_titlecase";
}

/// @throws RuntimeException
static OUString transliterate_titlecase_Impl(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    const Locale &rLocale,
    Sequence< sal_Int32 >& offset )
{
    const OUString aText( inStr.copy( startPos, nCount ) );

    OUString aRes;
    if (!aText.isEmpty())
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        rtl::Reference< CharacterClassificationImpl > xCharClassImpl( new CharacterClassificationImpl( xContext ) );

        // because xCharClassImpl.toTitle does not handle ligatures or Beta but will raise
        // an exception we need to handle the first chara manually...

        // we don't want to change surrogates by accident, thuse we use proper code point iteration
        sal_uInt32 cFirstChar = aText.iterateCodePoints( &o3tl::temporary(sal_Int32(0)) );
        OUString aResolvedLigature( &cFirstChar, 1 );
        // toUpper can be used to properly resolve ligatures and characters like Beta
        aResolvedLigature = xCharClassImpl->toUpper( aResolvedLigature, 0, aResolvedLigature.getLength(), rLocale );
        // since toTitle will leave all-uppercase text unchanged we first need to
        // use toLower to bring possible 2nd and following chars in lowercase
        aResolvedLigature = xCharClassImpl->toLower( aResolvedLigature, 0, aResolvedLigature.getLength(), rLocale );
        sal_Int32 nResolvedLen = aResolvedLigature.getLength();

        // now we can properly use toTitle to get the expected result for the resolved string.
        // The rest of the text should just become lowercase.
        aRes = xCharClassImpl->toTitle( aResolvedLigature, 0, nResolvedLen, rLocale ) +
               xCharClassImpl->toLower( aText, 1, aText.getLength() - 1, rLocale );
        offset.realloc( aRes.getLength() );

        sal_Int32* pOffset = std::fill_n(offset.begin(), nResolvedLen, 0);
        std::iota(pOffset, offset.end(), 1);
    }
    return aRes;
}

// this function expects to be called on a word-by-word basis,
// namely that startPos points to the first char of the word
OUString Transliteration_titlecase::transliterateImpl(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset, bool )
{
    return transliterate_titlecase_Impl( inStr, startPos, nCount, aLocale, offset );
}

Transliteration_sentencecase::Transliteration_sentencecase()
{
    nMappingType = MappingType::ToTitle;  // though only to be applied to the first word...
    transliterationName = "sentence(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_sentencecase";
}

// this function expects to be called on a sentence-by-sentence basis,
// namely that startPos points to the first word (NOT first char!) in the sentence
OUString Transliteration_sentencecase::transliterateImpl(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset, bool )
{
    return transliterate_titlecase_Impl( inStr, startPos, nCount, aLocale, offset );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
