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

#include <sal/config.h>

#include <vector>

#include <sal/types.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <comphelper/string.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/jobdata.hxx>
#include <vcl/printerinfomanager.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/fontcharmap.hxx>
#include <config_cairo_canvas.h>

#include "fontsubset.hxx"
#include "unx/geninst.h"
#include "unx/genpspgraphics.h"
#include "unx/glyphcache.hxx"
#include "unx/printergfx.hxx"
#include "impfont.hxx"
#include "langboost.hxx"
#include "fontinstance.hxx"
#include "fontattributes.hxx"
#include "impfontmetricdata.hxx"
#include "PhysicalFontCollection.hxx"
#include "PhysicalFontFace.hxx"
#include "salbmp.hxx"
#include "salprn.hxx"
#include "CommonSalLayout.hxx"

using namespace psp;

// ----- Implementation of PrinterBmp by means of SalBitmap/BitmapBuffer ---------------

class SalPrinterBmp : public psp::PrinterBmp
{
private:
    BitmapBuffer*       mpBmpBuffer;

    FncGetPixel         mpFncGetPixel;
    Scanline            mpScanAccess;
    sal_PtrDiff         mnScanOffset;

    sal_uInt32          ColorOf (BitmapColor& rColor) const;
    sal_uInt8           GrayOf  (BitmapColor& rColor) const;

    public:

        explicit        SalPrinterBmp (BitmapBuffer* pBitmap);
        virtual sal_uInt32  GetPaletteColor (sal_uInt32 nIdx) const override;
        virtual sal_uInt32  GetPaletteEntryCount () const override;
        virtual sal_uInt32  GetPixelRGB  (sal_uInt32 nRow, sal_uInt32 nColumn) const override;
        virtual sal_uInt8   GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const override;
        virtual sal_uInt8   GetPixelIdx  (sal_uInt32 nRow, sal_uInt32 nColumn) const override;
        virtual sal_uInt32  GetDepth () const override;
};

SalPrinterBmp::SalPrinterBmp (BitmapBuffer* pBuffer)
    : mpBmpBuffer(pBuffer)
{
    assert(mpBmpBuffer && "SalPrinterBmp::SalPrinterBmp () can't acquire Bitmap");

    // calibrate scanline buffer
    if( mpBmpBuffer->mnFormat & ScanlineFormat::TopDown )
    {
        mpScanAccess = mpBmpBuffer->mpBits;
        mnScanOffset = mpBmpBuffer->mnScanlineSize;
    }
    else
    {
        mpScanAccess = mpBmpBuffer->mpBits
                       + (mpBmpBuffer->mnHeight - 1) * mpBmpBuffer->mnScanlineSize;
        mnScanOffset = - mpBmpBuffer->mnScanlineSize;
    }

    // request read access to the pixels
    switch( RemoveScanline( mpBmpBuffer->mnFormat ) )
    {
        case ScanlineFormat::N1BitMsbPal:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN1BitMsbPal;  break;
        case ScanlineFormat::N1BitLsbPal:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN1BitLsbPal;  break;
        case ScanlineFormat::N4BitMsnPal:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN4BitMsnPal;  break;
        case ScanlineFormat::N4BitLsnPal:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN4BitLsnPal;  break;
        case ScanlineFormat::N8BitPal:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN8BitPal;      break;
        case ScanlineFormat::N8BitTcMask:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN8BitTcMask;  break;
        case ScanlineFormat::N16BitTcMsbMask:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN16BitTcMsbMask; break;
        case ScanlineFormat::N16BitTcLsbMask:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN16BitTcLsbMask; break;
        case ScanlineFormat::N24BitTcBgr:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN24BitTcBgr;  break;
        case ScanlineFormat::N24BitTcRgb:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN24BitTcRgb;  break;
        case ScanlineFormat::N32BitTcAbgr:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN32BitTcAbgr; break;
        case ScanlineFormat::N32BitTcArgb:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN32BitTcArgb; break;
        case ScanlineFormat::N32BitTcBgra:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN32BitTcBgra; break;
        case ScanlineFormat::N32BitTcRgba:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN32BitTcRgba; break;
        case ScanlineFormat::N32BitTcMask:
            mpFncGetPixel = BitmapReadAccess::GetPixelForN32BitTcMask; break;

        default:
            OSL_FAIL("Error: SalPrinterBmp::SalPrinterBmp() unknown bitmap format");
            mpFncGetPixel = nullptr;
        break;
    }
}

