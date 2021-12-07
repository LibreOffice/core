/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <unx/GenPspGfxBackend.hxx>
#include <unx/printergfx.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <salbmp.hxx>

// ----- Implementation of PrinterBmp by means of SalBitmap/BitmapBuffer ---------------

namespace
{
class SalPrinterBmp : public psp::PrinterBmp
{
private:
    BitmapBuffer* mpBmpBuffer;

    FncGetPixel mpFncGetPixel;
    Scanline mpScanAccess;
    sal_PtrDiff mnScanOffset;

public:
    explicit SalPrinterBmp(BitmapBuffer* pBitmap);

    virtual sal_uInt32 GetPaletteColor(sal_uInt32 nIdx) const override;
    virtual sal_uInt32 GetPaletteEntryCount() const override;
    virtual sal_uInt32 GetPixelRGB(sal_uInt32 nRow, sal_uInt32 nColumn) const override;
    virtual sal_uInt8 GetPixelGray(sal_uInt32 nRow, sal_uInt32 nColumn) const override;
    virtual sal_uInt8 GetPixelIdx(sal_uInt32 nRow, sal_uInt32 nColumn) const override;
    virtual sal_uInt32 GetDepth() const override;
};
}

SalPrinterBmp::SalPrinterBmp(BitmapBuffer* pBuffer)
    : mpBmpBuffer(pBuffer)
{
    assert(mpBmpBuffer && "SalPrinterBmp::SalPrinterBmp () can't acquire Bitmap");

    // calibrate scanline buffer
    if (mpBmpBuffer->mnFormat & ScanlineFormat::TopDown)
    {
        mpScanAccess = mpBmpBuffer->mpBits;
        mnScanOffset = mpBmpBuffer->mnScanlineSize;
    }
    else
    {
        mpScanAccess
            = mpBmpBuffer->mpBits + (mpBmpBuffer->mnHeight - 1) * mpBmpBuffer->mnScanlineSize;
        mnScanOffset = -mpBmpBuffer->mnScanlineSize;
    }

    // request read access to the pixels
    mpFncGetPixel = BitmapReadAccess::GetPixelFunction(mpBmpBuffer->mnFormat);
}

sal_uInt32 SalPrinterBmp::GetDepth() const
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

        case 24:
        case 32:
            nDepth = 24;
            break;

        default:
            nDepth = 1;
            assert(false && "Error: unsupported bitmap depth in SalPrinterBmp::GetDepth()");
            break;
    }

    return nDepth;
}

sal_uInt32 SalPrinterBmp::GetPaletteEntryCount() const
{
    return mpBmpBuffer->maPalette.GetEntryCount();
}

sal_uInt32 SalPrinterBmp::GetPaletteColor(sal_uInt32 nIdx) const
{
    BitmapColor aColor(mpBmpBuffer->maPalette[nIdx]);

    return ((aColor.GetBlue()) & 0x000000ff) | ((aColor.GetGreen() << 8) & 0x0000ff00)
           | ((aColor.GetRed() << 16) & 0x00ff0000);
}

sal_uInt32 SalPrinterBmp::GetPixelRGB(sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel(pScan, nColumn, mpBmpBuffer->maColorMask);

    if (!!mpBmpBuffer->maPalette)
        GetPaletteColor(aColor.GetIndex());

    return ((aColor.GetBlue()) & 0x000000ff) | ((aColor.GetGreen() << 8) & 0x0000ff00)
           | ((aColor.GetRed() << 16) & 0x00ff0000);
}

sal_uInt8 SalPrinterBmp::GetPixelGray(sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel(pScan, nColumn, mpBmpBuffer->maColorMask);

    if (!!mpBmpBuffer->maPalette)
        aColor = mpBmpBuffer->maPalette[aColor.GetIndex()];

    return (aColor.GetBlue() * 28UL + aColor.GetGreen() * 151UL + aColor.GetRed() * 77UL) >> 8;
}

