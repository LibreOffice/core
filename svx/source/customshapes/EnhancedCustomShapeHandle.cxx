/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnhancedCustomShapeHandle.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:07:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _ENHANCED_CUSTOMSHAPE_HANDLE_HXX
#include "EnhancedCustomShapeHandle.hxx"
#endif
#ifndef _ENHANCEDCUSTOMSHAPE2D_HXX
#include "EnhancedCustomShape2d.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif

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
