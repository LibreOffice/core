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
#ifndef INCLUDED_CONNECTIVITY_SQLITERATOR_HXX
#define INCLUDED_CONNECTIVITY_SQLITERATOR_HXX

#include <connectivity/dbtoolsdllapi.hxx>
#include <connectivity/sqlnode.hxx>
#include <connectivity/IParseContext.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <connectivity/CommonTools.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/weak.hxx>

#include <map>
#include <memory>
#include <vector>
#include <o3tl/typed_flags_set.hxx>

namespace connectivity
{
    enum class TraversalParts
    {
        Parameters      = 0x0001,
        TableNames      = 0x0002,
        SelectColumns   = 0x0006,   // note that this includes TableNames. No SelectColumns without TableNames

        // Those are not implemented currently
        // GroupColumns    = 0x0008,
        // OrderColumns    = 0x0010,
        // SelectColumns   = 0x0020,
        // CreateColumns   = 0x0040,

        All             = 0xFFFF
    };
}
namespace o3tl
{
    template<> struct typed_flags<connectivity::TraversalParts> : is_typed_flags<connectivity::TraversalParts, 0xffff> {};
}

namespace connectivity
{

    class OSQLParseNode;
    class OSQLParser;

    typedef ::std::pair<const OSQLParseNode*,const OSQLParseNode* > TNodePair;

    enum class OSQLStatementType {
        Unknown,
        Select,
        Insert,
        Update,
        Delete,
        OdbcCall,
        CreateTable
    };

    struct OSQLParseTreeIteratorImpl;

    class OOO_DLLPUBLIC_DBTOOLS OSQLParseTreeIterator
    {
    private:
        css::sdbc::SQLException                             m_aErrors;          // conatins the error while iterating through the statement
        const OSQLParseNode*                                m_pParseTree;       // current ParseTree
        const OSQLParser&                                   m_rParser;          // if set used for general error messages from the context
        OSQLStatementType                                   m_eStatementType;
        ::rtl::Reference<OSQLColumns>                       m_aSelectColumns;   // all columns from the Select clause
        ::rtl::Reference<OSQLColumns>                       m_aParameters;      // all parameters
        ::rtl::Reference<OSQLColumns>                       m_aGroupColumns;    // the group by columns
        ::rtl::Reference<OSQLColumns>                       m_aOrderColumns;    // the order by columns
        ::rtl::Reference<OSQLColumns>                       m_aCreateColumns;   // the columns for Create table clause

        ::std::unique_ptr< OSQLParseTreeIteratorImpl >  m_pImpl;

        void                traverseParameter(const OSQLParseNode* _pParseNode,const OSQLParseNode* _pColumnRef,const OUString& _aColumnName, OUString& _aTableRange, const OUString& _rColumnAlias);
        // inserts a table into the map
        void                traverseOneTableName( OSQLTables& _rTables,const OSQLParseNode * pTableName, const OUString & rTableRange );
        void                traverseSearchCondition(OSQLParseNode * pSearchCondition);
        void                traverseOnePredicate(
                                                OSQLParseNode * pColumnRef,
                                                OUString& aValue,
                                                OSQLParseNode * pParameter);
        void traverseByColumnNames(const OSQLParseNode* pSelectNode, bool _bOrder);
        void                traverseParameters(const OSQLParseNode* pSelectNode);

        const OSQLParseNode*    getTableNode( OSQLTables& _rTables, const OSQLParseNode* pTableRef, OUString& aTableRange );
        void                    getQualified_join( OSQLTables& _rTables, const OSQLParseNode *pTableRef, OUString& aTableRange );
        void                    getSelect_statement(OSQLTables& _rTables,const OSQLParseNode* pSelect);
        OUString         getUniqueColumnName(const OUString & rColumnName)    const;

        /** finds the column with a given name, belonging to a given table, in a given tables collection
            @param  _rTables
                the tables collection to look in
            @param  rColumnName
                the column name to look for
            @param  rTableRange
                the table alias name; if empty, look in all tables
            @return
                the desired column object, or <NULL/> if no such column could be found
        */
        static css::uno::Reference< css::beans::XPropertySet > findColumn(
            const OSQLTables& _rTables, const OUString & rColumnName, OUString & rTableRange );

        /** finds a column with a given name, belonging to a given table
            @param  rColumnName
                the column name to look for
            @param  rTableRange
                    the table alias name; if empty, look in all tables
            @param  _bLookInSubTables
                <TRUE/> if and only if not only our direct tables, but also our sub tables (from sub selects)
                should be searched
            @return
        */
        css::uno::Reference< css::beans::XPropertySet > findColumn(
            const OUString & rColumnName, OUString & rTableRange, bool _bLookInSubTables );

