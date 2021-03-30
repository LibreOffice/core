/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextFrame.hpp>

#include <wrtsh.hxx>
#include <unotextrange.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>

using namespace ::com::sun::star;

namespace
{
constexpr OUStringLiteral DATA_DIRECTORY = u"/sw/qa/core/unocore/data/";
}

/// Covers sw/source/core/unocore/ fixes.
class SwCoreUnocoreTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testTdf119081)
{
    // Load a doc with a nested table in it.
    load(DATA_DIRECTORY, "tdf119081.odt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();

    // Enter outer A1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/3);
    // Enter inner A1.
    pWrtShell->Right(CRSR_SKIP_CELLS, /*bSelect=*/false, /*nCount=*/1, /*bBasicCall=*/false,
                     /*bVisual=*/true);
    // Enter outer B1.
    pWrtShell->Down(/*bSelect=*/false, /*nCount=*/2);

    SwDoc* pDoc = pDocShell->GetDoc();
    SwPaM& rCursor = pWrtShell->GetCurrentShellCursor();
    uno::Reference<text::XTextRange> xInsertPosition
        = SwXTextRange::CreateXTextRange(*pDoc, *rCursor.GetPoint(), nullptr);
    uno::Reference<text::XTextAppend> xTextAppend(xInsertPosition->getText(), uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // An uncaught exception of type com.sun.star.uno.RuntimeException
    xTextAppend->insertTextPortion("x", {}, xInsertPosition);

    // Verify that the string is indeed inserted.
    pWrtShell->Left(CRSR_SKIP_CELLS, /*bSelect=*/true, /*nCount=*/1, /*bBasicCall=*/false,
                    /*bVisual=*/true);
    CPPUNIT_ASSERT_EQUAL(OUString("x"), pWrtShell->GetCurrentShellCursor().GetText());
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, flyAtParaAnchor)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<lang::XMultiServiceFactory> const xMSF(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextDocument> const xTD(mxComponent, uno::UNO_QUERY_THROW);
    uno::Reference<text::XTextFrame> const xTextFrame(
        xMSF->createInstance("com.sun.star.text.TextFrame"), uno::UNO_QUERY_THROW);
    uno::Reference<beans::XPropertySet> const xFrameProps(xTextFrame, uno::UNO_QUERY_THROW);
    xFrameProps->setPropertyValue("AnchorType",
                                  uno::makeAny(text::TextContentAnchorType_AT_PARAGRAPH));
    auto const xText = xTD->getText();
    auto const xTextCursor = xText->createTextCursor();
    CPPUNIT_ASSERT(xTextCursor.is());
    xText->insertTextContent(xTextCursor, xTextFrame, false);
    auto const xAnchor = xTextFrame->getAnchor();
    uno::Reference<text::XTextContent> const xFieldmark(
        xMSF->createInstance("com.sun.star.text.Fieldmark"), uno::UNO_QUERY_THROW);
    // this crashed because the anchor didn't have SwIndex
    xText->insertTextContent(xAnchor, xFieldmark, false);
}

CPPUNIT_TEST_FIXTURE(SwCoreUnocoreTest, testRtlGutter)
{
    mxComponent = loadFromDesktop("private:factory/swriter", "com.sun.star.text.TextDocument");
    uno::Reference<beans::XPropertySet> xPageStyle(getStyles("PageStyles")->getByName("Standard"),
                                                   uno::UNO_QUERY);
    // Without the accompanying fix in place, this test would have failed with:
    // - Unknown property: RtlGutter
    auto bRtlGutter = getProperty<bool>(xPageStyle, "RtlGutter");
    CPPUNIT_ASSERT(!bRtlGutter);
    xPageStyle->setPropertyValue("RtlGutter", uno::makeAny(true));
    bRtlGutter = getProperty<bool>(xPageStyle, "RtlGutter");
    CPPUNIT_ASSERT(bRtlGutter);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
