/*************************************************************************
 *
 *  $RCSfile: pspgraphics.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:37:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <pspgraphics.h>
#include <psprint/jobdata.hxx>
#include <psprint/printergfx.hxx>
#include <psprint/printerinfomanager.hxx>
#include <bmpacc.hxx>
#include <salbmp.hxx>
#include <glyphcache.hxx>
#include <impfont.hxx>
#include <outfont.hxx>
#include <svapp.hxx>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace psp;
using namespace rtl;

// ----- Implementation of PrinterBmp by means of SalBitmap/BitmapBuffer ---------------

class SalPrinterBmp : public psp::PrinterBmp
{
    private:
    BitmapBuffer*       mpBmpBuffer;

    FncGetPixel         mpFncGetPixel;
    Scanline            mpScanAccess;
    sal_Int32           mnScanOffset;

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
            DBG_ERROR("Error: SalPrinterBmp::SalPrinterBmp() unknown bitmap format");
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
            DBG_ERROR ("Error: unsupported bitmap depth in SalPrinterBmp::GetDepth()");
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
 * PspGraphics                                         *
 *******************************************************/

PspGraphics::~PspGraphics()
{
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

void PspGraphics::GetScreenFontResolution( sal_Int32 &rDPIX, sal_Int32 &rDPIY )
{
    m_pPrinterGfx->GetScreenFontResolution (rDPIX, rDPIY);
}

USHORT PspGraphics::GetBitCount()
{
    return m_pPrinterGfx->GetBitCount();
}

long PspGraphics::GetGraphicsWidth()
{
    return 0;
}

void PspGraphics::ResetClipRegion()
{
    m_pPrinterGfx->ResetClipRegion ();
}

void PspGraphics::BeginSetClipRegion( ULONG n )
{
    m_pPrinterGfx->BeginSetClipRegion(n);
}

BOOL PspGraphics::unionClipRegion( long nX, long nY, long nDX, long nDY )
{
    return (BOOL)m_pPrinterGfx->UnionClipRegion (nX, nY, nDX, nDY);
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

void PspGraphics::SetROPLineColor( SalROPColor nROPColor )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::SetROPLineColor() not implemented" );
}

void PspGraphics::SetROPFillColor( SalROPColor nROPColor )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::SetROPFillColor() not implemented" );
}

void PspGraphics::SetXORMode( BOOL bSet )
{
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

void PspGraphics::drawPolyLine( ULONG nPoints, const SalPoint *pPtAry )
{
    m_pPrinterGfx->DrawPolyLine (nPoints, (Point*)pPtAry);
}

void PspGraphics::drawPolygon( ULONG nPoints, const SalPoint* pPtAry )
{
    // Point must be equal to SalPoint! see vcl/inc/salgtype.hxx
    m_pPrinterGfx->DrawPolygon (nPoints, (Point*)pPtAry);
}

void PspGraphics::drawPolyPolygon( ULONG            nPoly,
                                   const ULONG     *pPoints,
                                   PCONSTSALPOINT  *pPtAry )
{
    m_pPrinterGfx->DrawPolyPolygon (nPoly, pPoints, (const Point**)pPtAry);
}

sal_Bool PspGraphics::drawPolyLineBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    m_pPrinterGfx->DrawPolyLineBezier (nPoints, (Point*)pPtAry, pFlgAry);
    return sal_True;
}

sal_Bool PspGraphics::drawPolygonBezier( ULONG nPoints, const SalPoint* pPtAry, const BYTE* pFlgAry )
{
    m_pPrinterGfx->DrawPolygonBezier (nPoints, (Point*)pPtAry, pFlgAry);
    return sal_True;
}

sal_Bool PspGraphics::drawPolyPolygonBezier( ULONG nPoly,
                                             const ULONG* pPoints,
                                             const SalPoint* const* pPtAry,
                                             const BYTE* const* pFlgAry )
{
    // Point must be equal to SalPoint! see vcl/inc/salgtype.hxx
    m_pPrinterGfx->DrawPolyPolygonBezier (nPoly, pPoints, (Point**)pPtAry, (BYTE**)pFlgAry);
    return sal_True;
}

