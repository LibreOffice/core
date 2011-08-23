/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertyset.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;


namespace oox { namespace drawingml { namespace table {

TableProperties::TableProperties()
: mbRtl( sal_False )
, mbFirstRow( sal_False )
, mbFirstCol( sal_False )
, mbLastRow( sal_False )
, mbLastCol( sal_False )
, mbBandRow( sal_False )
, mbBandCol( sal_False )
{
}
TableProperties::~TableProperties()
{
}

void TableProperties::apply( const TablePropertiesPtr& /* rSourceTableProperties */ )
{
}

void CreateTableRows( uno::Reference< XTableRows > xTableRows, const std::vector< TableRow >& rvTableRows )
{
    if ( rvTableRows.size() > 1 )
        xTableRows->insertByIndex( 0, rvTableRows.size() - 1 );
    std::vector< TableRow >::const_iterator aTableRowIter( rvTableRows.begin() );
    uno::Reference< container::XIndexAccess > xIndexAccess( xTableRows, UNO_QUERY_THROW );
    for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
    {
        static const rtl::OUString	sHeight( RTL_CONSTASCII_USTRINGPARAM ( "Height" ) );
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( sHeight, Any( static_cast< sal_Int32 >( aTableRowIter->getHeight() / 360 ) ) );
        aTableRowIter++;
    }
}

void CreateTableColumns( Reference< XTableColumns > xTableColumns, const std::vector< sal_Int32 >& rvTableGrid )
{
    if ( rvTableGrid.size() > 1 )
        xTableColumns->insertByIndex( 0, rvTableGrid.size() - 1 );
    std::vector< sal_Int32 >::const_iterator aTableGridIter( rvTableGrid.begin() );
    uno::Reference< container::XIndexAccess > xIndexAccess( xTableColumns, UNO_QUERY_THROW );
    for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
    {
        static const rtl::OUString	sWidth( RTL_CONSTASCII_USTRINGPARAM ( "Width" ) );
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( sWidth, Any( static_cast< sal_Int32 >( *aTableGridIter++ / 360 ) ) );
    }
}

void MergeCells( const uno::Reference< XTable >& xTable, sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan )
{
   if( xTable.is() ) try
   {
       Reference< XMergeableCellRange > xRange( xTable->createCursorByRange( xTable->getCellRangeByPosition( nCol, nRow,nCol + nColSpan - 1, nRow + nRowSpan - 1 ) ), UNO_QUERY_THROW );
       if( xRange->isMergeable() )
               xRange->merge();
   }
   catch( Exception& )
   {
   }
} 

static TableStyle* pDefaultTableStyle = new TableStyle();

const TableStyle& TableProperties::getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase )
{
    ::oox::core::XmlFilterBase& rBase( const_cast< ::oox::core::XmlFilterBase& >( rFilterBase ) );

    TableStyle* pTableStyle = NULL;
    if ( mpTableStyle )
        pTableStyle = &*mpTableStyle;
    else if ( rBase.getTableStyles() )
    {
        const std::vector< TableStyle >& rTableStyles( rBase.getTableStyles()->getTableStyles() );
        const rtl::OUString aStyleId( getStyleId().getLength() ? getStyleId() : rBase.getTableStyles()->getDefaultStyleId() );
        std::vector< TableStyle >::const_iterator aIter( rTableStyles.begin() );
        while( aIter != rTableStyles.end() )
        {
            if ( const_cast< TableStyle& >( *aIter ).getStyleId() == aStyleId )
            {
                pTableStyle = &const_cast< TableStyle& >( *aIter );
                break;	// we get the correct style
            }
            aIter++;
        }
    }
    if ( !pTableStyle )
        pTableStyle = pDefaultTableStyle;
    
    return *pTableStyle;
}

void TableProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XPropertySet >& xPropSet, TextListStylePtr pMasterTextListStyle )
{
    TableStyleListPtr( const_cast< ::oox::core::XmlFilterBase& >( rFilterBase ).getTableStyles() );

    uno::Reference< XColumnRowRange > xColumnRowRange(
         xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) ), uno::UNO_QUERY_THROW );

    CreateTableColumns( xColumnRowRange->getColumns(), mvTableGrid );
    CreateTableRows( xColumnRowRange->getRows(), mvTableRows );

    const TableStyle& rTableStyle( getUsedTableStyle( rFilterBase ) );
    sal_Int32 nRow = 0;
    std::vector< TableRow >::iterator aTableRowIter( mvTableRows.begin() );
    while( aTableRowIter != mvTableRows.end() )
    {
        sal_Int32 nColumn = 0;
        std::vector< TableCell >::iterator aTableCellIter( aTableRowIter->getTableCells().begin() );
        while( aTableCellIter != aTableRowIter->getTableCells().end() )
        {
            TableCell& rTableCell( *aTableCellIter );
            if ( !rTableCell.getvMerge() && !rTableCell.gethMerge() )
            {
                uno::Reference< XTable > xTable( xColumnRowRange, uno::UNO_QUERY_THROW );
                if ( ( rTableCell.getRowSpan() > 1 ) || ( rTableCell.getGridSpan() > 1 ) )
                    MergeCells( xTable, nColumn, nRow, rTableCell.getGridSpan(), rTableCell.getRowSpan() );

                Reference< XCellRange > xCellRange( xTable, UNO_QUERY_THROW );
                rTableCell.pushToXCell( rFilterBase, pMasterTextListStyle, xCellRange->getCellByPosition( nColumn, nRow ), *this, rTableStyle,
                    nColumn, aTableRowIter->getTableCells().size(), nRow, mvTableRows.size() );
            }
            nColumn++;
            aTableCellIter++;
        }
        nRow++;
        aTableRowIter++;
    }
}

} } }
