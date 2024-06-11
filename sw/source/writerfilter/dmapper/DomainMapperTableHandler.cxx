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

#include <sal/config.h>

#include <string_view>

#include "DomainMapperTableHandler.hxx"
#include "DomainMapper_Impl.hxx"
#include "StyleSheetTable.hxx"

#include <com/sun/star/beans/TolerantPropertySetResultType.hpp>
#include <com/sun/star/beans/XTolerantMultiPropertySet.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/table/TableBorderDistances.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/SizeType.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include "TablePositionHandler.hxx"
#include "TagLogger.hxx"
#include "util.hxx"
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <officecfg/Office/Writer.hxx>

#ifdef DBG_UTIL
#include "PropertyMapHelper.hxx"
#include <rtl/ustring.hxx>
#include <utility>
#endif

namespace writerfilter::dmapper {

using namespace ::com::sun::star;

#define DEF_BORDER_DIST 190  //0,19cm
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
#define CNF_ALL                     0xFFF

// total number of table columns
#define MAXTABLECELLS 63

DomainMapperTableHandler::DomainMapperTableHandler(
            css::uno::Reference<css::text::XTextAppendAndConvert> xText,
            DomainMapper_Impl& rDMapper_Impl)
    : m_xText(std::move(xText)),
        m_rDMapper_Impl( rDMapper_Impl )
{
}

DomainMapperTableHandler::~DomainMapperTableHandler()
{
}

void DomainMapperTableHandler::startTable(const TablePropertyMapPtr& pProps)
{
    m_aTableProperties = pProps;
    m_aTableRanges.clear();

#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablehandler.table");

    if (pProps)
        pProps->dumpXml();
#endif
}

static void lcl_mergeBorder( PropertyIds nId, const PropertyMapPtr& pOrig, const PropertyMapPtr& pDest )
{
    std::optional<PropertyMap::Property> pOrigVal = pOrig->getProperty(nId);

    if ( pOrigVal )
    {
        pDest->Insert( nId, pOrigVal->second, false );
    }
}

static void lcl_computeCellBorders( const PropertyMapPtr& pTableBorders, const PropertyMapPtr& pCellProps,
        sal_uInt32 nCell, sal_uInt32 nFirstCell, sal_uInt32 nLastCell, sal_Int32 nRow, bool bIsEndRow, bool bMergedVertically )
{
    const bool bIsStartCol = nCell == nFirstCell;
    const bool bIsEndCol = nCell == nLastCell;
    std::optional<PropertyMap::Property> pVerticalVal = pCellProps->getProperty(META_PROP_VERTICAL_BORDER);
    std::optional<PropertyMap::Property> pHorizontalVal = pCellProps->getProperty(META_PROP_HORIZONTAL_BORDER);

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

    if ( bIsStartCol )
        lcl_mergeBorder( PROP_LEFT_BORDER, pTableBorders, pCellProps );

    if ( bIsEndCol )
        lcl_mergeBorder( PROP_RIGHT_BORDER, pTableBorders, pCellProps );

    // <w:insideV> counts if there are multiple cells in this row.
    if ( pVerticalVal )
    {
        if ( !bIsEndCol && nCell >= nFirstCell )
            pCellProps->Insert( PROP_RIGHT_BORDER, aVertProp, false );
        if ( !bIsStartCol && nCell <= nLastCell )
            pCellProps->Insert( PROP_LEFT_BORDER, aVertProp, false );
    }

    if ( nRow == 0 )
    {
        lcl_mergeBorder( PROP_TOP_BORDER, pTableBorders, pCellProps );
        if ( pHorizontalVal && !bMergedVertically )
            pCellProps->Insert( PROP_BOTTOM_BORDER, aHorizProp, false );
    }

    if ( bMergedVertically )
        lcl_mergeBorder( PROP_BOTTOM_BORDER, pTableBorders, pCellProps );

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

#ifdef DBG_UTIL

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
    std::vector< PropertyIds > aTablePropertyIds;

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

    const std::optional<PropertyMap::Property> aTblBorder = pTableProperties->getProperty(nId);
    if( aTblBorder )
    {
        OSL_VERIFY(aTblBorder->second >>= rLine);

        rInfo.pTableBorders->Insert( nId, uno::Any( rLine ) );
        rInfo.pTableDefaults->Erase( nId );

        return true;
    }

    return false;
}

void lcl_extractHoriOrient(std::vector<beans::PropertyValue>& rFrameProperties, sal_Int32& nHoriOrient)
{
    // Shifts the frame left by the given value.
    for (const beans::PropertyValue & rFrameProperty : rFrameProperties)
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

TableStyleSheetEntry * DomainMapperTableHandler::endTableGetTableStyle(TableInfo & rInfo,
                std::vector<beans::PropertyValue>& rFrameProperties,
                bool bConvertToFloatingInFootnote)
{
    // will receive the table style if any
    TableStyleSheetEntry* pTableStyle = nullptr;

    if( m_aTableProperties )
    {
        //create properties from the table attributes
        sal_Int32 nLeftMargin = 0;

        comphelper::SequenceAsHashMap aGrabBag;

        if (bConvertToFloatingInFootnote)
        {
            // define empty "TablePosition" to avoid export temporary floating
            aGrabBag[u"TablePosition"_ustr] = uno::Any();
        }

        std::optional<PropertyMap::Property> aTableStyleVal = m_aTableProperties->getProperty(META_PROP_TABLE_STYLE_NAME);
        if(aTableStyleVal)
        {
            // Apply table style properties recursively
            OUString sTableStyleName;
            aTableStyleVal->second >>= sTableStyleName;
            StyleSheetTablePtr pStyleSheetTable = m_rDMapper_Impl.GetStyleSheetTable();
            const StyleSheetEntryPtr pStyleSheet = pStyleSheetTable->FindStyleSheetByISTD( sTableStyleName );
            pTableStyle = dynamic_cast<TableStyleSheetEntry*>( pStyleSheet.get( ) );
            m_aTableProperties->Erase( aTableStyleVal->first );

            aGrabBag[u"TableStyleName"_ustr] <<= sTableStyleName;

            if( pStyleSheet )
            {
                // First get the style properties, then the table ones
                PropertyMapPtr pTableProps( m_aTableProperties.get() );

                m_aTableProperties = TablePropertyMapPtr(new TablePropertyMap);

                PropertyMapPtr pMergedProperties = pStyleSheet->GetMergedInheritedProperties(pStyleSheetTable);

                table::BorderLine2 aBorderLine;
                TableInfo rStyleInfo;
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_TOP_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag[u"TableStyleTopBorder"_ustr] <<= aBorderLine;
                }
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_BOTTOM_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag[u"TableStyleBottomBorder"_ustr] <<= aBorderLine;
                }
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_LEFT_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag[u"TableStyleLeftBorder"_ustr] <<= aBorderLine;
                }
                if (lcl_extractTableBorderProperty(pMergedProperties, PROP_RIGHT_BORDER, rStyleInfo, aBorderLine))
                {
                    aGrabBag[u"TableStyleRightBorder"_ustr] <<= aBorderLine;
                }

#ifdef DBG_UTIL
                TagLogger::getInstance().startElement("mergedProps");
                if (pMergedProperties)
                    pMergedProperties->dumpXml();
                TagLogger::getInstance().endElement();
#endif

                m_aTableProperties->InsertProps(pMergedProperties);
                m_aTableProperties->InsertProps(pTableProps);

#ifdef DBG_UTIL
                TagLogger::getInstance().startElement("TableProperties");
                m_aTableProperties->dumpXml();
                TagLogger::getInstance().endElement();
#endif
                if (pTableStyle)
                {
                    // apply tblHeader setting of the table style
                    PropertyMapPtr pHeaderStyleProps = pTableStyle->GetProperties(CNF_FIRST_ROW);
                    if ( pHeaderStyleProps->getProperty(PROP_HEADER_ROW_COUNT) )
                        m_aTableProperties->Insert(PROP_HEADER_ROW_COUNT, uno::Any( sal_Int32(1)), false);
                }
            }
        }

