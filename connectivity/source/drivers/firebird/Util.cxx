/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Util.hxx"

using namespace ::connectivity;

using namespace ::rtl;

using namespace ::com::sun::star::sdbc;

sal_Int32 firebird::getColumnTypeFromFBType(short aType)
{
    aType &= ~1; // Remove last bit -- it is used to denote whether column
                 // can store Null, not needed for type determination
    switch (aType)
    {
    case SQL_TEXT:
        return DataType::CHAR;
    case SQL_VARYING:
        return DataType::VARCHAR;
    case SQL_SHORT:
        return DataType::SMALLINT;
    case SQL_LONG:
        return DataType::INTEGER;
    case SQL_FLOAT:
        return DataType::REAL;
    case SQL_DOUBLE:
        return DataType::DOUBLE;
    case SQL_D_FLOAT:
        return DataType::FLOAT;
    case SQL_TIMESTAMP:
        return DataType::TIMESTAMP;
    case SQL_BLOB:
        return DataType::BLOB;
    case SQL_ARRAY:
        return DataType::ARRAY;
    case SQL_TYPE_TIME:
        return DataType::TIME;
    case SQL_TYPE_DATE:
        return DataType::DATE;
    case SQL_INT64:
        return DataType::BIGINT;
    case SQL_NULL:
        return DataType::SQLNULL;
    case SQL_QUAD:      // Is a "Blob ID" according to the docs
        return 0;       // TODO: verify
    default:
        assert(false); // Should never happen
        return 0;
    }
}

OUString firebird::getColumnTypeNameFromFBType(short aType)
{
    aType &= ~1; // Remove last bit -- it is used to denote whether column
                // can store Null, not needed for type determination
    switch (aType)
    {
    case SQL_TEXT:
        return OUString("SQL_TEXT");
    case SQL_VARYING:
        return OUString("SQL_VARYING");
    case SQL_SHORT:
        return OUString("SQL_SHORT");
    case SQL_LONG:
        return OUString("SQL_LONG");
    case SQL_FLOAT:
        return OUString("SQL_FLOAT");
    case SQL_DOUBLE:
        return OUString("SQL_DOUBLE");
    case SQL_D_FLOAT:
        return OUString("SQL_D_FLOAT");
    case SQL_TIMESTAMP:
        return OUString("SQL_TIMESTAMP");
    case SQL_BLOB:
        return OUString("SQL_BLOB");
    case SQL_ARRAY:
        return OUString("SQL_ARRAY");
    case SQL_TYPE_TIME:
        return OUString("SQL_TYPE_TIME");
    case SQL_TYPE_DATE:
        return OUString("SQL_TYPE_DATE");
    case SQL_INT64:
        return OUString("SQL_INT64");
    case SQL_NULL:
        return OUString("SQL_NULL");
    case SQL_QUAD:
        return OUString("SQL_QUAD");
    default:
        assert(false); // Should never happen
        return OUString();
    }
}

short firebird::getFBTypeFromBlrType(short blrType)
{
    switch (blrType)
    {
    case blr_text:
        return SQL_TEXT;
    case blr_text2:
        assert(false);
        return 0; // No idea if this should be supported
    case blr_varying:
        return SQL_VARYING;
    case blr_varying2:
        assert(false);
        return 0; // No idea if this should be supported
    case blr_short:
        return SQL_SHORT;
    case blr_long:
        return SQL_LONG;
    case blr_float:
        return SQL_FLOAT;
    case blr_double:
        return SQL_DOUBLE;
    case blr_d_float:
        return SQL_D_FLOAT;
    case blr_timestamp:
        return SQL_TIMESTAMP;
    case blr_blob:
        return SQL_BLOB;
//     case blr_SQL_ARRAY:
//         return OUString("SQL_ARRAY");
    case blr_sql_time:
        return SQL_TYPE_TIME;
    case blr_sql_date:
        return SQL_TYPE_DATE;
    case blr_int64:
        return SQL_INT64;
//     case SQL_NULL:
//         return OUString("SQL_NULL");
    case blr_quad:
        return SQL_QUAD;
    default:
        // If this happens we have hit one of the extra types in ibase.h
        // look up blr_* for a list, e.g. blr_domain_name, blr_not_nullable etc.
        assert(false);
        return 0;
    }
}

void firebird::mallocSQLVAR(XSQLDA* pSqlda)
{
    // TODO: confirm the sizings below.
    XSQLVAR* pVar = pSqlda->sqlvar;
    for (int i=0; i < pSqlda->sqld; i++, pVar++)
    {
        int dtype = (pVar->sqltype & ~1); /* drop flag bit for now */
        switch(dtype) {
        case SQL_TEXT:
            pVar->sqldata = (char *)malloc(sizeof(char)*pVar->sqllen);
            break;
        case SQL_VARYING:
            pVar->sqltype = SQL_TEXT;
            pVar->sqldata = (char *)malloc(sizeof(char)*pVar->sqllen);
            break;
        case SQL_SHORT:
            pVar->sqldata = (char *)malloc(sizeof(short));
            break;
        case SQL_LONG:
            pVar->sqldata = (char *)malloc(sizeof(long));
            break;
        case SQL_FLOAT:
            pVar->sqldata = (char *)malloc(sizeof(double));
            break;
        case SQL_DOUBLE:
            pVar->sqldata = (char *)malloc(sizeof(double));
            break;
        case SQL_D_FLOAT:
            pVar->sqldata = (char *)malloc(sizeof(double));
            break;
        case SQL_TIMESTAMP:
            pVar->sqldata = (char *)malloc(sizeof(time_t));
            break;
        case SQL_BLOB:
            pVar->sqldata = (char*) malloc(sizeof(ISC_QUAD));
            break;
        case SQL_ARRAY:
            assert(false); // TODO: implement
            break;
        case SQL_TYPE_TIME:
            assert(false); // TODO: implement
            break;
        case SQL_TYPE_DATE:
            assert(false); // TODO: implement
            break;
        case SQL_INT64:
            pVar->sqldata = (char *)malloc(sizeof(int));
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
            pVar->sqlind = (short *)malloc(sizeof(short));
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */