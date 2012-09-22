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


#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "osl/process.h"
#include "osl/diagnose.hxx"
#include "boost/bind.hpp"
#include <vector>

// define own ones, independent of OSL_DEBUG_LEVEL:
#define DEBUGBASE_ENSURE_(c, f, l, m) \
    do \
    {  \
        if (!(c) && _OSL_GLOBAL osl_assertFailedLine(f, l, m)) \
            _OSL_GLOBAL osl_breakDebug(); \
    } while (0)
#define DEBUGBASE_ENSURE(c, m) DEBUGBASE_ENSURE_(c, OSL_THIS_FILE, __LINE__, m)

namespace {

typedef std::vector<rtl::OString, rtl::Allocator<rtl::OString> > OStringVec;

struct StaticDebugBaseAddressFilter
    : rtl::StaticWithInit<OStringVec, StaticDebugBaseAddressFilter> {
    OStringVec operator()() const {
        OStringVec vec;
        rtl_uString * pStr = 0;
        rtl::OUString const name(
            RTL_CONSTASCII_USTRINGPARAM("OSL_DEBUGBASE_STORE_ADDRESSES") );
        if (osl_getEnvironment( name.pData, &pStr ) == osl_Process_E_None) {
            rtl::OUString const str(pStr);
            rtl_uString_release(pStr);
            sal_Int32 nIndex = 0;
            do {
                vec.push_back( rtl::OUStringToOString(
                                   str.getToken( 0, ';', nIndex ),
                                   RTL_TEXTENCODING_ASCII_US ) );
            }
            while (nIndex >= 0);
        }
        return vec;
    }
};

inline bool isSubStr( char const* pStr, rtl::OString const& subStr )
{
    return rtl_str_indexOfStr( pStr, subStr.getStr() ) >= 0;
}

struct DebugBaseMutex : ::rtl::Static<osl::Mutex, DebugBaseMutex> {};

} // anon namespace

extern "C" {

// These functions presumably should not be extern "C", but changing
// that would break binary compatibility.
#if SUPD < 400
#ifdef __clang__
#pragma clang diagnostic push
// Guard against slightly older clang versions that don't have
// -Wreturn-type-c-linkage...
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif
#endif
osl::Mutex & SAL_CALL osl_detail_ObjectRegistry_getMutex()
    SAL_THROW_EXTERN_C()
{
    return DebugBaseMutex::get();
}
#if SUPD < 400
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif

bool SAL_CALL osl_detail_ObjectRegistry_storeAddresses( char const* pName )
    SAL_THROW_EXTERN_C()
{
    OStringVec const& rVec = StaticDebugBaseAddressFilter::get();
    if (rVec.empty())
        return false;
    // check for "all":
    rtl::OString const& rFirst = rVec[0];
    if (rtl_str_compare_WithLength( rFirst.getStr(), rFirst.getLength(),
                                    RTL_CONSTASCII_STRINGPARAM("all") ) == 0)
        return true;
    OStringVec::const_iterator const iEnd( rVec.end() );
    return std::find_if( rVec.begin(), iEnd,
                         boost::bind( &isSubStr, pName, _1 ) ) != iEnd;
}

bool SAL_CALL osl_detail_ObjectRegistry_checkObjectCount(
    osl::detail::ObjectRegistryData const& rData, std::size_t nExpected )
    SAL_THROW_EXTERN_C()
{
    std::size_t nSize;
    if (rData.m_bStoreAddresses)
        nSize = rData.m_addresses.size();
    else
        nSize = static_cast<std::size_t>(rData.m_nCount);

    bool const bRet = (nSize == nExpected);
    if (! bRet) {
        rtl::OStringBuffer buf;
        buf.append( RTL_CONSTASCII_STRINGPARAM("unexpected number of ") );
        buf.append( rData.m_pName );
        buf.append( RTL_CONSTASCII_STRINGPARAM(": ") );
        buf.append( static_cast<sal_Int64>(nSize) );
        buf.append("; Expected: ");
        buf.append( static_cast<sal_Int64>(nExpected) );
        DEBUGBASE_ENSURE( false, buf.makeStringAndClear().getStr() );
    }
    return bRet;
}

void SAL_CALL osl_detail_ObjectRegistry_registerObject(
    osl::detail::ObjectRegistryData & rData, void const* pObj )
    SAL_THROW_EXTERN_C()
{
    if (rData.m_bStoreAddresses) {
        osl::MutexGuard const guard( osl_detail_ObjectRegistry_getMutex() );
        std::pair<osl::detail::VoidPointerSet::iterator, bool> const insertion(
            rData.m_addresses.insert(pObj) );
        DEBUGBASE_ENSURE( insertion.second, "### insertion failed!?" );
        static_cast<void>(insertion);
    }
    else {
        osl_atomic_increment(&rData.m_nCount);
    }
}

void SAL_CALL osl_detail_ObjectRegistry_revokeObject(
    osl::detail::ObjectRegistryData & rData, void const* pObj )
    SAL_THROW_EXTERN_C()
{
    if (rData.m_bStoreAddresses) {
        osl::MutexGuard const guard( osl_detail_ObjectRegistry_getMutex() );
        std::size_t const n = rData.m_addresses.erase(pObj);
        DEBUGBASE_ENSURE( n == 1, "erased more than 1 entry!?" );
        static_cast<void>(n);
    }
    else {
        osl_atomic_decrement(&rData.m_nCount);
    }
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
