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

#include "connectivity/sqliterator.hxx"
#include "connectivity/sdbcx/VTable.hxx"
#include <connectivity/sqlparse.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/sqlerror.hxx>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/ErrorCondition.hpp>
#ifdef SQL_TEST_PARSETREEITERATOR
#include <iostream>
#endif
#include "connectivity/PColumn.hxx"
#include "connectivity/dbtools.hxx"
#include <tools/diagnose_ex.h>
#include "TConnection.hxx"
#include <comphelper/types.hxx>
#include <connectivity/dbmetadata.hxx>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include "diagnose_ex.h"
#include <rtl/logfile.hxx>


#include <iterator>

using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::connectivity::sdbcx;
using namespace ::dbtools;
using namespace ::connectivity::parse;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;

namespace connectivity
{
    struct OSQLParseTreeIteratorImpl
    {
        ::std::vector< TNodePair >      m_aJoinConditions;
        Reference< XConnection >        m_xConnection;
        Reference< XDatabaseMetaData >  m_xDatabaseMetaData;
        Reference< XNameAccess >        m_xTableContainer;
        Reference< XNameAccess >        m_xQueryContainer;

        ::boost::shared_ptr< OSQLTables >   m_pTables;      // all tables which participate in the SQL statement
        ::boost::shared_ptr< OSQLTables >   m_pSubTables;   // all tables from sub queries not the tables from the select tables
        ::boost::shared_ptr< QueryNameSet > m_pForbiddenQueryNames;

        sal_uInt32                      m_nIncludeMask;

        bool                            m_bIsCaseSensitive;

        OSQLParseTreeIteratorImpl( const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxTables )
            :m_xConnection( _rxConnection )
            ,m_nIncludeMask( OSQLParseTreeIterator::All )
            ,m_bIsCaseSensitive( true )
        {
            OSL_PRECOND( m_xConnection.is(), "OSQLParseTreeIteratorImpl::OSQLParseTreeIteratorImpl: invalid connection!" );
            m_xDatabaseMetaData = m_xConnection->getMetaData();

            m_bIsCaseSensitive = m_xDatabaseMetaData.is() && m_xDatabaseMetaData->supportsMixedCaseQuotedIdentifiers();
            m_pTables.reset( new OSQLTables( m_bIsCaseSensitive ) );
            m_pSubTables.reset( new OSQLTables( m_bIsCaseSensitive ) );

            m_xTableContainer = _rxTables;

            DatabaseMetaData aMetaData( m_xConnection );
            if ( aMetaData.supportsSubqueriesInFrom() )
            {
                // connections might support the XQueriesSupplier interface, if they implement the css.sdb.Connection
                // service
                Reference< XQueriesSupplier > xSuppQueries( m_xConnection, UNO_QUERY );
                if ( xSuppQueries.is() )
                    m_xQueryContainer = xSuppQueries->getQueries();
            }
        }

    public:
        inline  bool    isQueryAllowed( const ::rtl::OUString& _rQueryName )
        {
            if ( !m_pForbiddenQueryNames.get() )
                return true;
            if ( m_pForbiddenQueryNames->find( _rQueryName ) == m_pForbiddenQueryNames->end() )
                return true;
            return false;
        }
    };

    //-------------------------------------------------------------------------
    /** helper class for temporarily adding a query name to a list of forbidden query names
    */
    class ForbidQueryName
    {
        ::boost::shared_ptr< QueryNameSet >&    m_rpAllForbiddenNames;
        ::rtl::OUString                         m_sForbiddenQueryName;

    public:
        ForbidQueryName( OSQLParseTreeIteratorImpl& _rIteratorImpl, const ::rtl::OUString _rForbiddenQueryName )
            :m_rpAllForbiddenNames( _rIteratorImpl.m_pForbiddenQueryNames )
            ,m_sForbiddenQueryName( _rForbiddenQueryName )
        {
            if ( !m_rpAllForbiddenNames.get() )
                m_rpAllForbiddenNames.reset( new QueryNameSet );
            m_rpAllForbiddenNames->insert( m_sForbiddenQueryName );
        }

        ~ForbidQueryName()
        {
            m_rpAllForbiddenNames->erase( m_sForbiddenQueryName );
        }
    };
}
//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator(const Reference< XConnection >& _rxConnection,
                                             const Reference< XNameAccess >& _rxTables,
                                             const OSQLParser& _rParser,
                                             const OSQLParseNode* pRoot )
    :m_rParser( _rParser )
    ,m_pImpl( new OSQLParseTreeIteratorImpl( _rxConnection, _rxTables ) )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::OSQLParseTreeIterator" );
    setParseTree(pRoot);
}

//-----------------------------------------------------------------------------
OSQLParseTreeIterator::OSQLParseTreeIterator( const OSQLParseTreeIterator& _rParentIterator, const OSQLParser& _rParser, const OSQLParseNode* pRoot )
    :m_rParser( _rParser )
    ,m_pImpl( new OSQLParseTreeIteratorImpl( _rParentIterator.m_pImpl->m_xConnection, _rParentIterator.m_pImpl->m_xTableContainer ) )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::OSQLParseTreeIterator" );
    m_pImpl->m_pForbiddenQueryNames = _rParentIterator.m_pImpl->m_pForbiddenQueryNames;
    setParseTree( pRoot );
}

//-----------------------------------------------------------------------------
OSQLParseTreeIterator::~OSQLParseTreeIterator()
{
    dispose();
}

// -----------------------------------------------------------------------------
const OSQLTables& OSQLParseTreeIterator::getTables() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getTables" );
    return *m_pImpl->m_pTables;
}

// -----------------------------------------------------------------------------
bool OSQLParseTreeIterator::isCaseSensitive() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::isCaseSensitive" );
    return m_pImpl->m_bIsCaseSensitive;
}

// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::dispose()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::dispose" );
    m_aSelectColumns    = NULL;
    m_aGroupColumns     = NULL;
    m_aOrderColumns     = NULL;
    m_aParameters       = NULL;
    m_pImpl->m_xTableContainer  = NULL;
    m_pImpl->m_xDatabaseMetaData = NULL;
    m_aCreateColumns    = NULL;
    m_pImpl->m_pTables->clear();
    m_pImpl->m_pSubTables->clear();
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setParseTree(const OSQLParseNode * pNewParseTree)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::setParseTree" );
    m_pImpl->m_pTables->clear();
    m_pImpl->m_pSubTables->clear();

    m_aSelectColumns = new OSQLColumns();
    m_aGroupColumns = new OSQLColumns();
    m_aOrderColumns = new OSQLColumns();
    m_aParameters    = new OSQLColumns();
    m_aCreateColumns = new OSQLColumns();

    m_pParseTree = pNewParseTree;
    if (!m_pParseTree)
    {
        m_eStatementType = SQL_STATEMENT_UNKNOWN;
        return;
    }

    // If m_pParseTree, but no connection then return
    if ( !m_pImpl->m_xTableContainer.is() )
        return;

    m_aErrors = SQLException();


    // Determine statement type ...
    if (SQL_ISRULE(m_pParseTree,select_statement) || SQL_ISRULE(m_pParseTree,union_statement) )
    {
        m_eStatementType = SQL_STATEMENT_SELECT;
    }
    else if (SQL_ISRULE(m_pParseTree,insert_statement))
    {
        m_eStatementType = SQL_STATEMENT_INSERT;
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        m_eStatementType = SQL_STATEMENT_UPDATE;
    }
    else if (SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        m_eStatementType = SQL_STATEMENT_DELETE;
    }
    else if (m_pParseTree->count() == 3 && SQL_ISRULE(m_pParseTree->getChild(1),odbc_call_spec))
    {
        m_eStatementType = SQL_STATEMENT_ODBC_CALL;
    }
    else if (SQL_ISRULE(m_pParseTree->getChild(0),base_table_def))
    {
        m_eStatementType = SQL_STATEMENT_CREATE_TABLE;
        m_pParseTree = m_pParseTree->getChild(0);
    }
    else
    {
        m_eStatementType = SQL_STATEMENT_UNKNOWN;
        //aIteratorStatus.setInvalidStatement();
        return;
    }
}

//-----------------------------------------------------------------------------
namespace
{
    //.........................................................................
    static void impl_getRowString( const Reference< XRow >& _rxRow, const sal_Int32 _nColumnIndex, ::rtl::OUString& _out_rString )
    {
        _out_rString = _rxRow->getString( _nColumnIndex );
        if ( _rxRow->wasNull() )
            _out_rString= ::rtl::OUString();
    }

    //.........................................................................
    static ::rtl::OUString lcl_findTableInMetaData(
        const Reference< XDatabaseMetaData >& _rxDBMeta, const ::rtl::OUString& _rCatalog,
        const ::rtl::OUString& _rSchema, const ::rtl::OUString& _rTableName )
    {
        ::rtl::OUString sComposedName;

        static const ::rtl::OUString s_sTableTypeView("VIEW");
        static const ::rtl::OUString s_sTableTypeTable("TABLE");
        static const ::rtl::OUString s_sWildcard(  "%" );

        // we want all catalogues, all schemas, all tables
        Sequence< ::rtl::OUString > sTableTypes(3);
        sTableTypes[0] = s_sTableTypeView;
        sTableTypes[1] = s_sTableTypeTable;
        sTableTypes[2] = s_sWildcard;   // just to be sure to include anything else ....

        if ( _rxDBMeta.is() )
        {
            sComposedName = ::rtl::OUString();

            Reference< XResultSet> xRes = _rxDBMeta->getTables(
                !_rCatalog.isEmpty() ? makeAny( _rCatalog ) : Any(), !_rSchema.isEmpty() ? _rSchema : s_sWildcard, _rTableName, sTableTypes );

            Reference< XRow > xCurrentRow( xRes, UNO_QUERY );
            if ( xCurrentRow.is() && xRes->next() )
            {
                ::rtl::OUString sCatalog, sSchema, sName;

                impl_getRowString( xCurrentRow, 1, sCatalog );
                impl_getRowString( xCurrentRow, 2, sSchema );
                impl_getRowString( xCurrentRow, 3, sName );

                sComposedName = ::dbtools::composeTableName(
                    _rxDBMeta,
                    sCatalog,
                    sSchema,
                    sName,
                    sal_False,
                    ::dbtools::eInDataManipulation
                );
            }
        }
        return sComposedName;
    }
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::impl_getQueryParameterColumns( const OSQLTable& _rQuery  )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::impl_getQueryParameterColumns" );
    if ( ( m_pImpl->m_nIncludeMask & Parameters ) != Parameters )
        // parameters not to be included in the traversal
        return;

    ::rtl::Reference< OSQLColumns > pSubQueryParameterColumns( new OSQLColumns() );

    // get the command and the EscapeProcessing properties from the sub query
    ::rtl::OUString sSubQueryCommand;
    sal_Bool bEscapeProcessing = sal_False;
    try
    {
        Reference< XPropertySet > xQueryProperties( _rQuery, UNO_QUERY_THROW );
        OSL_VERIFY( xQueryProperties->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_COMMAND ) ) >>= sSubQueryCommand );
        OSL_VERIFY( xQueryProperties->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_ESCAPEPROCESSING ) ) >>= bEscapeProcessing );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // parse the sub query
    do {

    if ( !bEscapeProcessing || ( sSubQueryCommand.isEmpty() ) )
        break;

    ::rtl::OUString sError;
    ::std::auto_ptr< OSQLParseNode > pSubQueryNode( const_cast< OSQLParser& >( m_rParser ).parseTree( sError, sSubQueryCommand, sal_False ) );
    if ( !pSubQueryNode.get() )
        break;

    OSQLParseTreeIterator aSubQueryIterator( *this, m_rParser, pSubQueryNode.get() );
    aSubQueryIterator.traverseSome( Parameters | SelectColumns );
        // SelectColumns might also contain parameters
        // #i77635# - 2007-07-23 / frank.schoenheit@sun.com
    pSubQueryParameterColumns = aSubQueryIterator.getParameters();
    aSubQueryIterator.dispose();

    } while ( false );

    // copy the parameters of the sub query to our own parameter array
    ::std::copy( pSubQueryParameterColumns->get().begin(), pSubQueryParameterColumns->get().end(),
        ::std::insert_iterator< OSQLColumns::Vector >( m_aParameters->get(), m_aParameters->get().end() ) );
}

//-----------------------------------------------------------------------------
OSQLTable OSQLParseTreeIterator::impl_locateRecordSource( const ::rtl::OUString& _rComposedName )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::impl_locateRecordSource" );
    if ( _rComposedName.isEmpty() )
    {
        OSL_FAIL( "OSQLParseTreeIterator::impl_locateRecordSource: no object name at all?" );
        return OSQLTable();
    }

    OSQLTable aReturn;
    ::rtl::OUString sComposedName( _rComposedName );

    try
    {
        ::rtl::OUString sCatalog, sSchema, sName;
        qualifiedNameComponents( m_pImpl->m_xDatabaseMetaData, sComposedName, sCatalog, sSchema, sName, ::dbtools::eInDataManipulation );

        // check whether there is a query with the given name
        bool bQueryDoesExist = m_pImpl->m_xQueryContainer.is() && m_pImpl->m_xQueryContainer->hasByName( sComposedName );

        // check whether the table container contains an object with the given name
        if ( !bQueryDoesExist && !m_pImpl->m_xTableContainer->hasByName( sComposedName ) )
            sComposedName = lcl_findTableInMetaData( m_pImpl->m_xDatabaseMetaData, sCatalog, sSchema, sName );
        bool bTableDoesExist = m_pImpl->m_xTableContainer->hasByName( sComposedName );

        // now obtain the object

        // if we're creating a table, and there already is a table or query with the same name,
        // this is worth an error
        if ( SQL_STATEMENT_CREATE_TABLE == m_eStatementType )
        {
            if ( bQueryDoesExist )
                impl_appendError( IParseContext::ERROR_INVALID_QUERY_EXIST, &sName );
            else if ( bTableDoesExist )
                impl_appendError( IParseContext::ERROR_INVALID_TABLE_EXIST, &sName );
            else
                aReturn = impl_createTableObject( sName, sCatalog, sSchema );
        }
        else
        {
            // queries win over tables, so if there's a query with this name, take this, no matter if
            // there's a table, too
            if ( bQueryDoesExist )
            {
                if  ( !m_pImpl->isQueryAllowed( sComposedName ) )
                {
                    impl_appendError( m_rParser.getErrorHelper().getSQLException( sdb::ErrorCondition::PARSER_CYCLIC_SUB_QUERIES, NULL ) );
                    return NULL;
                }

                m_pImpl->m_xQueryContainer->getByName( sComposedName ) >>= aReturn;

                // collect the parameters from the sub query
                ForbidQueryName aForbidName( *m_pImpl, sComposedName );
                impl_getQueryParameterColumns( aReturn );
            }
            else if ( bTableDoesExist )
                m_pImpl->m_xTableContainer->getByName( sComposedName ) >>= aReturn;
            else
            {
                if ( m_pImpl->m_xQueryContainer.is() )
                    // the connection on which we're working supports sub queries in from (else
                    // m_xQueryContainer would not have been set), so emit a better error message
                    impl_appendError( IParseContext::ERROR_INVALID_TABLE_OR_QUERY, &sName );
                else
                    impl_appendError( IParseContext::ERROR_INVALID_TABLE, &sName );
            }
        }
    }
    catch(Exception&)
    {
        impl_appendError( IParseContext::ERROR_INVALID_TABLE, &sComposedName );
    }

    return aReturn;
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOneTableName( OSQLTables& _rTables,const OSQLParseNode * pTableName, const ::rtl::OUString & rTableRange )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseOneTableName" );
    if ( ( m_pImpl->m_nIncludeMask & TableNames ) != TableNames )
        // tables should not be included in the traversal
        return;

    OSL_ENSURE(pTableName != NULL,"OSQLParseTreeIterator::traverseOneTableName: pTableName == NULL");

    Any aCatalog;
    ::rtl::OUString aSchema,aTableName,aComposedName;
    ::rtl::OUString aTableRange(rTableRange);

    // Get table name
    OSQLParseNode::getTableComponents(pTableName,aCatalog,aSchema,aTableName,m_pImpl->m_xDatabaseMetaData);

    // create the composed name like DOMAIN.USER.TABLE1
    aComposedName = ::dbtools::composeTableName(m_pImpl->m_xDatabaseMetaData,
                                aCatalog.hasValue() ? ::comphelper::getString(aCatalog) : ::rtl::OUString(),
                                aSchema,
                                aTableName,
                                sal_False,
                                ::dbtools::eInDataManipulation);

    // if there is no alias for the table name assign the orignal name to it
    if ( aTableRange.isEmpty() )
        aTableRange = aComposedName;

    // get the object representing this table/query
    OSQLTable aTable = impl_locateRecordSource( aComposedName );
    if ( aTable.is() )
        _rTables[ aTableRange ] = aTable;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::impl_fillJoinConditions(const OSQLParseNode* i_pJoinCondition)
{
    if (i_pJoinCondition->count() == 3 &&   // Expression with brackets
        SQL_ISPUNCTUATION(i_pJoinCondition->getChild(0),"(") &&
        SQL_ISPUNCTUATION(i_pJoinCondition->getChild(2),")"))
    {
        impl_fillJoinConditions(i_pJoinCondition->getChild(1));
    }
    else if (SQL_ISRULEOR2(i_pJoinCondition,search_condition,boolean_term)  &&  // AND/OR logic operation:
             i_pJoinCondition->count() == 3)
    {
        // Only allow AND logic operation
        if ( SQL_ISTOKEN(i_pJoinCondition->getChild(1),AND) )
        {
            impl_fillJoinConditions(i_pJoinCondition->getChild(0));
            impl_fillJoinConditions(i_pJoinCondition->getChild(1));
        }
    }
    else if (SQL_ISRULE(i_pJoinCondition,comparison_predicate))
    {
        // only the comparison of columns is allowed
        OSL_ENSURE(i_pJoinCondition->count() == 3,"OQueryDesignView::InsertJoinConnection: error in the parse tree");
        if (SQL_ISRULE(i_pJoinCondition->getChild(0),column_ref) &&
              SQL_ISRULE(i_pJoinCondition->getChild(2),column_ref) &&
               i_pJoinCondition->getChild(1)->getNodeType() == SQL_NODE_EQUAL)
        {
            m_pImpl->m_aJoinConditions.push_back( TNodePair(i_pJoinCondition->getChild(0),i_pJoinCondition->getChild(2)) );
        }
    }
}
//-----------------------------------------------------------------------------
::std::vector< TNodePair >& OSQLParseTreeIterator::getJoinConditions() const
{
    return m_pImpl->m_aJoinConditions;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getQualified_join( OSQLTables& _rTables, const OSQLParseNode *pTableRef, ::rtl::OUString& aTableRange )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getQualified_join" );
    OSL_PRECOND( SQL_ISRULE( pTableRef, cross_union ) || SQL_ISRULE( pTableRef, qualified_join ) ,
        "OSQLParseTreeIterator::getQualified_join: illegal node!" );

    aTableRange = ::rtl::OUString();

    const OSQLParseNode* pNode = getTableNode(_rTables,pTableRef->getChild(0),aTableRange);
    if ( isTableNode( pNode ) )
        traverseOneTableName( _rTables, pNode, aTableRange );

    sal_uInt32 nPos = 4;
    if( SQL_ISRULE(pTableRef,cross_union) || pTableRef->getChild(1)->getTokenID() != SQL_TOKEN_NATURAL)
    {
        nPos = 3;
        // join_condition,named_columns_join
        if ( SQL_ISRULE( pTableRef, qualified_join ) )
        {
            const OSQLParseNode* pJoin_spec = pTableRef->getChild(4);
            if ( SQL_ISRULE( pJoin_spec, join_condition ) )
            {
                impl_fillJoinConditions(pJoin_spec->getChild(1));
            }
            else
            {
                const OSQLParseNode* pColumnCommalist = pJoin_spec->getChild(2);
                // All columns in the column_commalist ...
                for (sal_uInt32 i = 0; i < pColumnCommalist->count(); i++)
                {
                    const OSQLParseNode * pCol = pColumnCommalist->getChild(i);
                    // add twice because the column must exists in both tables
                    m_pImpl->m_aJoinConditions.push_back( TNodePair(pCol,pCol) );
                }
            }
        }
    }

    pNode = getTableNode(_rTables,pTableRef->getChild(nPos),aTableRange);
    if ( isTableNode( pNode ) )
        traverseOneTableName( _rTables, pNode, aTableRange );
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getTableNode( OSQLTables& _rTables, const OSQLParseNode *pTableRef,::rtl::OUString& rTableRange )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getTableNode" );
    OSL_PRECOND( SQL_ISRULE( pTableRef, table_ref ) || SQL_ISRULE( pTableRef, joined_table )
              || SQL_ISRULE( pTableRef, qualified_join ) || SQL_ISRULE( pTableRef, cross_union ),
        "OSQLParseTreeIterator::getTableNode: only to be called for table_ref nodes!" );

    const OSQLParseNode* pTableNameNode = NULL;

    if ( SQL_ISRULE( pTableRef, joined_table ) )
    {
        getQualified_join( _rTables, pTableRef->getChild(1), rTableRange );
    }
    if ( SQL_ISRULE( pTableRef, qualified_join ) || SQL_ISRULE( pTableRef, cross_union ) )
    {
        getQualified_join( _rTables, pTableRef, rTableRange );
    }
    else
    {
        rTableRange = OSQLParseNode::getTableRange(pTableRef);
        if  (   ( pTableRef->count() == 4 ) // '{' SQL_TOKEN_OJ joined_table '}'
            ||  ( pTableRef->count() == 5 ) // '(' joined_table ')' range_variable op_column_commalist
            )
        {
            getQualified_join( _rTables, pTableRef->getChild(6 - pTableRef->count()), rTableRange );
        }
        else if ( pTableRef->count() == 3 ) // subquery range_variable op_column_commalist || '(' joined_table ')'
        {
            const OSQLParseNode* pSubQuery = pTableRef->getChild(0);
            if ( pSubQuery->isToken() )
            {
                getQualified_join( _rTables, pTableRef->getChild(1), rTableRange );
            }
            else
            {
                OSL_ENSURE( pSubQuery->count() == 3, "sub queries should have 3 children!" );
                const OSQLParseNode* pQueryExpression = pSubQuery->getChild(1);
                if ( SQL_ISRULE( pQueryExpression, select_statement ) )
                {
                    getSelect_statement( *m_pImpl->m_pSubTables, pQueryExpression );
                }
                else
                {
                    OSL_FAIL( "OSQLParseTreeIterator::getTableNode: subquery which is no select_statement: not yet implemented!" );
                }
            }
        }
        else if ( pTableRef->count() == 2 ) // table_node table_primary_as_range_column
        {
            pTableNameNode = pTableRef->getChild(0);
        }
        else
            OSL_FAIL( "OSQLParseTreeIterator::getTableNode: unhandled case!" );
    }

    return pTableNameNode;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getSelect_statement(OSQLTables& _rTables,const OSQLParseNode* pSelect)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getSelect_statement" );
    if(SQL_ISRULE(pSelect,union_statement))
    {
        getSelect_statement(_rTables,pSelect->getChild(0));
        //getSelect_statement(pSelect->getChild(3));
        return;
    }
    OSQLParseNode * pTableRefCommalist = pSelect->getChild(3)->getChild(0)->getChild(1);

    OSL_ENSURE(pTableRefCommalist != NULL,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableRefCommalist,table_ref_commalist),"OSQLParseTreeIterator: error in parse tree!");

    const OSQLParseNode* pTableName = NULL;
    ::rtl::OUString aTableRange;
    for (sal_uInt32 i = 0; i < pTableRefCommalist->count(); i++)
    {   // Process FROM clause
        aTableRange = ::rtl::OUString();

        const OSQLParseNode* pTableListElement = pTableRefCommalist->getChild(i);
        if ( isTableNode( pTableListElement ) )
        {
            traverseOneTableName( _rTables, pTableListElement, aTableRange );
        }
        else if ( SQL_ISRULE( pTableListElement, table_ref ) )
        {
            // Table refereneces can be made up of table names, table names (+),'('joined_table')'(+)
            pTableName = pTableListElement->getChild(0);
            if( isTableNode( pTableName ) )
            {   // Found table names
                aTableRange = OSQLParseNode::getTableRange(pTableListElement);
                traverseOneTableName( _rTables, pTableName, aTableRange );
            }
            else if(SQL_ISPUNCTUATION(pTableName,"{"))
            {   // '{' SQL_TOKEN_OJ joined_table '}'
                getQualified_join( _rTables, pTableListElement->getChild(2), aTableRange );
            }
            else
            {   // '(' joined_table ')' range_variable op_column_commalist
                getTableNode( _rTables, pTableListElement, aTableRange );
            }
        }
        else if (SQL_ISRULE( pTableListElement, qualified_join ) || SQL_ISRULE( pTableListElement, cross_union ) )
        {
            getQualified_join( _rTables, pTableListElement, aTableRange );
        }
        else if ( SQL_ISRULE( pTableListElement, joined_table ) )
        {
            getQualified_join( _rTables, pTableListElement->getChild(1), aTableRange );
        }

        //  if (! aIteratorStatus.IsSuccessful()) break;
    }
}
//-----------------------------------------------------------------------------
bool OSQLParseTreeIterator::traverseTableNames(OSQLTables& _rTables)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseTableNames" );
    if ( m_pParseTree == NULL )
        return false;

    OSQLParseNode* pTableName = NULL;

    switch ( m_eStatementType )
    {
        case SQL_STATEMENT_SELECT:
            getSelect_statement( _rTables, m_pParseTree );
            break;

        case SQL_STATEMENT_CREATE_TABLE:
        case SQL_STATEMENT_INSERT:
        case SQL_STATEMENT_DELETE:
            pTableName = m_pParseTree->getChild(2);
            break;

        case SQL_STATEMENT_UPDATE:
            pTableName = m_pParseTree->getChild(1);
            break;
        default:
            break;
    }

    if ( pTableName )
    {
        ::rtl::OUString sTableRange;
        traverseOneTableName( _rTables, pTableName, sTableRange );
    }

    return !hasErrors();
}
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseTreeIterator::getColumnAlias(const OSQLParseNode* _pDerivedColumn)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getColumnAlias" );
    OSL_ENSURE(SQL_ISRULE(_pDerivedColumn,derived_column),"No derived column!");
    ::rtl::OUString sColumnAlias;
    if(_pDerivedColumn->getChild(1)->count() == 2)
        sColumnAlias = _pDerivedColumn->getChild(1)->getChild(1)->getTokenValue();
    else if(!_pDerivedColumn->getChild(1)->isRule())
        sColumnAlias = _pDerivedColumn->getChild(1)->getTokenValue();
    return sColumnAlias;
}

