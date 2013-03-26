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

sal_Bool TypeManager::isBaseType(const ::rtl::OString& name)
{
    if ( name == "short" )
        return sal_True;
    if ( name == "unsigned short" )
        return sal_True;
    if ( name == "long" )
        return sal_True;
    if ( name == "unsigned long" )
        return sal_True;
    if ( name == "hyper" )
        return sal_True;
    if ( name == "unsigned hyper" )
        return sal_True;
    if ( name == "string" )
        return sal_True;
    if ( name == "boolean" )
        return sal_True;
    if ( name == "char" )
        return sal_True;
    if ( name == "byte" )
        return sal_True;
    if ( name == "any" )
        return sal_True;
    if ( name == "type" )
        return sal_True;
    if ( name == "float" )
        return sal_True;
    if ( name == "double" )
        return sal_True;
    if ( name == "void" )
        return sal_True;

    return sal_False;
}

TypeManager::TypeManager() {}

TypeManager::~TypeManager()
{
    freeRegistries();
}

sal_Bool TypeManager::init(
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
            m_registries.push_back(new Registry(tmpReg));
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
            m_extra_registries.push_back(new Registry(tmpReg));
        else
        {
            freeRegistries();
            return sal_False;
        }
        ++iter;
    }

    return sal_True;
}

::rtl::OString TypeManager::getTypeName(RegistryKey& rTypeKey) const
{
    OString typeName = OUStringToOString(rTypeKey.getName(), RTL_TEXTENCODING_UTF8);

    if (m_base.getLength() > 1)
        typeName = typeName.copy(typeName.indexOf('/', 1) + 1);
    else
        typeName = typeName.copy(1);

    return typeName;
}

typereg::Reader TypeManager::getTypeReader(
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

typereg::Reader TypeManager::getTypeReader(RegistryKey& rTypeKey) const
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


RTTypeClass TypeManager::getTypeClass(const OString& name) const
{
    if (m_t2TypeClass.count(name) > 0)
    {
        return m_t2TypeClass[name];
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

                    m_t2TypeClass[name] = ret;
                    return ret;
                }
                rtl_freeMemory(pBuffer);
            }
        }
    }

    return RT_TYPE_INVALID;
}

RTTypeClass TypeManager::getTypeClass(RegistryKey& rTypeKey) const
{
    OString name = getTypeName(rTypeKey);

    if (m_t2TypeClass.count(name) > 0)
    {
        return m_t2TypeClass[name];
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

                    m_t2TypeClass[name] = ret;
                    return ret;
                }
                rtl_freeMemory(pBuffer);
            }
        }
    }

    return RT_TYPE_INVALID;
}

void TypeManager::setBase(const OString& base)
{

    if (base.lastIndexOf('/') == (base.getLength() - 1))
        m_base += base.copy(0, base.lastIndexOf('/') - 1);
    else
        m_base += base;
}

void TypeManager::freeRegistries()
{
    RegistryList::const_iterator iter = m_registries.begin();
    while (iter != m_registries.end())
    {
        delete *iter;
        ++iter;
    }
    iter = m_extra_registries.begin();
    while (iter != m_extra_registries.end())
    {
        delete *iter;
        ++iter;
    }
}

RegistryKey TypeManager::searchTypeKey(const OString& name_, sal_Bool * pIsExtraType )
    const
{
    OUString name( OStringToOUString(m_base + "/" + name_, RTL_TEXTENCODING_UTF8) );
    RegistryKey key, rootKey;

    RegistryList::const_iterator iter = m_registries.begin();
    while (iter != m_registries.end())
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
    iter = m_extra_registries.begin();
    while (iter != m_extra_registries.end())
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

RegistryKeyList TypeManager::getTypeKeys(const ::rtl::OString& name_) const
{
    RegistryKeyList keyList= RegistryKeyList();
    OString tmpName;
    if ( name_ == "/" || name_ == m_base ) {
        tmpName = m_base;
    } else {
        if ( m_base == "/" )
            tmpName = name_;
        else
            tmpName = m_base + "/" + name_;
    }

    OUString name( OStringToOUString(tmpName, RTL_TEXTENCODING_UTF8) );
    RegistryKey key, rootKey;

    RegistryList::const_iterator iter = m_registries.begin();
    while (iter != m_registries.end())
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
    iter = m_extra_registries.begin();
    while (iter != m_extra_registries.end())
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