        // This is the one preserving just all the table look attributes.
        std::optional<PropertyMap::Property> oTableLook = m_aTableProperties->getProperty(META_PROP_TABLE_LOOK);
        if (oTableLook)
        {
            aGrabBag[u"TableStyleLook"_ustr] = oTableLook->second;
            m_aTableProperties->Erase(oTableLook->first);
        }

        // This is just the "val" attribute's numeric value.
        const std::optional<PropertyMap::Property> aTblLook = m_aTableProperties->getProperty(PROP_TBL_LOOK);
        if(aTblLook)
        {
            aTblLook->second >>= rInfo.nTblLook;
            m_aTableProperties->Erase( aTblLook->first );
        }

        // apply cell margin settings of the table style
        const std::optional<PropertyMap::Property> oLeftMargin = m_aTableProperties->getProperty(META_PROP_CELL_MAR_LEFT);
        if (oLeftMargin)
        {
            oLeftMargin->second >>= rInfo.nLeftBorderDistance;
            m_aTableProperties->Erase(oLeftMargin->first);
        }
        const std::optional<PropertyMap::Property> oRightMargin = m_aTableProperties->getProperty(META_PROP_CELL_MAR_RIGHT);
        if (oRightMargin)
        {
            oRightMargin->second >>= rInfo.nRightBorderDistance;
            m_aTableProperties->Erase(oRightMargin->first);
        }
        const std::optional<PropertyMap::Property> oTopMargin = m_aTableProperties->getProperty(META_PROP_CELL_MAR_TOP);
        if (oTopMargin)
        {
            oTopMargin->second >>= rInfo.nTopBorderDistance;
            m_aTableProperties->Erase(oTopMargin->first);
        }
        const std::optional<PropertyMap::Property> oBottomMargin = m_aTableProperties->getProperty(META_PROP_CELL_MAR_BOTTOM);
        if (oBottomMargin)
        {
            oBottomMargin->second >>= rInfo.nBottomBorderDistance;
            m_aTableProperties->Erase(oBottomMargin->first);
        }

        // Set the table default attributes for the cells
        rInfo.pTableDefaults->InsertProps(m_aTableProperties.get());

#ifdef DBG_UTIL
        TagLogger::getInstance().startElement("TableDefaults");
        rInfo.pTableDefaults->dumpXml();
        TagLogger::getInstance().endElement();
#endif

        if (!aGrabBag.empty())
        {
            m_aTableProperties->Insert( PROP_TABLE_INTEROP_GRAB_BAG, uno::Any( aGrabBag.getAsConstPropertyValueList() ) );
        }

        std::optional<PropertyMap::Property> oLeftMarginFromStyle = m_aTableProperties->getProperty(PROP_LEFT_MARGIN);
        if (oLeftMarginFromStyle)
        {
            oLeftMarginFromStyle->second >>= nLeftMargin;
            // don't need to erase, we will push back the adjusted value
            // of this (or the direct formatting, if that exists) later
        }
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

        m_aTableProperties->Insert( PROP_TABLE_BORDER_DISTANCES, uno::Any( aDistances ) );

        sal_Int32 nMode = m_rDMapper_Impl.GetSettingsTable()->GetWordCompatibilityMode();
        if (!rFrameProperties.empty() && nMode < 15)
            lcl_DecrementHoriOrientPosition(rFrameProperties, rInfo.nLeftBorderDistance);

        // Set table above/bottom spacing to 0.
        m_aTableProperties->Insert( PROP_TOP_MARGIN, uno::Any( sal_Int32( 0 ) ) );
        m_aTableProperties->Insert( PROP_BOTTOM_MARGIN, uno::Any( sal_Int32( 0 ) ) );

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

        m_aTableProperties->Insert( PROP_TABLE_BORDER, uno::Any( aTableBorder ) );

#ifdef DBG_UTIL
        lcl_debug_TableBorder(aTableBorder);
#endif

        // Table position in Office is computed in 2 different ways :
        // - top level tables: the goal is to have in-cell text starting at table indent pos (tblInd),
        //   so table's position depends on table's cells margin
        // - nested tables: the goal is to have left-most border starting at table_indent pos

        // Only top level table position depends on border width of Column A.
        if ( !m_aCellProperties.empty() && !m_aCellProperties[0].empty() )
        {
            // aLeftBorder already contains tblBorder; overwrite if cell is different.
            std::optional<PropertyMap::Property> aCellBorder
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
        // the default behavior when DOCX doesn't define "compatibilityMode" option is to add the cell spacing

        if (0 < nMode && nMode <= 14 && rInfo.nNestLevel == 1)
        {
            const sal_Int32 nAdjustedMargin = nLeftMargin - rInfo.nLeftBorderDistance;
            m_aTableProperties->Insert( PROP_LEFT_MARGIN, uno::Any( nAdjustedMargin ) );
        }
        else
        {
            // Writer starts a table in the middle of the border.
            // Word starts a table at the left edge of the border,
            // so emulate that by adding the half the width. (also see docxattributeoutput)
            if ( rInfo.nNestLevel > 1 && nLeftMargin < 0 )
                nLeftMargin = 0;
            const sal_Int32 nAdjustedMargin = nLeftMargin + (aLeftBorder.LineWidth / 2);
            m_aTableProperties->Insert( PROP_LEFT_MARGIN, uno::Any( nAdjustedMargin ) );
        }

        sal_Int32 nTableWidth = 0;
        sal_Int32 nTableWidthType = text::SizeType::FIX;
        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH, nTableWidth );
        m_aTableProperties->getValue( TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType );
        if( nTableWidthType == text::SizeType::FIX )
        {
            if( nTableWidth > 0 )
                m_aTableProperties->Insert( PROP_WIDTH, uno::Any( nTableWidth ));
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
            m_aTableProperties->Insert( PROP_RELATIVE_WIDTH, uno::Any( sal_Int16( nTableWidth ) ) );
            m_aTableProperties->Insert( PROP_IS_WIDTH_RELATIVE, uno::Any( true ) );
        }

        sal_Int32 nHoriOrient = text::HoriOrientation::LEFT_AND_WIDTH;
        // Fetch Horizontal Orientation in rFrameProperties if not set in m_aTableProperties
        if ( !m_aTableProperties->getValue( TablePropertyMap::HORI_ORIENT, nHoriOrient ) )
            lcl_extractHoriOrient( rFrameProperties, nHoriOrient );
        m_aTableProperties->Insert( PROP_HORI_ORIENT, uno::Any( sal_Int16(nHoriOrient) ) );
        //fill default value - if not available
        m_aTableProperties->Insert( PROP_HEADER_ROW_COUNT, uno::Any( sal_Int32(0)), false);
        m_aTableProperties->Insert(PROP_WRITING_MODE,
                                   uno::Any(sal_Int16(text::WritingMode2::CONTEXT)),
                                   /*bOverWrite=*/false);

        // if table is only a single row, and row is set as don't split, set the same value for the whole table.
        if( m_aRowProperties.size() == 1 && m_aRowProperties[0] )
        {
            std::optional<PropertyMap::Property> oSplitAllowed = m_aRowProperties[0]->getProperty(PROP_IS_SPLIT_ALLOWED);
            if( oSplitAllowed )
            {
                bool bRowCanSplit = true;
                oSplitAllowed->second >>= bRowCanSplit;
                if( !bRowCanSplit )
                    m_aTableProperties->Insert( PROP_SPLIT, uno::Any(bRowCanSplit) );
            }
        }

        rInfo.aTableProperties = m_aTableProperties->GetPropertyValues();
        rInfo.aTablePropertyIds = m_aTableProperties->GetPropertyIds();

#ifdef DBG_UTIL
        TagLogger::getInstance().startElement("debug.tableprops");
        m_aTableProperties->dumpXml();
        TagLogger::getInstance().endElement();
#endif

    }

    return pTableStyle;
}

CellPropertyValuesSeq_t DomainMapperTableHandler::endTableGetCellProperties(TableInfo & rInfo, std::vector<HorizontallyMergedCell>& rMerges)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("getCellProperties");
#endif

    CellPropertyValuesSeq_t aCellProperties( m_aCellProperties.size() );

    if ( m_aCellProperties.empty() )
    {
        #ifdef DBG_UTIL
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

        // Note that this is intentionally called "cell" and not "column".
        // Don't make the mistake that all cell x's will be in the same column.
        // Merged cells (grid span) in a row will affect the actual column. (fake cells were added to handle gridBefore/After)
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
                        std::optional<PropertyMap::Property> oStyleCellBorder = pStyleProps->getProperty(rBorder);
                        std::optional<PropertyMap::Property> oDirectCellBorder = (*aCellIterator)->getProperty(rBorder);
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
                                std::optional<PropertyMap::Property> oTableBorder = rInfo.pTableBorders->getProperty(rBorder);
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

#ifdef DBG_UTIL
                TagLogger::getInstance().startElement("cell");
                TagLogger::getInstance().attribute("cell", nCell);
                TagLogger::getInstance().attribute("row", nRow);
#endif

                // Do not apply horizontal and vertical borders to a one cell table.
                if (m_aCellProperties.size() <= 1 && aRowOfCellsIterator->size() <= 1)
                {
                    rInfo.pTableBorders->Erase(META_PROP_HORIZONTAL_BORDER);
                    rInfo.pTableBorders->Erase(META_PROP_VERTICAL_BORDER);
                }
                // Do not apply vertical borders to a one column table.
                else if (m_aCellProperties.size() > 1 && aRowOfCellsIterator->size() <= 1)
                {
                    bool isOneCol = true;
                    for (size_t i = nRow; i < m_aCellProperties.size(); i++)
                    {
                        if (m_aCellProperties[i].size() > 1)
                        {
                            isOneCol = false;
                            break;
                        }
                    }
                    if (isOneCol)
                        rInfo.pTableBorders->Erase(META_PROP_VERTICAL_BORDER);
                }
                // Do not apply horizontal borders to a one row table.
                else if (m_aCellProperties.size() == 1 && aRowOfCellsIterator->size() > 1)
                {
                    rInfo.pTableBorders->Erase(META_PROP_HORIZONTAL_BORDER);
                }

                // tdf#129452 Checking if current cell is vertically merged with all the other cells below to the bottom.
                // This must be done in order to apply the bottom border of the table to the first cell in a vertical merge.
                std::optional<PropertyMap::Property> oProp = m_aCellProperties[nRow][nCell]->getProperty(PROP_VERTICAL_MERGE);
                bool bMergedVertically = oProp && oProp->second.get<bool>();  // starting cell
                if ( bMergedVertically )
                {
                    const sal_uInt32 nColumn = m_rDMapper_Impl.getTableManager().findColumn(nRow, nCell);
                    sal_Int32 nLastMergedRow = 0;
                    for (size_t i = nRow + 1; bMergedVertically && i < m_aCellProperties.size(); i++)
                    {
                        const sal_uInt32 nColumnCell = m_rDMapper_Impl.getTableManager().findColumnCell(i, nColumn);
                        if ( m_aCellProperties[i].size() > sal::static_int_cast<std::size_t>(nColumnCell) )
                        {
                            oProp = m_aCellProperties[i][nColumnCell]->getProperty(PROP_VERTICAL_MERGE);
                            bMergedVertically = oProp && !oProp->second.get<bool>(); //continuing cell
                            if ( bMergedVertically )
                                nLastMergedRow = i;
                        }
                        else
                            bMergedVertically = false;
                    }

                    // Only consider the bottom border setting from the last merged cell.
                    // Note: in MSO, left/right apply per-unmerged-row. Can't do that in LO, so just using the top cell's borders should be fine.
                    if ( nRow < nLastMergedRow )
                    {
                        (*aCellIterator)->Erase(PROP_BOTTOM_BORDER);
                        const sal_uInt32 nColumnCell = m_rDMapper_Impl.getTableManager().findColumnCell(nLastMergedRow, nColumn);
                        lcl_mergeBorder( PROP_BOTTOM_BORDER, m_aCellProperties[nLastMergedRow][nColumnCell], *aCellIterator );
                    }
                }

                const sal_uInt32 nFirstCell = m_rDMapper_Impl.getTableManager().getGridBefore(nRow);
                const sal_uInt32 nLastCell = m_aCellProperties[nRow].size() - m_rDMapper_Impl.getTableManager().getGridAfter(nRow) - 1;
                lcl_computeCellBorders( rInfo.pTableBorders, *aCellIterator, nCell, nFirstCell, nLastCell, nRow, bIsEndRow, bMergedVertically );

                //now set the default left+right border distance TODO: there's an sprm containing the default distance!
                aCellIterator->get()->Insert( PROP_LEFT_BORDER_DISTANCE,
                                                 uno::Any(rInfo.nLeftBorderDistance ), false);
                aCellIterator->get()->Insert( PROP_RIGHT_BORDER_DISTANCE,
                                                 uno::Any(rInfo.nRightBorderDistance ), false);
                aCellIterator->get()->Insert( PROP_TOP_BORDER_DISTANCE,
                                                 uno::Any(rInfo.nTopBorderDistance ), false);
                aCellIterator->get()->Insert( PROP_BOTTOM_BORDER_DISTANCE,
                                                 uno::Any(rInfo.nBottomBorderDistance ), false);

                // Horizontal merge is not a UNO property, extract that info here to rMerges, and then remove it from the map.
                const std::optional<PropertyMap::Property> aHorizontalMergeVal = (*aCellIterator)->getProperty(PROP_HORIZONTAL_MERGE);
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
                pSingleCellProperties[nCell] = (*aCellIterator)->GetPropertyValues();
#ifdef DBG_UTIL
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

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif

    return aCellProperties;
}

/// Do all cells in this row have a CellHideMark property?
static bool lcl_hideMarks(PropertyMapVector1& rCellProperties)
{
    for (const PropertyMapPtr & p : rCellProperties)
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

    const RowSequence_t rRowSeq = rTableRanges[nRow];
    if (!rRowSeq.hasElements())
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
        // See SwXText::Impl::ConvertCell(), we need to compare the start of
        // the start and the end of the end. However for our text ranges, only
        // the starts are set, so compareRegionStarts() does what we need.
        bool bRangesAreNotEqual = std::any_of(rRowSeq.begin(), rRowSeq.end(),
            [&xTextRangeCompare](const CellSequence_t& rCellSeq) {
                return xTextRangeCompare->compareRegionStarts(rCellSeq[0], rCellSeq[1]) != 0; });
        if (bRangesAreNotEqual)
            return false;
    }
    catch (const lang::IllegalArgumentException&)
    {
        TOOLS_WARN_EXCEPTION( "writerfilter.dmapper", "compareRegionStarts() failed");
        return false;
    }
    return true;
}

css::uno::Sequence<css::beans::PropertyValues> DomainMapperTableHandler::endTableGetRowProperties()
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("getRowProperties");
#endif

    css::uno::Sequence<css::beans::PropertyValues> aRowProperties( m_aRowProperties.size() );
    auto aRowPropertiesRange = asNonConstRange(aRowProperties);
    sal_Int32 nRow = 0;
    for( const auto& rRow : m_aRowProperties )
    {
#ifdef DBG_UTIL
        TagLogger::getInstance().startElement("rowProps.row");
#endif
        if (rRow)
        {
            //set default to 'break across pages"
            rRow->Insert( PROP_IS_SPLIT_ALLOWED, uno::Any(true ), false );
            // tblHeader is only our property, remove before the property map hits UNO
            rRow->Erase(PROP_TBL_HEADER);

            if (lcl_hideMarks(m_aCellProperties[nRow]) && lcl_emptyRow(m_aTableRanges, nRow))
            {
                // We have CellHideMark on all cells, and also all cells are empty:
                // Force the row height to be exactly as specified, and not just as the minimum suggestion.
                rRow->Insert(PROP_SIZE_TYPE, uno::Any(text::SizeType::FIX));
            }

            aRowPropertiesRange[nRow] = rRow->GetPropertyValues();
#ifdef DBG_UTIL
            rRow->dumpXml();
            lcl_DumpPropertyValues(aRowProperties[nRow]);
#endif
        }
        ++nRow;
#ifdef DBG_UTIL
        TagLogger::getInstance().endElement();
#endif
    }

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif

    return aRowProperties;
}

static bool isAbsent(const std::vector<beans::PropertyValue>& propvals, const OUString& name)
{
    return std::find_if(propvals.begin(), propvals.end(),
                        [&name](const beans::PropertyValue& propval)
                        { return propval.Name == name; })
           == propvals.end();
}

