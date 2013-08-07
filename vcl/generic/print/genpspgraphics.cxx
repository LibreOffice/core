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

// for mmap etc.
#if defined( UNX )
#  include <stdlib.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <sys/mman.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

#include "generic/geninst.h"
#include "generic/genpspgraphics.h"
#include "generic/glyphcache.hxx"

#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/svapp.hxx"
#include "vcl/sysdata.hxx"

#include "generic/printergfx.hxx"
#include "salbmp.hxx"
#include "impfont.hxx"
#include "outfont.hxx"
#include "fontsubset.hxx"
#include "salprn.hxx"
#include "langboost.hxx"

#include <config_graphite.h>
#if ENABLE_GRAPHITE
#include <graphite_layout.hxx>
#include <graphite_serverfont.hxx>
#endif

#include <comphelper/string.hxx>
#include <i18nlangtag/mslangid.hxx>

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

    SalPrinterBmp ();

    public:

                            SalPrinterBmp (BitmapBuffer* pBitmap);
        virtual             ~SalPrinterBmp ();
        virtual sal_uInt32  GetPaletteColor (sal_uInt32 nIdx) const;
        virtual sal_uInt32  GetPaletteEntryCount () const;
        virtual sal_uInt32  GetPixelRGB  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelGray (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt8   GetPixelIdx  (sal_uInt32 nRow, sal_uInt32 nColumn) const;
        virtual sal_uInt32  GetWidth () const;
        virtual sal_uInt32  GetHeight() const;
        virtual sal_uInt32  GetDepth () const;
};

SalPrinterBmp::SalPrinterBmp (BitmapBuffer* pBuffer) :
        mpBmpBuffer (pBuffer)
{
    DBG_ASSERT (mpBmpBuffer, "SalPrinterBmp::SalPrinterBmp () can't acquire Bitmap");

    // calibrate scanline buffer
    if( BMP_SCANLINE_ADJUSTMENT( mpBmpBuffer->mnFormat ) == BMP_FORMAT_TOP_DOWN )
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
    switch( BMP_SCANLINE_FORMAT( mpBmpBuffer->mnFormat ) )
    {
        case BMP_FORMAT_1BIT_MSB_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_1BIT_MSB_PAL;  break;
        case BMP_FORMAT_1BIT_LSB_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_1BIT_LSB_PAL;  break;
        case BMP_FORMAT_4BIT_MSN_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_4BIT_MSN_PAL;  break;
        case BMP_FORMAT_4BIT_LSN_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_4BIT_LSN_PAL;  break;
        case BMP_FORMAT_8BIT_PAL:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_8BIT_PAL;      break;
        case BMP_FORMAT_8BIT_TC_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_8BIT_TC_MASK;  break;
        case BMP_FORMAT_16BIT_TC_MSB_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_16BIT_TC_MSB_MASK; break;
        case BMP_FORMAT_16BIT_TC_LSB_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_16BIT_TC_LSB_MASK; break;
        case BMP_FORMAT_24BIT_TC_BGR:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_24BIT_TC_BGR;  break;
        case BMP_FORMAT_24BIT_TC_RGB:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_24BIT_TC_RGB;  break;
        case BMP_FORMAT_24BIT_TC_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_24BIT_TC_MASK; break;
        case BMP_FORMAT_32BIT_TC_ABGR:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_ABGR; break;
        case BMP_FORMAT_32BIT_TC_ARGB:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_ARGB; break;
        case BMP_FORMAT_32BIT_TC_BGRA:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_BGRA; break;
        case BMP_FORMAT_32BIT_TC_RGBA:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_RGBA; break;
        case BMP_FORMAT_32BIT_TC_MASK:
            mpFncGetPixel = BitmapReadAccess::GetPixelFor_32BIT_TC_MASK; break;

        default:
            OSL_FAIL("Error: SalPrinterBmp::SalPrinterBmp() unknown bitmap format");
        break;
    }
}

SalPrinterBmp::~SalPrinterBmp ()
{
}

sal_uInt32
SalPrinterBmp::GetWidth () const
{
    return mpBmpBuffer->mnWidth;
}

