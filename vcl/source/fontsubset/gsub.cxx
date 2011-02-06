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
#include "precompiled_vcl.hxx"

#include "sft.hxx"

#include "gsub.h"

#include <osl/diagnose.h>

#include <vector>
#include <map>
#include <algorithm>

namespace vcl
{

typedef sal_uInt32 ULONG;
typedef sal_uInt32 UINT32;
typedef sal_uInt16 USHORT;
typedef sal_uInt8 FT_Byte;

typedef std::map<USHORT,USHORT> GlyphSubstitution;


inline sal_uInt32 NEXT_Long( const unsigned char* &p )
{
    sal_uInt32 nVal = (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
    p += 4;
    return nVal;
}

inline USHORT NEXT_UShort( const unsigned char* &p )
{
    USHORT nVal = (p[0]<<8) + p[1];
    p += 2;
    return nVal;
}

#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

int ReadGSUB( struct _TrueTypeFont* pTTFile,
    int nRequestedScript, int nRequestedLangsys )
{
    const FT_Byte* pGsubBase = (FT_Byte*)pTTFile->tables[ O_gsub ];
    if( !pGsubBase )
        return -1;

    // #129682# check offsets inside GSUB table
    const FT_Byte* pGsubLimit = pGsubBase + pTTFile->tlens[ O_gsub ];

    // parse GSUB header
    const FT_Byte* pGsubHeader = pGsubBase;
    const ULONG nVersion            = NEXT_Long( pGsubHeader );
    const USHORT nOfsScriptList     = NEXT_UShort( pGsubHeader );
    const USHORT nOfsFeatureTable   = NEXT_UShort( pGsubHeader );
    const USHORT nOfsLookupList     = NEXT_UShort( pGsubHeader );

    // sanity check the GSUB header
    if( nVersion != 0x00010000 )
        if( nVersion != 0x00001000 )    // workaround for SunBatang etc.
            return -1;                  // unknown format or broken

    typedef std::vector<ULONG> ReqFeatureTagList;
    ReqFeatureTagList aReqFeatureTagList;

    aReqFeatureTagList.push_back( MKTAG("vert") );

    typedef std::vector<USHORT> UshortList;
    UshortList aFeatureIndexList;
    UshortList aFeatureOffsetList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const USHORT nCntScript = NEXT_UShort( pScriptHeader );
    if( pGsubLimit < pScriptHeader + 6 * nCntScript )
        return false;
    for( USHORT nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const ULONG nTag            = NEXT_Long( pScriptHeader ); // e.g. hani/arab/kana/hang
        const USHORT nOfsScriptTable= NEXT_UShort( pScriptHeader );
        if( (nTag != (USHORT)nRequestedScript) && (nRequestedScript != 0) )
            continue;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        if( pGsubLimit < pScriptTable + 4 )
            return false;
        const USHORT nDefaultLangsysOfs = NEXT_UShort( pScriptTable );
        const USHORT nCntLangSystem     = NEXT_UShort( pScriptTable );
        USHORT nLangsysOffset = 0;
        if( pGsubLimit < pScriptTable + 6 * nCntLangSystem )
            return false;
        for( USHORT nLangsysIndex = 0; nLangsysIndex < nCntLangSystem; ++nLangsysIndex )
        {
            const ULONG nInnerTag = NEXT_Long( pScriptTable );    // e.g. KOR/ZHS/ZHT/JAN
            const USHORT nOffset= NEXT_UShort( pScriptTable );
            if( (nInnerTag != (USHORT)nRequestedLangsys) && (nRequestedLangsys != 0) )
                continue;
            nLangsysOffset = nOffset;
            break;
        }

        if( (nDefaultLangsysOfs != 0) && (nDefaultLangsysOfs != nLangsysOffset) )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nDefaultLangsysOfs;
            if( pGsubLimit < pLangSys + 6 )
                return false;
            /*const USHORT nLookupOrder   =*/ NEXT_UShort( pLangSys );
            const USHORT nReqFeatureIdx = NEXT_UShort( pLangSys );
            const USHORT nCntFeature    = NEXT_UShort( pLangSys );
            if( pGsubLimit < pLangSys + 2 * nCntFeature )
                return false;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( USHORT i = 0; i < nCntFeature; ++i )
            {
                const USHORT nFeatureIndex = NEXT_UShort( pLangSys );
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }

        if( nLangsysOffset != 0 )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nLangsysOffset;
            if( pGsubLimit < pLangSys + 6 )
                return false;
            /*const USHORT nLookupOrder   =*/ NEXT_UShort( pLangSys );
            const USHORT nReqFeatureIdx = NEXT_UShort( pLangSys );
            const USHORT nCntFeature    = NEXT_UShort( pLangSys );
            if( pGsubLimit < pLangSys + 2 * nCntFeature )
                return false;
            aFeatureIndexList.push_back( nReqFeatureIdx );
            for( USHORT i = 0; i < nCntFeature; ++i )
            {
                const USHORT nFeatureIndex = NEXT_UShort( pLangSys );
                aFeatureIndexList.push_back( nFeatureIndex );
            }
        }
    }

    if( !aFeatureIndexList.size() )
        return true;

    UshortList aLookupIndexList;
    UshortList aLookupOffsetList;

    // parse Feature Table
    const FT_Byte* pFeatureHeader = pGsubBase + nOfsFeatureTable;
    if( pGsubLimit < pFeatureHeader + 2 )
          return false;
    const USHORT nCntFeature = NEXT_UShort( pFeatureHeader );
    if( pGsubLimit < pFeatureHeader + 6 * nCntFeature )
          return false;
    for( USHORT nFeatureIndex = 0; nFeatureIndex < nCntFeature; ++nFeatureIndex )
    {
        const ULONG nTag    = NEXT_Long( pFeatureHeader ); // e.g. locl/vert/trad/smpl/liga/fina/...
        const USHORT nOffset= NEXT_UShort( pFeatureHeader );

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
        const USHORT nCntLookups = NEXT_UShort( pFeatureTable );
        if( pGsubLimit < pFeatureTable + 2 * nCntLookups )
            return false;
        for( USHORT i = 0; i < nCntLookups; ++i )
        {
            const USHORT nLookupIndex = NEXT_UShort( pFeatureTable );
            aLookupIndexList.push_back( nLookupIndex );
        }
        if( nCntLookups == 0 ) //### hack needed by Mincho/Gothic/Mingliu/Simsun/...
            aLookupIndexList.push_back( 0 );
    }

    // parse Lookup List
    const FT_Byte* pLookupHeader = pGsubBase + nOfsLookupList;
    if( pGsubLimit < pLookupHeader + 2 )
        return false;
    const USHORT nCntLookupTable = NEXT_UShort( pLookupHeader );
    if( pGsubLimit < pLookupHeader + 2 * nCntLookupTable )
        return false;
    for( USHORT nLookupIdx = 0; nLookupIdx < nCntLookupTable; ++nLookupIdx )
    {
        const USHORT nOffset = NEXT_UShort( pLookupHeader );
        if( std::count( aLookupIndexList.begin(), aLookupIndexList.end(), nLookupIdx ) )
            aLookupOffsetList.push_back( nOffset );
    }

    UshortList::const_iterator it = aLookupOffsetList.begin();
    for(; it != aLookupOffsetList.end(); ++it )
    {
        const USHORT nOfsLookupTable = *it;
        const FT_Byte* pLookupTable = pGsubBase + nOfsLookupList + nOfsLookupTable;
        if( pGsubLimit < pLookupTable + 6 )
            return false;
        const USHORT eLookupType        = NEXT_UShort( pLookupTable );
        /*const USHORT eLookupFlag        =*/ NEXT_UShort( pLookupTable );
        const USHORT nCntLookupSubtable = NEXT_UShort( pLookupTable );

        // TODO: switch( eLookupType )
        if( eLookupType != 1 )  // TODO: once we go beyond SingleSubst
            continue;

        if( pGsubLimit < pLookupTable + 2 * nCntLookupSubtable )
            return false;
        for( USHORT nSubTableIdx = 0; nSubTableIdx < nCntLookupSubtable; ++nSubTableIdx )
        {
            const USHORT nOfsSubLookupTable = NEXT_UShort( pLookupTable );
            const FT_Byte* pSubLookup = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable;
            if( pGsubLimit < pSubLookup + 6 )
                return false;
            const USHORT nFmtSubstitution   = NEXT_UShort( pSubLookup );
            const USHORT nOfsCoverage       = NEXT_UShort( pSubLookup );

            typedef std::pair<USHORT,USHORT> GlyphSubst;
            typedef std::vector<GlyphSubst> SubstVector;
            SubstVector aSubstVector;

            const FT_Byte* pCoverage    = pGsubBase
                + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable + nOfsCoverage;
            if( pGsubLimit < pCoverage + 4 )
                return false;
            const USHORT nFmtCoverage   = NEXT_UShort( pCoverage );
            switch( nFmtCoverage )
            {
                case 1:         // Coverage Format 1
                {
                    const USHORT nCntGlyph = NEXT_UShort( pCoverage );
                    if( pGsubLimit < pCoverage + 2 * nCntGlyph )
                        // TODO? nCntGlyph = (pGsubLimit - pCoverage) / 2;
                        return false;
                    aSubstVector.reserve( nCntGlyph );
                    for( USHORT i = 0; i < nCntGlyph; ++i )
                    {
                        const USHORT nGlyphId = NEXT_UShort( pCoverage );
                        aSubstVector.push_back( GlyphSubst( nGlyphId, 0 ) );
                    }
                }
                break;

                case 2:         // Coverage Format 2
                {
                    const USHORT nCntRange = NEXT_UShort( pCoverage );
                    if( pGsubLimit < pCoverage + 6 * nCntRange )
                        // TODO? nCntGlyph = (pGsubLimit - pCoverage) / 6;
                        return false;
                    for( int i = nCntRange; --i >= 0; )
                    {
                        const UINT32 nGlyph0 = NEXT_UShort( pCoverage );
                        const UINT32 nGlyph1 = NEXT_UShort( pCoverage );
                        const USHORT nCovIdx = NEXT_UShort( pCoverage );
                        for( UINT32 j = nGlyph0; j <= nGlyph1; ++j )
                            aSubstVector.push_back( GlyphSubst( static_cast<USHORT>(j + nCovIdx), 0 ) );
                    }
                }
                break;
            }

            SubstVector::iterator subst_it( aSubstVector.begin() );

            switch( nFmtSubstitution )
            {
                case 1:     // Single Substitution Format 1
                {
                    const USHORT nDeltaGlyphId = NEXT_UShort( pSubLookup );

                    for(; subst_it != aSubstVector.end(); ++subst_it )
                        (*subst_it).second = (*subst_it).first + nDeltaGlyphId;
                }
                break;

                case 2:     // Single Substitution Format 2
                {
                    const USHORT nCntGlyph = NEXT_UShort( pSubLookup );
                    for( int i = nCntGlyph; (subst_it != aSubstVector.end()) && (--i>=0); ++subst_it )
                    {
                        if( pGsubLimit < pSubLookup + 2 )
                            return false;
                        const USHORT nGlyphId = NEXT_UShort( pSubLookup );
                        (*subst_it).second = nGlyphId;
                    }
                }
                break;
            }

            // now apply the glyph substitutions that have been collected in this subtable
            if( aSubstVector.size() > 0 )
            {
                GlyphSubstitution* pGSubstitution = new GlyphSubstitution;
                pTTFile->pGSubstitution = (void*)pGSubstitution;
                for( subst_it = aSubstVector.begin(); subst_it != aSubstVector.end(); ++subst_it )
                    (*pGSubstitution)[ (*subst_it).first ] =  (*subst_it).second;
            }
        }
    }
    return true;
}

void ReleaseGSUB(struct _TrueTypeFont* pTTFile)
{
    GlyphSubstitution* pGlyphSubstitution = (GlyphSubstitution*)pTTFile->pGSubstitution;
    if( pGlyphSubstitution )
        delete pGlyphSubstitution;
}

int UseGSUB( struct _TrueTypeFont* pTTFile, int nGlyph, int /*wmode*/ )
{
    GlyphSubstitution* pGlyphSubstitution = (GlyphSubstitution*)pTTFile->pGSubstitution;
    if( pGlyphSubstitution != 0 )
    {
        GlyphSubstitution::const_iterator it( pGlyphSubstitution->find( sal::static_int_cast<USHORT>(nGlyph) ) );
        if( it != pGlyphSubstitution->end() )
            nGlyph = (*it).second;
    }

    return nGlyph;
}

int HasVerticalGSUB( struct _TrueTypeFont* pTTFile )
{
    GlyphSubstitution* pGlyphSubstitution = (GlyphSubstitution*)pTTFile->pGSubstitution;
    return pGlyphSubstitution ? +1 : 0;
}

void getTTFontLayoutCapabilities(FontLayoutCapabilities &rFontLayoutCapabilities, const unsigned char* pBase)
{
    // parse GSUB/GPOS header
    const FT_Byte* pGsubHeader = pBase;
    pGsubHeader+=4;
    const USHORT nOfsScriptList = NEXT_UShort(pGsubHeader);

    // parse Script Table
    const FT_Byte* pScriptHeader = pBase + nOfsScriptList;
    const USHORT nCntScript = NEXT_UShort(pScriptHeader);
    for( USHORT nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        sal_uInt32 nTag = NEXT_Long(pScriptHeader);
        pScriptHeader += 2;
        rFontLayoutCapabilities.push_back(nTag); // e.g. hani/arab/kana/hang
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
