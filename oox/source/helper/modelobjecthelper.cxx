/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

// ============================================================================

ModelObjectHelper::ModelObjectHelper( const Reference< XMultiServiceFactory >& rxModelFactory ) :
    maMarkerContainer(    rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.MarkerTable" ) ),
    maDashContainer(      rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.DashTable" ) ),
    maGradientContainer(  rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.GradientTable" ) ),
    maBitmapUrlContainer( rxModelFactory, CREATE_OUSTRING( "com.sun.star.drawing.BitmapTable" ) ),
    maDashNameBase(      CREATE_OUSTRING( "msLineDash " ) ),
    maGradientNameBase(  CREATE_OUSTRING( "msFillGradient " ) ),
    maBitmapUrlNameBase( CREATE_OUSTRING( "msFillBitmap " ) )
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

OUString ModelObjectHelper::insertFillBitmapUrl( const OUString& rGraphicUrl )
{
    if( rGraphicUrl.getLength() > 0 )
        return maBitmapUrlContainer.insertObject( maBitmapUrlNameBase, Any( rGraphicUrl ), true );
    return OUString();
}

// ============================================================================

} // namespace oox
