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
#include <sal/types.h>
#include <sal/macros.h>
#include <osl/endian.h>
#include <rtl/alloc.h>
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"

#include "registry/reflwrit.hxx"
#include "registry/version.h"
#include "registry/writer.h"

#include "reflcnst.hxx"

using ::rtl::OString;


namespace {

inline rtl::OString toByteString(rtl_uString const * str) {
    return rtl::OString(
        str->buffer, str->length, RTL_TEXTENCODING_UTF8,
        OUSTRING_TO_OSTRING_CVTFLAGS);
}

}

static sal_Unicode NULL_WSTRING[1] = { 0 };

#define BLOP_OFFSET_MAGIC       0
#define BLOP_OFFSET_SIZE        (BLOP_OFFSET_MAGIC + sizeof(sal_uInt32))
#define BLOP_OFFSET_MINOR       (BLOP_OFFSET_SIZE + sizeof(sal_uInt32))
#define BLOP_OFFSET_MAJOR       (BLOP_OFFSET_MINOR + sizeof(sal_uInt16))
#define BLOP_OFFSET_N_ENTRIES   (BLOP_OFFSET_MAJOR + sizeof(sal_uInt16))
#define BLOP_OFFSET_TYPE_SOURCE (BLOP_OFFSET_N_ENTRIES + sizeof(sal_uInt16))
#define BLOP_OFFSET_TYPE_CLASS  (BLOP_OFFSET_TYPE_SOURCE + sizeof(sal_uInt16))
#define BLOP_OFFSET_THIS        (BLOP_OFFSET_TYPE_CLASS + sizeof(sal_uInt16))
#define BLOP_OFFSET_UIK         (BLOP_OFFSET_THIS + sizeof(sal_uInt16))
#define BLOP_OFFSET_DOKU        (BLOP_OFFSET_UIK + sizeof(sal_uInt16))
#define BLOP_OFFSET_FILENAME    (BLOP_OFFSET_DOKU + sizeof(sal_uInt16))
#define BLOP_HEADER_N_ENTRIES   6

#define BLOP_OFFSET_N_SUPERTYPES    0
#define BLOP_OFFSET_SUPERTYPES      (BLOP_OFFSET_N_SUPERTYPES + sizeof(sal_uInt16))

#define BLOP_FIELD_ENTRY_ACCESS     0
#define BLOP_FIELD_ENTRY_NAME       (BLOP_FIELD_ENTRY_ACCESS + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_TYPE       (BLOP_FIELD_ENTRY_NAME + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_VALUE      (BLOP_FIELD_ENTRY_TYPE + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_DOKU       (BLOP_FIELD_ENTRY_VALUE + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_FILENAME   (BLOP_FIELD_ENTRY_DOKU + sizeof(sal_uInt16))
#define BLOP_FIELD_N_ENTRIES        6

#define BLOP_METHOD_SIZE        0
#define BLOP_METHOD_MODE        (BLOP_METHOD_SIZE + sizeof(sal_uInt16))
#define BLOP_METHOD_NAME        (BLOP_METHOD_MODE + sizeof(sal_uInt16))
#define BLOP_METHOD_RETURN      (BLOP_METHOD_NAME + sizeof(sal_uInt16))
#define BLOP_METHOD_DOKU        (BLOP_METHOD_RETURN + sizeof(sal_uInt16))
#define BLOP_METHOD_N_ENTRIES   5

#define BLOP_PARAM_TYPE         0
#define BLOP_PARAM_MODE         (BLOP_PARAM_TYPE + sizeof(sal_uInt16))
#define BLOP_PARAM_NAME         (BLOP_PARAM_MODE + sizeof(sal_uInt16))
#define BLOP_PARAM_N_ENTRIES    3

