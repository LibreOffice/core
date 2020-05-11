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

#include <drawingml/lineproperties.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/LineJoint.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <oox/drawingml/shapepropertymap.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/graphichelper.hxx>
#include <oox/token/tokens.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;


namespace oox::drawingml {

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
 */
void lclConvertPresetDash(LineDash& orLineDash, sal_Int32 nPresetDash)
{
    switch( nPresetDash )
    {
        case XML_dot:           lclSetDashData( orLineDash, 1, 1, 0, 0, 3 );    break;
        case XML_dash:          lclSetDashData( orLineDash, 1, 4, 0, 0, 3 );    break;
        case XML_dashDot:       lclSetDashData( orLineDash, 1, 4, 1, 1, 3 );    break;

        case XML_lgDash:        lclSetDashData( orLineDash, 1, 8, 0, 0, 3 );    break;
        case XML_lgDashDot:     lclSetDashData( orLineDash, 1, 8, 1, 1, 3 );    break;
        case XML_lgDashDotDot:  lclSetDashData( orLineDash, 1, 8, 2, 1, 3 );    break;

        case XML_sysDot:        lclSetDashData( orLineDash, 1, 1, 0, 0, 1 );    break;
        case XML_sysDash:       lclSetDashData( orLineDash, 1, 3, 0, 0, 1 );    break;
        case XML_sysDashDot:    lclSetDashData( orLineDash, 1, 3, 1, 1, 1 );    break;
        case XML_sysDashDotDot: lclSetDashData( orLineDash, 1, 3, 2, 1, 1 );    break;

        default:
            OSL_FAIL( "lclConvertPresetDash - unsupported preset dash" );
            lclSetDashData( orLineDash, 1, 4, 0, 0, 3 );
    }
    orLineDash.DotLen *= 100;
    orLineDash.DashLen *= 100;
    orLineDash.Distance *= 100;
}

/** Converts the passed custom dash to API dash. rCustomDash should not be empty.
 * We assume, that there exist only two length values and the distance is the same
 * for all dashes. Other kind of dash stop sequences cannot be represented, neither
 * in model nor in ODF.
 */
void lclConvertCustomDash(LineDash& orLineDash, const LineProperties::DashStopVector& rCustomDash)
{
    OSL_ASSERT(!rCustomDash.empty());
    // Assume all dash stops have the same sp values.
    orLineDash.Distance = rCustomDash[0].second;
    // First kind of dashes go to "Dots"
    orLineDash.DotLen = rCustomDash[0].first;
    orLineDash.Dots = 0;
    for(const auto& rIt : rCustomDash)
    {
        if (rIt.first != orLineDash.DotLen)
            break;
        ++orLineDash.Dots;
    }
    // All others go to "Dashes", we cannot handle more than two kinds.
    orLineDash.Dashes = rCustomDash.size() - orLineDash.Dots;
    if (orLineDash.Dashes > 0)
        orLineDash.DashLen = rCustomDash[orLineDash.Dots].first;
    else
        orLineDash.DashLen = 0;

    // convert to API, e.g. 123% is 123000 in MS Office and 123 in our API
    orLineDash.DotLen = orLineDash.DotLen / 1000;
    orLineDash.DashLen = orLineDash.DashLen / 1000;
    orLineDash.Distance = orLineDash.Distance / 1000;
}

/** LibreOffice uses value 0, if a length attribute is missing in the
 * style definition, but treats it as 100%.
 * LibreOffice uses absolute values in some style definitions. Try to
 * reconstruct them from the imported relative values.
 */
void lclRecoverStandardDashStyles(LineDash& orLineDash, sal_Int32 nLineWidth)
{
    sal_uInt16 nDots = orLineDash.Dots;
    sal_uInt16 nDashes = orLineDash.Dashes;
    sal_uInt32 nDotLen = orLineDash.DotLen;
    sal_uInt32 nDashLen = orLineDash.DashLen;
    sal_uInt32 nDistance = orLineDash.Distance;
    // Use same ersatz for hairline as in export.
    double fWidthHelp = nLineWidth == 0 ? 26.95/100.0 : nLineWidth / 100.0;
    // start with (var) cases, because they have no rounding problems
    // "Fine Dashed", "Line Style 9" and "Dashed (var)" need no recover
    if (nDots == 3 && nDotLen == 197 &&nDashes == 3 && nDashLen == 100 && nDistance == 100)
    {   // "3 Dashes 3 Dots (var)"
        orLineDash.DashLen = 0;
    }
    else if (nDots == 1 && nDotLen == 100 && nDashes == 0 && nDistance == 50)
    {   // "Ultrafine Dotted (var)"
        orLineDash.DotLen = 0;
    }
    else if (nDots == 2 && nDashes == 0 && nDotLen == nDistance
        && std::abs(nDistance * fWidthHelp - 51.0) < fWidthHelp)
    {   // "Ultrafine Dashed"
        orLineDash.Dots = 1;
        orLineDash.DotLen = 51;
        orLineDash.Dashes = 1;
        orLineDash.DashLen = 51;
        orLineDash.Distance = 51;
        orLineDash.Style = orLineDash.Style == DashStyle_ROUNDRELATIVE ? DashStyle_ROUND : DashStyle_RECT;
    }
    else if (nDots == 2 && nDashes == 3 && std::abs(nDotLen * fWidthHelp - 51.0) < fWidthHelp
        && std::abs(nDashLen * fWidthHelp - 254.0) < fWidthHelp
        && std::abs(nDistance * fWidthHelp - 127.0) < fWidthHelp)
    {   // "Ultrafine 2 Dots 3 Dashes"
        orLineDash.DotLen = 51;
        orLineDash.DashLen = 254;
        orLineDash.Distance = 127;
        orLineDash.Style = orLineDash.Style == DashStyle_ROUNDRELATIVE ? DashStyle_ROUND : DashStyle_RECT;
    }
    else if (nDots == 1 && nDotLen == 100 && nDashes == 0
        && std::abs(nDistance * fWidthHelp - 457.0) < fWidthHelp)
    {    // "Fine Dotted"
        orLineDash.DotLen = 0;
        orLineDash.Distance = 457;
        orLineDash.Style = orLineDash.Style == DashStyle_ROUNDRELATIVE ? DashStyle_ROUND : DashStyle_RECT;
    }
    else if (nDots == 1 && nDashes == 10 && nDashLen == 100
        && std::abs(nDistance * fWidthHelp - 152.0) < fWidthHelp)
    {   // "Line with Fine Dots"
        orLineDash.DotLen = 2007;
        orLineDash.DashLen = 0;
        orLineDash.Distance = 152;
        orLineDash.Style = orLineDash.Style == DashStyle_ROUNDRELATIVE ? DashStyle_ROUND : DashStyle_RECT;
    }
    else if (nDots == 2 && nDotLen == 100 && nDashes == 1 && nDashLen == nDistance
        && std::abs(nDistance * fWidthHelp - 203.0) < fWidthHelp)
    {   // "2 Dots 1 Dash"
        orLineDash.DotLen = 0;
        orLineDash.DashLen = 203;
        orLineDash.Distance = 203;
        orLineDash.Style = orLineDash.Style == DashStyle_ROUNDRELATIVE ? DashStyle_ROUND : DashStyle_RECT;
    }
}

DashStyle lclGetDashStyle( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    // MS Office dashing is always relative to line width
    switch( nToken )
    {
        case XML_rnd:   return DashStyle_ROUNDRELATIVE;
        case XML_sq:    return DashStyle_RECTRELATIVE; // default in OOXML
        case XML_flat:  return DashStyle_RECTRELATIVE; // default in MS Office
    }
    return DashStyle_RECTRELATIVE;
}

LineCap lclGetLineCap( sal_Int32 nToken )
{
    OSL_ASSERT((nToken & sal_Int32(0xFFFF0000))==0);
    switch( nToken )
    {
        case XML_rnd:   return LineCap_ROUND;
        case XML_sq:    return LineCap_SQUARE; // default in OOXML
        case XML_flat:  return LineCap_BUTT; // default in MS Office
    }
    return LineCap_BUTT;
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
    if( aNamedMarker.Name.isEmpty() )
        return;

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
        const GraphicHelper& rGraphicHelper, ::Color nPhClr ) const
{
    // line fill type must exist, otherwise ignore other properties
    if( !maLineFill.moFillType.has() )
        return;

    // line style (our core only supports none and solid)
    drawing::LineStyle eLineStyle = (maLineFill.moFillType.get() == XML_noFill) ? drawing::LineStyle_NONE : drawing::LineStyle_SOLID;

    // line width in 1/100mm
    sal_Int32 nLineWidth = getLineWidth(); // includes conversion from EMUs to 1/100mm
    rPropMap.setProperty( ShapeProperty::LineWidth, nLineWidth );

    // create line dash from preset dash token or dash stop vector (not for invisible line)
    if( (eLineStyle != drawing::LineStyle_NONE) && (moPresetDash.differsFrom( XML_solid ) || !maCustomDash.empty()) )
    {
        LineDash aLineDash;
        aLineDash.Style = lclGetDashStyle( moLineCap.get( XML_flat ) );

        if(moPresetDash.differsFrom(XML_solid))
            lclConvertPresetDash(aLineDash, moPresetDash.get(XML_dash));
        else // !maCustomDash.empty()
        {
            lclConvertCustomDash(aLineDash, maCustomDash);
            lclRecoverStandardDashStyles(aLineDash, nLineWidth);
        }
        if( rPropMap.setProperty( ShapeProperty::LineDash, aLineDash ) )
            eLineStyle = drawing::LineStyle_DASH;
    }
    // line cap type
    if( moLineCap.has() )
        rPropMap.setProperty( ShapeProperty::LineCap, lclGetLineCap( moLineCap.get() ) );

    // set final line style property
    rPropMap.setProperty( ShapeProperty::LineStyle, eLineStyle );

    // line joint type
    if( moLineJoint.has() )
        rPropMap.setProperty( ShapeProperty::LineJoint, lclGetLineJoint( moLineJoint.get() ) );

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

drawing::LineStyle LineProperties::getLineStyle() const
{
    // rules to calculate the line style inferred from the code in LineProperties::pushToPropMap
    return (maLineFill.moFillType.get() == XML_noFill) ?
            drawing::LineStyle_NONE :
            (moPresetDash.differsFrom( XML_solid ) || (!moPresetDash && !maCustomDash.empty())) ?
                    drawing::LineStyle_DASH :
                    drawing::LineStyle_SOLID;
}

drawing::LineCap LineProperties::getLineCap() const
{
    if( moLineCap.has() )
        return lclGetLineCap( moLineCap.get() );

    return drawing::LineCap_BUTT;
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

double LineProperties::getPreciseLineWidth() const
{
    return convertEmuToHmm<double>(moLineWidth.get(0), false);
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
