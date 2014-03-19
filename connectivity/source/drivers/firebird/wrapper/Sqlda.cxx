/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Sqlda.hxx"
#include "Driver.hxx"
#include "Util.hxx"

#include <assert.h>
#include <cstdlib>

#include <sal/log.hxx>

using namespace ::connectivity::firebird;
using namespace ::connectivity::firebird::wrapper;

XSQLDA* lcl_allocateSqlda(unsigned int nLength)
{
    // We specifically calloc so that all the sqlvar->sqldata pointers are null
    // allowing us to easily track whether they have been allocated or not.
    XSQLDA* pSqlda = (XSQLDA*) calloc(1, XSQLDA_LENGTH(nLength));
    pSqlda->version = SQLDA_VERSION1;
    pSqlda->sqln = nLength;
    return pSqlda;
}

void lcl_allocateSQLVAR(XSQLDA* pSqlda)
{
    XSQLVAR* pVar = pSqlda->sqlvar;
    for (int i=0; i < pSqlda->sqld; i++, pVar++)
    {
        int dtype = (pVar->sqltype & ~1); /* drop flag bit for now */
        switch(dtype) {
        case SQL_TEXT:
            pVar->sqldata = (char*) malloc(sizeof(char)*pVar->sqllen);
            break;
        case SQL_VARYING:
            // First two bytes define the length of string actually present,
            // then we can have up to sqllen bytes of text.
            pVar->sqldata = (char*) malloc(sizeof(char)*pVar->sqllen + 2);
            break;
        case SQL_SHORT:
            pVar->sqldata = (char*) malloc(sizeof(sal_Int16));
            break;
        case SQL_LONG:
            pVar->sqldata = (char*) malloc(sizeof(sal_Int32));
            break;
        case SQL_FLOAT:
            pVar->sqldata = (char*) malloc(sizeof(float));
            break;
        case SQL_DOUBLE:
            pVar->sqldata = (char*) malloc(sizeof(double));
            break;
        case SQL_D_FLOAT:
            pVar->sqldata = (char*) malloc(sizeof(double));
            break;
        case SQL_TIMESTAMP:
            pVar->sqldata = (char*) malloc(sizeof(ISC_TIMESTAMP));
            break;
        case SQL_BLOB:
            pVar->sqldata = (char*) malloc(sizeof(ISC_QUAD));
            break;
        case SQL_ARRAY:
            assert(false); // TODO: implement
            break;
        case SQL_TYPE_TIME:
            pVar->sqldata = (char*) malloc(sizeof(ISC_TIME));
            break;
        case SQL_TYPE_DATE:
            pVar->sqldata = (char*) malloc(sizeof(ISC_DATE));
            break;
        case SQL_INT64:
            pVar->sqldata = (char *)malloc(sizeof(sal_Int64));
            break;
        case SQL_NULL:
            assert(false); // TODO: implement
            break;
        case SQL_QUAD:
            assert(false); // TODO: implement
            break;
        default:
            SAL_WARN("connectivity.firebird", "Unknown type: " << dtype);
            assert(false);
            break;
        }
        if (pVar->sqltype & 1)
        {
            /* allocate variable to hold NULL status */
            pVar->sqlind = (short*) malloc(sizeof(short));
        }
    }
}
void lcl_freeSQLVAR(XSQLDA* pSqlda)
{
    XSQLVAR* pVar = pSqlda->sqlvar;
    for (int i=0; i < pSqlda->sqld; i++, pVar++)
    {
        int dtype = (pVar->sqltype & ~1); /* drop flag bit for now */
        switch(dtype) {
        case SQL_TEXT:
        case SQL_VARYING:
        case SQL_SHORT:
        case SQL_LONG:
        case SQL_FLOAT:
        case SQL_DOUBLE:
        case SQL_D_FLOAT:
        case SQL_TIMESTAMP:
        case SQL_BLOB:
        case SQL_INT64:
        case SQL_TYPE_TIME:
        case SQL_TYPE_DATE:
            free(pVar->sqldata);
            break;
        case SQL_ARRAY:
            assert(false); // TODO: implement
            break;
        case SQL_NULL:
            assert(false); // TODO: implement
            break;
        case SQL_QUAD:
            assert(false); // TODO: implement
            break;
        default:
            SAL_WARN("connectivity.firebird", "Unknown type: " << dtype);
            assert(false);
            break;
        }

        if (pVar->sqltype & 1)
        {
            free(pVar->sqlind);
        }
    }
}

Sqlda::Sqlda()
    : mpSqlda(0)
{
}

Sqlda::~Sqlda()
{
    if (mpSqlda)
    {
        lcl_freeSQLVAR(mpSqlda);
        free(mpSqlda);
    }
}

void Sqlda::describeStatement(
    isc_stmt_handle& aStatementHandle,
    bool bDescribeBind)
{
    if (!mpSqlda)
    {
        mpSqlda = lcl_allocateSqlda(DEFAULT_SQLDA_SIZE);
    }
    else
    {
        // types might change, hence we need to completely wipe the
        // sqldatas.
        lcl_freeSQLVAR(mpSqlda);
    }

    try
    {
        ISC_STATUS_ARRAY aStatusVector;

        if (!bDescribeBind ?
            isc_dsql_describe(aStatusVector,
                              &aStatementHandle,
                              FIREBIRD_SQL_DIALECT,
                              mpSqlda)
            :
            isc_dsql_describe_bind(aStatusVector,
                              &aStatementHandle,
                              FIREBIRD_SQL_DIALECT,
                              mpSqlda))
            evaluateStatusVector(aStatusVector, "isc_dsql_describe", 0);

        // We cannot know how much space we need until after the first call of
        // isc_dsql_describe -- hence we need to check we have enough space
        // afterwards, and reallocate as necessary.
        if (mpSqlda->sqld > mpSqlda->sqln)
        {
            free(mpSqlda);
            mpSqlda = lcl_allocateSqlda(mpSqlda->sqld);
        }

        if (!bDescribeBind ?
            isc_dsql_describe(aStatusVector,
                              &aStatementHandle,
                              FIREBIRD_SQL_DIALECT,
                              mpSqlda)
            :
            isc_dsql_describe_bind(aStatusVector,
                              &aStatementHandle,
                              FIREBIRD_SQL_DIALECT,
                              mpSqlda))
            evaluateStatusVector(aStatusVector, "isc_dsql_describe", 0);

        lcl_allocateSQLVAR(mpSqlda);
    }
    catch (::com::sun::star::sdbc::SQLException e)
    {
        // We specifically free the sqlda since we are at an incomplete state
        // where the sqlda might be partially populated, and the sqlvar's
        // may or may not be allocated -- it is simplest to ensure consistency
        // by freeing the sqlda unless we have successfully described it
        // and allocated all sqlvars in it.
        free(mpSqlda);
        mpSqlda = 0;
        throw e;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */