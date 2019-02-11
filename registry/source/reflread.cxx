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

#include <sal/config.h>

#include <cstring>
#include <memory>
#include <new>
#include <vector>

#include <sal/types.h>
#include <osl/endian.h>
#include <osl/diagnose.h>
#include "reflread.hxx"
#include <sal/log.hxx>

#include <registry/typereg_reader.hxx>
#include <registry/version.h>

#include "reflcnst.hxx"

#include <cstddef>

static const sal_Char NULL_STRING[1] = { 0 };
static const sal_Unicode NULL_WSTRING[1] = { 0 };

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
    struct BoundsError {};

    const sal_uInt8* m_pBuffer;
    sal_uInt32      m_bufferLen;

    BlopObject(const sal_uInt8* buffer, sal_uInt32 len);
        // throws std::bad_alloc

    sal_uInt8 readBYTE(sal_uInt32 index) const
    {
        if (index >= m_bufferLen) {
            throw BoundsError();
        }
        return m_pBuffer[index];
    }

    sal_Int16 readINT16(sal_uInt32 index) const
    {
        if (m_bufferLen < 2 || index >= m_bufferLen - 1) {
            throw BoundsError();
        }
        return ((m_pBuffer[index] << 8) | (m_pBuffer[index+1] << 0));
    }

    sal_uInt16 readUINT16(sal_uInt32 index) const
    {
        if (m_bufferLen < 2 || index >= m_bufferLen - 1) {
            throw BoundsError();
        }
        return ((m_pBuffer[index] << 8) | (m_pBuffer[index+1] << 0));
    }

    sal_Int32 readINT32(sal_uInt32 index) const
    {
        if (m_bufferLen < 4 || index >= m_bufferLen - 3) {
            throw BoundsError();
        }
        return (
            (m_pBuffer[index]   << 24) |
            (m_pBuffer[index+1] << 16) |
            (m_pBuffer[index+2] << 8)  |
            (m_pBuffer[index+3] << 0)
        );
    }

    sal_uInt32 readUINT32(sal_uInt32 index) const
    {
        if (m_bufferLen < 4 || index >= m_bufferLen - 3) {
            throw BoundsError();
        }
        return (
            (m_pBuffer[index]   << 24) |
            (m_pBuffer[index+1] << 16) |
            (m_pBuffer[index+2] << 8)  |
            (m_pBuffer[index+3] << 0)
        );
    }

    sal_Int64 readINT64(sal_uInt32 index) const
    {
        if (m_bufferLen < 8 || index >= m_bufferLen - 7) {
            throw BoundsError();
        }
        return (
            (static_cast<sal_Int64>(m_pBuffer[index])   << 56) |
            (static_cast<sal_Int64>(m_pBuffer[index+1]) << 48) |
            (static_cast<sal_Int64>(m_pBuffer[index+2]) << 40) |
            (static_cast<sal_Int64>(m_pBuffer[index+3]) << 32) |
            (static_cast<sal_Int64>(m_pBuffer[index+4]) << 24) |
            (static_cast<sal_Int64>(m_pBuffer[index+5]) << 16) |
            (static_cast<sal_Int64>(m_pBuffer[index+6]) << 8)  |
            (static_cast<sal_Int64>(m_pBuffer[index+7]) << 0)
        );
    }

    sal_uInt64 readUINT64(sal_uInt32 index) const
    {
        if (m_bufferLen < 8 || index >= m_bufferLen - 7) {
            throw BoundsError();
        }
        return (
            (static_cast<sal_uInt64>(m_pBuffer[index])   << 56) |
            (static_cast<sal_uInt64>(m_pBuffer[index+1]) << 48) |
            (static_cast<sal_uInt64>(m_pBuffer[index+2]) << 40) |
            (static_cast<sal_uInt64>(m_pBuffer[index+3]) << 32) |
            (static_cast<sal_uInt64>(m_pBuffer[index+4]) << 24) |
            (static_cast<sal_uInt64>(m_pBuffer[index+5]) << 16) |
            (static_cast<sal_uInt64>(m_pBuffer[index+6]) << 8)  |
            (static_cast<sal_uInt64>(m_pBuffer[index+7]) << 0)
        );
    }
};

BlopObject::BlopObject(const sal_uInt8* buffer, sal_uInt32 len)
    : m_bufferLen(len)
{
    m_pBuffer = buffer;
}

/**************************************************************************

    class StringCache

**************************************************************************/

class StringCache
{
public:
    std::vector<std::unique_ptr<sal_Unicode[]>> m_stringTable;
    sal_uInt16      m_stringsCopied;

    explicit StringCache(sal_uInt16 size); // throws std::bad_alloc

    const sal_Unicode*  getString(sal_uInt16 index) const;
    sal_uInt16 createString(const sal_uInt8* buffer); // throws std::bad_alloc
};

