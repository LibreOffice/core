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
#include <DomainMapperTableHandler.hxx>
#include <DomainMapper_Impl.hxx>
#include <StyleSheetTable.hxx>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <dmapperLoggers.hxx>

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
#include <PropertyMapHelper.hxx>
#include <rtl/ustring.hxx>
#endif

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

#define DEF_BORDER_DIST 190  //0,19cm

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
static void  lcl_printProperties( PropertyMapPtr pProps )
{
    if( pProps.get() )
    {
        dmapper_logger->startElement("properties");

        PropertyMap::const_iterator aMapIter = pProps->begin();
        PropertyMap::const_iterator aEndIter = pProps->end();
        PropertyNameSupplier& rPropSupplier = PropertyNameSupplier::GetPropertyNameSupplier();
        for( ; aMapIter != aEndIter; ++aMapIter )
        {
            SAL_INFO("writerfilter", rPropSupplier.GetName(aMapIter->first.eId));

            table::BorderLine2 aLine;
            sal_Int32 nColor;
            if ( aMapIter->second >>= aLine )
            {
                dmapper_logger->startElement("borderline");
                dmapper_logger->attribute("color", aLine.Color);
                dmapper_logger->attribute("inner", aLine.InnerLineWidth);
                dmapper_logger->attribute("outer", aLine.OuterLineWidth);
                dmapper_logger->endElement();
            }
            else if ( aMapIter->second >>= nColor )
            {
                dmapper_logger->startElement("color");
                dmapper_logger->attribute("number", nColor);
                dmapper_logger->endElement();
            }
        }

        dmapper_logger->endElement();
    }
}
#endif

DomainMapperTableHandler::DomainMapperTableHandler(TextReference_t xText, DomainMapper_Impl& rDMapper_Impl)
    : m_xText(xText),
        m_rDMapper_Impl( rDMapper_Impl ),
        m_nCellIndex(0),
        m_nRowIndex(0)
{
}

DomainMapperTableHandler::~DomainMapperTableHandler()
{
}

void DomainMapperTableHandler::startTable(unsigned int nRows,
                                          unsigned int /*nDepth*/,
                                          TablePropertyMapPtr pProps)
{
    m_aTableProperties = pProps;
    m_pTableSeq = TableSequencePointer_t(new TableSequence_t(nRows));
    m_nRowIndex = 0;

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("tablehandler.table");
    dmapper_logger->attribute("rows", nRows);

    if (pProps.get() != NULL)
        pProps->dumpXml( dmapper_logger );
#endif
}



PropertyMapPtr lcl_SearchParentStyleSheetAndMergeProperties(const StyleSheetEntryPtr pStyleSheet, StyleSheetTablePtr pStyleSheetTable)
{
    PropertyMapPtr pRet;
    if(!pStyleSheet->sBaseStyleIdentifier.isEmpty())
    {
        const StyleSheetEntryPtr pParentStyleSheet = pStyleSheetTable->FindStyleSheetByISTD( pStyleSheet->sBaseStyleIdentifier );
        pRet = lcl_SearchParentStyleSheetAndMergeProperties( pParentStyleSheet, pStyleSheetTable );
    }
    else
    {
        pRet.reset( new PropertyMap );
    }

    pRet->InsertProps(pStyleSheet->pProperties);

    return pRet;
}

void lcl_mergeBorder( PropertyIds nId, PropertyMapPtr pOrig, PropertyMapPtr pDest )
{
    PropertyDefinition aDef( nId );
    PropertyMap::iterator pOrigIt = pOrig->find( aDef );

    if ( pOrigIt != pOrig->end( ) )
    {
        pDest->Insert( nId, pOrigIt->second, false );
    }
}

