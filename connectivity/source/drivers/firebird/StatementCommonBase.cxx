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

#include "Driver.hxx"
#include "ResultSet.hxx"
#include "StatementCommonBase.hxx"
#include "Util.hxx"

#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <propertyids.hxx>
#include <TConnection.hxx>

using namespace ::connectivity::firebird;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;

using namespace ::comphelper;
using namespace ::osl;
using namespace ::rtl;
using namespace ::std;

OStatementCommonBase::OStatementCommonBase(OConnection* _pConnection)
    : OStatementCommonBase_Base(_pConnection->getMutex()),
      OPropertySetHelper(OStatementCommonBase_Base::rBHelper),
      m_pConnection(_pConnection),
      m_aStatementHandle( 0 ),
      rBHelper(OStatementCommonBase_Base::rBHelper)
{
    m_pConnection->acquire();
}

OStatementCommonBase::~OStatementCommonBase()
{
}

void OStatementCommonBase::disposeResultSet()
{
    uno::Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet = uno::Reference< XResultSet>();
}

void OStatementCommonBase::freeStatementHandle()
    throw (SQLException)
{
    if (m_aStatementHandle)
    {
        isc_dsql_free_statement(m_statusVector,
                                &m_aStatementHandle,
                                DSQL_drop);
        try {
            evaluateStatusVector(m_statusVector,
                                 "isc_dsql_free_statement",
                                 *this);
        }
        catch (SQLException e)
        {
            // we cannot throw any exceptions here anyway
            SAL_WARN("connectivity.firebird",
                     "isc_dsql_free_statement failed\n" << e.Message);
        }
    }
}

//-----------------------------------------------------------------------------
Any SAL_CALL OStatementCommonBase::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatementCommonBase_Base::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OStatementCommonBase::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes(
        ::cppu::UnoType< uno::Reference< XMultiPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< XFastPropertySet > >::get(),
        ::cppu::UnoType< uno::Reference< XPropertySet > >::get());

    return concatSequences(aTypes.getTypes(),OStatementCommonBase_Base::getTypes());
}
// -------------------------------------------------------------------------

void SAL_CALL OStatementCommonBase::cancel(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    // cancel the current sql statement
}

void SAL_CALL OStatementCommonBase::close()
    throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close");

    {
        MutexGuard aGuard(m_pConnection->getMutex());
        checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
        disposeResultSet();
        freeStatementHandle();
    }

    dispose();
}

int OStatementCommonBase::prepareAndDescribeStatement(const OUString& sql,
                                                      XSQLDA*& pOutSqlda,
                                                      XSQLDA* pInSqlda)
{
    MutexGuard aGuard(m_pConnection->getMutex());

    freeStatementHandle();

    if (!pOutSqlda)
    {
        pOutSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(10));
        pOutSqlda->version = SQLDA_VERSION1;
        pOutSqlda->sqln = 10;
    }

    int aErr = 0;

    aErr = isc_dsql_allocate_statement(m_statusVector,
                                       &m_pConnection->getDBHandle(),
                                       &m_aStatementHandle);

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_allocate_statement failed");
        return aErr;
    }
    else
    {
        aErr = isc_dsql_prepare(m_statusVector,
                                &m_pConnection->getTransaction(),
                                &m_aStatementHandle,
                                0,
                                OUStringToOString(sql, RTL_TEXTENCODING_UTF8).getStr(),
                                FIREBIRD_SQL_DIALECT,
                                pInSqlda);
    }

    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_prepare failed");
        return aErr;
    }
    else
    {
        aErr = isc_dsql_describe(m_statusVector,
                                 &m_aStatementHandle,
                                 1,
                                 pOutSqlda);
    }

    // Ensure we have enough space in pOutSqlda
    if (aErr)
    {
        SAL_WARN("connectivity.firebird", "isc_dsql_describe failed");
        return aErr;
    }
    else if (!aErr && (pOutSqlda->sqld > pOutSqlda->sqln))
    {
        int n = pOutSqlda->sqld;
        free(pOutSqlda);
        pOutSqlda = (XSQLDA*) malloc(XSQLDA_LENGTH(n));
        pOutSqlda->version = SQLDA_VERSION1;
        aErr = isc_dsql_describe(m_statusVector,
                                 &m_aStatementHandle,
                                 1,
                                 pOutSqlda);
    }

    // Process each XSQLVAR parameter structure in the output XSQLDA
    if (aErr)
    {
        SAL_WARN("connectivity.firebird","isc_dsql_describe failed when resizing pOutSqlda");
        return aErr;
    }
    else
    {
        mallocSQLVAR(pOutSqlda);
    }

    return aErr;
}

