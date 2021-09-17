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


#include <sal/types.h>
#include <osl/endian.h>
#include <rtl/ustring.h>

#include <registry/refltype.hxx>

#include "reflcnst.hxx"


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
