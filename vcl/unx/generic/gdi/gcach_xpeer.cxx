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

#include "rtl/ustring.hxx"
#include "osl/module.h"
#include "osl/thread.h"

#include "unx/saldisp.hxx"
#include "unx/saldata.hxx"
#include "unx/salgdi.h"

#include "gcach_xpeer.hxx"
#include "xrender_peer.hxx"

// ===========================================================================

// all glyph specific data needed by the XGlyphPeer is quite trivial
// with one exception: if multiple screens are involved and non-antialiased
// glyph rendering is active, then we need screen specific pixmaps
struct MultiScreenGlyph
{
    const RawBitmap*    mpRawBitmap;
    Glyph               maXRGlyphId;
    Pixmap              maPixmaps[1];   // [mnMaxScreens]
};

// ===========================================================================

X11GlyphPeer::X11GlyphPeer()
:   mpDisplay( GetGenericData()->GetSalDisplay()->GetDisplay() )
,   mnMaxScreens(0)
,   mnDefaultScreen(0)
,   mnExtByteCount(0)
{
    if( !mpDisplay )
        return;

    SalDisplay& rSalDisplay = *GetGenericData()->GetSalDisplay();
    mpDisplay    = rSalDisplay.GetDisplay();
    mnMaxScreens = rSalDisplay.GetScreenCount();
    if( mnMaxScreens > MAX_GCACH_SCREENS )
        mnMaxScreens = MAX_GCACH_SCREENS;
    // if specific glyph data has to be kept for many screens
    // then prepare the allocation of MultiScreenGlyph objects
    if( mnMaxScreens > 1 )
        mnExtByteCount = sizeof(MultiScreenGlyph) + sizeof(Pixmap) * (mnMaxScreens - 1);
    mnDefaultScreen = rSalDisplay.GetDefaultScreenNumber();
}

// ---------------------------------------------------------------------------

X11GlyphPeer::~X11GlyphPeer()
{
    if( !ImplGetSVData() )
        return;

    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    Display* const pX11Disp = pSalDisp->GetDisplay();
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();
    for( int i = 0; i < mnMaxScreens; i++ )
    {
        SalDisplay::RenderEntryMap& rMap = pSalDisp->GetRenderEntries( i );
        for( SalDisplay::RenderEntryMap::iterator it = rMap.begin(); it != rMap.end(); ++it )
        {
            if( it->second.m_aPixmap )
                ::XFreePixmap( pX11Disp, it->second.m_aPixmap );
            if( it->second.m_aPicture )
                rRenderPeer.FreePicture( it->second.m_aPicture );
        }
        rMap.clear();
    }
}

// ===========================================================================

enum { INFO_EMPTY=0, INFO_PIXMAP, INFO_XRENDER, INFO_RAWBMP, INFO_MULTISCREEN };
static const Glyph NO_GLYPHID = 0;
static RawBitmap* const NO_RAWBMP = NULL;
static const Pixmap NO_PIXMAP = ~0;

// ---------------------------------------------------------------------------

MultiScreenGlyph* X11GlyphPeer::PrepareForMultiscreen( ExtGlyphData& rEGD ) const
{
    // prepare to store screen specific pixmaps
    MultiScreenGlyph* pMSGlyph = (MultiScreenGlyph*)new char[ mnExtByteCount ];

    // init the glyph formats
    pMSGlyph->mpRawBitmap = NO_RAWBMP;
    pMSGlyph->maXRGlyphId = NO_GLYPHID;
    for( int i = 0; i < mnMaxScreens; ++i )
        pMSGlyph->maPixmaps[i] = NO_PIXMAP;
    // reuse already available glyph formats
    if( rEGD.meInfo == INFO_XRENDER )
        pMSGlyph->maXRGlyphId = reinterpret_cast<Glyph>(rEGD.mpData);
    else if( rEGD.meInfo == INFO_RAWBMP )
        pMSGlyph->mpRawBitmap = reinterpret_cast<RawBitmap*>(rEGD.mpData);
    else if( rEGD.meInfo == INFO_PIXMAP )
    {
        Pixmap aPixmap = reinterpret_cast<Pixmap>(rEGD.mpData);
        if( aPixmap != None )
            // pixmap for the default screen is available
            pMSGlyph->maPixmaps[ mnDefaultScreen ] = aPixmap;
        else // empty pixmap for all screens is available
            for( int i = 0; i < mnMaxScreens; ++i )
                pMSGlyph->maPixmaps[ i ] = None;
    }
    // enable use of multiscreen glyph
    rEGD.mpData = (void*)pMSGlyph;
    rEGD.meInfo = INFO_MULTISCREEN;

    return pMSGlyph;
 }

// ---------------------------------------------------------------------------

