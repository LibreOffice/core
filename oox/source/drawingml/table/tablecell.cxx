/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/drawingml/table/tablecell.hxx"
#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "properties.hxx"
#include "tokens.hxx"
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/text/XText.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::com::sun::star::table::BorderLine2;
using ::com::sun::star::drawing::LineStyle;

namespace oox { namespace drawingml { namespace table {

TableCell::TableCell()
: mnRowSpan ( 1 )
, mnGridSpan( 1 )
, mbhMerge( sal_False )
, mbvMerge( sal_False )
, mnMarL( 91440 )
, mnMarR( 91440 )
, mnMarT( 45720 )
, mnMarB( 45720 )
, mnVertToken( XML_horz )
, mnAnchorToken( XML_t )
, mbAnchorCtr( sal_False )
, mnHorzOverflowToken( XML_clip )
{
}
TableCell::~TableCell()
{
}

void applyLineAttributes( const ::oox::core::XmlFilterBase& rFilterBase,
        Reference< XPropertySet >& rxPropSet, oox::drawingml::LineProperties& rLineProperties,
        sal_Int32 nPropId )
{
    BorderLine2 aBorderLine( 0, 0, 0, 0, API_LINE_SOLID );
    if( rLineProperties.maLineFill.moFillType.differsFrom( XML_noFill ) )
    {
        Color aColor = rLineProperties.maLineFill.getBestSolidColor();
        aBorderLine.Color = aColor.getColor( rFilterBase.getGraphicHelper() );
        aBorderLine.OuterLineWidth = static_cast< sal_Int16 >( GetCoordinate( rLineProperties.moLineWidth.get( 0 ) ) / 4 );
        aBorderLine.InnerLineWidth = static_cast< sal_Int16 >( GetCoordinate( rLineProperties.moLineWidth.get( 0 ) ) / 4 );
        aBorderLine.LineDistance = 0;
    }

    PropertySet aPropSet( rxPropSet );
    aPropSet.setProperty( nPropId, aBorderLine );
}

void applyBorder( TableStylePart& rTableStylePart, sal_Int32 nLineType, oox::drawingml::LineProperties& rLineProperties )
{
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >& rPartLineBorders( rTableStylePart.getLineBorders() );
    std::map < sal_Int32, ::oox::drawingml::LinePropertiesPtr >::const_iterator aIter( rPartLineBorders.find( nLineType ) );
    if ( ( aIter != rPartLineBorders.end() ) && aIter->second.get() )
        rLineProperties.assignUsed( *aIter->second );
}

void applyTableStylePart( const ::oox::core::XmlFilterBase& rFilterBase, const Reference < ::com::sun::star::table::XCell >& rxCell, oox::drawingml::FillProperties& rFillProperties,
     oox::drawingml::LineProperties& rLeftBorder,
     oox::drawingml::LineProperties& rRightBorder,
     oox::drawingml::LineProperties& rTopBorder,
     oox::drawingml::LineProperties& rBottomBorder,
     oox::drawingml::LineProperties& rTopLeftToBottomRightBorder,
     oox::drawingml::LineProperties& rBottomLeftToTopRightBorder,
    TableStylePart& rTableStylePart )
{
    boost::shared_ptr< ::oox::drawingml::FillProperties >& rPartFillPropertiesPtr( rTableStylePart.getFillProperties() );
    if ( rPartFillPropertiesPtr.get() )
        rFillProperties.assignUsed( *rPartFillPropertiesPtr );

    applyBorder( rTableStylePart, XML_left, rLeftBorder );
    applyBorder( rTableStylePart, XML_right, rRightBorder );
    applyBorder( rTableStylePart, XML_top, rTopBorder );
    applyBorder( rTableStylePart, XML_bottom, rBottomBorder );
    applyBorder( rTableStylePart, XML_tl2br, rTopLeftToBottomRightBorder );
    applyBorder( rTableStylePart, XML_tr2bl, rBottomLeftToTopRightBorder );

    TextCharacterProperties aTextCharProps;
    aTextCharProps.maLatinFont = rTableStylePart.getLatinFont();
    aTextCharProps.maAsianFont = rTableStylePart.getAsianFont();
    aTextCharProps.maComplexFont = rTableStylePart.getComplexFont();
    aTextCharProps.maSymbolFont = rTableStylePart.getSymbolFont();
    aTextCharProps.maCharColor = rTableStylePart.getTextColor();

    PropertySet aPropSet( rxCell );
    aTextCharProps.pushToPropSet( aPropSet, rFilterBase );
}

void applyTableCellProperties( const Reference < ::com::sun::star::table::XCell >& rxCell, const TableCell& rTableCell )
{
    static const rtl::OUString	sTopBorder( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) );
    static const rtl::OUString	sBottomBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) );
    static const rtl::OUString	sLeftBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) );
    static const rtl::OUString	sRightBorder( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) );
    static const rtl::OUString	sVerticalAdjust( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) );

    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    xPropSet->setPropertyValue( sTopBorder, Any( static_cast< sal_Int32 >( rTableCell.getTopMargin() / 360 ) ) );
    xPropSet->setPropertyValue( sRightBorder, Any( static_cast< sal_Int32 >( rTableCell.getRightMargin() / 360 ) ) );
    xPropSet->setPropertyValue( sLeftBorder, Any( static_cast< sal_Int32 >( rTableCell.getLeftMargin() / 360 ) ) );
    xPropSet->setPropertyValue( sBottomBorder, Any( static_cast< sal_Int32 >( rTableCell.getBottomMargin() / 360 ) ) );

    drawing::TextVerticalAdjust eVA;
    switch( rTableCell.getAnchorToken() )
    {
        case XML_ctr:	eVA = drawing::TextVerticalAdjust_CENTER; break;
        case XML_b:		eVA = drawing::TextVerticalAdjust_BOTTOM; break;
        case XML_just:
        case XML_dist:
        default:
        case XML_t:		eVA = drawing::TextVerticalAdjust_TOP; break;
    }
    xPropSet->setPropertyValue( sVerticalAdjust, Any( eVA ) );
}

