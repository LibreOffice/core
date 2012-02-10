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
#include <com/sun/star/sheet/DataResult.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "cppunit/extensions/HelperMacros.h"

using namespace com::sun::star::uno;

namespace apitest {

void XDataPilotTable2::testGetPositionData()
{
    uno::Reference< sheet::XDataPilotTable2 > xDPTable(initDP2(), UNO_QUERY_THROW);
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
    CPPUNIT_ASSERT( maRangeResult.StartColumn < maRangeTable.StartColumn );
    CPPUNIT_ASSERT( maRangeResult.StartRow < maRangeTable.StartRow );
    CPPUNIT_ASSERT( maRangeResult.EndRow == maRangeTable.EndRow );
    CPPUNIT_ASSERT( maRangeResult.EndColumn == maRangeTable.EndColumn );

}

/*
void XDataPilotTable2::testInsertDrillDownData()
{
    uno::Reference< sheet::XDataPilotTable2 > xDPTable(initDP2(), UNO_QUERY_THROW);

}
*/
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
        Any aAny = xPropSet->getPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Orientation")));
        sheet::DataPilotFieldOrientation aOrientation;
        CPPUNIT_ASSERT( aAny >>= aOrientation );

        if ( aOrientation == sheet::DataPilotFieldOrientation_DATA )
        {
            maDataFieldDims.push_back( i );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