sal_uInt32
SalPrinterBmp::GetDepth () const
{
    sal_uInt32 nDepth;

    switch (mpBmpBuffer->mnBitCount)
    {
        case 1:
            nDepth = 1;
            break;

        case 4:
        case 8:
            nDepth = 8;
            break;

        case 16:
        case 24:
        case 32:
            nDepth = 24;
            break;

        default:
            nDepth = 1;
            OSL_FAIL("Error: unsupported bitmap depth in SalPrinterBmp::GetDepth()");
            break;
    }

    return nDepth;
}

sal_uInt32
SalPrinterBmp::ColorOf (BitmapColor& rColor) const
{
    if (rColor.IsIndex())
        return ColorOf (mpBmpBuffer->maPalette[rColor.GetIndex()]);
    else
        return    ((rColor.GetBlue())        & 0x000000ff)
                | ((rColor.GetGreen() <<  8) & 0x0000ff00)
                | ((rColor.GetRed()   << 16) & 0x00ff0000);
}

sal_uInt8
SalPrinterBmp::GrayOf (BitmapColor& rColor) const
{
    if (rColor.IsIndex())
        return GrayOf (mpBmpBuffer->maPalette[rColor.GetIndex()]);
    else
        return (  rColor.GetBlue()  *  28UL
                + rColor.GetGreen() * 151UL
                + rColor.GetRed()   *  77UL ) >> 8;
}

sal_uInt32
SalPrinterBmp::GetPaletteEntryCount () const
{
    return mpBmpBuffer->maPalette.GetEntryCount ();
}

sal_uInt32
SalPrinterBmp::GetPaletteColor (sal_uInt32 nIdx) const
{
    return ColorOf (mpBmpBuffer->maPalette[nIdx]);
}

sal_uInt32
SalPrinterBmp::GetPixelRGB (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel (pScan, nColumn, mpBmpBuffer->maColorMask);

    return ColorOf (aColor);
}

sal_uInt8
SalPrinterBmp::GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel (pScan, nColumn, mpBmpBuffer->maColorMask);

    return GrayOf (aColor);
}

sal_uInt8
SalPrinterBmp::GetPixelIdx (sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel (pScan, nColumn, mpBmpBuffer->maColorMask);

    if (aColor.IsIndex())
        return aColor.GetIndex();
    else
        return 0;
}

/*******************************************************
 * GenPspGraphics                                         *
 *******************************************************/

GenPspGraphics::GenPspGraphics()
    : m_pJobData( nullptr ),
      m_pPrinterGfx( nullptr ),
      m_bFontVertical( false ),
      m_pInfoPrinter( nullptr )
{
    for(FreetypeFont* & rp : m_pFreetypeFont)
        rp = nullptr;
}

void GenPspGraphics::Init(psp::JobData* pJob, psp::PrinterGfx* pGfx,
                           SalInfoPrinter* pInfoPrinter)
{
    m_pJobData = pJob;
    m_pPrinterGfx = pGfx;
    m_pInfoPrinter = pInfoPrinter;
    SetLayout( SalLayoutFlags::NONE );
}

GenPspGraphics::~GenPspGraphics()
{
    ReleaseFonts();
}

void GenPspGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY )
{
    if (m_pJobData != nullptr)
    {
        int x = m_pJobData->m_aContext.getRenderResolution();

        rDPIX = x;
        rDPIY = x;
    }
}

