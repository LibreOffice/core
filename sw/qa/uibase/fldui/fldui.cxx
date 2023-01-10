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
#include <com/sun/star/text/XTextDocument.hpp>

#include <rtl/ustrbuf.hxx>
#include <comphelper/propertyvalue.hxx>

#include <docsh.hxx>
#include <wrtsh.hxx>
#include <fldmgr.hxx>
#include <authfld.hxx>
#include <ndtxt.hxx>

using namespace com::sun::star;

namespace
{
/// Covers sw/source/uibase/fldui/ fixes.
class Test : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(Test, testBiblioPageNumberUpdate)
{
    // Given a document with 2 biblio fields, same properties, but different page number in the URL:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
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
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);
    xField.set(xFactory->createInstance("com.sun.star.text.TextField.Bibliography"),
               uno::UNO_QUERY);
    aFields = {
        comphelper::makePropertyValue("BibiliographicType", text::BibliographyDataType::WWW),
        comphelper::makePropertyValue("Identifier", OUString("AT")),
        comphelper::makePropertyValue("Author", OUString("Author")),
        comphelper::makePropertyValue("Title", OUString("Title")),
        comphelper::makePropertyValue("URL", OUString("http://www.example.com/test.pdf#page=2")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    xContent.set(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // When changing the page number in the second field's URL:
    SwDocShell* pDocShell = pDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    OUString aCoreFields[AUTH_FIELD_END];
    aCoreFields[AUTH_FIELD_AUTHORITY_TYPE] = OUString::number(text::BibliographyDataType::WWW);
    aCoreFields[AUTH_FIELD_IDENTIFIER] = "AT";
    aCoreFields[AUTH_FIELD_AUTHOR] = "Author";
    aCoreFields[AUTH_FIELD_TITLE] = "Title";
    OUString aNewUrl = "http://www.example.com/test.pdf#page=42";
    aCoreFields[AUTH_FIELD_URL] = aNewUrl;
    OUStringBuffer aFieldBuffer;
    for (const auto& rField : aCoreFields)
    {
        aFieldBuffer.append(rField).append(TOX_STYLE_DELIMITER);
    }
    SwFieldMgr aMgr(pWrtShell);
    aMgr.UpdateCurField(0, aFieldBuffer.makeStringAndClear(), OUString());

    // Then make sure that the second field's URL is updated:
    auto pField = static_cast<SwAuthorityField*>(pWrtShell->GetCurField());
    const SwAuthEntry* pEntry = pField->GetAuthEntry();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: http://www.example.com/test.pdf#page=42
    // - Actual  : http://www.example.com/test.pdf#page=2
    // i.e. the second biblio field's URL was not updated.
    CPPUNIT_ASSERT_EQUAL(aNewUrl, pEntry->GetAuthorField(AUTH_FIELD_URL));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertRefmark)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();

    // When inserting a refmark with text:
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("TypeName", uno::Any(OUString("SetRef"))),
        comphelper::makePropertyValue(
            "Name", uno::Any(OUString("ZOTERO_ITEM CSL_CITATION {} RNDpyJknp173F"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("aaa<b>bbb</b>ccc"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertField", aArgs);

    // Then make sure that we create a refmark that covers that text:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    std::vector<SwTextAttr*> aAttrs = pTextNode->GetTextAttrsAt(0, RES_TXTATR_REFMARK);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no refmark was created, only the hard to read Type=12 created a refmark.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aAttrs.size());
    CPPUNIT_ASSERT_EQUAL(OUString("aaabbbccc"), pTextNode->GetText());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