sal_uInt32
SalPrinterBmp::GetHeight () const
{
    return mpBmpBuffer->mnHeight;
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
    : m_pJobData( NULL ),
      m_pPrinterGfx( NULL ),
      m_pPhoneNr( NULL ),
      m_bSwallowFaxNo( false ),
      m_bPhoneCollectionActive( false ),
      m_bFontVertical( false ),
      m_pInfoPrinter( NULL )
{
    for( int i = 0; i < MAX_FALLBACK; i++ )
        m_pServerFont[i] = NULL;
}

void GenPspGraphics::Init( psp::JobData* pJob, psp::PrinterGfx* pGfx,
                           OUString* pPhone, bool bSwallow,
                           SalInfoPrinter* pInfoPrinter )
{
    m_pJobData = pJob;
    m_pPrinterGfx = pGfx;
    m_pPhoneNr = pPhone;
    m_bSwallowFaxNo = bSwallow;
    m_pInfoPrinter = pInfoPrinter;
    SetLayout( 0 );
}

GenPspGraphics::~GenPspGraphics()
{
    ReleaseFonts();
}

void GenPspGraphics::GetResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY )
{
    if (m_pJobData != NULL)
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

bool GenPspGraphics::setClipRegion( const Region& i_rClip )
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
    //
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
    DBG_ASSERT( 0, "Error: PrinterGfx::SetROPLineColor() not implemented" );
}

void GenPspGraphics::SetROPFillColor( SalROPColor )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::SetROPFillColor() not implemented" );
}

void GenPspGraphics::SetXORMode( bool bSet, bool )
{
    (void)bSet;
    DBG_ASSERT( !bSet, "Error: PrinterGfx::SetXORMode() not implemented" );
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

void GenPspGraphics::drawPolyLine( sal_uLong nPoints, const SalPoint *pPtAry )
{
    m_pPrinterGfx->DrawPolyLine (nPoints, (Point*)pPtAry);
}

void GenPspGraphics::drawPolygon( sal_uLong nPoints, const SalPoint* pPtAry )
{
    // Point must be equal to SalPoint! see vcl/inc/salgtype.hxx
    m_pPrinterGfx->DrawPolygon (nPoints, (Point*)pPtAry);
}

void GenPspGraphics::drawPolyPolygon( sal_uInt32           nPoly,
                                   const sal_uInt32   *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    m_pPrinterGfx->DrawPolyPolygon (nPoly, pPoints, (const Point**)pPtAry);
}

bool GenPspGraphics::drawPolyPolygon( const ::basegfx::B2DPolyPolygon&, double /*fTransparency*/ )
{
        // TODO: implement and advertise OutDevSupport_B2DDraw support
        return false;
}

bool GenPspGraphics::drawPolyLine(
    const basegfx::B2DPolygon&,
    double /*fTranspareny*/,
    const basegfx::B2DVector& /*rLineWidths*/,
    basegfx::B2DLineJoin /*eJoin*/,
    com::sun::star::drawing::LineCap /*eLineCap*/)
{
    // TODO: a PS printer can draw B2DPolyLines almost directly
    return false;
}

sal_Bool GenPspGraphics::drawPolyLineBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    m_pPrinterGfx->DrawPolyLineBezier (nPoints, (Point*)pPtAry, pFlgAry);
    return sal_True;
}

sal_Bool GenPspGraphics::drawPolygonBezier( sal_uLong nPoints, const SalPoint* pPtAry, const sal_uInt8* pFlgAry )
{
    m_pPrinterGfx->DrawPolygonBezier (nPoints, (Point*)pPtAry, pFlgAry);
    return sal_True;
}

sal_Bool GenPspGraphics::drawPolyPolygonBezier( sal_uInt32 nPoly,
                                             const sal_uInt32* pPoints,
                                             const SalPoint* const* pPtAry,
                                             const sal_uInt8* const* pFlgAry )
{
    // Point must be equal to SalPoint! see vcl/inc/salgtype.hxx
    m_pPrinterGfx->DrawPolyPolygonBezier (nPoly, pPoints, (Point**)pPtAry, (sal_uInt8**)pFlgAry);
    return sal_True;
}

void GenPspGraphics::invert( sal_uLong,
                          const SalPoint*,
                          SalInvert )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::Invert() not implemented" );
}
sal_Bool GenPspGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize )
{
    return m_pPrinterGfx->DrawEPS( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ), pPtr, nSize );
}

void GenPspGraphics::copyBits( const SalTwoRect&,
                            SalGraphics* )
{
    OSL_FAIL( "Error: PrinterGfx::CopyBits() not implemented" );
}

void GenPspGraphics::copyArea ( long,long,long,long,long,long,sal_uInt16 )
{
    OSL_FAIL( "Error: PrinterGfx::CopyArea() not implemented" );
}

