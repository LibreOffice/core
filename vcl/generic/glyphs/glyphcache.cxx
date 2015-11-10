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

#include <stdlib.h>
#include <math.h>
#include <gcach_ftyp.hxx>

#include <vcl/svapp.hxx>
#include <vcl/bitmap.hxx>
#include <outfont.hxx>

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <graphite_features.hxx>
#endif

#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <tools/debug.hxx>

static GlyphCache* pInstance = nullptr;

GlyphCache::GlyphCache( GlyphCachePeer& rPeer )
:   mrPeer( rPeer ),
    mnMaxSize( 1500000 ),
    mnBytesUsed(sizeof(GlyphCache)),
    mnLruIndex(0),
    mnGlyphCount(0),
    mpCurrentGCFont(nullptr),
    mpFtManager(nullptr)
{
    pInstance = this;
    mpFtManager = new FreetypeManager;
}

GlyphCache::~GlyphCache()
{
    InvalidateAllGlyphs();
    delete mpFtManager;
}

void GlyphCache::InvalidateAllGlyphs()
{
    for( FontList::iterator it = maFontList.begin(), end = maFontList.end(); it != end; ++it )
    {
        ServerFont* pServerFont = it->second;
        // free all pServerFont related data
        pServerFont->GarbageCollect( mnLruIndex+0x10000000 );
        mrPeer.RemovingFont(*pServerFont);
        delete pServerFont;
    }

    maFontList.clear();
    mpCurrentGCFont = nullptr;
}

inline
size_t GlyphCache::IFSD_Hash::operator()( const FontSelectPattern& rFontSelData ) const
{
    // TODO: is it worth to improve this hash function?
    sal_IntPtr nFontId = reinterpret_cast<sal_IntPtr>( rFontSelData.mpFontData );
#if ENABLE_GRAPHITE
    if (rFontSelData.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
        != -1)
    {
        OString aFeatName = OUStringToOString( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
        nFontId ^= aFeatName.hashCode();
    }
#endif
    size_t nHash = nFontId << 8;
    nHash   += rFontSelData.mnHeight;
    nHash   += rFontSelData.mnOrientation;
    nHash   += size_t(rFontSelData.mbVertical);
    nHash   += rFontSelData.GetSlant();
    nHash   += rFontSelData.GetWeight();
#if ENABLE_GRAPHITE
    nHash   += rFontSelData.meLanguage;
#endif
    return nHash;
}

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

    if( (rA.GetSlant() != rB.GetSlant())
    ||  (rA.GetWeight() != rB.GetWeight()) )
        return false;

    // NOTE: ignoring meFamily deliberately

    // compare with the requested width, allow default width
    int nAWidth = rA.mnWidth != 0 ? rA.mnWidth : rA.mnHeight;
    int nBWidth = rB.mnWidth != 0 ? rB.mnWidth : rB.mnHeight;
    if( nAWidth != nBWidth )
        return false;

#if ENABLE_GRAPHITE
   if (rA.meLanguage != rB.meLanguage)
        return false;
   // check for features
   if ((rA.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
        != -1 ||
        rB.maTargetName.indexOf(grutils::GrFeatureParser::FEAT_PREFIX)
        != -1) && rA.maTargetName != rB.maTargetName)
        return false;
#endif

    if (rA.mbEmbolden != rB.mbEmbolden)
        return false;

    if (rA.maItalicMatrix != rB.maItalicMatrix)
        return false;

    return true;
}

GlyphCache& GlyphCache::GetInstance()
{
    return *pInstance;
}

void GlyphCache::AddFontFile( const OString& rNormalizedName, int nFaceNum,
    sal_IntPtr nFontId, const ImplDevFontAttributes& rDFA)
{
    if( mpFtManager )
        mpFtManager->AddFontFile( rNormalizedName, nFaceNum, nFontId, rDFA);
}

void GlyphCache::AnnounceFonts( PhysicalFontCollection* pFontCollection ) const
{
    if( mpFtManager )
        mpFtManager->AnnounceFonts( pFontCollection );
}

void GlyphCache::ClearFontCache()
{
    InvalidateAllGlyphs();
    if (mpFtManager)
        mpFtManager->ClearFontList();
}

