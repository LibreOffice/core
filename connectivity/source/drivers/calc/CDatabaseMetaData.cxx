/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "calc/CDatabaseMetaData.hxx"
#include "calc/CConnection.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include "FDatabaseMetaDataResultSet.hxx"
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/types.hxx>
#include <tools/string.hxx>

using namespace connectivity::calc;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::sheet;

// -------------------------------------------------------------------------

OCalcDatabaseMetaData::OCalcDatabaseMetaData(OConnection* _pCon)    :ODatabaseMetaData(_pCon)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::OCalcDatabaseMetaData" );
}

// -------------------------------------------------------------------------

OCalcDatabaseMetaData::~OCalcDatabaseMetaData()
{
}

// -------------------------------------------------------------------------
Reference< XResultSet > OCalcDatabaseMetaData::impl_getTypeInfo_throw(  )
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::impl_getTypeInfo_throw" );
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xRef = pResult;

    static ODatabaseMetaDataResultSet::ORows aRows;
    if(aRows.empty())
    {
        ODatabaseMetaDataResultSet::ORow aRow;

        aRows.reserve(6);
        aRow.reserve(18);

        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator(OUString("VARCHAR")));
        aRow.push_back(new ORowSetValueDecorator(DataType::VARCHAR));
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)65535));
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getQuoteValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value()); // ORowSetValue((sal_Int32)ColumnValue::NULLABLE)
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)ColumnSearch::CHAR));
        aRow.push_back(ODatabaseMetaDataResultSet::get1Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::get0Value());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
        aRow.push_back(new ORowSetValueDecorator((sal_Int32)10));


        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("DECIMAL"));
        aRow[2] = new ORowSetValueDecorator(DataType::DECIMAL);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("BOOL"));
        aRow[2] = new ORowSetValueDecorator(DataType::BIT);
        aRow[3] = new ORowSetValueDecorator((sal_Int32)20);
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = new ORowSetValueDecorator((sal_Int32)15);
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("DATE"));
        aRow[2] = new ORowSetValueDecorator(DataType::DATE);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIME"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIME);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);

        aRow[1] = new ORowSetValueDecorator(OUString("TIMESTAMP"));
        aRow[2] = new ORowSetValueDecorator(DataType::TIMESTAMP);
        aRow[3] = ODatabaseMetaDataResultSet::get0Value();
        aRow[9] = ODatabaseMetaDataResultSet::getBasicValue();
        aRow[15] = ODatabaseMetaDataResultSet::get0Value();
        aRows.push_back(aRow);
    }

    pResult->setRows(aRows);
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getColumns(
    const Any& /*catalog*/, const OUString& /*schemaPattern*/, const OUString& tableNamePattern,
        const OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getColumns" );
    ::osl::MutexGuard aGuard( m_aMutex );


    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
        throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
        throw SQLException();

    ODatabaseMetaDataResultSet::ORows aRows;
    ODatabaseMetaDataResultSet::ORow  aRow(19);

    aRow[10] = new ORowSetValueDecorator((sal_Int32)10);

    Sequence< OUString> aTabNames(xNames->getElementNames());
    const OUString* pTabIter = aTabNames.getConstArray();
    const OUString* pTabEnd      = pTabIter + aTabNames.getLength();
    for(;pTabIter != pTabEnd;++pTabIter)
    {
        if(match(tableNamePattern,*pTabIter,'\0'))
        {
            const Reference< XColumnsSupplier> xTable(xNames->getByName(*pTabIter),UNO_QUERY_THROW);
            OSL_ENSURE(xTable.is(),"Table not found! Normallya exception had to be thrown here!");
            aRow[3] = new ORowSetValueDecorator(*pTabIter);

            const Reference< XNameAccess> xColumns = xTable->getColumns();
            if(!xColumns.is())
                throw SQLException();

            const Sequence< OUString> aColNames(xColumns->getElementNames());

            const OUString* pColumnIter = aColNames.getConstArray();
            const OUString* pEnd = pColumnIter + aColNames.getLength();
            Reference< XPropertySet> xColumn;
            for(sal_Int32 i=1;pColumnIter != pEnd;++pColumnIter,++i)
            {
                if(match(columnNamePattern,*pColumnIter,'\0'))
                {
                    aRow[4]  = new ORowSetValueDecorator( *pColumnIter);

                    xColumns->getByName(*pColumnIter) >>= xColumn;
                    OSL_ENSURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                    aRow[5] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))));
                    aRow[6] = new ORowSetValueDecorator(::comphelper::getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME))));
                    aRow[7] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION))));
                    //  aRow[8] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    aRow[9] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE))));
                    aRow[11] = new ORowSetValueDecorator(::comphelper::getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE))));
                    //  aRow[12] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    aRow[13] = new ORowSetValueDecorator(::comphelper::getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE))));
                    //  aRow[14] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    //  aRow[15] = xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME));
                    switch(sal_Int32(aRow[5]->getValue()))
                    {
                    case DataType::CHAR:
                    case DataType::VARCHAR:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)254);
                        break;
                    case DataType::LONGVARCHAR:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)65535);
                        break;
                    default:
                        aRow[16] = new ORowSetValueDecorator((sal_Int32)0);
                    }
                    aRow[17] = new ORowSetValueDecorator(i);
                    switch(sal_Int32(aRow[11]->getValue()))
                    {
                    case ColumnValue::NO_NULLS:
                        aRow[18]  = new ORowSetValueDecorator(OUString("NO"));
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18]  = new ORowSetValueDecorator(OUString("YES"));
                        break;
                    default:
                        aRow[18]  = new ORowSetValueDecorator(OUString());
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eColumns);
    Reference< XResultSet > xRef = pResult;
    pResult->setRows(aRows);

    return xRef;
}