StringCache::StringCache(sal_uInt16 size)
    : m_stringTable(size)
    , m_stringsCopied(0)
{
}

const sal_Unicode* StringCache::getString(sal_uInt16 index) const
{
    if ((index > 0) && (index <= m_stringsCopied))
        return m_stringTable[index - 1].get();
    else
        return nullptr;
}

sal_uInt16 StringCache::createString(const sal_uInt8* buffer)
{
    if (m_stringsCopied < m_stringTable.size())
    {
        sal_uInt32 len = UINT16StringLen(buffer);

        m_stringTable[m_stringsCopied].reset( new sal_Unicode[len + 1] );

        readString(buffer, m_stringTable[m_stringsCopied].get(), (len + 1) * sizeof(sal_Unicode));

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

    sal_uInt16 const             m_numOfEntries;
    std::unique_ptr<sal_Int32[]> m_pIndex;           // index values may be < 0 for cached string constants

    std::unique_ptr<StringCache> m_pStringCache;

    ConstantPool(const sal_uInt8* buffer, sal_uInt32 len, sal_uInt16 numEntries)
        : BlopObject(buffer, len)
        , m_numOfEntries(numEntries)
    {
    }

    sal_uInt32 parseIndex(); // throws std::bad_alloc

    CPInfoTag           readTag(sal_uInt16 index) const;

    const sal_Char*     readUTF8NameConstant(sal_uInt16 index) const;
    bool                readBOOLConstant(sal_uInt16 index) const;
    sal_Int8            readBYTEConstant(sal_uInt16 index) const;
    sal_Int16           readINT16Constant(sal_uInt16 index) const;
    sal_uInt16          readUINT16Constant(sal_uInt16 index) const;
    sal_Int32           readINT32Constant(sal_uInt16 index) const;
    sal_uInt32          readUINT32Constant(sal_uInt16 index) const;
    sal_Int64           readINT64Constant(sal_uInt16 index) const;
    sal_uInt64          readUINT64Constant(sal_uInt16 index) const;
    float               readFloatConstant(sal_uInt16 index) const;
    double              readDoubleConstant(sal_uInt16 index) const;
    const sal_Unicode*  readStringConstant(sal_uInt16 index) const;
        // throws std::bad_alloc
};

sal_uInt32 ConstantPool::parseIndex()
{
    m_pIndex.reset();
    m_pStringCache.reset();

    sal_uInt32  offset = 0;
    sal_uInt16  numOfStrings = 0;

    if (m_numOfEntries)
    {
        m_pIndex.reset( new sal_Int32[m_numOfEntries] );

        for (int i = 0; i < m_numOfEntries; i++)
        {
            m_pIndex[i] = offset;

            offset += readUINT32(offset);

            if ( static_cast<CPInfoTag>(readUINT16(m_pIndex[i] + CP_OFFSET_ENTRY_TAG)) ==
                 CP_TAG_CONST_STRING )
            {
                numOfStrings++;
            }

        }
    }

    if (numOfStrings)
    {
        m_pStringCache.reset( new StringCache(numOfStrings) );
    }

    m_bufferLen = offset;

    return offset;
}

CPInfoTag ConstantPool::readTag(sal_uInt16 index) const
{
    CPInfoTag tag = CP_TAG_INVALID;

    if (m_pIndex && (index > 0) && (index <= m_numOfEntries))
    {
        tag = static_cast<CPInfoTag>(readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG));
    }

    return tag;
}

const sal_Char* ConstantPool::readUTF8NameConstant(sal_uInt16 index) const
{
    const sal_Char* aName = NULL_STRING;

    if (m_pIndex && (index > 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_UTF8_NAME)
        {
            sal_uInt32 n = m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA;
            if (n < m_bufferLen
                && std::memchr(m_pBuffer + n, 0, m_bufferLen - n) != nullptr)
            {
                aName = reinterpret_cast<const char*>(m_pBuffer + n);
            }
        }
    }

    return aName;
}

bool ConstantPool::readBOOLConstant(sal_uInt16 index) const
{
    bool aBool = false;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_BOOL)
        {
            aBool = readBYTE(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA) != 0;
        }
    }

    return aBool;
}

sal_Int8 ConstantPool::readBYTEConstant(sal_uInt16 index) const
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

sal_Int16 ConstantPool::readINT16Constant(sal_uInt16 index) const
{
    sal_Int16 aINT16 = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_INT16)
        {
            aINT16 = readINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aINT16;
}

sal_uInt16 ConstantPool::readUINT16Constant(sal_uInt16 index) const
{
    sal_uInt16 asal_uInt16 = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_UINT16)
        {
            asal_uInt16 = readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return asal_uInt16;
}

sal_Int32 ConstantPool::readINT32Constant(sal_uInt16 index) const
{
    sal_Int32 aINT32 = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_INT32)
        {
            aINT32 = readINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aINT32;
}

