/*************************************************************************
 *
 *  $RCSfile: typemanager.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include    <hash_map>

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#define _CODEMAKER_TYPEMANAGER_HXX_

#ifndef _CODEMAKER_REGISTRY_HXX_
#include    <codemaker/registry.hxx>
#endif

RegistryTypeReaderLoader & getRegistryTypeReaderLoader();


typedef NAMESPACE_STD(list) <Registry*>     RegistryList;

#if defined( _MSC_VER ) && ( _MSC_VER < 1200 )
typedef NAMESPACE_STD(__hash_map__)
<
    ::rtl::OString, // Typename
    RTTypeClass,    // TypeClass
    HashString,
    EqualString,
    NewAlloc
> T2TypeClassMap;
#else
typedef NAMESPACE_STD(hash_map)
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

    virtual sal_Bool init(sal_Bool bMerge, const StringVector& regFiles)
        { return sal_False; }
    virtual sal_Bool init(const ::rtl::OString& registryName)
        { return sal_False; }

    virtual sal_Bool isValidType(const ::rtl::OString& name)
        { return sal_False; }

    virtual RegistryKey getTypeKey(const ::rtl::OString& name)
        { return RegistryKey(); }
    virtual TypeReader getTypeReader(const ::rtl::OString& name)
        { return TypeReader(); }
    virtual RTTypeClass getTypeClass(const ::rtl::OString& name)
        { return RT_TYPE_INVALID; }

    virtual void setBase(const ::rtl::OString& base) {}
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
        , m_isMerged(sal_False)
        , m_base("/")
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
    ~RegistryTypeManager();

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
