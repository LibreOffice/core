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
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <java/sql/Connection.hxx>
#include <java/sql/ConnectionLog.hxx>

namespace connectivity
{

        typedef ::cppu::WeakComponentImplHelper<       css::sdbc::XStatement,
                                                       css::sdbc::XWarningsSupplier,
                                                       css::util::XCancellable,
                                                       css::sdbc::XCloseable,
                                                       css::sdbc::XGeneratedResultSet,
                                                       css::sdbc::XMultipleResults> java_sql_Statement_BASE;

    //************ Class: java.sql.Statement

    class java_sql_Statement_Base : public cppu::BaseMutex,
                                    public  java_sql_Statement_BASE,
                                    public  java_lang_Object,
                                    public  ::cppu::OPropertySetHelper,
                                    public  ::comphelper::OPropertyArrayUsageHelper<java_sql_Statement_Base>

    {

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getQueryTimeOut();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getMaxFieldSize();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getMaxRows();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getResultSetConcurrency();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getResultSetType();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getFetchDirection();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        sal_Int32 getFetchSize();
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        OUString getCursorName();

        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setQueryTimeOut(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setMaxFieldSize(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setMaxRows(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setResultSetConcurrency(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setResultSetType(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setFetchDirection(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setFetchSize(sal_Int32 _par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setCursorName(const OUString &_par0);
        /// @throws css::sdbc::SQLException
        /// @throws css::uno::RuntimeException
        void setEscapeProcessing(bool _par0);

    protected:
        css::uno::Reference< css::sdbc::XStatement>       m_xGeneratedStatement;
        rtl::Reference<java_sql_Connection>               m_pConnection;
        java::sql::ConnectionLog    m_aLogger;
        OUString             m_sSqlStatement;
        // Properties
        sal_Int32                   m_nResultSetConcurrency;
        sal_Int32                   m_nResultSetType;
        bool                    m_bEscapeProcessing;


    // Static data for the class
        static jclass theClass;

        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
        // OPropertySetHelper
        virtual ::cppu::IPropertyArrayHelper & getInfoHelper() override;

        virtual bool convertFastPropertyValue(
                                    cpo::uno::Any & rConvertedValue,
                                    cpo::uno::Any & rOldValue,
                                    sal_Int32 nHandle,
                                    const cpo::uno::Any& rValue
                                ) override;

        virtual void setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const cpo::uno::Any& rValue
                                ) override;

        virtual void getFastPropertyValue(
                                    cpo::uno::Any& rValue,
                                    sal_Int32 nHandle
                                ) const override;

        virtual void createStatement(JNIEnv* _pEnv) = 0;

        virtual ~java_sql_Statement_Base() override;

        sal_Int32 impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID);
        sal_Int32 impl_getProperty(const char* _pMethodName, jmethodID& _inout_MethodID,sal_Int32 _nDefault);

    public:
        virtual jclass getMyClass() const override;

        // A ctor that is needed for returning the object
        java_sql_Statement_Base( JNIEnv * pEnv, java_sql_Connection& _rCon );

        sal_Int32   getStatementObjectID() const { return m_aLogger.getObjectID(); }

        // OComponentHelper
        virtual void disposing() override;
        // XInterface
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        //XTypeProvider
        virtual cpo::uno::Sequence< cpo::uno::Type > getTypes(  ) override;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;
        // XStatement
        virtual css::uno::Reference< css::sdbc::XResultSet > executeQuery( const OUString& sql ) override ;
        virtual sal_Int32 executeUpdate( const OUString& sql ) override ;
        virtual bool execute( const OUString& sql ) override ;
        virtual css::uno::Reference< css::sdbc::XConnection > getConnection(  ) override ;
        // XWarningsSupplier
        virtual cpo::uno::Any getWarnings(  ) override;
        virtual void clearWarnings(  ) override;
        // XCancellable
        virtual void cancel(  ) override;
        // XCloseable
        virtual void close(  ) override;
        // XMultipleResults
        virtual css::uno::Reference< css::sdbc::XResultSet > getResultSet(  ) override;
        virtual sal_Int32 getUpdateCount(  ) override;
        virtual bool getMoreResults(  ) override;
        //XGeneratedResultSet
        virtual css::uno::Reference< css::sdbc::XResultSet > getGeneratedValues(  ) override;

    public:
        using ::cppu::OPropertySetHelper::getFastPropertyValue;
    };

    class OStatement_BASE2 : public java_sql_Statement_Base

    {
    public:
        OStatement_BASE2(JNIEnv * pEnv, java_sql_Connection& _rCon ) : java_sql_Statement_Base( pEnv, _rCon ) {}

        // OComponentHelper
        virtual void disposing() override;
    };

    class java_sql_Statement :  public OStatement_BASE2,
                                public css::sdbc::XBatchExecution,
                                public css::lang::XServiceInfo
    {
    protected:
        // Static data for the class
        static jclass theClass;

        virtual void createStatement(JNIEnv* _pEnv) override;

        virtual ~java_sql_Statement() override;
    public:
        DECLARE_SERVICE_INFO();
        virtual jclass getMyClass() const override;

        // A ctor that is needed for returning the object
        java_sql_Statement( JNIEnv * pEnv, java_sql_Connection& _rCon ) : OStatement_BASE2( pEnv, _rCon){};

        virtual cpo::uno::Any queryInterface( const cpo::uno::Type & rType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;
        // XBatchExecution
        virtual void addBatch( const OUString& sql ) override;
        virtual void clearBatch(  ) override;
        virtual cpo::uno::Sequence< sal_Int32 > executeBatch(  ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