sal_uInt16 GenPspGraphics::GetBitCount() const
{
    return m_pPrinterGfx->GetBitCount();
}

long GenPspGraphics::GetGraphicsWidth() const
{
    return 0;
}

void GenPspGraphics::ResetClipRegion()
{
    m_pPrinterGfx->ResetClipRegion();
}

bool GenPspGraphics::setClipRegion( const vcl::Region& i_rClip )
{
    // TODO: support polygonal clipregions here
    RectangleVector aRectangles;
    i_rClip.GetRegionRectangles(aRectangles);
    m_pPrinterGfx->BeginSetClipRegion(aRectangles.size());

    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
    {
        const long nW(aRectIter->GetWidth());

        if(nW)
        {
            const long nH(aRectIter->GetHeight());

            if(nH)
            {
                m_pPrinterGfx->UnionClipRegion(
                    aRectIter->Left(),
                    aRectIter->Top(),
                    nW,
                    nH);
            }
        }
    }

    m_pPrinterGfx->EndSetClipRegion();

    //m_pPrinterGfx->BeginSetClipRegion( i_rClip.GetRectCount() );

    //ImplRegionInfo aInfo;
    //long nX, nY, nW, nH;
    //bool bRegionRect = i_rClip.ImplGetFirstRect(aInfo, nX, nY, nW, nH );
    //while( bRegionRect )
    //{
    //    if ( nW && nH )
    //    {
    //        m_pPrinterGfx->UnionClipRegion( nX, nY, nW, nH );
    //    }
    //    bRegionRect = i_rClip.ImplGetNextRect( aInfo, nX, nY, nW, nH );
    //}
    //m_pPrinterGfx->EndSetClipRegion();
    return true;
}

void GenPspGraphics::SetLineColor()
{
    m_pPrinterGfx->SetLineColor ();
}

void GenPspGraphics::SetLineColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetLineColor (aColor);
}

void GenPspGraphics::SetFillColor()
{
    m_pPrinterGfx->SetFillColor ();
}

void GenPspGraphics::SetFillColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetFillColor (aColor);
}

void GenPspGraphics::SetROPLineColor( SalROPColor )
{
    SAL_WARN( "vcl", "Error: PrinterGfx::SetROPLineColor() not implemented" );
}

void GenPspGraphics::SetROPFillColor( SalROPColor )
{
    SAL_WARN( "vcl", "Error: PrinterGfx::SetROPFillColor() not implemented" );
}

void GenPspGraphics::SetXORMode( bool bSet )
{
    (void)bSet;
    SAL_WARN_IF( bSet, "vcl", "Error: PrinterGfx::SetXORMode() not implemented" );
}

void GenPspGraphics::drawPixel( long nX, long nY )
{
    m_pPrinterGfx->DrawPixel (Point(nX, nY));
}

void GenPspGraphics::drawPixel( long nX, long nY, SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->DrawPixel (Point(nX, nY), aColor);
}

void GenPspGraphics::drawLine( long nX1, long nY1, long nX2, long nY2 )
{
    m_pPrinterGfx->DrawLine (Point(nX1, nY1), Point(nX2, nY2));
}

void GenPspGraphics::drawRect( long nX, long nY, long nDX, long nDY )
{
    m_pPrinterGfx->DrawRect (Rectangle(Point(nX, nY), Size(nDX, nDY)));
}

void GenPspGraphics::drawPolyLine( sal_uInt32 nPoints, const SalPoint *pPtAry )
{
    m_pPrinterGfx->DrawPolyLine (nPoints, reinterpret_cast<Point const *>(pPtAry));
}

void GenPspGraphics::drawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry )
{
    // Point must be equal to SalPoint! see include/vcl/salgtype.hxx
    m_pPrinterGfx->DrawPolygon (nPoints, reinterpret_cast<Point const *>(pPtAry));
}

