/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>

#include <unotools/streamwrap.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/PropertyMap.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testFloatingTableHeader)
{
    loadFromFile(u"floating-table-header.docx");
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextViewCursorSupplier> xTextViewCursorSupplier(
        xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<text::XPageCursor> xCursor(xTextViewCursorSupplier->getViewCursor(),
                                              uno::UNO_QUERY);
    xCursor->jumpToLastPage();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 3
    // i.e. a document which is 1 page in Word was imported as a 3 page one.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int16>(1), xCursor->getPage());
}

// TODO - First Page Headers Support
CPPUNIT_TEST_FIXTURE(Test, testFollowPageTopMargin)
{
    // Load a document with 2 pages: first page has larger top margin, second page has smaller top
    // margin.
    loadFromFile(u"follow-page-top-margin.docx");
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue(u"TopMargin"_ustr).get<sal_Int32>();

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 250
    // - Actual  : 1249
    // i.e. the top margin on page 2 was too large.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1249), nTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testTableNegativeVerticalPos)
{
    // Given a document with a table which has a negative vertical position (moves up to overlap
    // with the header):
    loadFromFile(u"table-negative-vertical-pos.docx");

    // Then make sure we don't import that as a plain table, which can't have a negative top margin:
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. this was imported as a plain table, resulting in a 0 top margin (y pos too large).
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xDrawPage->getCount());
}

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorder)
{
    // Given a document with a top margin and a border which has more spacing than the margin:
    loadFromFile(u"negative-page-border.docx");

    // Then make sure that the border distance is negative, so it can appear at the correct
    // position:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue(u"TopMargin"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(501), nTopMargin);
    auto aTopBorder = xStyle->getPropertyValue(u"TopBorder"_ustr).get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance = xStyle->getPropertyValue(u"TopBorderDistance"_ustr).get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -646
    // - Actual  : 0
    // i.e. the border negative distance was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-646), nTopBorderDistance);
}

CPPUNIT_TEST_FIXTURE(Test, testNegativePageBorderNoMargin)
{
    // Given a document with no top margin and a border which has spacing:
    loadFromFile(u"negative-page-border-no-margin.docx");

    // Then make sure that the border distance is negative, so it can appear at the correct
    // position:
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    auto nTopMargin = xStyle->getPropertyValue(u"TopMargin"_ustr).get<sal_Int32>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(0), nTopMargin);
    auto aTopBorder = xStyle->getPropertyValue(u"TopBorder"_ustr).get<table::BorderLine2>();
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt32>(159), aTopBorder.LineWidth);
    auto nTopBorderDistance = xStyle->getPropertyValue(u"TopBorderDistance"_ustr).get<sal_Int32>();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: -1147
    // - Actual  : 0
    // i.e. the border negative distance was lost.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(-1147), nTopBorderDistance);
}

CPPUNIT_TEST_FIXTURE(Test, testPasteHeaderDisable)
{
    // Given an empty document with a turned on header:
    mxComponent
        = loadFromDesktop(u"private:factory/swriter"_ustr, u"com.sun.star.text.TextDocument"_ustr);
    uno::Reference<style::XStyleFamiliesSupplier> xStyleFamiliesSupplier(mxComponent,
                                                                         uno::UNO_QUERY);
    uno::Reference<container::XNameAccess> xStyleFamilies
        = xStyleFamiliesSupplier->getStyleFamilies();
    uno::Reference<container::XNameAccess> xStyleFamily(
        xStyleFamilies->getByName(u"PageStyles"_ustr), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xStyle(xStyleFamily->getByName(u"Standard"_ustr),
                                               uno::UNO_QUERY);
    xStyle->setPropertyValue(u"HeaderIsOn"_ustr, uno::Any(true));

    // When pasting RTF content:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xText = xTextDocument->getText();
    uno::Reference<text::XTextRange> xBodyEnd = xText->getEnd();
    uno::Reference<document::XFilter> xFilter(
        m_xSFactory->createInstance(u"com.sun.star.comp.Writer.RtfFilter"_ustr), uno::UNO_QUERY);
    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY);
    xImporter->setTargetDocument(mxComponent);
    std::unique_ptr<SvStream> pStream(new SvMemoryStream);
    pStream->WriteOString("{\\rtf1 paste}");
    pStream->Seek(0);
    uno::Reference<io::XStream> xStream(new utl::OStreamWrapper(std::move(pStream)));
    uno::Sequence aDescriptor{ comphelper::makePropertyValue(u"InputStream"_ustr, xStream),
                               comphelper::makePropertyValue(u"InsertMode"_ustr, true),
                               comphelper::makePropertyValue(u"TextInsertModeRange"_ustr,
                                                             xBodyEnd) };
    CPPUNIT_ASSERT(xFilter->filter(aDescriptor));

    // Then make sure the header stays on:
    CPPUNIT_ASSERT(xStyle->getPropertyValue(u"HeaderIsOn"_ustr).get<bool>());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
