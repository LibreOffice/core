/*************************************************************************
 *
 *  $RCSfile: dynamicresultsetwrapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-17 10:44:57 $
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

#ifndef _DYNAMIC_RESULTSET_WRAPPER_HXX
#define _DYNAMIC_RESULTSET_WRAPPER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSET_HPP_
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSOURCEINITIALIZATION_HPP_
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#endif

#ifndef __com_sun_star_lang_DisposedException_idl__
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XDYNAMICRESULTSETLISTENER_HPP_
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

//#define CACHED_CRS_STUB_SERVICE_NAME "com.sun.star.ucb.CachedContentResultSetStub"
//#define CACHED_CRS_STUB_FACTORY_NAME "com.sun.star.ucb.CachedContentResultSetStubFactory"

//=========================================================================

class DynamicResultSetWrapperListener;
class DynamicResultSetWrapper
                : public cppu::OWeakObject
                , public com::sun::star::ucb::XDynamicResultSet
                , public com::sun::star::ucb::XSourceInitialization
{
private:
    //management of listeners
    sal_Bool                m_bDisposed; ///Dispose call ready.
    sal_Bool                m_bInDispose;///In dispose call
    osl::Mutex              m_aContainerMutex;
    cppu::OInterfaceContainerHelper*
                            m_pDisposeEventListeners;
protected:
    com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >
                            m_xMyListenerImpl;
    DynamicResultSetWrapperListener*
                            m_pMyListenerImpl;

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
                            m_xSMgr;

    osl::Mutex              m_aMutex;
    sal_Bool                m_bStatic;
    sal_Bool                m_bGotWelcome;

    //different Interfaces from Origin:
    com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSet >
                            m_xSource;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xSourceResultOne;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xSourceResultTwo;
    //com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
    //                      m_xSourceResultCurrent;
    //sal_Bool              m_bUseOne;
    //
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xMyResultOne;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >
                            m_xMyResultTwo;
    //
    com::sun::star::uno::Reference< com::sun::star::ucb::XDynamicResultSetListener >
                            m_xListener;

    osl::Condition          m_aSourceSet;
    osl::Condition          m_aListenerSet;

protected:
    void SAL_CALL impl_init();
    void SAL_CALL impl_deinit();
    void SAL_CALL
    impl_EnsureNotDisposed()
        throw( com::sun::star::lang::DisposedException,
               com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );

public:

    DynamicResultSetWrapper(
        com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::lang::XMultiServiceFactory > & xSMgr );

    virtual ~DynamicResultSetWrapper();

    //-----------------------------------------------------------------
    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface( const com::sun::star::uno::Type & rType )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XDynamicResultSet
    virtual com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet > SAL_CALL
    getStaticResultSet()
        throw( com::sun::star::ucb::ListenerAlreadySetException
        , com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    setListener( const com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSetListener > & Listener )
        throw( com::sun::star::ucb::ListenerAlreadySetException
            , com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    connectToCache( const com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSet > & xCache )
        throw( com::sun::star::ucb::ListenerAlreadySetException
            , com::sun::star::ucb::AlreadyInitializedException
            , com::sun::star::ucb::ServiceNotFoundException
            , com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL
    getCapabilities() throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XComponent ( base of XDynamicResultSet )
    virtual void SAL_CALL
    dispose() throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
        com::sun::star::lang::XEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
        com::sun::star::lang::XEventListener >& Listener )
            throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // XSourceInitialization

    virtual void SAL_CALL
    setSource(  const com::sun::star::uno::Reference<
        com::sun::star::uno::XInterface > & Source )
         throw( com::sun::star::ucb::AlreadyInitializedException
                , com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // own methods:
    virtual void SAL_CALL
    impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL
    impl_notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( com::sun::star::uno::RuntimeException );
};

//=========================================================================

class DynamicResultSetWrapperListener
                : public cppu::OWeakObject
                , public com::sun::star::ucb::XDynamicResultSetListener
{
protected:
    DynamicResultSetWrapper*    m_pOwner;
    osl::Mutex                  m_aMutex;

public:
    DynamicResultSetWrapperListener( DynamicResultSetWrapper* pOwner );

    virtual ~DynamicResultSetWrapperListener();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XEventListener ( base of XDynamicResultSetListener )
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );
    //-----------------------------------------------------------------
    // XDynamicResultSetListener
    virtual void SAL_CALL
    notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( com::sun::star::uno::RuntimeException );

    //-----------------------------------------------------------------
    // own methods:
    void SAL_CALL impl_OwnerDies();
};


#endif

