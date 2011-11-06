/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <string.h>

#include "boost/static_assert.hpp"

#include "osl/diagnose.h"
#ifndef _OSL_INTERLCK_H
#include <osl/interlck.h>
#endif
#ifndef _RTL_ALLOC_H
#include <rtl/alloc.h>
#endif
#ifndef _RTL_MEMORY_H
#include <rtl/memory.h>
#endif
#include <rtl/tencinfo.h>
#include <rtl/instance.hxx>

#include <tools/string.hxx>
#include <impstrg.hxx>

// For shared byte convert tables
#ifndef _TOOLS_TOOLSIN_HXX
#include <toolsin.hxx>
#endif

#include <tools/debug.hxx>

// =======================================================================

DBG_NAME( ByteString )
DBG_NAMEEX( UniString )

// -----------------------------------------------------------------------

#define STRCODE         sal_Char
#define STRCODEU        unsigned char
#define STRING          ByteString
#define STRINGDATA      ByteStringData
#define DBGCHECKSTRING  DbgCheckByteString
#define STRING_TYPE     rtl_String
#define STRING_ACQUIRE  rtl_string_acquire
#define STRING_RELEASE  rtl_string_release
#define STRING_NEW      rtl_string_new


// -----------------------------------------------------------------------

xub_StrLen ImplStringLen( const sal_Char* pStr )
{
    const sal_Char* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

// -----------------------------------------------------------------------

xub_StrLen ImplStringLen( const sal_Unicode* pStr )
{
    const sal_Unicode* pTempStr = pStr;
    while( *pTempStr )
        ++pTempStr;
    return (xub_StrLen)(pTempStr-pStr);
}

// -----------------------------------------------------------------------

#include <strimp.cxx>
#include <strcvt.cxx>

// -----------------------------------------------------------------------

ByteString ByteString::CreateFromInt32( sal_Int32 n, sal_Int16 nRadix )
{
    sal_Char aBuf[RTL_STR_MAX_VALUEOFINT32];
    BOOST_STATIC_ASSERT(RTL_STR_MAX_VALUEOFINT32 <= STRING_MAXLEN);
    return ByteString(
        aBuf,
        static_cast< xub_StrLen >(rtl_str_valueOfInt32( aBuf, n, nRadix )) );
}

// -----------------------------------------------------------------------

ByteString ByteString::CreateFromInt64( sal_Int64 n, sal_Int16 nRadix )
{
    sal_Char aBuf[RTL_STR_MAX_VALUEOFINT64];
    BOOST_STATIC_ASSERT(RTL_STR_MAX_VALUEOFINT64 <= STRING_MAXLEN);
    return ByteString(
        aBuf,
        static_cast< xub_StrLen >(rtl_str_valueOfInt64( aBuf, n, nRadix )) );
}

// -----------------------------------------------------------------------

ByteString ByteString::CreateFromFloat( float f )
{
    sal_Char aBuf[RTL_STR_MAX_VALUEOFFLOAT];
    BOOST_STATIC_ASSERT(RTL_STR_MAX_VALUEOFFLOAT <= STRING_MAXLEN);
    return ByteString(
        aBuf, static_cast< xub_StrLen >(rtl_str_valueOfFloat( aBuf, f )) );
}

// -----------------------------------------------------------------------

ByteString ByteString::CreateFromDouble( double d )
{
    sal_Char aBuf[RTL_STR_MAX_VALUEOFDOUBLE];
    BOOST_STATIC_ASSERT(RTL_STR_MAX_VALUEOFDOUBLE <= STRING_MAXLEN);
    return ByteString(
        aBuf, static_cast< xub_StrLen >(rtl_str_valueOfDouble( aBuf, d )) );
}

// -----------------------------------------------------------------------

namespace { struct Empty : public rtl::Static< const ByteString, Empty> {}; }
const ByteString& ByteString::EmptyString()
{
    return Empty::get();
}

// -----------------------------------------------------------------------

sal_Int32 ByteString::ToInt32() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    return atoi( mpData->maStr );
}

// -----------------------------------------------------------------------

sal_Int64 ByteString::ToInt64() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    return atoi( mpData->maStr );
}

// -----------------------------------------------------------------------

float ByteString::ToFloat() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    OSL_ENSURE(false, "ByteString::ToFloat unusable");
    return 0;
}

// -----------------------------------------------------------------------

double ByteString::ToDouble() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    OSL_ENSURE(false, "ByteString::ToDouble unusable");
    return 0;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsLowerAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( (*pStr >= 65) && (*pStr <= 90) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsUpperAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( (*pStr >= 97) && (*pStr <= 122) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsAlphaAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !(((*pStr >= 97) && (*pStr <= 122)) ||
               ((*pStr >= 65) && (*pStr <=  90))) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsNumericAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !((*pStr >= 48) && (*pStr <= 57)) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}

// -----------------------------------------------------------------------

sal_Bool ByteString::IsAlphaNumericAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !(((*pStr >= 97) && (*pStr <= 122)) ||
               ((*pStr >= 65) && (*pStr <=  90)) ||
               ((*pStr >= 48) && (*pStr <=  57))) )
            return sal_False;

        ++pStr,
        ++nIndex;
    }

    return sal_True;
}