void lcl_computeCellBorders( PropertyMapPtr pTableBorders, PropertyMapPtr pCellProps,
        sal_Int32 nCell, sal_Int32 nRow, bool bIsEndCol, bool bIsEndRow )
{
    PropertyDefinition aVertPDef( META_PROP_VERTICAL_BORDER );
    PropertyDefinition aHorizPDef( META_PROP_HORIZONTAL_BORDER );

    PropertyMap::iterator aVerticalIter = pCellProps->find( aVertPDef );
    PropertyMap::iterator aHorizontalIter = pCellProps->find( aHorizPDef );

    // Handle the vertical and horizontal borders
    bool bHasVert = ( aVerticalIter != pCellProps->end(  ) );
    uno::Any aVertProp;
    if ( !bHasVert )
    {
        aVerticalIter = pTableBorders->find( aVertPDef );
        bHasVert = ( aVerticalIter != pTableBorders->end( ) );
        if ( bHasVert )
            aVertProp = aVerticalIter->second;
    }
    else
    {
        aVertProp = aVerticalIter->second;
        pCellProps->erase( aVerticalIter );
    }

    bool bHasHoriz = ( aHorizontalIter != pCellProps->end(  ) );
    uno::Any aHorizProp;
    if ( !bHasHoriz )
    {
        aHorizontalIter = pTableBorders->find( aHorizPDef );
        bHasHoriz = ( aHorizontalIter != pTableBorders->end( ) );
        if ( bHasHoriz )
            aHorizProp = aHorizontalIter->second;
    }
    else
    {
        aHorizProp = aHorizontalIter->second;
        pCellProps->erase( aHorizontalIter );
    }

    if ( nCell == 0 )
    {
        lcl_mergeBorder( PROP_LEFT_BORDER, pTableBorders, pCellProps );
        if ( bHasVert )
            pCellProps->Insert( PROP_RIGHT_BORDER, aVertProp, false );
    }

    if ( bIsEndCol )
    {
        lcl_mergeBorder( PROP_RIGHT_BORDER, pTableBorders, pCellProps );
        if ( bHasVert )
            pCellProps->Insert( PROP_LEFT_BORDER, aVertProp, false );
    }

    if ( nCell > 0 && !bIsEndCol )
    {
        if ( bHasVert )
        {
            pCellProps->Insert( PROP_RIGHT_BORDER, aVertProp, false );
            pCellProps->Insert( PROP_LEFT_BORDER, aVertProp, false );
        }
    }

    if ( nRow == 0 )
    {
        lcl_mergeBorder( PROP_TOP_BORDER, pTableBorders, pCellProps );
        if ( bHasHoriz )
            pCellProps->Insert( PROP_BOTTOM_BORDER, aHorizProp, false );
    }

    if ( bIsEndRow )
    {
        lcl_mergeBorder( PROP_BOTTOM_BORDER, pTableBorders, pCellProps );
        if ( bHasHoriz )
            pCellProps->Insert( PROP_TOP_BORDER, aHorizProp, false );
    }

    if ( nRow > 0 && !bIsEndRow )
    {
        if ( bHasHoriz )
        {
            pCellProps->Insert( PROP_TOP_BORDER, aHorizProp, false );
            pCellProps->Insert( PROP_BOTTOM_BORDER, aHorizProp, false );
        }
    }
}

#ifdef DEBUG_DMAPPER_TABLE_HANDLER

void lcl_debug_BorderLine(table::BorderLine & rLine)
{
    dmapper_logger->startElement("BorderLine");
    dmapper_logger->attribute("Color", rLine.Color);
    dmapper_logger->attribute("InnerLineWidth", rLine.InnerLineWidth);
    dmapper_logger->attribute("OuterLineWidth", rLine.OuterLineWidth);
    dmapper_logger->attribute("LineDistance", rLine.LineDistance);
    dmapper_logger->endElement();
}

void lcl_debug_TableBorder(table::TableBorder & rBorder)
{
    dmapper_logger->startElement("TableBorder");
    lcl_debug_BorderLine(rBorder.TopLine);
    dmapper_logger->attribute("IsTopLineValid", rBorder.IsTopLineValid);
    lcl_debug_BorderLine(rBorder.BottomLine);
    dmapper_logger->attribute("IsBottomLineValid", rBorder.IsBottomLineValid);
    lcl_debug_BorderLine(rBorder.LeftLine);
    dmapper_logger->attribute("IsLeftLineValid", rBorder.IsLeftLineValid);
    lcl_debug_BorderLine(rBorder.RightLine);
    dmapper_logger->attribute("IsRightLineValid", rBorder.IsRightLineValid);
    lcl_debug_BorderLine(rBorder.VerticalLine);
    dmapper_logger->attribute("IsVerticalLineValid", rBorder.IsVerticalLineValid);
    lcl_debug_BorderLine(rBorder.HorizontalLine);
    dmapper_logger->attribute("IsHorizontalLineValid", rBorder.IsHorizontalLineValid);
    dmapper_logger->attribute("Distance", rBorder.Distance);
    dmapper_logger->attribute("IsDistanceValid", rBorder.IsDistanceValid);
    dmapper_logger->endElement();
}
#endif

struct WRITERFILTER_DLLPRIVATE TableInfo
{
    sal_Int32 nLeftBorderDistance;
    sal_Int32 nRightBorderDistance;
    sal_Int32 nTopBorderDistance;
    sal_Int32 nBottomBorderDistance;
    sal_Int32 nTblLook;
    sal_Int32 nNestLevel;
    PropertyMapPtr pTableDefaults;
    PropertyMapPtr pTableBorders;
    TableStyleSheetEntry* pTableStyle;
    TablePropertyValues_t aTableProperties;