ServerFont* GlyphCache::CacheFont( const FontSelectPattern& rFontSelData )
{
    // a serverfont request has pFontData
    if( rFontSelData.mpFontData == nullptr )
        return nullptr;
    // a serverfont request has a fontid > 0
    sal_IntPtr nFontId = rFontSelData.mpFontData->GetFontId();
    if( nFontId <= 0 )
        return nullptr;

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
    ServerFont* pNew = nullptr;
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

void GlyphCache::UncacheFont( ServerFont& rServerFont )
{
    if( (rServerFont.Release() <= 0) && (mnMaxSize <= mnBytesUsed) )
    {
        mpCurrentGCFont = &rServerFont;
        GarbageCollect();
    }
}

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
            mpCurrentGCFont = nullptr;
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
            mpCurrentGCFont = nullptr;

        delete pServerFont;
    }
}

inline void GlyphCache::UsingGlyph( ServerFont&, GlyphData& rGlyphData )
{
    rGlyphData.SetLruValue( mnLruIndex++ );
}

inline void GlyphCache::AddedGlyph( ServerFont& rServerFont, GlyphData& rGlyphData )
{
    ++mnGlyphCount;
    mnBytesUsed += sizeof( rGlyphData );
    UsingGlyph( rServerFont, rGlyphData );
    GrowNotify();
}

void GlyphCache::GrowNotify()
{
    if( mnBytesUsed > mnMaxSize )
        GarbageCollect();
}

inline void GlyphCache::RemovingGlyph( GlyphData& rGD )
{
    mrPeer.RemovingGlyph( rGD );
    mnBytesUsed -= sizeof( GlyphData );
    --mnGlyphCount;
}

void ServerFont::ReleaseFromGarbageCollect()
{
    // remove from GC list
    ServerFont* pPrev = mpPrevGCFont;
    ServerFont* pNext = mpNextGCFont;
    if( pPrev ) pPrev->mpNextGCFont = pNext;
    if( pNext ) pNext->mpPrevGCFont = pPrev;
    mpPrevGCFont = nullptr;
    mpNextGCFont = nullptr;
}

long ServerFont::Release() const
{
    DBG_ASSERT( mnRefCount > 0, "ServerFont: RefCount underflow" );
    return --mnRefCount;
}

GlyphData& ServerFont::GetGlyphData( sal_GlyphId aGlyphId )
{
    // usually the GlyphData is cached
    GlyphList::iterator it = maGlyphList.find( aGlyphId );
    if( it != maGlyphList.end() ) {
        GlyphData& rGlyphData = it->second;
        GlyphCache::GetInstance().UsingGlyph( *this, rGlyphData );
        return rGlyphData;
    }

    // sometimes not => we need to create and initialize it ourselves
    GlyphData& rGlyphData = maGlyphList[ aGlyphId ];
    mnBytesUsed += sizeof( GlyphData );
    InitGlyphData( aGlyphId, rGlyphData );
    GlyphCache::GetInstance().AddedGlyph( *this, rGlyphData );
    return rGlyphData;
}

void ServerFont::GarbageCollect( long nMinLruIndex )
{
    GlyphList::iterator it = maGlyphList.begin();
    while( it != maGlyphList.end() )
    {
        GlyphData& rGD = it->second;
        if( (nMinLruIndex - rGD.GetLruValue()) > 0 )
        {
            OSL_ASSERT( mnBytesUsed >= sizeof(GlyphData) );
            mnBytesUsed -= sizeof( GlyphData );
            GlyphCache::GetInstance().RemovingGlyph( rGD );
            it = maGlyphList.erase( it );
        }
        else
            ++it;
    }
}

ImplServerFontEntry::ImplServerFontEntry( FontSelectPattern& rFSD )
:   ImplFontEntry( rFSD )
,   mpServerFont( nullptr )
,   mbGotFontOptions( false )
{}

void ImplServerFontEntry::SetServerFont(ServerFont* p)
{
    if (p == mpServerFont)
        return;
    if (mpServerFont)
        mpServerFont->Release();
    mpServerFont = p;
    if (mpServerFont)
        mpServerFont->AddRef();
}

ImplServerFontEntry::~ImplServerFontEntry()
{
    // TODO: remove the ServerFont here instead of in the GlyphCache
    if (mpServerFont)
        mpServerFont->Release();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
