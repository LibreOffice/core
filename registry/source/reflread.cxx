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


#include <memory>
#include <new>

#include <string.h>
#include <sal/types.h>
#include <osl/endian.h>
#include <registry/reflread.hxx>

#include "registry/reader.h"
#include "registry/version.h"

#include "reflcnst.hxx"

#include <cstddef>

static sal_Char NULL_STRING[1] = { 0 };
static sal_Unicode NULL_WSTRING[1] = { 0 };

const sal_uInt32    magic = 0x12345678;
const sal_uInt16 minorVersion = 0x0000;
const sal_uInt16 majorVersion = 0x0001;

/**************************************************************************

    class BlopObject

    holds any data in a flat memory buffer

**************************************************************************/

class BlopObject
{
public:
    const sal_uInt8* m_pBuffer;
    sal_uInt32      m_bufferLen;
    bool            m_isCopied;

    BlopObject(const sal_uInt8* buffer, sal_uInt32 len, bool copyBuffer);
        // throws std::bad_alloc

    ~BlopObject();

    inline sal_uInt8 readBYTE(sal_uInt32 index) const
    {
        return m_pBuffer[index];
    }

    inline sal_Int16 readINT16(sal_uInt32 index) const
    {
        return ((m_pBuffer[index] << 8) | (m_pBuffer[index+1] << 0));
    }

    inline sal_uInt16 readUINT16(sal_uInt32 index) const
    {
        return ((m_pBuffer[index] << 8) | (m_pBuffer[index+1] << 0));
    }

    inline sal_Int32 readINT32(sal_uInt32 index) const
    {
        return (
            (m_pBuffer[index]   << 24) |
            (m_pBuffer[index+1] << 16) |
            (m_pBuffer[index+2] << 8)  |
            (m_pBuffer[index+3] << 0)
        );
    }

    inline sal_uInt32 readUINT32(sal_uInt32 index) const
    {
        return (
            (m_pBuffer[index]   << 24) |
            (m_pBuffer[index+1] << 16) |
            (m_pBuffer[index+2] << 8)  |
            (m_pBuffer[index+3] << 0)
        );
    }

    inline sal_Int64 readINT64(sal_uInt32 index) const
    {
        return (
            ((sal_Int64)m_pBuffer[index]   << 56) |
            ((sal_Int64)m_pBuffer[index+1] << 48) |
            ((sal_Int64)m_pBuffer[index+2] << 40) |
            ((sal_Int64)m_pBuffer[index+3] << 32) |
            ((sal_Int64)m_pBuffer[index+4] << 24) |
            ((sal_Int64)m_pBuffer[index+5] << 16) |
            ((sal_Int64)m_pBuffer[index+6] << 8)  |
            ((sal_Int64)m_pBuffer[index+7] << 0)
        );
    }

    inline sal_uInt64 readUINT64(sal_uInt32 index) const
    {
        return (
            ((sal_uInt64)m_pBuffer[index]   << 56) |
            ((sal_uInt64)m_pBuffer[index+1] << 48) |
            ((sal_uInt64)m_pBuffer[index+2] << 40) |
            ((sal_uInt64)m_pBuffer[index+3] << 32) |
            ((sal_uInt64)m_pBuffer[index+4] << 24) |
            ((sal_uInt64)m_pBuffer[index+5] << 16) |
            ((sal_uInt64)m_pBuffer[index+6] << 8)  |
            ((sal_uInt64)m_pBuffer[index+7] << 0)
        );
    }
};

BlopObject::BlopObject(const sal_uInt8* buffer, sal_uInt32 len, bool copyBuffer)
    : m_bufferLen(len)
    , m_isCopied(copyBuffer)
{
    if (m_isCopied)
    {
        m_pBuffer = 0;
        sal_uInt8* newBuffer = new sal_uInt8[len];
        memcpy(newBuffer, buffer, len);
        m_pBuffer = newBuffer;
    }
    else
    {
        m_pBuffer = buffer;
    }
}

BlopObject::~BlopObject()
{
    if (m_isCopied)
    {
        delete[] const_cast<sal_uInt8*>(m_pBuffer);
    }
}

/**************************************************************************

    class StringCache

**************************************************************************/

class StringCache
{
public:
    sal_Unicode**   m_stringTable;
    sal_uInt16      m_numOfStrings;
    sal_uInt16      m_stringsCopied;

    StringCache(sal_uInt16 size); // throws std::bad_alloc
    ~StringCache();

    const sal_Unicode*  getString(sal_uInt16 index);
    sal_uInt16 createString(const sal_uInt8* buffer); // throws std::bad_alloc
};

StringCache::StringCache(sal_uInt16 size)
    : m_stringTable(NULL)
    , m_numOfStrings(size)
    , m_stringsCopied(0)
{
    m_stringTable = new sal_Unicode*[m_numOfStrings];

    for (sal_uInt16 i = 0; i < m_numOfStrings; i++)
    {
        m_stringTable[i] = NULL;
    }
}

StringCache::~StringCache()
{
    if (m_stringTable)
    {
        for (sal_uInt16 i = 0; i < m_stringsCopied; i++)
        {
            delete[] m_stringTable[i];
        }

        delete[] m_stringTable;
    }
}

