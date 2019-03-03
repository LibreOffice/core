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

#include <sal/config.h>
#include <sal/log.hxx>

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <com/sun/star/lang/DisposedException.hpp>

#include <algorithm>
#include <vector>

using namespace osl;
using namespace com::sun::star::uno;

/** */ //for docpp
namespace cppu
{

// due to static Reflection destruction from usr, there must be a mutex leak (#73272#)
// this is used to lock all instances of OWeakConnectionPoint and OWeakRefListener as well as OWeakObject::m_pWeakConnectionPoint
static Mutex & getWeakMutex()
{
    static Mutex * s_pMutex = new Mutex();
    return *s_pMutex;
}


//-- OWeakConnectionPoint ----------------------------------------------------

class OWeakConnectionPoint: public XAdapter
{
public:
    /**
        Hold the weak object without an acquire (only the pointer).
     */
    explicit OWeakConnectionPoint( OWeakObject* pObj )
        : m_aRefCount( 0 )
        , m_pObject(pObj)
    {}

    // noncopyable
    OWeakConnectionPoint(const OWeakConnectionPoint&) = delete;
    const OWeakConnectionPoint& operator=(const OWeakConnectionPoint&) = delete;

    // XInterface
    Any SAL_CALL        queryInterface( const Type & rType ) override;
    void SAL_CALL       acquire() throw() override;
    void SAL_CALL       release() throw() override;

    // XAdapter
    css::uno::Reference< css::uno::XInterface > SAL_CALL queryAdapted() override;
    void SAL_CALL addReference( const css::uno::Reference< css::uno::XReference >& xRef ) override;
    void SAL_CALL removeReference( const css::uno::Reference< css::uno::XReference >& xRef ) override;

    /// Called from the weak object if the reference count goes to zero.
    ///
    /// @throws css::uno::RuntimeException
    void dispose();

private:
    virtual ~OWeakConnectionPoint() {}

    /// The reference counter.
    oslInterlockedCount         m_aRefCount;
    /// The weak object
    OWeakObject*                m_pObject;
    /// The container to hold the weak references
    std::vector<Reference<XReference>>  m_aReferences;
};

// XInterface
Any SAL_CALL OWeakConnectionPoint::queryInterface( const Type & rType )
{
    return ::cppu::queryInterface(
        rType, static_cast< XAdapter * >( this ), static_cast< XInterface * >( this ) );
}

// XInterface
void SAL_CALL OWeakConnectionPoint::acquire() throw()
{
    osl_atomic_increment( &m_aRefCount );
}

// XInterface
void SAL_CALL OWeakConnectionPoint::release() throw()
{
    if (! osl_atomic_decrement( &m_aRefCount ))
        delete this;
}

void OWeakConnectionPoint::dispose()
{
    std::vector<Reference<XReference>> aCopy;
    { // only hold the mutex while we access the field
        MutexGuard aGuard(getWeakMutex());
        // OWeakObject is not the only owner of this, so clear m_pObject
        // so that queryAdapted() won't use it now that it's dead
        m_pObject = nullptr;
        // other code is going to call removeReference while we are doing this, so we need a
        // copy, but since we are disposing and going away, we can just take the original data
        aCopy.swap(m_aReferences);
    }
    Any ex;
    for (const Reference<XReference> & i : aCopy )
    {
        try
        {
            i->dispose();
        }
        catch (css::lang::DisposedException &) {}
        catch (RuntimeException &)
        {
            ex = cppu::getCaughtException();
        }
    }
    if (ex.hasValue())
    {
        cppu::throwException(ex);
    }
}

// XInterface
Reference< XInterface > SAL_CALL OWeakConnectionPoint::queryAdapted()
{
    Reference< XInterface > ret;

    ClearableMutexGuard guard(getWeakMutex());

    if (m_pObject)
    {
        oslInterlockedCount n = osl_atomic_increment( &m_pObject->m_refCount );

        if (n > 1)
        {
            // The reference is incremented. The object cannot be destroyed.
            // Release the guard at the earliest point.
            guard.clear();
            // WeakObject has a (XInterface *) cast operator
            ret = *m_pObject;
            osl_atomic_decrement( &m_pObject->m_refCount );
        }
        else
            // Another thread wait in the dispose method at the guard
            osl_atomic_decrement( &m_pObject->m_refCount );
    }

    return ret;
}

// XInterface
void SAL_CALL OWeakConnectionPoint::addReference(const Reference< XReference >& rRef)
{
    MutexGuard aGuard(getWeakMutex());
    m_aReferences.push_back( rRef );
}

// XInterface
void SAL_CALL OWeakConnectionPoint::removeReference(const Reference< XReference >& rRef)
{
    MutexGuard aGuard(getWeakMutex());
    // Search from end because the thing that last added a ref is most likely to be the
    // first to remove a ref.
    // It's not really valid to compare the pointer directly, but it's faster.
    auto it = std::find_if(m_aReferences.rbegin(), m_aReferences.rend(),
        [&rRef](const Reference<XReference>& rxRef) { return rxRef.get() == rRef.get(); });
    if (it != m_aReferences.rend()) {
        m_aReferences.erase( it.base()-1 );
        return;
    }
    // interface not found, use the correct compare method
    it = std::find(m_aReferences.rbegin(), m_aReferences.rend(), rRef);
    if ( it != m_aReferences.rend() )
        m_aReferences.erase( it.base()-1 );
}


//-- OWeakObject -------------------------------------------------------


#ifdef _MSC_VER
// Accidentally occurs in msvc mapfile = > had to be outlined.
OWeakObject::OWeakObject()
    : m_refCount( 0 ),
      m_pWeakConnectionPoint( nullptr )
{
}
#endif

// XInterface
Any SAL_CALL OWeakObject::queryInterface( const Type & rType )
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XWeak * >( this ), static_cast< XInterface * >( this ) );
}

