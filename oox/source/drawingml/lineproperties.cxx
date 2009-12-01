/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lineproperties.cxx,v $
 * $Revision: 1.8 $
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

#include "oox/drawingml/lineproperties.hxx"
#include <vector>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/core/modelobjectcontainer.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using namespace ::com::sun::star::drawing;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::Point;
using ::com::sun::star::container::XNameContainer;
using ::oox::core::ModelObjectContainer;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {

// ============================================================================

namespace {

static const sal_Int32 spnDefaultLineIds[ LineId_END ] =
{
    PROP_LineStyle,
    PROP_LineWidth,
    PROP_LineColor,
    PROP_LineTransparence,
    PROP_LineDash,
    PROP_LineJoint,
    PROP_LineStartName,
    PROP_LineStartWidth,
    PROP_LineStartCenter,
    PROP_LineEndName,
    PROP_LineEndWidth,
    PROP_LineEndCenter
};

// ----------------------------------------------------------------------------

DashStyle lclGetDashStyle( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_rnd:   return DashStyle_ROUNDRELATIVE;
        case XML_sq:    return DashStyle_RECTRELATIVE;
        case XML_flat:  return DashStyle_RECT;
    }
    return DashStyle_ROUNDRELATIVE;
}

LineJoint lclGetLineJoint( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_round: return LineJoint_ROUND;
        case XML_bevel: return LineJoint_BEVEL;
        case XML_miter: return LineJoint_MITER;
    }
    return LineJoint_ROUND;
}

const sal_Int32 OOX_ARROWSIZE_SMALL     = 0;
const sal_Int32 OOX_ARROWSIZE_MEDIUM    = 1;
const sal_Int32 OOX_ARROWSIZE_LARGE     = 2;

sal_Int32 lclGetArrowSize( sal_Int32 nToken )
{
    switch( nToken )
    {
        case XML_sm:    return OOX_ARROWSIZE_SMALL;
        case XML_med:   return OOX_ARROWSIZE_MEDIUM;
        case XML_lg:    return OOX_ARROWSIZE_LARGE;
    }
    return OOX_ARROWSIZE_MEDIUM;
}

// ----------------------------------------------------------------------------