void GenPspGraphics::drawBitmap( const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap )
{
    Rectangle aSrc (Point(rPosAry.mnSrcX, rPosAry.mnSrcY),
                    Size(rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));
    Rectangle aDst (Point(rPosAry.mnDestX, rPosAry.mnDestY),
                    Size(rPosAry.mnDestWidth, rPosAry.mnDestHeight));

    BitmapBuffer* pBuffer= const_cast<SalBitmap&>(rSalBitmap).AcquireBuffer(sal_True);

    SalPrinterBmp aBmp (pBuffer);
    m_pPrinterGfx->DrawBitmap (aDst, aSrc, aBmp);

    const_cast<SalBitmap&>(rSalBitmap).ReleaseBuffer (pBuffer, sal_True);
}

void GenPspGraphics::drawBitmap( const SalTwoRect&,
                              const SalBitmap&,
                              const SalBitmap& )
{
    OSL_FAIL("Error: no PrinterGfx::DrawBitmap() for transparent bitmap");
}

void GenPspGraphics::drawBitmap( const SalTwoRect&,
                              const SalBitmap&,
                              SalColor )
{
    OSL_FAIL("Error: no PrinterGfx::DrawBitmap() for transparent color");
}

void GenPspGraphics::drawMask( const SalTwoRect&,
                            const SalBitmap &,
                            SalColor )
{
    OSL_FAIL("Error: PrinterGfx::DrawMask() not implemented");
}

SalBitmap* GenPspGraphics::getBitmap( long, long, long, long )
{
    DBG_WARNING ("Warning: PrinterGfx::GetBitmap() not implemented");
    return NULL;
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

//==========================================================================

class ImplPspFontData : public PhysicalFontFace
{
private:
    enum { PSPFD_MAGIC = 0xb5bf01f0 };
    sal_IntPtr              mnFontId;

public:
                            ImplPspFontData( const psp::FastPrintFontInfo& );
    virtual sal_IntPtr      GetFontId() const { return mnFontId; }
    virtual PhysicalFontFace*   Clone() const { return new ImplPspFontData( *this ); }
    virtual ImplFontEntry*  CreateFontInstance( FontSelectPattern& ) const;
    static bool             CheckFontData( const PhysicalFontFace& r ) { return r.CheckMagic( PSPFD_MAGIC ); }
};

//--------------------------------------------------------------------------

ImplPspFontData::ImplPspFontData( const psp::FastPrintFontInfo& rInfo )
:   PhysicalFontFace( GenPspGraphics::Info2DevFontAttributes(rInfo), PSPFD_MAGIC ),
    mnFontId( rInfo.m_nID )
{}

//--------------------------------------------------------------------------

ImplFontEntry* ImplPspFontData::CreateFontInstance( FontSelectPattern& rFSD ) const
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

    Reserve(rArgs.mnLength);

    for(;;)
    {
        bool bRightToLeft;
        if( !rArgs.GetNextPos( &nCharPos, &bRightToLeft ) )
            break;

        sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
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
    OUString       maText;
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
    sal_uInt32 aGlyphAry[ nMaxGlyphs ]; // TODO: use sal_GlyphId
    sal_Int32   aWidthAry[ nMaxGlyphs ];
    sal_Int32   aIdxAry  [ nMaxGlyphs ];
    sal_Unicode aUnicodes[ nMaxGlyphs ];
    int         aCharPosAry [ nMaxGlyphs ];

    Point aPos;
    long nUnitsPerPixel = rLayout.GetUnitsPerPixel();
    const sal_Unicode* pText = NULL;
    int nMinCharPos = 0;
    int nMaxCharPos = 0;
    if (bIsPspServerFontLayout)
    {
        const PspServerFontLayout * pPspLayout = dynamic_cast<const PspServerFontLayout*>(&rLayout);
#if ENABLE_GRAPHITE
        const GraphiteServerFontLayout * pGrLayout = dynamic_cast<const GraphiteServerFontLayout*>(&rLayout);
#endif
        if (pPspLayout)
        {
            pText = pPspLayout->getTextPtr();
            nMinCharPos = pPspLayout->getMinCharPos();
            nMaxCharPos = pPspLayout->getMaxCharPos();
        }
#if ENABLE_GRAPHITE
        else if (pGrLayout)
        {
        }
#endif
    }
    for( int nStart = 0;; )
    {
        int nGlyphCount = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart, aWidthAry, pText ? aCharPosAry : NULL );
        if( !nGlyphCount )
            break;

        sal_Int32 nXOffset = 0;
        for( int i = 0; i < nGlyphCount; ++i )
        {
            nXOffset += aWidthAry[ i ];
            aIdxAry[ i ] = nXOffset / nUnitsPerPixel;
            sal_Int32 nGlyphIdx = aGlyphAry[i] & (GF_IDXMASK | GF_ROTMASK);
            if( pText )
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

void GenPspGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    // print complex text
    DrawPrinterLayout( rLayout, *m_pPrinterGfx, true );
}

const ImplFontCharMap* GenPspGraphics::GetImplFontCharMap() const
{
    if( !m_pServerFont[0] )
        return NULL;

    const ImplFontCharMap* pIFCMap = m_pServerFont[0]->GetImplFontCharMap();
    return pIFCMap;
}

bool GenPspGraphics::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities) const
{
    if (!m_pServerFont[0])
        return false;
    return m_pServerFont[0]->GetFontCapabilities(rFontCapabilities);
}

sal_uInt16 GenPspGraphics::SetFont( FontSelectPattern *pEntry, int nFallbackLevel )
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
    if( pEntry->GetSlant() == ITALIC_OBLIQUE || pEntry->GetSlant() == ITALIC_NORMAL )
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

void GenPspGraphics::SetTextColor( SalColor nSalColor )
{
    psp::PrinterColor aColor (SALCOLOR_RED   (nSalColor),
                              SALCOLOR_GREEN (nSalColor),
                              SALCOLOR_BLUE  (nSalColor));
    m_pPrinterGfx->SetTextColor (aColor);
}

bool GenPspGraphics::AddTempDevFont( ImplDevFontList*, const OUString&,const OUString& )
{
    return false;
}

void GenPspGraphics::GetDevFontList( ImplDevFontList *pList )
{
    ::std::list< psp::fontID > aList;
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aList, m_pJobData->m_pParser );

    ::std::list< psp::fontID >::iterator it;
    psp::FastPrintFontInfo aInfo;
    for (it = aList.begin(); it != aList.end(); ++it)
        if (rMgr.getFontFastInfo (*it, aInfo))
            AnnounceFonts( pList, aInfo );

    // register platform specific font substitutions if available
    SalGenericInstance::RegisterFontSubstitutors( pList );
}