    TableInfo()
    : nLeftBorderDistance(DEF_BORDER_DIST)
    , nRightBorderDistance(DEF_BORDER_DIST)
    , nTopBorderDistance(0)
    , nBottomBorderDistance(0)
    , nTblLook(0x4a0)
    , nNestLevel(0)
    , pTableDefaults(new PropertyMap)
    , pTableBorders(new PropertyMap)
    , pTableStyle(NULL)
    {
    }

};

namespace
{

bool lcl_extractTableBorderProperty(PropertyMapPtr pTableProperties, const PropertyIds nId, TableInfo& rInfo, table::BorderLine2& rLine)
{
    PropertyMap::iterator aTblBorderIter = pTableProperties->find( PropertyDefinition(nId) );
    if( aTblBorderIter != pTableProperties->end() )
    {
        OSL_VERIFY(aTblBorderIter->second >>= rLine);

        rInfo.pTableBorders->Insert( nId, uno::makeAny( rLine ) );
        PropertyMap::iterator pIt = rInfo.pTableDefaults->find( PropertyDefinition( nId ) );
        if ( pIt != rInfo.pTableDefaults->end( ) )
            rInfo.pTableDefaults->erase( pIt );

        return true;
    }

    return false;
}

}

void lcl_DecrementHoriOrientPosition(uno::Sequence<beans::PropertyValue>& rFrameProperties, sal_Int32 nAmount)
{
    // Shifts the frame left by the given value.
    for (sal_Int32 i = 0; i < rFrameProperties.getLength(); ++i)
    {
        beans::PropertyValue& rPropertyValue = rFrameProperties[i];
        if (rPropertyValue.Name == "HoriOrientPosition")
        {
            sal_Int32 nValue = rPropertyValue.Value.get<sal_Int32>();
            nValue -= nAmount;
            rPropertyValue.Value <<= nValue;
            return;
        }
    }
}

