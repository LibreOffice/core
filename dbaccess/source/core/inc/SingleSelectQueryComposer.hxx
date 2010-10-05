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
#ifndef DBACCESS_CORE_API_SINGLESELECTQUERYCOMPOSER_HXX
#define DBACCESS_CORE_API_SINGLESELECTQUERYCOMPOSER_HXX

#include <com/sun/star/sdb/XParametersSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <cppuhelper/implbase5.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlparse.hxx>
#include "apitools.hxx"
#include <comphelper/broadcasthelper.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/componentcontext.hxx>

#include <memory>

namespace com { namespace sun { namespace star { namespace util {
    class XNumberFormatsSupplier;
    class XNumberFormatter;
}}}}

namespace dbaccess
{
    typedef ::cppu::ImplHelper5<    ::com::sun::star::sdb::XSingleSelectQueryComposer,
                                    ::com::sun::star::sdb::XParametersSupplier,
                                    ::com::sun::star::sdbcx::XColumnsSupplier,
                                    ::com::sun::star::sdbcx::XTablesSupplier,
                                    ::com::sun::star::lang::XServiceInfo    > OSingleSelectQueryComposer_BASE;

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
        inline void incSQLPart( SQLPart& e ) { e = (SQLPart)(1 + (size_t)e); }
        enum EColumnType
        {
            SelectColumns       = 0,
            GroupByColumns      = 1,
            OrderColumns        = 2,
            ParameterColumns    = 3
        };
        typedef ::std::const_mem_fun_t< const ::connectivity::OSQLParseNode*, ::connectivity::OSQLParseTreeIterator >
                                                TGetParseNode;
        ::connectivity::OSQLParser              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator   m_aSqlIterator;         // the iterator for the complete statement
        ::connectivity::OSQLParseTreeIterator   m_aAdditiveIterator;    // the iterator for the "additive statement" (means without the clauses of the elementary statement)
        ::std::vector<OPrivateColumns*>         m_aColumnsCollection;   // used for columns and parameters of old queries
        ::std::vector<OPrivateTables*>          m_aTablesCollection;

        ::std::vector< ::rtl::OUString >        m_aElementaryParts;     // the filter/groupby/having/order of the elementary statement

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>              m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>        m_xMetaData;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>         m_xConnectionTables;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>         m_xConnectionQueries;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >  m_xNumberFormatsSupplier;
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>         m_xColumns;
        ::comphelper::ComponentContext                                                      m_aContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >        m_xTypeConverter;

        ::std::vector<OPrivateColumns*>         m_aCurrentColumns;
        OPrivateTables*                         m_pTables;      // currently used tables

        ::rtl::OUString                         m_aPureSelectSQL;   // the pure select statement, without filter/order/groupby/having
        ::rtl::OUString                         m_sDecimalSep;
        ::rtl::OUString                         m_sCommand;
        ::com::sun::star::lang::Locale          m_aLocale;
        sal_Int32                               m_nBoolCompareMode; // how to compare bool values
        sal_Int32                               m_nCommandType;

        // <properties>
        ::rtl::OUString                         m_sOrignal;
        // </properties>