sal_uInt32 ConstantPool::readUINT32Constant(sal_uInt16 index) const
{
    sal_uInt32 aUINT32 = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_UINT32)
        {
            aUINT32 = readUINT32(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aUINT32;
}

sal_Int64 ConstantPool::readINT64Constant(sal_uInt16 index) const
{
    sal_Int64 aINT64 = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_INT64)
        {
            aINT64 = readINT64(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aINT64;
}

sal_uInt64 ConstantPool::readUINT64Constant(sal_uInt16 index) const
{
    sal_uInt64 aUINT64 = 0;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_UINT64)
        {
            aUINT64 = readUINT64(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
        }
    }

    return aUINT64;
}

float ConstantPool::readFloatConstant(sal_uInt16 index) const
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

double ConstantPool::readDoubleConstant(sal_uInt16 index) const
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

const sal_Unicode* ConstantPool::readStringConstant(sal_uInt16 index) const
{
    const sal_Unicode* aString = NULL_WSTRING;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries) && m_pStringCache)
    {
        if (m_pIndex[index - 1] >= 0)
        {
            // create cached string now

            if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_STRING)
            {
                sal_uInt32 n = m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA;
                if (n >= m_bufferLen
                    || (std::memchr(m_pBuffer + n, 0, m_bufferLen - n)
                        == nullptr))
                {
                    throw BoundsError();
                }
                m_pIndex[index - 1] = -1 * m_pStringCache->createString(m_pBuffer + n);
            }
        }

        aString = m_pStringCache->getString(static_cast<sal_uInt16>(m_pIndex[index - 1] * -1));
    }

    return aString;
}

/**************************************************************************

    class FieldList

**************************************************************************/

class FieldList : public BlopObject
{
public:

    sal_uInt16 const m_numOfEntries;
    size_t          m_FIELD_ENTRY_SIZE;
    ConstantPool* const m_pCP;

    FieldList(const sal_uInt8* buffer, sal_uInt32 len, sal_uInt16 numEntries, ConstantPool* pCP)
        : BlopObject(buffer, len)
        , m_numOfEntries(numEntries)
        , m_pCP(pCP)
    {
        if ( m_numOfEntries > 0 )
        {
            sal_uInt16 numOfFieldEntries = readUINT16(0);
            m_FIELD_ENTRY_SIZE = numOfFieldEntries * sizeof(sal_uInt16);
        } else
        {
            m_FIELD_ENTRY_SIZE = 0;
        }
    }

    sal_uInt32 parseIndex() const { return ((m_numOfEntries ? sizeof(sal_uInt16) : 0) + (m_numOfEntries * m_FIELD_ENTRY_SIZE));}

    const sal_Char* getFieldName(sal_uInt16 index) const;
    const sal_Char* getFieldType(sal_uInt16 index) const;
    RTFieldAccess getFieldAccess(sal_uInt16 index) const;
    RTValueType     getFieldConstValue(sal_uInt16 index, RTConstValueUnion* value) const;
        // throws std::bad_alloc
    const sal_Char* getFieldDoku(sal_uInt16 index) const;
    const sal_Char* getFieldFileName(sal_uInt16 index) const;
};


const sal_Char* FieldList::getFieldName(sal_uInt16 index) const
{
    const sal_Char* aName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_NAME));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aName;
}

const sal_Char* FieldList::getFieldType(sal_uInt16 index) const
{
    const sal_Char* aName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_TYPE));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aName;
}

RTFieldAccess FieldList::getFieldAccess(sal_uInt16 index) const
{
    RTFieldAccess aAccess = RTFieldAccess::INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aAccess = static_cast<RTFieldAccess>(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_ACCESS));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aAccess;
}

RTValueType FieldList::getFieldConstValue(sal_uInt16 index, RTConstValueUnion* value) const
{
    RTValueType ret = RT_TYPE_NONE;
    try {
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
    } catch (BlopObject::BoundsError &) {
        SAL_WARN("registry", "bad data");
    }
    return ret;
}

const sal_Char* FieldList::getFieldDoku(sal_uInt16 index) const
{
    const sal_Char* aDoku = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aDoku = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_DOKU));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aDoku;
}

const sal_Char* FieldList::getFieldFileName(sal_uInt16 index) const
{
    const sal_Char* aFileName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aFileName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_FIELD_ENTRY_SIZE) + FIELD_OFFSET_FILENAME));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aFileName;
}

/**************************************************************************

    class ReferenceList

**************************************************************************/

class ReferenceList : public BlopObject
{
public:

    sal_uInt16 const m_numOfEntries;
    size_t          m_REFERENCE_ENTRY_SIZE;
    ConstantPool* const m_pCP;

