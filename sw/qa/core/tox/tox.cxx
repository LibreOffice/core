/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertyvalue.hxx>

namespace
{
/// Covers sw/source/core/tox/ fixes.
class Test : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testAuthorityLinkClick)
{
    // Create a document with a bibliography reference (of type WWW) in it.
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("ARJ00")),
        comphelper::makePropertyValue("Author", OUString("Ar, J")),
        comphelper::makePropertyValue("Title", OUString("mytitle")),
        comphelper::makePropertyValue("Year", OUString("2020")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf")),
    };
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    // Create a bibliography table.
    uno::Reference<text::XTextContent> xTable(
        xFactory->createInstance("com.sun.star.text.Bibliography"), uno::UNO_QUERY);
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xTable, /*bAbsorb=*/false);

    // Update it.
    uno::Reference<text::XDocumentIndex> xTableIndex(xTable, uno::UNO_QUERY);
    xTableIndex->update();

    // Paragraph index: Reference, table header, table row.
    // Portion index: ID, etc; then the URL.
    auto aActual = getProperty<OUString>(getRun(getParagraph(3), 2), "HyperLinkURL");
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.container.NoSuchElementException
    // i.e. the URL was not clickable and the table row was a single text portion.
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/test.pdf"), aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTableEntryURL)
{
    // Given a document with a bibliography reference (of type WWW) in it:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("Author", OUString("Author")),
        comphelper::makePropertyValue("Title", OUString("Title")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf#page=1")),
    };
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    // Create a bibliography table.
    uno::Reference<text::XTextContent> xTable(
        xFactory->createInstance("com.sun.star.text.Bibliography"), uno::UNO_QUERY);
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xTable, /*bAbsorb=*/false);

    // When updating that table:
    uno::Reference<text::XDocumentIndex> xTableIndex(xTable, uno::UNO_QUERY);
    xTableIndex->update();

    // Then the page number from the source's URL should be stripped:
    // Paragraph index: Reference, table header, table row.
    // Portion index: ID, etc; then the URL.
    auto aActual = getProperty<OUString>(getRun(getParagraph(3), 2), "HyperLinkURL");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: http://www.example.com/test.pdf
    // - Actual  : http://www.example.com/test.pdf#page=1
    // i.e. the page number was still part of the bibliography table.
    CPPUNIT_ASSERT_EQUAL(OUString("http://www.example.com/test.pdf"), aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTableURLDeduplication)
{
    // Given a document with 3 bibliography references (of type WWW) in it:
    static const std::initializer_list<std::u16string_view> aURLs = {
        u"http://www.example.com/test.pdf#page=1",
        u"http://www.example.com/test.pdf#page=2",
        u"http://www.example.com/test2.pdf",
    };
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    for (const auto& rURL : aURLs)
    {
        uno::Reference<beans::XPropertySet> xField(
            xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aFields = {
            comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
            comphelper::makePropertyValue("Identifier", OUString("AT")),
            comphelper::makePropertyValue("Author", OUString("Author")),
            comphelper::makePropertyValue("Title", OUString("Title")),
            comphelper::makePropertyValue("URL", OUString(rURL)),
        };
        xField->setPropertyValue("Fields", uno::makeAny(aFields));
        uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
        xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    }
    // Create a bibliography table.
    uno::Reference<text::XTextContent> xTable(
        xFactory->createInstance("com.sun.star.text.Bibliography"), uno::UNO_QUERY);
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xTable, /*bAbsorb=*/false);

    // When updating that table:
    uno::Reference<text::XDocumentIndex> xTableIndex(xTable, uno::UNO_QUERY);
    xTableIndex->update();

    // Then the first two fields should be merged to a single source, but not the third.
    CPPUNIT_ASSERT_EQUAL(OUString("AT: Author, Title, , http://www.example.com/test.pdf"),
                         getParagraph(3)->getString());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: AT: Author, Title, , http://www.example.com/test2.pdf
    // - Actual  : AT: Author, Title, , http://www.example.com/test.pdf
    // i.e. test.pdf was mentioned twice, without deduplication.
    CPPUNIT_ASSERT_EQUAL(OUString("AT: Author, Title, , http://www.example.com/test2.pdf"),
                         getParagraph(4)->getString());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
