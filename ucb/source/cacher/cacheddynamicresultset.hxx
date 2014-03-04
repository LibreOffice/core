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

#ifndef _CACHED_DYNAMIC_RESULTSET_HXX
#define _CACHED_DYNAMIC_RESULTSET_HXX

#include <dynamicresultsetwrapper.hxx>
#include <com/sun/star/ucb/XContentIdentifierMapping.hpp>
#include <com/sun/star/ucb/XCachedDynamicResultSetFactory.hpp>

#define CACHED_DRS_SERVICE_NAME "com.sun.star.ucb.CachedDynamicResultSet"
#define CACHED_DRS_FACTORY_NAME "com.sun.star.ucb.CachedDynamicResultSetFactory"



class CachedDynamicResultSet
                : public DynamicResultSetWrapper
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
{
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifierMapping >
                            m_xContentIdentifierMapping;

protected:
    virtual void SAL_CALL
    impl_InitResultSetOne( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );
    virtual void SAL_CALL
    impl_InitResultSetTwo( const com::sun::star::uno::Reference<
                com::sun::star::sdbc::XResultSet >& xResultSet );

public:
    CachedDynamicResultSet( com::sun::star::uno::Reference<
                        com::sun::star::ucb::XDynamicResultSet > xOrigin
        , const com::sun::star::uno::Reference<
                        com::sun::star::ucb::XContentIdentifierMapping > & xContentMapping
        , const com::sun::star::uno::Reference<
                        com::sun::star::uno::XComponentContext > & xContext );

    virtual ~CachedDynamicResultSet();


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider

    XTYPEPROVIDER_DECL()

    // XServiceInfo

    XSERVICEINFO_NOFACTORY_DECL()


    // own methods ( inherited )

    virtual void SAL_CALL
    impl_disposing( const com::sun::star::lang::EventObject& Source )
        throw( com::sun::star::uno::RuntimeException );
};



class CachedDynamicResultSetFactory
                : public cppu::OWeakObject
                , public com::sun::star::lang::XTypeProvider
                , public com::sun::star::lang::XServiceInfo
                , public com::sun::star::ucb::XCachedDynamicResultSetFactory
{
protected:
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >    m_xContext;

public:

    CachedDynamicResultSetFactory(
        const com::sun::star::uno::Reference<
           com::sun::star::uno::XComponentContext > & xContext);

    virtual ~CachedDynamicResultSetFactory();


    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
        throw( css::uno::RuntimeException, std::exception );
    virtual void SAL_CALL acquire()
        throw();
    virtual void SAL_CALL release()
        throw();

    // XTypeProvider
    XTYPEPROVIDER_DECL()


    // XServiceInfo
    XSERVICEINFO_DECL()


    // XCachedDynamicResultSetFactory

    virtual com::sun::star::uno::Reference<
                com::sun::star::ucb::XDynamicResultSet > SAL_CALL
    createCachedDynamicResultSet(
                  const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XDynamicResultSet > &
                        SourceStub
                , const com::sun::star::uno::Reference<
                    com::sun::star::ucb::XContentIdentifierMapping > &
                        ContentIdentifierMapping
                )
            throw( com::sun::star::uno::RuntimeException, std::exception );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
