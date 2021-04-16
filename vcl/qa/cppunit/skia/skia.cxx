/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>

#include <tools/stream.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

#include <vcl/graphicfilter.hxx>
#include <vcl/virdev.hxx>
#include <vcl/skia/SkiaHelper.hxx>

#include <skia/salbmp.hxx>
#include <skia/utils.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

using namespace SkiaHelper;

// This tests backends that use Skia (i.e. intentionally not the svp one, which is the default.)
// Note that you still may need to actually set for Skia to be used (see vcl/README.vars).
// If Skia is not enabled, all tests will be silently skipped.
namespace
{
class SkiaTest : public test::BootstrapFixture
{
public:
    SkiaTest()
        : test::BootstrapFixture(true, false)
    {
    }

    void testBitmapErase();
    void testDrawShaders();
    void testInterpretAs8Bit();
    void testAlphaBlendWith();
    void testBitmapCopyOnWrite();
    void testMatrixQuality();
    void testDelayedScale();
    void testDelayedScaleAlphaImage();
    void testDrawDelayedScaleImage();
    void testChecksum();
    void testTdf137329();
    void testTdf140848();
    void testTdf132367();

    CPPUNIT_TEST_SUITE(SkiaTest);
    CPPUNIT_TEST(testBitmapErase);
    CPPUNIT_TEST(testDrawShaders);
    CPPUNIT_TEST(testInterpretAs8Bit);
    CPPUNIT_TEST(testAlphaBlendWith);
    CPPUNIT_TEST(testBitmapCopyOnWrite);
    CPPUNIT_TEST(testMatrixQuality);
    CPPUNIT_TEST(testDelayedScale);
    CPPUNIT_TEST(testDelayedScaleAlphaImage);
    CPPUNIT_TEST(testDrawDelayedScaleImage);
    CPPUNIT_TEST(testChecksum);
    CPPUNIT_TEST(testTdf137329);
    CPPUNIT_TEST(testTdf140848);
    CPPUNIT_TEST(testTdf132367);
    CPPUNIT_TEST_SUITE_END();

private:
#if 0
    template <class BitmapT> // handle both Bitmap and BitmapEx
    void savePNG(const OUString& sWhere, const BitmapT& rBmp)
    {
        SvFileStream aStream(sWhere, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(BitmapEx(rBmp), aStream);
    }
    void savePNG(const OUString& sWhere, const ScopedVclPtr<VirtualDevice>& device)
    {
        SvFileStream aStream(sWhere, StreamMode::WRITE | StreamMode::TRUNC);
        GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
        rFilter.compressAsPNG(device->GetBitmapEx(Point(), device->GetOutputSizePixel()), aStream);
    }
#endif
};

void SkiaTest::testBitmapErase()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    SkiaSalBitmap* skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap);
    // Uninitialized bitmap.
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasAlphaImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
    // Test that Bitmap.Erase() just sets erase color and doesn't allocate pixels.
    bitmap.Erase(COL_RED);
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasAlphaImage());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasEraseColor());
    // Reading a pixel will create pixel data.
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_RED), BitmapReadAccess(bitmap).GetColor(0, 0));
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasAlphaImage());
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
}

