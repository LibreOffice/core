/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tablecell.cxx,v $
 * $Revision: 1.2 $
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
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCellRange.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::com::sun::star::table::BorderLine;
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
        const rtl::OUString& sPropertyName )
{
    try
    {
        BorderLine aBorderLine;
        const rtl::OUString sLineStyle( RTL_CONSTASCII_USTRINGPARAM( "LineStyle" ) );
        drawing::LineStyle eLineStyle( drawing::LineStyle_NONE );
        rLineProperties.getLinePropertyMap()[ sLineStyle ] >>= eLineStyle;
        switch( eLineStyle )
        {
            default:
                {
                    aBorderLine.Color = rLineProperties.getLineColor()->getColor( rFilterBase );
                    aBorderLine.OuterLineWidth = static_cast< sal_Int16 >( rLineProperties.getLineWidth() ? *rLineProperties.getLineWidth() : 0 );
                    aBorderLine.InnerLineWidth = 0;
                    aBorderLine.LineDistance = 0;
                }
                break;
            case drawing::LineStyle_NONE :
                {
                    aBorderLine.OuterLineWidth = 0;
                    aBorderLine.InnerLineWidth = 0;
                    aBorderLine.LineDistance = 0;
                }
            break;
        }
        rxPropSet->setPropertyValue( sPropertyName, Any( aBorderLine ) );
    }
    catch( uno::Exception& )
    {
    }
}

void applyBorder( TableStylePart& rTableStylePart, TableStylePart::LineType eLineType, oox::drawingml::LineProperties& rLineProperties )
{
    std::map < TableStylePart::LineType, boost::shared_ptr< ::oox::drawingml::LineProperties > >& rPartLineBorders( rTableStylePart.getLineBorders() );
    std::map < TableStylePart::LineType, boost::shared_ptr< ::oox::drawingml::LineProperties > >::const_iterator aIter( rPartLineBorders.find( eLineType ) );
    if ( ( aIter != rPartLineBorders.end() ) && ((*aIter).second).get() )
        rLineProperties.apply( *((*aIter).second).get() );
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
        rFillProperties.apply( *rPartFillPropertiesPtr.get() );

    applyBorder( rTableStylePart, TableStylePart::LEFT, rLeftBorder );
    applyBorder( rTableStylePart, TableStylePart::RIGHT, rRightBorder );
    applyBorder( rTableStylePart, TableStylePart::TOP, rTopBorder );
    applyBorder( rTableStylePart, TableStylePart::BOTTOM, rBottomBorder );
    applyBorder( rTableStylePart, TableStylePart::TL2BR, rTopLeftToBottomRightBorder );
    applyBorder( rTableStylePart, TableStylePart::TR2BL, rBottomLeftToTopRightBorder );

    const rtl::OUString sCharColor( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "CharColor" ) ) );
    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    if ( rTableStylePart.getTextColor().isUsed() )
        xPropSet->setPropertyValue( sCharColor, Any( rTableStylePart.getTextColor().getColor( rFilterBase ) ) );

    sal_Int16 nPitch, nFamily;
    ::oox::drawingml::TextFont& rLatinFont( rTableStylePart.getLatinFont() );
    if( rLatinFont.is() )
    {
        const rtl::OUString sCharFontName( CREATE_OUSTRING( "CharFontName" ) );
        const rtl::OUString sCharFontPitch( CREATE_OUSTRING( "CharFontPitch" ) );
        const rtl::OUString sCharFontFamily( CREATE_OUSTRING( "CharFontFamily" ) );
        GetFontPitch( rLatinFont.mnPitch, nPitch, nFamily );
        xPropSet->setPropertyValue( sCharFontName, Any( rLatinFont.msTypeface ) );
        xPropSet->setPropertyValue( sCharFontPitch, Any( nPitch ) );
        xPropSet->setPropertyValue( sCharFontFamily, Any( nFamily ) );
    }
    ::oox::drawingml::TextFont& rAsianFont( rTableStylePart.getAsianFont() );
    if( rAsianFont.is() )
    {
        const rtl::OUString sCharFontNameAsian( CREATE_OUSTRING( "CharFontNameAsian" ) );
        const rtl::OUString sCharFontPitchAsian( CREATE_OUSTRING( "CharFontPitchAsian" ) );
        const rtl::OUString sCharFontFamilyAsian( CREATE_OUSTRING( "CharFontFamilyAsian" ) );
        GetFontPitch( rAsianFont.mnPitch, nPitch, nFamily );
        xPropSet->setPropertyValue( sCharFontNameAsian, Any( rAsianFont.msTypeface ) );
        xPropSet->setPropertyValue( sCharFontPitchAsian, Any( nFamily ) );
        xPropSet->setPropertyValue( sCharFontFamilyAsian, Any( nPitch ) );
    }
    ::oox::drawingml::TextFont& rComplexFont( rTableStylePart.getComplexFont() );
    if( rComplexFont.is() )
    {
        const rtl::OUString sCharFontNameComplex( CREATE_OUSTRING( "CharFontNameComplex" ) );
        const rtl::OUString sCharFontPitchComplex( CREATE_OUSTRING( "CharFontPitchComplex" ) );
        const rtl::OUString sCharFontFamilyComplex( CREATE_OUSTRING( "CharFontFamilyComplex" ) );
        GetFontPitch( rComplexFont.mnPitch, nPitch, nFamily );
        xPropSet->setPropertyValue( sCharFontNameComplex, Any( rComplexFont.msTypeface ) );
        xPropSet->setPropertyValue( sCharFontPitchComplex, Any( nPitch ) );
        xPropSet->setPropertyValue( sCharFontFamilyComplex, Any( nFamily ) );
    }
}

