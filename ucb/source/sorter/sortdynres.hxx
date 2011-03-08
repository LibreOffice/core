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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
