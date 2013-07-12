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

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <basebmp/scanlineformats.hxx>

#include <tools/debug.hxx>

#include <outfont.hxx>
#include <impfont.hxx>
#include <rtl/instance.hxx>

#include "vcl/sysdata.hxx"
#include "generic/geninst.h"
#include "generic/genpspgraphics.h"
#include "generic/glyphcache.hxx"
#include "headless/svpgdi.hxx"
#include "headless/svpbmp.hxx"

using namespace basegfx;
using namespace basebmp;

class SvpGlyphPeer
:   public GlyphCachePeer
{
public:
    SvpGlyphPeer() {}

    BitmapDeviceSharedPtr GetGlyphBmp( ServerFont&, int nGlyphIndex,
                            basebmp::Format nBmpFormat, B2IPoint& rTargetPos );

protected:
    virtual void    RemovingFont( ServerFont& );
    virtual void    RemovingGlyph( ServerFont&, GlyphData&, int nGlyphIndex );

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
    int nGlyphIndex, basebmp::Format nBmpFormat, B2IPoint& rTargetPos )
{
    GlyphData& rGlyphData = rServerFont.GetGlyphData( nGlyphIndex );
    SvpGcpHelper* pGcpHelper = (SvpGcpHelper*)rGlyphData.ExtDataRef().mpData;

    // nothing to do if the GlyphPeer hasn't allocated resources for the glyph
    if( rGlyphData.ExtDataRef().meInfo != nBmpFormat )
    {
        if( rGlyphData.ExtDataRef().meInfo == FORMAT_NONE )
            pGcpHelper = new SvpGcpHelper;
        RawBitmap& rRawBitmap = pGcpHelper->maRawBitmap;

        // get glyph bitmap in matching format
        bool bFound = false;
        switch( nBmpFormat )
        {
            case FORMAT_ONE_BIT_LSB_GREY:
                bFound = rServerFont.GetGlyphBitmap1( nGlyphIndex, pGcpHelper->maRawBitmap );
                break;
            case FORMAT_EIGHT_BIT_GREY:
                bFound = rServerFont.GetGlyphBitmap8( nGlyphIndex, pGcpHelper->maRawBitmap );
                break;
            default:
                OSL_FAIL( "SVP GCP::GetGlyphBmp(): illegal scanline format");
                // fall back to black&white mask
                nBmpFormat = FORMAT_ONE_BIT_LSB_GREY;
                bFound = false;
                break;
        }

        // return .notdef glyph if needed
        if( !bFound && (nGlyphIndex != 0) )
        {
            delete pGcpHelper;
            return GetGlyphBmp( rServerFont, 0, nBmpFormat, rTargetPos );
        }

        // construct alpha mask from raw bitmap
        const B2IVector aSize( rRawBitmap.mnScanlineSize, rRawBitmap.mnHeight );
        if( aSize.getX() && aSize.getY() )
        {
            static PaletteMemorySharedVector aDummyPAL;
            RawMemorySharedArray aRawPtr( rRawBitmap.mpBits );
            pGcpHelper->maBitmapDev = createBitmapDevice( aSize, true, nBmpFormat, aRawPtr, aDummyPAL );
        }

        rServerFont.SetExtended( nBmpFormat, (void*)pGcpHelper );
    }

    rTargetPos += B2IPoint( pGcpHelper->maRawBitmap.mnXOffset, pGcpHelper->maRawBitmap.mnYOffset );
    return pGcpHelper->maBitmapDev;
}


void SvpGlyphPeer::RemovingFont( ServerFont& )
{
    // nothing to do: no font resources held in SvpGlyphPeer
}


void SvpGlyphPeer::RemovingGlyph( ServerFont&, GlyphData& rGlyphData, int /*nGlyphIndex*/ )
{
    if( rGlyphData.ExtDataRef().mpData != 0 )
    {
        // release the glyph related resources
        DBG_ASSERT( (rGlyphData.ExtDataRef().meInfo <= FORMAT_MAX), "SVP::RG() invalid alpha format" );
        SvpGcpHelper* pGcpHelper = (SvpGcpHelper*)rGlyphData.ExtDataRef().mpData;
        delete[] pGcpHelper->maRawBitmap.mpBits;
        delete pGcpHelper;
    }
}


// PspKernInfo allows on-demand-querying of psprint provided kerning info (#i29881#)
class PspKernInfo : public ExtraKernInfo
{
public:
    PspKernInfo( int nFontId ) : ExtraKernInfo(nFontId) {}
protected:
    virtual void Initialize() const;
};


