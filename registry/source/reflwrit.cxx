/*************************************************************************
 *
 *  $RCSfile: reflwrit.cxx,v $
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
#include <osl/types.h>

#include <rtl/alloc.h>
#include <registry/reflwrit.hxx>

#include "reflcnst.hxx"

using namespace rtl;
using namespace vos;

#ifdef MAC
#define strdup(str) strcpy((sal_Char*)malloc(strlen(str)+1),str)
#endif

static sal_Char NULL_STRING[1] = { 0 };
static sal_Unicode NULL_WSTRING[1] = { 0 };

#if defined ( GCC ) && ( defined ( SCO ) || defined ( NETBSD ) || defined ( OS2 ) )
ORealDynamicLoader* ODynamicLoader<RegistryTypeWriter_Api>::m_pLoader = NULL;
#endif

#define BLOP_OFFSET_MAGIC       0
#define BLOP_OFFSET_SIZE        (BLOP_OFFSET_MAGIC + sizeof(sal_uInt32))
#define BLOP_OFFSET_MINOR       (BLOP_OFFSET_SIZE + sizeof(sal_uInt32))
#define BLOP_OFFSET_MAJOR       (BLOP_OFFSET_MINOR + sizeof(sal_uInt16))
#define BLOP_OFFSET_TYPE_CLASS  (BLOP_OFFSET_MAJOR + sizeof(sal_uInt16))
#define BLOP_OFFSET_THIS        (BLOP_OFFSET_TYPE_CLASS + sizeof(sal_uInt16))
#define BLOP_OFFSET_SUPER       (BLOP_OFFSET_THIS + sizeof(sal_uInt16))
#define BLOP_OFFSET_UIK         (BLOP_OFFSET_SUPER + sizeof(sal_uInt16))
#define BLOP_OFFSET_DOKU        (BLOP_OFFSET_UIK + sizeof(sal_uInt16))
#define BLOP_OFFSET_FILENAME    (BLOP_OFFSET_DOKU + sizeof(sal_uInt16))
#define BLOP_HEADER_SIZE        (BLOP_OFFSET_FILENAME + sizeof(sal_uInt16))


#define BLOP_FIELD_ENTRY_ACCESS 0
#define BLOP_FIELD_ENTRY_NAME       (BLOP_FIELD_ENTRY_ACCESS + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_TYPE       (BLOP_FIELD_ENTRY_NAME + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_VALUE      (BLOP_FIELD_ENTRY_TYPE + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_DOKU       (BLOP_FIELD_ENTRY_VALUE + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_FILENAME   (BLOP_FIELD_ENTRY_DOKU + sizeof(sal_uInt16))
#define BLOP_FIELD_ENTRY_SIZE       (BLOP_FIELD_ENTRY_FILENAME + sizeof(sal_uInt16))

#define BLOP_METHOD_SIZE        0
#define BLOP_METHOD_MODE        (BLOP_METHOD_SIZE + sizeof(sal_uInt16))
#define BLOP_METHOD_NAME        (BLOP_METHOD_MODE + sizeof(sal_uInt16))
#define BLOP_METHOD_RETURN      (BLOP_METHOD_NAME + sizeof(sal_uInt16))
#define BLOP_METHOD_DOKU        (BLOP_METHOD_RETURN + sizeof(sal_uInt16))
#define BLOP_METHOD_HEADER_SIZE (BLOP_METHOD_DOKU + sizeof(sal_uInt16))

#define BLOP_PARAM_TYPE         0
#define BLOP_PARAM_MODE         (BLOP_PARAM_TYPE + sizeof(sal_uInt16))
#define BLOP_PARAM_NAME         (BLOP_PARAM_MODE + sizeof(sal_uInt16))
#define BLOP_PARAM_ENTRY_SIZE   (BLOP_PARAM_NAME + sizeof(sal_uInt16))

#define BLOP_REFERENCE_TYPE         0
#define BLOP_REFERENCE_NAME         (BLOP_REFERENCE_TYPE + sizeof(sal_uInt16))
#define BLOP_REFERENCE_DOKU         (BLOP_REFERENCE_NAME + sizeof(sal_uInt16))
#define BLOP_REFERENCE_ACCESS       (BLOP_REFERENCE_DOKU + sizeof(sal_uInt16))
#define BLOP_REFERENCE_ENTRY_SIZE   (BLOP_REFERENCE_ACCESS + sizeof(sal_uInt16))

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
    sal_uInt32 len = VOS_MIN(UINT16StringLen(buffer) + 1, maxSize / 2);
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

sal_uInt32 readFloat(const sal_uInt8* buffer, float& v)
{
    union
    {
        float   v;
        sal_uInt32  b;
    } x;

#ifdef REGTYPE_IEEE_NATIVE
    readUINT32(buffer, x.b);
#else
#   error no IEEE
#endif

    v = x.v;

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

sal_uInt32 readDouble(const sal_uInt8* buffer, double& v)
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

#ifdef REGTYPE_IEEE_NATIVE
#   ifdef OSL_BIGENDIAN
    readUINT32(buffer, x.b.b1);
    readUINT32(buffer + sizeof(sal_uInt32), x.b.b2);
#   else
    readUINT32(buffer, x.b.b2);
    readUINT32(buffer + sizeof(sal_uInt32), x.b.b1);
#   endif
#else
#   error no IEEE
#endif

    v = x.v;

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
        RTUik*          aUik;
        RTConstValueUnion   aConst;
/*      sal_Bool            aBool;
        sal_uInt8           aByte;
        sal_Int16           aShort;
        sal_uInt16          aUShort;
        sal_Int32           aLong;
        sal_uInt32          aULong;
//      INT64           aHyper;
//      UINT64          aUHyper;
        float           aFloat;
        double          aDouble;
        const sal_Unicode*  aString;*/
    } m_value;

    sal_uInt16          m_index;
    struct CPInfo*  m_next;

    CPInfo(CPInfoTag tag, struct CPInfo* prev);

    sal_uInt32 getBlopSize();

    sal_uInt32 toBlop(sal_uInt8* buffer, sal_uInt32 maxLen);
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
    sal_uInt32 size = sizeof(sal_uInt16) /* size */ + sizeof(sal_uInt16) /* tag */;

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
//          size += sizeof(sal_Int64);
            break;
        case CP_TAG_CONST_UINT64:
