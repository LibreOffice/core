/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: specialtypemanager.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _SPECIALTYPEMANAGER_HXX_
#define _SPECIALTYPEMANAGER_HXX_

#include    <codemaker/registry.hxx>
#include    <codemaker/typemanager.hxx>


struct SpecialTypeManagerImpl
{
    T2TypeClassMap  m_t2TypeClass;
};

class SpecialTypeManager : public TypeManager
{
public:
    SpecialTypeManager();
    ~SpecialTypeManager();

    SpecialTypeManager( const SpecialTypeManager& value )
        : TypeManager(value)
        , m_pImpl( value.m_pImpl )
    {
        acquire();
    }

    using TypeManager::init;
    sal_Bool init(const ::rtl::OString& registryName);

    sal_Bool    isValidType(const ::rtl::OString&)
        { return sal_True; }
    TypeReader  getTypeReader(const ::rtl::OString& name);
    RTTypeClass getTypeClass(const ::rtl::OString& name);

    sal_Int32 getSize() { return m_pImpl->m_t2TypeClass.size(); }

protected:
    void acquire();
    void release();

protected:
    SpecialTypeManagerImpl* m_pImpl;
};

#endif // _CODEMAKER_TYPEMANAGER_HXX_
