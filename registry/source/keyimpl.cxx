/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_registry.hxx"

#include "keyimpl.hxx"

#include "reflcnst.hxx"
#include "rtl/alloc.h"
#include "rtl/memory.h"
#include "rtl/ustrbuf.hxx"

using rtl::OUString;
using rtl::OUStringBuffer;
using namespace store;

namespace { static char const VALUE_PREFIX[] = "$VL_"; }

//*********************************************************************
//  ORegKey()
//
ORegKey::ORegKey(const OUString& keyName, ORegistry* pReg)
    : m_refCount(1)
    , m_name(keyName)
    , m_bDeleted(0)
    , m_bModified(0)
    , m_pRegistry(pReg)
{
}

//*********************************************************************
//  ~ORegKey()
//
ORegKey::~ORegKey()
{
    OSL_POSTCOND(m_refCount == 0, "registry::ORegKey::dtor(): refcount not zero.");
}

//*********************************************************************
//  acquireKey
//
RegError ORegKey::acquireKey(RegKeyHandle hKey)
{
    return m_pRegistry->acquireKey(hKey);
}

//*********************************************************************
//  releaseKey
//
RegError ORegKey::releaseKey(RegKeyHandle hKey)
{
    return m_pRegistry->releaseKey(hKey);
}

//*********************************************************************
//  createKey
//
RegError ORegKey::createKey(const OUString& keyName, RegKeyHandle* phNewKey)
{
    return m_pRegistry->createKey(this, keyName, phNewKey);
}


//*********************************************************************
//  openKey
//
RegError ORegKey::openKey(const OUString& keyName, RegKeyHandle* phOpenKey)
{
    return m_pRegistry->openKey(this, keyName, phOpenKey);
}


//*********************************************************************
//  openSubKeys
//
RegError ORegKey::openSubKeys(const OUString& keyName, RegKeyHandle** phOpenSubKeys, sal_uInt32* pnSubKeys)
{
    RegError _ret = REG_NO_ERROR;

    *phOpenSubKeys = 0;
    *pnSubKeys = 0;

    ORegKey* pKey = this;
    if ( keyName.getLength() )
    {
        _ret = openKey(keyName, (RegKeyHandle*)&pKey);
        if (_ret != REG_NO_ERROR)
            return _ret;
    }

    sal_uInt32 nSubKeys = pKey->countSubKeys();
    *pnSubKeys = nSubKeys;

    ORegKey** pSubKeys;
    pSubKeys = (ORegKey**)rtl_allocateZeroMemory(nSubKeys * sizeof(ORegKey*));

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    nSubKeys = 0;
    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            OUString const sSubKeyName = iter.m_pszName;

            ORegKey* pOpenSubKey = 0;
            _ret = pKey->openKey(sSubKeyName, (RegKeyHandle*)&pOpenSubKey);
            if (_ret != REG_NO_ERROR)
            {
                *phOpenSubKeys = NULL;
                *pnSubKeys = 0;
                rtl_freeMemory(pSubKeys); // @@@ leaking 'pSubKeys[0...nSubkeys-1]'
                return _ret;              // @@@ leaking 'pKey'
            }

            pSubKeys[nSubKeys] = pOpenSubKey;

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *phOpenSubKeys = (RegKeyHandle*)pSubKeys;
    if (keyName.getLength())
    {
        (void) releaseKey(pKey);
    }
    return REG_NO_ERROR;
}


//*********************************************************************
//  getKeyNames
//
RegError ORegKey::getKeyNames(const OUString& keyName,
                              rtl_uString*** pSubKeyNames,
                              sal_uInt32* pnSubKeys)
{
    RegError _ret = REG_NO_ERROR;

    *pSubKeyNames = 0;
    *pnSubKeys = 0;

    ORegKey* pKey = this;
    if (keyName.getLength())
    {
        _ret = openKey(keyName, (RegKeyHandle*)&pKey);
        if (_ret != REG_NO_ERROR)
            return _ret;
    }

    sal_uInt32 nSubKeys = pKey->countSubKeys();
    *pnSubKeys = nSubKeys;

    rtl_uString** pSubKeys = 0;
    pSubKeys = (rtl_uString**)rtl_allocateZeroMemory(nSubKeys * sizeof(rtl_uString*));

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
    if (keyName.getLength())
    {
        releaseKey(pKey);
    }
    return REG_NO_ERROR;
}


//*********************************************************************
//  closeKey
//
RegError ORegKey::closeKey(RegKeyHandle hKey)
{
    return (m_pRegistry->closeKey(hKey));
}