// table style has got bigger precedence than docDefault style,
// but lower precedence than the paragraph styles and direct paragraph formatting
void DomainMapperTableHandler::ApplyParagraphPropertiesFromTableStyle(TableParagraph rParaProp, std::vector< PropertyIds > aAllTableParaProperties, const css::beans::PropertyValues rCellProperties)
{
    // Setting paragraph or character properties using setPropertyValue may have unwanted
    // side effects; e.g., setting a paragraph's font size can reset font size in a runs
    // of the paragraph, which have own formatting, which should have highest precedence.
    // Thus we have to collect property values, construct an autostyle, and assign it to
    // the paragraph, to avoid such side effects.

    // 1. Collect all the table-style-defined properties, that aren't overridden by the
    //    paragraph style or direct formatting
    std::vector<beans::PropertyValue> aProps;
    std::optional<OUString> oParagraphText;

    for( auto eId : aAllTableParaProperties )
    {
        // apply paragraph and character properties of the table style on table paragraphs
        // if there is no direct paragraph formatting
        bool bSetDirectlyInParaLevel = rParaProp.m_pPropertyMap->isSet(eId);
        if ( !bSetDirectlyInParaLevel || isCharacterProperty(eId) )
        {
            if ( (eId == PROP_PARA_LEFT_MARGIN || eId == PROP_PARA_FIRST_LINE_INDENT) &&
                    rParaProp.m_pPropertyMap->isSet(PROP_NUMBERING_RULES) )
            {
                // indentation of direct numbering has bigger precedence, than table style
                continue;
            }

            PropertyIds eMappedId = eId;
            switch (eId)
            {
                case PROP_PARA_TOP_BORDER:
                    eMappedId = PROP_TOP_BORDER;
                    break;
                case PROP_PARA_TOP_BORDER_DISTANCE:
                    eMappedId = PROP_TOP_BORDER_DISTANCE;
                    break;
                case PROP_PARA_BORDER_TOP_COMPLEX_COLOR:
                    eMappedId = PROP_BORDER_TOP_COMPLEX_COLOR;
                    break;
                case PROP_PARA_LEFT_BORDER:
                    eMappedId = PROP_LEFT_BORDER;
                    break;
                case PROP_PARA_LEFT_BORDER_DISTANCE:
                    eMappedId = PROP_LEFT_BORDER_DISTANCE;
                    break;
                case PROP_PARA_BORDER_LEFT_COMPLEX_COLOR:
                    eMappedId = PROP_BORDER_LEFT_COMPLEX_COLOR;
                    break;
                case PROP_PARA_BOTTOM_BORDER:
                    eMappedId = PROP_BOTTOM_BORDER;
                    break;
                case PROP_PARA_BOTTOM_BORDER_DISTANCE:
                    eMappedId = PROP_BOTTOM_BORDER_DISTANCE;
                    break;
                case PROP_PARA_BORDER_BOTTOM_COMPLEX_COLOR:
                    eMappedId = PROP_BORDER_BOTTOM_COMPLEX_COLOR;
                    break;
                case PROP_PARA_RIGHT_BORDER:
                    eMappedId = PROP_RIGHT_BORDER;
                    break;
                case PROP_PARA_RIGHT_BORDER_DISTANCE:
                    eMappedId = PROP_RIGHT_BORDER_DISTANCE;
                    break;
                case PROP_PARA_BORDER_RIGHT_COMPLEX_COLOR:
                    eMappedId = PROP_BORDER_RIGHT_COMPLEX_COLOR;
                    break;
                default:
                    break;
            }

            OUString sPropertyName = getPropertyName(eId);
            OUString sMappedPropertyName = getPropertyName(eMappedId);

            auto pCellProp = std::find_if(rCellProperties.begin(), rCellProperties.end(),
                [&](const beans::PropertyValue& rProp) { return rProp.Name == sPropertyName; });
            // this cell applies the table style property
            if (pCellProp != rCellProperties.end())
            {
                if (bSetDirectlyInParaLevel) // it is a character property set directly in the paragraph
                {
                    if (!oParagraphText) // do it only once
                    {
                        uno::Reference<text::XParagraphCursor> xParagraph(
                            rParaProp.m_rEndParagraph->getText()->createTextCursorByRange(rParaProp.m_rEndParagraph), uno::UNO_QUERY_THROW );
                        // select paragraph
                        xParagraph->gotoStartOfParagraph( true );
                        oParagraphText = xParagraph->getString();
                    }
                    // don't overwrite empty paragraph with table style, if it has a direct paragraph formatting
                    if (oParagraphText->isEmpty())
                        continue;
                }
                // handle paragraph background color defined in CellColorHandler
                if (eId == PROP_FILL_COLOR)
                {
                    auto pFillStyleProp = std::find_if(rCellProperties.begin(), rCellProperties.end(),
                        [](const beans::PropertyValue& rProp) { return rProp.Name == "FillStyle"; });
                    if ( pFillStyleProp == rCellProperties.end() ||
                         pFillStyleProp->Value != uno::Any(drawing::FillStyle_SOLID) )
                    {
                        // FillStyle_NONE, skip table style usage for paragraph background color
                        continue;
                    }
                }
                OUString sParaStyleName;
                rParaProp.m_rPropertySet->getPropertyValue(u"ParaStyleName"_ustr) >>= sParaStyleName;
                bool bDocDefault;
                uno::Any aParaStyle = m_rDMapper_Impl.GetPropertyFromStyleSheet(eId,
                        m_rDMapper_Impl.GetStyleSheetTable()->FindStyleSheetByConvertedStyleName(sParaStyleName),
                        true, true, &bDocDefault);
                // A very strange compatibility rule says that the DEFAULT style's specified fontsize of 11 or 12
                // or a specified left justify will always be overridden by the table-style.
                // Normally this rule is applied, so always do this unless a compatSetting indicates otherwise.
                bool bCompatOverride = false;
                if ( (eId == PROP_CHAR_HEIGHT || eId == PROP_PARA_ADJUST) && sParaStyleName == m_rDMapper_Impl.GetDefaultParaStyleName() )
                {
                    if ( eId == PROP_CHAR_HEIGHT )
                        bCompatOverride = aParaStyle == uno::Any(double(11)) || aParaStyle == uno::Any(double(12));
                    else if ( eId == PROP_PARA_ADJUST )
                    {
                        style::ParagraphAdjust eAdjust(style::ParagraphAdjust_CENTER);
                        aParaStyle >>= eAdjust;
                        bCompatOverride = eAdjust == style::ParagraphAdjust_LEFT;
                    }

                    // The wording is confusing here. Normally, the paragraph style DOES override the table-style.
                    // But for these two special situations, do not override the table-style. So the default is false.
                    // If false, then "CompatOverride" the normal behaviour, and apply the table-style's value.
                    bCompatOverride &= !m_rDMapper_Impl.GetSettingsTable()->
                        GetCompatSettingHasAndValue(u"overrideTableStyleFontSizeAndJustification").second;
                }

                // use table style when no paragraph style setting or a docDefault value is applied instead of it
                if (!aParaStyle.hasValue() || bDocDefault || bCompatOverride) try
                {
                    // apply style setting when the paragraph doesn't modify it
                    aProps.push_back(comphelper::makePropertyValue(sMappedPropertyName, pCellProp->Value));
                    if (eId == PROP_FILL_COLOR)
                    {
                        // we need this for complete import of table-style based paragraph background color
                        aProps.push_back(comphelper::makePropertyValue(u"FillStyle"_ustr,  uno::Any(drawing::FillStyle_SOLID)));
                    }
                }
                catch ( const uno::Exception & )
                {
                    TOOLS_INFO_EXCEPTION("writerfilter.dmapper", "Exception during table style correction");
                }
            }
        }
    }

    if (!aProps.empty())
    {
        // 2. Get all properties directly defined in the paragraph
        uno::Reference<beans::XPropertySetInfo> xPropSetInfo(
            rParaProp.m_rPropertySet->getPropertySetInfo(), uno::UNO_SET_THROW);
        auto props = xPropSetInfo->getProperties();
        uno::Sequence<OUString> propNames(props.getLength());
        std::transform(props.begin(), props.end(), propNames.getArray(),
                       [](const beans::Property& prop) { return prop.Name; });
        uno::Reference<beans::XTolerantMultiPropertySet> xTolPara(rParaProp.m_rPropertySet,
                                                                  uno::UNO_QUERY_THROW);
        // getDirectPropertyValuesTolerant requires a sorted sequence.
        // Let's hope XPropertySetInfo::getProperties returns a sorted sequence.
        for (auto& val : xTolPara->getDirectPropertyValuesTolerant(propNames))
        {
            // 3. Add them to aProps, unless such properties are already there
            //    (which means, that 'val' comes from docDefault)
            if (val.Result == beans::TolerantPropertySetResultType::SUCCESS
                && val.State == beans::PropertyState_DIRECT_VALUE
                && isAbsent(aProps, val.Name))
            {
                aProps.push_back(comphelper::makePropertyValue(val.Name, val.Value));
            }
        }

        // 4. Create an autostyle, and assign it to the paragraph. The hidden ParaAutoStyleDef
        //    property is handled in SwXTextCursor::setPropertyValue.
        uno::Reference<beans::XPropertySet> xCursorProps(
            rParaProp.m_rEndParagraph->getText()->createTextCursorByRange(
                rParaProp.m_rEndParagraph),
            uno::UNO_QUERY_THROW);
        xCursorProps->setPropertyValue(u"ParaAutoStyleDef"_ustr,
                                       uno::Any(comphelper::containerToSequence(aProps)));
    }
}