void PspGraphics::invert( ULONG nPoints,
                          const SalPoint* pPtAry,
                          SalInvert nFlags )
{
    DBG_ASSERT( 0, "Error: PrinterGfx::Invert() not implemented" );
}
BOOL PspGraphics::drawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, ULONG nSize )
{
    return m_pPrinterGfx->DrawEPS( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ), pPtr, nSize );
}

void PspGraphics::copyBits( const SalTwoRect *pPosAry,
                            SalGraphics    *pSSrcGraphics )
{
    DBG_ERROR( "Error: PrinterGfx::CopyBits() not implemented" );
}

void PspGraphics::copyArea ( long nDestX,    long nDestY,
                             long nSrcX,     long nSrcY,
                             long nSrcWidth, long nSrcHeight,
                             USHORT nFlags )
{
    DBG_ERROR( "Error: PrinterGfx::CopyArea() not implemented" );
}

void PspGraphics::drawBitmap( const SalTwoRect* pPosAry, const SalBitmap& rSalBitmap )
{
    Rectangle aSrc (Point(pPosAry->mnSrcX, pPosAry->mnSrcY),
                    Size(pPosAry->mnSrcWidth, pPosAry->mnSrcHeight));
    Rectangle aDst (Point(pPosAry->mnDestX, pPosAry->mnDestY),
                    Size(pPosAry->mnDestWidth, pPosAry->mnDestHeight));

    BitmapBuffer* pBuffer= const_cast<SalBitmap&>(rSalBitmap).AcquireBuffer(sal_True);

    SalPrinterBmp aBmp (pBuffer);
    m_pPrinterGfx->DrawBitmap (aDst, aSrc, aBmp);

    const_cast<SalBitmap&>(rSalBitmap).ReleaseBuffer (pBuffer, sal_True);
}

void PspGraphics::drawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransBitmap )
{
    DBG_ERROR("Error: no PrinterGfx::DrawBitmap() for transparent bitmap");
}

void PspGraphics::drawBitmap( const SalTwoRect* pPosAry,
                              const SalBitmap& rSalBitmap,
                              SalColor nTransparentColor )
{
    DBG_ERROR("Error: no PrinterGfx::DrawBitmap() for transparent color");
}

void PspGraphics::drawMask( const SalTwoRect* pPosAry,
                            const SalBitmap &rSalBitmap,
                            SalColor nMaskColor )
{
    DBG_ERROR("Error: PrinterGfx::DrawMask() not implemented");
}

SalBitmap* PspGraphics::getBitmap( long nX, long nY, long nDX, long nDY )
{
    DBG_WARNING ("Warning: PrinterGfx::GetBitmap() not implemented");
    return NULL;
}

SalColor PspGraphics::getPixel( long nX, long nY )
{
    DBG_ERROR ("Warning: PrinterGfx::GetPixel() not implemented");
    return 0;
}

void PspGraphics::invert(
                         long       nX,
                         long       nY,
                         long       nDX,
                         long       nDY,
                         SalInvert  nFlags )
{
    DBG_ERROR ("Warning: PrinterGfx::Invert() not implemented");
}

/*
 *  returns:
 *  true: cut out positions rStart to rStop from output because fax number was swallowed
 *  false: do nothing
 */

