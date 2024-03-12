/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>

#include <docary.hxx>
#include <docsh.hxx>
#include <frmmgr.hxx>
#include <wrtsh.hxx>
#include <formatflysplit.hxx>
#include <view.hxx>
#include <cmdid.h>

namespace
{
/// Covers sw/source/uibase/shells/textsh.cxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/uibase/shells/data/")
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testDeleteSections)
{
    // Given a document with a section:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    uno::Sequence<css::beans::PropertyValue> aArgs = {
        comphelper::makePropertyValue("RegionName",
                                      uno::Any(OUString("ZOTERO_BIBL {} CSL_BIBLIOGRAPHY RND"))),
        comphelper::makePropertyValue("Content", uno::Any(OUString("old content"))),
    };
    dispatchCommand(mxComponent, ".uno:InsertSection", aArgs);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetSections().size());

    // When deleting sections:
    std::vector<beans::PropertyValue> aArgsVec = comphelper::JsonToPropertyValues(R"json(
{
    "SectionNamePrefix": {
        "type": "string",
        "value": "ZOTERO_BIBL"
    }
}
)json"_ostr);
    aArgs = comphelper::containerToSequence(aArgsVec);
    dispatchCommand(mxComponent, ".uno:DeleteSections", aArgs);

    // Then make sure that the section is deleted:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 0
    // - Actual  : 1
    // i.e. the section was not deleted.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), pDoc->GetSections().size());
}

CPPUNIT_TEST_FIXTURE(Test, testSplitFlyFootnoteUI)
{
    // Given a document with a split fly (to host a table):
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    SwFlyFrameAttrMgr aMgr(true, pWrtShell, Frmmgr_Type::TEXT, nullptr);
    RndStdIds eAnchor = RndStdIds::FLY_AT_PARA;
    pWrtShell->StartAllAction();
    aMgr.InsertFlyFrame(eAnchor, aMgr.GetPos(), aMgr.GetSize());
    pWrtShell->EndAllAction();
    pWrtShell->StartAllAction();
    sw::FrameFormats<sw::SpzFrameFormat*>& rFlys = *pDoc->GetSpzFrameFormats();
    sw::SpzFrameFormat* pFly = rFlys[0];
    {
        SwAttrSet aSet(pFly->GetAttrSet());
        aSet.Put(SwFormatFlySplit(true));
        pDoc->SetAttr(aSet, *pFly);
    }
    pWrtShell->EndAllAction();
    pWrtShell->UnSelectFrame();
    pWrtShell->LeaveSelFrameMode();
    pWrtShell->GetView().TriggerAttrChangedNotify();
    pWrtShell->MoveSection(GoCurrSection, fnSectionEnd);

    // When checking if we can insert a footnote inside the split fly:
    SwView& rView = pWrtShell->GetView();
    std::unique_ptr<SfxPoolItem> pItem;
    SfxItemState eState = rView.GetViewFrame().GetBindings().QueryState(FN_INSERT_FOOTNOTE, pItem);

    // Then make sure that the insertion is allowed:
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 32 (DEFAULT)
    // - Actual  : 1 (DISABLED)
    // i.e. the insertion was denied.
    CPPUNIT_ASSERT_EQUAL(SfxItemState::DEFAULT, eState);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
