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

#include <connectivity/sqliterator.hxx>
#include <connectivity/sdbcx/VTable.hxx>
#include <connectivity/sqlparse.hxx>
#include <sqlbison.hxx>
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
#include <connectivity/PColumn.hxx>
#include <tools/diagnose_ex.h>
#include <TConnection.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbmetadata.hxx>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <sal/log.hxx>

#include <iterator>
#include <memory>

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
        std::vector< TNodePair >      m_aJoinConditions;
        Reference< XConnection >        m_xConnection;
        Reference< XDatabaseMetaData >  m_xDatabaseMetaData;
        Reference< XNameAccess >        m_xTableContainer;
        Reference< XNameAccess >        m_xQueryContainer;

        std::shared_ptr< OSQLTables >   m_pTables;      // all tables which participate in the SQL statement
        std::shared_ptr< OSQLTables >   m_pSubTables;   // all tables from sub queries not the tables from the select tables
        std::shared_ptr< QueryNameSet > m_pForbiddenQueryNames;

        TraversalParts                  m_nIncludeMask;

        bool                            m_bIsCaseSensitive;

        OSQLParseTreeIteratorImpl( const Reference< XConnection >& _rxConnection, const Reference< XNameAccess >& _rxTables )
            :m_xConnection( _rxConnection )
            ,m_nIncludeMask( TraversalParts::All )
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
        bool    isQueryAllowed( const OUString& _rQueryName )
        {
            if ( !m_pForbiddenQueryNames.get() )
                return true;
            if ( m_pForbiddenQueryNames->find( _rQueryName ) == m_pForbiddenQueryNames->end() )
                return true;
            return false;
        }
    };


    /** helper class for temporarily adding a query name to a list of forbidden query names
    */
    class ForbidQueryName
    {
        std::shared_ptr< QueryNameSet >&    m_rpAllForbiddenNames;
        OUString                         m_sForbiddenQueryName;

    public:
        ForbidQueryName( OSQLParseTreeIteratorImpl& _rIteratorImpl, const OUString& _rForbiddenQueryName )
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

OSQLParseTreeIterator::OSQLParseTreeIterator(const Reference< XConnection >& _rxConnection,
                                             const Reference< XNameAccess >& _rxTables,
                                             const OSQLParser& _rParser )
    :m_rParser( _rParser )
    ,m_pImpl( new OSQLParseTreeIteratorImpl( _rxConnection, _rxTables ) )
{
    setParseTree(nullptr);
}


OSQLParseTreeIterator::OSQLParseTreeIterator( const OSQLParseTreeIterator& _rParentIterator, const OSQLParser& _rParser, const OSQLParseNode* pRoot )
    :m_rParser( _rParser )
    ,m_pImpl( new OSQLParseTreeIteratorImpl( _rParentIterator.m_pImpl->m_xConnection, _rParentIterator.m_pImpl->m_xTableContainer ) )
{
    m_pImpl->m_pForbiddenQueryNames = _rParentIterator.m_pImpl->m_pForbiddenQueryNames;
    setParseTree( pRoot );
}


OSQLParseTreeIterator::~OSQLParseTreeIterator()
{
    dispose();
}


const OSQLTables& OSQLParseTreeIterator::getTables() const
{
    return *m_pImpl->m_pTables;
}


bool OSQLParseTreeIterator::isCaseSensitive() const
{
    return m_pImpl->m_bIsCaseSensitive;
}


void OSQLParseTreeIterator::dispose()
{
    m_aSelectColumns    = nullptr;
    m_aGroupColumns     = nullptr;
    m_aOrderColumns     = nullptr;
    m_aParameters       = nullptr;
    m_pImpl->m_xTableContainer  = nullptr;
    m_pImpl->m_xDatabaseMetaData = nullptr;
    m_aCreateColumns    = nullptr;
    m_pImpl->m_pTables->clear();
    m_pImpl->m_pSubTables->clear();
}

void OSQLParseTreeIterator::setParseTree(const OSQLParseNode * pNewParseTree)
{
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
        m_eStatementType = OSQLStatementType::Unknown;
        return;
    }

    // If m_pParseTree, but no connection then return
    if ( !m_pImpl->m_xTableContainer.is() )
        return;

    m_aErrors = SQLException();


    // Determine statement type ...
    if (SQL_ISRULE(m_pParseTree,select_statement) || SQL_ISRULE(m_pParseTree,union_statement) )
    {
        m_eStatementType = OSQLStatementType::Select;
    }
    else if (SQL_ISRULE(m_pParseTree,insert_statement))
    {
        m_eStatementType = OSQLStatementType::Insert;
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        m_eStatementType = OSQLStatementType::Update;
    }
    else if (SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        m_eStatementType = OSQLStatementType::Delete;
    }
    else if (m_pParseTree->count() == 3 && SQL_ISRULE(m_pParseTree->getChild(1),odbc_call_spec))
    {
        m_eStatementType = OSQLStatementType::OdbcCall;
    }
    else if (SQL_ISRULE(m_pParseTree->getChild(0),base_table_def))
    {
        m_eStatementType = OSQLStatementType::CreateTable;
        m_pParseTree = m_pParseTree->getChild(0);
    }
    else
    {
        m_eStatementType = OSQLStatementType::Unknown;
        //aIteratorStatus.setInvalidStatement();
        return;
    }
}


namespace
{

    void impl_getRowString( const Reference< XRow >& _rxRow, const sal_Int32 _nColumnIndex, OUString& _out_rString )
    {
        _out_rString = _rxRow->getString( _nColumnIndex );
        if ( _rxRow->wasNull() )
            _out_rString.clear();
    }


    OUString lcl_findTableInMetaData(
        const Reference< XDatabaseMetaData >& _rxDBMeta, const OUString& _rCatalog,
        const OUString& _rSchema, const OUString& _rTableName )
    {
        OUString sComposedName;

        static const char s_sWildcard[] = "%" ;

        // we want all catalogues, all schemas, all tables
        Sequence< OUString > sTableTypes(3);
        sTableTypes[0] = "VIEW";
        sTableTypes[1] = "TABLE";
        sTableTypes[2] = s_sWildcard;   // just to be sure to include anything else ....

        if ( _rxDBMeta.is() )
        {
            sComposedName.clear();

            Reference< XResultSet> xRes = _rxDBMeta->getTables(
                !_rCatalog.isEmpty() ? makeAny( _rCatalog ) : Any(), !_rSchema.isEmpty() ? _rSchema : s_sWildcard, _rTableName, sTableTypes );

            Reference< XRow > xCurrentRow( xRes, UNO_QUERY );
            if ( xCurrentRow.is() && xRes->next() )
            {
                OUString sCatalog, sSchema, sName;

                impl_getRowString( xCurrentRow, 1, sCatalog );
                impl_getRowString( xCurrentRow, 2, sSchema );
                impl_getRowString( xCurrentRow, 3, sName );

                sComposedName = ::dbtools::composeTableName(
                    _rxDBMeta,
                    sCatalog,
                    sSchema,
                    sName,
                    false,
                    ::dbtools::EComposeRule::InDataManipulation
                );
            }
        }
        return sComposedName;
    }
}


