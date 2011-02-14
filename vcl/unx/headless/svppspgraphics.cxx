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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "basegfx/vector/b2ivector.hxx"
#include "basegfx/point/b2ipoint.hxx"

#include "basebmp/color.hxx"

#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/svapp.hxx"
#include "vcl/sysdata.hxx"

#include "salprn.hxx"
#include "salbmp.hxx"
#include "glyphcache.hxx"
#include "impfont.hxx"
#include "outfont.hxx"
#include "fontsubset.hxx"
#include "printergfx.hxx"
#include "svppspgraphics.hxx"
#include "svpbmp.hxx"

using namespace psp;
using namespace rtl;
using namespace basebmp;
using namespace basegfx;

// ----- Implementation of PrinterBmp by means of SalBitmap/BitmapBuffer ---------------

class SalPrinterBmp : public psp::PrinterBmp
{
    private:
    SalPrinterBmp ();

    BitmapDeviceSharedPtr       m_aBitmap;
    public:

                            SalPrinterBmp (const BitmapDeviceSharedPtr& rDevice);
        virtual             ~SalPrinterBmp ();
        virtual sal_uInt32  GetPaletteColor (sal_uInt32 nIdx) const;
        virtual sal_uInt32  GetPaletteEntryCount () const;
        virtual sal_uInt32  GetPixelRGB  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelIdx  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt32  GetWidth () const;
        virtual sal_uInt32  GetHeight() const;
        virtual sal_uInt32  GetDepth () const;

        static sal_uInt32 getRGBFromColor( const basebmp::Color& rCol )
        {
            return    ((rCol.getBlue())          & 0x000000ff)
                    | ((rCol.getGreen() <<  8) & 0x0000ff00)
                    | ((rCol.getRed()   << 16) & 0x00ff0000);
        }
};

SalPrinterBmp::SalPrinterBmp(const BitmapDeviceSharedPtr& rDevice) :
    m_aBitmap( rDevice )
{
}

SalPrinterBmp::~SalPrinterBmp ()
{
}

sal_uInt32
SalPrinterBmp::GetWidth () const
{
    return m_aBitmap.get() ? m_aBitmap->getSize().getX() : 0;
}

sal_uInt32
SalPrinterBmp::GetHeight () const
{
    return m_aBitmap.get() ? m_aBitmap->getSize().getY() : 0;
}

sal_uInt32
SalPrinterBmp::GetDepth () const
{
    return m_aBitmap.get() ?
           SvpElement::getBitCountFromScanlineFormat( m_aBitmap->getScanlineFormat() )
           : 0;
}


sal_uInt32
SalPrinterBmp::GetPaletteEntryCount () const
{
    return m_aBitmap.get() ? m_aBitmap->getPaletteEntryCount() : 0;
}

sal_uInt32
SalPrinterBmp::GetPaletteColor (sal_uInt32 nIdx) const
{
    sal_uInt32 nCol = 0;
    if( m_aBitmap.get() && nIdx < static_cast<sal_uInt32>(m_aBitmap->getPaletteEntryCount()) )
    {
        const basebmp::Color& rColor = (*m_aBitmap->getPalette().get())[ nIdx ];
        nCol = getRGBFromColor( rColor );
    }
    return nCol;
}

sal_uInt32
SalPrinterBmp::GetPixelRGB (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    sal_uInt32 nCol = 0;
    if( m_aBitmap.get() )
        nCol = getRGBFromColor( m_aBitmap->getPixel( B2IPoint( nColumn, nRow ) ) );
    return nCol;
}

sal_uInt8
SalPrinterBmp::GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    sal_uInt8 nGray = 0;
    if( m_aBitmap.get() )
    {
        // TODO: don't use tools color
        basebmp::Color aCol = m_aBitmap->getPixel( B2IPoint( nColumn, nRow ) );
        ::Color aColor( aCol.getRed(), aCol.getGreen(), aCol.getBlue() );
        nGray = aColor.GetLuminance();
    }
    return nGray;
}

sal_uInt8
SalPrinterBmp::GetPixelIdx (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    sal_uInt8 nIdx = 0;
    if( m_aBitmap.get() )
        nIdx = static_cast<sal_uInt8>(m_aBitmap->getPixelData( B2IPoint( nColumn, nRow ) ));
    return nIdx;
}

/*******************************************************
 * PspGraphics                                         *
 *******************************************************/

bool PspGraphics::drawAlphaBitmap( const SalTwoRect&, const SalBitmap& /*rSourceBitmap*/, const SalBitmap& /*rAlphaBitmap*/ )
{
    return false;
}

bool PspGraphics::drawAlphaRect( long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/, sal_uInt8 /*nTransparency*/ )
{
    return false;
}

bool PspGraphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}

PspGraphics::~PspGraphics()
{
    ReleaseFonts();
}

void PspGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY )
{
    if (m_pJobData != NULL)
    {
        int x = m_pJobData->m_aContext.getRenderResolution();

        rDPIX = x;
        rDPIY = x;
    }
}

sal_uInt16 PspGraphics::GetBitCount()
{
    return m_pPrinterGfx->GetBitCount();
}

long PspGraphics::GetGraphicsWidth() const
{
    return 0;
}

void PspGraphics::ResetClipRegion()
{
    m_pPrinterGfx->ResetClipRegion ();
}

void PspGraphics::BeginSetClipRegion( sal_uLong n )
{
    m_pPrinterGfx->BeginSetClipRegion(n);
}