// Test that draw calls that internally result in SkShader calls work properly.
void SkiaTest::testDrawShaders()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(20, 20));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->Erase();
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(COL_RED);
    SkiaSalBitmap* skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->PreferSkShader());
    AlphaMask alpha(Size(10, 10));
    alpha.Erase(64);
    SkiaSalBitmap* skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaAlpha->PreferSkShader());

    device->DrawBitmap(Point(5, 5), bitmap);
    //savePNG("/tmp/a1.png", device);
    // Check that the area is painted, but nothing else.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(COL_RED, device->GetPixel(Point(5, 5)));
    CPPUNIT_ASSERT_EQUAL(COL_RED, device->GetPixel(Point(14, 14)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(15, 15)));
    device->Erase();

    device->DrawBitmapEx(Point(5, 5), BitmapEx(bitmap, alpha));
    //savePNG("/tmp/a2.png", device);
    Color resultRed(COL_RED.GetRed() * 3 / 4 + 64, 64, 64); // 3/4 red, 1/4 white
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(resultRed, device->GetPixel(Point(5, 5)));
    CPPUNIT_ASSERT_EQUAL(resultRed, device->GetPixel(Point(14, 14)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(15, 15)));
    device->Erase();

    basegfx::B2DHomMatrix matrix;
    matrix.scale(10, 10);
    matrix.rotate(M_PI / 4);
    device->DrawTransformedBitmapEx(matrix, BitmapEx(bitmap, alpha));
    //savePNG("/tmp/a3.png", device);
    CPPUNIT_ASSERT_EQUAL(resultRed, device->GetPixel(Point(0, 1)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(1, 0)));
    CPPUNIT_ASSERT_EQUAL(resultRed, device->GetPixel(Point(0, 10)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(10, 10)));
    device->Erase();

    // Test with scaling. Use everything 10x larger to reduce the impact of smoothscaling.
    ScopedVclPtr<VirtualDevice> deviceLarge = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    deviceLarge->SetOutputSizePixel(Size(200, 200));
    deviceLarge->SetBackground(Wallpaper(COL_WHITE));
    deviceLarge->Erase();
    Bitmap bitmapLarge(Size(100, 100), vcl::PixelFormat::N24_BPP);
    bitmapLarge.Erase(COL_RED);
    SkiaSalBitmap* skiaBitmapLarge
        = dynamic_cast<SkiaSalBitmap*>(bitmapLarge.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmapLarge->PreferSkShader());
    AlphaMask alphaLarge(Size(100, 100));
    alphaLarge.Erase(64);
    {
        BitmapWriteAccess access(bitmapLarge);
        access.SetFillColor(COL_BLUE);
        access.FillRect(tools::Rectangle(Point(20, 40), Size(10, 10)));
    }
    // Using alpha will still lead to shaders being used.
    deviceLarge->DrawBitmapEx(Point(50, 50), Size(60, 60), Point(20, 20), Size(30, 30),
                              BitmapEx(bitmapLarge, alphaLarge));
    //savePNG("/tmp/a4.png", deviceLarge);
    Color resultBlue(64, 64, COL_BLUE.GetBlue() * 3 / 4 + 64); // 3/4 blue, 1/4 white
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, deviceLarge->GetPixel(Point(40, 40)));
    CPPUNIT_ASSERT_EQUAL(resultRed, deviceLarge->GetPixel(Point(50, 50)));
    CPPUNIT_ASSERT_EQUAL(resultRed, deviceLarge->GetPixel(Point(100, 100)));
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, deviceLarge->GetPixel(Point(110, 110)));
    // Don't test colors near the edge between the colors, smoothscaling affects them.
    const int diff = 3;
    CPPUNIT_ASSERT_EQUAL(resultRed, deviceLarge->GetPixel(Point(50 + diff, 89 - diff)));
    CPPUNIT_ASSERT_EQUAL(resultBlue, deviceLarge->GetPixel(Point(50 + diff, 90 + diff)));
    CPPUNIT_ASSERT_EQUAL(resultBlue, deviceLarge->GetPixel(Point(69 - diff, 100 - diff)));
    CPPUNIT_ASSERT_EQUAL(resultRed, deviceLarge->GetPixel(Point(70 + diff, 100 - diff)));
    CPPUNIT_ASSERT_EQUAL(resultBlue, deviceLarge->GetPixel(Point(50 + diff, 100 - diff)));
    device->Erase();
}

void SkiaTest::testInterpretAs8Bit()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    // Test with erase color.
    bitmap.Erase(Color(33, 33, 33));
    SkiaSalBitmap* skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasEraseColor());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N24_BPP, bitmap.getPixelFormat());
    bitmap.Convert(BmpConversion::N8BitNoConversion);
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasEraseColor());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, bitmap.getPixelFormat());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(33), BitmapReadAccess(bitmap).GetPixelIndex(0, 0));

    // Test with image.
    bitmap = Bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(Color(34, 34, 34));
    BitmapReadAccess(bitmap).GetColor(0, 0); // Create pixel data, reset erase color.
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    skiaBitmap->GetSkImage();
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N24_BPP, bitmap.getPixelFormat());
    bitmap.Convert(BmpConversion::N8BitNoConversion);
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasImage());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, bitmap.getPixelFormat());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(34), BitmapReadAccess(bitmap).GetPixelIndex(0, 0));
}

