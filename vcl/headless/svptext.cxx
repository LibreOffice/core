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

#include <sal/types.h>

#include <cassert>

#include <basebmp/scanlineformats.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <rtl/instance.hxx>
#include <tools/debug.hxx>
#include <vcl/sysdata.hxx>

#include "generic/geninst.h"
#include "generic/genpspgraphics.h"
#include "generic/glyphcache.hxx"
#include "headless/svpbmp.hxx"
#include "headless/svpgdi.hxx"
#include "impfont.hxx"
#include "outfont.hxx"
#include "PhysicalFontFace.hxx"

using namespace basegfx;
using namespace basebmp;

class SvpGlyphPeer : public GlyphCachePeer
{
public:
    SvpGlyphPeer() {}

    BitmapDeviceSharedPtr GetGlyphBmp( ServerFont&, sal_GlyphId,
                                       basebmp::Format nBmpFormat, B2IPoint& rTargetPos );

protected:
    virtual void    RemovingFont( ServerFont& );
    virtual void    RemovingGlyph( GlyphData& );

    class SvpGcpHelper
    {
    public:
        RawBitmap               maRawBitmap;
        BitmapDeviceSharedPtr   maBitmapDev;
    };
};

class SvpGlyphCache : public GlyphCache
{
public:
    SvpGlyphCache( SvpGlyphPeer& rPeer ) : GlyphCache( rPeer) {}
    SvpGlyphPeer& GetPeer() { return reinterpret_cast<SvpGlyphPeer&>( mrPeer ); }
    static SvpGlyphCache& GetInstance();
};