const sal_Unicode* StringCache::getString(sal_uInt16 index)
{
    if ((index > 0) && (index <= m_stringsCopied))
        return m_stringTable[index - 1];
    else
        return NULL;
}

sal_uInt16 StringCache::createString(const sal_uInt8* buffer)
{
    if (m_stringsCopied < m_numOfStrings)
    {
        sal_uInt32 len = UINT16StringLen(buffer);

        m_stringTable[m_stringsCopied] = new sal_Unicode[len + 1];

        readString(buffer, m_stringTable[m_stringsCopied], (len + 1) * sizeof(sal_Unicode));

        return ++m_stringsCopied;
    }
    else
        return 0;
}

/**************************************************************************

    class ConstantPool

**************************************************************************/

class ConstantPool : public BlopObject
{
public:

    sal_uInt16  m_numOfEntries;
    sal_Int32*  m_pIndex;           // index values may be < 0 for cached string constants

    StringCache* m_pStringCache;

    ConstantPool(const sal_uInt8* buffer, sal_uInt16 numEntries)
        : BlopObject(buffer, 0, sal_False)
        , m_numOfEntries(numEntries)
        , m_pIndex(NULL)
        , m_pStringCache(NULL)
    {
    }

    ~ConstantPool();

    sal_uInt32 parseIndex(); // throws std::bad_alloc

    CPInfoTag       readTag(sal_uInt16 index);

    const sal_Char*     readUTF8NameConstant(sal_uInt16 index);
    sal_Bool            readBOOLConstant(sal_uInt16 index);
    sal_Int8            readBYTEConstant(sal_uInt16 index);
    sal_Int16           readINT16Constant(sal_uInt16 index);
    sal_uInt16          readUINT16Constant(sal_uInt16 index);
    sal_Int32           readINT32Constant(sal_uInt16 index);
    sal_uInt32          readUINT32Constant(sal_uInt16 index);
    sal_Int64           readINT64Constant(sal_uInt16 index);
    sal_uInt64          readUINT64Constant(sal_uInt16 index);
    float               readFloatConstant(sal_uInt16 index);
    double              readDoubleConstant(sal_uInt16 index);
    const sal_Unicode*  readStringConstant(sal_uInt16 index);
        // throws std::bad_alloc
    void                readUIK(sal_uInt16 index, RTUik* uik);
};

ConstantPool::~ConstantPool()
{
    delete[] m_pIndex;
    delete m_pStringCache;
}

sal_uInt32 ConstantPool::parseIndex()
{
    if (m_pIndex)
    {
        delete[] m_pIndex;
        m_pIndex = NULL;
    }

    if (m_pStringCache)
    {
        delete m_pStringCache;
        m_pStringCache = NULL;
    }

    sal_uInt32  offset = 0;
    sal_uInt16  numOfStrings = 0;

    if (m_numOfEntries)
    {
        m_pIndex = new sal_Int32[m_numOfEntries];

        for (int i = 0; i < m_numOfEntries; i++)
        {
            m_pIndex[i] = offset;

            offset += readUINT32(offset);

            if ( ((CPInfoTag) readUINT16(m_pIndex[i] + CP_OFFSET_ENTRY_TAG)) ==
                 CP_TAG_CONST_STRING )
            {
                numOfStrings++;
            }

        }
    }

    if (numOfStrings)
    {
        m_pStringCache = new StringCache(numOfStrings);
    }

    m_bufferLen = offset;

    return offset;
}

CPInfoTag ConstantPool::readTag(sal_uInt16 index)
{
    CPInfoTag tag = CP_TAG_INVALID;

    if (m_pIndex && (index > 0) && (index <= m_numOfEntries))
    {
        tag = (CPInfoTag) readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG);
    }

    return tag;
}

const sal_Char* ConstantPool::readUTF8NameConstant(sal_uInt16 index)
{
    const sal_Char* aName = NULL_STRING;

    if (m_pIndex && (index > 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_UTF8_NAME)
        {
            aName = (const sal_Char*) (m_pBuffer + m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aName;
}

sal_Bool ConstantPool::readBOOLConstant(sal_uInt16 index)
{
    sal_Bool aBool = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_BOOL)
        {
            aBool = (sal_Bool) readBYTE(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aBool;
}

sal_Int8 ConstantPool::readBYTEConstant(sal_uInt16 index)
{
    sal_Int8 aByte = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_BYTE)
        {
            aByte = static_cast< sal_Int8 >(
                readBYTE(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA));
        }
    }

    return aByte;
}

sal_Int16 ConstantPool::readINT16Constant(sal_uInt16 index)
{
    sal_Int16 aINT16 = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_INT16)
        {
            aINT16 = readINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aINT16;
}

sal_uInt16 ConstantPool::readUINT16Constant(sal_uInt16 index)
{
    sal_uInt16 asal_uInt16 = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_UINT16)
        {
            asal_uInt16 = readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return asal_uInt16;
}

sal_Int32 ConstantPool::readINT32Constant(sal_uInt16 index)
{
    sal_Int32 aINT32 = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_INT32)
        {
            aINT32 = readINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aINT32;
}

sal_uInt32 ConstantPool::readUINT32Constant(sal_uInt16 index)
{
    sal_uInt32 aUINT32 = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_UINT32)
        {
            aUINT32 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aUINT32;
}

sal_Int64 ConstantPool::readINT64Constant(sal_uInt16 index)
{
    sal_Int64 aINT64 = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_INT64)
        {
            aINT64 = readINT64(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aINT64;
}

sal_uInt64 ConstantPool::readUINT64Constant(sal_uInt16 index)
{
    sal_uInt64 aUINT64 = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_UINT64)
        {
            aUINT64 = readUINT64(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aUINT64;
}

float ConstantPool::readFloatConstant(sal_uInt16 index)
{
    union
    {
        float   v;
        sal_uInt32  b;
    } x = { 0.0f };

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_FLOAT)
        {
#ifdef REGTYPE_IEEE_NATIVE
            x.b = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
#else
#   error no IEEE
#endif
        }
    }

    return  x.v;
}

double ConstantPool::readDoubleConstant(sal_uInt16 index)
{
    union
    {
        double v;
        struct
        {
            sal_uInt32  b1;
            sal_uInt32  b2;
        } b;
    } x = { 0.0 };

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_DOUBLE)
        {

#ifdef REGTYPE_IEEE_NATIVE
#   ifdef OSL_BIGENDIAN
            x.b.b1 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
            x.b.b2 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA + sizeof(sal_uInt32));
#   else
            x.b.b1 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA + sizeof(sal_uInt32));
            x.b.b2 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
#   endif
#else
#   error no IEEE
#endif
        }
    }

    return x.v;
}

