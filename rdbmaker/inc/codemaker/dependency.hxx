/*************************************************************************
 *
 *  $RCSfile: dependency.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:08 $
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

#ifndef _CODEMAKER_DEPENDENCY_HXX_
#define _CODEMAKER_DEPENDENCY_HXX_

#include    <hash_map>

#ifndef _REGISTRY_REGISTRY_HXX_
#include    <registry/registry.hxx>
#endif
#ifndef __REGISTRY_REFLREAD_HXX__
#include    <registry/reflread.hxx>
#endif

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include    <codemaker/typemanager.hxx>
#endif
#ifndef _CODEMAKER_GLOBAL_HXX_
#include    <codemaker/global.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#define TYPEUSE_NORMAL      0x0001
#define TYPEUSE_SUPER       0x0002
#define TYPEUSE_MEMBER      0x0004
#define TYPEUSE_INPARAM     0x0008
#define TYPEUSE_OUTPARAM    0x0010
#define TYPEUSE_INOUTPARAM  0x0020
#define TYPEUSE_RETURN      0x0040
#define TYPEUSE_EXCEPTION   0x0080

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

typedef NAMESPACE_STD(set) <TypeUsing, LessTypeUsing> TypeUsingSet;


#if (defined( _MSC_VER ) && ( _MSC_VER < 1200 ))
typedef NAMESPACE_STD(__hash_map__)
<
    ::rtl::OString,
    TypeUsingSet,
    HashString,
    EqualString,
    NewAlloc
> DependencyMap;

typedef NAMESPACE_STD(__hash_map__)
<
    ::rtl::OString,
    sal_uInt16,
    HashString,
    EqualString,
    NewAlloc
> GenerationMap;
#else
typedef NAMESPACE_STD(hash_map)
<
    ::rtl::OString,
    TypeUsingSet,
    HashString,
    EqualString
> DependencyMap;

typedef NAMESPACE_STD(hash_map)
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
    sal_Bool        lookupDependency(const ::rtl::OString& type, const ::rtl::OString& depend, sal_uInt16);
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

sal_Bool checkTypeDependencies(TypeManager& typeMgr, TypeDependency& dependencies, const ::rtl::OString& type);

#endif // _CODEMAKER_DEPENDENCY_HXX_
