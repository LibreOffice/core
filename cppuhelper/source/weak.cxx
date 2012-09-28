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

#include <osl/mutex.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include "cppuhelper/exc_hlp.hxx"

using namespace osl;
using namespace com::sun::star::uno;

/** */ //for docpp
namespace cppu
{

// due to static Reflection destruction from usr, ther must be a mutex leak (#73272#)
inline static Mutex & getWeakMutex() SAL_THROW(())
{
    static Mutex * s_pMutex = 0;
    if (! s_pMutex)
        s_pMutex = new Mutex();
    return *s_pMutex;
}

//------------------------------------------------------------------------
//-- OWeakConnectionPoint ----------------------------------------------------
//------------------------------------------------------------------------
class OWeakConnectionPoint : public XAdapter
{
public:
    /**
        Hold the weak object without an acquire (only the pointer).
     */
    OWeakConnectionPoint( OWeakObject* pObj ) SAL_THROW(())
        : m_aRefCount( 0 )
        , m_pObject(pObj)
        , m_aReferences( getWeakMutex() )
        {}

    // XInterface
    Any SAL_CALL        queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL       acquire() throw();
    void SAL_CALL       release() throw();

    // XAdapter
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL queryAdapted() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addReference( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XReference >& xRef ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeReference( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XReference >& xRef ) throw(::com::sun::star::uno::RuntimeException);

    /// Called from the weak object if the reference count goes to zero.
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

private:
    OWeakConnectionPoint(OWeakConnectionPoint &); // not defined
    void operator =(OWeakConnectionPoint &); // not defined

    virtual ~OWeakConnectionPoint() {}

    /// The reference counter.
    oslInterlockedCount         m_aRefCount;
    /// The weak object
    OWeakObject*                m_pObject;
    /// The container to hold the weak references
    OInterfaceContainerHelper   m_aReferences;
};

// XInterface
Any SAL_CALL OWeakConnectionPoint::queryInterface( const Type & rType )
    throw(com::sun::star::uno::RuntimeException)
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

void SAL_CALL OWeakConnectionPoint::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    Any ex;
    OInterfaceIteratorHelper aIt( m_aReferences );
    while( aIt.hasMoreElements() )
    {
        try
        {
            ((XReference *)aIt.next())->dispose();
        }
        catch (com::sun::star::lang::DisposedException &) {}
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
Reference< XInterface > SAL_CALL OWeakConnectionPoint::queryAdapted() throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > ret;

    ClearableMutexGuard guard(getWeakMutex());

    if (m_pObject)
    {
        oslInterlockedCount n = osl_atomic_increment( &m_pObject->m_refCount );

        if (n > 1)
        {
            // The refence is incremented. The object cannot be destroyed.
            // Release the guard at the earliest point.
            guard.clear();
            // WeakObject has a (XInterface *) cast operator
            ret = *m_pObject;
            n = osl_atomic_decrement( &m_pObject->m_refCount );
        }
        else
            // Another thread wait in the dispose method at the guard
            n = osl_atomic_decrement( &m_pObject->m_refCount );
    }

    return ret;
}

// XInterface
void SAL_CALL OWeakConnectionPoint::addReference(const Reference< XReference >& rRef)
    throw(::com::sun::star::uno::RuntimeException)
{
    m_aReferences.addInterface( (const Reference< XInterface > &)rRef );
}

// XInterface
void SAL_CALL OWeakConnectionPoint::removeReference(const Reference< XReference >& rRef)
    throw(::com::sun::star::uno::RuntimeException)
{
    m_aReferences.removeInterface( (const Reference< XInterface > &)rRef );
}


//------------------------------------------------------------------------
//-- OWeakObject -------------------------------------------------------
//------------------------------------------------------------------------

#ifdef _MSC_VER
// Accidentally occurs in msvc mapfile = > had to be outlined.
OWeakObject::OWeakObject() SAL_THROW(())
    : m_refCount( 0 ),
      m_pWeakConnectionPoint( 0 )
{
}
#endif

// XInterface
Any SAL_CALL OWeakObject::queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
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
    if (m_pWeakConnectionPoint != 0) {
        OWeakConnectionPoint * const p = m_pWeakConnectionPoint;
        m_pWeakConnectionPoint = 0;
        try {
            p->dispose();
        }
        catch (RuntimeException const& exc) {
            OSL_FAIL(
                OUStringToOString(
                    exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
            static_cast<void>(exc);
        }
        p->release();
    }
}

OWeakObject::~OWeakObject() SAL_THROW( (RuntimeException) )
{
}

// XWeak
Reference< XAdapter > SAL_CALL OWeakObject::queryAdapter()
    throw (::com::sun::star::uno::RuntimeException)
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

//------------------------------------------------------------------------
//-- OWeakAggObject ----------------------------------------------------
//------------------------------------------------------------------------
OWeakAggObject::~OWeakAggObject() SAL_THROW( (RuntimeException) )
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
Any OWeakAggObject::queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > x( xDelegator ); // harden ref
    return (x.is() ? x->queryInterface( rType ) : queryAggregation( rType ));
}

// XAggregation
Any OWeakAggObject::queryAggregation( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XInterface * >( static_cast< OWeakObject * >( this ) ),
        static_cast< XAggregation * >( this ),
        static_cast< XWeak * >( this ) );
}

// XAggregation
void OWeakAggObject::setDelegator( const Reference<XInterface > & rDelegator ) throw(::com::sun::star::uno::RuntimeException)
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


//------------------------------------------------------------------------
//-- OWeakRefListener -----------------------------------------------------
//------------------------------------------------------------------------
class OWeakRefListener : public XReference
{
public:
    OWeakRefListener(const OWeakRefListener& rRef) SAL_THROW(());
    OWeakRefListener(const Reference< XInterface >& xInt) SAL_THROW(());
    virtual ~OWeakRefListener() SAL_THROW(());

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType ) throw(RuntimeException);
    void SAL_CALL acquire() throw();
    void SAL_CALL release() throw();