sal_Bool PspGraphics::unionClipRegion( long nX, long nY, long nDX, long nDY )
{
    return (sal_Bool)m_pPrinterGfx->UnionClipRegion (nX, nY, nDX, nDY);
}

bool PspGraphics::unionClipRegion( const ::basegfx::B2DPolyPolygon& )
{
        // TODO: implement and advertise OutDevSupport_B2DClip support
        return false;
}

void PspGraphics::EndSetClipRegion()
{
    m_pPrinterGfx->EndSetClipRegion ();
}

void PspGraphics::SetLineColor()
{
    m_pPrinterGfx->SetLineColor ();
}

void PspGraphics::SetLineColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetLineColor (aColor);
}

void PspGraphics::SetFillColor()
{
    m_pPrinterGfx->SetFillColor ();
}

void PspGraphics::SetFillColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetFillColor (aColor);
}

void PspGraphics::SetROPLineColor( SalROPColor )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::SetROPLineColor() not implemented" );
}

void PspGraphics::SetROPFillColor( SalROPColor )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::SetROPFillColor() not implemented" );
}

void PspGraphics::SetXORMode( bool bSet, bool )
{
    (void)bSet;
    DBG_ASSERT( !bSet, "Error: PrinterGfx::SetXORMode() not implemented" );
}

void PspGraphics::drawPixel( long nX, long nY )
{
    m_pPrinterGfx->DrawPixel (Point(nX, nY));
}

void PspGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->DrawPixel (Point(nX, nY), aColor);
}

void PspGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    m_pPrinterGfx->DrawLine (Point(nX1, nY1), Point(nX2, nY2));
}

void PspGraphics::drawRect( long nX, long nY, long nDX, long nDY )
{
    m_pPrinterGfx->DrawRect (Rectangle(Point(nX, nY), Size(nDX, nDY)));
}

void PspGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint *pPtAry )
{
    m_pPrinterGfx->DrawPolyLine (nPoints, (Point*)pPtAry);
}

void PspGraphics::drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry )
{
    // Point must be equal to SalPoint! see vcl/inc/salgtype.hxx
    m_pPrinterGfx->DrawPolygon (nPoints, (Point*)pPtAry);
}

void PspGraphics::drawPolyPolygon( sal_uInt32           nPoly,
                                   const sal_uInt32   *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    m_pPrinterGfx->DrawPolyPolygon (nPoly, pPoints, (const Point**)pPtAry);
}

bool PspGraphics::drawPolyLine( const ::basegfx::B2DPolygon&, double /*fTransparency*/, const ::basegfx::B2DVector& /*rLineWidths*/, basegfx::B2DLineJoin /*eJoin*/ )
{
        // TODO: implement and advertise OutDevSupport_B2DDraw support
        return false;
}

sal_Bool PspGraphics::drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    m_pPrinterGfx->DrawPolyLineBezier (nPoints, (Point*)pPtAry, pFlgAry);
    return sal_True;
}

sal_Bool PspGraphics::drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    m_pPrinterGfx->DrawPolygonBezier (nPoints, (Point*)pPtAry, pFlgAry);
    return sal_True;
}

sal_Bool PspGraphics::drawPolyPolygonBezier( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             const SalPoint* const* pPtAry,
                                             const sal_uInt8* const* pFlgAry )
{
    // Point must be equal to SalPoint! see vcl/inc/salgtype.hxx
    m_pPrinterGfx->DrawPolyPolygonBezier (nPoly, pPoints, (Point**)pPtAry, (sal_uInt8**)pFlgAry);
    return sal_True;
}

bool PspGraphics::drawPolyPolygon( const basegfx::B2DPolyPolygon&, double /*fTransparency*/ )
{
    // TODO: implement and advertise OutDevSupport_B2DDraw support
    return false;
}

void PspGraphics::invert( sal_uLong /*nPoints*/,
                          const SalPoint* /*pPtAry*/,
                          SalInvert /*nFlags*/ )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::Invert() not implemented" );
}
sal_Bool PspGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    return m_pPrinterGfx->DrawEPS( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ), pPtr, nSize );
}

void PspGraphics::copyBits( const SalTwoRect* /*pPosAry*/,
                            SalGraphics* /*pSSrcGraphics*/ )
{
    DBG_ERROR( "Error: PrinterGfx::CopyBits() not implemented" );
}

void PspGraphics::copyArea ( long /*nDestX*/,    long /*nDestY*/,
                             long /*nSrcX*/,     long /*nSrcY*/,
                             long /*nSrcWidth*/, long /*nSrcHeight*/,
                             sal_uInt16 /*nFlags*/ )
{
    DBG_ERROR( "Error: PrinterGfx::CopyArea() not implemented" );
}

void PspGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
    Rectangle aSrc (Point(pPosAry->mnSrcX, pPosAry->mnSrcY),
                    Size(pPosAry->mnSrcWidth, pPosAry->mnSrcHeight));
    Rectangle aDst (Point(pPosAry->mnDestX, pPosAry->mnDestY),
                    Size(pPosAry->mnDestWidth, pPosAry->mnDestHeight));

    const SvpSalBitmap* pBmp = dynamic_cast<const SvpSalBitmap*>(&rSalBitmap);
    if( pBmp )
    {
        SalPrinterBmp aBmp(pBmp->getBitmap());
        m_pPrinterGfx->DrawBitmap(aDst, aSrc, aBmp);
    }
}

