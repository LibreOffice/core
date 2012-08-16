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
#include <comphelper/property.hxx>
#include <comphelper/uno3.hxx>
#include <osl/thread.h>
#include <tools/diagnose_ex.h>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <algorithm>

#include "diagnose_ex.h"
#include "MDriver.hxx"
#include "MStatement.hxx"
#include "MConnection.hxx"
#include "MResultSet.hxx"
#include "MDatabaseMetaData.hxx"
#include "resource/mozab_res.hrc"
#include "resource/common_res.hrc"

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( ::rtl::OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

static ::osl::Mutex m_ThreadMutex;

using namespace ::comphelper;
using namespace connectivity::mozab;
using namespace connectivity;
//------------------------------------------------------------------------------
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;
//------------------------------------------------------------------------------
OCommonStatement::OCommonStatement(OConnection* _pConnection )
    :OCommonStatement_IBASE(m_aMutex)
    ,OPropertySetHelper(OCommonStatement_IBASE::rBHelper)
    ,OCommonStatement_SBASE((::cppu::OWeakObject*)_pConnection, this)
    ,m_xDBMetaData(_pConnection->getMetaData())
    ,m_pTable(NULL)
    ,m_pConnection(_pConnection)
    ,m_aParser(_pConnection->getDriver()->getMSFactory())
    ,m_pSQLIterator( new OSQLParseTreeIterator( _pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL ) )
    ,m_pParseTree(NULL)
    ,rBHelper(OCommonStatement_IBASE::rBHelper)
{
    m_pConnection->acquire();
    OSL_TRACE("In/Out: OCommonStatement::OCommonStatement" );
}
// -----------------------------------------------------------------------------
OCommonStatement::~OCommonStatement()
{
}

//------------------------------------------------------------------------------
void OCommonStatement::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    clearWarnings();
    clearCachedResultSet();

    if (m_pConnection)
        m_pConnection->release();
    m_pConnection = NULL;

    m_pSQLIterator->dispose();

    dispose_ChildImpl();
    OCommonStatement_IBASE::disposing();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OCommonStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OCommonStatement_IBASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OCommonStatement::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OCommonStatement_IBASE::getTypes());
}
// -------------------------------------------------------------------------
void SAL_CALL OCommonStatement::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);
    }
    dispose();
}


