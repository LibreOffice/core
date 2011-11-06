/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SORTDYNRES_HXX
#define _SORTDYNRES_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XDynamicResultSetListener.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/XSortedDynamicResultSetFactory.hpp>
#include <cppuhelper/weak.hxx>
#include <osl/mutex.hxx>
#include <ucbhelper/macros.hxx>
#include "sortresult.hxx"

namespace cppu {
    class OInterfaceContainerHelper;
}

//-----------------------------------------------------------------------------

#define NUMBERED_SORTINGINFO        com::sun::star::ucb::NumberedSortingInfo
#define RUNTIMEEXCEPTION            com::sun::star::uno::RuntimeException
#define REFERENCE                   com::sun::star::uno::Reference
#define SEQUENCE                    com::sun::star::uno::Sequence
#define EVENTOBJECT                 com::sun::star::lang::EventObject
#define XEVENTLISTENER              com::sun::star::lang::XEventListener
#define XMULTISERVICEFACTORY        com::sun::star::lang::XMultiServiceFactory
#define XRESULTSET                  com::sun::star::sdbc::XResultSet
#define SQLEXCEPTION                com::sun::star::sdbc::SQLException
#define XANYCOMPAREFACTORY          com::sun::star::ucb::XAnyCompareFactory
#define XDYNAMICRESULTSET           com::sun::star::ucb::XDynamicResultSet
#define XDYNAMICRESULTSETLISTENER   com::sun::star::ucb::XDynamicResultSetListener
#define LISTENERALREADYSETEXCEPTION com::sun::star::ucb::ListenerAlreadySetException

#define DYNAMIC_RESULTSET_SERVICE_NAME  "com.sun.star.ucb.SortedDynamicResultSet"
#define DYNAMIC_RESULTSET_FACTORY_NAME  "com.sun.star.ucb.SortedDynamicResultSetFactory"

//-----------------------------------------------------------------------------
class SortedDynamicResultSetListener;

class SortedDynamicResultSet:
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XDynamicResultSet
{
    cppu::OInterfaceContainerHelper *mpDisposeEventListeners;

    REFERENCE < XDYNAMICRESULTSETLISTENER > mxListener;
    REFERENCE < XDYNAMICRESULTSETLISTENER > mxOwnListener;

    REFERENCE < XRESULTSET >            mxOne;
    REFERENCE < XRESULTSET >            mxTwo;
    REFERENCE < XDYNAMICRESULTSET >     mxOriginal;
    SEQUENCE  < NUMBERED_SORTINGINFO >  maOptions;
    REFERENCE < XANYCOMPAREFACTORY >    mxCompFac;
    REFERENCE < XMULTISERVICEFACTORY >  mxSMgr;

    SortedResultSet*                    mpOne;
    SortedResultSet*                    mpTwo;
    SortedDynamicResultSetListener*     mpOwnListener;

    EventList                           maActions;
    osl::Mutex                          maMutex;
    sal_Bool                            mbGotWelcome    :1;
    sal_Bool                            mbUseOne        :1;
    sal_Bool                            mbStatic        :1;

private:

    void                SendNotify();

public:
    SortedDynamicResultSet( const REFERENCE < XDYNAMICRESULTSET >    &xOriginal,
                            const SEQUENCE  < NUMBERED_SORTINGINFO > &aOptions,
                            const REFERENCE < XANYCOMPAREFACTORY >   &xCompFac,
                            const REFERENCE < XMULTISERVICEFACTORY > &xSMgr );

    ~SortedDynamicResultSet();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_NOFACTORY_DECL()

    //-----------------------------------------------------------------
    // XComponent
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    dispose() throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    addEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    virtual void SAL_CALL
    removeEventListener( const REFERENCE< XEVENTLISTENER >& Listener )
        throw( RUNTIME_EXCEPTION );

    //-----------------------------------------------------------------
    // XDynamicResultSet
    //-----------------------------------------------------------------
    virtual REFERENCE< XRESULTSET > SAL_CALL
    getStaticResultSet(  )
        throw( LISTENERALREADYSETEXCEPTION, RUNTIMEEXCEPTION );

    virtual void SAL_CALL
    setListener( const REFERENCE< XDYNAMICRESULTSETLISTENER >& Listener )
        throw( LISTENERALREADYSETEXCEPTION, RUNTIMEEXCEPTION );

    virtual void SAL_CALL
    connectToCache( const REFERENCE< XDYNAMICRESULTSET > & xCache )
        throw( LISTENERALREADYSETEXCEPTION,
               com::sun::star::ucb::AlreadyInitializedException,
               com::sun::star::ucb::ServiceNotFoundException,
               RUNTIMEEXCEPTION );

    virtual sal_Int16 SAL_CALL
    getCapabilities()
        throw( RUNTIMEEXCEPTION );

    //-----------------------------------------------------------------
    // own methods:
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    impl_disposing( const EVENTOBJECT& Source )
        throw( RUNTIMEEXCEPTION );

    virtual void SAL_CALL
    impl_notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( RUNTIMEEXCEPTION );
};

//-----------------------------------------------------------------------------

class SortedDynamicResultSetListener:
                public cppu::OWeakObject,
                public com::sun::star::ucb::XDynamicResultSetListener
{
    SortedDynamicResultSet  *mpOwner;
    osl::Mutex              maMutex;

public:
     SortedDynamicResultSetListener( SortedDynamicResultSet *mOwner );
    ~SortedDynamicResultSetListener();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XEventListener ( base of XDynamicResultSetListener )
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    disposing( const EVENTOBJECT& Source )
        throw( RUNTIMEEXCEPTION );

    //-----------------------------------------------------------------
    // XDynamicResultSetListener
    //-----------------------------------------------------------------
    virtual void SAL_CALL
    notify( const ::com::sun::star::ucb::ListEvent& Changes )
        throw( RUNTIMEEXCEPTION );

    //-----------------------------------------------------------------
    // own methods:
    //-----------------------------------------------------------------
    void SAL_CALL impl_OwnerDies();
};

//-----------------------------------------------------------------------------

class SortedDynamicResultSetFactory:
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XSortedDynamicResultSetFactory
{

    REFERENCE< XMULTISERVICEFACTORY >   mxSMgr;

public:

    SortedDynamicResultSetFactory(
        const REFERENCE< XMULTISERVICEFACTORY > & rSMgr);

    ~SortedDynamicResultSetFactory();

    //-----------------------------------------------------------------
    // XInterface
    //-----------------------------------------------------------------
    XINTERFACE_DECL()

    //-----------------------------------------------------------------
    // XTypeProvider
    //-----------------------------------------------------------------
    XTYPEPROVIDER_DECL()

    //-----------------------------------------------------------------
    // XServiceInfo
    //-----------------------------------------------------------------
    XSERVICEINFO_DECL()

    //-----------------------------------------------------------------
    // XSortedDynamicResultSetFactory

    virtual REFERENCE< XDYNAMICRESULTSET > SAL_CALL
    createSortedDynamicResultSet(
                const REFERENCE< XDYNAMICRESULTSET > & Source,
                const SEQUENCE< NUMBERED_SORTINGINFO > & Info,
                const REFERENCE< XANYCOMPAREFACTORY > & CompareFactory )
        throw( RUNTIMEEXCEPTION );
};

#endif
