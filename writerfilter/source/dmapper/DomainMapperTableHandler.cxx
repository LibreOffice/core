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
#include "DomainMapperTableHandler.hxx"
#include "DomainMapper_Impl.hxx"
#include "StyleSheetTable.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include "TablePositionHandler.hxx"
#include "ConversionHelper.hxx"
#include "util.hxx"
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/sequence.hxx>

#ifdef DEBUG_WRITERFILTER
#include "PropertyMapHelper.hxx"
#include <rtl/ustring.hxx>
#endif

namespace writerfilter {
namespace dmapper {

using namespace ::com::sun::star;
using namespace ::std;

#define DEF_BORDER_DIST 190  //0,19cm

DomainMapperTableHandler::DomainMapperTableHandler(
            css::uno::Reference<css::text::XTextAppendAndConvert> const& xText,
            DomainMapper_Impl& rDMapper_Impl)
    : m_xText(xText),
        m_rDMapper_Impl( rDMapper_Impl ),
        m_bHadFootOrEndnote(false)
{
}

DomainMapperTableHandler::~DomainMapperTableHandler()
{
}

void DomainMapperTableHandler::startTable(const TablePropertyMapPtr& pProps)
{
    m_aTableProperties = pProps;
    m_aTableRanges.clear();

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablehandler.table");

    if (pProps.get() != nullptr)
        pProps->dumpXml();
#endif
}

static void lcl_mergeBorder( PropertyIds nId, const PropertyMapPtr& pOrig, const PropertyMapPtr& pDest )
{
    boost::optional<PropertyMap::Property> pOrigVal = pOrig->getProperty(nId);

    if ( pOrigVal )
    {
        pDest->Insert( nId, pOrigVal->second, false );
    }
}

static void lcl_computeCellBorders( const PropertyMapPtr& pTableBorders, const PropertyMapPtr& pCellProps,
        sal_Int32 nCell, sal_Int32 nRow, bool bIsEndCol, bool bIsEndRow )
{
    boost::optional<PropertyMap::Property> pVerticalVal = pCellProps->getProperty(META_PROP_VERTICAL_BORDER);
    boost::optional<PropertyMap::Property> pHorizontalVal = pCellProps->getProperty(META_PROP_HORIZONTAL_BORDER);

    // Handle the vertical and horizontal borders
    uno::Any aVertProp;
    if ( !pVerticalVal)
    {
        pVerticalVal = pTableBorders->getProperty(META_PROP_VERTICAL_BORDER);
        if ( pVerticalVal )
            aVertProp = pVerticalVal->second;
    }
    else
    {
        aVertProp = pVerticalVal->second;
        pCellProps->Erase( pVerticalVal->first );
    }

    uno::Any aHorizProp;
    if ( !pHorizontalVal )
    {
        pHorizontalVal = pTableBorders->getProperty(META_PROP_HORIZONTAL_BORDER);
        if ( pHorizontalVal )
            aHorizProp = pHorizontalVal->second;
    }
    else
    {
        aHorizProp = pHorizontalVal->second;
        pCellProps->Erase( pHorizontalVal->first );
    }

    if ( nCell == 0 )
    {
        lcl_mergeBorder( PROP_LEFT_BORDER, pTableBorders, pCellProps );
        if ( pVerticalVal )
            pCellProps->Insert( PROP_RIGHT_BORDER, aVertProp, false );
    }

    if ( bIsEndCol )
    {
        lcl_mergeBorder( PROP_RIGHT_BORDER, pTableBorders, pCellProps );
        if ( pVerticalVal )
            pCellProps->Insert( PROP_LEFT_BORDER, aVertProp, false );
    }

    if ( nCell > 0 && !bIsEndCol )
    {
        if ( pVerticalVal )
        {
            pCellProps->Insert( PROP_RIGHT_BORDER, aVertProp, false );
            pCellProps->Insert( PROP_LEFT_BORDER, aVertProp, false );
        }
    }

    if ( nRow == 0 )
    {
        lcl_mergeBorder( PROP_TOP_BORDER, pTableBorders, pCellProps );
        if ( pHorizontalVal )
            pCellProps->Insert( PROP_BOTTOM_BORDER, aHorizProp, false );
    }

    if ( bIsEndRow )
    {
        lcl_mergeBorder( PROP_BOTTOM_BORDER, pTableBorders, pCellProps );
        if ( pHorizontalVal )
            pCellProps->Insert( PROP_TOP_BORDER, aHorizProp, false );
    }

    if ( nRow > 0 && !bIsEndRow )
    {
        if ( pHorizontalVal )
        {
            pCellProps->Insert( PROP_TOP_BORDER, aHorizProp, false );
            pCellProps->Insert( PROP_BOTTOM_BORDER, aHorizProp, false );
        }
    }
}

#ifdef DEBUG_WRITERFILTER

static void lcl_debug_BorderLine(table::BorderLine const & rLine)
{
    TagLogger::getInstance().startElement("BorderLine");
    TagLogger::getInstance().attribute("Color", rLine.Color);
    TagLogger::getInstance().attribute("InnerLineWidth", rLine.InnerLineWidth);
    TagLogger::getInstance().attribute("OuterLineWidth", rLine.OuterLineWidth);
    TagLogger::getInstance().attribute("LineDistance", rLine.LineDistance);
    TagLogger::getInstance().endElement();
}

static void lcl_debug_TableBorder(table::TableBorder const & rBorder)
{
    TagLogger::getInstance().startElement("TableBorder");
    lcl_debug_BorderLine(rBorder.TopLine);
    TagLogger::getInstance().attribute("IsTopLineValid", sal_uInt32(rBorder.IsTopLineValid));
    lcl_debug_BorderLine(rBorder.BottomLine);
    TagLogger::getInstance().attribute("IsBottomLineValid", sal_uInt32(rBorder.IsBottomLineValid));
    lcl_debug_BorderLine(rBorder.LeftLine);
    TagLogger::getInstance().attribute("IsLeftLineValid", sal_uInt32(rBorder.IsLeftLineValid));
    lcl_debug_BorderLine(rBorder.RightLine);
    TagLogger::getInstance().attribute("IsRightLineValid", sal_uInt32(rBorder.IsRightLineValid));
    lcl_debug_BorderLine(rBorder.VerticalLine);
    TagLogger::getInstance().attribute("IsVerticalLineValid", sal_uInt32(rBorder.IsVerticalLineValid));
    lcl_debug_BorderLine(rBorder.HorizontalLine);
    TagLogger::getInstance().attribute("IsHorizontalLineValid", sal_uInt32(rBorder.IsHorizontalLineValid));
    TagLogger::getInstance().attribute("Distance", rBorder.Distance);
    TagLogger::getInstance().attribute("IsDistanceValid", sal_uInt32(rBorder.IsDistanceValid));
    TagLogger::getInstance().endElement();
}
#endif

struct TableInfo
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
    css::beans::PropertyValues aTableProperties;

