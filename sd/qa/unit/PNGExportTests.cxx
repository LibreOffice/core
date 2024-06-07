/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/propertyvalue.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/filter/PngImageReader.hxx>

using namespace ::com::sun::star;

class SdPNGExportTest : public UnoApiTest
{
public:
    SdPNGExportTest()
        : UnoApiTest("/sd/qa/unit/data/")
    {
    }
};

static void assertColorsAreSimilar(const std::string& message, const BitmapColor& expected,
                                   const BitmapColor& actual, int nDelta)
{
    // Check that the two colors match or are reasonably similar.
    if (expected.GetColorError(actual) <= nDelta)
        return;

    CPPUNIT_ASSERT_EQUAL_MESSAGE(message, expected, actual);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf105998)
{
    loadFromFile(u"odp/tdf105998.odp");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG"))
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xShape);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure only the shape is exported
    Size aSize = aBMPEx.GetSizePixel();
    const auto[scalingX, scalingY] = getDPIScaling();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(193 * scalingX, aSize.getWidth(), 1.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(193 * scalingY, aSize.getHeight(), 1.5);
    if (!IsDefaultDPI())
        return;

    // Check all borders are red
    // use assertColorsAreSimilar since the color might differ a little bit on mac
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        BitmapScopedReadAccess pReadAccess(aBMP);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            const Color aColorTop = pReadAccess->GetColor(0, nX);
            const Color aColorBottom = pReadAccess->GetColor(aSize.Height() - 1, nX);

            assertColorsAreSimilar("Incorrect top border", COL_LIGHTRED, aColorTop, 5);

            // Without the fix in place, this test would have failed with
            // - Expected: Color: R:255 G:0 B:0 A:0
            // - Actual  : Color: R:9 G:9 B:9 A:0
            assertColorsAreSimilar("Incorrect bottom border", COL_LIGHTRED, aColorBottom, 5);
        }

        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColorLeft = pReadAccess->GetColor(nY, 0);
            const Color aColorRight = pReadAccess->GetColor(nY, aSize.Width() - 1);

            assertColorsAreSimilar("Incorrect left border", COL_LIGHTRED, aColorLeft, 5);
            assertColorsAreSimilar("Incorrect right border", COL_LIGHTRED, aColorRight, 5);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf126319)
{
    loadFromFile(u"odg/tdf126319.odg");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG"))
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xShape);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure only the shape is exported
    Size aSize = aBMPEx.GetSizePixel();
    const auto[scalingX, scalingY] = getDPIScaling();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(295 * scalingX, aSize.getWidth(), 1.5);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(134 * scalingY, aSize.getHeight(), 1.5);
    if (!IsDefaultDPI())
        return;

    // Check all borders are red or similar. Ignore the corners
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        BitmapScopedReadAccess pReadAccess(aBMP);
        for (tools::Long nX = 2; nX < aSize.Width() - 2; ++nX)
        {
            const Color aColorTop = pReadAccess->GetColor(0, nX);
            // tdf#150102 the exported graphic now has no longer an empty bottom
            // row, so test with '-1' in height , same as already in test
            // testTdf105998 above
            const Color aColorBottom = pReadAccess->GetColor(aSize.Height() - 1, nX);

            assertColorsAreSimilar("Incorrect top border", COL_LIGHTRED, aColorTop, 5);

            // Without the fix in place, this test would have failed with
            // - Expected: Color: R:255 G:0 B:0 A:0
            // - Actual  : Color: R:77 G:0 B:0 A:0
            assertColorsAreSimilar("Incorrect bottom border", COL_LIGHTRED, aColorBottom, 5);
        }

        for (tools::Long nY = 2; nY < aSize.Height() - 2; ++nY)
        {
            const Color aColorLeft = pReadAccess->GetColor(nY, 0);
            // tdf#150102 the exported graphic now has no longer an empty right
            // column, so test with '-1' in width , same as already in test
            // testTdf105998 above
            const Color aColorRight = pReadAccess->GetColor(nY, aSize.Width() - 1);

            assertColorsAreSimilar("Incorrect left border", COL_LIGHTRED, aColorLeft, 5);
            assertColorsAreSimilar("Incorrect right border", COL_LIGHTRED, aColorRight, 5);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf136632)
{
    // Reuse existing file
    loadFromFile(u"odp/tdf105998.odp");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{ comphelper::makePropertyValue("Translucent",
                                                                                   sal_Int32(0)) };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xShape);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();
    AlphaMask aAlpha = aBMPEx.GetAlphaMask();
    BitmapScopedReadAccess pReadAccess(aAlpha);

    // Without the fix in place, this test would have failed here
    CPPUNIT_ASSERT(!pReadAccess);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf157652)
{
    loadFromFile(u"odp/tdf157652.odp");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);

            // Without the fix in place, this test would have failed with
            // - Expected: rgba[ffffffff]
            // - Actual  : rgba[ff953eff]
            CPPUNIT_ASSERT_EQUAL(COL_WHITE, aColor);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf156808)
{
    loadFromFile(u"pptx/tdf156808.pptx");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    int nBlackCount = 0;
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);
            if (aColor == COL_BLACK)
                ++nBlackCount;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected greater than: 9000
    // - Actual  : 0
    CPPUNIT_ASSERT_GREATER(9000, nBlackCount);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf157795)
{
    loadFromFile(u"odp/tdf157795.odp");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100)),
        comphelper::makePropertyValue("Translucent", sal_Int32(1)),
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);

            // Without the fix in place, this test would have failed with
            // - Expected: rgba[000000ff]
            // - Actual  : rgba[010101ff]
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, aColor);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf105362)
{
    loadFromFile(u"odp/tdf105362.odp");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100)),
        comphelper::makePropertyValue("Translucent", sal_Int32(1)),
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);

            // Without the fix in place, this test would have failed with
            // - Expected: rgba[000000ff]
            // - Actual  : rgba[ffffffff]
            CPPUNIT_ASSERT_EQUAL(COL_BLACK, aColor);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf157636)
{
    loadFromFile(u"ppt/tdf157636.ppt");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    int nBlackCount = 0;
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);
            if (aColor == COL_BLACK)
                ++nBlackCount;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected greater than: 6600
    // - Actual  : 559
    CPPUNIT_ASSERT_GREATER(6600, nBlackCount);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf157793)
{
    loadFromFile(u"pptx/tdf157793.pptx");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    int nWhiteCount = 0;
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);
            if (aColor == 0xffffff)
                ++nWhiteCount;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected greater than: 7800
    // - Actual  : 0
    CPPUNIT_ASSERT_GREATER(7800, nWhiteCount);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf157635)
{
    loadFromFile(u"pptx/tdf157635.pptx");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    BitmapScopedReadAccess pReadAccess(aBMP);
    int nBlackCount = 0;
    for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
    {
        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColor = pReadAccess->GetColor(nY, nX);
            if (aColor == COL_BLACK)
                ++nBlackCount;
        }
    }

    // Without the fix in place, this test would have failed with
    // - Expected greater than: 7100
    // - Actual  : 0
    CPPUNIT_ASSERT_GREATER(7100, nBlackCount);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf113163)
{
    loadFromFile(u"pptx/tdf113163.pptx");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        BitmapScopedReadAccess pReadAccess(aBMP);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
            {
                // Check all pixels in the image are black
                // Without the fix in place, this test would have failed with
                // - Expected: 0
                // - Actual  : 16777215
                const Color aColor = pReadAccess->GetColor(nY, nX);
                CPPUNIT_ASSERT_EQUAL(COL_BLACK, aColor);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf147119)
{
    loadFromFile(u"odg/tdf147119.odg");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100)),
        comphelper::makePropertyValue("Translucent", sal_Int32(1)),
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    AlphaMask aAlpha = aBMPEx.GetAlphaMask();
    {
        BitmapScopedReadAccess pReadAccess(aAlpha);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
            {
                // Without the fix in place, this test would have failed with
                // - Expected: Color: R:255 G:255 B:255 A:0
                // - Actual  : Color: R:0 G:0 B:0 A:0
                const Color aColor = pReadAccess->GetColor(nY, nX);
                CPPUNIT_ASSERT_EQUAL(COL_ALPHA_TRANSPARENT, aColor);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf113197)
{
    loadFromFile(u"odp/tdf113197.odp");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100)),
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        BitmapScopedReadAccess pReadAccess(aBMP);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            // Check the bottom half of the document is white
            for (tools::Long nY = 50; nY < aSize.Height() - 1; ++nY)
            {
                // Check all pixels in the image are black
                // Without the fix in place, this test would have failed with
                // - Expected: Color: R:255 G:255 B:255 A:0
                // - Actual  : Color: R:153 G:0 B:51 A:0
                const Color aColor = pReadAccess->GetColor(nY, nX);
                CPPUNIT_ASSERT_EQUAL(COL_WHITE, aColor);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf93124)
{
    loadFromFile(u"ppt/tdf93124.ppt");
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(320)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(180))
    };

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", maTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    CPPUNIT_ASSERT_EQUAL(Size(320, 180), aBMPEx.GetSizePixel());
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        BitmapScopedReadAccess pReadAccess(aBMP);
        int nNonWhiteCount = 0;
        // The word "Top" should be in rectangle 34,4 - 76,30. If text alignment is wrong, the rectangle will be white.
        for (tools::Long nY = 4; nY < (4 + 26); ++nY)
        {
            for (tools::Long nX = 34; nX < (34 + 43); ++nX)
            {
                const Color aColor = pReadAccess->GetColor(nY, nX);
                if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff)
                    || (aColor.GetBlue() != 0xff))
                    ++nNonWhiteCount;
            }
        }
        CPPUNIT_ASSERT_MESSAGE("Tdf93124: vertical alignment of text is incorrect!",
                               nNonWhiteCount > 50);
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf99729)
{
    const OUString filenames[] = { "odp/tdf99729-new.odp", "odp/tdf99729-legacy.odp" };
    int nonwhitecounts[] = { 0, 0 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(filenames); ++i)
    {
        // 1st check for new behaviour - having AnchoredTextOverflowLegacy compatibility flag set to false in settings.xml
        loadFromFile(filenames[i]);

        uno::Reference<uno::XComponentContext> xContext = getComponentContext();
        CPPUNIT_ASSERT(xContext.is());
        uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
            = drawing::GraphicExportFilter::create(xContext);
        CPPUNIT_ASSERT(xGraphicExporter.is());

        uno::Sequence<beans::PropertyValue> aFilterData{
            comphelper::makePropertyValue("PixelWidth", sal_Int32(320)),
            comphelper::makePropertyValue("PixelHeight", sal_Int32(240))
        };

        uno::Sequence<beans::PropertyValue> aDescriptor{
            comphelper::makePropertyValue("URL", maTempFile.GetURL()),
            comphelper::makePropertyValue("FilterName", OUString("PNG")),
            comphelper::makePropertyValue("FilterData", aFilterData)
        };

        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                               uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPage.is());
        xGraphicExporter->setSourceDocument(xPage);
        xGraphicExporter->filter(aDescriptor);

        SvFileStream aFileStream(maTempFile.GetURL(), StreamMode::READ);
        vcl::PngImageReader aPNGReader(aFileStream);
        BitmapEx aBMPEx = aPNGReader.read();
        Bitmap aBMP = aBMPEx.GetBitmap();
        BitmapScopedReadAccess pRead(aBMP);
        for (tools::Long nX = 154; nX < (154 + 12); ++nX)
        {
            for (tools::Long nY = 16; nY < (16 + 96); ++nY)
            {
                const Color aColor = pRead->GetColor(nY, nX);
                if ((aColor.GetRed() != 0xff) || (aColor.GetGreen() != 0xff)
                    || (aColor.GetBlue() != 0xff))
                    ++nonwhitecounts[i];
            }
        }
    }
    // The numbers 1-9 should be above the Text Box in rectangle 154,16 - 170,112.
    // If text alignment is wrong, the rectangle will be white.
    CPPUNIT_ASSERT_MESSAGE("Tdf99729: vertical alignment of text is incorrect!",
                           nonwhitecounts[0] > 100); // it is 134 with cleartype disabled
    // The numbers 1-9 should be below the Text Box -> rectangle 154,16 - 170,112 should be white.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tdf99729: legacy vertical alignment of text is incorrect!", 0,
                                 nonwhitecounts[1]);
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf155048)
{
    loadFromFile(u"odg/diagonalLine.fodg");

    auto xGraphicExporter = drawing::GraphicExportFilter::create(getComponentContext());
    CPPUNIT_ASSERT(xGraphicExporter);

    auto xSupplier = mxComponent.queryThrow<css::drawing::XDrawPagesSupplier>();
    auto xPage = xSupplier->getDrawPages()->getByIndex(0).queryThrow<css::lang::XComponent>();
    xGraphicExporter->setSourceDocument(xPage);

    // 1. AA disabled
    {
#ifdef MACOSX
    // Disable test that always fails with Apple Retina displays
    // With a HiDPI display on macOS, each logical pixel is backed
    // by 4 physical pixels. Turning off antialiasing and drawing a
    // diagonal line causes many logical pixels on the edge of the
    // diagonal line to have a mixture of white and blue physical
    // pixels. Then, when such logical pixels are fetched via
    // BitmapEx::GetPixelColor(), their underlying 4 white and blue
    // physical pixels are combined into blended shades of white
    // and blue.
#else
        css::uno::Sequence<css::beans::PropertyValue> aFilterData{
            comphelper::makePropertyValue("PixelWidth", sal_Int32(200)),
            comphelper::makePropertyValue("PixelHeight", sal_Int32(200)),
            comphelper::makePropertyValue("AntiAliasing", false),
        };

        css::uno::Sequence<css::beans::PropertyValue> aDescriptor{
            comphelper::makePropertyValue("URL", maTempFile.GetURL()),
            comphelper::makePropertyValue("FilterName", OUString("PNG")),
            comphelper::makePropertyValue("FilterData", aFilterData)
        };

        xGraphicExporter->filter(aDescriptor);

        BitmapEx bmp = vcl::PngImageReader(*maTempFile.GetStream(StreamMode::READ)).read();
        std::set<Color> foundColors;
        for (tools::Long x = 0; x < bmp.GetSizePixel().Width(); ++x)
            for (tools::Long y = 0; y < bmp.GetSizePixel().Height(); ++y)
                foundColors.insert(bmp.GetPixelColor(x, y));

        // There must be only two colors (white and blue) in the bitmap generated without AA
        CPPUNIT_ASSERT_EQUAL(size_t(2), foundColors.size());
        maTempFile.CloseStream();
#endif
    }

    // 2. AA enabled
    {
        css::uno::Sequence<css::beans::PropertyValue> aFilterData{
            comphelper::makePropertyValue("PixelWidth", sal_Int32(200)),
            comphelper::makePropertyValue("PixelHeight", sal_Int32(200)),
            comphelper::makePropertyValue("AntiAliasing", true),
        };

        css::uno::Sequence<css::beans::PropertyValue> aDescriptor{
            comphelper::makePropertyValue("URL", maTempFile.GetURL()),
            comphelper::makePropertyValue("FilterName", OUString("PNG")),
            comphelper::makePropertyValue("FilterData", aFilterData)
        };

        xGraphicExporter->filter(aDescriptor);

        BitmapEx bmp = vcl::PngImageReader(*maTempFile.GetStream(StreamMode::READ)).read();
        std::set<Color> foundColors;
        for (tools::Long x = 0; x < bmp.GetSizePixel().Width(); ++x)
            for (tools::Long y = 0; y < bmp.GetSizePixel().Height(); ++y)
                foundColors.insert(bmp.GetPixelColor(x, y));

        // With AA, the number of colors will be greater - it is 19 on my system
        CPPUNIT_ASSERT_GREATER(size_t(2), foundColors.size());
        maTempFile.CloseStream();
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();