//*********************************************************************
//  deleteKey
//
RegError ORegKey::deleteKey(const OUString& keyName)
{
    return (m_pRegistry->deleteKey(this, keyName));
}


//*********************************************************************
//  getValueType
//
RegError ORegKey::getValueInfo(const OUString& valueName, RegValueType* pValueType, sal_uInt32* pValueSize) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    storeAccessMode accessMode = VALUE_MODE_OPEN;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = VALUE_MODE_OPENREAD;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(m_pRegistry->getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        *pValueType = RG_VALUETYPE_NOT_DEFINED;
        *pValueSize = 0;
        return REG_VALUE_NOT_EXISTS;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt32  size;
    sal_uInt8   type = *((sal_uInt8*)pBuffer);
    readUINT32(pBuffer+VALUE_TYPEOFFSET, size);

    *pValueType = (RegValueType)type;
//    if (*pValueType == RG_VALUETYPE_UNICODE)
//    {
//        *pValueSize = (size / 2) * sizeof(sal_Unicode);
//    } else
//    {
        if (*pValueType > 4)
        {
            rtl_freeMemory(pBuffer);
            pBuffer = (sal_uInt8*)rtl_allocateMemory(4);
            rValue.readAt(VALUE_HEADEROFFSET, pBuffer, 4, readBytes);

            readUINT32(pBuffer, size);
        }

        *pValueSize = size;
//    }

    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}