sal_uInt8 SalPrinterBmp::GetPixelIdx(sal_uInt32 nRow, sal_uInt32 nColumn) const
{
    Scanline pScan = mpScanAccess + nRow * mnScanOffset;
    BitmapColor aColor = mpFncGetPixel(pScan, nColumn, mpBmpBuffer->maColorMask);

    if (!!mpBmpBuffer->maPalette)
        return aColor.GetIndex();
    else
        return 0;
}

GenPspGfxBackend::GenPspGfxBackend(psp::PrinterGfx* pPrinterGfx)
    : m_pPrinterGfx(pPrinterGfx)
{
}

GenPspGfxBackend::~GenPspGfxBackend() {}

void GenPspGfxBackend::Init() {}
void GenPspGfxBackend::freeResources() {}

bool GenPspGfxBackend::setClipRegion(vcl::Region const& rRegion)
{
    // TODO: support polygonal clipregions here
    RectangleVector aRectangles;
    rRegion.GetRegionRectangles(aRectangles);
    m_pPrinterGfx->BeginSetClipRegion();

    for (auto const& rectangle : aRectangles)
    {
        const tools::Long nWidth(rectangle.GetWidth());
        const tools::Long nHeight(rectangle.GetHeight());

        if (nWidth && nHeight)
        {
            m_pPrinterGfx->UnionClipRegion(rectangle.Left(), rectangle.Top(), nWidth, nHeight);
        }
    }

    m_pPrinterGfx->EndSetClipRegion();

    return true;
}

void GenPspGfxBackend::ResetClipRegion() { m_pPrinterGfx->ResetClipRegion(); }

sal_uInt16 GenPspGfxBackend::GetBitCount() const { return m_pPrinterGfx->GetBitCount(); }

tools::Long GenPspGfxBackend::GetGraphicsWidth() const { return 0; }

void GenPspGfxBackend::SetLineColor() { m_pPrinterGfx->SetLineColor(); }

void GenPspGfxBackend::SetLineColor(Color nColor)
{
    psp::PrinterColor aColor(nColor.GetRed(), nColor.GetGreen(), nColor.GetBlue());
    m_pPrinterGfx->SetLineColor(aColor);
}

void GenPspGfxBackend::SetFillColor() { m_pPrinterGfx->SetFillColor(); }

void GenPspGfxBackend::SetFillColor(Color nColor)
{
    psp::PrinterColor aColor(nColor.GetRed(), nColor.GetGreen(), nColor.GetBlue());
    m_pPrinterGfx->SetFillColor(aColor);
}

void GenPspGfxBackend::SetXORMode(bool bSet, bool /*bInvertOnly*/)
{
    SAL_WARN_IF(bSet, "vcl", "Error: PrinterGfx::SetXORMode() not implemented");
}

void GenPspGfxBackend::SetROPLineColor(SalROPColor /*nROPColor*/)
{
    SAL_WARN("vcl", "Error: PrinterGfx::SetROPLineColor() not implemented");
}

void GenPspGfxBackend::SetROPFillColor(SalROPColor /*nROPColor*/)
{
    SAL_WARN("vcl", "Error: PrinterGfx::SetROPFillColor() not implemented");
}

void GenPspGfxBackend::drawPixel(tools::Long nX, tools::Long nY)
{
    m_pPrinterGfx->DrawPixel(Point(nX, nY));
}
void GenPspGfxBackend::drawPixel(tools::Long nX, tools::Long nY, Color nColor)
{
    psp::PrinterColor aColor(nColor.GetRed(), nColor.GetGreen(), nColor.GetBlue());
    m_pPrinterGfx->DrawPixel(Point(nX, nY), aColor);
}

void GenPspGfxBackend::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2)
{
    m_pPrinterGfx->DrawLine(Point(nX1, nY1), Point(nX2, nY2));
}
void GenPspGfxBackend::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                tools::Long nHeight)
{
    m_pPrinterGfx->DrawRect(tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight)));
}