// -----------------------------------------------------------------------------
namespace
{
    void lcl_getColumnRange( const OSQLParseNode* _pColumnRef, const Reference< XConnection >& _rxConnection,
        ::rtl::OUString& _out_rColumnName, ::rtl::OUString& _out_rTableRange,
        const OSQLColumns* _pSelectColumns, ::rtl::OUString& _out_rColumnAliasIfPresent )
    {
        _out_rColumnName = _out_rTableRange = _out_rColumnAliasIfPresent = ::rtl::OUString();
        if ( SQL_ISRULE( _pColumnRef, column_ref ) )
        {
            if( _pColumnRef->count() > 1 )
            {
                for ( sal_Int32 i=0; i<((sal_Int32)_pColumnRef->count())-2; ++i )
                    _pColumnRef->getChild(i)->parseNodeToStr( _out_rTableRange, _rxConnection, NULL, sal_False, sal_False );
                _out_rColumnName = _pColumnRef->getChild( _pColumnRef->count()-1 )->getChild(0)->getTokenValue();
            }
            else
                _out_rColumnName = _pColumnRef->getChild(0)->getTokenValue();

            // look up the column in the select column, to find an possible alias
            if ( _pSelectColumns )
            {
                for (   OSQLColumns::Vector::const_iterator lookupColumn = _pSelectColumns->get().begin();
                        lookupColumn != _pSelectColumns->get().end();
                        ++lookupColumn
                    )
                {
                    Reference< XPropertySet > xColumn( *lookupColumn );
                    try
                    {
                        ::rtl::OUString sName, sTableName;
                        xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_REALNAME ) ) >>= sName;
                        xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_TABLENAME ) ) >>= sTableName;
                        if ( sName == _out_rColumnName && sTableName == _out_rTableRange )
                            xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_NAME ) ) >>= _out_rColumnAliasIfPresent;
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
        }
        else if(SQL_ISRULE(_pColumnRef,general_set_fct) || SQL_ISRULE(_pColumnRef,set_fct_spec))
        { // Function
            _pColumnRef->parseNodeToStr( _out_rColumnName, _rxConnection );
        }
        else  if(_pColumnRef->getNodeType() == SQL_NODE_NAME)
            _out_rColumnName = _pColumnRef->getTokenValue();
    }
}

// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
                        ::rtl::OUString& _rColumnName,
                        ::rtl::OUString& _rTableRange) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getColumnRange" );
    ::rtl::OUString sDummy;
    lcl_getColumnRange( _pColumnRef, m_pImpl->m_xConnection, _rColumnName, _rTableRange, NULL, sDummy );
}

// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
                        ::rtl::OUString& _rColumnName,
                        ::rtl::OUString& _rTableRange,
                        ::rtl::OUString& _out_rColumnAliasIfPresent ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getColumnRange" );
    lcl_getColumnRange( _pColumnRef, m_pImpl->m_xConnection, _rColumnName, _rTableRange, &*m_aSelectColumns, _out_rColumnAliasIfPresent );
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
    const Reference< XConnection >& _rxConnection, ::rtl::OUString& _out_rColumnName, ::rtl::OUString& _out_rTableRange )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getColumnRange" );
    ::rtl::OUString sDummy;
    lcl_getColumnRange( _pColumnRef, _rxConnection, _out_rColumnName, _out_rTableRange, NULL, sDummy );
}

