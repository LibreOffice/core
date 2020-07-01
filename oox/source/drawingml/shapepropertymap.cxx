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

#include <oox/drawingml/shapepropertymap.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>

#include <oox/helper/modelobjecthelper.hxx>
#include <oox/token/properties.hxx>

namespace oox::drawingml {

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;

namespace {

const ShapePropertyIds spnDefaultShapeIds =
{
    PROP_LineStyle, PROP_LineWidth, PROP_LineColor, PROP_LineTransparence, PROP_LineDash, PROP_LineCap, PROP_LineJoint,
    PROP_LineStartName, PROP_LineStartWidth, PROP_LineStartCenter, PROP_LineEndName, PROP_LineEndWidth, PROP_LineEndCenter,
    PROP_FillStyle, PROP_FillColor, PROP_FillTransparence, PROP_FillTransparenceGradientName, PROP_FillGradient,
    PROP_FillBitmap, PROP_FillBitmapMode, PROP_FillBitmapSizeX, PROP_FillBitmapSizeY,
    PROP_FillBitmapPositionOffsetX, PROP_FillBitmapPositionOffsetY, PROP_FillBitmapRectanglePoint,
    PROP_FillHatch,
    PROP_FillBackground,
    PROP_FillBitmapName,
    PROP_ShadowXDistance,
    PROP_ShadowSizeX,
    PROP_ShadowSizeY
};

} // namespace

ShapePropertyInfo ShapePropertyInfo::DEFAULT( spnDefaultShapeIds, true, false, false, false, false );

ShapePropertyInfo::ShapePropertyInfo( const ShapePropertyIds& rnPropertyIds,
        bool bNamedLineMarker, bool bNamedLineDash, bool bNamedFillGradient, bool bNamedFillBitmap, bool bNamedFillHatch ) :
    mrPropertyIds(rnPropertyIds),
    mbNamedLineMarker( bNamedLineMarker ),
    mbNamedLineDash( bNamedLineDash ),
    mbNamedFillGradient( bNamedFillGradient ),
    mbNamedFillBitmap( bNamedFillBitmap ),
    mbNamedFillHatch( bNamedFillHatch )
{
}

ShapePropertyMap::ShapePropertyMap( ModelObjectHelper& rModelObjHelper, const ShapePropertyInfo& rShapePropInfo ) :
    mrModelObjHelper( rModelObjHelper ),
    maShapePropInfo( rShapePropInfo )
{
}

bool ShapePropertyMap::supportsProperty( ShapeProperty ePropId ) const
{
    return maShapePropInfo.has( ePropId );
}

bool ShapePropertyMap::hasNamedLineMarkerInTable( const OUString& rMarkerName ) const
{
    return maShapePropInfo.mbNamedLineMarker && mrModelObjHelper.hasLineMarker( rMarkerName );
}

bool ShapePropertyMap::setAnyProperty( ShapeProperty ePropId, const Any& rValue )
{
    // get current property identifier for the specified property
    sal_Int32 nPropId = maShapePropInfo[ ePropId ];
    if( nPropId < 0 ) return false;

    // special handling for properties supporting named objects in tables
    switch( ePropId )
    {
        case ShapeProperty::LineStart:
        case ShapeProperty::LineEnd:
            return setLineMarker( nPropId, rValue );

        case ShapeProperty::LineDash:
            return setLineDash( nPropId, rValue );

        case ShapeProperty::FillGradient:
            return setFillGradient( nPropId, rValue );

        case ShapeProperty::GradientTransparency:
            return setGradientTrans( nPropId, rValue );

        case ShapeProperty::FillBitmap:
            return setFillBitmap(nPropId, rValue);

        case ShapeProperty::FillBitmapName:
            return setFillBitmapName(rValue);

        case ShapeProperty::FillHatch:
            return setFillHatch( nPropId, rValue );

        default:;   // suppress compiler warnings
    }

    // set plain property value
    setAnyProperty( nPropId, rValue );
    return true;
}

// private --------------------------------------------------------------------

bool ShapePropertyMap::setLineMarker( sal_Int32 nPropId, const Any& rValue )
{
    NamedValue aNamedMarker;
    if( (rValue >>= aNamedMarker) && !aNamedMarker.Name.isEmpty() )
    {
        // push line marker explicitly
        if( !maShapePropInfo.mbNamedLineMarker )
            return setAnyProperty( nPropId, aNamedMarker.Value );

        // create named line marker (if coordinates have been passed) and push its name
        bool bInserted = !aNamedMarker.Value.has< PolyPolygonBezierCoords >() ||
            mrModelObjHelper.insertLineMarker( aNamedMarker.Name, aNamedMarker.Value.get< PolyPolygonBezierCoords >() );
        return bInserted && setProperty( nPropId, aNamedMarker.Name );
    }
    return false;
}

bool ShapePropertyMap::setLineDash( sal_Int32 nPropId, const Any& rValue )
{
    // push line dash explicitly
    if( !maShapePropInfo.mbNamedLineDash )
        return setAnyProperty( nPropId, rValue );

    // create named line dash and push its name
    if( rValue.has< LineDash >() )
    {
        OUString aDashName = mrModelObjHelper.insertLineDash( rValue.get< LineDash >() );
        return !aDashName.isEmpty() && setProperty( nPropId, aDashName );
    }

    return false;
}

bool ShapePropertyMap::setFillGradient( sal_Int32 nPropId, const Any& rValue )
{
    // push gradient explicitly
    if( !maShapePropInfo.mbNamedFillGradient )
        return setAnyProperty( nPropId, rValue );

    // create named gradient and push its name
    if( rValue.has< awt::Gradient >() )
    {
        OUString aGradientName = mrModelObjHelper.insertFillGradient( rValue.get< awt::Gradient >() );
        return !aGradientName.isEmpty() && setProperty( nPropId, aGradientName );
    }

    return false;
}

bool ShapePropertyMap::setFillHatch( sal_Int32 nPropId, const Any& rValue )
{
    // push hatch explicitly
    if( !maShapePropInfo.mbNamedFillHatch )
        return setAnyProperty( nPropId, rValue );

    // create named hatch and push its name
    if (rValue.has<drawing::Hatch>())
    {
        OUString aHatchName = mrModelObjHelper.insertFillHatch(rValue.get<drawing::Hatch>());
        return !aHatchName.isEmpty() && setProperty( nPropId, aHatchName );
    }

    return false;
}

bool ShapePropertyMap::setGradientTrans( sal_Int32 nPropId, const Any& rValue )
{
    // create named gradient and push its name
    if( rValue.has< awt::Gradient >() )
    {
        OUString aGradientName = mrModelObjHelper.insertTransGrandient( rValue.get< awt::Gradient >() );
        return !aGradientName.isEmpty()  && setProperty( nPropId, aGradientName );
    }

    return false;
}

bool ShapePropertyMap::setFillBitmap(sal_Int32 nPropId, const Any& rValue)
{
    // push bitmap explicitly
    if (!maShapePropInfo.mbNamedFillBitmap)
    {
        return setAnyProperty(nPropId, rValue);
    }

    // create named bitmap URL and push its name
    if (rValue.has<uno::Reference<graphic::XGraphic>>())
    {
        auto xGraphic = rValue.get<uno::Reference<graphic::XGraphic>>();
        OUString aBitmapName = mrModelObjHelper.insertFillBitmapXGraphic(xGraphic);
        return !aBitmapName.isEmpty() && setProperty(nPropId, aBitmapName);
    }

    return false;
}

bool ShapePropertyMap::setFillBitmapName(const Any& rValue)
{
    if (rValue.has<uno::Reference<graphic::XGraphic>>())
    {
        auto xGraphic = rValue.get<uno::Reference<graphic::XGraphic>>();
        OUString aBitmapUrlName = mrModelObjHelper.insertFillBitmapXGraphic(xGraphic);
        return !aBitmapUrlName.isEmpty() && setProperty(PROP_FillBitmapName, aBitmapUrlName);
    }
    return false;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
