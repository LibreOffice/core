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


#include <tools/diagnose_ex.h>
#include <sal/log.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/processfactory.hxx>
#include <connectivity/dbexception.hxx>

#include <algorithm>

#include "MDriver.hxx"
#include "MStatement.hxx"
#include <sqlbison.hxx>
#include "MResultSet.hxx"

#include <strings.hrc>

static ::osl::Mutex m_ThreadMutex;

using namespace ::comphelper;
using namespace connectivity::mork;
using namespace connectivity;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;


OStatement::OStatement( OConnection* _pConnection) : OCommonStatement( _pConnection)
{
}

OCommonStatement::OCommonStatement(OConnection* _pConnection )
    :OCommonStatement_IBASE(m_aMutex)
    ,OPropertySetHelper(OCommonStatement_IBASE::rBHelper)
    ,m_pTable(nullptr)
    ,m_pConnection(_pConnection)
    ,m_aParser( comphelper::getComponentContext(_pConnection->getDriver()->getFactory()) )
    ,m_pSQLIterator( new OSQLParseTreeIterator( _pConnection, _pConnection->createCatalog()->getTables(), m_aParser ) )
{
    m_xDBMetaData = _pConnection->getMetaData();
    m_pParseTree = nullptr;
}


OCommonStatement::~OCommonStatement()
{
}


void OCommonStatement::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    clearWarnings();
    clearCachedResultSet();

    m_pConnection.clear();

    m_pSQLIterator->dispose();
    m_pParseTree.reset();

    OCommonStatement_IBASE::disposing();
}

Any SAL_CALL OCommonStatement::queryInterface( const Type & rType )
{
    Any aRet = OCommonStatement_IBASE::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL OCommonStatement::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                    cppu::UnoType<XFastPropertySet>::get(),
                                    cppu::UnoType<XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OCommonStatement_IBASE::getTypes());
}

void SAL_CALL OCommonStatement::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);
    }
    dispose();
}

OCommonStatement::StatementType OCommonStatement::parseSql( const OUString& sql , bool bAdjusted)
{
    OUString aErr;

    m_pParseTree = m_aParser.parseTree(aErr,sql);

    if(m_pParseTree)
    {
        m_pSQLIterator->setParseTree(m_pParseTree.get());
        m_pSQLIterator->traverseAll();
        const OSQLTables& rTabs = m_pSQLIterator->getTables();

        if (rTabs.empty())
        {
            getOwnConnection()->throwSQLException( STR_QUERY_AT_LEAST_ONE_TABLES, *this );
        }

        Reference<XIndexAccess> xNames;
        switch(m_pSQLIterator->getStatementType())
        {
        case OSQLStatementType::Select:

            // at this moment we support only one table per select statement

            OSL_ENSURE( !rTabs.empty(), "Need a Table");

            m_pTable = static_cast< OTable* > (rTabs.begin()->second.get());
            m_xColNames     = m_pTable->getColumns();
            xNames.set(m_xColNames,UNO_QUERY);
            // set the binding of the resultrow
            m_aRow          = new OValueVector(xNames->getCount());
            (m_aRow->get())[0].setBound(true);
            std::for_each(m_aRow->get().begin()+1,m_aRow->get().end(),TSetBound(false));
            // create the column mapping
            createColumnMapping();

            analyseSQL();
            return eSelect;

        case OSQLStatementType::CreateTable:
            return eCreateTable;

        default:
            break;
        }
    }
    else if(!bAdjusted) //Our sql parser does not support a statement like "create table foo"
                        // So we append ("E-mail" varchar) to the last of it to make it work
    {
        return parseSql(sql + "(""E-mail"" character)", true);
    }

    getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
    OSL_FAIL( "OCommonStatement::parseSql: unreachable!" );
    return eSelect;

}

Reference< XResultSet > OCommonStatement::impl_executeCurrentQuery()
{
    clearCachedResultSet();

    ::rtl::Reference pResult( new OResultSet( this, m_pSQLIterator ) );
    initializeResultSet( pResult.get() );

    pResult->executeQuery();
    cacheResultSet( pResult );  // only cache if we survived the execution

    return pResult.get();

}


void OCommonStatement::initializeResultSet( OResultSet* _pResult )
{
    ENSURE_OR_THROW( _pResult, "invalid result set" );

    _pResult->setColumnMapping(m_aColMapping);
    _pResult->setOrderByColumns(m_aOrderbyColumnNumber);
    _pResult->setOrderByAscending(m_aOrderbyAscending);
    _pResult->setBindingRow(m_aRow);
    _pResult->setTable(m_pTable);
}


void OCommonStatement::clearCachedResultSet()
{
    Reference< XResultSet > xResultSet( m_xResultSet.get(), UNO_QUERY );
    if ( !xResultSet.is() )
        return;

    Reference< XCloseable >( xResultSet, UNO_QUERY_THROW )->close();

    m_xResultSet.clear();
}