        /** finds a column with a given name among the select columns
            @param  rColumnName
                the column name to look for
            @return
        */
        css::uno::Reference< css::beans::XPropertySet > findSelectColumn(
            const OUString & rColumnName );

      protected:
        void setSelectColumnName(::rtl::Reference<OSQLColumns>& _rColumns,const OUString & rColumnName,const OUString & rColumnAlias, const OUString & rTableRange, bool bFkt=false, sal_Int32 _nType = css::sdbc::DataType::VARCHAR, bool bAggFkt=false);
        void appendColumns(::rtl::Reference<OSQLColumns>& _rColumns,const OUString& _rTableAlias,const OSQLTable& _rTable);
        // Other member variables that should be available in the "set" functions
        // can be defined in the derived class. They can be initialized
        // in its constructor and, after the "traverse" routines have been used,
        // they can be queried using other functions.


      private:
        OSQLParseTreeIterator(const OSQLParseTreeIterator & rIter) = delete;

      public:
        OSQLParseTreeIterator(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::container::XNameAccess >& _rxTables,
            const OSQLParser& _rParser,
            const OSQLParseNode* pRoot = nullptr );
        ~OSQLParseTreeIterator();

        inline static void * SAL_CALL operator new( size_t nSize )
            { return ::rtl_allocateMemory( nSize ); }
        inline static void * SAL_CALL operator new( size_t,void* _pHint )
            { return _pHint; }
        inline static void SAL_CALL operator delete( void * pMem )
            { ::rtl_freeMemory( pMem ); }
        inline static void SAL_CALL operator delete( void *,void* )
            {  }

        void dispose();
        bool isCaseSensitive() const;
        // The parse tree to be analysed/traversed:
        // If NULL is passed, the current parse tree will be deleted and the error status cleared.
        void setParseTree(const OSQLParseNode * pNewParseTree);
//      void setParser(const OSQLParser* _pParser) { m_pParser = _pParser; }
        const OSQLParseNode * getParseTree() const { return m_pParseTree; };

        // subtrees in case of a select statement
        const OSQLParseNode* getWhereTree() const;
        const OSQLParseNode* getOrderTree() const;
        const OSQLParseNode* getGroupByTree() const;
        const OSQLParseNode* getHavingTree() const;

        const OSQLParseNode* getSimpleWhereTree() const;
        const OSQLParseNode* getSimpleOrderTree() const;
        const OSQLParseNode* getSimpleGroupByTree() const;
        const OSQLParseNode* getSimpleHavingTree() const;

        /** returns the errors which occurred during parsing.

            The returned object contains a chain (via SQLException::NextException) of SQLExceptions.
        */
        inline const css::sdbc::SQLException&   getErrors() const { return m_aErrors; }
        inline bool hasErrors() const { return !m_aErrors.Message.isEmpty(); }

        // statement type (already set in setParseTree):
        OSQLStatementType getStatementType() const { return m_eStatementType; }

        /** traverses the complete statement tree, and fills all our data with
            the information obatined during traversal.

            Implemented by calling the single traverse* methods in the proper
            order (depending on the statement type).
        */
        void traverseAll();

        /** traverses selected parts of the statement tree, and fills our data with
            the information obtained during traversal

            @param _nIncludeMask
                set of TraversalParts bits, specifying which information is to be collected.
                Note TraversalParts is currently not
        */
        void traverseSome( TraversalParts _nIncludeMask );

        // The TableRangeMap contains all tables associated with the range name found first.
        const OSQLTables& getTables() const;

        const ::rtl::Reference<OSQLColumns>& getSelectColumns() const { return m_aSelectColumns;}
        const ::rtl::Reference<OSQLColumns>& getGroupColumns() const { return m_aGroupColumns;}
        const ::rtl::Reference<OSQLColumns>& getOrderColumns() const { return m_aOrderColumns;}
        const ::rtl::Reference<OSQLColumns>& getParameters()   const { return m_aParameters; }

        /** return the columname and the table range
            @param  _pColumnRef
                The column ref parse node.
            @param  _rColumnName
                The column name to be set.
            @param  _rTableRange
                The table range to be set.
        */
        void getColumnRange(    const OSQLParseNode* _pColumnRef,
                                OUString &_rColumnName,
                                OUString& _rTableRange) const;

