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


#include <string.h>
#include <string_view>

#include "keyimpl.hxx"

#include "reflcnst.hxx"
#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <memory>
#include <utility>

using namespace store;

namespace { char const VALUE_PREFIX[] = "$VL_"; }

ORegKey::ORegKey(OUString keyName, ORegistry* pReg)
    : m_refCount(1)
    , m_name(std::move(keyName))
    , m_bDeleted(false)
    , m_bModified(false)
    , m_pRegistry(pReg)
{
}

ORegKey::~ORegKey()
{
    SAL_WARN_IF(m_refCount != 0, "registry", "registry::ORegKey::dtor(): refcount not zero.");
}

RegError ORegKey::releaseKey(RegKeyHandle hKey)
{
    return m_pRegistry->releaseKey(hKey);
}

RegError ORegKey::createKey(std::u16string_view keyName, RegKeyHandle* phNewKey)
{
    return m_pRegistry->createKey(this, keyName, phNewKey);
}

RegError ORegKey::openKey(std::u16string_view keyName, RegKeyHandle* phOpenKey)
{
    return m_pRegistry->openKey(this, keyName, phOpenKey);
}

RegError ORegKey::openSubKeys(std::u16string_view keyName, RegKeyHandle** phOpenSubKeys, sal_uInt32* pnSubKeys)
{
    RegError _ret = RegError::NO_ERROR;

    *phOpenSubKeys = nullptr;
    *pnSubKeys = 0;

    ORegKey* pKey = this;
    if ( !keyName.empty() )
    {
        _ret = openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pKey));
        if (_ret != RegError::NO_ERROR)
            return _ret;
    }

    sal_uInt32 nSubKeys = pKey->countSubKeys();
    *pnSubKeys = nSubKeys;

    ORegKey** pSubKeys;
    pSubKeys = static_cast<ORegKey**>(rtl_allocateZeroMemory(nSubKeys * sizeof(ORegKey*)));

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    nSubKeys = 0;
    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            OUString const sSubKeyName(iter.m_pszName, iter.m_nLength);

            ORegKey* pOpenSubKey = nullptr;
            _ret = pKey->openKey(sSubKeyName, reinterpret_cast<RegKeyHandle*>(&pOpenSubKey));
            if (_ret != RegError::NO_ERROR)
            {
                *phOpenSubKeys = nullptr;
                *pnSubKeys = 0;
                std::free(pSubKeys); // @@@ leaking 'pSubKeys[0...nSubkeys-1]'
                return _ret;              // @@@ leaking 'pKey'
            }

            pSubKeys[nSubKeys] = pOpenSubKey;

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *phOpenSubKeys = reinterpret_cast<RegKeyHandle*>(pSubKeys);
    if (!keyName.empty())
    {
        (void) releaseKey(pKey);
    }
    return RegError::NO_ERROR;
}

RegError ORegKey::getKeyNames(std::u16string_view keyName,
                              rtl_uString*** pSubKeyNames,
                              sal_uInt32* pnSubKeys)
{
    *pSubKeyNames = nullptr;
    *pnSubKeys = 0;

    ORegKey* pKey = this;
    if (!keyName.empty())
    {
        RegError _ret = openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pKey));
        if (_ret != RegError::NO_ERROR)
            return _ret;
    }

    sal_uInt32 nSubKeys = pKey->countSubKeys();
    *pnSubKeys = nSubKeys;

    rtl_uString** pSubKeys
        = static_cast<rtl_uString**>(rtl_allocateZeroMemory(nSubKeys * sizeof(rtl_uString*)));

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    nSubKeys = 0;

    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            OUString const sSubKeyName(iter.m_pszName, iter.m_nLength);

            OUString sFullKeyName(pKey->getName());
            if (sFullKeyName.getLength() > 1)
                sFullKeyName += ORegistry::ROOT;
            sFullKeyName += sSubKeyName;

            rtl_uString_newFromString(&pSubKeys[nSubKeys], sFullKeyName.pData);

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *pSubKeyNames = pSubKeys;
    if (!keyName.empty())
    {
        releaseKey(pKey);
    }
    return RegError::NO_ERROR;
}

