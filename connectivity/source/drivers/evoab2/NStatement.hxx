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

#ifndef _CONNECTIVITY_EVOAB_STATEMENT_HXX_
#define _CONNECTIVITY_EVOAB_STATEMENT_HXX_

#include <com/sun/star/sdbc/XStatement.hpp>
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdbc/XMultipleResults.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <comphelper/proparrhlp.hxx>
#include <cppuhelper/compbase2.hxx>
#include <comphelper/uno3.hxx>
#include "connectivity/CommonTools.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/broadcasthelper.hxx>
#include "connectivity/sqliterator.hxx"
#include "connectivity/sqlparse.hxx"
#include <connectivity/FValue.hxx>
#include "connectivity/OSubComponent.hxx"
#include <com/sun/star/util/XCancellable.hpp>
#include <cppuhelper/compbase5.hxx>
#include <comphelper/propertycontainer.hxx>

#include "EApi.h"

#include <list>

namespace connectivity
{
    namespace evoab
    {
        class OEvoabConnection;
        typedef ::cppu::WeakComponentImplHelper2    <   ::com::sun::star::sdbc::XWarningsSupplier
                                                    ,   ::com::sun::star::sdbc::XCloseable
                                                    >   OCommonStatement_IBase;

        struct FieldSort
        {
            sal_Int32       nField;
            bool            bAscending;

            FieldSort() : nField(0), bAscending( true ) { }
            FieldSort( const sal_Int32 _nField, const bool _bAscending ) : nField( _nField ), bAscending( _bAscending ) { }
        };
        typedef ::std::vector< FieldSort >  SortDescriptor;

        enum QueryFilterType
        {
            eFilterAlwaysFalse,
            eFilterNone,
            eFilterOther
        };

        struct QueryData
        {
        private:
            EBookQuery*     pQuery;

        public:
            OUString                             sTable;
            QueryFilterType                             eFilterType;
            ::rtl::Reference< ::connectivity::OSQLColumns >  xSelectColumns;
            SortDescriptor                              aSortOrder;

            QueryData()
                :pQuery( NULL )
                ,sTable()
                ,eFilterType( eFilterOther )
                ,xSelectColumns()
                ,aSortOrder()
            {
            }

            QueryData( const QueryData& _rhs )
                :pQuery( NULL )
                ,sTable()
                ,eFilterType( eFilterOther )
                ,xSelectColumns()
                ,aSortOrder()
            {
                *this = _rhs;
            }

            QueryData& operator=( const QueryData& _rhs )
            {
                if ( this == &_rhs )
                    return *this;

                setQuery( _rhs.pQuery );
                sTable = _rhs.sTable;
                eFilterType = _rhs.eFilterType;
                xSelectColumns = _rhs.xSelectColumns;
                aSortOrder = _rhs.aSortOrder;

                return *this;
            }

            ~QueryData()
            {
                setQuery( NULL );
            }

            EBookQuery* getQuery() const { return pQuery; }

            void setQuery( EBookQuery* _pQuery )
            {
                if ( pQuery )
                    e_book_query_unref( pQuery );
                pQuery = _pQuery;
                if ( pQuery )
                    e_book_query_ref( pQuery );
            }
        };


        //************ Class: OCommonStatement
        // is a base class for the normal statement and for the prepared statement

        class OCommonStatement;
        typedef OSubComponent< OCommonStatement, OCommonStatement_IBase >   OStatement_CBase;

        class OCommonStatement  :public comphelper::OBaseMutex
                                ,public OCommonStatement_IBase
                                ,public ::comphelper::OPropertyContainer
                                ,public ::comphelper::OPropertyArrayUsageHelper< OCommonStatement >
                                ,public OStatement_CBase
        {
            friend class OSubComponent< OCommonStatement, OCommonStatement_IBase >;

        private:
            ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XResultSet>    m_xResultSet;   // The last ResultSet created
            OEvoabConnection                     *m_pConnection;
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
            sal_Bool                                    m_bEscapeProcessing;
            // </properties>

        protected:

            void disposeResultSet();

            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

            virtual ~OCommonStatement();

        protected:
            void         parseSql( const OUString& sql, QueryData& _out_rQueryData );
            EBookQuery  *whereAnalysis( const OSQLParseNode*  parseTree );
            void         orderByAnalysis( const OSQLParseNode* _pOrderByClause, SortDescriptor& _out_rSort );
            OUString getTableName();
            EBookQuery  *createTrue();
            EBookQuery  *createTest( const OUString &aColumnName,
                                     EBookQueryTest eTest,
                                     const OUString &aMatch );

        public:

            // other methods
            OEvoabConnection* getOwnConnection() const { return m_pConnection;}

            using OCommonStatement_IBase::operator ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >;

        protected:
            OCommonStatement( OEvoabConnection* _pConnection );

            // OComponentHelper
            virtual void SAL_CALL disposing(void);
            // XInterface
            virtual void SAL_CALL release() throw();
            virtual void SAL_CALL acquire() throw();
            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
            //XTypeProvider
            virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

            // XPropertySet
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception);

            // XWarningsSupplier
            virtual ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);
            virtual void SAL_CALL clearWarnings(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);

            // XCloseable
            virtual void SAL_CALL close(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception);

        protected:
            /** will return the EBookQuery representing the stamement's WHERE condition, or throw

                Also, all statement dependent members (such as the parser/iterator) will be inited afterwards.
            */
            QueryData
                impl_getEBookQuery_throw( const OUString& _rSql );

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
                impl_executeQuery_throw( const OUString& _rSql );

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >
                impl_executeQuery_throw( const QueryData& _rData );

            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                impl_getConnection() { return ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >( (::com::sun::star::sdbc::XConnection*)m_pConnection ); }

            OUString
                impl_getColumnRefColumnName_throw( const ::connectivity::OSQLParseNode& _rColumnRef );
        };

        typedef ::cppu::ImplHelper2 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::sdbc::XStatement
                                    >   OStatement_IBase;
        class OStatement    :public OCommonStatement
                            ,public OStatement_IBase
        {
        protected:
            virtual ~OStatement(){}

        public:
            OStatement( OEvoabConnection* _pConnection)
                :OCommonStatement( _pConnection)
            {
            }

            // XInterface
            virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XTypeProvider
            DECLARE_XTYPEPROVIDER()

            // XServiceInfo
            DECLARE_SERVICE_INFO();

            // XStatement
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > SAL_CALL executeQuery( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) ;
            virtual sal_Int32 SAL_CALL executeUpdate( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) ;
            virtual sal_Bool SAL_CALL execute( const OUString& sql ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) ;
            virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) ;
        };
    }
}
#endif // CONNECTIVITY_SSTATEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