void GenPspGfxBackend::drawPolyLine(sal_uInt32 nPoints, const Point* pPointArray)
{
    m_pPrinterGfx->DrawPolyLine(nPoints, pPointArray);
}

void GenPspGfxBackend::drawPolygon(sal_uInt32 nPoints, const Point* pPointArray)
{
    // Point must be equal to Point! see include/vcl/salgtype.hxx
    m_pPrinterGfx->DrawPolygon(nPoints, pPointArray);
}

void GenPspGfxBackend::drawPolyPolygon(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                       const Point** pPointArray)
{
    m_pPrinterGfx->DrawPolyPolygon(nPoly, pPoints, pPointArray);
}

bool GenPspGfxBackend::drawPolyPolygon(const basegfx::B2DHomMatrix& /*rObjectToDevice*/,
                                       const basegfx::B2DPolyPolygon&, double /*fTransparency*/)
{
    // TODO: implement and advertise OutDevSupportType::B2DDraw support
    return false;
}

bool GenPspGfxBackend::drawPolyLine(const basegfx::B2DHomMatrix& /*rObjectToDevice*/,
                                    const basegfx::B2DPolygon& /*rPolygon*/,
                                    double /*fTransparency*/, double /*fLineWidth*/,
                                    const std::vector<double>* /*pStroke*/, basegfx::B2DLineJoin,
                                    css::drawing::LineCap, double /*fMiterMinimumAngle*/,
                                    bool /*bPixelSnapHairline*/)
{
    // TODO: a PS printer can draw B2DPolyLines almost directly
    return false;
}

bool GenPspGfxBackend::drawPolyLineBezier(sal_uInt32 nPoints, const Point* pPointArray,
                                          const PolyFlags* pFlagArray)
{
    m_pPrinterGfx->DrawPolyLineBezier(nPoints, pPointArray, pFlagArray);
    return true;
}

bool GenPspGfxBackend::drawPolygonBezier(sal_uInt32 nPoints, const Point* pPointArray,
                                         const PolyFlags* pFlagArray)
{
    m_pPrinterGfx->DrawPolygonBezier(nPoints, pPointArray, pFlagArray);
    return true;
}

bool GenPspGfxBackend::drawPolyPolygonBezier(sal_uInt32 nPoly, const sal_uInt32* pPoints,
                                             const Point* const* pPointArray,
                                             const PolyFlags* const* pFlagArray)
{
    // Point must be equal to Point! see include/vcl/salgtype.hxx
    m_pPrinterGfx->DrawPolyPolygonBezier(nPoly, pPoints, pPointArray, pFlagArray);
    return true;
}

void GenPspGfxBackend::copyArea(tools::Long /*nDestX*/, tools::Long /*nDestY*/,
                                tools::Long /*nSrcX*/, tools::Long /*nSrcY*/,
                                tools::Long /*nSrcWidth*/, tools::Long /*nSrcHeight*/,
                                bool /*bWindowInvalidate*/)
{
    OSL_FAIL("Error: PrinterGfx::CopyArea() not implemented");
}

void GenPspGfxBackend::copyBits(const SalTwoRect& /*rPosAry*/, SalGraphics* /*pSrcGraphics*/)
{
    OSL_FAIL("Error: PrinterGfx::CopyBits() not implemented");
}

void GenPspGfxBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    tools::Rectangle aSrc(Point(rPosAry.mnSrcX, rPosAry.mnSrcY),
                          Size(rPosAry.mnSrcWidth, rPosAry.mnSrcHeight));

    tools::Rectangle aDst(Point(rPosAry.mnDestX, rPosAry.mnDestY),
                          Size(rPosAry.mnDestWidth, rPosAry.mnDestHeight));

    BitmapBuffer* pBuffer
        = const_cast<SalBitmap&>(rSalBitmap).AcquireBuffer(BitmapAccessMode::Read);

    SalPrinterBmp aBmp(pBuffer);
    m_pPrinterGfx->DrawBitmap(aDst, aSrc, aBmp);

    const_cast<SalBitmap&>(rSalBitmap).ReleaseBuffer(pBuffer, BitmapAccessMode::Read);
}

