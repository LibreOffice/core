/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <comphelper/propertyvalue.hxx>

#include <IDocumentFieldsAccess.hxx>
#include <authfld.hxx>
#include <fmtfld.hxx>

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
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"ARJ00"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Ar, J"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"mytitle"_ustr),
        comphelper::makePropertyValue(u"Year"_ustr, u"2020"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    // Create a bibliography table.
    uno::Reference<text::XTextContent> xTable(
        xFactory->createInstance(u"com.sun.star.text.Bibliography"_ustr), uno::UNO_QUERY);
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xTable, /*bAbsorb=*/false);

    // Update it.
    uno::Reference<text::XDocumentIndex> xTableIndex(xTable, uno::UNO_QUERY);
    xTableIndex->update();

    // Paragraph index: Reference, table header, table row.
    // Portion index: ID, etc; then the URL.
    auto aActual = getProperty<OUString>(getRun(getParagraph(3), 2), u"HyperLinkURL"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.container.NoSuchElementException
    // i.e. the URL was not clickable and the table row was a single text portion.
    CPPUNIT_ASSERT_EQUAL(u"http://www.example.com/test.pdf"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTableEntryURL)
{
    // Given a document with a bibliography reference (of type WWW) in it:
    createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf#page=1"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    // Create a bibliography table.
    uno::Reference<text::XTextContent> xTable(
        xFactory->createInstance(u"com.sun.star.text.Bibliography"_ustr), uno::UNO_QUERY);
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
    auto aActual = getProperty<OUString>(getRun(getParagraph(3), 2), u"HyperLinkURL"_ustr);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: http://www.example.com/test.pdf
    // - Actual  : http://www.example.com/test.pdf#page=1
    // i.e. the page number was still part of the bibliography table.
    CPPUNIT_ASSERT_EQUAL(u"http://www.example.com/test.pdf"_ustr, aActual);
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTableEntryClick)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a biblio entry field with an URL:
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf#page=1"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Then make sure that the field is clickable, since the page part will not be part of the
    // bibliography table:
    const SwFieldTypes* pTypes = pDoc->getIDocumentFieldsAccess().GetFieldTypes();
    auto it = std::find_if(pTypes->begin(), pTypes->end(),
                           [](const std::unique_ptr<SwFieldType>& pType) {
                               return pType->Which() == SwFieldIds::TableOfAuthorities;
                           });
    CPPUNIT_ASSERT(it != pTypes->end());
    const SwFieldType* pType = it->get();
    std::vector<SwFormatField*> aFormatFields;
    pType->GatherFields(aFormatFields);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aFormatFields.size());
    SwField* pField = aFormatFields[0]->GetField();
    // Without the accompanying fix in place, this test would have failed, as the field was not
    // clickable.
    CPPUNIT_ASSERT(pField->IsClickable());
    // This is needed, so the mouse has the correct RefHand pointer style.
    CPPUNIT_ASSERT(pField->HasClickHdl());
}

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTableEntryRelClick)
{
    // Given an empty document with a file:// base URL:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Reference<frame::XStorable> xStorable(mxComponent, uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue(u"FilterName"_ustr, u"writer8"_ustr),
    };
    xStorable->storeAsURL(maTempFile.GetURL(), aArgs);

    // When inserting a biblio entry field with a relative URL:
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"test.pdf#page=1"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Then make sure that the field is clickable:
    const SwFieldTypes* pTypes = pDoc->getIDocumentFieldsAccess().GetFieldTypes();
    auto it = std::find_if(pTypes->begin(), pTypes->end(),
                           [](const std::unique_ptr<SwFieldType>& pType) {
                               return pType->Which() == SwFieldIds::TableOfAuthorities;
                           });
    CPPUNIT_ASSERT(it != pTypes->end());
    const SwFieldType* pType = it->get();
    std::vector<SwFormatField*> aFormatFields;
    pType->GatherFields(aFormatFields);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aFormatFields.size());
    auto pField = static_cast<SwAuthorityField*>(aFormatFields[0]->GetField());
    CPPUNIT_ASSERT(pField->GetAbsoluteURL().startsWith("file://"));
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
            xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr),
            uno::UNO_QUERY);
        uno::Sequence<beans::PropertyValue> aFields = {
            comphelper::makePropertyValue(u"BibiliographicType"_ustr,
                                          text::BibliographyDataType::WWW),
            comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
            comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
            comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
            comphelper::makePropertyValue(u"URL"_ustr, OUString(rURL)),
        };
        xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
        uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
        xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    }
    // Create a bibliography table.
    uno::Reference<text::XTextContent> xTable(
        xFactory->createInstance(u"com.sun.star.text.Bibliography"_ustr), uno::UNO_QUERY);
    xCursor->gotoEnd(/*bExpand=*/false);
    xText->insertControlCharacter(xCursor, text::ControlCharacter::APPEND_PARAGRAPH,
                                  /*bAbsorb=*/false);
    xText->insertTextContent(xCursor, xTable, /*bAbsorb=*/false);

    // When updating that table:
    uno::Reference<text::XDocumentIndex> xTableIndex(xTable, uno::UNO_QUERY);
    xTableIndex->update();

    // Then the first two fields should be merged to a single source, but not the third.
    CPPUNIT_ASSERT_EQUAL(u"AT: Author, Title, , http://www.example.com/test.pdf"_ustr,
                         getParagraph(3)->getString());
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: AT: Author, Title, , http://www.example.com/test2.pdf
    // - Actual  : AT: Author, Title, , http://www.example.com/test.pdf
    // i.e. test.pdf was mentioned twice, without deduplication.
    CPPUNIT_ASSERT_EQUAL(u"AT: Author, Title, , http://www.example.com/test2.pdf"_ustr,
                         getParagraph(4)->getString());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