// convert formula range identifier ABOVE, BELOW, LEFT and RIGHT
static void lcl_convertFormulaRanges(const uno::Reference<text::XTextTable> & xTable)
{
    uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY_THROW);
    uno::Reference<container::XIndexAccess> xTableRows(xTable->getRows(), uno::UNO_QUERY_THROW);
    sal_Int32 nRows = xTableRows->getCount();
    for (sal_Int32 nRow = 0; nRow < nRows; ++nRow)
    {
        for (sal_Int16 nCol = 0; nCol < MAXTABLECELLS; ++nCol)
        {
            try
            {
                uno::Reference<beans::XPropertySet> xCellProperties(xCellRange->getCellByPosition(nCol, nRow), uno::UNO_QUERY_THROW);
                uno::Sequence<beans::PropertyValue> aCellGrabBag;
                xCellProperties->getPropertyValue(u"CellInteropGrabBag"_ustr) >>= aCellGrabBag;
                OUString sFormula;
                bool bReplace = false;
                for (const auto& rProp : aCellGrabBag)
                {
                    if ( rProp.Name == "CellFormulaConverted" )
                    {
                        rProp.Value >>= sFormula;
                        struct RangeDirection
                        {
                            OUString m_sName;
                            sal_Int16 m_nCol;
                            sal_Int16 m_nRow;
                        };
                        static const RangeDirection pDirections[] =
                        {
                            { u" LEFT "_ustr, -1, 0},
                            { u" RIGHT "_ustr, 1, 0},
                            { u" ABOVE "_ustr, 0, -1},
                            { u" BELOW "_ustr, 0, 1 }
                        };
                        for (const RangeDirection& rRange : pDirections)
                        {
                            if ( sFormula.indexOf(rRange.m_sName) > -1 )
                            {
                                // range starts at the first cell above/below/left/right, but ends at the
                                // table border or at the first non-value cell after a value cell
                                bool bFoundFirst = false;
                                OUString sNextCell;
                                OUString sLastCell;
                                OUString sLastValueCell;
                                // walk through the cells of the range
                                try
                                {
                                    sal_Int32 nCell = 0;
                                    while (++nCell)
                                    {
                                        uno::Reference<beans::XPropertySet> xCell(
                                                xCellRange->getCellByPosition(nCol + nCell * rRange.m_nCol, nRow + nCell * rRange.m_nRow),
                                                uno::UNO_QUERY_THROW);
                                        // empty cell or cell with text content is end of the range
                                        uno::Reference<text::XText> xText(xCell, uno::UNO_QUERY_THROW);
                                        sLastCell = xCell->getPropertyValue(u"CellName"_ustr).get<OUString>();
                                        if (sNextCell.isEmpty())
                                            sNextCell = sLastCell;

                                        // accept numbers with comma and percent
                                        OUString sCellText = xText->getString().replace(',', '.');
                                        if (sCellText.endsWith("%"))
                                            sCellText = sCellText.copy(0, sCellText.getLength()-1);

                                        rtl_math_ConversionStatus eConversionStatus;
                                        sal_Int32 nParsedEnd;
                                        rtl::math::stringToDouble(sCellText, '.', ',', &eConversionStatus, &nParsedEnd);
                                        if ( eConversionStatus != rtl_math_ConversionStatus_Ok || nParsedEnd == 0 )
                                        {
                                            if ( !bFoundFirst )
                                            {
                                                // still search value cells
                                                continue;
                                            }
                                            else
                                            {
                                                // end of range
                                                break;
                                            }
                                        }
                                        sLastValueCell = sLastCell;
                                        bFoundFirst = true;
                                    }
                                }
                                catch ( const lang::IndexOutOfBoundsException & )
                                {
                                }

                                if ( !sNextCell.isEmpty() )
                                {
                                    OUString sRange = "<" + sNextCell + ":" +
                                            ( sLastValueCell.isEmpty() ? sLastCell : sLastValueCell ) + ">";
                                    sFormula = sFormula.replaceAll(rRange.m_sName, sRange);
                                    bReplace = true;
                                }
                            }
                        }

                        // update formula field
                        if (bReplace)
                        {
                            uno::Reference<text::XText> xCell(xCellRange->getCellByPosition(nCol, nRow), uno::UNO_QUERY);
                            uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xCell, uno::UNO_QUERY);
                            uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
                            uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(), uno::UNO_QUERY);
                            uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
                            while ( xRunEnum->hasMoreElements() )
                            {
                                uno::Reference<text::XTextRange> xRun(xRunEnum->nextElement(), uno::UNO_QUERY);
                                uno::Reference< beans::XPropertySet > xRunProperties( xRun, uno::UNO_QUERY_THROW );
                                if ( xRunProperties->getPropertyValue(u"TextPortionType"_ustr) == uno::Any(u"TextField"_ustr) )
                                {
                                    uno::Reference<text::XTextField> const xField(xRunProperties->getPropertyValue(u"TextField"_ustr).get<uno::Reference<text::XTextField>>());
                                    uno::Reference< beans::XPropertySet > xFieldProperties( xField, uno::UNO_QUERY_THROW );
                                    // cell can contain multiple text fields, but only one is handled now (~formula cell)
                                    if ( rProp.Value != xFieldProperties->getPropertyValue(u"Content"_ustr) )
                                        continue;
                                    xFieldProperties->setPropertyValue(u"Content"_ustr, uno::Any(sFormula));
                                    // update grab bag
                                    auto aGrabBag = comphelper::sequenceToContainer< std::vector<beans::PropertyValue> >(aCellGrabBag);
                                    beans::PropertyValue aValue;
                                    aValue.Name = "CellFormulaConverted";
                                    aValue.Value <<= sFormula;
                                    aGrabBag.push_back(aValue);
                                    xCellProperties->setPropertyValue(u"CellInteropGrabBag"_ustr, uno::Any(comphelper::containerToSequence(aGrabBag)));
                                }
                            }
                        }
                    }
                }
            }
            catch ( const lang::IndexOutOfBoundsException & )
            {
                // jump to next table row
                break;
            }
        }
    }
}

void DomainMapperTableHandler::endTable(unsigned int nestedTableLevel)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("tablehandler.endTable");
#endif

    // If we want to make this table a floating one.
    std::vector<beans::PropertyValue> aFrameProperties = comphelper::sequenceToContainer<std::vector<beans::PropertyValue> >
            (m_rDMapper_Impl.getTableManager().getCurrentTablePosition());
    TableInfo aTableInfo;
    aTableInfo.nNestLevel = nestedTableLevel;

    // non-floating tables need floating in footnotes and endnotes, because
    // Writer core cannot handle (i.e. save in ODT, copy, edit etc.) them otherwise
    bool bConvertToFloating = aFrameProperties.empty() &&
            nestedTableLevel <= 1 &&
            m_rDMapper_Impl.IsInFootOrEndnote();
    bool bFloating = !aFrameProperties.empty() || bConvertToFloating;

    aTableInfo.pTableStyle = endTableGetTableStyle(aTableInfo, aFrameProperties, bConvertToFloating);
    //  expands to uno::Sequence< Sequence< beans::PropertyValues > >

    std::vector<HorizontallyMergedCell> aMerges;
    CellPropertyValuesSeq_t aCellProperties = endTableGetCellProperties(aTableInfo, aMerges);

    css::uno::Sequence<css::beans::PropertyValues> aRowProperties = endTableGetRowProperties();

#ifdef DBG_UTIL
    lcl_DumpPropertyValueSeq(aRowProperties);