    ReferenceList(const sal_uInt8* buffer, sal_uInt32 len, sal_uInt16 numEntries, ConstantPool* pCP)
        : BlopObject(buffer, len)
        , m_numOfEntries(numEntries)
        , m_pCP(pCP)
    {
        if ( m_numOfEntries > 0 )
        {
            sal_uInt16 numOfReferenceEntries = readUINT16(0);
            m_REFERENCE_ENTRY_SIZE = numOfReferenceEntries * sizeof(sal_uInt16);
        } else
        {
            m_REFERENCE_ENTRY_SIZE = 0;
        }
    }

    const sal_Char* getReferenceName(sal_uInt16 index) const;
    RTReferenceType getReferenceType(sal_uInt16 index) const;
    const sal_Char* getReferenceDoku(sal_uInt16 index) const;
    RTFieldAccess   getReferenceAccess(sal_uInt16 index) const;
};


const sal_Char* ReferenceList::getReferenceName(sal_uInt16 index) const
{
    const sal_Char* aName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aName = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_NAME));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aName;
}

RTReferenceType ReferenceList::getReferenceType(sal_uInt16 index) const
{
    RTReferenceType refType = RTReferenceType::INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            refType = static_cast<RTReferenceType>(readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_TYPE));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return refType;
}

const sal_Char* ReferenceList::getReferenceDoku(sal_uInt16 index) const
{
    const sal_Char* aDoku = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aDoku = m_pCP->readUTF8NameConstant(readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_DOKU));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aDoku;
}

RTFieldAccess ReferenceList::getReferenceAccess(sal_uInt16 index) const
{
    RTFieldAccess aAccess = RTFieldAccess::INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aAccess = static_cast<RTFieldAccess>(readUINT16(sizeof(sal_uInt16) + (index * m_REFERENCE_ENTRY_SIZE) + REFERENCE_OFFSET_ACCESS));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aAccess;
}

/**************************************************************************

    class MethodList

**************************************************************************/

class MethodList : public BlopObject
{
public:

    sal_uInt16 const m_numOfEntries;
    size_t          m_PARAM_ENTRY_SIZE;
    std::unique_ptr<sal_uInt32[]>  m_pIndex;
    ConstantPool* const m_pCP;

    MethodList(const sal_uInt8* buffer, sal_uInt32 len, sal_uInt16 numEntries, ConstantPool* pCP)
        : BlopObject(buffer, len)
        , m_numOfEntries(numEntries)
        , m_pCP(pCP)
    {
        if ( m_numOfEntries > 0 )
        {
            readUINT16(0) /* numOfMethodEntries */;
            sal_uInt16 numOfParamEntries = readUINT16(sizeof(sal_uInt16));
            m_PARAM_ENTRY_SIZE = numOfParamEntries * sizeof(sal_uInt16);
        } else
        {
            m_PARAM_ENTRY_SIZE = 0;
        }
    }

    sal_uInt32 parseIndex(); // throws std::bad_alloc

    const sal_Char* getMethodName(sal_uInt16 index) const;
    sal_uInt16      getMethodParamCount(sal_uInt16 index) const;
    const sal_Char* getMethodParamType(sal_uInt16 index, sal_uInt16 paramIndex) const;
    const sal_Char* getMethodParamName(sal_uInt16 index, sal_uInt16 paramIndex) const;
    RTParamMode     getMethodParamMode(sal_uInt16 index, sal_uInt16 paramIndex) const;
    sal_uInt16      getMethodExcCount(sal_uInt16 index) const;
    const sal_Char* getMethodExcType(sal_uInt16 index, sal_uInt16 excIndex) const;
    const sal_Char* getMethodReturnType(sal_uInt16 index) const;
    RTMethodMode    getMethodMode(sal_uInt16 index) const;
    const sal_Char* getMethodDoku(sal_uInt16 index) const;

private:
    sal_uInt16 calcMethodParamIndex( const sal_uInt16 index ) const;
};

sal_uInt16 MethodList::calcMethodParamIndex( const sal_uInt16 index ) const
{
    return (METHOD_OFFSET_PARAM_COUNT + sizeof(sal_uInt16) + (index * m_PARAM_ENTRY_SIZE));
}

sal_uInt32 MethodList::parseIndex()
{
    m_pIndex.reset();

    sal_uInt32 offset = 0;

    if (m_numOfEntries)
    {
        offset = 2 * sizeof(sal_uInt16);
        m_pIndex.reset( new sal_uInt32[m_numOfEntries] );

        for (int i = 0; i < m_numOfEntries; i++)
        {
            m_pIndex[i] = offset;

            offset += readUINT16(offset);
        }
    }

    return offset;
}

const sal_Char* MethodList::getMethodName(sal_uInt16 index) const
{
    const sal_Char* aName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aName = m_pCP->readUTF8NameConstant(readUINT16(m_pIndex[index] + METHOD_OFFSET_NAME));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aName;
}

