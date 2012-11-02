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

#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include "propertyids.hxx"
#include "NStatement.hxx"
#include "NConnection.hxx"
#include "NDatabaseMetaData.hxx"
#include "NResultSet.hxx"
#include "resource/evoab2_res.hrc"
#include <resource/common_res.hrc>
#include <connectivity/dbexception.hxx>
#include <tools/diagnose_ex.h>

namespace connectivity { namespace evoab {

//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
//------------------------------------------------------------------------------
OCommonStatement::OCommonStatement(OEvoabConnection* _pConnection)
    : OCommonStatement_IBase(m_aMutex)
    , ::comphelper::OPropertyContainer(OCommonStatement_IBase::rBHelper)
    , OStatement_CBase( (::cppu::OWeakObject*)_pConnection, this )
    , m_xResultSet(NULL)
    , m_pResultSet(NULL)
    , m_pConnection(_pConnection)
    , m_aParser(_pConnection->getDriver().getMSFactory())
    , m_aSQLIterator( _pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL )
    , m_pParseTree(NULL)
    , m_nMaxFieldSize(0)
    , m_nMaxRows(0)
    , m_nQueryTimeOut(0)
    , m_nFetchSize(0)
    , m_nResultSetType(ResultSetType::FORWARD_ONLY)
    , m_nFetchDirection(FetchDirection::FORWARD)
    , m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
    , m_bEscapeProcessing(sal_True)
    , rBHelper(OCommonStatement_IBase::rBHelper)
{
    m_pConnection->acquire();

#define REGISTER_PROP( id, member ) \
    registerProperty( \
        OMetaConnection::getPropMap().getNameByIndex( id ), \
        id, \
        0, \
        &member, \
        ::getCppuType( &member ) \
    );

    REGISTER_PROP( PROPERTY_ID_CURSORNAME, m_aCursorName );
    REGISTER_PROP( PROPERTY_ID_MAXFIELDSIZE, m_nMaxFieldSize );
    REGISTER_PROP( PROPERTY_ID_MAXROWS, m_nMaxRows );
    REGISTER_PROP( PROPERTY_ID_QUERYTIMEOUT, m_nQueryTimeOut );
    REGISTER_PROP( PROPERTY_ID_FETCHSIZE, m_nFetchSize );
    REGISTER_PROP( PROPERTY_ID_RESULTSETTYPE, m_nResultSetType );
    REGISTER_PROP( PROPERTY_ID_FETCHDIRECTION, m_nFetchDirection );
    REGISTER_PROP( PROPERTY_ID_ESCAPEPROCESSING, m_bEscapeProcessing );
    REGISTER_PROP( PROPERTY_ID_RESULTSETCONCURRENCY, m_nResultSetConcurrency );
}
// -----------------------------------------------------------------------------
OCommonStatement::~OCommonStatement()
{
}
//------------------------------------------------------------------------------
void OCommonStatement::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}
//------------------------------------------------------------------------------
void OCommonStatement::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

    dispose_ChildImpl();
    OCommonStatement_IBase::disposing();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OCommonStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OCommonStatement_IBase::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::comphelper::OPropertyContainer::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OCommonStatement::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OCommonStatement_IBase::getTypes());
}
// -------------------------------------------------------------------------

//void SAL_CALL OCommonStatement::cancel(  ) throw(RuntimeException)
//{
//::osl::MutexGuard aGuard( m_aMutex );
//checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);
//// cancel the current sql statement
//}

// -------------------------------------------------------------------------
void SAL_CALL OCommonStatement::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

EBookQuery *
OCommonStatement::createTrue()
{ // Not the world's most efficient unconditional true but ...
    return e_book_query_from_string("(exists \"full_name\")");
}

EBookQuery *
OCommonStatement::createTest( const ::rtl::OUString &aColumnName,
                             EBookQueryTest eTest,
                             const ::rtl::OUString &aMatch )
{
    ::rtl::OString sMatch = rtl::OUStringToOString( aMatch, RTL_TEXTENCODING_UTF8 );
    ::rtl::OString sColumnName = rtl::OUStringToOString( aColumnName, RTL_TEXTENCODING_UTF8 );

    return e_book_query_field_test( e_contact_field_id( sColumnName.getStr() ),
                                    eTest, sMatch.getStr() );
}

// -------------------------------------------------------------------------

