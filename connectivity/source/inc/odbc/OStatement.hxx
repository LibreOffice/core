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

#pragma once

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XBatchExecution.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/XGeneratedResultSet.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <connectivity/CommonTools.hxx>
#include <odbc/OFunctions.hxx>
#include <odbc/OConnection.hxx>
#include <odbc/odbcbasedllapi.hxx>
#include <string_view>
#include <vector>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity::odbc
    {

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XStatement,
                                                   css::sdbc::XWarningsSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XCloseable,
                                                   css::sdbc::XGeneratedResultSet,
                                                   css::sdbc::XMultipleResults> OStatement_BASE;

        class OResultSet;

        //************ Class: java.sql.Statement

        class OOO_DLLPUBLIC_ODBCBASE OStatement_Base :
                                        public cppu::BaseMutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>

        {
            css::sdbc::SQLWarning                             m_aLastWarning;
        protected:
            css::uno::WeakReference< css::sdbc::XResultSet>   m_xResultSet;   // The last ResultSet created
            css::uno::Reference< css::sdbc::XStatement>       m_xGeneratedStatement;
            //  for this Statement

            std::vector< OUString>   m_aBatchVector;
            OUString                 m_sSqlStatement;

            rtl::Reference<OConnection>     m_pConnection;// The owning Connection object
            SQLHANDLE                       m_aStatementHandle;
            SQLUSMALLINT*                   m_pRowStatusArray;

        protected:

            sal_Int64 getQueryTimeOut()         const;
            sal_Int64 getMaxFieldSize()         const;
            sal_Int64 getMaxRows()              const;
            sal_Int32 getResultSetConcurrency() const;
            sal_Int32 getResultSetType()        const;
            sal_Int32 getFetchDirection()       const;
            sal_Int32 getFetchSize()            const;
            OUString getCursorName()     const;
            bool isUsingBookmarks()         const;
            bool getEscapeProcessing()      const;
            template < typename T, SQLINTEGER BufferLength > T getStmtOption (SQLINTEGER fOption) const;

            void setQueryTimeOut(sal_Int64 _par0)           ;
            void setMaxFieldSize(sal_Int64 _par0)           ;
            void setMaxRows(sal_Int64 _par0)                ;
            void setFetchDirection(sal_Int32 _par0)         ;
            void setFetchSize(sal_Int32 _par0)              ;
            void setCursorName(std::u16string_view _par0);
            void setEscapeProcessing( const bool _bEscapeProc );
            template < typename T, SQLINTEGER BufferLength > SQLRETURN setStmtOption (SQLINTEGER fOption, T value) const;

            void setResultSetConcurrency(sal_Int32 _par0)   ;
            void setResultSetType(sal_Int32 _par0)          ;
            void setUsingBookmarks(bool _bUseBookmark)  ;

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void reset();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void clearMyResultSet();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            void setWarning (const css::sdbc::SQLWarning &ex);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            bool lockIfNecessary (const OUString& sql);
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getColumnCount();


            // getResultSet
            // getResultSet returns the current result as a ResultSet.  It
            // returns NULL if the current result is not a ResultSet.

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            css::uno::Reference<css::sdbc::XResultSet> getResultSet(bool checkCount);
            /**
                creates the driver specific resultset (factory)
            */
            virtual rtl::Reference<OResultSet> createResultSet();

            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            SQLLEN getRowCount();


            void disposeResultSet();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                css::uno::Any & rConvertedValue,
                                css::uno::Any & rOldValue,
                                sal_Int32 nHandle,
                                const css::uno::Any& rValue ) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const css::uno::Any& rValue
                                                     ) override;
            virtual void SAL_CALL getFastPropertyValue(
                                    css::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const override;
            virtual ~OStatement_Base() override;

        public:
            OStatement_Base(OConnection* _pConnection );
            using OStatement_BASE::operator css::uno::Reference< css::uno::XInterface >;

            oslGenericFunction getOdbcFunction(ODBC3SQLFunctionId _nIndex)  const
            {
                return m_pConnection->getOdbcFunction(_nIndex);
            }
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL release() throw() override;
            virtual void SAL_CALL acquire() throw() override;
            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) override ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) override ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) override ;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override ;
            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;
            // XCancellable
            virtual void SAL_CALL cancel(  ) override;
            // XCloseable
            virtual void SAL_CALL close(  ) override;
            // XMultipleResults
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSet(  ) override;
            virtual sal_Int32 SAL_CALL getUpdateCount(  ) override;
            virtual sal_Bool SAL_CALL getMoreResults(  ) override;
            //XGeneratedResultSet
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getGeneratedValues(  ) override;

            // other methods
            SQLHANDLE getConnectionHandle() { return m_pConnection->getConnection(); }
            OConnection* getOwnConnection() const { return m_pConnection.get();}
            /** getCursorProperties return the properties for a specific cursor type
                @param _nCursorType     the CursorType
                @param bFirst           when true the first property set is returned

                @return the cursor properties
            */
            SQLUINTEGER getCursorProperties(SQLINTEGER _nCursorType, bool bFirst);

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };

        class OOO_DLLPUBLIC_ODBCBASE OStatement_BASE2 : public OStatement_Base
        {
        public:
            OStatement_BASE2(OConnection* _pConnection ) :
                OStatement_Base(_pConnection )
            {}
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
        };

        class OOO_DLLPUBLIC_ODBCBASE OStatement :
                            public OStatement_BASE2,
                            public css::sdbc::XBatchExecution,
                            public css::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement() override {}
        public:
            // A ctor that is needed for returning the object
            OStatement( OConnection* _pConnection) : OStatement_BASE2( _pConnection){}
            DECLARE_SERVICE_INFO();

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            // XBatchExecution
            virtual void SAL_CALL addBatch( const OUString& sql ) override;
            virtual void SAL_CALL clearBatch(  ) override;
            virtual css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
