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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MSTATEMENT_HXX

#include <comphelper/proparrhlp.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <TSortIndex.hxx>
#include "MTable.hxx"

#include <memory>

namespace connectivity
{
    namespace mork
    {
        class OResultSet;

        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XStatement,
                                                   css::sdbc::XWarningsSupplier,
                                                   css::sdbc::XCloseable> OCommonStatement_IBASE;


        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class OCommonStatement;

        class OCommonStatement  :public cppu::BaseMutex
                                ,public OCommonStatement_IBASE
                                ,public ::cppu::OPropertySetHelper
                                ,public ::comphelper::OPropertyArrayUsageHelper< OCommonStatement >
        {
        private:
            css::sdbc::SQLWarning                              m_aLastWarning;

        protected:
            css::uno::WeakReference< css::sdbc::XResultSet >   m_xResultSet;
            css::uno::Reference< css::sdbc::XDatabaseMetaData> m_xDBMetaData;
            css::uno::Reference< css::container::XNameAccess>  m_xColNames; // table columns

            //  for this Statement

            OTable*                                     m_pTable;
            rtl::Reference<OConnection>                 m_pConnection;  // The owning Connection object

            OValueRow                                   m_aRow;

            connectivity::OSQLParser                    m_aParser;
            std::shared_ptr< ::connectivity::OSQLParseTreeIterator >
                                                        m_pSQLIterator;

            std::unique_ptr<connectivity::OSQLParseNode> m_pParseTree;

            std::vector<sal_Int32>                    m_aColMapping;
            std::vector<sal_Int32>                    m_aOrderbyColumnNumber;
            std::vector<TAscendingOrder>              m_aOrderbyAscending;

        protected:

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
                                                                const css::uno::Any& rValue) override;
            virtual void SAL_CALL getFastPropertyValue(
                                                                css::uno::Any& rValue,
                                                                sal_Int32 nHandle) const override;
            virtual ~OCommonStatement() override;

        protected:

            // Driver Internal Methods

            enum StatementType { eSelect, eCreateTable };
            /** called to do the parsing of a to-be-executed SQL statement, and set all members as needed

                @throws css::sdbc::SQLException
                @throws css::uno::RuntimeException
            */
            virtual StatementType
                            parseSql( const OUString& sql , bool bAdjusted = false);
            /** called to initialize a result set, according to a previously parsed SQL statement
            */
            virtual void    initializeResultSet( OResultSet* _pResult );
            /** called when a possible cached instance of our last result set should be cleared
            */
            virtual void    clearCachedResultSet();
            /** caches a result set which has just been created by an execution of an SQL statement
            */
            virtual void    cacheResultSet( const ::rtl::Reference< OResultSet >& _pResult );


            /** executes the current query (the one which has been passed to the last parseSql call)
            */
            css::uno::Reference< css::sdbc::XResultSet >
                            impl_executeCurrentQuery();

            void         createColumnMapping();
            void         analyseSQL();
            void         setOrderbyColumn( connectivity::OSQLParseNode const * pColumnRef,
                                           connectivity::OSQLParseNode const * pAscendingDescending);

        public:
            // other methods
            OConnection* getOwnConnection() const { return m_pConnection.get(); }

            explicit OCommonStatement(OConnection* _pConnection );
            using OCommonStatement_IBASE::operator css::uno::Reference< css::uno::XInterface >;

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
            // XCloseable
            virtual void SAL_CALL close(  ) override;

        protected:
            using OPropertySetHelper::getFastPropertyValue;
        };

        class OStatement :  public OCommonStatement,
                            public css::lang::XServiceInfo
        {
        protected:
            virtual ~OStatement() override {}
        public:
            // a constructor, for when the object needs to be returned:
            explicit OStatement( OConnection* _pConnection);
            DECLARE_SERVICE_INFO();

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