#define BLOP_REFERENCE_TYPE         0
#define BLOP_REFERENCE_NAME         (BLOP_REFERENCE_TYPE + sizeof(sal_uInt16))
#define BLOP_REFERENCE_DOKU         (BLOP_REFERENCE_NAME + sizeof(sal_uInt16))
#define BLOP_REFERENCE_ACCESS       (BLOP_REFERENCE_DOKU + sizeof(sal_uInt16))
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
        buff += writeUINT16(buff, (sal_uInt16) v[i]);
    }

    return (buff - buffer);
}

sal_uInt32 readString(const sal_uInt8* buffer, sal_Unicode* v, sal_uInt32 maxSize)
{
    sal_uInt32 len = SAL_MIN(UINT16StringLen(buffer) + 1, maxSize / 2);
    sal_uInt32 i;
    sal_uInt8* buff = (sal_uInt8*)buffer;

    for (i = 0; i < (len - 1); i++)
    {
        sal_uInt16 aChar;

        buff += readUINT16(buff, aChar);

        v[i] = (sal_Unicode) aChar;
    }

    v[len - 1] = L'\0';

    return (buff - ((sal_uInt8*)buffer));
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
            sal_uInt32  b1;
            sal_uInt32  b2;
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
    CPInfoTag   m_tag;
    union
    {
        const sal_Char*     aUtf8;
        RTUik*              aUik;
        RTConstValueUnion   aConst;
    } m_value;

    sal_uInt16      m_index;
    struct CPInfo*  m_next;

    CPInfo(CPInfoTag tag, struct CPInfo* prev);

    sal_uInt32 getBlopSize();

    sal_uInt32 toBlop(sal_uInt8* buffer);
};

CPInfo::CPInfo(CPInfoTag tag, struct CPInfo* prev)
    : m_tag(tag)
    , m_index(0)
    , m_next(NULL)
{
    if (prev)
    {
        m_index = prev->m_index + 1;
        prev->m_next = this;
    }
}

sal_uInt32 CPInfo::getBlopSize()
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
    buff += writeUINT16(buff, (sal_uInt16) m_tag);

    switch (m_tag)
    {
        case CP_TAG_CONST_BOOL:
            buff += writeBYTE(buff, (sal_uInt8) m_value.aConst.aBool);
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
    : m_access(RT_ACCESS_INVALID)
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
        delete[] (sal_Unicode*)m_constValue.aString;
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
    sal_Unicode * newValue = 0;
    if (constValueType == RT_TYPE_STRING && constValue.aString != 0) {
        sal_Int32 n = rtl_ustr_getLength(constValue.aString) + 1;
        newValue = new sal_Unicode[n];
        memcpy(newValue, constValue.aString, n * sizeof (sal_Unicode));
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
        delete[] (sal_Unicode*)m_constValue.aString;
    }

    m_access = access;
    m_constValueType = constValueType;

    if (m_constValueType == RT_TYPE_STRING)
    {
        if (constValue.aString == NULL)
            m_constValue.aString = NULL_WSTRING;
        else
        {
            m_constValue.aString = newValue;
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
    ~ParamEntry();

    void setData(const OString& typeName,
                 const OString& name,
                 RTParamMode    mode);
};

ParamEntry::ParamEntry()
    : m_mode(RT_PARAM_INVALID)
{
}

ParamEntry::~ParamEntry()
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
    ~ReferenceEntry();

    void setData(const OString&     name,
                 RTReferenceType    refType,
                 const OString&     doku,
                 RTFieldAccess      access);
};

ReferenceEntry::ReferenceEntry()
    : m_type(RT_REF_INVALID)
    , m_access(RT_ACCESS_INVALID)
{
}

ReferenceEntry::~ReferenceEntry()
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
    ParamEntry*     m_params;
    sal_uInt16      m_excCount;
    OString*        m_excNames;
    OString         m_doku;

    MethodEntry();
    ~MethodEntry();

    void setData(const OString&    name,
                 const OString&    returnTypeName,
                 RTMethodMode      mode,
                 sal_uInt16        paramCount,
                 sal_uInt16        excCount,
                 const OString&    doku);

    void setExcName(sal_uInt16 excIndex, const OString& name);

protected:

    void reallocParams(sal_uInt16 size);
    void reallocExcs(sal_uInt16 size);
};

