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

#include "Connection.hxx"
#include "Driver.hxx"
#include "ResultSet.hxx"
#include "Statement.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>

using namespace connectivity::firebird;

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::std;

// ---- XBatchExecution - UNSUPPORTED ----------------------------------------
void SAL_CALL OStatement::addBatch(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    (void) sql;
}

void SAL_CALL OStatement::clearBatch() throw(SQLException, RuntimeException)
{
}

Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch() throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int32 >();
}

IMPLEMENT_SERVICE_INFO(OStatement,"com.sun.star.sdbcx.OStatement","com.sun.star.sdbc.Statement");

void SAL_CALL OStatement::acquire() throw()
{
    OStatementCommonBase::acquire();
}

void SAL_CALL OStatement::release() throw()
{
    OStatementCommonBase::release();
}

// ---- XStatement -----------------------------------------------------------
sal_Int32 SAL_CALL OStatement::executeUpdate(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    int aErr = isc_dsql_execute_immediate(m_statusVector,
                                          &m_pConnection->getDBHandle(),
                                          &m_pConnection->getTransaction(),
                                          0,
                                          OUStringToOString(sql, RTL_TEXTENCODING_UTF8).getStr(),
                                          FIREBIRD_SQL_DIALECT,
                                          NULL);

    if (aErr)
        SAL_WARN("connectivity.firebird", "isc_dsql_execute_immediate failed" );

    evaluateStatusVector(m_statusVector, sql, *this);
    // TODO: get number of changed rows with SELECT ROW_COUNT (use executeQuery)
    //     return getUpdateCount();
    return 0;
}


uno::Reference< XResultSet > SAL_CALL OStatement::executeQuery(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    XSQLDA* pOutSqlda = 0;
    int aErr = 0;

    aErr = prepareAndDescribeStatement(sql,
                                       pOutSqlda);
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "prepareAndDescribeStatement failed");
    }
    else
    {
        aErr = isc_dsql_execute(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &m_aStatementHandle,
                                1,
                                NULL);
        if (aErr)
            SAL_WARN("connectivity.firebird", "isc_dsql_execute failed" );
    }

    m_xResultSet = new OResultSet(m_pConnection,
                                  uno::Reference< XInterface >(*this),
                                  m_aStatementHandle,
                                  pOutSqlda);

    // TODO: deal with cleanup
//    close();

    evaluateStatusVector(m_statusVector, sql, *this);

    if (isDDLStatement(m_aStatementHandle))
    {
        m_pConnection->commit();
    }

    return m_xResultSet;
}

sal_Bool SAL_CALL OStatement::execute(const OUString& sql)
    throw(SQLException, RuntimeException)
{
    uno::Reference< XResultSet > xResults = executeQuery(sql);
    return xResults.is();
    // TODO: what if we have multiple results?
}

uno::Reference< XConnection > SAL_CALL OStatement::getConnection()
    throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return (uno::Reference< XConnection >)m_pConnection;
}

Any SAL_CALL OStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_Base::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if(!aRet.hasValue())
        aRet = OStatementCommonBase::queryInterface(rType);
    return aRet;
}

uno::Sequence< Type > SAL_CALL OStatement::getTypes()
    throw(RuntimeException)
{
    return concatSequences(OStatement_Base::getTypes(),
                           OStatementCommonBase::getTypes());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
