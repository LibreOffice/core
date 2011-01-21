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

#include "oox/helper/modelobjecthelper.hxx"

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/helper.hxx"

namespace oox {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

ObjectContainer::ObjectContainer( const Reference< XMultiServiceFactory >& rxFactory, const OUString& rServiceName ) :
    mxFactory( rxFactory ),
    maServiceName( rServiceName ),
    mnIndex( 0 )
{
    OSL_ENSURE( mxFactory.is(), "ObjectContainer::ObjectContainer - missing service factory" );
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
    createContainer();
    if( mxContainer.is() ) try
    {
        return mxContainer->getByName( rObjName );
    }
    catch( Exception& )
    {
    }
    return Any();
}

OUString ObjectContainer::insertObject( const OUString& rObjName, const Any& rObj, bool bInsertByUnusedName )
{
    createContainer();
    if( mxContainer.is() )
    {
        if( bInsertByUnusedName )
            return ContainerHelper::insertByUnusedName( mxContainer, rObjName + OUString::valueOf( ++mnIndex ), ' ', rObj );
        if( ContainerHelper::insertByName( mxContainer, rObjName, rObj ) )
            return rObjName;
    }
    return OUString();
}

void ObjectContainer::createContainer() const
{
    if( !mxContainer.is() && mxFactory.is() ) try
    {
        mxContainer.set( mxFactory->createInstance( maServiceName ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( mxContainer.is(), "ObjectContainer::createContainer - container not found" );
}

// ============================================================================

ModelObjectHelper::ModelObjectHelper( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    maMarkerContainer(   rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.MarkerTable" ) ),
    maDashContainer(     rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.DashTable" ) ),
    maGradientContainer( rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.GradientTable" ) ),
    maBitmapContainer(   rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.BitmapTable" ) ),
    maDashNameBase( CREATE_OUSTRING( "msLineDash " ) ),
    maGradientNameBase( CREATE_OUSTRING( "msFillGradient " ) ),
    maBitmapNameBase( CREATE_OUSTRING( "msFillBitmap " ) )
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
        return maMarkerContainer.insertObject( rMarkerName, Any( rMarker ), false ).getLength() > 0;
    return false;
}

OUString ModelObjectHelper::insertLineDash( const LineDash& rDash )
{
    return maDashContainer.insertObject( maDashNameBase, Any( rDash ), true );
}

OUString ModelObjectHelper::insertFillGradient( const Gradient& rGradient )
{
    return maGradientContainer.insertObject( maGradientNameBase, Any( rGradient ), true );
}

OUString ModelObjectHelper::insertFillBitmap( const OUString& rGraphicUrl )
{
    if( rGraphicUrl.getLength() > 0 )
        return maBitmapContainer.insertObject( maBitmapNameBase, Any( rGraphicUrl ), true );
    return OUString();
}

// ============================================================================

} // namespace oox
