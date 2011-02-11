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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "EnhancedCustomShapeHandle.hxx"
#include "svx/EnhancedCustomShape2d.hxx"
#include "svx/unoapi.hxx"

// -----------------------------------------------------------------------------

EnhancedCustomShapeHandle::EnhancedCustomShapeHandle( com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& xCustomShape, sal_uInt32 nIndex ) :
    mnIndex     ( nIndex ),
    mxCustomShape ( xCustomShape )
{
}

// -----------------------------------------------------------------------------

EnhancedCustomShapeHandle::~EnhancedCustomShapeHandle()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL EnhancedCustomShapeHandle::acquire() throw()
{
    OWeakObject::acquire();
}

// -----------------------------------------------------------------------------

void SAL_CALL EnhancedCustomShapeHandle::release() throw()
{
    OWeakObject::release();
}

// XCustomShapeHandle
com::sun::star::awt::Point SAL_CALL EnhancedCustomShapeHandle::getPosition()
    throw ( com::sun::star::uno::RuntimeException )
{
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxCustomShape ) );
    if ( !pSdrObjCustomShape )
        throw com::sun::star::uno::RuntimeException();

    Point aPosition;
    EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
    if ( !aCustomShape2d.GetHandlePosition( mnIndex, aPosition ) )
        throw com::sun::star::uno::RuntimeException();
    return com::sun::star::awt::Point( aPosition.X(), aPosition.Y() );
}

void SAL_CALL EnhancedCustomShapeHandle::setControllerPosition( const com::sun::star::awt::Point& aPnt )
    throw ( com::sun::star::uno::RuntimeException )
{
    SdrObject* pSdrObjCustomShape( GetSdrObjectFromXShape( mxCustomShape ) );
    if ( !pSdrObjCustomShape )
        throw com::sun::star::uno::RuntimeException();

    EnhancedCustomShape2d aCustomShape2d( pSdrObjCustomShape );
    if ( !aCustomShape2d.SetHandleControllerPosition( mnIndex, aPnt ) )
        throw com::sun::star::uno::RuntimeException();
}

// XInitialization
void SAL_CALL EnhancedCustomShapeHandle::initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any >& /* aArguments */ )
    throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException )
{
}