String PspGraphics::FaxPhoneComment( const String& rOrig, xub_StrLen nIndex, xub_StrLen& rLen, xub_StrLen& rCutStart, xub_StrLen& rCutStop ) const
{
    rCutStop = rCutStart = STRING_NOTFOUND;

    if( ! m_pPhoneNr )
        return String( rOrig );

#define FAX_PHONE_TOKEN          "@@#"
#define FAX_PHONE_TOKEN_LENGTH   3
#define FAX_END_TOKEN            "@@"
#define FAX_END_TOKEN_LENGTH     2

    bool bRet = false;
    bool bStarted = false;
    bool bStopped = false;
    USHORT nPos;
    USHORT nStart = 0;
    USHORT nStop = rLen;
    String aPhone = rOrig.Copy( nIndex, rLen );

    static String aPhoneNumber;
    static bool bIsCollecting = false;

    if( ! bIsCollecting )
    {
        if( ( nPos = aPhone.SearchAscii( FAX_PHONE_TOKEN ) ) != STRING_NOTFOUND )
        {
            nStart = nPos;
            bIsCollecting = true;
            aPhoneNumber.Erase();
            bRet = true;
            bStarted = true;
        }
    }
    if( bIsCollecting )
    {
        bRet = true;
        nPos = bStarted ? nStart + FAX_PHONE_TOKEN_LENGTH : 0;
        if( ( nPos = aPhone.SearchAscii( FAX_END_TOKEN, nPos ) ) != STRING_NOTFOUND )
        {
            bIsCollecting = false;
            nStop = nPos + FAX_END_TOKEN_LENGTH;
            bStopped = true;
        }
        int nTokenStart = nStart + (bStarted ? FAX_PHONE_TOKEN_LENGTH : 0);
        int nTokenStop = nStop - (bStopped ? FAX_END_TOKEN_LENGTH : 0);
        aPhoneNumber += aPhone.Copy( nTokenStart, nTokenStop - nTokenStart );
        if( ! bIsCollecting )
        {
            *m_pPhoneNr = aPhoneNumber;
            aPhoneNumber.Erase();
        }
    }
    if( aPhoneNumber.Len() > 1024 )
    {
        bIsCollecting = false;
        aPhoneNumber.Erase();
        bRet = false;
    }

    String aRet;
    if( bRet && m_bSwallowFaxNo )
    {
        rLen -= nStop - nStart;
        rCutStart = nStart+nIndex;
        rCutStop = nStop+nIndex;
        if( rCutStart )
            aRet = rOrig.Copy( 0, rCutStart );
        aRet += rOrig.Copy( rCutStop );
    }
    else
        aRet = rOrig;

    return aRet;
}

//==========================================================================

class ImplPspFontData : public ImplFontData
{
private:
    enum { PSPFD_MAGIC = 0xb5bf01f0 };
    int                     mnFontId;

public:
                            ImplPspFontData( const psp::FastPrintFontInfo& );
    virtual                 ~ImplPspFontData();
    virtual int             GetFontId() const { return mnFontId; }
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

ImplPspFontData::~ImplPspFontData()
{
    // TODO: better integration with GlyphCache
    int nFontId = GetFontId();
    GlyphCache::GetInstance().RemoveFont( nFontId );
}

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
    int                 mnFontID;
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
    PspServerFontLayout( ::psp::PrinterGfx&, ServerFont& rFont );

    virtual void        InitFont() const;
private:
    ::psp::PrinterGfx&  mrPrinterGfx;
    int                 mnFontID;
    int                 mnFontHeight;
    int                 mnFontWidth;
    bool                mbVertical;
    bool                mbArtItalic;
    bool                mbArtBold;
};

PspServerFontLayout::PspServerFontLayout( ::psp::PrinterGfx& rGfx, ServerFont& rFont )
        :   ServerFontLayout( rFont ),
            mrPrinterGfx( rGfx )
{
    mnFontID     = mrPrinterGfx.GetFontID();
    mnFontHeight = mrPrinterGfx.GetFontHeight();
    mnFontWidth  = mrPrinterGfx.GetFontWidth();
    mbVertical   = mrPrinterGfx.GetFontVertical();
    mbArtItalic  = mrPrinterGfx.GetArtificialItalic();
    mbArtBold    = mrPrinterGfx.GetArtificialBold();
}

void PspServerFontLayout::InitFont() const
{
    mrPrinterGfx.SetFont( mnFontID, mnFontHeight, mnFontWidth,
                          mnOrientation, mbVertical, mbArtItalic, mbArtBold );
}

//--------------------------------------------------------------------------

void DrawPrinterLayout( const SalLayout& rLayout, ::psp::PrinterGfx& rGfx )
{
    const int nMaxGlyphs = 200;
    sal_Int32   aGlyphAry[ nMaxGlyphs ];
    sal_Int32   aWidthAry[ nMaxGlyphs ];
    sal_Int32   aIdxAry  [ nMaxGlyphs ];
    sal_Unicode aUnicodes[ nMaxGlyphs ];
    Point aPos;
    long nUnitsPerPixel = rLayout.GetUnitsPerPixel();
    for( int nStart = 0;; )
    {
        int nGlyphCount = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart, aWidthAry );
        if( !nGlyphCount )
            break;

        sal_Int32 nXOffset = 0;
        for( int i = 0; i < nGlyphCount; ++i )
        {
            nXOffset += aWidthAry[ i ];
            aIdxAry[ i ] = nXOffset / nUnitsPerPixel;
            sal_uInt32 nGlyphIdx = aGlyphAry[i] & (GF_IDXMASK | GF_ROTMASK);
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
    DrawPrinterLayout( *this, mrPrinterGfx );
}

void PspGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    // print complex text
    DrawPrinterLayout( rLayout, *m_pPrinterGfx );
}