void OSQLParseTreeIterator::impl_getQueryParameterColumns( const OSQLTable& _rQuery  )
{
    if ( !( m_pImpl->m_nIncludeMask & TraversalParts::Parameters ) )
        // parameters not to be included in the traversal
        return;

    ::rtl::Reference pSubQueryParameterColumns( new OSQLColumns() );

    // get the command and the EscapeProcessing properties from the sub query
    OUString sSubQueryCommand;
    bool bEscapeProcessing = false;
    try
    {
        Reference< XPropertySet > xQueryProperties( _rQuery, UNO_QUERY_THROW );
        OSL_VERIFY( xQueryProperties->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_COMMAND ) ) >>= sSubQueryCommand );
        OSL_VERIFY( xQueryProperties->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_ESCAPEPROCESSING ) ) >>= bEscapeProcessing );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("connectivity.parse");
    }

    // parse the sub query
    do {

    if ( !bEscapeProcessing || ( sSubQueryCommand.isEmpty() ) )
        break;

    OUString sError;
    std::unique_ptr< OSQLParseNode > pSubQueryNode( const_cast< OSQLParser& >( m_rParser ).parseTree( sError, sSubQueryCommand ) );
    if (!pSubQueryNode)
        break;

    OSQLParseTreeIterator aSubQueryIterator( *this, m_rParser, pSubQueryNode.get() );
    aSubQueryIterator.impl_traverse( TraversalParts::Parameters | TraversalParts::SelectColumns );
        // SelectColumns might also contain parameters #i77635#
    pSubQueryParameterColumns = aSubQueryIterator.getParameters();
    aSubQueryIterator.dispose();

    } while ( false );

    // copy the parameters of the sub query to our own parameter array
    std::copy( pSubQueryParameterColumns->get().begin(), pSubQueryParameterColumns->get().end(),
        std::insert_iterator< OSQLColumns::Vector >( m_aParameters->get(), m_aParameters->get().end() ) );
}


OSQLTable OSQLParseTreeIterator::impl_locateRecordSource( const OUString& _rComposedName )
{
    if ( _rComposedName.isEmpty() )
    {
        SAL_WARN( "connectivity.parse", "OSQLParseTreeIterator::impl_locateRecordSource: no object name at all?" );
        return OSQLTable();
    }

    OSQLTable aReturn;
    OUString sComposedName( _rComposedName );

    try
    {
        OUString sCatalog, sSchema, sName;
        qualifiedNameComponents( m_pImpl->m_xDatabaseMetaData, sComposedName, sCatalog, sSchema, sName, ::dbtools::EComposeRule::InDataManipulation );

        // check whether there is a query with the given name
        bool bQueryDoesExist = m_pImpl->m_xQueryContainer.is() && m_pImpl->m_xQueryContainer->hasByName( sComposedName );

        // check whether the table container contains an object with the given name
        if ( !bQueryDoesExist && !m_pImpl->m_xTableContainer->hasByName( sComposedName ) )
            sComposedName = lcl_findTableInMetaData( m_pImpl->m_xDatabaseMetaData, sCatalog, sSchema, sName );
        bool bTableDoesExist = m_pImpl->m_xTableContainer->hasByName( sComposedName );

        // now obtain the object

        // if we're creating a table, and there already is a table or query with the same name,
        // this is worth an error
        if ( OSQLStatementType::CreateTable == m_eStatementType )
        {
            if ( bQueryDoesExist )
                impl_appendError( IParseContext::ErrorCode::InvalidQueryExist, &sName );
            else if ( bTableDoesExist )
                impl_appendError( IParseContext::ErrorCode::InvalidTableExist, &sName );
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
                    impl_appendError( m_rParser.getErrorHelper().getSQLException( sdb::ErrorCondition::PARSER_CYCLIC_SUB_QUERIES, nullptr ) );
                    return nullptr;
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
                    impl_appendError( IParseContext::ErrorCode::InvalidTableOrQuery, &sName );
                else
                    impl_appendError( IParseContext::ErrorCode::InvalidTableNosuch, &sName );
            }
        }
    }
    catch(Exception&)
    {
        impl_appendError( IParseContext::ErrorCode::InvalidTableNosuch, &sComposedName );
    }

    return aReturn;
}


void OSQLParseTreeIterator::traverseOneTableName( OSQLTables& _rTables,const OSQLParseNode * pTableName, const OUString & rTableRange )
{
    if ( !( m_pImpl->m_nIncludeMask & TraversalParts::TableNames ) )
        // tables should not be included in the traversal
        return;

    OSL_ENSURE(pTableName != nullptr,"OSQLParseTreeIterator::traverseOneTableName: pTableName == NULL");

    Any aCatalog;
    OUString aSchema,aTableName,aComposedName;
    OUString aTableRange(rTableRange);

    // Get table name
    OSQLParseNode::getTableComponents(pTableName,aCatalog,aSchema,aTableName,m_pImpl->m_xDatabaseMetaData);

    // create the composed name like DOMAIN.USER.TABLE1
    aComposedName = ::dbtools::composeTableName(m_pImpl->m_xDatabaseMetaData,
                                aCatalog.hasValue() ? ::comphelper::getString(aCatalog) : OUString(),
                                aSchema,
                                aTableName,
                                false,
                                ::dbtools::EComposeRule::InDataManipulation);

    // if there is no alias for the table name assign the original name to it
    if ( aTableRange.isEmpty() )
        aTableRange = aComposedName;

    // get the object representing this table/query
    OSQLTable aTable = impl_locateRecordSource( aComposedName );
    if ( aTable.is() )
        _rTables[ aTableRange ] = aTable;
}

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
               i_pJoinCondition->getChild(1)->getNodeType() == SQLNodeType::Equal)
        {
            m_pImpl->m_aJoinConditions.push_back( TNodePair(i_pJoinCondition->getChild(0),i_pJoinCondition->getChild(2)) );
        }
    }
}

std::vector< TNodePair >& OSQLParseTreeIterator::getJoinConditions() const
{
    return m_pImpl->m_aJoinConditions;
}

void OSQLParseTreeIterator::getQualified_join( OSQLTables& _rTables, const OSQLParseNode *pTableRef, OUString& aTableRange )
{
    OSL_PRECOND( SQL_ISRULE( pTableRef, cross_union ) || SQL_ISRULE( pTableRef, qualified_join ) ,
        "OSQLParseTreeIterator::getQualified_join: illegal node!" );

    aTableRange.clear();

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
                for (size_t i = 0; i < pColumnCommalist->count(); i++)
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

const OSQLParseNode* OSQLParseTreeIterator::getTableNode( OSQLTables& _rTables, const OSQLParseNode *pTableRef,OUString& rTableRange )
{
    OSL_PRECOND( SQL_ISRULE( pTableRef, table_ref ) || SQL_ISRULE( pTableRef, joined_table )
              || SQL_ISRULE( pTableRef, qualified_join ) || SQL_ISRULE( pTableRef, cross_union ),
        "OSQLParseTreeIterator::getTableNode: only to be called for table_ref nodes!" );

    const OSQLParseNode* pTableNameNode = nullptr;

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
                    // TODO: now, we need to setup a OSQLTable from pQueryExpression in some way
                    //       and stick it in _rTables[rTableRange]. Probably fake it by
                    //       setting up a full OSQLParseTreeIterator on pQueryExpression
                    //       and using its m_aSelectColumns
                    //       This is necessary in stuff like "SELECT * FROM tbl1 INNER JOIN (SELECT foo, bar FROM tbl2) AS tbl3"
                    //       so that setSelectColumnName() can expand the "*" correctly.
                    //       See e.g. R_UserAndLastSubscription query of https://bugs.libreoffice.org/attachment.cgi?id=71871
                }
                else
                {
                    SAL_WARN( "connectivity.parse", "OSQLParseTreeIterator::getTableNode: subquery which is no select_statement: not yet implemented!" );
                }
            }
        }
        else if ( pTableRef->count() == 2 ) // table_node table_primary_as_range_column
        {
            pTableNameNode = pTableRef->getChild(0);
        }
        else
            SAL_WARN( "connectivity.parse", "OSQLParseTreeIterator::getTableNode: unhandled case!" );
    }

    return pTableNameNode;
}