void GenPspGraphics::drawPolyPolygon( sal_uInt32           nPoly,
                                   const sal_uInt32   *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    m_pPrinterGfx->DrawPolyPolygon (nPoly, pPoints, reinterpret_cast<const Point**>(pPtAry));
}

bool GenPspGraphics::drawPolyPolygon( const basegfx::B2DPolyPolygon&, double /*fTransparency*/ )
{
        // TODO: implement and advertise OutDevSupportType::B2DDraw support
        return false;
}

bool GenPspGraphics::drawPolyLine(
    const basegfx::B2DPolygon&,
    double /*fTranspareny*/,
    const basegfx::B2DVector& /*rLineWidths*/,
    basegfx::B2DLineJoin /*eJoin*/,
    css::drawing::LineCap /*eLineCap*/,
    double /*fMiterMinimumAngle*/)
{
    // TODO: a PS printer can draw B2DPolyLines almost directly
    return false;
}

bool GenPspGraphics::drawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    m_pPrinterGfx->DrawPolyLineBezier (nPoints, reinterpret_cast<Point const *>(pPtAry), pFlgAry);
    return true;
}

bool GenPspGraphics::drawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry )
{
    m_pPrinterGfx->DrawPolygonBezier (nPoints, reinterpret_cast<Point const *>(pPtAry), pFlgAry);
    return true;
}

bool GenPspGraphics::drawPolyPolygonBezier( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             const SalPoint* const* pPtAry,
                                             const PolyFlags* const* pFlgAry )
{
    // Point must be equal to SalPoint! see include/vcl/salgtype.hxx
    m_pPrinterGfx->DrawPolyPolygonBezier (nPoly, pPoints, reinterpret_cast<Point const * const *>(pPtAry), pFlgAry);
    return true;
}

void GenPspGraphics::invert( sal_uInt32,
                          const SalPoint*,
                          SalInvert )
{
    SAL_WARN( "vcl", "Error: PrinterGfx::Invert() not implemented" );
}

bool GenPspGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    return m_pPrinterGfx->DrawEPS( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ), pPtr, nSize );
}

void GenPspGraphics::copyBits( const SalTwoRect&,
                            SalGraphics* )
{
    OSL_FAIL( "Error: PrinterGfx::CopyBits() not implemented" );
}

void GenPspGraphics::copyArea ( long,long,long,long,long,long,bool )
{
    OSL_FAIL( "Error: PrinterGfx::CopyArea() not implemented" );
}

void GenPspGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    Rectangle aSrc (Point(rPosAry.mnSrcX, rPosAry.mnSrcY),
                    Size(rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));
    Rectangle aDst (Point(rPosAry.mnDestX, rPosAry.mnDestY),
                    Size(rPosAry.mnDestWidth, rPosAry.mnDestHeight));

    BitmapBuffer* pBuffer= const_cast<SalBitmap&>(rSalBitmap).AcquireBuffer(BitmapAccessMode::Read);

    SalPrinterBmp aBmp (pBuffer);
    m_pPrinterGfx->DrawBitmap (aDst, aSrc, aBmp);

    const_cast<SalBitmap&>(rSalBitmap).ReleaseBuffer (pBuffer, BitmapAccessMode::Read);
}

void GenPspGraphics::drawBitmap( const SalTwoRect&,
                              const SalBitmap&,
                              const SalBitmap& )
{
    OSL_FAIL("Error: no PrinterGfx::DrawBitmap() for transparent bitmap");
}

void GenPspGraphics::drawMask( const SalTwoRect&,
                            const SalBitmap &,
                            SalColor )
{
    OSL_FAIL("Error: PrinterGfx::DrawMask() not implemented");
}

SalBitmap* GenPspGraphics::getBitmap( long, long, long, long )
{
    SAL_INFO("vcl", "Warning: PrinterGfx::GetBitmap() not implemented");
    return nullptr;
}

