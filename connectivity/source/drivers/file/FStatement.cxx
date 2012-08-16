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

#include <osl/diagnose.h>
#include "file/FStatement.hxx"
#include "file/FConnection.hxx"
#include "file/FDriver.hxx"
#include "file/FResultSet.hxx"
#include <comphelper/property.hxx>
#include <comphelper/uno3.hxx>
#include <osl/thread.h>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include "connectivity/dbexception.hxx"
#include "resource/file_res.hrc"
#include <algorithm>
#include <tools/debug.hxx>
#include <rtl/logfile.hxx>

namespace connectivity
{
    namespace file
    {

//------------------------------------------------------------------------------
using namespace dbtools;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
DBG_NAME( file_OStatement_Base )

//------------------------------------------------------------------------------
OStatement_Base::OStatement_Base(OConnection* _pConnection )
    :OStatement_BASE(m_aMutex)
    ,::comphelper::OPropertyContainer(OStatement_BASE::rBHelper)
    ,m_xDBMetaData(_pConnection->getMetaData())
    ,m_aParser(_pConnection->getDriver()->getFactory())
    ,m_aSQLIterator( _pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL )
    ,m_pConnection(_pConnection)
    ,m_pParseTree(NULL)
    ,m_pSQLAnalyzer(NULL)
    ,m_pEvaluationKeySet(NULL)
    ,m_pTable(NULL)
    ,m_nMaxFieldSize(0)
    ,m_nMaxRows(0)
    ,m_nQueryTimeOut(0)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::FORWARD_ONLY)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
    ,m_bEscapeProcessing(sal_True)
    ,rBHelper(OStatement_BASE::rBHelper)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::OStatement_Base" );
    DBG_CTOR( file_OStatement_Base, NULL );

    m_pConnection->acquire();