::rtl::OUString OCommonStatement::impl_getColumnRefColumnName_throw( const OSQLParseNode& _rColumnRef )
{
    ENSURE_OR_THROW( SQL_ISRULE( &_rColumnRef, column_ref ), "internal error: only column_refs supported as LHS" );

    ::rtl::OUString sColumnName;
    switch ( _rColumnRef.count() )
    {
    case 3: // SQL_TOKEN_NAME '.' column_val
    {
        const OSQLParseNode* pPunct = _rColumnRef.getChild( 1 );
        const OSQLParseNode* pColVal = _rColumnRef.getChild( 2 );
        if  (   SQL_ISPUNCTUATION( pPunct, "." )
            &&  ( pColVal->count() == 1 )
            )
        {
            sColumnName = pColVal->getChild( 0 )->getTokenValue();
        }
    }
    break;

    case 1: // column
    {
        sColumnName = _rColumnRef.getChild( 0 )->getTokenValue();
    }
    break;
    }

    if ( !sColumnName.getLength() )
        m_pConnection->throwGenericSQLException( STR_QUERY_TOO_COMPLEX, *this );

    return sColumnName;
}

// -------------------------------------------------------------------------
void OCommonStatement::orderByAnalysis( const OSQLParseNode* _pOrderByClause, SortDescriptor& _out_rSort )
{
    ENSURE_OR_THROW( _pOrderByClause, "NULL node" );
    ENSURE_OR_THROW( SQL_ISRULE( _pOrderByClause, opt_order_by_clause ), "wrong node type" );

    _out_rSort.clear();

    const OSQLParseNode* pOrderList = _pOrderByClause->getByRule( OSQLParseNode::ordering_spec_commalist );
    ENSURE_OR_THROW( pOrderList, "unexpected parse tree structure" );

    for ( sal_uInt32 i=0; i<pOrderList->count(); ++i )
    {
        const OSQLParseNode* pOrderBy = pOrderList->getChild(i);
        if ( !pOrderBy || !SQL_ISRULE( pOrderBy, ordering_spec ) )
            continue;
        const OSQLParseNode* pColumnRef = pOrderBy->count() == 2 ? pOrderBy->getChild(0) : NULL;
        const OSQLParseNode* pAscDesc = pOrderBy->count() == 2 ? pOrderBy->getChild(1) : NULL;
        ENSURE_OR_THROW(
                ( pColumnRef != NULL )
            &&  ( pAscDesc != NULL )
            &&  SQL_ISRULE( pAscDesc, opt_asc_desc )
            &&  ( pAscDesc->count() < 2 ),
            "ordering_spec structure error" );

        // column name -> column field
        if ( !SQL_ISRULE( pColumnRef, column_ref ) )
            m_pConnection->throwGenericSQLException( STR_SORT_BY_COL_ONLY, *this );
        const ::rtl::OUString sColumnName( impl_getColumnRefColumnName_throw( *pColumnRef ) );
        guint nField = evoab::findEvoabField( sColumnName );
        // ascending/descending?
        bool bAscending = true;
        if  (   ( pAscDesc->count() == 1 )
            &&  SQL_ISTOKEN( pAscDesc->getChild( 0 ), DESC )
            )
            bAscending = false;

        _out_rSort.push_back( FieldSort( nField, bAscending ) );
    }
}