TableStyleSheetEntry * DomainMapperTableHandler::endTableGetTableStyle(TableInfo & rInfo, uno::Sequence<beans::PropertyValue>& rFrameProperties)
{
    // will receive the table style if any
    TableStyleSheetEntry* pTableStyle = NULL;

    if( m_aTableProperties.get() )
    {
        //create properties from the table attributes
        //...pPropMap->Insert( PROP_LEFT_MARGIN, uno::makeAny( m_nLeftMargin - m_nGapHalf ));
        //pPropMap->Insert( PROP_HORI_ORIENT, uno::makeAny( text::HoriOrientation::RIGHT ));
        sal_Int32 nGapHalf = 0;
        sal_Int32 nLeftMargin = 0;
        sal_Int32 nTableWidth = 0;
        sal_Int32 nTableWidthType = text::SizeType::FIX;

        PropertyMap::iterator aTableStyleIter =
        m_aTableProperties->find( PropertyDefinition( META_PROP_TABLE_STYLE_NAME ) );
        if(aTableStyleIter != m_aTableProperties->end())
        {
            // Apply table style properties recursively
            OUString sTableStyleName;
            aTableStyleIter->second >>= sTableStyleName;
            StyleSheetTablePtr pStyleSheetTable = m_rDMapper_Impl.GetStyleSheetTable();
            const StyleSheetEntryPtr pStyleSheet = pStyleSheetTable->FindStyleSheetByISTD( sTableStyleName );
            pTableStyle = dynamic_cast<TableStyleSheetEntry*>( pStyleSheet.get( ) );
            m_aTableProperties->erase( aTableStyleIter );

            if( pStyleSheet )
            {
                // First get the style properties, then the table ones
                PropertyMapPtr pTableProps( m_aTableProperties );
                TablePropertyMapPtr pEmptyProps( new TablePropertyMap );

                m_aTableProperties = pEmptyProps;

                PropertyMapPtr pMergedProperties = lcl_SearchParentStyleSheetAndMergeProperties(pStyleSheet, pStyleSheetTable);

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
                dmapper_logger->startElement("mergedProps");
                pMergedProperties->dumpXml( dmapper_logger );
                dmapper_logger->endElement();
#endif

                m_aTableProperties->InsertProps(pMergedProperties);
                m_aTableProperties->InsertProps(pTableProps);

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
                dmapper_logger->startElement("TableProperties");
                m_aTableProperties->dumpXml( dmapper_logger );
                dmapper_logger->endElement();
#endif
            }
        }

        PropertyMap::iterator const aTblLookIter =
            m_aTableProperties->find(PropertyDefinition(PROP_TBL_LOOK));
        if(aTblLookIter != m_aTableProperties->end())
        {
            aTblLookIter->second >>= rInfo.nTblLook;
            m_aTableProperties->erase( aTblLookIter );
        }

        // Set the table default attributes for the cells
        rInfo.pTableDefaults->InsertProps(m_aTableProperties);

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
        dmapper_logger->startElement("TableDefaults");
        rInfo.pTableDefaults->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif

        m_aTableProperties->getValue( TablePropertyMap::GAP_HALF, nGapHalf );
        m_aTableProperties->getValue( TablePropertyMap::LEFT_MARGIN, nLeftMargin );

        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_LEFT,
                                     rInfo.nLeftBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_RIGHT,
                                     rInfo.nRightBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_TOP,
                                     rInfo.nTopBorderDistance );
        m_aTableProperties->getValue( TablePropertyMap::CELL_MAR_BOTTOM,
                                     rInfo.nBottomBorderDistance );

        table::TableBorderDistances aDistances;
        aDistances.IsTopDistanceValid =
        aDistances.IsBottomDistanceValid =
        aDistances.IsLeftDistanceValid =
        aDistances.IsRightDistanceValid = sal_True;
        aDistances.TopDistance = static_cast<sal_Int16>( rInfo.nTopBorderDistance );
        aDistances.BottomDistance = static_cast<sal_Int16>( rInfo.nBottomBorderDistance );
        aDistances.LeftDistance = static_cast<sal_Int16>( rInfo.nLeftBorderDistance );
        aDistances.RightDistance = static_cast<sal_Int16>( rInfo.nRightBorderDistance );

        m_aTableProperties->Insert( PROP_TABLE_BORDER_DISTANCES, uno::makeAny( aDistances ) );

        if (rFrameProperties.hasElements())
            lcl_DecrementHoriOrientPosition(rFrameProperties, rInfo.nLeftBorderDistance);

        // Set table above/bottom spacing to 0.
        m_aTableProperties->Insert( PROP_TOP_MARGIN, uno::makeAny( sal_Int32( 0 ) ) );
        m_aTableProperties->Insert( PROP_BOTTOM_MARGIN, uno::makeAny( sal_Int32( 0 ) ) );

        //table border settings
        table::TableBorder aTableBorder;
        table::BorderLine2 aBorderLine, aLeftBorder;

        if (lcl_extractTableBorderProperty(m_aTableProperties, PROP_TOP_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.TopLine = aBorderLine;
            aTableBorder.IsTopLineValid = sal_True;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties, PROP_BOTTOM_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.BottomLine = aBorderLine;
            aTableBorder.IsBottomLineValid = sal_True;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties, PROP_LEFT_BORDER, rInfo, aLeftBorder))
        {
            aTableBorder.LeftLine = aLeftBorder;
            aTableBorder.IsLeftLineValid = sal_True;
            // Only top level table position depends on border width
            if (rInfo.nNestLevel == 1)
            {
                if (!rFrameProperties.hasElements())
                    rInfo.nLeftBorderDistance += aLeftBorder.LineWidth * 0.5;
                else
                    lcl_DecrementHoriOrientPosition(rFrameProperties, aLeftBorder.LineWidth * 0.5);
            }
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties, PROP_RIGHT_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.RightLine = aBorderLine;
            aTableBorder.IsRightLineValid = sal_True;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties, META_PROP_HORIZONTAL_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.HorizontalLine = aBorderLine;
            aTableBorder.IsHorizontalLineValid = sal_True;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties, META_PROP_VERTICAL_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.VerticalLine = aBorderLine;
            aTableBorder.IsVerticalLineValid = sal_True;
        }

        aTableBorder.Distance = 0;
        aTableBorder.IsDistanceValid = sal_False;

        m_aTableProperties->Insert( PROP_TABLE_BORDER, uno::makeAny( aTableBorder ) );

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
        lcl_debug_TableBorder(aTableBorder);
#endif

        // Table position in Office is computed in 2 different ways :
        // - top level tables: the goal is to have in-cell text starting at table indent pos (tblInd),
        //   so table's position depends on table's cells margin
        // - nested tables: the goal is to have left-most border starting at table_indent pos
        if (rInfo.nNestLevel > 1)
        {
            m_aTableProperties->Insert( PROP_LEFT_MARGIN, uno::makeAny( nLeftMargin - nGapHalf ));
        }
        else
        {
            m_aTableProperties->Insert( PROP_LEFT_MARGIN, uno::makeAny( nLeftMargin - nGapHalf - rInfo.nLeftBorderDistance ));
        }

        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH, nTableWidth );
        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType );
        if( nTableWidthType == text::SizeType::FIX )
        {
            if( nTableWidth > 0 )
                m_aTableProperties->Insert( PROP_WIDTH, uno::makeAny( nTableWidth ));
        }
        else
        {
            m_aTableProperties->Insert( PROP_RELATIVE_WIDTH, uno::makeAny( sal_Int16( nTableWidth ) ) );
            m_aTableProperties->Insert( PROP_IS_WIDTH_RELATIVE, uno::makeAny( sal_Bool( sal_True ) ) );
        }

        sal_Int32 nHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH;
        m_aTableProperties->getValue( TablePropertyMap::HORI_ORIENT, nHoriOrient ) ;
        m_aTableProperties->Insert( PROP_HORI_ORIENT, uno::makeAny( sal_Int16(nHoriOrient) ) );

        //fill default value - if not available
        const PropertyMap::const_iterator aRepeatIter =
        m_aTableProperties->find( PropertyDefinition( PROP_HEADER_ROW_COUNT ) );
        if( aRepeatIter == m_aTableProperties->end() )
            m_aTableProperties->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny( (sal_Int32)0 ));

        rInfo.aTableProperties = m_aTableProperties->GetPropertyValues();

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
        dmapper_logger->startElement("debug.tableprops");
        m_aTableProperties->dumpXml( dmapper_logger );
        dmapper_logger->endElement();
