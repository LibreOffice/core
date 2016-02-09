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

#include "keyimpl.hxx"

#include "reflcnst.hxx"
#include "rtl/alloc.h"
#include "rtl/ustrbuf.hxx"
#include <osl/diagnose.h>
#include <sal/log.hxx>

using namespace store;

namespace { static char const VALUE_PREFIX[] = "$VL_"; }


//  ORegKey()

ORegKey::ORegKey(const OUString& keyName, ORegistry* pReg)
    : m_refCount(1)
    , m_name(keyName)
    , m_bDeleted(false)
    , m_bModified(false)
    , m_pRegistry(pReg)
{
}


//  ~ORegKey()

ORegKey::~ORegKey()
{
    SAL_WARN_IF(m_refCount != 0, "registry", "registry::ORegKey::dtor(): refcount not zero.");
}


//  releaseKey

RegError ORegKey::releaseKey(RegKeyHandle hKey)
{
    return m_pRegistry->releaseKey(hKey);
}


//  createKey

RegError ORegKey::createKey(const OUString& keyName, RegKeyHandle* phNewKey)
{
    return m_pRegistry->createKey(this, keyName, phNewKey);
}


//  openKey

RegError ORegKey::openKey(const OUString& keyName, RegKeyHandle* phOpenKey)
{
    return m_pRegistry->openKey(this, keyName, phOpenKey);
}


//  openSubKeys

RegError ORegKey::openSubKeys(const OUString& keyName, RegKeyHandle** phOpenSubKeys, sal_uInt32* pnSubKeys)
{
    RegError _ret = RegError::NO_ERROR;

    *phOpenSubKeys = nullptr;
    *pnSubKeys = 0;

    ORegKey* pKey = this;
    if ( !keyName.isEmpty() )
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
            OUString const sSubKeyName = iter.m_pszName;

            ORegKey* pOpenSubKey = nullptr;
            _ret = pKey->openKey(sSubKeyName, reinterpret_cast<RegKeyHandle*>(&pOpenSubKey));
            if (_ret != RegError::NO_ERROR)
            {
                *phOpenSubKeys = nullptr;
                *pnSubKeys = 0;
                rtl_freeMemory(pSubKeys); // @@@ leaking 'pSubKeys[0...nSubkeys-1]'
                return _ret;              // @@@ leaking 'pKey'
            }

            pSubKeys[nSubKeys] = pOpenSubKey;

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *phOpenSubKeys = reinterpret_cast<RegKeyHandle*>(pSubKeys);
    if (!keyName.isEmpty())
    {
        (void) releaseKey(pKey);
    }
    return RegError::NO_ERROR;
}


//  getKeyNames

RegError ORegKey::getKeyNames(const OUString& keyName,
                              rtl_uString*** pSubKeyNames,
                              sal_uInt32* pnSubKeys)
{
    RegError _ret = RegError::NO_ERROR;

    *pSubKeyNames = nullptr;
    *pnSubKeys = 0;

    ORegKey* pKey = this;
    if (!keyName.isEmpty())
    {
        _ret = openKey(keyName, reinterpret_cast<RegKeyHandle*>(&pKey));
        if (_ret != RegError::NO_ERROR)
            return _ret;
    }

    sal_uInt32 nSubKeys = pKey->countSubKeys();
    *pnSubKeys = nSubKeys;

    rtl_uString** pSubKeys = nullptr;
    pSubKeys = static_cast<rtl_uString**>(rtl_allocateZeroMemory(nSubKeys * sizeof(rtl_uString*)));

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    nSubKeys = 0;

    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            OUString const sSubKeyName = iter.m_pszName;

            OUString sFullKeyName(pKey->getName());
            if (sFullKeyName.getLength() > 1)
                sFullKeyName += m_pRegistry->ROOT;
            sFullKeyName += sSubKeyName;

            rtl_uString_newFromString(&pSubKeys[nSubKeys], sFullKeyName.pData);

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *pSubKeyNames = pSubKeys;
    if (!keyName.isEmpty())
    {
        releaseKey(pKey);
    }
    return RegError::NO_ERROR;
}


//  closeKey

RegError ORegKey::closeKey(RegKeyHandle hKey)
{
    return (m_pRegistry->closeKey(hKey));
}


//  deleteKey

RegError ORegKey::deleteKey(const OUString& keyName)
{
    return (m_pRegistry->deleteKey(this, keyName));
}


//  getValueType

RegError ORegKey::getValueInfo(const OUString& valueName, RegValueType* pValueType, sal_uInt32* pValueSize) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    storeAccessMode accessMode = store_AccessReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = store_AccessReadOnly;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(m_pRegistry->getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        *pValueType = RegValueType::NOT_DEFINED;
        *pValueSize = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE));

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt32  size;
    sal_uInt8   type = *pBuffer;
    readUINT32(pBuffer+VALUE_TYPEOFFSET, size);

    *pValueType = (RegValueType)type;
