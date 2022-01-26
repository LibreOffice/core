
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
public:
    virtual void setUp() override;

    void testTdf113163();
    void testTdf93124();
    void testTdf99729();

    CPPUNIT_TEST_SUITE(SdPNGExportTest);

    CPPUNIT_TEST(testTdf113163);
    CPPUNIT_TEST(testTdf93124);
    CPPUNIT_TEST(testTdf99729);

    CPPUNIT_TEST_SUITE_END();
};

void SdPNGExportTest::setUp()
{
    SdModelTestBase::setUp();
    mxDesktop.set(frame::Desktop::create(getComponentContext()));
}

void SdPNGExportTest::testTdf113163()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/tdf113163.pptx"), PPTX);
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
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

    uno::Reference<lang::XComponent> xPage(getPage(0, xDocShRef), uno::UNO_QUERY);
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
                const Color aColor = pReadAccess->GetColor(nX, nY);
                CPPUNIT_ASSERT_EQUAL(COL_BLACK, aColor);
            }
        }
    }
    xDocShRef->DoClose();
}

void SdPNGExportTest::testTdf93124()
{
    sd::DrawDocShellRef xDocShRef
        = loadURL(m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/tdf93124.ppt"), PPT);
    uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
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

    uno::Reference<lang::XComponent> xPage(getPage(0, xDocShRef), uno::UNO_QUERY);
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
    xDocShRef->DoClose();
}

void SdPNGExportTest::testTdf99729()
{
    const OUString filenames[]
        = { "/sd/qa/unit/data/odp/tdf99729-new.odp", "/sd/qa/unit/data/odp/tdf99729-legacy.odp" };
    int nonwhitecounts[] = { 0, 0 };
    for (size_t i = 0; i < SAL_N_ELEMENTS(filenames); ++i)
    {
        // 1st check for new behaviour - having AnchoredTextOverflowLegacy compatibility flag set to false in settings.xml
        uno::Reference<lang::XComponent> xComponent
            = loadFromDesktop(m_directories.getURLFromSrc(filenames[i]),
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

        uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(xComponent, uno::UNO_QUERY);
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
        xComponent->dispose();
    }
    // The numbers 1-9 should be above the Text Box in rectangle 154,16 - 170,112.
    // If text alignment is wrong, the rectangle will be white.
    CPPUNIT_ASSERT_MESSAGE("Tdf99729: vertical alignment of text is incorrect!",
                           nonwhitecounts[0] > 100); // it is 134 with cleartype disabled
    // The numbers 1-9 should be below the Text Box -> rectangle 154,16 - 170,112 should be white.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Tdf99729: legacy vertical alignment of text is incorrect!", 0,
                                 nonwhitecounts[1]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdPNGExportTest);

CPPUNIT_PLUGIN_IMPLEMENT();
