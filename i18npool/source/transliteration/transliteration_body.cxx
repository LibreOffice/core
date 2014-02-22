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

#include <rtl/ustrbuf.hxx>
#include <i18nutil/casefolding.hxx>
#include <i18nutil/unicode.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <osl/diagnose.h>

#include <string.h>

#include "characterclassificationImpl.hxx"
#include "breakiteratorImpl.hxx"

#define TRANSLITERATION_ALL
#include "transliteration_body.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

Transliteration_body::Transliteration_body()
{
    nMappingType = 0;
    transliterationName = "Transliteration_body";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_body";
}

sal_Int16 SAL_CALL Transliteration_body::getType() throw(RuntimeException)
{
    return TransliterationType::ONE_TO_ONE;
}

sal_Bool SAL_CALL Transliteration_body::equals(
    const OUString& /*str1*/, sal_Int32 /*pos1*/, sal_Int32 /*nCount1*/, sal_Int32& /*nMatch1*/,
    const OUString& /*str2*/, sal_Int32 /*pos2*/, sal_Int32 /*nCount2*/, sal_Int32& /*nMatch2*/)
    throw(RuntimeException)
{
    throw RuntimeException();
}

Sequence< OUString > SAL_CALL
Transliteration_body::transliterateRange( const OUString& str1, const OUString& str2 )
    throw( RuntimeException)
{
    Sequence< OUString > ostr(2);
    ostr[0] = str1;
    ostr[1] = str2;
    return ostr;
}

static sal_uInt8 lcl_getMappingTypeForToggleCase( sal_uInt8 nMappingType, sal_Unicode cChar )
{
    sal_uInt8 nRes = nMappingType;

    
    
    
    if (nMappingType == (MappingTypeLowerToUpper | MappingTypeUpperToLower))
    {
        const sal_Int16 nType = unicode::getUnicodeType( cChar );
        if (nType & 0x02 /* lower case*/)
            nRes = MappingTypeLowerToUpper;
        else
        {
            
            nRes = MappingTypeUpperToLower;
        }
    }

    return nRes;
}

OUString SAL_CALL
Transliteration_body::transliterate(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset)
    throw(RuntimeException)
{
    const sal_Unicode *in = inStr.getStr() + startPos;

    
    
    if ( useOffset )
    {
        sal_Int32 nOffCount = 0, i;
        for (i = 0; i < nCount; i++)
        {
            
            sal_uInt8 nTmpMappingType = nMappingType;
            if (nMappingType == (MappingTypeLowerToUpper | MappingTypeUpperToLower))
                nTmpMappingType = lcl_getMappingTypeForToggleCase( nMappingType, in[i] );

            const Mapping &map = casefolding::getValue( in, i, nCount, aLocale, nTmpMappingType );
            nOffCount += map.nmap;
        }
        rtl_uString* pStr = rtl_uString_alloc(nOffCount);
        sal_Unicode* out = pStr->buffer;

        if ( nOffCount != offset.getLength() )
            offset.realloc( nOffCount );

        sal_Int32 j = 0;
        sal_Int32 * pArr = offset.getArray();
        for (i = 0; i < nCount; i++)
        {
            
            sal_uInt8 nTmpMappingType = nMappingType;
            if (nMappingType == (MappingTypeLowerToUpper | MappingTypeUpperToLower))
                nTmpMappingType = lcl_getMappingTypeForToggleCase( nMappingType, in[i] );

            const Mapping &map = casefolding::getValue( in, i, nCount, aLocale, nTmpMappingType );
            for (sal_Int32 k = 0; k < map.nmap; k++)
            {
                pArr[j] = i + startPos;
                out[j++] = map.map[k];
            }
        }
        out[j] = 0;

        return OUString( pStr, SAL_NO_ACQUIRE );
    }
    else
    {
        
        
        
        
        
        
        
        

        
        
        const sal_Int32 nLocalBuf = 2048;
        sal_Unicode aLocalBuf[ nLocalBuf * NMAPPINGMAX ], *out = aLocalBuf, *pHeapBuf = NULL;
        if ( nCount > nLocalBuf )
            out = pHeapBuf = new sal_Unicode[ nCount * NMAPPINGMAX ];

        sal_Int32 j = 0;
        for ( sal_Int32 i = 0; i < nCount; i++)
        {
            
            sal_uInt8 nTmpMappingType = nMappingType;
            if (nMappingType == (MappingTypeLowerToUpper | MappingTypeUpperToLower))
                nTmpMappingType = lcl_getMappingTypeForToggleCase( nMappingType, in[i] );

            const Mapping &map = casefolding::getValue( in, i, nCount, aLocale, nTmpMappingType );
            for (sal_Int32 k = 0; k < map.nmap; k++)
            {
                out[j++] = map.map[k];
            }
        }

        OUString aRet( out, j );
        if ( pHeapBuf )
            delete [] pHeapBuf;
        return aRet;
    }
}

