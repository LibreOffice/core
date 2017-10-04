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

#include "drawingml/lineproperties.hxx"
#include <vector>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/FlagSequence.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/token/tokens.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;


namespace oox {
namespace drawingml {

namespace {

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
            OSL_FAIL( "lclConvertPresetDash - unsupported preset dash" );
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
        OSL_FAIL( "lclConvertCustomDash - unexpected empty custom dash" );
        lclSetDashData( orLineDash, 0, 0, 1, 4, 3 );
        return;
    }

    // count dashes and dots (stops equal or less than 2 are assumed to be dots)
    sal_Int16 nDots = 0;
    sal_Int32 nDotLen = 0;
    sal_Int16 nDashes = 0;
    sal_Int32 nDashLen = 0;
    sal_Int32 nDistance = 0;
    sal_Int32 nConvertedLen = 0;
    sal_Int32 nConvertedDistance = 0;
    for( LineProperties::DashStopVector::const_iterator aIt = rCustomDash.begin(), aEnd = rCustomDash.end(); aIt != aEnd; ++aIt )
    {
        // Get from "1000th of percent" ==> percent ==> multiplier
        nConvertedLen      = aIt->first  / 1000 / 100;
        nConvertedDistance = aIt->second / 1000 / 100;

        // Check if it is a dot (100% = dot)
        if( nConvertedLen == 1 )
        {
            ++nDots;
            nDotLen += nConvertedLen;
        }
        else
        {
            ++nDashes;
            nDashLen += nConvertedLen;
        }
        nDistance += nConvertedDistance;
    }
    orLineDash.DotLen = (nDots > 0) ? ::std::max< sal_Int32 >( nDotLen / nDots, 1 ) : 0;
    orLineDash.Dots = nDots;
    orLineDash.DashLen = (nDashes > 0) ? ::std::max< sal_Int32 >( nDashLen / nDashes, 1 ) : 0;
    orLineDash.Dashes = nDashes;
    orLineDash.Distance = ::std::max< sal_Int32 >( nDistance / rCustomDash.size(), 1 );
}

DashStyle lclGetDashStyle( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
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
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
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
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    switch( nToken )
    {
        case XML_sm:    return OOX_ARROWSIZE_SMALL;
        case XML_med:   return OOX_ARROWSIZE_MEDIUM;
        case XML_lg:    return OOX_ARROWSIZE_LARGE;
    }
    return OOX_ARROWSIZE_MEDIUM;
}

void lclPushMarkerProperties( ShapePropertyMap& rPropMap,
        const LineArrowProperties& rArrowProps, sal_Int32 nLineWidth, bool bLineEnd )
{
    /*  Store the marker polygon and the marker name in a single value, to be
        able to pass both to the ShapePropertyMap::setProperty() function. */
    NamedValue aNamedMarker;

    OUStringBuffer aBuffer;
    sal_Int32 nMarkerWidth = 0;
    bool bMarkerCenter = false;
    sal_Int32 nArrowType = rArrowProps.moArrowType.get( XML_none );
    OSL_ASSERT((nArrowType & sal_Int32(0xFFFF0000))==0);
    switch( nArrowType )
    {
        case XML_triangle:
            aBuffer.append( "msArrowEnd" );
        break;
        case XML_arrow:
            aBuffer.append( "msArrowOpenEnd" );
        break;
        case XML_stealth:
            aBuffer.append( "msArrowStealthEnd" );
        break;
        case XML_diamond:
            aBuffer.append( "msArrowDiamondEnd" );
            bMarkerCenter = true;
        break;
        case XML_oval:
            aBuffer.append( "msArrowOvalEnd" );
            bMarkerCenter = true;
        break;
    }

    if( !aBuffer.isEmpty() )
    {
        bool bIsArrow = nArrowType == XML_arrow;
        sal_Int32 nLength = lclGetArrowSize( rArrowProps.moArrowLength.get( XML_med ) );
        sal_Int32 nWidth  = lclGetArrowSize( rArrowProps.moArrowWidth.get( XML_med ) );

        sal_Int32 nNameIndex = nWidth * 3 + nLength + 1;
        aBuffer.append( ' ' ).append( nNameIndex );
        if (bIsArrow)
        {
            // Arrow marker form depends also on line width
            aBuffer.append(' ').append(nLineWidth);
        }
        OUString aMarkerName = aBuffer.makeStringAndClear();

        double fArrowLength = 1.0;
        switch( nLength )
        {
            case OOX_ARROWSIZE_SMALL:   fArrowLength = (bIsArrow ? 2.5 : 2.0); break;
            case OOX_ARROWSIZE_MEDIUM:  fArrowLength = (bIsArrow ? 3.5 : 3.0); break;
            case OOX_ARROWSIZE_LARGE:   fArrowLength = (bIsArrow ? 5.5 : 5.0); break;
        }
        double fArrowWidth = 1.0;
        switch( nWidth )
        {
            case OOX_ARROWSIZE_SMALL:   fArrowWidth = (bIsArrow ? 2.5 : 2.0);  break;
            case OOX_ARROWSIZE_MEDIUM:  fArrowWidth = (bIsArrow ? 3.5 : 3.0);  break;
            case OOX_ARROWSIZE_LARGE:   fArrowWidth = (bIsArrow ? 5.5 : 5.0);  break;
        }
        // set arrow width relative to line width
        sal_Int32 nBaseLineWidth = ::std::max< sal_Int32 >( nLineWidth, 70 );
        nMarkerWidth = static_cast<sal_Int32>( fArrowWidth * nBaseLineWidth );

        /*  Test if the marker already exists in the marker table, do not
            create it again in this case. If markers are inserted explicitly
            instead by their name, the polygon will be created always.
            TODO: this can be optimized by using a map. */
        if( !rPropMap.hasNamedLineMarkerInTable( aMarkerName ) )
        {
            // pass X and Y as percentage to OOX_ARROW_POINT
            auto OOX_ARROW_POINT = [fArrowLength, fArrowWidth]( double x, double y ) { return awt::Point( static_cast< sal_Int32 >( fArrowWidth * x ), static_cast< sal_Int32 >( fArrowLength * y ) ); };
            // tdf#100491 Arrow line marker, unlike other markers, depends on line width.
            // So calculate width of half line (more convenient during drawing) taking into account
            // further conversions/scaling done in OOX_ARROW_POINT and scaling to nMarkerWidth.
            const double fArrowLineHalfWidth = ::std::max< double >( 100.0 * 0.5 * nLineWidth / nMarkerWidth, 1 );

            ::std::vector< awt::Point > aPoints;
            OSL_ASSERT((rArrowProps.moArrowType.get() & sal_Int32(0xFFFF0000))==0);
            switch( rArrowProps.moArrowType.get() )
            {
                case XML_triangle:
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(   0, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT(  50,   0 ) );
                break;
                case XML_arrow:
                    aPoints.push_back( OOX_ARROW_POINT( 50, 0 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 100, 100 - fArrowLineHalfWidth * 1.5) );
                    aPoints.push_back( OOX_ARROW_POINT( 100 - fArrowLineHalfWidth * 1.5, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 50.0 + fArrowLineHalfWidth, 5.5 * fArrowLineHalfWidth) );
                    aPoints.push_back( OOX_ARROW_POINT( 50.0 + fArrowLineHalfWidth, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 50.0 - fArrowLineHalfWidth, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 50.0 - fArrowLineHalfWidth, 5.5 * fArrowLineHalfWidth) );
                    aPoints.push_back( OOX_ARROW_POINT( fArrowLineHalfWidth * 1.5, 100 ) );
                    aPoints.push_back( OOX_ARROW_POINT( 0, 100 - fArrowLineHalfWidth * 1.5) );
                    aPoints.push_back( OOX_ARROW_POINT( 50, 0 ) );
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

            OSL_ENSURE( !aPoints.empty(), "lclPushMarkerProperties - missing arrow coordinates" );
            if( !aPoints.empty() )
            {
                PolyPolygonBezierCoords aMarkerCoords;
                aMarkerCoords.Coordinates.realloc( 1 );
                aMarkerCoords.Coordinates[ 0 ] = ContainerHelper::vectorToSequence( aPoints );

                ::std::vector< PolygonFlags > aFlags( aPoints.size(), PolygonFlags_NORMAL );
                aMarkerCoords.Flags.realloc( 1 );
                aMarkerCoords.Flags[ 0 ] = ContainerHelper::vectorToSequence( aFlags );

                aNamedMarker.Name = aMarkerName;
                aNamedMarker.Value <<= aMarkerCoords;
            }
        }
        else
        {
            /*  Named marker object exists already in the marker table, pass
                its name only. This will set the name as property value, but
                does not create a new object in the marker table. */
            aNamedMarker.Name = aMarkerName;
        }
    }

    // push the properties (filled aNamedMarker.Name indicates valid marker)
    if( !aNamedMarker.Name.isEmpty() )
    {
        if( bLineEnd )
        {
            rPropMap.setProperty( ShapeProperty::LineEnd, aNamedMarker );
            rPropMap.setProperty( ShapeProperty::LineEndWidth, nMarkerWidth );
            rPropMap.setProperty( ShapeProperty::LineEndCenter, bMarkerCenter );
        }
        else
        {
            rPropMap.setProperty( ShapeProperty::LineStart, aNamedMarker );
            rPropMap.setProperty( ShapeProperty::LineStartWidth, nMarkerWidth );
            rPropMap.setProperty( ShapeProperty::LineStartCenter, bMarkerCenter );
        }
    }
}

} // namespace

void LineArrowProperties::assignUsed( const LineArrowProperties& rSourceProps )
{
    moArrowType.assignIfUsed( rSourceProps.moArrowType );
    moArrowWidth.assignIfUsed( rSourceProps.moArrowWidth );
    moArrowLength.assignIfUsed( rSourceProps.moArrowLength );
}

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

void LineProperties::pushToPropMap( ShapePropertyMap& rPropMap,
        const GraphicHelper& rGraphicHelper, sal_Int32 nPhClr ) const
{
    // line fill type must exist, otherwise ignore other properties
    if( maLineFill.moFillType.has() )
    {
        // line style (our core only supports none and solid)
        drawing::LineStyle eLineStyle = (maLineFill.moFillType.get() == XML_noFill) ? drawing::LineStyle_NONE : drawing::LineStyle_SOLID;

        // convert line width from EMUs to 1/100mm
        sal_Int32 nLineWidth = getLineWidth();

        // create line dash from preset dash token (not for invisible line)
        if( (eLineStyle != drawing::LineStyle_NONE) && (moPresetDash.differsFrom( XML_solid ) || !maCustomDash.empty()) )
        {
            LineDash aLineDash;
            aLineDash.Style = lclGetDashStyle( moLineCap.get( XML_rnd ) );

            // convert preset dash or custom dash
            if( moPresetDash.differsFrom( XML_solid ) )
                lclConvertPresetDash( aLineDash, moPresetDash.get() );
            else
                lclConvertCustomDash( aLineDash, maCustomDash );

            // convert relative dash/dot length to absolute length
            sal_Int32 nBaseLineWidth = ::std::max< sal_Int32 >( nLineWidth, 35 );
            aLineDash.DotLen *= nBaseLineWidth;
            aLineDash.DashLen *= nBaseLineWidth;
            aLineDash.Distance *= nBaseLineWidth;

            if( rPropMap.setProperty( ShapeProperty::LineDash, aLineDash ) )
                eLineStyle = drawing::LineStyle_DASH;
        }

        // set final line style property
        rPropMap.setProperty( ShapeProperty::LineStyle, eLineStyle );

        // line joint type
        if( moLineJoint.has() )
            rPropMap.setProperty( ShapeProperty::LineJoint, lclGetLineJoint( moLineJoint.get() ) );

        // line width in 1/100mm
        rPropMap.setProperty( ShapeProperty::LineWidth, nLineWidth );

        // line color and transparence
        Color aLineColor = maLineFill.getBestSolidColor();
        if( aLineColor.isUsed() )
        {
            rPropMap.setProperty( ShapeProperty::LineColor, aLineColor.getColor( rGraphicHelper, nPhClr ) );
            if( aLineColor.hasTransparency() )
                rPropMap.setProperty( ShapeProperty::LineTransparency, aLineColor.getTransparency() );
        }

        // line markers
        lclPushMarkerProperties( rPropMap, maStartArrow, nLineWidth, false );
        lclPushMarkerProperties( rPropMap, maEndArrow,   nLineWidth, true );
    }
}

drawing::LineStyle LineProperties::getLineStyle() const
{
    // rules to calculate the line style inferred from the code in LineProperties::pushToPropMap
    return (maLineFill.moFillType.get() == XML_noFill) ?
            drawing::LineStyle_NONE :
            (moPresetDash.differsFrom( XML_solid ) || (!moPresetDash && !maCustomDash.empty())) ?
                    drawing::LineStyle_DASH :
                    drawing::LineStyle_SOLID;
}

drawing::LineJoint LineProperties::getLineJoint() const
{
    if( moLineJoint.has() )
        return lclGetLineJoint( moLineJoint.get() );

    return drawing::LineJoint_NONE;
}

sal_Int32 LineProperties::getLineWidth() const
{
    return convertEmuToHmm( moLineWidth.get( 0 ) );
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
