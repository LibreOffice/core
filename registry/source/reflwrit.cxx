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


#include <new>
#include <memory>
#include <algorithm>
#include <sal/types.h>
#include <osl/endian.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include "reflwrit.hxx"
#include <registry/refltype.hxx>
#include <registry/version.h>
#include <registry/writer.h>

#include "reflcnst.hxx"


namespace {

OString toByteString(rtl_uString const * str) {
    return OString(
        str->buffer, str->length, RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS);
}

}

static const sal_Unicode NULL_WSTRING[1] = { 0 };

#define BLOP_OFFSET_MAGIC       0
#define BLOP_OFFSET_SIZE        (BLOP_OFFSET_MAGIC + sizeof(sal_uInt32))
#define BLOP_OFFSET_MINOR       (BLOP_OFFSET_SIZE + sizeof(sal_uInt32))
#define BLOP_OFFSET_MAJOR       (BLOP_OFFSET_MINOR + sizeof(sal_uInt16))
#define BLOP_OFFSET_N_ENTRIES   (BLOP_OFFSET_MAJOR + sizeof(sal_uInt16))
#define BLOP_HEADER_N_ENTRIES   6

#define BLOP_FIELD_N_ENTRIES        6

#define BLOP_METHOD_N_ENTRIES   5

#define BLOP_PARAM_N_ENTRIES    3

#define BLOP_REFERENCE_N_ENTRIES    4

sal_uInt32 UINT16StringLen(const sal_uInt8* wstring)
{
    if (!wstring) return 0;

    const sal_uInt8* b = wstring;

    while (b[0] || b[1]) b += sizeof(sal_uInt16);

    return ((b - wstring) / sizeof(sal_uInt16));
}

sal_uInt32 writeString(sal_uInt8* buffer, const sal_Unicode* v)
{
    sal_uInt32 len = rtl_ustr_getLength(v) + 1;
    sal_uInt32 i;
    sal_uInt8* buff = buffer;

    for (i = 0; i < len; i++)
    {
        buff += writeUINT16(buff, static_cast<sal_uInt16>(v[i]));
    }

    return (buff - buffer);
}

sal_uInt32 readString(const sal_uInt8* buffer, sal_Unicode* v, sal_uInt32 maxSize)
{
    sal_uInt32 len = UINT16StringLen(buffer) + 1;
    sal_uInt32 i;
    sal_uInt8* buff = const_cast<sal_uInt8*>(buffer);

    if(len > maxSize / 2)
    {
        len = maxSize / 2;
    }

    for (i = 0; i < (len - 1); i++)
    {
        sal_uInt16 aChar;

        buff += readUINT16(buff, aChar);

        v[i] = static_cast<sal_Unicode>(aChar);
    }

    v[len - 1] = L'\0';

    return (buff - buffer);
}

sal_uInt32 writeFloat(sal_uInt8* buffer, float v)
{
    union
    {
        float   v;
        sal_uInt32  b;
    } x;

    x.v = v;

#ifdef REGTYPE_IEEE_NATIVE
    writeUINT32(buffer, x.b);
#else
#   error no IEEE
#endif

    return sizeof(sal_uInt32);
}

sal_uInt32 writeDouble(sal_uInt8* buffer, double v)
{
    union
    {
        double v;
        struct
        {
            sal_uInt32 b1;
            sal_uInt32 b2;
        } b;
    } x;

    x.v = v;

#ifdef REGTYPE_IEEE_NATIVE
#   ifdef OSL_BIGENDIAN
    writeUINT32(buffer, x.b.b1);
    writeUINT32(buffer + sizeof(sal_uInt32), x.b.b2);
#   else
    writeUINT32(buffer, x.b.b2);
    writeUINT32(buffer + sizeof(sal_uInt32), x.b.b1);
#   endif
#else
#   error no IEEE
#endif

    return (sizeof(sal_uInt32) + sizeof(sal_uInt32));
}

/**************************************************************************

    buffer write functions

**************************************************************************/


/**************************************************************************

    struct CPInfo

**************************************************************************/

struct CPInfo
{
    CPInfoTag const m_tag;
    union
    {
        const sal_Char*     aUtf8;
        RTUik*              aUik;
        RTConstValueUnion   aConst;
    } m_value;

    sal_uInt16      m_index;
    struct CPInfo*  m_next;

    CPInfo(CPInfoTag tag, struct CPInfo* prev);

    sal_uInt32 getBlopSize() const;

    sal_uInt32 toBlop(sal_uInt8* buffer);
};

CPInfo::CPInfo(CPInfoTag tag, struct CPInfo* prev)
    : m_tag(tag)
    , m_index(0)
    , m_next(nullptr)
{
    if (prev)
    {
        m_index = prev->m_index + 1;
        prev->m_next = this;
    }
}