void lclPushMarkerProperties( PropertyMap& rPropMap, const LineArrowProperties& rArrowProps,
        const LinePropertyIds& rPropIds, ModelObjectContainer& rObjContainer, sal_Int32 nLineWidth, bool bLineEnd )
{
    PolyPolygonBezierCoords aMarker;
    OUString aMarkerName;
    sal_Int32 nMarkerWidth = 0;
    bool bMarkerCenter = false;

    OUStringBuffer aBuffer;
    sal_Int32 nArrowType = rArrowProps.moArrowType.get( XML_none );
    switch( nArrowType )
    {
        case XML_triangle:
            aBuffer.append( CREATE_OUSTRING( "msArrowEnd" ) );
        break;
        case XML_arrow:
            aBuffer.append( CREATE_OUSTRING( "msArrowOpenEnd" ) );
        break;
        case XML_stealth:
            aBuffer.append( CREATE_OUSTRING( "msArrowStealthEnd" ) );
        break;
        case XML_diamond:
            aBuffer.append( CREATE_OUSTRING( "msArrowDiamondEnd" ) );
            bMarkerCenter = true;
        break;
        case XML_oval:
            aBuffer.append( CREATE_OUSTRING( "msArrowOvalEnd" ) );
            bMarkerCenter = true;
        break;
    }

    if( aBuffer.getLength() > 0 )
    {
        sal_Int32 nLength = lclGetArrowSize( rArrowProps.moArrowLength.get( XML_med ) );
        sal_Int32 nWidth  = lclGetArrowSize( rArrowProps.moArrowWidth.get( XML_med ) );

        sal_Int32 nNameIndex = nWidth * 3 + nLength + 1;
        aBuffer.append( sal_Unicode( ' ' ) ).append( nNameIndex );
        aMarkerName = aBuffer.makeStringAndClear();

        bool bIsArrow = nArrowType == XML_arrow;
        double fArrowLength = 1.0;
        switch( nLength )
        {
            case OOX_ARROWSIZE_SMALL:   fArrowLength = (bIsArrow ? 3.5 : 2.0); break;
            case OOX_ARROWSIZE_MEDIUM:  fArrowLength = (bIsArrow ? 4.5 : 3.0); break;
            case OOX_ARROWSIZE_LARGE:   fArrowLength = (bIsArrow ? 6.0 : 5.0); break;
        }
        double fArrowWidth = 1.0;
        switch( nWidth )
        {
            case OOX_ARROWSIZE_SMALL:   fArrowWidth = (bIsArrow ? 3.5 : 2.0);  break;
            case OOX_ARROWSIZE_MEDIUM:  fArrowWidth = (bIsArrow ? 4.5 : 3.0);  break;
            case OOX_ARROWSIZE_LARGE:   fArrowWidth = (bIsArrow ? 6.0 : 5.0);  break;
        }
        // set arrow width relative to line width (convert line width from EMUs to 1/100 mm)
        sal_Int32 nApiLineWidth = ::std::max< sal_Int32 >( GetCoordinate( nLineWidth ), 70 );
        nMarkerWidth = static_cast< sal_Int32 >( fArrowWidth * nApiLineWidth );

        // test if the arrow already exists, do not create it again in this case
        if( !rPropIds.mbNamedLineMarker || !rObjContainer.hasLineMarker( aMarkerName ) )
        {
// pass X and Y as percentage to OOX_ARROW_POINT
#define OOX_ARROW_POINT( x, y ) Point( static_cast< sal_Int32 >( fArrowWidth * x ), static_cast< sal_Int32 >( fArrowLength * y ) )

            ::std::vector< Point > aPoints;
            switch( rArrowProps.moArrowType.get() )
            {
                case XML_triangle:
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   0, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                break;
                case XML_arrow:
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100,  91 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  85, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,  36 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  15, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   0,  91 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                break;
                case XML_stealth:
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,  60 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   0, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                break;
                case XML_diamond:
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100,  50 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   0,  50 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                break;
                case XML_oval:
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  75,   7 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  93,  25 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100,  50 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  93,  75 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  75,  93 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  25,  93 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   7,  75 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   0,  50 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   7,  25 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  25,   7 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                break;
            }
#undef OOX_ARROW_POINT

            OSL_ENSURE( !aPoints.empty(), "ApiLineMarkerProperties::ApiLineMarkerProperties - missing arrow coordinates" );
            if( !aPoints.empty() )
            {
                aMarker.Coordinates.realloc( 1 );
                aMarker.Coordinates[ 0 ] = ContainerHelper::vectorToSequence( aPoints );

                ::std::vector< PolygonFlags > aFlags( aPoints.size(), PolygonFlags_NORMAL );
                aMarker.Flags.realloc( 1 );
                aMarker.Flags[ 0 ] = ContainerHelper::vectorToSequence( aFlags );

                if( rPropIds.mbNamedLineMarker && !rObjContainer.insertLineMarker( aMarkerName, aMarker ) )
                    aMarkerName = OUString();
            }
            else
            {
                aMarkerName = OUString();
            }
        }
    }

    // push the properties (filled aMarkerName indicates valid marker)
    if( aMarkerName.getLength() > 0 )
    {
        if( bLineEnd )
        {
            if( rPropIds.mbNamedLineMarker )
                rPropMap.setProperty( rPropIds[ LineEndId ], aMarkerName );
            else
                rPropMap.setProperty( rPropIds[ LineEndId ], aMarker );
            rPropMap.setProperty( rPropIds[ LineEndWidthId ], nMarkerWidth );
            rPropMap.setProperty( rPropIds[ LineEndCenterId ], bMarkerCenter );
        }
        else
        {
            if( rPropIds.mbNamedLineMarker )
                rPropMap.setProperty( rPropIds[ LineStartId ], aMarkerName );
            else
                rPropMap.setProperty( rPropIds[ LineStartId ], aMarker );
            rPropMap.setProperty( rPropIds[ LineStartWidthId ], nMarkerWidth );
            rPropMap.setProperty( rPropIds[ LineStartCenterId ], bMarkerCenter );
        }
    }
}

} // namespace

// ============================================================================

LinePropertyIds::LinePropertyIds( const sal_Int32* pnPropertyIds, bool bNamedLineDash, bool bNamedLineMarker ) :
    mpnPropertyIds( pnPropertyIds ),
    mbNamedLineDash( bNamedLineDash ),
    mbNamedLineMarker( bNamedLineMarker )
{
    OSL_ENSURE( mpnPropertyIds != 0, "LinePropertyIds::LinePropertyIds - missing property identifiers" );
}

// ============================================================================

void LineArrowProperties::assignUsed( const LineArrowProperties& rSourceProps )
{
    moArrowType.assignIfUsed( rSourceProps.moArrowType );
    moArrowWidth.assignIfUsed( rSourceProps.moArrowWidth );
    moArrowLength.assignIfUsed( rSourceProps.moArrowLength );
}

// ============================================================================

LinePropertyIds LineProperties::DEFAULT_IDS( spnDefaultLineIds, false, true );

void LineProperties::assignUsed( const LineProperties& rSourceProps )
{
    maStartArrow.assignUsed( rSourceProps.maStartArrow );
    maEndArrow.assignUsed( rSourceProps.maEndArrow );
    maLineFill.assignUsed( rSourceProps.maLineFill );
    moLineWidth.assignIfUsed( rSourceProps.moLineWidth );
    moPresetDash.assignIfUsed( rSourceProps.moPresetDash );
    moLineCap.assignIfUsed( rSourceProps.moLineCap );
    moLineJoint.assignIfUsed( rSourceProps.moLineJoint );
}

void LineProperties::pushToPropMap( PropertyMap& rPropMap, const LinePropertyIds& rPropIds,
        const XmlFilterBase& rFilter, ModelObjectContainer& rObjContainer, sal_Int32 nPhClr ) const
{
    // line fill type must exist, otherwise ignore other properties
    if( maLineFill.moFillType.has() )
    {
        // line style (our core only supports none and solid)
        LineStyle eLineStyle = (maLineFill.moFillType.get() == XML_noFill) ? LineStyle_NONE : LineStyle_SOLID;

        // create line dash from preset dash token (not for invisible line)
        if( (eLineStyle != LineStyle_NONE) && moPresetDash.differsFrom( XML_solid ) )
        {
            LineDash aLineDash;

            sal_Int32 nLineWidth = GetCoordinate( moLineWidth.get( 103500 ) );
            aLineDash.Style = lclGetDashStyle( moLineCap.get( XML_rnd ) );
            aLineDash.Dots  = 1;
            aLineDash.DotLen = nLineWidth;
            aLineDash.Dashes = 0;
            aLineDash.DashLen = 8 * nLineWidth;
            aLineDash.Distance = 3 * nLineWidth;

            switch( moPresetDash.get() )
            {
                default:
                case XML_dash:
                case XML_sysDash:
                    aLineDash.DashLen = 4 * nLineWidth;
                // passthrough intended
                case XML_lgDash:
                    aLineDash.Dots = 0;
                    aLineDash.Dashes = 1;
                break;

                case XML_dashDot:
                case XML_sysDashDot:
                    aLineDash.DashLen = 4 * nLineWidth;
                // passthrough intended
                case XML_lgDashDot:
                    aLineDash.Dashes = 1;
                break;

                case XML_sysDashDotDot:
                    aLineDash.DashLen = 4 * nLineWidth;
                // passthrough intended
                case XML_lgDashDotDot:
                    aLineDash.Dots = 2;
                    aLineDash.Dashes = 1;
                break;

                case XML_dot:
                case XML_sysDot:
                    aLineDash.Distance = aLineDash.DotLen;
                break;
            }

            if( rPropIds.mbNamedLineDash )
            {
                OUString aDashName = rObjContainer.insertLineDash( aLineDash );
                if( aDashName.getLength() > 0 )
                {
                    rPropMap.setProperty( rPropIds[ LineDashId ], aDashName );
                    eLineStyle = LineStyle_DASH;
                }
            }
            else
            {
                rPropMap.setProperty( rPropIds[ LineDashId ], aLineDash );
                eLineStyle = LineStyle_DASH;
            }
        }

        // set final line style property
        rPropMap.setProperty( rPropIds[ LineStyleId ], eLineStyle );

        // line joint type
        if( moLineJoint.has() )
            rPropMap.setProperty( rPropIds[ LineJointId ], lclGetLineJoint( moLineJoint.get() ) );

        // convert line width from EMUs to 1/100 mm
        if( moLineWidth.has() )
            rPropMap.setProperty( rPropIds[ LineWidthId ], GetCoordinate( moLineWidth.get() ) );

        // line color and transparence
        Color aLineColor = maLineFill.getBestSolidColor();
        if( aLineColor.isUsed() )
        {
            rPropMap.setProperty( rPropIds[ LineColorId ], aLineColor.getColor( rFilter, nPhClr ) );
            if( aLineColor.hasTransparence() )
                rPropMap.setProperty( rPropIds[ LineTransparenceId ], aLineColor.getTransparence() );
        }

        // line markers
        lclPushMarkerProperties( rPropMap, maStartArrow, rPropIds, rObjContainer, moLineWidth.get( 0 ), false );
        lclPushMarkerProperties( rPropMap, maEndArrow,   rPropIds, rObjContainer, moLineWidth.get( 0 ), true );
    }
}

void LineProperties::pushToPropSet( PropertySet& rPropSet, const LinePropertyIds& rPropIds,
        const XmlFilterBase& rFilter, ModelObjectContainer& rObjContainer, sal_Int32 nPhClr ) const
{
    PropertyMap aPropMap;
    pushToPropMap( aPropMap, rPropIds, rFilter, rObjContainer, nPhClr );
    rPropSet.setProperties( aPropMap );
}

// ============================================================================

} // namespace drawingml
} // namespace oox

