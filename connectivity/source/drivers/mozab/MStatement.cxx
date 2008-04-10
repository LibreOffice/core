/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MStatement.cxx,v $
 * $Revision: 1.18 $
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
#include <comphelper/property.hxx>
#include <comphelper/uno3.hxx>
#include <osl/thread.h>
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
#ifndef CONNECTIVITY_SRESULTSET_HXX
#include "MResultSet.hxx"
#endif
#include "MDatabaseMetaData.hxx"

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
OStatement_Base::OStatement_Base(OConnection* _pConnection )
    :OStatement_BASE(m_aMutex)
    ,OPropertySetHelper(OStatement_BASE::rBHelper)
    ,m_xDBMetaData(_pConnection->getMetaData())
    ,m_pTable(NULL)
    ,m_pConnection(_pConnection)
    ,m_aParser(_pConnection->getDriver()->getMSFactory())
    ,m_pSQLIterator( new OSQLParseTreeIterator( _pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL ) )
    ,m_pParseTree(NULL)
    ,rBHelper(OStatement_BASE::rBHelper)
{
    m_pConnection->acquire();
    OSL_TRACE("In/Out: OStatement_Base::OStatement_Base" );
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

    m_pSQLIterator->dispose();

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
//--------------------------------------------------------------------
// clearMyResultSet
// If a ResultSet was created for this Statement, close it
//--------------------------------------------------------------------

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

void OStatement_Base::createTable( )
    throw ( SQLException, RuntimeException )
{
    if(m_pParseTree)
    {
        ::vos::ORef<connectivity::OSQLColumns> xCreateColumn;
        if (m_pSQLIterator->getStatementType() == SQL_STATEMENT_CREATE_TABLE)
        {
            const OSQLTables& xTabs = m_pSQLIterator->getTables();
            OSL_ENSURE( !xTabs.empty(), "Need a Table");
            ::rtl::OUString ouTableName=xTabs.begin()->first;
            xCreateColumn     = m_pSQLIterator->getCreateColumns();
            OSL_ENSURE(xCreateColumn.isValid(), "Need the Columns!!");

            const OColumnAlias& aColumnAlias = m_pConnection->getColumnAlias();

            OSQLColumns::const_iterator aIter = xCreateColumn->begin();
            const ::rtl::OUString sProprtyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
            ::rtl::OUString sName;
            for (sal_Int32 i = 1; aIter != xCreateColumn->end();++aIter, i++)
            {
                (*aIter)->getPropertyValue(sProprtyName) >>= sName;
                if ( !aColumnAlias.hasAlias( sName ) )
                {
                    ::dbtools::throwGenericSQLException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Driver does not support column name: " ) ) + sName,
                        NULL );
                }
            }
            MDatabaseMetaDataHelper     _aDbHelper;
            if (!_aDbHelper.NewAddressBook(m_pConnection,ouTableName))
            {
                getOwnConnection()->throwGenericSQLException( _aDbHelper.getErrorResourceId() );
            }
            m_pSQLIterator.reset( new ::connectivity::OSQLParseTreeIterator(
                m_pConnection, m_pConnection->createCatalog()->getTables(), m_aParser, NULL ) );
        }

    }
    else
        ::dbtools::throwGenericSQLException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Problem parsing SQL!")),NULL);
}
// -------------------------------------------------------------------------
sal_Bool OStatement_Base::parseSql( const ::rtl::OUString& sql , sal_Bool bAdjusted)
    throw ( SQLException, RuntimeException )
{
    ::rtl::OUString aErr;

    OSL_TRACE("In/Out :: OStatement::parseSql(%s)\n", OUtoCStr( sql ) );

    m_pParseTree = m_aParser.parseTree(aErr,sql);

#if OSL_DEBUG_LEVEL > 0
    {
        const char* str = OUtoCStr(sql);
        OSL_UNUSED( str );
        OSL_TRACE("ParseSQL: %s\n", OUtoCStr( sql ) );
    }
#endif // OSL_DEBUG_LEVEL

    if(m_pParseTree)
    {
        m_pSQLIterator->setParseTree(m_pParseTree);
        m_pSQLIterator->traverseAll();
        const OSQLTables& xTabs = m_pSQLIterator->getTables();
        if(xTabs.empty())
            ::dbtools::throwGenericSQLException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Driver requires a single table to be specified in query")),NULL);
#if OSL_DEBUG_LEVEL > 0
        OSQLTables::const_iterator citer;
        for( citer = xTabs.begin(); citer != xTabs.end(); ++citer ) {
            OSL_TRACE("SELECT Table : %s\n", OUtoCStr(citer->first) );
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
            (*m_aRow)[0].setBound(sal_True);
            ::std::for_each(m_aRow->begin()+1,m_aRow->end(),TSetBound(sal_False));
            // create the column mapping
            createColumnMapping();

            analyseSQL();
            break;
        case SQL_STATEMENT_CREATE_TABLE:
            createTable();
            return sal_False;
        default:
            ::dbtools::throwGenericSQLException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Problem parsing SQL!")),NULL);
        }
    }
    else if(!bAdjusted) //Our sql parser does not support a statement like "create table foo"
                        // So we append ("E-mail" varchar) to the last of it to make it work
    {
        return parseSql(sql + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("(""E-mail"" caracter)")),sal_True);
    }
    else
        ::dbtools::throwGenericSQLException(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Problem parsing SQL!")),NULL);
    return sal_True;

}
// -------------------------------------------------------------------------