#endif

    if (!m_aTableRanges.empty())
    {
        uno::Reference<text::XTextRange> xStart;
        uno::Reference<text::XTextRange> xEnd;

        // fill empty frame properties to create an invisible frame around the table:
        // hide frame borders and zero inner and outer frame margins
        if (bConvertToFloating)
            DomainMapper_Impl::fillEmptyFrameProperties(aFrameProperties, true);

        // OOXML table style may contain paragraph properties, apply these on cell paragraphs
        if ( m_aTableRanges[0].hasElements() && m_aTableRanges[0][0].hasElements() )
        {
            // collect all paragraph properties used in table styles
            PropertyMapPtr pAllTableProps( new PropertyMap );
            pAllTableProps->InsertProps(aTableInfo.pTableDefaults);
            if ( aTableInfo.pTableStyle )
                pAllTableProps->InsertProps(aTableInfo.pTableStyle->GetProperties( CNF_ALL ));
            for (const auto& eId : pAllTableProps->GetPropertyIds())
            {
                if ( !isParagraphProperty(eId) && !isCharacterProperty(eId) )
                    pAllTableProps->Erase(eId);
            }
            std::vector< PropertyIds > aAllTableParaProperties = pAllTableProps->GetPropertyIds();

            if ( !aAllTableParaProperties.empty() )
            {
                TableParagraphVectorPtr pTableParagraphs = m_rDMapper_Impl.getTableManager().getCurrentParagraphs();
                for (size_t nRow = 0; nRow < m_aTableRanges.size(); ++nRow)
                {
                    // Note that this is "cell" since you must not treat it as "column".
                    for (size_t nCell = 0; nCell < m_aTableRanges[nRow].size(); ++nCell)
                    {
                        auto rStartPara = m_aTableRanges[nRow][nCell][0];
                        if (!rStartPara.is())
                            continue;
                        auto rEndPara = m_aTableRanges[nRow][nCell][1];
                        uno::Reference<text::XTextRangeCompare> xTextRangeCompare(rStartPara->getText(), uno::UNO_QUERY);
                        bool bApply = false;
                        // search paragraphs of the cell
                        std::vector<TableParagraph>::iterator aIt = pTableParagraphs->begin();
                        while ( aIt != pTableParagraphs->end() ) try
                        {
                            if (!bApply && xTextRangeCompare->compareRegionStarts(rStartPara, aIt->m_rStartParagraph) == 0)
                                bApply = true;
                            if (bApply)
                            {
                                bool bEndOfApply = (xTextRangeCompare->compareRegionEnds(rEndPara, aIt->m_rEndParagraph) == 0);
                                // tdf#153891 handle missing cell properties (exception in style handling?)
                                if ( nCell < sal::static_int_cast<std::size_t>(aCellProperties[nRow].getLength()) )
                                    ApplyParagraphPropertiesFromTableStyle(*aIt, aAllTableParaProperties, aCellProperties[nRow][nCell]);
                                // erase processed paragraph from list of pending paragraphs
                                aIt = pTableParagraphs->erase(aIt);
                                if (bEndOfApply)
                                    break;
                            }
                            else
                                ++aIt;
                        }
                        catch( const lang::IllegalArgumentException & )
                        {
                            // skip compareRegion with nested tables
                            ++aIt;
                        }
                    }
                }
            }
        }

        // Additional checks: if we can do this.
        if (bFloating && m_aTableRanges[0].hasElements() && m_aTableRanges[0][0].hasElements())
        {
            xStart = m_aTableRanges[0][0][0];
            uno::Sequence< uno::Sequence< uno::Reference<text::XTextRange> > >& rLastRow = m_aTableRanges[m_aTableRanges.size() - 1];
            if (rLastRow.hasElements())
            {
                const uno::Sequence< uno::Reference<text::XTextRange> >& rLastCell = rLastRow[rLastRow.getLength() - 1];
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
                        static const std::vector<std::u16string_view> aBorderNames
                            = { u"TopBorder", u"LeftBorder", u"BottomBorder", u"RightBorder" };

                        // Perform horizontal merges in reverse order, so the fact that merging changes the position of cells won't cause a problem for us.
                        for (std::vector<HorizontallyMergedCell>::reverse_iterator it = aMerges.rbegin(); it != aMerges.rend(); ++it)
                        {
                            uno::Reference<table::XCellRange> xCellRange(xTable, uno::UNO_QUERY_THROW);
                            uno::Reference<beans::XPropertySet> xFirstCell(
                                xCellRange->getCellByPosition(it->m_nFirstCol, it->m_nFirstRow),
                                uno::UNO_QUERY_THROW);
                            OUString aFirst
                                = xFirstCell->getPropertyValue(u"CellName"_ustr).get<OUString>();
                            // tdf#105852: Only try to merge if m_nLastCol is set (i.e. there were some merge continuation cells)
                            if (it->m_nLastCol != -1)
                            {
                                // Save border properties of the first cell
                                // before merge.
                                table::BorderLine2 aBorderValues[4];
                                for (size_t i = 0; i < aBorderNames.size(); ++i)
                                    xFirstCell->getPropertyValue(OUString(aBorderNames[i]))
                                        >>= aBorderValues[i];

                                uno::Reference<beans::XPropertySet> xLastCell(
                                    xCellRange->getCellByPosition(it->m_nLastCol, it->m_nLastRow),
                                    uno::UNO_QUERY_THROW);
                                OUString aLast
                                    = xLastCell->getPropertyValue(u"CellName"_ustr).get<OUString>();

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
                                            OUString(aBorderNames[i]), uno::Any(aBorderValues[i]));
                                }
                            }
                        }
                    }

                    // convert special range IDs ABOVE, BELOW, LEFT and RIGHT
                    lcl_convertFormulaRanges(xTable);
                }
            }
        }
        catch ( const lang::IllegalArgumentException & )
        {
            TOOLS_INFO_EXCEPTION("writerfilter.dmapper", "Conversion to table error");
#ifdef DBG_UTIL
            TagLogger::getInstance().chars(std::string("failed to import table!"));
#endif
        }
        catch ( const uno::Exception & )
        {
            TOOLS_INFO_EXCEPTION("writerfilter.dmapper", "Exception during table creation");
        }

        // If we have a table with a start and an end position, we should make it a floating one.
        if (xTable.is() && xStart.is() && xEnd.is())
        {
            uno::Reference<beans::XPropertySet> xTableProperties(xTable, uno::UNO_QUERY);
            bool bIsRelative = false;
            xTableProperties->getPropertyValue(u"IsWidthRelative"_ustr) >>= bIsRelative;
            if (!bIsRelative)
            {
                beans::PropertyValue aValue;
                aValue.Name = "Width";
                aValue.Value = xTableProperties->getPropertyValue(u"Width"_ustr);
                aFrameProperties.push_back(aValue);
            }
            else
            {
                beans::PropertyValue aValue;
                aValue.Name = "FrameWidthPercent";
                aValue.Value = xTableProperties->getPropertyValue(u"RelativeWidth"_ustr);
                aFrameProperties.push_back(aValue);

                // Applying the relative width to the frame, needs to have the table width to be 100% of the frame width
                xTableProperties->setPropertyValue(u"RelativeWidth"_ustr, uno::Any(sal_Int16(100)));
            }

            // A non-zero left margin would move the table out of the frame, move the frame itself instead.
            xTableProperties->setPropertyValue(u"LeftMargin"_ustr, uno::Any(sal_Int32(0)));

            style::BreakType eBreakType{};
            xTableProperties->getPropertyValue(u"BreakType"_ustr) >>= eBreakType;
            if (eBreakType != style::BreakType_NONE)
            {
                // A break before the table was requested. Reset that break here, since the table
                // will be at the start of the fly frame, not in the body frame.
                xTableProperties->setPropertyValue(u"BreakType"_ustr, uno::Any(style::BreakType_NONE));
            }

            if (nestedTableLevel >= 2 || m_rDMapper_Impl.IsInHeaderFooter())
            {
                // Floating tables inside a table always stay inside the cell.
                // Also extend the header/footer area if needed, so an in-header floating table
                // typically doesn't overlap with body test.
                bool bIsFollowingTextFlow = true;

                sal_Int16 nVertOrientRelation{};
                auto it = std::find_if(aFrameProperties.begin(), aFrameProperties.end(),
                                       [](const beans::PropertyValue& rPropertyValue) -> bool
                                       { return rPropertyValue.Name == "VertOrientRelation"; });
                if (it != aFrameProperties.end())
                {
                    it->Value >>= nVertOrientRelation;
                    if (nVertOrientRelation == text::RelOrientation::PAGE_FRAME)
                    {
                        // If vertical relation is page, follow-text-flow is not useful and causes
                        // unwanted wrap of body text around in-header floating table, so avoid it.
                        bIsFollowingTextFlow = false;
                    }
                }

                aFrameProperties.push_back(
                    comphelper::makePropertyValue(u"IsFollowingTextFlow"_ustr, bIsFollowingTextFlow));
            }

            // A text frame created for floating tables is always allowed to split.
            aFrameProperties.push_back(comphelper::makePropertyValue(u"IsSplitAllowed"_ustr, true));

            sal_Int32 nTableWidth = 0;
            m_aTableProperties->getValue(TablePropertyMap::TABLE_WIDTH, nTableWidth);
            sal_Int32 nTableWidthType = text::SizeType::FIX;
            m_aTableProperties->getValue(TablePropertyMap::TABLE_WIDTH_TYPE, nTableWidthType);
            // m_xText points to the body text, get the current xText from m_rDMapper_Impl, in case e.g. we would be in a header.
            uno::Reference<text::XTextAppendAndConvert> xTextAppendAndConvert(m_rDMapper_Impl.GetTopTextAppend(), uno::UNO_QUERY);
            uno::Reference<beans::XPropertySet> xFrameAnchor;

            // Writer layout has problems with redlines on floating table rows in footnotes, avoid
            // them.
            bool bInFootnote = m_rDMapper_Impl.IsInFootOrEndnote();
            bool bRecordChanges = m_rDMapper_Impl.GetSettingsTable()->GetRecordChanges();
            if (xTextAppendAndConvert.is() && !(bInFootnote && bRecordChanges))
            {
                std::deque<css::uno::Any> aFramedRedlines = m_rDMapper_Impl.m_aStoredRedlines[StoredRedlines::FRAME];
                std::vector<sal_Int32> redPos, redLen;
                std::vector<OUString> redCell;
                std::vector<OUString> redTable;
                BeforeConvertToTextFrame(aFramedRedlines, redPos, redLen, redCell, redTable);

                uno::Reference<text::XTextContent> xContent = xTextAppendAndConvert->convertToTextFrame(xStart, xEnd, comphelper::containerToSequence(aFrameProperties));
                xFrameAnchor.set(xContent->getAnchor(), uno::UNO_QUERY);

                bool bConvertToFloatingInFootnote = false;
                if (xContent.is() && xContent->getAnchor().is())
                {
                    uno::Reference<lang::XServiceInfo> xText(xContent->getAnchor()->getText(), uno::UNO_QUERY);
                    if (xText.is())
                    {
                        bConvertToFloatingInFootnote = xText->supportsService(u"com.sun.star.text.Footnote"_ustr);
                    }
                }

                // paragraph of the anchoring point of the floating table needs zero top and bottom
                // margins, if the table was a not floating table in the footnote, otherwise
                // docDefault margins could result bigger vertical spaces around the table
                if ( bConvertToFloatingInFootnote && xContent.is() )
                {
                    uno::Reference<beans::XPropertySet> xParagraph(
                                    xContent->getAnchor(), uno::UNO_QUERY);
                    if ( xParagraph.is() )
                    {
                        xParagraph->setPropertyValue(u"ParaTopMargin"_ustr,
                                    uno::Any(static_cast<sal_Int32>(0)));
                        xParagraph->setPropertyValue(u"ParaBottomMargin"_ustr,
                                    uno::Any(static_cast<sal_Int32>(0)));
                    }
                }

                if (xContent.is())
                {
                    // By the time the frame is created, the anchor's paragraph marker character
                    // properties are already imported. Check if we need to disable "vanish", that
                    // would lead to a hidden floating table in Writer, but it does not in Word.
                    uno::Reference<beans::XPropertySet> xParagraph(xContent->getAnchor(),
                                                                   uno::UNO_QUERY);
                    if (xParagraph.is())
                    {
                        bool bCharHidden{};
                        xParagraph->getPropertyValue(u"CharHidden"_ustr) >>= bCharHidden;
                        if (bCharHidden)
                        {
                            xParagraph->setPropertyValue(u"CharHidden"_ustr, uno::Any(false));
                        }
                    }
                }

                AfterConvertToTextFrame(m_rDMapper_Impl, aFramedRedlines, redPos, redLen, redCell, redTable);
            }

            if (xFrameAnchor.is() && eBreakType != style::BreakType_NONE)
            {
                // A break before the table was requested. Restore that on the anchor.
                xFrameAnchor->setPropertyValue(u"BreakType"_ustr, uno::Any(eBreakType));
            }
        }
    }

    m_aTableProperties.clear();
    m_aCellProperties.clear();
    m_aRowProperties.clear();

