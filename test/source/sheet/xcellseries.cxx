/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xcellseries.hxx>

#include <com/sun/star/sheet/XCellSeries.hpp>
#include <com/sun/star/table/XCellRange.hpp>

#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XCellSeries::testFillAuto()
{
    uno::Reference<table::XCellRange> xCellRange(init(), UNO_QUERY_THROW);

    uno::Reference<table::XCellRange> xCellRangeH(xCellRange->getCellRangeByPosition(1, 0, 4, 0), UNO_QUERY_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesH(xCellRangeH, UNO_QUERY_THROW);
    xCellSeriesH->fillAuto(sheet::FillDirection_TO_RIGHT, 1);
    uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(5, 0), UNO_QUERY_THROW);
    xCellResultH->setFormula("=SUM(B1:E1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillAuto with TO_RIGHT", 14., xCellResultH->getValue());

    uno::Reference<table::XCellRange> xCellRangeV(xCellRange->getCellRangeByPosition(1, 0, 1, 3), UNO_QUERY_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesV(xCellRangeV, UNO_QUERY_THROW);
    xCellSeriesV->fillAuto(sheet::FillDirection_TO_BOTTOM, 1);
    uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(1, 4), UNO_QUERY_THROW);
    xCellResultV->setFormula("=SUM(B1:B4)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillAuto with TO_BOTTOM", 14., xCellResultV->getValue());
}

void XCellSeries::testFillSeries()
{
    uno::Reference<table::XCellRange> xCellRange(init(), UNO_QUERY_THROW);

    uno::Reference<table::XCellRange> xCellRangeH(xCellRange->getCellRangeByPosition(1, 0, 4, 0), UNO_QUERY_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesH(xCellRangeH, UNO_QUERY_THROW);
    xCellSeriesH->fillSeries(sheet::FillDirection_TO_RIGHT,
                             sheet::FillMode_LINEAR,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 8);
    uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(5, 0), UNO_QUERY_THROW);
    xCellResultH->setFormula("=SUM(B1:E1)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_RIGHT and LINEAR", 20., xCellResultH->getValue());

    xCellSeriesH->fillSeries(sheet::FillDirection_TO_RIGHT,
                             sheet::FillMode_GROWTH,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 16);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_RIGHT and GROWTH", 30., xCellResultH->getValue());

    uno::Reference<table::XCellRange> xCellRangeV(xCellRange->getCellRangeByPosition(1, 0, 1, 3), UNO_QUERY_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesV(xCellRangeV, UNO_QUERY_THROW);
    xCellSeriesV->fillSeries(sheet::FillDirection_TO_BOTTOM,
                             sheet::FillMode_LINEAR,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 8);
    uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(1, 4), UNO_QUERY_THROW);
    xCellResultV->setFormula("=SUM(B1:B4)");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_BOTTOM and LINEAR", 20., xCellResultV->getValue());

    xCellSeriesV->fillSeries(sheet::FillDirection_TO_BOTTOM,
                             sheet::FillMode_GROWTH,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 16);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_BOTTOM and GROWTH", 30., xCellResultV->getValue());
}

}
