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
#ifndef INCLUDED_DBACCESS_SOURCE_CORE_INC_SINGLESELECTQUERYCOMPOSER_HXX
#define INCLUDED_DBACCESS_SOURCE_CORE_INC_SINGLESELECTQUERYCOMPOSER_HXX

#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <cppuhelper/implbase.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include "apitools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <svx/ParseContext.hxx>

namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormatsSupplier;
    class XNumberFormatter;
}}}}

namespace dbaccess
{
    typedef ::cppu::ImplHelper <    css::sdb::XSingleSelectQueryComposer,
                                    css::sdb::XParametersSupplier,
                                    css::sdbcx::XColumnsSupplier,
                                    css::sdbcx::XTablesSupplier,
                                    css::lang::XServiceInfo    > OSingleSelectQueryComposer_BASE;

    class OPrivateColumns;
    class OPrivateTables;

    class OSingleSelectQueryComposer :   public ::comphelper::OMutexAndBroadcastHelper
                                        ,public OSubComponent
                                        ,public ::comphelper::OPropertyContainer
                                        ,public ::comphelper::OPropertyArrayUsageHelper < OSingleSelectQueryComposer >
                                        ,public OSingleSelectQueryComposer_BASE
    {
        enum SQLPart
        {
            Where = 0,      // the 0 is important, as it will be used as index into arrays
            Group,
            Having,
            Order,

            SQLPartCount
        };
        static inline void incSQLPart( SQLPart& e ) { e = (SQLPart)(1 + (size_t)e); }
        enum EColumnType
        {
            SelectColumns       = 0,
            GroupByColumns      = 1,
            OrderColumns        = 2,
            ParameterColumns    = 3
        };
        typedef ::std::const_mem_fun_t< const ::connectivity::OSQLParseNode*, ::connectivity::OSQLParseTreeIterator >
                                                TGetParseNode;
        ::svxform::OSystemParseContext          m_aParseContext;
        ::connectivity::OSQLParser              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator   m_aSqlIterator;         // the iterator for the complete statement
        ::connectivity::OSQLParseTreeIterator   m_aAdditiveIterator;    // the iterator for the "additive statement" (means without the clauses of the elementary statement)
        ::std::vector<OPrivateColumns*>         m_aColumnsCollection;   // used for columns and parameters of old queries
        ::std::vector<OPrivateTables*>          m_aTablesCollection;

        ::std::vector< OUString >        m_aElementaryParts;     // the filter/groupby/having/order of the elementary statement

        css::uno::Reference< css::sdbc::XConnection>              m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData>        m_xMetaData;
        css::uno::Reference< css::container::XNameAccess>         m_xConnectionTables;
        css::uno::Reference< css::container::XNameAccess>         m_xConnectionQueries;
        css::uno::Reference< css::util::XNumberFormatsSupplier >  m_xNumberFormatsSupplier;
        css::uno::Reference< css::uno::XComponentContext>         m_aContext;
        css::uno::Reference< css::script::XTypeConverter >        m_xTypeConverter;

        ::std::vector<OPrivateColumns*>         m_aCurrentColumns;
        OPrivateTables*                         m_pTables;      // currently used tables

        OUString                                m_aPureSelectSQL;   // the pure select statement, without filter/order/groupby/having
        OUString                                m_sDecimalSep;
        OUString                                m_sCommand;
        css::lang::Locale                       m_aLocale;
        sal_Int32                               m_nBoolCompareMode; // how to compare bool values
        sal_Int32                               m_nCommandType;

        // <properties>
        OUString                         m_sOrignal;
        // </properties>


        bool setORCriteria(::connectivity::OSQLParseNode* pCondition, ::connectivity::OSQLParseTreeIterator& _rIterator,
            ::std::vector< ::std::vector < css::beans::PropertyValue > >& rFilters, const css::uno::Reference< css::util::XNumberFormatter > & xFormatter) const;
        bool setANDCriteria(::connectivity::OSQLParseNode* pCondition, ::connectivity::OSQLParseTreeIterator& _rIterator,
            ::std::vector < css::beans::PropertyValue > & rFilters, const css::uno::Reference< css::util::XNumberFormatter > & xFormatter) const;
        bool setComparsionPredicate(::connectivity::OSQLParseNode* pCondition, ::connectivity::OSQLParseTreeIterator& _rIterator,
            ::std::vector < css::beans::PropertyValue > & rFilters, const css::uno::Reference< css::util::XNumberFormatter > & xFormatter) const;

        static OUString getColumnName(::connectivity::OSQLParseNode* pColumnRef,::connectivity::OSQLParseTreeIterator& _rIterator);
        OUString getTableAlias(const css::uno::Reference< css::beans::XPropertySet >& column ) const;
        static sal_Int32 getPredicateType(::connectivity::OSQLParseNode * _pPredicate);
        // clears all Columns,Parameters and tables and insert it to their vectors
        void clearCurrentCollections();
        // clears the columns collection given by EColumnType
        void clearColumns( const EColumnType _eType );

        /** retrieves a particular part of a statement
            @param _rIterator
                the iterator to use.
        */
        OUString getStatementPart( TGetParseNode& _aGetFunctor, ::connectivity::OSQLParseTreeIterator& _rIterator );
        void setQuery_Impl( const OUString& command );

        void setConditionByColumn( const css::uno::Reference< css::beans::XPropertySet >& column
                                , bool andCriteria
                                ,::std::mem_fun1_t<bool,OSingleSelectQueryComposer,const OUString& >& _aSetFunctor
                                ,sal_Int32 filterOperator);

        /** getStructuredCondition returns the structured condition for the where or having clause
            @param  _aGetFunctor
                A member function to get the correct parse node.

            @return
                The structured filter
        */
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >
                    getStructuredCondition( TGetParseNode& _aGetFunctor );

        css::uno::Reference< css::container::XIndexAccess >
                    setCurrentColumns( EColumnType _eType, const ::rtl::Reference< ::connectivity::OSQLColumns >& _rCols );

        //helper methods for mem_fun_t
        inline bool implSetFilter(const OUString& _sFilter) { setFilter(_sFilter); return true;}
        inline bool implSetHavingClause(const OUString& _sFilter) { setHavingClause(_sFilter); return true;}

        /** returns the part of the select statement
            @param  _ePart
                Which part should be returned.
            @param  _bWithKeyword
                If <TRUE/> the keyword will be added too. Otherwise not.
            @param _rIterator
                The iterator to use.

            @return
                The part of the select statement.
        */
        OUString getSQLPart( SQLPart _ePart, ::connectivity::OSQLParseTreeIterator& _rIterator, bool _bWithKeyword );

        /** retrieves the keyword for the given SQLPart
        */
        static OUString getKeyword( SQLPart _ePart );

        /** sets a single "additive" clause, means a filter/groupby/having/order clause
        */
        void setSingleAdditiveClause( SQLPart _ePart, const OUString& _rClause );

        /** composes a statement from m_aPureSelectSQL and the 4 usual clauses
        */
        OUString composeStatementFromParts( const ::std::vector< OUString >& _rParts );

        /** return the name of the column in the *source* *table*.

            That is, for (SELECT a AS b FROM t), it returns A or "t"."A", as appropriate.

            Use e.g. for WHERE, GROUP BY and HAVING clauses.

            @param bGroupBy: for GROUP BY clause? In that case, throw exception if trying to use an unrelated column and the database does not support that.
        */
        OUString impl_getColumnRealName_throw(const css::uno::Reference< css::beans::XPropertySet >& column, bool bGroupBy);

        /** return the name of the column in the *query* for ORDER BY clause.

            That is, for (SELECT a AS b FROM t), it returns "b"

            Throws exception if trying to use an unrelated column and the database does not support that.
        */
        OUString impl_getColumnNameOrderBy_throw(const css::uno::Reference< css::beans::XPropertySet >& column);

    protected:
        virtual ~OSingleSelectQueryComposer() override;
    public:

        OSingleSelectQueryComposer( const css::uno::Reference< css::container::XNameAccess>& _xTableSupplier,
                        const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                        const css::uno::Reference< css::uno::XComponentContext>& _rContext);


        void SAL_CALL disposing() override;

        virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId()
            throw (css::uno::RuntimeException, std::exception) override;

        // css::uno::XInterface
        DECLARE_XINTERFACE( )

        // XServiceInfo
        DECLARE_SERVICE_INFO();

        DECLARE_PROPERTYCONTAINER_DEFAULTS();

        // css::sdb::XSingleSelectQueryComposer
        virtual OUString SAL_CALL getElementaryQuery() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setElementaryQuery( const OUString& _rElementary ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setFilter( const OUString& filter ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setStructuredFilter( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& filter ) throw (css::sdbc::SQLException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendFilterByColumn( const css::uno::Reference< css::beans::XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw (css::sdbc::SQLException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendGroupByColumn( const css::uno::Reference< css::beans::XPropertySet >& column ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setGroup( const OUString& group ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setHavingClause( const OUString& filter ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setStructuredHavingClause( const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& filter ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendHavingClauseByColumn( const css::uno::Reference< css::beans::XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw (css::sdbc::SQLException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL appendOrderByColumn( const css::uno::Reference< css::beans::XPropertySet >& column, sal_Bool ascending ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setOrder( const OUString& order ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

        // XSingleSelectQueryAnalyzer
        virtual OUString SAL_CALL getQuery(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setQuery( const OUString& command ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL setCommand( const OUString& command,sal_Int32 CommandType ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getFilter(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getStructuredFilter(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getGroup(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getGroupColumns(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getHavingClause(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getStructuredHavingClause(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getOrder(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getOrderColumns(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getQueryWithSubstitution(  ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

        // XColumnsSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getColumns(  ) throw(css::uno::RuntimeException, std::exception) override;
        // XTablesSupplier
        virtual css::uno::Reference< css::container::XNameAccess > SAL_CALL getTables(  ) throw(css::uno::RuntimeException, std::exception) override;
        // XParametersSupplier
        virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getParameters(  ) throw(css::uno::RuntimeException, std::exception) override;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_CORE_INC_SINGLESELECTQUERYCOMPOSER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
