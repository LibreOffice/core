/*************************************************************************
 *
 *  $RCSfile: component.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:10 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::lang;

namespace cppu
{

//  ----------------------------------------------------
//  class OComponentHelper
//  ----------------------------------------------------
OComponentHelper::OComponentHelper( Mutex & rMutex )
    : rBHelper( rMutex )
{
}

/**
 * Call dispose if not previous called.
 */
OComponentHelper::~OComponentHelper()
{
}

// XInterface
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
        osl_incrementInterlockedCount( &m_refCount );
    }
    OWeakAggObject::release();
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

    OSL_ENSHURE( bDoDispose, "dispose called twice" );
    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            Reference<XInterface > xSource( Reference<XInterface >::query( (XComponent *)this ) );
            EventObject aEvt;
            aEvt.Source = xSource;
            // inform all listeners to release this object
            // The listener container are automaticly cleared
            rBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(::com::sun::star::uno::Exception& e)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            rBHelper.bDisposed = sal_True;
            rBHelper.bInDispose = sal_False;
            throw e;
        }

        // the values bDispose and bInDisposing must set in this order.
        // No multithread call overcome the "!rBHelper.bDisposed && !rBHelper.bInDispose" guard.
        rBHelper.bDisposed = sal_True;
        rBHelper.bInDispose = sal_False;
    }
}

// XComponent
void OComponentHelper::addEventListener(const Reference<XEventListener > & rxListener)
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSHURE( !rBHelper.bInDispose, "do not add listeners in the dispose call" );
    OSL_ENSHURE( !rBHelper.bDisposed, "object is disposed" );
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
        // only add listeners if you are not disposed
        rBHelper.aLC.addInterface( ::getCppuType( (Reference< XEventListener > *)0 ), rxListener );
}

// XComponent
void OComponentHelper::removeEventListener(const Reference<XEventListener > & rxListener)
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( rBHelper.rMutex );
    OSL_ENSHURE( !rBHelper.bDisposed, "object is disposed" );
    // all listeners are automaticly released in a dispose call
    if( !rBHelper.bInDispose && !rBHelper.bDisposed )
        rBHelper.aLC.removeInterface( ::getCppuType( (Reference< XEventListener > *)0 ), rxListener );
}

}