const sal_Unicode* ConstantPool::readStringConstant(sal_uInt16 index)
{
    const sal_Unicode* aString = NULL_WSTRING;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries) && m_pStringCache)
    {
        if (m_pIndex[index - 1] >= 0)
        {
            // create cached string now

            if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_STRING)
            {
                m_pIndex[index - 1] = -1 * m_pStringCache->createString(m_pBuffer + m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
            }
        }

        aString = m_pStringCache->getString((sal_uInt16) (m_pIndex[index - 1] * -1));
    }

    return aString;
}

void ConstantPool::readUIK(sal_uInt16 index, RTUik* uik)
{
    if (index == 0)
    {
        uik->m_Data1 = 0;
        uik->m_Data2 = 0;
        uik->m_Data3 = 0;
        uik->m_Data4 = 0;
        uik->m_Data5 = 0;
    }
    else if (m_pIndex && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_UIK)
        {
            uik->m_Data1 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_UIK1);
            uik->m_Data2 = readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_UIK2);
            uik->m_Data3 = readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_UIK3);
            uik->m_Data4 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_UIK4);
            uik->m_Data5 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_UIK5);
        }
    }
}

/**************************************************************************

    class FieldList

**************************************************************************/

class FieldList : public BlopObject
{
public:

    sal_uInt16      m_numOfEntries;
    sal_uInt16      m_numOfFieldEntries;
    sal_uInt16      m_FIELD_ENTRY_SIZE;
    ConstantPool*   m_pCP;

    FieldList(const sal_uInt8* buffer, sal_uInt16 numEntries, ConstantPool* pCP)
        : BlopObject(buffer, 0, sal_False)
        , m_numOfEntries(numEntries)
        , m_pCP(pCP)
    {
        if ( m_numOfEntries > 0 )
        {
            m_numOfFieldEntries = readUINT16(0);
            m_FIELD_ENTRY_SIZE = m_numOfFieldEntries * sizeof(sal_uInt16);
        } else
        {
            m_numOfFieldEntries = 0;
            m_FIELD_ENTRY_SIZE = 0;
        }
    }

    sal_uInt32 parseIndex();

    const sal_Char* getFieldName(sal_uInt16 index);
    const sal_Char* getFieldType(sal_uInt16 index);
    RTFieldAccess getFieldAccess(sal_uInt16 index);
    RTValueType     getFieldConstValue(sal_uInt16 index, RTConstValueUnion* value);
        // throws std::bad_alloc
    const sal_Char* getFieldDoku(sal_uInt16 index);
    const sal_Char* getFieldFileName(sal_uInt16 index);
};

sal_uInt32 FieldList::parseIndex()
{
    return ((m_numOfEntries ? sizeof(sal_uInt16) : 0) + (m_numOfEntries * m_FIELD_ENTRY_SIZE));
}

const sal_Char* FieldList::getFieldName(sal_uInt16 index)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_NAME));
    }

    return aName;
}

const sal_Char* FieldList::getFieldType(sal_uInt16 index)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_TYPE));
    }

    return aName;
}

RTFieldAccess FieldList::getFieldAccess(sal_uInt16 index)
{
    RTFieldAccess aAccess = RT_ACCESS_INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aAccess = (RTFieldAccess) readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_ACCESS);
    }

    return aAccess;
}

