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

#include <tools/debug.hxx>

// =======================================================================

DBG_NAME( UniString )
DBG_NAMEEX( ByteString )

// -----------------------------------------------------------------------

#define STRCODE         sal_Unicode
#define STRCODEU        sal_Unicode
#define STRING          UniString
#define STRINGDATA      UniStringData
#define DBGCHECKSTRING  DbgCheckUniString
#define STRING_TYPE     rtl_uString
#define STRING_ACQUIRE  rtl_uString_acquire
#define STRING_RELEASE  rtl_uString_release
#define STRING_NEW      rtl_uString_new

// -----------------------------------------------------------------------

#include <strimp.cxx>
#include <strucvt.cxx>
#include <strascii.cxx>

UniString::UniString(char c): mpData(ImplAllocData(1)) { mpData->maStr[0] = c; }

// -----------------------------------------------------------------------

UniString UniString::CreateFromInt32( sal_Int32 n, sal_Int16 nRadix )
{
    sal_Unicode aBuf[RTL_USTR_MAX_VALUEOFINT32];
    BOOST_STATIC_ASSERT(RTL_USTR_MAX_VALUEOFINT32 <= STRING_MAXLEN);
    return UniString(
        aBuf,
        static_cast< xub_StrLen >(rtl_ustr_valueOfInt32( aBuf, n, nRadix )) );
}

// -----------------------------------------------------------------------

UniString UniString::CreateFromInt64( sal_Int64 n, sal_Int16 nRadix )
{
    sal_Unicode aBuf[RTL_USTR_MAX_VALUEOFINT64];
    BOOST_STATIC_ASSERT(RTL_USTR_MAX_VALUEOFINT64 <= STRING_MAXLEN);
    return UniString(
        aBuf,
        static_cast< xub_StrLen >(rtl_ustr_valueOfInt64( aBuf, n, nRadix )) );
}

// -----------------------------------------------------------------------

UniString UniString::CreateFromFloat( float f )
{
    sal_Unicode aBuf[RTL_USTR_MAX_VALUEOFFLOAT];
    BOOST_STATIC_ASSERT(RTL_USTR_MAX_VALUEOFFLOAT <= STRING_MAXLEN);
    return UniString(
        aBuf, static_cast< xub_StrLen >(rtl_ustr_valueOfFloat( aBuf, f )) );
}

// -----------------------------------------------------------------------

UniString UniString::CreateFromDouble( double d )
{
    sal_Unicode aBuf[RTL_USTR_MAX_VALUEOFDOUBLE];
    BOOST_STATIC_ASSERT(RTL_USTR_MAX_VALUEOFDOUBLE <= STRING_MAXLEN);
    return UniString(
        aBuf, static_cast< xub_StrLen >(rtl_ustr_valueOfDouble( aBuf, d )) );
}

// -----------------------------------------------------------------------

namespace { struct Empty : public rtl::Static< const UniString, Empty> {}; }
const UniString& UniString::EmptyString()
{
    return Empty::get();
}

// -----------------------------------------------------------------------

sal_Int32 UniString::ToInt32() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toInt32( mpData->maStr, 10 );
}

// -----------------------------------------------------------------------

sal_Int64 UniString::ToInt64() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toInt64( mpData->maStr, 10 );
}

// -----------------------------------------------------------------------

float UniString::ToFloat() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toFloat( mpData->maStr );
}

// -----------------------------------------------------------------------

double UniString::ToDouble() const
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    return rtl_ustr_toDouble( mpData->maStr );
}