OUString SAL_CALL
Transliteration_body::transliterateChar2String( sal_Unicode inChar ) throw(RuntimeException)
{
    const Mapping &map = casefolding::getValue(&inChar, 0, 1, aLocale, nMappingType);
    rtl_uString* pStr = rtl_uString_alloc(map.nmap);
    sal_Unicode* out = pStr->buffer;
    sal_Int32 i;

    for (i = 0; i < map.nmap; i++)
        out[i] = map.map[i];
    out[i] = 0;

    return OUString( pStr, SAL_NO_ACQUIRE );
}

sal_Unicode SAL_CALL
Transliteration_body::transliterateChar2Char( sal_Unicode inChar ) throw(MultipleCharsOutputException, RuntimeException)
{
    const Mapping &map = casefolding::getValue(&inChar, 0, 1, aLocale, nMappingType);
    if (map.nmap > 1)
        throw MultipleCharsOutputException();
    return map.map[0];
}

OUString SAL_CALL
Transliteration_body::folding( const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset) throw(RuntimeException)
{
    return this->transliterate(inStr, startPos, nCount, offset);
}

Transliteration_casemapping::Transliteration_casemapping()
{
    nMappingType = 0;
    transliterationName = "casemapping(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_casemapping";
}

void SAL_CALL
Transliteration_casemapping::setMappingType( const sal_uInt8 rMappingType, const Locale& rLocale )
{
    nMappingType = rMappingType;
    aLocale = rLocale;
}

Transliteration_u2l::Transliteration_u2l()
{
    nMappingType = MappingTypeUpperToLower;
    transliterationName = "upper_to_lower(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_u2l";
}

Transliteration_l2u::Transliteration_l2u()
{
    nMappingType = MappingTypeLowerToUpper;
    transliterationName = "lower_to_upper(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_l2u";
}

Transliteration_togglecase::Transliteration_togglecase()
{
    
    
    
    
    nMappingType = MappingTypeLowerToUpper | MappingTypeUpperToLower;
    transliterationName = "toggle(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_togglecase";
}

Transliteration_titlecase::Transliteration_titlecase()
{
    nMappingType = MappingTypeToTitle;
    transliterationName = "title(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_titlecase";
}

static OUString transliterate_titlecase_Impl(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    const Locale &rLocale,
    Sequence< sal_Int32 >& offset )
    throw(RuntimeException)
{
    const OUString aText( inStr.copy( startPos, nCount ) );

    OUString aRes;
    if (!aText.isEmpty())
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        CharacterClassificationImpl aCharClassImpl( xContext );

        
        

        
        sal_Int32 nPos = 0;
        sal_uInt32 cFirstChar = aText.iterateCodePoints( &nPos );
        OUString aResolvedLigature( &cFirstChar, 1 );
        
        aResolvedLigature = aCharClassImpl.toUpper( aResolvedLigature, 0, aResolvedLigature.getLength(), rLocale );
        
        
        aResolvedLigature = aCharClassImpl.toLower( aResolvedLigature, 0, aResolvedLigature.getLength(), rLocale );
        sal_Int32 nResolvedLen = aResolvedLigature.getLength();

        
        
        aRes = aCharClassImpl.toTitle( aResolvedLigature, 0, nResolvedLen, rLocale );
        aRes += aCharClassImpl.toLower( aText, 1, aText.getLength() - 1, rLocale );
        offset.realloc( aRes.getLength() );

        sal_Int32 *pOffset = offset.getArray();
        sal_Int32 nLen = offset.getLength();
        for (sal_Int32 i = 0; i < nLen; ++i)
        {
            sal_Int32 nIdx = 0;
            if (i >= nResolvedLen)
                nIdx = i - nResolvedLen + 1;
            pOffset[i] = nIdx;
        }
    }
#if OSL_DEBUG_LEVEL > 1
    const sal_Int32 *pCOffset = offset.getConstArray();
    (void) pCOffset;
#endif

    return aRes;
}



OUString SAL_CALL Transliteration_titlecase::transliterate(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset )
    throw(RuntimeException)
{
    return transliterate_titlecase_Impl( inStr, startPos, nCount, aLocale, offset );
}

Transliteration_sentencecase::Transliteration_sentencecase()
{
    nMappingType = MappingTypeToTitle;  
    transliterationName = "sentence(generic)";
    implementationName = "com.sun.star.i18n.Transliteration.Transliteration_sentencecase";
}



OUString SAL_CALL Transliteration_sentencecase::transliterate(
    const OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
    Sequence< sal_Int32 >& offset )
    throw(RuntimeException)
{
    return transliterate_titlecase_Impl( inStr, startPos, nCount, aLocale, offset );
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