sal_uInt16 MethodList::getMethodParamCount(sal_uInt16 index) const
{
    sal_uInt16 aCount = 0;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aCount = readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT);
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aCount;
}

const sal_Char* MethodList::getMethodParamType(sal_uInt16 index, sal_uInt16 paramIndex) const
{
    const sal_Char* aName = nullptr;
    try {
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
    } catch (BlopObject::BoundsError &) {
        SAL_WARN("registry", "bad data");
    }
    return aName;
}

const sal_Char* MethodList::getMethodParamName(sal_uInt16 index, sal_uInt16 paramIndex) const
{
    const sal_Char* aName = nullptr;
    try {
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
    } catch (BlopObject::BoundsError &) {
        SAL_WARN("registry", "bad data");
    }
    return aName;
}

RTParamMode MethodList::getMethodParamMode(sal_uInt16 index, sal_uInt16 paramIndex) const
{
    RTParamMode aMode = RT_PARAM_INVALID;
    try {
        if ((m_numOfEntries > 0) &&
            (index <= m_numOfEntries) &&
            (paramIndex <= readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT)))
        {
            aMode = static_cast<RTParamMode>(readUINT16(
                m_pIndex[index] +
                calcMethodParamIndex(paramIndex) +
                PARAM_OFFSET_MODE));
        }
    } catch (BlopObject::BoundsError &) {
        SAL_WARN("registry", "bad data");
    }
    return aMode;
}

#if defined(__COVERITY__)
extern "C" void __coverity_tainted_data_sanitize__(void *);
#endif

sal_uInt16 MethodList::getMethodExcCount(sal_uInt16 index) const
{
    sal_uInt16 aCount = 0;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aCount = readUINT16(m_pIndex[index] + calcMethodParamIndex(readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT)));
#if defined(__COVERITY__)
            __coverity_tainted_data_sanitize__(&aCount);
#endif
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aCount;
}

const sal_Char* MethodList::getMethodExcType(sal_uInt16 index, sal_uInt16 excIndex) const
{
    const sal_Char* aName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            sal_uInt32 excOffset = m_pIndex[index] + calcMethodParamIndex(readUINT16(m_pIndex[index] + METHOD_OFFSET_PARAM_COUNT));
            if (excIndex <= readUINT16(excOffset))
            {
                aName = m_pCP->readUTF8NameConstant(
                    readUINT16(
                        excOffset +
                        sizeof(sal_uInt16) +
                        (excIndex * sizeof(sal_uInt16))));
            }
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aName;
}

const sal_Char* MethodList::getMethodReturnType(sal_uInt16 index) const
{
    const sal_Char* aName = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aName = m_pCP->readUTF8NameConstant(readUINT16(m_pIndex[index] + METHOD_OFFSET_RETURN));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aName;
}

RTMethodMode MethodList::getMethodMode(sal_uInt16 index) const
{
    RTMethodMode aMode = RTMethodMode::INVALID;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aMode = static_cast<RTMethodMode>(readUINT16(m_pIndex[index] + METHOD_OFFSET_MODE));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aMode;
}