void GenPspGraphics::ClearDevFontCache()
{
    GlyphCache::GetInstance().ClearFontCache();
}

void GenPspGraphics::GetDevFontSubstList( OutputDevice* pOutDev )
{
    const psp::PrinterInfo& rInfo = psp::PrinterInfoManager::get().getPrinterInfo( m_pJobData->m_aPrinterName );
    if( rInfo.m_bPerformFontSubstitution )
    {
        for( boost::unordered_map< OUString, OUString, OUStringHash >::const_iterator it = rInfo.m_aFontSubstitutes.begin(); it != rInfo.m_aFontSubstitutes.end(); ++it )
            pOutDev->ImplAddDevFontSubstitute( it->first, it->second, FONT_SUBSTITUTE_ALWAYS );
    }
}

void GenPspGraphics::GetFontMetric( ImplFontMetricData *pMetric, int )
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

sal_uLong GenPspGraphics::GetKernPairs( sal_uLong nPairs, ImplKernPairData *pKernPairs )
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

sal_Bool GenPspGraphics::GetGlyphBoundRect( sal_GlyphId nGlyphIndex, Rectangle& rRect )
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

sal_Bool GenPspGraphics::GetGlyphOutline( sal_GlyphId nGlyphIndex,
    ::basegfx::B2DPolyPolygon& rB2DPolyPoly )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return sal_False;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return sal_False;

    nGlyphIndex &= GF_IDXMASK;
    if( pSF->GetGlyphOutline( nGlyphIndex, rB2DPolyPoly ) )
        return sal_True;

    return sal_False;
}

SalLayout* GenPspGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
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
    {
#if ENABLE_GRAPHITE
        // Is this a Graphite font?
        if (GraphiteServerFontLayout::IsGraphiteEnabledFont(*m_pServerFont[nFallbackLevel]))
        {
            pLayout = new GraphiteServerFontLayout(*m_pServerFont[nFallbackLevel]);
        }
        else
#endif
            pLayout = new PspServerFontLayout( *m_pPrinterGfx, *m_pServerFont[nFallbackLevel], rArgs );
    }
    else
        pLayout = new PspFontLayout( *m_pPrinterGfx );

    return pLayout;
}

