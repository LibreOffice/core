/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include <cmdid.h>
#include <view.hxx>
#include <docsh.hxx>

namespace
{
/// Covers sw/source/uibase/shells/basesh.hxx fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/uibase/shells/data/")
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testShowChangesStatus)
{
    // Given an empty document:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwView* pView = pDoc->GetDocShell()->GetView();

    // When showing changes inline:
    pView->GetViewFrame().GetDispatcher()->Execute(FN_SET_TRACKED_CHANGES_IN_TEXT,
                                                   SfxCallMode::SYNCHRON);

    // Then make sure the state of the 3 show modes are correct:
    std::unique_ptr<SfxPoolItem> pItem;
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_CHANGES_IN_TEXT, pItem);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 20267 (FN_SET_TRACKED_CHANGES_IN_TEXT)
    // - Actual  : 0
    // i.e. the status of these uno commands were not implemented.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_CHANGES_IN_TEXT), pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_DELETIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_DELETIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_INSERTIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_INSERTIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());

    // When showing deletions on the margin:
    pView->GetViewFrame().GetDispatcher()->Execute(FN_SET_TRACKED_DELETIONS_IN_MARGIN,
                                                   SfxCallMode::SYNCHRON);

    // Then make sure the state of the 3 show modes are correct:
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_CHANGES_IN_TEXT, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_CHANGES_IN_TEXT), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_DELETIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_DELETIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_INSERTIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_INSERTIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());

    // When showing insertions on the margin:
    pView->GetViewFrame().GetDispatcher()->Execute(FN_SET_TRACKED_INSERTIONS_IN_MARGIN,
                                                   SfxCallMode::SYNCHRON);

    // Then make sure the state of the 3 show modes are correct:
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_CHANGES_IN_TEXT, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_CHANGES_IN_TEXT), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_DELETIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_DELETIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_INSERTIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_INSERTIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());

    // Finally, when not showing changes:
    SfxBoolItem aShow(FN_REDLINE_SHOW, false);
    pView->GetViewFrame().GetDispatcher()->ExecuteList(FN_REDLINE_SHOW, SfxCallMode::SYNCHRON,
                                                       { &aShow });

    // Then make sure the state of the 3 show modes are correct:
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_CHANGES_IN_TEXT, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_CHANGES_IN_TEXT), pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_DELETIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_DELETIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
    pView->GetViewFrame().GetBindings().QueryState(FN_SET_TRACKED_INSERTIONS_IN_MARGIN, pItem);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_uInt16>(FN_SET_TRACKED_INSERTIONS_IN_MARGIN),
                         pItem->Which());
    CPPUNIT_ASSERT(!dynamic_cast<SfxBoolItem*>(pItem.get())->GetValue());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