void OSQLParseTreeIterator::getSelect_statement(OSQLTables& _rTables,const OSQLParseNode* pSelect)
{
    if(SQL_ISRULE(pSelect,union_statement))
    {
        getSelect_statement(_rTables,pSelect->getChild(0));
        //getSelect_statement(pSelect->getChild(3));
        return;
    }
    OSQLParseNode * pTableRefCommalist = pSelect->getChild(3)->getChild(0)->getChild(1);

    OSL_ENSURE(pTableRefCommalist != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableRefCommalist,table_ref_commalist),"OSQLParseTreeIterator: error in parse tree!");

    const OSQLParseNode* pTableName = nullptr;
    OUString aTableRange;
    for (size_t i = 0; i < pTableRefCommalist->count(); i++)
    {   // Process FROM clause
        aTableRange.clear();

        const OSQLParseNode* pTableListElement = pTableRefCommalist->getChild(i);
        if ( isTableNode( pTableListElement ) )
        {
            traverseOneTableName( _rTables, pTableListElement, aTableRange );
        }
        else if ( SQL_ISRULE( pTableListElement, table_ref ) )
        {
            // Table references can be made up of table names, table names (+),'('joined_table')'(+)
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

bool OSQLParseTreeIterator::traverseTableNames(OSQLTables& _rTables)
{
    if ( m_pParseTree == nullptr )
        return false;

    OSQLParseNode* pTableName = nullptr;

    switch ( m_eStatementType )
    {
        case OSQLStatementType::Select:
            getSelect_statement( _rTables, m_pParseTree );
            break;

        case OSQLStatementType::CreateTable:
        case OSQLStatementType::Insert:
        case OSQLStatementType::Delete:
            pTableName = m_pParseTree->getChild(2);
            break;

        case OSQLStatementType::Update:
            pTableName = m_pParseTree->getChild(1);
            break;
        default:
            break;
    }

    if ( pTableName )
    {
        traverseOneTableName( _rTables, pTableName, OUString() );
    }

    return !hasErrors();
}

OUString OSQLParseTreeIterator::getColumnAlias(const OSQLParseNode* _pDerivedColumn)
{
    OSL_ENSURE(SQL_ISRULE(_pDerivedColumn,derived_column),"No derived column!");
    OUString sColumnAlias;
    if(_pDerivedColumn->getChild(1)->count() == 2)
        sColumnAlias = _pDerivedColumn->getChild(1)->getChild(1)->getTokenValue();
    else if(!_pDerivedColumn->getChild(1)->isRule())
        sColumnAlias = _pDerivedColumn->getChild(1)->getTokenValue();
    return sColumnAlias;
}


namespace
{
    void lcl_getColumnRange( const OSQLParseNode* _pColumnRef, const Reference< XConnection >& _rxConnection,
        OUString& _out_rColumnName, OUString& _out_rTableRange,
        const OSQLColumns* _pSelectColumns, OUString& _out_rColumnAliasIfPresent )
    {
        _out_rColumnName.clear();
        _out_rTableRange.clear();
        _out_rColumnAliasIfPresent.clear();
        if ( SQL_ISRULE( _pColumnRef, column_ref ) )
        {
            if( _pColumnRef->count() > 1 )
            {
                for ( sal_Int32 i=0; i<static_cast<sal_Int32>(_pColumnRef->count())-2; ++i )
                    _pColumnRef->getChild(i)->parseNodeToStr( _out_rTableRange, _rxConnection, nullptr, false, false );
                _out_rColumnName = _pColumnRef->getChild( _pColumnRef->count()-1 )->getChild(0)->getTokenValue();
            }
            else
                _out_rColumnName = _pColumnRef->getChild(0)->getTokenValue();

            // look up the column in the select column, to find an possible alias
            if ( _pSelectColumns )
            {
                for (const Reference< XPropertySet >& xColumn : _pSelectColumns->get())
                {
                    try
                    {
                        OUString sName, sTableName;
                        xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_REALNAME ) ) >>= sName;
                        xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_TABLENAME ) ) >>= sTableName;
                        if ( sName == _out_rColumnName && ( _out_rTableRange.isEmpty() || sTableName == _out_rTableRange ) )
                        {
                            xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_NAME ) ) >>= _out_rColumnAliasIfPresent;
                            break;
                        }
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("connectivity.parse");
                    }
                }
            }
        }
        else if(SQL_ISRULE(_pColumnRef,general_set_fct) || SQL_ISRULE(_pColumnRef,set_fct_spec))
        { // Function
            _pColumnRef->parseNodeToStr( _out_rColumnName, _rxConnection );
        }
        else  if(_pColumnRef->getNodeType() == SQLNodeType::Name)
            _out_rColumnName = _pColumnRef->getTokenValue();
    }
}


void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
                        OUString& _rColumnName,
                        OUString& _rTableRange) const
{
    OUString sDummy;
    lcl_getColumnRange( _pColumnRef, m_pImpl->m_xConnection, _rColumnName, _rTableRange, nullptr, sDummy );
}


void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
                        OUString& _rColumnName,
                        OUString& _rTableRange,
                        OUString& _out_rColumnAliasIfPresent ) const
{
    lcl_getColumnRange( _pColumnRef, m_pImpl->m_xConnection, _rColumnName, _rTableRange, &*m_aSelectColumns, _out_rColumnAliasIfPresent );
}


void OSQLParseTreeIterator::getColumnRange( const OSQLParseNode* _pColumnRef,
    const Reference< XConnection >& _rxConnection, OUString& _out_rColumnName, OUString& _out_rTableRange )
{
    OUString sDummy;
    lcl_getColumnRange( _pColumnRef, _rxConnection, _out_rColumnName, _out_rTableRange, nullptr, sDummy );
}


void OSQLParseTreeIterator::traverseCreateColumns(const OSQLParseNode* pSelectNode)
{
    //  aIteratorStatus.Clear();

    if (!pSelectNode || m_eStatementType != OSQLStatementType::CreateTable || m_pImpl->m_pTables->empty())
    {
        impl_appendError( IParseContext::ErrorCode::General );
        return;
    }
    if (!SQL_ISRULE(pSelectNode,base_table_element_commalist))
        return ;

    for (size_t i = 0; i < pSelectNode->count(); i++)
    {
        OSQLParseNode *pColumnRef = pSelectNode->getChild(i);

        if (SQL_ISRULE(pColumnRef,column_def))
        {
            OUString aColumnName;
            OUString aTypeName;
            sal_Int32 nType = DataType::VARCHAR;
            aColumnName = pColumnRef->getChild(0)->getTokenValue();

            OSQLParseNode *pDatatype = pColumnRef->getChild(1);
            if (pDatatype && SQL_ISRULE(pDatatype,character_string_type))
            {
                const OSQLParseNode *pType = pDatatype->getChild(0);
                aTypeName = pType->getTokenValue();
                if (pDatatype->count() == 2 && (pType->getTokenID() == SQL_TOKEN_CHAR || pType->getTokenID() == SQL_TOKEN_CHARACTER ))
                    nType = DataType::CHAR;
            }
            else if(pDatatype && pDatatype->getNodeType() == SQLNodeType::Keyword)
            {
                aTypeName = "VARCHAR";
            }

            if (!aTypeName.isEmpty())
            {
                //TODO:Create a new class for create statement to handle field length
                OParseColumn* pColumn = new OParseColumn(aColumnName,aTypeName,OUString(),OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,nType,false,false,isCaseSensitive(),
                    OUString(),OUString(),OUString());
                pColumn->setFunction(false);
                pColumn->setRealName(aColumnName);

                Reference< XPropertySet> xCol = pColumn;
                m_aCreateColumns->get().push_back(xCol);
            }
        }

    }
}