namespace
{
    struct GlyphCacheHolder
    {
    private:
        SvpGlyphPeer* m_pSvpGlyphPeer;
        SvpGlyphCache* m_pSvpGlyphCache;
    public:
        GlyphCacheHolder()
        {
            m_pSvpGlyphPeer = new SvpGlyphPeer();
            m_pSvpGlyphCache = new SvpGlyphCache( *m_pSvpGlyphPeer );
        }
        void release()
        {
            delete m_pSvpGlyphCache;
            delete m_pSvpGlyphPeer;
            m_pSvpGlyphCache = NULL;
            m_pSvpGlyphPeer = NULL;
        }
        SvpGlyphCache& getGlyphCache()
        {
            return *m_pSvpGlyphCache;
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

SvpGlyphCache& SvpGlyphCache::GetInstance()
{
    return theGlyphCacheHolder::get().getGlyphCache();
}


BitmapDeviceSharedPtr SvpGlyphPeer::GetGlyphBmp( ServerFont& rServerFont,
    sal_GlyphId aGlyphId, basebmp::Format nBmpFormat, B2IPoint& rTargetPos )
{
    GlyphData& rGlyphData = rServerFont.GetGlyphData( aGlyphId );

    if( rGlyphData.ExtDataRef().meInfo != nBmpFormat )
    {
        SvpGcpHelper* pGcpHelper = static_cast<SvpGcpHelper*>(
            rGlyphData.ExtDataRef().mpData);
        bool bNew = pGcpHelper == 0;
        if( bNew )
            pGcpHelper = new SvpGcpHelper;

        // get glyph bitmap in matching format
        bool bFound = false;
        switch( nBmpFormat )
        {
            case FORMAT_ONE_BIT_LSB_GREY:
                bFound = rServerFont.GetGlyphBitmap1( aGlyphId, pGcpHelper->maRawBitmap );
                break;
            case FORMAT_EIGHT_BIT_GREY:
                bFound = rServerFont.GetGlyphBitmap8( aGlyphId, pGcpHelper->maRawBitmap );
                break;
            default:
                OSL_FAIL( "SVP GCP::GetGlyphBmp(): illegal scanline format");
                // fall back to black&white mask
                nBmpFormat = FORMAT_ONE_BIT_LSB_GREY;
                bFound = false;
                break;
        }

        // return .notdef glyph if needed
        if( !bFound && (aGlyphId != 0) )
        {
            if( bNew )
                delete pGcpHelper;
            return GetGlyphBmp( rServerFont, 0, nBmpFormat, rTargetPos );
        }

        // construct alpha mask from raw bitmap
        const B2IVector aSize(
            pGcpHelper->maRawBitmap.mnScanlineSize,
            pGcpHelper->maRawBitmap.mnHeight );
        if( aSize.getX() && aSize.getY() )
        {
            static PaletteMemorySharedVector aDummyPAL;
            pGcpHelper->maBitmapDev = createBitmapDevice( aSize, true, nBmpFormat, pGcpHelper->maRawBitmap.mpBits, aDummyPAL );
        }

        rGlyphData.ExtDataRef().meInfo = nBmpFormat;
        rGlyphData.ExtDataRef().mpData = pGcpHelper;
    }

    SvpGcpHelper* pGcpHelper = static_cast<SvpGcpHelper*>(
        rGlyphData.ExtDataRef().mpData);
    assert(pGcpHelper != 0);
    rTargetPos += B2IPoint( pGcpHelper->maRawBitmap.mnXOffset, pGcpHelper->maRawBitmap.mnYOffset );
    return pGcpHelper->maBitmapDev;
}


void SvpGlyphPeer::RemovingFont( ServerFont& )
{
    // nothing to do: no font resources held in SvpGlyphPeer
}


void SvpGlyphPeer::RemovingGlyph( GlyphData& rGlyphData )
{
    SvpGcpHelper* pGcpHelper = static_cast<SvpGcpHelper*>(
        rGlyphData.ExtDataRef().mpData);
    rGlyphData.ExtDataRef().meInfo = basebmp::FORMAT_NONE;
    rGlyphData.ExtDataRef().mpData = 0;
    delete pGcpHelper;
}

sal_uInt16 SvpSalGraphics::SetFont( FontSelectPattern* pIFSD, int nFallbackLevel )
{
    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( m_pServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            SvpGlyphCache::GetInstance().UncacheFont( *m_pServerFont[i] );
            m_pServerFont[i] = NULL;
        }
    }

    // return early if there is no new font
    if( !pIFSD )
        return 0;

    // handle the request for a non-native X11-font => use the GlyphCache
    ServerFont* pServerFont = SvpGlyphCache::GetInstance().CacheFont( *pIFSD );
    if( !pServerFont )
        return SAL_SETFONT_BADFONT;

    // check selected font
    if( !pServerFont->TestFont() )
    {
        SvpGlyphCache::GetInstance().UncacheFont( *pServerFont );
        return SAL_SETFONT_BADFONT;
    }

    // update SalGraphics font settings
    m_pServerFont[ nFallbackLevel ] = pServerFont;
    return SAL_SETFONT_USEDRAWTEXTARRAY;
}


void SvpSalGraphics::GetFontMetric( ImplFontMetricData* pMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if( m_pServerFont[nFallbackLevel] != NULL )
    {
        long rDummyFactor;
        m_pServerFont[nFallbackLevel]->FetchFontMetric( *pMetric, rDummyFactor );
    }
}


const ImplFontCharMap* SvpSalGraphics::GetImplFontCharMap() const
{
    if( !m_pServerFont[0] )
        return NULL;

    const ImplFontCharMap* pIFCMap = m_pServerFont[0]->GetImplFontCharMap();
    return pIFCMap;
}

bool SvpSalGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!m_pServerFont[0])
        return false;

    return m_pServerFont[0]->GetFontCapabilities(rFontCapabilities);
}


void SvpSalGraphics::GetDevFontList( PhysicalFontCollection* pFontCollection )
{
    GlyphCache& rGC = SvpGlyphCache::GetInstance();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    psp::FastPrintFontInfo aInfo;
    ::std::list< psp::fontID > aList;
    rMgr.getFontList( aList );
    ::std::list< psp::fontID >::iterator it;
    for( it = aList.begin(); it != aList.end(); ++it )
    {
        if( !rMgr.getFontFastInfo( *it, aInfo ) )
            continue;

        // normalize face number to the GlyphCache
        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );

        // inform GlyphCache about this font provided by the PsPrint subsystem
        ImplDevFontAttributes aDFA = GenPspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 4096;
        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pFontCollection );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pFontCollection );

    ImplGetSVData()->maGDIData.mbNativeFontConfig = true;
}

