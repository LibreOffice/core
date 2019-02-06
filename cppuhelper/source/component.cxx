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

#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace cppu
{


//  class OComponentHelper


OComponentHelper::OComponentHelper( Mutex & rMutex )
    : rBHelper( rMutex )
{
}
OComponentHelper::~OComponentHelper()
{
}

Any OComponentHelper::queryInterface( Type const & rType )
{
    return OWeakAggObject::queryInterface( rType );
}
Any OComponentHelper::queryAggregation( Type const & rType )
{
    if (rType == cppu::UnoType<lang::XComponent>::get())
    {
        void * p = static_cast< lang::XComponent * >( this );
        return Any( &p, rType );
    }
    if (rType == cppu::UnoType<lang::XTypeProvider>::get())
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
                catch (css::uno::RuntimeException & exc)
                {
                    // release should not throw exceptions
                    SAL_WARN( "cppuhelper", exc );
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

Sequence< Type > OComponentHelper::getTypes()
{
    static OTypeCollection s_aTypes(
        cppu::UnoType<lang::XComponent>::get(),
        cppu::UnoType<lang::XTypeProvider>::get(),
        cppu::UnoType<XAggregation>::get(),
        cppu::UnoType<XWeak>::get() );

    return s_aTypes.getTypes();
}

// XComponent
void OComponentHelper::disposing()
{
}

// XComponent
void OComponentHelper::dispose()
{
    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it robust, hold a self Reference.
    Reference<XComponent > xSelf( this );

    // Guard dispose against multiple threading
    // Remark: It is an error to call dispose more than once
    bool bDoDispose = false;
    {
        MutexGuard aGuard( rBHelper.rMutex );
        if( !rBHelper.bDisposed && !rBHelper.bInDispose )
        {
            // only one call go into this section
            rBHelper.bInDispose = true;
            bDoDispose = true;
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
                    Reference<XInterface >::query( static_cast<XComponent *>(this) ) );
                EventObject aEvt;
                aEvt.Source = xSource;
                // inform all listeners to release this object
                // The listener container are automatically cleared
                rBHelper.aLC.disposeAndClear( aEvt );
                // notify subclasses to do their dispose
                disposing();
            }
            catch (...)
            {
                MutexGuard aGuard( rBHelper.rMutex );
                // bDispose and bInDisposing must be set in this order:
                rBHelper.bDisposed = true;
                rBHelper.bInDispose = false;
                throw;
            }
            MutexGuard aGuard( rBHelper.rMutex );
            // bDispose and bInDisposing must be set in this order:
            rBHelper.bDisposed = true;
            rBHelper.bInDispose = false;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception & exc)
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw lang::WrappedTargetRuntimeException(
                "unexpected UNO exception caught: " + exc.Message,
                nullptr, anyEx );
        }
    }
    else
    {
        // in a multithreaded environment, it can't be avoided
        // that dispose is called twice.
        // However this condition is traced, because it MAY indicate an error.
        SAL_WARN("cppuhelper",  "OComponentHelper::dispose() - dispose called twice" );
    }
}

// XComponent
void OComponentHelper::addEventListener(
    const Reference<XEventListener > & rxListener )
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        Reference< XInterface > x( static_cast<XComponent *>(this), UNO_QUERY );
        rxListener->disposing( EventObject( x ) );
    }
    else
    {
        rBHelper.addListener( cppu::UnoType<decltype(rxListener)>::get(), rxListener );
    }
}

// XComponent
void OComponentHelper::removeEventListener(
    const Reference<XEventListener > & rxListener )
{
    rBHelper.removeListener( cppu::UnoType<decltype(rxListener)>::get(), rxListener );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
