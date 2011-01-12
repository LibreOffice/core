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

#include <rtl/ustring.hxx>
#include <osl/module.h>
#include <osl/thread.h>

#include <gcach_xpeer.hxx>
#include <xrender_peer.hxx>
#include <saldisp.hxx>
#include <saldata.hxx>
#include <salgdi.h>

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
:   mpDisplay( GetX11SalData()->GetDisplay()->GetDisplay() )
,   mnMaxScreens(0)
,   mnDefaultScreen(0)
,   mnExtByteCount(0)
,   mnForcedAA(0)
,   mnUsingXRender(0)
{
    maRawBitmap.mnAllocated = 0;
    maRawBitmap.mpBits = NULL;
    if( !mpDisplay )
        return;

    SalDisplay& rSalDisplay = *GetX11SalData()->GetDisplay();
    mpDisplay    = rSalDisplay.GetDisplay();
    mnMaxScreens = rSalDisplay.GetScreenCount();
    if( mnMaxScreens > MAX_GCACH_SCREENS )
        mnMaxScreens = MAX_GCACH_SCREENS;
    // if specific glyph data has to be kept for many screens
    // then prepare the allocation of MultiScreenGlyph objects
    if( mnMaxScreens > 1 )
        mnExtByteCount = sizeof(MultiScreenGlyph) + sizeof(Pixmap) * (mnMaxScreens - 1);
    mnDefaultScreen = rSalDisplay.GetDefaultScreenNumber();

    InitAntialiasing();
}

// ---------------------------------------------------------------------------

