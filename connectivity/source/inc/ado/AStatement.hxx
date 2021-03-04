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
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/proparrhlp.hxx>
#include <ado/AConnection.hxx>
#include <string_view>
#include <vector>
#include <ado/Awrapado.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

namespace connectivity::ado
{
        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XStatement,
                                                   css::sdbc::XWarningsSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XCloseable,
                                                   css::sdbc::XMultipleResults> OStatement_BASE;


        //************ Class: java.sql.Statement

        class OStatement_Base       :   public cppu::BaseMutex,
                                        public  OStatement_BASE,
                                        public  ::cppu::OPropertySetHelper,
                                        public  ::comphelper::OPropertyArrayUsageHelper<OStatement_Base>

        {
            friend class OResultSet;

            css::sdbc::SQLWarning          m_aLastWarning;

        protected:
            std::vector< OUString>               m_aBatchVector;

            css::uno::WeakReference< css::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
                                                                        //  for this Statement

            OConnection*                                m_pConnection;// The owning Connection object
            WpADOCommand                                m_Command;
            WpADORecordset                              m_RecordSet;
            OLEVariant                                  m_RecordsAffected;
            OLEVariant                                  m_Parameters;
            std::vector<connectivity::OTypeInfo>      m_aTypeInfo;    // Hashtable containing an entry
                                                                        //  for each row returned by
                                                                        //  DatabaseMetaData.getTypeInfo.
            ADO_LONGPTR                                 m_nMaxRows;
            sal_Int32                                   m_nFetchSize;
            LockTypeEnum                                m_eLockType;
            CursorTypeEnum                              m_eCursorType;

            using OStatement_BASE::rBHelper;
        private:
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getQueryTimeOut()         const;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static sal_Int32 getMaxFieldSize();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getMaxRows()              const;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getResultSetConcurrency() const;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getResultSetType()        const;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            static sal_Int32 getFetchDirection();
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            sal_Int32 getFetchSize()            const;
            /// @throws css::sdbc::SQLException
            /// @throws css::uno::RuntimeException
            OUString getCursorName()     const;

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
            void setCursorName(std::u16string_view _par0);

        protected:

            void assignRecordSet( ADORecordset* _pRS );

            /// @throws css::sdbc::SQLException
            void reset ();
            /// @throws css::sdbc::SQLException
            void clearMyResultSet ();
            /// @throws css::sdbc::SQLException
            void setWarning (const  css::sdbc::SQLWarning &ex);
            /// @throws css::sdbc::SQLException
            sal_Int32 getRowCount ();
            sal_Int32 getPrecision ( sal_Int32 sqlType);

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
        public:
            OStatement_Base(OConnection* _pConnection );

            using OStatement_BASE::operator css::uno::Reference< css::uno::XInterface >;
            // OComponentHelper
            virtual void SAL_CALL disposing() override;
            // XInterface
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;

            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) override;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) override;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override;
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
        };

        class OStatement :  public OStatement_Base,
                            public css::sdbc::XBatchExecution,
                            public css::lang::XServiceInfo
        {
        public:
            // a Constructor, that is needed for when Returning the Object is needed:
            OStatement( OConnection* _pConnection) : OStatement_Base( _pConnection){};
            ~OStatement() override;

            DECLARE_SERVICE_INFO();

            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            // XBatchExecution
            virtual void SAL_CALL addBatch( const OUString& sql ) override;
            virtual void SAL_CALL clearBatch(  ) override;
            virtual css::uno::Sequence< sal_Int32 > SAL_CALL executeBatch(  ) override;
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
