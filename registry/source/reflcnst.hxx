/*************************************************************************
 *
 *  $RCSfile: reflcnst.hxx,v $
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

#ifndef __REFLCNST_HXX__
#define __REFLCNST_HXX__

#ifndef __REGISTRY_REFLTYPE_HXX__
#include    <registry/refltype.hxx>
#endif

#include <string.h>
#include <vos/macros.hxx>

#define REGTYPE_IEEE_NATIVE 1

extern const sal_uInt32 magic;
extern const sal_uInt16 minorVersion;
extern const sal_uInt16 majorVersion;

#define OFFSET_MAGIC                0
#define OFFSET_SIZE                 (OFFSET_MAGIC + sizeof(magic))
#define OFFSET_MINOR_VERSION        (OFFSET_SIZE + sizeof(sal_uInt32))
#define OFFSET_MAJOR_VERSION        (OFFSET_MINOR_VERSION + sizeof(minorVersion))
#define OFFSET_N_ENTRIES            (OFFSET_MAJOR_VERSION + sizeof(sal_uInt16))
#define OFFSET_TYPE_SOURCE          (OFFSET_N_ENTRIES + sizeof(sal_uInt16))
#define OFFSET_TYPE_CLASS           (OFFSET_TYPE_SOURCE + sizeof(sal_uInt16))
#define OFFSET_THIS_TYPE            (OFFSET_TYPE_CLASS + sizeof(sal_uInt16))
#define OFFSET_SUPER_TYPE           (OFFSET_THIS_TYPE + sizeof(sal_uInt16))
#define OFFSET_UIK                  (OFFSET_SUPER_TYPE + sizeof(sal_uInt16))
#define OFFSET_DOKU                 (OFFSET_UIK + sizeof(sal_uInt16))
#define OFFSET_FILENAME             (OFFSET_DOKU + sizeof(sal_uInt16))

#define OFFSET_N_SUPERTYPES         (OFFSET_FILENAME + sizeof(sal_uInt16))
#define OFFSET_SUPERTYPES           (OFFSET_N_SUPERTYPES + sizeof(sal_uInt16))

#define OFFSET_CP_SIZE              (OFFSET_SUPERTYPES + sizeof(sal_uInt16))
#define OFFSET_CP                   (OFFSET_CP_SIZE + sizeof(sal_uInt16))

#define CP_OFFSET_ENTRY_SIZE        0
#define CP_OFFSET_ENTRY_TAG         (CP_OFFSET_ENTRY_SIZE + sizeof(sal_uInt32))
#define CP_OFFSET_ENTRY_DATA        (CP_OFFSET_ENTRY_TAG + sizeof(sal_uInt16))
#define CP_OFFSET_ENTRY_UIK1        CP_OFFSET_ENTRY_DATA
#define CP_OFFSET_ENTRY_UIK2        (CP_OFFSET_ENTRY_UIK1 + sizeof(sal_uInt32))
#define CP_OFFSET_ENTRY_UIK3        (CP_OFFSET_ENTRY_UIK2 + sizeof(sal_uInt16))
#define CP_OFFSET_ENTRY_UIK4        (CP_OFFSET_ENTRY_UIK3 + sizeof(sal_uInt16))
#define CP_OFFSET_ENTRY_UIK5        (CP_OFFSET_ENTRY_UIK4 + sizeof(sal_uInt32))

#define FIELD_OFFSET_ACCESS         0
#define FIELD_OFFSET_NAME           (FIELD_OFFSET_ACCESS + sizeof(sal_uInt16))
#define FIELD_OFFSET_TYPE           (FIELD_OFFSET_NAME + sizeof(sal_uInt16))
#define FIELD_OFFSET_VALUE          (FIELD_OFFSET_TYPE + sizeof(sal_uInt16))
#define FIELD_OFFSET_DOKU           (FIELD_OFFSET_VALUE + sizeof(sal_uInt16))
#define FIELD_OFFSET_FILENAME       (FIELD_OFFSET_DOKU + sizeof(sal_uInt16))
//#define FIELD_ENTRY_SIZE          (FIELD_OFFSET_FILENAME + sizeof(sal_uInt16))

#define PARAM_OFFSET_TYPE           0
#define PARAM_OFFSET_MODE           (PARAM_OFFSET_TYPE + sizeof(sal_uInt16))
#define PARAM_OFFSET_NAME           (PARAM_OFFSET_MODE + sizeof(sal_uInt16))
//#define PARAM_ENTRY_SIZE          (PARAM_OFFSET_NAME + sizeof(sal_uInt16))

#define METHOD_OFFSET_SIZE          0
#define METHOD_OFFSET_MODE          (METHOD_OFFSET_SIZE + sizeof(sal_uInt16))
#define METHOD_OFFSET_NAME          (METHOD_OFFSET_MODE + sizeof(sal_uInt16))
#define METHOD_OFFSET_RETURN        (METHOD_OFFSET_NAME + sizeof(sal_uInt16))
#define METHOD_OFFSET_DOKU          (METHOD_OFFSET_RETURN + sizeof(sal_uInt16))
#define METHOD_OFFSET_PARAM_COUNT   (METHOD_OFFSET_DOKU + sizeof(sal_uInt16))
//#define METHOD_OFFSET_PARAM(i)        (METHOD_OFFSET_PARAM_COUNT + sizeof(sal_uInt16) + (i * PARAM_ENTRY_SIZE))

#define REFERENCE_OFFSET_TYPE       0
#define REFERENCE_OFFSET_NAME       (REFERENCE_OFFSET_TYPE + sizeof(sal_uInt16))
#define REFERENCE_OFFSET_DOKU       (REFERENCE_OFFSET_NAME + sizeof(sal_uInt16))
#define REFERENCE_OFFSET_ACCESS     (REFERENCE_OFFSET_DOKU + sizeof(sal_uInt16))
//#define REFERENCE_ENTRY_SIZE      (REFERENCE_OFFSET_ACCESS + sizeof(sal_uInt16))

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

inline sal_uInt16 readBYTE(const sal_uInt8* buffer, sal_uInt8& v)
{
    v = buffer[0];

    return sizeof(sal_uInt8);
}

inline sal_uInt32 writeINT16(sal_uInt8* buffer, sal_Int16 v)
{
    buffer[0] = (sal_uInt8)((v >> 8) & 0xFF);
    buffer[1] = (sal_uInt8)((v >> 0) & 0xFF);

    return sizeof(sal_Int16);
}

inline sal_uInt32 readINT16(const sal_uInt8* buffer, sal_Int16& v)
{
    v = ((buffer[0] << 8) | (buffer[1] << 0));

    return sizeof(sal_Int16);
}

inline sal_uInt32 writeUINT16(sal_uInt8* buffer, sal_uInt16 v)
{
    buffer[0] = (sal_uInt8)((v >> 8) & 0xFF);
    buffer[1] = (sal_uInt8)((v >> 0) & 0xFF);

    return sizeof(sal_uInt16);
}

inline sal_uInt32 readUINT16(const sal_uInt8* buffer, sal_uInt16& v)
{
    v = ((buffer[0] << 8) | (buffer[1] << 0));

    return sizeof(sal_uInt16);
}

inline sal_uInt32 writeINT32(sal_uInt8* buffer, sal_Int32 v)
{
    buffer[0] = (sal_uInt8)((v >> 24) & 0xFF);
    buffer[1] = (sal_uInt8)((v >> 16) & 0xFF);
    buffer[2] = (sal_uInt8)((v >> 8) & 0xFF);
    buffer[3] = (sal_uInt8)((v >> 0) & 0xFF);

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
    buffer[0] = (sal_uInt8)((v >> 24) & 0xFF);
    buffer[1] = (sal_uInt8)((v >> 16) & 0xFF);
    buffer[2] = (sal_uInt8)((v >> 8) & 0xFF);
    buffer[3] = (sal_uInt8)((v >> 0) & 0xFF);

    return sizeof(sal_uInt32);
}

inline sal_uInt32 readUINT32(const sal_uInt8* buffer, sal_uInt32& v)
{
    v = (
            (buffer[0] << 24) |
            (buffer[1] << 16) |
            (buffer[2] << 8)  |
            (buffer[3] << 0)
        );

    return sizeof(sal_uInt32);
}

inline sal_uInt32 writeUtf8(sal_uInt8* buffer, const sal_Char* v)
{
    sal_uInt32 size = strlen(v) + 1;

    memcpy(buffer, v, size);

    return (size);
}

inline sal_uInt32 readUtf8(const sal_uInt8* buffer, sal_Char* v, sal_uInt32 maxSize)
{
    sal_uInt32 size = VOS_MIN(strlen((const sal_Char*) buffer) + 1, maxSize);

    memcpy(v, buffer, size);

    if (size == maxSize) v[size - 1] = '\0';

    return (size);
}


sal_uInt32 writeFloat(sal_uInt8* buffer, float v);
sal_uInt32 readFloat(const sal_uInt8* buffer, float& v);
sal_uInt32 writeDouble(sal_uInt8* buffer, double v);
sal_uInt32 readDouble(const sal_uInt8* buffer, double& v);
sal_uInt32 writeString(sal_uInt8* buffer, const sal_Unicode* v);
sal_uInt32 readString(const sal_uInt8* buffer, sal_Unicode* v, sal_uInt32 maxSize);

sal_uInt32 UINT16StringLen(const sal_uInt8* wstring);

#endif