// -------------------------------------------------------------------------
EBookQuery *OCommonStatement::whereAnalysis( const OSQLParseNode* parseTree )
{
    EBookQuery *pResult = NULL;

    ENSURE_OR_THROW( parseTree, "invalid parse tree" );

    // Nested brackets
    if( parseTree->count() == 3 &&
        SQL_ISPUNCTUATION( parseTree->getChild( 0 ), "(" ) &&
        SQL_ISPUNCTUATION( parseTree->getChild( 2 ), ")" ) )
    {
        pResult = whereAnalysis( parseTree->getChild( 1 ) );
    }

    // SQL AND, OR
    else if( ( SQL_ISRULE( parseTree, search_condition ) ||
          SQL_ISRULE( parseTree, boolean_term ) ) &&
        parseTree->count() == 3 )
    {
        ENSURE_OR_THROW(    SQL_ISTOKEN( parseTree->getChild( 1 ), OR )
                        ||  SQL_ISTOKEN( parseTree->getChild( 1 ), AND ),
                        "unexpected search_condition structure" );

        EBookQuery *pArgs[2];
        pArgs[0] = whereAnalysis( parseTree->getChild( 0 ) );
        pArgs[1] = whereAnalysis( parseTree->getChild( 2 ) );

        if( SQL_ISTOKEN( parseTree->getChild( 1 ), OR ) )
            pResult = e_book_query_or( 2, pArgs, TRUE );
        else
            pResult = e_book_query_and( 2, pArgs, TRUE );
    }
    // SQL =, !=
    else if( SQL_ISRULE( parseTree, comparison_predicate ) )
    {
        OSQLParseNode *pPrec = parseTree->getChild( 1 );

        ENSURE_OR_THROW( parseTree->count() == 3, "unexpected comparison_predicate structure" );

        OSQLParseNode* pLHS = parseTree->getChild( 0 );
        OSQLParseNode* pRHS = parseTree->getChild( 2 );

        if  (   (   !( SQL_ISRULE( pLHS, column_ref ) )         // on the LHS, we accept a column or a constant int value
                &&  ( pLHS->getNodeType() != SQL_NODE_INTNUM )
                )
            ||  (   ( pRHS->getNodeType() != SQL_NODE_STRING )  // on the RHS, certain literals are acceptable
                &&  ( pRHS->getNodeType() != SQL_NODE_INTNUM )
                &&  ( pRHS->getNodeType() != SQL_NODE_APPROXNUM )
                &&  !( SQL_ISTOKEN( pRHS, TRUE ) )
                &&  !( SQL_ISTOKEN( pRHS, FALSE ) )
                )
            ||  (   ( pLHS->getNodeType() == SQL_NODE_INTNUM )  // an int on LHS requires an int on RHS
                &&  ( pRHS->getNodeType() != SQL_NODE_INTNUM )
                )
            )
        {
            m_pConnection->throwGenericSQLException( STR_QUERY_TOO_COMPLEX, *this );
        }

        if  (   ( pPrec->getNodeType() != SQL_NODE_EQUAL )
            &&  ( pPrec->getNodeType() != SQL_NODE_NOTEQUAL )
            )
        {
            m_pConnection->throwGenericSQLException( STR_OPERATOR_TOO_COMPLEX, *this );
        }

        // recognize the special "0 = 1" condition
        if  (   ( pLHS->getNodeType() == SQL_NODE_INTNUM )
            &&  ( pRHS->getNodeType() == SQL_NODE_INTNUM )
            &&  ( pPrec->getNodeType() == SQL_NODE_EQUAL )
            )
        {
            const sal_Int32 nLHS = pLHS->getTokenValue().toInt64();
            const sal_Int32 nRHS = pRHS->getTokenValue().toInt64();
            return ( nLHS == nRHS ) ? createTrue() : NULL;
        }

        ::rtl::OUString aColumnName( impl_getColumnRefColumnName_throw( *pLHS ) );

        ::rtl::OUString aMatchString;
        if ( pRHS->isToken() )
            aMatchString = pRHS->getTokenValue();
        else
            aMatchString = pRHS->getChild( 0 )->getTokenValue();

        pResult = createTest( aColumnName, E_BOOK_QUERY_IS, aMatchString );

        if ( pResult && ( pPrec->getNodeType() == SQL_NODE_NOTEQUAL ) )
            pResult = e_book_query_not( pResult, TRUE );
    }
    // SQL like
    else if( SQL_ISRULE( parseTree, like_predicate ) )
    {
        ENSURE_OR_THROW( parseTree->count() == 2, "unexpected like_predicate structure" );
        const OSQLParseNode* pPart2 = parseTree->getChild(1);

        if( ! SQL_ISRULE( parseTree->getChild( 0 ), column_ref) )
            m_pConnection->throwGenericSQLException(STR_QUERY_INVALID_LIKE_COLUMN,*this);

        ::rtl::OUString aColumnName( impl_getColumnRefColumnName_throw( *parseTree->getChild( 0 ) ) );

        OSQLParseNode *pAtom      = pPart2->getChild( pPart2->count() - 2 );     // Match String
        bool bNotLike             = pPart2->getChild(0)->isToken();

        if( !( pAtom->getNodeType() == SQL_NODE_STRING ||
               pAtom->getNodeType() == SQL_NODE_NAME ||
               SQL_ISRULE( pAtom,parameter ) ||
               ( pAtom->getChild( 0 ) && pAtom->getChild( 0 )->getNodeType() == SQL_NODE_NAME ) ||
               ( pAtom->getChild( 0 ) && pAtom->getChild( 0 )->getNodeType() == SQL_NODE_STRING ) ) )
        {
            OSL_TRACE( "analyseSQL : pAtom->count() = %d", pAtom->count() );
            m_pConnection->throwGenericSQLException(STR_QUERY_INVALID_LIKE_STRING,*this);
        }

        const sal_Unicode WILDCARD = '%';

        rtl::OUString aMatchString;
        aMatchString = pAtom->getTokenValue();

        // Determine where '%' character is...
        if( aMatchString.equals( ::rtl::OUString::valueOf( WILDCARD ) ) )
        {
            // String containing only a '%' and nothing else matches everything
            pResult = createTest( aColumnName, E_BOOK_QUERY_CONTAINS,
                                  rtl::OUString("") );
        }
        else if( aMatchString.indexOf( WILDCARD ) == -1 )
        {   // Simple string , eg. "to match" "contains in evo"
            SAL_INFO( "evoab2", "Plain contains '" << aMatchString << "'" );
            pResult = createTest( aColumnName, E_BOOK_QUERY_CONTAINS, aMatchString );
            if( pResult && bNotLike )
                pResult = e_book_query_not( pResult, TRUE );
        }
        else if( bNotLike )
        {
            // We currently can't handle a 'NOT LIKE' when there are '%'
            m_pConnection->throwGenericSQLException(STR_QUERY_NOT_LIKE_TOO_COMPLEX,*this);
        }
        else if( (aMatchString.indexOf ( WILDCARD ) == aMatchString.lastIndexOf ( WILDCARD ) ) )
        {   // One occurrence of '%'  matches...
            if ( aMatchString.indexOf ( WILDCARD ) == 0 )
                pResult = createTest( aColumnName, E_BOOK_QUERY_ENDS_WITH, aMatchString.copy( 1 ) );
            else if ( aMatchString.indexOf ( WILDCARD ) == aMatchString.getLength() - 1 )
                pResult = createTest( aColumnName, E_BOOK_QUERY_BEGINS_WITH, aMatchString.copy( 0, aMatchString.getLength() - 1 ) );
            else
                m_pConnection->throwGenericSQLException(STR_QUERY_LIKE_WILDCARD,*this);

            if( pResult && bNotLike )
                pResult = e_book_query_not( pResult, TRUE );
        }
        else if( aMatchString.getLength() >= 3 &&
                 aMatchString.indexOf ( WILDCARD ) == 0 &&
                 aMatchString.indexOf ( WILDCARD, 1) == aMatchString.getLength() - 1 ) {
            // one '%' at the start and another at the end
            pResult = createTest( aColumnName, E_BOOK_QUERY_CONTAINS, aMatchString.copy (1, aMatchString.getLength() - 2) );
        }
        else
            m_pConnection->throwGenericSQLException(STR_QUERY_LIKE_WILDCARD_MANY,*this);
    }

    return pResult;
}

