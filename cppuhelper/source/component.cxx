/*************************************************************************
 *
 *  $RCSfile: component.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-11 10:41:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace cppu
{

//  ----------------------------------------------------
//  class OComponentHelper
//  ----------------------------------------------------

OComponentHelper::OComponentHelper( Mutex & rMutex ) SAL_THROW( () )
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
        if (osl_decrementInterlockedCount( &m_refCount ) == 0)
        {
            if (! rBHelper.bDisposed)
            {
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
                    OSL_ENSURE( 0, msg.getStr() );
#endif
                }

                // only the alive ref holds the object
                OSL_ASSERT( m_refCount == 1 );
                // destroy the object if xHoldAlive decrement the refcount to 0
                return;
            }
        }
        // restore the reference count
        osl_incrementInterlockedCount( &m_refCount );
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
        // in a multithreaded environment, it can't be avoided,
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