    sal_Int32 nAttrib = 0;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),      PROPERTY_ID_CURSORNAME,         nAttrib,&m_aCursorName,     ::getCppuType(static_cast< ::rtl::OUString*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),    PROPERTY_ID_MAXFIELDSIZE,       nAttrib,&m_nMaxFieldSize,       ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),         PROPERTY_ID_MAXROWS,            nAttrib,&m_nMaxRows,        ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),    PROPERTY_ID_QUERYTIMEOUT,       nAttrib,&m_nQueryTimeOut,   ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),       PROPERTY_ID_FETCHSIZE,          nAttrib,&m_nFetchSize,      ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),   PROPERTY_ID_RESULTSETTYPE,      nAttrib,&m_nResultSetType,  ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),  PROPERTY_ID_FETCHDIRECTION,     nAttrib,&m_nFetchDirection, ::getCppuType(static_cast<sal_Int32*>(0)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),PROPERTY_ID_ESCAPEPROCESSING,   nAttrib,&m_bEscapeProcessing,::getCppuBooleanType());

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),        PROPERTY_ID_RESULTSETCONCURRENCY,   nAttrib,&m_nResultSetConcurrency,       ::getCppuType(static_cast<sal_Int32*>(0)));
}
// -----------------------------------------------------------------------------
OStatement_Base::~OStatement_Base()
{
    osl_incrementInterlockedCount( &m_refCount );
    disposing();
    delete m_pSQLAnalyzer;

    DBG_DTOR( file_OStatement_Base, NULL );
}
//------------------------------------------------------------------------------
void OStatement_Base::disposeResultSet()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::disposeResultSet" );
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

    if(m_pSQLAnalyzer)
        m_pSQLAnalyzer->dispose();

    if(m_aRow.is())
    {
        m_aRow->get().clear();
        m_aRow = NULL;
    }

    m_aSQLIterator.dispose();

    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }

    if (m_pConnection)
    {
        m_pConnection->release();
        m_pConnection = NULL;
    }

    dispose_ChildImpl();

    if ( m_pParseTree )
    {
        delete m_pParseTree;
        m_pParseTree = NULL;
    }

    OStatement_Base::disposing();
}
// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::acquire() throw()
{
    OStatement_BASE::acquire();
}
//-----------------------------------------------------------------------------
void SAL_CALL OStatement_BASE2::release() throw()
{
    relase_ChildImpl();
}
//-----------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::queryInterface( const Type & rType ) throw(RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::queryInterface" );
    const Any aRet = OStatement_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatement_Base::getTypes(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::getTypes" );
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                                    ::getCppuType( (const Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                                    ::getCppuType( (const Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::cancel(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::cancel" );
}
// -------------------------------------------------------------------------

void SAL_CALL OStatement_Base::close(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::close" );
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    }
    dispose();
}
// -------------------------------------------------------------------------

void OStatement_Base::reset() throw (SQLException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::reset" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::clearMyResultSet " );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    try
    {
        Reference<XCloseable> xCloseable;
        if ( ::comphelper::query_interface( m_xResultSet.get(), xCloseable ) )
            xCloseable->close();
    }
    catch( const DisposedException& ) { }

    m_xResultSet = Reference< XResultSet>();
}

// -------------------------------------------------------------------------
Any SAL_CALL OStatement_Base::getWarnings(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::getWarnings" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}
// -------------------------------------------------------------------------
void SAL_CALL OStatement_Base::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::clearWarnings" );
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OStatement_Base::createArrayHelper( ) const
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::createArrayHelper" );
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OStatement_Base::getInfoHelper()
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::getInfoHelper" );
    return *const_cast<OStatement_Base*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
OResultSet* OStatement::createResultSet()
{
    return new OResultSet(this,m_aSQLIterator);
}
// -------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbc.driver.file.Statement","com.sun.star.sdbc.Statement");
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
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OStatement::execute( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    executeQuery(sql);

    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT;
}

// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OStatement::executeQuery( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    construct(sql);
    Reference< XResultSet > xRS;
    OResultSet* pResult = createResultSet();
    xRS = pResult;
    initializeResultSet(pResult);
    m_xResultSet = Reference<XResultSet>(pResult);

    pResult->OpenImpl();

    return xRS;
}
// -------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OStatement::executeUpdate( const ::rtl::OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    construct(sql);
    OResultSet* pResult = createResultSet();
    Reference< XResultSet > xRS = pResult;
    initializeResultSet(pResult);
    pResult->OpenImpl();

    return pResult->getRowCountResult();
}

// -----------------------------------------------------------------------------
void SAL_CALL OStatement_Base::disposing(void)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::disposing" );
    if(m_aEvaluateRow.is())
    {
        m_aEvaluateRow->get().clear();
        m_aEvaluateRow = NULL;
    }
    delete m_pEvaluationKeySet;
    OStatement_BASE::disposing();
}
// -----------------------------------------------------------------------------
Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatement_Base::getPropertySetInfo(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::getPropertySetInfo" );
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_XStatement::queryInterface( rType);
    return aRet.hasValue() ? aRet : OStatement_BASE2::queryInterface( rType);
}
// -----------------------------------------------------------------------------
OSQLAnalyzer* OStatement_Base::createAnalyzer()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::createAnalyzer" );
    return new OSQLAnalyzer(m_pConnection);
}
// -----------------------------------------------------------------------------
void OStatement_Base::anylizeSQL()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::anylizeSQL" );
    OSL_ENSURE(m_pSQLAnalyzer,"OResultSet::anylizeSQL: Analyzer isn't set!");
    // start analysing the statement
    m_pSQLAnalyzer->setOrigColumns(m_xColNames);
    m_pSQLAnalyzer->start(m_pParseTree);

    const OSQLParseNode* pOrderbyClause = m_aSQLIterator.getOrderTree();
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::setOrderbyColumn" );
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
    // Everything tested and we have the name of the Column.
    // What number is the Column?
    ::rtl::Reference<OSQLColumns> aSelectColumns = m_aSQLIterator.getSelectColumns();
    ::comphelper::UStringMixEqual aCase;
    OSQLColumns::Vector::const_iterator aFind = ::connectivity::find(aSelectColumns->get().begin(),aSelectColumns->get().end(),aColumnName,aCase);
    if ( aFind == aSelectColumns->get().end() )
        throw SQLException();
    m_aOrderbyColumnNumber.push_back((aFind - aSelectColumns->get().begin()) + 1);

    // Ascending or Descending?
    m_aOrderbyAscending.push_back((SQL_ISTOKEN(pAscendingDescending,DESC)) ? SQL_DESC : SQL_ASC);
}

