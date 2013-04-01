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

#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <rtl/instance.hxx>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertyset.hxx"

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

void CreateTableRows( uno::Reference< XTableRows > xTableRows, const std::vector< TableRow >& rvTableRows )
{
    if ( rvTableRows.size() > 1 )
        xTableRows->insertByIndex( 0, rvTableRows.size() - 1 );
    std::vector< TableRow >::const_iterator aTableRowIter( rvTableRows.begin() );
    uno::Reference< container::XIndexAccess > xIndexAccess( xTableRows, UNO_QUERY_THROW );
    const OUString sHeight("Height");
    for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
    {
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( sHeight, Any( static_cast< sal_Int32 >( aTableRowIter->getHeight() / 360 ) ) );
        ++aTableRowIter;
    }
}

void CreateTableColumns( Reference< XTableColumns > xTableColumns, const std::vector< sal_Int32 >& rvTableGrid )
{
    if ( rvTableGrid.size() > 1 )
        xTableColumns->insertByIndex( 0, rvTableGrid.size() - 1 );
    std::vector< sal_Int32 >::const_iterator aTableGridIter( rvTableGrid.begin() );
    uno::Reference< container::XIndexAccess > xIndexAccess( xTableColumns, UNO_QUERY_THROW );
    const OUString sWidth("Width");
    for ( sal_Int32 n = 0; n < xIndexAccess->getCount(); n++ )
    {
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

namespace
{
    struct theDefaultTableStyle : public ::rtl::Static< TableStyle, theDefaultTableStyle > {};
}

const TableStyle& TableProperties::getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase )
{
    ::oox::core::XmlFilterBase& rBase( const_cast< ::oox::core::XmlFilterBase& >( rFilterBase ) );

    TableStyle* pTableStyle = NULL;
    if ( mpTableStyle )
        pTableStyle = &*mpTableStyle;
    else if ( rBase.getTableStyles() )
    {
        const std::vector< TableStyle >& rTableStyles( rBase.getTableStyles()->getTableStyles() );
        const OUString aStyleId( getStyleId().isEmpty() ? rBase.getTableStyles()->getDefaultStyleId() : getStyleId() );
        std::vector< TableStyle >::const_iterator aIter( rTableStyles.begin() );
        while( aIter != rTableStyles.end() )
        {
            if ( const_cast< TableStyle& >( *aIter ).getStyleId() == aStyleId )
            {
                pTableStyle = &const_cast< TableStyle& >( *aIter );
                break;  // we get the correct style
            }
            ++aIter;
        }
    }

    if ( !pTableStyle )
        return theDefaultTableStyle::get();

    return *pTableStyle;
}

void TableProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XPropertySet >& xPropSet, TextListStylePtr pMasterTextListStyle )
{
    uno::Reference< XColumnRowRange > xColumnRowRange(
         xPropSet->getPropertyValue("Model"), uno::UNO_QUERY_THROW );

    CreateTableColumns( xColumnRowRange->getColumns(), mvTableGrid );
    CreateTableRows( xColumnRowRange->getRows(), mvTableRows );

    const TableStyle& rTableStyle( getUsedTableStyle( rFilterBase ) );
    sal_Int32 nRow = 0;
    const std::vector< TableRow >::const_iterator aTableRowEnd( mvTableRows.end() );
    for (std::vector< TableRow >::iterator aTableRowIter( mvTableRows.begin() );
         aTableRowIter != aTableRowEnd ; ++aTableRowIter, ++nRow)
    {
        sal_Int32 nColumn = 0;
        const std::vector< TableCell >::const_iterator aTableCellEnd( aTableRowIter->getTableCells().end() );
        for (std::vector< TableCell >::iterator aTableCellIter( aTableRowIter->getTableCells().begin() );
            aTableCellIter != aTableCellEnd ; ++aTableCellIter, ++nColumn)
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
        }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
