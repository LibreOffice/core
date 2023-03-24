/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Util.hxx"
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <o3tl/string_view.hxx>

using namespace ::connectivity;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

using namespace firebird;

OUString firebird::sanitizeIdentifier(std::u16string_view rIdentifier)
{
    std::u16string_view sRet = o3tl::trim(rIdentifier);
    assert(sRet.size() <= 31); // Firebird identifiers cannot be longer than this.

    return OUString(sRet);
}

OUString firebird::StatusVectorToString(const ISC_STATUS_ARRAY& rStatusVector,
                                    std::u16string_view rCause)
{
    OUStringBuffer buf;
    const ISC_STATUS* pStatus = reinterpret_cast<const ISC_STATUS*>(&rStatusVector);

    buf.append("firebird_sdbc error:");
    try
    {
        char msg[512]; // Size is based on suggestion in docs.
        while(fb_interpret(msg, sizeof(msg), &pStatus))
        {
            // TODO: verify encoding
            buf.append("\n*"
                + OUString(msg, strlen(msg), RTL_TEXTENCODING_UTF8));
        }
    }
    catch (...)
    {
        SAL_WARN("connectivity.firebird", "ignore fb_interpret exception");
    }
    buf.append(OUString::Concat("\ncaused by\n'") + rCause + "'\n");

    OUString error = buf.makeStringAndClear();
    SAL_WARN("connectivity.firebird", error);
    return error;
}

void firebird::evaluateStatusVector(const ISC_STATUS_ARRAY& rStatusVector,
                                    std::u16string_view rCause,
                                    const uno::Reference< XInterface >& _rxContext)
{
    if (IndicatesError(rStatusVector))
    {
        OUString error = StatusVectorToString(rStatusVector, rCause);
        throw SQLException(error, _rxContext, OUString(), 1, Any());
    }
}

static sal_Int32 lcl_getNumberType( short aType, NumberSubType aSubType )
{
    switch(aSubType)
    {
        case NumberSubType::Numeric:
            return DataType::NUMERIC;
        case NumberSubType::Decimal:
            return DataType::DECIMAL;
        default:
            switch(aType)
            {
                case SQL_SHORT:
                    return DataType::SMALLINT;
                case SQL_LONG:
                    return DataType::INTEGER;
                case SQL_DOUBLE:
                    return DataType::DOUBLE;
                case SQL_INT64:
                    return DataType::BIGINT;
                default:
                    assert(false); // not a number
                    return 0;
            }
    }
}
static sal_Int32 lcl_getCharColumnType( short aType, std::u16string_view sCharset )
{
    switch(aType)
    {
        case SQL_TEXT:
            if( sCharset == u"OCTETS")
                return DataType::BINARY;
            else
                return DataType::CHAR;
        case SQL_VARYING:
            if( sCharset == u"OCTETS")
                return DataType::VARBINARY;
            else
                return DataType::VARCHAR;
        default:
            assert(false);
            return 0;
    }
}

sal_Int32 firebird::ColumnTypeInfo::getSdbcType() const
{
    short aType = m_aType & ~1; // Remove last bit -- it is used to denote whether column
                 // can store Null, not needed for type determination
    short aSubType = m_aSubType;
    if( m_nScale > 0 )
    {
        // numeric / decimal
        if(aType == SQL_SHORT || aType == SQL_LONG || aType == SQL_DOUBLE
                || aType == SQL_INT64)
        {
            // if scale is set without subtype then imply numeric
            if( static_cast<NumberSubType>(aSubType) == NumberSubType::Other )
                aSubType = static_cast<short>(NumberSubType::Numeric);
        }
    }

    switch (aType)
    {
    case SQL_TEXT:
    case SQL_VARYING:
        return lcl_getCharColumnType(aType, m_sCharsetName);
    case SQL_SHORT:
    case SQL_LONG:
    case SQL_DOUBLE:
    case SQL_INT64:
        return lcl_getNumberType(aType, static_cast<NumberSubType>(aSubType) );
    case SQL_FLOAT:
        return DataType::FLOAT;
    case SQL_D_FLOAT:
        return DataType::DOUBLE;
    case SQL_TIMESTAMP:
        return DataType::TIMESTAMP;
    case SQL_BLOB:
        switch (static_cast<BlobSubtype>(aSubType))
        {
            case BlobSubtype::Blob:
                return DataType::BLOB;
            case BlobSubtype::Clob:
                return DataType::CLOB;
            case BlobSubtype::Image:
                return DataType::LONGVARBINARY;
            default:
                SAL_WARN("connectivity.firebird", "Unknown subtype for Blob type: " << aSubType);
                assert(!"Unknown subtype for Blob type"); // Should never happen
                return 0;
        }
    case SQL_ARRAY:
        return DataType::ARRAY;
    case SQL_TYPE_TIME:
        return DataType::TIME;
    case SQL_TYPE_DATE:
        return DataType::DATE;
    case SQL_NULL:
        return DataType::SQLNULL;
    case SQL_QUAD:      // Is a "Blob ID" according to the docs
        return 0;       // TODO: verify
    case SQL_BOOLEAN:
        return DataType::BOOLEAN;
    default:
        assert(false); // Should never happen
        return 0;
    }
}

