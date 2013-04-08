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

#include "sal/config.h"

#include <cstdlib>

#include "rtl/alloc.h"
#include "codemaker/typemanager.hxx"
#include "registry/reader.hxx"
#include "registry/version.h"
#include "unoidl/unoidl.hxx"

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUStringToOString;

TypeManager::TypeManager(): m_base("UCR"), manager_(new unoidl::Manager) {}

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
        m_base = base.copy(0, base.lastIndexOf('/') - 1);
    else
        m_base = base;
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


void TypeManager::loadProvider(rtl::OUString const & uri, bool primary) {
    rtl::Reference< unoidl::Provider > prov(
        unoidl::loadProvider(manager_, uri));
    manager_->addProvider(prov);
    if (primary) {
        primaryProviders_.push_back(prov);
    }
}

bool TypeManager::foundAtPrimaryProvider(rtl::OUString const & name) const {
    if (name.isEmpty()) {
        return !primaryProviders_.empty();
    }
    for (std::vector< rtl::Reference< unoidl::Provider > >::const_iterator i(
             primaryProviders_.begin());
         i != primaryProviders_.end(); ++i)
    {
        if ((*i)->findEntity(name).is()) {
            return true;
        }
    }
    return false;
}

codemaker::UnoType::Sort TypeManager::getSort(
    rtl::OUString const & name, rtl::Reference< unoidl::Entity > * entity,
    rtl::Reference< unoidl::MapCursor > * cursor) const
{
    if (name.isEmpty()) {
        if (cursor != 0) {
            *cursor = manager_->createCursor("");
        }
        return codemaker::UnoType::SORT_MODULE;
    }
    if (name == "void") {
        return codemaker::UnoType::SORT_VOID;
    }
    if (name == "boolean") {
        return codemaker::UnoType::SORT_BOOLEAN;
    }
    if (name == "byte") {
        return codemaker::UnoType::SORT_BYTE;
    }
    if (name == "short") {
        return codemaker::UnoType::SORT_SHORT;
    }
    if (name == "unsigned short") {
        return codemaker::UnoType::SORT_UNSIGNED_SHORT;
    }
    if (name == "long") {
        return codemaker::UnoType::SORT_LONG;
    }
    if (name == "unsigned long") {
        return codemaker::UnoType::SORT_UNSIGNED_LONG;
    }
    if (name == "hyper") {
        return codemaker::UnoType::SORT_HYPER;
    }
    if (name == "unsigned hyper") {
        return codemaker::UnoType::SORT_UNSIGNED_HYPER;
    }
    if (name == "float") {
        return codemaker::UnoType::SORT_FLOAT;
    }
    if (name == "double") {
        return codemaker::UnoType::SORT_DOUBLE;
    }
    if (name == "char") {
        return codemaker::UnoType::SORT_CHAR;
    }
    if (name == "string") {
        return codemaker::UnoType::SORT_STRING;
    }
    if (name == "type") {
        return codemaker::UnoType::SORT_TYPE;
    }
    if (name == "any") {
        return codemaker::UnoType::SORT_ANY;
    }
    if (name.startsWith("[")) {
        return codemaker::UnoType::SORT_SEQUENCE_TYPE;
    }
    if (name.indexOf('<') != -1) {
        return codemaker::UnoType::SORT_INSTANTIATED_POLYMORPHIC_STRUCT_TYPE;
    }
    rtl::Reference< unoidl::Entity > ent(manager_->findEntity(name));
    if (!ent.is()) {
        throw CannotDumpException("Unknown entity '" + name + "'");
    }
    if (entity != 0) {
        *entity = ent;
    }
    switch (ent->getSort()) {
    case unoidl::Entity::SORT_MODULE:
        if (cursor != 0) {
            *cursor = manager_->createCursor(name);
        }
        return codemaker::UnoType::SORT_MODULE;
    case unoidl::Entity::SORT_ENUM_TYPE:
        return codemaker::UnoType::SORT_ENUM_TYPE;
    case unoidl::Entity::SORT_PLAIN_STRUCT_TYPE:
        return codemaker::UnoType::SORT_PLAIN_STRUCT_TYPE;
    case unoidl::Entity::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE:
        return codemaker::UnoType::SORT_POLYMORPHIC_STRUCT_TYPE_TEMPLATE;
    case unoidl::Entity::SORT_EXCEPTION_TYPE:
        return codemaker::UnoType::SORT_EXCEPTION_TYPE;
    case unoidl::Entity::SORT_INTERFACE_TYPE:
        return codemaker::UnoType::SORT_INTERFACE_TYPE;
    case unoidl::Entity::SORT_TYPEDEF:
        return codemaker::UnoType::SORT_TYPEDEF;
    case unoidl::Entity::SORT_CONSTANT_GROUP:
        return codemaker::UnoType::SORT_CONSTANT_GROUP;
    case unoidl::Entity::SORT_SINGLE_INTERFACE_BASED_SERVICE:
        return codemaker::UnoType::SORT_SINGLE_INTERFACE_BASED_SERVICE;
    case unoidl::Entity::SORT_ACCUMULATION_BASED_SERVICE:
        return codemaker::UnoType::SORT_ACCUMULATION_BASED_SERVICE;
    case unoidl::Entity::SORT_INTERFACE_BASED_SINGLETON:
        return codemaker::UnoType::SORT_INTERFACE_BASED_SINGLETON;
    case unoidl::Entity::SORT_SERVICE_BASED_SINGLETON:
        return codemaker::UnoType::SORT_SERVICE_BASED_SINGLETON;
    default:
        for (;;) { std::abort(); } // this cannot happen
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