// -------------------------------------------------------------------------

OUString SAL_CALL OCalcDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getURL" );
    ::osl::MutexGuard aGuard( m_aMutex );

    return OUString("sdbc:calc:") + m_pConnection->getURL();
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getMaxBinaryLiteralLength" );
    return STRING_MAXLEN;
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getMaxCharLiteralLength" );
    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getMaxColumnNameLength" );
    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getMaxColumnsInIndex" );
    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getMaxColumnsInTable" );
    return 256;
}

// -------------------------------------------------------------------------

static sal_Bool lcl_IsEmptyOrHidden( const Reference<XSpreadsheets>& xSheets, const OUString& rName )
{
    Any aAny = xSheets->getByName( rName );
    Reference<XSpreadsheet> xSheet;
    if ( aAny >>= xSheet )
    {
        //  test if sheet is hidden

        Reference<XPropertySet> xProp( xSheet, UNO_QUERY );
        if (xProp.is())
        {
            sal_Bool bVisible = sal_Bool();
            Any aVisAny = xProp->getPropertyValue("IsVisible");
            if ( aVisAny >>= bVisible )
                if (!bVisible)
                    return sal_True;                // hidden
        }

        //  use the same data area as in OCalcTable to test for empty table

        Reference<XSheetCellCursor> xCursor = xSheet->createCursor();
        Reference<XCellRangeAddressable> xRange( xCursor, UNO_QUERY );
        if ( xRange.is() )
        {
            xCursor->collapseToSize( 1, 1 );        // single (first) cell
            xCursor->collapseToCurrentRegion();     // contiguous data area

            CellRangeAddress aRangeAddr = xRange->getRangeAddress();
            if ( aRangeAddr.StartColumn == aRangeAddr.EndColumn &&
                 aRangeAddr.StartRow == aRangeAddr.EndRow )
            {
                //  single cell -> check content
                Reference<XCell> xCell = xCursor->getCellByPosition( 0, 0 );
                if ( xCell.is() && xCell->getType() == CellContentType_EMPTY )
                    return sal_True;
            }
        }
    }

    return sal_False;
}