bool OSQLParseTreeIterator::traverseSelectColumnNames(const OSQLParseNode* pSelectNode)
{
    if ( !( m_pImpl->m_nIncludeMask & TraversalParts::SelectColumns ) )
        return true;

    if (!pSelectNode || m_eStatementType != OSQLStatementType::Select || m_pImpl->m_pTables->empty())
    {
        impl_appendError( IParseContext::ErrorCode::General );
        return false;
    }

    if(SQL_ISRULE(pSelectNode,union_statement))
    {
        return  traverseSelectColumnNames( pSelectNode->getChild( 0 ) )
            /*&&  traverseSelectColumnNames( pSelectNode->getChild( 3 ) )*/;
    }

    // nyi: more checks for correct structure!
    if (pSelectNode->getChild(2)->isRule() && SQL_ISPUNCTUATION(pSelectNode->getChild(2)->getChild(0),"*"))
    {
        // SELECT * ...
        setSelectColumnName(m_aSelectColumns, "*", "", "");
    }
    else if (SQL_ISRULE(pSelectNode->getChild(2),scalar_exp_commalist))
    {
        // SELECT column[,column] or SELECT COUNT(*) ...
        OSQLParseNode * pSelection = pSelectNode->getChild(2);

        for (size_t i = 0; i < pSelection->count(); i++)
        {
            OSQLParseNode *pColumnRef = pSelection->getChild(i);

            //if (SQL_ISRULE(pColumnRef,select_sublist))
            if (SQL_ISRULE(pColumnRef,derived_column) &&
                SQL_ISRULE(pColumnRef->getChild(0),column_ref) &&
                pColumnRef->getChild(0)->count() == 3 &&
                SQL_ISPUNCTUATION(pColumnRef->getChild(0)->getChild(2),"*"))
            {
                // All the table's columns
                OUString aTableRange;
                pColumnRef->getChild(0)->parseNodeToStr( aTableRange, m_pImpl->m_xConnection, nullptr, false, false );
                setSelectColumnName(m_aSelectColumns, "*", "", aTableRange);
                continue;
            }
            else if (SQL_ISRULE(pColumnRef,derived_column))
            {
                OUString aColumnAlias(getColumnAlias(pColumnRef)); // can be empty
                OUString sColumnName;
                OUString aTableRange;
                sal_Int32 nType = DataType::VARCHAR;
                bool bFkt(false);
                pColumnRef = pColumnRef->getChild(0);
                while (
                        pColumnRef->getKnownRuleID() != OSQLParseNode::subquery &&
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
                    pColumnRef->parseNodeToStr( sColumnName, m_pImpl->m_xConnection );
                    // check if the column is also a parameter
                    traverseSearchCondition(pColumnRef); // num_value_exp

                    if ( pColumnRef->isRule() )
                    {
                        // FIXME: the if condition is not quite right
                        //        many expressions are rules, e.g. "5+3"
                        //        or even: "colName + 1"
                        bFkt = true;
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


bool OSQLParseTreeIterator::traverseOrderByColumnNames(const OSQLParseNode* pSelectNode)
{
    traverseByColumnNames( pSelectNode, true );
    return !hasErrors();
}

void OSQLParseTreeIterator::traverseByColumnNames(const OSQLParseNode* pSelectNode, bool _bOrder)
{
    //  aIteratorStatus.Clear();

    if (pSelectNode == nullptr)
    {
        //aIteratorStatus.setInvalidStatement();
        return;
    }

    if (m_eStatementType != OSQLStatementType::Select)
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
    OSL_ENSURE(pTableExp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator:table_exp error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    sal_uInt32 nPos = ( _bOrder ? ORDER_BY_CHILD_POS : 2 );

    OSQLParseNode * pOptByClause = pTableExp->getChild(nPos);
    OSL_ENSURE(pOptByClause != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    if ( pOptByClause->count() == 0 )
        return;

    OSL_ENSURE(pOptByClause->count() == 3,"OSQLParseTreeIterator: error in parse tree!");

    OSQLParseNode * pOrderingSpecCommalist = pOptByClause->getChild(2);
    OSL_ENSURE(pOrderingSpecCommalist != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(!_bOrder || SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OSQLParseTreeIterator:ordering_spec_commalist error in parse tree!");
    OSL_ENSURE(pOrderingSpecCommalist->count() > 0,"OSQLParseTreeIterator: error in parse tree!");

    OUString sColumnName;
    OUString aTableRange;
    sal_uInt32 nCount = pOrderingSpecCommalist->count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        OSQLParseNode* pColumnRef  = pOrderingSpecCommalist->getChild(i);
        OSL_ENSURE(pColumnRef  != nullptr,"OSQLParseTreeIterator: error in parse tree!");
        if ( _bOrder )
        {
            OSL_ENSURE(SQL_ISRULE(pColumnRef,ordering_spec),"OSQLParseTreeIterator:ordering_spec error in parse tree!");
            OSL_ENSURE(pColumnRef->count() == 2,"OSQLParseTreeIterator: error in parse tree!");

            pColumnRef = pColumnRef->getChild(0);
        }
        OSL_ENSURE(pColumnRef != nullptr,"OSQLParseTreeIterator: error in parse tree!");
        aTableRange.clear();
        sColumnName.clear();
        if ( SQL_ISRULE(pColumnRef,column_ref) )
        {
            // Column name (and TableRange):
            getColumnRange(pColumnRef,sColumnName,aTableRange);
        }
        else
        {   // here I found a predicate
            pColumnRef->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, nullptr, false, false );
        }
        OSL_ENSURE(!sColumnName.isEmpty(),"sColumnName must not be empty!");
        if ( _bOrder )
        {
            // Ascending/Descending
            OSQLParseNode * pOptAscDesc = pColumnRef->getParent()->getChild(1);
            OSL_ENSURE(pOptAscDesc != nullptr,"OSQLParseTreeIterator: error in parse tree!");

            bool bAscending = ! (pOptAscDesc && SQL_ISTOKEN(pOptAscDesc,DESC));
            setOrderByColumnName(sColumnName, aTableRange,bAscending);
        }
        else
            setGroupByColumnName(sColumnName, aTableRange);
    }
}

bool OSQLParseTreeIterator::traverseGroupByColumnNames(const OSQLParseNode* pSelectNode)
{
    traverseByColumnNames( pSelectNode, false );
    return !hasErrors();
}


namespace
{
    OUString lcl_generateParameterName( const OSQLParseNode& _rParentNode, const OSQLParseNode& _rParamNode )
    {
        OUString sColumnName(  "param"  );
        const sal_Int32 nCount = static_cast<sal_Int32>(_rParentNode.count());
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            if ( _rParentNode.getChild(i) == &_rParamNode )
            {
                sColumnName += OUString::number( i+1 );
                break;
            }
        }
        return sColumnName;
    }
}


void OSQLParseTreeIterator::traverseParameters(const OSQLParseNode* _pNode)
{
    if ( _pNode == nullptr )
        return;

    OUString sColumnName, sTableRange, aColumnAlias;
    const OSQLParseNode* pParent = _pNode->getParent();
    if ( pParent != nullptr )
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
                pOther->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, nullptr, false, false );
        } // if ( SQL_ISRULE(pParent,comparison_predicate) ) // x = X
        else if ( SQL_ISRULE(pParent,other_like_predicate_part_2) )
        {
            const OSQLParseNode* pOther = pParent->getParent()->getChild(0);
            if ( SQL_ISRULE( pOther, column_ref ) )
                getColumnRange( pOther, sColumnName, sTableRange, aColumnAlias);
            else
                pOther->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, nullptr, false, false );
        }
        else if ( SQL_ISRULE(pParent,between_predicate_part_2) )
        {
            const OSQLParseNode* pOther = pParent->getParent()->getChild(0);
            if ( SQL_ISRULE( pOther, column_ref ) )
                getColumnRange( pOther, sColumnName, sTableRange, aColumnAlias);
            else
            {
                pOther->parseNodeToStr( sColumnName, m_pImpl->m_xConnection, nullptr, false, false );
                lcl_generateParameterName( *pParent, *_pNode );
            }
        }
        else if ( pParent->getNodeType() == SQLNodeType::CommaListRule )
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

bool OSQLParseTreeIterator::traverseSelectionCriteria(const OSQLParseNode* pSelectNode)
{
    if ( pSelectNode == nullptr )
        return false;


    // Analyse parse tree (depending on statement type)
    // and set pointer to WHERE clause:
    OSQLParseNode * pWhereClause = nullptr;

    if (m_eStatementType == OSQLStatementType::Select)
    {
        if(SQL_ISRULE(pSelectNode,union_statement))
        {
            return  traverseSelectionCriteria( pSelectNode->getChild( 0 ) )
                &&  traverseSelectionCriteria( pSelectNode->getChild( 3 ) );
        }
        OSL_ENSURE(pSelectNode->count() >= 4,"OSQLParseTreeIterator: error in parse tree!");

        OSQLParseNode * pTableExp = pSelectNode->getChild(3);
        OSL_ENSURE(pTableExp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
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
        SAL_WARN( "connectivity.parse","OSQLParseTreeIterator::getSelectionCriteria: positioned nyi");
    } else {
        // Other statement, no selection criteria
        return false;
    }

    if (!pWhereClause || !SQL_ISRULE(pWhereClause,where_clause))
    {
        // The WHERE clause is optional most of the time; which means it could be a "optional_where_clause".
        OSL_ENSURE(pWhereClause && SQL_ISRULE(pWhereClause,opt_where_clause),"OSQLParseTreeIterator: error in parse tree!");
        return false;
    }

    // But if it's a where_clause, then it must not be empty
    OSL_ENSURE(pWhereClause->count() == 2,"OSQLParseTreeIterator: error in parse tree!");

    OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
    OSL_ENSURE(pComparisonPredicate != nullptr,"OSQLParseTreeIterator: error in parse tree!");


    // Process the comparison criteria now


    traverseSearchCondition(pComparisonPredicate);

    return !hasErrors();
}


void OSQLParseTreeIterator::traverseSearchCondition(OSQLParseNode const * pSearchCondition)
{
    if (
            SQL_ISRULE(pSearchCondition,boolean_primary) &&
            pSearchCondition->count() == 3 &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
            SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")")
        )
    {
        // Round brackets
        traverseSearchCondition(pSearchCondition->getChild(1));
    }
    // The first element is an OR logical operation
    else  if ( SQL_ISRULE(pSearchCondition,search_condition) && pSearchCondition->count() == 3 )
    {
        // if this assert fails, the SQL grammar has changed!
        assert(SQL_ISTOKEN(pSearchCondition->getChild(1),OR));
        // Then process recursively (use the same row) ...
        traverseSearchCondition(pSearchCondition->getChild(0));
//      if (! aIteratorStatus.IsSuccessful())
//          return;

        // Continue with the right child
        traverseSearchCondition(pSearchCondition->getChild(2));
    }
    // The first element is an AND logical operation (again)
    else if ( SQL_ISRULE(pSearchCondition,boolean_term) && pSearchCondition->count() == 3 )
    {
        // Then process recursively (use the same row)
        traverseSearchCondition(pSearchCondition->getChild(0));
//      if (! aIteratorStatus.IsSuccessful())
//          return;

        // Continue with the right child
        traverseSearchCondition(pSearchCondition->getChild(2));
    }
    // Else, process single search criteria (like =, !=, ..., LIKE, IS NULL etc.)
    else if (SQL_ISRULE(pSearchCondition,comparison_predicate) )
    {
        OUString aValue;
        pSearchCondition->getChild(2)->parseNodeToStr( aValue, m_pImpl->m_xConnection, nullptr, false, false );
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

        OSL_ENSURE(pNum_value_exp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(pOptEscape != nullptr,"OSQLParseTreeIterator: error in parse tree!");

        if (pOptEscape->count() != 0)
        {
            //  aIteratorStatus.setStatementTooComplex();
            return;
        }

        OUString aValue;
        OSQLParseNode * pParam = nullptr;
        if (SQL_ISRULE(pNum_value_exp,parameter))
            pParam = pNum_value_exp;
        else if(pNum_value_exp->isToken())
            // Normal value
            aValue = pNum_value_exp->getTokenValue();
        else
        {
            pNum_value_exp->parseNodeToStr( aValue, m_pImpl->m_xConnection, nullptr, false, false );
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

        traverseSearchCondition(pSearchCondition->getChild(0));
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
                traverseSearchCondition(pChild->getChild(i));
            }
        }
    }
    else if (SQL_ISRULE(pSearchCondition,test_for_null) /*&& SQL_ISRULE(pSearchCondition->getChild(0),column_ref)*/)
    {
        OSL_ENSURE(pSearchCondition->count() == 2,"OSQLParseTreeIterator: error in parse tree!");
        const OSQLParseNode* pPart2 = pSearchCondition->getChild(1);
        OSL_ENSURE(SQL_ISTOKEN(pPart2->getChild(0),IS),"OSQLParseTreeIterator: error in parse tree!");

        OUString aString;
        traverseOnePredicate(pSearchCondition->getChild(0),aString,nullptr);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    }
    else if (SQL_ISRULE(pSearchCondition,num_value_exp) || SQL_ISRULE(pSearchCondition,term))
    {
        OUString aString;
        traverseOnePredicate(pSearchCondition->getChild(0),aString,pSearchCondition->getChild(0));
        traverseOnePredicate(pSearchCondition->getChild(2),aString,pSearchCondition->getChild(2));
    }
    // Just pass on the error
}

void OSQLParseTreeIterator::traverseParameter(const OSQLParseNode* _pParseNode
                                              ,const OSQLParseNode* _pParentNode
                                              ,const OUString& _aColumnName
                                              ,OUString& _aTableRange
                                              ,const OUString& _rColumnAlias)
{
    if ( !SQL_ISRULE( _pParseNode, parameter ) )
        return;

    if ( !( m_pImpl->m_nIncludeMask & TraversalParts::Parameters ) )
        // parameters not to be included in the traversal
        return;

    OSL_ENSURE(_pParseNode->count() > 0,"OSQLParseTreeIterator: error in parse tree!");
    OSQLParseNode * pMark = _pParseNode->getChild(0);
    OUString sParameterName;

    if (SQL_ISPUNCTUATION(pMark,"?"))
    {
        sParameterName =    !_rColumnAlias.isEmpty()
                        ?   _rColumnAlias
                        :   !_aColumnName.isEmpty()
                        ?   _aColumnName
                        :   OUString("?");
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
        SAL_WARN( "connectivity.parse","OSQLParseTreeIterator: error in parse tree!");
    }

    // found a parameter
    if ( _pParentNode && (SQL_ISRULE(_pParentNode,general_set_fct) || SQL_ISRULE(_pParentNode,set_fct_spec)) )
    {// found a function as column_ref
        OUString sFunctionName;
        _pParentNode->getChild(0)->parseNodeToStr( sFunctionName, m_pImpl->m_xConnection, nullptr, false, false );
        const sal_uInt32 nCount = _pParentNode->count();
        sal_uInt32 i = 0;
        for(; i < nCount;++i)
        {
            if ( _pParentNode->getChild(i) == _pParseNode )
                break;
        }
        sal_Int32 nType = ::connectivity::OSQLParser::getFunctionParameterType( _pParentNode->getChild(0)->getTokenID(), i-1);

        OParseColumn* pColumn = new OParseColumn(   sParameterName,
                                                    OUString(),
                                                    OUString(),
                                                    OUString(),
                                                    ColumnValue::NULLABLE_UNKNOWN,
                                                    0,
                                                    0,
                                                    nType,
                                                    false,
                                                    false,
                                                    isCaseSensitive(),
                                                    OUString(),
                                                    OUString(),
                                                    OUString());
        pColumn->setFunction(true);
        pColumn->setAggregateFunction(true);
        pColumn->setRealName(sFunctionName);
        m_aParameters->get().push_back(pColumn);
    }
    else
    {
        bool bNotFound = true;
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
            bNotFound = false;
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
                bNotFound = false;
            }
        }
        if ( bNotFound )
        {
            sal_Int32 nType = DataType::VARCHAR;
            OSQLParseNode* pParent = _pParentNode ? _pParentNode->getParent() : nullptr;
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

            OUString aNewColName( getUniqueColumnName( sParameterName ) );

            OParseColumn* pColumn = new OParseColumn(aNewColName,
                                                    OUString(),
                                                    OUString(),
                                                    OUString(),
                                                    ColumnValue::NULLABLE_UNKNOWN,
                                                    0,
                                                    0,
                                                    nType,
                                                    false,
                                                    false,
                                                    isCaseSensitive(),
                                                    OUString(),
                                                    OUString(),
                                                    OUString());
            pColumn->setName(aNewColName);
            pColumn->setRealName(sParameterName);
            m_aParameters->get().push_back(pColumn);
        }
    }
}

void OSQLParseTreeIterator::traverseOnePredicate(
                                OSQLParseNode const * pColumnRef,
                                OUString& rValue,
                                OSQLParseNode const * pParseNode)
{
    if ( !pParseNode )
        return;

    // Column name (and TableRange):
    OUString aColumnName, aTableRange, sColumnAlias;
    getColumnRange( pColumnRef, aColumnName, aTableRange, sColumnAlias);

    OUString aName;

    /*if (SQL_ISRULE(pParseNode,parameter))
        traverseParameter( pParseNode, pColumnRef, aColumnName, aTableRange, sColumnAlias );
    else */if (SQL_ISRULE(pParseNode,column_ref))// Column-Name (and TableRange):
        getColumnRange(pParseNode,aName,rValue);
    else
    {
        traverseSearchCondition(pParseNode);
        //  if (! aIteratorStatus.IsSuccessful()) return;
    }
}


void OSQLParseTreeIterator::traverseAll()
{
    impl_traverse( TraversalParts::All );
}


void OSQLParseTreeIterator::impl_traverse( TraversalParts _nIncludeMask )
{
    // resets our errors
    m_aErrors = css::sdbc::SQLException();

    m_pImpl->m_nIncludeMask = _nIncludeMask;

    if ( !traverseTableNames( *m_pImpl->m_pTables ) )
        return;

    switch ( m_eStatementType )
    {
    case OSQLStatementType::Select:
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
    case OSQLStatementType::CreateTable:
    {
        //0     |  1  |  2   |3|        4         |5
        //create table sc.foo ( a char(20), b char )
        const OSQLParseNode* pCreateNode = m_pParseTree->getChild(4);
        traverseCreateColumns(pCreateNode);
    }
    break;
    case OSQLStatementType::Insert:
        break;
    default:
        break;
    }
}

// Dummy implementations


OSQLTable OSQLParseTreeIterator::impl_createTableObject( const OUString& rTableName,
    const OUString& rCatalogName, const OUString& rSchemaName )
{
    OSL_PRECOND( m_eStatementType == OSQLStatementType::CreateTable,
        "OSQLParseTreeIterator::impl_createTableObject: only to be called for CREATE TABLE statements!" );
        // (in all other cases, m_pTables is to contain the table objects as obtained from the tables
        // container of the connection (m_xTablesContainer)

    OSQLTable aReturnTable = new OTable(
        nullptr,
        false,
        rTableName,
        "Table",
        "New Created Table",
        rSchemaName,
        rCatalogName
    );
    return aReturnTable;
}

void OSQLParseTreeIterator::appendColumns(::rtl::Reference<OSQLColumns> const & _rColumns,const OUString& _rTableAlias,const OSQLTable& _rTable)
{
    if (!_rTable.is())
        return;

    Reference<XNameAccess> xColumns = _rTable->getColumns();
    if ( !xColumns.is() )
        return;

    Sequence< OUString > aColNames =  xColumns->getElementNames();
    const OUString* pBegin = aColNames.getConstArray();
    const OUString* pEnd = pBegin + aColNames.getLength();

    for(;pBegin != pEnd;++pBegin)
    {

        OUString aName(getUniqueColumnName(*pBegin));
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
                                                ,   isCaseSensitive()
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME)))
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME)))
                                                ,   getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TABLENAME))));

            pColumn->setTableName(_rTableAlias);
            pColumn->setRealName(*pBegin);
            Reference< XPropertySet> xCol = pColumn;
            _rColumns->get().push_back(xCol);
        }
        else
            impl_appendError( IParseContext::ErrorCode::InvalidColumn, pBegin, &_rTableAlias );
    }
}

