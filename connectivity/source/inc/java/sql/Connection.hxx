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
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#define _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_

#include "java/lang/Object.hxx"
#include "TConnection.hxx"
#include "connectivity/CommonTools.hxx"
#include "connectivity/OSubComponent.hxx"
#include <cppuhelper/weakref.hxx>
#include "AutoRetrievingBase.hxx"
#include "java/sql/ConnectionLog.hxx"
#include "java/LocalRef.hxx"
#include "java/GlobalRef.hxx"

#include <com/sun/star/beans/NamedValue.hpp>

namespace connectivity
{
    class java_sql_Driver;

    typedef OMetaConnection     java_sql_Connection_BASE;

    class java_sql_Connection : public java_sql_Connection_BASE,
                                public java_lang_Object,
                                public OSubComponent<java_sql_Connection, java_sql_Connection_BASE>,
                                public OAutoRetrievingBase
    {
        friend class OSubComponent<java_sql_Connection, java_sql_Connection_BASE>;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        const java_sql_Driver*  m_pDriver;
        jobject                 m_pDriverobject;
        jdbc::GlobalRef< jobject >
                                m_pDriverClassLoader;

        jclass                  m_Driver_theClass;
        java::sql::ConnectionLog
                                m_aLogger;
        sal_Bool                m_bParameterSubstitution;
        sal_Bool                m_bIgnoreDriverPrivileges;
        sal_Bool                m_bIgnoreCurrency;
        ::com::sun::star::uno::Any  m_aCatalogRestriction;
        ::com::sun::star::uno::Any  m_aSchemaRestriction;

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
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rSystemProperties
            );
        /** load driver class path from system configuration.
            @param  _sDriverClass
                The driver class name to look for in the configuration.
        */
        OUString impl_getJavaDriverClassPath_nothrow(const OUString& _sDriverClass);

    protected:
    // Static data for the class
        static jclass theClass;

        virtual ~java_sql_Connection();

    public:
        virtual jclass getMyClass() const SAL_OVERRIDE;

        DECLARE_SERVICE_INFO();
        // A ctor that is needed for returning the object
        java_sql_Connection( const java_sql_Driver& _rDriver );
        sal_Bool construct( const OUString& url,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& info);

        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&
            getConnectionInfo() const { return m_aConnectionInfo; }

        inline  sal_Bool isIgnoreDriverPrivilegesEnabled() const { return   m_bIgnoreDriverPrivileges;}
        inline  sal_Bool isIgnoreCurrencyEnabled() const { return   m_bIgnoreCurrency; }
        inline const ::com::sun::star::uno::Any& getCatalogRestriction() const { return m_aCatalogRestriction; }
        inline const ::com::sun::star::uno::Any& getSchemaRestriction() const { return m_aSchemaRestriction; }

        /** returns the instance used for logging events related to this connection
        */
        const java::sql::ConnectionLog& getLogger() const { return m_aLogger; }

        /** returns the class loader which was used to load the driver class

            Usually used in conjunction with a ContextClassLoaderScope instance.
        */
        const jdbc::GlobalRef< jobject >& getDriverClassLoader() const { return m_pDriverClassLoader; }

        // OComponentHelper
        virtual void SAL_CALL disposing(void) SAL_OVERRIDE;
        // XInterface
        virtual void SAL_CALL release() throw() SAL_OVERRIDE;

        // XConnection
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL nativeSQL( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setCatalog( const OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        // XWarningsSupplier
        virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