RTValueType FieldList::getFieldConstValue(sal_uInt16 index, RTConstValueUnion* value)
{
    RTValueType ret = RT_TYPE_NONE;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        sal_uInt16 cpIndex = readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_VALUE);

        switch (m_pCP->readTag(cpIndex))
        {
            case CP_TAG_CONST_BOOL:
                value->aBool = m_pCP->readBOOLConstant(cpIndex);
                ret = RT_TYPE_BOOL;
                break;
            case CP_TAG_CONST_BYTE:
                value->aByte = m_pCP->readBYTEConstant(cpIndex);
                ret = RT_TYPE_BYTE;
                break;
            case CP_TAG_CONST_INT16:
                value->aShort = m_pCP->readINT16Constant(cpIndex);
                ret = RT_TYPE_INT16;
                break;
            case CP_TAG_CONST_UINT16:
                value->aUShort = m_pCP->readUINT16Constant(cpIndex);
                ret = RT_TYPE_UINT16;
                break;
            case CP_TAG_CONST_INT32:
                value->aLong = m_pCP->readINT32Constant(cpIndex);
                ret = RT_TYPE_INT32;
                break;
            case CP_TAG_CONST_UINT32:
                value->aULong = m_pCP->readUINT32Constant(cpIndex);
                ret = RT_TYPE_UINT32;
                break;
            case CP_TAG_CONST_INT64:
              value->aHyper = m_pCP->readINT64Constant(cpIndex);
                ret = RT_TYPE_INT64;
                break;
            case CP_TAG_CONST_UINT64:
              value->aUHyper = m_pCP->readUINT64Constant(cpIndex);
                ret = RT_TYPE_UINT64;
                break;
            case CP_TAG_CONST_FLOAT:
                value->aFloat = m_pCP->readFloatConstant(cpIndex);
                ret = RT_TYPE_FLOAT;
                break;
            case CP_TAG_CONST_DOUBLE:
                value->aDouble = m_pCP->readDoubleConstant(cpIndex);
                ret = RT_TYPE_DOUBLE;
                break;
            case CP_TAG_CONST_STRING:
                value->aString = m_pCP->readStringConstant(cpIndex);
                ret = RT_TYPE_STRING;
                break;
            default:
                break;
        }
    }

    return ret;
}

const sal_Char* FieldList::getFieldDoku(sal_uInt16 index)
{
    const sal_Char* aDoku = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aDoku = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_DOKU));
    }

    return aDoku;
}

const sal_Char* FieldList::getFieldFileName(sal_uInt16 index)
{
    const sal_Char* aFileName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aFileName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_FILENAME));
    }

    return aFileName;
}

/**************************************************************************

    class ReferenceList

**************************************************************************/

class ReferenceList : public BlopObject
{
public:

    sal_uInt16      m_numOfEntries;
    sal_uInt16      m_numOfReferenceEntries;
    sal_uInt16      m_REFERENCE_ENTRY_SIZE;
    ConstantPool*   m_pCP;

    ReferenceList(const sal_uInt8* buffer, sal_uInt16 numEntries, ConstantPool* pCP)
        : BlopObject(buffer, 0, sal_False)
        , m_numOfEntries(numEntries)
        , m_pCP(pCP)
    {
        if ( m_numOfEntries > 0 )
        {
            m_numOfReferenceEntries = readUINT16(0);
            m_REFERENCE_ENTRY_SIZE = m_numOfReferenceEntries * sizeof(sal_uInt16);
        } else
        {
            m_numOfReferenceEntries = 0;
            m_REFERENCE_ENTRY_SIZE = 0;
        }
    }

    sal_uInt32 parseIndex();

    const sal_Char* getReferenceName(sal_uInt16 index);
    RTReferenceType getReferenceType(sal_uInt16 index);
    const sal_Char* getReferenceDoku(sal_uInt16 index);
    RTFieldAccess   getReferenceAccess(sal_uInt16 index);
};

sal_uInt32 ReferenceList::parseIndex()
{
    return ((m_numOfEntries ? sizeof(sal_uInt16) : 0) + (m_numOfEntries * m_REFERENCE_ENTRY_SIZE));
}

const sal_Char* ReferenceList::getReferenceName(sal_uInt16 index)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_NAME));
    }

    return aName;
}

RTReferenceType ReferenceList::getReferenceType(sal_uInt16 index)
{
    RTReferenceType refType = RT_REF_INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        refType = (RTReferenceType) readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_TYPE);
    }

    return refType;
}

const sal_Char* ReferenceList::getReferenceDoku(sal_uInt16 index)
{
    const sal_Char* aDoku = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aDoku = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_DOKU));
    }

    return aDoku;
}

RTFieldAccess ReferenceList::getReferenceAccess(sal_uInt16 index)
{
    RTFieldAccess aAccess = RT_ACCESS_INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aAccess = (RTFieldAccess) readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_ACCESS);
    }

    return aAccess;
}

/**************************************************************************

    class MethodList

**************************************************************************/

class MethodList : public BlopObject
{
public:

    sal_uInt16      m_numOfEntries;
    sal_uInt16      m_numOfMethodEntries;
    sal_uInt16      m_numOfParamEntries;
    sal_uInt16      m_PARAM_ENTRY_SIZE;
    sal_uInt32*     m_pIndex;
    ConstantPool*   m_pCP;

    MethodList(const sal_uInt8* buffer, sal_uInt16 numEntries, ConstantPool* pCP)
        : BlopObject(buffer, 0, sal_False)
        , m_numOfEntries(numEntries)
        , m_pIndex(NULL)
        , m_pCP(pCP)
    {
        if ( m_numOfEntries > 0 )
        {
            m_numOfMethodEntries = readUINT16(0);
            m_numOfParamEntries = readUINT16(sizeof(sal_uInt16));
            m_PARAM_ENTRY_SIZE = m_numOfParamEntries * sizeof(sal_uInt16);
        } else
        {
            m_numOfMethodEntries = 0;
            m_numOfParamEntries = 0;
            m_PARAM_ENTRY_SIZE = 0;
        }
    }

