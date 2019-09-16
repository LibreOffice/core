/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <comphelper/propertyvalue.hxx>

namespace
{
char const DATA_DIRECTORY[] = "/sw/qa/extras/uiwriter/data2/";
}

/// Second set of tests asserting the behavior of Writer user interface shells.
class SwUiWriterTest2 : public SwModelTestBase
{
public:
    void testTdf101534();

    CPPUNIT_TEST_SUITE(SwUiWriterTest2);
    CPPUNIT_TEST(testTdf101534);
    CPPUNIT_TEST_SUITE_END();
};

void SwUiWriterTest2::testTdf101534()
{
    // Copy the first paragraph of the document.
    load(DATA_DIRECTORY, "tdf101534.fodt");
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->EndPara(/*bSelect=*/true);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Copy();

    // Go to the second paragraph, assert that we have margins as direct
    // formatting.
    pWrtShell->Down(/*bSelect=*/false);
    SfxItemSet aSet(pWrtShell->GetAttrPool(), svl::Items<RES_LR_SPACE, RES_LR_SPACE>{});
    pWrtShell->GetCurAttr(aSet);
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));

    // Make sure that direct formatting is preserved during paste.
    pWrtShell->EndPara(/*bSelect=*/false);
    TransferableDataHelper aHelper(pTransfer.get());
    SwTransferable::Paste(*pWrtShell, aHelper);
    pWrtShell->GetCurAttr(aSet);
    // This failed, direct formatting was lost.
    CPPUNIT_ASSERT(aSet.HasItem(RES_LR_SPACE));
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwUiWriterTest2);

CPPUNIT_TEST_FIXTURE(SwUiWriterTest2, testTdf105330)
{
    load(DATA_DIRECTORY, "tdf105330.odt");

    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);

    SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetWrtShell();
    pWrtShell->Down(/*bSelect=*/false);
    uno::Sequence<beans::PropertyValue> aTableArgs = {
        comphelper::makePropertyValue("Rows", static_cast<sal_uInt16>(1)),
        comphelper::makePropertyValue("Columns", static_cast<sal_uInt16>(1)),
    };
    lcl_dispatchCommand(mxComponent, ".uno:InsertTable", aTableArgs);

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    sw::UndoManager& rUndoManager = pDoc->GetUndoManager();
    rUndoManager.Undo();

    //  Without the accompanying fix in place, height was only 1 twips (practically invisible).
    CPPUNIT_ASSERT_EQUAL(static_cast<long>(276),
                         pWrtShell->GetVisibleCursor()->GetTextCursor().GetSize().getHeight());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
