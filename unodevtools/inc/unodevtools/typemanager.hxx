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



#ifndef INCLUDED_UNODEVTOOLS_TYPEMANAGER_HXX
#define INCLUDED_UNODEVTOOLS_TYPEMANAGER_HXX

#include <codemaker/typemanager.hxx>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <hash_map>
#include <vector>

class RegistryKey;

namespace typereg { class Reader; }

#if defined( _MSC_VER ) && ( _MSC_VER < 1200 )
typedef ::std::__hash_map__
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString,
    NewAlloc
> T2TypeClassMap;
#else
typedef ::std::hash_map
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString
> T2TypeClassMap;
#endif

namespace unodevtools {

struct UnoTypeManagerImpl
{
    UnoTypeManagerImpl() {}

    T2TypeClassMap  m_t2TypeClass;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XHierarchicalNameAccess> m_tdmgr;
};

class UnoTypeManager : public TypeManager
{
public:
    UnoTypeManager();
    ~UnoTypeManager();

    UnoTypeManager( const UnoTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {}

    sal_Bool init(const ::std::vector< ::rtl::OUString > registries);

    sal_Bool    isValidType(const ::rtl::OString& name) const;
    ::rtl::OString getTypeName(RegistryKey& rTypeKey) const;
    typereg::Reader getTypeReader(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    typereg::Reader getTypeReader(RegistryKey& rTypeKey) const;
    RTTypeClass getTypeClass(const ::rtl::OString& name) const;
    RTTypeClass getTypeClass(RegistryKey& rTypeKey) const;

protected:
    void release();

    UnoTypeManagerImpl* m_pImpl;
};

}

#endif // _UNODEVTOOLS_TYPEMANAGER_HXX_