#endif

    }

    return pTableStyle;
}

#define CNF_FIRST_ROW               0x800
#define CNF_LAST_ROW                0x400
#define CNF_FIRST_COLUMN            0x200
#define CNF_LAST_COLUMN             0x100
#define CNF_ODD_VBAND               0x080
#define CNF_EVEN_VBAND              0x040
#define CNF_ODD_HBAND               0x020
#define CNF_EVEN_HBAND              0x010
#define CNF_FIRST_ROW_LAST_COLUMN   0x008
#define CNF_FIRST_ROW_FIRST_COLUMN  0x004
#define CNF_LAST_ROW_LAST_COLUMN    0x002
#define CNF_LAST_ROW_FIRST_COLUMN   0x001

CellPropertyValuesSeq_t DomainMapperTableHandler::endTableGetCellProperties(TableInfo & rInfo)
{
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("getCellProperties");
#endif

    CellPropertyValuesSeq_t aCellProperties( m_aCellProperties.size() );

    if ( !m_aCellProperties.size() )
    {
        #ifdef DEBUG_DOMAINMAPPER
        dmapper_logger->endElement();
        #endif
        return aCellProperties;
    }
    // std::vector< std::vector<PropertyMapPtr> > m_aCellProperties
    PropertyMapVector2::const_iterator aRowOfCellsIterator = m_aCellProperties.begin();
    PropertyMapVector2::const_iterator aRowOfCellsIteratorEnd = m_aCellProperties.end();
    PropertyMapVector2::const_iterator aLastRowIterator = m_aCellProperties.end() - 1;
    sal_Int32 nRow = 0;

    //it's a uno::Sequence< beans::PropertyValues >*
    RowPropertyValuesSeq_t* pCellProperties = aCellProperties.getArray();
    while( aRowOfCellsIterator != aRowOfCellsIteratorEnd )
    {
        //aRowOfCellsIterator points to a vector of PropertyMapPtr
        PropertyMapVector1::const_iterator aCellIterator = aRowOfCellsIterator->begin();
        PropertyMapVector1::const_iterator aCellIteratorEnd = aRowOfCellsIterator->end();

        sal_Int32 nRowStyleMask = 0;

        if (aRowOfCellsIterator==m_aCellProperties.begin())
        {
            if(rInfo.nTblLook&0x20)
                nRowStyleMask |= CNF_FIRST_ROW;     // first row style used
        }
        else if (aRowOfCellsIterator==aLastRowIterator)
        {
            if(rInfo.nTblLook&0x40)
                nRowStyleMask |= CNF_LAST_ROW;      // last row style used
        }
        if(!nRowStyleMask)                          // if no row style used yet
        {
            // banding used only if not first and or last row style used
            if(!(rInfo.nTblLook&0x200))
            {   // hbanding used
                int n = nRow + 1;
                if(rInfo.nTblLook&0x20)
                    n++;
                if(n & 1)
                    nRowStyleMask = CNF_ODD_HBAND;
                else
                    nRowStyleMask = CNF_EVEN_HBAND;
            }
        }

        sal_Int32 nCell = 0;
        pCellProperties[nRow].realloc( aRowOfCellsIterator->size() );
        beans::PropertyValues* pSingleCellProperties = pCellProperties[nRow].getArray();
        while( aCellIterator != aCellIteratorEnd )
        {
            PropertyMapPtr pAllCellProps( new PropertyMap );

            PropertyMapVector1::const_iterator aLastCellIterator = aRowOfCellsIterator->end() - 1;
            bool bIsEndCol = aCellIterator == aLastCellIterator;
            bool bIsEndRow = aRowOfCellsIterator == aLastRowIterator;

            //aCellIterator points to a PropertyMapPtr;
            if( aCellIterator->get() )
            {
                if ( rInfo.pTableDefaults->size( ) )
                    pAllCellProps->InsertProps(rInfo.pTableDefaults);

                sal_Int32 nCellStyleMask = 0;
                if (aCellIterator==aRowOfCellsIterator->begin())
                {
                    if(rInfo.nTblLook&0x80)
                        nCellStyleMask = CNF_FIRST_COLUMN;      // first col style used
                }
                else if (bIsEndCol)
                {
                    if(rInfo.nTblLook&0x100)
                        nCellStyleMask = CNF_LAST_COLUMN;       // last col style used
                }
                if(!nCellStyleMask)                 // if no cell style is used yet
                {
                    if(!(rInfo.nTblLook&0x400))
                    {   // vbanding used
                        int n = nCell + 1;
                        if(rInfo.nTblLook&0x80)
                            n++;
                        if(n & 1)
                            nCellStyleMask = CNF_ODD_VBAND;
                        else
                            nCellStyleMask = CNF_EVEN_VBAND;
                    }
                }
                sal_Int32 nCnfStyleMask = nCellStyleMask + nRowStyleMask;
                if(nCnfStyleMask == CNF_FIRST_COLUMN + CNF_FIRST_ROW)
                    nCnfStyleMask |= CNF_FIRST_ROW_FIRST_COLUMN;
                else if(nCnfStyleMask == CNF_FIRST_COLUMN + CNF_LAST_ROW)
                    nCnfStyleMask |= CNF_LAST_ROW_FIRST_COLUMN;
                else if(nCnfStyleMask == CNF_LAST_COLUMN + CNF_FIRST_ROW)
                    nCnfStyleMask |= CNF_FIRST_ROW_LAST_COLUMN;
                else if(nCnfStyleMask == CNF_LAST_COLUMN + CNF_LAST_ROW)
                    nCnfStyleMask |= CNF_LAST_ROW_LAST_COLUMN;

                if ( rInfo.pTableStyle )
                {
                    PropertyMapPtr pStyleProps = rInfo.pTableStyle->GetProperties( nCnfStyleMask );
                    pAllCellProps->InsertProps( pStyleProps );
                }

                // Remove properties from style/row that aren't allowed in cells
                const PropertyMap::iterator aDefaultRepeatIt =
                    pAllCellProps->find(
                        PropertyDefinition( PROP_HEADER_ROW_COUNT ) );
                if ( aDefaultRepeatIt != pAllCellProps->end( ) )
                    pAllCellProps->erase( aDefaultRepeatIt );

                // Then add the cell properties
                pAllCellProps->InsertProps(*aCellIterator);
                aCellIterator->get( )->swap( *pAllCellProps.get( ) );

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
                dmapper_logger->startElement("cell");
                dmapper_logger->attribute("cell", nCell);
                dmapper_logger->attribute("row", nRow);
#endif

                lcl_computeCellBorders( rInfo.pTableBorders, *aCellIterator, nCell, nRow, bIsEndCol, bIsEndRow );

                //now set the default left+right border distance TODO: there's an sprm containing the default distance!
                const PropertyMap::const_iterator aLeftDistanceIter =
                aCellIterator->get()->find( PropertyDefinition(PROP_LEFT_BORDER_DISTANCE) );
                if( aLeftDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_LEFT_BORDER_DISTANCE,
                                                 uno::makeAny(rInfo.nLeftBorderDistance ) );
                const PropertyMap::const_iterator aRightDistanceIter =
                aCellIterator->get()->find( PropertyDefinition(PROP_RIGHT_BORDER_DISTANCE) );
                if( aRightDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_RIGHT_BORDER_DISTANCE,
                                                 uno::makeAny((sal_Int32) rInfo.nRightBorderDistance ) );

                const PropertyMap::const_iterator aTopDistanceIter =
                aCellIterator->get()->find( PropertyDefinition(PROP_TOP_BORDER_DISTANCE) );
                if( aTopDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_TOP_BORDER_DISTANCE,
                                                 uno::makeAny((sal_Int32) rInfo.nTopBorderDistance ) );

                const PropertyMap::const_iterator aBottomDistanceIter =
                aCellIterator->get()->find( PropertyDefinition(PROP_BOTTOM_BORDER_DISTANCE) );
                if( aBottomDistanceIter == aCellIterator->get()->end() )
                    aCellIterator->get()->Insert( PROP_BOTTOM_BORDER_DISTANCE,
                                                 uno::makeAny((sal_Int32) rInfo.nBottomBorderDistance ) );

                pSingleCellProperties[nCell] = aCellIterator->get()->GetPropertyValues();
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
                dmapper_logger->endElement();
#endif
            }
            ++nCell;
            ++aCellIterator;
        }
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
        //-->debug cell properties
        {
            OUString sNames;
            const uno::Sequence< beans::PropertyValues > aDebugCurrentRow = aCellProperties[nRow];
            sal_Int32 nDebugCells = aDebugCurrentRow.getLength();
            (void) nDebugCells;
            for( sal_Int32  nDebugCell = 0; nDebugCell < nDebugCells; ++nDebugCell)
            {
                const uno::Sequence< beans::PropertyValue >& aDebugCellProperties = aDebugCurrentRow[nDebugCell];
                sal_Int32 nDebugCellProperties = aDebugCellProperties.getLength();
                for( sal_Int32  nDebugProperty = 0; nDebugProperty < nDebugCellProperties; ++nDebugProperty)
                {
                    const OUString sName = aDebugCellProperties[nDebugProperty].Name;
                    sNames += sName;
                    sNames += OUString('-');
                }
                sNames += OUString('\n');
            }
            (void)sNames;
        }
        //--<
#endif
        ++nRow;
        ++aRowOfCellsIterator;
    }

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->endElement();
#endif

    return aCellProperties;
}

RowPropertyValuesSeq_t DomainMapperTableHandler::endTableGetRowProperties()
{
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("getRowProperties");
#endif

    RowPropertyValuesSeq_t aRowProperties( m_aRowProperties.size() );
    PropertyMapVector1::const_iterator aRowIter = m_aRowProperties.begin();
    PropertyMapVector1::const_iterator aRowIterEnd = m_aRowProperties.end();
    sal_Int32 nRow = 0;
    while( aRowIter != aRowIterEnd )
    {
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
        dmapper_logger->startElement("rowProps.row");
#endif
        if( aRowIter->get() )
        {
            //set default to 'break across pages"
            if( aRowIter->get()->find( PropertyDefinition( PROP_IS_SPLIT_ALLOWED )) == aRowIter->get()->end())
                aRowIter->get()->Insert( PROP_IS_SPLIT_ALLOWED, uno::makeAny(sal_True ) );

            aRowProperties[nRow] = (*aRowIter)->GetPropertyValues();
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
            ((*aRowIter)->dumpXml( dmapper_logger ));
            lcl_DumpPropertyValues(dmapper_logger, aRowProperties[nRow]);
#endif
        }
        ++nRow;
        ++aRowIter;
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
        dmapper_logger->endElement();
#endif
    }

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->endElement();
#endif

    return aRowProperties;
}

void DomainMapperTableHandler::endTable(unsigned int nestedTableLevel)
{
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("tablehandler.endTable");
#endif

    // If we want to make this table a floating one.
    uno::Sequence<beans::PropertyValue> aFrameProperties = m_rDMapper_Impl.getTableManager().getCurrentTablePosition();
    TableInfo aTableInfo;
    aTableInfo.nNestLevel = nestedTableLevel;
    aTableInfo.pTableStyle = endTableGetTableStyle(aTableInfo, aFrameProperties);
    //  expands to uno::Sequence< Sequence< beans::PropertyValues > >

    CellPropertyValuesSeq_t aCellProperties = endTableGetCellProperties(aTableInfo);

    RowPropertyValuesSeq_t aRowProperties = endTableGetRowProperties();

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    lcl_DumpPropertyValueSeq(dmapper_logger, aRowProperties);
#endif

    if (m_pTableSeq->getLength() > 0)
    {
        uno::Reference<text::XTextRange> xStart;
        uno::Reference<text::XTextRange> xEnd;
        bool bFloating = aFrameProperties.hasElements();
        // Additional checks: if we can do this.
        if (bFloating && (*m_pTableSeq)[0].getLength() > 0 && (*m_pTableSeq)[0][0].getLength() > 0)
        {
            xStart = (*m_pTableSeq)[0][0][0];
            uno::Sequence< uno::Sequence< uno::Reference<text::XTextRange> > >& rLastRow = (*m_pTableSeq)[m_pTableSeq->getLength() - 1];
            uno::Sequence< uno::Reference<text::XTextRange> >& rLastCell = rLastRow[rLastRow.getLength() - 1];
            xEnd = rLastCell[1];
        }
        uno::Reference<text::XTextTable> xTable;
        try
        {
            if (m_xText.is())
            {
                xTable = m_xText->convertToTable(*m_pTableSeq,
                        aCellProperties,
                        aRowProperties,
                        aTableInfo.aTableProperties);

                if (xTable.is())
                    m_xTableRange = xTable->getAnchor( );
            }
        }
        catch ( const lang::IllegalArgumentException &e )
        {
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
            fprintf( stderr, "Conversion to table error: %s\n",
                    OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            dmapper_logger->chars(std::string("failed to import table!"));
#else
            (void)e;
#endif
        }
        catch ( const uno::Exception &e )
        {
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
            fprintf( stderr, "Exception during table creation: %s\n",
                    OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr( ) );
#else
            (void) e;
#endif
        }

        // If we have a table with a start and an end position, we should make it a floating one.
        if (xTable.is() && xStart.is() && xEnd.is())
        {
            uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
            sal_Bool bIsRelative = sal_False;
            xTableProperties->getPropertyValue("IsWidthRelative") >>= bIsRelative;
            if (!bIsRelative)
            {
                aFrameProperties.realloc(aFrameProperties.getLength() + 1);
                aFrameProperties[aFrameProperties.getLength() - 1].Name = "Width";
                aFrameProperties[aFrameProperties.getLength() - 1].Value = xTableProperties->getPropertyValue("Width");
            }
            else
            {
                aFrameProperties.realloc(aFrameProperties.getLength() + 1);
                aFrameProperties[aFrameProperties.getLength() - 1].Name = "FrameWidthPercent";
                aFrameProperties[aFrameProperties.getLength() - 1].Value = xTableProperties->getPropertyValue("RelativeWidth");

                // Applying the relative width to the frame, needs to have the table width to be 100% of the frame width
                xTableProperties->setPropertyValue("RelativeWidth", uno::makeAny(sal_Int16(100)));
            }

            // A non-zero left margin would move the table out of the frame, move the frame itself instead.
            xTableProperties->setPropertyValue("LeftMargin", uno::makeAny(sal_Int32(0)));

            uno::Reference< text::XTextContent > xFrame = m_xText->convertToTextFrame(xStart, xEnd, aFrameProperties);
        }
    }

    m_aTableProperties.reset();
    m_aCellProperties.clear();
    m_aRowProperties.clear();

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->endElement();
    dmapper_logger->endElement();
#endif
}

void DomainMapperTableHandler::startRow(unsigned int nCells,
                                        TablePropertyMapPtr pProps)
{
    m_aRowProperties.push_back( pProps );
    m_aCellProperties.push_back( PropertyMapVector1() );

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("table.row");
    dmapper_logger->attribute("cells", nCells);
    if (pProps != NULL)
        pProps->dumpXml(dmapper_logger);
#endif

    m_pRowSeq = RowSequencePointer_t(new RowSequence_t(nCells));
    m_nCellIndex = 0;
}

void DomainMapperTableHandler::endRow()
{
    (*m_pTableSeq)[m_nRowIndex] = *m_pRowSeq;
    ++m_nRowIndex;
    m_nCellIndex = 0;
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->endElement();
#endif
}

void DomainMapperTableHandler::startCell(const Handle_t & start,
                                         TablePropertyMapPtr pProps )
{
    sal_uInt32 nRow = m_aRowProperties.size();
    if ( pProps.get( ) )
        m_aCellProperties[nRow - 1].push_back( pProps );
    else
    {
        // Adding an empty cell properties map to be able to get
        // the table defaults properties
        TablePropertyMapPtr pEmptyProps( new TablePropertyMap( ) );
        m_aCellProperties[nRow - 1].push_back( pEmptyProps );
    }

#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("table.cell");
    dmapper_logger->startElement("table.cell.start");
    dmapper_logger->chars(toString(start));
    dmapper_logger->endElement();
    lcl_printProperties( pProps );
#endif

    //add a new 'row' of properties
    m_pCellSeq = CellSequencePointer_t(new CellSequence_t(2));
    if (!start.get())
        return;
    (*m_pCellSeq)[0] = start->getStart();
}

void DomainMapperTableHandler::endCell(const Handle_t & end)
{
#ifdef DEBUG_DMAPPER_TABLE_HANDLER
    dmapper_logger->startElement("table.cell.end");
    dmapper_logger->chars(toString(end));
    dmapper_logger->endElement();
    dmapper_logger->endElement();
#endif

    if (!end.get())
        return;
    (*m_pCellSeq)[1] = end->getEnd();
    (*m_pRowSeq)[m_nCellIndex] = *m_pCellSeq;
    ++m_nCellIndex;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
