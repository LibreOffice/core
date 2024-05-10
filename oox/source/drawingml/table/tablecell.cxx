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

#include <drawingml/table/tablecell.hxx>
#include <drawingml/table/tableproperties.hxx>
#include <basegfx/color/bcolor.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <drawingml/textbody.hxx>
#include <drawingml/textcharacterproperties.hxx>
#include <drawingml/textparagraph.hxx>
#include <oox/drawingml/theme.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/tokenmap.hxx>
#include <tools/color.hxx>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::com::sun::star::table::BorderLine2;

namespace oox::drawingml::table {

TableCell::TableCell()
: mpTextBody( std::make_shared<TextBody>() )
, mnRowSpan ( 1 )
, mnGridSpan( 1 )
, mbhMerge( false )
, mbvMerge( false )
, mnMarL( 91440 )
, mnMarR( 91440 )
, mnMarT( 45720 )
, mnMarB( 45720 )
, mnVertToken( XML_horz )
, mnAnchorToken( XML_t )
, mbAnchorCtr( false )
, mnHorzOverflowToken( XML_clip )
{
}

static void applyLineAttributes(const ::oox::core::XmlFilterBase& rFilterBase,
                                Reference<XPropertySet> const& rxPropSet,
                                oox::drawingml::table::TableStyle const& rTableStyle,
                                oox::drawingml::LineProperties const& rLineProperties,
                                sal_Int32 nPropId)
{
    BorderLine2 aBorderLine;
    TableStyle& rTable(const_cast<TableStyle&>(rTableStyle));
    if (!rTable.getStyleId().isEmpty())
    {
        Color aColor = rLineProperties.maLineFill.getBestSolidColor();
        aBorderLine.Color = sal_Int32(aColor.getColor(rFilterBase.getGraphicHelper()));
        aBorderLine.OuterLineWidth = static_cast<sal_Int16>(GetCoordinate(rLineProperties.moLineWidth.value_or(0)) / 4);
        aBorderLine.InnerLineWidth = static_cast<sal_Int16>(GetCoordinate(rLineProperties.moLineWidth.value_or(0)) / 4);
        aBorderLine.LineWidth = static_cast<sal_Int16>(GetCoordinate(rLineProperties.moLineWidth.value_or(0)) / 2);
        aBorderLine.LineDistance = 0;
    }
    else
    {
        if (rLineProperties.maLineFill.moFillType.has_value())
        {
            if (rLineProperties.maLineFill.moFillType.value() != XML_noFill)
            {
                Color aColor = rLineProperties.maLineFill.getBestSolidColor();
                aBorderLine.Color = sal_Int32(aColor.getColor(rFilterBase.getGraphicHelper()));
            }
            else
                aBorderLine.Color = sal_Int32(COL_AUTO);
        }
        else
            aBorderLine.Color = sal_Int32(COL_BLACK);

        aBorderLine.OuterLineWidth = static_cast<sal_Int16>(GetCoordinate(rLineProperties.moLineWidth.value_or(12700)) / 4);
        aBorderLine.InnerLineWidth = static_cast<sal_Int16>(GetCoordinate(rLineProperties.moLineWidth.value_or(12700)) / 4);
        aBorderLine.LineWidth = static_cast<sal_Int16>(GetCoordinate(rLineProperties.moLineWidth.value_or(12700)) / 2);
        aBorderLine.LineDistance = 0;
    }

    if ( rLineProperties.moPresetDash.has_value() )
    {
        switch ( rLineProperties.moPresetDash.value() )
        {
        case XML_dot:
        case XML_sysDot:
            aBorderLine.LineStyle = ::table::BorderLineStyle::DOTTED;
            break;
        case XML_dash:
        case XML_lgDash:
        case XML_sysDash:
            aBorderLine.LineStyle = ::table::BorderLineStyle::DASHED;
            break;
        case XML_dashDot:
        case XML_lgDashDot:
        case XML_sysDashDot:
            aBorderLine.LineStyle = ::table::BorderLineStyle::DASH_DOT;
            break;
        case XML_lgDashDotDot:
        case XML_sysDashDotDot:
            aBorderLine.LineStyle = ::table::BorderLineStyle::DASH_DOT_DOT;
            break;
        case XML_solid:
            aBorderLine.LineStyle = ::table::BorderLineStyle::SOLID;
            break;
        default:
            aBorderLine.LineStyle = ::table::BorderLineStyle::DASHED;
            break;
        }
    }
    else if ( !rLineProperties.maCustomDash.empty() )
    {
        aBorderLine.LineStyle = ::table::BorderLineStyle::DASHED;
    }
    else
    {
        aBorderLine.LineStyle = ::table::BorderLineStyle::NONE;
    }

    PropertySet aPropSet( rxPropSet );
    aPropSet.setProperty( nPropId, aBorderLine );
}

static void applyBorder( const ::oox::core::XmlFilterBase& rFilterBase, TableStylePart& rTableStylePart, sal_Int32 nLineType, oox::drawingml::LineProperties& rLineProperties )
{
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >& rPartLineBorders( rTableStylePart.getLineBorders() );
    ::oox::drawingml::ShapeStyleRef& rLineStyleRef = rTableStylePart.getStyleRefs()[ nLineType ];
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >::const_iterator aIter( rPartLineBorders.find( nLineType ) );
    if ( ( aIter != rPartLineBorders.end() ) && aIter->second )
        rLineProperties.assignUsed( *aIter->second );
    else if (rLineStyleRef.mnThemedIdx != 0)
    {
        if (const Theme* pTheme = rFilterBase.getCurrentTheme())
        {
            rLineProperties.assignUsed( *pTheme->getLineStyle(rLineStyleRef.mnThemedIdx) );
            ::Color nPhClr = rLineStyleRef.maPhClr.getColor( rFilterBase.getGraphicHelper() );
            rLineProperties.maLineFill.maFillColor.setSrgbClr( nPhClr );
        }
    }
}

static void applyTableStylePart( const ::oox::core::XmlFilterBase& rFilterBase,
                          oox::drawingml::FillProperties& rFillProperties,
                          TextCharacterProperties& aTextCharProps,
                          oox::drawingml::LineProperties& rLeftBorder,
                          oox::drawingml::LineProperties& rRightBorder,
                          oox::drawingml::LineProperties& rTopBorder,
                          oox::drawingml::LineProperties& rBottomBorder,
                          oox::drawingml::LineProperties& rInsideHBorder,
                          oox::drawingml::LineProperties& rInsideVBorder,
                          oox::drawingml::LineProperties& rTopLeftToBottomRightBorder,
                          oox::drawingml::LineProperties& rBottomLeftToTopRightBorder,
                          TableStylePart& rTableStylePart,
                          bool bIsWholeTable = false,
                          sal_Int32 nCol = 0,
                          sal_Int32 nMaxCol = 0,
                          sal_Int32 nRow = 0,
                          sal_Int32 nMaxRow = 0)
{
    ::oox::drawingml::FillPropertiesPtr& rPartFillPropertiesPtr( rTableStylePart.getFillProperties() );
    if ( rPartFillPropertiesPtr )
        rFillProperties.assignUsed( *rPartFillPropertiesPtr );
    else
    {
        ::oox::drawingml::ShapeStyleRef& rFillStyleRef = rTableStylePart.getStyleRefs()[ XML_fillRef ];
        const Theme* pTheme = rFilterBase.getCurrentTheme();
        if (pTheme && rFillStyleRef.mnThemedIdx != 0 )
        {
            rFillProperties.assignUsed( *pTheme->getFillStyle( rFillStyleRef.mnThemedIdx ) );
            ::Color nPhClr = rFillStyleRef.maPhClr.getColor( rFilterBase.getGraphicHelper() );
            rFillProperties.maFillColor.setSrgbClr( nPhClr );
        }
    }

    // Left, right, top and bottom side of the whole table should be mean outer frame of the whole table.
    // Without this check it means left top right and bottom of whole cells of whole table.
    if (bIsWholeTable)
    {
        if (nCol == 0)
            applyBorder( rFilterBase, rTableStylePart, XML_left, rLeftBorder );
        if (nCol == nMaxCol)
            applyBorder( rFilterBase, rTableStylePart, XML_right, rRightBorder );
        if (nRow == 0)
            applyBorder( rFilterBase, rTableStylePart, XML_top, rTopBorder );
        if (nRow == nMaxRow)
            applyBorder( rFilterBase, rTableStylePart, XML_bottom, rBottomBorder );

        applyBorder( rFilterBase, rTableStylePart, XML_insideH, rInsideHBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_insideV, rInsideVBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_tl2br, rTopLeftToBottomRightBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_tr2bl, rBottomLeftToTopRightBorder );
    }
    else
    {
        applyBorder( rFilterBase, rTableStylePart, XML_left, rLeftBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_right, rRightBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_top, rTopBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_bottom, rBottomBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_tl2br, rTopLeftToBottomRightBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_tr2bl, rBottomLeftToTopRightBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_insideH, rInsideHBorder );
        applyBorder( rFilterBase, rTableStylePart, XML_insideV, rInsideVBorder );
    }

    aTextCharProps.maLatinFont = rTableStylePart.getLatinFont();
    aTextCharProps.maAsianFont = rTableStylePart.getAsianFont();
    aTextCharProps.maComplexFont = rTableStylePart.getComplexFont();
    aTextCharProps.maSymbolFont = rTableStylePart.getSymbolFont();
    if ( rTableStylePart.getTextColor().isUsed() )
    {
        aTextCharProps.maFillProperties.maFillColor = rTableStylePart.getTextColor();
        aTextCharProps.maFillProperties.moFillType = XML_solidFill;
    }
    if( rTableStylePart.getTextBoldStyle() )
        aTextCharProps.moBold = *rTableStylePart.getTextBoldStyle();
    if( rTableStylePart.getTextItalicStyle() )
        aTextCharProps.moItalic = *rTableStylePart.getTextItalicStyle();
}

static void applyTableCellProperties( const Reference < css::table::XCell >& rxCell, const TableCell& rTableCell )
{
    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    xPropSet->setPropertyValue( u"TextUpperDistance"_ustr, Any( static_cast< sal_Int32 >( rTableCell.getTopMargin() / 360 ) ) );
    xPropSet->setPropertyValue( u"TextRightDistance"_ustr, Any( static_cast< sal_Int32 >( rTableCell.getRightMargin() / 360 ) ) );
    xPropSet->setPropertyValue( u"TextLeftDistance"_ustr, Any( static_cast< sal_Int32 >( rTableCell.getLeftMargin() / 360 ) ) );
    xPropSet->setPropertyValue( u"TextLowerDistance"_ustr, Any( static_cast< sal_Int32 >( rTableCell.getBottomMargin() / 360 ) ) );

    drawing::TextVerticalAdjust eVA;
    switch( rTableCell.getAnchorToken() )
    {
        case XML_ctr:   eVA = drawing::TextVerticalAdjust_CENTER; break;
        case XML_b:     eVA = drawing::TextVerticalAdjust_BOTTOM; break;
        case XML_just:
        case XML_dist:
        default:
        case XML_t:     eVA = drawing::TextVerticalAdjust_TOP; break;
    }
    xPropSet->setPropertyValue( u"TextVerticalAdjust"_ustr, Any( eVA ) );
}

void TableCell::pushToXCell( const ::oox::core::XmlFilterBase& rFilterBase, const ::oox::drawingml::TextListStylePtr& pMasterTextListStyle,
    const css::uno::Reference < css::table::XCell >& rxCell, const TableProperties& rTableProperties,
    const TableStyle& rTableStyle, sal_Int32 nColumn, sal_Int32 nMaxColumn, sal_Int32 nRow, sal_Int32 nMaxRow )
{
    TableStyle& rTable( const_cast< TableStyle& >( rTableStyle ) );
    TableProperties& rProperties( const_cast< TableProperties& >( rTableProperties ) );

    Reference< text::XText > xText( rxCell, UNO_QUERY_THROW );
    Reference< text::XTextCursor > xAt = xText->createTextCursor();

    applyTableCellProperties( rxCell, *this );
    TextCharacterProperties aTextStyleProps;
    xAt->gotoStart( true );
    xAt->gotoEnd( true );

    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    oox::drawingml::FillProperties aFillProperties;
    oox::drawingml::LineProperties aLinePropertiesLeft;
    oox::drawingml::LineProperties aLinePropertiesRight;
    oox::drawingml::LineProperties aLinePropertiesTop;
    oox::drawingml::LineProperties aLinePropertiesBottom;
    oox::drawingml::LineProperties aLinePropertiesInsideH;
    oox::drawingml::LineProperties aLinePropertiesInsideV;
    oox::drawingml::LineProperties aLinePropertiesTopLeftToBottomRight;
    oox::drawingml::LineProperties aLinePropertiesBottomLeftToTopRight;

    applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
        aLinePropertiesLeft,
        aLinePropertiesRight,
        aLinePropertiesTop,
        aLinePropertiesBottom,
        aLinePropertiesInsideH,
        aLinePropertiesInsideV,
        aLinePropertiesTopLeftToBottomRight,
        aLinePropertiesBottomLeftToTopRight,
        rTable.getWholeTbl(),
        true,
        nColumn,
        nMaxColumn,
        nRow,
        nMaxRow );

