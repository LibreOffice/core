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
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <rtl/instance.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/tokens.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::table;

namespace oox { namespace drawingml { namespace table {

TableProperties::TableProperties()
: mbFirstRow( false )
, mbFirstCol( false )
, mbLastRow( false )
, mbLastCol( false )
, mbBandRow( false )
, mbBandCol( false )
{
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

//for pptx just has table style id
static void SetTableStyleProperties(TableStyle* &pTableStyle , sal_Int32 tblFillClr, sal_Int32 tblTextClr, sal_Int32 lineBdrClr)
{
    //whole table fill style and color
    oox::drawingml::FillPropertiesPtr pWholeTabFillProperties( new oox::drawingml::FillProperties );
    pWholeTabFillProperties->moFillType.set(XML_solidFill);
    pWholeTabFillProperties->maFillColor.setSchemeClr(tblFillClr);
    pWholeTabFillProperties->maFillColor.addTransformation(XML_tint,20000);
    pTableStyle->getWholeTbl().getFillProperties() = pWholeTabFillProperties;
    //whole table text color
    ::oox::drawingml::Color tableTextColor;
    tableTextColor.setSchemeClr(tblTextClr);
    pTableStyle->getWholeTbl().getTextColor() = tableTextColor;
    //whole table line border
    oox::drawingml::LinePropertiesPtr pLeftBorder( new oox::drawingml::LineProperties);
    pLeftBorder->moLineWidth = 12700;
    pLeftBorder->moPresetDash = XML_sng;
    pLeftBorder->maLineFill.moFillType.set(XML_solidFill);
    pLeftBorder->maLineFill.maFillColor.setSchemeClr(lineBdrClr);
    pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_left,pLeftBorder));
    pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_right,pLeftBorder));
    pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_top,pLeftBorder));
    pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_bottom,pLeftBorder));
    pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_insideH,pLeftBorder));
    pTableStyle->getWholeTbl().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_insideV,pLeftBorder));

    //Band1H style
    oox::drawingml::FillPropertiesPtr pBand1HFillProperties( new oox::drawingml::FillProperties );
    pBand1HFillProperties->moFillType.set(XML_solidFill);
    pBand1HFillProperties->maFillColor.setSchemeClr(tblFillClr);
    pBand1HFillProperties->maFillColor.addTransformation(XML_tint,40000);
    pTableStyle->getBand1H().getFillProperties() = pBand1HFillProperties;

    //Band1V style
    pTableStyle->getBand1V().getFillProperties() = pBand1HFillProperties;

    //tet bold for 1st row/last row/column
    ::boost::optional< sal_Bool > textBoldStyle(true);
    pTableStyle->getFirstRow().getTextBoldStyle() = textBoldStyle;
    pTableStyle->getLastRow().getTextBoldStyle() = textBoldStyle;
    pTableStyle->getFirstCol().getTextBoldStyle() = textBoldStyle;
    pTableStyle->getLastCol().getTextBoldStyle() = textBoldStyle;
}

