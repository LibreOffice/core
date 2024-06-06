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
#include <unotxdoc.hxx>

namespace
{
/// Covers sw/source/core/edit/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/core/edit/data/"_ustr)
    {
    }
};
}

CPPUNIT_TEST_FIXTURE(Test, testRedlineHidden)
{
    // Given a document with ShowRedlineChanges=false:
    createSwDoc("redline-hidden.fodt");
    SwDoc* pDoc = getSwDoc();

    // When formatting a paragraph by setting the para adjust to center, then make sure setting the
    // new item set on the paragraph doesn't crash:
    SwView* pView = pDoc->GetDocShell()->GetView();
    SfxItemSet aSet(pView->GetPool(), svl::Items<RES_PARATR_ADJUST, RES_PARATR_ADJUST>);
    SvxAdjustItem aItem(SvxAdjust::Center, RES_PARATR_ADJUST);
    aSet.Put(aItem);
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SetAttrSet(aSet, SetAttrMode::DEFAULT, nullptr, true);
}

CPPUNIT_TEST_FIXTURE(Test, testAutocorrect)
{
    // Given an empty document:
    createSwDoc();

    // When typing a string, which contains a "-", then make sure no memory corruption happens when
    // it gets auto-corrected to "–":
    auto pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    // Without the accompanying fix in place, this test would have failed with a
    // heap-use-after-free:
    emulateTyping(*pTextDoc, u"But not now - with ");
}

CPPUNIT_TEST_FIXTURE(Test, testDeleteSelNormalize)
{
    // Given a read-only document with a fillable form, the placeholder text is selected:
    createSwDoc("delete-sel-normalize.odt");
    SwDoc* pDoc = getSwDoc();
    SwWrtShell* pWrtShell = pDoc->GetDocShell()->GetWrtShell();
    pWrtShell->SttEndDoc(/*bStt=*/true);
    pWrtShell->GotoFormControl(/*bNext=*/true);

    // When you press 'delete' to type some content instead:
    pWrtShell->DelRight();

    // Then make sure the position after the delete is correct:
    SwPosition& rCursor = *pWrtShell->GetCursor()->GetPoint();
    // The full text is "key <field start><field separator><field end>", so this marks the position
    // after the field separator but before the field end.
    sal_Int32 nExpectedCharPos = strlen("key **");
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 6
    // - Actual  : 5
    // so we started to type between the field start and the field separator, nothing was visible on
    // the screen.
    CPPUNIT_ASSERT_EQUAL(nExpectedCharPos, rCursor.nContent.GetIndex());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
