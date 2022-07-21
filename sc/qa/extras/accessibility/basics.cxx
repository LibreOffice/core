/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>

#include <com/sun/star/awt/Key.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>

#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestCalcMenu)
{
    load(u"private:factory/scalc");

    // in cell A1, insert the date
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Date"));
    // move down to A2
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, css::awt::Key::DOWN);
    // in cell A2, insert the time
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Time"));

    uno::Reference<accessibility::XAccessibleTable> sheet(
        getDocumentAccessibleContext()->getAccessibleChild(0)->getAccessibleContext(), // sheet 1
        uno::UNO_QUERY_THROW);

    /* it's very tricky to check the dates and times are correct (imagine running on
     * 1970-12-31 23:59:59.99, it's gonna shift *everything* in a 100th of a second) because clock
     * can have changed between generating the two values to compare, so do something simpler.
     * Ideally, we'd check the data looks like a date/time, but we're actually not so much
     * interested in the data itself than its mere existence. */
    // cell A1
    uno::Reference<accessibility::XAccessibleText> xText(
        sheet->getAccessibleCellAt(0, 0)->getAccessibleContext(), uno::UNO_QUERY_THROW);
    std::cout << "A1: " << xText->getText() << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(10 /* len("01/02/1970") */, xText->getText().getLength());

    // cell A2
    xText.set(sheet->getAccessibleCellAt(1, 0)->getAccessibleContext(), uno::UNO_QUERY_THROW);
    std::cout << "A2: " << xText->getText() << std::endl;
    CPPUNIT_ASSERT_GREATEREQUAL(8 /* len("01:02:03") */, xText->getText().getLength());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