void PspGraphics::drawBitmap( const SalTwoRect* /*pPosAry*/,
                              const SalBitmap& /*rSalBitmap*/,
                              const SalBitmap& /*rTransBitmap*/ )
{
    DBG_ERROR("Error: no PrinterGfx::DrawBitmap() for transparent bitmap");
}

void PspGraphics::drawBitmap( const SalTwoRect* /*pPosAry*/,
                              const SalBitmap& /*rSalBitmap*/,
                              SalColor /*nTransparentColor*/ )
{
    DBG_ERROR("Error: no PrinterGfx::DrawBitmap() for transparent color");
}

void PspGraphics::drawMask( const SalTwoRect* /*pPosAry*/,
                            const SalBitmap& /*rSalBitmap*/,
                            SalColor /*nMaskColor*/ )
{
    DBG_ERROR("Error: PrinterGfx::DrawMask() not implemented");
}

SalBitmap* PspGraphics::getBitmap( long /*nX*/, long /*nY*/, long /*nDX*/, long /*nDY*/ )
{
    DBG_WARNING ("Warning: PrinterGfx::GetBitmap() not implemented");
    return NULL;
}

SalColor PspGraphics::getPixel( long /*nX*/, long /*nY*/ )
{
    DBG_ERROR ("Warning: PrinterGfx::GetPixel() not implemented");
    return 0;
}

void PspGraphics::invert(
                         long       /*nX*/,
                         long       /*nY*/,
                         long       /*nDX*/,
                         long       /*nDY*/,
                         SalInvert  /*nFlags*/ )
{
    DBG_ERROR ("Warning: PrinterGfx::Invert() not implemented");
}

//==========================================================================

class ImplPspFontData : public ImplFontData
{
private:
    enum { PSPFD_MAGIC = 0xb5bf01f0 };
    sal_IntPtr              mnFontId;

public:
                            ImplPspFontData( const psp::FastPrintFontInfo& );
    virtual sal_IntPtr      GetFontId() const { return mnFontId; }
    virtual ImplFontData*   Clone() const { return new ImplPspFontData( *this ); }
    virtual ImplFontEntry*  CreateFontInstance( ImplFontSelectData& ) const;
    static bool             CheckFontData( const ImplFontData& r ) { return r.CheckMagic( PSPFD_MAGIC ); }
};

//--------------------------------------------------------------------------

ImplPspFontData::ImplPspFontData( const psp::FastPrintFontInfo& rInfo )
:   ImplFontData( PspGraphics::Info2DevFontAttributes(rInfo), PSPFD_MAGIC ),
    mnFontId( rInfo.m_nID )
{}

//--------------------------------------------------------------------------

ImplFontEntry* ImplPspFontData::CreateFontInstance( ImplFontSelectData& rFSD ) const
{
    ImplServerFontEntry* pEntry = new ImplServerFontEntry( rFSD );
    return pEntry;
}

//==========================================================================

class PspFontLayout : public GenericSalLayout
{
public:
                        PspFontLayout( ::psp::PrinterGfx& );
    virtual bool        LayoutText( ImplLayoutArgs& );
    virtual void        InitFont() const;
    virtual void        DrawText( SalGraphics& ) const;
private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    sal_IntPtr          mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
};

//--------------------------------------------------------------------------

PspFontLayout::PspFontLayout( ::psp::PrinterGfx& rGfx )
:   mrPrinterGfx( rGfx )
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
}

//--------------------------------------------------------------------------

bool PspFontLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    mbVertical = ((rArgs.mnFlags & SAL_LAYOUT_VERTICAL) != 0);

    long nUnitsPerPixel = 1;
    int nOldGlyphId = -1;
    long nGlyphWidth = 0;
    int nCharPos = -1;
    Point aNewPos( 0, 0 );
    GlyphItem aPrevItem;
    rtl_TextEncoding aFontEnc = mrPrinterGfx.GetFontMgr().getFontEncoding( mnFontID );
    for(;;)
    {
        bool bRightToLeft;
        if( !rArgs.GetNextPos( &nCharPos, &bRightToLeft ) )
            break;

        sal_UCS4 cChar = rArgs.mpStr[ nCharPos ];
        if( bRightToLeft )
            cChar = GetMirroredChar( cChar );
        // symbol font aliasing: 0x0020-0x00ff -> 0xf020 -> 0xf0ff
        if( aFontEnc == RTL_TEXTENCODING_SYMBOL )
            if( cChar < 256 )
                cChar += 0xf000;
        int nGlyphIndex = cChar;  // printer glyphs = unicode

        // update fallback_runs if needed
        psp::CharacterMetric aMetric;
        mrPrinterGfx.GetFontMgr().getMetrics( mnFontID, cChar, cChar, &aMetric, mbVertical );
        if( aMetric.width == -1 && aMetric.height == -1 )
            rArgs.NeedFallback( nCharPos, bRightToLeft );

        // apply pair kerning to prev glyph if requested
        if( SAL_LAYOUT_KERNING_PAIRS & rArgs.mnFlags )
        {
            if( nOldGlyphId > 0 )
            {
                const std::list< KernPair >& rKernPairs = mrPrinterGfx.getKernPairs(mbVertical);
                for( std::list< KernPair >::const_iterator it = rKernPairs.begin();
                     it != rKernPairs.end(); ++it )
                {
                    if( it->first == nOldGlyphId && it->second == nGlyphIndex )
                    {
                        int nTextScale = mrPrinterGfx.GetFontWidth();
                        if( ! nTextScale )
                            nTextScale = mrPrinterGfx.GetFontHeight();
                        int nKern = (mbVertical ? it->kern_y : it->kern_x) * nTextScale;
                        nGlyphWidth += nKern;
                        aPrevItem.mnNewWidth = nGlyphWidth;
                        break;
                    }
                }
            }
        }

        // finish previous glyph
        if( nOldGlyphId >= 0 )
            AppendGlyph( aPrevItem );
        nOldGlyphId = nGlyphIndex;
        aNewPos.X() += nGlyphWidth;

        // prepare GlyphItem for appending it in next round
        nUnitsPerPixel = mrPrinterGfx.GetCharWidth( cChar, cChar, &nGlyphWidth );
        int nGlyphFlags = bRightToLeft ? GlyphItem::IS_RTL_GLYPH : 0;
        nGlyphIndex |= GF_ISCHAR;
        aPrevItem = GlyphItem( nCharPos, nGlyphIndex, aNewPos, nGlyphFlags, nGlyphWidth );
    }

    // append last glyph item if any
    if( nOldGlyphId >= 0 )
        AppendGlyph( aPrevItem );

    SetOrientation( mrPrinterGfx.GetFontAngle() );
    SetUnitsPerPixel( nUnitsPerPixel );
    return (nOldGlyphId >= 0);
}