//    if (*pValueType == RegValueType::UNICODE)
//    {
//        *pValueSize = (size / 2) * sizeof(sal_Unicode);
//    } else
//    {
        if (*pValueType > RegValueType::BINARY)
        {
            rtl_freeMemory(pBuffer);
            pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(4));
            rValue.readAt(VALUE_HEADEROFFSET, pBuffer, 4, readBytes);

            readUINT32(pBuffer, size);
        }

        *pValueSize = size;
//    }

    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  setValue

RegError ORegKey::setValue(const OUString& valueName, RegValueType vType, RegValue value, sal_uInt32 vSize)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    if (vType > RegValueType::BINARY)
    {
        return RegError::INVALID_VALUE;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT , sImplValueName, store_AccessCreate) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = vSize;

    sal_uInt8 type = (sal_uInt8)vType;
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE + size));
    memcpy(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);

    switch (vType)
    {
        case RegValueType::NOT_DEFINED:
            memcpy(pBuffer+VALUE_HEADEROFFSET, value, size);
            break;
        case RegValueType::LONG:
            writeINT32(pBuffer+VALUE_HEADEROFFSET, *static_cast<sal_Int32*>(value));
            break;
        case RegValueType::STRING:
            writeUtf8(pBuffer+VALUE_HEADEROFFSET, static_cast<const sal_Char*>(value));
            break;
        case RegValueType::UNICODE:
            writeString(pBuffer+VALUE_HEADEROFFSET, static_cast<const sal_Unicode*>(value));
            break;
        case RegValueType::BINARY:
            memcpy(pBuffer+VALUE_HEADEROFFSET, value, size);
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+size, writenBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  setLongListValue

RegError ORegKey::setLongListValue(const OUString& valueName, sal_Int32* pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, store_AccessCreate) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 bytes (sal_uInt32) for the length

    size += len * 4;

    sal_uInt8 type = (sal_uInt8)RegValueType::LONGLIST;
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE + size));
    memcpy(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array

    for (sal_uInt32 i=0; i < len; i++)
    {
        writeINT32(pBuffer+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += 4;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+size, writenBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADEROFFSET+size))
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  setStringListValue

RegError ORegKey::setStringListValue(const OUString& valueName, sal_Char** pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, store_AccessCreate) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 bytes (sal_uInt32) for the length

    sal_uInt32 i;
    for (i=0; i < len; i++)
    {
        size +=  4 + strlen(pValueList[i]) + 1;
    }

    sal_uInt8 type = (sal_uInt8)RegValueType::STRINGLIST;
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE + size));
    memcpy(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    for (i=0; i < len; i++)
    {
        sLen = strlen(pValueList[i]) + 1;
        writeUINT32(pBuffer+VALUE_HEADEROFFSET+offset, sLen);

        offset += 4;
        writeUtf8(pBuffer+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += sLen;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+size, writenBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  setUnicodeListValue

RegError ORegKey::setUnicodeListValue(const OUString& valueName, sal_Unicode** pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return RegError::REGISTRY_READONLY;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, store_AccessCreate) )
    {
        return RegError::SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 bytes (sal_uInt32) for the length

    sal_uInt32 i;
    for (i=0; i < len; i++)
    {
        size +=  4 + ((rtl_ustr_getLength(pValueList[i]) +1) * 2);
    }

    sal_uInt8 type = (sal_uInt8)RegValueType::UNICODELIST;
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE + size));
    memcpy(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    for (i=0; i < len; i++)
    {
        sLen = (rtl_ustr_getLength(pValueList[i]) + 1) * 2;
        writeUINT32(pBuffer+VALUE_HEADEROFFSET+offset, sLen);

        offset += 4;
        writeString(pBuffer+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += sLen;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+size, writenBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        rtl_freeMemory(pBuffer);
        return RegError::SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  getValue

RegError ORegKey::getValue(const OUString& valueName, RegValue value) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = store_AccessReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = store_AccessReadOnly;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE));

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = *pBuffer;
    valueType = (RegValueType)type;
    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    if (valueType > RegValueType::BINARY)
    {
        return RegError::INVALID_VALUE;
    }

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(valueSize));

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    switch (valueType)
    {
        case RegValueType::NOT_DEFINED:
            memcpy(value, pBuffer, valueSize);
            break;
        case RegValueType::LONG:
            readINT32(pBuffer, *static_cast<sal_Int32*>(value));
            break;
        case RegValueType::STRING:
            readUtf8(pBuffer, static_cast<sal_Char*>(value), valueSize);
            break;
        case RegValueType::UNICODE:
            readString(pBuffer, static_cast<sal_Unicode*>(value), valueSize);
            break;
        case RegValueType::BINARY:
            memcpy(value, pBuffer, valueSize);
            break;
        // coverity[dead_error_begin] - following conditions exist to avoid compiler warning
        case RegValueType::LONGLIST:
        case RegValueType::STRINGLIST:
        case RegValueType::UNICODELIST:
            memcpy(value, pBuffer, valueSize);
            break;
    }


    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  getLongListValue

RegError ORegKey::getLongListValue(const OUString& valueName, sal_Int32** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = store_AccessReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = store_AccessReadOnly;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE));

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = *pBuffer;
    valueType = (RegValueType)type;

    if (valueType != RegValueType::LONGLIST)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    /* check for 'reasonable' value */
    /* surely 10 millions entry in a registry list should be enough */
    if(valueSize > 40000000)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(valueSize));

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer, len);

    /* make sure the declared size of the arry is consistent with the amount of data we have read */
    if(len > (valueSize - 4) / 4)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    *pLen = len;
    sal_Int32* pVList = static_cast<sal_Int32*>(rtl_allocateZeroMemory(len * sizeof(sal_Int32)));

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;

    for (sal_uInt32 i = 0; i < len; i++)
    {
        readINT32(pBuffer+offset, pVList[i]);
        offset += 4;
    }

    *pValueList = pVList;
    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  getStringListValue

