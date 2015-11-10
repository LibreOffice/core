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

#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "osl/process.h"
#include "osl/diagnose.hxx"
#include "sal/log.hxx"
#include "boost/bind.hpp"
#include <algorithm>
#include <vector>

namespace {

typedef std::vector<rtl::OString> OStringVec;

struct StaticDebugBaseAddressFilter
    : rtl::StaticWithInit<OStringVec, StaticDebugBaseAddressFilter> {
    OStringVec operator()() const {
        OStringVec vec;
        rtl_uString * pStr = nullptr;
        rtl::OUString const name(
            "OSL_DEBUGBASE_STORE_ADDRESSES" );
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

struct DebugBaseMutex : rtl::Static<osl::Mutex, DebugBaseMutex> {};

} // anon namespace

extern "C" {

// These functions presumably should not be extern "C", but changing
// that would break binary compatibility.
#ifdef __clang__
#pragma clang diagnostic push
// Guard against slightly older clang versions that don't have
// -Wreturn-type-c-linkage...
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
#endif

osl::Mutex & SAL_CALL osl_detail_ObjectRegistry_getMutex()
    SAL_THROW_EXTERN_C()
{
    return DebugBaseMutex::get();
}
#ifdef __clang__
#pragma clang diagnostic pop
#endif

bool SAL_CALL osl_detail_ObjectRegistry_storeAddresses( char const* pName )
    SAL_THROW_EXTERN_C()
{
    OStringVec const& rVec = StaticDebugBaseAddressFilter::get();
    if (rVec.empty())
        return false;
    // check for "all":
    rtl::OString const& rFirst = rVec[0];
    if ( rFirst == "all" )
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
    SAL_WARN_IF(
        !bRet, "sal.osl",
        "unexpected number of " << rData.m_pName << ": " << nSize
            << "; Expected: " << nExpected);
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
        SAL_WARN_IF(!insertion.second, "sal.osl", "insertion failed!?");
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
        SAL_WARN_IF(n != 1, "sal.osl", "erased more than 1 entry!?");
    }
    else {
        osl_atomic_decrement(&rData.m_nCount);
    }
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
