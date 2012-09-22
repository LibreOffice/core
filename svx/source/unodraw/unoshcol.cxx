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

#include <com/sun/star/document/EventObject.hpp>

#include "svx/unoshcol.hxx"
#include <svx/unoprov.hxx>
#include <comphelper/serviceinfohelper.hxx>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::drawing;

/***********************************************************************
*                                                                      *
***********************************************************************/
SvxShapeCollection::SvxShapeCollection() throw()
: maShapeContainer( maMutex ), mrBHelper( maMutex )
{
}

//----------------------------------------------------------------------
SvxShapeCollection::~SvxShapeCollection() throw()
{
}


//----------------------------------------------------------------------
Reference< uno::XInterface > SvxShapeCollection_NewInstance() throw()
{
    Reference< drawing::XShapes > xShapes( new SvxShapeCollection() );
    Reference< uno::XInterface > xRef( xShapes, UNO_QUERY );
    return xRef;
}

// XInterface
void SvxShapeCollection::release() throw()
{
    uno::Reference< uno::XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_atomic_decrement( &m_refCount ) == 0)
        {
            if (! mrBHelper.bDisposed)
            {
                uno::Reference< uno::XInterface > xHoldAlive( (uno::XWeak*)this );
                // First dispose
                try
                {
                    dispose();
                }
                catch(::com::sun::star::uno::Exception&)
                {
                    // release should not throw exceptions
                }

                // only the alive ref holds the object
                OSL_ASSERT( m_refCount == 1 );
                // destroy the object if xHoldAlive decrement the refcount to 0
                return;
            }
        }
        // restore the reference count
        osl_atomic_increment( &m_refCount );
    }
    OWeakAggObject::release();
}

// XComponent
void SvxShapeCollection::disposing() throw()
{
    maShapeContainer.clear();
}

// XComponent
void SvxShapeCollection::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it rubust, hold a self Reference.
    uno::Reference< lang::XComponent > xSelf( this );

    // Guard dispose against multible threading
    // Remark: It is an error to call dispose more than once
    sal_Bool bDoDispose = sal_False;
    {
    osl::MutexGuard aGuard( mrBHelper.rMutex );
    if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
    {
        // only one call go into this section
        mrBHelper.bInDispose = sal_True;
        bDoDispose = sal_True;
    }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            uno::Reference< uno::XInterface > xSource( uno::Reference< uno::XInterface >::query( (lang::XComponent *)this ) );
            document::EventObject aEvt;
            aEvt.Source = xSource;
            // inform all listeners to release this object
            // The listener container are automaticly cleared
            mrBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(const ::com::sun::star::uno::Exception&)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
            throw;
        }

        // the values bDispose and bInDisposing must set in this order.
        // No multithread call overcome the "!rBHelper.bDisposed && !rBHelper.bInDispose" guard.
        mrBHelper.bDisposed = sal_True;
        mrBHelper.bInDispose = sal_False;
    }
    else
    {
        // in a multithreaded environment, it can't be avoided, that dispose is called twice.
        // However this condition is traced, because it MAY indicate an error.
        OSL_TRACE( "OComponentHelper::dispose() - dispose called twice" );
    }
}

// XComponent
void SAL_CALL SvxShapeCollection::addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    mrBHelper.addListener( ::getCppuType( &aListener ) , aListener );
}

// XComponent
void SAL_CALL SvxShapeCollection::removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException)
{
    mrBHelper.removeListener( ::getCppuType( &aListener ) , aListener );
}

// XShapes
//----------------------------------------------------------------------
void SAL_CALL SvxShapeCollection::add( const Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException )
{
    maShapeContainer.addInterface( xShape );
}

//----------------------------------------------------------------------
void SAL_CALL SvxShapeCollection::remove( const uno::Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException )
{
    maShapeContainer.removeInterface( xShape );
}

//----------------------------------------------------------------------
sal_Int32 SAL_CALL SvxShapeCollection::getCount() throw( uno::RuntimeException )
{
    return maShapeContainer.getLength();
}

//----------------------------------------------------------------------
uno::Any SAL_CALL SvxShapeCollection::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence< Reference< uno::XInterface> > xElements( maShapeContainer.getElements() );


    return uno::makeAny( Reference< XShape>(static_cast< drawing::XShape* >( xElements.getArray()[Index].get())) );
}

// XElementAccess

//----------------------------------------------------------------------
uno::Type SAL_CALL SvxShapeCollection::getElementType() throw( uno::RuntimeException )
{
    return ::getCppuType(( const Reference< drawing::XShape >*)0);
}

//----------------------------------------------------------------------
sal_Bool SAL_CALL SvxShapeCollection::hasElements() throw( uno::RuntimeException )
{
    return getCount() != 0;
}

//----------------------------------------------------------------------
// XServiceInfo
//----------------------------------------------------------------------
::rtl::OUString SAL_CALL SvxShapeCollection::getImplementationName()
    throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

::rtl::OUString SvxShapeCollection::getImplementationName_Static()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.SvxShapeCollection") );
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const ::rtl::OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< ::rtl::OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< ::rtl::OUString > SvxShapeCollection::getSupportedServiceNames_Static()
{
    uno::Sequence< ::rtl::OUString > aSeq(2);
    aSeq.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shapes") );
    aSeq.getArray()[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ShapeCollection") );
    return aSeq;
}

Reference< XInterface > SAL_CALL SvxShapeCollection_createInstance( const Reference< ::com::sun::star::lang::XMultiServiceFactory >& )
{
    return *( new SvxShapeCollection() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
