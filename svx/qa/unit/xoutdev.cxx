/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sal/types.h>
#include <tools/stream.hxx>
#include <unotest/directories.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/graph.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace com::sun::star;

class XOutdevTest : public test::BootstrapFixture, public unotest::MacrosTest
{
    uno::Reference<lang::XComponent> mxComponent;

public:
    virtual void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void XOutdevTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void XOutdevTest::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_FIXTURE(XOutdevTest, testPdfGraphicExport)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return;
    }

    // Import the graphic.
    Graphic aGraphic;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"svx/qa/unit/data/graphic.pdf");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    XOutFlags const eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension
                             | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = aTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "pdf", eFlags);

    // Assert that the output looks like a PDF.
    SvStream* pStream = aTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream->TellEnd() > 5);
    sal_uInt8 sFirstBytes[5];
    pStream->ReadBytes(sFirstBytes, 5);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('%'), sFirstBytes[0]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('D'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('F'), sFirstBytes[3]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('-'), sFirstBytes[4]);
}

CPPUNIT_TEST_FIXTURE(XOutdevTest, testTdf60684)
{
    Graphic aGraphic;
    test::Directories aDirectories;
    OUString aURL = aDirectories.getURLFromSrc(u"svx/qa/unit/data/tdf60684.jpg");
    SvFileStream aStream(aURL, StreamMode::READ);
    CPPUNIT_ASSERT_EQUAL(ERRCODE_NONE,
                         GraphicFilter::GetGraphicFilter().ImportGraphic(aGraphic, aURL, aStream));

    // Export it.
    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    XOutFlags const eFlags = XOutFlags::DontExpandFilename | XOutFlags::DontAddExtension
                             | XOutFlags::UseNativeIfPossible;
    OUString aTempURL = aTempFile.GetURL();
    XOutBitmap::WriteGraphic(aGraphic, aTempURL, "png", eFlags);

    SvStream* pStream = aTempFile.GetStream(StreamMode::READ);
    CPPUNIT_ASSERT(pStream->TellEnd() > 4);
    sal_uInt8 sFirstBytes[4];
    pStream->ReadBytes(sFirstBytes, 4);

    //Checks if the file's header matches a PNG's expected header
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('P'), sFirstBytes[1]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('N'), sFirstBytes[2]);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt8>('G'), sFirstBytes[3]);
}

CPPUNIT_TEST_FIXTURE(XOutdevTest, testFillColorThemeUnoApi)
{
    // Given an empty Impress document with a (title) shape:
    getComponent() = loadFromDesktop("private:factory/simpress",
                                     "com.sun.star.presentation.PresentationDocument");

    // When setting the theme index of the shape's fill color:
    uno::Reference<drawing::XDrawPagesSupplier> xPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xPage(xPagesSupplier->getDrawPages()->getByIndex(0),
                                             uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xShape(xPage->getByIndex(0), uno::UNO_QUERY);
    sal_Int16 nExpected = 4; // Accent 1
    xShape->setPropertyValue("FillColorTheme", uno::Any(nExpected));

    // 80% lighter
    sal_Int16 nExpectedLumMod = 2000;
    xShape->setPropertyValue("FillColorLumMod", uno::Any(nExpectedLumMod));
    sal_Int16 nExpectedLumOff = 8000;
    xShape->setPropertyValue("FillColorLumOff", uno::Any(nExpectedLumOff));

    // Then make sure the value we read back is the expected one:
    sal_Int16 nActual = -1;
    xShape->getPropertyValue("FillColorTheme") >>= nActual;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : -1
    // i.e. setting the value was broken.
    CPPUNIT_ASSERT_EQUAL(nExpected, nActual);
    xShape->getPropertyValue("FillColorLumMod") >>= nActual;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2000
    // - Actual  : 8000
    // i.e. FillColorLumOff was set as FillColor, then getting FillColorLumMod returned FillColor.
    CPPUNIT_ASSERT_EQUAL(nExpectedLumMod, nActual);
    xShape->getPropertyValue("FillColorLumOff") >>= nActual;
    CPPUNIT_ASSERT_EQUAL(nExpectedLumOff, nActual);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
