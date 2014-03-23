/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2irange.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

#include <basebmp/scanlineformats.hxx>

#include <tools/debug.hxx>

#if OSL_DEBUG_LEVEL > 2
#include <basebmp/debug.hxx>
#endif

#include <outfont.hxx>
#include <glyphcache.hxx>
#include <impfont.hxx>

#include "svpgdi.hxx"
#include "svpbmp.hxx"
#include "svppspgraphics.hxx"

using namespace basegfx;
using namespace basebmp;

// ===========================================================================

class SvpGlyphPeer
:   public GlyphCachePeer
{
public:
    SvpGlyphPeer() {}

    BitmapDeviceSharedPtr GetGlyphBmp( ServerFont&, sal_GlyphId,
                            sal_uInt32 nBmpFormat, B2IPoint& rTargetPos );

protected:
    virtual void    RemovingFont( ServerFont& );
    virtual void    RemovingGlyph( ServerFont&, GlyphData&, sal_GlyphId );

    class SvpGcpHelper
    {
    public:
        RawBitmap               maRawBitmap;
        BitmapDeviceSharedPtr   maBitmapDev;
    };
};

// ===========================================================================

class SvpGlyphCache : public GlyphCache
{
public:
    SvpGlyphPeer&       GetPeer() { return reinterpret_cast<SvpGlyphPeer&>( mrPeer ); }
static SvpGlyphCache&   GetInstance();
private:
    SvpGlyphCache( SvpGlyphPeer& rPeer ) : GlyphCache( rPeer) {}
};

//--------------------------------------------------------------------------

SvpGlyphCache& SvpGlyphCache::GetInstance()
{
    static SvpGlyphPeer aSvpGlyphPeer;
    static SvpGlyphCache aGC( aSvpGlyphPeer );
    return aGC;
}

// ===========================================================================

BitmapDeviceSharedPtr SvpGlyphPeer::GetGlyphBmp( ServerFont& rServerFont,
    sal_GlyphId aGlyphId, sal_uInt32 nBmpFormat, B2IPoint& rTargetPos )
{
    GlyphData& rGlyphData = rServerFont.GetGlyphData( aGlyphId );
    SvpGcpHelper* pGcpHelper = (SvpGcpHelper*)rGlyphData.ExtDataRef().mpData;

    // nothing to do if the GlyphPeer hasn't allocated resources for the glyph
    if( rGlyphData.ExtDataRef().meInfo != sal::static_int_cast<int>(nBmpFormat) )
    {
        if( rGlyphData.ExtDataRef().meInfo == Format::NONE )
            pGcpHelper = new SvpGcpHelper;
        RawBitmap& rRawBitmap = pGcpHelper->maRawBitmap;

        // get glyph bitmap in matching format
        bool bFound = false;
        switch( nBmpFormat )
        {
            case Format::ONE_BIT_LSB_GREY:
                bFound = rServerFont.GetGlyphBitmap1( aGlyphId, pGcpHelper->maRawBitmap );
                break;
            case Format::EIGHT_BIT_GREY:
                bFound = rServerFont.GetGlyphBitmap8( aGlyphId, pGcpHelper->maRawBitmap );
                break;
            default:
                DBG_ERROR( "SVP GCP::GetGlyphBmp(): illegal scanline format");
                // fall back to black&white mask
                nBmpFormat = Format::ONE_BIT_LSB_GREY;
                bFound = false;
                break;
        }

        // return .notdef glyph if needed
        if( !bFound && (aGlyphId != 0) )
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

//--------------------------------------------------------------------------

void SvpGlyphPeer::RemovingFont( ServerFont& )
{
    // nothing to do: no font resources held in SvpGlyphPeer
}

//--------------------------------------------------------------------------

void SvpGlyphPeer::RemovingGlyph( ServerFont&, GlyphData& rGlyphData, sal_GlyphId /*aGlyphId*/ )
{
    if( rGlyphData.ExtDataRef().mpData != Format::NONE )
    {
        // release the glyph related resources
        DBG_ASSERT( (rGlyphData.ExtDataRef().meInfo <= Format::MAX), "SVP::RG() invalid alpha format" );
        SvpGcpHelper* pGcpHelper = (SvpGcpHelper*)rGlyphData.ExtDataRef().mpData;
        delete[] pGcpHelper->maRawBitmap.mpBits;
        delete pGcpHelper;
    }
}

// ===========================================================================

// PspKernInfo allows on-demand-querying of psprint provided kerning info (#i29881#)
class PspKernInfo : public ExtraKernInfo
{
public:
    PspKernInfo( int nFontId ) : ExtraKernInfo(nFontId) {}
protected:
    virtual void Initialize() const;
};

//--------------------------------------------------------------------------

void PspKernInfo::Initialize() const
{
    mbInitialized = true;

    // get the kerning pairs from psprint
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    typedef std::list< psp::KernPair > PspKernPairs;
    const PspKernPairs& rKernPairs = rMgr.getKernPairs( mnFontId );
    if( rKernPairs.empty() )
        return;

    // feed psprint's kerning list into a lookup-friendly container
    maUnicodeKernPairs.rehash( rKernPairs.size() );
    PspKernPairs::const_iterator it = rKernPairs.begin();
    for(; it != rKernPairs.end(); ++it )
    {
        ImplKernPairData aKernPair = { it->first, it->second, it->kern_x };
        maUnicodeKernPairs.insert( aKernPair );
    }
}

// ===========================================================================

sal_uInt16 SvpSalGraphics::SetFont( ImplFontSelectData* pIFSD, int nFallbackLevel )
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

// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------

const ImplFontCharMap* SvpSalGraphics::GetImplFontCharMap() const
{
    if( !m_pServerFont[0] )
        return NULL;

    const ImplFontCharMap* pIFCMap = m_pServerFont[0]->GetImplFontCharMap();
    return pIFCMap;
}

// ---------------------------------------------------------------------------

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
        if( nFaceNum < 0 )
            nFaceNum = 0;

        // for fonts where extra kerning info can be provided on demand
        // an ExtraKernInfo object is supplied
        const ExtraKernInfo* pExtraKernInfo = NULL;
        if( aInfo.m_eType == psp::fonttype::Type1 )
            pExtraKernInfo = new PspKernInfo( *it );

        // inform GlyphCache about this font provided by the PsPrint subsystem
        ImplDevFontAttributes aDFA = PspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 4096;
        const rtl::OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA, pExtraKernInfo );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pDevFontList );
}

