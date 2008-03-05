/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lineproperties.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:24:01 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "oox/helper/propertyset.hxx"
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

namespace oox { namespace drawingml {

LineProperties::LineProperties()
: maLineColor( new Color() )
{
}
LineProperties::~LineProperties()
{
}

void LineProperties::apply( const LinePropertiesPtr& rSourceLineProperties )
{
    PropertyMapBase::const_iterator aIter( rSourceLineProperties->maLineProperties.begin() );
    PropertyMapBase::const_iterator aEnd( rSourceLineProperties->maLineProperties.end() );
    while( aIter != aEnd )
    {
        maLineProperties[ (*aIter).first ] = (*aIter).second;
        aIter++;
    }
    if ( rSourceLineProperties->maLineColor->isUsed() )
        maLineColor = rSourceLineProperties->maLineColor;
    if ( rSourceLineProperties->moLineWidth )
        moLineWidth = rSourceLineProperties->moLineWidth;
    if ( rSourceLineProperties->moStartArrow )
        moStartArrow = rSourceLineProperties->moStartArrow;
    if ( rSourceLineProperties->moStartArrowWidth )
        moStartArrowWidth = rSourceLineProperties->moStartArrowWidth;
    if ( rSourceLineProperties->moStartArrowLength )
        moStartArrowLength = rSourceLineProperties->moStartArrowLength;
    if ( rSourceLineProperties->moEndArrow )
        moEndArrow = rSourceLineProperties->moEndArrow;
    if ( rSourceLineProperties->moEndArrowWidth )
        moEndArrowWidth = rSourceLineProperties->moEndArrowWidth;
    if ( rSourceLineProperties->moEndArrowLength )
        moEndArrowLength = rSourceLineProperties->moEndArrowLength;
    if ( rSourceLineProperties->moPresetDash )
        moPresetDash = rSourceLineProperties->moPresetDash;
    if ( rSourceLineProperties->moLineCap )
        moLineCap = rSourceLineProperties->moLineCap;
}

static com::sun::star::drawing::PolyPolygonBezierCoords GetLineArrow( const sal_Int32 nLineWidth, const sal_Int32 nLineEndToken,
    const sal_Int32 nArrowWidthToken, const sal_Int32 nArrowLengthToken, sal_Int32& rnArrowWidth, sal_Bool& rbArrowCenter, rtl::OUString& rsArrowName )
{
    uno::Sequence< awt::Point > aPoly;

    double      fLineWidth = nLineWidth < 70 ? 70.0 : nLineWidth;
    double      fLenghtMul, fWidthMul;
    sal_Int32   nLineNumber;
    switch( nArrowLengthToken )
    {
        default :
        case XML_med: fLenghtMul = 3.0; nLineNumber = 2; break;
        case XML_sm : fLenghtMul = 2.0; nLineNumber = 1; break;
        case XML_lg : fLenghtMul = 5.0; nLineNumber = 3; break;
    }
    switch( nArrowWidthToken )
    {
        default :
        case XML_med: fWidthMul = 3.0; nLineNumber += 3; break;
        case XML_sm : fWidthMul = 2.0; break;
        case XML_lg : fWidthMul = 5.0; nLineNumber += 6; break;
    }
    rbArrowCenter = sal_False;
    switch ( nLineEndToken )
    {
        case XML_triangle :
        {
            aPoly.realloc( 4 );
            aPoly[ 0 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            aPoly[ 1 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 2 ] = awt::Point( 0, static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 3 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            static const OUString sArrowEnd( RTL_CONSTASCII_USTRINGPARAM( "msArrowEnd " ) );
            rsArrowName = sArrowEnd;
        }
        break;

        case XML_arrow :
        {
            switch( nArrowLengthToken )
            {
                default :
                case XML_med: fLenghtMul = 4.5; break;
                case XML_sm : fLenghtMul = 3.5; break;
                case XML_lg : fLenghtMul = 6.0; break;
            }
            switch( nArrowWidthToken )
            {
                default :
                case XML_med: fWidthMul = 4.5; break;
                case XML_sm : fWidthMul = 3.5; break;
                case XML_lg : fWidthMul = 6.0; break;
            }
            aPoly.realloc( 7 );
            aPoly[ 0 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            aPoly[ 1 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.91 ) );
            aPoly[ 2 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.85 ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 3 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.36 ) );
            aPoly[ 4 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.15 ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 5 ] = awt::Point( 0, static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.91 ) );
            aPoly[ 6 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            static const OUString sArrowOpenEnd( RTL_CONSTASCII_USTRINGPARAM( "msArrowOpenEnd " ) );
            rsArrowName = sArrowOpenEnd;
        }
        break;
        case XML_stealth :
        {
            aPoly.realloc( 5 );
            aPoly[ 0 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            aPoly[ 1 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 2 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.60 ) );
            aPoly[ 3 ] = awt::Point( 0, static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 4 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            static const OUString sArrowStealthEnd( RTL_CONSTASCII_USTRINGPARAM( "msArrowStealthEnd " ) );
            rsArrowName = sArrowStealthEnd;
        }
        break;
        case XML_diamond :
        {
            aPoly.realloc( 5 );
            aPoly[ 0 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            aPoly[ 1 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.50 ) );
            aPoly[ 2 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 3 ] = awt::Point( 0, static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.50 ) );
            aPoly[ 4 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            static const OUString sArrowDiamondEnd( RTL_CONSTASCII_USTRINGPARAM( "msArrowDiamondEnd " ) );
            rsArrowName = sArrowDiamondEnd;
            rbArrowCenter = sal_True;
        }
        break;
        case XML_oval :
        {
            aPoly.realloc( 5 );
            aPoly[ 0 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            aPoly[ 1 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.50 ) );
            aPoly[ 2 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), static_cast< sal_Int32 >( fLenghtMul * fLineWidth ) );
            aPoly[ 3 ] = awt::Point( 0, static_cast< sal_Int32 >( fLenghtMul * fLineWidth * 0.50 ) );
            aPoly[ 4 ] = awt::Point( static_cast< sal_Int32 >( fWidthMul * fLineWidth * 0.50 ), 0 );
            static const OUString sArrowOvalEnd( RTL_CONSTASCII_USTRINGPARAM( "msArrowOvalEnd " ) );
            rsArrowName = sArrowOvalEnd;
            rbArrowCenter = sal_True;
        }
        break;
        default: break;
    }
    rsArrowName += rtl::OUString::valueOf( nLineNumber );
    rnArrowWidth = static_cast< sal_Int32 >( fLineWidth * fWidthMul );

    com::sun::star::drawing::PolyPolygonBezierCoords aPolyPolyBezier;
    aPolyPolyBezier.Coordinates.realloc( 1 );
    aPolyPolyBezier.Flags.realloc( 1 );
    ::com::sun::star::drawing::PointSequence* pOuterSequence = aPolyPolyBezier.Coordinates.getArray();
    ::com::sun::star::drawing::FlagSequence*  pOuterFlags = aPolyPolyBezier.Flags.getArray();
    pOuterSequence[ 0 ] = aPoly;
    pOuterFlags[ 0 ] = ::com::sun::star::drawing::PolygonFlags( aPoly.getLength() );
    return aPolyPolyBezier;
}

void setArrow( const ::oox::core::XmlFilterBase& rFilterBase, rtl::OUString& rName, com::sun::star::drawing::PolyPolygonBezierCoords& rPoly )
{
    uno::Reference< container::XNameContainer >& xMarker( rFilterBase.getMarkerTable() );
    try
    {
        if( xMarker.is() )
        {
            if( xMarker->hasByName( rName ) )
                xMarker->replaceByName( rName, Any( rPoly ) );
            else
                xMarker->insertByName( rName, Any( rPoly ) );
        }
    }
    catch( container::ElementExistException& )
    {}
}

void LineProperties::pushToPropSet( const ::oox::core::XmlFilterBase& rFilterBase,
    const Reference < XPropertySet >& xPropSet ) const
{
    PropertySet aPropSet( xPropSet );
    Sequence< OUString > aNames;
    Sequence< Any > aValues;

    maLineProperties.makeSequence( aNames, aValues );
    aPropSet.setProperties( aNames, aValues );
    if ( maLineColor->isUsed() )
    {
        const rtl::OUString sLineColor( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineColor" ) ) );
        xPropSet->setPropertyValue( sLineColor, Any( maLineColor->getColor( rFilterBase ) ) );

        if ( maLineColor->hasAlpha() )
        {
            const rtl::OUString sLineTransparence( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineTransparence" ) ) );
            xPropSet->setPropertyValue( sLineTransparence, Any( static_cast< sal_Int16 >( ( 100000 - maLineColor->getAlpha() ) / 1000 ) ) );
        }
    }
    if ( moLineWidth )
    {
        const rtl::OUString sLineWidth( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineWidth" ) ) );
        xPropSet->setPropertyValue( sLineWidth, Any( *moLineWidth ) );
    }
    if ( moStartArrow && ( *moStartArrow != XML_none ) )
    {
        sal_Int32   nArrowWidth;
        sal_Bool    bArrowCenter;
        rtl::OUString aArrowName;
        com::sun::star::drawing::PolyPolygonBezierCoords aPoly( GetLineArrow( moLineWidth ? *moLineWidth : 70, *moStartArrow,
            moStartArrowWidth ? *moStartArrowWidth : XML_med, moStartArrowLength ? *moStartArrowLength : XML_med, nArrowWidth, bArrowCenter, aArrowName ) );
        const rtl::OUString sLineStart( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineStart" ) ) );
        const rtl::OUString sLineStartName( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineStartName" ) ) );
        const rtl::OUString sLineStartCenter( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineStartCenter" ) ) );
        const rtl::OUString sLineStartWidth( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineStartWidth" ) ) );
//      xPropSet->setPropertyValue( sLineStart, Any( aPoly ) );
        setArrow( rFilterBase, aArrowName, aPoly  );
        xPropSet->setPropertyValue( sLineStartName, Any( aArrowName ) );
        xPropSet->setPropertyValue( sLineStartCenter, Any( bArrowCenter ) );
        xPropSet->setPropertyValue( sLineStartWidth, Any( nArrowWidth ) );
    }
    if ( moEndArrow && ( *moEndArrow != XML_none ) )
    {
        sal_Int32   nArrowWidth;
        sal_Bool    bArrowCenter;
        rtl::OUString aArrowName;
        com::sun::star::drawing::PolyPolygonBezierCoords aPoly( GetLineArrow( moLineWidth ? *moLineWidth : 70, *moEndArrow,
            moEndArrowWidth ? *moEndArrowWidth : XML_med, moEndArrowLength ? *moEndArrowLength : XML_med, nArrowWidth, bArrowCenter, aArrowName ) );
        const rtl::OUString sLineEnd( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineEnd" ) ) );
        const rtl::OUString sLineEndName( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineEndName" ) ) );
        const rtl::OUString sLineEndCenter( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineEndCenter" ) ) );
        const rtl::OUString sLineEndWidth( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineEndWidth" ) ) );
//      xPropSet->setPropertyValue( sLineEnd, Any( aPoly ) );
        setArrow( rFilterBase, aArrowName, aPoly );
        xPropSet->setPropertyValue( sLineEndName, Any( aArrowName ) );
        xPropSet->setPropertyValue( sLineEndCenter, Any( bArrowCenter ) );
        xPropSet->setPropertyValue( sLineEndWidth, Any( nArrowWidth ) );
    }
    if ( moPresetDash ) // ST_PresetLineDashVal
    {
        const rtl::OUString sLineStyle( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineStyle" ) ) );
        if ( *moPresetDash == XML_solid )
            xPropSet->setPropertyValue( sLineStyle, Any( drawing::LineStyle_SOLID ) );
        else
        {
            sal_Int32 nLineWidth = moLineWidth ? *moLineWidth : 288;
            drawing::LineDash aLineDash;
            aLineDash.Style = drawing::DashStyle_ROUNDRELATIVE;
            if ( moLineCap )
            {
                switch( *moLineCap )
                {
                    default:
                    case XML_rnd:   // Rounded ends. Semi-circle protrudes by half line width.
                        aLineDash.Style = drawing::DashStyle_ROUNDRELATIVE;
                    break;
                    case XML_sq:    // Square protrudes by half line width.
                        aLineDash.Style = drawing::DashStyle_RECTRELATIVE;
                    break;
                    case XML_flat:  // Line ends at end point.
                        aLineDash.Style = drawing::DashStyle_RECT;
                    break;
                }
            }
            aLineDash.Dots  = 1;
            aLineDash.DotLen = nLineWidth;
            aLineDash.Dashes = 0;
            aLineDash.DashLen = ( 8 * nLineWidth );
            aLineDash.Distance = ( 3 * nLineWidth );

            switch( *moPresetDash )
            {
                default :
                case XML_dash :
                case XML_sysDash :
                    aLineDash.DashLen = ( 4 * nLineWidth ); // !!PASSTHROUGH INTENDED
                case XML_lgDash :
                {
                    aLineDash.Dots = 0;
                    aLineDash.Dashes = 1;
                }
                break;
                case XML_dashDot :
                case XML_sysDashDot :
                    aLineDash.DashLen = ( 4 * nLineWidth ); // !!PASSTHROUGH INTENDED
                case XML_lgDashDot :
                    aLineDash.Dashes = 1;
                break;
                case XML_sysDashDotDot :
                    aLineDash.DashLen = ( 4 * nLineWidth ); // !!PASSTHROUGH INTENDED
                case XML_lgDashDotDot :
                {
                    aLineDash.Dots = 2;
                    aLineDash.Dashes = 1;
                }
                break;
                case XML_dot :
                case XML_sysDot :
                    aLineDash.Distance = aLineDash.DotLen;
                break;
            }
            const rtl::OUString sLineDash( OUString::intern( RTL_CONSTASCII_USTRINGPARAM( "LineDash" ) ) );
            xPropSet->setPropertyValue( sLineStyle, Any( drawing::LineStyle_DASH ) );
            xPropSet->setPropertyValue( sLineDash, Any( aLineDash ) );
        }
    }
}

} }