SalColor GenPspGraphics::getPixel( long, long )
{
    OSL_FAIL("Warning: PrinterGfx::GetPixel() not implemented");
    return 0;
}

void GenPspGraphics::invert(long,long,long,long,SalInvert)
{
    OSL_FAIL("Warning: PrinterGfx::Invert() not implemented");
}

class ImplPspFontData : public PhysicalFontFace
{
private:
    sal_IntPtr              mnFontId;

public:
    explicit ImplPspFontData( const psp::FastPrintFontInfo& );
    virtual sal_IntPtr      GetFontId() const override { return mnFontId; }
    virtual PhysicalFontFace*   Clone() const override { return new ImplPspFontData( *this ); }
    virtual LogicalFontInstance*  CreateFontInstance( FontSelectPattern& ) const override;
};

ImplPspFontData::ImplPspFontData( const psp::FastPrintFontInfo& rInfo )
:   PhysicalFontFace( GenPspGraphics::Info2FontAttributes(rInfo) ),
    mnFontId( rInfo.m_nID )
{}

LogicalFontInstance* ImplPspFontData::CreateFontInstance( FontSelectPattern& rFSD ) const
{
    FreetypeFontInstance* pEntry = new FreetypeFontInstance( rFSD );
    return pEntry;
}

class PspCommonSalLayout : public CommonSalLayout
{
public:
    PspCommonSalLayout(psp::PrinterGfx&, FreetypeFont& rFont);

    virtual void        InitFont() const override;

private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    sal_IntPtr          mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
};

PspCommonSalLayout::PspCommonSalLayout(::psp::PrinterGfx& rGfx, FreetypeFont& rFont)
:   CommonSalLayout(rFont)
,   mrPrinterGfx(rGfx)
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
}

void PspCommonSalLayout::InitFont() const
{
    CommonSalLayout::InitFont();
    mrPrinterGfx.SetFont(mnFontID, mnFontHeight, mnFontWidth,
                         mnOrientation, mbVertical, mbArtItalic, mbArtBold);
}

void GenPspGraphics::DrawTextLayout(const CommonSalLayout& rLayout)
{
    const GlyphItem* pGlyph;
    Point aPos;
    int nStart = 0;
    while (rLayout.GetNextGlyphs(1, &pGlyph, aPos, nStart))
    {
        sal_Int32 nAdvance = pGlyph->mnNewWidth / rLayout.GetUnitsPerPixel();
        m_pPrinterGfx->DrawGlyph(aPos, *pGlyph, nAdvance);
    }
}

const FontCharMapRef GenPspGraphics::GetFontCharMap() const
{
    if( !m_pFreetypeFont[0] )
        return nullptr;

    const FontCharMapRef xFCMap = m_pFreetypeFont[0]->GetFontCharMap();
    return xFCMap;
}

bool GenPspGraphics::GetFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!m_pFreetypeFont[0])
        return false;
    return m_pFreetypeFont[0]->GetFontCapabilities(rFontCapabilities);
}

