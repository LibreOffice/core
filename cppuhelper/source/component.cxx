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

#include <rtl/string.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/component.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

namespace cppu
{

//  ----------------------------------------------------
//  class OComponentHelper
//  ----------------------------------------------------

OComponentHelper::OComponentHelper( Mutex & rMutex ) SAL_THROW(())
    : rBHelper( rMutex )
{
}
OComponentHelper::~OComponentHelper() SAL_THROW( (RuntimeException) )
{
}

Any OComponentHelper::queryInterface( Type const & rType ) throw (RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}
Any OComponentHelper::queryAggregation( Type const & rType ) throw (RuntimeException)
{
    if (rType == ::getCppuType( (Reference< lang::XComponent > const *)0 ))
    {
        void * p = static_cast< lang::XComponent * >( this );
        return Any( &p, rType );
    }
    else if (rType == ::getCppuType( (Reference< lang::XTypeProvider > const *)0 ))
    {
        void * p = static_cast< lang::XTypeProvider * >( this );
        return Any( &p, rType );
    }
    return OWeakAggObject::queryAggregation( rType );
}
void OComponentHelper::acquire() throw ()
{
    OWeakAggObject::acquire();
}

void OComponentHelper::release() throw()
{
    Reference<XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_atomic_decrement( &m_refCount ) == 0)
        {
            if (! rBHelper.bDisposed)
            {
                // *before* again incrementing our ref count, ensure that our weak connection point
                // will not create references to us anymore (via XAdapter::queryAdapted)
                disposeWeakConnectionPoint();

                Reference<XInterface > xHoldAlive( *this );
                // First dispose
                try
                {
                    dispose();
                }
                catch (::com::sun::star::uno::RuntimeException & exc)
                {
                    // release should not throw exceptions
#if OSL_DEBUG_LEVEL > 0
                    OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
                    OSL_FAIL( msg.getStr() );
#else
                    (void) exc; // avoid warning about unused variable
#endif
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

Sequence< Type > OComponentHelper::getTypes() throw (RuntimeException)
{
    static OTypeCollection * s_pTypes = 0;
    if (! s_pTypes)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pTypes)
        {
            static OTypeCollection s_aTypes(
                ::getCppuType( (const Reference< lang::XComponent > *)0 ),
                ::getCppuType( (const Reference< lang::XTypeProvider > *)0 ),
                ::getCppuType( (const Reference< XAggregation > *)0 ),
                ::getCppuType( (const Reference< XWeak > *)0 ) );
            s_pTypes = &s_aTypes;
        }
    }
    return s_pTypes->getTypes();
}

// XComponent
void OComponentHelper::disposing()
{
}

// XComponent
void OComponentHelper::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it rubust, hold a self Reference.
    Reference<XComponent > xSelf( this );

    // Guard dispose against multible threading
    // Remark: It is an error to call dispose more than once
    sal_Bool bDoDispose = sal_False;
    {
    MutexGuard aGuard( rBHelper.rMutex );
    if( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        // only one call go into this section
        rBHelper.bInDispose = sal_True;
        bDoDispose = sal_True;
    }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            try
            {
                Reference<XInterface > xSource(
                    Reference<XInterface >::query( (XComponent *)this ) );
                EventObject aEvt;
                aEvt.Source = xSource;
                // inform all listeners to release this object
                // The listener container are automaticly cleared
                rBHelper.aLC.disposeAndClear( aEvt );
                // notify subclasses to do their dispose
                disposing();
            }
            catch (...)
            {
                MutexGuard aGuard( rBHelper.rMutex );
                // bDispose and bInDisposing must be set in this order:
                rBHelper.bDisposed = sal_True;
                rBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard( rBHelper.rMutex );
            // bDispose and bInDisposing must be set in this order:
            rBHelper.bDisposed = sal_True;
            rBHelper.bInDispose = sal_False;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception & exc)
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "unexpected UNO exception caught: ") ) +
                exc.Message, Reference< XInterface >() );
        }
    }
    else
    {
        // in a multithreaded environment, it can't be avoided
        // that dispose is called twice.
        // However this condition is traced, because it MAY indicate an error.
        OSL_TRACE( "OComponentHelper::dispose() - dispose called twice" );
    }
}

// XComponent
void OComponentHelper::addEventListener(
    const Reference<XEventListener > & rxListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        Reference< XInterface > x( (XComponent *)this, UNO_QUERY );
        rxListener->disposing( EventObject( x ) );
    }
    else
    {
        rBHelper.addListener( ::getCppuType( &rxListener ) , rxListener );
    }
}

// XComponent
void OComponentHelper::removeEventListener(
    const Reference<XEventListener > & rxListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( &rxListener ) , rxListener );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