void SkiaTest::testAlphaBlendWith()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    AlphaMask alpha(Size(10, 10));
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    // Test with erase colors set.
    alpha.Erase(64);
    SkiaSalBitmap* skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaAlpha->unittestHasEraseColor());
    bitmap.Erase(Color(64, 64, 64));
    SkiaSalBitmap* skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasEraseColor());
    alpha.BlendWith(bitmap);
    skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaAlpha->unittestHasEraseColor());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, alpha.getPixelFormat());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(208),
                         AlphaMask::ScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    // Test with images set.
    alpha.Erase(64);
    AlphaMask::ScopedReadAccess(alpha)->GetColor(0, 0); // Reading a pixel will create pixel data.
    skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    skiaAlpha->GetSkImage();
    CPPUNIT_ASSERT(!skiaAlpha->unittestHasEraseColor());
    CPPUNIT_ASSERT(skiaAlpha->unittestHasImage());
    bitmap.Erase(Color(64, 64, 64));
    Bitmap::ScopedReadAccess(bitmap)->GetColor(0, 0); // Reading a pixel will create pixel data.
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    skiaBitmap->GetSkImage();
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasImage());
    alpha.BlendWith(bitmap);
    skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaAlpha->unittestHasImage());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, alpha.getPixelFormat());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(207),
                         AlphaMask::ScopedReadAccess(alpha)->GetPixelIndex(0, 0));

    // Test with erase color for alpha and image for other bitmap.
    alpha.Erase(64);
    skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaAlpha->unittestHasEraseColor());
    bitmap.Erase(Color(64, 64, 64));
    Bitmap::ScopedReadAccess(bitmap)->GetColor(0, 0); // Reading a pixel will create pixel data.
    skiaBitmap = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    skiaBitmap->GetSkImage();
    CPPUNIT_ASSERT(!skiaBitmap->unittestHasEraseColor());
    CPPUNIT_ASSERT(skiaBitmap->unittestHasImage());
    alpha.BlendWith(bitmap);
    skiaAlpha = dynamic_cast<SkiaSalBitmap*>(alpha.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaAlpha->unittestHasImage());
    CPPUNIT_ASSERT_EQUAL(vcl::PixelFormat::N8_BPP, alpha.getPixelFormat());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>(207),
                         AlphaMask::ScopedReadAccess(alpha)->GetPixelIndex(0, 0));
}

void SkiaTest::testBitmapCopyOnWrite()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    SkiaSalBitmap bitmap;
    CPPUNIT_ASSERT(bitmap.Create(Size(10, 10), vcl::PixelFormat::N24_BPP, BitmapPalette()));
    bitmap.GetSkImage();
    bitmap.GetAlphaSkImage();
    CPPUNIT_ASSERT(bitmap.unittestHasBuffer());
    CPPUNIT_ASSERT(bitmap.unittestHasImage());
    CPPUNIT_ASSERT(bitmap.unittestHasAlphaImage());
    SkiaSalBitmap bitmap2;
    CPPUNIT_ASSERT(bitmap2.Create(bitmap));
    // Data should be shared.
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetBuffer(), bitmap2.unittestGetBuffer());
    CPPUNIT_ASSERT(bitmap2.unittestHasImage());
    CPPUNIT_ASSERT(bitmap2.unittestHasAlphaImage());
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetImage(), bitmap2.unittestGetImage());
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetAlphaImage(), bitmap2.unittestGetAlphaImage());
    // Reading still should keep the data shared.
    const SkImage* oldImage = bitmap.unittestGetImage();
    const SkImage* oldAlphaImage = bitmap.unittestGetAlphaImage();
    BitmapBuffer* buffer = bitmap.AcquireBuffer(BitmapAccessMode::Read);
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetBuffer(), bitmap2.unittestGetBuffer());
    bitmap.ReleaseBuffer(buffer, BitmapAccessMode::Read);
    // Images get possibly updated only after releasing the buffer.
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetImage(), bitmap2.unittestGetImage());
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetAlphaImage(), bitmap2.unittestGetAlphaImage());
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetImage(), oldImage);
    CPPUNIT_ASSERT_EQUAL(bitmap.unittestGetAlphaImage(), oldAlphaImage);
    // Writing should unshare.
    buffer = bitmap.AcquireBuffer(BitmapAccessMode::Write);
    CPPUNIT_ASSERT(bitmap.unittestGetBuffer() != bitmap2.unittestGetBuffer());
    bitmap.ReleaseBuffer(buffer, BitmapAccessMode::Write);
    CPPUNIT_ASSERT(bitmap.unittestGetImage() != bitmap2.unittestGetImage());
    CPPUNIT_ASSERT(bitmap.unittestGetAlphaImage() != bitmap2.unittestGetAlphaImage());
    CPPUNIT_ASSERT(bitmap.unittestGetImage() != oldImage);
    CPPUNIT_ASSERT(bitmap.unittestGetAlphaImage() != oldAlphaImage);
}