MethodEntry::MethodEntry()
    : m_mode(RT_MODE_INVALID)
    , m_paramCount(0)
    , m_params(NULL)
    , m_excCount(0)
    , m_excNames(NULL)
{
}

MethodEntry::~MethodEntry()
{
    if (m_params)
        delete[] m_params;

    if (m_excNames)
        delete[] m_excNames;
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

void MethodEntry::setExcName(sal_uInt16 excIndex, const OString& name)
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
        newParams = NULL;

    if (m_paramCount)
    {
        sal_uInt16 i;

        for (i = 0; i < SAL_MIN(size, m_paramCount); i++)
        {
            newParams[i].setData(m_params[i].m_typeName, m_params[i].m_name, m_params[i].m_mode);
        }

        delete[] m_params;
    }

    m_paramCount = size;
    m_params = newParams;
}

void MethodEntry::reallocExcs(sal_uInt16 size)
{
    OString* newExcNames;

    if (size)
        newExcNames = new OString[size];
    else
        newExcNames = NULL;

    sal_uInt16 i;

    for (i = 0; i < SAL_MIN(size, m_excCount); i++)
    {
        newExcNames[i] = m_excNames[i];
    }

    delete[] m_excNames;

    m_excCount = size;
    m_excNames = newExcNames;
}


/**************************************************************************

    class TypeRegistryEntry

**************************************************************************/

class TypeWriter
{

public:

    sal_uInt32          m_refCount;
    typereg_Version     m_version;
    RTTypeClass         m_typeClass;
    OString             m_typeName;
    sal_uInt16          m_nSuperTypes;
    OString*            m_superTypeNames;
    RTUik*              m_pUik;
    OString             m_doku;
    OString             m_fileName;
    sal_uInt16          m_fieldCount;
    FieldEntry*         m_fields;
    sal_uInt16          m_methodCount;
    MethodEntry*        m_methods;
    sal_uInt16          m_referenceCount;
    ReferenceEntry*     m_references;

    sal_uInt8*          m_blop;
    sal_uInt32          m_blopSize;

    TypeWriter(typereg_Version version,
               rtl::OString const & documentation,
               rtl::OString const & fileName,
               RTTypeClass      RTTypeClass,
               bool             published,
               const OString&   typeName,
               sal_uInt16       superTypeCount,
               sal_uInt16       FieldCount,
               sal_uInt16       methodCount,
               sal_uInt16       referenceCount);

    ~TypeWriter();

    void setSuperType(sal_uInt16 index, OString const & name);

    void createBlop(); // throws std::bad_alloc
};