    TableInfo()
    : nLeftBorderDistance(DEF_BORDER_DIST)
    , nRightBorderDistance(DEF_BORDER_DIST)
    , nTopBorderDistance(0)
    , nBottomBorderDistance(0)
    , nTblLook(0x4a0)
    , nNestLevel(0)
    , pTableDefaults(new PropertyMap)
    , pTableBorders(new PropertyMap)
    , pTableStyle(nullptr)
    {
    }

};

namespace
{

bool lcl_extractTableBorderProperty(const PropertyMapPtr& pTableProperties, const PropertyIds nId, TableInfo const & rInfo, table::BorderLine2& rLine)
{
    if (!pTableProperties)
        return false;

    const boost::optional<PropertyMap::Property> aTblBorder = pTableProperties->getProperty(nId);
    if( aTblBorder )
    {
        OSL_VERIFY(aTblBorder->second >>= rLine);

        rInfo.pTableBorders->Insert( nId, uno::makeAny( rLine ) );
        rInfo.pTableDefaults->Erase( nId );

        return true;
    }

    return false;
}

void lcl_extractHoriOrient(std::vector<beans::PropertyValue>& rFrameProperties, sal_Int32& nHoriOrient)
{
    // Shifts the frame left by the given value.
    for (beans::PropertyValue & rFrameProperty : rFrameProperties)
    {
        if (rFrameProperty.Name == "HoriOrient")
        {
            sal_Int32 nValue = rFrameProperty.Value.get<sal_Int32>();
            if (nValue != text::HoriOrientation::NONE)
                nHoriOrient = nValue;
            return;
        }
    }
}

void lcl_DecrementHoriOrientPosition(std::vector<beans::PropertyValue>& rFrameProperties, sal_Int32 nAmount)
{
    // Shifts the frame left by the given value.
    for (beans::PropertyValue & rPropertyValue : rFrameProperties)
    {
        if (rPropertyValue.Name == "HoriOrientPosition")
        {
            sal_Int32 nValue = rPropertyValue.Value.get<sal_Int32>();
            nValue -= nAmount;
            rPropertyValue.Value <<= nValue;
            return;
        }
    }
}

void lcl_adjustBorderDistance(TableInfo& rInfo, const table::BorderLine2& rLeftBorder,
                              const table::BorderLine2& rRightBorder)
{
    // MS Word appears to do these things to adjust the cell horizontal area:
    //
    // bll = left borderline width
    // blr = right borderline width
    // cea = cell's edit area rectangle
    // cea_w = cea width
    // cml = cell's left margin (padding) defined in cell settings
    // cmr = cell's right margin (padding) defined in cell settings
    // cw  = cell width (distance between middles of left borderline and right borderline)
    // pad_l = actual cea left padding = (its left pos relative to middle of bll)
    // pad_r = actual cea right padding = abs (its right pos relative to middle of blr)
    //
    // pad_l = max(bll/2, cml) -> cea does not overlap left borderline
    // cea_w = cw-max(pad_l+blr/2, cml+cmr) -> cea does not overlap right borderline
    // pad_r = max(pad_l+blr/2, cml+cmr) - pad_l
    //
    // It means that e.g. for border widths of 6 pt (~2.12 mm), left margin 0 mm, and right margin
    // 2 mm, actual left and right margins will (unexpectedly) coincide with inner edges of cell's
    // borderlines - the right margin won't create spacing between right of edit rectangle and the
    // inner edge of right borderline.

    const sal_Int32 nActualL
        = std::max<sal_Int32>(rLeftBorder.LineWidth / 2, rInfo.nLeftBorderDistance);
    const sal_Int32 nActualR
        = std::max<sal_Int32>(nActualL + rRightBorder.LineWidth / 2,
                              rInfo.nLeftBorderDistance + rInfo.nRightBorderDistance)
          - nActualL;
    rInfo.nLeftBorderDistance = nActualL;
    rInfo.nRightBorderDistance = nActualR;
}

}

TableStyleSheetEntry * DomainMapperTableHandler::endTableGetTableStyle(TableInfo & rInfo, std::vector<beans::PropertyValue>& rFrameProperties)
{
    // will receive the table style if any
    TableStyleSheetEntry* pTableStyle = nullptr;

    if( m_aTableProperties.get() )
    {
        //create properties from the table attributes
        //...pPropMap->Insert( PROP_LEFT_MARGIN, uno::makeAny( m_nLeftMargin - m_nGapHalf ));
        //pPropMap->Insert( PROP_HORI_ORIENT, uno::makeAny( text::HoriOrientation::RIGHT ));
        sal_Int32 nGapHalf = 0;
        sal_Int32 nLeftMargin = 0;

        comphelper::SequenceAsHashMap aGrabBag;

        if (nullptr != m_rDMapper_Impl.getTableManager().getCurrentTableRealPosition())
        {
            TablePositionHandler *pTablePositions = m_rDMapper_Impl.getTableManager().getCurrentTableRealPosition();

            uno::Sequence< beans::PropertyValue  > aGrabBagTS( 10 );

            aGrabBagTS[0].Name = "bottomFromText";
            aGrabBagTS[0].Value <<= pTablePositions->getBottomFromText();

            aGrabBagTS[1].Name = "horzAnchor";
            aGrabBagTS[1].Value <<= pTablePositions->getHorzAnchor();

            aGrabBagTS[2].Name = "leftFromText";
            aGrabBagTS[2].Value <<= pTablePositions->getLeftFromText();

            aGrabBagTS[3].Name = "rightFromText";
            aGrabBagTS[3].Value <<= pTablePositions->getRightFromText();

            aGrabBagTS[4].Name = "tblpX";
            aGrabBagTS[4].Value <<= pTablePositions->getX();

            aGrabBagTS[5].Name = "tblpXSpec";
            aGrabBagTS[5].Value <<= pTablePositions->getXSpec();

            aGrabBagTS[6].Name = "tblpY";
            aGrabBagTS[6].Value <<= pTablePositions->getY();

            aGrabBagTS[7].Name = "tblpYSpec";
            aGrabBagTS[7].Value <<= pTablePositions->getYSpec();

            aGrabBagTS[8].Name = "topFromText";
            aGrabBagTS[8].Value <<= pTablePositions->getTopFromText();

            aGrabBagTS[9].Name = "vertAnchor";
            aGrabBagTS[9].Value <<= pTablePositions->getVertAnchor();

            aGrabBag["TablePosition"] <<= aGrabBagTS;
        }

        boost::optional<PropertyMap::Property> aTableStyleVal = m_aTableProperties->getProperty(META_PROP_TABLE_STYLE_NAME);
        if(aTableStyleVal)
        {
            // Apply table style properties recursively
            OUString sTableStyleName;
            aTableStyleVal->second >>= sTableStyleName;
            StyleSheetTablePtr pStyleSheetTable = m_rDMapper_Impl.GetStyleSheetTable();
            const StyleSheetEntryPtr pStyleSheet = pStyleSheetTable->FindStyleSheetByISTD( sTableStyleName );
            pTableStyle = dynamic_cast<TableStyleSheetEntry*>( pStyleSheet.get( ) );
            m_aTableProperties->Erase( aTableStyleVal->first );

            aGrabBag["TableStyleName"] <<= sTableStyleName;

            if( pStyleSheet )
            {
                // First get the style properties, then the table ones
                PropertyMapPtr pTableProps( m_aTableProperties.get() );
                TablePropertyMapPtr pEmptyProps( new TablePropertyMap );

                m_aTableProperties = pEmptyProps;

                PropertyMapPtr pMergedProperties = pStyleSheet->GetMergedInheritedProperties(pStyleSheetTable);

                table::BorderLine2 aBorderLine;
                TableInfo rStyleInfo;
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_TOP_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag["TableStyleTopBorder"] <<= aBorderLine;
                }
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_BOTTOM_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag["TableStyleBottomBorder"] <<= aBorderLine;
                }
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_LEFT_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag["TableStyleLeftBorder"] <<= aBorderLine;
                }
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_RIGHT_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag["TableStyleRightBorder"] <<= aBorderLine;
                }

#ifdef DEBUG_WRITERFILTER
                TagLogger::getInstance().startElement("mergedProps");
                if (pMergedProperties)
                    pMergedProperties->dumpXml();
                TagLogger::getInstance().endElement();
#endif

                m_aTableProperties->InsertProps(pMergedProperties);
                m_aTableProperties->InsertProps(pTableProps);

#ifdef DEBUG_WRITERFILTER
                TagLogger::getInstance().startElement("TableProperties");
                m_aTableProperties->dumpXml();
                TagLogger::getInstance().endElement();
#endif
            }
        }

        // This is the one preserving just all the table look attributes.
        boost::optional<PropertyMap::Property> oTableLook = m_aTableProperties->getProperty(META_PROP_TABLE_LOOK);
        if (oTableLook)
        {
            aGrabBag["TableStyleLook"] = oTableLook->second;
            m_aTableProperties->Erase(oTableLook->first);
        }

        // This is just the "val" attribute's numeric value.
        const boost::optional<PropertyMap::Property> aTblLook = m_aTableProperties->getProperty(PROP_TBL_LOOK);
        if(aTblLook)
        {
            aTblLook->second >>= rInfo.nTblLook;
            m_aTableProperties->Erase( aTblLook->first );
        }

        // Set the table default attributes for the cells
        rInfo.pTableDefaults->InsertProps(m_aTableProperties.get());

#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().startElement("TableDefaults");
        rInfo.pTableDefaults->dumpXml();
        TagLogger::getInstance().endElement();
#endif

        if (!aGrabBag.empty())
        {
            m_aTableProperties->Insert( PROP_TABLE_INTEROP_GRAB_BAG, uno::makeAny( aGrabBag.getAsConstPropertyValueList() ) );
        }

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
        aDistances.IsRightDistanceValid = true;
        aDistances.TopDistance = static_cast<sal_Int16>( rInfo.nTopBorderDistance );
        aDistances.BottomDistance = static_cast<sal_Int16>( rInfo.nBottomBorderDistance );
        aDistances.LeftDistance = static_cast<sal_Int16>( rInfo.nLeftBorderDistance );
        aDistances.RightDistance = static_cast<sal_Int16>( rInfo.nRightBorderDistance );

        m_aTableProperties->Insert( PROP_TABLE_BORDER_DISTANCES, uno::makeAny( aDistances ) );

        if (!rFrameProperties.empty())
            lcl_DecrementHoriOrientPosition(rFrameProperties, rInfo.nLeftBorderDistance);

        // Set table above/bottom spacing to 0.
        m_aTableProperties->Insert( PROP_TOP_MARGIN, uno::makeAny( sal_Int32( 0 ) ) );
        m_aTableProperties->Insert( PROP_BOTTOM_MARGIN, uno::makeAny( sal_Int32( 0 ) ) );

        //table border settings
        table::TableBorder aTableBorder;
        table::BorderLine2 aBorderLine, aLeftBorder, aRightBorder;

        if (lcl_extractTableBorderProperty(m_aTableProperties.get(), PROP_TOP_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.TopLine = aBorderLine;
            aTableBorder.IsTopLineValid = true;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties.get(), PROP_BOTTOM_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.BottomLine = aBorderLine;
            aTableBorder.IsBottomLineValid = true;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties.get(), PROP_LEFT_BORDER, rInfo, aLeftBorder))
        {
            aTableBorder.LeftLine = aLeftBorder;
            aTableBorder.IsLeftLineValid = true;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties.get(), PROP_RIGHT_BORDER, rInfo,
                                           aRightBorder))
        {
            aTableBorder.RightLine = aRightBorder;
            aTableBorder.IsRightLineValid = true;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties.get(), META_PROP_HORIZONTAL_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.HorizontalLine = aBorderLine;
            aTableBorder.IsHorizontalLineValid = true;
        }
        if (lcl_extractTableBorderProperty(m_aTableProperties.get(), META_PROP_VERTICAL_BORDER, rInfo, aBorderLine))
        {
            aTableBorder.VerticalLine = aBorderLine;
            aTableBorder.IsVerticalLineValid = true;
        }

        aTableBorder.Distance = 0;
        aTableBorder.IsDistanceValid = false;

        m_aTableProperties->Insert( PROP_TABLE_BORDER, uno::makeAny( aTableBorder ) );

#ifdef DEBUG_WRITERFILTER
        lcl_debug_TableBorder(aTableBorder);
#endif

        // Table position in Office is computed in 2 different ways :
        // - top level tables: the goal is to have in-cell text starting at table indent pos (tblInd),
        //   so table's position depends on table's cells margin
        // - nested tables: the goal is to have left-most border starting at table_indent pos

        // Only top level table position depends on border width of Column A.
        // TODO: Position based on last row (at least in MSOffice 2016), but first row in Office 2003.
        //       Export code is also based on first cell, so using first row here...
        if ( !m_aCellProperties.empty() && !m_aCellProperties[0].empty() )
        {
            // aLeftBorder already contains tblBorder; overwrite if cell is different.
            boost::optional<PropertyMap::Property> aCellBorder
                = m_aCellProperties[0][0]->getProperty(PROP_LEFT_BORDER);
            if ( aCellBorder )
                aCellBorder->second >>= aLeftBorder;
            aCellBorder = m_aCellProperties[0][0]->getProperty(PROP_RIGHT_BORDER);
            if (aCellBorder)
                aCellBorder->second >>= aRightBorder;
        }
        if (rInfo.nNestLevel == 1 && aLeftBorder.LineWidth && !rFrameProperties.empty())
        {
            lcl_DecrementHoriOrientPosition(rFrameProperties, aLeftBorder.LineWidth * 0.5);
        }
        lcl_adjustBorderDistance(rInfo, aLeftBorder, aRightBorder);

        // tdf#106742: since MS Word 2013 (compatibilityMode >= 15), top-level tables are handled the same as nested tables;
        // this is also the default behavior in LO when DOCX doesn't define "compatibilityMode" option
        sal_Int32 nMode = m_rDMapper_Impl.GetSettingsTable()->GetWordCompatibilityMode();

        if ( nMode > 0 && nMode <= 14 && rInfo.nNestLevel == 1 )
        {
            m_aTableProperties->Insert( PROP_LEFT_MARGIN, uno::makeAny( nLeftMargin - nGapHalf - rInfo.nLeftBorderDistance ) );
        }
        else
        {
            m_aTableProperties->Insert( PROP_LEFT_MARGIN, uno::makeAny( nLeftMargin - nGapHalf ) );
        }

        sal_Int32 nTableWidth = 0;
        sal_Int32 nTableWidthType = text::SizeType::FIX;
        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH, nTableWidth );
        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType );
        if( nTableWidthType == text::SizeType::FIX )
        {
            if( nTableWidth > 0 )
                m_aTableProperties->Insert( PROP_WIDTH, uno::makeAny( nTableWidth ));
            else
            {
                // tdf#109524: If there is no width for the table, make it simply 100% by default.
                // TODO: use cell contents to evaluate width (according to ECMA-376-1:2016 17.18.87)
                nTableWidth = 100;
                nTableWidthType = text::SizeType::VARIABLE;
            }
        }
        if (nTableWidthType != text::SizeType::FIX)
        {
            m_aTableProperties->Insert( PROP_RELATIVE_WIDTH, uno::makeAny( sal_Int16( nTableWidth ) ) );
            m_aTableProperties->Insert( PROP_IS_WIDTH_RELATIVE, uno::makeAny( true ) );
        }

        sal_Int32 nHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH;
        // Fetch Horizontal Orientation in rFrameProperties if not set in m_aTableProperties
        if ( !m_aTableProperties->getValue( TablePropertyMap::HORI_ORIENT, nHoriOrient ) )
            lcl_extractHoriOrient( rFrameProperties, nHoriOrient );
        m_aTableProperties->Insert( PROP_HORI_ORIENT, uno::makeAny( sal_Int16(nHoriOrient) ) );
        //fill default value - if not available
        m_aTableProperties->Insert( PROP_HEADER_ROW_COUNT, uno::makeAny( sal_Int32(0)), false);

        // if table is only a single row, and row is set as don't split, set the same value for the whole table.
        if( m_aRowProperties.size() == 1 && m_aRowProperties[0].get() )
        {
            boost::optional<PropertyMap::Property> oSplitAllowed = m_aRowProperties[0]->getProperty(PROP_IS_SPLIT_ALLOWED);
            if( oSplitAllowed )
            {
                bool bRowCanSplit = true;
                oSplitAllowed->second >>= bRowCanSplit;
                if( !bRowCanSplit )
                    m_aTableProperties->Insert( PROP_SPLIT, uno::makeAny(bRowCanSplit) );
            }
        }

        rInfo.aTableProperties = m_aTableProperties->GetPropertyValues();

#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().startElement("debug.tableprops");
        m_aTableProperties->dumpXml();
        TagLogger::getInstance().endElement();
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

CellPropertyValuesSeq_t DomainMapperTableHandler::endTableGetCellProperties(TableInfo & rInfo, std::vector<HorizontallyMergedCell>& rMerges)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("getCellProperties");
#endif

    CellPropertyValuesSeq_t aCellProperties( m_aCellProperties.size() );

    if ( m_aCellProperties.empty() )
    {
        #ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().endElement();
        #endif
        return aCellProperties;
    }
    // std::vector< std::vector<PropertyMapPtr> > m_aCellProperties
    PropertyMapVector2::const_iterator aRowOfCellsIterator = m_aCellProperties.begin();
    PropertyMapVector2::const_iterator aRowOfCellsIteratorEnd = m_aCellProperties.end();
    PropertyMapVector2::const_iterator aLastRowIterator = m_aCellProperties.end() - 1;
    sal_Int32 nRow = 0;

    css::uno::Sequence<css::beans::PropertyValues>* pCellProperties = aCellProperties.getArray();
    PropertyMapVector1::const_iterator aRowIter = m_aRowProperties.begin();
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
        else if (*aRowIter && (*aRowIter)->isSet(PROP_TBL_HEADER))
            nRowStyleMask |= CNF_FIRST_ROW; // table header implies first row
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
            if( *aCellIterator )
            {
                // remove directly applied insideV/H borders since they are meaningless without a context (tdf#82177)
                (*aCellIterator)->Erase(META_PROP_VERTICAL_BORDER);
                (*aCellIterator)->Erase(META_PROP_HORIZONTAL_BORDER);

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

                    // Check if we need to clean up some empty border definitions to match what Word does.
                    static const PropertyIds pBorders[] =
                    {
                        PROP_TOP_BORDER, PROP_LEFT_BORDER, PROP_BOTTOM_BORDER, PROP_RIGHT_BORDER
                    };
                    for (const PropertyIds& rBorder : pBorders)
                    {
                        boost::optional<PropertyMap::Property> oStyleCellBorder = pStyleProps->getProperty(rBorder);
                        boost::optional<PropertyMap::Property> oDirectCellBorder = (*aCellIterator)->getProperty(rBorder);
                        if (oStyleCellBorder && oDirectCellBorder)
                        {
                            // We have a cell border from the table style and as direct formatting as well.
                            table::BorderLine2 aStyleCellBorder = oStyleCellBorder->second.get<table::BorderLine2>();
                            table::BorderLine2 aDirectCellBorder = oDirectCellBorder->second.get<table::BorderLine2>();
                            if (aStyleCellBorder.LineStyle != table::BorderLineStyle::NONE && aDirectCellBorder.LineStyle == table::BorderLineStyle::NONE)
                            {
                                // The style one would be visible, but then cleared away as direct formatting.
                                // Delete both, so that table formatting can become visible.
                                pStyleProps->Erase(rBorder);
                                (*aCellIterator)->Erase(rBorder);
                            }
                            else
                            {
                                boost::optional<PropertyMap::Property> oTableBorder = rInfo.pTableBorders->getProperty(rBorder);
                                if (oTableBorder)
                                {
                                    table::BorderLine2 aTableBorder = oTableBorder->second.get<table::BorderLine2>();
                                    // Both style and direct formatting says that the cell has no border.
                                    bool bNoCellBorder = aStyleCellBorder.LineStyle == table::BorderLineStyle::NONE && aDirectCellBorder.LineStyle == table::BorderLineStyle::NONE;
                                    if (aTableBorder.LineStyle != table::BorderLineStyle::NONE && bNoCellBorder)
                                    {
                                        // But at a table-level, there is a border, then again delete both cell properties.
                                        pStyleProps->Erase(rBorder);
                                        (*aCellIterator)->Erase(rBorder);
                                    }
                                }
                            }
                        }
                    }

                    pAllCellProps->InsertProps( pStyleProps );
                }

                // Remove properties from style/row that aren't allowed in cells
                pAllCellProps->Erase( PROP_HEADER_ROW_COUNT );
                pAllCellProps->Erase( PROP_TBL_HEADER );

                // Then add the cell properties
                pAllCellProps->InsertProps(*aCellIterator);
                std::swap(*(*aCellIterator), *pAllCellProps );

#ifdef DEBUG_WRITERFILTER
                TagLogger::getInstance().startElement("cell");
                TagLogger::getInstance().attribute("cell", nCell);
                TagLogger::getInstance().attribute("row", nRow);
#endif

                lcl_computeCellBorders( rInfo.pTableBorders, *aCellIterator, nCell, nRow, bIsEndCol, bIsEndRow );

                //now set the default left+right border distance TODO: there's an sprm containing the default distance!
                aCellIterator->get()->Insert( PROP_LEFT_BORDER_DISTANCE,
                                                 uno::makeAny(rInfo.nLeftBorderDistance ), false);
                aCellIterator->get()->Insert( PROP_RIGHT_BORDER_DISTANCE,
                                                 uno::makeAny(rInfo.nRightBorderDistance ), false);
                aCellIterator->get()->Insert( PROP_TOP_BORDER_DISTANCE,
                                                 uno::makeAny(rInfo.nTopBorderDistance ), false);
                aCellIterator->get()->Insert( PROP_BOTTOM_BORDER_DISTANCE,
                                                 uno::makeAny(rInfo.nBottomBorderDistance ), false);

                // Horizontal merge is not an UNO property, extract that info here to rMerges, and then remove it from the map.
                const boost::optional<PropertyMap::Property> aHorizontalMergeVal = (*aCellIterator)->getProperty(PROP_HORIZONTAL_MERGE);
                if (aHorizontalMergeVal)
                {
                    if (aHorizontalMergeVal->second.get<bool>())
                    {
                        // first cell in a merge
                        HorizontallyMergedCell aMerge(nRow, nCell);
                        rMerges.push_back(aMerge);
                    }
                    else if (!rMerges.empty())
                    {
                        // resuming an earlier merge
                        HorizontallyMergedCell& rMerge = rMerges.back();
                        rMerge.m_nLastRow = nRow;
                        rMerge.m_nLastCol = nCell;
                    }
                    (*aCellIterator)->Erase(PROP_HORIZONTAL_MERGE);
                }

                // Cell direction is not an UNO Property, either.
                const boost::optional<PropertyMap::Property> aCellDirectionVal = (*aCellIterator)->getProperty(PROP_CELL_DIRECTION);
                if (aCellDirectionVal)
                {
                    if (aCellDirectionVal->second.get<sal_Int32>() == static_cast<sal_Int32>(NS_ooxml::LN_Value_ST_TextDirection_btLr))
                    {
                        // btLr, so map ParagraphAdjust_CENTER to VertOrientation::CENTER.
                        uno::Reference<beans::XPropertySet> xPropertySet(m_aTableRanges[nRow][nCell][0], uno::UNO_QUERY);
                        if (xPropertySet.is() && xPropertySet->getPropertyValue("ParaAdjust").get<sal_Int16>() == sal_Int16(style::ParagraphAdjust_CENTER))
                            (*aCellIterator)->Insert(PROP_VERT_ORIENT, uno::makeAny(text::VertOrientation::CENTER));
                    }
                    (*aCellIterator)->Erase(PROP_CELL_DIRECTION);
                }

                pSingleCellProperties[nCell] = (*aCellIterator)->GetPropertyValues();
#ifdef DEBUG_WRITERFILTER
                TagLogger::getInstance().endElement();
#endif
            }
            ++nCell;
            ++aCellIterator;
        }
        ++nRow;
        ++aRowOfCellsIterator;
        ++aRowIter;
    }

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif

    return aCellProperties;
}

/// Do all cells in this row have a CellHideMark property?
static bool lcl_hideMarks(PropertyMapVector1& rCellProperties)
{
    for (PropertyMapPtr & p : rCellProperties)
    {
        // if anything is vertically merged, the row must not be set to fixed
        // as Writer's layout doesn't handle that well
        if (!p->isSet(PROP_CELL_HIDE_MARK) || p->isSet(PROP_VERTICAL_MERGE))
            return false;
    }
    return true;
}

/// Are all cells in this row empty?
static bool lcl_emptyRow(std::vector<RowSequence_t>& rTableRanges, sal_Int32 nRow)
{
    if (nRow >= static_cast<sal_Int32>(rTableRanges.size()))
    {
        SAL_WARN("writerfilter.dmapper", "m_aCellProperties not in sync with rTableRanges?");
        return false;
    }

    RowSequence_t rRowSeq = rTableRanges[nRow];
    if (rRowSeq.getLength() == 0)
    {
        SAL_WARN("writerfilter.dmapper", "m_aCellProperties not in sync with rTableRanges?");
        return false;
    }

    if (!rRowSeq[0][0].is())
    {
        // This can happen when we can't import the table, e.g. we're inside a
        // comment.
        SAL_WARN("writerfilter.dmapper", "rRowSeq[0][0] is an empty reference");
        return false;
    }

    uno::Reference<text::XTextRangeCompare> xTextRangeCompare(rRowSeq[0][0]->getText(), uno::UNO_QUERY);
    try
    {
        for (sal_Int32 nCell = 0; nCell < rRowSeq.getLength(); ++nCell)
            // See SwXText::Impl::ConvertCell(), we need to compare the start of
            // the start and the end of the end. However for our text ranges, only
            // the starts are set, so compareRegionStarts() does what we need.
            if (xTextRangeCompare->compareRegionStarts(rRowSeq[nCell][0], rRowSeq[nCell][1]) != 0)
                return false;
    }
    catch (const lang::IllegalArgumentException& e)
    {
        SAL_WARN( "writerfilter.dmapper", "compareRegionStarts() failed: " << e);
        return false;
    }
    return true;
}

css::uno::Sequence<css::beans::PropertyValues> DomainMapperTableHandler::endTableGetRowProperties()
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("getRowProperties");
#endif

    css::uno::Sequence<css::beans::PropertyValues> aRowProperties( m_aRowProperties.size() );
    sal_Int32 nRow = 0;
    for( const auto& rRow : m_aRowProperties )
    {
#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().startElement("rowProps.row");
#endif
        if (rRow)
        {
            //set default to 'break across pages"
            rRow->Insert( PROP_IS_SPLIT_ALLOWED, uno::makeAny(true ), false );
            // tblHeader is only our property, remove before the property map hits UNO
            rRow->Erase(PROP_TBL_HEADER);

            if (lcl_hideMarks(m_aCellProperties[nRow]) && lcl_emptyRow(m_aTableRanges, nRow))
            {
                // We have CellHideMark on all cells, and also all cells are empty:
                // Force the row height to be exactly as specified, and not just as the minimum suggestion.
                rRow->Insert(PROP_SIZE_TYPE, uno::makeAny(text::SizeType::FIX));
            }

            aRowProperties[nRow] = rRow->GetPropertyValues();
#ifdef DEBUG_WRITERFILTER
            rRow->dumpXml();
            lcl_DumpPropertyValues(aRowProperties[nRow]);
#endif
        }
        ++nRow;
#ifdef DEBUG_WRITERFILTER
        TagLogger::getInstance().endElement();
#endif
    }

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif

    return aRowProperties;
}

