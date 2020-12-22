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
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>

using namespace ::com::sun::star;

namespace
{
/// Tests for writerfilter/source/dmapper/DomainMapper_Impl.cxx.
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

char const DATA_DIRECTORY[] = "/writerfilter/qa/cppunittests/dmapper/data/";

CPPUNIT_TEST_FIXTURE(Test, testPageBreakFooterTable)
{
    // Load a document which refers to a footer which ends with a table, and there is a page break
    // in the body text right after the footer reference.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "page-break-footer-table.docx";
    getComponent() = loadFromDesktop(aURL);

    // Check the last paragraph.
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara;
    while (xParaEnum->hasMoreElements())
    {
        xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    }
    style::BreakType eType = style::BreakType_NONE;
    xPara->getPropertyValue("BreakType") >>= eType;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 4
    // - Actual  : 0
    // i.e. there was no page break before the last paragraph.
    CPPUNIT_ASSERT_EQUAL(style::BreakType_PAGE_BEFORE, eType);
}

CPPUNIT_TEST_FIXTURE(Test, testNumberingRestartStyleParent)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "num-restart-style-parent.docx";
    getComponent() = loadFromDesktop(aURL);

    // The paragraphs are A 1 2 B 1 2.
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara;
    static OUStringLiteral aProp(u"ListLabelString");
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("A."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("1."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("B."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1.
    // - Actual  : 3.
    // i.e. the numbering was not restarted after B.
    CPPUNIT_ASSERT_EQUAL(OUString("1."), xPara->getPropertyValue(aProp).get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2."), xPara->getPropertyValue(aProp).get<OUString>());
}

CPPUNIT_TEST_FIXTURE(Test, testFrameDirection)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "frame-direction.docx";
    getComponent() = loadFromDesktop(aURL);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<beans::XPropertySet> xFrame0(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame1(xDrawPage->getByIndex(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xFrame2(xDrawPage->getByIndex(2), uno::UNO_QUERY);
    // Without the accompanying fix in place, all of the following values would be text::WritingMode2::CONTEXT
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::CONTEXT,
                         xFrame0->getPropertyValue("WritingMode").get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::BT_LR,
                         xFrame1->getPropertyValue("WritingMode").get<sal_Int16>());
    CPPUNIT_ASSERT_EQUAL(text::WritingMode2::TB_RL,
                         xFrame2->getPropertyValue("WritingMode").get<sal_Int16>());
}

CPPUNIT_TEST_FIXTURE(Test, testAltChunk)
{
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "alt-chunk.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<text::XTextDocument> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara;
    uno::Reference<beans::XPropertySet> xParaProps;
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xParaProps.set(xPara, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("outer, before sect break"), xPara->getString());
    CPPUNIT_ASSERT_EQUAL(OUString("Standard"),
                         xParaProps->getPropertyValue("PageStyleName").get<OUString>());
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    xParaProps.set(xPara, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("outer, after sect break"), xPara->getString());

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: Converted1
    // - Actual  : Standard
    // i.e. the page break between the first and the second paragraph was missing.
    CPPUNIT_ASSERT_EQUAL(OUString("Converted1"),
                         xParaProps->getPropertyValue("PageStyleName").get<OUString>());

    // Without the accompanying fix in place, this test would have failed with a
    // container.NoSuchElementException, as the document had only 2 paragraphs, all the "inner"
    // content was lost.
    xPara.set(xParaEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("inner doc, first para"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testFieldIfInsideIf)
{
    // Load a document with a field in a table cell: it contains an IF field with various nested
    // fields.
    OUString aURL = m_directories.getURLFromSrc(DATA_DIRECTORY) + "field-if-inside-if.docx";
    getComponent() = loadFromDesktop(aURL);
    uno::Reference<text::XTextTablesSupplier> xTextDocument(getComponent(), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextDocument->getTextTables(), uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    // Get the result of the topmost field.
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName("A1"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 0** Expression is faulty **2
    // i.e. some of the inner fields escaped outside the outer field.
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xCell->getString());

    // Test the second cell: it contains "IF ", not the usual " IF ".
    xCell.set(xTable->getCellByName("A2"), uno::UNO_QUERY);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 25
    // - Actual  : 025
    // i.e. some of the inner fields escaped outside the outer field.
    CPPUNIT_ASSERT_EQUAL(OUString("25"), xCell->getString());
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