    ~MethodList();

    sal_uInt32 parseIndex(); // throws std::bad_alloc

    const sal_Char* getMethodName(sal_uInt16 index);
    sal_uInt16      getMethodParamCount(sal_uInt16 index);
    const sal_Char* getMethodParamType(sal_uInt16 index, sal_uInt16 paramIndex);
    const sal_Char* getMethodParamName(sal_uInt16 index, sal_uInt16 paramIndex);
    RTParamMode     getMethodParamMode(sal_uInt16 index, sal_uInt16 paramIndex);
    sal_uInt16      getMethodExcCount(sal_uInt16 index);
    const sal_Char* getMethodExcType(sal_uInt16 index, sal_uInt16 excIndex);
    const sal_Char* getMethodReturnType(sal_uInt16 index);
    RTMethodMode    getMethodMode(sal_uInt16 index);
    const sal_Char* getMethodDoku(sal_uInt16 index);

private:
    sal_uInt16 calcMethodParamIndex( const sal_uInt16 index );
};

MethodList::~MethodList()
{
    if (m_pIndex) delete[] m_pIndex;
}

sal_uInt16 MethodList::calcMethodParamIndex( const sal_uInt16 index )
{
    return (METHOD_OFFSET_PARAM_COUNT + sizeof(sal_uInt16) + (index * m_PARAM_ENTRY_SIZE));
}

sal_uInt32 MethodList::parseIndex()
{
    if (m_pIndex)
    {
        delete[] m_pIndex;
        m_pIndex = NULL;
    }

    sal_uInt32 offset = 0;

    if (m_numOfEntries)
    {
        offset = 2 * sizeof(sal_uInt16);
        m_pIndex = new sal_uInt32[m_numOfEntries];

        for (int i = 0; i < m_numOfEntries; i++)
        {
            m_pIndex[i] = offset;

            offset += readUINT16(offset);
        }
    }

    return offset;
}

const sal_Char* MethodList::getMethodName(sal_uInt16 index)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aName = m_pCP->readUTF8NameConstant(readUINT16(m_pIndex[index] + METHOD_OFFSET_NAME));
    }

    return aName;
}

sal_uInt16 MethodList::getMethodParamCount(sal_uInt16 index)
{
    sal_uInt16 aCount = 0;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aCount = readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT);
    }

    return aCount;
}

const sal_Char* MethodList::getMethodParamType(sal_uInt16 index, sal_uInt16 paramIndex)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) &&
        (index <= m_numOfEntries) &&
        (paramIndex <= readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT)))
    {
        aName = m_pCP->readUTF8NameConstant(
            readUINT16(
                m_pIndex[index] +
                calcMethodParamIndex(paramIndex) +
                PARAM_OFFSET_TYPE));
    }

    return aName;
}

const sal_Char* MethodList::getMethodParamName(sal_uInt16 index, sal_uInt16 paramIndex)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) &&
        (index <= m_numOfEntries) &&
        (paramIndex <= readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT)))
    {
        aName = m_pCP->readUTF8NameConstant(
            readUINT16(
                m_pIndex[index] +
                calcMethodParamIndex(paramIndex) +
                PARAM_OFFSET_NAME));
    }

    return aName;
}

RTParamMode MethodList::getMethodParamMode(sal_uInt16 index, sal_uInt16 paramIndex)
{
    RTParamMode aMode = RT_PARAM_INVALID;

    if ((m_numOfEntries > 0) &&
        (index <= m_numOfEntries) &&
        (paramIndex <= readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT)))
    {
        aMode = (RTParamMode) readUINT16(
                m_pIndex[index] +
                calcMethodParamIndex(paramIndex) +
                PARAM_OFFSET_MODE);
    }

    return aMode;
}

sal_uInt16 MethodList::getMethodExcCount(sal_uInt16 index)
{
    sal_uInt16 aCount = 0;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aCount = readUINT16(m_pIndex[index] + calcMethodParamIndex(readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT)));
    }

    return aCount;
}

const sal_Char* MethodList::getMethodExcType(sal_uInt16 index, sal_uInt16 excIndex)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        sal_uInt32 excOffset = m_pIndex[index] + calcMethodParamIndex(readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT));

        if (excIndex <= readUINT16(excOffset))
        {
            aName = m_pCP->readUTF8NameConstant(
                readUINT16(
                    excOffset +
                    sizeof(sal_uInt16) +
                    (excIndex * sizeof(sal_uInt16))));
        }
    }

    return aName;
}

const sal_Char* MethodList::getMethodReturnType(sal_uInt16 index)
{
    const sal_Char* aName = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aName = m_pCP->readUTF8NameConstant(readUINT16(m_pIndex[index] + METHOD_OFFSET_RETURN));
    }

    return aName;
}

RTMethodMode MethodList::getMethodMode(sal_uInt16 index)
{
    RTMethodMode aMode = RT_MODE_INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aMode = (RTMethodMode) readUINT16(m_pIndex[index] + METHOD_OFFSET_MODE);
    }

    return aMode;
}

