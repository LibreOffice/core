/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <docsh.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <textboxhelper.hxx>

#include <editeng/adjustitem.hxx>

char const DATA_DIRECTORY[] = "/sw/qa/core/edit/data/";

namespace
{
/// Covers sw/source/core/edit/ fixes.
class Test : public SwModelTestBase
{
public:
    SwDoc* createSwDoc(const char* pName)
    {
        if (!pName)
            loadURL("private:factory/swriter", nullptr);
        else
            load(DATA_DIRECTORY, pName);

        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);
        return pTextDoc->GetDocShell()->GetDoc();
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineHidden)
{
    // Given a document with ShowRedlineChanges=false:
    SwDoc* pDoc = createSwDoc("redline-hidden.fodt");

    // When formatting a paragraph by setting the para adjust to center, then make sure setting the
    // new item set on the paragraph doesn't crash:
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxItemSet aSet(pView->GetPool(), svl::Items<RES_PARATR_ADJUST, RES_PARATR_ADJUST>{});
    SvxAdjustItem aItem(SvxAdjust::Center, RES_PARATR_ADJUST);
    aSet.Put(aItem);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SetAttrSet(aSet, SetAttrMode::DEFAULT, nullptr, true);
}

CPPUNIT_TEST_FIXTURE(Test, testTextboxUndoOrdNum)
{
    // Given a document with 5 frame formats:
    // - picture
    // - draw format + fly format and a picture in it
    // - picture
    SwDoc* pDoc = createSwDoc("textbox-undo-ordnum.docx");
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    const SwFrameFormats& rFormats = *pDoc->GetSpzFrameFormats();
    // Test the state before del + undo.
    for (const auto& pFormat : rFormats)
    {
        const SwFrameFormat* pFlyFormat
            = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT);
        if (!pFlyFormat)
        {
            continue;
        }

        sal_Int32 nDrawOrdNum = pFormat->FindRealSdrObject()->GetOrdNum();
        sal_Int32 nFlyOrdNum = pFlyFormat->FindRealSdrObject()->GetOrdNum();
        CPPUNIT_ASSERT_EQUAL(nDrawOrdNum + 1, nFlyOrdNum);
    }

    // When selecting the first page, deleting the selection and undoing:
    pWrtShell->Down(true, 3);
    pWrtShell->DelLeft();
    pWrtShell->Undo();

    // Then the z-order of the fly format should be still the z-order of the draw format + 1, when
    // the fly and draw formats form a textbox pair.
    for (const auto& pFormat : rFormats)
    {
        const SwFrameFormat* pFlyFormat
            = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT);
        if (!pFlyFormat)
        {
            continue;
        }

        sal_Int32 nDrawOrdNum = pFormat->FindRealSdrObject()->GetOrdNum();
        sal_Int32 nFlyOrdNum = pFlyFormat->FindRealSdrObject()->GetOrdNum();
        // Without the accompanying fix in place, this test would have failed with:
        // - Expected: 4
        // - Actual  : 2
        // i.e. the fly format was behind the draw format, not visible.
        CPPUNIT_ASSERT_EQUAL(nDrawOrdNum + 1, nFlyOrdNum);
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