sal_uInt32 CPInfo::getBlopSize() const
{
    sal_uInt32 size = sizeof(sal_uInt32) /* size */ + sizeof(sal_uInt16) /* tag */;

    switch (m_tag)
    {
        case CP_TAG_CONST_BOOL:
            size += sizeof(sal_uInt8);
            break;
        case CP_TAG_CONST_BYTE:
            size += sizeof(sal_uInt8);
            break;
        case CP_TAG_CONST_INT16:
            size += sizeof(sal_Int16);
            break;
        case CP_TAG_CONST_UINT16:
            size += sizeof(sal_uInt16);
            break;
        case CP_TAG_CONST_INT32:
            size += sizeof(sal_Int32);
            break;
        case CP_TAG_CONST_UINT32:
            size += sizeof(sal_uInt32);
            break;
        case CP_TAG_CONST_INT64:
              size += sizeof(sal_Int64);
            break;
        case CP_TAG_CONST_UINT64:
            size += sizeof(sal_uInt64);
            break;
        case CP_TAG_CONST_FLOAT:
            size += sizeof(sal_uInt32);
            break;
        case CP_TAG_CONST_DOUBLE:
            size += sizeof(sal_uInt32) + sizeof(sal_uInt32);
            break;
        case CP_TAG_CONST_STRING:
            size += (rtl_ustr_getLength(m_value.aConst.aString) + 1) * sizeof(sal_uInt16);
            break;
        case CP_TAG_UTF8_NAME:
            size += strlen(m_value.aUtf8) + 1;
            break;
        case CP_TAG_UIK:
            size += sizeof(sal_uInt32) + sizeof(sal_uInt16) + sizeof(sal_uInt16) + sizeof(sal_uInt32) + sizeof(sal_uInt32);
            break;
        default:
            break;
    }

    return size;
}


sal_uInt32 CPInfo::toBlop(sal_uInt8* buffer)
{
    sal_uInt8* buff = buffer;

    buff += writeUINT32(buff, getBlopSize());
    buff += writeUINT16(buff, static_cast<sal_uInt16>(m_tag));

    switch (m_tag)
    {
        case CP_TAG_CONST_BOOL:
            buff += writeBYTE(buff, static_cast<sal_uInt8>(m_value.aConst.aBool));
            break;
        case CP_TAG_CONST_BYTE:
            buff += writeBYTE(
                buff, static_cast< sal_uInt8 >(m_value.aConst.aByte));
            break;
        case CP_TAG_CONST_INT16:
            buff += writeINT16(buff, m_value.aConst.aShort);
            break;
        case CP_TAG_CONST_UINT16:
            buff += writeINT16(buff, m_value.aConst.aUShort);
            break;
        case CP_TAG_CONST_INT32:
            buff += writeINT32(buff, m_value.aConst.aLong);
            break;
        case CP_TAG_CONST_UINT32:
            buff += writeUINT32(buff, m_value.aConst.aULong);
            break;
        case CP_TAG_CONST_INT64:
            buff += writeUINT64(buff, m_value.aConst.aHyper);
            break;
        case CP_TAG_CONST_UINT64:
            buff += writeUINT64(buff, m_value.aConst.aUHyper);
            break;
        case CP_TAG_CONST_FLOAT:
            buff += writeFloat(buff, m_value.aConst.aFloat);
            break;
        case CP_TAG_CONST_DOUBLE:
            buff += writeDouble(buff, m_value.aConst.aDouble);
            break;
        case CP_TAG_CONST_STRING:
            buff += writeString(buff, m_value.aConst.aString);
            break;
        case CP_TAG_UTF8_NAME:
            buff += writeUtf8(buff, m_value.aUtf8);
            break;
        case CP_TAG_UIK:
            buff += writeUINT32(buff, m_value.aUik->m_Data1);
            buff += writeUINT16(buff, m_value.aUik->m_Data2);
            buff += writeUINT16(buff, m_value.aUik->m_Data3);
            buff += writeUINT32(buff, m_value.aUik->m_Data4);
            buff += writeUINT32(buff, m_value.aUik->m_Data5);
            break;
        default:
            break;
    }

    return (buff - buffer);
}


/**************************************************************************

    class FieldEntry

**************************************************************************/

class FieldEntry
{

public:

    OString           m_name;
    OString           m_typeName;
    OString           m_doku;
    OString           m_fileName;
    RTFieldAccess     m_access;
    RTValueType       m_constValueType;
    RTConstValueUnion m_constValue;

    FieldEntry();
    ~FieldEntry();

    void setData(const OString&    name,
                 const OString&    typeName,
                 const OString&    doku,
                 const OString&    fileName,
                 RTFieldAccess     access,
                 RTValueType       constValueType,
                 RTConstValueUnion constValue);
        // throws std::bad_alloc
};

