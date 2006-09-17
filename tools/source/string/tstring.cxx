/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tstring.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:04:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef INCLUDED_RTL_INSTANCE_HXX
#include <rtl/instance.hxx>
#endif

#include <string.hxx>
#include <impstrg.hxx>

// For shared byte convert tables
#ifndef _TOOLS_TOOLSIN_HXX
#include <toolsin.hxx>
#endif

#include <debug.hxx>

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

BOOL ByteString::IsLowerAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( (*pStr >= 65) && (*pStr <= 90) )
            return FALSE;

        ++pStr,
        ++nIndex;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL ByteString::IsUpperAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( (*pStr >= 97) && (*pStr <= 122) )
            return FALSE;

        ++pStr,
        ++nIndex;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL ByteString::IsAlphaAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !(((*pStr >= 97) && (*pStr <= 122)) ||
               ((*pStr >= 65) && (*pStr <=  90))) )
            return FALSE;

        ++pStr,
        ++nIndex;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL ByteString::IsNumericAscii() const
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    sal_Int32 nIndex = 0;
    sal_Int32 nLen = mpData->mnLen;
    const sal_Char* pStr = mpData->maStr;
    while ( nIndex < nLen )
    {
        if ( !((*pStr >= 48) && (*pStr <= 57)) )
            return FALSE;

        ++pStr,
        ++nIndex;
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL ByteString::IsAlphaNumericAscii() const
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
            return FALSE;

        ++pStr,
        ++nIndex;
    }

    return TRUE;
}