// XInterface
void SAL_CALL OWeakObject::acquire() throw()
{
    osl_atomic_increment( &m_refCount );
}

// XInterface
void SAL_CALL OWeakObject::release() throw()
{
    if (osl_atomic_decrement( &m_refCount ) == 0) {
        // notify/clear all weak-refs before object's dtor is executed
        // (which may check weak-refs to this object):
        disposeWeakConnectionPoint();
        // destroy object:
        delete this;
    }
}

void OWeakObject::disposeWeakConnectionPoint()
{
    OSL_PRECOND( m_refCount == 0, "OWeakObject::disposeWeakConnectionPoint: only to be called with a ref count of 0!" );
    if (m_pWeakConnectionPoint != nullptr) {
        OWeakConnectionPoint * const p = m_pWeakConnectionPoint;
        m_pWeakConnectionPoint = nullptr;
        try {
            p->dispose();
        }
        catch (RuntimeException const& exc) {
            SAL_WARN( "cppuhelper", exc );
        }
        p->release();
    }
}

OWeakObject::~OWeakObject() COVERITY_NOEXCEPT_FALSE
{
}

// XWeak
Reference< XAdapter > SAL_CALL OWeakObject::queryAdapter()
{
    if (!m_pWeakConnectionPoint)
    {
        // only acquire mutex if member is not created
        MutexGuard aGuard( getWeakMutex() );
        if( !m_pWeakConnectionPoint )
        {
            OWeakConnectionPoint * p = new OWeakConnectionPoint(this);
            p->acquire();
            m_pWeakConnectionPoint = p;
        }
    }

    return m_pWeakConnectionPoint;
}


//-- OWeakAggObject ----------------------------------------------------

OWeakAggObject::~OWeakAggObject()
{
}

// XInterface
void OWeakAggObject::acquire() throw()
{
    Reference<XInterface > x( xDelegator );
    if (x.is())
        x->acquire();
    else
        OWeakObject::acquire();
}

// XInterface
void OWeakAggObject::release() throw()
{
    Reference<XInterface > x( xDelegator );
    if (x.is())
        x->release();
    else
        OWeakObject::release();
}

// XInterface
Any OWeakAggObject::queryInterface( const Type & rType )
{
    Reference< XInterface > x( xDelegator ); // harden ref
    return (x.is() ? x->queryInterface( rType ) : queryAggregation( rType ));
}

// XAggregation
Any OWeakAggObject::queryAggregation( const Type & rType )
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XInterface * >( static_cast< OWeakObject * >( this ) ),
        static_cast< XAggregation * >( this ),
        static_cast< XWeak * >( this ) );
}

// XAggregation
void OWeakAggObject::setDelegator( const Reference<XInterface > & rDelegator )
{
    xDelegator = rDelegator;
}

}

/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{


//-- OWeakRefListener -----------------------------------------------------

class OWeakRefListener: public XReference
{
public:
    explicit OWeakRefListener(const Reference< XInterface >& xInt);
    virtual ~OWeakRefListener();

    // noncopyable
    OWeakRefListener(const OWeakRefListener&) = delete;
    const OWeakRefListener& operator=(const OWeakRefListener&) = delete;

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType ) override;
    void SAL_CALL acquire() throw() override;
    void SAL_CALL release() throw() override;

    // XReference
    void SAL_CALL   dispose() override;

    /// The reference counter.
    oslInterlockedCount         m_aRefCount;
    /// The connection point of the weak object, guarded by getWeakMutex()
    Reference< XAdapter >       m_XWeakConnectionPoint;
};