FieldEntry::FieldEntry()
    : m_access(RTFieldAccess::INVALID)
    , m_constValueType(RT_TYPE_NONE)
{
}

FieldEntry::~FieldEntry()
{
    if (
        (m_constValueType == RT_TYPE_STRING) &&
        m_constValue.aString &&
        (m_constValue.aString != NULL_WSTRING)
       )
    {
        delete[] m_constValue.aString;
    }
}

void FieldEntry::setData(const OString&    name,
                         const OString&    typeName,
                         const OString&    doku,
                         const OString&    fileName,
                         RTFieldAccess      access,
                         RTValueType        constValueType,
                         RTConstValueUnion  constValue)
{
    std::unique_ptr<sal_Unicode[]> newValue;
    if (constValueType == RT_TYPE_STRING && constValue.aString != nullptr) {
        sal_Int32 n = rtl_ustr_getLength(constValue.aString) + 1;
        newValue.reset(new sal_Unicode[n]);
        memcpy(newValue.get(), constValue.aString, n * sizeof (sal_Unicode));
    }

    m_name = name;
    m_typeName = typeName;
    m_doku = doku;
    m_fileName = fileName;

    if (
        (m_constValueType == RT_TYPE_STRING) &&
        m_constValue.aString &&
        (m_constValue.aString != NULL_WSTRING)
       )
    {
        delete[] m_constValue.aString;
    }

    m_access = access;
    m_constValueType = constValueType;

    if (m_constValueType == RT_TYPE_STRING)
    {
        if (constValue.aString == nullptr)
            m_constValue.aString = NULL_WSTRING;
        else
        {
            m_constValue.aString = newValue.release();
        }
    }
    else
    {
        m_constValue = constValue;
    }
}

/**************************************************************************

    class ParamEntry

**************************************************************************/

class ParamEntry
{
public:

    OString     m_typeName;
    OString     m_name;
    RTParamMode m_mode;

    ParamEntry();

    void setData(const OString& typeName,
                 const OString& name,
                 RTParamMode    mode);
};

ParamEntry::ParamEntry()
    : m_mode(RT_PARAM_INVALID)
{
}

void ParamEntry::setData(const OString& typeName,
                         const OString& name,
                         RTParamMode    mode)
{
    m_name = name;
    m_typeName = typeName;
    m_mode = mode;
}

/**************************************************************************

    class ReferenceEntry

**************************************************************************/

class ReferenceEntry
{
public:

    OString         m_name;
    OString         m_doku;
    RTReferenceType m_type;
    RTFieldAccess   m_access;

    ReferenceEntry();

    void setData(const OString&     name,
                 RTReferenceType    refType,
                 const OString&     doku,
                 RTFieldAccess      access);
};

ReferenceEntry::ReferenceEntry()
    : m_type(RTReferenceType::INVALID)
    , m_access(RTFieldAccess::INVALID)
{
}

void ReferenceEntry::setData(const OString&    name,
                             RTReferenceType   refType,
                             const OString&    doku,
                             RTFieldAccess     access)
{
    m_name = name;
    m_doku = doku;
    m_type = refType;
    m_access = access;
}

/**************************************************************************

    class MethodEntry

**************************************************************************/

class MethodEntry
{
public:

    OString         m_name;
    OString         m_returnTypeName;
    RTMethodMode    m_mode;
    sal_uInt16      m_paramCount;
    std::unique_ptr<ParamEntry[]> m_params;
    sal_uInt16      m_excCount;
    std::unique_ptr<OString[]>    m_excNames;
    OString         m_doku;

    MethodEntry();

    void setData(const OString&    name,
                 const OString&    returnTypeName,
                 RTMethodMode      mode,
                 sal_uInt16        paramCount,
                 sal_uInt16        excCount,
                 const OString&    doku);

    void setExcName(sal_uInt16 excIndex, const OString& name) const;

protected:

    void reallocParams(sal_uInt16 size);
    void reallocExcs(sal_uInt16 size);
};

MethodEntry::MethodEntry()
    : m_mode(RTMethodMode::INVALID)
    , m_paramCount(0)
    , m_excCount(0)
{
}

void MethodEntry::setData(const OString&    name,
                          const OString&    returnTypeName,
                          RTMethodMode      mode,
                          sal_uInt16        paramCount,
                          sal_uInt16        excCount,
                          const OString&    doku)
{
    m_name = name;
    m_returnTypeName = returnTypeName;
    m_doku = doku;

    m_mode = mode;

    reallocParams(paramCount);
    reallocExcs(excCount);
}

void MethodEntry::setExcName(sal_uInt16 excIndex, const OString& name) const
{
    if (excIndex < m_excCount)
    {
        m_excNames[excIndex] = name;
    }
}