// ---------------------------------------------------------------------------

void SvpSalGraphics::GetDevFontSubstList( OutputDevice* )
{}

// ---------------------------------------------------------------------------

bool SvpSalGraphics::AddTempDevFont( ImplDevFontList*,
    const String&, const String& )
{
    return false;
}

// ---------------------------------------------------------------------------

sal_Bool SvpSalGraphics::CreateFontSubset(
    const rtl::OUString& rToFile,
    const ImplFontData* pFont,
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
    // be to have the GlyphCache search for the ImplFontData pFont
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

// ---------------------------------------------------------------------------

const Ucs2SIntMap* SvpSalGraphics::GetFontEncodingVector( const ImplFontData* pFont, const Ucs2OStrMap** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}

// ---------------------------------------------------------------------------

const void* SvpSalGraphics::GetEmbedFontData(
    const ImplFontData* pFont,
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
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
}

// ---------------------------------------------------------------------------

void SvpSalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    PspGraphics::DoFreeEmbedFontData( pData, nLen );
}

void SvpSalGraphics::GetGlyphWidths( const ImplFontData* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    PspGraphics::DoGetGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}

// ---------------------------------------------------------------------------

bool SvpSalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= ~GF_FONTMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( aGlyphId );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return true;
}

// ---------------------------------------------------------------------------

bool SvpSalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId, B2DPolyPolygon& rPolyPoly )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    const ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= ~GF_FONTMASK;
    bool bOK = pSF->GetGlyphOutline( aGlyphId, rPolyPoly );
    return bOK;
}

// ---------------------------------------------------------------------------

SalLayout* SvpSalGraphics::GetTextLayout( ImplLayoutArgs&, int nFallbackLevel )
{
    GenericSalLayout* pLayout = NULL;

    if( m_pServerFont[ nFallbackLevel ] )
        pLayout = new ServerFontLayout( *m_pServerFont[ nFallbackLevel ] );

    return pLayout;
}

// ---------------------------------------------------------------------------

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
        aGlyphId &= ~GF_FONTMASK;
        B2IPoint aDstPoint( aPos.X(), aPos.Y() );
        BitmapDeviceSharedPtr aAlphaMask
            = rGlyphPeer.GetGlyphBmp( *pSF, aGlyphId, m_eTextFmt, aDstPoint );
        if( !aAlphaMask )   // ignore empty glyphs
            continue;

        // blend text color into target using the glyph's mask
        const B2IRange aSrcRect( B2ITuple(0,0), aAlphaMask->getSize() );
        m_aDevice->drawMaskedColor( m_aTextColor, aAlphaMask, aSrcRect, aDstPoint, m_aClipMap );
    }
}

// ===========================================================================
