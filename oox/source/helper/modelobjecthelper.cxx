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

#include <oox/helper/modelobjecthelper.hxx>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/Hatch.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <oox/helper/containerhelper.hxx>
#include <osl/diagnose.h>

namespace oox {

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

ObjectContainer::ObjectContainer( const Reference< XMultiServiceFactory >& rxModelFactory, const OUString& rServiceName ) :
    mxModelFactory( rxModelFactory ),
    maServiceName( rServiceName ),
    mnIndex( 0 )
{
    OSL_ENSURE( mxModelFactory.is(), "ObjectContainer::ObjectContainer - missing service factory" );
}

ObjectContainer::~ObjectContainer()
{
}

bool ObjectContainer::hasObject( const OUString& rObjName ) const
{
    createContainer();
    return mxContainer.is() && mxContainer->hasByName( rObjName );
}

Any ObjectContainer::getObject( const OUString& rObjName ) const
{
    if( hasObject( rObjName ) )
        return mxContainer->getByName( rObjName );
    return Any();
}

OUString ObjectContainer::insertObject( const OUString& rObjName, const Any& rObj, bool bInsertByUnusedName )
{
    createContainer();
    if( mxContainer.is() )
    {
        if( bInsertByUnusedName )
            return ContainerHelper::insertByUnusedName( mxContainer, rObjName + OUString::number( ++mnIndex ), ' ', rObj );
        if( ContainerHelper::insertByName( mxContainer, rObjName, rObj ) )
            return rObjName;
    }
    return OUString();
}

void ObjectContainer::createContainer() const
{
    if( !mxContainer.is() && mxModelFactory.is() ) try
    {
        mxContainer.set( mxModelFactory->createInstance( maServiceName ), UNO_QUERY_THROW );
        mxModelFactory.clear();
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxContainer.is(), "ObjectContainer::createContainer - container not found" );
}

constexpr OUStringLiteral gaDashNameBase(      u"msLineDash " );      ///< Base name for all named line dashes.
constexpr OUStringLiteral gaGradientNameBase(  u"msFillGradient " );  ///< Base name for all named fill gradients.
constexpr OUStringLiteral gaTransGradNameBase( u"msTransGradient " ); ///< Base name for all named fill gradients.
constexpr OUStringLiteral gaBitmapUrlNameBase( u"msFillBitmap " );    ///< Base name for all named fill bitmap URLs.
constexpr OUStringLiteral gaHatchNameBase(     u"msFillHatch " );     ///< Base name for all named fill hatches.

ModelObjectHelper::ModelObjectHelper( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    maMarkerContainer(    rxModelFactory, "com.sun.star.drawing.MarkerTable" ),
    maDashContainer(      rxModelFactory, "com.sun.star.drawing.DashTable" ),
    maGradientContainer(  rxModelFactory, "com.sun.star.drawing.GradientTable" ),
    maTransGradContainer( rxModelFactory, "com.sun.star.drawing.TransparencyGradientTable" ),
    maBitmapUrlContainer( rxModelFactory, "com.sun.star.drawing.BitmapTable" ),
    maHatchContainer(     rxModelFactory, "com.sun.star.drawing.HatchTable" )
{
}

bool ModelObjectHelper::hasLineMarker( const OUString& rMarkerName ) const
{
    return maMarkerContainer.hasObject( rMarkerName );
}

bool ModelObjectHelper::insertLineMarker( const OUString& rMarkerName, const PolyPolygonBezierCoords& rMarker )
{
    OSL_ENSURE( rMarker.Coordinates.hasElements(), "ModelObjectHelper::insertLineMarker - line marker without coordinates" );
    if( rMarker.Coordinates.hasElements() )
        return !maMarkerContainer.insertObject( rMarkerName, Any( rMarker ), false ).isEmpty();
    return false;
}

OUString ModelObjectHelper::insertLineDash( const LineDash& rDash )
{
    return maDashContainer.insertObject( gaDashNameBase, Any( rDash ), true );
}

OUString ModelObjectHelper::insertFillGradient( const awt::Gradient& rGradient )
{
    return maGradientContainer.insertObject( gaGradientNameBase, Any( rGradient ), true );
}

OUString ModelObjectHelper::insertTransGrandient( const awt::Gradient& rGradient )
{
    return maTransGradContainer.insertObject( gaTransGradNameBase, Any( rGradient ), true );
}

OUString ModelObjectHelper::insertFillBitmapXGraphic(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    uno::Reference<awt::XBitmap> xBitmap(rxGraphic, uno::UNO_QUERY);
    if (xBitmap.is())
        return maBitmapUrlContainer.insertObject(gaBitmapUrlNameBase, Any(xBitmap), true);
    return OUString();
}

OUString ModelObjectHelper::insertFillHatch(const drawing::Hatch& rHatch)
{
    return maHatchContainer.insertObject( gaHatchNameBase, Any( rHatch ), true );
}

uno::Reference<awt::XBitmap> ModelObjectHelper::getFillBitmap(OUString const & rGraphicName)
{
    uno::Reference<awt::XBitmap> xBitmap;
    uno::Any aAny = maBitmapUrlContainer.getObject(rGraphicName);
    if (aAny.has<uno::Reference<awt::XBitmap>>())
        xBitmap = aAny.get<uno::Reference<awt::XBitmap>>();
    return xBitmap;
}

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
