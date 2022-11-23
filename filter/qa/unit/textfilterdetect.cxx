/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/sheet/XCellRangesAccess.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertyvalue.hxx>
#include <sfx2/docfac.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/stream.hxx>

namespace com::sun::star::io
{
class XInputStream;
}

using namespace com::sun::star;

namespace
{
/// Test class for PlainTextFilterDetect.
class TextFilterDetectTest : public UnoApiTest
{
public:
    TextFilterDetectTest()
        : UnoApiTest("/filter/qa/unit/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testTdf114428)
{
    uno::Reference<document::XExtendedFilterDetection> xDetect(
        getMultiServiceFactory()->createInstance("com.sun.star.comp.filters.PlainTextFilterDetect"),
        uno::UNO_QUERY);
    OUString aURL = createFileURL(u"tdf114428.xhtml");
    SvFileStream aStream(aURL, StreamMode::READ);
    uno::Reference<io::XInputStream> xStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor
        = { comphelper::makePropertyValue("DocumentService",
                                          OUString("com.sun.star.text.TextDocument")),
            comphelper::makePropertyValue("InputStream", xStream),
            comphelper::makePropertyValue("TypeName", OUString("generic_HTML")) };
    xDetect->detect(aDescriptor);
    utl::MediaDescriptor aMediaDesc(aDescriptor);
    OUString aFilterName = aMediaDesc.getUnpackedValueOrDefault("FilterName", OUString());
    // This was empty, XML declaration caused HTML detect to not handle XHTML.
    CPPUNIT_ASSERT_EQUAL(OUString("HTML (StarWriter)"), aFilterName);
}

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testEmptyFile)
{
    auto supportsService = [](const uno::Reference<lang::XComponent>& x, const OUString& s) {
        return uno::Reference<lang::XServiceInfo>(x, uno::UNO_QUERY_THROW)->supportsService(s);
    };

    // Given an empty file, with a pptx extension
    // When loading the file
    loadFromURL(u"empty.pptx");

    // Then make sure it is opened in Impress.
    // Without the accompanying fix in place, this test would have failed, as it was opened in
    // Writer instead.
    CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.presentation.PresentationDocument"));

    // Now also test ODT
    loadFromURL(u"empty.odt");
    // Make sure it opens in Writer.
    CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.text.TextDocument"));

    // ... and ODS
    loadFromURL(u"empty.ods");
    // Make sure it opens in Calc.
    CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.sheet.SpreadsheetDocument"));

    // ... and ODP
    loadFromURL(u"empty.odp");
    // Without the accompanying fix in place, this test would have failed, as it was opened in
    // Writer instead.
    CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.presentation.PresentationDocument"));

    // ... and DOC
    // Without the accompanying fix in place, this test would have failed, the import filter aborted
    // loading.
    loadFromURL(u"empty.doc");
    CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.text.TextDocument"));
    {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aArgs = xModel->getArgs();
        comphelper::SequenceAsHashMap aMap(aArgs);
        OUString aFilterName;
        aMap["FilterName"] >>= aFilterName;
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: MS Word 97
        // - Actual  : MS WinWord 6.0
        // i.e. opening worked, but saving back failed instead of producing a WW8 binary file.
        CPPUNIT_ASSERT_EQUAL(OUString("MS Word 97"), aFilterName);
    }

    // Now test with default templates set

    SfxObjectFactory::SetStandardTemplate("com.sun.star.presentation.PresentationDocument",
                                          createFileURL(u"impress.otp"));
    SfxObjectFactory::SetStandardTemplate("com.sun.star.text.TextDocument",
                                          createFileURL(u"writer.ott"));
    SfxObjectFactory::SetStandardTemplate("com.sun.star.sheet.SpreadsheetDocument",
                                          createFileURL(u"calc.ots"));

    loadFromURL(u"empty.pptx");
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages(xDoc->getDrawPages(), uno::UNO_SET_THROW);
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xBox(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(OUString("Title of Impress template"), xBox->getString());
    }

    loadFromURL(u"empty.odt");
    {
        uno::Reference<text::XTextDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xEA(xDoc->getText(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration(), uno::UNO_SET_THROW);
        uno::Reference<text::XTextRange> xParagraph(xEnum->nextElement(), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(OUString(u"Writer template’s first line"), xParagraph->getString());
    }

    loadFromURL(u"empty.ods");
    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XCellRangesAccess> xRA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xC(xRA->getCellByPosition(0, 0, 0), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(OUString(u"Calc template’s first cell"), xC->getString());
    }

    loadFromURL(u"empty.odp");
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages(xDoc->getDrawPages(), uno::UNO_SET_THROW);
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xBox(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(OUString("Title of Impress template"), xBox->getString());
    }
    loadFromURL(u"empty.doc");
    {
        uno::Reference<text::XTextDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xEA(xDoc->getText(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration(), uno::UNO_SET_THROW);
        uno::Reference<text::XTextRange> xParagraph(xEnum->nextElement(), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(OUString(u"Writer template’s first line"), xParagraph->getString());
    }
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