void SkiaTest::testMatrixQuality()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    // Not changing the size (but possibly rotated/flipped) does not need high quality transformations.
    CPPUNIT_ASSERT(!SkiaTests::matrixNeedsHighQuality(SkMatrix()));
    CPPUNIT_ASSERT(!SkiaTests::matrixNeedsHighQuality(SkMatrix::RotateDeg(90)));
    CPPUNIT_ASSERT(!SkiaTests::matrixNeedsHighQuality(SkMatrix::RotateDeg(180)));
    CPPUNIT_ASSERT(!SkiaTests::matrixNeedsHighQuality(SkMatrix::RotateDeg(270)));
    CPPUNIT_ASSERT(!SkiaTests::matrixNeedsHighQuality(SkMatrix::Scale(1, -1)));
    CPPUNIT_ASSERT(SkiaTests::matrixNeedsHighQuality(SkMatrix::Scale(0, -1)));
    CPPUNIT_ASSERT(SkiaTests::matrixNeedsHighQuality(SkMatrix::Scale(2, 1)));
    CPPUNIT_ASSERT(SkiaTests::matrixNeedsHighQuality(SkMatrix::RotateDeg(89)));
}

void SkiaTest::testDelayedScale()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    Bitmap bitmap1(Size(10, 10), vcl::PixelFormat::N24_BPP);
    SkiaSalBitmap* skiaBitmap1 = dynamic_cast<SkiaSalBitmap*>(bitmap1.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap1);
    // Do scaling based on mBuffer.
    (void)BitmapReadAccess(bitmap1); // allocates mBuffer
    CPPUNIT_ASSERT(skiaBitmap1->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap1->unittestHasImage());
    CPPUNIT_ASSERT(bitmap1.Scale(2, 2, BmpScaleFlag::Default));
    skiaBitmap1 = dynamic_cast<SkiaSalBitmap*>(bitmap1.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap1);
    CPPUNIT_ASSERT(skiaBitmap1->unittestHasBuffer());
    CPPUNIT_ASSERT(!skiaBitmap1->unittestHasImage());
    CPPUNIT_ASSERT_EQUAL(Size(20, 20), bitmap1.GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(Size(20, 20), imageSize(skiaBitmap1->GetSkImage()));
    BitmapBuffer* buffer1 = skiaBitmap1->AcquireBuffer(BitmapAccessMode::Read);
    CPPUNIT_ASSERT(buffer1);
    CPPUNIT_ASSERT_EQUAL(tools::Long(20), buffer1->mnWidth);
    CPPUNIT_ASSERT_EQUAL(tools::Long(20), buffer1->mnHeight);
    skiaBitmap1->ReleaseBuffer(buffer1, BitmapAccessMode::Read);
    // Do scaling based on mImage.
    SkiaSalBitmap skiaBitmap2(skiaBitmap1->GetSkImage());
    CPPUNIT_ASSERT(!skiaBitmap2.unittestHasBuffer());
    CPPUNIT_ASSERT(skiaBitmap2.unittestHasImage());
    CPPUNIT_ASSERT(skiaBitmap2.Scale(2, 3, BmpScaleFlag::Default));
    CPPUNIT_ASSERT(!skiaBitmap2.unittestHasBuffer());
    CPPUNIT_ASSERT(skiaBitmap2.unittestHasImage());
    CPPUNIT_ASSERT_EQUAL(Size(40, 60), skiaBitmap2.GetSize());
    CPPUNIT_ASSERT_EQUAL(Size(40, 60), imageSize(skiaBitmap2.GetSkImage()));
    BitmapBuffer* buffer2 = skiaBitmap2.AcquireBuffer(BitmapAccessMode::Read);
    CPPUNIT_ASSERT(buffer2);
    CPPUNIT_ASSERT_EQUAL(tools::Long(40), buffer2->mnWidth);
    CPPUNIT_ASSERT_EQUAL(tools::Long(60), buffer2->mnHeight);
    skiaBitmap2.ReleaseBuffer(buffer2, BitmapAccessMode::Read);
}

