/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typemanager.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:07:02 $
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

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#define _CODEMAKER_TYPEMANAGER_HXX_

#include "codemaker/global.hxx"

#include "registry/registry.hxx"
#include "registry/types.h"

#include <hash_map>
#include <list>

namespace typereg { class Reader; }

//typedef ::std::list< Registry* >  RegistryList;
typedef ::std::vector< Registry* >  RegistryList;
typedef ::std::pair< RegistryKey, sal_Bool >    KeyPair;
typedef ::std::vector< KeyPair >    RegistryKeyList;

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
    ~TypeManager();

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

    virtual sal_Bool isValidType(const ::rtl::OString& name) const
        { return sal_False; }

    virtual ::rtl::OString getTypeName(RegistryKey& rTypeKey) const
        { return ::rtl::OString(); }

    virtual RegistryKey getTypeKey(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const
        { return RegistryKey(); }
    virtual RegistryKeyList getTypeKeys(const ::rtl::OString& name) const
        { return RegistryKeyList(); }
    virtual typereg::Reader getTypeReader(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const = 0;
    virtual typereg::Reader getTypeReader(RegistryKey& rTypeKey) const = 0;
    virtual RTTypeClass getTypeClass(const ::rtl::OString& name) const
        { return RT_TYPE_INVALID; }
    virtual RTTypeClass getTypeClass(RegistryKey& rTypeKey) const
        { return RT_TYPE_INVALID; }

    virtual void setBase(const ::rtl::OString& base) {}
    virtual ::rtl::OString getBase() const { return ::rtl::OString(); }

    virtual sal_Int32 getSize() const { return 0; }

    static sal_Bool isBaseType(const ::rtl::OString& name);
protected:
    sal_Int32 acquire();
    sal_Int32 release();

protected:
    TypeManagerImpl* m_pImpl;
};

struct RegistryTypeManagerImpl
{
    RegistryTypeManagerImpl()
        : m_base("/")
        {}

    T2TypeClassMap  m_t2TypeClass;
    RegistryList    m_registries;
    RegistryList    m_extra_registries;
    ::rtl::OString  m_base;
};

class RegistryTypeManager : public TypeManager
{
public:
    RegistryTypeManager();
    ~RegistryTypeManager();

    RegistryTypeManager( const RegistryTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {
        acquire();
    }

    sal_Bool init(const StringVector& regFiles, const StringVector& extraFiles = StringVector() );

    ::rtl::OString getTypeName(RegistryKey& rTypeKey) const;

    sal_Bool    isValidType(const ::rtl::OString& name) const
        { return searchTypeKey(name, 0).isValid(); }
    RegistryKey getTypeKey(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const
        { return searchTypeKey(name, pIsExtraType); }
    RegistryKeyList getTypeKeys(const ::rtl::OString& name) const;
    typereg::Reader getTypeReader(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    typereg::Reader getTypeReader(RegistryKey& rTypeKey) const;
    RTTypeClass getTypeClass(const ::rtl::OString& name) const;
    RTTypeClass getTypeClass(RegistryKey& rTypeKey) const;

    void setBase(const ::rtl::OString& base);
    ::rtl::OString getBase() const { return m_pImpl->m_base; }

    sal_Int32 getSize() const { return m_pImpl->m_t2TypeClass.size(); }
protected:
    RegistryKey searchTypeKey(
        const ::rtl::OString& name, sal_Bool * pIsExtraType = 0 ) const;
    void        freeRegistries();

    void acquire();
    void release();

protected:
    RegistryTypeManagerImpl* m_pImpl;
};

#endif // _CODEMAKER_TYPEMANAGER_HXX_
