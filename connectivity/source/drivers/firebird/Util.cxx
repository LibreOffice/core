/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "Util.hxx"
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

using namespace ::connectivity;

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;

OUString firebird::sanitizeIdentifier(const OUString& rIdentifier)
{
    OUString sRet = rIdentifier.trim();
    assert(sRet.getLength() <= 31); // Firebird identifiers cannot be longer than this.

    return sRet;
}

OUString firebird::StatusVectorToString(const ISC_STATUS_ARRAY& rStatusVector,
                                    const OUString& rCause)
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
            buf.append("\n*");
            buf.append(OUString(msg, strlen(msg), RTL_TEXTENCODING_UTF8));
        }
    }
    catch (...)
    {
        SAL_WARN("connectivity.firebird", "ignore fb_interpret exception");
    }
    buf.append("\ncaused by\n'").append(rCause).append("'\n");

    OUString error = buf.makeStringAndClear();
    SAL_WARN("connectivity.firebird", error);
    return error;
}

void firebird::evaluateStatusVector(const ISC_STATUS_ARRAY& rStatusVector,
                                    const OUString& rCause,
                                    const uno::Reference< XInterface >& _rxContext)
    throw(SQLException)
{
    if (IndicatesError(rStatusVector))
    {
        OUString error = StatusVectorToString(rStatusVector, rCause);
        throw SQLException(error, _rxContext, OUString(), 1, Any());
    }
}

sal_Int32 firebird::getColumnTypeFromFBType(short aType, short aSubType)
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
        if(aSubType == 1)
            return DataType::NUMERIC;
        if(aSubType == 2)
            return DataType::DECIMAL;
        return DataType::SMALLINT;
    case SQL_LONG:
        if(aSubType == 1)
            return DataType::NUMERIC;
        if(aSubType == 2)
            return DataType::DECIMAL;
        return DataType::INTEGER;
    case SQL_FLOAT:
        return DataType::FLOAT;
    case SQL_DOUBLE:
        if(aSubType == 1)
            return DataType::NUMERIC;
        if(aSubType == 2)
            return DataType::DECIMAL;
        return DataType::DOUBLE;
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
    case SQL_INT64:
        if(aSubType == 1)
            return DataType::NUMERIC;
        if(aSubType == 2)
            return DataType::DECIMAL;
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

OUString firebird::getColumnTypeNameFromFBType(short aType, short aSubType)
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
        if(aSubType == 1)
            return OUString("SQL_NUMERIC");
        if(aSubType == 2)
            return OUString("SQL_DECIMAL");
        return OUString("SQL_SHORT");
    case SQL_LONG:
        if(aSubType == 1)
            return OUString("SQL_NUMERIC");
        if(aSubType == 2)
            return OUString("SQL_DECIMAL");
        return OUString("SQL_LONG");
    case SQL_FLOAT:
        return OUString("SQL_FLOAT");
    case SQL_DOUBLE:
        if(aSubType == 1)
            return OUString("SQL_NUMERIC");
        if(aSubType == 2)
            return OUString("SQL_DECIMAL");
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
        if(aSubType == 1)
            return OUString("SQL_NUMERIC");
        if(aSubType == 2)
            return OUString("SQL_DECIMAL");
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
        case SQL_BLOB:
            pVar->sqldata = static_cast<char*>(malloc(sizeof(ISC_QUAD)));
            break;
        case SQL_ARRAY:
            assert(false); // TODO: implement
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
        case SQL_BLOB:
        case SQL_INT64:
        case SQL_TYPE_TIME:
        case SQL_TYPE_DATE:
            if(pVar->sqldata)
            {
                free(pVar->sqldata);
                pVar->sqldata = nullptr;
            }
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

static bool isWhitespace( sal_Unicode c )
{
    return ' ' == c || 9 == c || 10 == c || 13 == c;
}

static bool isOperator( char c )
{
    bool ret;
    switch(c)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '<':
    case '>':
    case '=':
    case '~':
    case '!':
    case '@':
    case '#':
    case '%':
    case '^':
    case '&':
    case '|':
    case '`':
    case '?':
    case '$':
        ret = true;
        break;
    default:
        ret = false;
    }
    return ret;
}