void OSQLParseTreeIterator::setSelectColumnName(::rtl::Reference<OSQLColumns> const & _rColumns,const OUString & rColumnName,const OUString & rColumnAlias, const OUString & rTableRange, bool bFkt, sal_Int32 _nType, bool bAggFkt)
{
    if(rColumnName.toChar() == '*' && rTableRange.isEmpty())
    {   // SELECT * ...
        OSL_ENSURE(_rColumns == m_aSelectColumns,"Invalid columns used here!");
        for (auto const& table : *m_pImpl->m_pTables)
            appendColumns(_rColumns,table.first,table.second);
    }
    else if( rColumnName.toChar() == '*' && !rTableRange.isEmpty() )
    {   // SELECT <table>.*
        OSL_ENSURE(_rColumns == m_aSelectColumns,"Invalid columns used here!");
        OSQLTables::const_iterator aFind = m_pImpl->m_pTables->find(rTableRange);

        if(aFind != m_pImpl->m_pTables->end())
            appendColumns(_rColumns,rTableRange,aFind->second);
    }
    else if ( rTableRange.isEmpty() )
    {   // SELECT <something> ...
        // without table specified
        if ( !bFkt )
        {
            Reference< XPropertySet> xNewColumn;

            for (auto const& table : *m_pImpl->m_pTables)
            {
                if ( !table.second.is() )
                    continue;

                Reference<XNameAccess> xColumns = table.second->getColumns();
                Reference< XPropertySet > xColumn;
                if  (   !xColumns->hasByName( rColumnName )
                    ||  !( xColumns->getByName( rColumnName ) >>= xColumn )
                    )
                    continue;

                OUString aNewColName(getUniqueColumnName(rColumnAlias));

                OParseColumn* pColumn = new OParseColumn(xColumn,isCaseSensitive());
                xNewColumn = pColumn;
                pColumn->setTableName(table.first);
                pColumn->setName(aNewColName);
                pColumn->setRealName(rColumnName);

                break;
            }

            if ( !xNewColumn.is() )
            {
                // no function (due to the above !bFkt), no existing column
                // => assume an expression
                OUString aNewColName( getUniqueColumnName( rColumnAlias ) );
                // did not find a column with this name in any of the tables
                OParseColumn* pColumn = new OParseColumn(
                    aNewColName,
                    "VARCHAR",
                        // TODO: does this match with _nType?
                        // Or should be fill this from the getTypeInfo of the connection?
                    OUString(),
                    OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,
                    0,
                    0,
                    _nType,
                    false,
                    false,
                    isCaseSensitive(),
                    OUString(),
                    OUString(),
                    OUString()
                );

                xNewColumn = pColumn;
                pColumn->setRealName( rColumnName );
            }

            _rColumns->get().push_back( xNewColumn );
        }
        else
        {
            OUString aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,OUString(),OUString(),OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,_nType,false,false,isCaseSensitive(),
                OUString(),OUString(),OUString());
            pColumn->setFunction(true);
            pColumn->setAggregateFunction(bAggFkt);
            pColumn->setRealName(rColumnName);

            Reference< XPropertySet> xCol = pColumn;
            _rColumns->get().push_back(xCol);
        }
    }
    else    // ColumnName and TableName exist
    {
        OSQLTables::const_iterator aFind = m_pImpl->m_pTables->find(rTableRange);

        bool bError = false;
        if (aFind != m_pImpl->m_pTables->end() && aFind->second.is())
        {
            if (bFkt)
            {
                OUString aNewColName(getUniqueColumnName(rColumnAlias));

                OParseColumn* pColumn = new OParseColumn(aNewColName,OUString(),OUString(),OUString(),
                    ColumnValue::NULLABLE_UNKNOWN,0,0,_nType,false,false,isCaseSensitive(),
                    OUString(),OUString(),OUString());
                pColumn->setFunction(true);
                pColumn->setAggregateFunction(bAggFkt);
                pColumn->setRealName(rColumnName);
                SAL_WARN("connectivity.parse", "Trying to construct a column with Function==true and a TableName; this makes no sense.");
                assert(false);
                pColumn->setTableName(aFind->first);

                Reference< XPropertySet> xCol = pColumn;
                _rColumns->get().push_back(xCol);
            }
            else
            {
                Reference< XPropertySet > xColumn;
                if (aFind->second->getColumns()->hasByName(rColumnName) && (aFind->second->getColumns()->getByName(rColumnName) >>= xColumn))
                {
                    OUString aNewColName(getUniqueColumnName(rColumnAlias));

                    OParseColumn* pColumn = new OParseColumn(xColumn,isCaseSensitive());
                    pColumn->setName(aNewColName);
                    pColumn->setRealName(rColumnName);
                    pColumn->setTableName(aFind->first);

                    Reference< XPropertySet> xCol = pColumn;
                    _rColumns->get().push_back(xCol);
                }
                else
                    bError = true;
            }
        }
        else
            bError = true;

        // Table does not exist or lacking field
        if (bError)
        {
            OUString aNewColName(getUniqueColumnName(rColumnAlias));

            OParseColumn* pColumn = new OParseColumn(aNewColName,OUString(),OUString(),OUString(),
                ColumnValue::NULLABLE_UNKNOWN,0,0,DataType::VARCHAR,false,false,isCaseSensitive(),
                OUString(),OUString(),OUString());
            pColumn->setFunction(true);
            pColumn->setAggregateFunction(bAggFkt);

            Reference< XPropertySet> xCol = pColumn;
            _rColumns->get().push_back(xCol);
        }
    }
}

