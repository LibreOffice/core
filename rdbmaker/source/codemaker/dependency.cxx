/*************************************************************************
 *
 *  $RCSfile: dependency.cxx,v $
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

#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _CODEMAKER_DEPENDENCY_HXX_
#include    <codemaker/dependency.hxx>
#endif

using namespace rtl;

TypeDependency::TypeDependency()
{
    m_pImpl = new TypeDependencyImpl();
    acquire();
}

TypeDependency::~TypeDependency()
{
    release();
}

void TypeDependency::acquire()
{
    osl_incrementInterlockedCount(&m_pImpl->m_refCount);
}

void TypeDependency::release()
{
    if (0 == osl_decrementInterlockedCount(&m_pImpl->m_refCount))
    {
        delete m_pImpl;
    }
}

sal_Bool TypeDependency::insert(const OString& type, const OString& depend, sal_uInt16 use)
{
    sal_Bool ret =  sal_False;

    if (type.getLength() > 0 && depend.getLength() > 0)
    {
        if (m_pImpl->m_dependencies.count(type) > 0)
        {
            TypeUsing typeUsing(depend, use);
            TypeUsingSet::iterator iter;
            if ((iter = m_pImpl->m_dependencies[type].find(typeUsing)) != m_pImpl->m_dependencies[type].end())
            {
                  (((TypeUsing *) &(*iter))->m_use) = (*iter).m_use | use;
            } else
            {
                m_pImpl->m_dependencies[type].insert(typeUsing);
            }
        } else
        {
            TypeUsing typeUsing(depend, use);
            TypeUsingSet tmpSet;
            tmpSet.insert(typeUsing);
            m_pImpl->m_dependencies[type]=tmpSet;
        }
    }

    return ret;
}

TypeUsingSet TypeDependency::getDependencies(const OString& type)
{
    if (type.getLength() > 0)
    {
        if (m_pImpl->m_dependencies.count(type) > 0)
        {
            return m_pImpl->m_dependencies[type];
        }
    }

    return TypeUsingSet();
}

sal_Bool TypeDependency::lookupDependency(const OString& type, const OString& depend, sal_uInt16 use)
{
    sal_Bool ret =  sal_False;

    if (type.getLength() > 0 && depend.getLength() > 0)
    {
        if (m_pImpl->m_dependencies.count(type) > 0)
        {
            TypeUsingSet::const_iterator iter = m_pImpl->m_dependencies[type].begin();

            while (iter != m_pImpl->m_dependencies[type].end())
            {
                if (depend == (*iter).m_type &&
                    (use & (*iter).m_use))
                {
                    ret = sal_True;
                    break;
                }
                iter++;
            }
        } else
        {
            ret = sal_False;
        }
    }

    return ret;
}

sal_Bool TypeDependency::hasDependencies(const OString& type)
{
    if (type.getLength() > 0)
    {
        if (m_pImpl->m_dependencies.count(type) > 0)
        {
            return sal_True;
        }
    }

    return sal_False;
}

void TypeDependency::setGenerated(const OString& type, sal_uInt16 genFlag)
{
//  m_pImpl->m_generatedTypes.insert(type);
    if (m_pImpl->m_generatedTypes.count(type) > 0)
        m_pImpl->m_generatedTypes[type]= m_pImpl->m_generatedTypes[type] | genFlag;
    else
        m_pImpl->m_generatedTypes[type]=genFlag;
}

sal_Bool TypeDependency::isGenerated(const OString& type, sal_uInt16 genFlag)
{
/*
    if (m_pImpl->m_generatedTypes.count(type) > 0)
        return sal_True;

    return sal_False;
*/
    if (m_pImpl->m_generatedTypes.count(type) > 0 &&
        m_pImpl->m_generatedTypes[type] & genFlag)
    {
        return sal_True;
    }

    return sal_False;
}

