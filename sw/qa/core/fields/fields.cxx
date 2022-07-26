/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>

#include <comphelper/propertyvalue.hxx>

#include <authfld.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <wrtsh.hxx>
#include <ndtxt.hxx>

namespace
{
/// Covers sw/source/core/fields/ fixes.
class Test : public SwModelTestBase
{
};

constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/fields/data/";

CPPUNIT_TEST_FIXTURE(Test, testAuthorityTooltip)
{
    // Create a document with a bibliography reference in it.
    SwDoc* pDoc = createSwDoc();
    uno::Reference<lang::XMultiServiceFactory> xFactory(mxComponent, uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xField(
        xFactory->createInstance("com.sun.star.text.TextField.Bibliography"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aFields = {
        comphelper::makePropertyValue("Identifier", OUString("ARJ00")),
        comphelper::makePropertyValue("Author", OUString("Ar, J")),
        comphelper::makePropertyValue("Title", OUString("mytitle")),
        comphelper::makePropertyValue("Year", OUString("2020")),
    };
    xField->setPropertyValue("Fields", uno::Any(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Get the tooltip of the field.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    SwPaM* pCursor = pWrtShell->GetCursor();
    auto pField = dynamic_cast<SwAuthorityField*>(
        SwCursorShell::GetFieldAtCursor(pCursor, /*bIncludeInputFieldAtStart=*/true));
    CPPUNIT_ASSERT(pField);
    SwTextNode* pTextNode = pCursor->GetNode().GetTextNode();
    const SwTextAttr* pTextAttr = pTextNode->GetSwpHints().Get(0);
    const SwRootFrame* pLayout = pWrtShell->GetLayout();
    OUString aTooltip = pField->GetAuthority(pTextAttr, pLayout);

    // Without the accompanying fix in place, generating this tooltip text was not possible without
    // first inserting an empty bibliography table into the document.
    CPPUNIT_ASSERT_EQUAL(OUString("ARJ00: Ar, J, mytitle, 2020"), aTooltip);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143424)
{
    createSwDoc(DATA_DIRECTORY, "tdf143424.odt");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // TODO: I have no idea why fields are enumerated in invalid order, not like in document

    // Field: Chapter Format: Chapter name
    uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Another title"), xField->getPresentation(false));

    // Field: Chapter Format: Chapter number and name
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 2 - Another title"), xField->getPresentation(false));

    // Field: Chapter Format: Chapter number
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("Chapter 2 -"), xField->getPresentation(false));

    // Field: Chapter Format: Chapter number without separator
    xField.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(OUString("2"), xField->getPresentation(false));
}

CPPUNIT_TEST_FIXTURE(Test, testChapterFieldsFollowedBy)
{
    createSwDoc(DATA_DIRECTORY, "chapter_field_followedby.odt");

    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());

    // TODO: I have no idea why fields are enumerated in invalid order, not like in document
    std::vector<OUString> aFieldValues = {
        "Followed by tab", // #1
        "I.I.I.I", // #16
        ">I.I.I.I< Followed by newline", // #15 Linefeed is replaced by space
        ">I.I.I.I<", // #14
        "Followed by newline", // #13
        "I.I.I", // #12
        ">I.I.I<Followed by nothing", // #11 Nothing between text & outline
        ">I.I.I<", // #10
        "Followed by nothing", // #9
        "I.I", // #8
        ">I.I< Followed by space", // #7 Space as is
        ">I.I<", // #6
        "Followed by space", // #5
        "I", // #4
        ">I< Followed by tab", // #3 Here is a tab, but replaced by space in field
        ">I<", // #2
    };

    for (const auto& sValue : aFieldValues)
    {
        CPPUNIT_ASSERT(xFields->hasMoreElements());
        uno::Reference<text::XTextField> xField(xFields->nextElement(), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(sValue, xField->getPresentation(false));
    }
}
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