OWeakRefListener::OWeakRefListener(const Reference< XInterface >& xInt)
    : m_aRefCount( 1 )
{
    try
    {
    Reference< XWeak > xWeak( Reference< XWeak >::query( xInt ) );

    if (xWeak.is())
    {
        m_XWeakConnectionPoint = xWeak->queryAdapter();

        if (m_XWeakConnectionPoint.is())
        {
            m_XWeakConnectionPoint->addReference(static_cast<XReference*>(this));
        }
    }
    }
    catch (RuntimeException &) { OSL_ASSERT( false ); } // assert here, but no unexpected()
    osl_atomic_decrement( &m_aRefCount );
}

OWeakRefListener::~OWeakRefListener()
{
    try
    {
    if (m_XWeakConnectionPoint.is())
    {
        acquire(); // don't die again
        m_XWeakConnectionPoint->removeReference(static_cast<XReference*>(this));
    }
    }
    catch (RuntimeException &) { OSL_ASSERT( false ); } // assert here, but no unexpected()
}

// XInterface
Any SAL_CALL OWeakRefListener::queryInterface( const Type & rType )
{
    return ::cppu::queryInterface(
        rType, static_cast< XReference * >( this ), static_cast< XInterface * >( this ) );
}

// XInterface
void SAL_CALL OWeakRefListener::acquire() throw()
{
    osl_atomic_increment( &m_aRefCount );
}

// XInterface
void SAL_CALL OWeakRefListener::release() throw()
{
    if( ! osl_atomic_decrement( &m_aRefCount ) )
        delete this;
}

void SAL_CALL OWeakRefListener::dispose()
{
    Reference< XAdapter > xAdp;
    {
        MutexGuard guard(cppu::getWeakMutex());
        if( m_XWeakConnectionPoint.is() )
        {
            xAdp = m_XWeakConnectionPoint;
            m_XWeakConnectionPoint.clear();
        }
    }

    if( xAdp.is() )
        xAdp->removeReference(static_cast<XReference*>(this));
}


//-- WeakReferenceHelper ----------------------------------------------------------

WeakReferenceHelper::WeakReferenceHelper(const Reference< XInterface >& xInt)
    : m_pImpl( nullptr )
{
    if (xInt.is())
    {
        m_pImpl = new OWeakRefListener(xInt);
        m_pImpl->acquire();
    }
}

WeakReferenceHelper::WeakReferenceHelper(const WeakReferenceHelper& rWeakRef)
    : m_pImpl( nullptr )
{
    Reference< XInterface > xInt( rWeakRef.get() );
    if (xInt.is())
    {
        m_pImpl = new OWeakRefListener(xInt);
        m_pImpl->acquire();
    }
}

void WeakReferenceHelper::clear()
{
    try
    {
        if (m_pImpl)
        {
            m_pImpl->dispose();
            m_pImpl->release();
            m_pImpl = nullptr;
        }
    }
    catch (RuntimeException &) { OSL_ASSERT( false ); } // assert here, but no unexpected()
}

WeakReferenceHelper& WeakReferenceHelper::operator=(const WeakReferenceHelper& rWeakRef)
{
    if (this == &rWeakRef)
    {
        return *this;
    }
    Reference< XInterface > xInt( rWeakRef.get() );
    return operator = ( xInt );
}

WeakReferenceHelper & WeakReferenceHelper::operator =(
    WeakReferenceHelper && other)
{
    clear();
    std::swap(m_pImpl, other.m_pImpl);
    return *this;
}

WeakReferenceHelper & SAL_CALL
WeakReferenceHelper::operator= (const Reference< XInterface > & xInt)
{
    try
    {
        clear();
        if (xInt.is())
        {
            m_pImpl = new OWeakRefListener(xInt);
            m_pImpl->acquire();
        }
    }
    catch (RuntimeException &) { OSL_ASSERT( false ); } // assert here, but no unexpected()
    return *this;
}

WeakReferenceHelper::~WeakReferenceHelper()
{
    clear();
}

Reference< XInterface > WeakReferenceHelper::get() const
{
    try
    {
        Reference< XAdapter > xAdp;
        {
            // must lock to access m_XWeakConnectionPoint
            MutexGuard guard(cppu::getWeakMutex());
            if( m_pImpl && m_pImpl->m_XWeakConnectionPoint.is() )
                xAdp = m_pImpl->m_XWeakConnectionPoint;
        }

        if (xAdp.is())
            return xAdp->queryAdapted();
    }
    catch (RuntimeException &)
    {
        OSL_ASSERT( false );
    } // assert here, but no unexpected()

    return Reference< XInterface >();
}

}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