ImplFontCharMap* PspGraphics::GetImplFontCharMap() const
{
    // TODO: get ImplFontCharMap directly from fonts
    int nPairCount = 0;
    if( m_pServerFont[0] )
        nPairCount = m_pServerFont[0]->GetFontCodeRanges( NULL );
    if( !nPairCount )
        return NULL;

    sal_uInt32* pCodePairs = new sal_uInt32[ 2 * nPairCount ];
    if( m_pServerFont[0] )
        m_pServerFont[0]->GetFontCodeRanges( pCodePairs );
    return new ImplFontCharMap( nPairCount, pCodePairs );
}

USHORT PspGraphics::SetFont( ImplFontSelectData *pEntry, int nFallbackLevel )
{
    sal_Bool bVertical = pEntry->mbVertical;
    sal_Int32 nID = pEntry->mpFontData ? (sal_Int32)pEntry->mpFontData->GetFontId() : 0;

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
    if( nRealWeight < nWeight-2 ||
        ( nRealWeight < (int)psp::weight::SemiBold && nWeight > (int)WEIGHT_NORMAL) )
    {
        bArtBold = true;
    }

    // also set the serverside font for layouting
    m_bFontVertical = pEntry->mbVertical;
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( m_pServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *m_pServerFont[i] );
            m_pServerFont[i] = NULL;
        }
    }

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
        else
            m_pServerFont[ nFallbackLevel ] = pServerFont;
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

bool PspGraphics::AddTempDevFont( ImplDevFontList*, const String& rFileURL, const String& rFontName )
{
    return false;
}

void PspGraphics::GetDevFontList( ImplDevFontList *pList )
{
    ::std::list< psp::fontID > aList;
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    rMgr.getFontList( aList, m_pJobData->m_pParser );

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
            pOutDev->ImplAddDevFontSubstitute( it->first, it->second, FONT_SUBSTITUTE_ALWAYS );
    }
}

void PspGraphics::GetFontMetric( ImplFontMetricData *pMetric )
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

ULONG PspGraphics::GetKernPairs( ULONG nPairs, ImplKernPairData *pKernPairs )
{
    const ::std::list< ::psp::KernPair >& rPairs( m_pPrinterGfx->getKernPairs() );
    ULONG nHavePairs = rPairs.size();
    if( pKernPairs && nPairs )
    {
        ::std::list< ::psp::KernPair >::const_iterator it;
        int i;
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

BOOL PspGraphics::GetGlyphBoundRect( long nGlyphIndex, Rectangle& rRect )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return FALSE;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return FALSE;

    nGlyphIndex &= ~GF_FONTMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( nGlyphIndex );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return TRUE;
}

BOOL PspGraphics::GetGlyphOutline( long nGlyphIndex, PolyPolygon& rPolyPoly )
{
    int nLevel = nGlyphIndex >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return FALSE;

    ServerFont* pSF = m_pServerFont[ nLevel ];
    if( !pSF )
        return FALSE;

    nGlyphIndex &= ~GF_FONTMASK;
    if( pSF->GetGlyphOutline( nGlyphIndex, rPolyPoly ) )
        return TRUE;

    return FALSE;
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
        pLayout = new PspServerFontLayout( *m_pPrinterGfx, *m_pServerFont[nFallbackLevel] );
    else
        pLayout = new PspFontLayout( *m_pPrinterGfx );

    return pLayout;
}

//--------------------------------------------------------------------------

BOOL PspGraphics::CreateFontSubset(
                                   const rtl::OUString& rToFile,
                                   ImplFontData* pFont,
                                   sal_Int32* pGlyphIDs,
                                   sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphs,
                                   FontSubsetInfo& rInfo
                                   )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoCreateFontSubset( rToFile, aFont, pGlyphIDs, pEncoding, pWidths, nGlyphs, rInfo );
}

//--------------------------------------------------------------------------

