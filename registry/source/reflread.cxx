/*************************************************************************
 *
 *  $RCSfile: reflread.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jsc $ $Date: 2000-10-09 11:54:41 $
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

#ifndef MAC
#include <memory.h>
#endif
#include <osl/types.h>

#include <registry/reflread.hxx>
#include "reflcnst.hxx"

using namespace vos;

static CPInfoTag aTag;

static sal_Char NULL_STRING[1] = { 0 };
static sal_Unicode NULL_WSTRING[1] = { 0 };

const sal_uInt32    magic = 0x12345678;
const sal_uInt16 minorVersion = 0x0000;
const sal_uInt16 majorVersion = 0x0001;

#if defined ( GCC ) && ( defined ( SCO ) || defined ( NETBSD ) )
ORealDynamicLoader* ODynamicLoader<RegistryTypeReader_Api>::m_pLoader = NULL;
#endif

/**************************************************************************

    class BlopObject

    holds any data in a flat memory buffer

**************************************************************************/

class BlopObject
{
public:
    const sal_uInt8* m_pBuffer;
    sal_uInt32      m_bufferLen;
    sal_Bool        m_isCopied;

    BlopObject(const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer);
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
};

BlopObject::BlopObject(const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer)
    : m_isCopied(copyBuffer)
    , m_bufferLen(len)
{
    if (m_isCopied)
    {
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
    if (m_isCopied && m_pBuffer)
    {
#ifdef OS2
        delete (sal_uInt8 *)m_pBuffer;
#else
        delete[] const_cast<sal_uInt8*>(m_pBuffer);
#endif
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

    StringCache(sal_uInt16 size);
    ~StringCache();

    const sal_Unicode*  getString(sal_uInt16 index);
    sal_uInt16          createString(const sal_uInt8* buffer);
};

StringCache::StringCache(sal_uInt16 size)
    : m_numOfStrings(size)
    , m_stringsCopied(0)
    , m_stringTable(NULL)
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

    sal_uInt32 parseIndex();

    CPInfoTag       readTag(sal_uInt16 index);

    const sal_Char*     readUTF8NameConstant(sal_uInt16 index);
    sal_Bool            readBOOLConstant(sal_uInt16 index);
    sal_uInt8           readBYTEConstant(sal_uInt16 index);
    sal_Int16           readINT16Constant(sal_uInt16 index);
    sal_uInt16      readUINT16Constant(sal_uInt16 index);
    sal_Int32           readINT32Constant(sal_uInt16 index);
    sal_uInt32          readUINT32Constant(sal_uInt16 index);
    float           readFloatConstant(sal_uInt16 index);
    double          readDoubleConstant(sal_uInt16 index);
    const sal_Unicode*  readStringConstant(sal_uInt16 index);
    void            readUIK(sal_uInt16 index, RTUik* uik);
};

ConstantPool::~ConstantPool()
{
    if (m_pIndex) delete[] m_pIndex;

    if (m_pStringCache)
    {
        delete m_pStringCache;
    }
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

sal_uInt8 ConstantPool::readBYTEConstant(sal_uInt16 index)
{
    sal_uInt8 aByte = sal_False;

    if (m_pIndex && (index> 0) && (index <= m_numOfEntries))
    {
        if (readUINT16(m_pIndex[index - 1] + CP_OFFSET_ENTRY_TAG) == CP_TAG_CONST_BYTE)
        {
            aByte = readBYTE(m_pIndex[index - 1] + CP_OFFSET_ENTRY_DATA);
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

float ConstantPool::readFloatConstant(sal_uInt16 index)
{
    union
    {
        float   v;
        sal_uInt32  b;
    } x;

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
    } x;

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
    sal_uInt8       m_FIELD_ENTRY_SIZE;
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
    RTFieldAccess aAccess;

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
//              value->aHyper = m_pCP->readINT64Constant(cpIndex);
                ret = RT_TYPE_INT64;
                break;
            case CP_TAG_CONST_UINT64:
//              value->aUHyper = m_pCP->readUINT64Constant(cpIndex);
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
    sal_uInt8       m_REFERENCE_ENTRY_SIZE;
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
    RTReferenceType refType;

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
    RTFieldAccess aAccess;

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
    sal_uInt8       m_PARAM_ENTRY_SIZE;
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

    sal_uInt32 parseIndex();

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
    sal_uInt8 calcMethodParamIndex( const sal_uInt8 index );
};

MethodList::~MethodList()
{
    if (m_pIndex) delete[] m_pIndex;
}

sal_uInt8 MethodList::calcMethodParamIndex( const sal_uInt8 index )
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

class TypeRegistryEntry : public BlopObject
{

public:

    ConstantPool*   m_pCP;
    FieldList*      m_pFields;
    MethodList*     m_pMethods;
    ReferenceList*  m_pReferences;
    sal_uInt32      m_refCount;
    sal_uInt16      m_nSuperTypes;
    sal_uInt16      m_offset_SUPERTYPES;

    TypeRegistryEntry(const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer)
        : BlopObject(buffer, len, copyBuffer)
        , m_pCP(NULL)
        , m_pFields(NULL)
        , m_pMethods(NULL)
        , m_pReferences(NULL)
        , m_refCount(1)
        , m_nSuperTypes(0)
        , m_offset_SUPERTYPES(0)
    {
    }

    ~TypeRegistryEntry();

    void init();

};

TypeRegistryEntry::~TypeRegistryEntry()
{
    if (m_pCP) delete m_pCP;
    if (m_pFields) delete m_pFields;
    if (m_pMethods) delete m_pMethods;
    if (m_pReferences) delete m_pReferences;
}


void TypeRegistryEntry::init()
{
#ifdef OS2
    TypeRegistryEntry* _This = (TypeRegistryEntry*)(this);
#else
    TypeRegistryEntry* _This = const_cast<TypeRegistryEntry*>(this);
#endif

    if (m_pCP)
    {
        delete _This->m_pCP;
        _This->m_pCP = NULL;
    }

    sal_uInt16 entrySize = sizeof(sal_uInt16);
    sal_uInt16 nHeaderEntries = readUINT16(OFFSET_N_ENTRIES);
    sal_uInt16 offset_N_SUPERTYPES = OFFSET_N_ENTRIES + entrySize + (nHeaderEntries * entrySize);
    m_offset_SUPERTYPES = offset_N_SUPERTYPES + entrySize;
    m_nSuperTypes = readUINT16(offset_N_SUPERTYPES);

    sal_uInt16 offset_CP_SIZE = m_offset_SUPERTYPES + (m_nSuperTypes * entrySize);
    sal_uInt16 offset_CP = offset_CP_SIZE + entrySize;

    _This->m_pCP = new ConstantPool(m_pBuffer + offset_CP, readUINT16(offset_CP_SIZE));

    sal_uInt32 offset = offset_CP + _This->m_pCP->parseIndex();

    _This->m_pFields = new FieldList(m_pBuffer + offset + entrySize,
                                     readUINT16(offset), _This->m_pCP);

    offset += sizeof(sal_uInt16) + _This->m_pFields->parseIndex();

    _This->m_pMethods = new MethodList(m_pBuffer + offset + entrySize,
                                       readUINT16(offset), _This->m_pCP);

    offset += sizeof(sal_uInt16) + _This->m_pMethods->parseIndex();

    _This->m_pReferences = new ReferenceList(m_pBuffer + offset + entrySize,
                                             readUINT16(offset), _This->m_pCP);

    offset += sizeof(sal_uInt16) + _This->m_pReferences->parseIndex();
}


/**************************************************************************

    C-API

**************************************************************************/

static TypeReaderImpl TYPEREG_CALLTYPE createEntry(const sal_uInt8* buffer, sal_uInt32 len, sal_Bool copyBuffer)
{
    TypeRegistryEntry* ret = NULL;

    if (len >= OFFSET_CP)
    {
        ret = new TypeRegistryEntry(buffer, len, copyBuffer);

        if (
            (ret->readUINT32(OFFSET_MAGIC) != magic) ||
            (ret->readUINT32(OFFSET_SIZE) > len)
           )
        {
            delete ret;
            ret = NULL;
        }
    }

    return ret;
}

static void TYPEREG_CALLTYPE acquire(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry != NULL)
        pEntry->m_refCount++;
}

static void TYPEREG_CALLTYPE release(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry != NULL)
    {
        if (--pEntry->m_refCount == 0)
            delete pEntry;
    }
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

static RTTypeClass TYPEREG_CALLTYPE getTypeClass(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_TYPE_INVALID;

    return (RTTypeClass) pEntry->readUINT16(OFFSET_TYPE_CLASS);
}

static void TYPEREG_CALLTYPE getTypeName(TypeReaderImpl hEntry, rtl_uString** pTypeName)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pTypeName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_THIS_TYPE));
    if ( pTmp )
        rtl_string2UString( pTypeName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getSuperTypeName(TypeReaderImpl hEntry, rtl_uString** pSuperTypeName)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pSuperTypeName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    if (pEntry->m_nSuperTypes == 0)
    {
        rtl_uString_new(pSuperTypeName);
        return;
    }

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(pEntry->m_offset_SUPERTYPES )); //+ (index * sizeof(sal_uInt16))));
    if ( pTmp )
        rtl_string2UString( pSuperTypeName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getUik(TypeReaderImpl hEntry, RTUik* uik)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry != NULL)
    {
        if (pEntry->m_pCP == NULL) pEntry->init();

        pEntry->m_pCP->readUIK(pEntry->readUINT16(OFFSET_UIK), uik);
    }
}

