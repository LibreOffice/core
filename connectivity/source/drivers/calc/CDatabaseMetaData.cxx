/*************************************************************************
 *
 *  $RCSfile: CDatabaseMetaData.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: nn $ $Date: 2001-02-21 11:35:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _CONNECTIVITY_CALC_OCALCDATABASEMETADATA_HXX_
#include "calc/CDatabaseMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_CONNECTION_HXX_
#include "calc/CConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPropertySet_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGESQUERY_HPP_
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGES_HPP_
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGE_HPP_
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _CONNECTIVITY_FILE_ADATABASEMETADATARESULTSET_HXX_
#include "file/FDatabaseMetaDataResultSet.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE calc
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif

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
}

// -------------------------------------------------------------------------

OCalcDatabaseMetaData::~OCalcDatabaseMetaData()
{
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getTypeInfo(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setTypeInfoMap();
    ORows aRows;
    ORow aRow;

    aRow.push_back(ORowSetValue());
    aRow.push_back(ORowSetValue(::rtl::OUString::createFromAscii("VARCHAR")));
    aRow.push_back(ORowSetValue(DataType::VARCHAR));
    aRow.push_back(ORowSetValue((sal_Int32)65535));
    aRow.push_back(ORowSetValue(::rtl::OUString::createFromAscii("'")));
    aRow.push_back(ORowSetValue(::rtl::OUString::createFromAscii("'")));
    aRow.push_back(ORowSetValue());
    aRow.push_back(ORowSetValue((sal_Int32)ColumnValue::NULLABLE));
    aRow.push_back(ORowSetValue((sal_Int32)1));
    aRow.push_back(ORowSetValue((sal_Int32)ColumnSearch::CHAR));
    aRow.push_back(ORowSetValue((sal_Int32)1));
    aRow.push_back(ORowSetValue((sal_Int32)0));
    aRow.push_back(ORowSetValue((sal_Int32)0));
    aRow.push_back(ORowSetValue());
    aRow.push_back(ORowSetValue((sal_Int32)0));
    aRow.push_back(ORowSetValue((sal_Int32)0));
    aRow.push_back(ORowSetValue());
    aRow.push_back(ORowSetValue());
    aRow.push_back(ORowSetValue((sal_Int32)10));
    aRows.push_back(aRow);

    aRow[1] = ORowSetValue(::rtl::OUString::createFromAscii("DECIMAL"));
    aRow[2] = ORowSetValue(DataType::DECIMAL);
    aRow[3] = ORowSetValue((sal_Int32)0);
    aRow[9] = ORowSetValue(ColumnSearch::BASIC);
    aRow[15] = ORowSetValue((sal_Int32)0);
    aRows.push_back(aRow);

    aRow[1] = ORowSetValue(::rtl::OUString::createFromAscii("BOOL"));
    aRow[2] = ORowSetValue(DataType::BIT);
    aRow[3] = ORowSetValue((sal_Int32)20);
    aRow[9] = ORowSetValue(ColumnSearch::BASIC);
    aRow[15] = ORowSetValue((sal_Int32)15);
    aRows.push_back(aRow);

    aRow[1] = ORowSetValue(::rtl::OUString::createFromAscii("DATE"));
    aRow[2] = ORowSetValue(DataType::DATE);
    aRow[3] = ORowSetValue((sal_Int32)0);
    aRow[9] = ORowSetValue(ColumnSearch::BASIC);
    aRow[15] = ORowSetValue((sal_Int32)0);
    aRows.push_back(aRow);

    aRow[1] = ORowSetValue(::rtl::OUString::createFromAscii("TIME"));
    aRow[2] = ORowSetValue(DataType::TIME);
    aRow[3] = ORowSetValue((sal_Int32)0);
    aRow[9] = ORowSetValue(ColumnSearch::BASIC);
    aRow[15] = ORowSetValue((sal_Int32)0);
    aRows.push_back(aRow);

    aRow[1] = ORowSetValue(::rtl::OUString::createFromAscii("TIMESTAMP"));
    aRow[2] = ORowSetValue(DataType::TIMESTAMP);
    aRow[3] = ORowSetValue((sal_Int32)0);
    aRow[9] = ORowSetValue(ColumnSearch::BASIC);
    aRow[15] = ORowSetValue((sal_Int32)0);
    aRows.push_back(aRow);

    pResult->setRows(aRows);
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getColumnPrivileges(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
    Reference< XResultSet > xRef = pResult;
    pResult->setColumnPrivilegesMap();
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getColumns(
    const Any& catalog, const ::rtl::OUString& schemaPattern, const ::rtl::OUString& tableNamePattern,
        const ::rtl::OUString& columnNamePattern ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );


    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
        throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
        throw SQLException();

    ORows aRows;
    ORow aRow(19);
    aRow[10] = (sal_Int32)10;
    Sequence< ::rtl::OUString> aTabNames(xNames->getElementNames());
    const ::rtl::OUString* pTabBegin    = aTabNames.getConstArray();
    const ::rtl::OUString* pTabEnd      = pTabBegin + aTabNames.getLength();
    for(;pTabBegin != pTabEnd;++pTabBegin)
    {
        if(match(tableNamePattern,*pTabBegin,'\0'))
        {
            Reference< XColumnsSupplier> xTable;
            xNames->getByName(*pTabBegin) >>= xTable;
            OSL_ENSHURE(xTable.is(),"Table not found! Normallya exception had to be thrown here!");
            aRow[3] = *pTabBegin;

            Reference< XNameAccess> xColumns = xTable->getColumns();
            if(!xColumns.is())
                throw SQLException();

            Sequence< ::rtl::OUString> aColNames(xColumns->getElementNames());

            const ::rtl::OUString* pBegin = aColNames.getConstArray();
            const ::rtl::OUString* pEnd = pBegin + aColNames.getLength();
            Reference< XPropertySet> xColumn;
            for(sal_Int32 i=1;pBegin != pEnd;++pBegin,++i)
            {
                if(match(columnNamePattern,*pBegin,'\0'))
                {
                    aRow[4] = *pBegin;

                    xColumns->getByName(*pBegin) >>= xColumn;
                    OSL_ENSHURE(xColumn.is(),"Columns contains a column who isn't a fastpropertyset!");
                    aRow[5] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_TYPE));
                    aRow[6] = ::comphelper::getString(xColumn->getPropertyValue(PROPERTY_TYPENAME));
                    aRow[7] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_PRECISION));
                    //  aRow[8] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    aRow[9] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_SCALE));
                    aRow[11] = ::comphelper::getINT32(xColumn->getPropertyValue(PROPERTY_ISNULLABLE));
                    //  aRow[12] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    aRow[13] = ::comphelper::getString(xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE));
                    //  aRow[14] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    //  aRow[15] = xColumn->getPropertyValue(PROPERTY_TYPENAME);
                    switch(sal_Int32(aRow[5]))
                    {
                    case DataType::CHAR:
                    case DataType::VARCHAR:
                        aRow[16] = (sal_Int32)254;
                        break;
                    case DataType::LONGVARCHAR:
                        aRow[16] = (sal_Int32)65535;
                        break;
                    default:
                        aRow[16] = (sal_Int32)0;
                    }
                    aRow[17] = i;
                    switch(sal_Int32(aRow[11]))
                    {
                    case ColumnValue::NO_NULLS:
                        aRow[18] = ::rtl::OUString::createFromAscii("NO");
                        break;
                    case ColumnValue::NULLABLE:
                        aRow[18] = ::rtl::OUString::createFromAscii("YES");
                        break;
                    default:
                        aRow[18] = ::rtl::OUString();
                    }
                    aRows.push_back(aRow);
                }
            }
        }
    }

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
    Reference< XResultSet > xRef = pResult;
    pResult->setColumnsMap();
    pResult->setRows(aRows);

    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getVersionColumns(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setVersionColumnsMap();
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getExportedKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setExportedKeysMap();
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getImportedKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setImportedKeysMap();
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getPrimaryKeys(
        const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setPrimaryKeysMap();
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getIndexInfo(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table,
        sal_Bool unique, sal_Bool approximate ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XTablesSupplier > xTables = m_pConnection->createCatalog();
    if(!xTables.is())
        throw SQLException();

    Reference< XNameAccess> xNames = xTables->getTables();
    if(!xNames.is())
        throw SQLException();

    ORows aRows;
    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
    Reference< XResultSet > xRef = pResult;
    pResult->setIndexInfoMap();
    pResult->setRows(aRows);
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getBestRowIdentifier(
    const Any& catalog, const ::rtl::OUString& schema, const ::rtl::OUString& table, sal_Int32 scope,
        sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setBestRowIdentifierMap();
    return xRef;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCalcDatabaseMetaData::getCrossReference(
    const Any& primaryCatalog, const ::rtl::OUString& primarySchema,
    const ::rtl::OUString& primaryTable, const Any& foreignCatalog,
        const ::rtl::OUString& foreignSchema, const ::rtl::OUString& foreignTable ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
        Reference< XResultSet > xRef = pResult;
    pResult->setCrossReferenceMap();
    return xRef;
}

// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL OCalcDatabaseMetaData::getURL(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return ::rtl::OUString::createFromAscii("sdbc:calc:");
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxBinaryLiteralLength(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return STRING_MAXLEN;
}

// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxCharLiteralLength(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxColumnNameLength(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return STRING_MAXLEN;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxColumnsInIndex(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return 1;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCalcDatabaseMetaData::getMaxColumnsInTable(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return 256;
}

// -------------------------------------------------------------------------

sal_Bool lcl_IsEmptyOrHidden( const Reference<XSpreadsheets>& xSheets, const ::rtl::OUString& rName )
{
    Any aAny = xSheets->getByName( rName );
    Reference<XSpreadsheet> xSheet;
    if ( aAny >>= xSheet )
    {
        //  test if sheet is hidden

        Reference<XPropertySet> xProp( xSheet, UNO_QUERY );
        if (xProp.is())
        {
            sal_Bool bVisible;
            Any aVisAny = xProp->getPropertyValue( ::rtl::OUString::createFromAscii("IsVisible") );
            if ( aVisAny >>= bVisible )
                if (!bVisible)
                    return sal_True;                // hidden
        }

#if 0
        //  test if whole sheet is empty

        Reference<XCellRangeAddressable> xAddr( xSheet, UNO_QUERY );
        Reference<XCellRangesQuery> xQuery( xSheet, UNO_QUERY );
        if ( xAddr.is() && xQuery.is() )
        {
            CellRangeAddress aTotalRange = xAddr->getRangeAddress();
            // queryIntersection to get a ranges object
            Reference<XSheetCellRanges> xRanges = xQuery->queryIntersection( aTotalRange );
            if (xRanges.is())
            {
                Reference<XEnumerationAccess> xCells = xRanges->getCells();
                if (xCells.is())
                {
                    if ( !xCells->hasElements() )
                        return sal_True;            // empty
                }
            }
        }
#endif

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

sal_Bool lcl_IsUnnamed( const Reference<XDatabaseRanges>& xRanges, const ::rtl::OUString& rName )
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
                Any aUserAny = xRangeProp->getPropertyValue( ::rtl::OUString::createFromAscii("IsUserDefined") );
                sal_Bool bUserDefined;
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
        const Any& catalog, const ::rtl::OUString& schemaPattern,
        const ::rtl::OUString& tableNamePattern, const Sequence< ::rtl::OUString >& types )
        throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ODatabaseMetaDataResultSet* pResult = new ODatabaseMetaDataResultSet();
    Reference< XResultSet > xRef = pResult;
    pResult->setTablesMap();

    // check if ORowSetValue type is given
    // when no types are given then we have to return all tables e.g. TABLE

    ::rtl::OUString aTable(::rtl::OUString::createFromAscii("TABLE"));

    sal_Bool bTableFound = sal_True;
    sal_Int32 nLength = types.getLength();
    if(nLength)
    {
        bTableFound = sal_False;

        const ::rtl::OUString* pBegin = types.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + nLength;
        for(;pBegin != pEnd;++pBegin)
        {
            if(*pBegin == aTable)
            {
                bTableFound = sal_True;
                break;
            }
        }
    }
    if(!bTableFound)
        return xRef;

    // get the sheet names from the document

    Reference<XSpreadsheetDocument> xDoc = ((OCalcConnection*)m_pConnection)->getDoc();
    if ( !xDoc.is() )
        throw SQLException();
    Reference<XSpreadsheets> xSheets = xDoc->getSheets();
    if ( !xSheets.is() )
        throw SQLException();
    Sequence< ::rtl::OUString > aSheetNames = xSheets->getElementNames();

    ORows aRows;
    sal_Int32 nSheetCount = aSheetNames.getLength();
    for (sal_Int32 nSheet=0; nSheet<nSheetCount; nSheet++)
    {
        ::rtl::OUString aName = aSheetNames[nSheet];
        if ( !lcl_IsEmptyOrHidden( xSheets, aName ) )
        {
            ORow aRow(3);
            aRow.push_back(ORowSetValue(aName));
            aRow.push_back(ORowSetValue(aTable));
            aRow.push_back(ORowSetValue());
            aRows.push_back(aRow);
        }
    }

    // also use database ranges

    Reference<XPropertySet> xDocProp( xDoc, UNO_QUERY );
    if ( xDocProp.is() )
    {
        Any aRangesAny = xDocProp->getPropertyValue( ::rtl::OUString::createFromAscii("DatabaseRanges") );
        Reference<XDatabaseRanges> xRanges;
        if ( aRangesAny >>= xRanges )
        {
            Sequence< ::rtl::OUString > aDBNames = xRanges->getElementNames();
            sal_Int32 nDBCount = aDBNames.getLength();
            for (sal_Int32 nRange=0; nRange<nDBCount; nRange++)
            {
                ::rtl::OUString aName = aDBNames[nRange];
                if ( !lcl_IsUnnamed( xRanges, aName ) )
                {
                    ORow aRow(3);
                    aRow.push_back(ORowSetValue(aName));
                    aRow.push_back(ORowSetValue(aTable));
                    aRow.push_back(ORowSetValue());
                    aRows.push_back(aRow);
                }
            }
        }
    }

    pResult->setRows(aRows);

    return xRef;
}

