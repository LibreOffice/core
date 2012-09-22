/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "rtl/alloc.h"
#include "codemaker/typemanager.hxx"
#include "registry/reader.hxx"
#include "registry/version.h"

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringToOString;

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
    return osl_atomic_increment(&m_pImpl->m_refCount);
}

sal_Int32 TypeManager::release()
{
    sal_Int32 refCount = 0;
    if (0 == (refCount = osl_atomic_decrement(&m_pImpl->m_refCount)) )
    {
        delete m_pImpl;
    }
    return refCount;
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

    Registry tmpReg;
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

::rtl::OString RegistryTypeManager::getTypeName(RegistryKey& rTypeKey) const
{
    OString typeName = OUStringToOString(rTypeKey.getName(), RTL_TEXTENCODING_UTF8);

    if (m_pImpl->m_base.getLength() > 1)
        typeName = typeName.copy(typeName.indexOf('/', 1) + 1);
    else
        typeName = typeName.copy(1);

    return typeName;
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

typereg::Reader RegistryTypeManager::getTypeReader(RegistryKey& rTypeKey) const
{
    typereg::Reader reader;

    if (rTypeKey.isValid())
    {
        RegValueType    valueType;
        sal_uInt32      valueSize;

        if (!rTypeKey.getValueInfo(OUString(), &valueType, &valueSize))
        {
            sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
            if (!rTypeKey.getValue(OUString(), pBuffer))
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

RTTypeClass RegistryTypeManager::getTypeClass(RegistryKey& rTypeKey) const
{
    OString name = getTypeName(rTypeKey);

    if (m_pImpl->m_t2TypeClass.count(name) > 0)
    {
        return m_pImpl->m_t2TypeClass[name];
    } else
    {
        if (rTypeKey.isValid())
        {
            RegValueType    valueType;
            sal_uInt32      valueSize;

            if (!rTypeKey.getValueInfo(OUString(), &valueType, &valueSize))
            {
                sal_uInt8*  pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);
                if (!rTypeKey.getValue(OUString(), pBuffer))
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

    if (base.lastIndexOf('/') == (base.getLength() - 1))
        m_pImpl->m_base += base.copy(0, base.lastIndexOf('/') - 1);
    else
        m_pImpl->m_base += base;
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
    OUString name( OStringToOUString(m_pImpl->m_base + "/" + name_, RTL_TEXTENCODING_UTF8) );
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

RegistryKeyList RegistryTypeManager::getTypeKeys(const ::rtl::OString& name_) const
{
    RegistryKeyList keyList= RegistryKeyList();
    OString tmpName;
    if ( name_.equals("/") || name_.equals(m_pImpl->m_base) ) {
        tmpName = m_pImpl->m_base;
    } else {
        if ( m_pImpl->m_base.equals("/") )
            tmpName = name_;
        else
            tmpName = m_pImpl->m_base + "/" + name_;
    }

    OUString name( OStringToOUString(tmpName, RTL_TEXTENCODING_UTF8) );
    RegistryKey key, rootKey;

    RegistryList::const_iterator iter = m_pImpl->m_registries.begin();
    while (iter != m_pImpl->m_registries.end())
    {
        if (!(*iter)->openRootKey(rootKey))
        {
            if (!rootKey.openKey(name, key))
            {
                keyList.push_back(KeyPair(key, sal_False));
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
                keyList.push_back(KeyPair(key, sal_True));
            }
        }
        ++iter;
    }

    return keyList;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
