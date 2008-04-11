/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typemanager.hxx,v $
 * $Revision: 1.7 $
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
#include    <hash_map>

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#define _CODEMAKER_TYPEMANAGER_HXX_
#include    <codemaker/registry.hxx>

typedef ::std::list< Registry* >    RegistryList;

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

struct TypeManagerImpl
{
    TypeManagerImpl()
        : m_refCount(0)
        {}

    sal_Int32       m_refCount;
};

class TypeManager
{
public:
    TypeManager();
    virtual ~TypeManager();

    TypeManager( const TypeManager& value )
        : m_pImpl( value.m_pImpl )
    {
        acquire();
    }

    TypeManager& operator = ( const TypeManager& value )
    {
        release();
        m_pImpl = value.m_pImpl;
        acquire();
        return *this;
    }

    virtual sal_Bool init(sal_Bool /*bMerge*/, const StringVector& /*regFiles*/)
        { return sal_False; }
    virtual sal_Bool init(const ::rtl::OString& /*registryName*/)
        { return sal_False; }

    virtual sal_Bool isValidType(const ::rtl::OString& /*name*/)
        { return sal_False; }

    virtual RegistryKey getTypeKey(const ::rtl::OString& /*name*/)
        { return RegistryKey(); }
    virtual TypeReader getTypeReader(const ::rtl::OString& /*name*/)
        { return TypeReader(); }
    virtual RTTypeClass getTypeClass(const ::rtl::OString& /*name*/)
        { return RT_TYPE_INVALID; }

    virtual void setBase(const ::rtl::OString& /*base*/) {}
    virtual ::rtl::OString getBase() { return ::rtl::OString(); }

    virtual sal_Int32 getSize() { return 0; }

protected:
    sal_Int32 acquire();
    sal_Int32 release();

protected:
    TypeManagerImpl* m_pImpl;
};

struct RegistryTypeManagerImpl
{
    RegistryTypeManagerImpl()
        : m_pMergedRegistry(NULL)
        , m_base("/")
        , m_isMerged(sal_False)
        {}

    T2TypeClassMap  m_t2TypeClass;
    RegistryList    m_registries;
    Registry*       m_pMergedRegistry;
    ::rtl::OString  m_base;
    sal_Bool        m_isMerged;
};

class RegistryTypeManager : public TypeManager
{
public:
    RegistryTypeManager();
    virtual ~RegistryTypeManager();

    RegistryTypeManager( const RegistryTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {
        acquire();
    }
/*
    RegistryTypeManager& operator = ( const RegistryTypeManager& value )
    {
        release();
        m_pImpl = value.m_pImpl;
        acquire();
        return *this;
    }
*/
    using TypeManager::init;
    sal_Bool init(sal_Bool bMerge, const StringVector& regFiles);

    sal_Bool    isValidType(const ::rtl::OString& name)
        { return searchTypeKey(name).isValid(); }
    RegistryKey getTypeKey(const ::rtl::OString& name)
        { return searchTypeKey(name); }
    TypeReader  getTypeReader(const ::rtl::OString& name);
    RTTypeClass getTypeClass(const ::rtl::OString& name);

    void setBase(const ::rtl::OString& base);
    ::rtl::OString getBase() { return m_pImpl->m_base; }

    sal_Int32 getSize() { return m_pImpl->m_t2TypeClass.size(); }
protected:
    RegistryKey searchTypeKey(const ::rtl::OString& name);
    void        freeRegistries();

    void acquire();
    void release();

protected:
    RegistryTypeManagerImpl* m_pImpl;
};

#endif // _CODEMAKER_TYPEMANAGER_HXX_
