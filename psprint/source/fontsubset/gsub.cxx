/*************************************************************************
 *
 *  $RCSfile: gsub.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pl $ $Date: 2002-05-29 08:49:19 $
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

extern "C"
{
#include "sft.h"
#undef true
#undef false

#include "gsub.h"
}

#include <vector>
#include <map>
#include <algorithm>

typedef uint32 ULONG;
typedef uint16 USHORT;
typedef uint8 FT_Byte;

typedef std::map<USHORT,USHORT> GlyphSubstitution;


inline long NEXT_Long( const unsigned char* &p )
{
    long nVal = (p[0]<<24) + (p[1]<<16) + (p[2]<<8) + p[3];
    p += 4;
    return nVal;
}

inline long NEXT_UShort( const unsigned char* &p )
{
    long nVal = (p[0]<<8) + p[1];
    p += 2;
    return nVal;
}

#define MKTAG(s) ((((((s[0]<<8)+s[1])<<8)+s[2])<<8)+s[3])

int ReadGSUB( struct _TrueTypeFont* pTTFile, unsigned char* pGsubBase,
              int nRequestedScript, int nRequestedLangsys )
{
    if( !pGsubBase )
        return -1;

    // parse GSUB header
    const FT_Byte* pGsubHeader = pGsubBase;
    const ULONG nVersion            = NEXT_Long( pGsubHeader );
    const USHORT nOfsScriptList     = NEXT_UShort( pGsubHeader );
    const USHORT nOfsFeatureTable   = NEXT_UShort( pGsubHeader );
    const USHORT nOfsLookupList     = NEXT_UShort( pGsubHeader );

    // sanity check
    if( nVersion != 0x00010000 )
        return -1; // unknown format or broken

    typedef std::vector<ULONG> ReqFeatureTagList;
    ReqFeatureTagList aReqFeatureTagList;

    aReqFeatureTagList.push_back( MKTAG("vert") );

    typedef std::vector<USHORT> UshortList;
    UshortList aFeatureIndexList;
    UshortList aFeatureOffsetList;

    // parse Script Table
    const FT_Byte* pScriptHeader = pGsubBase + nOfsScriptList;
    const USHORT nCntScript = NEXT_UShort( pScriptHeader );
    for( USHORT nScriptIndex = 0; nScriptIndex < nCntScript; ++nScriptIndex )
    {
        const ULONG nTag            = NEXT_Long( pScriptHeader ); // e.g. hani/arab/kana/hang
        const USHORT nOfsScriptTable= NEXT_UShort( pScriptHeader );
        if( (nTag != nRequestedScript) && (nRequestedScript != 0) )
            continue;

        const FT_Byte* pScriptTable     = pGsubBase + nOfsScriptList + nOfsScriptTable;
        const USHORT nDefaultLangsysOfs = NEXT_UShort( pScriptTable );
        const USHORT nCntLangSystem     = NEXT_UShort( pScriptTable );
        USHORT nLangsysOffset = 0;
        for( USHORT nLangsysIndex = 0; nLangsysIndex < nCntLangSystem; ++nLangsysIndex )
        {
            const ULONG nTag    = NEXT_Long( pScriptTable );    // e.g. KOR/ZHS/ZHT/JAN
            const USHORT nOffset= NEXT_UShort( pScriptTable );
            if( (nTag != nRequestedLangsys) && (nRequestedLangsys != 0) )
                continue;
            nLangsysOffset = nOffset;
            break;
        }

        if( (nDefaultLangsysOfs != 0) && (nDefaultLangsysOfs != nLangsysOffset) )
        {
            const FT_Byte* pLangSys = pGsubBase + nOfsScriptList + nOfsScriptTable + nDefaultLangsysOfs;
            const USHORT nLookupOrder   = NEXT_UShort( pLangSys );
            const USHORT nReqFeatureIdx = NEXT_UShort( pLangSys );
            const USHORT nCntFeature    = NEXT_UShort( pLangSys );
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
            const USHORT nLookupOrder   = NEXT_UShort( pLangSys );
            const USHORT nReqFeatureIdx = NEXT_UShort( pLangSys );
            const USHORT nCntFeature    = NEXT_UShort( pLangSys );
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
    const USHORT nCntFeature = NEXT_UShort( pFeatureHeader );
    for( USHORT nFeatureIndex = 0; nFeatureIndex < nCntFeature; ++nFeatureIndex )
    {
        const ULONG nTag    = NEXT_Long( pFeatureHeader ); // e.g. locl/vert/trad/smpl/liga/fina/...
        const USHORT nOffset= NEXT_UShort( pFeatureHeader );

        // feature (required && (requested || available))?
        if( (aFeatureIndexList[0] != nFeatureIndex)
            &&  (!std::count( aReqFeatureTagList.begin(), aReqFeatureTagList.end(), nTag))
            ||  (!std::count( aFeatureIndexList.begin(), aFeatureIndexList.end(), nFeatureIndex) ) )
            continue;

        const FT_Byte* pFeatureTable = pGsubBase + nOfsFeatureTable + nOffset;
        const USHORT nCntLookups = NEXT_UShort( pFeatureTable );
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
    const USHORT nCntLookupTable = NEXT_UShort( pLookupHeader );
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
        const USHORT eLookupType        = NEXT_UShort( pLookupTable );
        const USHORT eLookupFlag        = NEXT_UShort( pLookupTable );
        const USHORT nCntLookupSubtable = NEXT_UShort( pLookupTable );

        // TODO: switch( eLookupType )
        if( eLookupType != 1 )  // TODO: once we go beyond SingleSubst
            continue;

        for( USHORT nSubTableIdx = 0; nSubTableIdx < nCntLookupSubtable; ++nSubTableIdx )
        {
            const USHORT nOfsSubLookupTable = NEXT_UShort( pLookupTable );
            const FT_Byte* pSubLookup = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable;

            const USHORT nFmtSubstitution   = NEXT_UShort( pSubLookup );
            const USHORT nOfsCoverage       = NEXT_UShort( pSubLookup );

            typedef std::pair<USHORT,USHORT> GlyphSubst;
            typedef std::vector<GlyphSubst> SubstVector;
            SubstVector aSubstVector;

            const FT_Byte* pCoverage    = pGsubBase + nOfsLookupList + nOfsLookupTable + nOfsSubLookupTable + nOfsCoverage;
            const USHORT nFmtCoverage   = NEXT_UShort( pCoverage );
            switch( nFmtCoverage )
            {
                case 1:         // Coverage Format 1
                {
                    const USHORT nCntGlyph = NEXT_UShort( pCoverage );
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
                    for( int i = nCntRange; --i >= 0; )
                    {
                        const USHORT nGlyph0 = NEXT_UShort( pCoverage );
                        const USHORT nGlyph1 = NEXT_UShort( pCoverage );
                        const USHORT nCovIdx = NEXT_UShort( pCoverage );
                        for( USHORT j = nGlyph0; j <= nGlyph1; ++j )
                            aSubstVector.push_back( GlyphSubst( j + nCovIdx, 0 ) );
                    }
                }
                break;
            }

            SubstVector::iterator it( aSubstVector.begin() );

            switch( nFmtSubstitution )
            {
                case 1:     // Single Substitution Format 1
                {
                    const USHORT nDeltaGlyphId = NEXT_UShort( pSubLookup );

                    for(; it != aSubstVector.end(); ++it )
                        (*it).second = (*it).first + nDeltaGlyphId;
                }
                break;

                case 2:     // Single Substitution Format 2
                {
                    const USHORT nCntGlyph = NEXT_UShort( pSubLookup );
                    for( int i = nCntGlyph; (it != aSubstVector.end()) && (--i>=0); ++it )
                    {
                        const USHORT nGlyphId = NEXT_UShort( pSubLookup );
                        (*it).second = nGlyphId;
                    }
                }
                break;
            }

            // now apply the glyph substitutions that have been collected in this subtable
            if( aSubstVector.size() > 0 )
            {
                GlyphSubstitution* pGSubstitution = new GlyphSubstitution;
                pTTFile->pGSubstitution = (void*)pGSubstitution;
                for( it = aSubstVector.begin(); it != aSubstVector.end(); ++it )
                    (*pGSubstitution)[ (*it).first ] =  (*it).second;
            }
        }
    }

    return true;
}

int UseGSUB( struct _TrueTypeFont* pTTFile, int nGlyph, int wmode )
{
    GlyphSubstitution* pGlyphSubstitution = (GlyphSubstitution*)pTTFile->pGSubstitution;
    if( pGlyphSubstitution != 0 )
    {
        GlyphSubstitution::const_iterator it( pGlyphSubstitution->find( nGlyph ) );
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