RegError ORegKey::closeKey(RegKeyHandle hKey)
{
    return m_pRegistry->closeKey(hKey);
}

RegError ORegKey::deleteKey(std::u16string_view keyName)
{
    return m_pRegistry->deleteKey(this, keyName);
}

RegError ORegKey::getValueInfo(std::u16string_view valueName, RegValueType* pValueType, sal_uInt32* pValueSize) const
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(m_pRegistry->getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, accessMode) )
    {
        *pValueType = RegValueType::NOT_DEFINED;
        *pValueSize = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE]);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer.get(), VALUE_HEADERSIZE, readBytes) )
    {
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        return RegError::INVALID_VALUE;
    }

    sal_uInt32  size;
    sal_uInt8   type = pBuffer[0];
    readUINT32(pBuffer.get()+VALUE_TYPEOFFSET, size);

    *pValueType = static_cast<RegValueType>(type);
    if (*pValueType > RegValueType::BINARY)
    {
        pBuffer.reset(new sal_uInt8[4]);
        rValue.readAt(VALUE_HEADEROFFSET, pBuffer.get(), 4, readBytes);

        readUINT32(pBuffer.get(), size);
    }

    *pValueSize = size;

    return RegError::NO_ERROR;
}

RegError ORegKey::setValue(std::u16string_view valueName, RegValueType vType, RegValue value, sal_uInt32 vSize)
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    if (vType > RegValueType::BINARY)
    {
        return RegError::INVALID_VALUE;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + ORegistry::ROOT , sImplValueName, storeAccessMode::Create) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = vSize;

    sal_uInt8 type = static_cast<sal_uInt8>(vType);
    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE + size]);
    memcpy(pBuffer.get(), &type, 1);

    writeUINT32(pBuffer.get()+VALUE_TYPEOFFSET, size);

    switch (vType)
    {
        case RegValueType::NOT_DEFINED:
            memcpy(pBuffer.get()+VALUE_HEADEROFFSET, value, size);
            break;
        case RegValueType::LONG:
            writeINT32(pBuffer.get()+VALUE_HEADEROFFSET, *static_cast<sal_Int32*>(value));
            break;
        case RegValueType::STRING:
            writeUtf8(pBuffer.get()+VALUE_HEADEROFFSET, static_cast<const char*>(value));
            break;
        case RegValueType::UNICODE:
            writeString(pBuffer.get()+VALUE_HEADEROFFSET, static_cast<const sal_Unicode*>(value));
            break;
        case RegValueType::BINARY:
            memcpy(pBuffer.get()+VALUE_HEADEROFFSET, value, size);
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer.get(), VALUE_HEADERSIZE+size, writenBytes) )
    {
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    return RegError::NO_ERROR;
}

RegError ORegKey::setLongListValue(std::u16string_view valueName, sal_Int32 const * pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, storeAccessMode::Create) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 bytes (sal_uInt32) for the length

    size += len * 4;

    sal_uInt8 type = sal_uInt8(RegValueType::LONGLIST);
    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE + size]);
    memcpy(pBuffer.get(), &type, 1);

    writeUINT32(pBuffer.get()+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer.get()+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array

    for (sal_uInt32 i=0; i < len; i++)
    {
        writeINT32(pBuffer.get()+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += 4;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer.get(), VALUE_HEADERSIZE+size, writenBytes) )
    {
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADEROFFSET+size))
    {
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    return RegError::NO_ERROR;
}