class PspServerFontLayout : public ServerFontLayout
{
public:
    PspServerFontLayout( psp::PrinterGfx&, ServerFont& rFont, const ImplLayoutArgs& rArgs );

    virtual void        InitFont() const;
    const sal_Unicode*  getTextPtr() const { return maText.getStr() - mnMinCharPos; }
    int                 getMinCharPos() const { return mnMinCharPos; }
    int                 getMaxCharPos() const { return mnMinCharPos+maText.getLength()-1; }
private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    sal_IntPtr          mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
    rtl::OUString       maText;
    int                 mnMinCharPos;
};

PspServerFontLayout::PspServerFontLayout( ::psp::PrinterGfx& rGfx, ServerFont& rFont, const ImplLayoutArgs& rArgs )
        :   ServerFontLayout( rFont ),
            mrPrinterGfx( rGfx )
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
    maText       = OUString( rArgs.mpStr + rArgs.mnMinCharPos, rArgs.mnEndCharPos - rArgs.mnMinCharPos+1 );
    mnMinCharPos = rArgs.mnMinCharPos;
}

void PspServerFontLayout::InitFont() const
{
    mrPrinterGfx.SetFont( mnFontID, mnFontHeight, mnFontWidth,
                          mnOrientation, mbVertical, mbArtItalic, mbArtBold );
}

//--------------------------------------------------------------------------

static void DrawPrinterLayout( const SalLayout& rLayout, ::psp::PrinterGfx& rGfx, bool bIsPspServerFontLayout )
{
    const int nMaxGlyphs = 200;
    sal_GlyphId aGlyphAry[ nMaxGlyphs ];
    sal_Int32   aWidthAry[ nMaxGlyphs ];
    sal_Int32   aIdxAry  [ nMaxGlyphs ];
    sal_Ucs     aUnicodes[ nMaxGlyphs ];
    int         aCharPosAry [ nMaxGlyphs ];

    Point aPos;
    long nUnitsPerPixel = rLayout.GetUnitsPerPixel();
    const sal_Unicode* pText = bIsPspServerFontLayout ? static_cast<const PspServerFontLayout&>(rLayout).getTextPtr() : NULL;
    int nMinCharPos = bIsPspServerFontLayout ? static_cast<const PspServerFontLayout&>(rLayout).getMinCharPos() : 0;
    int nMaxCharPos = bIsPspServerFontLayout ? static_cast<const PspServerFontLayout&>(rLayout).getMaxCharPos() : 0;
    for( int nStart = 0;; )
    {
        int nGlyphCount = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart, aWidthAry, bIsPspServerFontLayout ? aCharPosAry : NULL );
        if( !nGlyphCount )
            break;

        sal_Int32 nXOffset = 0;
        for( int i = 0; i < nGlyphCount; ++i )
        {
            nXOffset += aWidthAry[ i ];
            aIdxAry[ i ] = nXOffset / nUnitsPerPixel;
            sal_Int32 nGlyphIdx = aGlyphAry[i] & (GF_IDXMASK | GF_ROTMASK);
            if( bIsPspServerFontLayout )
                aUnicodes[i] = (aCharPosAry[i] >= nMinCharPos && aCharPosAry[i] <= nMaxCharPos) ? pText[ aCharPosAry[i] ] : 0;
            else
                aUnicodes[i] = (aGlyphAry[i] & GF_ISCHAR) ? nGlyphIdx : 0;
            aGlyphAry[i] = nGlyphIdx;
        }

        rGfx.DrawGlyphs( aPos, (sal_uInt32 *)aGlyphAry, aUnicodes, nGlyphCount, aIdxAry );
    }
}

//--------------------------------------------------------------------------

void PspFontLayout::InitFont() const
{
    mrPrinterGfx.SetFont( mnFontID, mnFontHeight, mnFontWidth,
        mnOrientation, mbVertical, mbArtItalic, mbArtBold );
}

//--------------------------------------------------------------------------

void PspFontLayout::DrawText( SalGraphics& ) const
{
    DrawPrinterLayout( *this, mrPrinterGfx, false );
}

void PspGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    // print complex text
    DrawPrinterLayout( rLayout, *m_pPrinterGfx, true );
}

const ImplFontCharMap* PspGraphics::GetImplFontCharMap() const
{
    if( !m_pServerFont[0] )
        return NULL;

    const ImplFontCharMap* pIFCMap = m_pServerFont[0]->GetImplFontCharMap();
    return pIFCMap;
}

sal_uInt16 PspGraphics::SetFont( ImplFontSelectData *pEntry, int nFallbackLevel )
{
    // release all fonts that are to be overridden
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( m_pServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *m_pServerFont[i] );
            m_pServerFont[i] = NULL;
        }
    }

    // return early if there is no new font
    if( !pEntry )
        return 0;

    sal_IntPtr nID = pEntry->mpFontData ? pEntry->mpFontData->GetFontId() : 0;

    // determine which font attributes need to be emulated
    bool bArtItalic = false;
    bool bArtBold = false;
    if( pEntry->meItalic == ITALIC_OBLIQUE || pEntry->meItalic == ITALIC_NORMAL )
    {
        psp::italic::type eItalic = m_pPrinterGfx->GetFontMgr().getFontItalic( nID );
        if( eItalic != psp::italic::Italic && eItalic != psp::italic::Oblique )
            bArtItalic = true;
    }
    int nWeight = (int)pEntry->meWeight;
    int nRealWeight = (int)m_pPrinterGfx->GetFontMgr().getFontWeight( nID );
    if( nRealWeight <= (int)psp::weight::Medium && nWeight > (int)WEIGHT_MEDIUM )
    {
        bArtBold = true;
    }

    // also set the serverside font for layouting
    m_bFontVertical = pEntry->mbVertical;
    if( pEntry->mpFontData )
    {
        // requesting a font provided by builtin rasterizer
        ServerFont* pServerFont = GlyphCache::GetInstance().CacheFont( *pEntry );
        if( pServerFont != NULL )
        {
            if( pServerFont->TestFont() )
                m_pServerFont[ nFallbackLevel ] = pServerFont;
            else
                GlyphCache::GetInstance().UncacheFont( *pServerFont );
        }
    }

    // set the printer font
    return m_pPrinterGfx->SetFont( nID,
                                   pEntry->mnHeight,
                                   pEntry->mnWidth,
                                   pEntry->mnOrientation,
                                   pEntry->mbVertical,
                                   bArtItalic,
                                   bArtBold
                                   );
}

void PspGraphics::SetTextColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetTextColor (aColor);
}

bool PspGraphics::AddTempDevFont( ImplDevFontList*, const String&, const String& )
{
    return false;
}

void PspGraphics::GetDevFontList( ImplDevFontList *pList )
{
    ::std::list< psp::fontID > aList;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aList, m_pJobData->m_pParser, m_pInfoPrinter->m_bCompatMetrics );

    ::std::list< psp::fontID >::iterator it;
    psp::FastPrintFontInfo aInfo;
    for (it = aList.begin(); it != aList.end(); ++it)
        if (rMgr.getFontFastInfo (*it, aInfo))
            AnnounceFonts( pList, aInfo );
}

void PspGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{
    const psp::PrinterInfo& rInfo = psp::PrinterInfoManager::get().getPrinterInfo( m_pJobData->m_aPrinterName );
    if( rInfo.m_bPerformFontSubstitution )
    {
        for( std::hash_map< rtl::OUString, rtl::OUString, rtl::OUStringHash >::const_iterator it = rInfo.m_aFontSubstitutes.begin(); it != rInfo.m_aFontSubstitutes.end(); ++it )
            AddDevFontSubstitute( pOutDev, it->first, it->second, FONT_SUBSTITUTE_ALWAYS );
    }
}

void PspGraphics::GetFontMetric( ImplFontMetricData *pMetric, int )
{
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    psp::PrintFontInfo aInfo;

    if (rMgr.getFontInfo (m_pPrinterGfx->GetFontID(), aInfo))
    {
        ImplDevFontAttributes aDFA = Info2DevFontAttributes( aInfo );
        static_cast<ImplFontAttributes&>(*pMetric) = aDFA;
        pMetric->mbDevice       = aDFA.mbDevice;
        pMetric->mbScalableFont = true;

        pMetric->mnOrientation  = m_pPrinterGfx->GetFontAngle();
        pMetric->mnSlant        = 0;

        sal_Int32 nTextHeight   = m_pPrinterGfx->GetFontHeight();
        sal_Int32 nTextWidth    = m_pPrinterGfx->GetFontWidth();
        if( ! nTextWidth )
            nTextWidth = nTextHeight;

        pMetric->mnWidth        = nTextWidth;
        pMetric->mnAscent       = ( aInfo.m_nAscend * nTextHeight + 500 ) / 1000;
        pMetric->mnDescent      = ( aInfo.m_nDescend * nTextHeight + 500 ) / 1000;
        pMetric->mnIntLeading   = ( aInfo.m_nLeading * nTextHeight + 500 ) / 1000;
        pMetric->mnExtLeading   = 0;
    }
}

sal_uLong PspGraphics::GetKernPairs( sal_uLong nPairs, ImplKernPairData *pKernPairs )
{
    const ::std::list< ::psp::KernPair >& rPairs( m_pPrinterGfx->getKernPairs() );
    sal_uLong nHavePairs = rPairs.size();
    if( pKernPairs && nPairs )
    {
        ::std::list< ::psp::KernPair >::const_iterator it;
        unsigned int i;
        int nTextScale = m_pPrinterGfx->GetFontWidth();
        if( ! nTextScale )
            nTextScale = m_pPrinterGfx->GetFontHeight();
        for( i = 0, it = rPairs.begin(); i < nPairs && i < nHavePairs; i++, ++it )
        {
            pKernPairs[i].mnChar1   = it->first;
            pKernPairs[i].mnChar2   = it->second;
            pKernPairs[i].mnKern    = it->kern_x * nTextScale / 1000;
        }

    }
    return nHavePairs;
}