rtl::OUString OCommonStatement::getTableName()
{
    ::rtl::OUString aTableName;

    if( m_pParseTree && m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT )
    {
        Any aCatalog;
        ::rtl::OUString aSchema;
        const OSQLParseNode *pSelectStmnt = m_aSQLIterator.getParseTree();
        const OSQLParseNode *pAllTableNames = pSelectStmnt->getChild( 3 )->getChild( 0 )->getChild( 1 );

        if( m_aSQLIterator.isTableNode( pAllTableNames->getChild( 0 ) ) )
            OSQLParseNode::getTableComponents( pAllTableNames->getChild( 0 ),
                                               aCatalog,aSchema, aTableName,NULL );

        else if( SQL_ISRULE( pAllTableNames->getChild( 0 ), table_ref ) )
        {
            OSQLParseNode *pNodeForTableName = pAllTableNames->getChild( 0 )->getChild( 0 );
            if( m_aSQLIterator.isTableNode( pNodeForTableName ) )
            {
                aTableName = OSQLParseNode::getTableRange(pAllTableNames->getChild( 0 ));
                if( !aTableName.getLength() )
                    OSQLParseNode::getTableComponents( pNodeForTableName, aCatalog, aSchema, aTableName,NULL);
            }
            else
                OSL_FAIL( "odd table layout" );
        }
        else
                OSL_FAIL( "unusual table layout" );
    }
    return aTableName;
}

