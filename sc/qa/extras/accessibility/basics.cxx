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

#include <tools/date.hxx>
#include <tools/time.hxx>

#include <test/a11y/accessibletestbase.hxx>

using namespace css;

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, TestCalcMenu)
{
    load(u"private:factory/scalc"_ustr);

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
    CPPUNIT_ASSERT(xSupplier->getNumberFormatSettings()->getPropertyValue(u"NullDate"_ustr)
                   >>= nullDate);
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

// test that converting cell row/col number <-> child index works
// for the case where 32-bit a11y child indices don't suffice (tdf#150683)
CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, Test64BitChildIndices)
{
    load(u"private:factory/scalc"_ustr);

    const sal_Int32 nRow = 1048575;
    const sal_Int32 nCol = 16383;

    uno::Reference<accessibility::XAccessibleTable> xTable(
        getDocumentAccessibleContext()->getAccessibleChild(0)->getAccessibleContext(), // sheet 1
        uno::UNO_QUERY_THROW);

    uno::Reference<accessibility::XAccessible> xCell = xTable->getAccessibleCellAt(nRow, nCol);
    const sal_Int64 nChildIndex = xCell->getAccessibleContext()->getAccessibleIndexInParent();
    // child index should be positive for all cells except the first one (A1)
    CPPUNIT_ASSERT_GREATER(sal_Int64(0), nChildIndex);

    // test that retrieving the row and column number via the child index again works
    CPPUNIT_ASSERT_EQUAL(nRow, xTable->getAccessibleRow(nChildIndex));
    CPPUNIT_ASSERT_EQUAL(nCol, xTable->getAccessibleColumn(nChildIndex));
}

CPPUNIT_TEST_FIXTURE(test::AccessibleTestBase, tdf157568)
{
    load(u"private:factory/scalc"_ustr);

    uno::Reference<accessibility::XAccessibleTable> sheet(
        getDocumentAccessibleContext()->getAccessibleChild(0)->getAccessibleContext(), // sheet 1
        uno::UNO_QUERY_THROW);

    uno::Reference<accessibility::XAccessible> xCell = sheet->getAccessibleCellAt(1, 1);
    CPPUNIT_ASSERT(xCell);
    uno::WeakReference<accessibility::XAccessible> xCellWeak(xCell);
    xCell.clear();
    // Verify that there are no reference cycles and that the ScAccessibleCell object dies after we
    // are done with it.
    uno::Reference<accessibility::XAccessible> xCell2(xCellWeak);
    CPPUNIT_ASSERT(!xCell2.is());
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
