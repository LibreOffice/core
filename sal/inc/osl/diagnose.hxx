/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: diagnose.hxx,v $
 * $Revision: 1.3 $
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
#if ! defined(OSL_DIAGNOSE_HXX_INCLUDED)
#define OSL_DIAGNOSE_HXX_INCLUDED

#if ! defined(_OSL_DIAGNOSE_H_)
#include "osl/diagnose.h"
#endif
#if ! defined(_OSL_INTERLOCK_H_)
#include "osl/interlck.h"
#endif
#if ! defined(_OSL_MUTEX_HXX_)
#include "osl/mutex.hxx"
#endif
#if ! defined(INCLUDED_RTL_ALLOCATOR_HXX)
#include "rtl/allocator.hxx"
#endif
#if ! defined(_RTL_INSTANCE_HXX_)
#include "rtl/instance.hxx"
#endif
#include <hash_set>
#include <functional>
#include <typeinfo>

namespace osl {
/// @internal
namespace detail {

struct ObjectRegistryData;

} // namespace detail
} // namespace osl

extern "C" {

/** @internal */
bool SAL_CALL osl_detail_ObjectRegistry_storeAddresses( char const* pName )
    SAL_THROW_EXTERN_C();

/** @internal */
bool SAL_CALL osl_detail_ObjectRegistry_checkObjectCount(
    ::osl::detail::ObjectRegistryData const& rData, ::std::size_t nExpected )
    SAL_THROW_EXTERN_C();

/** @internal */
void SAL_CALL osl_detail_ObjectRegistry_registerObject(
    ::osl::detail::ObjectRegistryData & rData, void const* pObj )
    SAL_THROW_EXTERN_C();

/** @internal */
void SAL_CALL osl_detail_ObjectRegistry_revokeObject(
    ::osl::detail::ObjectRegistryData & rData, void const* pObj )
    SAL_THROW_EXTERN_C();

/** @internal */
::osl::Mutex & SAL_CALL osl_detail_ObjectRegistry_getMutex()
    SAL_THROW_EXTERN_C();

} // extern "C"

namespace osl {

/// @internal
namespace detail {

struct VoidPtrHash : ::std::unary_function<void const*, ::std::size_t> {
    ::std::size_t operator()( void const* p ) const {
        ::std::size_t const d = static_cast< ::std::size_t >(
            reinterpret_cast< ::std::ptrdiff_t >(p) );
        return d + (d >> 3);
    }
};

typedef ::std::hash_set<void const*, VoidPtrHash, ::std::equal_to<void const*>,
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
                T const* pLeakingObj = static_cast<T const*>(*iPos);
                OSL_ASSERT( pLeakingObj != 0 );
                static_cast<void>(pLeakingObj);
            }
        }
        return bRet;
    }

    void registerObject( T const* pObj ) {
        osl_detail_ObjectRegistry_registerObject(m_data, pObj);
    }

    void revokeObject( T const* pObj ) {
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

    @tpl InheritingClassT binds the template instance to that class
    @internal Use at own risk.
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
                else this function OSL_ASSERTs
    */
    static bool checkObjectCount( ::std::size_t nExpected = 0 ) {
        return StaticObjectRegistry::get().checkObjectCount(nExpected);
    }

protected:
    DebugBase() {
        StaticObjectRegistry::get().registerObject(
            static_cast<InheritingClassT const*>(this) );
    }
    ~DebugBase() {
        StaticObjectRegistry::get().revokeObject(
            static_cast<InheritingClassT const*>(this) );
    }

private:
    struct StaticObjectRegistry
        : ::rtl::Static<detail::ObjectRegistry<InheritingClassT>,
                        StaticObjectRegistry> {};
#endif
};

} // namespace osl

#endif // ! defined(OSL_DIAGNOSE_HXX_INCLUDED)

