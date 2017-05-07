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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/interfacecontainer2.hxx>
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
    comphelper::OInterfaceContainerHelper2 maShapeContainer;

    cppu::OBroadcastHelper mrBHelper;

public:
    SvxShapeCollection() throw();

    // XInterface
    virtual void SAL_CALL release() throw() override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override ;
    virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

    // XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // XShapes
    virtual void SAL_CALL add( const css::uno::Reference< css::drawing::XShape >& xShape ) override;
    virtual void SAL_CALL remove( const css::uno::Reference< css::drawing::XShape >& xShape ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
};

SvxShapeCollection::SvxShapeCollection() throw()
: maShapeContainer( maMutex ), mrBHelper( maMutex )
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
void SvxShapeCollection::dispose()
{
    // An frequently programming error is to release the last
    // reference to this object in the disposing message.
    // Make it robust, hold a self Reference.
    uno::Reference< lang::XComponent > xSelf( this );

    // Guard dispose against multiple threading
    // Remark: It is an error to call dispose more than once
    bool bDoDispose = false;
    {
    osl::MutexGuard aGuard( mrBHelper.rMutex );
    if( !mrBHelper.bDisposed && !mrBHelper.bInDispose )
    {
        // only one call go into this section
        mrBHelper.bInDispose = true;
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
            maShapeContainer.clear();
        }
        catch(const css::uno::Exception&)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            mrBHelper.bDisposed = true;
            mrBHelper.bInDispose = false;
            throw;
        }

        // the values bDispose and bInDisposing must set in this order.
        // No multithread call overcome the "!rBHelper.bDisposed && !rBHelper.bInDispose" guard.
        mrBHelper.bDisposed = true;
        mrBHelper.bInDispose = false;
    }
    else
    {
        // in a multithreaded environment, it can't be avoided, that dispose is called twice.
        // However this condition is traced, because it MAY indicate an error.
        SAL_INFO("svx", "dispose called twice" );
    }
}

// XComponent
void SAL_CALL SvxShapeCollection::addEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
{
    mrBHelper.addListener( cppu::UnoType<decltype(aListener)>::get() , aListener );
}

// XComponent
void SAL_CALL SvxShapeCollection::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
{
    mrBHelper.removeListener( cppu::UnoType<decltype(aListener)>::get() , aListener );
}

// XShapes

void SAL_CALL SvxShapeCollection::add( const Reference< drawing::XShape >& xShape )
{
    maShapeContainer.addInterface( xShape );
}


void SAL_CALL SvxShapeCollection::remove( const uno::Reference< drawing::XShape >& xShape )
{
    maShapeContainer.removeInterface( xShape );
}


sal_Int32 SAL_CALL SvxShapeCollection::getCount()
{
    return maShapeContainer.getLength();
}


uno::Any SAL_CALL SvxShapeCollection::getByIndex( sal_Int32 Index )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    std::vector< Reference< uno::XInterface> > aElements( maShapeContainer.getElements() );


    return uno::makeAny( Reference< drawing::XShape>(static_cast< drawing::XShape* >( aElements[Index].get())) );
}

// XElementAccess
uno::Type SAL_CALL SvxShapeCollection::getElementType()
{
    return cppu::UnoType<drawing::XShape>::get();
}

sal_Bool SAL_CALL SvxShapeCollection::hasElements()
{
    return getCount() != 0;
}

// XServiceInfo
OUString SAL_CALL SvxShapeCollection::getImplementationName()
{
    return OUString("com.sun.star.drawing.SvxShapeCollection");
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames()
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
