/*************************************************************************
 *
 *  $RCSfile: specialtypemanager.cxx,v $
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
        RegistryTypeReaderLoader & rReaderLoader = getRegistryTypeReaderLoader();

        reader = TypeReader(rReaderLoader, pBlop, blopSize, sal_True);
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


