/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <vcl/virdev.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <tools/stream.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/PdfPrimitive2D.hxx>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <o3tl/unit_conversion.hxx>

using namespace drawinglayer;

namespace
{
class PdfPrimitive2DTest : public test::BootstrapFixture
{
    const OUString maDataUrl = u"/drawinglayer/qa/unit/data/"_ustr;

public:
    PdfPrimitive2DTest()
        : BootstrapFixture(true, false)
    {
    }

    BinaryDataContainer loadPdfData(std::u16string_view rFileName)
    {
        OUString aPath = m_directories.getURLFromSrc(maDataUrl) + rFileName;
        SvFileStream aStream(aPath, StreamMode::READ);
        return BinaryDataContainer(aStream, aStream.remainingSize());
    }
};

CPPUNIT_TEST_FIXTURE(PdfPrimitive2DTest, testCreatePdfPrimitive)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return; // skip if PDFium not available

    BinaryDataContainer aData = loadPdfData(u"Pangram.pdf");
    CPPUNIT_ASSERT(aData.getSize() > 0);

    // create a transform for a page
    const double fWidthMM100 = o3tl::convert(612.0, o3tl::Length::pt, o3tl::Length::mm100);
    const double fHeightMM100 = o3tl::convert(792.0, o3tl::Length::pt, o3tl::Length::mm100);
    const basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createScaleB2DHomMatrix(fWidthMM100, fHeightMM100));

    rtl::Reference<primitive2d::PdfPrimitive2D> pPrimitive(
        new primitive2d::PdfPrimitive2D(aData, 0, aTransform));

    // verify data access
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), pPrimitive->getPageIndex());
    CPPUNIT_ASSERT_EQUAL(aData.getSize(), pPrimitive->getDataContainer().getSize());
    CPPUNIT_ASSERT_EQUAL(aTransform, pPrimitive->getTransform());

    // verify range
    geometry::ViewInformation2D aViewInfo;
    basegfx::B2DRange aRange = pPrimitive->getB2DRange(aViewInfo);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fWidthMM100, aRange.getWidth(), 1.0);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(fHeightMM100, aRange.getHeight(), 1.0);

    // verify cached PDFium objects
    CPPUNIT_ASSERT(pPrimitive->getPdfDocument() != nullptr);
    CPPUNIT_ASSERT(pPrimitive->getPdfPage() != nullptr);
}

CPPUNIT_TEST_FIXTURE(PdfPrimitive2DTest, testPdfPrimitiveDecomposition)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    BinaryDataContainer aData = loadPdfData(u"Pangram.pdf");
    CPPUNIT_ASSERT(aData.getSize() > 0);

    const double fWidthMM100 = o3tl::convert(612.0, o3tl::Length::pt, o3tl::Length::mm100);
    const double fHeightMM100 = o3tl::convert(792.0, o3tl::Length::pt, o3tl::Length::mm100);
    const basegfx::B2DHomMatrix aTransform
        = basegfx::utils::createScaleB2DHomMatrix(fWidthMM100, fHeightMM100);

    rtl::Reference<primitive2d::PdfPrimitive2D> pPrimitive(
        new primitive2d::PdfPrimitive2D(aData, 0, aTransform));

    // set up a virtual device and processor with a proper view transform
    // that maps mm100 coordinates to the device pixel space
    const sal_Int32 nPixelWidth = 800;
    const sal_Int32 nPixelHeight = 600;

    ScopedVclPtr<VirtualDevice> pDevice
        = VclPtr<VirtualDevice>::Create(DeviceFormat::WITHOUT_ALPHA);
    pDevice->SetOutputSizePixel(Size(nPixelWidth, nPixelHeight));
    pDevice->SetBackground(Wallpaper(COL_WHITE));
    pDevice->Erase();

    // scale to fit the PDF page into the device
    const double fScaleX = nPixelWidth / fWidthMM100;
    const double fScaleY = nPixelHeight / fHeightMM100;
    const double fScale = std::min(fScaleX, fScaleY);
    const basegfx::B2DHomMatrix aObjectToView
        = basegfx::utils::createScaleB2DHomMatrix(fScale, fScale);

    geometry::ViewInformation2D aViewInfo;
    aViewInfo.setObjectTransformation(aObjectToView);
    aViewInfo.setViewport(basegfx::B2DRange(0, 0, nPixelWidth, nPixelHeight));

    std::unique_ptr<processor2d::BaseProcessor2D> pProcessor(
        processor2d::createProcessor2DFromOutputDevice(*pDevice, aViewInfo));
    CPPUNIT_ASSERT(pProcessor);

    // process the primitive - this triggers the decomposition
    primitive2d::Primitive2DContainer aContainer;
    aContainer.push_back(pPrimitive);
    pProcessor->process(aContainer);

    // verify something was actually rendered (not all white)
    Bitmap aBitmap = pDevice->GetBitmap(Point(0, 0), pDevice->GetOutputSizePixel());
    BitmapReadAccess aAccess(aBitmap);
    bool bHasNonWhite = false;
    for (tools::Long y = 0; y < aAccess.Height() && !bHasNonWhite; ++y)
    {
        for (tools::Long x = 0; x < aAccess.Width() && !bHasNonWhite; ++x)
        {
            if (aAccess.GetColor(y, x) != COL_WHITE)
                bHasNonWhite = true;
        }
    }
    CPPUNIT_ASSERT_MESSAGE("PDF should render non-white content", bHasNonWhite);
}

CPPUNIT_TEST_FIXTURE(PdfPrimitive2DTest, testPdfPrimitiveEquality)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
        return;

    BinaryDataContainer aData = loadPdfData(u"Pangram.pdf");
    const basegfx::B2DHomMatrix aTransform(
        basegfx::utils::createScaleB2DHomMatrix(21590.0, 27940.0));

    rtl::Reference<primitive2d::PdfPrimitive2D> pPrimitive1(
        new primitive2d::PdfPrimitive2D(aData, 0, aTransform));
    rtl::Reference<primitive2d::PdfPrimitive2D> pPrimitive2(
        new primitive2d::PdfPrimitive2D(aData, 0, aTransform));

    // same data, same page, same transform - should be equal
    CPPUNIT_ASSERT(pPrimitive1->operator==(*pPrimitive2));

    // different page index - should not be equal
    rtl::Reference<primitive2d::PdfPrimitive2D> pPrimitive3(
        new primitive2d::PdfPrimitive2D(aData, 1, aTransform));
    CPPUNIT_ASSERT(!(*pPrimitive1 == *pPrimitive3));

    // different transforms - should not be equal
    const basegfx::B2DHomMatrix aTransform2(
        basegfx::utils::createScaleB2DHomMatrix(10000.0, 10000.0));
    rtl::Reference<primitive2d::PdfPrimitive2D> pPrimitive4(
        new primitive2d::PdfPrimitive2D(aData, 0, aTransform2));
    CPPUNIT_ASSERT(!(*pPrimitive1 == *pPrimitive4));
}

} // anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