//-----------------------------------------------------------------------------
sal_Bool OSQLParseTreeIterator::getColumnTableRange(const OSQLParseNode* pNode, ::rtl::OUString &rTableRange) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getColumnTableRange" );
    // See if all columns belong to one table
    if (SQL_ISRULE(pNode,column_ref))
    {
        ::rtl::OUString aColName, aTableRange;
        getColumnRange(pNode, aColName, aTableRange);
        if (aTableRange.isEmpty())   // None found
        {
            // Look for the columns in the tables
            for (ConstOSQLTablesIterator aIter = m_pImpl->m_pTables->begin(); aIter != m_pImpl->m_pTables->end(); ++aIter)
            {
                if (aIter->second.is())
                {
                    try
                    {
                        Reference< XNameAccess > xColumns = aIter->second->getColumns();
                        if(xColumns->hasByName(aColName))
                        {
                            Reference< XPropertySet > xColumn;
                            if (xColumns->getByName(aColName) >>= xColumn)
                            {
                                OSL_ENSURE(xColumn.is(),"Column isn't a propertyset!");
                                aTableRange = aIter->first;
                                break;
                            }
                        }
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
            if (aTableRange.isEmpty())
                return sal_False;
        }


        if (rTableRange.isEmpty())
            rTableRange = aTableRange;
        else if (rTableRange != aTableRange)
            return sal_False;
    }
    else
    {
        for (sal_uInt32 i = 0, ncount = pNode->count(); i < ncount; i++)
        {
            if (!getColumnTableRange(pNode->getChild(i), rTableRange))
                return sal_False;
        }
    }
    return sal_True;
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseCreateColumns(const OSQLParseNode* pSelectNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseCreateColumns" );
    //  aIteratorStatus.Clear();

    if (!pSelectNode || m_eStatementType != SQL_STATEMENT_CREATE_TABLE || m_pImpl->m_pTables->empty())
    {
        impl_appendError( IParseContext::ERROR_GENERAL );
        return;
    }
    if (!SQL_ISRULE(pSelectNode,base_table_element_commalist))
        return ;

    for (sal_uInt32 i = 0; i < pSelectNode->count(); i++)
    {
        OSQLParseNode *pColumnRef = pSelectNode->getChild(i);

        if (SQL_ISRULE(pColumnRef,column_def))
        {
            ::rtl::OUString aColumnName;
            ::rtl::OUString aTypeName;
            ::rtl::OUString aTableRange;
            sal_Int32 nType = DataType::VARCHAR;
            aColumnName = pColumnRef->getChild(0)->getTokenValue();

            OSQLParseNode *pDatatype = pColumnRef->getChild(1);
            if (pDatatype && SQL_ISRULE(pDatatype,character_string_type))
            {
                const OSQLParseNode *pType = pDatatype->getChild(0);
                aTypeName = pType->getTokenValue();
                if (pDatatype->count() == 2 && (pType->getTokenID() == SQL_TOKEN_CHAR || pType->getTokenID() == SQL_TOKEN_CHARACTER ))
                    nType = DataType::CHAR;

                const OSQLParseNode *pParams = pDatatype->getChild(pDatatype->count()-1);
                if ( pParams->count() )
                {
                    sal_Int32 nLen = pParams->getChild(1)->getTokenValue().toInt32();
                    (void)nLen;
                }
            }
            else if(pDatatype && pDatatype->getNodeType() == SQL_NODE_KEYWORD)
            {
                aTypeName = ::rtl::OUString("VARCHAR");
            }

            if (!aTypeName.isEmpty())
            {
                //TODO:Create a new class for create statement to handle field length
                OParseColumn* pColumn = new OParseColumn(aColumnName,aTypeName,::rtl::OUString(),::rtl::OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,nType,sal_False,sal_False,isCaseSensitive());
                pColumn->setFunction(sal_False);
                pColumn->setRealName(aColumnName);

                Reference< XPropertySet> xCol = pColumn;
                m_aCreateColumns->get().push_back(xCol);
            }
        }

    }
}
//-----------------------------------------------------------------------------
bool OSQLParseTreeIterator::traverseSelectColumnNames(const OSQLParseNode* pSelectNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseSelectColumnNames" );
    if ( ( m_pImpl->m_nIncludeMask & SelectColumns ) != SelectColumns )
        return true;

    if (!pSelectNode || m_eStatementType != SQL_STATEMENT_SELECT || m_pImpl->m_pTables->empty())
    {
        impl_appendError( IParseContext::ERROR_GENERAL );
        return false;
    }

    if(SQL_ISRULE(pSelectNode,union_statement))
    {
        return  traverseSelectColumnNames( pSelectNode->getChild( 0 ) )
            /*&&  traverseSelectColumnNames( pSelectNode->getChild( 3 ) )*/;
    }

    static ::rtl::OUString aEmptyString;
    // nyi: more checks for correct structure!
    if (pSelectNode->getChild(2)->isRule() && SQL_ISPUNCTUATION(pSelectNode->getChild(2)->getChild(0),"*"))
    {
        // SELECT * ...
        setSelectColumnName(m_aSelectColumns,::rtl::OUString("*"), aEmptyString,aEmptyString);
    }
    else if (SQL_ISRULE(pSelectNode->getChild(2),scalar_exp_commalist))
    {
        // SELECT column[,column] oder SELECT COUNT(*) ...
        OSQLParseNode * pSelection = pSelectNode->getChild(2);

        for (sal_uInt32 i = 0; i < pSelection->count(); i++)
        {
            OSQLParseNode *pColumnRef = pSelection->getChild(i);

            //if (SQL_ISRULE(pColumnRef,select_sublist))
            if (SQL_ISRULE(pColumnRef,derived_column) &&
                SQL_ISRULE(pColumnRef->getChild(0),column_ref) &&
                pColumnRef->getChild(0)->count() == 3 &&
                SQL_ISPUNCTUATION(pColumnRef->getChild(0)->getChild(2),"*"))
            {
                // All the table's columns
                ::rtl::OUString aTableRange;
                pColumnRef->getChild(0)->parseNodeToStr( aTableRange, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
                setSelectColumnName(m_aSelectColumns,::rtl::OUString("*"), aEmptyString,aTableRange);
                continue;
            }
            else if (SQL_ISRULE(pColumnRef,derived_column))
            {
                ::rtl::OUString aColumnAlias(getColumnAlias(pColumnRef)); // can be empty
                ::rtl::OUString sColumnName;
                ::rtl::OUString aTableRange;
                sal_Int32 nType = DataType::VARCHAR;
                sal_Bool bFkt(sal_False);
                pColumnRef = pColumnRef->getChild(0);
                if (
                        pColumnRef->count() == 3 &&
                        SQL_ISPUNCTUATION(pColumnRef->getChild(0),"(") &&
                        SQL_ISPUNCTUATION(pColumnRef->getChild(2),")")
                    )
                    pColumnRef = pColumnRef->getChild(1);

                if (SQL_ISRULE(pColumnRef,column_ref))
                {
                    getColumnRange(pColumnRef,sColumnName,aTableRange);
                    OSL_ENSURE(!sColumnName.isEmpty(),"Column name must not be empty!");
                }
                else /*if (SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec)    ||
                         SQL_ISRULE(pColumnRef,position_exp)    || SQL_ISRULE(pColumnRef,extract_exp)   ||
                         SQL_ISRULE(pColumnRef,length_exp)      || SQL_ISRULE(pColumnRef,char_value_fct)||
                         SQL_ISRULE(pColumnRef,num_value_exp)   || SQL_ISRULE(pColumnRef,term))*/
                {
                    // Function call present
                    pColumnRef->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, NULL, sal_False, sal_True );
                    ::rtl::OUString sTableRange;
                    // check if the column is also a parameter
                    traverseORCriteria(pColumnRef); // num_value_exp

                    // Do all involved columns of the function belong to one table?
                    if (m_pImpl->m_pTables->size() == 1)
                    {
                        aTableRange = m_pImpl->m_pTables->begin()->first;
                    }
                    else
                    {
                        getColumnTableRange(pColumnRef,aTableRange);
                    }
                    if ( pColumnRef->isRule() )
                    {
                        bFkt = sal_True;
                        nType = getFunctionReturnType(pColumnRef);
                    }
                }
                /*
                else
                {
                    aIteratorStatus.setStatementTooComplex();
                    return;
                }
                */
                if(aColumnAlias.isEmpty())
                    aColumnAlias = sColumnName;
                setSelectColumnName(m_aSelectColumns,sColumnName,aColumnAlias,aTableRange,bFkt,nType,SQL_ISRULE(pColumnRef,general_set_fct) || SQL_ISRULE(pColumnRef,set_fct_spec));
            }
        }
    }

    return !hasErrors();
}


//-----------------------------------------------------------------------------
bool OSQLParseTreeIterator::traverseOrderByColumnNames(const OSQLParseNode* pSelectNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseOrderByColumnNames" );
    traverseByColumnNames( pSelectNode, sal_True );
    return !hasErrors();
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseByColumnNames(const OSQLParseNode* pSelectNode,sal_Bool _bOrder)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseByColumnNames" );
    //  aIteratorStatus.Clear();

    if (pSelectNode == NULL)
    {
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    if (m_eStatementType != SQL_STATEMENT_SELECT)
    {
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    if(SQL_ISRULE(pSelectNode,union_statement))
    {
        traverseByColumnNames(pSelectNode->getChild(0),_bOrder);
        return;
    }

    OSL_ENSURE(pSelectNode->count() >= 4,"OSQLParseTreeIterator: error in parse tree!");

    OSQLParseNode * pTableExp = pSelectNode->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator:table_exp error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    sal_uInt32 nPos = ( _bOrder ? ORDER_BY_CHILD_POS : 2 );

    OSQLParseNode * pOptByClause = pTableExp->getChild(nPos);
    OSL_ENSURE(pOptByClause != NULL,"OSQLParseTreeIterator: error in parse tree!");
    if ( pOptByClause->count() == 0 )
        return;

    OSL_ENSURE(pOptByClause->count() == 3,"OSQLParseTreeIterator: error in parse tree!");

    OSQLParseNode * pOrderingSpecCommalist = pOptByClause->getChild(2);
    OSL_ENSURE(pOrderingSpecCommalist != NULL,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(!_bOrder || SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OSQLParseTreeIterator:ordering_spec_commalist error in parse tree!");
    OSL_ENSURE(pOrderingSpecCommalist->count() > 0,"OSQLParseTreeIterator: error in parse tree!");

    ::rtl::OUString sColumnName,aColumnAlias;
    ::rtl::OUString aTableRange;
    sal_uInt32 nCount = pOrderingSpecCommalist->count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        OSQLParseNode* pColumnRef  = pOrderingSpecCommalist->getChild(i);
        OSL_ENSURE(pColumnRef  != NULL,"OSQLParseTreeIterator: error in parse tree!");
        if ( _bOrder )
        {
            OSL_ENSURE(SQL_ISRULE(pColumnRef,ordering_spec),"OSQLParseTreeIterator:ordering_spec error in parse tree!");
            OSL_ENSURE(pColumnRef->count() == 2,"OSQLParseTreeIterator: error in parse tree!");

            pColumnRef = pColumnRef->getChild(0);
        }
        aTableRange = ::rtl::OUString();
        sColumnName = ::rtl::OUString();
        if ( SQL_ISRULE(pColumnRef,column_ref) )
        {
            // Column name (and TableRange):
            if(SQL_ISRULE(pColumnRef,column_ref))
                getColumnRange(pColumnRef,sColumnName,aTableRange);
            else // an expression
                pColumnRef->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );

            OSL_ENSURE(!sColumnName.isEmpty(),"sColumnName must not be empty!");
        }
        else
        {   // here I found a predicate
            pColumnRef->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
        }
        OSL_ENSURE(pColumnRef != NULL,"OSQLParseTreeIterator: error in parse tree!");
        if ( _bOrder )
        {
            // Ascending/Descending
            OSQLParseNode * pOptAscDesc = pColumnRef->getParent()->getChild(1);
            OSL_ENSURE(pOptAscDesc != NULL,"OSQLParseTreeIterator: error in parse tree!");

            sal_Bool bAscending = pOptAscDesc && SQL_ISTOKEN(pOptAscDesc,ASC);
            setOrderByColumnName(sColumnName, aTableRange,bAscending);
        }
        else
            setGroupByColumnName(sColumnName, aTableRange);
    }
}
//-----------------------------------------------------------------------------
bool OSQLParseTreeIterator::traverseGroupByColumnNames(const OSQLParseNode* pSelectNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseGroupByColumnNames" );
    traverseByColumnNames( pSelectNode, sal_False );
    return !hasErrors();
}

// -----------------------------------------------------------------------------
namespace
{
    ::rtl::OUString lcl_generateParameterName( const OSQLParseNode& _rParentNode, const OSQLParseNode& _rParamNode )
    {
        ::rtl::OUString sColumnName(  "param"  );
        const sal_Int32 nCount = (sal_Int32)_rParentNode.count();
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            if ( _rParentNode.getChild(i) == &_rParamNode )
            {
                sColumnName += ::rtl::OUString::valueOf( i+1 );
                break;
            }
        }
        return sColumnName;
    }
}

// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseParameters(const OSQLParseNode* _pNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseParameters" );
    if ( _pNode == NULL )
        return;

    ::rtl::OUString sColumnName, sTableRange, aColumnAlias;
    const OSQLParseNode* pParent = _pNode->getParent();
    if ( pParent != NULL )
    {
        if ( SQL_ISRULE(pParent,comparison_predicate) ) // x = X
        {
            sal_uInt32 nPos = 0;
            if ( pParent->getChild(nPos) == _pNode )
                nPos = 2;
            const OSQLParseNode* pOther = pParent->getChild(nPos);
            if ( SQL_ISRULE( pOther, column_ref ) )
                getColumnRange( pOther, sColumnName, sTableRange, aColumnAlias);
            else
                pOther->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
        } // if ( SQL_ISRULE(pParent,comparison_predicate) ) // x = X
        else if ( SQL_ISRULE(pParent,other_like_predicate_part_2) )
        {
            const OSQLParseNode* pOther = pParent->getParent()->getChild(0);
            if ( SQL_ISRULE( pOther, column_ref ) )
                getColumnRange( pOther, sColumnName, sTableRange, aColumnAlias);
            else
                pOther->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
        }
        else if ( SQL_ISRULE(pParent,between_predicate_part_2) )
        {
            const OSQLParseNode* pOther = pParent->getParent()->getChild(0);
            if ( SQL_ISRULE( pOther, column_ref ) )
                getColumnRange( pOther, sColumnName, sTableRange, aColumnAlias);
            else
            {
                pOther->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
                lcl_generateParameterName( *pParent, *_pNode );
            }
        }
        else if ( pParent->getNodeType() == SQL_NODE_COMMALISTRULE )
        {
            lcl_generateParameterName( *pParent, *_pNode );
        }
    }
    traverseParameter( _pNode, pParent, sColumnName, sTableRange, aColumnAlias );
    const sal_uInt32 nCount = _pNode->count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        const OSQLParseNode* pChild  = _pNode->getChild(i);
        traverseParameters( pChild );
    }
}
//-----------------------------------------------------------------------------
bool OSQLParseTreeIterator::traverseSelectionCriteria(const OSQLParseNode* pSelectNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseSelectionCriteria" );
    if ( pSelectNode == NULL )
        return false;


    // Analyse parse tree (depending on statement type)
    // and set pointer to WHERE clause:
    OSQLParseNode * pWhereClause = NULL;

    if (m_eStatementType == SQL_STATEMENT_SELECT)
    {
        if(SQL_ISRULE(pSelectNode,union_statement))
        {
            return  traverseSelectionCriteria( pSelectNode->getChild( 0 ) )
                &&  traverseSelectionCriteria( pSelectNode->getChild( 3 ) );
        }
        OSL_ENSURE(pSelectNode->count() >= 4,"OSQLParseTreeIterator: error in parse tree!");

        OSQLParseNode * pTableExp = pSelectNode->getChild(3);
        OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

        pWhereClause = pTableExp->getChild(1);
    } else if (SQL_ISRULE(pSelectNode,update_statement_searched)) {
        OSL_ENSURE(pSelectNode->count() == 5,"OSQLParseTreeIterator: error in parse tree!");
        pWhereClause = pSelectNode->getChild(4);
    } else if (SQL_ISRULE(pSelectNode,delete_statement_searched)) {
        OSL_ENSURE(pSelectNode->count() == 4,"OSQLParseTreeIterator: error in parse tree!");
        pWhereClause = pSelectNode->getChild(3);
    } else if (SQL_ISRULE(pSelectNode,delete_statement_positioned)) {
        // nyi
        OSL_FAIL("OSQLParseTreeIterator::getSelectionCriteria: positioned nyi");
    } else {
        // Other statement, no selection criteria
        return false;
    }

    if (! SQL_ISRULE(pWhereClause,where_clause)) {
        // The WHERE clause is optional most of the time; which means it could be a "optional_where_clause".
        OSL_ENSURE(SQL_ISRULE(pWhereClause,opt_where_clause),"OSQLParseTreeIterator: error in parse tree!");
        return false;
    }

    // But if it's a where_clause, then it must not be empty
    OSL_ENSURE(pWhereClause->count() == 2,"OSQLParseTreeIterator: error in parse tree!");

    OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
    OSL_ENSURE(pComparisonPredicate != NULL,"OSQLParseTreeIterator: error in parse tree!");

    //
    // Process the comparison criteria now (recursively, for a start everything is an OR criterion)
    //

    traverseORCriteria(pComparisonPredicate);

    return !hasErrors();
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseORCriteria(OSQLParseNode * pSearchCondition)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseORCriteria" );


    if (
            pSearchCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")")
        )
    {
        // Round brackets around the expression
        traverseORCriteria(pSearchCondition->getChild(1));
    } else if (SQL_ISRULE(pSearchCondition,search_condition) &&
        pSearchCondition->count() == 3 &&
        SQL_ISTOKEN(pSearchCondition->getChild(1),OR))
    {
        // OR logic operation
        for (int i = 0; i < 3; i++) {
            if (i == 1) continue;       // Skip OR keyword

            // Is the first element an OR again?
            if (i == 0 &&
                SQL_ISRULE(pSearchCondition->getChild(0),search_condition) &&
                pSearchCondition->getChild(0)->count() == 3 &&
                SQL_ISTOKEN(pSearchCondition->getChild(0)->getChild(1),OR))
            {
                // Then process recursively
                traverseORCriteria(pSearchCondition->getChild(0));

            } else {
                // AND criteria
                traverseANDCriteria(pSearchCondition->getChild(i));
                //  if (! aIteratorStatus.IsSuccessful()) break;
            }

            //  if (! aIteratorStatus.IsSuccessful()) break;
        }
    } else {
        // Only *one* criterion or one AND logical operation of criteria
        // Process the AND criteria directly
        traverseANDCriteria(pSearchCondition);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    }

    // Just pass on the error
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseANDCriteria(OSQLParseNode * pSearchCondition)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseANDCriteria" );


    if (
            SQL_ISRULE(pSearchCondition,boolean_primary) &&
            pSearchCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")")
        )
    {
        // Round brackets
        traverseANDCriteria(pSearchCondition->getChild(1));
    }
    // The first element is an OR logical operation
    else  if ( SQL_ISRULE(pSearchCondition,search_condition) && pSearchCondition->count() == 3 )
    {
        // Then process recursively (use the same row) ...
        traverseORCriteria(pSearchCondition->getChild(0));
//      if (! aIteratorStatus.IsSuccessful())
//          return;

        // Continue with the right child
        traverseANDCriteria(pSearchCondition->getChild(2));
    }
    // The first element is an AND logical operation (again)
    else if ( SQL_ISRULE(pSearchCondition,boolean_term) && pSearchCondition->count() == 3 )
    {
        // Then process recursively (use the same row)
        traverseANDCriteria(pSearchCondition->getChild(0));
//      if (! aIteratorStatus.IsSuccessful())
//          return;

        // Continue with the right child
        traverseANDCriteria(pSearchCondition->getChild(2));
    }
    // Else, process single search criteria (like =, !=, ..., LIKE, IS NULL etc.)
    else if (SQL_ISRULE(pSearchCondition,comparison_predicate) )
    {
        ::rtl::OUString aValue;
        pSearchCondition->getChild(2)->parseNodeToStr( aValue, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
        traverseOnePredicate(pSearchCondition->getChild(0),aValue,pSearchCondition->getChild(2));
        impl_fillJoinConditions(pSearchCondition);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
    }
    else if (SQL_ISRULE(pSearchCondition,like_predicate) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        OSL_ENSURE(pSearchCondition->count() == 2,"OSQLParseTreeIterator: error in parse tree!");
        const OSQLParseNode* pPart2 = pSearchCondition->getChild(1);

        sal_Int32 nCurentPos = pPart2->count()-2;

        OSQLParseNode * pNum_value_exp  = pPart2->getChild(nCurentPos);
        OSQLParseNode * pOptEscape      = pPart2->getChild(nCurentPos+1);

        OSL_ENSURE(pNum_value_exp != NULL,"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(pOptEscape != NULL,"OSQLParseTreeIterator: error in parse tree!");

        if (pOptEscape->count() != 0)
        {
            //  aIteratorStatus.setStatementTooComplex();
            return;
        }

        ::rtl::OUString aValue;
        OSQLParseNode * pParam = NULL;
        if (SQL_ISRULE(pNum_value_exp,parameter))
            pParam = pNum_value_exp;
        else if(pNum_value_exp->isToken())
            // Normal value
            aValue = pNum_value_exp->getTokenValue();
        else
        {
            pNum_value_exp->parseNodeToStr( aValue, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
            pParam = pNum_value_exp;
        }

        traverseOnePredicate(pSearchCondition->getChild(0),aValue,pParam);
//      if (! aIteratorStatus.IsSuccessful())
//          return;
    }
    else if (SQL_ISRULE(pSearchCondition,in_predicate))
    {
        OSL_ENSURE(pSearchCondition->count() == 2,"OSQLParseTreeIterator: error in parse tree!");
        const OSQLParseNode* pPart2 = pSearchCondition->getChild(1);

        traverseORCriteria(pSearchCondition->getChild(0));
        //  if (! aIteratorStatus.IsSuccessful()) return;

        OSQLParseNode* pChild = pPart2->getChild(2);
        if ( SQL_ISRULE(pChild->getChild(0),subquery) )
        {
            traverseTableNames( *m_pImpl->m_pSubTables );
            traverseSelectionCriteria(pChild->getChild(0)->getChild(1));
        }
        else
        { // '(' value_exp_commalist ')'
            pChild = pChild->getChild(1);
            sal_Int32 nCount = pChild->count();
            for (sal_Int32 i=0; i < nCount; ++i)
            {
                traverseANDCriteria(pChild->getChild(i));
            }
        }
    }
    else if (SQL_ISRULE(pSearchCondition,test_for_null) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        OSL_ENSURE(pSearchCondition->count() == 2,"OSQLParseTreeIterator: error in parse tree!");
        const OSQLParseNode* pPart2 = pSearchCondition->getChild(1);
        (void)pPart2;
        OSL_ENSURE(SQL_ISTOKEN(pPart2->getChild(0),IS),"OSQLParseTreeIterator: error in parse tree!");

        ::rtl::OUString aString;
        traverseOnePredicate(pSearchCondition->getChild(0),aString,NULL);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    }
    else if (SQL_ISRULE(pSearchCondition,num_value_exp) || SQL_ISRULE(pSearchCondition,term))
    {
        ::rtl::OUString aString;
        traverseOnePredicate(pSearchCondition->getChild(0),aString,pSearchCondition->getChild(0));
        traverseOnePredicate(pSearchCondition->getChild(2),aString,pSearchCondition->getChild(2));
    }
    // Just pass on the error
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseParameter(const OSQLParseNode* _pParseNode
                                              ,const OSQLParseNode* _pParentNode
                                              ,const ::rtl::OUString& _aColumnName
                                              ,const ::rtl::OUString& _aTableRange
                                              ,const ::rtl::OUString& _rColumnAlias)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseParameter" );
    if ( !SQL_ISRULE( _pParseNode, parameter ) )
        return;

    if ( ( m_pImpl->m_nIncludeMask & Parameters ) != Parameters )
        // parameters not to be included in the traversal
        return;

    OSL_ENSURE(_pParseNode->count() > 0,"OSQLParseTreeIterator: error in parse tree!");
    OSQLParseNode * pMark = _pParseNode->getChild(0);
    ::rtl::OUString sParameterName;

    if (SQL_ISPUNCTUATION(pMark,"?"))
    {
        sParameterName =    !_rColumnAlias.isEmpty()
                        ?   _rColumnAlias
                        :   !_aColumnName.isEmpty()
                        ?   _aColumnName
                        :   ::rtl::OUString("?");
    }
    else if (SQL_ISPUNCTUATION(pMark,":"))
    {
        sParameterName = _pParseNode->getChild(1)->getTokenValue();
    }
    else if (SQL_ISPUNCTUATION(pMark,"["))
    {
        sParameterName = _pParseNode->getChild(1)->getTokenValue();
    }
    else
    {
        OSL_FAIL("OSQLParseTreeIterator: error in parse tree!");
    }

    // found a parameter
    if ( _pParentNode && (SQL_ISRULE(_pParentNode,general_set_fct) || SQL_ISRULE(_pParentNode,set_fct_spec)) )
    {// found a function as column_ref
        ::rtl::OUString sFunctionName;
        _pParentNode->getChild(0)->parseNodeToStr( sFunctionName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
        const sal_uInt32 nCount = _pParentNode->count();
        sal_uInt32 i = 0;
        for(; i < nCount;++i)
        {
            if ( _pParentNode->getChild(i) == _pParseNode )
                break;
        }
        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionParameterType( _pParentNode->getChild(0)->getTokenID(), i-1);

        OParseColumn* pColumn = new OParseColumn(   sParameterName,
                                                    ::rtl::OUString(),
                                                    ::rtl::OUString(),
                                                    ::rtl::OUString(),
                                                    ColumnValue::NULLABLE_UNKNOWN,
                                                    0,
                                                    0,
                                                    nType,
                                                    sal_False,
                                                    sal_False,
                                                    isCaseSensitive());
        pColumn->setFunction(sal_True);
        pColumn->setAggregateFunction(sal_True);
        pColumn->setRealName(sFunctionName);
        m_aParameters->get().push_back(pColumn);
    }
    else
    {
        sal_Bool bNotFound = sal_True;
        OSQLColumns::Vector::const_iterator aIter = ::connectivity::find(
            m_aSelectColumns->get().begin(),
            m_aSelectColumns->get().end(),
            _aColumnName,::comphelper::UStringMixEqual( isCaseSensitive() )
        );
        if(aIter != m_aSelectColumns->get().end())
        {
            OParseColumn* pNewColumn = new OParseColumn(*aIter,isCaseSensitive());
            pNewColumn->setName(sParameterName);
            pNewColumn->setRealName(_aColumnName);
            m_aParameters->get().push_back(pNewColumn);
            bNotFound = sal_False;
        }
        else if(!_aColumnName.isEmpty())// search in the tables for the right one
        {

            Reference<XPropertySet> xColumn = findColumn( _aColumnName, _aTableRange, true );

            if ( xColumn.is() )
            {
                OParseColumn* pNewColumn = new OParseColumn(xColumn,isCaseSensitive());
                pNewColumn->setName(sParameterName);
                pNewColumn->setRealName(_aColumnName);
                m_aParameters->get().push_back(pNewColumn);
                bNotFound = sal_False;
            }
        }
        if ( bNotFound )
        {
            sal_Int32 nType = DataType::VARCHAR;
            OSQLParseNode* pParent = _pParentNode ? _pParentNode->getParent() : NULL;
            if ( pParent && (SQL_ISRULE(pParent,general_set_fct) || SQL_ISRULE(pParent,set_fct_spec)) )
            {
                const sal_uInt32 nCount = _pParentNode->count();
                sal_uInt32 i = 0;
                for(; i < nCount;++i)
                {
                    if ( _pParentNode->getChild(i) == _pParseNode )
                        break;
                }
                nType = ::connectivity::OSQLParser::getFunctionParameterType( pParent->getChild(0)->getTokenID(), i+1);
            }

            ::rtl::OUString aNewColName( getUniqueColumnName( sParameterName ) );

            OParseColumn* pColumn = new OParseColumn(aNewColName,
                                                    ::rtl::OUString(),
                                                    ::rtl::OUString(),
                                                    ::rtl::OUString(),
                                                    ColumnValue::NULLABLE_UNKNOWN,
                                                    0,
                                                    0,
                                                    nType,
                                                    sal_False,
                                                    sal_False,
                                                    isCaseSensitive() );
            pColumn->setName(aNewColName);
            pColumn->setRealName(sParameterName);
            m_aParameters->get().push_back(pColumn);
        }
    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseOnePredicate(
                                OSQLParseNode * pColumnRef,
                                ::rtl::OUString& rValue,
                                OSQLParseNode * pParseNode)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseOnePredicate" );
    if ( !pParseNode )
        return;

    // Column name (and TableRange):
    ::rtl::OUString aColumnName, aTableRange, sColumnAlias;
    getColumnRange( pColumnRef, aColumnName, aTableRange, sColumnAlias);

    ::rtl::OUString aName;

    /*if (SQL_ISRULE(pParseNode,parameter))
        traverseParameter( pParseNode, pColumnRef, aColumnName, aTableRange, sColumnAlias );
    else */if (SQL_ISRULE(pParseNode,column_ref))// Column-Name (und TableRange):
        getColumnRange(pParseNode,aName,rValue);
    else
    {
        traverseORCriteria(pParseNode);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    }
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseSome( sal_uInt32 _nIncludeMask )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseSome" );
    impl_traverse( _nIncludeMask );
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::traverseAll()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::traverseAll" );
    impl_traverse( All );
}

//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::impl_traverse( sal_uInt32 _nIncludeMask )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::impl_traverse" );
    impl_resetErrors();
    m_pImpl->m_nIncludeMask = _nIncludeMask;

    if ( !traverseTableNames( *m_pImpl->m_pTables ) )
        return;

    switch ( m_eStatementType )
    {
    case SQL_STATEMENT_SELECT:
    {
        const OSQLParseNode* pSelectNode = m_pParseTree;
        traverseParameters( pSelectNode );
        if  (   !traverseSelectColumnNames( pSelectNode )
            ||  !traverseOrderByColumnNames( pSelectNode )
            ||  !traverseGroupByColumnNames( pSelectNode )
            ||  !traverseSelectionCriteria( pSelectNode )
            )
            return;
    }
    break;
    case SQL_STATEMENT_CREATE_TABLE:
    {
        //0     |  1  |  2   |3|        4         |5
        //create table sc.foo ( a char(20), b char )
        const OSQLParseNode* pCreateNode = m_pParseTree->getChild(4);
        traverseCreateColumns(pCreateNode);
    }
    break;
    case SQL_STATEMENT_INSERT:
        break;
    default:
        break;
    }
}

// Dummy implementations

//-----------------------------------------------------------------------------
OSQLTable OSQLParseTreeIterator::impl_createTableObject( const ::rtl::OUString& rTableName,
    const ::rtl::OUString& rCatalogName, const ::rtl::OUString& rSchemaName )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::impl_createTableObject" );
    OSL_PRECOND( m_eStatementType == SQL_STATEMENT_CREATE_TABLE,
        "OSQLParseTreeIterator::impl_createTableObject: only to be called for CREATE TABLE statements!" );
        // (in all other cases, m_pTables is to contain the table objects as obtained from the tables
        // container of the connection (m_xTablesContainer)

    OSQLTable aReturnTable = new OTable(
        NULL,
        sal_False,
        rTableName,
        ::rtl::OUString("Table"),
        ::rtl::OUString("New Created Table"),
        rSchemaName,
        rCatalogName
    );
    return aReturnTable;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::appendColumns(::rtl::Reference<OSQLColumns>& _rColumns,const ::rtl::OUString& _rTableAlias,const OSQLTable& _rTable)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::appendColumns" );

    if (!_rTable.is())
        return;

    Reference<XNameAccess> xColumns = _rTable->getColumns();
    if ( !xColumns.is() )
        return;

    Sequence< ::rtl::OUString > aColNames =  xColumns->getElementNames();
    const ::rtl::OUString* pBegin = aColNames.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + aColNames.getLength();

    for(;pBegin != pEnd;++pBegin)
    {

        ::rtl::OUString aName(getUniqueColumnName(*pBegin));
        Reference< XPropertySet > xColumn;
        if(xColumns->hasByName(*pBegin) && (xColumns->getByName(*pBegin) >>= xColumn) && xColumn.is())
        {
            OParseColumn* pColumn = new OParseColumn(aName
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME)))
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE)))
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE)))
                                                ,   getINT32(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)))
                                                ,   getBOOL(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)))
                                                ,   getBOOL(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY)))
                                                ,   isCaseSensitive() );

            pColumn->setTableName(_rTableAlias);
            pColumn->setRealName(*pBegin);
            Reference< XPropertySet> xCol = pColumn;
            _rColumns->get().push_back(xCol);
        }
        else
            impl_appendError( IParseContext::ERROR_INVALID_COLUMN, pBegin, &_rTableAlias );
    }
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setSelectColumnName(::rtl::Reference<OSQLColumns>& _rColumns,const ::rtl::OUString & rColumnName,const ::rtl::OUString & rColumnAlias, const ::rtl::OUString & rTableRange,sal_Bool bFkt,sal_Int32 _nType,sal_Bool bAggFkt)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::setSelectColumnName" );
    if(rColumnName.toChar() == '*' && rTableRange.isEmpty())
    {   // SELECT * ...
        OSL_ENSURE(_rColumns == m_aSelectColumns,"Invalid columns used here!");
        for(ConstOSQLTablesIterator aIter = m_pImpl->m_pTables->begin(); aIter != m_pImpl->m_pTables->end();++aIter)
            appendColumns(_rColumns,aIter->first,aIter->second);
    }
    else if( rColumnName.toChar() == '*' && !rTableRange.isEmpty() )
    {   // SELECT <table>.*
        OSL_ENSURE(_rColumns == m_aSelectColumns,"Invalid columns used here!");
        ConstOSQLTablesIterator aFind = m_pImpl->m_pTables->find(rTableRange);

        if(aFind != m_pImpl->m_pTables->end())
            appendColumns(_rColumns,rTableRange,aFind->second);
    }
    else if ( rTableRange.isEmpty() )
    {   // SELECT <something> ...
        // without table specified
        if ( !bFkt )
        {
            Reference< XPropertySet> xNewColumn;

            for ( OSQLTablesIterator aIter = m_pImpl->m_pTables->begin(); aIter != m_pImpl->m_pTables->end(); ++aIter )
            {
                if ( !aIter->second.is() )
                    continue;

                Reference<XNameAccess> xColumns = aIter->second->getColumns();
                Reference< XPropertySet > xColumn;
                if  (   !xColumns->hasByName( rColumnName )
                    ||  !( xColumns->getByName( rColumnName ) >>= xColumn )
                    )
                    continue;

                ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

                OParseColumn* pColumn = new OParseColumn(xColumn,isCaseSensitive());
                xNewColumn = pColumn;
                pColumn->setTableName(aIter->first);
                pColumn->setName(aNewColName);
                pColumn->setRealName(rColumnName);

                break;
            }

            if ( !xNewColumn.is() )
            {
                // no function (due to the above !bFkt), no existing column
                // => assume an expression
                ::rtl::OUString aNewColName( getUniqueColumnName( rColumnAlias ) );
                // did not find a column with this name in any of the tables
                OParseColumn* pColumn = new OParseColumn(
                    aNewColName,
                    ::rtl::OUString("VARCHAR"),
                        // TODO: does this match with _nType?
                        // Or should be fill this from the getTypeInfo of the connection?
                    ::rtl::OUString(),
                    ::rtl::OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,
                    0,
                    0,
                    _nType,
                    sal_False,
                    sal_False,
                    isCaseSensitive()
                );

                xNewColumn = pColumn;
                pColumn->setRealName( rColumnName );
            }

            _rColumns->get().push_back( xNewColumn );
        }
        else
        {
            ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),::rtl::OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,_nType,sal_False,sal_False,isCaseSensitive());
            pColumn->setFunction(sal_True);
            pColumn->setAggregateFunction(bAggFkt);
            pColumn->setRealName(rColumnName);

            Reference< XPropertySet> xCol = pColumn;
            _rColumns->get().push_back(xCol);
        }
    }
    else    // ColumnName and TableName exist
    {
        ConstOSQLTablesIterator aFind = m_pImpl->m_pTables->find(rTableRange);

        sal_Bool bError = sal_False;
        if (aFind != m_pImpl->m_pTables->end() && aFind->second.is())
        {
            if (bFkt)
            {
                ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

                OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),::rtl::OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,_nType,sal_False,sal_False,isCaseSensitive());
                pColumn->setFunction(sal_True);
                pColumn->setAggregateFunction(bAggFkt);
                pColumn->setRealName(rColumnName);
                pColumn->setTableName(aFind->first);

                Reference< XPropertySet> xCol = pColumn;
                _rColumns->get().push_back(xCol);
            }
            else
            {
                Reference< XPropertySet > xColumn;
                if (aFind->second->getColumns()->hasByName(rColumnName) && (aFind->second->getColumns()->getByName(rColumnName) >>= xColumn))
                {
                    ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

                    OParseColumn* pColumn = new OParseColumn(xColumn,isCaseSensitive());
                    pColumn->setName(aNewColName);
                    pColumn->setRealName(rColumnName);
                    pColumn->setTableName(aFind->first);

                    Reference< XPropertySet> xCol = pColumn;
                    _rColumns->get().push_back(xCol);
                }
                else
                    bError = sal_True;
            }
        }
        else
            bError = sal_True;

        // Table does not exist or lacking field
        if (bError)
        {
            ::rtl::OUString aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,::rtl::OUString(),::rtl::OUString(),::rtl::OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,DataType::VARCHAR,sal_False,sal_False,isCaseSensitive());
            pColumn->setFunction(sal_True);
            pColumn->setAggregateFunction(bAggFkt);

            Reference< XPropertySet> xCol = pColumn;
            _rColumns->get().push_back(xCol);
        }
    }
}
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseTreeIterator::getUniqueColumnName(const ::rtl::OUString & rColumnName) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getUniqueColumnName" );
    ::rtl::OUString aAlias(rColumnName);

    OSQLColumns::Vector::const_iterator aIter = find(
        m_aSelectColumns->get().begin(),
        m_aSelectColumns->get().end(),
        aAlias,
        ::comphelper::UStringMixEqual( isCaseSensitive() )
    );
    sal_Int32 i=1;
    while(aIter != m_aSelectColumns->get().end())
    {
        (aAlias = rColumnName) += ::rtl::OUString::valueOf(i++);
        aIter = find(
            m_aSelectColumns->get().begin(),
            m_aSelectColumns->get().end(),
            aAlias,
            ::comphelper::UStringMixEqual( isCaseSensitive() )
        );
    }
    return aAlias;
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setOrderByColumnName(const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange,sal_Bool bAscending)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::setOrderByColumnName" );
    Reference<XPropertySet> xColumn = findColumn( rColumnName, rTableRange, false );
    if ( xColumn.is() )
        m_aOrderColumns->get().push_back(new OOrderColumn( xColumn, rTableRange, isCaseSensitive(), bAscending ) );
    else
    {
        sal_Int32 nId = rColumnName.toInt32();
        if ( nId > 0 && nId < static_cast<sal_Int32>(m_aSelectColumns->get().size()) )
            m_aOrderColumns->get().push_back( new OOrderColumn( ( m_aSelectColumns->get() )[nId-1], isCaseSensitive(), bAscending ) );
    }

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setOrderByColumnName: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (bAscending ? "true" : "false")
         << "\n";