    // XReference
    void SAL_CALL   dispose() throw(::com::sun::star::uno::RuntimeException);

    /// The reference counter.
    oslInterlockedCount         m_aRefCount;
    /// The connection point of the weak object
    Reference< XAdapter >       m_XWeakConnectionPoint;

private:
    OWeakRefListener& SAL_CALL operator=(const OWeakRefListener& rRef) SAL_THROW(());
};

OWeakRefListener::OWeakRefListener(const OWeakRefListener& rRef) SAL_THROW(())
    : com::sun::star::uno::XReference()
    , m_aRefCount( 1 )
{
    try
    {
    m_XWeakConnectionPoint = rRef.m_XWeakConnectionPoint;

    if (m_XWeakConnectionPoint.is())
    {
        m_XWeakConnectionPoint->addReference((XReference*)this);
    }
    }
    catch (RuntimeException &) { OSL_ASSERT( 0 ); } // assert here, but no unexpected()
    osl_atomic_decrement( &m_aRefCount );
}

OWeakRefListener::OWeakRefListener(const Reference< XInterface >& xInt) SAL_THROW(())
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
            m_XWeakConnectionPoint->addReference((XReference*)this);
        }
    }
    }
    catch (RuntimeException &) { OSL_ASSERT( 0 ); } // assert here, but no unexpected()
    osl_atomic_decrement( &m_aRefCount );
}

OWeakRefListener::~OWeakRefListener() SAL_THROW(())
{
    try
    {
    if (m_XWeakConnectionPoint.is())
    {
        acquire(); // dont die again
        m_XWeakConnectionPoint->removeReference((XReference*)this);
    }
    }
    catch (RuntimeException &) { OSL_ASSERT( 0 ); } // assert here, but no unexpected()
}

// XInterface
Any SAL_CALL OWeakRefListener::queryInterface( const Type & rType ) throw(RuntimeException)
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
    throw(::com::sun::star::uno::RuntimeException)
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
        xAdp->removeReference((XReference*)this);
}

//------------------------------------------------------------------------
//-- WeakReferenceHelper ----------------------------------------------------------
//------------------------------------------------------------------------
WeakReferenceHelper::WeakReferenceHelper(const Reference< XInterface >& xInt) SAL_THROW(())
    : m_pImpl( 0 )
{
    if (xInt.is())
    {
        m_pImpl = new OWeakRefListener(xInt);
        m_pImpl->acquire();
    }
}

WeakReferenceHelper::WeakReferenceHelper(const WeakReferenceHelper& rWeakRef) SAL_THROW(())
    : m_pImpl( 0 )
{
    Reference< XInterface > xInt( rWeakRef.get() );
    if (xInt.is())
    {
        m_pImpl = new OWeakRefListener(xInt);
        m_pImpl->acquire();
    }
}

void WeakReferenceHelper::clear() SAL_THROW(())
{
    try
    {
        if (m_pImpl)
        {
            if (m_pImpl->m_XWeakConnectionPoint.is())
            {
                m_pImpl->m_XWeakConnectionPoint->removeReference(
                        (XReference*)m_pImpl);
                m_pImpl->m_XWeakConnectionPoint.clear();
            }
            m_pImpl->release();
            m_pImpl = 0;
        }
    }
    catch (RuntimeException &) { OSL_ASSERT( 0 ); } // assert here, but no unexpected()
}

WeakReferenceHelper& WeakReferenceHelper::operator=(const WeakReferenceHelper& rWeakRef) SAL_THROW(())
{
    if (this == &rWeakRef)
    {
        return *this;
    }
    Reference< XInterface > xInt( rWeakRef.get() );
    return operator = ( xInt );
}

WeakReferenceHelper & SAL_CALL
WeakReferenceHelper::operator= (const Reference< XInterface > & xInt)
SAL_THROW(())
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
    catch (RuntimeException &) { OSL_ASSERT( 0 ); } // assert here, but no unexpected()
    return *this;
}

WeakReferenceHelper::~WeakReferenceHelper() SAL_THROW(())
{
    clear();
}

Reference< XInterface > WeakReferenceHelper::get() const SAL_THROW(())
{
    try
    {
    Reference< XAdapter > xAdp;
    {
        MutexGuard guard(cppu::getWeakMutex());
        if( m_pImpl && m_pImpl->m_XWeakConnectionPoint.is() )
            xAdp = m_pImpl->m_XWeakConnectionPoint;
    }

    if (xAdp.is())
        return xAdp->queryAdapted();
    }
    catch (RuntimeException &) { OSL_ASSERT( 0 ); } // assert here, but no unexpected()

    return Reference< XInterface >();
}

}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