void X11GlyphPeer::RemovingFont( ServerFont& rServerFont )
{
    void* pFontExt = rServerFont.GetExtPointer();
    switch( rServerFont.GetExtInfo() )
    {
        case INFO_PIXMAP:
        case INFO_RAWBMP:
            // nothing to do
            break;
        case INFO_MULTISCREEN:
            // cannot happen...
            break;

        case INFO_XRENDER:
            XRenderPeer::GetInstance().FreeGlyphSet( (GlyphSet)pFontExt );
            break;
    }

    rServerFont.SetExtended( INFO_EMPTY, NULL );
}

// ---------------------------------------------------------------------------

// notification to clean up GlyphPeer resources for this glyph
void X11GlyphPeer::RemovingGlyph( ServerFont& /*rServerFont*/, GlyphData& rGlyphData, int /*nGlyphIndex*/ )
{
    // nothing to do if the GlyphPeer hasn't allocated resources for the glyph
    if( rGlyphData.ExtDataRef().meInfo == INFO_EMPTY )
        return;

    const GlyphMetric& rGM = rGlyphData.GetMetric();
    const int nWidth  = rGM.GetSize().Width();
    const int nHeight = rGM.GetSize().Height();

    void* pGlyphExt = rGlyphData.ExtDataRef().mpData;
    switch( rGlyphData.ExtDataRef().meInfo )
    {
        case INFO_PIXMAP:
            {
                Pixmap aPixmap = (Pixmap)pGlyphExt;
                if( aPixmap != None )
                {
                    XFreePixmap( mpDisplay, aPixmap );
                    mnBytesUsed -= nHeight * ((nWidth + 7) >> 3);
                }
            }
            break;

        case INFO_MULTISCREEN:
            {
                MultiScreenGlyph* pMSGlyph = reinterpret_cast<MultiScreenGlyph*>(pGlyphExt);
                for( int i = 0; i < mnMaxScreens; ++i)
                {
                    if( pMSGlyph->maPixmaps[i] == NO_PIXMAP )
                        continue;
                    if( pMSGlyph->maPixmaps[i] == None )
                        continue;
                    XFreePixmap( mpDisplay, pMSGlyph->maPixmaps[i] );
                    mnBytesUsed -= nHeight * ((nWidth + 7) >> 3);
                }
                delete pMSGlyph->mpRawBitmap;
                // Glyph nGlyphId = (Glyph)rGlyphData.GetExtPointer();
                // XRenderPeer::GetInstance().FreeGlyph( aGlyphSet, &nGlyphId );
                delete[] pMSGlyph; // it was allocated with new char[]
            }
            break;

        case INFO_RAWBMP:
            {
                RawBitmap* pRawBitmap = (RawBitmap*)pGlyphExt;
                if( pRawBitmap != NULL )
                {
                    mnBytesUsed -= pRawBitmap->mnScanlineSize * pRawBitmap->mnHeight;
                    mnBytesUsed -= sizeof(RawBitmap);
                    delete pRawBitmap;
                }
            }
            break;

        case INFO_XRENDER:
            {
                // Glyph nGlyphId = (Glyph)rGlyphData.GetExtPointer();
                // XRenderPeer::GetInstance().FreeGlyph( aGlyphSet, &nGlyphId );
                mnBytesUsed -= nHeight * ((nWidth + 3) & ~3);
            }
            break;
    }

    if( mnBytesUsed < 0 )   // TODO: eliminate nBytesUsed calc mismatch
        mnBytesUsed = 0;

    rGlyphData.ExtDataRef() = ExtGlyphData();
}

// ===========================================================================

X11GlyphCache::X11GlyphCache( X11GlyphPeer& rPeer )
:   GlyphCache( rPeer )
{
}

// ---------------------------------------------------------------------------

namespace
{
    struct GlyphCacheHolder
    {
    private:
        X11GlyphPeer* m_pX11GlyphPeer;
        X11GlyphCache* m_pX11GlyphCache;
    public:
        GlyphCacheHolder()
        {
            m_pX11GlyphPeer = new X11GlyphPeer();
            m_pX11GlyphCache = new X11GlyphCache( *m_pX11GlyphPeer );
        }
        void release()
        {
            delete m_pX11GlyphCache;
            delete m_pX11GlyphPeer;
            m_pX11GlyphCache = NULL;
            m_pX11GlyphPeer = NULL;
        }
        X11GlyphCache& getGlyphCache()
        {
            return *m_pX11GlyphCache;
        }
        ~GlyphCacheHolder()
        {
            release();
        }
    };

    struct theGlyphCacheHolder :
        public rtl::Static<GlyphCacheHolder, theGlyphCacheHolder>
    {};
}

X11GlyphCache& X11GlyphCache::GetInstance()
{
    return theGlyphCacheHolder::get().getGlyphCache();
}

// ---------------------------------------------------------------------------

void X11GlyphCache::KillInstance()
{
    return theGlyphCacheHolder::get().release();
}

// ===========================================================================

void X11SalGraphics::releaseGlyphPeer()
{
    X11GlyphCache::KillInstance();
}

// ===========================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