void SkiaTest::testDelayedScaleAlphaImage()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    auto bitmapTmp = std::make_unique<SkiaSalBitmap>();
    CPPUNIT_ASSERT(bitmapTmp->Create(Size(10, 10), vcl::PixelFormat::N24_BPP, BitmapPalette()));
    bitmapTmp->Erase(COL_RED);
    // Create a bitmap that has only an image, not a pixel buffer.
    SkiaSalBitmap bitmap(bitmapTmp->GetSkImage());
    bitmapTmp.release();
    CPPUNIT_ASSERT(!bitmap.unittestHasBuffer());
    CPPUNIT_ASSERT(bitmap.unittestHasImage());
    CPPUNIT_ASSERT(!bitmap.unittestHasAlphaImage());
    // Set up pending scale.
    CPPUNIT_ASSERT(bitmap.Scale(2.0, 2.0, BmpScaleFlag::Fast));
    CPPUNIT_ASSERT(bitmap.unittestHasPendingScale());
    CPPUNIT_ASSERT(bitmap.InterpretAs8Bit());
    // Ask for SkImage and make sure it's scaled up.
    sk_sp<SkImage> image = bitmap.GetSkImage();
    CPPUNIT_ASSERT_EQUAL(20, image->width());
    // Ask again, this time it should be cached.
    sk_sp<SkImage> image2 = bitmap.GetSkImage();
    CPPUNIT_ASSERT_EQUAL(image.get(), image2.get());
    // Add another scale.
    CPPUNIT_ASSERT(bitmap.Scale(3.0, 3.0, BmpScaleFlag::Fast));
    // Ask for alpha SkImage and make sure it's scaled up.
    sk_sp<SkImage> alphaImage = bitmap.GetAlphaSkImage();
    CPPUNIT_ASSERT_EQUAL(60, alphaImage->width());
    // Ask again, this time it should be cached.
    sk_sp<SkImage> alphaImage2 = bitmap.GetAlphaSkImage();
    CPPUNIT_ASSERT_EQUAL(alphaImage.get(), alphaImage2.get());
    // Ask again for non-alpha image, it should be scaled again.
    sk_sp<SkImage> image3 = bitmap.GetSkImage();
    CPPUNIT_ASSERT_EQUAL(60, image3->width());
    CPPUNIT_ASSERT(image3.get() != image2.get());
    CPPUNIT_ASSERT(image3.get() != image.get());
    // Create pixel buffer from the image (it should convert from alpha image because the bitmap is 8bpp
    // and the alpha image size matches).
    SkiaSalBitmap bitmapCopy;
    bitmapCopy.Create(bitmap);
    CPPUNIT_ASSERT(!bitmap.unittestHasBuffer());
    BitmapBuffer* buffer1 = bitmap.AcquireBuffer(BitmapAccessMode::Read);
    CPPUNIT_ASSERT(bitmap.unittestHasBuffer());
    bitmap.ReleaseBuffer(buffer1, BitmapAccessMode::Read);
    CPPUNIT_ASSERT_EQUAL(Size(60, 60), bitmap.GetSize());
    // Scale the copy before the buffer was created (this time it should convert from non-alpha image
    // because of the different size).
    CPPUNIT_ASSERT(!bitmapCopy.unittestHasBuffer());
    CPPUNIT_ASSERT(bitmapCopy.Scale(4.0, 4.0, BmpScaleFlag::Fast));
    BitmapBuffer* buffer2 = bitmapCopy.AcquireBuffer(BitmapAccessMode::Read);
    CPPUNIT_ASSERT(bitmapCopy.unittestHasBuffer());
    bitmapCopy.ReleaseBuffer(buffer2, BitmapAccessMode::Read);
    CPPUNIT_ASSERT_EQUAL(Size(240, 240), bitmapCopy.GetSize());
}

