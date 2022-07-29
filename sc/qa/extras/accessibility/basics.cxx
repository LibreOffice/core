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
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <com/sun/star/awt/Key.hpp>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <vcl/scheduler.hxx>

#include <tools/date.hxx>
#include <tools/time.hxx>

#include <test/a11y/accessibletestbase.hxx>
#include <test/a11y/AccessibilityTools.hxx>

using namespace css;

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestCalcMenu)
{
    load(u"private:factory/scalc");

    const Date beforeDate(Date::SYSTEM);
    const double beforeTime = tools::Time(tools::Time::SYSTEM).GetTimeInDays();

    // in cell A1, insert the date
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Date"));
    // move down to A2
    documentPostKeyEvent(LOK_KEYEVENT_KEYINPUT, 0, css::awt::Key::DOWN);
    // in cell A2, insert the time
    CPPUNIT_ASSERT(activateMenuItem(u"Insert", u"Time"));

    uno::Reference<accessibility::XAccessibleTable> sheet(
        getDocumentAccessibleContext()->getAccessibleChild(0)->getAccessibleContext(), // sheet 1
        uno::UNO_QUERY_THROW);

    /* As it's very tricky to check the dates and times are correct in text format (imagine running
     * on 1970-12-31 23:59:59.99, it's gonna shift *everything* in a 100th of a second) because
     * clock can have changed between generating the two values to compare.  So instead we just
     * check the text is not empty, and the underlying value (representing the date or time) is
     * between the time it was before and after the call. */

    // cell A1 contains a date
    auto xCell = sheet->getAccessibleCellAt(0, 0)->getAccessibleContext();
    uno::Reference<accessibility::XAccessibleText> xText(xCell, uno::UNO_QUERY_THROW);
    std::cout << "A1 (text):  " << xText->getText() << std::endl;
    CPPUNIT_ASSERT(!xText->getText().isEmpty());
    uno::Reference<accessibility::XAccessibleValue> xValue(xCell, uno::UNO_QUERY_THROW);
    double value;
    CPPUNIT_ASSERT(xValue->getCurrentValue() >>= value);
    std::cout << "A1 (value): " << value << std::endl;
    uno::Reference<util::XNumberFormatsSupplier> xSupplier(mxDocument, uno::UNO_QUERY_THROW);
    util::Date nullDate;
    CPPUNIT_ASSERT(xSupplier->getNumberFormatSettings()->getPropertyValue("NullDate") >>= nullDate);
    const Date afterDate(Date::SYSTEM);
    CPPUNIT_ASSERT_GREATEREQUAL(double(beforeDate - nullDate), value);
    CPPUNIT_ASSERT_LESSEQUAL(double(afterDate - nullDate), value);

    // cell A2 contains time, no date, so we have to be careful passing midnight
    xCell = sheet->getAccessibleCellAt(1, 0)->getAccessibleContext();
    xText.set(xCell, uno::UNO_QUERY_THROW);
    std::cout << "A2 (text):  " << xText->getText() << std::endl;
    CPPUNIT_ASSERT(!xText->getText().isEmpty());
    xValue.set(xCell, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT(xValue->getCurrentValue() >>= value);
    std::cout << "A2 (value): " << value << std::endl;
    double afterTime = tools::Time(tools::Time::SYSTEM).GetTimeInDays();
    // in case day changed -- assuming no more than 24 hours passed
    if (afterTime < beforeTime)
    {
        afterTime += 1;
        if (value < beforeTime)
            value += 1;
    }
    CPPUNIT_ASSERT_GREATEREQUAL(beforeTime, value);
    CPPUNIT_ASSERT_LESSEQUAL(afterTime, value);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
