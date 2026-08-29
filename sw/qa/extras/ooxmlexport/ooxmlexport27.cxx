/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <pam.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <wrtsh.hxx>

namespace
{
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr)
    {
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf38575_fullWidthLine, "tdf38575_fullWidthLine.docx")
{
    // given a document where the header contains a graphic that spans the entire left side,
    // reducing the available body text area for every page in the document,
    // and an AS_CHAR horizontal line that wants the entire width (and thus could never fit)

    // for compatibilityMode 15, in MS Word 2024, this is 4 pages long (not infinitely long)
    CPPUNIT_ASSERT_EQUAL(4, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124398_groupshapeChart)
{
    // given a document with grouped chart and textbox
    // TODO: seeing the chart is broken since 24.8.4

    createSwDoc("tdf124398_groupshapeChart.docx");

    // MS Word considered this document to be corrupt for several reasons.
    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // DOCX/wps: doesn't use nvGraphicFramePr - only PPTX/p: and XLSX/xdr: do.
    assertXPath(pXmlDoc, "//wpg:graphicFrame/wpg:cNvPr", 1);
    assertXPath(pXmlDoc, "//wpg:graphicFrame/wpg:xfrm", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf138027_pageBreakAfterShape, "tdf138027_pageBreakAfterShape.docx")
{
    xmlDocUniquePtr pDump = parseLayoutDump();
    // The image and the textbox are on page 1, not after the page break on page 2
    assertXPath(pDump, "/root/page[1]/sorted_objs/fly", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf168607_tabstopZero)
{
    // Given a 1 page document, where the default tab-stop length is zero

    createSwDoc("tdf168607_tabstopZero.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    saveAndReload(TestFilter::DOC);

    CPPUNIT_ASSERT_EQUAL(1, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf163178_columnbreakInTable)
{
    // Given a 1 page document, where a page break is defined inside of a table run

    createSwDoc("tdf163178_columnbreakInTable.docx");
    // CPPUNIT_ASSERT_EQUAL(1, getPages());

    saveAndReload(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:br", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171527_flyInFramePr)
{
    // given a with a framePr'd image anchoring a drawing shape
    createSwDoc("tdf171527_flyInFramePr.doc");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, MS Word complained the file was corrupt.
    // Round-trip the framePr'd image as a framePr instead of as a drawingML
    assertXPath(pXmlDoc, "//w:body/w:p/w:pPr/w:framePr", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf171433_equation)
{
    // given a document with formula
    createSwDoc("tdf171433_equation.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Without the fix, the formula was completely broken - missing SubSup entirely
    assertXPath(pXmlDoc, "//m:sSubSup/m:sup", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtPictureDataBinding)
{
    // Given a DOCX with a picture content control (w:picture + w:dataBinding)
    // where the data binding resolves to base64 image data from custom XML.
    // The sdtContent has a w:drawing with the actual rendered image.
    // Without the fix, the drawing was replaced by the raw base64 text.
    createSwDoc("sdt-picture-databinding.docx");

    // The first paragraph should contain a graphic, not text
    CPPUNIT_ASSERT_EQUAL(1, getShapes());
}

CPPUNIT_TEST_FIXTURE(Test, testCool15788_symbolContentControl)
{
    createSwDoc("Cool15788_symbolContentControl.docx");

    save(TestFilter::DOCX);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // this must not exist, otherwise Word complains about the file
    // <w:sym w:font="Wingdings" w:char="f04b"/>
    CPPUNIT_ASSERT_EQUAL(
        0, countXPathNodes(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:sym"));
    // simply a <w:t> element with the symbol must exist instead
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r[2]/w:t", u"\xf04b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146973_dateFieldNumberFormatLocale)
{
    // Given an en-US document with a fixed DATE field whose number format is pt-BR:
    createSwDoc();

    uno::Reference<util::XNumberFormatsSupplier> xNumberSupplier(mxComponent, uno::UNO_QUERY_THROW);
    lang::Locale aUSLocale;
    aUSLocale.Language = "en";
    aUSLocale.Country = "US";
    lang::Locale aBRLocale;
    aBRLocale.Language = "pt";
    aBRLocale.Country = "BR";
    sal_Int32 nFormat = xNumberSupplier->getNumberFormats()->addNewConverted(
        u"d\" de \"MMMM\" de \"yyyy"_ustr, aUSLocale, aBRLocale);

    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.DateTime"_ustr), uno::UNO_QUERY);
    xField->setPropertyValue(u"IsDate"_ustr, uno::Any(true));
    xField->setPropertyValue(u"IsFixed"_ustr, uno::Any(true));
    xField->setPropertyValue(u"NumberFormat"_ustr, uno::Any(nFormat));

    uno::Reference<text::XTextDocument> xDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When exporting to DOCX:
    saveAndReload(TestFilter::DOCX);

    // Then the field run must carry the number-format locale (pt-BR), not the ambient en-US.
    // Without the fix the run kept en-US, so the month name reverted to English on reopen.
    assertXPath(parseExport(u"word/document.xml"_ustr), "//w:r[w:instrText]/w:rPr/w:lang", "val",
                u"pt-BR");

    // ... and reopening the file must give the field its pt-BR format back.
    uno::Reference<text::XTextFieldsSupplier> xTextFields(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xFields(
        xTextFields->getTextFields()->createEnumeration());
    uno::Reference<beans::XPropertySet> xReloaded(xFields->nextElement(), uno::UNO_QUERY_THROW);
    uno::Reference<util::XNumberFormatsSupplier> xReloadedNumbers(mxComponent,
                                                                  uno::UNO_QUERY_THROW);
    lang::Locale aLocale;
    xReloadedNumbers->getNumberFormats()
        ->getByKey(xReloaded->getPropertyValue(u"NumberFormat"_ustr).get<sal_Int32>())
        ->getPropertyValue(u"Locale"_ustr)
        >>= aLocale;
    CPPUNIT_ASSERT_EQUAL(u"pt"_ustr, aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(u"BR"_ustr, aLocale.Country);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146973_perFieldDateLocale)
{
    // Word file: two date fields in one paragraph - English (explicit en-US) and
    // Ukrainian (inherited from the uk-UA document default). Each field's language
    // must survive a roundtrip instead of both collapsing to the document default.
    createSwDoc("tdf146973_signatureBlock.docx");
    save(TestFilter::DOCX);
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // Without the import fix both fields imported with the document locale, so the
    // English field's date reverted to Ukrainian (a single locale for both).
    assertXPath(pXmlDoc, "(//w:r[w:instrText])[1]/w:rPr/w:lang", "val", u"en-US");
    assertXPath(pXmlDoc, "(//w:r[w:instrText])[2]/w:rPr/w:lang", "val", u"uk-UA");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf146973_rtlDateLocale)
{
    // The runs of this Arabic date field only set a complex script language (w:lang w:bidi),
    // while the western language of the document defaults to en-GB.
    createSwDoc("tdf146973_arabicDateField.docx");
    saveAndReload(TestFilter::DOCX);

    uno::Reference<text::XTextFieldsSupplier> xTextFields(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<container::XEnumeration> xFields(
        xTextFields->getTextFields()->createEnumeration());
    uno::Reference<beans::XPropertySet> xField(xFields->nextElement(), uno::UNO_QUERY_THROW);
    sal_Int32 nFormat = 0;
    xField->getPropertyValue(u"NumberFormat"_ustr) >>= nFormat;

    uno::Reference<util::XNumberFormatsSupplier> xNumberSupplier(mxComponent, uno::UNO_QUERY_THROW);
    lang::Locale aLocale;
    xNumberSupplier->getNumberFormats()->getByKey(nFormat)->getPropertyValue(u"Locale"_ustr)
        >>= aLocale;

    // Without the fix this was en-GB, so the date got English month names.
    CPPUNIT_ASSERT_EQUAL(u"ar"_ustr, aLocale.Language);
    CPPUNIT_ASSERT_EQUAL(u"AE"_ustr, aLocale.Country);

    // Arabic is a complex script language, so it must be exported as w:bidi.
    // Without the fix the run's western language was set to Arabic instead.
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "(//w:r[w:instrText])[1]/w:rPr/w:lang", "bidi", u"ar-AE");
    assertXPathNoAttribute(pXmlDoc, "(//w:r[w:instrText])[1]/w:rPr/w:lang", "val");
}

} // end of anonymous namespace
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
