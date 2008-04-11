/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: specialtypemanager.cxx,v $
 * $Revision: 1.4 $
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

#include    <rtl/alloc.h>

#include    "specialtypemanager.hxx"

extern "C"
{
sal_Bool SAL_CALL initTypeMapper( const sal_Char* pRegName );
sal_uInt32 SAL_CALL getTypeBlop(const sal_Char* pTypeName, sal_uInt8** pBlop);
}

using namespace rtl;

SpecialTypeManager::SpecialTypeManager()
{
    m_pImpl = new SpecialTypeManagerImpl();
    acquire();
}

SpecialTypeManager::~SpecialTypeManager()
{
    release();
}

void SpecialTypeManager::acquire()
{
    TypeManager::acquire();
}

void SpecialTypeManager::release()
{
    if (0 == TypeManager::release())
    {
        delete m_pImpl;
    }
}

sal_Bool SpecialTypeManager::init(const OString& registryName)
{
    return initTypeMapper( registryName.getStr() );
}

TypeReader SpecialTypeManager::getTypeReader(const OString& name)
{
    TypeReader reader;

    sal_uInt8* pBlop = NULL;
    sal_uInt32 blopSize = 0;

    if ( (blopSize = getTypeBlop( name.getStr(), &pBlop)) > 0 )
    {
        reader = TypeReader(pBlop, blopSize, sal_True);
    }

    if ( pBlop )
    {
        rtl_freeMemory(pBlop);
    }

    return reader;
}

RTTypeClass SpecialTypeManager::getTypeClass(const OString& name)
{
    if (m_pImpl->m_t2TypeClass.count(name) > 0)
    {
        return m_pImpl->m_t2TypeClass[name];
    } else
    {
    }

    return RT_TYPE_INVALID;
}


