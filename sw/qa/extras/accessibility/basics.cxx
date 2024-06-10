/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/awt/Key.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>

#include <test/a11y/swaccessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, TestBasicStructure)
{
    loadFromSrc(u"/sw/qa/python/testdocuments/xtextcontent.odt"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    dumpA11YTree(xContext);
    CPPUNIT_ASSERT_EQUAL(
        u"<PARAGRAPH>String1</PARAGRAPH><PARAGRAPH/><PARAGRAPH/><PARAGRAPH/>"
        "<TABLE name=\"Table1-1\" description=\"Table1 on page 1\">"
        "<TABLE_CELL name=\"A1\" description=\"A1\">"
        "<PARAGRAPH>String2</PARAGRAPH>"
        "</TABLE_CELL>"
        "</TABLE>"
        "<PARAGRAPH/>"
        "<TEXT_FRAME name=\"Frame1\"><PARAGRAPH>Frame1</PARAGRAPH></TEXT_FRAME>"
        "<TEXT_FRAME name=\"Frame2\"><PARAGRAPH>Frame2</PARAGRAPH></TEXT_FRAME>"_ustr,
        collectText(xContext));
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, TestTypeSimple)
{
    load(u"private:factory/swriter"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'h', 0);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'e', 0);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'l', 0);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'l', 0);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'o', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>hello</PARAGRAPH>"_ustr, collectText(xContext));
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, TestTypeMultiPara)
{
    load(u"private:factory/swriter"_ustr);
    auto xContext = getDocumentAccessibleContext();
    CPPUNIT_ASSERT(xContext.is());

    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'A', 0);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'B', 0);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::RETURN);
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 'C', 0);
    Scheduler::ProcessEventsToIdle();

    CPPUNIT_ASSERT_EQUAL(
        u"<PARAGRAPH>A</PARAGRAPH><PARAGRAPH>B</PARAGRAPH><PARAGRAPH>C</PARAGRAPH>"_ustr,
        collectText(xContext));
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, TestMenuInsertPageNumber)
{
    load(u"private:factory/swriter"_ustr);
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Field", u"Page Number"));
    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>1</PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_TEST_FIXTURE(test::SwAccessibleTestBase, TestMenuInsertPageBreak)
{
    load(u"private:factory/swriter"_ustr);

    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Field", u"Page Number"));
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Page Break"));
    // we need to move focus to the paragraph after the page break to insert the page number there
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, awt::Key::DOWN);
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Field", u"Page Number"));

    CPPUNIT_ASSERT_EQUAL(u"<PARAGRAPH>1</PARAGRAPH><PARAGRAPH>2</PARAGRAPH>"_ustr, collectText());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
