/*************************************************************************
 *
 *  $RCSfile: SchXMLTableContext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:07:03 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
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

#include "SchXMLTableContext.hxx"
#include "SchXMLParagraphContext.hxx"
#include "SchXMLImport.hxx"
#include "transporttypes.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTSERIESADDRESS_HPP_
#include <com/sun/star/chart/ChartSeriesAddress.hpp>
#endif

using namespace com::sun::star;

// ----------------------------------------
// class SchXMLTableContext
// ----------------------------------------

SchXMLTableContext::SchXMLTableContext( SchXMLImportHelper& rImpHelper,
                                        SvXMLImport& rImport,
                                        const rtl::OUString& rLName,
                                        SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
    mrTable.nColumnIndex = -1;
    mrTable.nMaxColumnIndex = -1;
    mrTable.nRowIndex = -1;
    mrTable.aData.clear();
}

SchXMLTableContext::~SchXMLTableContext()
{
}

SvXMLImportContext *SchXMLTableContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;
    const SvXMLTokenMap& rTokenMap = mrImportHelper.GetTableElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ))
    {
        case XML_TOK_TABLE_HEADER_COLS:
        case XML_TOK_TABLE_COLUMNS:
            pContext = new SchXMLTableColumnsContext( mrImportHelper, GetImport(), rLocalName, mrTable );
            break;

        case XML_TOK_TABLE_COLUMN:
            pContext = new SchXMLTableColumnContext( mrImportHelper, GetImport(), rLocalName, mrTable );
            break;

        case XML_TOK_TABLE_HEADER_ROWS:
        case XML_TOK_TABLE_ROWS:
            pContext = new SchXMLTableRowsContext( mrImportHelper, GetImport(), rLocalName, mrTable );
            break;

        case XML_TOK_TABLE_ROW:
            pContext = new SchXMLTableRowContext( mrImportHelper, GetImport(), rLocalName, mrTable );
            break;

        default:
            pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

// ========================================
// classes for columns
// ========================================

// ----------------------------------------
// class SchXMLTableColumnsContext
// ----------------------------------------

SchXMLTableColumnsContext::SchXMLTableColumnsContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const rtl::OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
}

SchXMLTableColumnsContext::~SchXMLTableColumnsContext()
{
}

SvXMLImportContext* SchXMLTableColumnsContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_TABLE &&
        rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_table_column )))
    {
        pContext = new SchXMLTableColumnContext( mrImportHelper, GetImport(), rLocalName, mrTable );
    }
    else
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}

// ----------------------------------------
// class SchXMLTableColumnContext
// ----------------------------------------

SchXMLTableColumnContext::SchXMLTableColumnContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const rtl::OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
}

void SchXMLTableColumnContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // get number-columns-repeated attribute
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        rtl::OUString aLocalName;
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        if( nPrefix == XML_NAMESPACE_TABLE &&
            aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_number_columns_repeated )))
        {
            aValue = xAttrList->getValueByIndex( i );
            break;   // we only need this attribute
        }
    }

    if( aValue.getLength())
    {
        sal_Int32 nRepeated = aValue.toInt32();
        mrTable.nNumberOfColsEstimate += nRepeated;
    }
    else
    {
        mrTable.nNumberOfColsEstimate++;
    }
}

SchXMLTableColumnContext::~SchXMLTableColumnContext()
{
}

// ========================================
// classes for rows
// ========================================

// ----------------------------------------
// class SchXMLTableRowsContext
// ----------------------------------------

SchXMLTableRowsContext::SchXMLTableRowsContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const rtl::OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
}

SchXMLTableRowsContext::~SchXMLTableRowsContext()
{
}

SvXMLImportContext* SchXMLTableRowsContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    if( nPrefix == XML_NAMESPACE_TABLE &&
        rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_table_row )))
    {
        pContext = new SchXMLTableRowContext( mrImportHelper, GetImport(), rLocalName, mrTable );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

// ----------------------------------------
// class SchXMLTableRowContext
// ----------------------------------------

SchXMLTableRowContext::SchXMLTableRowContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const rtl::OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
    mrTable.nColumnIndex = -1;
    mrTable.nRowIndex++;

    std::vector< SchXMLCell > aNewRow;
    aNewRow.reserve( mrTable.nNumberOfColsEstimate );
    while( mrTable.aData.size() <= mrTable.nRowIndex )
        mrTable.aData.push_back( aNewRow );
}

SchXMLTableRowContext::~SchXMLTableRowContext()
{
}

SvXMLImportContext* SchXMLTableRowContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    // <table:table-cell> element
    if( nPrefix == XML_NAMESPACE_TABLE &&
        rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_table_cell )))
    {
        pContext = new SchXMLTableCellContext( mrImportHelper, GetImport(), rLocalName, mrTable );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}


// ========================================
// classes for cells and their content
// ========================================

// ----------------------------------------
// class SchXMLTableCellContext
// ----------------------------------------

SchXMLTableCellContext::SchXMLTableCellContext(
    SchXMLImportHelper& rImpHelper,
    SvXMLImport& rImport,
    const rtl::OUString& rLocalName,
    SchXMLTable& aTable ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TABLE, rLocalName ),
        mrImportHelper( rImpHelper ),
        mrTable( aTable )
{
}

SchXMLTableCellContext::~SchXMLTableCellContext()
{
}

void SchXMLTableCellContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
    rtl::OUString aValue;
    rtl::OUString aLocalName;
    rtl::OUString aCellContent;
    SchXMLCellType eValueType  = SCH_CELL_TYPE_UNKNOWN;
    const SvXMLTokenMap& rAttrTokenMap = mrImportHelper.GetCellAttrTokenMap();

    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
        USHORT nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

        switch( rAttrTokenMap.Get( nPrefix, aLocalName ))
        {
            case XML_TOK_CELL_VAL_TYPE:
                aValue = xAttrList->getValueByIndex( i );
                if( aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_float )))
                    eValueType = SCH_CELL_TYPE_FLOAT;
                else if( aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_string )))
                    eValueType = SCH_CELL_TYPE_STRING;
                break;

            case XML_TOK_CELL_VALUE:
                aCellContent = xAttrList->getValueByIndex( i );
                break;
        }
    }

    mbReadPara = sal_True;
    SchXMLCell aCell;
    aCell.eType = eValueType;

    if( eValueType == SCH_CELL_TYPE_FLOAT )
    {
        double fData;
        sal_Bool bResult = SvXMLUnitConverter::convertNumber( fData, aCellContent );
        DBG_ASSERT( bResult, "Error converting string to double" );

        aCell.fValue = fData;
        // dont read following <text:p> element
        mbReadPara = sal_False;
    }

    mrTable.aData[ mrTable.nRowIndex ].push_back( aCell );
    mrTable.nColumnIndex++;
    if( mrTable.nMaxColumnIndex < mrTable.nColumnIndex )
        mrTable.nMaxColumnIndex = mrTable.nColumnIndex;
}

SvXMLImportContext* SchXMLTableCellContext::CreateChildContext(
    USHORT nPrefix,
    const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    // <text:p> element
    if( mbReadPara &&
        nPrefix == XML_NAMESPACE_TEXT &&
        rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_p )))
    {
        // we have to read a string here (not a float)
        pContext = new SchXMLParagraphContext( GetImport(), rLocalName, maCellContent );
    }
    else
    {
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return pContext;
}

void SchXMLTableCellContext::EndElement()
{
    if( mbReadPara && maCellContent.getLength())
        mrTable.aData[ mrTable.nRowIndex ][ mrTable.nColumnIndex ].aString = maCellContent;
}

// ========================================

void SchXMLTableHelper::applyTable(
    const SchXMLTable& rTable,
    uno::Sequence< chart::ChartSeriesAddress >& rSeriesAddresses,
    rtl::OUString& rCategoriesAddress,
    uno::Reference< chart::XChartDocument > xChartDoc )
{
    // general note: series are always interpreted as columns in import

    // first check if data can be attached to an appropriate object
    if( rTable.nRowIndex > -1 &&
        xChartDoc.is())
    {
        uno::Reference< chart::XChartDataArray > xData( xChartDoc->getData(), uno::UNO_QUERY );
        if( xData.is())
        {
            sal_Int32 nNumSeriesAddresses = rSeriesAddresses.getLength();
            sal_Int32 nNumLabelAddresses = nNumSeriesAddresses;
            sal_Int32 nDomainOffset = 0;
            sal_Int32 nNumAddrSize = nNumSeriesAddresses;

            uno::Reference< chart::XChartData > xChartData( xData, uno::UNO_QUERY );
            if( xChartData.is())
            {
                sal_Int32 nColumns = 0;
                sal_Int32 nRows = 0;
                sal_Int32 i, j;

                // set data
                if( nNumSeriesAddresses )
                {
                    // get NaN
                    double fSolarNaN;
                    SolarMath::SetNAN( fSolarNaN, FALSE );
                    double fNaN = fSolarNaN;
                    fNaN = xChartData->getNotANumber();

                    // convert data from std::vector to uno::Sequence
                    // ----------------------------------------------

                    // determine size of data
                    std::vector< SchNumericCellRangeAddress > aNumericAddresses( nNumSeriesAddresses );

                    for( i = 0; i < nNumSeriesAddresses; i++ )
                    {
                        if( rSeriesAddresses[ i ].DomainRangeAddresses.getLength())
                        {
                            GetCellRangeAddress( rSeriesAddresses[ i ].DomainRangeAddresses[ 0 ],
                                                 aNumericAddresses[ i + nDomainOffset ] );
                            AdjustMax( aNumericAddresses[ i + nDomainOffset ], nRows, nColumns );
                            nDomainOffset++;
                            aNumericAddresses.reserve( nNumSeriesAddresses + nDomainOffset );
                        }

                        GetCellRangeAddress( rSeriesAddresses[ i ].DataRangeAddress,
                                             aNumericAddresses[ i + nDomainOffset ] );
                        AdjustMax( aNumericAddresses[ i + nDomainOffset ], nRows, nColumns );
                    }
                    nNumAddrSize += nDomainOffset;

                    // allocate memory for sequence
                    uno::Sequence< uno::Sequence< double > > aSequence( nRows );
                    for( i = 0; i < nRows; i++ )
                    {
                        aSequence[ i ].realloc( nColumns );

                        // initialize values with NaN
                        for( j = 0; j < nColumns; j++ )
                            aSequence[ i ][ j ] = fNaN;
                    }

                    // copy data
                    for( i = 0; i < nNumAddrSize; i++ )
                        PutTableContentIntoSequence( rTable, aNumericAddresses[ i ], i, aSequence );

                    // set data to XChartDataArray
                    xData->setData( aSequence );
                }

                // set labels
                uno::Sequence< rtl::OUString > aLabels;
                aLabels.realloc( nNumAddrSize );

                sal_Int32 nRow, nCol;
                for( i = 0; i < nNumSeriesAddresses; i++ )
                {
                    if( rSeriesAddresses[ i ].LabelAddress.getLength())
                    {
                        GetCellAddress( rSeriesAddresses[ i ].LabelAddress, nCol, nRow );
                        aLabels[ i + nDomainOffset ] = rTable.aData[ nRow ][ nCol ].aString;
                    }
                }
                xData->setColumnDescriptions( aLabels );

                // set categories
                aLabels = xData->getRowDescriptions();

                if( rCategoriesAddress.getLength())
                {
                    SchNumericCellRangeAddress aAddress;
                    if( GetCellRangeAddress( rCategoriesAddress, aAddress ))
                    {
                        uno::Sequence< rtl::OUString > aLabels;

                        if( aAddress.nCol1 == aAddress.nCol2 )
                        {
                            sal_Int32 nWidth = aAddress.nRow2 - aAddress.nRow1 + 1;
                            aLabels.realloc( nWidth );

                            for( i = 0; i < nWidth; i++ )
                            {
                                DBG_ASSERT( rTable.aData[ aAddress.nRow1 + i ][ aAddress.nCol1 ].eType == SCH_CELL_TYPE_STRING, "expecting string" );
                                aLabels[ i ] = rTable.aData[ aAddress.nRow1 + i ][ aAddress.nCol1 ].aString;
                            }
                        }
                        else
                        {
                            DBG_ASSERT( aAddress.nRow1 == aAddress.nRow2, "range must be in one row or one column" );

                            sal_Int32 nWidth = aAddress.nCol2 - aAddress.nCol1 + 1;
                            aLabels.realloc( nWidth );

                            for( sal_Int32 i = 0; i < nWidth; i++ )
                            {
                                DBG_ASSERT( rTable.aData[ aAddress.nRow1 ][ aAddress.nCol1 + i ].eType == SCH_CELL_TYPE_STRING, "expecting string" );
                                aLabels[ i ] = rTable.aData[ aAddress.nRow1 ][ aAddress.nCol1 + i ].aString;
                            }
                        }

                        xData->setRowDescriptions( aLabels );
                    }
                }

                // apply data to chart
                // not necessary ?
                // xChartDoc->attachData( xChartData );
            }
        }
    }
}

void SchXMLTableHelper::AdjustMax( const SchNumericCellRangeAddress& rAddr,
                                   sal_Int32& nRows, sal_Int32& nColumns )
{
    // rows and columns are both mapped to columns ( == series )
    if( rAddr.nCol1 == rAddr.nCol2 )
    {
        if( rAddr.nRow1 > nRows )
            nRows = rAddr.nRow1;
        if( rAddr.nRow2 > nRows )
            nRows = rAddr.nRow2;
        if( rAddr.nCol1 > nColumns )
            nColumns = rAddr.nCol1;
        if( rAddr.nCol2 > nColumns )
            nColumns = rAddr.nCol2;
    }
    else
    {
        DBG_ASSERT( rAddr.nRow1 == rAddr.nRow2, "row indexes should be equal" );
        if( rAddr.nRow1 > nRows )
            nColumns = rAddr.nRow1;
        if( rAddr.nRow2 > nRows )
            nColumns = rAddr.nRow2;
        if( rAddr.nCol1 > nColumns )
            nRows = rAddr.nCol1;
        if( rAddr.nCol2 > nColumns )
            nRows = rAddr.nCol2;
    }
}

void SchXMLTableHelper::GetCellAddress( const rtl::OUString& rStr, sal_Int32& rCol, sal_Int32& rRow )
{
    sal_Int32 nPos = rStr.indexOf( sal_Unicode( '.' ));
    if( nPos != -1 )
    {
        // currently just one letter is accepted
        sal_Unicode aLetter = rStr.getStr()[ nPos + 1 ];
        if( 'a' <= aLetter && aLetter <= 'z' )
            rCol = aLetter - 'a';
        else
            rCol = aLetter - 'A';

        rRow = (rStr.copy( nPos + 2 )).toInt32() - 1;
    }
}

sal_Bool SchXMLTableHelper::GetCellRangeAddress(
    const rtl::OUString& rStr, SchNumericCellRangeAddress& rResult )
{
    sal_Int32 nBreakAt = rStr.indexOf( sal_Unicode( ':' ));
    if( nBreakAt != -1 )
    {
        GetCellAddress( rStr.copy( 0, nBreakAt ), rResult.nCol1, rResult.nRow1 );
        GetCellAddress( rStr.copy( nBreakAt + 1 ), rResult.nCol2, rResult.nRow2 );
        return sal_True;
    }

    return sal_False;
}


// returns true if datarange was inside one column
void SchXMLTableHelper::PutTableContentIntoSequence(
    const SchXMLTable& rTable,
    SchNumericCellRangeAddress& rAddress,
    sal_Int32 nSeriesIndex,
    uno::Sequence< uno::Sequence< double > >& aSequence )
{
    if( rAddress.nCol2 > rTable.nMaxColumnIndex + 1 ||
        rAddress.nRow2 > rTable.nRowIndex + 1 )
    {
        DBG_ERROR( "Invalid references" );
        //ToDo: strip the range
        return;
    }

    // currently only ranges that span one row or one column are supported

    sal_Int32 nSeqPos = 0;
    uno::Sequence< double >* pSeqArray = aSequence.getArray();

    // same column
    if( rAddress.nCol1 == rAddress.nCol2 )
    {
        if( rAddress.nRow1 <= rAddress.nRow2 )
        {
            for( sal_Int32 nRow = rAddress.nRow1; nRow <= rAddress.nRow2; nRow++, nSeqPos++ )
            {
                DBG_ASSERT( rTable.aData[ nRow ][ rAddress.nCol1 ].eType != SCH_CELL_TYPE_UNKNOWN, "trying to refer to unknown cell" );
                pSeqArray[ nSeqPos ][ nSeriesIndex ] = rTable.aData[ nRow ][ rAddress.nCol1 ].fValue;
            }
        }
        else    // reverse
        {
            for( sal_Int32 nRow = rAddress.nRow1; nRow >= rAddress.nRow2; nRow--, nSeqPos++ )
            {
                DBG_ASSERT( rTable.aData[ nRow ][ rAddress.nCol1 ].eType != SCH_CELL_TYPE_UNKNOWN, "trying to refer to unknown cell" );
                pSeqArray[ nSeqPos ][ nSeriesIndex ] = rTable.aData[ nRow ][ rAddress.nCol1 ].fValue;
            }
        }
    }
    else    // same row
    {
        DBG_ASSERT( rAddress.nRow1 == rAddress.nRow2, "range must be in one row or one column" );

        if( rAddress.nCol1 <= rAddress.nCol2 )
        {
            for( sal_Int32 nCol = rAddress.nCol1; nCol <= rAddress.nCol2; nCol++, nSeqPos++ )
            {
                DBG_ASSERT( rTable.aData[ rAddress.nRow1 ][ nCol ].eType != SCH_CELL_TYPE_UNKNOWN, "trying to refer to unknown cell" );
                pSeqArray[ nSeqPos ][ nSeriesIndex ] = rTable.aData[ rAddress.nRow1 ][ nCol ].fValue;
            }
        }
        else    // reverse
        {
            for( sal_Int32 nCol = rAddress.nCol1; nCol >= rAddress.nCol2; nCol--, nSeqPos++ )
            {
                DBG_ASSERT( rTable.aData[ rAddress.nRow1 ][ nCol ].eType != SCH_CELL_TYPE_UNKNOWN, "trying to refer to unknown cell" );
                pSeqArray[ nSeqPos ][ nSeriesIndex ] = rTable.aData[ rAddress.nRow1 ][ nCol ].fValue;
            }
        }
    }
}