// -------------------------------------------------------------------------
void OCommonStatement::createTable( ) throw ( SQLException, RuntimeException )
{
    if(m_pParseTree)
    {
        ::rtl::Reference<connectivity::OSQLColumns> xCreateColumn;
        if (m_pSQLIterator->getStatementType() == SQL_STATEMENT_CREATE_TABLE)
        {
            const OSQLTables& xTabs = m_pSQLIterator->getTables();
            OSL_ENSURE( !xTabs.empty(), "Need a Table");
            ::rtl::OUString ouTableName=xTabs.begin()->first;
            xCreateColumn     = m_pSQLIterator->getCreateColumns();
            OSL_ENSURE(xCreateColumn.is(), "Need the Columns!!");

            const OColumnAlias& aColumnAlias = m_pConnection->getColumnAlias();

            OSQLColumns::Vector::const_iterator aIter = xCreateColumn->get().begin();
            const ::rtl::OUString sProprtyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
            ::rtl::OUString sName;
            for (sal_Int32 i = 1; aIter != xCreateColumn->get().end();++aIter, i++)
            {
                (*aIter)->getPropertyValue(sProprtyName) >>= sName;
                if ( !aColumnAlias.hasAlias( sName ) )
                {

                    const ::rtl::OUString sError( getOwnConnection()->getResources().getResourceStringWithSubstitution(
                            STR_INVALID_COLUMNNAME,
                            "$columnname$", sName
                         ) );
                    ::dbtools::throwGenericSQLException(sError,*this);
                }
            }
            MDatabaseMetaDataHelper     _aDbHelper;
            if (!_aDbHelper.NewAddressBook(m_pConnection,ouTableName))
            {
                getOwnConnection()->throwSQLException( _aDbHelper.getError(), *this );
            }
            m_pSQLIterator.reset( new ::connectivity::OSQLParseTreeIterator(
                m_pConnection, m_pConnection->createCatalog()->getTables(), m_aParser, NULL ) );
        }

    }
    else
        getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
}
// -------------------------------------------------------------------------
OCommonStatement::StatementType OCommonStatement::parseSql( const ::rtl::OUString& sql , sal_Bool bAdjusted)
    throw ( SQLException, RuntimeException )
{
    ::rtl::OUString aErr;

    m_pParseTree = m_aParser.parseTree(aErr,sql);

#if OSL_DEBUG_LEVEL > 0
    {
        const char* str = OUtoCStr(sql);
        OSL_UNUSED( str );
        OSL_TRACE("ParseSQL: %s", OUtoCStr( sql ) );
    }
#endif // OSL_DEBUG_LEVEL

    if(m_pParseTree)
    {
        m_pSQLIterator->setParseTree(m_pParseTree);
        m_pSQLIterator->traverseAll();
        const OSQLTables& xTabs = m_pSQLIterator->getTables();
        if(xTabs.empty())
            getOwnConnection()->throwSQLException( STR_QUERY_AT_LEAST_ONE_TABLES, *this );

#if OSL_DEBUG_LEVEL > 0
        OSQLTables::const_iterator citer;
        for( citer = xTabs.begin(); citer != xTabs.end(); ++citer ) {
            OSL_TRACE("SELECT Table : %s", OUtoCStr(citer->first) );
        }
#endif

        Reference<XIndexAccess> xNames;
        switch(m_pSQLIterator->getStatementType())
        {
        case SQL_STATEMENT_SELECT:

            // at this moment we support only one table per select statement

            OSL_ENSURE( xTabs.begin() != xTabs.end(), "Need a Table");

            m_pTable = static_cast< OTable* > (xTabs.begin()->second.get());
            m_xColNames     = m_pTable->getColumns();
            xNames = Reference<XIndexAccess>(m_xColNames,UNO_QUERY);
            // set the binding of the resultrow
            m_aRow          = new OValueVector(xNames->getCount());
            (m_aRow->get())[0].setBound(sal_True);
            ::std::for_each(m_aRow->get().begin()+1,m_aRow->get().end(),TSetBound(sal_False));
            // create the column mapping
            createColumnMapping();

            analyseSQL();
            return eSelect;

        case SQL_STATEMENT_CREATE_TABLE:
            createTable();
            return eCreateTable;

        default:
            break;
        }
    }
    else if(!bAdjusted) //Our sql parser does not support a statement like "create table foo"
                        // So we append ("E-mail" varchar) to the last of it to make it work
    {
        return parseSql(sql + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("(""E-mail"" caracter)")),sal_True);
    }

    getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
    OSL_FAIL( "OCommonStatement::parseSql: unreachable!" );
    return eSelect;

}
// -------------------------------------------------------------------------
Reference< XResultSet > OCommonStatement::impl_executeCurrentQuery()
{
    clearCachedResultSet();

    ::rtl::Reference< OResultSet > pResult( new OResultSet( this, m_pSQLIterator ) );
    initializeResultSet( pResult.get() );

    pResult->executeQuery();
    cacheResultSet( pResult );  // only cache if we survived the execution

    return pResult.get();

}

// -------------------------------------------------------------------------
void OCommonStatement::initializeResultSet( OResultSet* _pResult )
{
    ENSURE_OR_THROW( _pResult, "invalid result set" );

    _pResult->setColumnMapping(m_aColMapping);
    _pResult->setOrderByColumns(m_aOrderbyColumnNumber);
    _pResult->setOrderByAscending(m_aOrderbyAscending);
    _pResult->setBindingRow(m_aRow);
    _pResult->setTable(m_pTable);
}