OUString OSQLParseTreeIterator::getUniqueColumnName(const OUString & rColumnName) const
{
    OUString aAlias(rColumnName);

    OSQLColumns::Vector::const_iterator aIter = find(
        m_aSelectColumns->get().begin(),
        m_aSelectColumns->get().end(),
        aAlias,
        ::comphelper::UStringMixEqual( isCaseSensitive() )
    );
    sal_Int32 i=1;
    while(aIter != m_aSelectColumns->get().end())
    {
        aAlias = rColumnName + OUString::number(i++);
        aIter = find(
            m_aSelectColumns->get().begin(),
            m_aSelectColumns->get().end(),
            aAlias,
            ::comphelper::UStringMixEqual( isCaseSensitive() )
        );
    }
    return aAlias;
}

void OSQLParseTreeIterator::setOrderByColumnName(const OUString & rColumnName, OUString & rTableRange, bool bAscending)
{
    Reference<XPropertySet> xColumn = findSelectColumn( rColumnName );
    if ( !xColumn.is() )
        xColumn = findColumn ( rColumnName, rTableRange, false );
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

void OSQLParseTreeIterator::setGroupByColumnName(const OUString & rColumnName, OUString & rTableRange)
{
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
    cout << "OSQLParseTreeIterator::setGroupByColumnName: "
         << (const char *) rColumnName << ", "
         << (const char *) rTableRange << ", "
         << (bAscending ? "true" : "false")
         << "\n";
#endif
}


const OSQLParseNode* OSQLParseTreeIterator::getWhereTree() const
{
    if (!m_pParseTree)
        return nullptr;

    // Analyse parse tree (depending on statement type)
    // and set pointer to WHERE clause:
    OSQLParseNode * pWhereClause = nullptr;
    if(getStatementType() == OSQLStatementType::Select)
    {
        OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
        OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
        OSL_ENSURE(pTableExp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
        OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

        pWhereClause = pTableExp->getChild(1);
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched) ||
             SQL_ISRULE(m_pParseTree,delete_statement_searched))
    {
        pWhereClause = m_pParseTree->getChild(m_pParseTree->count()-1);
    }
    if(pWhereClause && pWhereClause->count() != 2)
        pWhereClause = nullptr;
    return pWhereClause;
}


const OSQLParseNode* OSQLParseTreeIterator::getOrderTree() const
{
    if (!m_pParseTree || getStatementType() != OSQLStatementType::Select)
        return nullptr;

    // Analyse parse tree (depending on statement type)
    // and set pointer to ORDER clause:
    OSQLParseNode * pOrderClause = nullptr;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    pOrderClause = pTableExp->getChild(ORDER_BY_CHILD_POS);
    // If it is a order_by, it must not be empty
    if(pOrderClause->count() != 3)
        pOrderClause = nullptr;
    return pOrderClause;
}

const OSQLParseNode* OSQLParseTreeIterator::getGroupByTree() const
{
    if (!m_pParseTree || getStatementType() != OSQLStatementType::Select)
        return nullptr;

    // Analyse parse tree (depending on statement type)
    // and set pointer to ORDER clause:
    OSQLParseNode * pGroupClause = nullptr;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    pGroupClause = pTableExp->getChild(2);
    // If it is an order_by, it must not be empty
    if(pGroupClause->count() != 3)
        pGroupClause = nullptr;
    return pGroupClause;
}

const OSQLParseNode* OSQLParseTreeIterator::getHavingTree() const
{
    if (!m_pParseTree || getStatementType() != OSQLStatementType::Select)
        return nullptr;

    // Analyse parse tree (depending on statement type)
    // and set pointer to ORDER clause:
    OSQLParseNode * pHavingClause = nullptr;
    OSL_ENSURE(m_pParseTree->count() >= 4,"ParseTreeIterator: error in parse tree!");
    OSQLParseNode * pTableExp = m_pParseTree->getChild(3);
    OSL_ENSURE(pTableExp != nullptr,"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp),"OSQLParseTreeIterator: error in parse tree!");
    OSL_ENSURE(pTableExp->count() == TABLE_EXPRESSION_CHILD_COUNT,"OSQLParseTreeIterator: error in parse tree!");

    pHavingClause = pTableExp->getChild(3);
    // If it is an order_by, then it must not be empty
    if(pHavingClause->count() < 1)
        pHavingClause = nullptr;
    return pHavingClause;
}

bool OSQLParseTreeIterator::isTableNode(const OSQLParseNode* _pTableNode)
{
    return _pTableNode && (SQL_ISRULE(_pTableNode,catalog_name) ||
                           SQL_ISRULE(_pTableNode,schema_name)  ||
                           SQL_ISRULE(_pTableNode,table_name));
}

const OSQLParseNode* OSQLParseTreeIterator::getSimpleWhereTree() const
{
    const OSQLParseNode* pNode = getWhereTree();
    return pNode ? pNode->getChild(1) : nullptr;
}

const OSQLParseNode* OSQLParseTreeIterator::getSimpleOrderTree() const
{
    const OSQLParseNode* pNode = getOrderTree();
    return pNode ? pNode->getChild(2) : nullptr;
}

const OSQLParseNode* OSQLParseTreeIterator::getSimpleGroupByTree() const
{
    const OSQLParseNode* pNode = getGroupByTree();
    return pNode ? pNode->getChild(2) : nullptr;
}

const OSQLParseNode* OSQLParseTreeIterator::getSimpleHavingTree() const
{
    const OSQLParseNode* pNode = getHavingTree();
    return pNode ? pNode->getChild(1) : nullptr;
}


Reference< XPropertySet > OSQLParseTreeIterator::findSelectColumn( const OUString & rColumnName )
{
    for (auto const& lookupColumn : m_aSelectColumns->get())
    {
        Reference< XPropertySet > xColumn( lookupColumn );
        try
        {
            OUString sName;
            xColumn->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_NAME ) ) >>= sName;
            if ( sName == rColumnName )
                return xColumn;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("connectivity.parse");
        }
    }
    return nullptr;
}