        /** retrieves a column's name, table range, and alias

            @param  _pColumnRef
                The column_ref parse node.
            @param  _out_rColumnName
                The column name to be set.
            @param  _out_rTableRange
                The table range to be set.
            @param _out_rColumnAliasIfPresent
                If the column specified by _pColumnRef is part of the select columns, and contains a column alias there,
                this alias is returned here.
        */
        void getColumnRange(    const OSQLParseNode* _pColumnRef,
                                OUString& _out_rColumnName,
                                OUString& _out_rTableRange,
                                OUString& _out_rColumnAliasIfPresent
                                ) const;

        /** return the alias name of a column
            @param  _pDerivedColumn
                The parse node where SQL_ISRULE(_pDerivedColumn,derived_column) must be true
            @return
                The alias name of the column or an empty string.
        */
        static OUString getColumnAlias(const OSQLParseNode* _pDerivedColumn);

        /** return the columname and the table range
            @param  _pColumnRef
                The column ref parse node.
            @param  _xMetaData
                The database meta data.
            @param  _rColumnName
                The column name to be set.
            @param  _rTableRange
                The table range to be set.
        */
        static void getColumnRange( const OSQLParseNode* _pColumnRef,
                                    const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
                                    OUString &_rColumnName,
                                    OUString& _rTableRange);

        // empty if ambiguous
        bool getColumnTableRange(const OSQLParseNode* pNode, OUString &rTableRange) const;

        // return true when the tableNode is a rule like catalog_name, schema_name or table_name
        static bool isTableNode(const OSQLParseNode* _pTableNode);

        // tries to find the correct type of the function
        sal_Int32 getFunctionReturnType(const OSQLParseNode* _pNode );

        // returns a lis of all joined columns
        ::std::vector< TNodePair >& getJoinConditions() const;

    private:
        // helper to implement getColumnTableRange
        bool impl_getColumnTableRange(const OSQLParseNode* pNode, OUString &rTableRange) const;

        /** traverses the list of table names, and filles _rTables
        */
        bool traverseTableNames( OSQLTables& _rTables );

        /// traverses columns in a SELECT statement
        bool traverseSelectColumnNames(const OSQLParseNode* pSelectNode);
        /// traverses columns in a CREATE TABLE statement
        void traverseCreateColumns(const OSQLParseNode* pSelectNode);

        bool traverseOrderByColumnNames(const OSQLParseNode* pSelectNode);
        bool traverseGroupByColumnNames(const OSQLParseNode* pSelectNode);

        bool traverseSelectionCriteria(const OSQLParseNode* pSelectNode);

    private:
        /** constructs a new iterator, which inherits some of the settings from a parent iterator
        */
        OSQLParseTreeIterator(
            const OSQLParseTreeIterator& _rParentIterator,
            const OSQLParser& _rParser,
            const OSQLParseNode* pRoot );

        /** creates a table object and inserts it into our tables collection

            only used when we're iterating through a CREATE TABLE statement
        */
        OSQLTable   impl_createTableObject(
            const OUString& rTableName, const OUString& rCatalogName, const OUString& rSchemaName );

        /** locates a record source (a table or query) with the given name
        */
        OSQLTable   impl_locateRecordSource(
            const OUString& _rComposedName
        );

        /** implementation for both traverseAll and traverseSome
        */
        void    impl_traverse( TraversalParts _nIncludeMask );

        /** retrieves the parameter columns of the given query
        */
        void    impl_getQueryParameterColumns( const OSQLTable& _rQuery );

        void setOrderByColumnName(const OUString & rColumnName, OUString & rTableRange, bool bAscending);
        void setGroupByColumnName(const OUString & rColumnName, OUString & rTableRange);

    private:
        /** appends an SQLException corresponding to the given error code to our error collection

            @param  _eError
                the code of the error which occurred
            @param  _pReplaceToken1
                if not <NULL/>, the first occurrence of '#' in the error message will be replaced
                with the given token
            @param  _pReplaceToken2
                if not <NULL/>, and if _rReplaceToken1 is not <NULL/>, the second occurrence of '#'
                in the error message will be replaced with _rReplaceToken2
        */
        void impl_appendError( IParseContext::ErrorCode _eError,
            const OUString* _pReplaceToken1 = nullptr, const OUString* _pReplaceToken2 = nullptr );

        /** appends an SQLException corresponding to the given error code to our error collection
        */
        void impl_appendError( const css::sdbc::SQLException& _rError );

        /** resets our errors
        */
        inline void impl_resetErrors()
        {
            m_aErrors = css::sdbc::SQLException();
        }
        void impl_fillJoinConditions(const OSQLParseNode* i_pJoinCondition);
    };
}

#endif // INCLUDED_CONNECTIVITY_SQLITERATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