#endif
}
//-----------------------------------------------------------------------------
void OSQLParseTreeIterator::setGroupByColumnName(const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::setGroupByColumnName" );
    Reference<XPropertySet> xColumn = findColumn( rColumnName, rTableRange, false );
    if ( xColumn.is() )
        m_aGroupColumns->get().push_back(new OParseColumn(xColumn,isCaseSensitive()));
    else
    {
        sal_Int32 nId = rColumnName.toInt32();
        if ( nId > 0 && nId < static_cast<sal_Int32>(m_aSelectColumns->get().size()) )
            m_aGroupColumns->get().push_back(new OParseColumn((m_aSelectColumns->get())[nId-1],isCaseSensitive()));
    }

#ifdef SQL_TEST_PARSETREEITERATOR
    cout << "OSQLParseTreeIterator::setOrderByColumnName: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (bAscending ? "true" : "false")
         << "\n";
#endif
}

//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getWhereTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getWhereTree" );


    if (!m_pParseTree)
        return NULL;

    // Analyse parse tree (depending on statement type)
    // and set pointer to WHERE clause:
    OSQLParseNode * pWhereClause = NULL;
    if(getStatementType() == SQL_STATEMENT_SELECT)
    {
        OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
        OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
        OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

        pWhereClause = pTableExp->getChild(1);
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched) ||
             SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        pWhereClause = m_pParseTree->getChild(m_pParseTree->count()-1);
    }
    if(pWhereClause->count() != 2)
        pWhereClause = NULL;
    return pWhereClause;
}

