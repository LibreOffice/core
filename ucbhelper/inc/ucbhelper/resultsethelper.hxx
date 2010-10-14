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

#ifndef _UCBHELPER_RESULTSETHELPER_HXX
#define _UCBHELPER_RESULTSETHELPER_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/contenthelper.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace cppu {
    class OInterfaceContainerHelper;
}

namespace ucbhelper {

//=========================================================================

#define DYNAMICRESULTSET_SERVICE_NAME "com.sun.star.ucb.DynamicResultSet"

//=========================================================================

/**
  * This is an abstract base class for implementations of the service
  * com.sun.star.ucb.DynamicResultSet, which is the result of the command
  * "open" executed at a UCB folder content.
  *
  * Features of the base class implementation:
  * - standard interfaces ( XInterface, XTypeProvider, XServiceInfo )
  * - all required interfaces for service com::sun::star::ucb::DynamicResultSet
  */
class UCBHELPER_DLLPUBLIC ResultSetImplHelper :
                public cppu::OWeakObject,
                public com::sun::star::lang::XTypeProvider,
                public com::sun::star::lang::XServiceInfo,
                public com::sun::star::ucb::XDynamicResultSet
{
    cppu::OInterfaceContainerHelper* m_pDisposeEventListeners;
    sal_Bool                         m_bStatic;
    sal_Bool                         m_bInitDone;

protected:
    osl::Mutex                                           m_aMutex;
    com::sun::star::ucb::OpenCommandArgument2            m_aCommand;
    com::sun::star::uno::Reference<
        com::sun::star::lang::XMultiServiceFactory >     m_xSMgr;
    // Resultset #1
    com::sun::star::uno::Reference<
        com::sun::star::sdbc::XResultSet >               m_xResultSet1;
    // Resultset #2
    com::sun::star::uno::Reference<
        com::sun::star::sdbc::XResultSet >               m_xResultSet2;
    // Resultset changes listener.
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XDynamicResultSetListener > m_xListener;

private:
    UCBHELPER_DLLPRIVATE void init( sal_Bool bStatic );

    /**
      * Your implementation of this method has to fill the protected member
      * m_xResultSet1. This resultset must implement a complete static
      * resultset ( service com.sun.star.ucb.ContentResultSet ). This method
      * will be called at most once in the life of your implementation object.
      * After this method was called, the type of this resultset will be
      * "static". There is no way to change the type afterwards.
      * If this method gets called the client wants to use your resultset
      * exclusively statically. You may deploy this factum to optimize your
      * implementation (i.e. "switch off" all changes detection code in
      * your implementation).
      * Note that you may use the class ucb::ResultSet to implement the
      * static resultset, that is required here.
      */
    UCBHELPER_DLLPRIVATE virtual void initStatic() = 0;

    /**
      * Your implementation of this method has to fill the protected members
      * m_xResultSet1 and m_xResultSet2 of this base class. Each of these
      * resultsets must implement a complete static resultset
      * ( service com.sun.star.ucb.ContentResultSet ). This method will be
      * called at most once in the life of your implementation object.
      * After this method was called, the type of this resultset will be
      * "dynamic". There is no way to change the type afterwards.
      * If this method gets called the client wants to use your resultset
      * exclusively dynamically. This means, it is interested in getting
      * notifications on changes of data of the resultset contents. ( These
      * changes are to propagate by your implementation throw the member
      * m_xListener of this base class ).
      * If your implementation cannot detect changes of relevant data, you
      * may fill m_xResultSet1 and m_xResultSet2 with the same static resultset
      * implementation object. This normally will be the same instance you put
      * into m_xResultSet1 when initStatic() is called.
      */
    UCBHELPER_DLLPRIVATE virtual void initDynamic() = 0;

public:
    /**
      * Construtor.
      *
      * @param rxSMgr is a Service Manager.
      */
    ResultSetImplHelper(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr );

    /**
      * Construtor.
      *
      * @param rxSMgr is a Service Manager.
      * @param rCommand is the paramter for the open command that produces
      *        this resultset.
      */
    ResultSetImplHelper(
            const com::sun::star::uno::Reference<
                com::sun::star::lang::XMultiServiceFactory >& rxSMgr,
            const com::sun::star::ucb::OpenCommandArgument2& rCommand );

    /**
      * Destructor.
      */
    virtual ~ResultSetImplHelper();

    // XInterface
    XINTERFACE_DECL()

    // XTypeProvider
    XTYPEPROVIDER_DECL()

    // XServiceInfo
    XSERVICEINFO_NOFACTORY_DECL()

    // XComponent ( base class of XDynamicResultSet )
    virtual void SAL_CALL
    dispose()
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    addEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    removeEventListener( const com::sun::star::uno::Reference<
                            com::sun::star::lang::XEventListener >& Listener )
        throw( com::sun::star::uno::RuntimeException );

    // XDynamicResultSet
    virtual com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet > SAL_CALL
    getStaticResultSet()
        throw( com::sun::star::ucb::ListenerAlreadySetException,
        com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    setListener( const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSetListener >& Listener )
        throw( com::sun::star::ucb::ListenerAlreadySetException,
               com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL
    connectToCache( const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > & xCache )
        throw( com::sun::star::ucb::ListenerAlreadySetException,
               com::sun::star::ucb::AlreadyInitializedException,
               com::sun::star::ucb::ServiceNotFoundException,
               com::sun::star::uno::RuntimeException );

    /**
      * The implemetation of this method always returns 0. Override this
      * method, if necassary.
      */
    virtual sal_Int16 SAL_CALL
    getCapabilities()
        throw( com::sun::star::uno::RuntimeException );

    //////////////////////////////////////////////////////////////////////
    // Non-interface methods.
    //////////////////////////////////////////////////////////////////////

    /**
      * This method returns, whether the resultset is static or dynamic.
      * If neither getStatic() nor getDynamic() was called, the type
      * of the resultset is "dynamic".
      *
      * @return true, if the resultset type is "static". False, otherwise.
      */
    sal_Bool isStatic() const { return m_bStatic; }
};

}

#endif /* !_UCBHELPER_RESULTSETHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
