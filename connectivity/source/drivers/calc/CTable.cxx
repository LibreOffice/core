/*************************************************************************
 *
 *  $RCSfile: CTable.cxx,v $
 *
 *  $Revision: 1.3 $
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

#ifndef _CONNECTIVITY_CALC_TABLE_HXX_
#include "calc/CTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTACCESS_HPP_
#include <com/sun/star/ucb/XContentAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEETDOCUMENT_HPP_
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XSPREADSHEET_HPP_
#include <com/sun/star/sheet/XSpreadsheet.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEADDRESSABLE_HPP_
#include <com/sun/star/sheet/XCellRangeAddressable.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGESQUERY_HPP_
#include <com/sun/star/sheet/XCellRangesQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGES_HPP_
#include <com/sun/star/sheet/XDatabaseRanges.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XDATABASERANGE_HPP_
#include <com/sun/star/sheet/XDatabaseRange.hpp>
#endif
#ifndef _COM_SUN_STAR_SHEET_XCELLRANGEREFERRER_HPP_
#include <com/sun/star/sheet/XCellRangeReferrer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _SV_CONVERTER_HXX_
#include <svtools/converter.hxx>
#endif
#ifndef _CONNECTIVITY_CALC_CONNECTION_HXX_
#include "calc/CConnection.hxx"
#endif
#ifndef _CONNECTIVITY_CALC_COLUMNS_HXX_
#include "calc/CColumns.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _CONFIG_HXX //autogen
#include <vcl/config.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "connectivity/DateConversion.hxx"
#endif
#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _SOLMATH_HXX //autogen wg. SolarMath
#include <tools/solmath.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE calc
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

using namespace connectivity;
using namespace connectivity::calc;
using namespace connectivity::file;
using namespace ::ucb;
using namespace ::cppu;
using namespace ::dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sheet;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::util;

// -------------------------------------------------------------------------

sal_Int32 lcl_ColumnCount( const Reference<XSpreadsheet>& xSheet )
{
    Reference<XSheetCellCursor> xCursor = xSheet->createCursor();
    Reference<XCellRangeAddressable> xRange( xCursor, UNO_QUERY );
    if ( !xRange.is() )
        return 0;

    xCursor->collapseToSize( 1, 1 );        // single (first) cell
    xCursor->collapseToCurrentRegion();     // contiguous data area

    CellRangeAddress aRangeAddr = xRange->getRangeAddress();
    return aRangeAddr.EndColumn + 1;
}

sal_Int32 lcl_RowCount( const Reference<XSpreadsheet>& xSheet )
{
    //! detect missing header row

    Reference<XSheetCellCursor> xCursor = xSheet->createCursor();
    Reference<XCellRangeAddressable> xRange( xCursor, UNO_QUERY );
    if ( !xRange.is() )
        return 0;

    xCursor->collapseToSize( 1, 1 );        // single (first) cell
    xCursor->collapseToCurrentRegion();     // contiguous data area

    CellRangeAddress aRangeAddr = xRange->getRangeAddress();
    return aRangeAddr.EndRow;       // first row (headers) is not counted
}

CellContentType lcl_GetContentOrResultType( const Reference<XCell>& xCell )
{
    CellContentType eCellType = xCell->getType();
    if ( eCellType == CellContentType_FORMULA )
    {
        Reference<XPropertySet> xProp( xCell, UNO_QUERY );
        try
        {
            Any aTypeAny = xProp->getPropertyValue( ::rtl::OUString::createFromAscii("FormulaResultType") );
            aTypeAny >>= eCellType;     // type of formula result
        }
        catch (UnknownPropertyException&)
        {
            eCellType = CellContentType_VALUE;  // if FormulaResultType property not available
        }
    }
    return eCellType;
}

Reference<XCell> lcl_GetUsedCell( const Reference<XSpreadsheet>& xSheet, sal_Int32 nDocColumn, sal_Int32 nDocRow )
{
    Reference<XCell> xCell = xSheet->getCellByPosition( nDocColumn, nDocRow );
    if ( xCell.is() && xCell->getType() == CellContentType_EMPTY )
    {
        //  get first non-empty cell

        Reference<XCellRangeAddressable> xAddr( xSheet, UNO_QUERY );
        if (xAddr.is())
        {
            CellRangeAddress aTotalRange = xAddr->getRangeAddress();
            sal_Int32 nLastRow = aTotalRange.EndRow;
            Reference<XCellRange> xRange =
                    xSheet->getCellRangeByPosition( nDocColumn, nDocRow, nDocColumn, nLastRow );
            Reference<XCellRangesQuery> xQuery( xRange, UNO_QUERY );
            if (xQuery.is())
            {
                // queryIntersection to get a ranges object
                Reference<XSheetCellRanges> xRanges = xQuery->queryIntersection( aTotalRange );
                if (xRanges.is())
                {
                    Reference<XEnumerationAccess> xCells = xRanges->getCells();
                    if (xCells.is())
                    {
                        Reference<XEnumeration> xEnum = xCells->createEnumeration();
                        if ( xEnum.is() && xEnum->hasMoreElements() )
                        {
                            // get first non-empty cell from enumeration
                            Any aCellAny = xEnum->nextElement();
                            aCellAny >>= xCell;
                        }
                        // otherwise, keep empty cell
                    }
                }
            }
        }
    }
    return xCell;
}

void lcl_GetColumnInfo( const Reference<XSpreadsheet>& xSheet, const Reference<XNumberFormats>& xFormats,
                        sal_Int32 nDocColumn, sal_Int32 nStartRow, sal_Bool bHasHeaders,
                        ::rtl::OUString& rName, sal_Int32& rDataType, sal_Bool& rCurrency )
{
    //! avoid duplicate field names

    //  get column name from first row, if range contains headers

    if ( bHasHeaders )
    {
        Reference<XCell> xHeaderCell = xSheet->getCellByPosition( nDocColumn, nStartRow );
        Reference<XText> xHeaderText( xHeaderCell, UNO_QUERY );
        if ( xHeaderText.is() )
            rName = xHeaderText->getString();
    }

    // get column type from first data row

    sal_Int32 nDataRow = nStartRow;
    if ( bHasHeaders )
        ++nDataRow;
    Reference<XCell> xDataCell = lcl_GetUsedCell( xSheet, nDocColumn, nDataRow );

    Reference<XPropertySet> xProp( xDataCell, UNO_QUERY );
    if ( xProp.is() )
    {
        rCurrency = sal_False;          // set to true for currency below

        CellContentType eCellType = lcl_GetContentOrResultType( xDataCell );
        if ( eCellType == CellContentType_TEXT )
            rDataType = DataType::VARCHAR;
        else if ( eCellType == CellContentType_VALUE )
        {
            //  get number format to distinguish between different types

            sal_Int16 nNumType = NumberFormat::NUMBER;
            try
            {
                Any aNumAny = xProp->getPropertyValue( ::rtl::OUString::createFromAscii("NumberFormat") );
                sal_Int32 nKey;
                if ( aNumAny >>= nKey )
                {
                    Reference<XPropertySet> xFormat = xFormats->getByKey( nKey );
                    if ( xFormat.is() )
                    {
                        Any aTypeAny = xFormat->getPropertyValue( ::rtl::OUString::createFromAscii("Type") );
                        aTypeAny >>= nNumType;
                    }
                }
            }
            catch ( Exception& )
            {
            }

            if ( nNumType & NumberFormat::NUMBER )
                rDataType = DataType::DECIMAL;
            else if ( nNumType & NumberFormat::CURRENCY )
            {
                rCurrency = sal_True;
                rDataType = DataType::DECIMAL;
            }
            else if ( ( nNumType & NumberFormat::DATETIME ) == NumberFormat::DATETIME )
            {
                //  NumberFormat::DATETIME is DATE | TIME
                rDataType = DataType::TIMESTAMP;
            }
            else if ( nNumType & NumberFormat::DATE )
                rDataType = DataType::DATE;
            else if ( nNumType & NumberFormat::TIME )
                rDataType = DataType::TIME;
            else if ( nNumType & NumberFormat::LOGICAL )
                rDataType = DataType::BIT;
            else
                rDataType = DataType::DECIMAL;
        }
        else
        {
            //  whole column empty
            rDataType = DataType::VARCHAR;
        }
    }
}

// -------------------------------------------------------------------------

void lcl_SetValue( file::ORowSetValue& rValue, const Reference<XSpreadsheet>& xSheet,
                    sal_Int32 nStartCol, sal_Int32 nStartRow, sal_Bool bHasHeaders,
                    const ::Date& rNullDate,
                    sal_Int32 nDBRow, sal_Int32 nDBColumn, sal_Int32 nType )
{
    sal_Int32 nDocColumn = nStartCol + nDBColumn - 1;   // database counts from 1
    sal_Int32 nDocRow = nStartRow + nDBRow - 1;
    if (bHasHeaders)
        ++nDocRow;

    Reference<XCell> xCell = xSheet->getCellByPosition( nDocColumn, nDocRow );
    if ( xCell.is() )
    {
        CellContentType eCellType = lcl_GetContentOrResultType( xCell );
        switch (nType)
        {
            case DataType::VARCHAR:
                {
                    // no difference between empty cell and empty string in spreadsheet
                    Reference<XText> xText( xCell, UNO_QUERY );
                    if ( xText.is() )
                    {
                        ::rtl::OUString sVal = xText->getString();
                        rValue = sVal;
                    }
                }
                break;
            case DataType::DECIMAL:
                if ( eCellType == CellContentType_VALUE )
                    rValue = xCell->getValue();         // double
                else
                    rValue.setNull();
                break;
            case DataType::BIT:
                if ( eCellType == CellContentType_VALUE )
                    rValue = (sal_Bool)( xCell->getValue() != 0.0 );
                else
                    rValue.setNull();
                break;
            case DataType::DATE:
                if ( eCellType == CellContentType_VALUE )
                {
                    ::Date aDate( rNullDate );
                    aDate += (long)SolarMath::ApproxFloor( xCell->getValue() );
                    ::com::sun::star::util::Date aDateStruct( aDate.GetDay(), aDate.GetMonth(), aDate.GetYear() );
                    rValue = aDateStruct;
                }
                else
                    rValue.setNull();
                break;
            case DataType::TIME:
                if ( eCellType == CellContentType_VALUE )
                {
                    double fCellVal = xCell->getValue();
                    double fTime = fCellVal - SolarMath::ApproxFloor( fCellVal );
                    long nIntTime = (long)SolarMath::Round( fTime * 8640000.0 );
                    if ( nIntTime == 8640000 )
                        nIntTime = 0;                       // 23:59:59.995 and above is 00:00:00.00
                    ::com::sun::star::util::Time aTime;
                    aTime.HundredthSeconds = (sal_uInt16)( nIntTime % 100 );
                    nIntTime /= 100;
                    aTime.Seconds = (sal_uInt16)( nIntTime % 60 );
                    nIntTime /= 60;
                    aTime.Minutes = (sal_uInt16)( nIntTime % 60 );
                    nIntTime /= 60;
                    OSL_ENSHURE( nIntTime < 24, "error in time calculation" );
                    aTime.Hours = (sal_uInt16) nIntTime;
                    rValue = aTime;
                }
                else
                    rValue.setNull();
                break;
            case DataType::TIMESTAMP:
                if ( eCellType == CellContentType_VALUE )
                {
                    double fCellVal = xCell->getValue();
                    double fDays = SolarMath::ApproxFloor( fCellVal );
                    double fTime = fCellVal - fDays;
                    long nIntDays = (long)fDays;
                    long nIntTime = (long)SolarMath::Round( fTime * 8640000.0 );
                    if ( nIntTime == 8640000 )
                    {
                        nIntTime = 0;                       // 23:59:59.995 and above is 00:00:00.00
                        ++nIntDays;                         // (next day)
                    }

                    ::com::sun::star::util::DateTime aDateTime;

                    aDateTime.HundredthSeconds = (sal_uInt16)( nIntTime % 100 );
                    nIntTime /= 100;
                    aDateTime.Seconds = (sal_uInt16)( nIntTime % 60 );
                    nIntTime /= 60;
                    aDateTime.Minutes = (sal_uInt16)( nIntTime % 60 );
                    nIntTime /= 60;
                    OSL_ENSHURE( nIntTime < 24, "error in time calculation" );
                    aDateTime.Hours = (sal_uInt16) nIntTime;

                    ::Date aDate( rNullDate );
                    aDate += nIntDays;
                    aDateTime.Day = aDate.GetDay();
                    aDateTime.Month = aDate.GetMonth();
                    aDateTime.Year = aDate.GetYear();

                    rValue = aDateTime;
                }
                else
                    rValue.setNull();
                break;
        }
    }

//  rValue.setTypeKind(nType);
}

// -------------------------------------------------------------------------

::rtl::OUString lcl_GetColumnStr( sal_Int32 nColumn )
{
    if ( nColumn < 26 )
        return ::rtl::OUString::valueOf( (sal_Unicode) ( 'A' + nColumn ) );
    else
    {
        ::rtl::OUStringBuffer aBuffer(2);
        aBuffer.setCharAt( 0, (sal_Unicode) ( 'A' + ( nColumn / 26 ) - 1 ) );
        aBuffer.setCharAt( 1, (sal_Unicode) ( 'A' + ( nColumn % 26 ) ) );
        return aBuffer.makeStringAndClear();
    }
}

void OCalcTable::fillColumns()
{
    if ( !m_xSheet.is() )
        throw SQLException();

    String aStrFieldName;
    aStrFieldName.AssignAscii("Column");
    ::rtl::OUString aTypeName;
    ::comphelper::UStringMixEqual aCase(m_pConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());

    for (sal_Int32 i = 0; i < m_nDataCols; i++)
    {
        ::rtl::OUString aColumnName;
        sal_Int32 eType = DataType::OTHER;
        sal_Bool bCurrency = sal_False;

        lcl_GetColumnInfo( m_xSheet, m_xFormats, m_nStartCol + i, m_nStartRow, m_bHasHeaders,
                            aColumnName, eType, bCurrency );

        if ( !aColumnName.getLength() )
            aColumnName = lcl_GetColumnStr( i );

        sal_Int32 nPrecision = 0;   //! ...
        sal_Int32 nDecimals = 0;    //! ...

        switch ( eType )
        {
            case DataType::VARCHAR:
                aTypeName = ::rtl::OUString::createFromAscii("VARCHAR");
                break;
            case DataType::DECIMAL:
                aTypeName = ::rtl::OUString::createFromAscii("DECIMAL");
                break;
            case DataType::BIT:
                aTypeName = ::rtl::OUString::createFromAscii("BOOL");
                break;
            case DataType::DATE:
                aTypeName = ::rtl::OUString::createFromAscii("DATE");
                break;
            case DataType::TIME:
                aTypeName = ::rtl::OUString::createFromAscii("TIME");
                break;
            case DataType::TIMESTAMP:
                aTypeName = ::rtl::OUString::createFromAscii("TIMESTAMP");
                break;
            default:
                OSL_ASSERT("missing type name");
                aTypeName = ::rtl::OUString();
        }

        // check if the column name already exists
        ::rtl::OUString aAlias = aColumnName;
        OSQLColumns::const_iterator aFind = connectivity::find(m_aColumns->begin(),m_aColumns->end(),aAlias,aCase);
        sal_Int32 nExprCnt = 0;
        while(aFind != m_aColumns->end())
        {
            (aAlias = aColumnName) += ::rtl::OUString::valueOf((sal_Int32)++nExprCnt);
            aFind = connectivity::find(m_aColumns->begin(),m_aColumns->end(),aAlias,aCase);
        }

        sdbcx::OColumn* pColumn = new sdbcx::OColumn( aAlias, aTypeName, ::rtl::OUString(),
                                                ColumnValue::NULLABLE, nPrecision, nDecimals,
                                                eType, sal_False, sal_False, bCurrency,
                                                getConnection()->getMetaData()->storesMixedCaseQuotedIdentifiers() );
        Reference< XPropertySet> xCol = pColumn;
        m_aColumns->push_back(xCol);
        m_aTypes.push_back(eType);
        m_aPrecisions.push_back(nPrecision);
        m_aScales.push_back(nDecimals);
    }
}

// -------------------------------------------------------------------------
OCalcTable::OCalcTable(OCalcConnection* _pConnection)
        :OCalcTable_BASE(_pConnection)
        ,m_nStartCol(0)
        ,m_nStartRow(0)
        ,m_nDataCols(0)
        ,m_nDataRows(0)
        ,m_bHasHeaders(sal_False)
{
}
// -------------------------------------------------------------------------
OCalcTable::OCalcTable(OCalcConnection* _pConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName
                ) : OCalcTable_BASE(_pConnection,_Name,
                                  _Type,
                                  _Description,
                                  _SchemaName,
                                  _CatalogName)
                ,m_nStartCol(0)
                ,m_nStartRow(0)
                ,m_nDataCols(0)
                ,m_nDataRows(0)
                ,m_bHasHeaders(sal_False)
{
    //  get sheet object

    Reference<XSpreadsheetDocument> xDoc = _pConnection->getDoc();
    if (xDoc.is())
    {
        Reference<XSpreadsheets> xSheets = xDoc->getSheets();
        if ( xSheets.is() && xSheets->hasByName( _Name ) )
        {
            Any aAny = xSheets->getByName( _Name );
            if ( aAny >>= m_xSheet )
            {
                m_nDataCols = lcl_ColumnCount( m_xSheet );
                m_nDataRows = lcl_RowCount( m_xSheet );
                m_bHasHeaders = sal_True;
                // whole sheet is always assumed to include a header row
            }
        }
        else        // no sheet -> try database range
        {
            Reference<XPropertySet> xDocProp( xDoc, UNO_QUERY );
            if ( xDocProp.is() )
            {
                Any aRangesAny = xDocProp->getPropertyValue( ::rtl::OUString::createFromAscii("DatabaseRanges") );
                Reference<XDatabaseRanges> xRanges;
                if ( aRangesAny >>= xRanges )
                {
                    if ( xRanges.is() && xRanges->hasByName( _Name ) )
                    {
                        Any aAny = xRanges->getByName( _Name );
                        Reference<XDatabaseRange> xDBRange;
                        if ( aAny >>= xDBRange )
                        {
                            Reference<XCellRangeReferrer> xRefer( xDBRange, UNO_QUERY );
                            if ( xRefer.is() )
                            {
                                //  Header flag is always stored with database range
                                //  Get flag from FilterDescriptor

                                sal_Bool bRangeHeader = sal_True;
                                Reference<XSheetFilterDescriptor> xFilter = xDBRange->getFilterDescriptor();
                                Reference<XPropertySet> xFiltProp( xFilter, UNO_QUERY );
                                if ( xFiltProp.is() )
                                {
                                    Any aHdrAny = xFiltProp->getPropertyValue(
                                            ::rtl::OUString::createFromAscii("ContainsHeader") );
                                    aHdrAny >>= bRangeHeader;
                                }

                                Reference<XCellRange> xCellRange = xRefer->getReferredCells();
                                Reference<XSheetCellRange> xSheetRange( xCellRange, UNO_QUERY );
                                Reference<XCellRangeAddressable> xAddr( xCellRange, UNO_QUERY );
                                if ( xSheetRange.is() && xAddr.is() )
                                {
                                    m_xSheet = xSheetRange->getSpreadsheet();
                                    CellRangeAddress aRangeAddr = xAddr->getRangeAddress();
                                    m_nStartCol = aRangeAddr.StartColumn;
                                    m_nStartRow = aRangeAddr.StartRow;
                                    m_nDataCols = aRangeAddr.EndColumn - m_nStartCol + 1;
                                    if ( bRangeHeader )
                                    {
                                        //  m_nDataRows is excluding header row
                                        m_nDataRows = aRangeAddr.EndRow - m_nStartRow;
                                    }
                                    else
                                    {
                                        //  m_nDataRows counts the whole range
                                        m_nDataRows = aRangeAddr.EndRow - m_nStartRow + 1;
                                    }

                                    m_bHasHeaders = bRangeHeader;
                                }
                            }
                        }
                    }
                }
            }
        }

        Reference<XNumberFormatsSupplier> xSupp( xDoc, UNO_QUERY );
        if (xSupp.is())
            m_xFormats = xSupp->getNumberFormats();

        Reference<XPropertySet> xProp( xDoc, UNO_QUERY );
        if (xProp.is())
        {
            Any aDateAny = xProp->getPropertyValue( ::rtl::OUString::createFromAscii("NullDate") );
            ::com::sun::star::util::Date aDateStruct;
            if ( aDateAny >>= aDateStruct )
                m_aNullDate = ::Date( aDateStruct.Day, aDateStruct.Month, aDateStruct.Year );
        }
    }

    //! default if no null date available?

    fillColumns();

    refreshColumns();
}
// -------------------------------------------------------------------------
void OCalcTable::refreshColumns()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    ::std::vector< ::rtl::OUString> aVector;

    for(OSQLColumns::const_iterator aIter = m_aColumns->begin();aIter != m_aColumns->end();++aIter)
        aVector.push_back(Reference< XNamed>(*aIter,UNO_QUERY)->getName());

    if(m_pColumns)
        delete m_pColumns;
    m_pColumns  = new OCalcColumns(this,m_aMutex,aVector);
}
// -------------------------------------------------------------------------
void OCalcTable::refreshIndexes()
{
    //  Calc table has no index
}

// -------------------------------------------------------------------------
void SAL_CALL OCalcTable::disposing(void)
{
    OFileTable::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
#ifdef DEBUG
    for(OSQLColumns::const_iterator aIter = m_aColumns->begin();aIter != m_aColumns->end();++aIter)
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xProp = *aIter;
        xProp = NULL;
    }
#endif
    m_aColumns->clear();
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OCalcTable::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes = OTable_TYPEDEF::getTypes();
    Sequence< Type > aRet(aTypes.getLength()-3);
    const Type* pBegin = aTypes.getConstArray();
    const Type* pEnd = pBegin + aTypes.getLength();
    sal_Int32 i=0;
    for(;pBegin != pEnd;++pBegin,++i)
    {
        if(!(*pBegin == ::getCppuType((const Reference<XKeysSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XIndexesSupplier>*)0) ||
            *pBegin == ::getCppuType((const Reference<XRename>*)0) ||
            *pBegin == ::getCppuType((const Reference<XAlterTable>*)0) ||
            *pBegin == ::getCppuType((const Reference<XDataDescriptorFactory>*)0)))
        {
            aRet.getArray()[i] = *pBegin;
        }
    }
    aRet.getArray()[i] = ::getCppuType( (const Reference< ::com::sun::star::lang::XUnoTunnel > *)0 );

    return aRet;
}

// -------------------------------------------------------------------------
Any SAL_CALL OCalcTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if( rType == ::getCppuType((const Reference<XKeysSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XIndexesSupplier>*)0) ||
        rType == ::getCppuType((const Reference<XRename>*)0) ||
        rType == ::getCppuType((const Reference<XAlterTable>*)0) ||
        rType == ::getCppuType((const Reference<XDataDescriptorFactory>*)0))
        return Any();

    Any aRet = ::cppu::queryInterface(rType,static_cast< ::com::sun::star::lang::XUnoTunnel*> (this));
    if(aRet.hasValue())
        return aRet;

    return OTable_TYPEDEF::queryInterface(rType);
}

//--------------------------------------------------------------------------
Sequence< sal_Int8 > OCalcTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OCalcTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return OCalcTable_BASE::getSomething(rId);
}
//------------------------------------------------------------------
sal_Int32 OCalcTable::getCurrentLastPos() const
{
    return m_nDataRows;
}
//------------------------------------------------------------------
sal_Bool OCalcTable::seekRow(FilePosition eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos)
{
    // ----------------------------------------------------------
    // Positionierung vorbereiten:

    sal_uInt32 nNumberOfRecords = m_nDataRows;
    sal_uInt32 nTempPos = m_nFilePos;
    m_nFilePos = nCurPos;

    switch(eCursorPosition)
    {
        case FILE_NEXT:
            m_nFilePos++;
            break;
        case FILE_PRIOR:
            if (m_nFilePos > 0)
                m_nFilePos--;
            break;
        case FILE_FIRST:
            m_nFilePos = 1;
            break;
        case FILE_LAST:
            m_nFilePos = nNumberOfRecords;
            break;
        case FILE_RELATIVE:
            m_nFilePos = (((sal_Int32)m_nFilePos) + nOffset < 0) ? 0L
                            : (sal_uInt32)(((sal_Int32)m_nFilePos) + nOffset);
            break;
        case FILE_ABSOLUTE:
        case FILE_BOOKMARK:
            m_nFilePos = (sal_uInt32)nOffset;
            break;
    }

    if (m_nFilePos > (sal_Int32)nNumberOfRecords)
        m_nFilePos = (sal_Int32)nNumberOfRecords + 1;

    if (m_nFilePos == 0 || m_nFilePos == (sal_Int32)nNumberOfRecords + 1)
        goto Error;
    else
    {
        //! read buffer / setup row object etc?
    }
    goto End;

Error:
    switch(eCursorPosition)
    {
        case FILE_PRIOR:
        case FILE_FIRST:
            m_nFilePos = 0;
            break;
        case FILE_LAST:
        case FILE_NEXT:
        case FILE_ABSOLUTE:
        case FILE_RELATIVE:
            if (nOffset > 0)
                m_nFilePos = nNumberOfRecords + 1;
            else if (nOffset < 0)
                m_nFilePos = 0;
            break;
        case FILE_BOOKMARK:
            m_nFilePos = nTempPos;   // vorherige Position
    }
    //  aStatus.Set(SDB_STAT_NO_DATA_FOUND);
    return sal_False;

End:
    nCurPos = m_nFilePos;
    return sal_True;
}
//------------------------------------------------------------------
sal_Bool OCalcTable::fetchRow( file::OValueRow _rRow, const OSQLColumns & _rCols,
                                sal_Bool _bUseTableDefs, sal_Bool bRetrieveData )
{
    // read the bookmark

    BOOL bIsCurRecordDeleted = sal_False;
    _rRow->setDeleted(bIsCurRecordDeleted);
    (*_rRow)[0] = m_nFilePos;

    if (!bRetrieveData)
        return TRUE;

    // fields

    OSQLColumns::const_iterator aIter = _rCols.begin();
    for (sal_Int32 i = 1; aIter != _rCols.end();++aIter, i++)
    {
        Reference< XPropertySet> xColumn = *aIter;

//      ::rtl::OUString aName;
//      xColumn->getPropertyValue(PROPERTY_NAME) >>= aName;

        sal_Int32 nType;
        if(_bUseTableDefs)
            nType = m_aTypes[i-1];
        else
            xColumn->getPropertyValue(PROPERTY_TYPE) >>= nType;

        if ((*_rRow)[i].isBound())
            lcl_SetValue( (*_rRow)[i], m_xSheet, m_nStartCol, m_nStartRow, m_bHasHeaders,
                            m_aNullDate, m_nFilePos, i, nType );
    }
    return sal_True;
}
// -------------------------------------------------------------------------
void OCalcTable::FileClose()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    OCalcTable_BASE::FileClose();
}
// -------------------------------------------------------------------------
BOOL OCalcTable::CreateImpl()
{
    return sal_False;       // read-only for now
}

//------------------------------------------------------------------
BOOL OCalcTable::DropImpl()
{
    return sal_False;       // read-only for now
}
//------------------------------------------------------------------
BOOL OCalcTable::InsertRow(OValueVector& rRow, BOOL bFlush,const Reference<XIndexAccess>& _xCols)
{
    return sal_False;       // read-only for now
}

//------------------------------------------------------------------
BOOL OCalcTable::UpdateRow(file::OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    return sal_False;       // read-only for now
}

//------------------------------------------------------------------
BOOL OCalcTable::DeleteRow(const OSQLColumns& _rCols)
{
    return sal_False;       // read-only for now
}

// -------------------------------------------------------------------------
Reference<XPropertySet> OCalcTable::isUniqueByColumnName(const ::rtl::OUString& _rColName)
{
    return Reference<XPropertySet>();
}
//------------------------------------------------------------------
BOOL OCalcTable::UpdateBuffer(OValueVector& rRow, OValueRow pOrgRow,const Reference<XIndexAccess>& _xCols)
{
    return sal_False;       // read-only for now
}
//------------------------------------------------------------------
BOOL OCalcTable::WriteBuffer()
{
    return sal_False;       // read-only for now
}
// -----------------------------------------------------------------------------

