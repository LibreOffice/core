/*************************************************************************
 *
 *  $RCSfile: typemanager.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 03:11:06 $
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

#ifndef _CODEMAKER_TYPEMANAGER_HXX_
#include    <codemaker/typemanager.hxx>
#endif

#include "registry/reader.hxx"
#include "registry/version.h"

using namespace rtl;

TypeManager::TypeManager()
{
    m_pImpl = new TypeManagerImpl();
    acquire();
}

TypeManager::~TypeManager()
{
    release();
}

sal_Int32 TypeManager::acquire()
{
    return osl_incrementInterlockedCount(&m_pImpl->m_refCount);
}

sal_Int32 TypeManager::release()
{
    sal_Int32 refCount = 0;
    if (0 == (refCount = osl_decrementInterlockedCount(&m_pImpl->m_refCount)) )
    {
        delete m_pImpl;
    }
    return refCount;;
}

sal_Bool TypeManager::isBaseType(const ::rtl::OString& name)
{
    if ( name.equals(OString("short")) )
        return sal_True;
    if ( name.equals(OString("unsigned short")) )
        return sal_True;
    if ( name.equals(OString("long")) )
        return sal_True;
    if ( name.equals(OString("unsigned long")) )
        return sal_True;
    if ( name.equals(OString("hyper")) )
        return sal_True;
    if ( name.equals(OString("unsigned hyper")) )
        return sal_True;
    if ( name.equals(OString("string")) )
        return sal_True;
    if ( name.equals(OString("boolean")) )
        return sal_True;
    if ( name.equals(OString("char")) )
        return sal_True;
    if ( name.equals(OString("byte")) )
        return sal_True;
    if ( name.equals(OString("any")) )
        return sal_True;
    if ( name.equals(OString("type")) )
        return sal_True;
    if ( name.equals(OString("float")) )
        return sal_True;
    if ( name.equals(OString("double")) )
        return sal_True;
    if ( name.equals(OString("void")) )
        return sal_True;

    return sal_False;
}

RegistryTypeManager::RegistryTypeManager()
{
    m_pImpl = new RegistryTypeManagerImpl();
    acquire();
}

RegistryTypeManager::~RegistryTypeManager()
{
    release();
}

void RegistryTypeManager::acquire()
{
    TypeManager::acquire();
}

void RegistryTypeManager::release()
{
    if (0 == TypeManager::release())
    {
        freeRegistries();

        delete m_pImpl;
    }
}

sal_Bool RegistryTypeManager::init(
    const StringVector& regFiles,
    StringVector const & extraFiles )
{
    if (regFiles.empty())
        return sal_False;

    StringVector::const_iterator iter = regFiles.begin();

    RegistryLoader loader;
    Registry tmpReg(loader);
    while (iter != regFiles.end())
    {
        if (!tmpReg.open( convertToFileUrl(*iter), REG_READONLY))
            m_pImpl->m_registries.push_back(new Registry(tmpReg));
        else
        {
            freeRegistries();
            return sal_False;
        }
        ++iter;
    }
    iter = extraFiles.begin();
    while (iter != extraFiles.end())
    {
        if (!tmpReg.open( convertToFileUrl(*iter), REG_READONLY))
            m_pImpl->m_extra_registries.push_back(new Registry(tmpReg));
        else
        {
            freeRegistries();
            return sal_False;
        }
        ++iter;
    }

    return sal_True;
}

typereg::Reader RegistryTypeManager::getTypeReader(
    const OString& name, sal_Bool * pIsExtraType ) const
{
    typereg::Reader reader;
    RegistryKey key(searchTypeKey(name, pIsExtraType));

    if (key.isValid())
    {
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!key.getValueInfo(OUString(), &valueType, &valueSize))
        {
            sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if (!key.getValue(OUString(), pBuffer))
            {
                reader = typereg::Reader(
                    pBuffer, valueSize, true, TYPEREG_VERSION_1);
            }
            rtl_freeMemory(pBuffer);
        }
    }
    return reader;
}

RTTypeClass RegistryTypeManager::getTypeClass(const OString& name) const
{
    if (m_pImpl->m_t2TypeClass.count(name) > 0)
    {
        return m_pImpl->m_t2TypeClass[name];
    } else
    {
        RegistryKey key(searchTypeKey(name));

        if (key.isValid())
        {
            RegValueType    valueType;
            sal_uInt32      valueSize;

            if (!key.getValueInfo(OUString(), &valueType, &valueSize))
            {
                sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if (!key.getValue(OUString(), pBuffer))
                {
                    typereg::Reader reader(
                        pBuffer, valueSize, false, TYPEREG_VERSION_1);

                    RTTypeClass ret = reader.getTypeClass();

                    rtl_freeMemory(pBuffer);

                    m_pImpl->m_t2TypeClass[name] = ret;
                    return ret;
                }
                rtl_freeMemory(pBuffer);
            }
        }
    }

    return RT_TYPE_INVALID;
}

void RegistryTypeManager::setBase(const OString& base)
{
    m_pImpl->m_base = base;

    if (base.lastIndexOf('/') != (base.getLength() - 1))
    {
        m_pImpl->m_base += "/";
    }
}

void RegistryTypeManager::freeRegistries()
{
    RegistryList::const_iterator iter = m_pImpl->m_registries.begin();
    while (iter != m_pImpl->m_registries.end())
    {
        delete *iter;
        ++iter;
    }
    iter = m_pImpl->m_extra_registries.begin();
    while (iter != m_pImpl->m_extra_registries.end())
    {
        delete *iter;
        ++iter;
    }
}

RegistryKey RegistryTypeManager::searchTypeKey(const OString& name_, sal_Bool * pIsExtraType )
    const
{
    OUString name( OStringToOUString(m_pImpl->m_base + name_, RTL_TEXTENCODING_UTF8) );
    RegistryKey key, rootKey;

    RegistryList::const_iterator iter = m_pImpl->m_registries.begin();
    while (iter != m_pImpl->m_registries.end())
    {
        if (!(*iter)->openRootKey(rootKey))
        {
            if (!rootKey.openKey(name, key))
            {
                if (pIsExtraType)
                    *pIsExtraType = sal_False;
                return key;
            }
        }
        ++iter;
    }
    iter = m_pImpl->m_extra_registries.begin();
    while (iter != m_pImpl->m_extra_registries.end())
    {
        if (!(*iter)->openRootKey(rootKey))
        {
            if (!rootKey.openKey(name, key))
            {
                if (pIsExtraType)
                    *pIsExtraType = sal_True;
                break;
            }
        }
        ++iter;
    }

    return key;
}