Reference< XPropertySet > OSQLParseTreeIterator::findColumn( const OUString & rColumnName, OUString & rTableRange, bool _bLookInSubTables )
{
    Reference< XPropertySet > xColumn = findColumn( *m_pImpl->m_pTables, rColumnName, rTableRange );
    if ( !xColumn.is() && _bLookInSubTables )
        xColumn = findColumn( *m_pImpl->m_pSubTables, rColumnName, rTableRange );
    return xColumn;
}


Reference< XPropertySet > OSQLParseTreeIterator::findColumn(const OSQLTables& _rTables, const OUString & rColumnName, OUString & rTableRange)
{
    Reference< XPropertySet > xColumn;
    if ( !rTableRange.isEmpty() )
    {
        OSQLTables::const_iterator aFind = _rTables.find(rTableRange);

        if ( aFind != _rTables.end()
            && aFind->second.is()
            && aFind->second->getColumns().is()
            && aFind->second->getColumns()->hasByName(rColumnName) )
            aFind->second->getColumns()->getByName(rColumnName) >>= xColumn;
    }
    if ( !xColumn.is() )
    {
        for (auto const& table : _rTables)
        {
            if ( table.second.is() )
            {
                Reference<XNameAccess> xColumns = table.second->getColumns();
                if( xColumns.is() && xColumns->hasByName(rColumnName) && (xColumns->getByName(rColumnName) >>= xColumn) )
                {
                    OSL_ENSURE(xColumn.is(),"Column isn't a propertyset!");
                    // Cannot take "rTableRange = table.first" because that is the fully composed name
                    // that is, catalogName.schemaName.tableName
                    rTableRange = getString(xColumn->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TABLENAME)));
                    break; // This column must only exits once
                }
            }
        }
    }
    return xColumn;
}