//--------------------------------------------------------------------------

sal_Bool GenPspGraphics::CreateFontSubset(
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

//--------------------------------------------------------------------------

const Ucs2SIntMap* GenPspGraphics::GetFontEncodingVector( const PhysicalFontFace* pFont, const Ucs2OStrMap** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return GenPspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}

//--------------------------------------------------------------------------

void GenPspGraphics::GetGlyphWidths( const PhysicalFontFace* pFont,
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

const Ucs2SIntMap* GenPspGraphics::DoGetFontEncodingVector( fontID aFont, const Ucs2OStrMap** pNonEncoded )
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

void GenPspGraphics::DoGetGlyphWidths( psp::fontID aFont,
                                    bool bVertical,
                                    Int32Vector& rWidths,
                                    Ucs2UIntMap& rUnicodeEnc )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}
// ----------------------------------------------------------------------------

ImplDevFontAttributes GenPspGraphics::Info2DevFontAttributes( const psp::FastPrintFontInfo& rInfo )
{
    ImplDevFontAttributes aDFA;
    aDFA.SetFamilyName( rInfo.m_aFamilyName );
    aDFA.SetStyleName( rInfo.m_aStyleName );
    aDFA.SetFamilyType( rInfo.m_eFamilyStyle );
    aDFA.SetWeight( rInfo.m_eWeight );
    aDFA.SetItalic( rInfo.m_eItalic );
    aDFA.SetWidthType( rInfo.m_eWidth );
    aDFA.SetPitch( rInfo.m_ePitch );
    aDFA.SetSymbolFlag( (rInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL) );
    aDFA.mbSubsettable  = rInfo.m_bSubsettable;
    aDFA.mbEmbeddable   = rInfo.m_bEmbeddable;

    switch( rInfo.m_eType )
    {
        case psp::fonttype::Builtin:
            aDFA.mnQuality       = 1024;
            aDFA.mbDevice        = true;
            break;
        case psp::fonttype::TrueType:
            aDFA.mnQuality       = 512;
            aDFA.mbDevice        = false;
            break;
        case psp::fonttype::Type1:
            aDFA.mnQuality       = 0;
            aDFA.mbDevice        = false;
            break;
        default:
            aDFA.mnQuality       = 0;
            aDFA.mbDevice        = false;
            break;
    }

    aDFA.mbOrientation   = true;

    // add font family name aliases
    ::std::list< OUString >::const_iterator it = rInfo.m_aAliases.begin();
    bool bHasMapNames = false;
    for(; it != rInfo.m_aAliases.end(); ++it )
    {
        if( bHasMapNames )
            aDFA.maMapNames += OUString(';');
        aDFA.maMapNames += *it;
        bHasMapNames = true;
    }

#if OSL_DEBUG_LEVEL > 2
    if( bHasMapNames )
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
            pLangBoost = NULL;
        return pLangBoost;
    }
}

// -----------------------------------------------------------------------

