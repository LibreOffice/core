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
    xField.set(xFactory->createInstance(u"com.sun.star.text.TextField.Bibliography"_ustr),
               uno::UNO_QUERY);
    aFields = {
        comphelper::makePropertyValue(u"BibiliographicType"_ustr, text::BibliographyDataType::WWW),
        comphelper::makePropertyValue(u"Identifier"_ustr, u"AT"_ustr),
        comphelper::makePropertyValue(u"Author"_ustr, u"Author"_ustr),
        comphelper::makePropertyValue(u"Title"_ustr, u"Title"_ustr),
        comphelper::makePropertyValue(u"URL"_ustr, u"http://www.example.com/test.pdf#page=2"_ustr),
    };
    xField->setPropertyValue(u"Fields"_ustr, uno::Any(aFields));
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
    OUString aNewUrl = u"http://www.example.com/test.pdf#page=42"_ustr;
    aCoreFields[AUTH_FIELD_URL] = aNewUrl;
    OUStringBuffer aFieldBuffer;
    for (const auto& rField : aCoreFields)
    {
        aFieldBuffer.append(rField + OUStringChar(TOX_STYLE_DELIMITER));
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
        comphelper::makePropertyValue(u"TypeName"_ustr, uno::Any(u"SetRef"_ustr)),
        comphelper::makePropertyValue(u"Name"_ustr,
                                      uno::Any(u"ZOTERO_ITEM CSL_CITATION {} RNDpyJknp173F"_ustr)),
        comphelper::makePropertyValue(u"Content"_ustr, uno::Any(u"aaa<b>bbb</b>ccc"_ustr)),
    };
    dispatchCommand(mxComponent, u".uno:InsertField"_ustr, aArgs);

    // Then make sure that we create a refmark that covers that text:
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    std::vector<SwTextAttr*> aAttrs = pTextNode->GetTextAttrsAt(0, RES_TXTATR_REFMARK);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. no refmark was created, only the hard to read Type=12 created a refmark.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), aAttrs.size());
    CPPUNIT_ASSERT_EQUAL(u"aaabbbccc"_ustr, pTextNode->GetText());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf68364InsertConditionalFieldWithTwoDots)
{
    // Create an empty document
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();

    // Insert a conditional field containing exactly two dots for its condition
    SwFieldMgr aFieldMgr(pWrtShell);
    SwInsertField_Data aFieldData(SwFieldTypesEnum::ConditionalText, 0, u"true"_ustr,
                                  u"19.12.2023"_ustr, 0);
    CPPUNIT_ASSERT(aFieldMgr.InsertField(aFieldData));
    pWrtShell->SttEndDoc(true);

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 19.12.2023
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(u"19.12.2023"_ustr, pWrtShell->GetCurField()->ExpandField(true, nullptr));
}

CPPUNIT_TEST_FIXTURE(Test, testInsertRefmarkSelection)
{
    // Given a document with a single selected word:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Insert2(u"myword"_ustr);
    pWrtShell->SelAll();

    // When inserting a refmark:
    SwFieldMgr aMgr(pWrtShell);
    SwInsertField_Data aData(SwFieldTypesEnum::SetRef, /*nSubType=*/0, u"myname"_ustr,
                             u"myword"_ustr,
                             /*nFormatId=*/0);
    aMgr.InsertField(aData);

    // Then make sure the document still just contains that word only once:
    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: myword
    // - Actual  : mywordmyword
    // i.e. the content of the selection was duplicated.
    CPPUNIT_ASSERT_EQUAL(u"myword"_ustr, pTextNode->GetText());
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
