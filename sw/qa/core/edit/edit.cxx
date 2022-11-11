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

namespace
{
/// Covers sw/source/core/edit/ fixes.
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/core/edit/data/")
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

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