void MethodEntry::reallocParams(sal_uInt16 size)
{
    ParamEntry* newParams;

    if (size)
        newParams = new ParamEntry[size];
    else
        newParams = nullptr;

    if (m_paramCount)
    {
        sal_uInt16 i;
        sal_uInt16 mn = std::min(size, m_paramCount);

        for (i = 0; i < mn; i++)
        {
            newParams[i].setData(m_params[i].m_typeName, m_params[i].m_name, m_params[i].m_mode);
        }

        m_params.reset();
    }

    m_paramCount = size;
    m_params.reset( newParams );
}

void MethodEntry::reallocExcs(sal_uInt16 size)
{
    OString* newExcNames;

    if (size)
        newExcNames = new OString[size];
    else
        newExcNames = nullptr;

    sal_uInt16 i;
    sal_uInt16 mn = std::min(size, m_excCount);

    for (i = 0; i < mn; i++)
    {
        newExcNames[i] = m_excNames[i];
    }

    m_excCount = size;
    m_excNames.reset( newExcNames );
}


/**************************************************************************

    class TypeRegistryEntry

**************************************************************************/

class TypeWriter
{

public:

    sal_uInt32          m_refCount;
    typereg_Version const m_version;
    RTTypeClass const   m_typeClass;
    OString const       m_typeName;
    sal_uInt16 const    m_nSuperTypes;
    std::unique_ptr<OString[]>
                        m_superTypeNames;
    OString const       m_doku;
    OString const       m_fileName;
    sal_uInt16 const    m_fieldCount;
    FieldEntry*         m_fields;
    sal_uInt16 const    m_methodCount;
    MethodEntry*        m_methods;
    sal_uInt16 const    m_referenceCount;
    ReferenceEntry*     m_references;

    std::unique_ptr<sal_uInt8[]> m_blop;
    sal_uInt32          m_blopSize;

    TypeWriter(typereg_Version version,
               OString const & documentation,
               OString const & fileName,
               RTTypeClass      RTTypeClass,
               bool             published,
               const OString&   typeName,
               sal_uInt16       superTypeCount,
               sal_uInt16       FieldCount,
               sal_uInt16       methodCount,
               sal_uInt16       referenceCount);

    ~TypeWriter();

    void setSuperType(sal_uInt16 index, OString const & name) const;

    void createBlop(); // throws std::bad_alloc
};

TypeWriter::TypeWriter(typereg_Version version,
                       OString const & documentation,
                       OString const & fileName,
                       RTTypeClass      RTTypeClass,
                       bool             published,
                       const OString&   typeName,
                       sal_uInt16       superTypeCount,
                       sal_uInt16       fieldCount,
                       sal_uInt16       methodCount,
                       sal_uInt16       referenceCount)
    : m_refCount(1)
    , m_version(version)
    , m_typeClass(
        static_cast< enum RTTypeClass >(
            RTTypeClass | (published ? RT_TYPE_PUBLISHED : 0)))
     , m_typeName(typeName)
    , m_nSuperTypes(superTypeCount)
    , m_doku(documentation)
    , m_fileName(fileName)
    , m_fieldCount(fieldCount)
    , m_fields(nullptr)
    , m_methodCount(methodCount)
    , m_methods(nullptr)
    , m_referenceCount(referenceCount)
    , m_references(nullptr)
    , m_blopSize(0)
{
    if (m_nSuperTypes > 0)
    {
        m_superTypeNames.reset( new OString[m_nSuperTypes] );
    }

    if (m_fieldCount)
        m_fields = new FieldEntry[fieldCount];

    if (m_methodCount)
        m_methods = new MethodEntry[methodCount];

    if (m_referenceCount)
        m_references = new ReferenceEntry[referenceCount];
}

TypeWriter::~TypeWriter()
{
    if (m_fieldCount)
        delete[] m_fields;

    if (m_methodCount)
        delete[] m_methods;

    if (m_referenceCount)
        delete[] m_references;
}

void TypeWriter::setSuperType(sal_uInt16 index, OString const & name) const
{
    m_superTypeNames[index] = name;
}