void GenPspGraphics::AnnounceFonts( ImplDevFontList* pFontList, const psp::FastPrintFontInfo& aInfo )
{
    int nQuality = 0;

    if( aInfo.m_eType == psp::fonttype::TrueType )
    {
        // asian type 1 fonts are not known
        psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
        OString aFileName( rMgr.getFontFileSysPath( aInfo.m_nID ) );
        int nPos = aFileName.lastIndexOf( '_' );
        if( nPos == -1 || aFileName[nPos+1] == '.' )
            nQuality += 5;
        else
        {
            static const char* pLangBoost = NULL;
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
    }

    ImplPspFontData* pFD = new ImplPspFontData( aInfo );
    pFD->mnQuality += nQuality;
    pFontList->Add( pFD );
}

bool GenPspGraphics::filterText( const OUString& rOrig, OUString& rNewText, sal_Int32 nIndex, sal_Int32& rLen, sal_Int32& rCutStart, sal_Int32& rCutStop )
{
    if( ! m_pPhoneNr )
        return false;

    rNewText = rOrig;
    rCutStop = rCutStart = -1;

#define FAX_PHONE_TOKEN          "@@#"
#define FAX_PHONE_TOKEN_LENGTH   3
#define FAX_END_TOKEN            "@@"
#define FAX_END_TOKEN_LENGTH     2

    bool bRet = false;
    bool bStarted = false;
    sal_Int32 nPos;
    sal_Int32 nStart = 0;
    sal_Int32 nStop = rLen;
    OUString aPhone = rOrig.copy( nIndex, rLen );

    if( ! m_bPhoneCollectionActive )
    {
        if( ( nPos = aPhone.indexOfAsciiL( FAX_PHONE_TOKEN, FAX_PHONE_TOKEN_LENGTH ) ) != -1 )
        {
            nStart = nPos;
            m_bPhoneCollectionActive = true;
            m_aPhoneCollection = "";
            bRet = true;
            bStarted = true;
        }
    }
    if( m_bPhoneCollectionActive )
    {
        bool bStopped = false;
        bRet = true;
        nPos = bStarted ? nStart + FAX_PHONE_TOKEN_LENGTH : 0;
        if( ( nPos = aPhone.indexOfAsciiL( FAX_END_TOKEN, FAX_END_TOKEN_LENGTH, nPos ) ) != -1 )
        {
            m_bPhoneCollectionActive = false;
            nStop = nPos + FAX_END_TOKEN_LENGTH;
            bStopped = true;
        }
        int nTokenStart = nStart + (bStarted ? FAX_PHONE_TOKEN_LENGTH : 0);
        int nTokenStop = nStop - (bStopped ? FAX_END_TOKEN_LENGTH : 0);
        m_aPhoneCollection += aPhone.copy( nTokenStart, nTokenStop - nTokenStart );
        if( ! m_bPhoneCollectionActive )
        {
            OUStringBuffer aPhoneNr;
            aPhoneNr.append( "<Fax#>" );
            aPhoneNr.append( m_aPhoneCollection );
            aPhoneNr.append( "</Fax#>" );
            *m_pPhoneNr = aPhoneNr.makeStringAndClear();
            m_aPhoneCollection = "";
        }
    }
    if( m_aPhoneCollection.getLength() > 1024 )
    {
        m_bPhoneCollectionActive = false;
        m_aPhoneCollection = "";
        bRet = false;
    }

    if( bRet && m_bSwallowFaxNo )
    {
        rLen -= nStop - nStart;
        rCutStart = nStart+nIndex;
        rCutStop = nStop+nIndex;
        if (rCutStart != rCutStop)
            rNewText = ( rCutStart ? rOrig.copy( 0, rCutStart ) : OUString() ) + rOrig.copy( rCutStop );
    }

    return bRet && m_bSwallowFaxNo;
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
    // here direct support for transformed bitmaps can be impemented
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

SystemFontData GenPspGraphics::GetSysFontData( int /* nFallbacklevel */ ) const
{
    return SystemFontData();
}

bool GenPspGraphics::supportsOperation( OutDevSupportType ) const
{
    return false;
}

void GenPspGraphics::DoFreeEmbedFontData( const void* pData, long nLen )
{
#if defined( UNX )
    if( pData )
        munmap( (char*)pData, nLen );
#else
    (void)nLen;
    rtl_freeMemory( (void *)pData );
#endif
}

const void* GenPspGraphics::DoGetEmbedFontData( psp::fontID aFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
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

#if defined( UNX )
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
#else
    // FIXME: test me ! ...
    OUString aURL;
    if( !osl::File::getFileURLFromSystemPath( OStringToOUString( aSysPath, osl_getThreadTextEncoding() ), aURL ) )
        return NULL;
    osl::File aFile( aURL );
    if( aFile.open( osl_File_OpenFlag_Read | osl_File_OpenFlag_NoLock ) != osl::File::E_None )
        return NULL;

    osl::DirectoryItem aItem;
    osl::DirectoryItem::get( aURL, aItem );
    osl::FileStatus aFileStatus( osl_FileStatus_Mask_FileSize );
    aItem.getFileStatus( aFileStatus );

    void *pFile = rtl_allocateMemory( aFileStatus.getFileSize() );
    sal_uInt64 nRead = 0;
    aFile.read( pFile, aFileStatus.getFileSize(), nRead );
    *pDataLen = (long) nRead;
#endif

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
            DoFreeEmbedFontData( pFile, *pDataLen );
            return NULL;
    }

    return pFile;
}

void GenPspGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    DoFreeEmbedFontData( pData, nLen );
}

const void* GenPspGraphics::GetEmbedFontData( const PhysicalFontFace* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the PhysicalFontFace pFont
    psp::fontID aFont = pFont->GetFontId();
    return DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
