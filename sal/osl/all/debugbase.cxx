/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: debugbase.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 11:55:46 $
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
#include "precompiled_sal.hxx"

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
    : rtl::StaticWithInit<OStringVec const, StaticDebugBaseAddressFilter> {
    OStringVec const operator()() const {
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

osl::Mutex & SAL_CALL osl_detail_ObjectRegistry_getMutex()
    SAL_THROW_EXTERN_C()
{
    return DebugBaseMutex::get();
}

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
        osl_incrementInterlockedCount(&rData.m_nCount);
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
        osl_decrementInterlockedCount(&rData.m_nCount);
    }
}

} // extern "C"

