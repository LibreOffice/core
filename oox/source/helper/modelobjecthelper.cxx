/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include "oox/helper/helper.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::awt::Gradient;
using ::com::sun::star::drawing::LineDash;
using ::com::sun::star::drawing::PolyPolygonBezierCoords;

namespace oox {

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