static sal_Bool checkFieldDependencies(TypeManager& typeMgr, TypeDependency& dependencies,
                                       TypeReader& reader, const OString& type)
{
    sal_uInt32 count = reader.getFieldCount();

    if (count == 0 || reader.getTypeClass() == RT_TYPE_ENUM)
        return sal_True;

    OString fieldType;
    for (sal_uInt16 i=0; i < count; i++)
    {
        fieldType = reader.getFieldType(i);

        if (fieldType.getLength() > 0)
        {
            dependencies.insert(type, fieldType, TYPEUSE_MEMBER);
            checkTypeDependencies(typeMgr, dependencies, fieldType);
        }
    }

    return sal_True;
}

static sal_Bool checkMethodDependencies(TypeManager& typeMgr, TypeDependency& dependencies,
                                        TypeReader& reader, const OString& type)
{
    sal_uInt32 count = reader.getMethodCount();

    if (count == 0)
        return sal_True;

    OString returnType, paramType, excType;
    sal_uInt32 paramCount = 0;
    sal_uInt32 excCount = 0;
    RTParamMode paramMode = RT_PARAM_INVALID;
    for (sal_uInt16 i=0; i < count; i++)
    {
        returnType = reader.getMethodReturnType(i);

        dependencies.insert(type, returnType, TYPEUSE_RETURN);
        checkTypeDependencies(typeMgr, dependencies, returnType);

        paramCount = reader.getMethodParamCount(i);
        excCount = reader.getMethodExcCount(i);

        sal_uInt16 j;
        for (j=0; j < paramCount; j++)
        {
            paramType = reader.getMethodParamType(i, j);
            paramMode = reader.getMethodParamMode(i, j);

            switch (paramMode)
            {
                case RT_PARAM_IN:
                    dependencies.insert(type, paramType, TYPEUSE_INPARAM);
                    break;
                case RT_PARAM_OUT:
                    dependencies.insert(type, paramType, TYPEUSE_OUTPARAM);
                    break;
                case RT_PARAM_INOUT:
                    dependencies.insert(type, paramType, TYPEUSE_INOUTPARAM);
                    break;
            }

            checkTypeDependencies(typeMgr, dependencies, paramType);
        }

        for (j=0; j < excCount; j++)
        {
            excType = reader.getMethodExcType(i, j);
            dependencies.insert(type, excType, TYPEUSE_EXCEPTION);
            checkTypeDependencies(typeMgr, dependencies, excType);
        }

    }

    return sal_True;
}

static sal_Bool checkReferenceDependencies(TypeManager& typeMgr, TypeDependency& dependencies,
                                           TypeReader& reader, const OString& type)
{
    sal_uInt32 count = reader.getReferenceCount();

    if (count == 0)
        return sal_True;

    OString referenceName;
    for (sal_uInt16 i=0; i < count; i++)
    {
        referenceName = reader.getReferenceName(i);

        dependencies.insert(type, referenceName, TYPEUSE_NORMAL);
        checkTypeDependencies(typeMgr, dependencies, referenceName);
    }

    return sal_True;
}

sal_Bool checkTypeDependencies(TypeManager& typeMgr, TypeDependency& dependencies, const OString& type)
{
    if (!typeMgr.isValidType(type))
        return sal_False;

    if (dependencies.hasDependencies(type))
        return sal_True;

    TypeReader reader = typeMgr.getTypeReader(type);

    if ( !reader.isValid() )
    {
        if (type.equals("/"))
            return sal_True;
        else
            return sal_False;
    }

    OString superType(reader.getSuperTypeName());
    if (superType.getLength() > 0)
    {
        dependencies.insert(type, superType, TYPEUSE_SUPER);
        checkTypeDependencies(typeMgr, dependencies, superType);
    }

    if (reader.getTypeClass() == RT_TYPE_INTERFACE)
    {
        dependencies.insert(type, "com/sun/star/uno/RuntimeException", TYPEUSE_EXCEPTION);
        dependencies.insert(type, "com/sun/star/uno/TypeClass", TYPEUSE_NORMAL);
        checkTypeDependencies(typeMgr, dependencies, "com/sun/star/uno/RuntimeException");
    }

    checkFieldDependencies(typeMgr, dependencies, reader, type);
    checkMethodDependencies(typeMgr, dependencies, reader, type);
    checkReferenceDependencies(typeMgr, dependencies, reader, type);

    return sal_True;
}