OUString firebird::ColumnTypeInfo::getColumnTypeName() const
{
    sal_Int32 nDataType = this->getSdbcType();
    switch (nDataType)
    {
        case DataType::BIT:
            return "BIT";
        case DataType::TINYINT:
            return "TINYINT";
        case DataType::SMALLINT:
            return "SMALLINT";
        case DataType::INTEGER:
            return "INTEGER";
        case DataType::BIGINT:
            return "BIGINT";
        case DataType::FLOAT:
            return "FLOAT";
        case DataType::REAL:
            return "REAL";
        case DataType::DOUBLE:
            return "DOUBLE";
        case DataType::NUMERIC:
            return "NUMERIC";
        case DataType::DECIMAL:
            return "DECIMAL";
        case DataType::CHAR:
            return "CHAR";
        case DataType::VARCHAR:
            return "VARCHAR";
        case DataType::LONGVARCHAR:
            return "LONGVARCHAR";
        case DataType::DATE:
            return "DATE";
        case DataType::TIME:
            return "TIME";
        case DataType::TIMESTAMP:
            return "TIMESTAMP";
        case DataType::BINARY:
            // in Firebird, that is the same datatype "CHAR" as DataType::CHAR,
            // only with CHARACTER SET OCTETS; we use the synonym CHARACTER
            // to fool LO into seeing it as different types.
            return "CHARACTER";
        case DataType::VARBINARY:
            // see above comment about DataType::BINARY.
            return "CHARACTER VARYING";
        case DataType::LONGVARBINARY:
            return "BLOB SUB_TYPE " + OUString::number(static_cast<short>(BlobSubtype::Image));
        case DataType::ARRAY:
            return "ARRAY";
        case DataType::BLOB:
            return "BLOB SUB_TYPE BINARY";
        case DataType::CLOB:
            return "BLOB SUB_TYPE TEXT";
        case DataType::BOOLEAN:
            return "BOOLEAN";
        case DataType::SQLNULL:
            return "NULL";
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
    case blr_bool:
        return SQL_BOOLEAN;
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
            pVar->sqldata = static_cast<char *>(malloc(sizeof(char)*pVar->sqllen));
            break;
        case SQL_VARYING:
            pVar->sqldata = static_cast<char *>(malloc(sizeof(char)*pVar->sqllen + 2));
            break;
        case SQL_SHORT:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(sal_Int16)));
            break;
        case SQL_LONG:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(sal_Int32)));
            break;
        case SQL_FLOAT:
            pVar->sqldata = static_cast<char *>(malloc(sizeof(float)));
            break;
        case SQL_DOUBLE:
            pVar->sqldata = static_cast<char *>(malloc(sizeof(double)));
            break;
        case SQL_D_FLOAT:
            pVar->sqldata = static_cast<char *>(malloc(sizeof(double)));
            break;
        case SQL_TIMESTAMP:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(ISC_TIMESTAMP)));
            break;
        // an ARRAY is in fact a BLOB of a specialized type
        // See https://firebirdsql.org/file/documentation/reference_manuals/fblangref25-en/html/fblangref25-datatypes-bnrytypes.html#fblangref25-datatypes-array
        case SQL_ARRAY:
        case SQL_BLOB:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(ISC_QUAD)));
            break;
        case SQL_TYPE_TIME:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(ISC_TIME)));
            break;
        case SQL_TYPE_DATE:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(ISC_DATE)));
            break;
        case SQL_INT64:
            pVar->sqldata = static_cast<char *>(malloc(sizeof(sal_Int64)));
            break;
        case SQL_BOOLEAN:
            pVar->sqldata = static_cast<char *>(malloc(sizeof(sal_Bool)));
            break;
        // See https://www.firebirdsql.org/file/documentation/html/en/refdocs/fblangref25/firebird-25-language-reference.html#fblangref25-datatypes-special-sqlnull
        case SQL_NULL:
            pVar->sqldata = nullptr;
            break;
        case SQL_QUAD:
            assert(false); // TODO: implement
            break;
        default:
            SAL_WARN("connectivity.firebird", "Unknown type: " << dtype);
            assert(false);
            break;
        }
        /* allocate variable to hold NULL status */
        pVar->sqlind = static_cast<short *>(malloc(sizeof(short)));
    }
}

void firebird::freeSQLVAR(XSQLDA* pSqlda)
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
        // an ARRAY is in fact a BLOB of a specialized type
        // See https://firebirdsql.org/file/documentation/reference_manuals/fblangref25-en/html/fblangref25-datatypes-bnrytypes.html#fblangref25-datatypes-array
        case SQL_ARRAY:
        case SQL_BLOB:
        case SQL_INT64:
        case SQL_TYPE_TIME:
        case SQL_TYPE_DATE:
        case SQL_BOOLEAN:
            if(pVar->sqldata)
            {
                free(pVar->sqldata);
                pVar->sqldata = nullptr;
            }
            break;
        case SQL_NULL:
            // See SQL_NULL case in mallocSQLVAR
            assert(pVar->sqldata == nullptr);
            break;
        case SQL_QUAD:
            assert(false); // TODO: implement
            break;
        default:
            SAL_WARN("connectivity.firebird", "Unknown type: " << dtype);
//            assert(false);
            break;
        }

        if(pVar->sqlind)
        {
            free(pVar->sqlind);
            pVar->sqlind = nullptr;
        }
    }
}


sal_Int64 firebird::pow10Integer(int nDecimalCount)
{
    sal_Int64 nRet = 1;
    for(int i=0; i< nDecimalCount; i++)
    {
        nRet *= 10;
    }
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