static sal_Bool lcl_IsUnnamed( const Reference<XDatabaseRanges>& xRanges, const OUString& rName )
{
    sal_Bool bUnnamed = sal_False;

    Any aAny = xRanges->getByName( rName );
    Reference<XDatabaseRange> xRange;
    if ( aAny >>= xRange )
    {
        Reference<XPropertySet> xRangeProp( xRange, UNO_QUERY );
        if ( xRangeProp.is() )
        {
            try
            {
                Any aUserAny = xRangeProp->getPropertyValue("IsUserDefined");
                sal_Bool bUserDefined = sal_Bool();
                if ( aUserAny >>= bUserDefined )
                    bUnnamed = !bUserDefined;
            }
            catch ( UnknownPropertyException& )
            {
                // optional property
            }
        }
    }

    return bUnnamed;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getTables(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern, const Sequence< OUString >& types )
        throw(SQLException, RuntimeException)
{
    SAL_INFO( "connectivity.drivers", "calc Ocke.Janssen@sun.com OCalcDatabaseMetaData::getTables" );
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);
    Reference< XResultSet > xRef = pResult;

    // check if ORowSetValue type is given
    // when no types are given then we have to return all tables e.g. TABLE

    OUString aTable("TABLE");

    sal_Bool bTableFound = sal_True;
    sal_Int32 nLength = types.getLength();
    if(nLength)
    {
        bTableFound = sal_False;

        const OUString* pIter = types.getConstArray();
        const OUString* pEnd = pIter + nLength;
        for(;pIter != pEnd;++pIter)
        {
            if(*pIter == aTable)
            {
                bTableFound = sal_True;
                break;
            }
        }
    }
    if(!bTableFound)
        return xRef;

    // get the sheet names from the document

    OCalcConnection::ODocHolder aDocHodler(((OCalcConnection*)m_pConnection));
    Reference<XSpreadsheetDocument> xDoc = aDocHodler.getDoc();
    if ( !xDoc.is() )
        throw SQLException();
    Reference<XSpreadsheets> xSheets = xDoc->getSheets();
    if ( !xSheets.is() )
        throw SQLException();
    Sequence< OUString > aSheetNames = xSheets->getElementNames();

    ODatabaseMetaDataResultSet::ORows aRows;
    sal_Int32 nSheetCount = aSheetNames.getLength();
    for (sal_Int32 nSheet=0; nSheet<nSheetCount; nSheet++)
    {
        OUString aName = aSheetNames[nSheet];
        if ( !lcl_IsEmptyOrHidden( xSheets, aName ) && match(tableNamePattern,aName,'\0') )
        {
            ODatabaseMetaDataResultSet::ORow aRow(3);
            aRow.reserve(6);
            aRow.push_back(new ORowSetValueDecorator(aName));
            aRow.push_back(new ORowSetValueDecorator(aTable));
            aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
            aRows.push_back(aRow);
        }
    }

    // also use database ranges

    Reference<XPropertySet> xDocProp( xDoc, UNO_QUERY );
    if ( xDocProp.is() )
    {
        Any aRangesAny = xDocProp->getPropertyValue("DatabaseRanges");
        Reference<XDatabaseRanges> xRanges;
        if ( aRangesAny >>= xRanges )
        {
            Sequence< OUString > aDBNames = xRanges->getElementNames();
            sal_Int32 nDBCount = aDBNames.getLength();
            for (sal_Int32 nRange=0; nRange<nDBCount; nRange++)
            {
                OUString aName = aDBNames[nRange];
                if ( !lcl_IsUnnamed( xRanges, aName ) && match(tableNamePattern,aName,'\0') )
                {
                    ODatabaseMetaDataResultSet::ORow aRow(3);
                    aRow.reserve(6);
                    aRow.push_back(new ORowSetValueDecorator(aName));
                    aRow.push_back(new ORowSetValueDecorator(aTable));
                    aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
                    aRows.push_back(aRow);
                }
            }
        }
    }

    pResult->setRows(aRows);

    return xRef;
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