void SkiaTest::testDrawDelayedScaleImage()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    if (SkiaHelper::renderMethodToUse() != SkiaHelper::RenderRaster)
        return; // This test tests caching that's done only in raster mode.
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(10, 10));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->Erase();
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(COL_RED);
    // Set a pixel to create pixel data.
    BitmapWriteAccess(bitmap).SetPixel(0, 0, COL_BLUE);
    SkiaSalBitmap* skiaBitmap1 = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    // Force creating of image.
    sk_sp<SkImage> image1 = skiaBitmap1->GetSkImage();
    CPPUNIT_ASSERT(skiaBitmap1->unittestHasImage());
    CPPUNIT_ASSERT(bitmap.Scale(Size(5, 5)));
    // Make sure delayed scaling has not changed the image.
    SkiaSalBitmap* skiaBitmap2 = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap2->unittestHasImage());
    sk_sp<SkImage> image2 = skiaBitmap2->GetSkImage(SkiaHelper::DirectImage::Yes);
    CPPUNIT_ASSERT_EQUAL(image1, image2);
    CPPUNIT_ASSERT_EQUAL(Size(5, 5), bitmap.GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), SkiaHelper::imageSize(image2));
    // Draw the bitmap scaled to size 10x10 and check that the 10x10 image was used (and kept),
    // even though technically the bitmap is 5x5.
    device->DrawBitmap(Point(0, 0), Size(10, 10), bitmap);
    SkiaSalBitmap* skiaBitmap3 = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    CPPUNIT_ASSERT(skiaBitmap3->unittestHasImage());
    sk_sp<SkImage> image3 = skiaBitmap3->GetSkImage(SkiaHelper::DirectImage::Yes);
    CPPUNIT_ASSERT_EQUAL(image1, image3);
    CPPUNIT_ASSERT_EQUAL(Size(5, 5), bitmap.GetSizePixel());
    CPPUNIT_ASSERT_EQUAL(Size(10, 10), SkiaHelper::imageSize(image3));
}

void SkiaTest::testChecksum()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    Bitmap bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(COL_RED);
    BitmapChecksum checksum1 = bitmap.GetChecksum();
    // Set a pixel to create pixel data, that should change checksum.
    BitmapWriteAccess(bitmap).SetPixel(0, 0, COL_BLUE);
    BitmapChecksum checksum2 = bitmap.GetChecksum();
    CPPUNIT_ASSERT(checksum2 != checksum1);
    SkiaSalBitmap* skiaBitmap1 = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    // Creating an image should not change the checksum.
    sk_sp<SkImage> image1 = skiaBitmap1->GetSkImage();
    BitmapChecksum checksum3 = bitmap.GetChecksum();
    CPPUNIT_ASSERT_EQUAL(checksum2, checksum3);
    // Delayed scaling should change checksum even if the scaling has not taken place.
    bitmap.Scale(Size(20, 20));
    BitmapChecksum checksum4 = bitmap.GetChecksum();
    CPPUNIT_ASSERT(checksum4 != checksum3);
    // Setting back to the original red content should have the original checksum.
    // (This also makes sure this next step is not affected by the delayed scaling
    // above possibly taking place now.)
    bitmap = Bitmap(Size(10, 10), vcl::PixelFormat::N24_BPP);
    bitmap.Erase(COL_RED);
    BitmapChecksum checksum5 = bitmap.GetChecksum();
    CPPUNIT_ASSERT_EQUAL(checksum1, checksum5);
    // The optimized changing of images to greyscale should change the checksum.
    SkiaSalBitmap* skiaBitmap2 = dynamic_cast<SkiaSalBitmap*>(bitmap.ImplGetSalBitmap().get());
    skiaBitmap2->unittestResetToImage();
    BitmapChecksum checksum6 = skiaBitmap2->GetChecksum();
    CPPUNIT_ASSERT_EQUAL(checksum5, checksum6);
    CPPUNIT_ASSERT(skiaBitmap2->ConvertToGreyscale());
    BitmapChecksum checksum7 = skiaBitmap2->GetChecksum();
    CPPUNIT_ASSERT(checksum7 != checksum6);
}