const void* PspGraphics::GetEmbedFontData( ImplFontData* pFont, const sal_Unicode* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
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

const std::map< sal_Unicode, sal_Int32 >* PspGraphics::GetFontEncodingVector( ImplFontData* pFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}


// static helpers of PspGraphics

bool PspGraphics::DoCreateFontSubset( const rtl::OUString& rToFile,
                                      psp::fontID aFont,
                                      sal_Int32* pGlyphIDs,
                                      sal_uInt8* pEncoding,
                                      sal_Int32* pWidths,
                                      int nGlyphs,
                                      FontSubsetInfo& rInfo )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    psp::PrintFontInfo aFontInfo;

    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
        return false;

    // fill in font info
    switch( aFontInfo.m_eType )
    {
        case psp::fonttype::TrueType: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;break;
        case psp::fonttype::Type1: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TYPE1;break;
        default:
            return false;
    }
    rInfo.m_nAscent     = aFontInfo.m_nAscend;
    rInfo.m_nDescent    = aFontInfo.m_nDescend;
    rInfo.m_aPSName     = rMgr.getPSName( aFont );

    int xMin, yMin, xMax, yMax;
    rMgr.getFontBoundingBox( aFont, xMin, yMin, xMax, yMax );

    if( ! rMgr.createFontSubset( aFont,
                                 rToFile,
                                 pGlyphIDs,
                                 pEncoding,
                                 pWidths,
                                 nGlyphs
                                 ) )
        return false;

    rInfo.m_aFontBBox   = Rectangle( Point( xMin, yMin ), Size( xMax-xMin, yMax-yMin ) );
    rInfo.m_nCapHeight  = yMax; // Well ...

    return true;
}

const void* PspGraphics::DoGetEmbedFontData( fontID aFont, const sal_Unicode* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();

    psp::PrintFontInfo aFontInfo;
    if( ! rMgr.getFontInfo( aFont, aFontInfo ) )
        return NULL;

    // fill in font info
    switch( aFontInfo.m_eType )
    {
        case psp::fonttype::TrueType: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TRUETYPE;break;
        case psp::fonttype::Type1: rInfo.m_nFontType = SAL_FONTSUBSETINFO_TYPE_TYPE1;break;
        default:
            return NULL;
    }
    rInfo.m_nAscent     = aFontInfo.m_nAscend;
    rInfo.m_nDescent    = aFontInfo.m_nDescend;
    rInfo.m_aPSName     = rMgr.getPSName( aFont );

    int xMin, yMin, xMax, yMax;
    rMgr.getFontBoundingBox( aFont, xMin, yMin, xMax, yMax );

    psp::CharacterMetric aMetrics[256];
    sal_Unicode aUnicodes[256];
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

    return pFile;
}

void PspGraphics::DoFreeEmbedFontData( const void* pData, long nLen )
{
    if( pData )
        munmap( (char*)pData, nLen );
}

const std::map< sal_Unicode, sal_Int32 >* PspGraphics::DoGetFontEncodingVector( fontID aFont, const std::map< sal_Unicode, rtl::OString >** pNonEncoded )
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
    }
    return WEIGHT_DONTKNOW;
}

FontPitch PspGraphics::ToFontPitch (psp::pitch::type ePitch)
{
    switch (ePitch)
    {
        case psp::pitch::Fixed:     return PITCH_FIXED;
        case psp::pitch::Variable:  return PITCH_VARIABLE;
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
    }
    return FAMILY_DONTKNOW;
}

ImplDevFontAttributes PspGraphics::Info2DevFontAttributes( const psp::FastPrintFontInfo& rInfo )
{
    ImplDevFontAttributes aDFA;
    aDFA.maName         = rInfo.m_aFamilyName;
    //aDFA.maStyleName  = ???
    aDFA.meFamily       = ToFontFamily (rInfo.m_eFamilyStyle);
    aDFA.meWeight       = ToFontWeight (rInfo.m_eWeight);
    aDFA.meItalic       = ToFontItalic (rInfo.m_eItalic);
    aDFA.meWidthType    = ToFontWidth (rInfo.m_eWidth);
    aDFA.mePitch        = ToFontPitch (rInfo.m_ePitch);
    aDFA.mbSymbolFlag   = (rInfo.m_aEncoding == RTL_TEXTENCODING_SYMBOL);

    // special case for the ghostscript fonts
    if( aDFA.maName.CompareIgnoreCaseToAscii( "itc ", 4 ) == COMPARE_EQUAL )
        aDFA.maName = aDFA.maName.Copy( 4 );

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