//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getOrderTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getOrderTree" );


    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Analyse parse tree (depending on statement type)
    // and set pointer to ORDER clause:
    OSQLParseNode * pOrderClause = NULL;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    pOrderClause = pTableExp->getChild(ORDER_BY_CHILD_POS);
    // If it is a order_by, it must not be empty
    if(pOrderClause->count() != 3)
        pOrderClause = NULL;
    return pOrderClause;
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getGroupByTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getGroupByTree" );
    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Analyse parse tree (depending on statement type)
    // and set pointer to ORDER clause:
    OSQLParseNode * pGroupClause = NULL;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    pGroupClause = pTableExp->getChild(2);
    // If it is an order_by, it must not be empty
    if(pGroupClause->count() != 3)
        pGroupClause = NULL;
    return pGroupClause;
}
//-----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getHavingTree() const
{
    if (!m_pParseTree || getStatementType() != SQL_STATEMENT_SELECT)
        return NULL;

    // Analyse parse tree (depending on statement type)
    // and set pointer to ORDER clause:
    OSQLParseNode * pHavingClause = NULL;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != NULL,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    pHavingClause = pTableExp->getChild(3);
    // If it is an order_by, then it must not be empty
    if(pHavingClause->count() < 1)
        pHavingClause = NULL;
    return pHavingClause;
}
// -----------------------------------------------------------------------------
sal_Bool OSQLParseTreeIterator::isTableNode(const OSQLParseNode* _pTableNode) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::isTableNode" );
    return _pTableNode && (SQL_ISRULE(_pTableNode,catalog_name) ||
                           SQL_ISRULE(_pTableNode,schema_name)  ||
                           SQL_ISRULE(_pTableNode,table_name));
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleWhereTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getSimpleWhereTree" );
    const OSQLParseNode* pNode = getWhereTree();
    return pNode ? pNode->getChild(1) : NULL;
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleOrderTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getSimpleOrderTree" );
    const OSQLParseNode* pNode = getOrderTree();
    return pNode ? pNode->getChild(2) : NULL;
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleGroupByTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getSimpleGroupByTree" );
    const OSQLParseNode* pNode = getGroupByTree();
    return pNode ? pNode->getChild(2) : NULL;
}
// -----------------------------------------------------------------------------
const OSQLParseNode* OSQLParseTreeIterator::getSimpleHavingTree() const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::getSimpleHavingTree" );
    const OSQLParseNode* pNode = getHavingTree();
    return pNode ? pNode->getChild(1) : NULL;
}

