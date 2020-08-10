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

#include <drawingml/table/tableproperties.hxx>
#include <drawingml/table/tablestylelist.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textparagraph.hxx>
#include <oox/drawingml/drawingmltypes.hxx>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <rtl/instance.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include "predefined-table-styles.cxx"

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;

namespace oox::drawingml::table {

TableProperties::TableProperties()
: mbFirstRow( false )
, mbFirstCol( false )
, mbLastRow( false )
, mbLastCol( false )
, mbBandRow( false )
, mbBandCol( false )
{
    maBgColor.setUnused();
}

static void CreateTableRows( const uno::Reference< XTableRows >& xTableRows, const std::vector< TableRow >& rvTableRows )
{
    if ( rvTableRows.size() > 1 )
        xTableRows->insertByIndex( 0, rvTableRows.size() - 1 );
    std::vector< TableRow >::const_iterator aTableRowIter( rvTableRows.begin() );
    uno::Reference< container::XIndexAccess > xIndexAccess( xTableRows, UNO_QUERY_THROW );
    sal_Int32 nCols = std::min<sal_Int32>(xIndexAccess->getCount(), rvTableRows.size());
    for (sal_Int32 n = 0; n < nCols; ++n)
    {
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( "Height", Any( static_cast< sal_Int32 >( aTableRowIter->getHeight() / 360 ) ) );
        ++aTableRowIter;
    }
}

static void CreateTableColumns( const Reference< XTableColumns >& xTableColumns, const std::vector< sal_Int32 >& rvTableGrid )
{
    if ( rvTableGrid.size() > 1 )
        xTableColumns->insertByIndex( 0, rvTableGrid.size() - 1 );
    std::vector< sal_Int32 >::const_iterator aTableGridIter( rvTableGrid.begin() );
    uno::Reference< container::XIndexAccess > xIndexAccess( xTableColumns, UNO_QUERY_THROW );
    sal_Int32 nCols = std::min<sal_Int32>(xIndexAccess->getCount(), rvTableGrid.size());
    for (sal_Int32 n = 0; n < nCols; ++n)
    {
        Reference< XPropertySet > xPropSet( xIndexAccess->getByIndex( n ), UNO_QUERY_THROW );
        xPropSet->setPropertyValue( "Width", Any( static_cast< sal_Int32 >( *aTableGridIter++ / 360 ) ) );
    }
}

static void MergeCells( const uno::Reference< XTable >& xTable, sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nColSpan, sal_Int32 nRowSpan )
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

const TableStyle& TableProperties::getUsedTableStyle( const ::oox::core::XmlFilterBase& rFilterBase, std::unique_ptr<TableStyle>& rTableStyleToDelete )
{
    ::oox::core::XmlFilterBase& rBase( const_cast< ::oox::core::XmlFilterBase& >( rFilterBase ) );

    TableStyle* pTableStyle = nullptr;
    if ( mpTableStyle )
        pTableStyle = &*mpTableStyle;
    else if ( !getStyleId().isEmpty() && rBase.getTableStyles() )
    {
        const std::vector< TableStyle >& rTableStyles( rBase.getTableStyles()->getTableStyles() );
        const OUString aStyleId( getStyleId() );

        for (auto const& tableStyle : rTableStyles)
        {
            if ( const_cast< TableStyle& >(tableStyle).getStyleId() == aStyleId )
            {
                pTableStyle = &const_cast< TableStyle& >(tableStyle);
                break;  // we get the correct style
            }
        }
        //if the pptx just has table style id, but no table style content, we will create the table style ourselves
        if (!pTableStyle)
        {
            rTableStyleToDelete = CreateTableStyle(aStyleId);
            pTableStyle = rTableStyleToDelete.get();
        }
    }

    if ( !pTableStyle )
        return theDefaultTableStyle::get();

    return *pTableStyle;
}

void TableProperties::pushToPropSet(const ::oox::core::XmlFilterBase& rFilterBase,
                                    const Reference<XPropertySet>& xPropSet,
                                    const TextListStylePtr& pMasterTextListStyle)
{
    uno::Reference<XColumnRowRange> xColumnRowRange(xPropSet->getPropertyValue("Model"),
                                                    uno::UNO_QUERY_THROW);

    CreateTableColumns(xColumnRowRange->getColumns(), mvTableGrid);
    CreateTableRows(xColumnRowRange->getRows(), mvTableRows);

    std::unique_ptr<TableStyle> xTableStyleToDelete;
    const TableStyle& rTableStyle(getUsedTableStyle(rFilterBase, xTableStyleToDelete));
    sal_Int32 nRow = 0;

    for (auto& tableRow : mvTableRows)
    {
        sal_Int32 nColumn = 0;
        sal_Int32 nColumnSize = mvTableGrid.size();
        sal_Int32 nRemovedColumn = 0; //

        for (sal_Int32 nColIndex = 0; nColIndex < nColumnSize; nColIndex++)
        {
            TableCell& rTableCell(tableRow.getTableCells().at(nColIndex));

            if (!rTableCell.getvMerge() && !rTableCell.gethMerge())
            {
                uno::Reference<XTable> xTable(xColumnRowRange, uno::UNO_QUERY_THROW);
                bool bMerged = false;

                if ((rTableCell.getRowSpan() > 1) || (rTableCell.getGridSpan() > 1))
                {
                    MergeCells(xTable, nColumn, nRow, rTableCell.getGridSpan(),
                               rTableCell.getRowSpan());

                    if (rTableCell.getGridSpan() > 1)
                    {
                        nRemovedColumn = (rTableCell.getGridSpan() - 1);
                        // MergeCells removes columns. Our loop does not know about those
                        // removed columns and we skip handling those removed columns.
                        nColIndex += nRemovedColumn;
                        // It will adjust new column number after push current column's
                        // props with pushToXCell.
                        bMerged = true;
                    }
                }

                Reference<XCellRange> xCellRange(xTable, UNO_QUERY_THROW);
                Reference<XCell> xCell;

                if (nRemovedColumn)
                {
                    try
                    {
                        xCell = xCellRange->getCellByPosition(nColumn, nRow);
                    }
                    // Exception can come from TableModel::getCellByPosition when a column
                    // is removed while merging columns. So adjust again here.
                    catch (Exception&)
                    {
                        xCell = xCellRange->getCellByPosition(nColumn - nRemovedColumn, nRow);
                    }
                }
                else
                    xCell = xCellRange->getCellByPosition(nColumn, nRow);

                rTableCell.pushToXCell(rFilterBase, pMasterTextListStyle, xCell, *this, rTableStyle,
                                       nColumn, tableRow.getTableCells().size() - 1, nRow,
                                       mvTableRows.size() - 1);
                if (bMerged)
                    nColumn += nRemovedColumn;
            }
            ++nColumn;
        }
        ++nRow;
    }

    xTableStyleToDelete.reset();
}

void TableProperties::pullFromTextBody(oox::drawingml::TextBodyPtr pTextBody, sal_Int32 nShapeWidth, bool bhasSameSubTypeIndex, bool bMaster)
{
    // Create table grid and a single row.
    sal_Int32 nNumCol = pTextBody->getTextProperties().mnNumCol;
    std::vector<sal_Int32>& rTableGrid(getTableGrid());
    std::vector<drawingml::table::TableRow>& rTableRows(getTableRows());
    sal_Int32 nColWidth = nShapeWidth / nNumCol;

    if(!bhasSameSubTypeIndex)
    {
        for (sal_Int32 nCol = 0; nCol < nNumCol; ++nCol)
            rTableGrid.push_back(nColWidth);

        rTableRows.emplace_back();
    }

    if(rTableRows.empty())
        rTableRows.emplace_back();

    oox::drawingml::table::TableRow& rTableRow = rTableRows.back();
    std::vector<oox::drawingml::table::TableCell>& rTableCells = rTableRow.getTableCells();

    // Create the cells and distribute the paragraphs from pTextBody.
    sal_Int32 nNumPara = pTextBody->getParagraphs().size();
    sal_Int32 nParaPerCol = std::ceil(double(nNumPara) / nNumCol);
    // Font scale of text body will be applied at a text run level.
    sal_Int32 nFontScale = pTextBody->getTextProperties().mnFontScale;
    size_t nPara = 0;
    for (sal_Int32 nCol = 0; nCol < nNumCol; ++nCol)
    {
        rTableCells.emplace_back();
        oox::drawingml::table::TableCell& rTableCell = rTableCells.at(nCol);
        TextBodyPtr pCellTextBody = std::make_shared<TextBody>();
        rTableCell.setTextBody(pCellTextBody);

        // Copy properties provided by <a:lstStyle>.
        pCellTextBody->getTextListStyle() = pTextBody->getTextListStyle();

        if (bMaster)
            continue;

        for (sal_Int32 nParaInCol = 0; nParaInCol < nParaPerCol; ++nParaInCol)
        {
            if (nPara < pTextBody->getParagraphs().size())
            {
                std::shared_ptr<oox::drawingml::TextParagraph> pParagraph
                    = pTextBody->getParagraphs()[nPara];
                if (nFontScale != 100000)
                {
                    for (auto& pRun : pParagraph->getRuns())
                        pRun->getTextCharacterProperties().moFontScale = nFontScale;
                }
                pCellTextBody->appendParagraph(pParagraph);
            }
            ++nPara;
        }
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
