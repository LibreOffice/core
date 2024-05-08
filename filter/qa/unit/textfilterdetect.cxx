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

#include <comphelper/configuration.hxx>
#include <comphelper/propertyvalue.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>
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
bool supportsService(const uno::Reference<lang::XComponent>& x, const OUString& s)
{
    return uno::Reference<lang::XServiceInfo>(x, uno::UNO_QUERY_THROW)->supportsService(s);
}

/// Test class for PlainTextFilterDetect.
class TextFilterDetectTest : public UnoApiTest
{
public:
    TextFilterDetectTest()
        : UnoApiTest(u"/filter/qa/unit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testTdf114428)
{
    uno::Reference<document::XExtendedFilterDetection> xDetect(
        getMultiServiceFactory()->createInstance(
            u"com.sun.star.comp.filters.PlainTextFilterDetect"_ustr),
        uno::UNO_QUERY);
    OUString aURL = createFileURL(u"tdf114428.xhtml");
    SvFileStream aStream(aURL, StreamMode::READ);
    uno::Reference<io::XInputStream> xStream(new utl::OStreamWrapper(aStream));
    uno::Sequence<beans::PropertyValue> aDescriptor
        = { comphelper::makePropertyValue(u"DocumentService"_ustr,
                                          u"com.sun.star.text.TextDocument"_ustr),
            comphelper::makePropertyValue(u"InputStream"_ustr, xStream),
            comphelper::makePropertyValue(u"TypeName"_ustr, u"generic_HTML"_ustr) };
    xDetect->detect(aDescriptor);
    utl::MediaDescriptor aMediaDesc(aDescriptor);
    OUString aFilterName = aMediaDesc.getUnpackedValueOrDefault(u"FilterName"_ustr, OUString());
    // This was empty, XML declaration caused HTML detect to not handle XHTML.
    CPPUNIT_ASSERT_EQUAL(u"HTML (StarWriter)"_ustr, aFilterName);
}

CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testEmptyFile)
{
    // Given an empty file, with a pptx extension
    // When loading the file
    loadFromFile(u"empty.pptx");

    // Then make sure it is opened in Impress.
    // Without the accompanying fix in place, this test would have failed, as it was opened in
    // Writer instead.
    CPPUNIT_ASSERT(
        supportsService(mxComponent, u"com.sun.star.presentation.PresentationDocument"_ustr));

    // Now also test ODT
    loadFromFile(u"empty.odt");
    // Make sure it opens in Writer.
    CPPUNIT_ASSERT(supportsService(mxComponent, u"com.sun.star.text.TextDocument"_ustr));

    // ... and ODS
    loadFromFile(u"empty.ods");
    // Make sure it opens in Calc.
    CPPUNIT_ASSERT(supportsService(mxComponent, u"com.sun.star.sheet.SpreadsheetDocument"_ustr));

    // ... and ODP
    loadFromFile(u"empty.odp");
    // Without the accompanying fix in place, this test would have failed, as it was opened in
    // Writer instead.
    CPPUNIT_ASSERT(
        supportsService(mxComponent, u"com.sun.star.presentation.PresentationDocument"_ustr));

    // ... and DOC
    // Without the accompanying fix in place, this test would have failed, the import filter aborted
    // loading.
    loadFromFile(u"empty.doc");
    CPPUNIT_ASSERT(supportsService(mxComponent, u"com.sun.star.text.TextDocument"_ustr));
    {
        uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aArgs = xModel->getArgs();
        comphelper::SequenceAsHashMap aMap(aArgs);
        OUString aFilterName;
        aMap[u"FilterName"_ustr] >>= aFilterName;
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: MS Word 97
        // - Actual  : MS WinWord 6.0
        // i.e. opening worked, but saving back failed instead of producing a WW8 binary file.
        CPPUNIT_ASSERT_EQUAL(u"MS Word 97"_ustr, aFilterName);
    }

    // Now test with default templates set

    SfxObjectFactory::SetStandardTemplate(u"com.sun.star.presentation.PresentationDocument"_ustr,
                                          createFileURL(u"impress.otp"));
    SfxObjectFactory::SetStandardTemplate(u"com.sun.star.text.TextDocument"_ustr,
                                          createFileURL(u"writer.ott"));
    SfxObjectFactory::SetStandardTemplate(u"com.sun.star.sheet.SpreadsheetDocument"_ustr,
                                          createFileURL(u"calc.ots"));

    loadFromFile(u"empty.pptx");
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages(xDoc->getDrawPages(), uno::UNO_SET_THROW);
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xBox(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(u"Title of Impress template"_ustr, xBox->getString());
    }

    loadFromFile(u"empty.odt");
    {
        uno::Reference<text::XTextDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xEA(xDoc->getText(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration(), uno::UNO_SET_THROW);
        uno::Reference<text::XTextRange> xParagraph(xEnum->nextElement(), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(u"Writer template’s first line"_ustr, xParagraph->getString());
    }

    loadFromFile(u"empty.ods");
    {
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<sheet::XCellRangesAccess> xRA(xDoc->getSheets(), uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xC(xRA->getCellByPosition(0, 0, 0), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(u"Calc template’s first cell"_ustr, xC->getString());
    }

    loadFromFile(u"empty.odp");
    {
        uno::Reference<drawing::XDrawPagesSupplier> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPages> xPages(xDoc->getDrawPages(), uno::UNO_SET_THROW);
        uno::Reference<drawing::XDrawPage> xPage(xPages->getByIndex(0), uno::UNO_QUERY_THROW);
        uno::Reference<text::XTextRange> xBox(xPage->getByIndex(0), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(u"Title of Impress template"_ustr, xBox->getString());
    }
    loadFromFile(u"empty.doc");
    {
        uno::Reference<text::XTextDocument> xDoc(mxComponent, uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumerationAccess> xEA(xDoc->getText(), uno::UNO_QUERY_THROW);
        uno::Reference<container::XEnumeration> xEnum(xEA->createEnumeration(), uno::UNO_SET_THROW);
        uno::Reference<text::XTextRange> xParagraph(xEnum->nextElement(), uno::UNO_QUERY_THROW);

        // Make sure the template's text was loaded
        CPPUNIT_ASSERT_EQUAL(u"Writer template’s first line"_ustr, xParagraph->getString());
    }
}

// The unit test fails on some Linux systems. Until it is found out why the file URLs are broken
// there, let it be Windows-only, since the original issue tested here was Windows-specific.
// See https://lists.freedesktop.org/archives/libreoffice/2023-December/091265.html for details.
#ifdef _WIN32
CPPUNIT_TEST_FIXTURE(TextFilterDetectTest, testHybridPDFFile)
{
    // Make sure that file locking is ON
    {
        std::shared_ptr<comphelper::ConfigurationChanges> xChanges(
            comphelper::ConfigurationChanges::create());
        officecfg::Office::Common::Misc::UseDocumentSystemFileLocking::set(true, xChanges);
        xChanges->commit();
    }

    // Given a hybrid PDF file

    {
        // Created in Writer
        utl::TempFileNamed nonAsciiName(u"абв_αβγ_");
        nonAsciiName.EnableKillingFile();
        CPPUNIT_ASSERT_EQUAL(
            osl::FileBase::E_None,
            osl::File::copy(createFileURL(u"hybrid_writer.pdf"), nonAsciiName.GetURL()));
        load(nonAsciiName.GetURL());
        // Make sure it opens in Writer.
        // Without the accompanying fix in place, this test would have failed on Windows, as it was
        // opened in Draw instead.
        CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.text.TextDocument"));
    }

    {
        // Created in Calc
        utl::TempFileNamed nonAsciiName(u"абв_αβγ_");
        nonAsciiName.EnableKillingFile();
        CPPUNIT_ASSERT_EQUAL(
            osl::FileBase::E_None,
            osl::File::copy(createFileURL(u"hybrid_calc.pdf"), nonAsciiName.GetURL()));
        load(nonAsciiName.GetURL());
        // Make sure it opens in Calc.
        CPPUNIT_ASSERT(supportsService(mxComponent, "com.sun.star.sheet.SpreadsheetDocument"));
    }

    {
        // Created in Impress
        utl::TempFileNamed nonAsciiName(u"абв_αβγ_");
        nonAsciiName.EnableKillingFile();
        CPPUNIT_ASSERT_EQUAL(
            osl::FileBase::E_None,
            osl::File::copy(createFileURL(u"hybrid_impress.pdf"), nonAsciiName.GetURL()));
        load(nonAsciiName.GetURL());
        // Make sure it opens in Impress.
        CPPUNIT_ASSERT(
            supportsService(mxComponent, "com.sun.star.presentation.PresentationDocument"));
    }
}
#endif // _WIN32
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
