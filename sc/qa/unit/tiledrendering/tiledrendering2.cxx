/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tiledrenderingmodeltestbase.cxx"

#include <com/sun/star/datatransfer/XTransferable2.hpp>

#include <vcl/scheduler.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/propertysequence.hxx>

using namespace com::sun::star;

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testSidebarLocale)
{
    ScModelObj* pModelObj = createDoc("chart.ods");
    int nView1 = SfxLokHelper::getView();
    ViewCallback aView1;
    SfxViewShell* pView1 = SfxViewShell::Current();
    pView1->SetLOKLocale(u"en-US"_ustr);
    SfxLokHelper::createView();
    ViewCallback aView2;
    SfxViewShell* pView2 = SfxViewShell::Current();
    pView2->SetLOKLocale(u"de-DE"_ustr);
    TestLokCallbackWrapper::InitializeSidebar();
    Scheduler::ProcessEventsToIdle();
    aView2.m_aStateChanges.clear();

    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, /*x=*/1, /*y=*/1, /*count=*/2,
                              /*buttons=*/1, /*modifier=*/0);
    SfxLokHelper::setView(nView1);
    Scheduler::ProcessEventsToIdle();

    auto it = aView2.m_aStateChanges.find(".uno:Sidebar");
    CPPUNIT_ASSERT(it != aView2.m_aStateChanges.end());
    std::string aLocale = it->second.get<std::string>("locale");
    CPPUNIT_ASSERT_EQUAL(std::string("de-DE"), aLocale);
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCopyMultiSelection)
{
    // Given a document with A1 and A3 as selected cells:
    ScModelObj* pModelObj = createDoc("multi-selection.ods");
    ViewCallback aView1;
    // Get the center of A3:
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$3"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    Point aPoint = aView1.m_aCellCursorBounds.Center();
    // Go to A1:
    aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$A$1"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);
    // Ctrl-click on A3:
    int nCtrl = KEY_MOD1;
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, nCtrl);

    // When getting the selection:
    uno::Reference<datatransfer::XTransferable> xTransferable = pModelObj->getSelection();

    // Make sure we get A1+A3 instead of an error:
    CPPUNIT_ASSERT(xTransferable.is());

    // Also make sure that just 2 cells is classified as a simple selection:
    uno::Reference<datatransfer::XTransferable2> xTransferable2(xTransferable, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xTransferable2.is());
    // Without the fix, the text selection was complex.
    CPPUNIT_ASSERT(!xTransferable2->isComplex());
}

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testCursorJumpOnFailedSearch)
{
    createDoc("empty.ods");
    ViewCallback aView;

    // Go to lower cell
    uno::Sequence<beans::PropertyValue> aPropertyValues = {
        comphelper::makePropertyValue(u"ToPoint"_ustr, u"$C$3"_ustr),
    };
    dispatchCommand(mxComponent, u".uno:GoToCell"_ustr, aPropertyValues);

    tools::Rectangle aInitialCursor = aView.m_aCellCursorBounds;

    // Search for a non-existing string using the start point parameters
    aPropertyValues = comphelper::InitPropertySequence(
        { { "SearchItem.SearchString", uno::Any(u"No-existing"_ustr) },
          { "SearchItem.Backward", uno::Any(false) },
          { "SearchItem.SearchStartPointX", uno::Any(static_cast<sal_Int32>(100)) },
          { "SearchItem.SearchStartPointY", uno::Any(static_cast<sal_Int32>(100)) } });
    dispatchCommand(mxComponent, u".uno:ExecuteSearch"_ustr, aPropertyValues);

    tools::Rectangle aFinalCursor = aView.m_aCellCursorBounds;

    // Without the fix, the cursor jumps even when no match is found
    CPPUNIT_ASSERT_EQUAL(aInitialCursor, aFinalCursor);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