OResultSet* OStatement_Base::createResultSet()
{
    return new OResultSet( this, m_pSQLIterator );
}
// -------------------------------------------------------------------------

void OStatement_Base::initializeResultSet( OResultSet* _pResult )
{
    OSL_TRACE("In : initializeResultSet");
    _pResult->setColumnMapping(m_aColMapping);
    _pResult->setOrderByColumns(m_aOrderbyColumnNumber);
    _pResult->setOrderByAscending(m_aOrderbyAscending);
    _pResult->setBindingRow(m_aRow);
    _pResult->setTable(m_pTable);
    OSL_TRACE("Out : initializeResultSet");
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStatement_Base::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    OSL_TRACE("Statement::execute( %s )", OUtoCStr( sql ) );

    Reference< XResultSet > xRS = executeQuery( sql );
    // returns true when a resultset is available
    return xRS.is();
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement_Base::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_ThreadMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    OSL_TRACE("Statement::executeQuery( %s )", OUtoCStr( sql ) );

    if (!parseSql( sql )) //parseSql return false means this sql is a create table statement
        return NULL;

    OResultSet* pResult = createResultSet();
    Reference< XResultSet > xRS = pResult;
    initializeResultSet( pResult );

    pResult->executeQuery();
    m_xResultSet = xRS; // we need a reference to it for later use

    return xRS;
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OStatement_Base::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // just return our connection here
    return (Reference< XConnection >)m_pConnection;
}
// -----------------------------------------------------------------------------
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
    ::dbtools::throwFeatureNotImplementedException( "XPreparedStatement::executeUpdate", *this );
    return 0;

}
// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
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
    sal_Bool bConverted = sal_False;
    // here we have to try to convert
    return bConverted;
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
void OStatement_Base::getFastPropertyValue(Any& /*rValue*/,sal_Int32 nHandle) const
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
void OStatement_Base::createColumnMapping()
{
    size_t i;

    // initialize the column index map (mapping select columns to table columns)
    ::vos::ORef<connectivity::OSQLColumns>  xColumns = m_pSQLIterator->getSelectColumns();
    m_aColMapping.resize(xColumns->size() + 1);
    for (i=0; i<m_aColMapping.size(); ++i)
        m_aColMapping[i] = i;

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

void OStatement_Base::analyseSQL()
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
void OStatement_Base::setOrderbyColumn( OSQLParseNode* pColumnRef,
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
