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

#include <sal/log.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <propertyids.hxx>
#include <vcl/svapp.hxx>
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
using namespace ::std;

OStatementCommonBase::OStatementCommonBase(Connection* _pConnection)
    : OStatementCommonBase_Base(m_aMutex),
      OPropertySetHelper(OStatementCommonBase_Base::rBHelper),
      m_pConnection(_pConnection),
#if SAL_TYPES_SIZEOFPOINTER == 8
      m_aStatementHandle(0)
#else
      m_aStatementHandle(nullptr)
#endif
{
}

OStatementCommonBase::~OStatementCommonBase()
{
}

void OStatementCommonBase::disposeResultSet()
{
    uno::Reference< XComponent > xComp(m_xResultSet.get(), UNO_QUERY);
    if (xComp.is())
        xComp->dispose();
    m_xResultSet.clear();
}

void OStatementCommonBase::freeStatementHandle()
{
    if (m_aStatementHandle)
    {
        isc_dsql_free_statement(m_statusVector,
                                &m_aStatementHandle,
                                DSQL_drop);
        evaluateStatusVector(m_statusVector,
                             "isc_dsql_free_statement",
                             *this);
    }
}


Any SAL_CALL OStatementCommonBase::queryInterface( const Type & rType )
{
    Any aRet = OStatementCommonBase_Base::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL OStatementCommonBase::getTypes(  )
{
    ::cppu::OTypeCollection aTypes(
        ::cppu::UnoType<XMultiPropertySet>::get(),
        ::cppu::UnoType<XFastPropertySet>::get(),
        ::cppu::UnoType<XPropertySet>::get());

    return concatSequences(aTypes.getTypes(),OStatementCommonBase_Base::getTypes());
}


void SAL_CALL OStatementCommonBase::cancel(  )
{
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
    // cancel the current sql statement
}

void SAL_CALL OStatementCommonBase::close()
{
    SAL_INFO("connectivity.firebird", "close");

    {
        MutexGuard aGuard(m_aMutex);
        checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
        disposeResultSet();
        freeStatementHandle();
    }

    dispose();
}

void OStatementCommonBase::prepareAndDescribeStatement(const OUString& sql,
                                                      XSQLDA*& pOutSqlda,
                                                      XSQLDA* pInSqlda)
{
    SolarMutexGuard g; // tdf#122129

    freeStatementHandle();

    if (!pOutSqlda)
    {
        pOutSqlda = static_cast<XSQLDA*>(calloc(1, XSQLDA_LENGTH(10)));
        pOutSqlda->version = SQLDA_VERSION1;
        pOutSqlda->sqln = 10;
    }

    ISC_STATUS aErr = 0;

    aErr = isc_dsql_allocate_statement(m_statusVector,
                                       &m_pConnection->getDBHandle(),
                                       &m_aStatementHandle);

    if (aErr)
    {
        evaluateStatusVector(m_statusVector,
                             "isc_dsql_allocate_statement",
                             *this);
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

        if (aErr)
        {
            evaluateStatusVector(m_statusVector,
                                 "isc_dsql_prepare",
                                 *this);
        }
        else
        {
            aErr = isc_dsql_describe(m_statusVector,
                                     &m_aStatementHandle,
                                     1,
                                     pOutSqlda);

            if (aErr)
            {
                // TODO: free statement handle, etc.?
                evaluateStatusVector(m_statusVector,
                                     "isc_dsql_describe",
                                     *this);
            }
            else
            {
                // Ensure we have enough space in pOutSqlda
                if (pOutSqlda->sqld > pOutSqlda->sqln)
                {
                    int n = pOutSqlda->sqld;
                    free(pOutSqlda);
                    pOutSqlda = static_cast<XSQLDA*>(calloc(1, XSQLDA_LENGTH(n)));
                    pOutSqlda->version = SQLDA_VERSION1;
                    pOutSqlda->sqln = n;
                    aErr = isc_dsql_describe(m_statusVector,
                                             &m_aStatementHandle,
                                             1,
                                             pOutSqlda);
                }

                // Process each XSQLVAR parameter structure in the output XSQLDA
                if (aErr)
                {
                    evaluateStatusVector(m_statusVector,
                                         "isc_dsql_describe",
                                         *this);
                }
                else
                {
                    mallocSQLVAR(pOutSqlda);
                }
            }
        }
        if(aErr)
        {
            freeStatementHandle();
        }
    }
    if(aErr)
    {
        free(pOutSqlda);
        pOutSqlda = nullptr;
    }
}

// ---- XMultipleResults - UNSUPPORTED ----------------------------------------
uno::Reference< XResultSet > SAL_CALL OStatementCommonBase::getResultSet()
{
    // TODO: verify we really can't support this
//     return uno::Reference< XResultSet >();
    MutexGuard aGuard(m_aMutex);
    checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);

    return m_xResultSet;
}