// Apply paragraph property to each paragraph within a cell.
static void lcl_ApplyCellParaProps(uno::Reference<table::XCell> const& xCell,
        const uno::Any& rBottomMargin)
{
    uno::Reference<container::XEnumerationAccess> xEnumerationAccess(xCell, uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xEnumeration = xEnumerationAccess->createEnumeration();
    while (xEnumeration->hasMoreElements())
    {
        uno::Reference<beans::XPropertySet> xParagraph(xEnumeration->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertyState> xPropertyState(xParagraph, uno::UNO_QUERY);
        // Don't apply in case direct formatting is already present.
        // TODO: probably paragraph style has priority over table style here.
        if (xPropertyState.is() && xPropertyState->getPropertyState("ParaBottomMargin") == beans::PropertyState_DEFAULT_VALUE)
            xParagraph->setPropertyValue("ParaBottomMargin", rBottomMargin);
    }
}

void DomainMapperTableHandler::endTable(unsigned int nestedTableLevel, bool bTableStartsAtCellStart)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("tablehandler.endTable");
#endif

    // If we want to make this table a floating one.
    std::vector<beans::PropertyValue> aFrameProperties = comphelper::sequenceToContainer<std::vector<beans::PropertyValue> >
            (m_rDMapper_Impl.getTableManager().getCurrentTablePosition());
    TableInfo aTableInfo;
    aTableInfo.nNestLevel = nestedTableLevel;
    aTableInfo.pTableStyle = endTableGetTableStyle(aTableInfo, aFrameProperties);
    //  expands to uno::Sequence< Sequence< beans::PropertyValues > >

    std::vector<HorizontallyMergedCell> aMerges;
    CellPropertyValuesSeq_t aCellProperties = endTableGetCellProperties(aTableInfo, aMerges);

    css::uno::Sequence<css::beans::PropertyValues> aRowProperties = endTableGetRowProperties();

#ifdef DEBUG_WRITERFILTER
    lcl_DumpPropertyValueSeq(aRowProperties);
#endif

    if (!m_aTableRanges.empty())
    {
        uno::Reference<text::XTextRange> xStart;
        uno::Reference<text::XTextRange> xEnd;

        bool bFloating = !aFrameProperties.empty();
        // Additional checks: if we can do this.
        if (bFloating && m_aTableRanges[0].getLength() > 0 && m_aTableRanges[0][0].getLength() > 0)
        {
            xStart = m_aTableRanges[0][0][0];
            uno::Sequence< uno::Sequence< uno::Reference<text::XTextRange> > >& rLastRow = m_aTableRanges[m_aTableRanges.size() - 1];
            if (rLastRow.getLength())
            {
                uno::Sequence< uno::Reference<text::XTextRange> >& rLastCell = rLastRow[rLastRow.getLength() - 1];
                xEnd = rLastCell[1];
            }
        }
        uno::Reference<text::XTextTable> xTable;
        try
        {
            if (m_xText.is())
            {
                xTable = m_xText->convertToTable(comphelper::containerToSequence(m_aTableRanges), aCellProperties, aRowProperties, aTableInfo.aTableProperties);

                if (xTable.is())
                {
                    if (!aMerges.empty())
                    {
                        static const std::vector<OUStringLiteral> aBorderNames
                            = { "TopBorder", "LeftBorder", "BottomBorder", "RightBorder" };

                        // Perform horizontal merges in reverse order, so the fact that merging changes the position of cells won't cause a problem for us.
                        for (std::vector<HorizontallyMergedCell>::reverse_iterator it = aMerges.rbegin(); it != aMerges.rend(); ++it)
                        {
                            uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY_THROW);
                            uno::Reference<beans::XPropertySet> xFirstCell(
                                xCellRange->getCellByPosition(it->m_nFirstCol, it->m_nFirstRow),
                                uno::UNO_QUERY_THROW);
                            OUString aFirst
                                = xFirstCell->getPropertyValue("CellName").get<OUString>();
                            // tdf#105852: Only try to merge if m_nLastCol is set (i.e. there were some merge continuation cells)
                            if (it->m_nLastCol != -1)
                            {
                                // Save border properties of the first cell
                                // before merge.
                                table::BorderLine2 aBorderValues[4];
                                for (size_t i = 0; i < aBorderNames.size(); ++i)
                                    xFirstCell->getPropertyValue(aBorderNames[i])
                                        >>= aBorderValues[i];

                                uno::Reference<beans::XPropertySet> xLastCell(
                                    xCellRange->getCellByPosition(it->m_nLastCol, it->m_nLastRow),
                                    uno::UNO_QUERY_THROW);
                                OUString aLast
                                    = xLastCell->getPropertyValue("CellName").get<OUString>();

                                uno::Reference<text::XTextTableCursor> xCursor = xTable->createCursorByCellName(aFirst);
                                xCursor->gotoCellByName(aLast, true);

                                xCursor->mergeRange();

                                // Handle conflicting properties: mergeRange()
                                // takes the last cell, Word takes the first
                                // cell.
                                for (size_t i = 0; i < aBorderNames.size(); ++i)
                                {
                                    if (aBorderValues[i].LineStyle != table::BorderLineStyle::NONE)
                                        xFirstCell->setPropertyValue(
                                            aBorderNames[i], uno::makeAny(aBorderValues[i]));
                                }
                            }
                        }
                    }
                }

                // OOXML table style may container paragraph properties, apply these now.
                for (int i = 0; i < aTableInfo.aTableProperties.getLength(); ++i)
                {
                    if (aTableInfo.aTableProperties[i].Name == "ParaBottomMargin")
                    {
                        uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY);
                        uno::Any aBottomMargin = aTableInfo.aTableProperties[i].Value;
                        sal_Int32 nRows = aCellProperties.getLength();
                        for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
                        {
                            const uno::Sequence< beans::PropertyValues > aCurrentRow = aCellProperties[nRow];
                            sal_Int32 nCells = aCurrentRow.getLength();
                            for (sal_Int32 nCell = 0; nCell < nCells; ++nCell)
                                lcl_ApplyCellParaProps(xCellRange->getCellByPosition(nCell, nRow), aBottomMargin);
                        }
                        break;
                    }
                }
            }
        }
        catch ( const lang::IllegalArgumentException &e )
        {
            SAL_INFO("writerfilter.dmapper",
                    "Conversion to table error: " << e);
#ifdef DEBUG_WRITERFILTER
            TagLogger::getInstance().chars(std::string("failed to import table!"));
#endif
        }
        catch ( const uno::Exception &e )
        {
            SAL_INFO("writerfilter.dmapper",
                    "Exception during table creation: " << e);
        }

        // If we have a table with a start and an end position, we should make it a floating one.
        // Unless the table had a foot or endnote, as Writer doesn't support those in TextFrames.
        if (xTable.is() && xStart.is() && xEnd.is() && !m_bHadFootOrEndnote)
        {
            uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
            bool bIsRelative = false;
            xTableProperties->getPropertyValue("IsWidthRelative") >>= bIsRelative;
            if (!bIsRelative)
            {
                beans::PropertyValue aValue;
                aValue.Name = "Width";
                aValue.Value = xTableProperties->getPropertyValue("Width");
                aFrameProperties.push_back(aValue);
            }
            else
            {
                beans::PropertyValue aValue;
                aValue.Name = "FrameWidthPercent";
                aValue.Value = xTableProperties->getPropertyValue("RelativeWidth");
                aFrameProperties.push_back(aValue);

                // Applying the relative width to the frame, needs to have the table width to be 100% of the frame width
                xTableProperties->setPropertyValue("RelativeWidth", uno::makeAny(sal_Int16(100)));
            }

            // A non-zero left margin would move the table out of the frame, move the frame itself instead.
            xTableProperties->setPropertyValue("LeftMargin", uno::makeAny(sal_Int32(0)));

            // In case the document ends with a table, we're called after
            // SectionPropertyMap::CloseSectionGroup(), so we'll have no idea
            // about the text area width, nor can fix this by delaying the text
            // frame conversion: just do it here.
            // Also, when the anchor is within a table, then do it here as well,
            // as xStart/xEnd would not point to the start/end at conversion
            // time anyway.
            // Next exception: it's pointless to delay the conversion if the
            // table is not in the body text.
            sal_Int32 nTableWidth = 0;
            m_aTableProperties->getValue(TablePropertyMap::TABLE_WIDTH, nTableWidth);
            sal_Int32 nTableWidthType = text::SizeType::FIX;
            m_aTableProperties->getValue(TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType);
            if (m_rDMapper_Impl.GetSectionContext() && nestedTableLevel <= 1 && !m_rDMapper_Impl.IsInHeaderFooter())
                m_rDMapper_Impl.m_aPendingFloatingTables.emplace_back(xStart, xEnd, comphelper::containerToSequence(aFrameProperties), nTableWidth, nTableWidthType);
            else
            {
                // m_xText points to the body text, get the current xText from m_rDMapper_Impl, in case e.g. we would be in a header.
                uno::Reference<text::XTextAppendAndConvert> xTextAppendAndConvert(m_rDMapper_Impl.GetTopTextAppend(), uno::UNO_QUERY);
                // Only execute the conversion if the table is not anchored at
                // the start of an outer table cell, that's not yet
                // implemented.
                if (xTextAppendAndConvert.is() && !bTableStartsAtCellStart)
                    xTextAppendAndConvert->convertToTextFrame(xStart, xEnd, comphelper::containerToSequence(aFrameProperties));
            }
        }

        // We're right after a table conversion.
        m_rDMapper_Impl.m_bConvertedTable = true;
    }

    m_aTableProperties.clear();
    m_aCellProperties.clear();
    m_aRowProperties.clear();
    m_bHadFootOrEndnote = false;

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapperTableHandler::startRow(const TablePropertyMapPtr& pProps)
{
    m_aRowProperties.push_back( pProps.get() );
    m_aCellProperties.emplace_back( );

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("table.row");
    if (pProps != nullptr)
        pProps->dumpXml();
#endif

    m_aRowRanges.clear();
}

