 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: NStatement.cxx,v $
 * $Revision: 1.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#include <stdio.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include "propertyids.hxx"
#include "NStatement.hxx"
#include "NConnection.hxx"
#include "NResultSet.hxx"
#include "NDebug.hxx"
#include <connectivity/dbexception.hxx>

using namespace connectivity::evoab;
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
OStatement_Base::OStatement_Base(OEvoabConnection* _pConnection)
    : OStatement_BASE(m_aMutex)
    , OPropertySetHelper(OStatement_BASE::rBHelper)
    , m_xResultSet(NULL)
    , m_pResultSet(NULL)
    , m_pConnection(_pConnection)
    , m_aParser(_pConnection->getDriver()->getMSFactory())
    , m_aSQLIterator( _pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL )
    , m_pParseTree(NULL)
    , rBHelper(OStatement_BASE::rBHelper)
{
    m_pConnection->acquire();
}
// -----------------------------------------------------------------------------
OStatement_Base::~OStatement_Base()
{
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    // free the cursor if alive
    Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = Reference< XResultSet>();
}
//------------------------------------------------------------------------------
void OStatement_BASE2::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    disposeResultSet();

    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

    dispose_ChildImpl();
    OStatement_Base::disposing();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw()
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

//void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
//{
//::osl::MutexGuard aGuard( m_aMutex );
//checkDisposed(OStatement_BASE::rBHelper.bDisposed);
//// cancel the current sql statement
//}

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

void OStatement_Base::reset() throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    clearWarnings ();

    if (m_xResultSet.get().is())
        clearMyResultSet();
}

void OStatement_Base::clearMyResultSet () throw (SQLException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    try
    {
        Reference<XCloseable> xCloseable;
        if ( ::comphelper::query_interface( m_xResultSet.get(), xCloseable ) )
            xCloseable->close();
    }
    catch( const DisposedException& ) { }

    m_xResultSet = Reference< XResultSet >();
}

EBookQuery *
OStatement_Base::createTrue()
{ // Not the world's most efficient unconditional true but ...
    return e_book_query_from_string("(exists \"full_name\")");
}

EBookQuery *
OStatement_Base::createTest( const ::rtl::OUString &aColumnName,
                             EBookQueryTest eTest,
                             const ::rtl::OUString &aMatch,
                             bool bGeneric )
{
    rtl::OString sMatch = rtl::OUStringToOString( aMatch, RTL_TEXTENCODING_UTF8 );
    rtl::OString sColumnName = rtl::OUStringToOString( aColumnName, RTL_TEXTENCODING_UTF8 );
    if ( bGeneric && (aColumnName.equalsAscii("0") ||
                        aColumnName.equalsAscii("1")))
    {
            if( aMatch.equals( aColumnName ) )
                return createTrue();
            else
                return NULL;
    }

    return e_book_query_field_test( e_contact_field_id( sColumnName ),
                                    eTest, sMatch );
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    executeQuery( sql );
    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT;
}