const sal_Char* MethodList::getMethodDoku(sal_uInt16 index)
{
    const sal_Char* aDoku = NULL;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        aDoku = m_pCP->readUTF8NameConstant(readUINT16(m_pIndex[index] + METHOD_OFFSET_DOKU));
    }

    return aDoku;
}

/**************************************************************************

    class TypeRegistryEntry

**************************************************************************/

class TypeRegistryEntry: public BlopObject {
public:
    ConstantPool*   m_pCP;
    FieldList*      m_pFields;
    MethodList*     m_pMethods;
    ReferenceList*  m_pReferences;
    sal_uInt32      m_refCount;
    sal_uInt16      m_nSuperTypes;
    sal_uInt16      m_offset_SUPERTYPES;

    TypeRegistryEntry(
        const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer);
        // throws std::bad_alloc

    ~TypeRegistryEntry();

    typereg_Version getVersion() const;
};

TypeRegistryEntry::TypeRegistryEntry(
    const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer):
    BlopObject(buffer, len, copyBuffer), m_pCP(NULL), m_pFields(NULL),
    m_pMethods(NULL), m_pReferences(NULL), m_refCount(1), m_nSuperTypes(0),
    m_offset_SUPERTYPES(0)
{
    std::size_t const entrySize = sizeof(sal_uInt16);
    sal_uInt16 nHeaderEntries = readUINT16(OFFSET_N_ENTRIES);
    sal_uInt16 offset_N_SUPERTYPES = OFFSET_N_ENTRIES + entrySize + (nHeaderEntries * entrySize);
    m_offset_SUPERTYPES = offset_N_SUPERTYPES + entrySize;
    m_nSuperTypes = readUINT16(offset_N_SUPERTYPES);

    sal_uInt16 offset_CP_SIZE = m_offset_SUPERTYPES + (m_nSuperTypes * entrySize);
    sal_uInt16 offset_CP = offset_CP_SIZE + entrySize;

    m_pCP = new ConstantPool(m_pBuffer + offset_CP, readUINT16(offset_CP_SIZE));

    sal_uInt32 offset = offset_CP + m_pCP->parseIndex();

    m_pFields = new FieldList(
        m_pBuffer + offset + entrySize, readUINT16(offset), m_pCP);

    offset += sizeof(sal_uInt16) + m_pFields->parseIndex();

    m_pMethods = new MethodList(
        m_pBuffer + offset + entrySize, readUINT16(offset), m_pCP);

    offset += sizeof(sal_uInt16) + m_pMethods->parseIndex();

    m_pReferences = new ReferenceList(
        m_pBuffer + offset + entrySize, readUINT16(offset), m_pCP);

    m_pReferences->parseIndex();
}

TypeRegistryEntry::~TypeRegistryEntry()
{
    delete m_pCP;
    delete m_pFields;
    delete m_pMethods;
    delete m_pReferences;
}

typereg_Version TypeRegistryEntry::getVersion() const {
    // Assumes two's complement arithmetic with modulo-semantics:
    return static_cast< typereg_Version >(readUINT32(OFFSET_MAGIC) - magic);
}

/**************************************************************************

    C-API

**************************************************************************/

extern "C" {

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_reader_create(
    void const * buffer, sal_uInt32 length, sal_Bool copy,
    typereg_Version maxVersion, void ** result)
    SAL_THROW_EXTERN_C()
{
    if (length < OFFSET_CP || length > SAL_MAX_UINT32) {
        *result = 0;
        return true;
    }
    std::auto_ptr< TypeRegistryEntry > entry;
    try {
        entry.reset(
            new TypeRegistryEntry(
                static_cast< sal_uInt8 const * >(buffer),
                static_cast< sal_uInt32 >(length), copy));
    } catch (std::bad_alloc &) {
        return false;
    }
    if (entry->readUINT32(OFFSET_SIZE) != length) {
        *result = 0;
        return true;
    }
    typereg_Version version = entry->getVersion();
    if (version < TYPEREG_VERSION_0 || version > maxVersion) {
        *result = 0;
        return true;
    }
    *result = entry.release();
    return true;
}

static TypeReaderImpl TYPEREG_CALLTYPE createEntry(const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer)
{
    void * handle;
    typereg_reader_create(buffer, len, copyBuffer, TYPEREG_VERSION_0, &handle);
    return handle;
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_acquire(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry != NULL)
        pEntry->m_refCount++;
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_release(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry != NULL)
    {
        if (--pEntry->m_refCount == 0)
            delete pEntry;
    }
}

REG_DLLPUBLIC typereg_Version TYPEREG_CALLTYPE typereg_reader_getVersion(void * handle) SAL_THROW_EXTERN_C() {
    return handle == 0
        ? TYPEREG_VERSION_0
        : static_cast< TypeRegistryEntry * >(handle)->getVersion();
}

static sal_uInt16 TYPEREG_CALLTYPE getMinorVersion(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->readUINT16(OFFSET_MINOR_VERSION);
}

static sal_uInt16 TYPEREG_CALLTYPE getMajorVersion(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->readUINT16(OFFSET_MAJOR_VERSION);
}

REG_DLLPUBLIC RTTypeClass TYPEREG_CALLTYPE typereg_reader_getTypeClass(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_TYPE_INVALID;

    return (RTTypeClass)
        (pEntry->readUINT16(OFFSET_TYPE_CLASS) & ~RT_TYPE_PUBLISHED);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_reader_isPublished(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry * entry = static_cast< TypeRegistryEntry * >(hEntry);
    return entry != 0
        && (entry->readUINT16(OFFSET_TYPE_CLASS) & RT_TYPE_PUBLISHED) != 0;
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getTypeName(void * hEntry, rtl_uString** pTypeName)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pTypeName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_THIS_TYPE));
    rtl_string2UString(
        pTypeName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}


static void TYPEREG_CALLTYPE getSuperTypeName(TypeReaderImpl hEntry, rtl_uString** pSuperTypeName)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pSuperTypeName);
        return;
    }

    if (pEntry->m_nSuperTypes == 0)
    {
        rtl_uString_new(pSuperTypeName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(pEntry->m_offset_SUPERTYPES )); //+ (index * sizeof(sal_uInt16))));
    rtl_string2UString(
        pSuperTypeName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getUik(TypeReaderImpl hEntry, RTUik* uik)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry != NULL)
    {
        pEntry->m_pCP->readUIK(pEntry->readUINT16(OFFSET_UIK), uik);
    }
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getDocumentation(void * hEntry, rtl_uString** pDoku)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_DOKU));
    rtl_string2UString(
        pDoku, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getFileName(void * hEntry, rtl_uString** pFileName)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFileName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_FILENAME));
    rtl_string2UString(
        pFileName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}