        sal_Bool setORCriteria(::connectivity::OSQLParseNode* pCondition, ::connectivity::OSQLParseTreeIterator& _rIterator,
            ::std::vector< ::std::vector < ::com::sun::star::beans::PropertyValue > >& rFilters, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const;
        sal_Bool setANDCriteria(::connectivity::OSQLParseNode* pCondition, ::connectivity::OSQLParseTreeIterator& _rIterator,
            ::std::vector < ::com::sun::star::beans::PropertyValue > & rFilters, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const;
        sal_Bool setComparsionPredicate(::connectivity::OSQLParseNode* pCondition, ::connectivity::OSQLParseTreeIterator& _rIterator,
            ::std::vector < ::com::sun::star::beans::PropertyValue > & rFilters, const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const;

        ::rtl::OUString getColumnName(::connectivity::OSQLParseNode* pColumnRef,::connectivity::OSQLParseTreeIterator& _rIterator) const;
        ::rtl::OUString getTableAlias(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column ) const;
        sal_Int32 getPredicateType(::connectivity::OSQLParseNode * _pPredicate) const;
        // clears all Columns,Parameters and tables and insert it to their vectors
        void clearCurrentCollections();
        // clears the columns collection given by EColumnType
        void clearColumns( const EColumnType _eType );

        /** retrieves a particular part of a statement
            @param _rIterator
                the iterator to use.
        */
        ::rtl::OUString getStatementPart( TGetParseNode& _aGetFunctor, ::connectivity::OSQLParseTreeIterator& _rIterator );
        void setQuery_Impl( const ::rtl::OUString& command );

        void setConditionByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column
                                , sal_Bool andCriteria
                                ,::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString>& _aSetFunctor
                                ,sal_Int32 filterOperator);

        /** getStructuredCondition returns the structured condition for the where or having clause
            @param  _aGetFunctor
                A member function to get the correct parse node.

            @return
                The structured filter
        */
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >
                    getStructuredCondition( TGetParseNode& _aGetFunctor );

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >
                    setCurrentColumns( EColumnType _eType, const ::vos::ORef< ::connectivity::OSQLColumns >& _rCols );

        //helper methods for mem_fun_t
        inline bool implSetFilter(::rtl::OUString _sFilter) { setFilter(_sFilter); return true;}
        inline bool implSetHavingClause(::rtl::OUString _sFilter) { setHavingClause(_sFilter); return true;}

        /** returns the part of the seelect statement
            @param  _ePart
                Which part should be returned.
            @param  _bWithKeyword
                If <TRUE/> the keyword will be added too. Otherwise not.
            @param _rIterator
                The iterator to use.

            @return
                The part of the select statement.
        */
        ::rtl::OUString getSQLPart( SQLPart _ePart, ::connectivity::OSQLParseTreeIterator& _rIterator, sal_Bool _bWithKeyword );

        /** retrieves the keyword for the given SQLPart
        */
        ::rtl::OUString getKeyword( SQLPart _ePart ) const;

        /** sets a single "additive" clause, means a filter/groupby/having/order clause
        */
        void setSingleAdditiveClause( SQLPart _ePart, const ::rtl::OUString& _rClause );

        /** composes a statement from m_aPureSelectSQL and the 4 usual clauses
        */
        ::rtl::OUString composeStatementFromParts( const ::std::vector< ::rtl::OUString >& _rParts );

        /** return the name of the column.
        */
        ::rtl::OUString impl_getColumnName_throw(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column);

    protected:
        virtual ~OSingleSelectQueryComposer();
    public:

        OSingleSelectQueryComposer( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xTableSupplier,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                        const ::comphelper::ComponentContext& _rContext);


        void SAL_CALL disposing(void);
        // ::com::sun::star::lang::XTypeProvider
        DECLARE_TYPEPROVIDER( );

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        // ::com::sun::star::uno::XInterface
        DECLARE_XINTERFACE( )

        // XServiceInfo
        DECLARE_SERVICE_INFO();

        DECLARE_PROPERTYCONTAINER_DEFAULTS();

        // ::com::sun::star::sdb::XSingleSelectQueryComposer
        virtual ::rtl::OUString SAL_CALL getElementaryQuery() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setElementaryQuery( const ::rtl::OUString& _rElementary ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFilter( const ::rtl::OUString& filter ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setStructuredFilter( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& filter ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendFilterByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendGroupByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setGroup( const ::rtl::OUString& group ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setHavingClause( const ::rtl::OUString& filter ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setStructuredHavingClause( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& filter ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendHavingClauseByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL appendOrderByColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& column, sal_Bool ascending ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setOrder( const ::rtl::OUString& order ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // XSingleSelectQueryAnalyzer
        virtual ::rtl::OUString SAL_CALL getQuery(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setQuery( const ::rtl::OUString& command ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setCommand( const ::rtl::OUString& command,sal_Int32 CommandType ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFilter(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getStructuredFilter(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getGroup(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getGroupColumns(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getHavingClause(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > SAL_CALL getStructuredHavingClause(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getOrder(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getOrderColumns(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getQueryWithSubstitution(  ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

        // XColumnsSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException);
        // XTablesSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);
        // XParametersSupplier
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL getParameters(  ) throw(::com::sun::star::uno::RuntimeException);
    };
}
#endif // DBACCESS_CORE_API_SINGLESELECTQUERYCOMPOSER_HXX