void applyTableCellProperties( const Reference < ::com::sun::star::table::XCell >& rxCell, const TableCell& rTableCell )
{
    static const rtl::OUString  sTopBorder( RTL_CONSTASCII_USTRINGPARAM( "TextUpperDistance" ) );
    static const rtl::OUString  sBottomBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLowerDistance" ) );
    static const rtl::OUString  sLeftBorder( RTL_CONSTASCII_USTRINGPARAM( "TextLeftDistance" ) );
    static const rtl::OUString  sRightBorder( RTL_CONSTASCII_USTRINGPARAM( "TextRightDistance" ) );
    static const rtl::OUString  sVerticalAdjust( RTL_CONSTASCII_USTRINGPARAM( "TextVerticalAdjust" ) );

    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    xPropSet->setPropertyValue( sTopBorder, Any( static_cast< sal_Int32 >( rTableCell.getTopMargin() / 360 ) ) );
    xPropSet->setPropertyValue( sRightBorder, Any( static_cast< sal_Int32 >( rTableCell.getRightMargin() / 360 ) ) );
    xPropSet->setPropertyValue( sLeftBorder, Any( static_cast< sal_Int32 >( rTableCell.getLeftMargin() / 360 ) ) );
    xPropSet->setPropertyValue( sBottomBorder, Any( static_cast< sal_Int32 >( rTableCell.getBottomMargin() / 360 ) ) );

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
    getTextBody()->insertAt( rFilterBase, xText, xAt, pMasterTextListStyle );

    Reference< XPropertySet > xPropSet( rxCell, UNO_QUERY_THROW );
    static const rtl::OUString sLeftBorder( RTL_CONSTASCII_USTRINGPARAM( "LeftBorder" ) );
    static const rtl::OUString sRightBorder( RTL_CONSTASCII_USTRINGPARAM( "RightBorder" ) );
    static const rtl::OUString sTopBorder( RTL_CONSTASCII_USTRINGPARAM( "TopBorder" ) );
    static const rtl::OUString sBottomBorder( RTL_CONSTASCII_USTRINGPARAM( "BottomBorder" ) );
    static const rtl::OUString sDiagonalTLBR( RTL_CONSTASCII_USTRINGPARAM ( "DiagonalTLBR" ) );
    static const rtl::OUString sDiagonalBLTR( RTL_CONSTASCII_USTRINGPARAM ( "DiagonalBLTR" ) );
    oox::drawingml::FillProperties aFillProperties;
    oox::drawingml::LineProperties aLinePropertiesLeft;
    oox::drawingml::LineProperties aLinePropertiesRight;
    oox::drawingml::LineProperties aLinePropertiesTop;
    oox::drawingml::LineProperties aLinePropertiesBottom;
    oox::drawingml::LineProperties aLinePropertiesTopLeftToBottomRight;
    oox::drawingml::LineProperties aLinePropertiesBottomLeftToTopRight;

    boost::shared_ptr< ::oox::drawingml::FillProperties >& rBackgroundFillPropertiesPtr( rTable.getBackgroundFillProperties() );
    if ( rBackgroundFillPropertiesPtr.get() )
        aFillProperties.apply( *rBackgroundFillPropertiesPtr.get() );

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
    aFillProperties.apply( maFillProperties );
    aLinePropertiesLeft.apply( maLinePropertiesLeft );
    aLinePropertiesLeft.apply( maLinePropertiesRight );
    aLinePropertiesLeft.apply( maLinePropertiesTop );
    aLinePropertiesLeft.apply( maLinePropertiesBottom );
    aLinePropertiesLeft.apply( maLinePropertiesTopLeftToBottomRight );
    aLinePropertiesLeft.apply( maLinePropertiesBottomLeftToTopRight );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesLeft, sLeftBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesRight, sRightBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesTop, sTopBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesBottom, sBottomBorder );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesTopLeftToBottomRight, sDiagonalTLBR );
    applyLineAttributes( rFilterBase, xPropSet, aLinePropertiesBottomLeftToTopRight, sDiagonalBLTR );

    aFillProperties.pushToPropSet( rFilterBase, xPropSet, 0 );
}

} } }
