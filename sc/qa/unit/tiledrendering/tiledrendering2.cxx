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
#include <postit.hxx>

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

CPPUNIT_TEST_FIXTURE(ScTiledRenderingTest, testTdf167042)
{
    ScModelObj* pModelObj = createDoc("tdf167042.ods");
    ScDocument* pDoc = pModelObj->GetDocument();
    ViewCallback aView1;

    uno::Sequence<beans::PropertyValue> aPropertyValues
        = { comphelper::makePropertyValue("ToPoint", OUString("$A$1")) };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);

    Point aPoint = aView1.m_aCellCursorBounds.Center();

    aPropertyValues = { comphelper::makePropertyValue("ToPoint", OUString("$B$1")) };
    dispatchCommand(mxComponent, ".uno:GoToCell", aPropertyValues);

    // Check that we have the comment on A1
    CPPUNIT_ASSERT_MESSAGE("There should be a note on A1", pDoc->HasNote(ScAddress(0, 0, 0)));
    ScPostIt* pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pNote->GetText());

    uno::Sequence aArgs{ comphelper::makePropertyValue(u"PersistentCopy"_ustr, false) };
    dispatchCommand(mxComponent, u".uno:FormatPaintbrush"_ustr, aArgs);

    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONDOWN, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, 0);
    pModelObj->postMouseEvent(LOK_MOUSEEVENT_MOUSEBUTTONUP, aPoint.getX(), aPoint.getY(), 1,
                              MOUSE_LEFT, 0);

    // Check that FormatPaintbrush worked
    vcl::Font aFont;
    pDoc->GetPattern(0, 0, 0)->fillFontOnly(aFont);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("font should be bold A1", WEIGHT_BOLD, aFont.GetWeight());

    // Check that we still have the comment on A1 after FormatPaintbrush
    pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pNote->GetText());

    dispatchCommand(mxComponent, u".uno:Undo"_ustr, {});

    // Check that we still have the comment on A1 after Undo
    pNote = pDoc->GetNote(ScAddress(0, 0, 0));
    CPPUNIT_ASSERT(pNote);
    // Without the fix in place, this test would have failed with
    // - Expected : test1
    // - Actual :
    CPPUNIT_ASSERT_EQUAL(u"test1"_ustr, pNote->GetText());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