//          size += sizeof(sal_uInt64);
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


sal_uInt32 CPInfo::toBlop(sal_uInt8* buffer, sal_uInt32 maxLen)
{
    sal_uInt8* buff = buffer;

    buff += writeUINT16(buff, (sal_uInt16) getBlopSize());
    buff += writeUINT16(buff, (sal_uInt16) m_tag);

    switch (m_tag)
    {
        case CP_TAG_CONST_BOOL:
            buff += writeBYTE(buff, (sal_uInt8) m_value.aConst.aBool);
            break;
        case CP_TAG_CONST_BYTE:
            buff += writeBYTE(buff, m_value.aConst.aByte);
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
//          buff += writeUINT64(buff, m_value.aConst.aHyper);
            break;
        case CP_TAG_CONST_UINT64:
//          buff += writeUINT64(buff, m_value.aConst.aUHyper);
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
            sal_uInt32 len = rtl_ustr_getLength(constValue.aString);

            m_constValue.aString = new sal_Unicode[len + 1];
            rtl_copyMemory( (sal_Unicode*)m_constValue.aString, constValue.aString, (len+1) * sizeof(sal_Unicode) );
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

        for (i = 0; i < VOS_MIN(size, m_paramCount); i++)
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

    for (i = 0; i < VOS_MIN(size, m_excCount); i++)
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
    RTTypeClass         m_typeClass;
    OString             m_typeName;
    OString             m_superTypeName;
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

    TypeWriter(RTTypeClass      RTTypeClass,
               const OString&   typeName,
               const OString&   superTypeName,
               sal_uInt16       FieldCount,
               sal_uInt16       methodCount,
               sal_uInt16       referenceCount);

    ~TypeWriter();

    void createBlop();
};

TypeWriter::TypeWriter(RTTypeClass      RTTypeClass,
                       const OString&   typeName,
                       const OString&   superTypeName,
                       sal_uInt16       fieldCount,
                       sal_uInt16       methodCount,
                       sal_uInt16       referenceCount)
    : m_refCount(1)
    , m_typeClass(RTTypeClass)
     , m_typeName(typeName)
    , m_superTypeName(superTypeName)
    , m_fieldCount(fieldCount)
    , m_methodCount(methodCount)
    , m_referenceCount(referenceCount)
    , m_blop(NULL)
    , m_blopSize(0)
    , m_pUik(NULL)
{
    if (m_fieldCount)
        m_fields = new FieldEntry[fieldCount];

    if (m_methodCount)
        m_methods = new MethodEntry[methodCount];

    if (m_referenceCount)
        m_references = new ReferenceEntry[referenceCount];
}

TypeWriter::~TypeWriter()
{
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

void TypeWriter::createBlop()
{
    sal_uInt8*  pBlopFields         = NULL;
    sal_uInt8*  pBlopMethods        = NULL;
    sal_uInt8*  pBlopReferences     = NULL;
    sal_uInt8*  pBuffer             = NULL;
    sal_uInt32  blopFieldsSize      = 0;
    sal_uInt32  blopMethodsSize     = 0;
    sal_uInt32  blopReferenceSize   = 0;
    sal_uInt16  i;

    if (m_blop)
        delete[] m_blop;

    CPInfo  root(CP_TAG_INVALID, NULL);
    sal_uInt16  cpIndexThisName = 0;
    sal_uInt16  cpIndexSuperName = 0;
    sal_uInt16  cpIndexUik = 0;
    sal_uInt16  cpIndexDoku = 0;
    sal_uInt16  cpIndexFileName = 0;
    CPInfo* pInfo = NULL;

    m_blopSize = BLOP_HEADER_SIZE;

    // create CP entry for this name
    pInfo = new CPInfo(CP_TAG_UTF8_NAME, &root);
    pInfo->m_value.aUtf8 = m_typeName.getStr();
    cpIndexThisName = pInfo->m_index;

    // create CP entry for super name
    if (m_superTypeName.getLength())
    {
        pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
        pInfo->m_value.aUtf8 = m_superTypeName.getStr();
        cpIndexSuperName = pInfo->m_index;
    }

    // create CP entry for uik
    if (m_pUik != NULL)
    {
        pInfo = new CPInfo(CP_TAG_UIK, pInfo);
        pInfo->m_value.aUik = m_pUik;
        cpIndexUik = pInfo->m_index;
    }

    // create CP entry for doku
    if (m_doku.getLength())
    {
        pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
        pInfo->m_value.aUtf8 = m_doku.getStr();
        cpIndexDoku = pInfo->m_index;
    }

    // create CP entry for idl source filename
    if (m_fileName.getLength())
    {
        pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
        pInfo->m_value.aUtf8 = m_fileName.getStr();
        cpIndexFileName = pInfo->m_index;
    }

    // fields blop

    m_blopSize += sizeof(sal_uInt16); // fieldCount

    if (m_fieldCount)
    {
        sal_uInt16 cpIndexName = 0;
        sal_uInt16 cpIndexTypeName = 0;
        sal_uInt16 cpIndexValue = 0;
        sal_uInt16 cpIndexDoku = 0;
        sal_uInt16 cpIndexFileName = 0;

        blopFieldsSize = m_fieldCount * BLOP_FIELD_ENTRY_SIZE;

        m_blopSize += blopFieldsSize;

        pBlopFields = new sal_uInt8[blopFieldsSize];
        pBuffer = pBlopFields;

        for (i = 0; i < m_fieldCount; i++)
        {
            cpIndexName = 0;
            cpIndexTypeName = 0;
            cpIndexValue = 0;
            cpIndexDoku = 0;
            cpIndexFileName = 0;

            pBuffer += writeUINT16(pBuffer, m_fields[i].m_access);

            if (m_fields[i].m_name.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_name.getStr();
                cpIndexName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexName);

            if (m_fields[i].m_typeName.getLength())
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

            if (m_fields[i].m_doku.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_doku.getStr();
                cpIndexDoku = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexDoku);

            if (m_fields[i].m_fileName.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_fields[i].m_fileName.getStr();
                cpIndexFileName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexFileName);
        }
    }

    // methods blop

    m_blopSize += sizeof(sal_uInt16); // methodCount

    if (m_methodCount)
    {
        sal_uInt16* pMethodEntrySize = new sal_uInt16[m_methodCount];
        sal_uInt16  cpIndexName = 0;
        sal_uInt16  cpIndexReturn = 0;
        sal_uInt16  cpIndexDoku = 0;

        blopMethodsSize = 0;

        for (i = 0; i < m_methodCount; i++)
        {
            pMethodEntrySize[i] =
                BLOP_METHOD_HEADER_SIZE +                               // header
                sizeof(sal_uInt16) +                                        // parameterCount
                (m_methods[i].m_paramCount * BLOP_PARAM_ENTRY_SIZE) +   // exceptions
                sizeof(sal_uInt16) +                                        // exceptionCount
                (m_methods[i].m_excCount * sizeof(sal_uInt16));             // exceptions

            blopMethodsSize += pMethodEntrySize[i];
        }

        pBlopMethods = new sal_uInt8[blopMethodsSize];

        m_blopSize += blopMethodsSize;

        pBuffer = pBlopMethods;

        for (i = 0; i < m_methodCount; i++)
        {
            cpIndexReturn = 0;
            cpIndexDoku = 0;

            pBuffer += writeUINT16(pBuffer, pMethodEntrySize[i]);
            pBuffer += writeUINT16(pBuffer, m_methods[i].m_mode);

            if (m_methods[i].m_name.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_methods[i].m_name.getStr();
                cpIndexName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexName);
            cpIndexName = 0;

            if (m_methods[i].m_returnTypeName.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_methods[i].m_returnTypeName.getStr();
                cpIndexReturn = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexReturn);

            if (m_methods[i].m_doku.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_methods[i].m_doku.getStr();
                cpIndexDoku = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexDoku);

            sal_uInt16 j;

            pBuffer += writeUINT16(pBuffer, m_methods[i].m_paramCount);

            for (j = 0; j < m_methods[i].m_paramCount; j++)
            {
                if (m_methods[i].m_params[j].m_typeName.getLength())
                {
                    pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                    pInfo->m_value.aUtf8 = m_methods[i].m_params[j].m_typeName.getStr();
                    cpIndexName = pInfo->m_index;
                }
                pBuffer += writeUINT16(pBuffer, cpIndexName);
                cpIndexName = 0;

                pBuffer += writeUINT16(pBuffer, m_methods[i].m_params[j].m_mode);

                if (m_methods[i].m_params[j].m_name.getLength())
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
                if (m_methods[i].m_excNames[j].getLength())
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

    m_blopSize += sizeof(sal_uInt16); // referenceCount

    if (m_referenceCount)
    {
        sal_uInt16 cpIndexName = 0;
        sal_uInt16 cpIndexDoku = 0;

        blopReferenceSize = m_referenceCount * BLOP_REFERENCE_ENTRY_SIZE;

        m_blopSize += blopReferenceSize;

        pBlopReferences = new sal_uInt8[blopReferenceSize];
        pBuffer = pBlopReferences;

        for (i = 0; i < m_referenceCount; i++)
        {
            pBuffer += writeUINT16(pBuffer, m_references[i].m_type);

            cpIndexName = 0;
            cpIndexDoku = 0;

            if (m_references[i].m_name.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_references[i].m_name.getStr();
                cpIndexName = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexName);

            if (m_references[i].m_doku.getLength())
            {
                pInfo = new CPInfo(CP_TAG_UTF8_NAME, pInfo);
                pInfo->m_value.aUtf8 = m_references[i].m_doku.getStr();
                cpIndexDoku = pInfo->m_index;
            }
            pBuffer += writeUINT16(pBuffer, cpIndexDoku);

            pBuffer += writeUINT16(pBuffer, m_references[i].m_access);
        }
    }


    // CP infos blop-laenge holen
    pInfo = root.m_next;
    sal_uInt32 cpBlopSize = 0;
    sal_uInt16 cpCount = 0;

    while (pInfo)
    {
        cpBlopSize += pInfo->getBlopSize();
        cpCount++;
        pInfo = pInfo->m_next;
    }

    m_blopSize += cpBlopSize;
    m_blopSize += sizeof(sal_uInt16);   // constantPoolCount

    // write all in flat buffer

    m_blop = new sal_uInt8[m_blopSize];

    pBuffer = m_blop;

    pBuffer += writeUINT32(pBuffer, magic);
    pBuffer += writeUINT32(pBuffer, m_blopSize);
    pBuffer += writeUINT16(pBuffer, minorVersion);
    pBuffer += writeUINT16(pBuffer, majorVersion);
    pBuffer += writeUINT16(pBuffer, (sal_uInt16)m_typeClass);
    pBuffer += writeUINT16(pBuffer, cpIndexThisName);
    pBuffer += writeUINT16(pBuffer, cpIndexSuperName);
    pBuffer += writeUINT16(pBuffer, cpIndexUik);
    pBuffer += writeUINT16(pBuffer, cpIndexDoku);
    pBuffer += writeUINT16(pBuffer, cpIndexFileName);
    pBuffer += writeUINT16(pBuffer, cpCount);

    // write and delete CP infos
    pInfo = root.m_next;

    while (pInfo)
    {
        CPInfo* pNextInfo = pInfo->m_next;

        pBuffer += pInfo->toBlop(pBuffer, m_blopSize - (pBuffer - m_blop));
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
}


/**************************************************************************

    C-API

**************************************************************************/

static TypeWriterImpl TYPEREG_CALLTYPE createEntry(RTTypeClass  RTTypeClass,
                                                   rtl_uString*  typeName,
                                                   rtl_uString*  superTypeName,
                                                   sal_uInt16       FieldCount,
                                                   sal_uInt16       methodCount,
                                                   sal_uInt16       referenceCount)
{
    OString rTypeName, rSuperTypeName;

    rtl_uString2String( &rTypeName.pData, typeName->buffer, typeName->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
    rtl_uString2String( &rSuperTypeName.pData, superTypeName->buffer, superTypeName->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

    return new TypeWriter(RTTypeClass,
                          rTypeName,
                          rSuperTypeName,
                          FieldCount,
                          methodCount,
                          referenceCount);
}

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
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
    {
        if (doku)
        {
            OString rDoku;
            rtl_uString2String( &rDoku.pData, doku->buffer, doku->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
            pEntry->m_doku = rDoku;
        } else
        {
            pEntry->m_doku = NULL_STRING;
        }
    }
}

static void TYPEREG_CALLTYPE setFileName(TypeWriterImpl hEntry, rtl_uString* fileName)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
    {
        if (fileName)
        {
            OString rFileName;
            rtl_uString2String( &rFileName.pData, fileName->buffer, fileName->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
            pEntry->m_fileName = rFileName;
        } else
        {
            pEntry->m_fileName = NULL_STRING;
        }
    }
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
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if ((pEntry != NULL) && (index < pEntry->m_fieldCount))
    {
        OString rName, rTypeName, rDoku, rFileName;
        rtl_uString2String( &rName.pData, name->buffer, name->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rTypeName.pData, typeName->buffer, typeName->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rDoku.pData, doku->buffer, doku->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rFileName.pData, fileName->buffer, fileName->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        pEntry->m_fields[index].setData(rName, rTypeName, rDoku, rFileName, access, valueType, constValue);
    }
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
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if ((pEntry != NULL) && (index < pEntry->m_methodCount))
    {
        OString rName, rReturnTypeName, rDoku;
        rtl_uString2String( &rName.pData, name->buffer, name->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rReturnTypeName.pData, returnTypeName->buffer, returnTypeName->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rDoku.pData, doku->buffer, doku->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        pEntry->m_methods[index].setData(rName, rReturnTypeName, mode, paramCount, excCount, rDoku);
    }
}

static void TYPEREG_CALLTYPE setParamData(TypeWriterImpl    hEntry,
                                          sal_uInt16        index,
                                          sal_uInt16        paramIndex,
                                          rtl_uString*      type,
                                          rtl_uString*      name,
                                          RTParamMode       mode)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if ((pEntry != NULL) &&
        (index < pEntry->m_methodCount) &&
        (paramIndex < pEntry->m_methods[index].m_paramCount))
    {
        OString rName, rType;
        rtl_uString2String( &rName.pData, name->buffer, name->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rType.pData, type->buffer, type->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        pEntry->m_methods[index].m_params[paramIndex].setData(rType, rName, mode);
    }
}

static void TYPEREG_CALLTYPE setExcData(TypeWriterImpl  hEntry,
                                        sal_uInt16      index,
                                        sal_uInt16      excIndex,
                                        rtl_uString*    type)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if ((pEntry != NULL) && (index < pEntry->m_methodCount))
    {
        OString rType;
        rtl_uString2String( &rType.pData, type->buffer, type->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        pEntry->m_methods[index].setExcName(excIndex, rType);
    }
}

static const sal_uInt8* TYPEREG_CALLTYPE getBlop(TypeWriterImpl hEntry)
{
    const sal_uInt8* blop = NULL;

    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
    {
        if (!pEntry->m_blop)
            pEntry->createBlop();

        blop = pEntry->m_blop;
    }

    return blop;
}

static sal_uInt32 TYPEREG_CALLTYPE getBlopSize(TypeWriterImpl hEntry)
{
    sal_uInt32 aSize = 0;

    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if (pEntry != NULL)
    {
        if (!pEntry->m_blop)
            pEntry->createBlop();

        aSize = pEntry->m_blopSize;
    }

    return aSize;
}

static void TYPEREG_CALLTYPE setReferenceData(TypeWriterImpl    hEntry,
                                              sal_uInt16        index,
                                              rtl_uString*      name,
                                              RTReferenceType   refType,
                                              rtl_uString*      doku,
                                              RTFieldAccess     access)
{
    TypeWriter* pEntry = (TypeWriter*) hEntry;

    if ((pEntry != NULL) && (index < pEntry->m_referenceCount))
    {
        OString rName, rDoku;
        rtl_uString2String( &rName.pData, name->buffer, name->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);
        rtl_uString2String( &rDoku.pData, doku->buffer, doku->length, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

        pEntry->m_references[index].setData(rName,
                                            refType,
                                            rDoku,
                                            access);
    }
}


extern "C" RegistryTypeWriter_Api* TYPEREG_CALLTYPE initRegistryTypeWriter_Api(void)
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