void OCommonStatement::parseSql( const rtl::OUString& sql, QueryData& _out_rQueryData )
{
    SAL_INFO( "evoab2", "parsing " << sql );

    _out_rQueryData.eFilterType = eFilterOther;

    ::rtl::OUString aErr;
    m_pParseTree = m_aParser.parseTree( aErr, sql );
    m_aSQLIterator.setParseTree( m_pParseTree );
    m_aSQLIterator.traverseAll();

    _out_rQueryData.sTable = getTableName();

    // to be sorted?
    const OSQLParseNode* pOrderByClause = m_aSQLIterator.getOrderTree();
    if ( pOrderByClause )
    {
    #if OSL_DEBUG_LEVEL > 1
        ::rtl::OUString sTreeDebug;
        pOrderByClause->showParseTree( sTreeDebug );
        SAL_INFO( "evoab2", "found order-by tree:\n" << sTreeDebug );
    #endif
        orderByAnalysis( pOrderByClause, _out_rQueryData.aSortOrder );
    }

    const OSQLParseNode* pWhereClause = m_aSQLIterator.getWhereTree();
    if ( pWhereClause && SQL_ISRULE( pWhereClause, where_clause ) )
    {
    #if OSL_DEBUG_LEVEL > 1
        ::rtl::OUString sTreeDebug;
        pWhereClause->showParseTree( sTreeDebug );
        SAL_INFO( "evoab2", "found where tree:\n" << sTreeDebug );
    #endif
        EBookQuery* pQuery = whereAnalysis( pWhereClause->getChild( 1 ) );
        if ( !pQuery )
        {
            _out_rQueryData.eFilterType = eFilterAlwaysFalse;
            pQuery = createTrue();
        }
        _out_rQueryData.setQuery( pQuery );
    }
    else
    {
        _out_rQueryData.eFilterType = eFilterNone;
        _out_rQueryData.setQuery( createTrue() );
    }
}

// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    // just return our connection here
    return impl_getConnection();
}

// -------------------------------------------------------------------------
Any SAL_CALL OCommonStatement::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);


    return makeAny(SQLWarning());
}

// -------------------------------------------------------------------------
void SAL_CALL OCommonStatement::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OCommonStatement::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties( aProps );
    return new ::cppu::OPropertyArrayHelper( aProps );
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OCommonStatement::getInfoHelper()
{
    return *const_cast< OCommonStatement* >( this )->getArrayHelper();
}

// -----------------------------------------------------------------------------
void SAL_CALL OCommonStatement::acquire() throw()
{
    OCommonStatement_IBase::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OCommonStatement::release() throw()
{
    relase_ChildImpl();
}

// -------------------------------------------------------------------------
QueryData OCommonStatement::impl_getEBookQuery_throw( const ::rtl::OUString& _rSql )
{
    QueryData aData;
    parseSql( _rSql, aData );

#if OSL_DEBUG_LEVEL > 1
    char *pSexpr = aData.getQuery() ? e_book_query_to_string( aData.getQuery() ) : g_strdup( "<map failed>" );
    g_message( "Parsed SQL to sexpr '%s'\n", pSexpr );
    g_free( pSexpr );
#endif

    if ( !aData.getQuery() )
        m_pConnection->throwGenericSQLException( STR_QUERY_TOO_COMPLEX, *this );

    // a postcondition of this method is that we properly determined the SELECT columns
    aData.xSelectColumns = m_aSQLIterator.getSelectColumns();
    if ( !aData.xSelectColumns.is() )
        m_pConnection->throwGenericSQLException( STR_QUERY_TOO_COMPLEX, *this );

    return aData;
}

// -------------------------------------------------------------------------
Reference< XResultSet > OCommonStatement::impl_executeQuery_throw( const QueryData& _rQueryData )
{
    // create result set
    OEvoabResultSet* pResult = new OEvoabResultSet( this, m_pConnection );
    Reference< XResultSet > xRS = pResult;
    pResult->construct( _rQueryData );

    // done
    m_xResultSet = xRS;
    return xRS;
}

// -------------------------------------------------------------------------
Reference< XResultSet > OCommonStatement::impl_executeQuery_throw( const ::rtl::OUString& _rSql )
{
    SAL_INFO( "evoab2", "OCommonStatement::impl_executeQuery_throw(" << _rSql << "%s)\n" );

#if OSL_DEBUG_LEVEL > 1
    g_message( "Parse SQL '%s'\n",
               OUStringToOString(_rSql, RTL_TEXTENCODING_UTF8).getStr() );
#endif

    return impl_executeQuery_throw( impl_getEBookQuery_throw( _rSql ) );
}

// -----------------------------------------------------------------------------
Reference< XPropertySetInfo > SAL_CALL OCommonStatement::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo( getInfoHelper() );
}

// =============================================================================
// = OStatement
// =============================================================================
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO( OStatement, "com.sun.star.comp.sdbcx.evoab.OStatement", "com.sun.star.sdbc.Statement" );

// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( OStatement, OCommonStatement, OStatement_IBase )

// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( OStatement, OCommonStatement, OStatement_IBase )

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStatement::execute( const ::rtl::OUString& _sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    Reference< XResultSet > xRS = impl_executeQuery_throw( _sql );
    return xRS.is();
}

// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatement::executeQuery( const ::rtl::OUString& _sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);

    return impl_executeQuery_throw( _sql );
}

// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatement::executeUpdate( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBase::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XStatement::executeUpdate", *this );
    return 0;
}

} } // namespace ::connectivity::evoab

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
