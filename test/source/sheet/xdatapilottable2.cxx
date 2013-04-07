/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/sheet/xdatapilottable2.hxx>
#include <com/sun/star/sheet/XDataPilotTable2.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>
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
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

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

    for (std::vector<table::CellAddress>::iterator itr = maResultCells.begin();
             itr != maResultCells.end(); ++itr)
    {
        sheet::DataPilotTablePositionData aPosData = xDPTable->getPositionData(*itr);
        Any aTempAny = aPosData.PositionData;
        sheet::DataPilotTableResultData aResData;
        CPPUNIT_ASSERT(aTempAny >>= aResData);
        sal_Int32 nDim = maDataFieldDims[aResData.DataFieldIndex];
        sheet::DataResult aRes = aResData.Result;
        double nVal = aRes.Value;

        Sequence< Sequence<Any> > aData = xDPTable->getDrillDownData(*itr);
        double sum = 0;

        if( aData.getLength() > 1 )
        {
            for ( sal_Int32 row = 1; row < aData.getLength(); ++row)
            {
                Any aAny = aData[row][nDim];
                double nValue = 0;
                if (aAny >>= nValue)
                    sum += nValue;
            }
        }

        std::cout << "Sum: " << sum << "; nVal: " << nVal << std::endl;
        CPPUNIT_ASSERT(sum == nVal);
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
    CPPUNIT_ASSERT( maRangeTable.Sheet == maRangeWhole.Sheet );
    CPPUNIT_ASSERT( maRangeTable.EndRow == maRangeWhole.EndRow );
    CPPUNIT_ASSERT( maRangeTable.StartColumn == maRangeWhole.StartColumn );
    CPPUNIT_ASSERT( maRangeTable.EndColumn == maRangeWhole.EndColumn );

    //result range must be smaller than the table range, and must share the same lower-right corner
    CPPUNIT_ASSERT( maRangeResult.Sheet == maRangeTable.Sheet );
    CPPUNIT_ASSERT( maRangeResult.StartColumn >= maRangeTable.StartColumn );
    CPPUNIT_ASSERT( maRangeResult.StartRow >= maRangeTable.StartRow );
    CPPUNIT_ASSERT( maRangeResult.EndRow == maRangeTable.EndRow );
    CPPUNIT_ASSERT( maRangeResult.EndColumn == maRangeTable.EndColumn );

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
            CPPUNIT_ASSERT(xSheet.is());

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

void XDataPilotTable2::buildResultCells( uno::Reference< sheet::XDataPilotTable2 > xDPTable)
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

void XDataPilotTable2::getOutputRanges( uno::Reference< sheet::XDataPilotTable2 > xDPTable)
{
    maRangeWhole = xDPTable->getOutputRangeByType(sheet::DataPilotOutputRangeType::WHOLE);
    maRangeTable = xDPTable->getOutputRangeByType(sheet::DataPilotOutputRangeType::TABLE);
    maRangeResult = xDPTable->getOutputRangeByType(sheet::DataPilotOutputRangeType::RESULT);
}

void XDataPilotTable2::buildDataFields( uno::Reference< sheet::XDataPilotTable2 > xDPTable )
{
    uno::Reference< sheet::XDataPilotDescriptor > xDesc(xDPTable, UNO_QUERY_THROW);
    uno::Reference< container::XIndexAccess > xIndex(xDesc->getDataPilotFields(), UNO_QUERY_THROW);

    sal_Int32 nFieldCount = xIndex->getCount();
    for( sal_Int32 i = 0; i < nFieldCount; ++i)
    {
        uno::Reference< beans::XPropertySet > xPropSet(xIndex->getByIndex(i), UNO_QUERY_THROW);
        Any aAny = xPropSet->getPropertyValue(OUString("Orientation"));
        sheet::DataPilotFieldOrientation aOrientation;
        CPPUNIT_ASSERT( aAny >>= aOrientation );

        if ( aOrientation == sheet::DataPilotFieldOrientation_DATA )
        {
            maDataFieldDims.push_back( i );
        }
    }
}

namespace {

table::CellAddress getLastUsedCellAddress( uno::Reference< sheet::XSpreadsheet > xSheet, sal_Int32 nCol, sal_Int32 nRow )
{
    uno::Reference< sheet::XSheetCellRange > xSheetRange( xSheet->getCellRangeByPosition(nCol, nRow, nCol, nRow), UNO_QUERY_THROW);
    uno::Reference< sheet::XSheetCellCursor > xCursor = xSheet->createCursorByRange(xSheetRange);
    uno::Reference< table::XCellCursor > xCellCursor(xCursor, UNO_QUERY_THROW);
    xCellCursor->gotoEnd();

    uno::Reference< sheet::XCellAddressable > xCellAddr(xCursor->getCellByPosition(0, 0), UNO_QUERY_THROW);
    return xCellAddr->getCellAddress();
}

}

bool XDataPilotTable2::checkDrillDownSheetContent(uno::Reference< sheet::XSpreadsheet > xSheet, uno::Sequence< uno::Sequence< Any > > aData)
{
    table::CellAddress aLastCell = getLastUsedCellAddress(xSheet, 0, 0);
    CPPUNIT_ASSERT(aData.getLength() > 0);
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
            Any& aCell1 = aSheetData[x][y];
            Any& aCell2 = aData[x][y];
            CPPUNIT_ASSERT(aCell1 == aCell2);
        }
    }
    return true;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