// -----------------------------------------------------------------------------
Reference< XPropertySet > OSQLParseTreeIterator::findColumn( const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange, bool _bLookInSubTables )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::findColumn" );
    Reference< XPropertySet > xColumn = findColumn( *m_pImpl->m_pTables, rColumnName, rTableRange );
    if ( !xColumn.is() && _bLookInSubTables )
        xColumn = findColumn( *m_pImpl->m_pSubTables, rColumnName, rTableRange );
    return xColumn;
}

// -----------------------------------------------------------------------------
Reference< XPropertySet > OSQLParseTreeIterator::findColumn(const OSQLTables& _rTables,const ::rtl::OUString & rColumnName, const ::rtl::OUString & rTableRange)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::findColumn" );
    Reference< XPropertySet > xColumn;
    if ( !rTableRange.isEmpty() )
    {
        ConstOSQLTablesIterator aFind = _rTables.find(rTableRange);

        if ( aFind != _rTables.end()
            && aFind->second.is()
            && aFind->second->getColumns().is()
            && aFind->second->getColumns()->hasByName(rColumnName) )
            aFind->second->getColumns()->getByName(rColumnName) >>= xColumn;
    }
    if ( !xColumn.is() )
    {
        OSQLTables::const_iterator aEnd = _rTables.end();
        for(OSQLTables::const_iterator aIter = _rTables.begin(); aIter != aEnd; ++aIter)
        {
            if ( aIter->second.is() )
            {
                Reference<XNameAccess> xColumns = aIter->second->getColumns();
                if( xColumns.is() && xColumns->hasByName(rColumnName) && (xColumns->getByName(rColumnName) >>= xColumn) )
                {
                    OSL_ENSURE(xColumn.is(),"Column isn't a propertyset!");
                    break; // This column must only exits once
                }
            }
        }
    }
    return xColumn;
}

// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::impl_appendError( IParseContext::ErrorCode _eError, const ::rtl::OUString* _pReplaceToken1, const ::rtl::OUString* _pReplaceToken2 )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::impl_appendError" );
    ::rtl::OUString sErrorMessage = m_rParser.getContext().getErrorMessage( _eError );
    if ( _pReplaceToken1 )
    {
        bool bTwoTokens = ( _pReplaceToken2 != NULL );
        const sal_Char* pPlaceHolder1 = bTwoTokens ? "#1" : "#";
        const ::rtl::OUString sPlaceHolder1 = ::rtl::OUString::createFromAscii( pPlaceHolder1 );

        sErrorMessage = sErrorMessage.replaceAt( sErrorMessage.indexOf( sPlaceHolder1 ), sPlaceHolder1.getLength(), *_pReplaceToken1 );
        if ( _pReplaceToken2 )
            sErrorMessage = sErrorMessage.replaceAt( sErrorMessage.indexOf( "#2" ), 2, *_pReplaceToken2 );
    }

    impl_appendError( SQLException(
        sErrorMessage, NULL, getStandardSQLState( SQL_GENERAL_ERROR ), 1000, Any() ) );
}

// -----------------------------------------------------------------------------
void OSQLParseTreeIterator::impl_appendError( const SQLException& _rError )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "parse", "Ocke.Janssen@sun.com", "OSQLParseTreeIterator::impl_appendError" );
    if ( !m_aErrors.Message.isEmpty() )
    {
        SQLException* pErrorChain = &m_aErrors;
        while ( pErrorChain->NextException.hasValue() )
            pErrorChain = static_cast< SQLException* >( pErrorChain->NextException.pData );
        pErrorChain->NextException <<= _rError;
    }
    else
        m_aErrors = _rError;
}
// -----------------------------------------------------------------------------
sal_Int32 OSQLParseTreeIterator::getFunctionReturnType(const OSQLParseNode* _pNode )
{
    sal_Int32 nType = DataType::OTHER;
    ::rtl::OUString sFunctionName;
    if ( SQL_ISRULE(_pNode,length_exp) )
    {
        _pNode->getChild(0)->getChild(0)->parseNodeToStr(sFunctionName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );
        nType = ::connectivity::OSQLParser::getFunctionReturnType( sFunctionName, &m_rParser.getContext() );
    }
    else if ( SQL_ISRULE(_pNode,num_value_exp) || SQL_ISRULE(_pNode,term) || SQL_ISRULE(_pNode,factor) )
    {
        nType = DataType::DOUBLE;
    }
    else
    {
        _pNode->getChild(0)->parseNodeToStr(sFunctionName, m_pImpl->m_xConnection, NULL, sal_False, sal_False );

        // MIN and MAX have another return type, we have to check the expression itself.
        // @see http://qa.openoffice.org/issues/show_bug.cgi?id=99566
        if ( SQL_ISRULE(_pNode,general_set_fct) && (SQL_ISTOKEN(_pNode->getChild(0),MIN) || SQL_ISTOKEN(_pNode->getChild(0),MAX) ))
        {
            const OSQLParseNode* pValueExp = _pNode->getChild(3);
            if (SQL_ISRULE(pValueExp,column_ref))
            {
                ::rtl::OUString sColumnName;
                ::rtl::OUString aTableRange;
                getColumnRange(pValueExp,sColumnName,aTableRange);
                OSL_ENSURE(!sColumnName.isEmpty(),"Columnname must not be empty!");
                Reference<XPropertySet> xColumn = findColumn( sColumnName, aTableRange, true );

                if ( xColumn.is() )
                {
                    xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_TYPE)) >>= nType;
                }
            }
            else
            {
                if ( SQL_ISRULE(pValueExp,num_value_exp) || SQL_ISRULE(pValueExp,term) || SQL_ISRULE(pValueExp,factor) )
                {
                    nType = DataType::DOUBLE;
                }
                else if ( SQL_ISRULE(pValueExp,datetime_primary) )
                {
                    switch(pValueExp->getChild(0)->getTokenID() )
                    {
                        case SQL_TOKEN_CURRENT_DATE:
                            nType = DataType::DATE;
                            break;
                        case SQL_TOKEN_CURRENT_TIME:
                            nType = DataType::TIME;
                            break;
                        case SQL_TOKEN_CURRENT_TIMESTAMP:
                            nType = DataType::TIMESTAMP;
                            break;
                    }
                }
                else if ( SQL_ISRULE(pValueExp,value_exp_primary) )
                {
                    nType = getFunctionReturnType(pValueExp->getChild(1));
                }
                else if ( SQL_ISRULE(pValueExp,concatenation)
                        || SQL_ISRULE(pValueExp,char_factor)
                        || SQL_ISRULE(pValueExp,bit_value_fct)
                        || SQL_ISRULE(pValueExp,char_value_fct)
                        || SQL_ISRULE(pValueExp,char_substring_fct)
                        || SQL_ISRULE(pValueExp,fold)
                        || SQL_ISTOKEN(pValueExp,STRING) )
                {
                    nType = DataType::VARCHAR;
                }
            }
            if ( nType == DataType::OTHER )
                nType = DataType::DOUBLE;
        }
        else
            nType = ::connectivity::OSQLParser::getFunctionReturnType( sFunctionName, &m_rParser.getContext() );
    }

    return nType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
