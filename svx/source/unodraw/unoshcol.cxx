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

#include <com/sun/star/document/EventObject.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/mutex.hxx>
#include <rtl/ref.hxx>
#include <svx/unoprov.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace {

class SvxShapeCollectionMutex
{
public:
    ::osl::Mutex maMutex;
};

class SvxShapeCollection :
    public cppu::WeakAggImplHelper3<drawing::XShapes, lang::XServiceInfo, lang::XComponent>,
    public SvxShapeCollectionMutex
{
private:
    cppu::OInterfaceContainerHelper maShapeContainer;

    cppu::OBroadcastHelper mrBHelper;

    void disposing() throw();

public:
    SvxShapeCollection() throw();
    virtual ~SvxShapeCollection() throw();

    // XInterface
    virtual void SAL_CALL release() throw() override;

    // XComponent
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(css::uno::RuntimeException, std::exception) override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) throw(css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};

SvxShapeCollection::SvxShapeCollection() throw()
: maShapeContainer( maMutex ), mrBHelper( maMutex )
{
}


SvxShapeCollection::~SvxShapeCollection() throw()
{
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
                uno::Reference< uno::XInterface > xHoldAlive( static_cast<uno::XWeak*>(this) );
                // First dispose
                try
                {
                    dispose();
                }
                catch(css::uno::Exception&)
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
    throw(css::uno::RuntimeException, std::exception)
{
    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it rubust, hold a self Reference.
    uno::Reference< lang::XComponent > xSelf( this );

    // Guard dispose against multible threading
    // Remark: It is an error to call dispose more than once
    bool bDoDispose = false;
    {
    osl::MutexGuard aGuard( mrBHelper.rMutex );
    if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
    {
        // only one call go into this section
        mrBHelper.bInDispose = sal_True;
        bDoDispose = true;
    }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            uno::Reference< uno::XInterface > xSource( uno::Reference< uno::XInterface >::query( static_cast<lang::XComponent *>(this) ) );
            document::EventObject aEvt;
            aEvt.Source = xSource;
            // inform all listeners to release this object
            // The listener container are automatically cleared
            mrBHelper.aLC.disposeAndClear( aEvt );
            // notify subclasses to do their dispose
            disposing();
        }
        catch(const css::uno::Exception&)
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
void SAL_CALL SvxShapeCollection::addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception)
{
    mrBHelper.addListener( cppu::UnoType<decltype(aListener)>::get() , aListener );
}

// XComponent
void SAL_CALL SvxShapeCollection::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw(css::uno::RuntimeException, std::exception)
{
    mrBHelper.removeListener( cppu::UnoType<decltype(aListener)>::get() , aListener );
}

// XShapes

void SAL_CALL SvxShapeCollection::add( const Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException, std::exception )
{
    maShapeContainer.addInterface( xShape );
}


void SAL_CALL SvxShapeCollection::remove( const uno::Reference< drawing::XShape >& xShape ) throw( uno::RuntimeException, std::exception )
{
    maShapeContainer.removeInterface( xShape );
}


sal_Int32 SAL_CALL SvxShapeCollection::getCount() throw( uno::RuntimeException, std::exception )
{
    return maShapeContainer.getLength();
}


uno::Any SAL_CALL SvxShapeCollection::getByIndex( sal_Int32 Index )
    throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    uno::Sequence< Reference< uno::XInterface> > xElements( maShapeContainer.getElements() );


    return uno::makeAny( Reference< drawing::XShape>(static_cast< drawing::XShape* >( xElements.getArray()[Index].get())) );
}

// XElementAccess
uno::Type SAL_CALL SvxShapeCollection::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SAL_CALL SvxShapeCollection::hasElements() throw( uno::RuntimeException, std::exception )
{
    return getCount() != 0;
}

// XServiceInfo
OUString SAL_CALL SvxShapeCollection::getImplementationName()
    throw( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.drawing.SvxShapeCollection");
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const OUString& ServiceName )
    throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService( this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aSeq(2);
    aSeq.getArray()[0] = "com.sun.star.drawing.Shapes";
    aSeq.getArray()[1] = "com.sun.star.drawing.ShapeCollection";
    return aSeq;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_drawing_SvxShapeCollection_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvxShapeCollection);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
