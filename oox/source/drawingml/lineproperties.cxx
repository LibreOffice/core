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
#include "properties.hxx"
#include "tokens.hxx"
#include "oox/helper/modelobjecthelper.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/drawingmltypes.hxx"

using namespace ::com::sun::star::drawing;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::Point;
using ::com::sun::star::container::XNameContainer;
using ::oox::core::FilterBase;

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

void lclSetDashData( LineDash& orLineDash, sal_Int16 nDots, sal_Int32 nDotLen,
        sal_Int16 nDashes, sal_Int32 nDashLen, sal_Int32 nDistance )
{
    orLineDash.Dots = nDots;
    orLineDash.DotLen = nDotLen;
    orLineDash.Dashes = nDashes;
    orLineDash.DashLen = nDashLen;
    orLineDash.Distance = nDistance;
}

/** Converts the specified preset dash to API dash.

    Line length and dot length are set relative to line width and have to be
    multiplied by the actual line width after this function.
 */
void lclConvertPresetDash( LineDash& orLineDash, sal_Int32 nPresetDash )
{
    switch( nPresetDash )
    {
        case XML_dot:           lclSetDashData( orLineDash, 1, 1, 0, 0, 3 );    break;
        case XML_dash:          lclSetDashData( orLineDash, 0, 0, 1, 4, 3 );    break;
        case XML_dashDot:       lclSetDashData( orLineDash, 1, 1, 1, 4, 3 );    break;

        case XML_lgDash:        lclSetDashData( orLineDash, 0, 0, 1, 8, 3 );    break;
        case XML_lgDashDot:     lclSetDashData( orLineDash, 1, 1, 1, 8, 3 );    break;
        case XML_lgDashDotDot:  lclSetDashData( orLineDash, 2, 1, 1, 8, 3 );    break;

        case XML_sysDot:        lclSetDashData( orLineDash, 1, 1, 0, 0, 1 );    break;
        case XML_sysDash:       lclSetDashData( orLineDash, 0, 0, 1, 3, 1 );    break;
        case XML_sysDashDot:    lclSetDashData( orLineDash, 1, 1, 1, 3, 1 );    break;
        case XML_sysDashDotDot: lclSetDashData( orLineDash, 2, 1, 1, 3, 1 );    break;

        default:
            OSL_ENSURE( false, "lclConvertPresetDash - unsupported preset dash" );
            lclSetDashData( orLineDash, 0, 0, 1, 4, 3 );
    }
}

/** Converts the passed custom dash to API dash.

    Line length and dot length are set relative to line width and have to be
    multiplied by the actual line width after this function.
 */
void lclConvertCustomDash( LineDash& orLineDash, const LineProperties::DashStopVector& rCustomDash )
{
    if( rCustomDash.empty() )
    {
        OSL_ENSURE( false, "lclConvertCustomDash - unexpected empty custom dash" );
        lclSetDashData( orLineDash, 0, 0, 1, 4, 3 );
        return;
    }

    // count dashes and dots (stops equal or less than 2 are assumed to be dots)
    sal_Int16 nDots = 0;
    sal_Int32 nDotLen = 0;
    sal_Int16 nDashes = 0;
    sal_Int32 nDashLen = 0;
    sal_Int32 nDistance = 0;
    for( LineProperties::DashStopVector::const_iterator aIt = rCustomDash.begin(), aEnd = rCustomDash.end(); aIt != aEnd; ++aIt )
    {
        if( aIt->first <= 2 )
        {
            ++nDots;
            nDotLen += aIt->first;
        }
        else
        {
            ++nDashes;
            nDashLen += aIt->first;
        }
        nDistance += aIt->second;
    }
    orLineDash.DotLen = (nDots > 0) ? ::std::max< sal_Int32 >( nDotLen / nDots, 1 ) : 0;
    orLineDash.Dots = nDots;
    orLineDash.DashLen = (nDashes > 0) ? ::std::max< sal_Int32 >( nDashLen / nDashes, 1 ) : 0;
    orLineDash.Dashes = nDashes;
    orLineDash.Distance = ::std::max< sal_Int32 >( nDistance / rCustomDash.size(), 1 );
}

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
        ModelObjectHelper& rModelObjHelper, const LinePropertyIds& rPropIds, sal_Int32 nLineWidth, bool bLineEnd )
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
        if( !rPropIds.mbNamedLineMarker || !rModelObjHelper.hasLineMarker( aMarkerName ) )
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

                if( rPropIds.mbNamedLineMarker && !rModelObjHelper.insertLineMarker( aMarkerName, aMarker ) )
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
    if( !rSourceProps.maCustomDash.empty() )
        maCustomDash = rSourceProps.maCustomDash;
    moLineWidth.assignIfUsed( rSourceProps.moLineWidth );
    moPresetDash.assignIfUsed( rSourceProps.moPresetDash );
    moLineCompound.assignIfUsed( rSourceProps.moLineCompound );
    moLineCap.assignIfUsed( rSourceProps.moLineCap );
    moLineJoint.assignIfUsed( rSourceProps.moLineJoint );
}

void LineProperties::pushToPropMap( PropertyMap& rPropMap, const FilterBase& rFilter,
        ModelObjectHelper& rModelObjHelper, const LinePropertyIds& rPropIds, sal_Int32 nPhClr ) const
{
    // line fill type must exist, otherwise ignore other properties
    if( maLineFill.moFillType.has() )
    {
        // line style (our core only supports none and solid)
        LineStyle eLineStyle = (maLineFill.moFillType.get() == XML_noFill) ? LineStyle_NONE : LineStyle_SOLID;

        // create line dash from preset dash token (not for invisible line)
        if( (eLineStyle != LineStyle_NONE) && (moPresetDash.differsFrom( XML_solid ) || (!moPresetDash && !maCustomDash.empty())) )
        {
            LineDash aLineDash;
            aLineDash.Style = lclGetDashStyle( moLineCap.get( XML_rnd ) );

            // convert preset dash or custom dash
            if( moPresetDash.has() )
                lclConvertPresetDash( aLineDash, moPresetDash.get() );
            else
                lclConvertCustomDash( aLineDash, maCustomDash );

            // convert relative dash/dot length to absolute length
            sal_Int32 nLineWidth = GetCoordinate( moLineWidth.get( 103500 ) );
            aLineDash.DotLen *= nLineWidth;
            aLineDash.DashLen *= nLineWidth;
            aLineDash.Distance *= nLineWidth;

            if( rPropIds.mbNamedLineDash )
            {
                OUString aDashName = rModelObjHelper.insertLineDash( aLineDash );
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
        lclPushMarkerProperties( rPropMap, maStartArrow, rModelObjHelper, rPropIds, moLineWidth.get( 0 ), false );
        lclPushMarkerProperties( rPropMap, maEndArrow,   rModelObjHelper, rPropIds, moLineWidth.get( 0 ), true );
    }
}

void LineProperties::pushToPropSet( PropertySet& rPropSet, const FilterBase& rFilter,
    ModelObjectHelper& rModelObjHelper, const LinePropertyIds& rPropIds, sal_Int32 nPhClr ) const
{
    PropertyMap aPropMap;
    pushToPropMap( aPropMap, rFilter, rModelObjHelper, rPropIds, nPhClr );
    rPropSet.setProperties( aPropMap );
}

// ============================================================================

} // namespace drawingml
} // namespace oox

