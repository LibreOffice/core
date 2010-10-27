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
#ifndef _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_
#define _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_

#include "java/lang/Object.hxx"
#include "TConnection.hxx"
#include "connectivity/CommonTools.hxx"
#include "OSubComponent.hxx"
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
                The new statement witgh unnamed parameters.
        */
        ::rtl::OUString transFormPreparedStatement(const ::rtl::OUString& _sSQL);
        void loadDriverFromProperties(
                const ::rtl::OUString& _sDriverClass,
                const ::rtl::OUString& _sDriverClassPath,
                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rSystemProperties
            );
        /** load driver class path from system configuration.
            @param  _sDriverClass
                The driver class name to look for in the configuration.
        */
        ::rtl::OUString impl_getJavaDriverClassPath_nothrow(const ::rtl::OUString& _sDriverClass);

    protected:
    // statische Daten fuer die Klasse
        static jclass theClass;

        virtual ~java_sql_Connection();

    public:
        virtual jclass getMyClass() const;

        DECLARE_SERVICE_INFO();
        // ein Konstruktor, der fuer das Returnen des Objektes benoetigt wird:
        java_sql_Connection( const java_sql_Driver& _rDriver );
        sal_Bool construct( const ::rtl::OUString& url,
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
        virtual void SAL_CALL disposing(void);
        // XInterface
        virtual void SAL_CALL release() throw();

        // XConnection
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement > SAL_CALL createStatement(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareStatement( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > SAL_CALL prepareCall( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL nativeSQL( const ::rtl::OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setAutoCommit( sal_Bool autoCommit ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL getAutoCommit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL commit(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rollback(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isClosed(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData > SAL_CALL getMetaData(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setReadOnly( sal_Bool readOnly ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL isReadOnly(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCatalog( const ::rtl::OUString& catalog ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getCatalog(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTransactionIsolation( sal_Int32 level ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual sal_Int32 SAL_CALL getTransactionIsolation(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTypeMap(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setTypeMap( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XCloseable
        virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        // XWarningsSupplier
        virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif // _CONNECTIVITY_JAVA_SQL_CONNECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