// ---- XMultipleResults - UNSUPPORTED ----------------------------------------
uno::Reference< XResultSet > SAL_CALL OStatementCommonBase::getResultSet() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
//     return uno::Reference< XResultSet >();
    MutexGuard aGuard(m_pConnection->getMutex());
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return m_xResultSet;
}

sal_Bool SAL_CALL OStatementCommonBase::getMoreResults() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return sal_False;
//     MutexGuard aGuard( m_aMutex );
//     checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
}

sal_Int32 SAL_CALL OStatementCommonBase::getUpdateCount() throw(SQLException, RuntimeException)
{
    // TODO: verify we really can't support this
    return 0;
}


// ---- XWarningsSupplier - UNSUPPORTED ----------------------------------------
Any SAL_CALL OStatementCommonBase::getWarnings() throw(SQLException, RuntimeException)
{
    return Any();
}

void SAL_CALL OStatementCommonBase::clearWarnings() throw(SQLException, RuntimeException)
{
}

::cppu::IPropertyArrayHelper* OStatementCommonBase::createArrayHelper( ) const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  OUString);
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


::cppu::IPropertyArrayHelper & OStatementCommonBase::getInfoHelper()
{
    return *const_cast<OStatementCommonBase*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OStatementCommonBase::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
    throw (IllegalArgumentException)
{
    (void) rConvertedValue;
    (void) rOldValue;
    (void) nHandle;
    (void) rValue;
    sal_Bool bConverted = sal_False;
    // here we have to try to convert
    return bConverted;
}
// -------------------------------------------------------------------------
void OStatementCommonBase::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    (void) rValue;
    // set the value to what ever is necessary
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
void OStatementCommonBase::getFastPropertyValue(Any& rValue,sal_Int32 nHandle) const
{
    (void) rValue;
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

void SAL_CALL OStatementCommonBase::acquire() throw()
{
    OStatementCommonBase_Base::acquire();
}

void SAL_CALL OStatementCommonBase::release() throw()
{
    OStatementCommonBase_Base::release();
}

uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OStatementCommonBase::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

bool OStatementCommonBase::isDDLStatement(isc_stmt_handle& aStatementHandle)
    throw (SQLException)
{
    ISC_STATUS_ARRAY aStatusVector;
    ISC_STATUS aErr;

    char aInfoItems[] = {isc_info_sql_stmt_type};
    char aResultsBuffer[8];

    aErr = isc_dsql_sql_info(aStatusVector,
                             &aStatementHandle,
                             sizeof(aInfoItems),
                             aInfoItems,
                             sizeof(aResultsBuffer),
                             aResultsBuffer);

    if (!aErr && aResultsBuffer[0] == isc_info_sql_stmt_type)
    {
        const short aBytes = (short) isc_vax_integer(aResultsBuffer+1, 2);
        const short aStatementType = (short) isc_vax_integer(aResultsBuffer+3, aBytes);
        if (aStatementType == isc_info_sql_stmt_ddl)
            return true;
    }
    evaluateStatusVector(aStatusVector,
                         "isc_dsq_sql_info",
                         *this);
    return false;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