TypeWriter::TypeWriter(typereg_Version version,
                       rtl::OString const & documentation,
                       rtl::OString const & fileName,
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
    , m_pUik(NULL)
    , m_doku(documentation)
    , m_fileName(fileName)
    , m_fieldCount(fieldCount)
    , m_methodCount(methodCount)
    , m_referenceCount(referenceCount)
    , m_blop(NULL)
    , m_blopSize(0)
{
    if (m_nSuperTypes > 0)
    {
        m_superTypeNames = new OString[m_nSuperTypes];
    } else
    {
        m_superTypeNames = NULL;
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
    if (m_superTypeNames)
        delete[] m_superTypeNames;

    if (m_blop)
        delete[] m_blop;

    if (m_fieldCount)
        delete[] m_fields;

    if (m_methodCount)
        delete[] m_methods;

    if (m_referenceCount)
        delete[] m_references;

    if (m_pUik)
        delete m_pUik;
}

void TypeWriter::setSuperType(sal_uInt16 index, OString const & name)
{
    m_superTypeNames[index] = name;
}

void TypeWriter::createBlop()
{
    //TODO: Fix memory leaks that occur when std::bad_alloc is thrown

    sal_uInt8*  pBlopFields         = NULL;
    sal_uInt8*  pBlopMethods        = NULL;
    sal_uInt8*  pBlopReferences     = NULL;
    sal_uInt8*  pBuffer             = NULL;
    sal_uInt32  blopFieldsSize      = 0;
    sal_uInt32  blopMethodsSize     = 0;
    sal_uInt32  blopReferenceSize   = 0;

    CPInfo  root(CP_TAG_INVALID, NULL);
    sal_uInt16  cpIndexThisName = 0;
    sal_uInt16* cpIndexSuperNames = NULL;
    sal_uInt16  cpIndexUik = 0;
    sal_uInt16  cpIndexDoku = 0;
    sal_uInt16  cpIndexFileName = 0;
    CPInfo* pInfo = NULL;

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

        cpIndexSuperNames = new sal_uInt16[m_nSuperTypes];

        for (sal_uInt32 i=0; i < m_nSuperTypes; i++)
        {
            pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
            pInfo->m_value.aUtf8 = m_superTypeNames[i].getStr();
            cpIndexSuperNames[i] = pInfo->m_index;
        }
    }

    // create CP entry for uik
    if (m_pUik != NULL)
    {
        pInfo = new CPInfo(CP_TAG_UIK, pInfo);
        pInfo->m_value.aUik = m_pUik;
        cpIndexUik = pInfo->m_index;
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

        pBlopFields = new sal_uInt8[blopFieldsSize];
        pBuffer = pBlopFields;

        pBuffer += writeUINT16(pBuffer, BLOP_FIELD_N_ENTRIES);

        for (sal_uInt16 i = 0; i < m_fieldCount; i++)
        {
            cpIndexName = 0;
            cpIndexTypeName = 0;
            cpIndexValue = 0;
            cpIndexDoku2 = 0;
            cpIndexFileName2 = 0;

            pBuffer += writeUINT16(pBuffer, m_fields[i].m_access);

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
                pInfo = new CPInfo((CPInfoTag)m_fields[i].m_constValueType, pInfo);
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
        sal_uInt16* pMethodEntrySize = new sal_uInt16[m_methodCount];
        sal_uInt16  cpIndexName = 0;
        sal_uInt16  cpIndexReturn = 0;
        sal_uInt16  cpIndexDoku2 = 0;

        // nMethodEntries + nParamEntries
        blopMethodsSize = (2 * sizeof(sal_uInt16));

        for (sal_uInt16 i = 0; i < m_methodCount; i++)
        {
            pMethodEntrySize[i] = (sal_uInt16)
                ( blopMethodEntrySize +                                 // header
                  sizeof(sal_uInt16) +                                  // parameterCount
                  (m_methods[i].m_paramCount * blopParamEntrySize) +    // exceptions
                  sizeof(sal_uInt16) +                                  // exceptionCount
                  (m_methods[i].m_excCount * sizeof(sal_uInt16)) );     // exceptions

            blopMethodsSize += pMethodEntrySize[i];
        }

        pBlopMethods = new sal_uInt8[blopMethodsSize];

        blopSize += blopMethodsSize;

        pBuffer = pBlopMethods;

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

        delete[] pMethodEntrySize;
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

        pBlopReferences = new sal_uInt8[blopReferenceSize];
        pBuffer = pBlopReferences;

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

            pBuffer += writeUINT16(pBuffer, m_references[i].m_access);
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

    pBuffer += writeUINT16(pBuffer, (sal_uInt16)RT_UNO_IDL);
    pBuffer += writeUINT16(pBuffer, (sal_uInt16)m_typeClass);
    pBuffer += writeUINT16(pBuffer, cpIndexThisName);
    pBuffer += writeUINT16(pBuffer, cpIndexUik);
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
        delete[] cpIndexSuperNames;
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

    // write fields
    pBuffer += writeUINT16(pBuffer, m_fieldCount);
    if (blopFieldsSize)
    {
        memcpy(pBuffer, pBlopFields, blopFieldsSize);
        pBuffer += blopFieldsSize;
    }

    // write methods
    pBuffer += writeUINT16(pBuffer, m_methodCount);
    if (blopMethodsSize)
    {
        memcpy(pBuffer, pBlopMethods, blopMethodsSize);
        pBuffer += blopMethodsSize;
    }

    // write references
    pBuffer += writeUINT16(pBuffer, m_referenceCount);
    if (blopReferenceSize)
    {
        memcpy(pBuffer, pBlopReferences, blopReferenceSize);
        pBuffer += blopReferenceSize;
    }

    delete[] pBlopFields;
    delete[] pBlopMethods;
    delete[] pBlopReferences;

    delete[] m_blop;
    m_blop = blop;
    m_blopSize = blopSize;
}


/**************************************************************************

    C-API

**************************************************************************/

extern "C" {

static void TYPEREG_CALLTYPE acquire(TypeWriterImpl hEntry)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
        pEntry->m_refCount++;
}

static void TYPEREG_CALLTYPE release(TypeWriterImpl hEntry)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
    {
        if (--pEntry->m_refCount == 0)
            delete pEntry;
    }
}

static void TYPEREG_CALLTYPE setUik(TypeWriterImpl  hEntry, const RTUik* uik)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
    {
        if (pEntry->m_pUik)
        {
            pEntry->m_pUik->m_Data1 = uik->m_Data1;
            pEntry->m_pUik->m_Data2 = uik->m_Data2;
            pEntry->m_pUik->m_Data3 = uik->m_Data3;
            pEntry->m_pUik->m_Data4 = uik->m_Data4;
            pEntry->m_pUik->m_Data5 = uik->m_Data5;
        }
        else
            pEntry->m_pUik = new RTUik(*uik);
    }
}

static void TYPEREG_CALLTYPE setDoku(TypeWriterImpl hEntry, rtl_uString* doku)
{
    static_cast< TypeWriter * >(hEntry)->m_doku = toByteString(doku);
}

static void TYPEREG_CALLTYPE setFileName(TypeWriterImpl hEntry, rtl_uString* fileName)
{
    static_cast< TypeWriter * >(hEntry)->m_fileName = toByteString(fileName);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_writer_setFieldData(
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
                                          rtl_uString*      name,
                                          rtl_uString*      typeName,
                                          rtl_uString*      doku,
                                          rtl_uString*      fileName,
                                          RTFieldAccess     access,
                                          RTValueType       valueType,
                                          RTConstValueUnion constValue)
{
    typereg_writer_setFieldData(
        hEntry, index, doku, fileName, access, name, typeName, valueType,
        constValue);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_writer_setMethodData(
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

static void TYPEREG_CALLTYPE setMethodData(TypeWriterImpl   hEntry,
                                           sal_uInt16       index,
                                           rtl_uString*     name,
                                           rtl_uString*     returnTypeName,
                                           RTMethodMode     mode,
                                           sal_uInt16       paramCount,
                                           sal_uInt16       excCount,
                                           rtl_uString*     doku)
{
    typereg_writer_setMethodData(
        hEntry, index, doku, mode, name, returnTypeName, paramCount, excCount);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_writer_setMethodParameterData(
    void * handle, sal_uInt16 methodIndex, sal_uInt16 parameterIndex,
    RTParamMode flags, rtl_uString const * name, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter * >(handle)->
            m_methods[methodIndex].m_params[parameterIndex].setData(
                toByteString(typeName), toByteString(name), flags);
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

static void TYPEREG_CALLTYPE setParamData(TypeWriterImpl    hEntry,
                                          sal_uInt16        index,
                                          sal_uInt16        paramIndex,
                                          rtl_uString*      type,
                                          rtl_uString*      name,
                                          RTParamMode       mode)
{
    typereg_writer_setMethodParameterData(
        hEntry, index, paramIndex, mode, name, type);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_writer_setMethodExceptionTypeName(
    void * handle, sal_uInt16 methodIndex, sal_uInt16 exceptionIndex,
    rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter * >(handle)->m_methods[methodIndex].setExcName(
            exceptionIndex, toByteString(typeName));
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

static void TYPEREG_CALLTYPE setExcData(TypeWriterImpl  hEntry,
                                        sal_uInt16      index,
                                        sal_uInt16      excIndex,
                                        rtl_uString*    type)
{
    typereg_writer_setMethodExceptionTypeName(hEntry, index, excIndex, type);
}

REG_DLLPUBLIC void const * TYPEREG_CALLTYPE typereg_writer_getBlob(void * handle, sal_uInt32 * size)
    SAL_THROW_EXTERN_C()
{
    TypeWriter * writer = static_cast< TypeWriter * >(handle);
    if (writer->m_blop == 0) {
        try {
            writer->createBlop();
        } catch (std::bad_alloc &) {
            return 0;
        }
    }
    *size = writer->m_blopSize;
    return writer->m_blop;
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

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_writer_setReferenceData(
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

static void TYPEREG_CALLTYPE setReferenceData(TypeWriterImpl    hEntry,
                                              sal_uInt16        index,
                                              rtl_uString*      name,
                                              RTReferenceType   refType,
                                              rtl_uString*      doku,
                                              RTFieldAccess     access)
{
    typereg_writer_setReferenceData(hEntry, index, doku, refType, access, name);
}

REG_DLLPUBLIC void * TYPEREG_CALLTYPE typereg_writer_create(
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
        return 0;
    }
}

REG_DLLPUBLIC void TYPEREG_CALLTYPE typereg_writer_destroy(void * handle) SAL_THROW_EXTERN_C() {
    delete static_cast< TypeWriter * >(handle);
}

REG_DLLPUBLIC sal_Bool TYPEREG_CALLTYPE typereg_writer_setSuperTypeName(
    void * handle, sal_uInt16 index, rtl_uString const * typeName)
    SAL_THROW_EXTERN_C()
{
    try {
        static_cast< TypeWriter * >(handle)->setSuperType(
            index, toByteString(typeName));
    } catch (std::bad_alloc &) {
        return false;
    }
    return true;
}

static TypeWriterImpl TYPEREG_CALLTYPE createEntry(
    RTTypeClass typeClass, rtl_uString * typeName, rtl_uString * superTypeName,
    sal_uInt16 fieldCount, sal_uInt16 methodCount, sal_uInt16 referenceCount)
{
    rtl::OUString empty;
    sal_uInt16 superTypeCount = rtl_uString_getLength(superTypeName) == 0
        ? 0 : 1;
    TypeWriterImpl t = typereg_writer_create(
        TYPEREG_VERSION_0, empty.pData, empty.pData, typeClass, false, typeName,
        superTypeCount, fieldCount, methodCount, referenceCount);
    if (superTypeCount > 0) {
        typereg_writer_setSuperTypeName(t, 0, superTypeName);
    }
    return t;
}

REG_DLLPUBLIC RegistryTypeWriter_Api* TYPEREG_CALLTYPE initRegistryTypeWriter_Api(void)
{
    static RegistryTypeWriter_Api aApi= {0,0,0,0,0,0,0,0,0,0,0,0,0};
    if (!aApi.acquire)
    {
        aApi.createEntry        = &createEntry;
        aApi.acquire            = &acquire;
        aApi.release            = &release;
        aApi.setUik             = &setUik;
        aApi.setDoku            = &setDoku;
        aApi.setFileName        = &setFileName;
        aApi.setFieldData       = &setFieldData;
        aApi.setMethodData      = &setMethodData;
        aApi.setParamData       = &setParamData;
        aApi.setExcData         = &setExcData;
        aApi.getBlop            = &getBlop;
        aApi.getBlopSize        = &getBlopSize;
        aApi.setReferenceData   = &setReferenceData;

        return (&aApi);
    }
    else
    {
        return (&aApi);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
