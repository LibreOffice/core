/*************************************************************************
 *
 *  $RCSfile: keyimpl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:42 $
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

#ifndef __REFLCNST_HXX__
#include    "reflcnst.hxx"
#endif

#include "keyimpl.hxx"

#ifndef _RTL_ALLOC_H_
#include    <rtl/alloc.h>
#endif

#ifndef _RTL_MEMORY_H_
#include    <rtl/memory.h>
#endif

//*********************************************************************
//  ORegKey()
//
ORegKey::ORegKey(const OUString& keyName, const OStoreDirectory& rStoreDir, ORegistry* pReg)
    : m_refCount(1)
    , m_name(keyName)
    , m_bDeleted(sal_False)
    , m_isLink(sal_False)
    , m_pRegistry(pReg)
{
    if (pReg)
    {
        m_storeFile = pReg->getStoreFile();
    }

    checkLink();
}

//*********************************************************************
//  ORegKey()
//
ORegKey::ORegKey(const OUString& keyName, const OUString& linkName,
                 const OStoreDirectory& rStoreDir, ORegistry* pReg)
    : m_refCount(1)
    , m_name(keyName)
    , m_bDeleted(sal_False)
    , m_pRegistry(pReg)
{
    if (linkName.getLength())
    {
        m_link = linkName;
        m_isLink = sal_True;

        setValue(OUString( RTL_CONSTASCII_USTRINGPARAM("LINK_TARGET") ), RG_VALUETYPE_UNICODE,
                 (RegValue*)linkName.pData->buffer, linkName.getLength()+1);
    } else
    {
        m_isLink = sal_False;
    }
}

//*********************************************************************
//  ORegKey()
//
ORegKey::ORegKey()
    : m_refCount(1)
    , m_bDeleted(sal_False)
    , m_isLink(sal_False)
    , m_pRegistry(NULL)
{
}


//*********************************************************************
//  ~ORegKey()
//
ORegKey::~ORegKey()
{
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
RegError ORegKey::openKey(const OUString& keyName, RegKeyHandle* phOpenKey, RESOLVE eResolve)
{
    return (m_pRegistry->openKey(this, keyName, phOpenKey, eResolve));
}


//*********************************************************************
//  openSubKeys
//
RegError ORegKey::openSubKeys(const OUString& keyName, RegKeyHandle** phOpenSubKeys, sal_uInt32* pnSubKeys)
{
    ORegKey*        pKey;
    OUString        sFullKeyName, sSubKeyName;
    RegKeyHandle    hSKey, hSubKey;
    RegError        _ret = REG_NO_ERROR;
    sal_uInt32      nSubKeys;
    ORegKey*        *subKeys;

    if ( keyName.getLength() )
    {
        if ((_ret = openKey(keyName, &hSKey)))
        {
            *phOpenSubKeys = NULL;
            *pnSubKeys = 0;
            return _ret;
        }

        pKey = (ORegKey*)hSKey;
    } else
    {
        pKey = this;
    }

    nSubKeys = pKey->countSubKeys();

    *pnSubKeys = nSubKeys;
    subKeys = (ORegKey**)rtl_allocateZeroMemory(nSubKeys * sizeof(ORegKey*));

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);

    nSubKeys = 0;
    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR )
        {
            sSubKeyName = iter.m_pszName;
            sFullKeyName = pKey->getName();
            if (sFullKeyName.getLength() > 1)
                sFullKeyName += m_pRegistry->ROOT;
            sFullKeyName += sSubKeyName;

            if (_ret = pKey->openKey(sSubKeyName, &hSubKey))
            {
                *phOpenSubKeys = NULL;
                *pnSubKeys = 0;
                rtl_freeMemory(subKeys);
                return _ret;
            }

            subKeys[nSubKeys] = ((ORegKey*)hSubKey);

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *phOpenSubKeys = (RegKeyHandle*)subKeys;
    if (keyName.getLength())
    {
        closeKey(hSKey);
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
    ORegKey*            pKey;
    OUString            sFullKeyName, sSubKeyName;
    RegError            _ret = REG_NO_ERROR;
    sal_uInt32          nSubKeys;
    rtl_uString**       pSubKeys;

    if (keyName.getLength())
    {
        if (_ret = openKey(keyName, (RegKeyHandle*)&pKey, RESOLVE_PART))
        {
            *pSubKeyNames = NULL;
            *pnSubKeys = 0;
            return _ret;
        }
    } else
    {
        pKey = this;
    }

    nSubKeys = pKey->countSubKeys();

    *pnSubKeys = nSubKeys;
    pSubKeys = (rtl_uString**)rtl_allocateZeroMemory(nSubKeys * sizeof(rtl_uString*));

    OStoreDirectory::iterator   iter;
    OStoreDirectory             rStoreDir(pKey->getStoreDir());
    storeError                  _err = rStoreDir.first(iter);
    OUString                    subKey;

    nSubKeys = 0;

    while ( _err == store_E_None )
    {
        if ( iter.m_nAttrib & STORE_ATTRIB_ISDIR)
        {
            sSubKeyName = iter.m_pszName;
            sFullKeyName = pKey->getName();
            if (sFullKeyName.getLength() > 1)
                sFullKeyName += m_pRegistry->ROOT;
            sFullKeyName += sSubKeyName;

            subKey = sFullKeyName;
            rtl_uString_newFromString(&pSubKeys[nSubKeys], subKey.pData);

            nSubKeys++;
        }

        _err = rStoreDir.next(iter);
    }

    *pSubKeyNames = pSubKeys;
    if (keyName.getLength())
    {
        closeKey((RegKeyHandle)pKey);
    }
    return REG_NO_ERROR;
}


//*********************************************************************
//  closeSubKeys
//
RegError ORegKey::closeSubKeys(RegKeyHandle* phSubKeys, sal_uInt32 nSubKeys)
{
    RegError    _ret = REG_NO_ERROR;

    for (int i=0; i < nSubKeys; i++)
    {
        _ret = closeKey(phSubKeys[i]);
    }

    rtl_freeMemory(phSubKeys);
    return _ret;
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

    if ( rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
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

    if ( rValue.create(m_storeFile, m_name + m_pRegistry->ROOT , sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = vSize;

//    if (vType == RG_VALUETYPE_UNICODE)
//    {
//        size = (rtl_ustr_getLength((sal_Unicode*)value)+1) * 2;
//    }

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

    if (rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, VALUE_MODE_CREATE) )
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

    for (int i=0; i < len; i++)
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

    if (rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 Bytes (sal_uInt32) fuer die Laenge

    int i;
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

    if (rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, VALUE_MODE_CREATE) )
    {
        return REG_SET_VALUE_FAILED;
    }

    sal_uInt32 size = 4; // 4 Bytes (sal_uInt32) fuer die Laenge

    int i;
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

    if (rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
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

//  rtl_copyMemory(value, pBuffer, valueSize);
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

    if (rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
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

    for (int i=0; i < len; i++)
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

    if ( rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
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
    for (int i=0; i < len; i++)
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

    if ( rValue.create(m_storeFile, m_name + m_pRegistry->ROOT, sImplValueName, accessMode) )
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
    for (int i=0; i < len; i++)
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
    ORegKey*    pKey;
    RegError    _ret = REG_NO_ERROR;

    *pKeyType = RG_KEYTYPE;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( name.getLength() )
    {
        if ((_ret = ((ORegKey*)this)->openKey(name, (RegKeyHandle*)&pKey, RESOLVE_PART)))
            return _ret;

        if (pKey->isLink())
            *pKeyType = RG_LINKTYPE;

        ((ORegKey*)this)->closeKey((RegKeyHandle)pKey);
        return _ret;
    } else
    {
        if (m_isLink)
            *pKeyType = RG_LINKTYPE;

        return _ret;
    }
}

//*********************************************************************
//  createLink()
//
RegError ORegKey::createLink(const OUString& linkName, const OUString& linkTarget)
{
    ORegKey*    pKey;
    RegError    _ret = REG_NO_ERROR;

    if ( !linkName.getLength() )
        return REG_INVALID_LINKNAME;

    if ( !linkTarget.getLength() )
        return REG_INVALID_LINKTARGET;

    REG_GUARD(m_pRegistry->m_mutex);

    if (m_pRegistry->openKey(this, linkName, (RegKeyHandle*)&pKey, RESOLVE_PART))
    {
        if (_ret = m_pRegistry->createKey(this, linkName, (RegKeyHandle*)&pKey))
            return _ret;
    } else
    {
        if (!pKey->isLink())
        {
            m_pRegistry->closeKey(pKey);
            return REG_INVALID_LINK;
        }
    }

    if (_ret = pKey->setValue(OUString( RTL_CONSTASCII_USTRINGPARAM("LINK_TARGET") ),
                              RG_VALUETYPE_UNICODE, (RegValue)linkTarget.getStr(), (linkTarget.getLength()+1)*2))
    {
        m_pRegistry->closeKey(pKey);
        _ret = m_pRegistry->deleteKey(this, linkName);
        return _ret;
    }

    return m_pRegistry->closeKey(pKey);
}

//*********************************************************************
//  deleteLink()
//
RegError ORegKey::deleteLink(const OUString& linkName)
{
    return (m_pRegistry->deleteLink(this, linkName));
}

//*********************************************************************
//  getLinkTarget()
//
RegError ORegKey::getLinkTarget(const OUString& linkName, OUString& linkTarget) const
{
    ORegKey*        pKey;
    RegError        _ret = REG_NO_ERROR;

    REG_GUARD(m_pRegistry->m_mutex);

    if ( linkName.getLength() )
    {
        if (_ret = ((ORegKey*)this)->openKey(linkName, (RegKeyHandle*)&pKey, RESOLVE_PART))
            return REG_INVALID_LINK;

        _ret = pKey->getLinkTarget(OUString(), linkTarget);
        ((ORegKey*)this)->closeKey((RegKeyHandle)pKey);
        return _ret;
    } else
    {
        if (m_isLink)
        {
            linkTarget = m_link;
            return REG_NO_ERROR;
        } else
            return REG_LINKTARGET_NOT_EXIST;
    }
}

RegError ORegKey::getResolvedKeyName(const OUString& keyName,
                                     OUString& resolvedName,
                                     sal_Bool firstLinkOnly)
{
    return m_pRegistry->getResolvedKeyName((ORegKey*)this, keyName, resolvedName, firstLinkOnly);
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


//*********************************************************************
//  checkLink()
//
sal_Bool ORegKey::checkLink()
{
    RegValueType    valueType = RG_VALUETYPE_NOT_DEFINED;
    sal_uInt32      valueSize = 0;

    OUString linkTarget( OUString( RTL_CONSTASCII_USTRINGPARAM("LINK_TARGET") ));
    if (!((ORegKey*)this)->getValueInfo( linkTarget, &valueType, &valueSize))
    {
        sal_Unicode* value = (sal_Unicode*)rtl_allocateMemory(valueSize);
        if (!((ORegKey*)this)->getValue( linkTarget, value))
        {
             m_link = OUString(value);
            m_isLink = sal_True;
        }

        rtl_freeMemory(value);

        return sal_True;
    }

    return sal_False;
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

    rStoreDir.create(m_storeFile, fullPath, relativName, accessMode);

    return rStoreDir;
}

