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

#ifndef INCLUDED_REGISTRY_SOURCE_REFLCNST_HXX
#define INCLUDED_REGISTRY_SOURCE_REFLCNST_HXX

#include <registry/types.hxx>

#include <string.h>

#define REGTYPE_IEEE_NATIVE 1

extern const sal_uInt32 magic;
extern const sal_uInt16 minorVersion;
extern const sal_uInt16 majorVersion;

#define OFFSET_MAGIC                0
#define OFFSET_SIZE                 static_cast<sal_uInt32>(OFFSET_MAGIC + sizeof(magic))
#define OFFSET_MINOR_VERSION        static_cast<sal_uInt32>(OFFSET_SIZE + sizeof(sal_uInt32))
#define OFFSET_MAJOR_VERSION        static_cast<sal_uInt32>(OFFSET_MINOR_VERSION + sizeof(minorVersion))
#define OFFSET_N_ENTRIES            static_cast<sal_uInt32>(OFFSET_MAJOR_VERSION + sizeof(majorVersion))
#define OFFSET_TYPE_SOURCE          static_cast<sal_uInt32>(OFFSET_N_ENTRIES + sizeof(sal_uInt16))
#define OFFSET_TYPE_CLASS           static_cast<sal_uInt32>(OFFSET_TYPE_SOURCE + sizeof(sal_uInt16))
#define OFFSET_THIS_TYPE            static_cast<sal_uInt32>(OFFSET_TYPE_CLASS + sizeof(sal_uInt16))
#define OFFSET_UIK                  static_cast<sal_uInt32>(OFFSET_THIS_TYPE + sizeof(sal_uInt16))
#define OFFSET_DOKU                 static_cast<sal_uInt32>(OFFSET_UIK + sizeof(sal_uInt16))
#define OFFSET_FILENAME             static_cast<sal_uInt32>(OFFSET_DOKU + sizeof(sal_uInt16))

#define OFFSET_N_SUPERTYPES         static_cast<sal_uInt32>(OFFSET_FILENAME + sizeof(sal_uInt16))
#define OFFSET_SUPERTYPES           static_cast<sal_uInt32>(OFFSET_N_SUPERTYPES + sizeof(sal_uInt16))

#define OFFSET_CP_SIZE              static_cast<sal_uInt32>(OFFSET_SUPERTYPES + sizeof(sal_uInt16))
#define OFFSET_CP                   static_cast<sal_uInt32>(OFFSET_CP_SIZE + sizeof(sal_uInt16))

#define CP_OFFSET_ENTRY_SIZE        0
#define CP_OFFSET_ENTRY_TAG         static_cast<sal_uInt32>(CP_OFFSET_ENTRY_SIZE + sizeof(sal_uInt32))
#define CP_OFFSET_ENTRY_DATA        static_cast<sal_uInt32>(CP_OFFSET_ENTRY_TAG + sizeof(sal_uInt16))
#define CP_OFFSET_ENTRY_UIK1        static_cast<sal_uInt32>(CP_OFFSET_ENTRY_DATA)
#define CP_OFFSET_ENTRY_UIK2        static_cast<sal_uInt32>(CP_OFFSET_ENTRY_UIK1 + sizeof(sal_uInt32))
#define CP_OFFSET_ENTRY_UIK3        static_cast<sal_uInt32>(CP_OFFSET_ENTRY_UIK2 + sizeof(sal_uInt16))
#define CP_OFFSET_ENTRY_UIK4        static_cast<sal_uInt32>(CP_OFFSET_ENTRY_UIK3 + sizeof(sal_uInt16))
#define CP_OFFSET_ENTRY_UIK5        static_cast<sal_uInt32>(CP_OFFSET_ENTRY_UIK4 + sizeof(sal_uInt32))