void TypeWriter::createBlop()
{
    //TODO: Fix memory leaks that occur when std::bad_alloc is thrown

    std::unique_ptr<sal_uInt8[]>  pBlopFields;
    std::unique_ptr<sal_uInt8[]>  pBlopMethods;
    std::unique_ptr<sal_uInt8[]>  pBlopReferences;
    sal_uInt8*  pBuffer             = nullptr;
    sal_uInt32  blopFieldsSize      = 0;
    sal_uInt32  blopMethodsSize     = 0;
    sal_uInt32  blopReferenceSize   = 0;

    CPInfo  root(CP_TAG_INVALID, nullptr);
    sal_uInt16  cpIndexThisName = 0;
    std::unique_ptr<sal_uInt16[]> cpIndexSuperNames;
    sal_uInt16  cpIndexDoku = 0;
    sal_uInt16  cpIndexFileName = 0;
    CPInfo* pInfo = nullptr;

    sal_uInt16  entrySize = sizeof(sal_uInt16);
    sal_uInt32  blopHeaderEntrySize = BLOP_OFFSET_N_ENTRIES + entrySize + (BLOP_HEADER_N_ENTRIES * entrySize);
    sal_uInt32  blopFieldEntrySize = BLOP_FIELD_N_ENTRIES * entrySize;
    sal_uInt32  blopMethodEntrySize = BLOP_METHOD_N_ENTRIES * entrySize;
    sal_uInt32  blopParamEntrySize = BLOP_PARAM_N_ENTRIES * entrySize;
    sal_uInt32  blopReferenceEntrySize = BLOP_REFERENCE_N_ENTRIES * entrySize;

    sal_uInt32 blopSize = blopHeaderEntrySize;

    // create CP entry for this name
    pInfo = new CPInfo(CP_TAG_UTF8_NAME, &root);
    pInfo->m_value.aUtf8 = m_typeName.getStr();
    cpIndexThisName = pInfo->m_index;

    // nSuperTypes
    blopSize += entrySize;

    // create CP entry for super names
    if (m_nSuperTypes)
    {
        blopSize += m_nSuperTypes * entrySize;

        cpIndexSuperNames.reset(new sal_uInt16[m_nSuperTypes]);

        for (sal_uInt32 i=0; i < m_nSuperTypes; i++)
        {
            pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
            pInfo->m_value.aUtf8 = m_superTypeNames[i].getStr();
            cpIndexSuperNames[i] = pInfo->m_index;
        }
    }

    // create CP entry for doku
    if (!m_doku.isEmpty())
    {
        pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
        pInfo->m_value.aUtf8 = m_doku.getStr();
        cpIndexDoku = pInfo->m_index;
    }

    // create CP entry for idl source filename
    if (!m_fileName.isEmpty())
    {
        pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
        pInfo->m_value.aUtf8 = m_fileName.getStr();
        cpIndexFileName = pInfo->m_index;
    }

    // fields blop
    blopSize += sizeof(sal_uInt16); // fieldCount + nFieldEntries

    if (m_fieldCount)
    {
        sal_uInt16 cpIndexName = 0;
        sal_uInt16 cpIndexTypeName = 0;
        sal_uInt16 cpIndexValue = 0;
        sal_uInt16 cpIndexDoku2 = 0;
        sal_uInt16 cpIndexFileName2 = 0;

        // nFieldEntries + n fields
        blopFieldsSize = sizeof(sal_uInt16) + (m_fieldCount * blopFieldEntrySize);

        blopSize += blopFieldsSize;

        pBlopFields.reset(new sal_uInt8[blopFieldsSize]);
        pBuffer = pBlopFields.get();

        pBuffer += writeUINT16(pBuffer, BLOP_FIELD_N_ENTRIES);

        for (sal_uInt16 i = 0; i < m_fieldCount; i++)
        {
            cpIndexName = 0;
            cpIndexTypeName = 0;
            cpIndexValue = 0;
            cpIndexDoku2 = 0;
            cpIndexFileName2 = 0;

            pBuffer += writeUINT16(pBuffer, static_cast<sal_uInt16>(m_fields[i].m_access));

            if (!m_fields[i].m_name.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_name.getStr();
                cpIndexName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexName);

            if (!m_fields[i].m_typeName.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_typeName.getStr();
                cpIndexTypeName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexTypeName);

            if (m_fields[i].m_constValueType != RT_TYPE_NONE)
            {
                pInfo = new CPInfo(static_cast<CPInfoTag>(m_fields[i].m_constValueType), pInfo);
                pInfo->m_value.aConst = m_fields[i].m_constValue;
                cpIndexValue = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexValue);

            if (!m_fields[i].m_doku.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_doku.getStr();
                cpIndexDoku2 = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexDoku2);

            if (!m_fields[i].m_fileName.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_fileName.getStr();
                cpIndexFileName2 = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexFileName2);
        }
    }

    // methods blop
    blopSize += sizeof(sal_uInt16); // methodCount

    if (m_methodCount)
    {
        std::unique_ptr<sal_uInt16[]> pMethodEntrySize( new sal_uInt16[m_methodCount] );
        sal_uInt16  cpIndexName = 0;
        sal_uInt16  cpIndexReturn = 0;
        sal_uInt16  cpIndexDoku2 = 0;

        // nMethodEntries + nParamEntries
        blopMethodsSize = (2 * sizeof(sal_uInt16));

        for (sal_uInt16 i = 0; i < m_methodCount; i++)
        {
            pMethodEntrySize[i] = static_cast<sal_uInt16>( blopMethodEntrySize +                                 // header
                  sizeof(sal_uInt16) +                                  // parameterCount
                  (m_methods[i].m_paramCount * blopParamEntrySize) +    // exceptions
                  sizeof(sal_uInt16) +                                  // exceptionCount
                  (m_methods[i].m_excCount * sizeof(sal_uInt16)) );     // exceptions

            blopMethodsSize += pMethodEntrySize[i];
        }

        pBlopMethods.reset(new sal_uInt8[blopMethodsSize]);

        blopSize += blopMethodsSize;

        pBuffer = pBlopMethods.get();

        pBuffer += writeUINT16(pBuffer, BLOP_METHOD_N_ENTRIES);
        pBuffer += writeUINT16(pBuffer, BLOP_PARAM_N_ENTRIES );

        for (sal_uInt16 i = 0; i < m_methodCount; i++)
        {
            cpIndexReturn = 0;
            cpIndexDoku2 = 0;

            pBuffer += writeUINT16(pBuffer, pMethodEntrySize[i]);
            pBuffer += writeUINT16(
                pBuffer,
                sal::static_int_cast< sal_uInt16 >(m_methods[i].m_mode));

            if (!m_methods[i].m_name.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_methods[i].m_name.getStr();
                cpIndexName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexName);
            cpIndexName = 0;

            if (!m_methods[i].m_returnTypeName.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_methods[i].m_returnTypeName.getStr();
                cpIndexReturn = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexReturn);

            if (!m_methods[i].m_doku.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_methods[i].m_doku.getStr();
                cpIndexDoku2 = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexDoku2);

            sal_uInt16 j;

            pBuffer += writeUINT16(pBuffer, m_methods[i].m_paramCount);

            for (j = 0; j < m_methods[i].m_paramCount; j++)
            {
                if (!m_methods[i].m_params[j].m_typeName.isEmpty())
                {
                    pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                    pInfo->m_value.aUtf8 = m_methods[i].m_params[j].m_typeName.getStr();
                    cpIndexName = pInfo->m_index;
                }
                pBuffer += writeUINT16(pBuffer, cpIndexName);
                cpIndexName = 0;

                pBuffer += writeUINT16(
                    pBuffer,
                    sal::static_int_cast< sal_uInt16 >(
                        m_methods[i].m_params[j].m_mode));

                if (!m_methods[i].m_params[j].m_name.isEmpty())
                {
                    pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                    pInfo->m_value.aUtf8 = m_methods[i].m_params[j].m_name.getStr();
                    cpIndexName = pInfo->m_index;
                }
                pBuffer += writeUINT16(pBuffer, cpIndexName);
                cpIndexName = 0;
            }

            pBuffer += writeUINT16(pBuffer, m_methods[i].m_excCount);

            for (j = 0; j < m_methods[i].m_excCount; j++)
            {
                if (!m_methods[i].m_excNames[j].isEmpty())
                {
                    pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                    pInfo->m_value.aUtf8 = m_methods[i].m_excNames[j].getStr();
                    cpIndexName = pInfo->m_index;
                }
                pBuffer += writeUINT16(pBuffer, cpIndexName);
                cpIndexName = 0;
            }
        }
    }

    // reference blop
    blopSize += entrySize; // referenceCount

    if (m_referenceCount)
    {
        sal_uInt16 cpIndexName = 0;
        sal_uInt16 cpIndexDoku2 = 0;

        // nReferenceEntries + n references
        blopReferenceSize = entrySize + (m_referenceCount * blopReferenceEntrySize);

        blopSize += blopReferenceSize;

        pBlopReferences.reset(new sal_uInt8[blopReferenceSize]);
        pBuffer = pBlopReferences.get();

        pBuffer += writeUINT16(pBuffer, BLOP_REFERENCE_N_ENTRIES);

        for (sal_uInt16 i = 0; i < m_referenceCount; i++)
        {
            pBuffer += writeUINT16(
                pBuffer,
                sal::static_int_cast< sal_uInt16 >(m_references[i].m_type));

            cpIndexName = 0;
            cpIndexDoku2 = 0;

            if (!m_references[i].m_name.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_references[i].m_name.getStr();
                cpIndexName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexName);

            if (!m_references[i].m_doku.isEmpty())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_references[i].m_doku.getStr();
                cpIndexDoku2 = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexDoku2);

            pBuffer += writeUINT16(pBuffer, static_cast<sal_uInt16>(m_references[i].m_access));
        }
    }


    // get CP infos blop-length
    pInfo = root.m_next;
    sal_uInt32 cpBlopSize = 0;
    sal_uInt16 cpCount = 0;

    while (pInfo)
    {
        cpBlopSize += pInfo->getBlopSize();
        cpCount++;
        pInfo = pInfo->m_next;
    }

    blopSize += cpBlopSize;
    blopSize += sizeof(sal_uInt16);   // constantPoolCount

    // write all in flat buffer

    sal_uInt8 * blop = new sal_uInt8[blopSize];

    pBuffer = blop;

    // Assumes two's complement arithmetic with modulo-semantics:
    pBuffer += writeUINT32(pBuffer, magic + m_version);
    pBuffer += writeUINT32(pBuffer, blopSize);
    pBuffer += writeUINT16(pBuffer, minorVersion);
    pBuffer += writeUINT16(pBuffer, majorVersion);
    pBuffer += writeUINT16(pBuffer, BLOP_HEADER_N_ENTRIES);

    pBuffer += writeUINT16(pBuffer, sal_uInt16(RT_UNO_IDL));
    pBuffer += writeUINT16(pBuffer, static_cast<sal_uInt16>(m_typeClass));
    pBuffer += writeUINT16(pBuffer, cpIndexThisName);
    pBuffer += writeUINT16(pBuffer, 0); // cpIndexUik
    pBuffer += writeUINT16(pBuffer, cpIndexDoku);
    pBuffer += writeUINT16(pBuffer, cpIndexFileName);

    // write supertypes
    pBuffer += writeUINT16(pBuffer, m_nSuperTypes);
    if (m_nSuperTypes)
    {
        for (sal_uInt32 i=0; i < m_nSuperTypes; i++)
        {
            pBuffer += writeUINT16(pBuffer, cpIndexSuperNames[i]);
        }
        cpIndexSuperNames.reset();
    }

    pBuffer += writeUINT16(pBuffer, cpCount);

    // write and delete CP infos
    pInfo = root.m_next;

    while (pInfo)
    {
        CPInfo* pNextInfo = pInfo->m_next;

        pBuffer += pInfo->toBlop(pBuffer);
        delete pInfo;

        pInfo = pNextInfo;
    }

    auto writeList = [&pBuffer](
        sal_uInt16 count, sal_uInt8 * data, sal_uInt32 size)
    {
        pBuffer += writeUINT16(pBuffer, count);
        if (size != 0) {
            memcpy(pBuffer, data, size);
            pBuffer += size;
        }
    };

    // write fields
    writeList(m_fieldCount, pBlopFields.get(), blopFieldsSize);

    // write methods
    writeList(m_methodCount, pBlopMethods.get(), blopMethodsSize);

    // write references
    writeList(m_referenceCount, pBlopReferences.get(), blopReferenceSize);

    m_blop.reset( blop );
    m_blopSize = blopSize;
}


