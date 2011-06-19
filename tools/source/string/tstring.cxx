/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include <string.h>

#include "boost/static_assert.hpp"

#include "osl/diagnose.h"
#include <osl/interlck.h>
#include <rtl/alloc.h>
#include <rtl/memory.h>
#include <rtl/tencinfo.h>
#include <rtl/instance.hxx>

#include <tools/string.hxx>
#include <impstrg.hxx>

// For shared byte convert tables
#include <toolsin.hxx>

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

    OSL_FAIL("ByteString::ToFloat unusable");
    return 0;
}

// -----------------------------------------------------------------------

double ByteString::ToDouble() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    OSL_FAIL("ByteString::ToDouble unusable");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
