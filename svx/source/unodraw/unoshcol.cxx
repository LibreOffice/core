/*************************************************************************
 *
 *  $RCSfile: unoshcol.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-27 11:58:51 $
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

#ifndef _COM_SUN_STAR_DOCUMENT_EVENTOBJECT_HPP_
#include <com/sun/star/document/EventObject.hpp>
#endif

#include "unoshcol.hxx"
#include "unoprov.hxx"

using namespace ::rtl;
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
        if (osl_decrementInterlockedCount( &m_refCount ) == 0)
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
        osl_incrementInterlockedCount( &m_refCount );
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
        catch(::com::sun::star::uno::Exception& e)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            mrBHelper.bDisposed = sal_True;
            mrBHelper.bInDispose = sal_False;
            throw e;
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

    Reference< drawing::XShape > xShape( xElements.getArray()[Index], UNO_QUERY );
    return uno::Any( &xShape, getElementType() );
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
OUString SAL_CALL SvxShapeCollection::getImplementationName()
    throw( uno::RuntimeException )
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SvxShapeCollection") );
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException )
{
    return SvxServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

uno::Sequence< OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames() throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aSeq(1);
    aSeq.getArray()[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shapes") );
    return aSeq;
}