EBookQuery *OStatement_Base::whereAnalysis( const OSQLParseNode* parseTree ) throw(SQLException, RuntimeException)
{
    EBookQuery *pResult = NULL;

    if (!parseTree)
            return NULL;

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
        EBookQuery *pArgs[2];
        pArgs[0] = whereAnalysis( parseTree->getChild( 0 ) );
        pArgs[1] = whereAnalysis( parseTree->getChild( 2 ) );

        if( SQL_ISTOKEN( parseTree->getChild( 1 ), OR ) )
                pResult = e_book_query_or( 2, pArgs, TRUE );
        else if( SQL_ISTOKEN( parseTree->getChild( 1 ), AND ) )
                pResult = e_book_query_and( 2, pArgs, TRUE );
        else
        {
            OSL_ASSERT("analyseSQL: Error in Parse Tree");
            e_book_query_unref( pArgs[ 0 ] );
            e_book_query_unref( pArgs[ 1 ] );
            return NULL;
        }
    }
    // SQL =, !=
    else if( SQL_ISRULE( parseTree, comparison_predicate ) )
    {
        OSQLParseNode *pPrec = parseTree->getChild( 1 );

        if( parseTree->count() != 3 )
        {
            OSL_ASSERT("analyseSQL: Error in Parse Tree");
            return NULL;
        }

        if( !( SQL_ISRULE( parseTree->getChild( 0 ), column_ref) ||
               parseTree->getChild( 2 )->getNodeType() == SQL_NODE_STRING ||
               parseTree->getChild( 2 )->getNodeType() == SQL_NODE_INTNUM ||
               parseTree->getChild( 2 )->getNodeType() == SQL_NODE_APPROXNUM ||
               SQL_ISTOKEN( parseTree->getChild( 2 ), TRUE ) ||
               SQL_ISTOKEN( parseTree->getChild( 2 ), FALSE ) ||
               SQL_ISRULE(  parseTree->getChild( 2 ), parameter ) ||
               // odbc date
               (SQL_ISRULE( parseTree->getChild( 2 ), set_fct_spec ) &&
                SQL_ISPUNCTUATION( parseTree->getChild( 2 )->getChild( 0 ), "{" ) ) ) )
            ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii( "Query too complex" ), NULL);

        if (pPrec->getNodeType() != SQL_NODE_EQUAL &&
            pPrec->getNodeType() != SQL_NODE_NOTEQUAL)
            ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii( "Operator too complex" ), NULL);

        rtl::OUString aMatchString;
        rtl::OUString aColumnName;
        if( SQL_ISRULE( parseTree->getChild( 0 ), column_ref ) )
        {
            aColumnName = parseTree->getChild( 0 )->getChild( 0 )->getTokenValue();

            if( parseTree->getChild( 2 )->isToken() )
                aMatchString = parseTree->getChild( 2 )->getTokenValue();
            else
                aMatchString = parseTree->getChild( 2 )->getChild( 0 )->getTokenValue();
        }
        else
        {
            aColumnName = parseTree->getChild( 0 )->getTokenValue();
            aMatchString = parseTree->getChild( 2 )->getTokenValue();
        }

        pResult = createTest( aColumnName, E_BOOK_QUERY_IS, aMatchString, true );

        if (pResult && pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
            pResult = e_book_query_not( pResult, TRUE );
    }
    // SQL like
    else if( SQL_ISRULE( parseTree, like_predicate ) )
    {
        if( parseTree->count() < 4 )
        {
            OSL_ASSERT( "analyseSQL: Error in parsing LIKE predicate" );
            return NULL;
        }

        if( ! SQL_ISRULE( parseTree->getChild( 0 ), column_ref) )
            ::dbtools::throwGenericSQLException(
                ::rtl::OUString::createFromAscii( "Invalid Statement - Not a Column"), NULL );

        OSQLParseNode *pColumn    = parseTree->getChild( 0 );                          // Match Item
        OSQLParseNode *pAtom      = parseTree->getChild( parseTree->count() - 2 );     // Match String
        bool bNotLike             = parseTree->count() == 5;

        if( !( pAtom->getNodeType() == SQL_NODE_STRING ||
               pAtom->getNodeType() == SQL_NODE_NAME ||
               SQL_ISRULE( pAtom,parameter ) ||
               ( pAtom->getChild( 0 ) && pAtom->getChild( 0 )->getNodeType() == SQL_NODE_NAME ) ||
               ( pAtom->getChild( 0 ) && pAtom->getChild( 0 )->getNodeType() == SQL_NODE_STRING ) ) )
        {
            OSL_TRACE( "analyseSQL : pAtom->count() = %d\n", pAtom->count() );
            ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii( "Invalid Statement - Not a String" ), NULL );
        }

        const sal_Unicode WILDCARD = '%';

        rtl::OUString aColumnName;
        rtl::OUString aMatchString;

        if( SQL_ISRULE( pColumn, column_ref ))
        {
            aColumnName = parseTree->getChild(0)->getChild(0)->getTokenValue();
            aMatchString = pAtom->getTokenValue();

            // Determine where '%' character is...
            if( aMatchString.equals( ::rtl::OUString::valueOf( WILDCARD ) ) )
            {
                // String containing only a '%' and nothing else matches everything
                pResult = createTest( aColumnName, E_BOOK_QUERY_CONTAINS,
                                      rtl::OUString::createFromAscii( "" ) );
            }
            else if( aMatchString.indexOf( WILDCARD ) == -1 )
            {   // Simple string , eg. "to match" "contains in evo"
                EVO_TRACE_STRING( "Plain contains '%s'", aMatchString );
                pResult = createTest( aColumnName, E_BOOK_QUERY_CONTAINS, aMatchString );
                if( pResult && bNotLike )
                    pResult = e_book_query_not( pResult, TRUE );
            }
            else if( bNotLike )
            {
                // We currently can't handle a 'NOT LIKE' when there are '%'
                ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii( "not like statement too complex" ), NULL );
            }
            else if( (aMatchString.indexOf ( WILDCARD ) == aMatchString.lastIndexOf ( WILDCARD ) ) )
            {   // One occurance of '%'  matches...
                if ( aMatchString.indexOf ( WILDCARD ) == 0 )
                    pResult = createTest( aColumnName, E_BOOK_QUERY_ENDS_WITH, aMatchString.copy( 1 ) );

                else if ( aMatchString.indexOf ( WILDCARD ) == aMatchString.getLength() - 1 )
                    pResult = createTest( aColumnName, E_BOOK_QUERY_BEGINS_WITH, aMatchString.copy( 0, aMatchString.getLength() - 1 ) );

                else
                    ::dbtools::throwGenericSQLException(
                        ::rtl::OUString::createFromAscii( "like statement contains wildcard in the middle" ), NULL );


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
                ::dbtools::throwGenericSQLException(
                    ::rtl::OUString::createFromAscii( "like statement contains too many wildcards" ), NULL );
        }
        else
                OSL_ASSERT( "Serious internal error" );
    }

    return pResult;
}

