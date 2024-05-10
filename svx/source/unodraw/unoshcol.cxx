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
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <shapecollection.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

SvxShapeCollection::SvxShapeCollection() noexcept
{
}

// XInterface
void SvxShapeCollection::release() noexcept
{
    uno::Reference< uno::XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_atomic_decrement( &m_refCount ) == 0)
        {
            if (! bDisposed)
            {
                uno::Reference< uno::XInterface > xHoldAlive( getXWeak() );
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
        std::unique_lock aGuard( m_aMutex );
        if( !bDisposed && !bInDispose )
        {
            // only one call go into this section
            bInDispose = true;
            bDoDispose = true;
        }
    }

    // Do not hold the mutex because we are broadcasting
    if( bDoDispose )
    {
        // Create an event with this as sender
        try
        {
            document::EventObject aEvt;
            aEvt.Source = uno::Reference< uno::XInterface >::query( static_cast<lang::XComponent *>(this) );
            // inform all listeners to release this object
            // The listener container are automatically cleared
            std::unique_lock g(m_aMutex);
            maEventListeners.disposeAndClear( g, aEvt );
            maShapeContainer.clear();
        }
        catch(const css::uno::Exception&)
        {
            // catch exception and throw again but signal that
            // the object was disposed. Dispose should be called
            // only once.
            bDisposed = true;
            bInDispose = false;
            throw;
        }

        // the values bDispose and bInDisposing must set in this order.
        // No multithread call overcome the "!rBHelper.bDisposed && !rBHelper.bInDispose" guard.
        bDisposed = true;
        bInDispose = false;
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
    std::unique_lock g(m_aMutex);
    maEventListeners.addInterface( g, aListener );
}

// XComponent
void SAL_CALL SvxShapeCollection::removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener )
{
    std::unique_lock g(m_aMutex);
    maEventListeners.removeInterface( g, aListener );
}

// XShapes

void SAL_CALL SvxShapeCollection::add( const Reference< drawing::XShape >& xShape )
{
    std::unique_lock g(m_aMutex);
    maShapeContainer.push_back( xShape );
}


void SAL_CALL SvxShapeCollection::remove( const uno::Reference< drawing::XShape >& xShape )
{
    std::unique_lock g(m_aMutex);
    std::erase(maShapeContainer, xShape);
}


sal_Int32 SAL_CALL SvxShapeCollection::getCount()
{
    std::unique_lock g(m_aMutex);
    return maShapeContainer.size();
}


uno::Any SAL_CALL SvxShapeCollection::getByIndex( sal_Int32 Index )
{
    if( Index < 0 || Index >= getCount() )
        throw lang::IndexOutOfBoundsException();

    std::unique_lock g(m_aMutex);
    Reference<drawing::XShape> xShape = maShapeContainer[Index];
    return uno::Any( xShape );
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
    return u"com.sun.star.drawing.SvxShapeCollection"_ustr;
}

sal_Bool SAL_CALL SvxShapeCollection::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL SvxShapeCollection::getSupportedServiceNames()
{
    return { u"com.sun.star.drawing.Shapes"_ustr, u"com.sun.star.drawing.ShapeCollection"_ustr };
}

void SvxShapeCollection::getAllShapes(std::vector<css::uno::Reference<css::drawing::XShape>>& rShapes) const
{
    rShapes = maShapeContainer;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_drawing_SvxShapeCollection_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvxShapeCollection);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
