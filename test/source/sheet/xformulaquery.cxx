/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/cppunitasserthelper.hxx>
#include <test/sheet/xformulaquery.hxx>

#include <com/sun/star/sheet/XFormulaQuery.hpp>
#include <com/sun/star/sheet/XSheetCellRanges.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <cppunit/TestAssert.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;

namespace apitest
{
void XFormulaQuery::testQueryDependents()
{
    uno::Reference<sheet::XSpreadsheet> xSheet(getXSpreadsheet(), UNO_QUERY_THROW);
    xSheet->getCellByPosition(0, 0)->setValue(1);
    xSheet->getCellByPosition(1, 0)->setValue(1);
    xSheet->getCellByPosition(2, 0)->setValue(1);
    xSheet->getCellByPosition(3, 0)->setValue(1);

    uno::Reference<table::XCell> xCell = getXCell();
    xCell->setFormula(u"=SUM(A1:D1)"_ustr);

    uno::Reference<sheet::XFormulaQuery> xFormulaQuery(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xSCR = xFormulaQuery->queryDependents(false);
    uno::Sequence<table::CellRangeAddress> aCellRangeAddress = xSCR->getRangeAddresses();

    CPPUNIT_ASSERT_EQUAL(aCellRangeAddress[m_nIdxDependents], m_aCellRangeAddressDependents);
}

void XFormulaQuery::testQueryPrecedents()
{
    uno::Reference<sheet::XSpreadsheet> xSheet(getXSpreadsheet(), UNO_QUERY_THROW);
    xSheet->getCellByPosition(0, 0)->setValue(1);
    xSheet->getCellByPosition(1, 0)->setValue(1);
    xSheet->getCellByPosition(2, 0)->setValue(1);
    xSheet->getCellByPosition(3, 0)->setValue(1);
    xSheet->getCellByPosition(1, 2)->setFormula(u"=A16 * 2"_ustr);

    uno::Reference<table::XCell> xCell = getXCell();
    xCell->setFormula(u"=SUM(A1:D1)"_ustr);

    uno::Reference<sheet::XFormulaQuery> xFormulaQuery(init(), UNO_QUERY_THROW);
    uno::Reference<sheet::XSheetCellRanges> xSCR = xFormulaQuery->queryPrecedents(false);
    uno::Sequence<table::CellRangeAddress> aCellRangeAddress = xSCR->getRangeAddresses();

    CPPUNIT_ASSERT_EQUAL(aCellRangeAddress[m_nIdxPrecedents], m_aCellRangeAddressPrecedents);
}
} // namespace apitest

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