RegError ORegKey::getStringListValue(const OUString& valueName, sal_Char*** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = store_AccessReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = store_AccessReadOnly;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE));

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = *pBuffer;
    valueType = (RegValueType)type;

    if (valueType != RegValueType::STRINGLIST)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(valueSize));

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer, len);

    *pLen = len;
    sal_Char** pVList = static_cast<sal_Char**>(rtl_allocateZeroMemory(len * sizeof(sal_Char*)));

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    sal_Char *pValue;
    for (sal_uInt32 i=0; i < len; i++)
    {
        readUINT32(pBuffer+offset, sLen);

        offset += 4;

        pValue = static_cast<sal_Char*>(rtl_allocateMemory(sLen));
        readUtf8(pBuffer+offset, pValue, sLen);
        pVList[i] = pValue;

        offset += sLen;
    }

    *pValueList = pVList;
    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


//  getUnicodeListValue

RegError ORegKey::getUnicodeListValue(const OUString& valueName, sal_Unicode*** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = store_AccessReadWrite;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = store_AccessReadOnly;
    }

    OUString sImplValueName( VALUE_PREFIX );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        pValueList = nullptr;
        *pLen = 0;
        return RegError::VALUE_NOT_EXISTS;
    }

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(VALUE_HEADERSIZE));

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt8   type = *pBuffer;
    valueType = (RegValueType)type;

    if (valueType != RegValueType::UNICODELIST)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    pBuffer = static_cast<sal_uInt8*>(rtl_allocateMemory(valueSize));

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = nullptr;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return RegError::INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer, len);

    *pLen = len;
    sal_Unicode** pVList = static_cast<sal_Unicode**>(rtl_allocateZeroMemory(len * sizeof(sal_Unicode*)));

    sal_uInt32 offset = 4; // initial 4 bytes for the size of the array;
    sal_uInt32 sLen = 0;

    sal_Unicode *pValue;
    for (sal_uInt32 i=0; i < len; i++)
    {
        readUINT32(pBuffer+offset, sLen);

        offset += 4;

        pValue = static_cast<sal_Unicode*>(rtl_allocateMemory((sLen / 2) * sizeof(sal_Unicode)));
        readString(pBuffer+offset, pValue, sLen);
        pVList[i] = pValue;

        offset += sLen;
    }

    *pValueList = pVList;
    rtl_freeMemory(pBuffer);
    return RegError::NO_ERROR;
}


RegError ORegKey::getResolvedKeyName(const OUString& keyName,
                                     OUString& resolvedName)
{
    if (keyName.isEmpty())
        return RegError::INVALID_KEYNAME;

    resolvedName = getFullPath(keyName);
    return RegError::NO_ERROR;
}


//  countSubKeys()

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

OStoreDirectory ORegKey::getStoreDir()
{
    OStoreDirectory rStoreDir;
    OUString        fullPath;
    OUString        relativName;
    storeAccessMode accessMode = store_AccessReadWrite;

    if ( m_name.equals(m_pRegistry->ROOT) )
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
        accessMode = store_AccessReadOnly;
    }

    rStoreDir.create(getStoreFile(), fullPath, relativName, accessMode);

    return rStoreDir;
}

OUString ORegKey::getFullPath(OUString const & path) const {
    OSL_ASSERT(!m_name.isEmpty() && !path.isEmpty());
    OUStringBuffer b(m_name);
    if (!b.isEmpty() && b[b.getLength() - 1] == '/') {
        if (path[0] == '/') {
            b.append(path.getStr() + 1, path.getLength() - 1);
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