const sal_Char* MethodList::getMethodDoku(sal_uInt16 index) const
{
    const sal_Char* aDoku = nullptr;

    if ((m_numOfEntries > 0) && (index <= m_numOfEntries))
    {
        try {
            aDoku = m_pCP->readUTF8NameConstant(readUINT16(m_pIndex[index] + METHOD_OFFSET_DOKU));
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }

    return aDoku;
}

/**************************************************************************

    class TypeRegistryEntry

**************************************************************************/

class TypeRegistryEntry: public BlopObject {
public:
    std::unique_ptr<ConstantPool> m_pCP;
    std::unique_ptr<FieldList> m_pFields;
    std::unique_ptr<MethodList> m_pMethods;
    std::unique_ptr<ReferenceList> m_pReferences;
    sal_uInt32      m_refCount;
    sal_uInt16      m_nSuperTypes;
    sal_uInt32      m_offset_SUPERTYPES;

    TypeRegistryEntry(
        const sal_uInt8* buffer, sal_uInt32 len);
        // throws std::bad_alloc

    typereg_Version getVersion() const;
};

TypeRegistryEntry::TypeRegistryEntry(
    const sal_uInt8* buffer, sal_uInt32 len):
    BlopObject(buffer, len), m_refCount(1), m_nSuperTypes(0),
    m_offset_SUPERTYPES(0)
{
    std::size_t const entrySize = sizeof(sal_uInt16);
    sal_uInt16 nHeaderEntries = readUINT16(OFFSET_N_ENTRIES);
    sal_uInt32 offset_N_SUPERTYPES = OFFSET_N_ENTRIES + entrySize + (nHeaderEntries * entrySize); // cannot overflow
    m_offset_SUPERTYPES = offset_N_SUPERTYPES + entrySize; // cannot overflow
    m_nSuperTypes = readUINT16(offset_N_SUPERTYPES);

    sal_uInt32 offset_CP_SIZE = m_offset_SUPERTYPES + (m_nSuperTypes * entrySize); // cannot overflow
    sal_uInt32 offset_CP = offset_CP_SIZE + entrySize; // cannot overflow

    if (offset_CP > m_bufferLen) {
        throw BoundsError();
    }
    m_pCP.reset(
        new ConstantPool(
            m_pBuffer + offset_CP, m_bufferLen - offset_CP,
            readUINT16(offset_CP_SIZE)));

    sal_uInt32 offset = offset_CP + m_pCP->parseIndex(); //TODO: overflow

    assert(m_bufferLen >= entrySize);
    if (offset > m_bufferLen - entrySize) {
        throw BoundsError();
    }
    m_pFields.reset(
        new FieldList(
            m_pBuffer + offset + entrySize, m_bufferLen - (offset + entrySize),
            readUINT16(offset), m_pCP.get()));

    offset += sizeof(sal_uInt16) + m_pFields->parseIndex(); //TODO: overflow

    assert(m_bufferLen >= entrySize);
    if (offset > m_bufferLen - entrySize) {
        throw BoundsError();
    }
    m_pMethods.reset(
        new MethodList(
            m_pBuffer + offset + entrySize, m_bufferLen - (offset + entrySize),
            readUINT16(offset), m_pCP.get()));

    offset += sizeof(sal_uInt16) + m_pMethods->parseIndex(); //TODO: overflow

    assert(m_bufferLen >= entrySize);
    if (offset > m_bufferLen - entrySize) {
        throw BoundsError();
    }
    m_pReferences.reset(
        new ReferenceList(
            m_pBuffer + offset + entrySize, m_bufferLen - (offset + entrySize),
            readUINT16(offset), m_pCP.get()));
}

typereg_Version TypeRegistryEntry::getVersion() const {
    // Assumes two's complement arithmetic with modulo-semantics:
    return static_cast< typereg_Version >(readUINT32(OFFSET_MAGIC) - magic);
}

/**************************************************************************

    C-API

**************************************************************************/

bool TYPEREG_CALLTYPE typereg_reader_create(
    void const * buffer, sal_uInt32 length,
    void ** result)
{
    if (length < OFFSET_CP || length > SAL_MAX_UINT32) {
        *result = nullptr;
        return true;
    }
    std::unique_ptr< TypeRegistryEntry > entry;
    try {
        try {
            entry.reset(
                new TypeRegistryEntry(
                    static_cast< sal_uInt8 const * >(buffer), length));
        } catch (std::bad_alloc &) {
            return false;
        }
        if (entry->readUINT32(OFFSET_SIZE) != length) {
            *result = nullptr;
            return true;
        }
        typereg_Version version = entry->getVersion();
        if (version < TYPEREG_VERSION_0 || version > TYPEREG_VERSION_1) {
            *result = nullptr;
            return true;
        }
        *result = entry.release();
        return true;
    } catch (BlopObject::BoundsError &) {
        SAL_WARN("registry", "bad data");
        return false;
    }
}

static TypeReaderImpl TYPEREG_CALLTYPE createEntry(const sal_uInt8* buffer, sal_uInt32 len)
{
    void * handle;
    typereg_reader_create(buffer, len, &handle);
    return handle;
}

void TYPEREG_CALLTYPE typereg_reader_acquire(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry != nullptr)
        pEntry->m_refCount++;
}

void TYPEREG_CALLTYPE typereg_reader_release(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry != nullptr)
    {
        if (--pEntry->m_refCount == 0)
            delete pEntry;
    }
}