sal_Bool SAL_CALL OStatementCommonBase::getMoreResults()
{
    // TODO: verify we really can't support this
    return false;
//     MutexGuard aGuard( m_aMutex );
//     checkDisposed(OStatementCommonBase_Base::rBHelper.bDisposed);
}

sal_Int32 SAL_CALL OStatementCommonBase::getUpdateCount()
{
    // TODO: verify we really can't support this
    return -1;
}


// ---- XWarningsSupplier - UNSUPPORTED ----------------------------------------
Any SAL_CALL OStatementCommonBase::getWarnings()
{
    return Any();
}

void SAL_CALL OStatementCommonBase::clearWarnings()
{
}

::cppu::IPropertyArrayHelper* OStatementCommonBase::createArrayHelper( ) const
{
    // this properties are define by the service statement
    // they must in alphabetic order
    Sequence< Property > aProps(10);
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
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_USEBOOKMARKS),
        PROPERTY_ID_USEBOOKMARKS, cppu::UnoType<bool>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}


::cppu::IPropertyArrayHelper & OStatementCommonBase::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool OStatementCommonBase::convertFastPropertyValue(
                            Any &,
                            Any &,
                            sal_Int32,
                            const Any& )
{
    // here we have to try to convert
    return false;
}

void OStatementCommonBase::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any&)
{
    // set the value to whatever is necessary
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

void OStatementCommonBase::getFastPropertyValue(Any&,sal_Int32 nHandle) const
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

void SAL_CALL OStatementCommonBase::acquire() throw()
{
    OStatementCommonBase_Base::acquire();
}

void SAL_CALL OStatementCommonBase::release() throw()
{
    OStatementCommonBase_Base::release();
}

uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OStatementCommonBase::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

short OStatementCommonBase::getSqlInfoItem(char aInfoItem)
{
    ISC_STATUS_ARRAY aStatusVector;
    ISC_STATUS aErr;

    char aInfoItems[] = {aInfoItem};
    char aResultsBuffer[8];

    aErr = isc_dsql_sql_info(aStatusVector,
                             &m_aStatementHandle,
                             sizeof(aInfoItems),
                             aInfoItems,
                             sizeof(aResultsBuffer),
                             aResultsBuffer);

    if (!aErr && aResultsBuffer[0] == aInfoItem)
    {
        const short aBytes = static_cast<short>(isc_vax_integer(aResultsBuffer+1, 2));
        return static_cast<short>(isc_vax_integer(aResultsBuffer+3, aBytes));
    }

    evaluateStatusVector(aStatusVector,
                         "isc_dsq_sql_info",
                         *this);
    return 0;
}

bool OStatementCommonBase::isDDLStatement()
{
    return getSqlInfoItem(isc_info_sql_stmt_type) == isc_info_sql_stmt_ddl;
}

sal_Int32 OStatementCommonBase::getStatementChangeCount()
{
    const short aStatementType = getSqlInfoItem(isc_info_sql_stmt_type);


    ISC_STATUS_ARRAY aStatusVector;
    ISC_STATUS aErr;

    // This is somewhat undocumented so I'm just guessing and hoping for the best.
    char aInfoItems[] = {isc_info_sql_records};
    char aResultsBuffer[1024];

    aErr = isc_dsql_sql_info(aStatusVector,
                             &m_aStatementHandle,
                             sizeof(aInfoItems),
                             aInfoItems,
                             sizeof(aResultsBuffer),
                             aResultsBuffer);

    if (aErr)
    {
        evaluateStatusVector(aStatusVector,
                             "isc_dsq_sql_info",
                             *this);
        return 0;
    }

    short aDesiredInfoType = 0;
    switch (aStatementType)
    {
        case isc_info_sql_stmt_select:
            aDesiredInfoType = isc_info_req_select_count;
            break;
        case isc_info_sql_stmt_insert:
            aDesiredInfoType = isc_info_req_insert_count;
            break;
        case isc_info_sql_stmt_update:
            aDesiredInfoType = isc_info_req_update_count;
            break;
        case isc_info_sql_stmt_delete:
            aDesiredInfoType = isc_info_req_delete_count;
            break;
        case isc_info_sql_stmt_exec_procedure:
            return 0; // cannot determine
        default:
            throw SQLException(); // TODO: better error message?
    }

    char* pResults = aResultsBuffer;
    if (static_cast<short>(*pResults++) == isc_info_sql_records)
    {
//         const short aTotalLength = (short) isc_vax_integer(pResults, 2);
        pResults += 2;

        // Seems to be of form TOKEN (1 byte), LENGTH (2 bytes), DATA (LENGTH bytes)
        while (*pResults != isc_info_rsb_end)
        {
            const char aToken = *pResults;
            const short aLength =  static_cast<short>(isc_vax_integer(pResults+1, 2));

            if (aToken == aDesiredInfoType)
            {
                return isc_vax_integer(pResults + 3, aLength);
            }

            pResults += (3 + aLength);
        }
    }

    return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