RegError ORegKey::setStringListValue(
    std::u16string_view valueName, char** pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, storeAccessMode::Create) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 bytes (sal_uInt32) for the length

    sal_uInt32 i;
    for (i=0; i < len; i++)
    {
        size +=  4 + strlen(pValueList[i]) + 1;
    }

    sal_uInt8 type = sal_uInt8(RegValueType::STRINGLIST);
    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE + size]);
    memcpy(pBuffer.get(), &type, 1);

    writeUINT32(pBuffer.get()+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer.get()+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    for (i=0; i < len; i++)
    {
        sLen = strlen(pValueList[i]) + 1;
        writeUINT32(pBuffer.get()+VALUE_HEADEROFFSET+offset, sLen);

        offset += 4;
        writeUtf8(pBuffer.get()+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += sLen;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer.get(), VALUE_HEADERSIZE+size, writenBytes) )
    {
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    return RegError::NO_ERROR;
}

RegError ORegKey::setUnicodeListValue(std::u16string_view valueName, sal_Unicode** pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, storeAccessMode::Create) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 bytes (sal_uInt32) for the length

    sal_uInt32 i;
    for (i=0; i < len; i++)
    {
        size +=  4 + ((rtl_ustr_getLength(pValueList[i]) +1) * 2);
    }

    sal_uInt8 type = sal_uInt8(RegValueType::UNICODELIST);
    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE + size]);
    memcpy(pBuffer.get(), &type, 1);

    writeUINT32(pBuffer.get()+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer.get()+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    for (i=0; i < len; i++)
    {
        sLen = (rtl_ustr_getLength(pValueList[i]) + 1) * 2;
        writeUINT32(pBuffer.get()+VALUE_HEADEROFFSET+offset, sLen);

        offset += 4;
        writeString(pBuffer.get()+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += sLen;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer.get(), VALUE_HEADERSIZE+size, writenBytes) )
    {
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    return RegError::NO_ERROR;
}

RegError ORegKey::getValue(std::u16string_view valueName, RegValue value) const
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, accessMode) )
    {
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE]);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer.get(), VALUE_HEADERSIZE, readBytes) )
    {
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = pBuffer[0];
    valueType = static_cast<RegValueType>(type);
    readUINT32(pBuffer.get()+VALUE_TYPEOFFSET, valueSize);

    if (valueType > RegValueType::BINARY)
    {
        return RegError::INVALID_VALUE;
    }

    pBuffer.reset(new sal_uInt8[valueSize]);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer.get(), valueSize, readBytes) )
    {
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        return RegError::INVALID_VALUE;
    }

    switch (valueType)
    {
        case RegValueType::LONG:
            readINT32(pBuffer.get(), *static_cast<sal_Int32*>(value));
            break;
        case RegValueType::STRING:
            readUtf8(pBuffer.get(), static_cast<char*>(value), valueSize);
            break;
        case RegValueType::UNICODE:
            readString(pBuffer.get(), static_cast<sal_Unicode*>(value), valueSize);
            break;
        case RegValueType::BINARY:
            memcpy(value, pBuffer.get(), valueSize);
            break;
        default:
            memcpy(value, pBuffer.get(), valueSize);
            break;
    }

    return RegError::NO_ERROR;
}

RegError ORegKey::getLongListValue(std::u16string_view valueName, sal_Int32** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, accessMode) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE]);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer.get(), VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = pBuffer[0];
    valueType = static_cast<RegValueType>(type);

    if (valueType != RegValueType::LONGLIST)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    readUINT32(pBuffer.get()+VALUE_TYPEOFFSET, valueSize);

    /* check for 'reasonable' value */
    /* surely 10 millions entry in a registry list should be enough */
    if(valueSize > 40000000)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    pBuffer.reset(new sal_uInt8[valueSize]);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer.get(), valueSize, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer.get(), len);

    /* make sure the declared size of the array is consistent with the amount of data we have read */
    if(len > (valueSize - 4) / 4)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    *pLen = len;
    sal_Int32* pVList = static_cast<sal_Int32*>(rtl_allocateZeroMemory(len * sizeof(sal_Int32)));

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;

    for (sal_uInt32 i = 0; i < len; i++)
    {
        readINT32(pBuffer.get()+offset, pVList[i]);
        offset += 4;
    }

    *pValueList = pVList;
    return RegError::NO_ERROR;
}

