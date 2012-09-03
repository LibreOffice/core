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


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <gcach_ftyp.hxx>

#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <outfont.hxx>

#ifdef ENABLE_GRAPHITE
#include <graphite_features.hxx>
#endif

#include <rtl/ustring.hxx>      // used only for string=>hashvalue
#include <osl/file.hxx>
#include <tools/debug.hxx>

// =======================================================================
// GlyphCache
// =======================================================================

static GlyphCache* pInstance = NULL;

GlyphCache::GlyphCache( GlyphCachePeer& rPeer )
:   mrPeer( rPeer ),
    mnMaxSize( 1500000 ),
    mnBytesUsed(sizeof(GlyphCache)),
    mnLruIndex(0),
    mnGlyphCount(0),
    mpCurrentGCFont(NULL),
    mpFtManager(NULL)
{
    pInstance = this;
    mpFtManager = new FreetypeManager;
}

// -----------------------------------------------------------------------

GlyphCache::~GlyphCache()
{
    InvalidateAllGlyphs();
    delete mpFtManager;
}

// -----------------------------------------------------------------------

void GlyphCache::InvalidateAllGlyphs()
{
    for( FontList::iterator it = maFontList.begin(), end = maFontList.end(); it != end; ++it )
    {
        ServerFont* pServerFont = it->second;
        mrPeer.RemovingFont(*pServerFont);
        delete pServerFont;
    }

    maFontList.clear();
    mpCurrentGCFont = NULL;
}

// -----------------------------------------------------------------------