void OCommonStatement::cacheResultSet( const ::rtl::Reference< OResultSet >& _pResult )
{
    ENSURE_OR_THROW( _pResult.is(), "invalid result set" );
    m_xResultSet = Reference< XResultSet >( _pResult.get() );
}


sal_Bool SAL_CALL OCommonStatement::execute( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    Reference< XResultSet > xRS = executeQuery( sql );
    // returns true when a resultset is available
    return xRS.is();
}


Reference< XResultSet > SAL_CALL OCommonStatement::executeQuery( const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_ThreadMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    // parse the statement
    StatementType eStatementType = parseSql( sql );
    if ( eStatementType != eSelect )
        return nullptr;

    return impl_executeCurrentQuery();
}


Reference< XConnection > SAL_CALL OCommonStatement::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    // just return our connection here
    return Reference< XConnection >(m_pConnection.get());
}

Any SAL_CALL OStatement::queryInterface( const Type & rType )
{
    Any aRet = ::cppu::queryInterface(rType,static_cast< XServiceInfo*> (this));
    if(!aRet.hasValue())
        aRet = OCommonStatement::queryInterface(rType);
    return aRet;
}

sal_Int32 SAL_CALL OCommonStatement::executeUpdate( const OUString& /*sql*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XStatement::executeUpdate", *this );
    return 0;

}

Any SAL_CALL OCommonStatement::getWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}


void SAL_CALL OCommonStatement::clearWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OCommonStatement_IBASE::rBHelper.bDisposed);


    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* OCommonStatement::createArrayHelper( ) const
{
    // this properties are define by the service resultset
    // they must in alphabetic order
    Sequence< Property > aProps(9);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),
        PROPERTY_ID_ESCAPEPROCESSING, cppu::UnoType<bool>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),
        PROPERTY_ID_MAXFIELDSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),
        PROPERTY_ID_MAXROWS, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),
        PROPERTY_ID_QUERYTIMEOUT, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}


::cppu::IPropertyArrayHelper & OCommonStatement::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool OCommonStatement::convertFastPropertyValue(
                            Any & /*rConvertedValue*/,
                            Any & /*rOldValue*/,
                            sal_Int32 /*nHandle*/,
                            const Any& /*rValue*/ )
{
    // here we have to try to convert
    return false;
}

void OCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& /*rValue*/)
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

IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");

void SAL_CALL OCommonStatement::acquire() throw()
{
    OCommonStatement_IBASE::acquire();
}

void SAL_CALL OCommonStatement::release() throw()
{
    OCommonStatement_IBASE::release();
}

void SAL_CALL OStatement::acquire() throw()
{
    OCommonStatement::acquire();
}

void SAL_CALL OStatement::release() throw()
{
    OCommonStatement::release();
}

Reference< css::beans::XPropertySetInfo > SAL_CALL OCommonStatement::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

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
        SAL_INFO(
            "connectivity.mork",
            "BEFORE Mapped: " << i << " -> " << m_aColMapping[i]);
#endif
    OResultSet::setBoundedColumns(m_aRow,xColumns,xNames,true,m_xDBMetaData,m_aColMapping);
#if OSL_DEBUG_LEVEL > 0
    for ( i = 0; i < m_aColMapping.size(); i++ )
        SAL_INFO(
            "connectivity.mork",
            "AFTER  Mapped: " << i << " -> " << m_aColMapping[i]);
#endif
}


void OCommonStatement::analyseSQL()
{
    const OSQLParseNode* pOrderbyClause = m_pSQLIterator->getOrderTree();
    if(pOrderbyClause)
    {
        OSQLParseNode * pOrderingSpecCommalist = pOrderbyClause->getChild(2);
        OSL_ENSURE(SQL_ISRULE(pOrderingSpecCommalist,ordering_spec_commalist),"OResultSet: Error in Parse Tree");

        for (size_t m = 0; m < pOrderingSpecCommalist->count(); m++)
        {
            OSQLParseNode * pOrderingSpec = pOrderingSpecCommalist->getChild(m);
            OSL_ENSURE(SQL_ISRULE(pOrderingSpec,ordering_spec),"OResultSet: Error in Parse Tree");
            OSL_ENSURE(pOrderingSpec->count() == 2,"OResultSet: Error in Parse Tree");

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

void OCommonStatement::setOrderbyColumn(OSQLParseNode const * pColumnRef,
                                        OSQLParseNode const * pAscendingDescending)
{
    OUString aColumnName;
    if (pColumnRef->count() == 1)
        aColumnName = pColumnRef->getChild(0)->getTokenValue();
    else if (pColumnRef->count() == 3)
    {
        pColumnRef->getChild(2)->parseNodeToStr( aColumnName, getOwnConnection(), nullptr, false, false );
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
    m_aOrderbyAscending.push_back(SQL_ISTOKEN(pAscendingDescending,DESC) ? TAscendingOrder::DESC : TAscendingOrder::ASC);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
