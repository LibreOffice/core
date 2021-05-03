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

#include "MacabConnection.hxx"
#include "MacabHeader.hxx"
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/proparrhlp.hxx>

namespace connectivity::macab
{
        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XStatement,
                                                   css::sdbc::XWarningsSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XCloseable> MacabCommonStatement_BASE;


        // Class MacabCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class MacabCommonStatement :    public cppu::BaseMutex,
                        public  MacabCommonStatement_BASE,
                        public  ::cppu::OPropertySetHelper,
                        public  comphelper::OPropertyArrayUsageHelper<MacabCommonStatement>

        {
            css::sdbc::SQLWarning               m_aLastWarning;

        protected:
            connectivity::OSQLParser            m_aParser;
            connectivity::OSQLParseTreeIterator m_aSQLIterator;
            connectivity::OSQLParseNode*        m_pParseTree;
            MacabConnection*                    m_pConnection;  // The owning Connection object
            MacabHeader*                        m_pHeader;  // The header of the address book on which to run queries (provided by m_pConnection)
            css::uno::WeakReference< css::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created


        protected:
            /// @throws css::sdbc::SQLException
            class MacabCondition *analyseWhereClause(
                const OSQLParseNode *pParseNode) const;
            /// @throws css::sdbc::SQLException
            class MacabOrder *analyseOrderByClause(
                const OSQLParseNode *pParseNode) const;
            OUString getTableName( ) const;
            /// @throws css::sdbc::SQLException
            void setMacabFields(class MacabResultSet *pResult) const;
            /// @throws css::sdbc::SQLException
            void selectRecords(MacabResultSet *pResult) const;
            /// @throws css::sdbc::SQLException
            void sortRecords(MacabResultSet *pResult) const;

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                    css::uno::Any & rConvertedValue,
                    css::uno::Any & rOldValue,
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue) override;
            virtual void SAL_CALL getFastPropertyValue(
                    css::uno::Any& rValue,
                    sal_Int32 nHandle) const override;

            /// @throws css::sdbc::SQLException
            virtual void resetParameters() const;
            /// @throws css::sdbc::SQLException
            virtual void getNextParameter(OUString &rParameter) const;
            virtual ~MacabCommonStatement() override;

        public:
            using MacabCommonStatement_BASE::rBHelper;

            explicit MacabCommonStatement(MacabConnection *_pConnection);
            using MacabCommonStatement_BASE::operator css::uno::Reference< css::uno::XInterface >;

            // OComponentHelper
            using MacabCommonStatement_BASE::disposing;

            // XInterface
            virtual void SAL_CALL release() noexcept override;
            virtual void SAL_CALL acquire() noexcept override;
            virtual css::uno::Any SAL_CALL queryInterface(
                    const css::uno::Type & rType
                    ) override;

            // XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(
                    ) override;

            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(
                    ) override;

            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(
                    const OUString& sql ) override;
            virtual sal_Int32 SAL_CALL executeUpdate(
                     const OUString& sql ) override;
            virtual sal_Bool SAL_CALL execute(
                    const OUString& sql ) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(
                    ) override;

            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(
                    ) override;
            virtual void SAL_CALL clearWarnings(
                    ) override;

            // XCancellable
            virtual void SAL_CALL cancel(
                    ) override;

            // XCloseable
            virtual void SAL_CALL close(
                    ) override;

            // other methods
            MacabConnection* getOwnConnection() const { return m_pConnection; }
        };


        // Class MacabStatement

        typedef ::cppu::ImplInheritanceHelper<
                MacabCommonStatement, css::lang::XServiceInfo > MacabStatement_BASE;

        class MacabStatement : public MacabStatement_BASE
        {
        protected:
            virtual ~MacabStatement() override { }

        public:
            explicit MacabStatement(MacabConnection* _pConnection);
            DECLARE_SERVICE_INFO();
        };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
