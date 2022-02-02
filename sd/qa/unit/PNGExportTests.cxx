
/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_poppler.h>
#include <memory>
#include <ostream>
#include <sdpage.hxx>
#include "sdmodeltestbase.hxx"

#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/propertyvalue.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/filter/PngImageReader.hxx>

class SdPNGExportTest : public SdModelTestBase
{
protected:
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    virtual void tearDown() override;
};

void SdPNGExportTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void SdPNGExportTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

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
    mxComponent
        = loadFromDesktop(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/tdf105998.odp"));
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG"))
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xShape);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure only the shape is exported
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(193, 193), aSize);

    // Check all borders are red
    // use assertColorsAreSimilar since the color might differ a little bit on mac
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
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
    mxComponent
        = loadFromDesktop(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/tdf126319.odg"));
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG"))
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xShape);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure only the shape is exported
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(295, 134), aSize);

#if defined(_WIN32) // FIXME
    const sal_Int32 nDiff = 1;
#else
    const sal_Int32 nDiff = 2;
#endif

    // Check all borders are red or similar
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            const Color aColorTop = pReadAccess->GetColor(0, nX);
            const Color aColorBottom = pReadAccess->GetColor(aSize.Height() - 1, nX);

            assertColorsAreSimilar("Incorrect top border", COL_LIGHTRED, aColorTop, 5);

            // Without the fix in place, this test would have failed with
            // - Expected: Color: R:255 G:0 B:0 A:0
            // - Actual  : Color: R:77 G:0 B:0 A:0
            assertColorsAreSimilar("Incorrect bottom border", COL_LIGHTRED, aColorBottom, 5);
        }

        for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
        {
            const Color aColorLeft = pReadAccess->GetColor(nY, 0);
            const Color aColorRight = pReadAccess->GetColor(nY, aSize.Width() - nDiff);

            assertColorsAreSimilar("Incorrect left border", COL_LIGHTRED, aColorLeft, 5);
            assertColorsAreSimilar("Incorrect right border", COL_LIGHTRED, aColorRight, 5);
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf113163)
{
    mxComponent
        = loadFromDesktop(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf113163.pptx"));
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100))
    };

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
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
    mxComponent
        = loadFromDesktop(m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/tdf147119.odg"));
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(100)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(100)),
        comphelper::makePropertyValue("Translucent", true),
    };

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    Size aSize = aBMPEx.GetSizePixel();
    CPPUNIT_ASSERT_EQUAL(Size(100, 100), aSize);
    AlphaMask aAlpha = aBMPEx.GetAlpha();
    {
        AlphaMask::ScopedReadAccess pReadAccess(aAlpha);
        for (tools::Long nX = 1; nX < aSize.Width() - 1; ++nX)
        {
            for (tools::Long nY = 1; nY < aSize.Height() - 1; ++nY)
            {
                // Without the fix in place, this test would have failed with
                // - Expected: Color: R:255 G:255 B:255 A:0
                // - Actual  : Color: R:0 G:0 B:0 A:0
                const Color aColor = pReadAccess->GetColor(nY, nX);
                CPPUNIT_ASSERT_EQUAL(COL_WHITE, aColor);
            }
        }
    }
}

CPPUNIT_TEST_FIXTURE(SdPNGExportTest, testTdf93124)
{
    mxComponent
        = loadFromDesktop(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf93124.ppt"));
    uno::Reference<uno::XComponentContext> xContext = getComponentContext();
    CPPUNIT_ASSERT(xContext.is());
    uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
        = drawing::GraphicExportFilter::create(xContext);

    uno::Sequence<beans::PropertyValue> aFilterData{
        comphelper::makePropertyValue("PixelWidth", sal_Int32(320)),
        comphelper::makePropertyValue("PixelHeight", sal_Int32(180))
    };

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();

    uno::Sequence<beans::PropertyValue> aDescriptor{
        comphelper::makePropertyValue("URL", aTempFile.GetURL()),
        comphelper::makePropertyValue("FilterName", OUString("PNG")),
        comphelper::makePropertyValue("FilterData", aFilterData)
    };

    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                           uno::UNO_QUERY);
    xGraphicExporter->setSourceDocument(xPage);
    xGraphicExporter->filter(aDescriptor);

    SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
    vcl::PngImageReader aPNGReader(aFileStream);
    BitmapEx aBMPEx = aPNGReader.read();

    // make sure the bitmap is not empty and correct size (PNG export->import was successful)
    CPPUNIT_ASSERT_EQUAL(Size(320, 180), aBMPEx.GetSizePixel());
    Bitmap aBMP = aBMPEx.GetBitmap();
    {
        Bitmap::ScopedReadAccess pReadAccess(aBMP);
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
    const OUString filenames[]
        = { "/sd/qa/unit/data/odp/tdf99729-new.odp", "/sd/qa/unit/data/odp/tdf99729-legacy.odp" };
    int nonwhitecounts[] = { 0, 0 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(filenames); ++i)
    {
        // 1st check for new behaviour - having AnchoredTextOverflowLegacy compatibility flag set to false in settings.xml
        mxComponent = loadFromDesktop(m_directories.getURLFromSrc(filenames[i]),
                                      "com.sun.star.presentation.PresentationDocument");

        uno::Reference<uno::XComponentContext> xContext = getComponentContext();
        CPPUNIT_ASSERT(xContext.is());
        uno::Reference<drawing::XGraphicExportFilter> xGraphicExporter
            = drawing::GraphicExportFilter::create(xContext);
        CPPUNIT_ASSERT(xGraphicExporter.is());

        uno::Sequence<beans::PropertyValue> aFilterData{
            comphelper::makePropertyValue("PixelWidth", sal_Int32(320)),
            comphelper::makePropertyValue("PixelHeight", sal_Int32(240))
        };

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();

        uno::Sequence<beans::PropertyValue> aDescriptor{
            comphelper::makePropertyValue("URL", aTempFile.GetURL()),
            comphelper::makePropertyValue("FilterName", OUString("PNG")),
            comphelper::makePropertyValue("FilterData", aFilterData)
        };

        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
        uno::Reference<lang::XComponent> xPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                               uno::UNO_QUERY);
        CPPUNIT_ASSERT(xPage.is());
        xGraphicExporter->setSourceDocument(xPage);
        xGraphicExporter->filter(aDescriptor);

        SvFileStream aFileStream(aTempFile.GetURL(), StreamMode::READ);
        vcl::PngImageReader aPNGReader(aFileStream);
        BitmapEx aBMPEx = aPNGReader.read();
        Bitmap aBMP = aBMPEx.GetBitmap();
        Bitmap::ScopedReadAccess pRead(aBMP);
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
        mxComponent->dispose();
    }
    // The numbers 1-9 should be above the Text Box in rectangle 154,16 - 170,112.
    // If text alignment is wrong, the rectangle will be white.
    CPPUNIT_ASSERT_MESSAGE("Tdf99729: vertical alignment of text is incorrect!",
                           nonwhitecounts[0] > 100); // it is 134 with cleartype disabled
    // The numbers 1-9 should be below the Text Box -> rectangle 154,16 - 170,112 should be white.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tdf99729: legacy vertical alignment of text is incorrect!", 0,
                                 nonwhitecounts[1]);
}

CPPUNIT_PLUGIN_IMPLEMENT();
