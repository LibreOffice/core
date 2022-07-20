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
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <comphelper/storagehelper.hxx>

#include <svx/graphichelper.hxx>
#include <svx/xmlgrhlp.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for svx/source/core/ code.
class Test : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    uno::Reference<lang::XComponent> mxComponent;

public:
    void setUp() override;
    void tearDown() override;
    uno::Reference<lang::XComponent>& getComponent() { return mxComponent; }
};

void Test::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop.set(frame::Desktop::create(mxComponentContext));
}

void Test::tearDown()
{
    if (mxComponent.is())
        mxComponent->dispose();

    test::BootstrapFixture::tearDown();
}

constexpr OUStringLiteral DATA_DIRECTORY = u"/svx/qa/unit/data/";

CPPUNIT_TEST_FIXTURE(Test, testChartExportToPdf)
{
    // Given a Calc document with a chart in it:
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "chart.ods";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // When exporting that chart to PDF:
    utl::TempFile aTempFile;
    GraphicHelper::SaveShapeAsGraphicToPath(getComponent(), xShape, "application/pdf",
                                            aTempFile.GetURL());

    // Then make sure we get a valid, non-empty PDF:
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    SvMemoryStream aMemory;
    aMemory.WriteStream(*aTempFile.GetStream(StreamMode::READ));
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPdfium->openDocument(aMemory.GetData(), aMemory.GetSize(), OString());
    // Without the accompanying fix in place, this test would have failed, because the output was
    // empty (0 bytes).
    CPPUNIT_ASSERT(pPdfDocument);
    int nPageCount = pPdfDocument->getPageCount();
    CPPUNIT_ASSERT_GREATER(0, nPageCount);
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicObjectResolver)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "GraphicObjectResolverTest.zip";
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL,
                                                                embed::ElementModes::READWRITE);
    CPPUNIT_ASSERT(xStorage.is());

    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper
        = SvXMLGraphicHelper::Create(xStorage, SvXMLGraphicHelperMode::Read);
    CPPUNIT_ASSERT(xGraphicHelper.is());

    // Test name in root folder
    {
        uno::Reference<graphic::XGraphic> xGraphic = xGraphicHelper->loadGraphic("SomeImage.png");
        CPPUNIT_ASSERT_EQUAL(true, xGraphic.is());
    }

    // Test name in sub-folder
    {
        uno::Reference<graphic::XGraphic> xGraphic
            = xGraphicHelper->loadGraphic("Pictures/SomeOtherImage.png");
        CPPUNIT_ASSERT_EQUAL(true, xGraphic.is());
    }

    // Test non-existent name
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        try
        {
            xGraphic = xGraphicHelper->loadGraphic("NoneExistent.png");
        }
        catch (const uno::Exception&)
        {
        }
        CPPUNIT_ASSERT_EQUAL(false, xGraphic.is());
    }
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
