/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#pragma once

#include <java/lang/Object.hxx>
#include <TConnection.hxx>
#include <connectivity/CommonTools.hxx>
#include <AutoRetrievingBase.hxx>
#include <java/sql/ConnectionLog.hxx>
#include <java/GlobalRef.hxx>

#include <com/sun/star/beans/NamedValue.hpp>

namespace connectivity
{
    class java_sql_Driver;

    typedef OMetaConnection     java_sql_Connection_BASE;

    class java_sql_Connection : public java_sql_Connection_BASE,
                                public java_lang_Object,
                                public OAutoRetrievingBase
    {
        css::uno::Reference< css::uno::XComponentContext > m_xContext;
        const java_sql_Driver*  m_pDriver;
        jobject                 m_pDriverobject;
        jdbc::GlobalRef< jobject >
                                m_pDriverClassLoader;

        jclass                  m_Driver_theClass;
        java::sql::ConnectionLog
                                m_aLogger;
        bool                    m_bIgnoreDriverPrivileges;
        bool                    m_bIgnoreCurrency;
        cpo::uno::Any           m_aCatalogRestriction;
        cpo::uno::Any           m_aSchemaRestriction;

        /** transform named parameter into unnamed one.
            @param  _sSQL
                The SQL statement to transform.
            @return
                The new statement with unnamed parameters.
        */
        OUString transFormPreparedStatement(const OUString& _sSQL);
        void loadDriverFromProperties(
                const OUString& _sDriverClass,
                const OUString& _sDriverClassPath,
                const cpo::uno::Sequence< css::beans::NamedValue >& _rSystemProperties
            );
        /** load driver class path from system configuration.
            @param  _sDriverClass
                The driver class name to look for in the configuration.
        */
        OUString impl_getJavaDriverClassPath_nothrow(const OUString& _sDriverClass);

    protected:
    // Static data for the class
        static jclass theClass;

        virtual ~java_sql_Connection() override;

    public:
        virtual jclass getMyClass() const override;

        DECLARE_SERVICE_INFO();
        // A ctor that is needed for returning the object
        java_sql_Connection( const java_sql_Driver& _rDriver );
        bool construct( const OUString& url,
                        const cpo::uno::Sequence< css::beans::PropertyValue >& info);

        const cpo::uno::Sequence< css::beans::PropertyValue >&
            getConnectionInfo() const { return m_aConnectionInfo; }

        bool isIgnoreDriverPrivilegesEnabled() const { return   m_bIgnoreDriverPrivileges;}
        bool isIgnoreCurrencyEnabled() const { return   m_bIgnoreCurrency; }
        const cpo::uno::Any& getCatalogRestriction() const { return m_aCatalogRestriction; }
        const cpo::uno::Any& getSchemaRestriction() const { return m_aSchemaRestriction; }

        /** returns the instance used for logging events related to this connection
        */
        const java::sql::ConnectionLog& getLogger() const { return m_aLogger; }

        /** returns the class loader which was used to load the driver class

            Usually used in conjunction with a ContextClassLoaderScope instance.
        */
        const jdbc::GlobalRef< jobject >& getDriverClassLoader() const { return m_pDriverClassLoader; }

        // OComponentHelper
        virtual void disposing() override;

        // XConnection
        virtual css::uno::Reference< css::sdbc::XStatement > createStatement(  ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareStatement( const OUString& sql ) override;
        virtual css::uno::Reference< css::sdbc::XPreparedStatement > prepareCall( const OUString& sql ) override;
        virtual OUString nativeSQL( const OUString& sql ) override;
        virtual void setAutoCommit( bool autoCommit ) override;
        virtual bool getAutoCommit(  ) override;
        virtual void commit(  ) override;
        virtual void rollback(  ) override;
        virtual bool isClosed(  ) override;
        virtual css::uno::Reference< css::sdbc::XDatabaseMetaData > getMetaData(  ) override;
        virtual void setReadOnly( bool readOnly ) override;
        virtual bool isReadOnly(  ) override;
        virtual void setCatalog( const OUString& catalog ) override;
        virtual OUString getCatalog(  ) override;
        virtual void setTransactionIsolation( sal_Int32 level ) override;
        virtual sal_Int32 getTransactionIsolation(  ) override;
        virtual css::uno::Reference< css::container::XNameAccess > getTypeMap(  ) override;
        virtual void setTypeMap( const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;
        // XCloseable
        virtual void close(  ) override;
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