X11GlyphPeer::~X11GlyphPeer()
{
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
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

// ---------------------------------------------------------------------------

void X11GlyphPeer::InitAntialiasing()
{
    int nEnvAntiAlias = 0;
    const char* pEnvAntiAlias = getenv( "SAL_ANTIALIAS_DISABLE" );
    if( pEnvAntiAlias )
    {
        nEnvAntiAlias = atoi( pEnvAntiAlias );
        if( nEnvAntiAlias == 0 )
            return;
    }

    mnUsingXRender = 0;
    mnForcedAA = 0;

    // enable XRENDER accelerated aliasing on screens that support it
    // unless it explicitly disabled by an environment variable
    if( (nEnvAntiAlias & 2) == 0 )
        mnUsingXRender = XRenderPeer::GetInstance().InitRenderText();

    // else enable client side antialiasing for these screens
    // unless it is explicitly disabled by an environment variable
    if( (nEnvAntiAlias & 1) != 0 )
        return;

    // enable client side antialiasing for screen visuals that are suitable
    // mnForcedAA is a bitmask of screens enabled for client side antialiasing
    mnForcedAA = (~(~0U << mnMaxScreens)) ^ mnUsingXRender;
    SalDisplay& rSalDisplay = *GetX11SalData()->GetDisplay();
    for( int nScreen = 0; nScreen < mnMaxScreens; ++nScreen)
    {
        Visual* pVisual = rSalDisplay.GetVisual( nScreen ).GetVisual();
        XVisualInfo aXVisualInfo;
        aXVisualInfo.visualid = pVisual->visualid;
        int nVisuals = 0;
        XVisualInfo* pXVisualInfo = XGetVisualInfo( mpDisplay, VisualIDMask, &aXVisualInfo, &nVisuals );
        for( int i = nVisuals; --i >= 0; )
        {
            if( ((pXVisualInfo[i].c_class==PseudoColor) || (pXVisualInfo[i].depth<24))
            && ((pXVisualInfo[i].c_class>GrayScale) || (pXVisualInfo[i].depth!=8) ) )
                mnForcedAA &= ~(1U << nScreen);
        }
        if( pXVisualInfo != NULL )
            XFree( pXVisualInfo );
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

Glyph X11GlyphPeer::GetRenderGlyph( const GlyphData& rGD ) const
{
    Glyph aGlyphId = NO_GLYPHID;
    const ExtGlyphData& rEGD = rGD.ExtDataRef();
    if( rEGD.meInfo == INFO_XRENDER )
        aGlyphId = reinterpret_cast<Glyph>(rEGD.mpData);
    else if( rEGD.meInfo == INFO_MULTISCREEN )
        aGlyphId = reinterpret_cast<MultiScreenGlyph*>(rEGD.mpData)->maXRGlyphId;
    return aGlyphId;
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::SetRenderGlyph( GlyphData& rGD, Glyph aGlyphId ) const
{
    ExtGlyphData& rEGD = rGD.ExtDataRef();
    switch( rEGD.meInfo )
    {
        case INFO_EMPTY:
            rEGD.meInfo = INFO_XRENDER;
            // fall through
        case INFO_XRENDER:
            rEGD.mpData = reinterpret_cast<void*>(aGlyphId);
            break;
        case INFO_PIXMAP:
        case INFO_RAWBMP:
            PrepareForMultiscreen( rEGD );
            // fall through
        case INFO_MULTISCREEN:
            reinterpret_cast<MultiScreenGlyph*>(rEGD.mpData)->maXRGlyphId = aGlyphId;
            break;
        default:
            break;  // cannot happen...
    }
}

// ---------------------------------------------------------------------------

const RawBitmap* X11GlyphPeer::GetRawBitmap( const GlyphData& rGD ) const
{
    const RawBitmap* pRawBitmap = NO_RAWBMP;
    const ExtGlyphData& rEGD = rGD.ExtDataRef();
    if( rEGD.meInfo == INFO_RAWBMP )
        pRawBitmap = reinterpret_cast<RawBitmap*>(rEGD.mpData);
    else if( rEGD.meInfo == INFO_MULTISCREEN )
        pRawBitmap = reinterpret_cast<MultiScreenGlyph*>(rEGD.mpData)->mpRawBitmap;
    return pRawBitmap;
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::SetRawBitmap( GlyphData& rGD, const RawBitmap* pRawBitmap ) const
{
    ExtGlyphData& rEGD = rGD.ExtDataRef();
    switch( rEGD.meInfo )
    {
        case INFO_EMPTY:
            rEGD.meInfo = INFO_RAWBMP;
            // fall through
        case INFO_RAWBMP:
            rEGD.mpData = (void*)pRawBitmap;
            break;
        case INFO_PIXMAP:
        case INFO_XRENDER:
            PrepareForMultiscreen( rEGD );
            // fall through
        case INFO_MULTISCREEN:
            reinterpret_cast<MultiScreenGlyph*>(rEGD.mpData)->mpRawBitmap = pRawBitmap;
            break;
        default:
            // cannot happen...
            break;
    }
}

// ---------------------------------------------------------------------------

Pixmap X11GlyphPeer::GetPixmap( const GlyphData& rGD, int nScreen ) const
{
    Pixmap aPixmap = NO_PIXMAP;
    const ExtGlyphData& rEGD = rGD.ExtDataRef();
    if( (rEGD.meInfo == INFO_PIXMAP) && (nScreen == mnDefaultScreen) )
        aPixmap = (Pixmap)rEGD.mpData;
    else if( rEGD.meInfo == INFO_MULTISCREEN )
        aPixmap = (Pixmap)(reinterpret_cast<MultiScreenGlyph*>(rEGD.mpData)->maPixmaps[nScreen]);
    return aPixmap;
}

// ---------------------------------------------------------------------------

void X11GlyphPeer::SetPixmap( GlyphData& rGD, Pixmap aPixmap, int nScreen ) const
{
    if( aPixmap == NO_PIXMAP )
        aPixmap = None;

    ExtGlyphData& rEGD = rGD.ExtDataRef();
    if( (rEGD.meInfo == INFO_EMPTY) && (nScreen == mnDefaultScreen) )
    {
        rEGD.meInfo = INFO_PIXMAP;
        rEGD.mpData = (void*)aPixmap;
    }
    else
    {
        MultiScreenGlyph* pMSGlyph;
        if( rEGD.meInfo == INFO_MULTISCREEN )
            pMSGlyph = reinterpret_cast<MultiScreenGlyph*>(rEGD.mpData);
        else
            pMSGlyph = PrepareForMultiscreen( rEGD );

        pMSGlyph->maPixmaps[ nScreen ] = aPixmap;
    }
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

// ---------------------------------------------------------------------------

bool X11GlyphPeer::ForcedAntialiasing( const ServerFont& rServerFont, int nScreen ) const
{
    bool bForceOk = rServerFont.GetAntialiasAdvice();
    // maximum size for antialiasing is 250 pixels
    bForceOk &= (rServerFont.GetFontSelData().mnHeight < 250);
    return (bForceOk && ((mnForcedAA >> nScreen) & 1));
}

// ---------------------------------------------------------------------------

GlyphSet X11GlyphPeer::GetGlyphSet( ServerFont& rServerFont, int nScreen )
{
    if( (nScreen >= 0) && ((mnUsingXRender >> nScreen) & 1) == 0 )
        return 0;

    GlyphSet aGlyphSet;

    switch( rServerFont.GetExtInfo() )
    {
        case INFO_XRENDER:
            aGlyphSet = (GlyphSet)rServerFont.GetExtPointer();
            break;

        case INFO_EMPTY:
            {
                // antialiasing for reasonable font heights only
                // => prevents crashes caused by X11 requests >= 256k
                // => prefer readablity of hinted glyphs at small sizes
                // => prefer "grey clouds" to "black clouds" at very small sizes
                int nHeight = rServerFont.GetFontSelData().mnHeight;
                if( nHeight<250 && rServerFont.GetAntialiasAdvice() )
                {
                    aGlyphSet = XRenderPeer::GetInstance().CreateGlyphSet();
                    rServerFont.SetExtended( INFO_XRENDER, (void*)aGlyphSet );
                }
                else
                    aGlyphSet = 0;
            }
            break;

        default:
            aGlyphSet = 0;
            break;
    }

    return aGlyphSet;
}

// ---------------------------------------------------------------------------

Pixmap X11GlyphPeer::GetPixmap( ServerFont& rServerFont, int nGlyphIndex, int nReqScreen )
{
    if( rServerFont.IsGlyphInvisible( nGlyphIndex ) )
        return None;

    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );
    Pixmap aPixmap = GetPixmap( rGlyphData, nReqScreen );
    if( aPixmap == NO_PIXMAP )
    {
        aPixmap = None;
        if( rServerFont.GetGlyphBitmap1( nGlyphIndex, maRawBitmap ) )
        {
            // #94666# circumvent bug in some X11 systems, e.g. XF410.LynxEM.v163
            sal_uLong nPixmapWidth = 8 * maRawBitmap.mnScanlineSize - 1;
            nPixmapWidth = Max( nPixmapWidth, maRawBitmap.mnWidth );

            rGlyphData.SetSize( Size( nPixmapWidth, maRawBitmap.mnHeight ) );
            rGlyphData.SetOffset( +maRawBitmap.mnXOffset, +maRawBitmap.mnYOffset );

            const sal_uLong nBytes = maRawBitmap.mnHeight * maRawBitmap.mnScanlineSize;
            if( nBytes > 0 )
            {
                // conversion table LSB<->MSB (for XCreatePixmapFromData)
                static const unsigned char lsb2msb[256] =
                {
                    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
                    0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
                    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
                    0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
                    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
                    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
                    0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
                    0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
                    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
                    0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
                    0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
                    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
                    0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
                    0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
                    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
                    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
                    0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
                };

                unsigned char* pTemp = maRawBitmap.mpBits;
                for( int i = nBytes; --i >= 0; ++pTemp )
                    *pTemp = lsb2msb[ *pTemp ];

                // often a glyph pixmap is only needed on the default screen
                // => optimize for this common case
                int nMinScreen = 0;
                int nEndScreen = mnMaxScreens;
                if( nReqScreen == mnDefaultScreen ) {
                    nMinScreen = mnDefaultScreen;
                    nEndScreen = mnDefaultScreen + 1;
                }
                // prepare glyph pixmaps for the different screens
                for( int i = nMinScreen; i < nEndScreen; ++i )
                {
                    // don't bother if the pixmap is already there
                    if( GetPixmap( rGlyphData, i ) != NO_PIXMAP )
                        continue;
                    // create the glyph pixmap
                    Pixmap aScreenPixmap = XCreatePixmapFromBitmapData( mpDisplay,
                        RootWindow( mpDisplay, i ), (char*)maRawBitmap.mpBits,
                        nPixmapWidth, maRawBitmap.mnHeight, 1, 0, 1 );
                    // and cache it as glyph specific data
                    SetPixmap( rGlyphData, aScreenPixmap, i );
                    mnBytesUsed += nBytes;
                    if( i == nReqScreen )
                        aPixmap = aScreenPixmap;
                }
            }
        }
        else
        {
            // fall back to .notdef glyph
            if( nGlyphIndex != 0 )  // recurse only once
                aPixmap = GetPixmap( rServerFont, 0, nReqScreen );

            if( aPixmap == NO_PIXMAP )
                aPixmap = None;
        }
    }

    return aPixmap;
}

// ---------------------------------------------------------------------------

const RawBitmap* X11GlyphPeer::GetRawBitmap( ServerFont& rServerFont,
    int nGlyphIndex )
{
    if( rServerFont.IsGlyphInvisible( nGlyphIndex ) )
        return NO_RAWBMP;

    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    const RawBitmap* pRawBitmap = GetRawBitmap( rGlyphData );
    if( pRawBitmap == NO_RAWBMP )
    {
        RawBitmap* pNewBitmap = new RawBitmap;
        if( rServerFont.GetGlyphBitmap8( nGlyphIndex, *pNewBitmap ) )
        {
            pRawBitmap = pNewBitmap;
            mnBytesUsed += pNewBitmap->mnScanlineSize * pNewBitmap->mnHeight;
            mnBytesUsed += sizeof(pNewBitmap);
        }
        else
        {
            delete pNewBitmap;
            // fall back to .notdef glyph
            if( nGlyphIndex != 0 )  // recurse only once
                pRawBitmap = GetRawBitmap( rServerFont, 0 );
        }

        SetRawBitmap( rGlyphData, pRawBitmap );
    }

    return pRawBitmap;
}

// ---------------------------------------------------------------------------

Glyph X11GlyphPeer::GetGlyphId( ServerFont& rServerFont, int nGlyphIndex )
{
    if( rServerFont.IsGlyphInvisible( nGlyphIndex ) )
        return NO_GLYPHID;

    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );

    Glyph aGlyphId = GetRenderGlyph( rGlyphData );
    if( aGlyphId == NO_GLYPHID )
    {
        // prepare GlyphInfo and Bitmap
        if( rServerFont.GetGlyphBitmap8( nGlyphIndex, maRawBitmap ) )
        {
            XGlyphInfo aGlyphInfo;
            aGlyphInfo.width    = maRawBitmap.mnWidth;
            aGlyphInfo.height   = maRawBitmap.mnHeight;
            aGlyphInfo.x        = -maRawBitmap.mnXOffset;
            aGlyphInfo.y        = -maRawBitmap.mnYOffset;

            rGlyphData.SetSize( Size( maRawBitmap.mnWidth, maRawBitmap.mnHeight ) );
            rGlyphData.SetOffset( +maRawBitmap.mnXOffset, +maRawBitmap.mnYOffset );

            const GlyphMetric& rGM = rGlyphData.GetMetric();
            aGlyphInfo.xOff     = +rGM.GetDelta().X();
            aGlyphInfo.yOff     = +rGM.GetDelta().Y();

            // upload glyph bitmap to server
            GlyphSet aGlyphSet = GetGlyphSet( rServerFont, -1 );

            aGlyphId = nGlyphIndex & 0x00FFFFFF;
            const sal_uLong nBytes = maRawBitmap.mnScanlineSize * maRawBitmap.mnHeight;
            XRenderPeer::GetInstance().AddGlyph( aGlyphSet, aGlyphId,
                aGlyphInfo, (char*)maRawBitmap.mpBits, nBytes );
            mnBytesUsed += nBytes;
        }
        else
        {
            // fall back to .notdef glyph
            if( nGlyphIndex != 0 )  // recurse only once
                aGlyphId = GetGlyphId( rServerFont, 0 );
        }

        SetRenderGlyph( rGlyphData, aGlyphId );
    }

    return aGlyphId;
}

// ===========================================================================

X11GlyphCache::X11GlyphCache( X11GlyphPeer& rPeer )
:   GlyphCache( rPeer )
{
}

// ---------------------------------------------------------------------------

static X11GlyphPeer* pX11GlyphPeer = NULL;
static X11GlyphCache* pX11GlyphCache = NULL;

X11GlyphCache& X11GlyphCache::GetInstance()
{
    if( !pX11GlyphCache )
    {
        pX11GlyphPeer = new X11GlyphPeer();
        pX11GlyphCache = new X11GlyphCache( *pX11GlyphPeer );
    }
    return *pX11GlyphCache;
}

// ---------------------------------------------------------------------------

void X11GlyphCache::KillInstance()
{
    delete pX11GlyphCache;
    delete pX11GlyphPeer;
    pX11GlyphCache = NULL;
    pX11GlyphPeer = NULL;
}

// ===========================================================================

void X11SalGraphics::releaseGlyphPeer()
{
    X11GlyphCache::KillInstance();
}

// ===========================================================================