void TableCell::pushToXCell( const ::oox::core::XmlFilterBase& rFilterBase, ::oox::drawingml::TextListStylePtr pMasterTextListStyle,
    const ::com::sun::star::uno::Reference < ::com::sun::star::table::XCell >& rxCell, const TableProperties& rTableProperties,
        const TableStyle& rTableStyle, sal_Int32 nColumn, sal_Int32 nMaxColumn, sal_Int32 nRow, sal_Int32 nMaxRow )
{
    TableStyle& rTable( const_cast< TableStyle& >( rTableStyle ) );
    TableProperties& rProperties( const_cast< TableProperties& >( rTableProperties ) );

    Reference< text::XText > xText( rxCell, UNO_QUERY_THROW );
    Reference< text::XTextCursor > xAt = xText->createTextCursor();

    applyTableCellProperties( rxCell, *this );
    TextCharacterProperties aTextStyleProps;
    getTextBody()->insertAt( rFilterBase, xText, xAt, aTextStyleProps, pMasterTextListStyle );

    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    oox::drawingml::FillProperties aFillProperties;
    oox::drawingml::LineProperties aLinePropertiesLeft;
    oox::drawingml::LineProperties aLinePropertiesRight;
    oox::drawingml::LineProperties aLinePropertiesTop;
    oox::drawingml::LineProperties aLinePropertiesBottom;
    oox::drawingml::LineProperties aLinePropertiesTopLeftToBottomRight;
    oox::drawingml::LineProperties aLinePropertiesBottomLeftToTopRight;

    boost::shared_ptr< ::oox::drawingml::FillProperties >& rBackgroundFillPropertiesPtr( rTable.getBackgroundFillProperties() );
    if ( rBackgroundFillPropertiesPtr.get() )
        aFillProperties.assignUsed( *rBackgroundFillPropertiesPtr );

    applyTableStylePart( rFilterBase, rxCell, aFillProperties,
        aLinePropertiesLeft,
        aLinePropertiesRight,
        aLinePropertiesTop,
        aLinePropertiesBottom,
        aLinePropertiesTopLeftToBottomRight,
        aLinePropertiesBottomLeftToTopRight,
        rTable.getWholeTbl() );

    if ( rProperties.isFirstRow() && ( nRow == 0 ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getFirstRow() );
    }
    if ( rProperties.isLastRow() && ( nRow == nMaxRow ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getLastRow() );
    }
    if ( rProperties.isFirstCol() && ( nColumn == 0 ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getFirstCol() );
    }
    if ( rProperties.isLastCol() && ( nColumn == nMaxColumn ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getLastCol() );
    }
    if ( rProperties.isBandRow() )
    {
        if ( ( !rProperties.isFirstRow() || ( nRow != 0 ) ) &&
            ( !rProperties.isLastRow() || ( nRow != nMaxRow ) ) )
        {
            sal_Int32 nBand = nRow;
            if ( rProperties.isFirstRow() )
                nBand++;
            if ( nBand & 1 )
            {
                applyTableStylePart( rFilterBase, rxCell, aFillProperties,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand2H() );
            }
            else
            {
                applyTableStylePart( rFilterBase, rxCell, aFillProperties,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand1H() );
            }
        }
    }
    if ( ( nRow == 0 ) && ( nColumn == 0 ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getNwCell() );
    }
    if ( ( nRow == nMaxRow ) && ( nColumn == 0 ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getSwCell() );
    }
    if ( ( nRow == 0 ) && ( nColumn == nMaxColumn ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getNeCell() );
    }
    if ( ( nRow == nMaxColumn ) && ( nColumn == nMaxColumn ) )
    {
        applyTableStylePart( rFilterBase, rxCell, aFillProperties,
            aLinePropertiesLeft,
            aLinePropertiesRight,
            aLinePropertiesTop,
            aLinePropertiesBottom,
            aLinePropertiesTopLeftToBottomRight,
            aLinePropertiesBottomLeftToTopRight,
            rTable.getSeCell() );
    }
    if ( rProperties.isBandCol() )
    {
        if ( ( !rProperties.isFirstCol() || ( nColumn != 0 ) ) &&
            ( !rProperties.isLastCol() || ( nColumn != nMaxColumn ) ) )
        {
            sal_Int32 nBand = nColumn;
            if ( rProperties.isFirstCol() )
                nBand++;
            if ( nBand & 1 )
            {
                applyTableStylePart( rFilterBase, rxCell, aFillProperties,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
                    aLinePropertiesTopLeftToBottomRight,
                    aLinePropertiesBottomLeftToTopRight,
                    rTable.getBand2V() );
            }
            else
            {
                applyTableStylePart( rFilterBase, rxCell, aFillProperties,
                    aLinePropertiesLeft,
                    aLinePropertiesRight,
                    aLinePropertiesTop,
                    aLinePropertiesBottom,
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
    aLinePropertiesTopLeftToBottomRight.assignUsed( maLinePropertiesTopLeftToBottomRight );
    aLinePropertiesBottomLeftToTopRight.assignUsed( maLinePropertiesBottomLeftToTopRight );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesLeft, PROP_LeftBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesRight, PROP_RightBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesTop, PROP_TopBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesBottom, PROP_BottomBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesTopLeftToBottomRight, PROP_DiagonalTLBR );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesBottomLeftToTopRight, PROP_DiagonalBLTR );

    aFillProperties.assignUsed( maFillProperties );
    PropertySet aPropSet( xPropSet );
    // TODO: phClr?
    aFillProperties.pushToPropSet( aPropSet, rFilterBase.getModelObjectHelper(), rFilterBase.getGraphicHelper() );
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