static TableStyle* CreateTableStyle(const OUString& styleId)
{
    TableStyle* pTableStyle = nullptr;

    // It is a bit silly to handle styleIds specifically and separately like this. Also note that
    // the first two code blocks below are mostly copy-pasted, modulo the comments and the fact that
    // one uses XML_accent1 and the other XML_accent2. Presumably it would be better to use a
    // table-based approach, to have a mapping from style ids to the parameters that actually change
    // between styles. See
    // https://msdn.microsoft.com/en-us/library/office/hh273476(v=office.14).aspx for an unofficial
    // list of these ids (that is said on that page to be undocumented, which doesn't sound credible?).

    // But the odd thing is that even if the code here handles only three specific style ids, if I
    // edit the bug doc from AOO bug #i120723# (for which this code was added) and choose some other
    // style, the table shows up mostly fine in Impress anyway. Thus I wonder whether this whole
    // code is actually unnecessary, and the .pptx contains explicit values for all the components
    // of a style anyway, and LibreOffice's code (at least now, maybe not when this code was added
    // to AOO in 2012) knows to look at that?

    if(styleId == "{5C22544A-7EE6-4342-B048-85BDC9FD1C3A}") {           //Medium Style 2 Accent 1
        pTableStyle = new TableStyle();
        //first row style
        //fill color and type
        oox::drawingml::FillPropertiesPtr pFirstRowFillProperties( new oox::drawingml::FillProperties );
        pFirstRowFillProperties->moFillType.set(XML_solidFill);
        pFirstRowFillProperties->maFillColor.setSchemeClr(XML_accent1);
        pTableStyle->getFirstRow().getFillProperties() = pFirstRowFillProperties;
        //text color
        ::oox::drawingml::Color firstRowTextColor;
        firstRowTextColor.setSchemeClr(XML_lt1);
        pTableStyle->getFirstRow().getTextColor() = firstRowTextColor;
        //bottom line border
        oox::drawingml::LinePropertiesPtr pFirstBottomBorder( new oox::drawingml::LineProperties);
        pFirstBottomBorder->moLineWidth = 38100;
        pFirstBottomBorder->moPresetDash = XML_sng;
        pFirstBottomBorder->maLineFill.moFillType.set(XML_solidFill);
        pFirstBottomBorder->maLineFill.maFillColor.setSchemeClr(XML_lt1);
        pTableStyle->getFirstRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_bottom,pFirstBottomBorder));

        //last row style
        pTableStyle->getLastRow().getFillProperties() = pFirstRowFillProperties;
        pTableStyle->getLastRow().getTextColor() = firstRowTextColor;
        pTableStyle->getLastRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_top,pFirstBottomBorder));

        //first column style
        pTableStyle->getFirstRow().getFillProperties() = pFirstRowFillProperties;
        pTableStyle->getFirstRow().getTextColor() = firstRowTextColor;

        //last column style
        pTableStyle->getLastCol().getFillProperties() = pFirstRowFillProperties;
        pTableStyle->getLastCol().getTextColor() = firstRowTextColor;

        SetTableStyleProperties(pTableStyle, XML_accent1, XML_dk1, XML_lt1);
    }
    else if (styleId == "{21E4AEA4-8DFA-4A89-87EB-49C32662AFE0}")         //Medium Style 2 Accent 2
    {
        pTableStyle = new TableStyle();
        oox::drawingml::FillPropertiesPtr pFirstRowFillProperties( new oox::drawingml::FillProperties );
        pFirstRowFillProperties->moFillType.set(XML_solidFill);
        pFirstRowFillProperties->maFillColor.setSchemeClr(XML_accent2);
        pTableStyle->getFirstRow().getFillProperties() = pFirstRowFillProperties;

        ::oox::drawingml::Color firstRowTextColor;
        firstRowTextColor.setSchemeClr(XML_lt1);
        pTableStyle->getFirstRow().getTextColor() = firstRowTextColor;

        oox::drawingml::LinePropertiesPtr pFirstBottomBorder( new oox::drawingml::LineProperties);
        pFirstBottomBorder->moLineWidth = 38100;
        pFirstBottomBorder->moPresetDash = XML_sng;
        pFirstBottomBorder->maLineFill.moFillType.set(XML_solidFill);
        pFirstBottomBorder->maLineFill.maFillColor.setSchemeClr(XML_lt1);
        pTableStyle->getFirstRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_bottom,pFirstBottomBorder));

        pTableStyle->getLastRow().getFillProperties() = pFirstRowFillProperties;
        pTableStyle->getLastRow().getTextColor() = firstRowTextColor;
        pTableStyle->getLastRow().getLineBorders().insert(std::pair<sal_Int32, ::oox::drawingml::LinePropertiesPtr>(XML_top,pFirstBottomBorder));

        pTableStyle->getFirstCol().getFillProperties() = pFirstRowFillProperties;
        pTableStyle->getFirstCol().getTextColor() = firstRowTextColor;

        pTableStyle->getLastCol().getFillProperties() = pFirstRowFillProperties;
        pTableStyle->getLastCol().getTextColor() = firstRowTextColor;

        SetTableStyleProperties(pTableStyle, XML_accent2, XML_dk1, XML_lt1);
    }
    else if (styleId == "{C4B1156A-380E-4F78-BDF5-A606A8083BF9}")         //Medium Style 4 Accent 4
    {
        pTableStyle = new TableStyle();
        SetTableStyleProperties(pTableStyle, XML_accent4, XML_dk1, XML_accent4);
    }

    return pTableStyle;
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
            rTableStyleToDelete.reset(CreateTableStyle(aStyleId));
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
        sal_Int32 nColumnSize = tableRow.getTableCells().size();
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

void TableProperties::pullFromTextBody(oox::drawingml::TextBodyPtr pTextBody, sal_Int32 nShapeWidth)
{
    // Create table grid and a single row.
    sal_Int32 nNumCol = pTextBody->getTextProperties().mnNumCol;
    std::vector<sal_Int32>& rTableGrid(getTableGrid());
    sal_Int32 nColWidth = nShapeWidth / nNumCol;
    for (sal_Int32 nCol = 0; nCol < nNumCol; ++nCol)
        rTableGrid.push_back(nColWidth);
    std::vector<drawingml::table::TableRow>& rTableRows(getTableRows());
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
        oox::drawingml::table::TableCell& rTableCell = rTableCells.back();
        TextBodyPtr pCellTextBody(new TextBody);
        rTableCell.setTextBody(pCellTextBody);

        // Copy properties provided by <a:lstStyle>.
        pCellTextBody->getTextListStyle() = pTextBody->getTextListStyle();

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
} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