RegError ORegKey::getStringListValue(std::u16string_view valueName, char*** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, accessMode) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE]);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer.get(), VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = pBuffer[0];
    valueType = static_cast<RegValueType>(type);

    if (valueType != RegValueType::STRINGLIST)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    readUINT32(pBuffer.get()+VALUE_TYPEOFFSET, valueSize);

    pBuffer.reset(new sal_uInt8[valueSize]);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer.get(), valueSize, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer.get(), len);

    *pLen = len;
    char** pVList = static_cast<char**>(rtl_allocateZeroMemory(len * sizeof(char*)));

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    char *pValue;
    for (sal_uInt32 i=0; i < len; i++)
    {
        readUINT32(pBuffer.get()+offset, sLen);

        offset += 4;

        pValue = static_cast<char*>(std::malloc(sLen));
        readUtf8(pBuffer.get()+offset, pValue, sLen);
        pVList[i] = pValue;

        offset += sLen;
    }

    *pValueList = pVList;
    return RegError::NO_ERROR;
}

RegError ORegKey::getUnicodeListValue(std::u16string_view valueName, sal_Unicode*** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    std::unique_ptr<sal_uInt8[]> pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    OUString sImplValueName = OUString::Concat(VALUE_PREFIX) + valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + ORegistry::ROOT, sImplValueName, accessMode) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer.reset(new sal_uInt8[VALUE_HEADERSIZE]);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer.get(), VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = pBuffer[0];
    valueType = static_cast<RegValueType>(type);

    if (valueType != RegValueType::UNICODELIST)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    readUINT32(pBuffer.get()+VALUE_TYPEOFFSET, valueSize);

    pBuffer.reset(new sal_uInt8[valueSize]);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer.get(), valueSize, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer.get(), len);

    *pLen = len;
    sal_Unicode** pVList = static_cast<sal_Unicode**>(rtl_allocateZeroMemory(len * sizeof(sal_Unicode*)));

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    sal_Unicode *pValue;
    for (sal_uInt32 i=0; i < len; i++)
    {
        readUINT32(pBuffer.get()+offset, sLen);

        offset += 4;

        pValue = static_cast<sal_Unicode*>(std::malloc((sLen / 2) * sizeof(sal_Unicode)));
        readString(pBuffer.get()+offset, pValue, sLen);
        pVList[i] = pValue;

        offset += sLen;
    }

    *pValueList = pVList;
    return RegError::NO_ERROR;
}


RegError ORegKey::getResolvedKeyName(std::u16string_view keyName,
                                     OUString& resolvedName) const
{
    if (keyName.empty())
        return RegError::INVALID_KEYNAME;

    resolvedName = getFullPath(keyName);
    return RegError::NO_ERROR;
}

sal_uInt32 ORegKey::countSubKeys()
{
    REG_GUARD(m_pRegistry->m_mutex);

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir = getStoreDir();
    storeError                  _err = rStoreDir.first(iter);
    sal_uInt32                  count = 0;

    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            count++;
        }

        _err = rStoreDir.next(iter);
    }

    return count;
}

OStoreDirectory ORegKey::getStoreDir() const
{
    OStoreDirectory rStoreDir;
    OUString        fullPath;
    OUString        relativName;
    storeAccessMode accessMode = storeAccessMode::ReadWrite;

    if ( m_name == ORegistry::ROOT )
    {
        fullPath.clear();
        relativName.clear();
    } else
    {
        fullPath = m_name.copy(0, m_name.lastIndexOf('/') + 1);
        relativName = m_name.copy(m_name.lastIndexOf('/') + 1);
    }

    if (m_pRegistry->isReadOnly())
    {
        accessMode = storeAccessMode::ReadOnly;
    }

    rStoreDir.create(getStoreFile(), fullPath, relativName, accessMode);

    return rStoreDir;
}

OUString ORegKey::getFullPath(std::u16string_view path) const {
    OSL_ASSERT(!m_name.isEmpty() && !path.empty());
    OUStringBuffer b(32);
    b.append(m_name);
    if (!b.isEmpty() && b[b.getLength() - 1] == '/') {
        if (path[0] == '/') {
            b.append(path.substr(1));
        } else {
            b.append(path);
        }
    } else {
        if (path[0] != '/') {
            b.append('/');
        }
        b.append(path);
    }
    return b.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