// -----------------------------------------------------------------------------
void OStatement_Base::construct(const ::rtl::OUString& sql)  throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::construct" );
    ::rtl::OUString aErr;
    m_pParseTree = m_aParser.parseTree(aErr,sql);
    if(m_pParseTree)
    {
        m_aSQLIterator.setParseTree(m_pParseTree);
        m_aSQLIterator.traverseAll();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();

        // sanity checks
        if ( xTabs.empty() )
            // no tables -> nothing to operate on -> error
            m_pConnection->throwGenericSQLException(STR_QUERY_NO_TABLE,*this);

        if ( xTabs.size() > 1 || m_aSQLIterator.hasErrors() )
            // more than one table -> can't operate on them -> error
            m_pConnection->throwGenericSQLException(STR_QUERY_MORE_TABLES,*this);

        if ( (m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT) && m_aSQLIterator.getSelectColumns()->get().empty() )
            // SELECT statement without columns -> error
            m_pConnection->throwGenericSQLException(STR_QUERY_NO_COLUMN,*this);

        switch(m_aSQLIterator.getStatementType())
        {
            case SQL_STATEMENT_CREATE_TABLE:
            case SQL_STATEMENT_ODBC_CALL:
            case SQL_STATEMENT_UNKNOWN:
                m_pConnection->throwGenericSQLException(STR_QUERY_TOO_COMPLEX,*this);
                break;
            default:
                break;
        }

        // at this moment we support only one table per select statement
        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xTabs.begin()->second,UNO_QUERY);
        if(xTunnel.is())
        {
            if(m_pTable)
                m_pTable->release();
            m_pTable = reinterpret_cast<OFileTable*>(xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId()));
            if(m_pTable)
                m_pTable->acquire();
        }
        OSL_ENSURE(m_pTable,"No table!");
        if ( m_pTable )
            m_xColNames     = m_pTable->getColumns();
        Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
        // set the binding of the resultrow
        m_aRow          = new OValueRefVector(xNames->getCount());
        (m_aRow->get())[0]->setBound(sal_True);
        ::std::for_each(m_aRow->get().begin()+1,m_aRow->get().end(),TSetRefBound(sal_False));

        // set the binding of the resultrow
        m_aEvaluateRow  = new OValueRefVector(xNames->getCount());

        (m_aEvaluateRow->get())[0]->setBound(sal_True);
        ::std::for_each(m_aEvaluateRow->get().begin()+1,m_aEvaluateRow->get().end(),TSetRefBound(sal_False));

        // set the select row
        m_aSelectRow = new OValueRefVector(m_aSQLIterator.getSelectColumns()->get().size());
        ::std::for_each(m_aSelectRow->get().begin(),m_aSelectRow->get().end(),TSetRefBound(sal_True));

        // create the column mapping
        createColumnMapping();

        m_pSQLAnalyzer  = createAnalyzer();

        Reference<XIndexesSupplier> xIndexSup(xTunnel,UNO_QUERY);
        if(xIndexSup.is())
            m_pSQLAnalyzer->setIndexes(xIndexSup->getIndexes());

        anylizeSQL();
    }
    else
        throw SQLException(aErr,*this,::rtl::OUString(),0,Any());
}
// -----------------------------------------------------------------------------
void OStatement_Base::createColumnMapping()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::createColumnMapping" );
    // initialize the column index map (mapping select columns to table columns)
    ::rtl::Reference<connectivity::OSQLColumns> xColumns = m_aSQLIterator.getSelectColumns();
    m_aColMapping.resize(xColumns->get().size() + 1);
    for (sal_Int32 i=0; i<(sal_Int32)m_aColMapping.size(); ++i)
        m_aColMapping[i] = i;

    Reference<XIndexAccess> xNames(m_xColNames,UNO_QUERY);
    // now check which columns are bound
    OResultSet::setBoundedColumns(m_aRow,m_aSelectRow,xColumns,xNames,sal_True,m_xDBMetaData,m_aColMapping);
}
// -----------------------------------------------------------------------------
void OStatement_Base::initializeResultSet(OResultSet* _pResult)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::initializeResultSet" );
    GetAssignValues();

    _pResult->setSqlAnalyzer(m_pSQLAnalyzer);
    _pResult->setOrderByColumns(m_aOrderbyColumnNumber);
    _pResult->setOrderByAscending(m_aOrderbyAscending);
    _pResult->setBindingRow(m_aRow);
    _pResult->setColumnMapping(m_aColMapping);
    _pResult->setEvaluationRow(m_aEvaluateRow);
    _pResult->setAssignValues(m_aAssignValues);
    _pResult->setSelectRow(m_aSelectRow);

    m_pSQLAnalyzer->bindSelectRow(m_aRow);
    m_pEvaluationKeySet = m_pSQLAnalyzer->bindEvaluationRow(m_aEvaluateRow);    // Set values in the code of the Compiler
    _pResult->setEvaluationKeySet(m_pEvaluationKeySet);
}
// -----------------------------------------------------------------------------
void OStatement_Base::GetAssignValues()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::GetAssignValues" );
    if (m_pParseTree == NULL)
    {
        ::dbtools::throwFunctionSequenceException(*this);
        return;
    }

    if (SQL_ISRULE(m_pParseTree,select_statement))
        // no values have to be set for SELECT
        return;
    else if (SQL_ISRULE(m_pParseTree,insert_statement))
    {
        // Create Row for the values to be set (Reference trough new)
        if(m_aAssignValues.is())
            m_aAssignValues->get().clear();
        sal_Int32 nCount = Reference<XIndexAccess>(m_xColNames,UNO_QUERY)->getCount();
        m_aAssignValues = new OAssignValues(nCount);
        // unbound all
        ::std::for_each(m_aAssignValues->get().begin()+1,m_aAssignValues->get().end(),TSetRefBound(sal_False));

        m_aParameterIndexes.resize(nCount+1,SQL_NO_PARAMETER);

        // List of Column-Names, that exist in the column_commalist (seperated by ;):
        ::std::vector<String> aColumnNameList;

        OSL_ENSURE(m_pParseTree->count() >= 4,"OResultSet: Fehler im Parse Tree");

        OSQLParseNode * pOptColumnCommalist = m_pParseTree->getChild(3);
        OSL_ENSURE(pOptColumnCommalist != NULL,"OResultSet: Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISRULE(pOptColumnCommalist,opt_column_commalist),"OResultSet: Fehler im Parse Tree");
        if (pOptColumnCommalist->count() == 0)
        {
            const Sequence< ::rtl::OUString>& aNames = m_xColNames->getElementNames();
            const ::rtl::OUString* pBegin = aNames.getConstArray();
            const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
            for (; pBegin != pEnd; ++pBegin)
                aColumnNameList.push_back(*pBegin);
        }
        else
        {
            OSL_ENSURE(pOptColumnCommalist->count() == 3,"OResultSet: Fehler im Parse Tree");

            OSQLParseNode * pColumnCommalist = pOptColumnCommalist->getChild(1);
            OSL_ENSURE(pColumnCommalist != NULL,"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(SQL_ISRULE(pColumnCommalist,column_commalist),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pColumnCommalist->count() > 0,"OResultSet: Fehler im Parse Tree");

            // All Columns in the column_commalist ...
            for (sal_uInt32 i = 0; i < pColumnCommalist->count(); i++)
            {
                OSQLParseNode * pCol = pColumnCommalist->getChild(i);
                OSL_ENSURE(pCol != NULL,"OResultSet: Fehler im Parse Tree");
                aColumnNameList.push_back(pCol->getTokenValue());
            }
        }
        if ( aColumnNameList.empty() )
            throwFunctionSequenceException(*this);

        // Values ...
        OSQLParseNode * pValuesOrQuerySpec = m_pParseTree->getChild(4);
        OSL_ENSURE(pValuesOrQuerySpec != NULL,"OResultSet: pValuesOrQuerySpec darf nicht NULL sein!");
        OSL_ENSURE(SQL_ISRULE(pValuesOrQuerySpec,values_or_query_spec),"OResultSet: ! SQL_ISRULE(pValuesOrQuerySpec,values_or_query_spec)");
        OSL_ENSURE(pValuesOrQuerySpec->count() > 0,"OResultSet: pValuesOrQuerySpec->count() <= 0");

        // just "VALUES" is allowed ...
        if (! SQL_ISTOKEN(pValuesOrQuerySpec->getChild(0),VALUES))
            throwFunctionSequenceException(*this);

        OSL_ENSURE(pValuesOrQuerySpec->count() == 4,"OResultSet: pValuesOrQuerySpec->count() != 4");

        // List of values
        OSQLParseNode * pInsertAtomCommalist = pValuesOrQuerySpec->getChild(2);
        OSL_ENSURE(pInsertAtomCommalist != NULL,"OResultSet: pInsertAtomCommalist darf nicht NULL sein!");
        OSL_ENSURE(pInsertAtomCommalist->count() > 0,"OResultSet: pInsertAtomCommalist <= 0");

        String aColumnName;
        OSQLParseNode * pRow_Value_Const;
        xub_StrLen nIndex=0;
        for (sal_uInt32 i = 0; i < pInsertAtomCommalist->count(); i++)
        {
            pRow_Value_Const = pInsertAtomCommalist->getChild(i); // row_value_constructor
            OSL_ENSURE(pRow_Value_Const != NULL,"OResultSet: pRow_Value_Const darf nicht NULL sein!");
            if(SQL_ISRULE(pRow_Value_Const,parameter))
            {
                ParseAssignValues(aColumnNameList,pRow_Value_Const,nIndex++); // only one Columnname allowed per loop
            }
            else if(pRow_Value_Const->isToken())
                ParseAssignValues(aColumnNameList,pRow_Value_Const,static_cast<xub_StrLen>(i));
            else
            {
                if(pRow_Value_Const->count() == aColumnNameList.size())
                {
                    for (sal_uInt32 j = 0; j < pRow_Value_Const->count(); ++j)
                        ParseAssignValues(aColumnNameList,pRow_Value_Const->getChild(j),nIndex++);
                }
                else
                    throwFunctionSequenceException(*this);
            }
        }
    }
    else if (SQL_ISRULE(m_pParseTree,update_statement_searched))
    {
        if(m_aAssignValues.is())
            m_aAssignValues->get().clear();
        sal_Int32 nCount = Reference<XIndexAccess>(m_xColNames,UNO_QUERY)->getCount();
        m_aAssignValues = new OAssignValues(nCount);
        // unbound all
        ::std::for_each(m_aAssignValues->get().begin()+1,m_aAssignValues->get().end(),TSetRefBound(sal_False));

        m_aParameterIndexes.resize(nCount+1,SQL_NO_PARAMETER);

        OSL_ENSURE(m_pParseTree->count() >= 4,"OResultSet: Fehler im Parse Tree");

        OSQLParseNode * pAssignmentCommalist = m_pParseTree->getChild(3);
        OSL_ENSURE(pAssignmentCommalist != NULL,"OResultSet: pAssignmentCommalist == NULL");
        OSL_ENSURE(SQL_ISRULE(pAssignmentCommalist,assignment_commalist),"OResultSet: Fehler im Parse Tree");
        OSL_ENSURE(pAssignmentCommalist->count() > 0,"OResultSet: pAssignmentCommalist->count() <= 0");

        // work on all assignments (commalist) ...
        ::std::vector< String> aList(1);
        for (sal_uInt32 i = 0; i < pAssignmentCommalist->count(); i++)
        {
            OSQLParseNode * pAssignment = pAssignmentCommalist->getChild(i);
            OSL_ENSURE(pAssignment != NULL,"OResultSet: pAssignment == NULL");
            OSL_ENSURE(SQL_ISRULE(pAssignment,assignment),"OResultSet: Fehler im Parse Tree");
            OSL_ENSURE(pAssignment->count() == 3,"OResultSet: pAssignment->count() != 3");

            OSQLParseNode * pCol = pAssignment->getChild(0);
            OSL_ENSURE(pCol != NULL,"OResultSet: pCol == NULL");

            OSQLParseNode * pComp = pAssignment->getChild(1);
            OSL_ENSURE(pComp != NULL,"OResultSet: pComp == NULL");
            OSL_ENSURE(pComp->getNodeType() == SQL_NODE_EQUAL,"OResultSet: pComp->getNodeType() != SQL_NODE_COMPARISON");
            if (pComp->getTokenValue().toChar() != '=')
            {
                throwFunctionSequenceException(*this);
            }

            OSQLParseNode * pVal = pAssignment->getChild(2);
            OSL_ENSURE(pVal != NULL,"OResultSet: pVal == NULL");
            aList[0] = pCol->getTokenValue();
            ParseAssignValues(aList,pVal,0);
        }

    }
}
// -------------------------------------------------------------------------
void OStatement_Base::ParseAssignValues(const ::std::vector< String>& aColumnNameList,OSQLParseNode* pRow_Value_Constructor_Elem,xub_StrLen nIndex)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::ParseAssignValues" );
    OSL_ENSURE(nIndex <= aColumnNameList.size(),"SdbFileCursor::ParseAssignValues: nIndex > aColumnNameList.GetTokenCount()");
    String aColumnName(aColumnNameList[nIndex]);
    OSL_ENSURE(aColumnName.Len() > 0,"OResultSet: Column-Name nicht gefunden");
    OSL_ENSURE(pRow_Value_Constructor_Elem != NULL,"OResultSet: pRow_Value_Constructor_Elem darf nicht NULL sein!");

    if (pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_STRING ||
        pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_INTNUM ||
        pRow_Value_Constructor_Elem->getNodeType() == SQL_NODE_APPROXNUM)
    {
        // set value:
        SetAssignValue(aColumnName, pRow_Value_Constructor_Elem->getTokenValue());
    }
    else if (SQL_ISTOKEN(pRow_Value_Constructor_Elem,NULL))
    {
        // set NULL
        SetAssignValue(aColumnName, String(), sal_True);
    }
    else if (SQL_ISRULE(pRow_Value_Constructor_Elem,parameter))
        parseParamterElem(aColumnName,pRow_Value_Constructor_Elem);
    else
    {
        throwFunctionSequenceException(*this);
    }
}
//------------------------------------------------------------------
void OStatement_Base::SetAssignValue(const String& aColumnName,
                                   const String& aValue,
                                   sal_Bool bSetNull,
                                   sal_uInt32 nParameter)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::SetAssignValue" );
    Reference<XPropertySet> xCol;
    m_xColNames->getByName(aColumnName) >>= xCol;
    sal_Int32 nId = Reference<XColumnLocate>(m_xColNames,UNO_QUERY)->findColumn(aColumnName);
    // does this column actually exist in the file?

    if (!xCol.is())
    {
        // This Column doesn't exist!
        throwFunctionSequenceException(*this);
    }


    // Everything tested and we have the names of the Column.
    // Now allocate one Value, set the value and tie the value to the Row.
    if (bSetNull)
        (m_aAssignValues->get())[nId]->setNull();
    else
    {
        switch (::comphelper::getINT32(xCol->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE))))
        {
            // put criteria depending on the Type as String or double in the variable
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                *(m_aAssignValues->get())[nId] = ORowSetValue(aValue);
                //Characterset is already converted, since the entire statement was converted
                break;

            case DataType::BIT:
                {
                    if (aValue.EqualsIgnoreCaseAscii("TRUE")  || aValue.GetChar(0) == '1')
                        *(m_aAssignValues->get())[nId] = sal_True;
                    else if (aValue.EqualsIgnoreCaseAscii("FALSE") || aValue.GetChar(0) == '0')
                        *(m_aAssignValues->get())[nId] = sal_False;
                    else
                    {
                        throwFunctionSequenceException(*this);
                    }
                }
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
            case DataType::INTEGER:
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::REAL:
            case DataType::DOUBLE:
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
            {
                *(m_aAssignValues->get())[nId] = ORowSetValue(aValue);
            }   break;
            default:
                throwFunctionSequenceException(*this);
        }
    }

    // save Parameter-No. (as User Data)
    // SQL_NO_PARAMETER = no Parameter.
    m_aAssignValues->setParameterIndex(nId,nParameter);
    if(nParameter != SQL_NO_PARAMETER)
        m_aParameterIndexes[nParameter] = nId;
}
// -----------------------------------------------------------------------------
void OStatement_Base::parseParamterElem(const String& /*_sColumnName*/,OSQLParseNode* /*pRow_Value_Constructor_Elem*/)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "file", "Ocke.Janssen@sun.com", "OStatement_Base::parseParamterElem" );
    // do nothing here
}
// =============================================================================
    } // namespace file
// =============================================================================
}// namespace connectivity
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