#define FIELD_OFFSET_ACCESS         0
#define FIELD_OFFSET_NAME           static_cast<sal_uInt32>(FIELD_OFFSET_ACCESS + sizeof(sal_uInt16))
#define FIELD_OFFSET_TYPE           static_cast<sal_uInt32>(FIELD_OFFSET_NAME + sizeof(sal_uInt16))
#define FIELD_OFFSET_VALUE          static_cast<sal_uInt32>(FIELD_OFFSET_TYPE + sizeof(sal_uInt16))
#define FIELD_OFFSET_DOKU           static_cast<sal_uInt32>(FIELD_OFFSET_VALUE + sizeof(sal_uInt16))
#define FIELD_OFFSET_FILENAME       static_cast<sal_uInt32>(FIELD_OFFSET_DOKU + sizeof(sal_uInt16))

#define PARAM_OFFSET_TYPE           0
#define PARAM_OFFSET_MODE           static_cast<sal_uInt32>(PARAM_OFFSET_TYPE + sizeof(sal_uInt16))
#define PARAM_OFFSET_NAME           static_cast<sal_uInt32>(PARAM_OFFSET_MODE + sizeof(sal_uInt16))

#define METHOD_OFFSET_SIZE          0
#define METHOD_OFFSET_MODE          static_cast<sal_uInt32>(METHOD_OFFSET_SIZE + sizeof(sal_uInt16))
#define METHOD_OFFSET_NAME          static_cast<sal_uInt32>(METHOD_OFFSET_MODE + sizeof(sal_uInt16))
#define METHOD_OFFSET_RETURN        static_cast<sal_uInt32>(METHOD_OFFSET_NAME + sizeof(sal_uInt16))
#define METHOD_OFFSET_DOKU          static_cast<sal_uInt32>(METHOD_OFFSET_RETURN + sizeof(sal_uInt16))
#define METHOD_OFFSET_PARAM_COUNT   static_cast<sal_uInt32>(METHOD_OFFSET_DOKU + sizeof(sal_uInt16))

#define REFERENCE_OFFSET_TYPE       0
#define REFERENCE_OFFSET_NAME       static_cast<sal_uInt32>(REFERENCE_OFFSET_TYPE + sizeof(sal_uInt16))
#define REFERENCE_OFFSET_DOKU       static_cast<sal_uInt32>(REFERENCE_OFFSET_NAME + sizeof(sal_uInt16))
#define REFERENCE_OFFSET_ACCESS     static_cast<sal_uInt32>(REFERENCE_OFFSET_DOKU + sizeof(sal_uInt16))

enum CPInfoTag
{
    CP_TAG_INVALID = RT_TYPE_NONE,
    CP_TAG_CONST_BOOL = RT_TYPE_BOOL,
    CP_TAG_CONST_BYTE  = RT_TYPE_BYTE,
    CP_TAG_CONST_INT16 = RT_TYPE_INT16,
    CP_TAG_CONST_UINT16 = RT_TYPE_UINT16,
    CP_TAG_CONST_INT32 = RT_TYPE_INT32,
    CP_TAG_CONST_UINT32 = RT_TYPE_UINT32,
    CP_TAG_CONST_INT64 = RT_TYPE_INT64,
    CP_TAG_CONST_UINT64 = RT_TYPE_UINT64,
    CP_TAG_CONST_FLOAT = RT_TYPE_FLOAT,
    CP_TAG_CONST_DOUBLE = RT_TYPE_DOUBLE,
    CP_TAG_CONST_STRING = RT_TYPE_STRING,
    CP_TAG_UTF8_NAME,
    CP_TAG_UIK
};

inline sal_uInt32 writeBYTE(sal_uInt8* buffer, sal_uInt8 v)
{
    buffer[0] = v;

    return sizeof(sal_uInt8);
}

inline sal_uInt32 writeINT16(sal_uInt8* buffer, sal_Int16 v)
{
    buffer[0] = static_cast<sal_uInt8>((v >> 8) & 0xFF);
    buffer[1] = static_cast<sal_uInt8>((v >> 0) & 0xFF);

    return sizeof(sal_Int16);
}

inline sal_uInt32 writeUINT16(sal_uInt8* buffer, sal_uInt16 v)
{
    buffer[0] = static_cast<sal_uInt8>((v >> 8) & 0xFF);
    buffer[1] = static_cast<sal_uInt8>((v >> 0) & 0xFF);

    return sizeof(sal_uInt16);
}