void SvpSalGraphics::ClearDevFontCache()
{
    GlyphCache& rGC = SvpGlyphCache::GetInstance();
    rGC.ClearFontCache();
}


bool SvpSalGraphics::AddTempDevFont( PhysicalFontCollection*,
    const OUString&, const OUString& )
{
    return false;
}


bool SvpSalGraphics::CreateFontSubset(
    const OUString& rToFile,
    const PhysicalFontFace* pFont,
    sal_GlyphId* pGlyphIds,
    sal_uInt8* pEncoding,
    sal_Int32* pWidths,
    int nGlyphCount,
    FontSubsetInfo& rInfo
    )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    bool bSuccess = rMgr.createFontSubset( rInfo,
                                 aFont,
                                 rToFile,
                                 pGlyphIds,
                                 pEncoding,
                                 pWidths,
                                 nGlyphCount );
    return bSuccess;
}


const Ucs2SIntMap* SvpSalGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}


const void* SvpSalGraphics::GetEmbedFontData(
    const PhysicalFontFace* pFont,
    const sal_Ucs* pUnicodes,
    sal_Int32* pWidths,
    FontSubsetInfo& rInfo,
    long* pDataLen
    )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
}


void SvpSalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    GenPspGraphics::DoFreeEmbedFontData( pData, nLen );
}

void SvpSalGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    GenPspGraphics::DoGetGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}


bool SvpSalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= GF_IDXMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( aGlyphId );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return true;
}


bool SvpSalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId, B2DPolyPolygon& rPolyPoly )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    const ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= GF_IDXMASK;
    if( pSF->GetGlyphOutline( aGlyphId, rPolyPoly ) )
        return true;

    return false;
}


SalLayout* SvpSalGraphics::GetTextLayout( ImplLayoutArgs&, int nFallbackLevel )
{
    GenericSalLayout* pLayout = NULL;

    if( m_pServerFont[ nFallbackLevel ] )
        pLayout = new ServerFontLayout( *m_pServerFont[ nFallbackLevel ] );

    return pLayout;
}


void SvpSalGraphics::DrawServerFontLayout( const ServerFontLayout& rSalLayout )
{
    // iterate over all glyphs in the layout
    Point aPos;
    sal_GlyphId aGlyphId;
    SvpGlyphPeer& rGlyphPeer = SvpGlyphCache::GetInstance().GetPeer();
    for( int nStart = 0; rSalLayout.GetNextGlyphs( 1, &aGlyphId, aPos, nStart ); )
    {
        int nLevel = aGlyphId >> GF_FONTSHIFT;
        DBG_ASSERT( nLevel < MAX_FALLBACK, "SvpGDI: invalid glyph fallback level" );
        ServerFont* pSF = m_pServerFont[ nLevel ];
        if( !pSF )
            continue;

        // get the glyph's alpha mask and adjust the drawing position
        aGlyphId &= GF_IDXMASK;
        B2IPoint aDstPoint( aPos.X(), aPos.Y() );
        BitmapDeviceSharedPtr aAlphaMask
            = rGlyphPeer.GetGlyphBmp( *pSF, aGlyphId, m_eTextFmt, aDstPoint );
        if( !aAlphaMask )   // ignore empty glyphs
            continue;

        // blend text color into target using the glyph's mask
        const B2IBox aSrcRect( B2ITuple(0,0), aAlphaMask->getSize() );
        const B2IBox aClipRect( aDstPoint, aAlphaMask->getSize() );

        SvpSalGraphics::ClipUndoHandle aUndo( this );
        if( !isClippedSetup( aClipRect, aUndo ) )
            m_aDevice->drawMaskedColor( m_aTextColor, aAlphaMask,
                                        aSrcRect, aDstPoint, m_aClipMap );
    }
}

void SvpSalGraphics::SetTextColor( SalColor nSalColor )
{
    m_aTextColor = basebmp::Color( nSalColor );
}

SystemFontData SvpSalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;

    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.nFontId = 0;
    aSysFontData.nFontFlags = 0;
    aSysFontData.bFakeBold = false;
    aSysFontData.bFakeItalic = false;
    aSysFontData.bAntialias = true;
    return aSysFontData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
