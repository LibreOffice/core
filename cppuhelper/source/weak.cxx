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

#include <osl/mutex.hxx>
#include <cppuhelper/weakagg.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include "cppuhelper/exc_hlp.hxx"

using namespace osl;
using namespace com::sun::star::uno;

/** */ 
namespace cppu
{


inline static Mutex & getWeakMutex() SAL_THROW(())
{
    static Mutex * s_pMutex = 0;
    if (! s_pMutex)
        s_pMutex = new Mutex();
    return *s_pMutex;
}




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

    
    Any SAL_CALL        queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL       acquire() throw();
    void SAL_CALL       release() throw();

    
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL queryAdapted() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL addReference( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XReference >& xRef ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeReference( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XReference >& xRef ) throw(::com::sun::star::uno::RuntimeException);

    /
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

private:
    OWeakConnectionPoint(OWeakConnectionPoint &); 
    void operator =(OWeakConnectionPoint &); 

    virtual ~OWeakConnectionPoint() {}

    /
    oslInterlockedCount         m_aRefCount;
    /
    OWeakObject*                m_pObject;
    /
    OInterfaceContainerHelper   m_aReferences;
};


Any SAL_CALL OWeakConnectionPoint::queryInterface( const Type & rType )
    throw(com::sun::star::uno::RuntimeException)
{
    return ::cppu::queryInterface(
        rType, static_cast< XAdapter * >( this ), static_cast< XInterface * >( this ) );
}


void SAL_CALL OWeakConnectionPoint::acquire() throw()
{
    osl_atomic_increment( &m_aRefCount );
}


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


Reference< XInterface > SAL_CALL OWeakConnectionPoint::queryAdapted() throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > ret;

    ClearableMutexGuard guard(getWeakMutex());

    if (m_pObject)
    {
        oslInterlockedCount n = osl_atomic_increment( &m_pObject->m_refCount );

        if (n > 1)
        {
            
            
            guard.clear();
            
            ret = *m_pObject;
            n = osl_atomic_decrement( &m_pObject->m_refCount );
        }
        else
            
            n = osl_atomic_decrement( &m_pObject->m_refCount );
    }

    return ret;
}


void SAL_CALL OWeakConnectionPoint::addReference(const Reference< XReference >& rRef)
    throw(::com::sun::star::uno::RuntimeException)
{
    m_aReferences.addInterface( (const Reference< XInterface > &)rRef );
}


void SAL_CALL OWeakConnectionPoint::removeReference(const Reference< XReference >& rRef)
    throw(::com::sun::star::uno::RuntimeException)
{
    m_aReferences.removeInterface( (const Reference< XInterface > &)rRef );
}






#ifdef _MSC_VER

OWeakObject::OWeakObject() SAL_THROW(())
    : m_refCount( 0 ),
      m_pWeakConnectionPoint( 0 )
{
}
#endif


Any SAL_CALL OWeakObject::queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XWeak * >( this ), static_cast< XInterface * >( this ) );
}


void SAL_CALL OWeakObject::acquire() throw()
{
    osl_atomic_increment( &m_refCount );
}


void SAL_CALL OWeakObject::release() throw()
{
    if (osl_atomic_decrement( &m_refCount ) == 0) {
        
        
        disposeWeakConnectionPoint();
        
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


Reference< XAdapter > SAL_CALL OWeakObject::queryAdapter()
    throw (::com::sun::star::uno::RuntimeException)
{
    if (!m_pWeakConnectionPoint)
    {
        
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




OWeakAggObject::~OWeakAggObject() SAL_THROW( (RuntimeException) )
{
}


void OWeakAggObject::acquire() throw()
{
    Reference<XInterface > x( xDelegator );
    if (x.is())
        x->acquire();
    else
        OWeakObject::acquire();
}


void OWeakAggObject::release() throw()
{
    Reference<XInterface > x( xDelegator );
    if (x.is())
        x->release();
    else
        OWeakObject::release();
}


Any OWeakAggObject::queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > x( xDelegator ); 
    return (x.is() ? x->queryInterface( rType ) : queryAggregation( rType ));
}


Any OWeakAggObject::queryAggregation( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XInterface * >( static_cast< OWeakObject * >( this ) ),
        static_cast< XAggregation * >( this ),
        static_cast< XWeak * >( this ) );
}


void OWeakAggObject::setDelegator( const Reference<XInterface > & rDelegator ) throw(::com::sun::star::uno::RuntimeException)
{
    xDelegator = rDelegator;
}

}

/** */ 
namespace com
{
/** */ 
namespace sun
{
/** */ 
namespace star
{
/** */ 
namespace uno
{





class OWeakRefListener : public XReference
{
public:
    OWeakRefListener(const OWeakRefListener& rRef) SAL_THROW(());
    OWeakRefListener(const Reference< XInterface >& xInt) SAL_THROW(());
    virtual ~OWeakRefListener() SAL_THROW(());

    
    Any SAL_CALL queryInterface( const Type & rType ) throw(RuntimeException);
    void SAL_CALL acquire() throw();
    void SAL_CALL release() throw();

    
    void SAL_CALL   dispose() throw(::com::sun::star::uno::RuntimeException);

    /
    oslInterlockedCount         m_aRefCount;
    /
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
    catch (RuntimeException &) { OSL_ASSERT( false ); } 
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
    catch (RuntimeException &) { OSL_ASSERT( false ); } 
    osl_atomic_decrement( &m_aRefCount );
}

OWeakRefListener::~OWeakRefListener() SAL_THROW(())
{
    try
    {
    if (m_XWeakConnectionPoint.is())
    {
        acquire(); 
        m_XWeakConnectionPoint->removeReference((XReference*)this);
    }
    }
    catch (RuntimeException &) { OSL_ASSERT( false ); } 
}


Any SAL_CALL OWeakRefListener::queryInterface( const Type & rType ) throw(RuntimeException)
{
    return ::cppu::queryInterface(
        rType, static_cast< XReference * >( this ), static_cast< XInterface * >( this ) );
}


void SAL_CALL OWeakRefListener::acquire() throw()
{
    osl_atomic_increment( &m_aRefCount );
}


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
    catch (RuntimeException &) { OSL_ASSERT( false ); } 
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
    catch (RuntimeException &) { OSL_ASSERT( false ); } 
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
    catch (RuntimeException &) { OSL_ASSERT( false ); } 

    return Reference< XInterface >();
}

}
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
