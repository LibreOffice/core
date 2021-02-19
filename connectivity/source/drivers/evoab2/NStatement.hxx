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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NSTATEMENT_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NSTATEMENT_HXX

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/CommonTools.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include <connectivity/FValue.hxx>
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase.hxx>
#include <comphelper/propertycontainer.hxx>

#include "EApi.h"
#include "NConnection.hxx"

#include <vector>

namespace connectivity::evoab
{
        typedef ::cppu::WeakComponentImplHelper<   css::sdbc::XWarningsSupplier
                                               ,   css::sdbc::XCloseable
                                               >   OCommonStatement_IBase;

        struct FieldSort
        {
            sal_Int32       nField;
            bool            bAscending;

            FieldSort( const sal_Int32 _nField, const bool _bAscending ) : nField( _nField ), bAscending( _bAscending ) { }
        };
        typedef std::vector< FieldSort >  SortDescriptor;

        enum QueryFilterType
        {
            eFilterAlwaysFalse,
            eFilterNone,
            eFilterOther
        };

        class EBookQueryWrapper
        {
        private:
            EBookQuery* mpQuery;
        public:
            EBookQueryWrapper()
                : mpQuery(nullptr)
            {
            }
            EBookQueryWrapper(const EBookQueryWrapper& rhs)
                : mpQuery(rhs.mpQuery)
            {
                if (mpQuery)
                    e_book_query_ref(mpQuery);
            }
            EBookQueryWrapper(EBookQueryWrapper&& rhs) noexcept
                : mpQuery(rhs.mpQuery)
            {
                rhs.mpQuery = nullptr;
            }
            void reset(EBookQuery* pQuery)
            {
                if (mpQuery)
                    e_book_query_unref(mpQuery);
                mpQuery = pQuery;
                if (mpQuery)
                    e_book_query_ref(mpQuery);
            }
            EBookQueryWrapper& operator=(const EBookQueryWrapper& rhs)
            {
                if (this != &rhs)
                    reset(rhs.mpQuery);
                return *this;
            }
            EBookQueryWrapper& operator=(EBookQueryWrapper&& rhs)
            {
                if (mpQuery)
                    e_book_query_unref(mpQuery);
                mpQuery = rhs.mpQuery;
                rhs.mpQuery = nullptr;
                return *this;
            }
            ~EBookQueryWrapper()
            {
                if (mpQuery)
                    e_book_query_unref(mpQuery);
            }
            EBookQuery* getQuery() const
            {
                return mpQuery;
            }
        };

        struct QueryData
        {
        private:
            EBookQueryWrapper aQuery;

        public:
            OUString sTable;
            QueryFilterType eFilterType;
            rtl::Reference<connectivity::OSQLColumns>  xSelectColumns;
            SortDescriptor aSortOrder;

            QueryData()
                : sTable()
                , eFilterType( eFilterOther )
                , xSelectColumns()
                , aSortOrder()
            {
            }

            EBookQuery* getQuery() const { return aQuery.getQuery(); }
            void setQuery(EBookQuery* pQuery) { aQuery.reset(pQuery); }
        };

        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class OCommonStatement  :public cppu::BaseMutex
                                ,public OCommonStatement_IBase
                                ,public ::comphelper::OPropertyContainer
                                ,public ::comphelper::OPropertyArrayUsageHelper< OCommonStatement >
        {
        private:
            css::uno::WeakReference< css::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
            rtl::Reference<OEvoabConnection>      m_xConnection;
            connectivity::OSQLParser              m_aParser;
            connectivity::OSQLParseTreeIterator   m_aSQLIterator;
            connectivity::OSQLParseNode          *m_pParseTree;

            // <properties>
            OUString                             m_aCursorName;
            sal_Int32                                   m_nMaxFieldSize;
            sal_Int32                                   m_nMaxRows;
            sal_Int32                                   m_nQueryTimeOut;
            sal_Int32                                   m_nFetchSize;
            sal_Int32                                   m_nResultSetType;
            sal_Int32                                   m_nFetchDirection;
            sal_Int32                                   m_nResultSetConcurrency;
            bool                                    m_bEscapeProcessing;
            // </properties>

        protected:

            void disposeResultSet();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const override;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

            virtual ~OCommonStatement() override;

        protected:
            void         parseSql( const OUString& sql, QueryData& _out_rQueryData );
            EBookQuery  *whereAnalysis( const OSQLParseNode*  parseTree );
            void         orderByAnalysis( const OSQLParseNode* _pOrderByClause, SortDescriptor& _out_rSort );
            OUString getTableName() const;

        public:

            // other methods
            OEvoabConnection* getOwnConnection() const { return m_xConnection.get(); }

            using OCommonStatement_IBase::operator css::uno::Reference< css::uno::XInterface >;

        protected:
            explicit OCommonStatement( OEvoabConnection* _pConnection );

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

            // XWarningsSupplier
            virtual css::uno::Any SAL_CALL getWarnings(  ) override;
            virtual void SAL_CALL clearWarnings(  ) override;

            // XCloseable
            virtual void SAL_CALL close(  ) override;

        protected:
            /** will return the EBookQuery representing the statement WHERE condition, or throw

                Also, all statement dependent members (such as the parser/iterator) will be inited afterwards.
            */
            QueryData
                impl_getEBookQuery_throw( const OUString& _rSql );

            css::uno::Reference< css::sdbc::XResultSet >
                impl_executeQuery_throw( const OUString& _rSql );

            css::uno::Reference< css::sdbc::XResultSet >
                impl_executeQuery_throw( const QueryData& _rData );

            css::uno::Reference< css::sdbc::XConnection >
                impl_getConnection() { return css::uno::Reference< css::sdbc::XConnection >( m_xConnection ); }

            OUString
                impl_getColumnRefColumnName_throw( const ::connectivity::OSQLParseNode& _rColumnRef );
        };

        typedef ::cppu::ImplHelper2 <   css::lang::XServiceInfo
                                    ,   css::sdbc::XStatement
                                    >   OStatement_IBase;
        class OStatement    :public OCommonStatement
                            ,public OStatement_IBase
        {
        protected:
            virtual ~OStatement() override {}

        public:
            explicit OStatement( OEvoabConnection* _pConnection)
                :OCommonStatement( _pConnection)
            {
            }

            // XInterface
            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            virtual void SAL_CALL acquire() throw() override;
            virtual void SAL_CALL release() throw() override;

            // XTypeProvider
            DECLARE_XTYPEPROVIDER()

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XStatement
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) override ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) override ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) override ;
            virtual css::uno::Reference< css::sdbc::XConnection > SAL_CALL getConnection(  ) override ;
        };
}


#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_EVOAB2_NSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
