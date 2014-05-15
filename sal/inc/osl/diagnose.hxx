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


#if ! defined(OSL_DIAGNOSE_HXX_INCLUDED)
#define OSL_DIAGNOSE_HXX_INCLUDED

#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/instance.hxx"
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

typedef ::std::hash_set<void const*, VoidPtrHash, ::std::equal_to<void const*> > VoidPointerSet;

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
                OSL_ASSERT( *iPos != 0 );
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

#endif // ! defined(OSL_DIAGNOSE_HXX_INCLUDED)