void firebird::tokenizeSQL( const OString & sql, OStringVector &vec  )
{
    int length = sql.getLength();

    int i = 0;
    bool singleQuote = false;
    bool doubleQuote = false;
    int start = 0;
    for( ; i < length ; i ++ )
    {
        char c = sql[i];
        if( doubleQuote  )
        {
            if( '"' == c )
            {
                vec.push_back( OString( &sql.getStr()[start], i-start  ) );
                start = i + 1;
                doubleQuote = false;
            }
        }
        else if( singleQuote )
        {
            if( '\'' == c )
            {
                vec.push_back( OString( &sql.getStr()[start], i - start +1 ) );
                start = i + 1; // leave single quotes !
                singleQuote = false;
            }
        }
        else
        {
            if( '"' == c )
            {
                doubleQuote = true;
                start = i +1; // skip double quotes !
            }
            else if( '\'' == c )
            {
                singleQuote = true;
                start = i; // leave single quotes
            }
            else if( isWhitespace( c ) )
            {
                if( i == start )
                    start ++;   // skip additional whitespace
                else
                {
                    vec.push_back( OString( &sql.getStr()[start], i - start  ) );
                    start = i +1;
                }
            }
            else if( ',' == c || isOperator( c ) || '(' == c || ')' == c )
            {
                if( i - start )
                    vec.push_back( OString( &sql.getStr()[start], i - start ) );
                vec.push_back( OString( &sql.getStr()[i], 1 ) );
                start = i + 1;
            }
            else if( '.' == c )
            {
                if( ( i > start && sql[start] >= '0' && sql[start] <= '9' ) ||
                    ( i == start && i > 1 && isWhitespace( sql[i-1] ) ) )
                {
                    // ignore, is a literal
                }
                else
                {
                    if( i - start )
                        vec.push_back( OString( &sql.getStr()[start], i - start ) );
                    vec.push_back( OString( "." ) );
                    start = i + 1;
                }
            }
        }
    }
    if( start < i )
        vec.push_back( OString( &sql.getStr()[start] , i - start ) );
}

OString firebird::extractSingleTableFromSelect( const OStringVector &vec )
{
    OString ret;

    if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
            vec[0].pData->buffer, vec[0].pData->length, "select" , 6 , 6 ) )
    {
        size_t token = 0;

        for( token = 1; token < vec.size() ; token ++ )
        {
            if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                    vec[token].getStr(), vec[token].getLength(), "from" , 4 , 4 ) )
            {
                // found from
                break;
            }
        }
        token ++;

        if( token < vec.size() && 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                vec[token].pData->buffer, vec[token].pData->length, "only " , 4 , 4 ) )
        {
            token ++;
        }

        if( token < vec.size() && vec[token] != "(" )
        {
            // it is a table or a function name
            OStringBuffer buf(128);
            if( '"' == vec[token][0] )
                buf.append( &(vec[token].getStr()[1]) , vec[token].getLength() -2 );
            else
                buf.append( vec[token] );
            token ++;

            if( token < vec.size() )
            {
                if( vec[token] == "." )
                {
                    buf.append( vec[token] );
                    token ++;
                    if( token < vec.size() )
                    {
                        if( '"' == vec[token][0] )
                            buf.append( &(vec[token].getStr()[1]) , vec[token].getLength() -2 );
                        else
                            buf.append( vec[token] );
                        token ++;
                    }
                }
            }

            ret = buf.makeStringAndClear();
            // now got my table candidate

            if( token < vec.size() && vec[token] == "(" )
            {
                // whoops, it is a function
                ret.clear();
            }
            else
            {
                if( token < vec.size() )
                {
                    if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                            vec[token].pData->buffer, vec[token].pData->length, "as" , 2, 2 ) )
                    {
                        token += 2; // skip alias
                    }
                }

                if( token < vec.size() )
                {
                    if( vec[token] == "," )
                    {
                        // whoops, multiple tables are used
                        ret.clear();
                    }
                    else
                    {
                        static const char * forbiddenKeywords[] =
                            { "join", "natural", "outer", "inner", "left", "right", "full" , nullptr };
                        for( int i = 0 ; forbiddenKeywords[i] ; i ++ )
                        {
                            size_t nKeywordLen = strlen(forbiddenKeywords[i]);
                            if( 0 == rtl_str_shortenedCompareIgnoreAsciiCase_WithLength(
                                 vec[token].pData->buffer, vec[token].pData->length,
                                 forbiddenKeywords[i], nKeywordLen,
                                 nKeywordLen ) )
                            {
                                // whoops, it is a join
                                ret.clear();
                            }
                        }
                    }
                }
            }
        }
    }
    return ret;

}

OUString firebird::escapeWith( const OUString& sText, const char aKey, const char aEscapeChar)
{
    OUString sRet(sText);
    sal_Int32 aIndex = 0;
    while( (aIndex = sRet.indexOf(aKey, aIndex)) > 0 &&
            aIndex < sRet.getLength())
    {
            sRet = sRet.replaceAt(aIndex, 1, OUString(aEscapeChar) + OUString(aKey)  );
            aIndex+= 2;
    }

    return sRet;
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