sal_Bool PspGraphics::GetGlyphBoundRect( long nGlyphIndex, Rectangle& rRect )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return sal_False;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return sal_False;

    nGlyphIndex &= ~GF_FONTMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( nGlyphIndex );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return sal_True;
}

sal_Bool PspGraphics::GetGlyphOutline( long nGlyphIndex,
    ::basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return sal_False;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return sal_False;

    nGlyphIndex &= ~GF_FONTMASK;
    if( pSF->GetGlyphOutline( nGlyphIndex, rB2DPolyPoly ) )
        return sal_True;

    return sal_False;
}

SalLayout* PspGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    // workaround for printers not handling glyph indexing for non-TT fonts
    int nFontId = m_pPrinterGfx->GetFontID();
    if( psp::fonttype::TrueType != psp::PrintFontManager::get().getFontType( nFontId ) )
        rArgs.mnFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
    else if( nFallbackLevel > 0 )
        rArgs.mnFlags &= ~SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;

    GenericSalLayout* pLayout = NULL;

    if( m_pServerFont[ nFallbackLevel ]
        && !(rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
        pLayout = new PspServerFontLayout( *m_pPrinterGfx, *m_pServerFont[nFallbackLevel], rArgs );
    else
        pLayout = new PspFontLayout( *m_pPrinterGfx );

    return pLayout;
}

//--------------------------------------------------------------------------

sal_Bool PspGraphics::CreateFontSubset(
                                   const rtl::OUString& rToFile,
                                   const ImplFontData* pFont,
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
    // be to have the GlyphCache search for the ImplFontData pFont
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

//--------------------------------------------------------------------------

const void* PspGraphics::GetEmbedFontData( const ImplFontData* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
}

//--------------------------------------------------------------------------

void PspGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    PspGraphics::DoFreeEmbedFontData( pData, nLen );
}

//--------------------------------------------------------------------------

const Ucs2SIntMap* PspGraphics::GetFontEncodingVector( const ImplFontData* pFont, const Ucs2OStrMap** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}

//--------------------------------------------------------------------------

void PspGraphics::GetGlyphWidths( const ImplFontData* pFont,
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

// static helpers of PspGraphics

const void* PspGraphics::DoGetEmbedFontData( fontID aFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    psp::PrintFontInfo aFontInfo;
    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
        return NULL;

    // fill in font info
    rInfo.m_nAscent     = aFontInfo.m_nAscend;
    rInfo.m_nDescent    = aFontInfo.m_nDescend;
    rInfo.m_aPSName     = rMgr.getPSName( aFont );

    int xMin, yMin, xMax, yMax;
    rMgr.getFontBoundingBox( aFont, xMin, yMin, xMax, yMax );

    psp::CharacterMetric aMetrics[256];
    sal_Ucs aUnicodes[256];
    if( aFontInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL && aFontInfo.m_eType == psp::fonttype::Type1 )
    {
        for( int i = 0; i < 256; i++ )
            aUnicodes[i] = pUnicodes[i] < 0x0100 ? pUnicodes[i] + 0xf000 : pUnicodes[i];
        pUnicodes = aUnicodes;
    }
    if( ! rMgr.getMetrics( aFont, pUnicodes, 256, aMetrics ) )
        return NULL;

    OString aSysPath = rMgr.getFontFileSysPath( aFont );
    struct stat aStat;
    if( stat( aSysPath.getStr(), &aStat ) )
        return NULL;
    int fd = open( aSysPath.getStr(), O_RDONLY );
    if( fd < 0 )
        return NULL;
    void* pFile = mmap( NULL, aStat.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    close( fd );
    if( pFile == MAP_FAILED )
        return NULL;

    *pDataLen = aStat.st_size;

    rInfo.m_aFontBBox   = Rectangle( Point( xMin, yMin ), Size( xMax-xMin, yMax-yMin ) );
    rInfo.m_nCapHeight  = yMax; // Well ...

    for( int i = 0; i < 256; i++ )
        pWidths[i] = (aMetrics[i].width > 0 ? aMetrics[i].width : 0);

    switch( aFontInfo.m_eType )
    {
        case psp::fonttype::TrueType:
            rInfo.m_nFontType = FontSubsetInfo::SFNT_TTF;
            break;
        case psp::fonttype::Type1: {
            const bool bPFA = ((*(unsigned char*)pFile) < 0x80);
            rInfo.m_nFontType = bPFA ? FontSubsetInfo::TYPE1_PFA : FontSubsetInfo::TYPE1_PFB;
            }
            break;
        default:
            return NULL;
    }

    return pFile;
}

void PspGraphics::DoFreeEmbedFontData( const void* pData, long nLen )
{
    if( pData )
        munmap( (char*)pData, nLen );
}

const Ucs2SIntMap* PspGraphics::DoGetFontEncodingVector( fontID aFont, const Ucs2OStrMap** pNonEncoded )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    psp::PrintFontInfo aFontInfo;
    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
    {
        if( pNonEncoded )
            *pNonEncoded = NULL;
        return NULL;
    }

    return rMgr.getEncodingMap( aFont, pNonEncoded );
}

void PspGraphics::DoGetGlyphWidths( psp::fontID aFont,
                                    bool bVertical,
                                    Int32Vector& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}

// ----------------------------------------------------------------------------

FontWidth PspGraphics::ToFontWidth (psp::width::type eWidth)
{
    switch (eWidth)
    {
        case psp::width::UltraCondensed: return WIDTH_ULTRA_CONDENSED;
        case psp::width::ExtraCondensed: return WIDTH_EXTRA_CONDENSED;
        case psp::width::Condensed:      return WIDTH_CONDENSED;
        case psp::width::SemiCondensed:  return WIDTH_SEMI_CONDENSED;
        case psp::width::Normal:         return WIDTH_NORMAL;
        case psp::width::SemiExpanded:   return WIDTH_SEMI_EXPANDED;
        case psp::width::Expanded:       return WIDTH_EXPANDED;
        case psp::width::ExtraExpanded:  return WIDTH_EXTRA_EXPANDED;
        case psp::width::UltraExpanded:  return WIDTH_ULTRA_EXPANDED;
        default: break;
    }
    return WIDTH_DONTKNOW;
}

FontWeight PspGraphics::ToFontWeight (psp::weight::type eWeight)
{
    switch (eWeight)
    {
        case psp::weight::Thin:       return WEIGHT_THIN;
        case psp::weight::UltraLight: return WEIGHT_ULTRALIGHT;
        case psp::weight::Light:      return WEIGHT_LIGHT;
        case psp::weight::SemiLight:  return WEIGHT_SEMILIGHT;
        case psp::weight::Normal:     return WEIGHT_NORMAL;
        case psp::weight::Medium:     return WEIGHT_MEDIUM;
        case psp::weight::SemiBold:   return WEIGHT_SEMIBOLD;
        case psp::weight::Bold:       return WEIGHT_BOLD;
        case psp::weight::UltraBold:  return WEIGHT_ULTRABOLD;
        case psp::weight::Black:      return WEIGHT_BLACK;
        default: break;
    }
    return WEIGHT_DONTKNOW;
}

FontPitch PspGraphics::ToFontPitch (psp::pitch::type ePitch)
{
    switch (ePitch)
    {
        case psp::pitch::Fixed:     return PITCH_FIXED;
        case psp::pitch::Variable:  return PITCH_VARIABLE;
        default: break;
    }
    return PITCH_DONTKNOW;
}

FontItalic PspGraphics::ToFontItalic (psp::italic::type eItalic)
{
    switch (eItalic)
    {
        case psp::italic::Upright:  return ITALIC_NONE;
        case psp::italic::Oblique:  return ITALIC_OBLIQUE;
        case psp::italic::Italic:   return ITALIC_NORMAL;
        default: break;
    }
    return ITALIC_DONTKNOW;
}

FontFamily PspGraphics::ToFontFamily (psp::family::type eFamily)
{
    switch (eFamily)
    {
        case psp::family::Decorative: return FAMILY_DECORATIVE;
        case psp::family::Modern:     return FAMILY_MODERN;
        case psp::family::Roman:      return FAMILY_ROMAN;
        case psp::family::Script:     return FAMILY_SCRIPT;
        case psp::family::Swiss:      return FAMILY_SWISS;
        case psp::family::System:     return FAMILY_SYSTEM;
        default: break;
    }
    return FAMILY_DONTKNOW;
}

ImplDevFontAttributes PspGraphics::Info2DevFontAttributes( const psp::FastPrintFontInfo& rInfo )
{
    ImplDevFontAttributes aDFA;
    aDFA.maName         = rInfo.m_aFamilyName;
    aDFA.maStyleName    = rInfo.m_aStyleName;
    aDFA.meFamily       = ToFontFamily (rInfo.m_eFamilyStyle);
    aDFA.meWeight       = ToFontWeight (rInfo.m_eWeight);
    aDFA.meItalic       = ToFontItalic (rInfo.m_eItalic);
    aDFA.meWidthType    = ToFontWidth (rInfo.m_eWidth);
    aDFA.mePitch        = ToFontPitch (rInfo.m_ePitch);
    aDFA.mbSymbolFlag   = (rInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL);

    switch( rInfo.m_eType )
    {
        case psp::fonttype::Builtin:
            aDFA.mnQuality       = 1024;
            aDFA.mbDevice        = true;
            aDFA.mbSubsettable   = false;
            aDFA.mbEmbeddable    = false;
            break;
        case psp::fonttype::TrueType:
            aDFA.mnQuality       = 512;
            aDFA.mbDevice        = false;
            aDFA.mbSubsettable   = true;
            aDFA.mbEmbeddable    = false;
            break;
        case psp::fonttype::Type1:
            aDFA.mnQuality       = 0;
            aDFA.mbDevice        = false;
            aDFA.mbSubsettable   = false;
            aDFA.mbEmbeddable    = true;
            break;
        default:
            aDFA.mnQuality       = 0;
            aDFA.mbDevice        = false;
            aDFA.mbSubsettable   = false;
            aDFA.mbEmbeddable    = false;
            break;
    }

    aDFA.mbOrientation   = true;

    // add font family name aliases
    ::std::list< OUString >::const_iterator it = rInfo.m_aAliases.begin();
    bool bHasMapNames = false;
    for(; it != rInfo.m_aAliases.end(); ++it )
    {
        if( bHasMapNames )
            aDFA.maMapNames.Append( ';' );
        aDFA.maMapNames.Append( (*it).getStr() );
        bHasMapNames = true;
    }

#if OSL_DEBUG_LEVEL > 2
    if( bHasMapNames )
    {
        ByteString aOrigName( aDFA.maName, osl_getThreadTextEncoding() );
        ByteString aAliasNames( aDFA.maMapNames, osl_getThreadTextEncoding() );
        fprintf( stderr, "using alias names \"%s\" for font family \"%s\"\n",
            aAliasNames.GetBuffer(), aOrigName.GetBuffer() );
    }
#endif

    return aDFA;
}

// -----------------------------------------------------------------------

void PspGraphics::AnnounceFonts( ImplDevFontList* pFontList, const psp::FastPrintFontInfo& aInfo )
{
    int nQuality = 0;

    if( aInfo.m_eType == psp::fonttype::TrueType )
    {
        // asian type 1 fonts are not known
        psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        ByteString aFileName( rMgr.getFontFileSysPath( aInfo.m_nID ) );
        int nPos = aFileName.SearchBackward( '_' );
        if( nPos == STRING_NOTFOUND || aFileName.GetChar( nPos+1 ) == '.' )
            nQuality += 5;
        else
        {
            static const char* pLangBoost = NULL;
            static bool bOnce = true;
            if( bOnce )
            {
                bOnce = false;
                const LanguageType aLang = Application::GetSettings().GetUILanguage();
                switch( aLang )
                {
                    case LANGUAGE_JAPANESE:
                        pLangBoost = "jan";
                        break;
                    case LANGUAGE_CHINESE:
                    case LANGUAGE_CHINESE_SIMPLIFIED:
                    case LANGUAGE_CHINESE_SINGAPORE:
                        pLangBoost = "zhs";
                        break;
                    case LANGUAGE_CHINESE_TRADITIONAL:
                    case LANGUAGE_CHINESE_HONGKONG:
                    case LANGUAGE_CHINESE_MACAU:
                        pLangBoost = "zht";
                        break;
                    case LANGUAGE_KOREAN:
                    case LANGUAGE_KOREAN_JOHAB:
                        pLangBoost = "kor";
                        break;
                }
            }

            if( pLangBoost )
                if( aFileName.Copy( nPos+1, 3 ).EqualsIgnoreCaseAscii( pLangBoost ) )
                    nQuality += 10;
        }
    }

    ImplPspFontData* pFD = new ImplPspFontData( aInfo );
    pFD->mnQuality += nQuality;
    pFontList->Add( pFD );
}

bool PspGraphics::filterText( const String& rOrig, String& rNewText, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop )
{
    if( ! m_pPhoneNr )
        return false;

    rCutStop = rCutStart = STRING_NOTFOUND;

#define FAX_PHONE_TOKEN          "@@#"
#define FAX_PHONE_TOKEN_LENGTH   3
#define FAX_END_TOKEN            "@@"
#define FAX_END_TOKEN_LENGTH     2

    bool bRet = false;
    bool bStarted = false;
    bool bStopped = false;
    sal_uInt16 nPos;
    sal_uInt16 nStart = 0;
    sal_uInt16 nStop = rLen;
    String aPhone = rOrig.Copy( nIndex, rLen );

    if( ! m_bPhoneCollectionActive )
    {
        if( ( nPos = aPhone.SearchAscii( FAX_PHONE_TOKEN ) ) != STRING_NOTFOUND )
        {
            nStart = nPos;
            m_bPhoneCollectionActive = true;
            m_aPhoneCollection.Erase();
            bRet = true;
            bStarted = true;
        }
    }
    if( m_bPhoneCollectionActive )
    {
        bRet = true;
        nPos = bStarted ? nStart + FAX_PHONE_TOKEN_LENGTH : 0;
        if( ( nPos = aPhone.SearchAscii( FAX_END_TOKEN, nPos ) ) != STRING_NOTFOUND )
        {
            m_bPhoneCollectionActive = false;
            nStop = nPos + FAX_END_TOKEN_LENGTH;
            bStopped = true;
        }
        int nTokenStart = nStart + (bStarted ? FAX_PHONE_TOKEN_LENGTH : 0);
        int nTokenStop = nStop - (bStopped ? FAX_END_TOKEN_LENGTH : 0);
        m_aPhoneCollection += aPhone.Copy( nTokenStart, nTokenStop - nTokenStart );
        if( ! m_bPhoneCollectionActive )
        {
            m_pPhoneNr->AppendAscii( "<Fax#>" );
            m_pPhoneNr->Append( m_aPhoneCollection );
            m_pPhoneNr->AppendAscii( "</Fax#>" );
            m_aPhoneCollection.Erase();
        }
    }
    if( m_aPhoneCollection.Len() > 1024 )
    {
        m_bPhoneCollectionActive = false;
        m_aPhoneCollection.Erase();
        bRet = false;
    }

    if( bRet && m_bSwallowFaxNo )
    {
        rLen -= nStop - nStart;
        rCutStart = nStart+nIndex;
        rCutStop = nStop+nIndex;
        if( rCutStart )
            rNewText = rOrig.Copy( 0, rCutStart );
        rNewText += rOrig.Copy( rCutStop );
    }

    return bRet && m_bSwallowFaxNo;
}

SystemFontData PspGraphics::GetSysFontData( int nFallbacklevel ) const
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

SystemGraphicsData PspGraphics::GetGraphicsData() const
{
    SystemGraphicsData aRes;
    aRes.nSize = sizeof(aRes);
        aRes.hDrawable = 0;
        aRes.pRenderFormat = 0;
    return aRes;
}