inline
size_t GlyphCache::IFSD_Hash::operator()( const FontSelectPattern& rFontSelData ) const
{
    // TODO: is it worth to improve this hash function?
    sal_IntPtr nFontId = reinterpret_cast<sal_IntPtr>( rFontSelData.mpFontData );
#ifdef ENABLE_GRAPHITE
    if (rFontSelData.maTargetName.Search(grutils::GrFeatureParser::FEAT_PREFIX)
        != STRING_NOTFOUND)
    {
        rtl::OString aFeatName = rtl::OUStringToOString( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
        nFontId ^= aFeatName.hashCode();
    }
#endif
    size_t nHash = nFontId << 8;
    nHash   += rFontSelData.mnHeight;
    nHash   += rFontSelData.mnOrientation;
    nHash   += rFontSelData.mbVertical;
    nHash   += rFontSelData.meItalic;
    nHash   += rFontSelData.meWeight;
#ifdef ENABLE_GRAPHITE
    nHash   += rFontSelData.meLanguage;
#endif
    return nHash;
}

// -----------------------------------------------------------------------

bool GlyphCache::IFSD_Equal::operator()( const FontSelectPattern& rA, const FontSelectPattern& rB) const
{
    // check font ids
    sal_IntPtr nFontIdA = reinterpret_cast<sal_IntPtr>( rA.mpFontData );
    sal_IntPtr nFontIdB = reinterpret_cast<sal_IntPtr>( rB.mpFontData );
    if( nFontIdA != nFontIdB )
        return false;

    // compare with the requested metrics
    if( (rA.mnHeight         != rB.mnHeight)
    ||  (rA.mnOrientation    != rB.mnOrientation)
    ||  (rA.mbVertical       != rB.mbVertical)
    ||  (rA.mbNonAntialiased != rB.mbNonAntialiased) )
        return false;

    if( (rA.meItalic != rB.meItalic)
    ||  (rA.meWeight != rB.meWeight) )
        return false;

    // NOTE: ignoring meFamily deliberately

    // compare with the requested width, allow default width
    int nAWidth = rA.mnWidth != 0 ? rA.mnWidth : rA.mnHeight;
    int nBWidth = rB.mnWidth != 0 ? rB.mnWidth : rB.mnHeight;
    if( nAWidth != nBWidth )
        return false;

#ifdef ENABLE_GRAPHITE
   if (rA.meLanguage != rB.meLanguage)
        return false;
   // check for features
   if ((rA.maTargetName.Search(grutils::GrFeatureParser::FEAT_PREFIX)
        != STRING_NOTFOUND ||
        rB.maTargetName.Search(grutils::GrFeatureParser::FEAT_PREFIX)
        != STRING_NOTFOUND) && rA.maTargetName != rB.maTargetName)
        return false;
#endif

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

// -----------------------------------------------------------------------

GlyphCache& GlyphCache::GetInstance()
{
    return *pInstance;
}

// -----------------------------------------------------------------------

void GlyphCache::AddFontFile( const rtl::OString& rNormalizedName, int nFaceNum,
    sal_IntPtr nFontId, const ImplDevFontAttributes& rDFA, const ExtraKernInfo* pExtraKern )
{
    if( mpFtManager )
        mpFtManager->AddFontFile( rNormalizedName, nFaceNum, nFontId, rDFA, pExtraKern );
}

// -----------------------------------------------------------------------

void GlyphCache::AnnounceFonts( ImplDevFontList* pList ) const
{
    if( mpFtManager )
        mpFtManager->AnnounceFonts( pList );
}

// -----------------------------------------------------------------------

ServerFont* GlyphCache::CacheFont( const FontSelectPattern& rFontSelData )
{
    // a serverfont request has pFontData
    if( rFontSelData.mpFontData == NULL )
        return NULL;
    // a serverfont request has a fontid > 0
    sal_IntPtr nFontId = rFontSelData.mpFontData->GetFontId();
    if( nFontId <= 0 )
        return NULL;

    // the FontList's key mpFontData member is reinterpreted as font id
    FontSelectPattern aFontSelData = rFontSelData;
    aFontSelData.mpFontData = reinterpret_cast<PhysicalFontFace*>( nFontId );
    FontList::iterator it = maFontList.find( aFontSelData );
    if( it != maFontList.end() )
    {
        ServerFont* pFound = it->second;
        if( pFound )
            pFound->AddRef();
        return pFound;
    }

    // font not cached yet => create new font item
    ServerFont* pNew = NULL;
    if( mpFtManager )
        pNew = mpFtManager->CreateFont( aFontSelData );

    if( pNew )
    {
        maFontList[ aFontSelData ] = pNew;
        mnBytesUsed += pNew->GetByteCount();

        // enable garbage collection for new font
        if( !mpCurrentGCFont )
        {
            mpCurrentGCFont = pNew;
            pNew->mpNextGCFont = pNew;
            pNew->mpPrevGCFont = pNew;
        }
        else
        {
            pNew->mpNextGCFont = mpCurrentGCFont;
            pNew->mpPrevGCFont = mpCurrentGCFont->mpPrevGCFont;
            pNew->mpPrevGCFont->mpNextGCFont = pNew;
            mpCurrentGCFont->mpPrevGCFont = pNew;
        }
    }

    return pNew;
}

// -----------------------------------------------------------------------

void GlyphCache::UncacheFont( ServerFont& rServerFont )
{
    // the interface for rServerFont must be const because a
    // user who wants to release it only got const ServerFonts.
    // The caching algorithm needs a non-const object
    ServerFont* pFont = const_cast<ServerFont*>( &rServerFont );
    if( (pFont->Release() <= 0)
    &&  (mnMaxSize <= (mnBytesUsed + mrPeer.GetByteCount())) )
    {
        mpCurrentGCFont = pFont;
        GarbageCollect();
    }
}

// -----------------------------------------------------------------------

void GlyphCache::GarbageCollect()
{
    // when current GC font has been destroyed get another one
    if( !mpCurrentGCFont )
    {
        FontList::iterator it = maFontList.begin();
        if( it != maFontList.end() )
            mpCurrentGCFont = it->second;
    }

    // unless there is no other font to collect
    if( !mpCurrentGCFont )
        return;

    // prepare advance to next font for garbage collection
    ServerFont* const pServerFont = mpCurrentGCFont;
    mpCurrentGCFont = pServerFont->mpNextGCFont;

    if( (pServerFont == mpCurrentGCFont)    // no other fonts
    ||  (pServerFont->GetRefCount() > 0) )  // font still used
    {
        // try to garbage collect at least a few bytes
        pServerFont->GarbageCollect( mnLruIndex - mnGlyphCount/2 );
    }
    else // current GC font is unreferenced
    {
        DBG_ASSERT( (pServerFont->GetRefCount() == 0),
            "GlyphCache::GC detected RefCount underflow" );

        // free all pServerFont related data
        pServerFont->GarbageCollect( mnLruIndex+0x10000000 );
        if( pServerFont == mpCurrentGCFont )
            mpCurrentGCFont = NULL;
        const FontSelectPattern& rIFSD = pServerFont->GetFontSelData();
        maFontList.erase( rIFSD );
        mrPeer.RemovingFont( *pServerFont );
        mnBytesUsed -= pServerFont->GetByteCount();

        // remove font from list of garbage collected fonts
        if( pServerFont->mpPrevGCFont )
            pServerFont->mpPrevGCFont->mpNextGCFont = pServerFont->mpNextGCFont;
        if( pServerFont->mpNextGCFont )
            pServerFont->mpNextGCFont->mpPrevGCFont = pServerFont->mpPrevGCFont;
        if( pServerFont == mpCurrentGCFont )
            mpCurrentGCFont = NULL;

        delete pServerFont;
    }
}

// -----------------------------------------------------------------------

inline void GlyphCache::UsingGlyph( ServerFont&, GlyphData& rGlyphData )
{
    rGlyphData.SetLruValue( mnLruIndex++ );
}

// -----------------------------------------------------------------------

inline void GlyphCache::AddedGlyph( ServerFont& rServerFont, GlyphData& rGlyphData )
{
    ++mnGlyphCount;
    mnBytesUsed += sizeof( rGlyphData );
    UsingGlyph( rServerFont, rGlyphData );
    GrowNotify();
}

// -----------------------------------------------------------------------

void GlyphCache::GrowNotify()
{
    if( (mnBytesUsed + mrPeer.GetByteCount()) > mnMaxSize )
        GarbageCollect();
}

// -----------------------------------------------------------------------

inline void GlyphCache::RemovingGlyph( ServerFont& rSF, GlyphData& rGD, int nGlyphIndex )
{
    mrPeer.RemovingGlyph( rSF, rGD, nGlyphIndex );
    mnBytesUsed -= sizeof( GlyphData );
    --mnGlyphCount;
}

// -----------------------------------------------------------------------

void ServerFont::ReleaseFromGarbageCollect()
{
   // remove from GC list
    ServerFont* pPrev = mpPrevGCFont;
    ServerFont* pNext = mpNextGCFont;
    if( pPrev ) pPrev->mpNextGCFont = pNext;
    if( pNext ) pNext->mpPrevGCFont = pPrev;
    mpPrevGCFont = NULL;
    mpNextGCFont = NULL;
}

// -----------------------------------------------------------------------

long ServerFont::Release() const
{
    DBG_ASSERT( mnRefCount > 0, "ServerFont: RefCount underflow" );
    return --mnRefCount;
}

// -----------------------------------------------------------------------

GlyphData& ServerFont::GetGlyphData( int nGlyphIndex )
{
    // usually the GlyphData is cached
    GlyphList::iterator it = maGlyphList.find( nGlyphIndex );
    if( it != maGlyphList.end() ) {
        GlyphData& rGlyphData = it->second;
        GlyphCache::GetInstance().UsingGlyph( *this, rGlyphData );
        return rGlyphData;
    }

    // sometimes not => we need to create and initialize it ourselves
    GlyphData& rGlyphData = maGlyphList[ nGlyphIndex ];
    mnBytesUsed += sizeof( GlyphData );
    InitGlyphData( nGlyphIndex, rGlyphData );
    GlyphCache::GetInstance().AddedGlyph( *this, rGlyphData );
    return rGlyphData;
}

// -----------------------------------------------------------------------

void ServerFont::GarbageCollect( long nMinLruIndex )
{
    GlyphList::iterator it_next = maGlyphList.begin();
    while( it_next != maGlyphList.end() )
    {
        GlyphList::iterator it = it_next++;
        GlyphData& rGD = it->second;
        if( (nMinLruIndex - rGD.GetLruValue()) > 0 )
        {
            OSL_ASSERT( mnBytesUsed >= sizeof(GlyphData) );
            mnBytesUsed -= sizeof( GlyphData );
            GlyphCache::GetInstance().RemovingGlyph( *this, rGD, it->first );
            maGlyphList.erase( it );
            it_next = maGlyphList.begin();
        }
    }
}

// =======================================================================

ImplServerFontEntry::ImplServerFontEntry( FontSelectPattern& rFSD )
:   ImplFontEntry( rFSD )
,   mpServerFont( NULL )
,   mbGotFontOptions( false )
{}

// -----------------------------------------------------------------------

ImplServerFontEntry::~ImplServerFontEntry()
{
    // TODO: remove the ServerFont here instead of in the GlyphCache
}

// =======================================================================

ExtraKernInfo::ExtraKernInfo( sal_IntPtr nFontId )
:   mbInitialized( false ),
    mnFontId( nFontId ),
    maUnicodeKernPairs( 0 )
{}

//--------------------------------------------------------------------------

int ExtraKernInfo::GetUnscaledKernPairs( ImplKernPairData** ppKernPairs ) const
{
    if( !mbInitialized )
        Initialize();

    // return early if no kerning available
    if( maUnicodeKernPairs.empty() )
        return 0;

    // allocate kern pair table
    int nKernCount = maUnicodeKernPairs.size();
    *ppKernPairs = new ImplKernPairData[ nKernCount ];

    // fill in unicode kern pairs with the kern value scaled to the font width
    ImplKernPairData* pKernData = *ppKernPairs;
    UnicodeKernPairs::const_iterator it = maUnicodeKernPairs.begin();
    for(; it != maUnicodeKernPairs.end(); ++it )
        *(pKernData++) = *it;

    return nKernCount;
}

//--------------------------------------------------------------------------

int ExtraKernInfo::GetUnscaledKernValue( sal_Unicode cLeft, sal_Unicode cRight ) const
{
    if( !mbInitialized )
        Initialize();

    if( maUnicodeKernPairs.empty() )
        return 0;

    ImplKernPairData aKernPair = { cLeft, cRight, 0 };
    UnicodeKernPairs::const_iterator it = maUnicodeKernPairs.find( aKernPair );
    if( it == maUnicodeKernPairs.end() )
        return 0;

    int nUnscaledValue = (*it).mnKern;
    return nUnscaledValue;
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