void PspKernInfo::Initialize() const
{
    mbInitialized = true;

    // get the kerning pairs from psprint
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    typedef std::list< psp::KernPair > PspKernPairs;
    const PspKernPairs& rKernPairs = rMgr.getKernPairs( mnFontId );
    if( rKernPairs.empty() )
        return;

    PspKernPairs::const_iterator it = rKernPairs.begin();
    for(; it != rKernPairs.end(); ++it )
    {
        ImplKernPairData aKernPair = { it->first, it->second, it->kern_x };
        maUnicodeKernPairs.insert( aKernPair );
    }
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


sal_uLong SvpSalGraphics::GetKernPairs( sal_uLong nPairs, ImplKernPairData* pKernPairs )
{
    sal_uLong nGotPairs = 0;

    if( m_pServerFont[0] != NULL )
    {
        ImplKernPairData* pTmpKernPairs = NULL;
        nGotPairs = m_pServerFont[0]->GetKernPairs( &pTmpKernPairs );
        for( sal_uLong i = 0; i < nPairs && i < nGotPairs; ++i )
            pKernPairs[ i ] = pTmpKernPairs[ i ];
        delete[] pTmpKernPairs;
    }

    return nGotPairs;
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


void SvpSalGraphics::GetDevFontList( ImplDevFontList* pDevFontList )
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

        // the GlyphCache must not bother with builtin fonts because
        // it cannot access or use them anyway
        if( aInfo.m_eType == psp::fonttype::Builtin )
            continue;

        // normalize face number to the GlyphCache
        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );

        // for fonts where extra kerning info can be provided on demand
        // an ExtraKernInfo object is supplied
        const ExtraKernInfo* pExtraKernInfo = NULL;
        if( aInfo.m_eType == psp::fonttype::Type1 )
            pExtraKernInfo = new PspKernInfo( *it );

        // inform GlyphCache about this font provided by the PsPrint subsystem
        ImplDevFontAttributes aDFA = GenPspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 4096;
        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA, pExtraKernInfo );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pDevFontList );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pDevFontList );

    ImplGetSVData()->maGDIData.mbNativeFontConfig = true;
}

void SvpSalGraphics::ClearDevFontCache()
{
    GlyphCache& rGC = SvpGlyphCache::GetInstance();
    rGC.ClearFontCache();
}


void SvpSalGraphics::GetDevFontSubstList( OutputDevice* )
{}


bool SvpSalGraphics::AddTempDevFont( ImplDevFontList*,
    const OUString&, const OUString& )
{
    return false;
}


sal_Bool SvpSalGraphics::CreateFontSubset(
    const OUString& rToFile,
    const PhysicalFontFace* pFont,
    sal_Int32* pGlyphIDs,
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
                                 pGlyphIDs,
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


sal_Bool SvpSalGraphics::GetGlyphBoundRect( sal_GlyphId nGlyphIndex, Rectangle& rRect )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return sal_False;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return sal_False;

    nGlyphIndex &= GF_IDXMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( nGlyphIndex );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return sal_True;
}


sal_Bool SvpSalGraphics::GetGlyphOutline( sal_GlyphId nGlyphIndex, B2DPolyPolygon& rPolyPoly )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return sal_False;

    const ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return sal_False;

    nGlyphIndex &= GF_IDXMASK;
    if( pSF->GetGlyphOutline( nGlyphIndex, rPolyPoly ) )
        return sal_True;

    return sal_False;
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
    sal_GlyphId nGlyphIndex;
    SvpGlyphPeer& rGlyphPeer = SvpGlyphCache::GetInstance().GetPeer();
    for( int nStart = 0; rSalLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart ); )
    {
        int nLevel = nGlyphIndex >> GF_FONTSHIFT;
        DBG_ASSERT( nLevel < MAX_FALLBACK, "SvpGDI: invalid glyph fallback level" );
        ServerFont* pSF = m_pServerFont[ nLevel ];
        if( !pSF )
            continue;

        // get the glyph's alpha mask and adjust the drawing position
        nGlyphIndex &= GF_IDXMASK;
        B2IPoint aDstPoint( aPos.X(), aPos.Y() );
        BitmapDeviceSharedPtr aAlphaMask
            = rGlyphPeer.GetGlyphBmp( *pSF, nGlyphIndex, m_eTextFmt, aDstPoint );
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
