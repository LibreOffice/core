/*************************************************************************
 *
 *  $RCSfile: LifeTime.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _LIFETIME_HXX
#define _LIFETIME_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HDL_
#include <com/sun/star/uno/Exception.hdl>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

namespace apphelper
{

class LifeTimeGuard;
class LifeTimeManager
{
friend class LifeTimeGuard;
public:
    LifeTimeManager( ::com::sun::star::lang::XComponent* pComponent, sal_Bool bLongLastingCallsCancelable = sal_False );
    virtual ~LifeTimeManager();

    sal_Bool    impl_isDisposed();
    sal_Bool    dispose() throw(::com::sun::star::uno::RuntimeException);

public:
    ::cppu::OMultiTypeInterfaceContainerHelper      m_aListenerContainer;

protected:
    virtual sal_Bool    impl_canStartApiCall();
    virtual void        impl_apiCallCountReachedNull(){};

    void        impl_registerApiCall(sal_Bool bLongLastingCall);
    void        impl_unregisterApiCall(sal_Bool bLongLastingCall);

    void        impl_init();

protected:
    mutable ::osl::Mutex                    m_aAccessMutex;

    ::com::sun::star::lang::XComponent*     m_pComponent;

    ::osl::Condition        m_aNoAccessCountCondition;
    sal_Int32 volatile      m_nAccessCount;

    sal_Bool volatile       m_bDisposed;
    sal_Bool volatile       m_bInDispose;

    //
    sal_Bool                m_bLongLastingCallsCancelable;
    ::osl::Condition        m_aNoLongLastingCallCountCondition;
    sal_Int32 volatile      m_nLongLastingCallCount;
};

class CloseableLifeTimeManager : public LifeTimeManager
{
protected:
    ::com::sun::star::util::XCloseable*         m_pCloseable;

    ::osl::Condition        m_aEndTryClosingCondition;
    sal_Bool volatile       m_bClosed;
    sal_Bool volatile       m_bInTryClose;
    //the ownership between model and controller is not clear at first
    //each controller might consider him as owner of the model first
    //at start the model is not considered as owner of itself
    sal_Bool volatile       m_bOwnership;
    //with a XCloseable::close call and during XCloseListener::queryClosing
    //the ownership can be regulated more explicit,
    //if so the ownership is considered to be well known
    sal_Bool volatile       m_bOwnershipIsWellKnown;

public:
    CloseableLifeTimeManager( ::com::sun::star::util::XCloseable* pCloseable
        , ::com::sun::star::lang::XComponent* pComponent
        , sal_Bool bLongLastingCallsCancelable = sal_False );
    virtual ~CloseableLifeTimeManager();

    sal_Bool    impl_isDisposedOrClosed();
    sal_Bool    g_close_startTryClose(sal_Bool bDeliverOwnership)
                    throw ( ::com::sun::star::uno::Exception );
    sal_Bool    g_close_isNeedToCancelLongLastingCalls( sal_Bool bDeliverOwnership, ::com::sun::star::uno::Exception& ex )
                    throw ( ::com::sun::star::uno::Exception );
    void        g_close_endTryClose(sal_Bool bDeliverOwnership, sal_Bool bMyVeto, ::com::sun::star::uno::Exception& ex)
                    throw ( ::com::sun::star::uno::Exception );
    void        g_close_endTryClose_doClose();
    sal_Bool    g_addCloseListener( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::util::XCloseListener > & xListener )
                    throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual sal_Bool    impl_canStartApiCall();
    virtual void        impl_apiCallCountReachedNull();

    void        impl_setOwnership( sal_Bool bDeliverOwnership, sal_Bool bMyVeto );
    sal_Bool    impl_shouldCloseAtNextChance();
    void        impl_doClose();

    void        impl_init()
    {
        m_bClosed = sal_False;
        m_bInTryClose = sal_False;
        m_bOwnership = sal_False;
        m_bOwnershipIsWellKnown = sal_False;
        m_aEndTryClosingCondition.set();
    }
};

//-----------------------------------------------------------------
/*
Use this Guard in your apicalls to protect access on resources
which will be released in dispose.
It's guarantied, that the release of resources only starts if your
guarded call has finished.
! It's only partly guaranteed that this resources will not change during the call.
See the example for details.

This class is to be used as described in the example.

If this guard is used in all api calls of an XCloseable object
it's guarantied, that the closeable will close itself after finishing the last call
if it should do so.

  ::ApiCall
{
    //hold no mutex!!!
    LifeTimeGuard aLifeTimeGuard(m_aLifeTimeManager);

    //mutex is acquired; call is not registered

    if(!aGuard.startApiCall())
        return ; //behave as passive as possible, if disposed or closed

    //mutex is acquired, call is registered
    {
        //you might access some private members here
        //but than you need to protect access to these members always like this
        //never call to the outside here
    }

    aLifeTimeGuard.clear(); //!!!

      //Mutex is released, the running call is still registered
    //this call will finish before the 'release-section' in dispose is allowed to start

    {
        //you might access some private members here guarded with your own mutex
        //but release your mutex at the end of this block
    }

    //you can call to the outside (without holding the mutex) without becoming disposed

    //End of method -> ~LifeTimeGuard
    //-> call is unregistered
    //-> this object might be disposed now
}

your XComponent::dispose method has to be implemented in the following way:

    ::dispose()
{
    //hold no mutex!!!
    if( !m_aLifeTimeManager.dispose() )
        return;

    //--release all resources and references
    //...
}

*/
//-----------------------------------------------------------------

class LifeTimeGuard : public ::osl::ResettableMutexGuard
{

public:
    LifeTimeGuard( LifeTimeManager& rManager )
        : ::osl::ResettableGuard< ::osl::Mutex >( rManager.m_aAccessMutex )
        , m_rManager(rManager)
        , m_bCallRegistered(sal_False)
        , m_bLongLastingCallRegistered(sal_False)
    {

    }
    sal_Bool startApiCall(sal_Bool bLongLastingCall=sal_False);
    virtual ~LifeTimeGuard();

private:
    LifeTimeManager&    m_rManager;
    sal_Bool            m_bCallRegistered;
    sal_Bool            m_bLongLastingCallRegistered;

private:
    // these make no sense
    LifeTimeGuard( ::osl::Mutex& rMutex );
    LifeTimeGuard( const LifeTimeGuard& );
    LifeTimeGuard& operator= ( const LifeTimeGuard& );
};


template<class T>
class NegativeGuard
{
protected:
    T * m_pT;
public:

    NegativeGuard(T * pT) : m_pT(pT)
    {
        m_pT->release();
    }

    NegativeGuard(T & t) : m_pT(&t)
    {
        m_pT->release();
    }

    ~NegativeGuard()
    {
        m_pT->acquire();
    }
};

}//end namespace apphelper
#endif
