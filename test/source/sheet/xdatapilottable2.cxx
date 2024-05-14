/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/sheet/xdatapilottable2.hxx>
#include <com/sun/star/sheet/XDataPilotTable2.hpp>
#include <com/sun/star/sheet/DataPilotTableResultData.hpp>
#include <com/sun/star/sheet/XDataPilotDescriptor.hpp>
#include <com/sun/star/sheet/DataPilotFieldOrientation.hpp>
#include <com/sun/star/sheet/DataPilotTablePositionType.hpp>
#include <com/sun/star/sheet/DataPilotOutputRangeType.hpp>
#include <com/sun/star/sheet/XSpreadsheets.hpp>
#include <com/sun/star/sheet/XCellAddressable.hpp>
#include <com/sun/star/table/XCellCursor.hpp>
#include <com/sun/star/sheet/XCellRangeData.hpp>
#include <com/sun/star/sheet/DataResult.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppunit/TestAssert.h>
#include <numeric>

using namespace css;
using namespace css::uno;

namespace apitest {

XDataPilotTable2::~XDataPilotTable2()
{
}

void XDataPilotTable2::testGetPositionData()
{
    uno::Reference< sheet::XDataPilotTable2 > xDPTable(initDP2(), UNO_QUERY_THROW);
    getOutputRanges(xDPTable);
    table::CellAddress aAddr;

    aAddr.Sheet = maRangeTable.Sheet;
    for (sal_Int32 x = maRangeTable.StartColumn; x <= maRangeTable.EndColumn; ++x)
    {
        for (sal_Int32 y = maRangeTable.StartRow; y <= maRangeTable.EndRow; ++y)
        {
            aAddr.Column = x;
            aAddr.Row = y;

            sheet::DataPilotTablePositionData aPosData = xDPTable->getPositionData(aAddr);
            if (aPosData.PositionType == sheet::DataPilotTablePositionType::NOT_IN_TABLE)
            {
                CPPUNIT_ASSERT(false);
            }
        }
    }
}

void XDataPilotTable2::testGetDrillDownData()
{
    uno::Reference< sheet::XDataPilotTable2 > xDPTable(initDP2(), UNO_QUERY_THROW);

    getOutputRanges(xDPTable);
    buildDataFields(xDPTable);
    buildResultCells(xDPTable);

    for (const auto& rResultCell : maResultCells)
    {
        sheet::DataPilotTablePositionData aPosData = xDPTable->getPositionData(rResultCell);
        Any aTempAny = aPosData.PositionData;
        sheet::DataPilotTableResultData aResData;
        CPPUNIT_ASSERT(aTempAny >>= aResData);
        sal_Int32 nDim = maDataFieldDims[aResData.DataFieldIndex];
        sheet::DataResult aRes = aResData.Result;
        double nVal = aRes.Value;

        const Sequence< Sequence<Any> > aData = xDPTable->getDrillDownData(rResultCell);
        double sum = 0;

        if( aData.getLength() > 1 )
        {
            sum = std::accumulate(std::next(aData.begin()), aData.end(), double(0),
                [nDim](double res, const Sequence<Any>& rSeq) {
                    double nValue = 0;
                    if (rSeq[nDim] >>= nValue)
                        return res + nValue;
                    return res;
                });
        }

        CPPUNIT_ASSERT_DOUBLES_EQUAL(nVal, sum, 1E-12);
    }


}

void XDataPilotTable2::testGetOutputRangeByType()
{
    uno::Reference< sheet::XDataPilotTable2 > xDPTable(initDP2(), UNO_QUERY_THROW);
    getOutputRanges(xDPTable);

    // check for wrong arguments
    bool bCaught = false;
    try
    {
        xDPTable->getOutputRangeByType(-1);
    }
    catch ( const lang::IllegalArgumentException& )
    {
        bCaught = true;
    }
    CPPUNIT_ASSERT(bCaught);

    bCaught = false;
    try
    {
        xDPTable->getOutputRangeByType(100);
    }
    catch ( const lang::IllegalArgumentException& )
    {
        bCaught = true;
    }
    CPPUNIT_ASSERT(bCaught);

    // make sure the whole range is not empty
    CPPUNIT_ASSERT( maRangeWhole.EndColumn - maRangeWhole.StartColumn > 0);
    CPPUNIT_ASSERT( maRangeWhole.EndRow - maRangeWhole.StartRow > 0);

    //table range must be of equal width with the whole range, and the same bottom
    CPPUNIT_ASSERT_EQUAL( maRangeWhole.Sheet, maRangeTable.Sheet );
    CPPUNIT_ASSERT_EQUAL( maRangeWhole.EndRow, maRangeTable.EndRow );
    CPPUNIT_ASSERT_EQUAL( maRangeWhole.StartColumn, maRangeTable.StartColumn );
    CPPUNIT_ASSERT_EQUAL( maRangeWhole.EndColumn, maRangeTable.EndColumn );

    //result range must be smaller than the table range, and must share the same lower-right corner
    CPPUNIT_ASSERT_EQUAL( maRangeTable.Sheet, maRangeResult.Sheet );
    CPPUNIT_ASSERT( maRangeResult.StartColumn >= maRangeTable.StartColumn );
    CPPUNIT_ASSERT( maRangeResult.StartRow >= maRangeTable.StartRow );
    CPPUNIT_ASSERT_EQUAL( maRangeTable.EndRow, maRangeResult.EndRow );
    CPPUNIT_ASSERT_EQUAL( maRangeTable.EndColumn, maRangeResult.EndColumn );

}

void XDataPilotTable2::testInsertDrillDownSheet()
{
    uno::Reference< sheet::XDataPilotTable2 > xDPTable(initDP2(), UNO_QUERY_THROW);
    sal_Int32 nCellCount = maResultCells.size();

    uno::Reference< sheet::XSpreadsheets > xSheets(getSheets(), UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIA(xSheets, UNO_QUERY_THROW);
    sal_Int32 nSheetCount = xIA->getCount();

    for (sal_Int32 i = 0; i < nCellCount; ++i)
    {
        table::CellAddress aAddr = maResultCells[i];
        uno::Sequence< uno::Sequence< Any > > aData = xDPTable->getDrillDownData(aAddr);
        xDPTable->insertDrillDownSheet(aAddr);

        sal_Int32 nNewSheetCount= xIA->getCount();
        if (nNewSheetCount == nSheetCount + 1)
        {
            CPPUNIT_ASSERT(aData.getLength() >= 2);
            uno::Reference< sheet::XSpreadsheet > xSheet(xIA->getByIndex(aAddr.Sheet),UNO_QUERY_THROW);

            checkDrillDownSheetContent(xSheet, aData);

            uno::Reference< container::XNamed > xNamed(xSheet, UNO_QUERY_THROW);
            OUString aName = xNamed->getName();
            xSheets->removeByName(aName);
        }
        else if (nNewSheetCount == nSheetCount)
        {
            if (aData.getLength() > 1)
            {
                CPPUNIT_ASSERT(false);
            }
        }
        else
        {
            CPPUNIT_ASSERT(false);
        }

    }
}

void XDataPilotTable2::buildResultCells( uno::Reference< sheet::XDataPilotTable2 > const & xDPTable)
{
    getOutputRanges(xDPTable);
    maResultCells.clear();

    for ( sal_Int32 x = maRangeResult.StartColumn; x < maRangeResult.EndColumn; ++x)
    {
        for( sal_Int32 y = maRangeResult.StartRow; y < maRangeResult.EndRow; ++y)
        {
            table::CellAddress aAddr;
            aAddr.Sheet = maRangeResult.Sheet;
            aAddr.Column = x;
            aAddr.Row = y;
            sheet::DataPilotTablePositionData aPosData = xDPTable->getPositionData(aAddr);
            if (aPosData.PositionType != sheet::DataPilotTablePositionType::RESULT)
            {
                CPPUNIT_ASSERT(false);
            }
            maResultCells.push_back(aAddr);
        }
    }
}

void XDataPilotTable2::getOutputRanges( uno::Reference< sheet::XDataPilotTable2 > const & xDPTable)
{
    maRangeWhole = xDPTable->getOutputRangeByType(sheet::DataPilotOutputRangeType::WHOLE);
    maRangeTable = xDPTable->getOutputRangeByType(sheet::DataPilotOutputRangeType::TABLE);
    maRangeResult = xDPTable->getOutputRangeByType(sheet::DataPilotOutputRangeType::RESULT);
}

void XDataPilotTable2::buildDataFields( uno::Reference< sheet::XDataPilotTable2 > const & xDPTable )
{
    uno::Reference< sheet::XDataPilotDescriptor > xDesc(xDPTable, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex(xDesc->getDataPilotFields(), UNO_SET_THROW);

    sal_Int32 nFieldCount = xIndex->getCount();
    for( sal_Int32 i = 0; i < nFieldCount; ++i)
    {
        uno::Reference< beans::XPropertySet > xPropSet(xIndex->getByIndex(i), UNO_QUERY_THROW);
        Any aAny = xPropSet->getPropertyValue(u"Orientation"_ustr);
        sheet::DataPilotFieldOrientation aOrientation;
        CPPUNIT_ASSERT( aAny >>= aOrientation );

        if ( aOrientation == sheet::DataPilotFieldOrientation_DATA )
        {
            maDataFieldDims.push_back( i );
        }
    }
}

namespace {

table::CellAddress getLastUsedCellAddress( uno::Reference< sheet::XSpreadsheet > const & xSheet, sal_Int32 nCol, sal_Int32 nRow )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xSheet->getCellRangeByPosition(nCol, nRow, nCol, nRow), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xCursor = xSheet->createCursorByRange(xSheetRange);
    uno::Reference< table::XCellCursor > xCellCursor(xCursor, UNO_QUERY_THROW);
    xCellCursor->gotoEnd();

    uno::Reference< sheet::XCellAddressable > xCellAddr(xCursor->getCellByPosition(0, 0), UNO_QUERY_THROW);
    return xCellAddr->getCellAddress();
}

}

void XDataPilotTable2::checkDrillDownSheetContent(uno::Reference< sheet::XSpreadsheet > const & xSheet, const uno::Sequence< uno::Sequence< Any > >& aData)
{
    table::CellAddress aLastCell = getLastUsedCellAddress(xSheet, 0, 0);
    CPPUNIT_ASSERT(aData.hasElements());
    CPPUNIT_ASSERT(aLastCell.Row);
    CPPUNIT_ASSERT(aLastCell.Column);

    CPPUNIT_ASSERT_EQUAL(aData.getLength(), aLastCell.Row + 1);
    CPPUNIT_ASSERT_EQUAL(aData[0].getLength(), aLastCell.Column + 1);

    uno::Reference< table::XCellRange > xCellRange = xSheet->getCellRangeByPosition(0, 0, aLastCell.Column, aLastCell.Row);
    uno::Reference< sheet::XCellRangeData > xCellRangeData(xCellRange, UNO_QUERY_THROW);

    uno::Sequence< uno::Sequence< Any > > aSheetData = xCellRangeData->getDataArray();
    for (sal_Int32 x = 0; x < aSheetData.getLength(); ++x)
    {
        for(sal_Int32 y = 0; y < aSheetData[x].getLength(); ++y)
        {
            const Any& aCell1 = aSheetData[x][y];
            const Any& aCell2 = aData[x][y];
            CPPUNIT_ASSERT_EQUAL(aCell2, aCell1);
        }
    }
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