static void TYPEREG_CALLTYPE getDoku(TypeReaderImpl hEntry, rtl_uString** pDoku)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pDoku);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_DOKU));
    if ( pTmp )
        rtl_string2UString( pDoku, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getFileName(TypeReaderImpl hEntry, rtl_uString** pFileName)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFileName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pCP->readUTF8NameConstant(pEntry->readUINT16(OFFSET_FILENAME));
    if ( pTmp )
        rtl_string2UString( pFileName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static sal_uInt32 TYPEREG_CALLTYPE getFieldCount(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pFields->m_numOfEntries;
}

static void TYPEREG_CALLTYPE getFieldName(TypeReaderImpl hEntry, rtl_uString** pFieldName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFieldName);
        return;
    }
    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pFields->getFieldName(index);
    if ( pTmp )
        rtl_string2UString( pFieldName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getFieldType(TypeReaderImpl hEntry, rtl_uString** pFieldType, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFieldType);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pFields->getFieldType(index);
    if ( pTmp )
        rtl_string2UString( pFieldType, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static RTFieldAccess TYPEREG_CALLTYPE getFieldAccess(TypeReaderImpl hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_ACCESS_INVALID;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pFields->getFieldAccess(index);
}

static RTValueType TYPEREG_CALLTYPE getFieldConstValue(TypeReaderImpl hEntry, sal_uInt16 index, RTConstValueUnion* value)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_TYPE_NONE;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pFields->getFieldConstValue(index, value);
}

static void TYPEREG_CALLTYPE getFieldDoku(TypeReaderImpl hEntry, rtl_uString** pDoku, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pDoku);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pFields->getFieldDoku(index);
    if ( pTmp )
        rtl_string2UString( pDoku, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getFieldFileName(TypeReaderImpl hEntry, rtl_uString** pFieldFileName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pFieldFileName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pFields->getFieldFileName(index);
    if ( pTmp )
        rtl_string2UString( pFieldFileName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static sal_uInt32 TYPEREG_CALLTYPE getMethodCount(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pMethods->m_numOfEntries;
}

static void TYPEREG_CALLTYPE getMethodName(TypeReaderImpl hEntry, rtl_uString** pMethodName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodName(index);
    if ( pTmp )
        rtl_string2UString( pMethodName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static sal_uInt32 TYPEREG_CALLTYPE getMethodParamCount(TypeReaderImpl hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pMethods->getMethodParamCount(index);
}

static void TYPEREG_CALLTYPE getMethodParamType(TypeReaderImpl hEntry, rtl_uString** pMethodParamType, sal_uInt16 index, sal_uInt16 paramIndex)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodParamType);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodParamType(index, paramIndex);
    if ( pTmp )
        rtl_string2UString( pMethodParamType, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getMethodParamName(TypeReaderImpl hEntry, rtl_uString** pMethodParamName, sal_uInt16 index, sal_uInt16 paramIndex)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodParamName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodParamName(index, paramIndex);
    if ( pTmp )
        rtl_string2UString( pMethodParamName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static RTParamMode TYPEREG_CALLTYPE getMethodParamMode(TypeReaderImpl hEntry, sal_uInt16 index, sal_uInt16 paramIndex)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_PARAM_INVALID;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pMethods->getMethodParamMode(index, paramIndex);
}

static sal_uInt32 TYPEREG_CALLTYPE getMethodExcCount(TypeReaderImpl hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pMethods->getMethodExcCount(index);
}

static void TYPEREG_CALLTYPE getMethodExcType(TypeReaderImpl hEntry, rtl_uString** pMethodExcpType, sal_uInt16 index, sal_uInt16 excIndex)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodExcpType);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodExcType(index, excIndex);
    if ( pTmp )
        rtl_string2UString( pMethodExcpType, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static void TYPEREG_CALLTYPE getMethodReturnType(TypeReaderImpl hEntry, rtl_uString** pMethodReturnType, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodReturnType);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodReturnType(index);
    if ( pTmp )
        rtl_string2UString( pMethodReturnType, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static RTMethodMode TYPEREG_CALLTYPE getMethodMode(TypeReaderImpl hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_MODE_INVALID;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pMethods->getMethodMode(index);
}

static void TYPEREG_CALLTYPE getMethodDoku(TypeReaderImpl hEntry, rtl_uString** pMethodDoku, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pMethodDoku);
        return;
    }
    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pMethods->getMethodDoku(index);
    if ( pTmp )
        rtl_string2UString( pMethodDoku, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static sal_uInt32 TYPEREG_CALLTYPE getReferenceCount(TypeReaderImpl hEntry)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return 0;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pReferences->m_numOfEntries;
}

static void TYPEREG_CALLTYPE getReferenceName(TypeReaderImpl hEntry, rtl_uString** pReferenceName, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pReferenceName);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pReferences->getReferenceName(index);
    if ( pTmp )
        rtl_string2UString( pReferenceName, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static RTReferenceType TYPEREG_CALLTYPE getReferenceType(TypeReaderImpl hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_REF_INVALID;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pReferences->getReferenceType(index);
}

static void TYPEREG_CALLTYPE getReferenceDoku(TypeReaderImpl hEntry, rtl_uString** pReferenceDoku, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL)
    {
        rtl_uString_new(pReferenceDoku);
        return;
    }

    if (pEntry->m_pCP == NULL) pEntry->init();

    const sal_Char* pTmp = pEntry->m_pReferences->getReferenceDoku(index);
    if ( pTmp )
        rtl_string2UString( pReferenceDoku, pTmp, rtl_str_getLength(pTmp), RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
}

static RTFieldAccess TYPEREG_CALLTYPE getReferenceAccess(TypeReaderImpl hEntry, sal_uInt16 index)
{
    TypeRegistryEntry* pEntry = (TypeRegistryEntry*) hEntry;

    if (pEntry == NULL) return RT_ACCESS_INVALID;

    if (pEntry->m_pCP == NULL) pEntry->init();

    return pEntry->m_pReferences->getReferenceAccess(index);
}


extern "C" RegistryTypeReader_Api* TYPEREG_CALLTYPE initRegistryTypeReader_Api(void)
{
    static RegistryTypeReader_Api aApi= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (!aApi.acquire)
    {
        aApi.createEntry            = &createEntry;
        aApi.acquire                = &acquire;
        aApi.release                = &release;
        aApi.getMinorVersion        = &getMinorVersion;
        aApi.getMajorVersion        = &getMajorVersion;
        aApi.getTypeClass           = &getTypeClass;
        aApi.getTypeName            = &getTypeName;
        aApi.getSuperTypeName       = &getSuperTypeName;
        aApi.getUik                 = &getUik;
        aApi.getDoku                = &getDoku;
        aApi.getFileName            = &getFileName;
        aApi.getFieldCount          = &getFieldCount;
        aApi.getFieldName           = &getFieldName;
        aApi.getFieldType           = &getFieldType;
        aApi.getFieldAccess         = &getFieldAccess;
        aApi.getFieldConstValue     = &getFieldConstValue;
        aApi.getFieldDoku           = &getFieldDoku;
        aApi.getFieldFileName       = &getFieldFileName;
        aApi.getMethodCount         = &getMethodCount;
        aApi.getMethodName          = &getMethodName;
        aApi.getMethodParamCount    = &getMethodParamCount;
        aApi.getMethodParamType     = &getMethodParamType;
        aApi.getMethodParamName     = &getMethodParamName;
        aApi.getMethodParamMode     = &getMethodParamMode;
        aApi.getMethodExcCount      = &getMethodExcCount;
        aApi.getMethodExcType       = &getMethodExcType;
        aApi.getMethodReturnType    = &getMethodReturnType;
        aApi.getMethodMode          = &getMethodMode;
        aApi.getMethodDoku          = &getMethodDoku;
        aApi.getReferenceCount      = &getReferenceCount;
        aApi.getReferenceName       = &getReferenceName;
        aApi.getReferenceType       = &getReferenceType;
        aApi.getReferenceDoku       = &getReferenceDoku;
        aApi.getReferenceAccess     = &getReferenceAccess;

        return (&aApi);
    }
    else
    {
        return (&aApi);
    }
}