REG_DLLPUBLIC sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getFieldCount(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->m_pFields->m_numOfEntries;
}

static sal_uInt32 TYPEREG_CALLTYPE getFieldCount(TypeReaderImpl hEntry)
{
    return typereg_reader_getFieldCount(hEntry);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getFieldName(void * hEntry, rtl_uString** pFieldName, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFieldName);
        return;
    }
    const sal_Char* pTmp = pEntry->m_pFields->getFieldName(index);
    rtl_string2UString(
        pFieldName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getFieldTypeName(void * hEntry, rtl_uString** pFieldType, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFieldType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pFields->getFieldType(index);
    rtl_string2UString(
        pFieldType, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC RTFieldAccess TYPEREG_CALLTYPE typereg_reader_getFieldFlags(void * hEntry, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_ACCESS_INVALID;

    return pEntry->m_pFields->getFieldAccess(index);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_reader_getFieldValue(
    void * hEntry, sal_uInt16 index, RTValueType * type,
    RTConstValueUnion * value)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) {
        *type = RT_TYPE_NONE;
        return true;
    }

    try {
        *type = pEntry->m_pFields->getFieldConstValue(index, value);
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

static RTValueType TYPEREG_CALLTYPE getFieldConstValue(TypeReaderImpl hEntry, sal_uInt16 index, RTConstValueUnion* value)
{
    RTValueType t = RT_TYPE_NONE;
    typereg_reader_getFieldValue(hEntry, index, &t, value);
    return t;
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getFieldDocumentation(void * hEntry, rtl_uString** pDoku, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pFields->getFieldDoku(index);
    rtl_string2UString(
        pDoku, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getFieldFileName(void * hEntry, rtl_uString** pFieldFileName, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFieldFileName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pFields->getFieldFileName(index);
    rtl_string2UString(
        pFieldFileName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}


REG_DLLPUBLIC sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getMethodCount(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->m_pMethods->m_numOfEntries;
}

static sal_uInt32 TYPEREG_CALLTYPE getMethodCount(TypeReaderImpl hEntry)
{
    return typereg_reader_getMethodCount(hEntry);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getMethodName(void * hEntry, rtl_uString** pMethodName, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodName(index);
    rtl_string2UString(
        pMethodName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getMethodParameterCount(
    void * hEntry, sal_uInt16 index) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->m_pMethods->getMethodParamCount(index);
}

static sal_uInt32 TYPEREG_CALLTYPE getMethodParamCount(TypeReaderImpl hEntry, sal_uInt16 index)
{
    return typereg_reader_getMethodParameterCount(hEntry, index);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getMethodParameterTypeName(void * hEntry, rtl_uString** pMethodParamType, sal_uInt16 index, sal_uInt16 paramIndex)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodParamType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodParamType(index, paramIndex);
    rtl_string2UString(
        pMethodParamType, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getMethodParameterName(void * hEntry, rtl_uString** pMethodParamName, sal_uInt16 index, sal_uInt16 paramIndex)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodParamName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodParamName(index, paramIndex);
    rtl_string2UString(
        pMethodParamName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC RTParamMode TYPEREG_CALLTYPE typereg_reader_getMethodParameterFlags(void * hEntry, sal_uInt16 index, sal_uInt16 paramIndex)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_PARAM_INVALID;

    return pEntry->m_pMethods->getMethodParamMode(index, paramIndex);
}

REG_DLLPUBLIC sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getMethodExceptionCount(
    void * hEntry, sal_uInt16 index) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->m_pMethods->getMethodExcCount(index);
}

static sal_uInt32 TYPEREG_CALLTYPE getMethodExcCount(TypeReaderImpl hEntry, sal_uInt16 index)
{
    return typereg_reader_getMethodExceptionCount(hEntry, index);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getMethodExceptionTypeName(void * hEntry, rtl_uString** pMethodExcpType, sal_uInt16 index, sal_uInt16 excIndex)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodExcpType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodExcType(index, excIndex);
    rtl_string2UString(
        pMethodExcpType, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getMethodReturnTypeName(void * hEntry, rtl_uString** pMethodReturnType, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodReturnType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodReturnType(index);
    rtl_string2UString(
        pMethodReturnType, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC RTMethodMode TYPEREG_CALLTYPE typereg_reader_getMethodFlags(void * hEntry, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_MODE_INVALID;

    return pEntry->m_pMethods->getMethodMode(index);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getMethodDocumentation(void * hEntry, rtl_uString** pMethodDoku, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodDoku(index);
    rtl_string2UString(
        pMethodDoku, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getReferenceCount(void * hEntry) SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->m_pReferences->m_numOfEntries;
}

static sal_uInt32 TYPEREG_CALLTYPE getReferenceCount(TypeReaderImpl hEntry)
{
    return typereg_reader_getReferenceCount(hEntry);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getReferenceTypeName(void * hEntry, rtl_uString** pReferenceName, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pReferenceName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pReferences->getReferenceName(index);
    rtl_string2UString(
        pReferenceName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC RTReferenceType TYPEREG_CALLTYPE typereg_reader_getReferenceSort(void * hEntry, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_REF_INVALID;

    return pEntry->m_pReferences->getReferenceType(index);
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getReferenceDocumentation(void * hEntry, rtl_uString** pReferenceDoku, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pReferenceDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pReferences->getReferenceDoku(index);
    rtl_string2UString(
        pReferenceDoku, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC RTFieldAccess TYPEREG_CALLTYPE typereg_reader_getReferenceFlags(void * hEntry, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_ACCESS_INVALID;

    return pEntry->m_pReferences->getReferenceAccess(index);
}

REG_DLLPUBLIC sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getSuperTypeCount(void * hEntry)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    return pEntry->m_nSuperTypes;
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_reader_getSuperTypeName(
    void * hEntry, rtl_uString ** pSuperTypeName, sal_uInt16 index)
    SAL_THROW_EXTERN_C()
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pSuperTypeName);
        return;
    }

    OSL_ASSERT(index < pEntry->m_nSuperTypes);
    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(pEntry->m_offset_SUPERTYPES + (index * sizeof(sal_uInt16))));
    rtl_string2UString(
        pSuperTypeName, pTmp, pTmp == 0 ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

REG_DLLPUBLIC RegistryTypeReader_Api* TYPEREG_CALLTYPE initRegistryTypeReader_Api(void)
{
    static RegistryTypeReader_Api aApi= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (!aApi.acquire)
    {
        aApi.createEntry            = &createEntry;
        aApi.acquire                = &typereg_reader_acquire;
        aApi.release                = &typereg_reader_release;
        aApi.getMinorVersion        = &getMinorVersion;
        aApi.getMajorVersion        = &getMajorVersion;
        aApi.getTypeClass           = &typereg_reader_getTypeClass;
        aApi.getTypeName            = &typereg_reader_getTypeName;
        aApi.getSuperTypeName       = &getSuperTypeName;
        aApi.getUik                 = &getUik;
        aApi.getDoku                = &typereg_reader_getDocumentation;
        aApi.getFileName            = &typereg_reader_getFileName;
        aApi.getFieldCount          = &getFieldCount;
        aApi.getFieldName           = &typereg_reader_getFieldName;
        aApi.getFieldType           = &typereg_reader_getFieldTypeName;
        aApi.getFieldAccess         = &typereg_reader_getFieldFlags;
        aApi.getFieldConstValue     = &getFieldConstValue;
        aApi.getFieldDoku           = &typereg_reader_getFieldDocumentation;
        aApi.getFieldFileName       = &typereg_reader_getFieldFileName;
        aApi.getMethodCount         = &getMethodCount;
        aApi.getMethodName          = &typereg_reader_getMethodName;
        aApi.getMethodParamCount    = &getMethodParamCount;
        aApi.getMethodParamType = &typereg_reader_getMethodParameterTypeName;
        aApi.getMethodParamName     = &typereg_reader_getMethodParameterName;
        aApi.getMethodParamMode     = &typereg_reader_getMethodParameterFlags;
        aApi.getMethodExcCount      = &getMethodExcCount;
        aApi.getMethodExcType = &typereg_reader_getMethodExceptionTypeName;
        aApi.getMethodReturnType    = &typereg_reader_getMethodReturnTypeName;
        aApi.getMethodMode          = &typereg_reader_getMethodFlags;
        aApi.getMethodDoku          = &typereg_reader_getMethodDocumentation;
        aApi.getReferenceCount      = &getReferenceCount;
        aApi.getReferenceName       = &typereg_reader_getReferenceTypeName;
        aApi.getReferenceType       = &typereg_reader_getReferenceSort;
        aApi.getReferenceDoku       = &typereg_reader_getReferenceDocumentation;
        aApi.getReferenceAccess     = &typereg_reader_getReferenceFlags;

        return (&aApi);
    }
    else
    {
        return (&aApi);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