// -------------------------------------------------------------------------
void OCommonStatement::clearCachedResultSet()
{
    Reference< XResultSet > xResultSet( m_xResultSet.get(), UNO_QUERY );
    if ( !xResultSet.is() )
        return;

    try
    {
        Reference< XCloseable > xCloseable( xResultSet, UNO_QUERY_THROW );
        xCloseable->close();
    }
    catch( const DisposedException& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xResultSet = Reference< XResultSet >();
}

// -------------------------------------------------------------------------
void OCommonStatement::cacheResultSet( const ::rtl::Reference< OResultSet >& _pResult )
{
    ENSURE_OR_THROW( _pResult.is(), "invalid result set" );
    m_xResultSet = Reference< XResultSet >( _pResult.get() );
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OCommonStatement::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    OSL_TRACE("Statement::execute( %s )", OUtoCStr( sql ) );

    Reference< XResultSet > xRS = executeQuery( sql );
    // returns true when a resultset is available
    return xRS.is();
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OCommonStatement::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_ThreadMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    OSL_TRACE("Statement::executeQuery( %s )", OUtoCStr( sql ) );

    // parse the statement
    StatementType eStatementType = parseSql( sql );
    if ( eStatementType != eSelect )
        return NULL;

    return impl_executeCurrentQuery();
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OCommonStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    // just return our connection here
    return (Reference< XConnection >)m_pConnection;
}
// -----------------------------------------------------------------------------
Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XServiceInfo*> (this));
    if(!aRet.hasValue())
        aRet = OCommonStatement::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OCommonStatement::executeUpdate( const ::rtl::OUString& /*sql*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XStatement::executeUpdate", *this );
    return 0;

}
// -------------------------------------------------------------------------
Any SAL_CALL OCommonStatement::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OCommonStatement::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OCommonStatement::createArrayHelper( ) const
{
    // this properties are define by the service resultset
    // they must in alphabetic order
    Sequence< Property > aProps(9);
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

    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OCommonStatement::getInfoHelper()
{
    return *const_cast<OCommonStatement*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OCommonStatement::convertFastPropertyValue(
                            Any & /*rConvertedValue*/,
                            Any & /*rOldValue*/,
                            sal_Int32 /*nHandle*/,
                            const Any& /*rValue*/ )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bConverted = sal_False;
    // here we have to try to convert
    return bConverted;
}
// -------------------------------------------------------------------------
void OCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& /*rValue*/) throw (Exception)
{
    // set the value to whatever is necessary
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OCommonStatement::getFastPropertyValue(Any& /*rValue*/,sal_Int32 nHandle) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        default:
            ;
    }
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");
// -----------------------------------------------------------------------------
void SAL_CALL OCommonStatement::acquire() throw()
{
    OCommonStatement_IBASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OCommonStatement::release() throw()
{
    relase_ChildImpl();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::acquire() throw()
{
    OCommonStatement::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement::release() throw()
{
    OCommonStatement::release();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OCommonStatement::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
void OCommonStatement::createColumnMapping()
{
    size_t i;

    // initialize the column index map (mapping select columns to table columns)
    ::rtl::Reference<connectivity::OSQLColumns> xColumns = m_pSQLIterator->getSelectColumns();
    m_aColMapping.resize(xColumns->get().size() + 1);
    for (i=0; i<m_aColMapping.size(); ++i)
        m_aColMapping[i] = static_cast<sal_Int32>(i);

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
    // now check which columns are bound
#if OSL_DEBUG_LEVEL > 0
    for ( i = 0; i < m_aColMapping.size(); i++ )
        OSL_TRACE("BEFORE Mapped: %d -> %d", i, m_aColMapping[i] );
#endif
    OResultSet::setBoundedColumns(m_aRow,xColumns,xNames,sal_True,m_xDBMetaData,m_aColMapping);
#if OSL_DEBUG_LEVEL > 0
    for ( i = 0; i < m_aColMapping.size(); i++ )
        OSL_TRACE("AFTER  Mapped: %d -> %d", i, m_aColMapping[i] );
#endif
}
// -----------------------------------------------------------------------------

void OCommonStatement::analyseSQL()
{
    const OSQLParseNode* pOrderbyClause = m_pSQLIterator->getOrderTree();
    if(pOrderbyClause)
    {
        OSQLParseNode * pOrderingSpecCommalist = pOrderbyClause->getChild(2);
        OSL_ENSURE(SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OResultSet: Fehler im Parse Tree");

        for (sal_uInt32 m = 0; m < pOrderingSpecCommalist->count(); m++)
        {
            OSQLParseNode * pOrderingSpec = pOrderingSpecCommalist->getChild(m);
            OSL_ENSURE(SQL_ISRULE(pOrderingSpec,ordering_spec),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pOrderingSpec->count() == 2,"OResultSet: Fehler im Parse Tree");

            OSQLParseNode * pColumnRef = pOrderingSpec->getChild(0);
            if(!SQL_ISRULE(pColumnRef,column_ref))
            {
                throw SQLException();
            }
            OSQLParseNode * pAscendingDescending = pOrderingSpec->getChild(1);
            setOrderbyColumn(pColumnRef,pAscendingDescending);
        }
    }
}
//------------------------------------------------------------------
void OCommonStatement::setOrderbyColumn(    OSQLParseNode* pColumnRef,
                                        OSQLParseNode* pAscendingDescending)
{
    ::rtl::OUString aColumnName;
    if (pColumnRef->count() == 1)
        aColumnName = pColumnRef->getChild(0)->getTokenValue();
    else if (pColumnRef->count() == 3)
    {
        pColumnRef->getChild(2)->parseNodeToStr( aColumnName, getOwnConnection(), NULL, sal_False, sal_False );
    }
    else
    {
        throw SQLException();
    }

    Reference<XColumnLocate> xColLocate(m_xColNames,UNO_QUERY);
    if(!xColLocate.is())
        return;

    m_aOrderbyColumnNumber.push_back(xColLocate->findColumn(aColumnName));

    // Ascending or Descending?
    m_aOrderbyAscending.push_back((SQL_ISTOKEN(pAscendingDescending,DESC)) ? SQL_DESC : SQL_ASC);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