typereg_Version TYPEREG_CALLTYPE typereg_reader_getVersion(void const * handle) {
    if (handle != nullptr) {
        try {
            return static_cast< TypeRegistryEntry const * >(handle)->getVersion();
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    return TYPEREG_VERSION_0;
}

RTTypeClass TYPEREG_CALLTYPE typereg_reader_getTypeClass(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);
    if (pEntry != nullptr) {
        try {
            return static_cast<RTTypeClass>(pEntry->readUINT16(OFFSET_TYPE_CLASS) & ~RT_TYPE_PUBLISHED);
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    return RT_TYPE_INVALID;
}

bool TYPEREG_CALLTYPE typereg_reader_isPublished(void * hEntry)
{
    TypeRegistryEntry * entry = static_cast< TypeRegistryEntry * >(hEntry);
    if (entry != nullptr) {
        try {
            return (entry->readUINT16(OFFSET_TYPE_CLASS) & RT_TYPE_PUBLISHED) != 0;
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    return false;
}

void TYPEREG_CALLTYPE typereg_reader_getTypeName(void * hEntry, rtl_uString** pTypeName)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);
    if (pEntry != nullptr) {
        try {
            const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_THIS_TYPE));
            rtl_string2UString(
                pTypeName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
                RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
            return;
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    rtl_uString_new(pTypeName);
}


static void TYPEREG_CALLTYPE getSuperTypeName(TypeReaderImpl hEntry, rtl_uString** pSuperTypeName)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);
    if (pEntry != nullptr && pEntry->m_nSuperTypes != 0) {
        try {
            const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(pEntry->m_offset_SUPERTYPES )); //+ (index * sizeof(sal_uInt16))));
            rtl_string2UString(
                pSuperTypeName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
                RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
            return;
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    rtl_uString_new(pSuperTypeName);
}

void TYPEREG_CALLTYPE typereg_reader_getDocumentation(void * hEntry, rtl_uString** pDoku)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);
    if (pEntry != nullptr) {
        try {
            const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_DOKU));
            rtl_string2UString(
                pDoku, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
                RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
            return;
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    rtl_uString_new(pDoku);
}

void TYPEREG_CALLTYPE typereg_reader_getFileName(void * hEntry, rtl_uString** pFileName)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);
    if (pEntry != nullptr) {
        try {
            const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_FILENAME));
            rtl_string2UString(
                pFileName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
                RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
            return;
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    rtl_uString_new(pFileName);
}


sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getFieldCount(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return 0;

    return pEntry->m_pFields->m_numOfEntries;
}

static sal_uInt32 TYPEREG_CALLTYPE getFieldCount(TypeReaderImpl hEntry)
{
    return typereg_reader_getFieldCount(hEntry);
}

void TYPEREG_CALLTYPE typereg_reader_getFieldName(void * hEntry, rtl_uString** pFieldName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pFieldName);
        return;
    }
    const sal_Char* pTmp = pEntry->m_pFields->getFieldName(index);
    rtl_string2UString(
        pFieldName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

void TYPEREG_CALLTYPE typereg_reader_getFieldTypeName(void * hEntry, rtl_uString** pFieldType, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pFieldType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pFields->getFieldType(index);
    rtl_string2UString(
        pFieldType, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

RTFieldAccess TYPEREG_CALLTYPE typereg_reader_getFieldFlags(void * hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return RTFieldAccess::INVALID;

    return pEntry->m_pFields->getFieldAccess(index);
}

bool TYPEREG_CALLTYPE typereg_reader_getFieldValue(
    void * hEntry, sal_uInt16 index, RTValueType * type,
    RTConstValueUnion * value)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) {
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

void TYPEREG_CALLTYPE typereg_reader_getFieldDocumentation(void * hEntry, rtl_uString** pDoku, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pFields->getFieldDoku(index);
    rtl_string2UString(
        pDoku, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

void TYPEREG_CALLTYPE typereg_reader_getFieldFileName(void * hEntry, rtl_uString** pFieldFileName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pFieldFileName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pFields->getFieldFileName(index);
    rtl_string2UString(
        pFieldFileName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}


sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getMethodCount(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return 0;

    return pEntry->m_pMethods->m_numOfEntries;
}

void TYPEREG_CALLTYPE typereg_reader_getMethodName(void * hEntry, rtl_uString** pMethodName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pMethodName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodName(index);
    rtl_string2UString(
        pMethodName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getMethodParameterCount(
    void * hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return 0;

    return pEntry->m_pMethods->getMethodParamCount(index);
}

void TYPEREG_CALLTYPE typereg_reader_getMethodParameterTypeName(void * hEntry, rtl_uString** pMethodParamType, sal_uInt16 index, sal_uInt16 paramIndex)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pMethodParamType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodParamType(index, paramIndex);
    rtl_string2UString(
        pMethodParamType, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

void TYPEREG_CALLTYPE typereg_reader_getMethodParameterName(void * hEntry, rtl_uString** pMethodParamName, sal_uInt16 index, sal_uInt16 paramIndex)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pMethodParamName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodParamName(index, paramIndex);
    rtl_string2UString(
        pMethodParamName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

RTParamMode TYPEREG_CALLTYPE typereg_reader_getMethodParameterFlags(void * hEntry, sal_uInt16 index, sal_uInt16 paramIndex)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return RT_PARAM_INVALID;

    return pEntry->m_pMethods->getMethodParamMode(index, paramIndex);
}

sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getMethodExceptionCount(
    void * hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return 0;

    return pEntry->m_pMethods->getMethodExcCount(index);
}

void TYPEREG_CALLTYPE typereg_reader_getMethodExceptionTypeName(void * hEntry, rtl_uString** pMethodExcpType, sal_uInt16 index, sal_uInt16 excIndex)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pMethodExcpType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodExcType(index, excIndex);
    rtl_string2UString(
        pMethodExcpType, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

void TYPEREG_CALLTYPE typereg_reader_getMethodReturnTypeName(void * hEntry, rtl_uString** pMethodReturnType, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pMethodReturnType);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodReturnType(index);
    rtl_string2UString(
        pMethodReturnType, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

RTMethodMode TYPEREG_CALLTYPE typereg_reader_getMethodFlags(void * hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return RTMethodMode::INVALID;

    return pEntry->m_pMethods->getMethodMode(index);
}

void TYPEREG_CALLTYPE typereg_reader_getMethodDocumentation(void * hEntry, rtl_uString** pMethodDoku, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pMethodDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodDoku(index);
    rtl_string2UString(
        pMethodDoku, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getReferenceCount(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return 0;

    return pEntry->m_pReferences->m_numOfEntries;
}

void TYPEREG_CALLTYPE typereg_reader_getReferenceTypeName(void * hEntry, rtl_uString** pReferenceName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pReferenceName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pReferences->getReferenceName(index);
    rtl_string2UString(
        pReferenceName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

RTReferenceType TYPEREG_CALLTYPE typereg_reader_getReferenceSort(void * hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return RTReferenceType::INVALID;

    return pEntry->m_pReferences->getReferenceType(index);
}

void TYPEREG_CALLTYPE typereg_reader_getReferenceDocumentation(void * hEntry, rtl_uString** pReferenceDoku, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr)
    {
        rtl_uString_new(pReferenceDoku);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pReferences->getReferenceDoku(index);
    rtl_string2UString(
        pReferenceDoku, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
        RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
}

RTFieldAccess TYPEREG_CALLTYPE typereg_reader_getReferenceFlags(void * hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return RTFieldAccess::INVALID;

    return pEntry->m_pReferences->getReferenceAccess(index);
}

sal_uInt16 TYPEREG_CALLTYPE typereg_reader_getSuperTypeCount(void * hEntry)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);

    if (pEntry == nullptr) return 0;

    return pEntry->m_nSuperTypes;
}

void TYPEREG_CALLTYPE typereg_reader_getSuperTypeName(
    void * hEntry, rtl_uString ** pSuperTypeName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = static_cast<TypeRegistryEntry*>(hEntry);
    if (pEntry != nullptr) {
        try {
            OSL_ASSERT(index < pEntry->m_nSuperTypes);
            const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(pEntry->m_offset_SUPERTYPES + (index * sizeof(sal_uInt16))));
            rtl_string2UString(
                pSuperTypeName, pTmp, pTmp == nullptr ? 0 : rtl_str_getLength(pTmp),
                RTL_TEXTENCODING_UTF8, OSTRING_TO_OUSTRING_CVTFLAGS);
            return;
        } catch (BlopObject::BoundsError &) {
            SAL_WARN("registry", "bad data");
        }
    }
    rtl_uString_new(pSuperTypeName);
}

RegistryTypeReader::RegistryTypeReader(const sal_uInt8* buffer,
                                              sal_uInt32 bufferLen)
    : m_hImpl(nullptr)
{
    m_hImpl = createEntry(buffer, bufferLen);
}

RegistryTypeReader::~RegistryTypeReader()
{ typereg_reader_release(m_hImpl); }

RTTypeClass RegistryTypeReader::getTypeClass() const
{  return typereg_reader_getTypeClass(m_hImpl); }

OUString RegistryTypeReader::getTypeName() const
{
    OUString sRet;
    typereg_reader_getTypeName(m_hImpl, &sRet.pData);
    return sRet;
}

OUString RegistryTypeReader::getSuperTypeName() const
{
    OUString sRet;
    ::getSuperTypeName(m_hImpl, &sRet.pData);
    return sRet;
}

sal_uInt32 RegistryTypeReader::getFieldCount() const
{   return ::getFieldCount(m_hImpl); }

OUString RegistryTypeReader::getFieldName( sal_uInt16 index ) const
{
    OUString sRet;
    typereg_reader_getFieldName(m_hImpl, &sRet.pData, index);
    return sRet;
}

OUString RegistryTypeReader::getFieldType( sal_uInt16 index ) const
{
    OUString sRet;
    typereg_reader_getFieldTypeName(m_hImpl, &sRet.pData, index);
    return sRet;
}

RTFieldAccess RegistryTypeReader::getFieldAccess( sal_uInt16 index ) const
{  return typereg_reader_getFieldFlags(m_hImpl, index); }

RTConstValue RegistryTypeReader::getFieldConstValue( sal_uInt16 index ) const
{
    RTConstValue ret;
    ret.m_type = ::getFieldConstValue(m_hImpl, index, &ret.m_value);
    return ret;
}

OUString RegistryTypeReader::getFieldDoku( sal_uInt16 index ) const
{
    OUString sRet;
    typereg_reader_getFieldDocumentation(m_hImpl, &sRet.pData, index);
    return sRet;
}

OUString RegistryTypeReader::getFieldFileName( sal_uInt16 index ) const
{
    OUString sRet;
    typereg_reader_getFieldFileName(m_hImpl, &sRet.pData, index);
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
