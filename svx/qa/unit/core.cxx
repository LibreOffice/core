/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>
#include <com/sun/star/embed/XStorage.hpp>
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
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest(u"svx/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testChartExportToPdf)
{
    // Given a Calc document with a chart in it:
    loadFromFile(u"chart.ods");
    uno::Reference<drawing::XDrawPagesSupplier> xDrawPagesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage(xDrawPagesSupplier->getDrawPages()->getByIndex(0),
                                                 uno::UNO_QUERY);
    uno::Reference<drawing::XShape> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);

    // When exporting that chart to PDF:
    GraphicHelper::SaveShapeAsGraphicToPath(mxComponent, xShape, u"application/pdf"_ustr,
                                            maTempFile.GetURL());

    // Then make sure we get a valid, non-empty PDF:
    // Without the accompanying fix in place, this test would have failed, because the output was
    // empty (0 bytes).
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument = parsePDFExport();
    if (!pPdfDocument)
    {
        return;
    }
    int nPageCount = pPdfDocument->getPageCount();
    CPPUNIT_ASSERT_GREATER(0, nPageCount);
}

CPPUNIT_TEST_FIXTURE(Test, testGraphicObjectResolver)
{
    OUString aURL = createFileURL(u"GraphicObjectResolverTest.zip");
    uno::Reference<embed::XStorage> xStorage
        = comphelper::OStorageHelper::GetStorageOfFormatFromURL(ZIP_STORAGE_FORMAT_STRING, aURL,
                                                                embed::ElementModes::READ);
    CPPUNIT_ASSERT(xStorage.is());

    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper
        = SvXMLGraphicHelper::Create(xStorage, SvXMLGraphicHelperMode::Read);
    CPPUNIT_ASSERT(xGraphicHelper.is());

    // Test name in root folder
    {
        uno::Reference<graphic::XGraphic> xGraphic
            = xGraphicHelper->loadGraphic(u"SomeImage.png"_ustr);
        CPPUNIT_ASSERT_EQUAL(true, xGraphic.is());
    }

    // Test name in sub-folder
    {
        uno::Reference<graphic::XGraphic> xGraphic
            = xGraphicHelper->loadGraphic(u"Pictures/SomeOtherImage.png"_ustr);
        CPPUNIT_ASSERT_EQUAL(true, xGraphic.is());
    }

    // Test non-existent name
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        try
        {
            xGraphic = xGraphicHelper->loadGraphic(u"NoneExistent.png"_ustr);
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
