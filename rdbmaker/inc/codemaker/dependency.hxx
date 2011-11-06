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



#ifndef _CODEMAKER_DEPENDENCY_HXX_
#define _CODEMAKER_DEPENDENCY_HXX_

#include    <hash_map>
#include    <registry/registry.hxx>
#ifndef __REGISTRY_REFLREAD_HXX__
#include    <registry/reflread.hxx>
#endif
#include    <codemaker/typemanager.hxx>
#include    <codemaker/global.hxx>
#include <osl/diagnose.h>

#define TYPEUSE_NORMAL      0x0001
#define TYPEUSE_SUPER       0x0002
#define TYPEUSE_MEMBER      0x0004
#define TYPEUSE_INPARAM     0x0008
#define TYPEUSE_OUTPARAM    0x0010
#define TYPEUSE_INOUTPARAM  0x0020
#define TYPEUSE_RETURN      0x0040
#define TYPEUSE_EXCEPTION   0x0080
#define TYPEUSE_SCOPE       0x0100

/**
 * Flag shows the state of the code generation. If the Flag is set
 * the code for this type is generated.
 */
#define CODEGEN_DEFAULT     0x0001

struct TypeUsing
{
    TypeUsing(const ::rtl::OString& type, sal_uInt16 use)
        : m_type(type)
        , m_use(use)
        {}

    ::rtl::OString  m_type;
    sal_uInt16      m_use;

    sal_Bool operator == (const TypeUsing & typeUsing) const
    {
        OSL_ASSERT(0);
        return m_type == typeUsing.m_type && m_use == typeUsing.m_use;
    }
};

struct LessTypeUsing
{
    sal_Bool operator()(const TypeUsing& tuse1, const TypeUsing& tuse2) const
    {
        return (tuse1.m_type < tuse2.m_type);
    }
};

typedef ::std::set< TypeUsing, LessTypeUsing > TypeUsingSet;


#if (defined( _MSC_VER ) && ( _MSC_VER < 1200 ))
typedef ::std::__hash_map__
<
    ::rtl::OString,
    TypeUsingSet,
    HashString,
    EqualString,
    NewAlloc
> DependencyMap;

typedef ::std::__hash_map__
<
    ::rtl::OString,
    sal_uInt16,
    HashString,
    EqualString,
    NewAlloc
> GenerationMap;
#else
typedef ::std::hash_map
<
    ::rtl::OString,
    TypeUsingSet,
    HashString,
    EqualString
> DependencyMap;

typedef ::std::hash_map
<
    ::rtl::OString,
    sal_uInt16,
    HashString,
    EqualString
> GenerationMap;

#endif

struct TypeDependencyImpl
{
    TypeDependencyImpl()
        : m_refCount(0)
        {}

    sal_Int32       m_refCount;
    DependencyMap   m_dependencies;
    GenerationMap   m_generatedTypes;
};

class TypeDependency
{
public:
    TypeDependency();
    ~TypeDependency();

    TypeDependency( const TypeDependency& value )
        : m_pImpl( value.m_pImpl )
    {
        acquire();
    }

    TypeDependency& operator = ( const TypeDependency& value )
    {
        release();
        m_pImpl = value.m_pImpl;
        acquire();
        return *this;
    }

    sal_Bool        insert(const ::rtl::OString& type, const ::rtl::OString& depend, sal_uInt16);
    TypeUsingSet    getDependencies(const ::rtl::OString& type);
    sal_Bool        hasDependencies(const ::rtl::OString& type);

    void        setGenerated(const ::rtl::OString& type, sal_uInt16 genFlag=CODEGEN_DEFAULT);
    sal_Bool    isGenerated(const ::rtl::OString& type, sal_uInt16 genFlag=CODEGEN_DEFAULT);

    sal_Int32 getSize() { return m_pImpl->m_generatedTypes.size(); }
protected:
    void acquire();
    void release();

protected:
    TypeDependencyImpl* m_pImpl;
};

sal_Bool checkTypeDependencies(TypeManager& typeMgr, TypeDependency& dependencies, const ::rtl::OString& type, sal_Bool bDepend = sal_False);

#endif // _CODEMAKER_DEPENDENCY_HXX_
