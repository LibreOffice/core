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
#ifndef OSL_DIAGNOSE_HXX_INCLUDED
#define OSL_DIAGNOSE_HXX_INCLUDED

#include "sal/config.h"

#include <functional>
#include <typeinfo>

#include "boost/unordered_set.hpp"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/allocator.hxx"
#include "rtl/instance.hxx"
#include "sal/log.hxx"
#include "sal/saldllapi.h"
#include "sal/types.h"

/// @cond INTERNAL

namespace osl {
namespace detail {

struct ObjectRegistryData;

} // namespace detail
} // namespace osl

extern "C" {

SAL_DLLPUBLIC bool SAL_CALL osl_detail_ObjectRegistry_storeAddresses(
        char const* pName )
    SAL_THROW_EXTERN_C();

SAL_DLLPUBLIC bool SAL_CALL osl_detail_ObjectRegistry_checkObjectCount(
    ::osl::detail::ObjectRegistryData const& rData, ::std::size_t nExpected )
    SAL_THROW_EXTERN_C();

SAL_DLLPUBLIC void SAL_CALL osl_detail_ObjectRegistry_registerObject(
    ::osl::detail::ObjectRegistryData & rData, void const* pObj )
    SAL_THROW_EXTERN_C();

SAL_DLLPUBLIC void SAL_CALL osl_detail_ObjectRegistry_revokeObject(
    ::osl::detail::ObjectRegistryData & rData, void const* pObj )
    SAL_THROW_EXTERN_C();

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
SAL_DLLPUBLIC ::osl::Mutex & SAL_CALL osl_detail_ObjectRegistry_getMutex()
    SAL_THROW_EXTERN_C();
#if SUPD < 400
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif

} // extern "C"

namespace osl {

namespace detail {

struct VoidPtrHash : ::std::unary_function<void const*, ::std::size_t> {
    ::std::size_t operator()( void const* p ) const {
        ::std::size_t const d = static_cast< ::std::size_t >(
            reinterpret_cast< ::std::ptrdiff_t >(p) );
        return d + (d >> 3);
    }
};

typedef ::boost::unordered_set<void const*, VoidPtrHash, ::std::equal_to<void const*>,
                        ::rtl::Allocator<void const*> > VoidPointerSet;

struct ObjectRegistryData {
    ObjectRegistryData( ::std::type_info const& rTypeInfo )
        : m_pName(rTypeInfo.name()), m_nCount(0), m_addresses(),
          m_bStoreAddresses(osl_detail_ObjectRegistry_storeAddresses(m_pName)){}

    char const* const m_pName;
    oslInterlockedCount m_nCount;
    VoidPointerSet m_addresses;
    bool const m_bStoreAddresses;
};

template <typename T>
class ObjectRegistry
{
public:
    ObjectRegistry() : m_data( typeid(T) ) {}
    ~ObjectRegistry() { checkObjectCount(0); }

    bool checkObjectCount( ::std::size_t nExpected ) const {
        bool const bRet = osl_detail_ObjectRegistry_checkObjectCount(
            m_data, nExpected );
        if (!bRet && m_data.m_bStoreAddresses) {
            MutexGuard const guard( osl_detail_ObjectRegistry_getMutex() );
            // following loop is for debugging purposes, iterating over map:
            VoidPointerSet::const_iterator iPos(m_data.m_addresses.begin());
            VoidPointerSet::const_iterator const iEnd(m_data.m_addresses.end());
            for ( ; iPos != iEnd; ++iPos ) {
                SAL_WARN_IF( *iPos == 0, "sal.debug", "null pointer" );
            }
        }
        return bRet;
    }

    void registerObject( void const* pObj ) {
        osl_detail_ObjectRegistry_registerObject(m_data, pObj);
    }

    void revokeObject( void const* pObj ) {
        osl_detail_ObjectRegistry_revokeObject(m_data, pObj);
    }

private:
    // not impl:
    ObjectRegistry( ObjectRegistry const& );
    ObjectRegistry const& operator=( ObjectRegistry const& );

    ObjectRegistryData m_data;
};

} // namespace detail

/** Helper class which indicates leaking object(s) of a particular class in
    non-pro builds; use e.g.

    <pre>
    class MyClass : private osl::DebugBase<MyClass> {...};
    </pre>

    Using the environment variable

    OSL_DEBUGBASE_STORE_ADDRESSES=MyClass;YourClass;...

    you can specify a ';'-separated list of strings matching to class names
    (or "all" for all classes), for which DebugBase stores addresses to created
    objects instead of just counting them.  This enables you to iterate over
    leaking objects in your debugger.

    @tparam InheritingClassT binds the template instance to that class
    @attention Use at own risk.
              For now this is just public (yet unpublished) API and may change
              in the future!
*/
template <typename InheritingClassT>
class DebugBase
{
public:
#if OSL_DEBUG_LEVEL <= 0
    static bool checkObjectCount( ::std::size_t = 0 ) { return true; }
#else // OSL_DEBUG_LEVEL > 0
    /** @return whether the expected number of objects is alive,
                else this function SAL_WARNs
    */
    static bool checkObjectCount( ::std::size_t nExpected = 0 ) {
        return StaticObjectRegistry::get().checkObjectCount(nExpected);
    }

protected:
    DebugBase() {
        StaticObjectRegistry::get().registerObject( this );
    }
    ~DebugBase() {
        StaticObjectRegistry::get().revokeObject( this );
    }

private:
    struct StaticObjectRegistry
        : ::rtl::Static<detail::ObjectRegistry<InheritingClassT>,
                        StaticObjectRegistry> {};
#endif
};

} // namespace osl

/// @endcond

#endif // ! defined(OSL_DIAGNOSE_HXX_INCLUDED)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