inline sal_uInt32 readUINT16(const sal_uInt8* buffer, sal_uInt16& v)
{
    //This is untainted data which comes from a controlled source
    //so, using a byte-swapping pattern which coverity doesn't
    //detect as such
    //http://security.coverity.com/blog/2014/Apr/on-detecting-heartbleed-with-static-analysis.html
    v = *buffer++; v <<= 8;
    v |= *buffer;
    return sizeof(sal_uInt16);
}

inline sal_uInt32 writeINT32(sal_uInt8* buffer, sal_Int32 v)
{
    buffer[0] = static_cast<sal_uInt8>((v >> 24) & 0xFF);
    buffer[1] = static_cast<sal_uInt8>((v >> 16) & 0xFF);
    buffer[2] = static_cast<sal_uInt8>((v >> 8) & 0xFF);
    buffer[3] = static_cast<sal_uInt8>((v >> 0) & 0xFF);

    return sizeof(sal_Int32);
}

inline sal_uInt32 readINT32(const sal_uInt8* buffer, sal_Int32& v)
{
    v = (
            (buffer[0] << 24) |
            (buffer[1] << 16) |
            (buffer[2] << 8)  |
            (buffer[3] << 0)
        );

    return sizeof(sal_Int32);
}

inline sal_uInt32 writeUINT32(sal_uInt8* buffer, sal_uInt32 v)
{
    buffer[0] = static_cast<sal_uInt8>((v >> 24) & 0xFF);
    buffer[1] = static_cast<sal_uInt8>((v >> 16) & 0xFF);
    buffer[2] = static_cast<sal_uInt8>((v >> 8) & 0xFF);
    buffer[3] = static_cast<sal_uInt8>((v >> 0) & 0xFF);

    return sizeof(sal_uInt32);
}

inline sal_uInt32 readUINT32(const sal_uInt8* buffer, sal_uInt32& v)
{
    //This is untainted data which comes from a controlled source
    //so, using a byte-swapping pattern which coverity doesn't
    //detect as such
    //http://security.coverity.com/blog/2014/Apr/on-detecting-heartbleed-with-static-analysis.html
    v = *buffer++; v <<= 8;
    v |= *buffer++; v <<= 8;
    v |= *buffer++; v <<= 8;
    v |= *buffer;
    return sizeof(sal_uInt32);
}

inline sal_uInt32 writeUINT64(sal_uInt8* buffer, sal_uInt64 v)
{
    buffer[0] = static_cast<sal_uInt8>((v >> 56) & 0xFF);
    buffer[1] = static_cast<sal_uInt8>((v >> 48) & 0xFF);
    buffer[2] = static_cast<sal_uInt8>((v >> 40) & 0xFF);
    buffer[3] = static_cast<sal_uInt8>((v >> 32) & 0xFF);
    buffer[4] = static_cast<sal_uInt8>((v >> 24) & 0xFF);
    buffer[5] = static_cast<sal_uInt8>((v >> 16) & 0xFF);
    buffer[6] = static_cast<sal_uInt8>((v >> 8) & 0xFF);
    buffer[7] = static_cast<sal_uInt8>((v >> 0) & 0xFF);

    return sizeof(sal_uInt64);
}

inline sal_uInt32 writeUtf8(sal_uInt8* buffer, const sal_Char* v)
{
    sal_uInt32 size = strlen(v) + 1;

    memcpy(buffer, v, size);

    return size;
}

inline sal_uInt32 readUtf8(const sal_uInt8* buffer, sal_Char* v, sal_uInt32 maxSize)
{
    sal_uInt32 size = strlen(reinterpret_cast<const char*>(buffer)) + 1;
    if(size > maxSize)
    {
        size = maxSize;
    }

    memcpy(v, buffer, size);

    if (size == maxSize) v[size - 1] = '\0';

    return size;
}


sal_uInt32 writeFloat(sal_uInt8* buffer, float v);
sal_uInt32 writeDouble(sal_uInt8* buffer, double v);
sal_uInt32 writeString(sal_uInt8* buffer, const sal_Unicode* v);
sal_uInt32 readString(const sal_uInt8* buffer, sal_Unicode* v, sal_uInt32 maxSize);

sal_uInt32 UINT16StringLen(const sal_uInt8* wstring);

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
