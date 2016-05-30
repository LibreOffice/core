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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABSTATEMENT_HXX

#include "MacabConnection.hxx"
#include "MacabHeader.hxx"
#include <list>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/proparrhlp.hxx>

namespace connectivity
{
    namespace macab
    {
        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XStatement,
                                                   css::sdbc::XWarningsSupplier,
                                                   css::util::XCancellable,
                                                   css::sdbc::XCloseable> MacabCommonStatement_BASE;


        // Class MacabCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class MacabCommonStatement :    public comphelper::OBaseMutex,
                        public  MacabCommonStatement_BASE,
                        public  ::cppu::OPropertySetHelper,
                        public  comphelper::OPropertyArrayUsageHelper<MacabCommonStatement>

        {
            css::sdbc::SQLWarning               m_aLastWarning;

        protected:
            ::std::list< OUString>              m_aBatchList;
            connectivity::OSQLParser            m_aParser;
            connectivity::OSQLParseTreeIterator m_aSQLIterator;
            connectivity::OSQLParseNode*        m_pParseTree;
            MacabConnection*                    m_pConnection;  // The owning Connection object
            MacabHeader*                        m_pHeader;  // The header of the address book on which to run queries (provided by m_pConnection)
            css::uno::WeakReference< css::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created


        protected:
            class MacabCondition *analyseWhereClause(
                const OSQLParseNode *pParseNode) const throw(css::sdbc::SQLException);
            class MacabOrder *analyseOrderByClause(
                const OSQLParseNode *pParseNode) const throw(css::sdbc::SQLException);
            OUString getTableName( ) const;
            void setMacabFields(class MacabResultSet *pResult) const throw(css::sdbc::SQLException);
            void selectRecords(MacabResultSet *pResult) const throw(css::sdbc::SQLException);
            void sortRecords(MacabResultSet *pResult) const throw(css::sdbc::SQLException);

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;

            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
            virtual sal_Bool SAL_CALL convertFastPropertyValue(
                    css::uno::Any & rConvertedValue,
                    css::uno::Any & rOldValue,
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue) throw (css::lang::IllegalArgumentException) override;
            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                    sal_Int32 nHandle,
                    const css::uno::Any& rValue) throw (css::uno::Exception) override;
            virtual void SAL_CALL getFastPropertyValue(
                    css::uno::Any& rValue,
                    sal_Int32 nHandle) const override;

            virtual void resetParameters() const throw(css::sdbc::SQLException);
            virtual void getNextParameter(OUString &rParameter) const throw(css::sdbc::SQLException);
            virtual ~MacabCommonStatement();

        public:
            ::cppu::OBroadcastHelper& rBHelper;

            explicit MacabCommonStatement(MacabConnection *_pConnection);
            using MacabCommonStatement_BASE::operator css::uno::Reference< css::uno::XInterface >;

            // OComponentHelper
            virtual void SAL_CALL disposing() override;

            // XInterface
            virtual void SAL_CALL release() throw() override;
            virtual void SAL_CALL acquire() throw() override;
            virtual css::uno::Any SAL_CALL queryInterface(
                    const css::uno::Type & rType
                    ) throw(css::uno::RuntimeException, std::exception) override;

            // XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(
                    ) throw(css::uno::RuntimeException, std::exception) override;

            // XPropertySet
            virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(
                    ) throw(css::uno::RuntimeException) override;

            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery(
                    const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Int32 SAL_CALL executeUpdate(
                     const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual sal_Bool SAL_CALL execute(
                    const OUString& sql ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(
                    ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(
                    ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;
            virtual void SAL_CALL clearWarnings(
                    ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            // XCancellable
            virtual void SAL_CALL cancel(
                    ) throw(css::uno::RuntimeException) override;

            // XCloseable
            virtual void SAL_CALL close(
                    ) throw(css::sdbc::SQLException, css::uno::RuntimeException) override;

            // other methods
            inline MacabConnection* getOwnConnection() const { return m_pConnection; }
        };


        // Class MacabStatement

        typedef ::cppu::ImplInheritanceHelper<
                MacabCommonStatement, css::lang::XServiceInfo > MacabStatement_BASE;

        class MacabStatement : public MacabStatement_BASE
        {
        protected:
            virtual ~MacabStatement() { }

        public:
            explicit MacabStatement(MacabConnection* _pConnection);
            DECLARE_SERVICE_INFO();
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MACAB_MACABSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
