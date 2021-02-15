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
#include "ResultSet.hxx"
#include "Statement.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <sal/log.hxx>

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
using namespace ::std;

// ---- XBatchExecution - UNSUPPORTED ----------------------------------------
void SAL_CALL OStatement::addBatch(const OUString&)
{
}

void SAL_CALL OStatement::clearBatch()
{
}

Sequence< sal_Int32 > SAL_CALL OStatement::executeBatch()
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

void OStatement::disposeResultSet()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    OStatementCommonBase::disposeResultSet();

    if (m_pSqlda)
    {
        freeSQLVAR(m_pSqlda);
        free(m_pSqlda);
        m_pSqlda = nullptr;
    }
}

// ---- XStatement -----------------------------------------------------------
sal_Int32 SAL_CALL OStatement::executeUpdate(const OUString& sql)
{
    execute(sql);
    return getStatementChangeCount();
}


uno::Reference< XResultSet > SAL_CALL OStatement::executeQuery(const OUString& sql)
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    SAL_INFO("connectivity.firebird", "executeQuery(" << sql << ")");

    ISC_STATUS aErr = 0;

    disposeResultSet();

    prepareAndDescribeStatement(sql,
                                m_pSqlda);

    aErr = isc_dsql_execute(m_statusVector,
                            &m_pConnection->getTransaction(),
                            &m_aStatementHandle,
                            1,
                            nullptr);
    if (aErr)
        SAL_WARN("connectivity.firebird", "isc_dsql_execute failed");

    m_xResultSet = new OResultSet(m_pConnection.get(),
                                  m_aMutex,
                                  uno::Reference< XInterface >(*this),
                                  m_aStatementHandle,
                                  m_pSqlda );

    // TODO: deal with cleanup

    evaluateStatusVector(m_statusVector, sql, *this);

    if (isDDLStatement())
    {
        m_pConnection->commit();
        m_pConnection->notifyDatabaseModified();
    }
    else if (getStatementChangeCount() > 0)
    {
        m_pConnection->notifyDatabaseModified();
    }

    return m_xResultSet;
}

sal_Bool SAL_CALL OStatement::execute(const OUString& sql)
{
    uno::Reference< XResultSet > xResults = executeQuery(sql);
    return xResults.is();
    // TODO: what if we have multiple results?
}

uno::Reference< XConnection > SAL_CALL OStatement::getConnection()
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return m_pConnection;
}

Any SAL_CALL OStatement::queryInterface( const Type & rType )
{
    Any aRet = OStatement_Base::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(rType,static_cast< XBatchExecution*> (this));
    if(!aRet.hasValue())
        aRet = OStatementCommonBase::queryInterface(rType);
    return aRet;
}

uno::Sequence< Type > SAL_CALL OStatement::getTypes()
{
    return concatSequences(OStatement_Base::getTypes(),
                           OStatementCommonBase::getTypes());
}

void SAL_CALL OStatement::disposing()
{
    disposeResultSet();
    close();
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
