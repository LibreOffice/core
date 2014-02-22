/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
                
                
                disposeWeakConnectionPoint();

                Reference<XInterface > xHoldAlive( *this );
                
                try
                {
                    dispose();
                }
                catch (::com::sun::star::uno::RuntimeException & exc)
                {
                    
#if OSL_DEBUG_LEVEL > 0
                    OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
                    OSL_FAIL( msg.getStr() );
#else
                    (void) exc; 
#endif
                }

                
                OSL_ASSERT( m_refCount == 1 );
                
                return;
            }
        }
        
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


void OComponentHelper::disposing()
{
}


void OComponentHelper::dispose()
    throw(::com::sun::star::uno::RuntimeException)
{
    
    
    
    Reference<XComponent > xSelf( this );

    
    
    bool bDoDispose = false;
    {
    MutexGuard aGuard( rBHelper.rMutex );
    if( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        
        rBHelper.bInDispose = sal_True;
        bDoDispose = true;
    }
    }

    
    if( bDoDispose )
    {
        
        try
        {
            try
            {
                Reference<XInterface > xSource(
                    Reference<XInterface >::query( (XComponent *)this ) );
                EventObject aEvt;
                aEvt.Source = xSource;
                
                
                rBHelper.aLC.disposeAndClear( aEvt );
                
                disposing();
            }
            catch (...)
            {
                MutexGuard aGuard( rBHelper.rMutex );
                
                rBHelper.bDisposed = sal_True;
                rBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard( rBHelper.rMutex );
            
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
                OUString(
                              "unexpected UNO exception caught: ") +
                exc.Message, Reference< XInterface >() );
        }
    }
    else
    {
        
        
        
        OSL_TRACE( "OComponentHelper::dispose() - dispose called twice" );
    }
}


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


void OComponentHelper::removeEventListener(
    const Reference<XEventListener > & rxListener )
    throw(::com::sun::star::uno::RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( &rxListener ) , rxListener );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
