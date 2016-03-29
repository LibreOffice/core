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

#include "sft.hxx"

#include "gsub.h"

#include <osl/diagnose.h>

#include <vector>
#include <map>
#include <algorithm>

namespace vcl
{

typedef sal_uInt8 FT_Byte;

typedef std::map<sal_uInt16,sal_uInt16> GlyphSubstitution;

inline sal_uInt32 NEXT_Long( const unsigned char* &p )
{
    sal_uInt32 nVal = (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
    p += 4;
    return nVal;
}

inline sal_uInt16 NEXT_UShort( const unsigned char* &p )
{
    sal_uInt16 nVal = (p[0]<<8) + p[1];
    p += 2;
    return nVal;
}

#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

bool ReadGSUB( struct TrueTypeFont* pTTFile,
    int nRequestedScript, int nRequestedLangsys )
{
    const FT_Byte* pGsubBase = pTTFile->tables[ O_gsub ];
    if( !pGsubBase )
        return false;

    // #129682# check offsets inside GSUB table
    const FT_Byte* pGsubLimit = pGsubBase + pTTFile->tlens[ O_gsub ];

    // parse GSUB header
    const FT_Byte* pGsubHeader = pGsubBase;
    const sal_uInt32 nVersion           = NEXT_Long( pGsubHeader );
    const sal_uInt16 nOfsScriptList     = NEXT_UShort( pGsubHeader );
    const sal_uInt16 nOfsFeatureTable   = NEXT_UShort( pGsubHeader );
    const sal_uInt16 nOfsLookupList     = NEXT_UShort( pGsubHeader );

    // sanity check the GSUB header
    if( nVersion != 0x00010000 )
        if( nVersion != 0x00001000 )    // workaround for SunBatang etc.
            return false;               // unknown format or broken

    typedef std::vector<sal_uInt32> ReqFeatureTagList;
    ReqFeatureTagList aReqFeatureTagList;

    aReqFeatureTagList.push_back( MKTAG("vert") );

    std::vector<sal_uInt16> aFeatureIndexList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const sal_uInt16 nCntScript = NEXT_UShort( pScriptHeader );
    if( pGsubLimit < pScriptHeader + 6 * nCntScript )
        return false;
    for( sal_uInt16 nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const sal_uInt32 nTag            = NEXT_Long( pScriptHeader ); // e.g. hani/arab/kana/hang
        const sal_uInt16 nOfsScriptTable= NEXT_UShort( pScriptHeader );
        if( (nTag != (sal_uInt16)nRequestedScript) && (nRequestedScript != 0) )
            continue;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        if( pGsubLimit < pScriptTable + 4 )
            return false;
        const sal_uInt16 nDefaultLangsysOfs = NEXT_UShort( pScriptTable );
        const sal_uInt16 nCntLangSystem     = NEXT_UShort( pScriptTable );
        sal_uInt16 nLangsysOffset = 0;
        if( pGsubLimit < pScriptTable + 6 * nCntLangSystem )
            return false;
        for( sal_uInt16 nLangsysIndex = 0; nLangsysIndex < nCntLangSystem; ++nLangsysIndex )
        {
            const sal_uInt32 nInnerTag = NEXT_Long( pScriptTable );    // e.g. KOR/ZHS/ZHT/JAN
            const sal_uInt16 nOffset= NEXT_UShort( pScriptTable );
            if( (nInnerTag != (sal_uInt16)nRequestedLangsys) && (nRequestedLangsys != 0) )
                continue;
            nLangsysOffset = nOffset;
            break;
        }

        if( (nDefaultLangsysOfs != 0) && (nDefaultLangsysOfs != nLangsysOffset) )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nDefaultLangsysOfs;
            if( pGsubLimit < pLangSys + 6 )
                return false;
            /*const sal_uInt16 nLookupOrder   =*/ NEXT_UShort( pLangSys );
            const sal_uInt16 nReqFeatureIdx = NEXT_UShort( pLangSys );
            const sal_uInt16 nCntFeature    = NEXT_UShort( pLangSys );
            if( pGsubLimit < pLangSys + 2 * nCntFeature )
                return false;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( sal_uInt16 i = 0; i < nCntFeature; ++i )
            {
                const sal_uInt16 nFeatureIndex = NEXT_UShort( pLangSys );
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }

        if( nLangsysOffset != 0 )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nLangsysOffset;
            if( pGsubLimit < pLangSys + 6 )
                return false;
            /*const sal_uInt16 nLookupOrder   =*/ NEXT_UShort( pLangSys );
            const sal_uInt16 nReqFeatureIdx = NEXT_UShort( pLangSys );
            const sal_uInt16 nCntFeature    = NEXT_UShort( pLangSys );
            if( pGsubLimit < pLangSys + 2 * nCntFeature )
                return false;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( sal_uInt16 i = 0; i < nCntFeature; ++i )
            {
                const sal_uInt16 nFeatureIndex = NEXT_UShort( pLangSys );
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }
    }

    if( aFeatureIndexList.empty() )
        return true;

    std::vector<sal_uInt16> aLookupIndexList;
    std::vector<sal_uInt16> aLookupOffsetList;

    // parse Feature Table
    const FT_Byte* pFeatureHeader = pGsubBase + nOfsFeatureTable;
    if( pGsubLimit < pFeatureHeader + 2 )
          return false;
    const sal_uInt16 nCntFeature = NEXT_UShort( pFeatureHeader );
    if( pGsubLimit < pFeatureHeader + 6 * nCntFeature )
          return false;
    for( sal_uInt16 nFeatureIndex = 0; nFeatureIndex < nCntFeature; ++nFeatureIndex )
    {
        const sal_uInt32 nTag    = NEXT_Long( pFeatureHeader ); // e.g. locl/vert/trad/smpl/liga/fina/...
        const sal_uInt16 nOffset= NEXT_UShort( pFeatureHeader );

        // ignore unneeded feature lookups
        if( aFeatureIndexList[0] != nFeatureIndex ) // do not ignore the required feature
        {
            const int nRequested = std::count( aFeatureIndexList.begin(), aFeatureIndexList.end(), nFeatureIndex);
            if( !nRequested )   // ignore features that are not requested
                continue;
            const int nAvailable = std::count( aReqFeatureTagList.begin(), aReqFeatureTagList.end(), nTag);
            if( !nAvailable )   // some fonts don't provide features they request!
                continue;
        }

        const FT_Byte* pFeatureTable = pGsubBase + nOfsFeatureTable + nOffset;
        if( pGsubLimit < pFeatureTable + 2 )
            return false;
        const sal_uInt16 nCntLookups = NEXT_UShort( pFeatureTable );
        if( pGsubLimit < pFeatureTable + 2 * nCntLookups )
            return false;
        for( sal_uInt16 i = 0; i < nCntLookups; ++i )
        {
            const sal_uInt16 nLookupIndex = NEXT_UShort( pFeatureTable );
            aLookupIndexList.push_back( nLookupIndex );
        }
        if( nCntLookups == 0 ) //### hack needed by Mincho/Gothic/Mingliu/Simsun/...
            aLookupIndexList.push_back( 0 );
    }

    // parse Lookup List
    const FT_Byte* pLookupHeader = pGsubBase + nOfsLookupList;
    if( pGsubLimit < pLookupHeader + 2 )
        return false;
    const sal_uInt16 nCntLookupTable = NEXT_UShort( pLookupHeader );
    if( pGsubLimit < pLookupHeader + 2 * nCntLookupTable )
        return false;
    for( sal_uInt16 nLookupIdx = 0; nLookupIdx < nCntLookupTable; ++nLookupIdx )
    {
        const sal_uInt16 nOffset = NEXT_UShort( pLookupHeader );
        if( std::count( aLookupIndexList.begin(), aLookupIndexList.end(), nLookupIdx ) )
            aLookupOffsetList.push_back( nOffset );
    }

    std::vector<sal_uInt16>::const_iterator it = aLookupOffsetList.begin();
    for(; it != aLookupOffsetList.end(); ++it )
    {
        const sal_uInt16 nOfsLookupTable = *it;
        const FT_Byte* pLookupTable = pGsubBase + nOfsLookupList + nOfsLookupTable;
        if( pGsubLimit < pLookupTable + 6 )
            return false;
        const sal_uInt16 eLookupType        = NEXT_UShort( pLookupTable );
        /*const sal_uInt16 eLookupFlag        =*/ NEXT_UShort( pLookupTable );
        const sal_uInt16 nCntLookupSubtable = NEXT_UShort( pLookupTable );

        // TODO: switch( eLookupType )
        if( eLookupType != 1 )  // TODO: once we go beyond SingleSubst
            continue;

        if( pGsubLimit < pLookupTable + 2 * nCntLookupSubtable )
            return false;
        for( sal_uInt16 nSubTableIdx = 0; nSubTableIdx < nCntLookupSubtable; ++nSubTableIdx )
        {
            const sal_uInt16 nOfsSubLookupTable = NEXT_UShort( pLookupTable );
            const FT_Byte* pSubLookup = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable;
            if( pGsubLimit < pSubLookup + 6 )
                return false;
            const sal_uInt16 nFmtSubstitution   = NEXT_UShort( pSubLookup );
            const sal_uInt16 nOfsCoverage       = NEXT_UShort( pSubLookup );

            typedef std::pair<sal_uInt16,sal_uInt16> GlyphSubst;
            typedef std::vector<GlyphSubst> SubstVector;
            SubstVector aSubstVector;

            const FT_Byte* pCoverage    = pGsubBase
                + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable + nOfsCoverage;
            if( pGsubLimit < pCoverage + 4 )
                return false;
            const sal_uInt16 nFmtCoverage   = NEXT_UShort( pCoverage );
            switch( nFmtCoverage )
            {
                case 1:         // Coverage Format 1
                {
                    const sal_uInt16 nCntGlyph = NEXT_UShort( pCoverage );
                    if( pGsubLimit < pCoverage + 2 * nCntGlyph )
                        // TODO? nCntGlyph = (pGsubLimit - pCoverage) / 2;
                        return false;
                    aSubstVector.reserve( nCntGlyph );
                    for( sal_uInt16 i = 0; i < nCntGlyph; ++i )
                    {
                        const sal_uInt16 nGlyphId = NEXT_UShort( pCoverage );
                        aSubstVector.push_back( GlyphSubst( nGlyphId, 0 ) );
                    }
                }
                break;

                case 2:         // Coverage Format 2
                {
                    const sal_uInt16 nCntRange = NEXT_UShort( pCoverage );
                    if( pGsubLimit < pCoverage + 6 * nCntRange )
                        // TODO? nCntGlyph = (pGsubLimit - pCoverage) / 6;
                        return false;
                    for( int i = nCntRange; --i >= 0; )
                    {
                        const sal_uInt32 nGlyph0 = NEXT_UShort( pCoverage );
                        const sal_uInt32 nGlyph1 = NEXT_UShort( pCoverage );
                        const sal_uInt16 nCovIdx = NEXT_UShort( pCoverage );
                        for( sal_uInt32 j = nGlyph0; j <= nGlyph1; ++j )
                            aSubstVector.push_back( GlyphSubst( static_cast<sal_uInt16>(j + nCovIdx), 0 ) );
                    }
                }
                break;
            }

            SubstVector::iterator subst_it( aSubstVector.begin() );

            switch( nFmtSubstitution )
            {
                case 1:     // Single Substitution Format 1
                {
                    const sal_uInt16 nDeltaGlyphId = NEXT_UShort( pSubLookup );

                    for(; subst_it != aSubstVector.end(); ++subst_it )
                        (*subst_it).second = (*subst_it).first + nDeltaGlyphId;
                }
                break;

                case 2:     // Single Substitution Format 2
                {
                    const sal_uInt16 nCntGlyph = NEXT_UShort( pSubLookup );
                    for( int i = nCntGlyph; (subst_it != aSubstVector.end()) && (--i>=0); ++subst_it )
                    {
                        if( pGsubLimit < pSubLookup + 2 )
                            return false;
                        const sal_uInt16 nGlyphId = NEXT_UShort( pSubLookup );
                        (*subst_it).second = nGlyphId;
                    }
                }
                break;
            }

            // now apply the glyph substitutions that have been collected in this subtable
            if( !aSubstVector.empty() )
            {
                GlyphSubstitution* pGSubstitution = new GlyphSubstitution;
                pTTFile->pGSubstitution = static_cast<void*>(pGSubstitution);
                for( subst_it = aSubstVector.begin(); subst_it != aSubstVector.end(); ++subst_it )
                    (*pGSubstitution)[ (*subst_it).first ] =  (*subst_it).second;
            }
        }
    }
    return true;
}

void ReleaseGSUB(struct TrueTypeFont* pTTFile)
{
    GlyphSubstitution* pGlyphSubstitution = static_cast<GlyphSubstitution*>(pTTFile->pGSubstitution);
    delete pGlyphSubstitution;
}

int UseGSUB( struct TrueTypeFont* pTTFile, int nGlyph )
{
    GlyphSubstitution* pGlyphSubstitution = static_cast<GlyphSubstitution*>(pTTFile->pGSubstitution);
    if( pGlyphSubstitution != nullptr )
    {
        GlyphSubstitution::const_iterator it( pGlyphSubstitution->find( sal::static_int_cast<sal_uInt16>(nGlyph) ) );
        if( it != pGlyphSubstitution->end() )
            nGlyph = (*it).second;
    }

    return nGlyph;
}

int HasVerticalGSUB( struct TrueTypeFont* pTTFile )
{
    GlyphSubstitution* pGlyphSubstitution = static_cast<GlyphSubstitution*>(pTTFile->pGSubstitution);
    return pGlyphSubstitution ? +1 : 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
