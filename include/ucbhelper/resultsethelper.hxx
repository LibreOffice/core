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

#ifndef INCLUDED_UCBHELPER_RESULTSETHELPER_HXX
#define INCLUDED_UCBHELPER_RESULTSETHELPER_HXX

#include <osl/mutex.hxx>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XDynamicResultSet.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <cppuhelper/weak.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/contenthelper.hxx>
#include <ucbhelper/ucbhelperdllapi.h>

namespace cppu {
    class OInterfaceContainerHelper;
}

namespace ucbhelper {


#define DYNAMICRESULTSET_SERVICE_NAME "com.sun.star.ucb.DynamicResultSet"


/**
  * This is an abstract base class for implementations of the service
  * com.sun.star.ucb.DynamicResultSet, which is the result of the command
  * "open" executed at a UCB folder content.
  *
  * Features of the base class implementation:
  * - standard interfaces ( XInterface, XTypeProvider, XServiceInfo )
  * - all required interfaces for service css::ucb::DynamicResultSet
  */
class UCBHELPER_DLLPUBLIC ResultSetImplHelper :
                public cppu::OWeakObject,
                public css::lang::XTypeProvider,
                public css::lang::XServiceInfo,
                public css::ucb::XDynamicResultSet
{
    cppu::OInterfaceContainerHelper* m_pDisposeEventListeners;
    bool                         m_bStatic;
    bool                         m_bInitDone;

protected:
    osl::Mutex                                                 m_aMutex;
    css::ucb::OpenCommandArgument2                             m_aCommand;
    css::uno::Reference< css::uno::XComponentContext >         m_xContext;
    // Resultset #1
    css::uno::Reference< css::sdbc::XResultSet >               m_xResultSet1;
    // Resultset #2
    css::uno::Reference< css::sdbc::XResultSet >               m_xResultSet2;
    // Resultset changes listener.
    css::uno::Reference< css::ucb::XDynamicResultSetListener > m_xListener;

private:
    UCBHELPER_DLLPRIVATE void init( bool bStatic );

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
      * Constructor.
      *
      * @param rxContext is a Service Manager.
      * @param rCommand is the parameter for the open command that produces
      *        this resultset.
      */
    ResultSetImplHelper(
            const css::uno::Reference<
                css::uno::XComponentContext >& rxContext,
            const css::ucb::OpenCommandArgument2& rCommand );

    /**
      * Destructor.
      */
    virtual ~ResultSetImplHelper();

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL acquire()
        throw() override;
    virtual void SAL_CALL release()
        throw() override;

    // XTypeProvider
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL
    getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw( css::uno::RuntimeException, std::exception ) override;

    static OUString getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

    // XComponent ( base class of XDynamicResultSet )
    virtual void SAL_CALL
    dispose()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    addEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    removeEventListener( const css::uno::Reference< css::lang::XEventListener >& Listener )
        throw( css::uno::RuntimeException, std::exception ) override;

    // XDynamicResultSet
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    getStaticResultSet()
        throw( css::ucb::ListenerAlreadySetException,
        css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    setListener( const css::uno::Reference< css::ucb::XDynamicResultSetListener >& Listener )
        throw( css::ucb::ListenerAlreadySetException,
               css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL
    connectToCache( const css::uno::Reference< css::ucb::XDynamicResultSet > & xCache )
        throw( css::ucb::ListenerAlreadySetException,
               css::ucb::AlreadyInitializedException,
               css::ucb::ServiceNotFoundException,
               css::uno::RuntimeException, std::exception ) override;

    /**
      * The implementation of this method always returns 0. Override this
      * method, if necessary.
      */
    virtual sal_Int16 SAL_CALL
    getCapabilities()
        throw( css::uno::RuntimeException, std::exception ) override;

};

}

#endif /* ! INCLUDED_UCBHELPER_RESULTSETHELPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