void GenPspGraphics::SetFont( FontSelectPattern *pEntry, int nFallbackLevel )
{
    // release all fonts that are to be overridden
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( m_pFreetypeFont[i] != nullptr )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *m_pFreetypeFont[i] );
            m_pFreetypeFont[i] = nullptr;
        }
    }

    // return early if there is no new font
    if( !pEntry )
        return;

    sal_IntPtr nID = pEntry->mpFontData ? pEntry->mpFontData->GetFontId() : 0;

    // determine which font attributes need to be emulated
    bool bArtItalic = false;
    bool bArtBold = false;
    if( pEntry->GetItalic() == ITALIC_OBLIQUE || pEntry->GetItalic() == ITALIC_NORMAL )
    {
        FontItalic eItalic = m_pPrinterGfx->GetFontMgr().getFontItalic( nID );
        if( eItalic != ITALIC_NORMAL && eItalic != ITALIC_OBLIQUE )
            bArtItalic = true;
    }
    int nWeight = (int)pEntry->GetWeight();
    int nRealWeight = (int)m_pPrinterGfx->GetFontMgr().getFontWeight( nID );
    if( nRealWeight <= (int)WEIGHT_MEDIUM && nWeight > (int)WEIGHT_MEDIUM )
    {
        bArtBold = true;
    }

    // also set the serverside font for layouting
    m_bFontVertical = pEntry->mbVertical;
    if( pEntry->mpFontData )
    {
        // requesting a font provided by builtin rasterizer
        FreetypeFont* pFreetypeFont = GlyphCache::GetInstance().CacheFont( *pEntry );
        if( pFreetypeFont != nullptr )
        {
            if( pFreetypeFont->TestFont() )
                m_pFreetypeFont[ nFallbackLevel ] = pFreetypeFont;
            else
                GlyphCache::GetInstance().UncacheFont( *pFreetypeFont );
        }
    }

    // set the printer font
    m_pPrinterGfx->SetFont( nID,
                            pEntry->mnHeight,
                            pEntry->mnWidth,
                            pEntry->mnOrientation,
                            pEntry->mbVertical,
                            bArtItalic,
                            bArtBold
                            );
}

void GenPspGraphics::SetTextColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetTextColor (aColor);
}

bool GenPspGraphics::AddTempDevFont( PhysicalFontCollection*, const OUString&,const OUString& )
{
    return false;
}

bool GenPspGraphics::AddTempDevFontHelper( PhysicalFontCollection* pFontCollection,
                                           const OUString& rFileURL,
                                           const OUString& rFontName,
                                           GlyphCache &rGC )
{
    // inform PSP font manager
    OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFileURL, aUSystemPath ) );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aOFileName( OUStringToOString( aUSystemPath, aEncoding ) );
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    std::vector<psp::fontID> aFontIds = rMgr.addFontFile( aOFileName );
    if( aFontIds.empty() )
        return false;

    for (std::vector<psp::fontID>::iterator aI = aFontIds.begin(), aEnd = aFontIds.end(); aI != aEnd; ++aI)
    {
        // prepare font data
        psp::FastPrintFontInfo aInfo;
        rMgr.getFontFastInfo( *aI, aInfo );
        aInfo.m_aFamilyName = rFontName;

        // inform glyph cache of new font
        FontAttributes aDFA = GenPspGraphics::Info2FontAttributes( aInfo );
        aDFA.IncreaseQualityBy( 5800 );

        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );

        const OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );
    }

    // announce new font to device's font list
    rGC.AnnounceFonts( pFontCollection );
    return true;
}

void GenPspGraphics::GetDevFontList( PhysicalFontCollection *pFontCollection )
{
    ::std::list< psp::fontID > aList;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aList );

    ::std::list< psp::fontID >::iterator it;
    psp::FastPrintFontInfo aInfo;
    for (it = aList.begin(); it != aList.end(); ++it)
        if (rMgr.getFontFastInfo (*it, aInfo))
            AnnounceFonts( pFontCollection, aInfo );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pFontCollection );
}

void GenPspGraphics::ClearDevFontCache()
{
    GlyphCache::GetInstance().ClearFontCache();
}

void GenPspGraphics::GetFontMetric(ImplFontMetricDataRef& rxFontMetric, int nFallbackLevel)
{
    if (nFallbackLevel >= MAX_FALLBACK)
        return;

    if (m_pFreetypeFont[nFallbackLevel])
        m_pFreetypeFont[nFallbackLevel]->GetFontMetric(rxFontMetric);
}

bool GenPspGraphics::GetGlyphBoundRect(const GlyphItem& rGlyph, Rectangle& rRect)
{
    const int nLevel = rGlyph.mnFallbackLevel;
    if( nLevel >= MAX_FALLBACK )
        return false;

    FreetypeFont* pSF = m_pFreetypeFont[ nLevel ];
    if( !pSF )
        return false;

    rRect = pSF->GetGlyphBoundRect(rGlyph);
    return true;
}