//*********************************************************************
//  setValue
//
RegError ORegKey::setValue(const OUString& valueName, RegValueType vType, RegValue value, sal_uInt32 vSize)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return REG_REGISTRY_READONLY;
    }

    if (vType > 4)
    {
        return REG_INVALID_VALUE;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT , sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = vSize;

    sal_uInt8 type = (sal_uInt8)vType;
    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE + size);
    rtl_copyMemory(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);

    switch (vType)
    {
        case RG_VALUETYPE_NOT_DEFINED:
            rtl_copyMemory(pBuffer+VALUE_HEADEROFFSET, value, size);
            break;
        case RG_VALUETYPE_LONG:
            writeINT32(pBuffer+VALUE_HEADEROFFSET, *((sal_Int32*)value));
            break;
        case RG_VALUETYPE_STRING:
            writeUtf8(pBuffer+VALUE_HEADEROFFSET, (const sal_Char*)value);
            break;
        case RG_VALUETYPE_UNICODE:
            writeString(pBuffer+VALUE_HEADEROFFSET, (const sal_Unicode*)value);
            break;
        case RG_VALUETYPE_BINARY:
            rtl_copyMemory(pBuffer+VALUE_HEADEROFFSET, value, size);
            break;
        default:
            OSL_ASSERT(false);
            break;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+size, writenBytes) )
    {
        rtl_freeMemory(pBuffer);
        return REG_SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        rtl_freeMemory(pBuffer);
        return REG_SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  setLongListValue
//
RegError ORegKey::setLongListValue(const OUString& valueName, sal_Int32* pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return REG_REGISTRY_READONLY;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 Bytes (sal_uInt32) fuer die Laenge

    size += len * 4;

    sal_uInt8 type = (sal_uInt8)RG_VALUETYPE_LONGLIST;
    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE + size);
    rtl_copyMemory(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays

    for (sal_uInt32 i=0; i < len; i++)
    {
        writeINT32(pBuffer+VALUE_HEADEROFFSET+offset, pValueList[i]);
        offset += 4;
    }

    sal_uInt32  writenBytes;
    if ( rValue.writeAt(0, pBuffer, VALUE_HEADERSIZE+size, writenBytes) )
    {
        rtl_freeMemory(pBuffer);
        return REG_SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADEROFFSET+size))
    {
        rtl_freeMemory(pBuffer);
        return REG_SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  setStringListValue
//
RegError ORegKey::setStringListValue(const OUString& valueName, sal_Char** pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return REG_REGISTRY_READONLY;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 Bytes (sal_uInt32) fuer die Laenge

    sal_uInt32 i;
    for (i=0; i < len; i++)
    {
        size +=  4 + strlen(pValueList[i]) + 1;
    }

    sal_uInt8 type = (sal_uInt8)RG_VALUETYPE_STRINGLIST;
    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE + size);
    rtl_copyMemory(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays;
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
        return REG_SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        rtl_freeMemory(pBuffer);
        return REG_SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  setUnicodeListValue
//
RegError ORegKey::setUnicodeListValue(const OUString& valueName, sal_Unicode** pValueList, sal_uInt32 len)
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;

    if (m_pRegistry->isReadOnly())
    {
        return REG_REGISTRY_READONLY;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 Bytes (sal_uInt32) fuer die Laenge

    sal_uInt32 i;
    for (i=0; i < len; i++)
    {
        size +=  4 + ((rtl_ustr_getLength(pValueList[i]) +1) * 2);
    }

    sal_uInt8 type = (sal_uInt8)RG_VALUETYPE_UNICODELIST;
    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE + size);
    rtl_copyMemory(pBuffer, &type, 1);

    writeUINT32(pBuffer+VALUE_TYPEOFFSET, size);
    writeUINT32(pBuffer+VALUE_HEADEROFFSET, len);

    sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays;
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
        return REG_SET_VALUE_FAILED;
    }
    if (writenBytes != (VALUE_HEADERSIZE+size))
    {
        rtl_freeMemory(pBuffer);
        return REG_SET_VALUE_FAILED;
    }
    setModified();

    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  getValue
//
RegError ORegKey::getValue(const OUString& valueName, RegValue value) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = VALUE_MODE_OPEN;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = VALUE_MODE_OPENREAD;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        return REG_VALUE_NOT_EXISTS;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt8   type = *((sal_uInt8*)pBuffer);
    valueType = (RegValueType)type;
    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    if (valueType > 4)
    {
        return REG_INVALID_VALUE;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    switch (valueType)
    {
        case RG_VALUETYPE_NOT_DEFINED:
            rtl_copyMemory(value, pBuffer, valueSize);
            break;
        case RG_VALUETYPE_LONG:
            readINT32(pBuffer, *((sal_Int32*)value));
            break;
        case RG_VALUETYPE_STRING:
            readUtf8(pBuffer, (sal_Char*)value, valueSize);
            break;
        case RG_VALUETYPE_UNICODE:
            readString(pBuffer, (sal_Unicode*)value, valueSize);
            break;
        case RG_VALUETYPE_BINARY:
            rtl_copyMemory(value, pBuffer, valueSize);
            break;
        case RG_VALUETYPE_LONGLIST:
        case RG_VALUETYPE_STRINGLIST:
        case RG_VALUETYPE_UNICODELIST:
            rtl_copyMemory(value, pBuffer, valueSize);
            break;
    }


    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  getLongListValue
//
RegError ORegKey::getLongListValue(const OUString& valueName, sal_Int32** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = VALUE_MODE_OPEN;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = VALUE_MODE_OPENREAD;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if (rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        pValueList = NULL;
        *pLen = 0;
        return REG_VALUE_NOT_EXISTS;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt8   type = *((sal_uInt8*)pBuffer);
    valueType = (RegValueType)type;

    if (valueType != RG_VALUETYPE_LONGLIST)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer, len);

    *pLen = len;
    sal_Int32* pVList = (sal_Int32*)rtl_allocateZeroMemory(len * sizeof(sal_Int32));

    sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays;

    for (sal_uInt32 i=0; i < len; i++)
    {
        readINT32(pBuffer+offset, pVList[i]);
        offset += 4;
    }

    *pValueList = pVList;
    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  getStringListValue
//
RegError ORegKey::getStringListValue(const OUString& valueName, sal_Char*** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = VALUE_MODE_OPEN;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = VALUE_MODE_OPENREAD;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        pValueList = NULL;
        *pLen = 0;
        return REG_VALUE_NOT_EXISTS;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt8   type = *((sal_uInt8*)pBuffer);
    valueType = (RegValueType)type;

    if (valueType != RG_VALUETYPE_STRINGLIST)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer, len);

    *pLen = len;
    sal_Char** pVList = (sal_Char**)rtl_allocateZeroMemory(len * sizeof(sal_Char*));

    sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays;
    sal_uInt32 sLen = 0;

    sal_Char *pValue;
    for (sal_uInt32 i=0; i < len; i++)
    {
        readUINT32(pBuffer+offset, sLen);

        offset += 4;

        pValue = (sal_Char*)rtl_allocateMemory(sLen);
        readUtf8(pBuffer+offset, pValue, sLen);
        pVList[i] = pValue;

        offset += sLen;
    }

    *pValueList = pVList;
    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  getUnicodeListValue
//
RegError ORegKey::getUnicodeListValue(const OUString& valueName, sal_Unicode*** pValueList, sal_uInt32* pLen) const
{
    OStoreStream    rValue;
    sal_uInt8*      pBuffer;
    RegValueType    valueType;
    sal_uInt32      valueSize;
    storeAccessMode accessMode = VALUE_MODE_OPEN;

    if (m_pRegistry->isReadOnly())
    {
        accessMode = VALUE_MODE_OPENREAD;
    }

    OUString sImplValueName( RTL_CONSTASCII_USTRINGPARAM(VALUE_PREFIX) );
    sImplValueName += valueName;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( rValue.create(getStoreFile(), m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
    {
        pValueList = NULL;
        *pLen = 0;
        return REG_VALUE_NOT_EXISTS;
    }

    pBuffer = (sal_uInt8*)rtl_allocateMemory(VALUE_HEADERSIZE);

    sal_uInt32  readBytes;
    if ( rValue.readAt(0, pBuffer, VALUE_HEADERSIZE, readBytes) )
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != VALUE_HEADERSIZE)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt8   type = *((sal_uInt8*)pBuffer);
    valueType = (RegValueType)type;

    if (valueType != RG_VALUETYPE_UNICODELIST)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    readUINT32(pBuffer+VALUE_TYPEOFFSET, valueSize);

    rtl_freeMemory(pBuffer);

    pBuffer = (sal_uInt8*)rtl_allocateMemory(valueSize);

    if ( rValue.readAt(VALUE_HEADEROFFSET, pBuffer, valueSize, readBytes) )
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }
    if (readBytes != valueSize)
    {
        pValueList = NULL;
        *pLen = 0;
        rtl_freeMemory(pBuffer);
        return REG_INVALID_VALUE;
    }

    sal_uInt32 len = 0;
    readUINT32(pBuffer, len);

    *pLen = len;
    sal_Unicode** pVList = (sal_Unicode**)rtl_allocateZeroMemory(len * sizeof(sal_Unicode*));

    sal_uInt32 offset = 4; // initial 4 Bytes fuer die Laenge des Arrays;
    sal_uInt32 sLen = 0;

    sal_Unicode *pValue;
    for (sal_uInt32 i=0; i < len; i++)
    {
        readUINT32(pBuffer+offset, sLen);

        offset += 4;

        pValue = (sal_Unicode*)rtl_allocateMemory((sLen / 2) * sizeof(sal_Unicode));
        readString(pBuffer+offset, pValue, sLen);
        pVList[i] = pValue;

        offset += sLen;
    }

    *pValueList = pVList;
    rtl_freeMemory(pBuffer);
    return REG_NO_ERROR;
}

//*********************************************************************
//  getKeyType()
//
RegError ORegKey::getKeyType(const OUString& name, RegKeyType* pKeyType) const
{
    *pKeyType = RG_KEYTYPE;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( name.getLength() )
    {
        ORegKey* pThis = const_cast< ORegKey* >(this);

        RegKeyHandle hKey = 0;
        RegError _ret = pThis->openKey(name, &hKey);
        if (_ret != REG_NO_ERROR)
            return _ret;
        (void) pThis->releaseKey(hKey);
    }

    return REG_NO_ERROR;
}

RegError ORegKey::getResolvedKeyName(const OUString& keyName,
                                     OUString& resolvedName)
{
    if (keyName.getLength() == 0)
        return REG_INVALID_KEYNAME;

    resolvedName = getFullPath(keyName);
    return REG_NO_ERROR;
}

//*********************************************************************
//  countSubKeys()
//
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
    storeAccessMode accessMode = KEY_MODE_OPEN;

    if ( m_name.equals(m_pRegistry->ROOT) )
    {
        fullPath = OUString();
        relativName = OUString();
    } else
    {
        fullPath = m_name.copy(0, m_name.lastIndexOf('/') + 1);
        relativName = m_name.copy(m_name.lastIndexOf('/') + 1);
    }

    if (m_pRegistry->isReadOnly())
    {
        accessMode = KEY_MODE_OPENREAD;
    }

    rStoreDir.create(getStoreFile(), fullPath, relativName, accessMode);

    return rStoreDir;
}

OUString ORegKey::getFullPath(OUString const & path) const {
    OSL_ASSERT(m_name.getLength() != 0 && path.getLength() != 0);
    OUStringBuffer b(m_name);
    if (b.charAt(b.getLength() - 1) == '/') {
        if (path[0] == '/') {
            b.append(path.getStr() + 1, path.getLength() - 1);
        } else {
            b.append(path);
        }
    } else {
        if (path[0] != '/') {
            b.append(sal_Unicode('/'));
        }
        b.append(path);
    }
    return b.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
