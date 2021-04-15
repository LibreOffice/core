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
    xField->setPropertyValue("Fields", uno::makeAny(aFields));
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<text::XText> xText = xTextDocument->getText();
    uno::Reference<text::XTextCursor> xCursor = xText->createTextCursor();
    uno::Reference<text::XTextContent> xContent(xField, uno::UNO_QUERY);
    xText->insertTextContent(xCursor, xContent, /*bAbsorb=*/false);

    // Get the tooltip of the field.
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Left(CRSR_SKIP_CHARS, /*bSelect=*/false, 1, /*bBasicCall=*/false);
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
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
