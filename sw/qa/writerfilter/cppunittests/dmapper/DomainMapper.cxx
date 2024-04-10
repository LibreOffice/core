/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>

#include <tools/UnitConversion.hxx>

using namespace ::com::sun::star;

namespace
{
/// Tests for sw/source/writerfilter/dmapper/DomainMapper.cxx.
class Test : public UnoApiTest
{
public:
    Test()
        : UnoApiTest("/sw/qa/writerfilter/cppunittests/dmapper/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testLargeParaTopMargin)
{
    // Given a document with a paragraph with a large "before" spacing.
    loadFromFile(u"large-para-top-margin.docx");

    // When checking the first paragraph.
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<beans::XPropertySet> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);

    // Then assert its top margin.
    sal_Int32 nParaTopMargin{};
    xPara->getPropertyValue("ParaTopMargin") >>= nParaTopMargin;
    // <w:spacing w:before="37050"/> in the document.
    sal_Int32 nExpected = convertTwipToMm100(37050);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 65352
    // - Actual  : 0
    // i.e. the paragraph margin was lost, which shifted the paragraph to the right (no top margin
    // -> wrap around a TextBox), which shifted the triangle shape out of the page frame.
    CPPUNIT_ASSERT_EQUAL(nExpected, nParaTopMargin);
}

CPPUNIT_TEST_FIXTURE(Test, testSdtRunInPara)
{
    // Given a document with a block SDT, and inside that some content + a run SDT:
    loadFromFile(u"sdt-run-in-para.docx");

    // Then make sure the content inside the block SDT but outside the run SDT is not lost:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<text::XTextRange> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: first-second
    // - Actual  : second
    // i.e. the block-SDT-only string was lost.
    CPPUNIT_ASSERT_EQUAL(OUString("first-second"), xPara->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtDropdownNoDisplayText)
{
    // Given a document with <w:listItem w:value="..."/> (no display text):
    loadFromFile(u"sdt-dropdown-no-display-text.docx");

    // Then make sure we create a dropdown content control, not a rich text one:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParagraphsAccess(xTextDocument->getText(),
                                                                    uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParagraphs = xParagraphsAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xParagraph(xParagraphs->nextElement(),
                                                             uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortions = xParagraph->createEnumeration();
    uno::Reference<beans::XPropertySet> xTextPortion(xPortions->nextElement(), uno::UNO_QUERY);
    OUString aPortionType;
    xTextPortion->getPropertyValue("TextPortionType") >>= aPortionType;
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aPortionType);
    uno::Reference<text::XTextContent> xContentControl;
    xTextPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValues> aListItems;
    xContentControlProps->getPropertyValue("ListItems") >>= aListItems;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the list item was lost on import.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), aListItems.getLength());
}

CPPUNIT_TEST_FIXTURE(Test, testFloattableThenTable)
{
    // Given a document with an in-section floating table, followed by a table:
    // When laying out that document:
    loadFromFile(u"floattable-then-table.docx");

    // Then make sure that instead of crashing, the floating table is anchored inside the body text
    // (and not a cell):
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xBodyText = xTextDocument->getText();
    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<drawing::XDrawPage> xDrawPage = xDrawPageSupplier->getDrawPage();
    uno::Reference<text::XTextContent> xShape(xDrawPage->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xAnchor = xShape->getAnchor();
    // Make sure the anchor text is the body text, not some cell.
    CPPUNIT_ASSERT_EQUAL(xBodyText, xAnchor->getText());
}

CPPUNIT_TEST_FIXTURE(Test, testSdtBlockText)
{
    // Given a document with a block SDT that only contains text:
    loadFromFile(u"sdt-block-text.docx");

    // Then make sure that the text inside the SDT is imported as a content control:
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xPara(xParaEnum->nextElement(), uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xPortionEnum = xPara->createEnumeration();
    uno::Reference<beans::XPropertySet> xPortion(xPortionEnum->nextElement(), uno::UNO_QUERY);
    OUString aTextPortionType;
    xPortion->getPropertyValue("TextPortionType") >>= aTextPortionType;
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: ContentControl
    // - Actual  : TextField
    // i.e. the SDT was imported as a text field, not as a content control.
    CPPUNIT_ASSERT_EQUAL(OUString("ContentControl"), aTextPortionType);

    // Make sure the properties are imported
    uno::Reference<text::XTextContent> xContentControl;
    xPortion->getPropertyValue("ContentControl") >>= xContentControl;
    uno::Reference<beans::XPropertySet> xContentControlProps(xContentControl, uno::UNO_QUERY);
    OUString aAlias;
    xContentControlProps->getPropertyValue("Alias") >>= aAlias;
    CPPUNIT_ASSERT_EQUAL(OUString("myalias"), aAlias);
}

CPPUNIT_TEST_FIXTURE(Test, testFdo78333)
{
    // just care that it doesn't crash/assert
    loadFromFile(u"fdo78333-1-minimized.docx");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf158360)
{
    // just test that doc with annotation in TOC doesn't crash/assert
    loadFromFile(u"tdf158360.docx");
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