bool GenPspGraphics::GetGlyphOutline(const GlyphItem& rGlyph,
    basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    const int nLevel = rGlyph.mnFallbackLevel;
    if( nLevel >= MAX_FALLBACK )
        return false;

    FreetypeFont* pSF = m_pFreetypeFont[ nLevel ];
    if( !pSF )
        return false;

    if (pSF->GetGlyphOutline(rGlyph, rB2DPolyPoly))
        return true;

    return false;
}

SalLayout* GenPspGraphics::GetTextLayout(ImplLayoutArgs& /*rArgs*/, int nFallbackLevel)
{
    if (m_pFreetypeFont[nFallbackLevel])
        return new PspCommonSalLayout(*m_pPrinterGfx, *m_pFreetypeFont[nFallbackLevel]);

    return nullptr;
}

bool GenPspGraphics::CreateFontSubset(
                                   const OUString& rToFile,
                                   const PhysicalFontFace* pFont,
                                   const sal_GlyphId* pGlyphIds,
                                   const sal_uInt8* pEncoding,
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

void GenPspGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
                                  bool bVertical,
                                  std::vector< sal_Int32 >& rWidths,
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

void GenPspGraphics::DoGetGlyphWidths( psp::fontID aFont,
                                    bool bVertical,
                                    std::vector< sal_Int32 >& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}

FontAttributes GenPspGraphics::Info2FontAttributes( const psp::FastPrintFontInfo& rInfo )
{
    FontAttributes aDFA;
    aDFA.SetFamilyName( rInfo.m_aFamilyName );
    aDFA.SetStyleName( rInfo.m_aStyleName );
    aDFA.SetFamilyType( rInfo.m_eFamilyStyle );
    aDFA.SetWeight( rInfo.m_eWeight );
    aDFA.SetItalic( rInfo.m_eItalic );
    aDFA.SetWidthType( rInfo.m_eWidth );
    aDFA.SetPitch( rInfo.m_ePitch );
    aDFA.SetSymbolFlag( (rInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL) );
    aDFA.SetQuality(512);

    // add font family name aliases
    ::std::list< OUString >::const_iterator it = rInfo.m_aAliases.begin();
    for(; it != rInfo.m_aAliases.end(); ++it )
        aDFA.AddMapName( *it );

#if OSL_DEBUG_LEVEL > 2
    if( aDFA.HasMapNames() )
    {
        OString aOrigName(OUStringToOString(aDFA.GetFamilyName(), osl_getThreadTextEncoding()));
        OString aAliasNames(OUStringToOString(aDFA.GetAliasNames(), osl_getThreadTextEncoding()));
        SAL_INFO( "vcl.fonts", "using alias names " << aAliasNames.getStr() << " for font family " << aOrigName.getStr() );
    }
#endif

    return aDFA;
}

namespace vcl
{
    const char* getLangBoost()
    {
        const char* pLangBoost;
        const LanguageType eLang = Application::GetSettings().GetUILanguageTag().getLanguageType();
        if (eLang == LANGUAGE_JAPANESE)
            pLangBoost = "jan";
        else if (MsLangId::isKorean(eLang))
            pLangBoost = "kor";
        else if (MsLangId::isSimplifiedChinese(eLang))
            pLangBoost = "zhs";
        else if (MsLangId::isTraditionalChinese(eLang))
            pLangBoost = "zht";
        else
            pLangBoost = nullptr;
        return pLangBoost;
    }
}

void GenPspGraphics::AnnounceFonts( PhysicalFontCollection* pFontCollection, const psp::FastPrintFontInfo& aInfo )
{
    int nQuality = 0;

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    OString aFileName( rMgr.getFontFileSysPath( aInfo.m_nID ) );
    int nPos = aFileName.lastIndexOf( '_' );
    if( nPos == -1 || aFileName[nPos+1] == '.' )
        nQuality += 5;
    else
    {
        static const char* pLangBoost = nullptr;
        static bool bOnce = true;
        if( bOnce )
        {
            bOnce = false;
            pLangBoost = vcl::getLangBoost();
        }

        if( pLangBoost )
            if( aFileName.copy( nPos+1, 3 ).equalsIgnoreAsciiCase( pLangBoost ) )
                nQuality += 10;
    }

    ImplPspFontData* pFD = new ImplPspFontData( aInfo );
    pFD->IncreaseQualityBy( nQuality );
    pFontCollection->Add( pFD );
}

bool GenPspGraphics::blendBitmap( const SalTwoRect&, const SalBitmap& )
{
    return false;
}

bool GenPspGraphics::blendAlphaBitmap( const SalTwoRect&, const SalBitmap&, const SalBitmap&, const SalBitmap& )
{
    return false;
}

bool GenPspGraphics::drawAlphaBitmap( const SalTwoRect&,
                                   const SalBitmap&,
                                   const SalBitmap& )
{
    return false;
}

bool GenPspGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap)
{
    // here direct support for transformed bitmaps can be implemented
    (void)rNull; (void)rX; (void)rY; (void)rSourceBitmap; (void)pAlphaBitmap;
    return false;
}

