/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: specialtypemanager.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:06:45 $
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

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

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


