/*************************************************************************
 *
 *  $RCSfile: weak.cxx,v $
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
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _CPPU_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif
#ifndef _CPPU_HELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

using namespace osl;
using namespace com::sun::star::uno;

/** */ //for docpp
namespace cppu
{

/**
   The mutex to synchronize the the queryAdapted call throug the connection point
   with the release call at the weak object.
 */
/*
struct WeakMutexStatic
{
    Mutex       aMutex;
    sal_Bool    bDestructed;

    WeakMutexStatic()
        : bDestructed( sal_False )
        {}
    ~WeakMutexStatic()
        { bDestructed = sal_True; }
};

inline static Mutex & getWeakMutex() throw()
{
    static WeakMutexStatic s_wmstatic;
    if (s_wmstatic.bDestructed)
        return *Mutex::getGlobalMutex();
    else
        return s_wmstatic.aMutex;
}
*/

// due to static Reflection destruction from usr, ther must be a mutex leak (#73272#)
inline static Mutex & getWeakMutex() throw()
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
    OWeakConnectionPoint( OWeakObject* pObj ) throw()
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
protected:
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
    osl_incrementInterlockedCount( &m_aRefCount );
}

// XInterface
void SAL_CALL OWeakConnectionPoint::release() throw()
{
    if (! osl_decrementInterlockedCount( &m_aRefCount ))
        delete this;
}

void SAL_CALL OWeakConnectionPoint::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    OInterfaceIteratorHelper aIt( m_aReferences );
    while( aIt.hasMoreElements() )
        ((XReference *)aIt.next())->dispose();
}

// XInterface
Reference< XInterface > SAL_CALL OWeakConnectionPoint::queryAdapted() throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > ret;

    ClearableMutexGuard guard(getWeakMutex());

    if (m_pObject)
    {
        oslInterlockedCount n = osl_incrementInterlockedCount( &m_pObject->m_refCount );

        if (n > 1)
        {
            // The refence is incremented. The object cannot be destroyed.
            // Release the guard at the earliest point.
            guard.clear();
            // WeakObject has a (XInterface *) cast operator
            ret = *m_pObject;
            n = osl_decrementInterlockedCount( &m_pObject->m_refCount );
        }
        else
            // Another thread wait in the dispose method at the guard
            n = osl_decrementInterlockedCount( &m_pObject->m_refCount );
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
    osl_incrementInterlockedCount( &m_refCount );
}

// XInterface
void SAL_CALL OWeakObject::release() throw()
{
    if (osl_decrementInterlockedCount( &m_refCount ) == 0)
    {
        if (m_pWeakConnectionPoint)
        {
            OWeakConnectionPoint * p = m_pWeakConnectionPoint;
            m_pWeakConnectionPoint = 0;
            p->dispose();
            p->release();
        }
        delete this;
    }
}

OWeakObject::~OWeakObject() throw(::com::sun::star::uno::RuntimeException)
{
}

// XWeak
Reference< XAdapter > SAL_CALL OWeakObject::queryAdapter(void) throw()
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
Any OWeakAggObject::queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
{
    Reference< XInterface > x( xDelegator ); // harden ref
    return (x.is() ? x->queryInterface( rType ) : queryAggregation( rType ));

//      // set rOut to zero, if failed
//      if( !xDelegator.queryHardRef( aUik, rOut ) )
//      {
//          XInterfaceRef x;
//          if( !xDelegator.queryHardRef( ((XInterface*)0)->getSmartUik(), x ) )
//              // reference is not valid
//              queryAggregation( aUik, rOut );
//      }
//      return rOut.is();
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
    OWeakRefListener();
    OWeakRefListener(const OWeakRefListener& rRef);
    OWeakRefListener(const Reference< XInterface >& xInt);
    ~OWeakRefListener();

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType ) throw();
    void SAL_CALL acquire() throw();
    void SAL_CALL release() throw();

    // XReference
    void SAL_CALL   dispose() throw(::com::sun::star::uno::RuntimeException);

    /// The reference counter.
    oslInterlockedCount         m_aRefCount;
    /// The connection point of the weak object
    Reference< XAdapter >       m_XWeakConnectionPoint;

private:
    OWeakRefListener& SAL_CALL operator=(const OWeakRefListener& rRef);
};

OWeakRefListener::OWeakRefListener()
    : m_aRefCount( 0 )
{
}

OWeakRefListener::OWeakRefListener(const OWeakRefListener& rRef)
    : m_aRefCount( 0 )
{
    m_XWeakConnectionPoint = rRef.m_XWeakConnectionPoint;

    if (m_XWeakConnectionPoint.is())
        m_XWeakConnectionPoint->addReference((XReference*)this);
}

OWeakRefListener::OWeakRefListener(const Reference< XInterface >& xInt)
    : m_aRefCount( 0 )
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

OWeakRefListener::~OWeakRefListener()
{
    acquire(); // dont die again
    if (m_XWeakConnectionPoint.is())
        m_XWeakConnectionPoint->removeReference((XReference*)this);
}

// XInterface
Any SAL_CALL OWeakRefListener::queryInterface( const Type & rType ) throw()
{
    return ::cppu::queryInterface(
        rType, static_cast< XReference * >( this ), static_cast< XInterface * >( this ) );
}

// XInterface
void SAL_CALL OWeakRefListener::acquire() throw()
{
    osl_incrementInterlockedCount( &m_aRefCount );
}

// XInterface
void SAL_CALL OWeakRefListener::release() throw()
{
    if( ! osl_decrementInterlockedCount( &m_aRefCount ) )
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
WeakReferenceHelper::WeakReferenceHelper(const Reference< XInterface >& xInt)
{
    m_pImpl = new OWeakRefListener(xInt);
    m_pImpl->acquire();
}

WeakReferenceHelper::WeakReferenceHelper(const WeakReferenceHelper& rWeakRef)
{
    m_pImpl = rWeakRef.m_pImpl;

    if( m_pImpl )
        m_pImpl->acquire();
}

WeakReferenceHelper& WeakReferenceHelper::operator=(const WeakReferenceHelper& rWeakRef)
{
    OWeakRefListener* pOldImpl;
    {
        // the weak reference is multithread save
        MutexGuard guard(cppu::getWeakMutex());
        if (m_pImpl == rWeakRef.m_pImpl)
            return *this;

        pOldImpl = m_pImpl;

        m_pImpl = rWeakRef.m_pImpl;
        if ( m_pImpl )
        {
            m_pImpl->acquire();
        }
    }

    // maybe call the destructor. It is better to release the guard before this call.
    if( pOldImpl )
        pOldImpl->release();
    return *this;
}

WeakReferenceHelper::~WeakReferenceHelper()
{
    if (m_pImpl)
        m_pImpl->release();
}

Reference< XInterface > WeakReferenceHelper::get() const
{
    Reference< XAdapter > xAdp;
    {
        MutexGuard guard(cppu::getWeakMutex());
        if( m_pImpl && m_pImpl->m_XWeakConnectionPoint.is() )
            xAdp = m_pImpl->m_XWeakConnectionPoint;
    }

    if (xAdp.is())
        return xAdp->queryAdapted();

    return Reference< XInterface >();
}

}
}
}
}

