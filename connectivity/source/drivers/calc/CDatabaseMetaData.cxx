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

#include <calc/CDatabaseMetaData.hxx>
#include <calc/CConnection.hxx>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#include <FDatabaseMetaDataResultSet.hxx>

using namespace connectivity::calc;
using namespace connectivity::file;
using namespace connectivity::component;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::sheet;

OCalcDatabaseMetaData::OCalcDatabaseMetaData(OConnection* _pCon)    :OComponentDatabaseMetaData(_pCon)
{
}

OCalcDatabaseMetaData::~OCalcDatabaseMetaData()
{
}

OUString SAL_CALL OCalcDatabaseMetaData::getURL(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return "sdbc:calc:" + m_pConnection->getURL();
}

static bool lcl_IsEmptyOrHidden( const Reference<XSpreadsheets>& xSheets, const OUString& rName )
{
    Any aAny = xSheets->getByName( rName );
    Reference<XSpreadsheet> xSheet;
    if ( aAny >>= xSheet )
    {
        //  test if sheet is hidden

        Reference<XPropertySet> xProp( xSheet, UNO_QUERY );
        if (xProp.is())
        {
            bool bVisible;
            Any aVisAny = xProp->getPropertyValue("IsVisible");
            if ( (aVisAny >>= bVisible) && !bVisible)
                return true;                // hidden
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
                    return true;
            }
        }
    }

    return false;
}

static bool lcl_IsUnnamed( const Reference<XDatabaseRanges>& xRanges, const OUString& rName )
{
    bool bUnnamed = false;

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
                bool bUserDefined;
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

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getTables(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& tableNamePattern, const Sequence< OUString >& types )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    rtl::Reference<ODatabaseMetaDataResultSet> pResult = new ODatabaseMetaDataResultSet(ODatabaseMetaDataResultSet::eTables);

    // check if ORowSetValue type is given
    // when no types are given then we have to return all tables e.g. TABLE

    OUString aTable("TABLE");

    bool bTableFound = true;
    sal_Int32 nLength = types.getLength();
    if(nLength)
    {
        bTableFound = false;

        const OUString* pIter = types.getConstArray();
        const OUString* pEnd = pIter + nLength;
        for(;pIter != pEnd;++pIter)
        {
            if(*pIter == aTable)
            {
                bTableFound = true;
                break;
            }
        }
    }
    if(!bTableFound)
        return pResult;

    // get the sheet names from the document

    OCalcConnection::ODocHolder aDocHolder(static_cast<OCalcConnection*>(m_pConnection));
    const Reference<XSpreadsheetDocument>& xDoc = aDocHolder.getDoc();
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
            ODatabaseMetaDataResultSet::ORow aRow { nullptr, nullptr, nullptr };
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
                    ODatabaseMetaDataResultSet::ORow aRow { nullptr, nullptr, nullptr };
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

    return pResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