void SkiaTest::testTdf137329()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    // Draw a filled polygon in the entire device, with AA enabled.
    // All pixels in the device should be black, even those at edges (i.e. not affected by AA).
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(10, 10));
    device->SetBackground(Wallpaper(COL_WHITE));
    device->SetAntialiasing(AntialiasingFlags::Enable);
    device->Erase();
    device->SetLineColor();
    device->SetFillColor(COL_BLACK);
    device->DrawPolyPolygon(
        basegfx::B2DPolyPolygon(basegfx::B2DPolygon{ { 0, 0 }, { 10, 0 }, { 10, 10 }, { 0, 10 } }));
    // savePNG("/tmp/tdf137329.png", device);
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(0, 0)));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(9, 0)));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(9, 9)));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(0, 9)));
    CPPUNIT_ASSERT_EQUAL(COL_BLACK, device->GetPixel(Point(4, 4)));
}

void SkiaTest::testTdf140848()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(1300, 400));
    device->SetBackground(Wallpaper(COL_BLACK));
    device->SetAntialiasing(AntialiasingFlags::Enable);
    device->Erase();
    device->SetLineColor();
    device->SetFillColor(COL_WHITE);
    basegfx::B2DPolygon p1 = { { 952.73121259842514519, 102.4599685039370911 },
                               { 952.73121259842514519, 66.55445669291347599 },
                               { 1239.9753070866140661, 66.554456692913390725 },
                               { 1239.9753070866140661, 138.36548031496062094 },
                               { 952.73121259842514519, 138.36548031496070621 } };
    basegfx::B2DPolygon p2 = { { 1168.1642834645670064, 210.17650393700790801 },
                               { 1168.1642834645670064, 66.554456692913404936 },
                               { 1239.9753070866140661, 66.554456692913390725 },
                               { 1239.9753070866142934, 353.79855118110236845 },
                               { 1168.1642834645670064, 353.79855118110236845 } };
    device->DrawPolyPolygon(basegfx::B2DPolyPolygon(p1));
    device->DrawPolyPolygon(basegfx::B2DPolyPolygon(p2));
    //savePNG("/tmp/tdf140848.png", device);
    // Rounding errors caused the overlapping part not to be drawn.
    CPPUNIT_ASSERT_EQUAL(COL_WHITE, device->GetPixel(Point(1200, 100)));
}

void SkiaTest::testTdf132367()
{
    if (!SkiaHelper::isVCLSkiaEnabled())
        return;
    ScopedVclPtr<VirtualDevice> device = VclPtr<VirtualDevice>::Create(DeviceFormat::DEFAULT);
    device->SetOutputSizePixel(Size(2, 2));
    device->SetBackground(Wallpaper(COL_BLACK));
    device->Erase();
    device->DrawPixel(Point(1, 1), COL_WHITE);
    // This will make the bitmap store data in SkImage.
    Bitmap bitmap = device->GetBitmap(Point(0, 0), Size(2, 2));
    // Scaling will only set up delayed scaling of the SkImage.
    bitmap.Scale(Size(4, 4), BmpScaleFlag::NearestNeighbor);
    // Now it will need to be converted to pixel buffer, check it's converted properly
    // from the SkImage.
    BitmapReadAccess access(bitmap);
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), access.Width());
    CPPUNIT_ASSERT_EQUAL(tools::Long(4), access.Height());
    CPPUNIT_ASSERT_EQUAL(BitmapColor(COL_WHITE), access.GetColor(3, 3));
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(SkiaTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