/**************************************************************************

    C-API

**************************************************************************/

extern "C" {

static void TYPEREG_CALLTYPE release(TypeWriterImpl hEntry)
{
    TypeWriter* pEntry = static_cast<TypeWriter*>(hEntry);

    if (pEntry != nullptr)
    {
        if (--pEntry->m_refCount == 0)
            delete pEntry;
    }
}

sal_Bool TYPEREG_CALLTYPE typereg_writer_setFieldData(
    void * handle, sal_uInt16 index, rtl_uString const * documentation,
    rtl_uString const * fileName, RTFieldAccess flags, rtl_uString const * name,
    rtl_uString const * typeName, RTValueType valueType,
    RTConstValueUnion valueValue)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter * >(handle)->m_fields[index].setData(
            toByteString(name), toByteString(typeName),
            toByteString(documentation), toByteString(fileName), flags,
            valueType, valueValue);
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

static void TYPEREG_CALLTYPE setFieldData(TypeWriterImpl    hEntry,
                                          sal_uInt16        index,
                                          rtl_uString const * name,
                                          rtl_uString const * typeName,
                                          rtl_uString const * doku,
                                          rtl_uString const * fileName,
                                          RTFieldAccess     access,
                                          RTValueType       valueType,
                                          RTConstValueUnion constValue)
{
    typereg_writer_setFieldData(
        hEntry, index, doku, fileName, access, name, typeName, valueType,
        constValue);
}

sal_Bool TYPEREG_CALLTYPE typereg_writer_setMethodData(
    void * handle, sal_uInt16 index, rtl_uString const * documentation,
    RTMethodMode flags, rtl_uString const * name,
    rtl_uString const * returnTypeName, sal_uInt16 parameterCount,
    sal_uInt16 exceptionCount)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter * >(handle)->m_methods[index].setData(
            toByteString(name), toByteString(returnTypeName), flags,
            parameterCount, exceptionCount, toByteString(documentation));
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

sal_Bool TYPEREG_CALLTYPE typereg_writer_setMethodParameterData(
    void const * handle, sal_uInt16 methodIndex, sal_uInt16 parameterIndex,
    RTParamMode flags, rtl_uString const * name, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter const * >(handle)->
            m_methods[methodIndex].m_params[parameterIndex].setData(
                toByteString(typeName), toByteString(name), flags);
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

sal_Bool TYPEREG_CALLTYPE typereg_writer_setMethodExceptionTypeName(
    void const * handle, sal_uInt16 methodIndex, sal_uInt16 exceptionIndex,
    rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter const * >(handle)->m_methods[methodIndex].setExcName(
            exceptionIndex, toByteString(typeName));
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

void const * TYPEREG_CALLTYPE typereg_writer_getBlob(void * handle, sal_uInt32 * size)
    SAL_THROW_EXTERN_C()
{
    TypeWriter * writer = static_cast< TypeWriter * >(handle);
    if (!writer->m_blop) {
        try {
            writer->createBlop();
        } catch (std::bad_alloc &) {
            return nullptr;
        }
    }
    *size = writer->m_blopSize;
    return writer->m_blop.get();
}

static const sal_uInt8* TYPEREG_CALLTYPE getBlop(TypeWriterImpl hEntry)
{
    sal_uInt32 size;
    return static_cast< sal_uInt8 const * >(
        typereg_writer_getBlob(hEntry, &size));
}

static sal_uInt32 TYPEREG_CALLTYPE getBlopSize(TypeWriterImpl hEntry)
{
    sal_uInt32 size;
    typereg_writer_getBlob(hEntry, &size);
    return size;
}

sal_Bool TYPEREG_CALLTYPE typereg_writer_setReferenceData(
    void * handle, sal_uInt16 index, rtl_uString const * documentation,
    RTReferenceType sort, RTFieldAccess flags, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter * >(handle)->m_references[index].setData(
            toByteString(typeName), sort, toByteString(documentation), flags);
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

void * TYPEREG_CALLTYPE typereg_writer_create(
    typereg_Version version, rtl_uString const * documentation,
    rtl_uString const * fileName, RTTypeClass typeClass, sal_Bool published,
    rtl_uString const * typeName, sal_uInt16 superTypeCount,
    sal_uInt16 fieldCount, sal_uInt16 methodCount, sal_uInt16 referenceCount)
    SAL_THROW_EXTERN_C()
{
    try {
        return new TypeWriter(
            version, toByteString(documentation), toByteString(fileName),
            typeClass, published, toByteString(typeName), superTypeCount,
            fieldCount, methodCount, referenceCount);
    } catch (std::bad_alloc &) {
        return nullptr;
    }
}

void TYPEREG_CALLTYPE typereg_writer_destroy(void * handle) SAL_THROW_EXTERN_C() {
    delete static_cast< TypeWriter * >(handle);
}

sal_Bool TYPEREG_CALLTYPE typereg_writer_setSuperTypeName(
    void const * handle, sal_uInt16 index, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter const * >(handle)->setSuperType(
            index, toByteString(typeName));
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

static TypeWriterImpl TYPEREG_CALLTYPE createEntry(
    RTTypeClass typeClass, rtl_uString const * typeName, rtl_uString const * superTypeName,
    sal_uInt16 fieldCount)
{
    OUString empty;
    sal_uInt16 superTypeCount = rtl_uString_getLength(superTypeName) == 0
        ? 0 : 1;
    TypeWriterImpl t = typereg_writer_create(
        TYPEREG_VERSION_0, empty.pData, empty.pData, typeClass, false, typeName,
        superTypeCount, fieldCount, 0/*methodCount*/, 0/*referenceCount*/);
    if (superTypeCount > 0) {
        typereg_writer_setSuperTypeName(t, 0, superTypeName);
    }
    return t;
}

}

RegistryTypeWriter::RegistryTypeWriter(RTTypeClass               RTTypeClass,
                                              const OUString&    typeName,
                                              const OUString&    superTypeName,
                                              sal_uInt16                fieldCount)
    : m_hImpl(nullptr)
{
    m_hImpl = createEntry(RTTypeClass,
                                  typeName.pData,
                                  superTypeName.pData,
                                  fieldCount);
}

RegistryTypeWriter::~RegistryTypeWriter()
{
    release(m_hImpl);
}

void RegistryTypeWriter::setFieldData( sal_uInt16              index,
                                              const OUString&    name,
                                              const OUString&    typeName,
                                              const OUString&    doku,
                                              const OUString&    fileName,
                                              RTFieldAccess           access,
                                              const RTConstValue&     constValue)
{
    ::setFieldData(m_hImpl, index, name.pData, typeName.pData, doku.pData, fileName.pData, access, constValue.m_type, constValue.m_value);
}

const sal_uInt8* RegistryTypeWriter::getBlop()
{
    return ::getBlop(m_hImpl);
}

sal_uInt32 RegistryTypeWriter::getBlopSize()
{
    return ::getBlopSize(m_hImpl);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