void OSQLParseTreeIterator::impl_appendError( IParseContext::ErrorCode _eError, const OUString* _pReplaceToken1, const OUString* _pReplaceToken2 )
{
    OUString sErrorMessage = m_rParser.getContext().getErrorMessage( _eError );
    if ( _pReplaceToken1 )
    {
        bool bTwoTokens = ( _pReplaceToken2 != nullptr );
        const sal_Char* pPlaceHolder1 = bTwoTokens ? "#1" : "#";
        const OUString sPlaceHolder1 = OUString::createFromAscii( pPlaceHolder1 );

        sErrorMessage = sErrorMessage.replaceFirst( sPlaceHolder1, *_pReplaceToken1 );
        if ( _pReplaceToken2 )
            sErrorMessage = sErrorMessage.replaceFirst( "#2" , *_pReplaceToken2 );
    }

    impl_appendError( SQLException(
        sErrorMessage, nullptr, getStandardSQLState( StandardSQLState::GENERAL_ERROR ), 1000, Any() ) );
}


void OSQLParseTreeIterator::impl_appendError( const SQLException& _rError )
{
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

sal_Int32 OSQLParseTreeIterator::getFunctionReturnType(const OSQLParseNode* _pNode )
{
    sal_Int32 nType = DataType::OTHER;
    OUString sFunctionName;
    if ( SQL_ISRULE(_pNode,length_exp) )
    {
        _pNode->getChild(0)->getChild(0)->parseNodeToStr(sFunctionName, m_pImpl->m_xConnection, nullptr, false, false );
        nType = ::connectivity::OSQLParser::getFunctionReturnType( sFunctionName, &m_rParser.getContext() );
    }
    else if ( SQL_ISRULE(_pNode,num_value_exp) || SQL_ISRULE(_pNode,term) || SQL_ISRULE(_pNode,factor) )
    {
        nType = DataType::DOUBLE;
    }
    else
    {
        _pNode->getChild(0)->parseNodeToStr(sFunctionName, m_pImpl->m_xConnection, nullptr, false, false );

        // MIN and MAX have another return type, we have to check the expression itself.
        // @see http://qa.openoffice.org/issues/show_bug.cgi?id=99566
        if ( SQL_ISRULE(_pNode,general_set_fct) && (SQL_ISTOKEN(_pNode->getChild(0),MIN) || SQL_ISTOKEN(_pNode->getChild(0),MAX) ))
        {
            const OSQLParseNode* pValueExp = _pNode->getChild(3);
            if (SQL_ISRULE(pValueExp,column_ref))
            {
                OUString sColumnName;
                OUString aTableRange;
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