rtl::OUString OStatement_Base::getTableName()
{
    ::rtl::OUString aTableName;

    if( m_pParseTree && m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT )
    {
        Any aCatalog;
        ::rtl::OUString aSchema, aComposedName;
        const OSQLParseNode *pSelectStmnt = m_aSQLIterator.getParseTree();
        const OSQLParseNode *pAllTableNames = pSelectStmnt->getChild( 3 )->getChild( 0 )->getChild( 1 );

        if( m_aSQLIterator.isTableNode( pAllTableNames->getChild( 0 ) ) )
            OSQLParseNode::getTableComponents( pAllTableNames->getChild( 0 ),
                                               aCatalog,aSchema, aTableName );

        else if( SQL_ISRULE( pAllTableNames->getChild( 0 ), table_ref ) )
        {
            OSQLParseNode *pNodeForTableName = pAllTableNames->getChild( 0 )->getChild( 0 );
            if( m_aSQLIterator.isTableNode( pNodeForTableName ) )
            {
                if( pAllTableNames->getChild( 0 )->count() == 4 )
                    aTableName = pAllTableNames->getChild( 0 )->getChild( 2 )->getTokenValue();
                else
                    OSQLParseNode::getTableComponents( pNodeForTableName, aCatalog, aSchema, aTableName);
            }
            else
                OSL_ASSERT( "odd table layout" );
        }
        else
                OSL_ASSERT( "unusual table layout" );
    }
    return aTableName;
}

EBookQuery *OStatement_Base::parseSql( const rtl::OUString& sql,
                                       rtl::OString &rTable,
                                       bool &bIsWithoutWhere )
    throw ( SQLException, RuntimeException )
{
    ::rtl::OUString aErr;

    m_pParseTree = m_aParser.parseTree( aErr, sql );
    m_aSQLIterator.setParseTree( m_pParseTree );
    m_aSQLIterator.traverseAll();

    const OSQLTables aTables = m_aSQLIterator.getTables();
    rTable = rtl::OUStringToOString( getTableName(), RTL_TEXTENCODING_UTF8 );
    m_pConnection->setCurrentTableName(getTableName());
    const OSQLParseNode*  pParseTree = m_aSQLIterator.getWhereTree();
    if ( pParseTree && SQL_ISRULE( pParseTree, where_clause ) )
        return whereAnalysis( pParseTree->getChild( 1 ) );
    else
    {
        bIsWithoutWhere = true;
        return createTrue();
    }
}

// -------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    EVO_TRACE_STRING( "NConnection::executeQuery(%s)\n", sql );

    OEvoabResultSet* pResult = createResultSet();
    Reference< XResultSet > xRS = pResult;

#ifdef DEBUG
    g_message( "Parse SQL '%s'\n",
               (const sal_Char *)OUStringToOString( sql, RTL_TEXTENCODING_UTF8 ) );
#endif

    rtl::OString aTable;
    bool bIsWithoutWhere = false;

    EBookQuery *pQuery = parseSql( sql, aTable, bIsWithoutWhere );

#ifdef DEBUG
    char *pSexpr = pQuery ? e_book_query_to_string( pQuery ) : g_strdup( "<map failed>" );
    g_message( "Parsed SQL to sexpr '%s'\n", pSexpr );
    g_free( pSexpr );
#endif
     ::vos::ORef<connectivity::OSQLColumns> xColumns;
    if (pQuery)
    {
        pResult->construct( pQuery, aTable, bIsWithoutWhere );
        e_book_query_unref( pQuery );
        xColumns = m_aSQLIterator.getSelectColumns();
        if (!xColumns.isValid())
        {
            ::dbtools::throwGenericSQLException(
                ::rtl::OUString::createFromAscii("Invalid selection of columns"),
                NULL);
        }
        OEvoabResultSetMetaData *pMeta = (OEvoabResultSetMetaData *) pResult->getMetaData().get();
        pMeta->setEvoabFields(xColumns);
    }
    m_xResultSet = xRS;

    return xRS;
}
// -------------------------------------------------------------------------
OEvoabResultSet* OStatement_Base::createResultSet()
{
    return new OEvoabResultSet(this,m_pConnection,m_aSQLIterator);
}

// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // just return our connection here
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------
Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XServiceInfo*> (this));
    if(!aRet.hasValue())
        aRet = OStatement_Base::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatement_Base::executeUpdate( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XStatement::executeUpdate", *this );
    // the return values gives information about how many rows are affected by executing the sql statement
    return 0;
}

// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    return makeAny(SQLWarning());
}

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  ::rtl::OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::convertFastPropertyValue(
                            Any & /*rConvertedValue*/,
                            Any & /*rOldValue*/,
                            sal_Int32 /*nHandle*/,
                            const Any& /*rValue*/ )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bModified = sal_False;
    // here we have to try to convert
    return bModified;
}
// -------------------------------------------------------------------------
void OStatement_Base::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& /*rValue*/) throw (Exception)
{
    // set the value to what ever is nescessary
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OStatement_Base::getFastPropertyValue(Any& /*rValue*/,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::release() throw()
{
    OStatement_BASE::release();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------