#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapperTableHandler::startRow(const TablePropertyMapPtr& pProps)
{
    m_aRowProperties.push_back( pProps.get() );
    m_aCellProperties.emplace_back( );

#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("table.row");
    if (pProps != nullptr)
        pProps->dumpXml();
#endif

    m_aRowRanges.clear();
}

void DomainMapperTableHandler::endRow()
{
    m_aTableRanges.push_back(comphelper::containerToSequence(m_aRowRanges));
#ifdef DBG_UTIL
    TagLogger::getInstance().endElement();
#endif
}

void DomainMapperTableHandler::startCell(const css::uno::Reference< css::text::XTextRange > & start,
                                         const TablePropertyMapPtr& pProps )
{
    sal_uInt32 nRow = m_aRowProperties.size();
    if ( pProps )
        m_aCellProperties[nRow - 1].push_back( pProps.get() );
    else
    {
        // Adding an empty cell properties map to be able to get
        // the table defaults properties
        TablePropertyMapPtr pEmptyProps( new TablePropertyMap( ) );
        m_aCellProperties[nRow - 1].push_back( pEmptyProps.get() );
    }

#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("table.cell");
    TagLogger::getInstance().startElement("table.cell.start");
    TagLogger::getInstance().chars(XTextRangeToString(start));
    TagLogger::getInstance().endElement();
    if (pProps)
        pProps->printProperties();
#endif

    //add a new 'row' of properties
    m_aCellRange.clear();
    uno::Reference<text::XTextRange> xStart;
    if (start)
        xStart = start->getStart();
    m_aCellRange.push_back(xStart);
}

void DomainMapperTableHandler::endCell(const css::uno::Reference< css::text::XTextRange > & end)
{
#ifdef DBG_UTIL
    TagLogger::getInstance().startElement("table.cell.end");
    TagLogger::getInstance().chars(XTextRangeToString(end));
    TagLogger::getInstance().endElement();
    TagLogger::getInstance().endElement();
#endif

    uno::Reference<text::XTextRange> xEnd;
    if (end)
        xEnd = end->getEnd();
    m_aCellRange.push_back(xEnd);
    m_aRowRanges.push_back(comphelper::containerToSequence(m_aCellRange));
}

DomainMapper_Impl& DomainMapperTableHandler::getDomainMapperImpl()
{
    return m_rDMapper_Impl;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
