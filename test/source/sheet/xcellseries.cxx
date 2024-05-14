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

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest {

void XCellSeries::testFillAuto()
{
    uno::Reference<table::XCellRange> xCellRange(init(), UNO_QUERY_THROW);
    sal_Int32 maValue = xCellRange->getCellByPosition(maStartX, maStartY)->getValue();

    uno::Reference<table::XCellRange> xCellRangeH(xCellRange->getCellRangeByPosition(maStartX, maStartY, maStartX + 2, maStartY), UNO_SET_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesH(xCellRangeH, UNO_QUERY_THROW);
    xCellSeriesH->fillAuto(sheet::FillDirection_TO_RIGHT, 1);
    sal_Int32 sumH = 0;
    for(sal_Int32 i = 0; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(maStartX + i, maStartY), UNO_SET_THROW);
        sumH += xCellResultH->getValue();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillAuto with TO_RIGHT", maValue * 3 + 3, sumH);

    uno::Reference<table::XCellRange> xCellRangeV(xCellRange->getCellRangeByPosition(maStartX, maStartY, maStartX, maStartY + 2), UNO_SET_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesV(xCellRangeV, UNO_QUERY_THROW);
    xCellSeriesV->fillAuto(sheet::FillDirection_TO_BOTTOM, 1);
    sal_Int32 sumV = 0;
    for(sal_Int32 i = 0; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(maStartX, maStartY + i), UNO_SET_THROW);
        sumV += xCellResultV->getValue();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillAuto with TO_BOTTOM", maValue * 3 + 3, sumV);

    for(sal_Int32 i = 1; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(maStartX + i, maStartY), UNO_SET_THROW);
        xCellResultH->setFormula(u""_ustr);
        uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(maStartX, maStartY + i), UNO_SET_THROW);
        xCellResultV->setFormula(u""_ustr);
    }
}

void XCellSeries::testFillSeries()
{
    uno::Reference<table::XCellRange> xCellRange(init(), UNO_QUERY_THROW);
    sal_Int32 maValue = xCellRange->getCellByPosition(maStartX, maStartY)->getValue();

    uno::Reference<table::XCellRange> xCellRangeH(xCellRange->getCellRangeByPosition(maStartX, maStartY, maStartX + 2, maStartY), UNO_SET_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesH(xCellRangeH, UNO_QUERY_THROW);
    xCellSeriesH->fillSeries(sheet::FillDirection_TO_RIGHT,
                             sheet::FillMode_LINEAR,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 1000);
    sal_Int32 sumH = 0;
    for(sal_Int32 i = 0; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(maStartX + i, maStartY), UNO_SET_THROW);
        sumH += xCellResultH->getValue();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_RIGHT and LINEAR", maValue * 3 + 6, sumH);

    xCellSeriesH->fillSeries(sheet::FillDirection_TO_RIGHT,
                             sheet::FillMode_GROWTH,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 1000);
    sumH = 0;
    for(sal_Int32 i = 0; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(maStartX + i, maStartY), UNO_SET_THROW);
        sumH += xCellResultH->getValue();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_RIGHT and GROWTH", maValue + maValue * 2 + maValue * 4, sumH);

    uno::Reference<table::XCellRange> xCellRangeV(xCellRange->getCellRangeByPosition(maStartX, maStartY, maStartX, maStartY + 2), UNO_SET_THROW);
    uno::Reference<sheet::XCellSeries> xCellSeriesV(xCellRangeV, UNO_QUERY_THROW);
    xCellSeriesV->fillSeries(sheet::FillDirection_TO_BOTTOM,
                             sheet::FillMode_LINEAR,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 1000);
    sal_Int32 sumV = 0;
    for(sal_Int32 i = 0; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(maStartX, maStartY + i), UNO_SET_THROW);
        sumV += xCellResultV->getValue();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_BOTTOM and LINEAR", maValue * 3 + 6, sumV);

    xCellSeriesV->fillSeries(sheet::FillDirection_TO_BOTTOM,
                             sheet::FillMode_GROWTH,
                             sheet::FillDateMode_FILL_DATE_DAY, 2, 1000);
    sumV = 0;
    for(sal_Int32 i = 0; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(maStartX, maStartY + i), UNO_SET_THROW);
        sumV += xCellResultV->getValue();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong result for fillSeries with TO_BOTTOM and GROWTH", maValue + maValue * 2 + maValue * 4, sumV);

    for(sal_Int32 i = 1; i < 3; i++) {
        uno::Reference<table::XCell> xCellResultH(xCellRange->getCellByPosition(maStartX + i, maStartY), UNO_SET_THROW);
        xCellResultH->setFormula(u""_ustr);
        uno::Reference<table::XCell> xCellResultV(xCellRange->getCellByPosition(maStartX, maStartY + i), UNO_SET_THROW);
        xCellResultV->setFormula(u""_ustr);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