    if ( rProperties.isFirstRow() && ( nRow == 0 ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getFirstRow() );
    }
    if ( rProperties.isLastRow() && ( nRow == nMaxRow ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getLastRow() );
    }
    if ( rProperties.isFirstCol() && ( nColumn == 0 ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getFirstCol() );
    }
    if ( rProperties.isLastCol() && ( nColumn == nMaxColumn ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getLastCol() );
    }
    if ( rProperties.isBandRow() )
    {
        bool bHasFirstColFillColor
            = (rProperties.isFirstCol() && rTable.getFirstCol().getFillProperties()
               && rTable.getFirstCol().getFillProperties()->maFillColor.isUsed());

        bool bHasLastColFillColor
            = (rProperties.isLastCol() && rTable.getLastCol().getFillProperties()
               && rTable.getLastCol().getFillProperties()->maFillColor.isUsed());

        if ( ( !rProperties.isFirstRow() || ( nRow != 0 ) ) &&
            ( !rProperties.isLastRow() || ( nRow != nMaxRow ) ) &&
            ( !rProperties.isFirstCol() || ( nColumn != 0 ) || !bHasFirstColFillColor ) &&
            ( !rProperties.isLastCol() || ( nColumn != nMaxColumn ) || !bHasLastColFillColor ) )
        {
            sal_Int32 nBand = nRow;
            if ( rProperties.isFirstRow() )
                nBand++;
            if ( nBand & 1 )
            {
                applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesInsideH,
                    aLinePropertiesInsideV,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand2H() );
            }
            else
            {
                applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesInsideH,
                    aLinePropertiesInsideV,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand1H() );
            }
        }
    }
    if ( ( nRow == 0 ) && ( nColumn == 0 ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getNwCell() );
    }
    if ( ( nRow == nMaxRow ) && ( nColumn == 0 ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getSwCell() );
    }
    if ( ( nRow == 0 ) && ( nColumn == nMaxColumn ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getNeCell() );
    }
    if ( ( nRow == nMaxRow ) && ( nColumn == nMaxColumn ) )
    {
        applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesInsideH,
            aLinePropertiesInsideV,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getSeCell() );
    }
    if ( rProperties.isBandCol() )
    {
        if ( ( !rProperties.isFirstRow() || ( nRow != 0 ) ) &&
            ( !rProperties.isLastRow() || ( nRow != nMaxRow ) ) &&
            ( !rProperties.isFirstCol() || ( nColumn != 0 ) ) &&
            ( !rProperties.isLastCol() || ( nColumn != nMaxColumn ) ) )
        {
            sal_Int32 nBand = nColumn;
            if ( rProperties.isFirstCol() )
                nBand++;
            if ( nBand & 1 )
            {
                applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesInsideH,
                    aLinePropertiesInsideV,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand2V() );
            }
            else
            {
                applyTableStylePart( rFilterBase, aFillProperties, aTextStyleProps,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesInsideH,
                    aLinePropertiesInsideV,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand1V() );
            }
        }
    }
    aLinePropertiesLeft.assignUsed( maLinePropertiesLeft );
    aLinePropertiesRight.assignUsed( maLinePropertiesRight );
    aLinePropertiesTop.assignUsed( maLinePropertiesTop );
    aLinePropertiesBottom.assignUsed( maLinePropertiesBottom );
    aLinePropertiesInsideH.assignUsed( maLinePropertiesInsideH );
    aLinePropertiesInsideV.assignUsed( maLinePropertiesInsideV );
    aLinePropertiesTopLeftToBottomRight.assignUsed( maLinePropertiesTopLeftToBottomRight );
    aLinePropertiesBottomLeftToTopRight.assignUsed( maLinePropertiesBottomLeftToTopRight );

    applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesLeft, PROP_LeftBorder );
    applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesRight, PROP_RightBorder );
    applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesTop, PROP_TopBorder );
    applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesBottom, PROP_BottomBorder );
    applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesTopLeftToBottomRight, PROP_DiagonalTLBR );
    applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesBottomLeftToTopRight, PROP_DiagonalBLTR );

    // Convert insideH to Top and Bottom, InsideV to Left and Right. Exclude the outer borders.
    if(nRow != 0)
    {
        aLinePropertiesInsideH.assignUsed( aLinePropertiesTop );
        applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesInsideH, PROP_TopBorder );
    }
    if(nRow != nMaxRow)
    {
        aLinePropertiesInsideH.assignUsed( aLinePropertiesBottom );
        applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesInsideH, PROP_BottomBorder );
    }
    if(nColumn != 0)
    {
        aLinePropertiesInsideV.assignUsed( aLinePropertiesLeft );
        applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesInsideV, PROP_LeftBorder );
    }
    if(nColumn != nMaxColumn)
    {
        aLinePropertiesInsideV.assignUsed( aLinePropertiesRight );
        applyLineAttributes( rFilterBase, xPropSet, rTable, aLinePropertiesInsideV, PROP_RightBorder );
    }

    if (rProperties.getBgColor().isUsed() && !maFillProperties.maFillColor.isUsed() &&
        maFillProperties.moFillType.value_or(0) == XML_noFill)
    {
        maFillProperties.moFillType = XML_solidFill;
        maFillProperties.maFillColor = rProperties.getBgColor();
    }

    aFillProperties.assignUsed( maFillProperties );
    ShapePropertyMap aPropMap( rFilterBase.getModelObjectHelper() );

    Color aBgColor;
    ::Color nPhClr = API_RGB_TRANSPARENT;
    std::shared_ptr< ::oox::drawingml::FillProperties >& rBackgroundFillPropertiesPtr( rTable.getBackgroundFillProperties() );
    ::oox::drawingml::ShapeStyleRef& rBackgroundFillStyle( rTable.getBackgroundFillStyleRef() );
    if (rBackgroundFillPropertiesPtr)
        aBgColor = rBackgroundFillPropertiesPtr->getBestSolidColor();
    else if (rBackgroundFillStyle.mnThemedIdx != 0)
    {
        if (const Theme* pTheme = rFilterBase.getCurrentTheme())
        {
            aBgColor = pTheme->getFillStyle(rBackgroundFillStyle.mnThemedIdx)->getBestSolidColor();
            nPhClr = rBackgroundFillStyle.maPhClr.getColor(rFilterBase.getGraphicHelper());
        }
    }
    if (aBgColor.isUsed())
    {
        const Color& rCellColor = aFillProperties.getBestSolidColor();
        const double fTransparency = rCellColor.isUsed() ? 0.01 * rCellColor.getTransparency() : 1.0;
        ::Color nBgColor( aBgColor.getColor(rFilterBase.getGraphicHelper(), nPhClr) );
        ::Color nCellColor( rCellColor.getColor(rFilterBase.getGraphicHelper()) );
        ::Color aResult( basegfx::interpolate(nBgColor.getBColor(), nCellColor.getBColor(), 1.0 - fTransparency) );
        aFillProperties.maFillColor.clearTransformations();
        aFillProperties.maFillColor.setSrgbClr(sal_Int32(aResult.GetRGBColor()));
        aFillProperties.moFillType = XML_solidFill;
    }
    if (!aFillProperties.moFillType.has_value())
        aFillProperties.moFillType = XML_noFill;

    // TODO: phClr?
    aFillProperties.pushToPropMap( aPropMap, rFilterBase.getGraphicHelper() );
    PropertySet aPropSet{xPropSet};
    aPropSet.setProperties( aPropMap );

    if ( getVertToken() == XML_eaVert )
    {
        xPropSet->setPropertyValue(u"TextWritingMode"_ustr, Any(css::text::WritingMode_TB_RL));
    }

    getTextBody()->insertAt( rFilterBase, xText, xAt, aTextStyleProps, pMasterTextListStyle );

    // tdf#144092 For empty cells push character styles & endParaRPr to the Cell's properties
    const TextParagraphVector& rParagraphs = getTextBody()->getParagraphs();
    if (rParagraphs.size() == 1)
    {
        const auto pFirstParagraph = rParagraphs.at(0);
        if (pFirstParagraph->getRuns().empty())
        {
            TextCharacterProperties aTextCharacterProps{ pFirstParagraph->getCharacterStyle(
                aTextStyleProps, *pMasterTextListStyle, getTextBody()->getTextListStyle()) };
            aTextCharacterProps.assignUsed(pFirstParagraph->getEndProperties());
            aTextCharacterProps.pushToPropSet(aPropSet, rFilterBase);
        }
    }

    if ( getVertToken() == XML_vert )
    {
        xPropSet->setPropertyValue(u"RotateAngle"_ustr, Any(short(27000)));
    }
    else if ( getVertToken() == XML_vert270 )
    {
        xPropSet->setPropertyValue(u"RotateAngle"_ustr, Any(short(9000)));
    }
    else if ( getVertToken() != XML_horz && getVertToken() != XML_eaVert )
    {
        // put the vert value in the grab bag for roundtrip
        const OUString aTokenName(StaticTokenMap().getUnicodeTokenName(getVertToken()));
        Sequence<PropertyValue> aGrabBag;
        xPropSet->getPropertyValue(u"CellInteropGrabBag"_ustr) >>= aGrabBag;
        PropertyValue aPropertyValue = comphelper::makePropertyValue(u"mso-tcPr-vert-value"_ustr, aTokenName);
        if (aGrabBag.hasElements())
        {
            sal_Int32 nLength = aGrabBag.getLength();
            aGrabBag.realloc(nLength + 1);
            aGrabBag.getArray()[nLength] = aPropertyValue;
        }
        else
        {
            aGrabBag = { aPropertyValue };
        }
        xPropSet->setPropertyValue(u"CellInteropGrabBag"_ustr, Any(aGrabBag));
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
