/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapi_test.hxx>

#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sfx2/app.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <svl/itemset.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>

using namespace com::sun::star;

/// Covers sfx2/source/view/ fixes.
class Sfx2ViewTest : public UnoApiTest
{
public:
    Sfx2ViewTest()
        : UnoApiTest(u"/sfx2/qa/cppunit/data/"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Sfx2ViewTest, testReloadPage)
{
    // Load a document, which has 2 pages.
    loadFromFile(u"reload-page.odg");

    // Reload, and request to start on page 2.
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    SfxAllItemSet aSet(SfxGetpApp()->GetPool());
    aSet.Put(SfxInt32Item(SID_PAGE_NUMBER, 1));
    SfxRequest aReq(SID_RELOAD, SfxCallMode::SLOT, aSet);
    pFrame->ExecReload_Impl(aReq);
    uno::Reference<frame::XModel> xModel = SfxObjectShell::Current()->GetBaseModel();
    mxComponent = xModel;

    // Check the current page after reload.
    uno::Reference<drawing::XDrawView> xController(xModel->getCurrentController(), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPage(xController->getCurrentPage(), uno::UNO_QUERY);
    sal_Int32 nPage{};
    xPage->getPropertyValue(u"Number"_ustr) >>= nPage;

    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 2
    // - Actual  : 1
    // i.e. the document was opened on page 1, not page 2, SID_PAGE_NUMBER was ignored.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), nPage);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