void GenPspGfxBackend::drawBitmap(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rSalBitmap*/,
                                  const SalBitmap& /*rMaskBitmap*/)
{
    OSL_FAIL("Error: no PrinterGfx::DrawBitmap() for transparent bitmap");
}

void GenPspGfxBackend::drawMask(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rSalBitmap*/,
                                Color /*nMaskColor*/)
{
    OSL_FAIL("Error: PrinterGfx::DrawMask() not implemented");
}

std::shared_ptr<SalBitmap> GenPspGfxBackend::getBitmap(tools::Long /*nX*/, tools::Long /*nY*/,
                                                       tools::Long /*nWidth*/,
                                                       tools::Long /*nHeight*/)
{
    SAL_INFO("vcl", "Warning: PrinterGfx::GetBitmap() not implemented");
    return nullptr;
}

Color GenPspGfxBackend::getPixel(tools::Long /*nX*/, tools::Long /*nY*/)
{
    OSL_FAIL("Warning: PrinterGfx::GetPixel() not implemented");
    return Color();
}

void GenPspGfxBackend::invert(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                              tools::Long /*nHeight*/, SalInvert /*nFlags*/)
{
    OSL_FAIL("Warning: PrinterGfx::Invert() not implemented");
}

void GenPspGfxBackend::invert(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/, SalInvert /*nFlags*/)
{
    SAL_WARN("vcl", "Error: PrinterGfx::Invert() not implemented");
}

bool GenPspGfxBackend::drawEPS(tools::Long nX, tools::Long nY, tools::Long nWidth,
                               tools::Long nHeight, void* pPtr, sal_uInt32 nSize)
{
    return m_pPrinterGfx->DrawEPS(tools::Rectangle(Point(nX, nY), Size(nWidth, nHeight)), pPtr,
                                  nSize);
}

bool GenPspGfxBackend::blendBitmap(const SalTwoRect& /*rPosAry*/, const SalBitmap& /*rBitmap*/)
{
    return false;
}

bool GenPspGfxBackend::blendAlphaBitmap(const SalTwoRect& /*rPosAry*/,
                                        const SalBitmap& /*rSrcBitmap*/,
                                        const SalBitmap& /*rMaskBitmap*/,
                                        const SalBitmap& /*rAlphaBitmap*/)
{
    return false;
}

bool GenPspGfxBackend::drawAlphaBitmap(const SalTwoRect& /*rPosAry*/,
                                       const SalBitmap& /*rSourceBitmap*/,
                                       const SalBitmap& /*rAlphaBitmap*/)
{
    return false;
}

bool GenPspGfxBackend::drawTransformedBitmap(const basegfx::B2DPoint& /*rNull*/,
                                             const basegfx::B2DPoint& /*rX*/,
                                             const basegfx::B2DPoint& /*rY*/,
                                             const SalBitmap& /*rSourceBitmap*/,
                                             const SalBitmap* /*pAlphaBitmap*/, double /*fAlpha*/)
{
    return false;
}

bool GenPspGfxBackend::drawAlphaRect(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                                     tools::Long /*nHeight*/, sal_uInt8 /*nTransparency*/)
{
    return false;
}

bool GenPspGfxBackend::drawGradient(const tools::PolyPolygon& /*rPolygon*/,
                                    const Gradient& /*rGradient*/)
{
    return false;
}

bool GenPspGfxBackend::implDrawGradient(basegfx::B2DPolyPolygon const& /*rPolyPolygon*/,
                                        SalGradient const& /*rGradient*/)
{
    return false;
}

bool GenPspGfxBackend::supportsOperation(OutDevSupportType /*eType*/) const { return false; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