void DomainMapperTableHandler::endRow()
{
    m_aTableRanges.push_back(comphelper::containerToSequence(m_aRowRanges));
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapperTableHandler::startCell(const css::uno::Reference< css::text::XTextRange > & start,
                                         const TablePropertyMapPtr& pProps )
{
    sal_uInt32 nRow = m_aRowProperties.size();
    if ( pProps.get( ) )
        m_aCellProperties[nRow - 1].push_back( pProps.get() );
    else
    {
        // Adding an empty cell properties map to be able to get
        // the table defaults properties
        TablePropertyMapPtr pEmptyProps( new TablePropertyMap( ) );
        m_aCellProperties[nRow - 1].push_back( pEmptyProps.get() );
    }

#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("table.cell");
    TagLogger::getInstance().startElement("table.cell.start");
    TagLogger::getInstance().chars(XTextRangeToString(start));
    TagLogger::getInstance().endElement();
    if (pProps.get())
        pProps->printProperties();
#endif

    //add a new 'row' of properties
    m_aCellRange.clear();
    uno::Reference<text::XTextRange> xStart;
    if (start.get())
        xStart = start->getStart();
    m_aCellRange.push_back(xStart);
}

void DomainMapperTableHandler::endCell(const css::uno::Reference< css::text::XTextRange > & end)
{
#ifdef DEBUG_WRITERFILTER
    TagLogger::getInstance().startElement("table.cell.end");
    TagLogger::getInstance().chars(XTextRangeToString(end));
    TagLogger::getInstance().endElement();
    TagLogger::getInstance().endElement();
#endif

    uno::Reference<text::XTextRange> xEnd;
    if (end.get())
        xEnd = end->getEnd();
    m_aCellRange.push_back(xEnd);
    m_aRowRanges.push_back(comphelper::containerToSequence(m_aCellRange));
}

void DomainMapperTableHandler::setHadFootOrEndnote(bool bHadFootOrEndnote)
{
    m_bHadFootOrEndnote = bHadFootOrEndnote;
}

DomainMapper_Impl& DomainMapperTableHandler::getDomainMapperImpl()
{
    return m_rDMapper_Impl;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