bool GenPspGraphics::drawAlphaRect( long, long, long, long, sal_uInt8 )
{
    return false;
}

SystemGraphicsData GenPspGraphics::GetGraphicsData() const
{
    return SystemGraphicsData();
}

#if ENABLE_CAIRO_CANVAS

bool GenPspGraphics::SupportsCairo() const
{
    return false;
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateSurface(const cairo::CairoSurfaceSharedPtr& /*rSurface*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateSurface(const OutputDevice& /*rRefDevice*/, int /*x*/, int /*y*/, int /*width*/, int /*height*/) const
{
    return cairo::SurfaceSharedPtr();
}

cairo::SurfaceSharedPtr GenPspGraphics::CreateBitmapSurface(const OutputDevice& /*rRefDevice*/, const BitmapSystemData& /*rData*/, const Size& /*rSize*/) const
{
    return cairo::SurfaceSharedPtr();
}

css::uno::Any GenPspGraphics::GetNativeSurfaceHandle(cairo::SurfaceSharedPtr& /*rSurface*/, const basegfx::B2ISize& /*rSize*/) const
{
    return css::uno::Any();
}

SystemFontData GenPspGraphics::GetSysFontData( int /* nFallbacklevel */ ) const
{
    return SystemFontData();
}

#endif // ENABLE_CAIRO_CANVAS

bool GenPspGraphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}

void GenPspGraphics::DoFreeEmbedFontData( const void* pData, long nLen )
{
    if( pData )
        munmap( const_cast<void *>(pData), nLen );
}

const void* GenPspGraphics::DoGetEmbedFontData(psp::fontID aFont, long* pDataLen)
{

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    OString aSysPath = rMgr.getFontFileSysPath( aFont );

    int fd = open( aSysPath.getStr(), O_RDONLY );
    if( fd < 0 )
        return nullptr;
    struct stat aStat;
    if( fstat( fd, &aStat ) )
    {
        close( fd );
        return nullptr;
    }
    void* pFile = mmap( nullptr, aStat.st_size, PROT_READ, MAP_SHARED, fd, 0 );
    close( fd );
    if( pFile == MAP_FAILED )
        return nullptr;
    *pDataLen = aStat.st_size;

    return pFile;
}

void GenPspGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    DoFreeEmbedFontData( pData, nLen );
}

const void* GenPspGraphics::GetEmbedFontData(const PhysicalFontFace* pFont, long* pDataLen)
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return DoGetEmbedFontData(aFont, pDataLen);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
