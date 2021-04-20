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

#include <buffer.hxx>

namespace
{
const sal_uInt32 UP_LIMIT=0xFFFFFF00;

template <class I, typename T> void write(I it, T n)
{
    *it = static_cast<sal_uInt8>(n & 0xFF);
    // coverity[stray_semicolon : FALSE] - coverity parse error
    if constexpr (sizeof(n) > 1)
    {
        for (std::size_t i = 1; i < sizeof(n); ++i)
        {
            n >>= 8;
            *++it = static_cast<sal_uInt8>(n & 0xFF);
        }
    }
}
}

template <typename T> void SbiBuffer::append(T n)
{
    if (m_aErrCode)
        return;
    if ((m_aBuf.size() + sizeof(n)) > UP_LIMIT)
    {
        m_aErrCode = ERRCODE_BASIC_PROG_TOO_LARGE;
        m_aBuf.clear();
        return;
    }
    m_aBuf.reserve(m_aBuf.size() + sizeof(n));
    write(std::back_inserter(m_aBuf), n);
}

void SbiBuffer::operator+=(sal_Int8 n) { append(n); }
void SbiBuffer::operator+=(sal_Int16 n) { append(n); }
void SbiBuffer::operator+=(sal_uInt8 n) { append(n); }
void SbiBuffer::operator+=(sal_uInt16 n) { append(n); }
void SbiBuffer::operator+=(sal_uInt32 n) { append(n); }
void SbiBuffer::operator+=(sal_Int32 n) { append(n); }

// Patch of a Location

void SbiBuffer::Patch( sal_uInt32 off, sal_uInt32 val )
{
    if (m_aErrCode)
        return;
    if ((off + sizeof(sal_uInt32)) <= GetSize())
        write(m_aBuf.begin() + off, val);
}

// Forward References upon label and procedures
// establish a linkage. The beginning of the linkage is at the passed parameter,
// the end of the linkage is 0.

void SbiBuffer::Chain( sal_uInt32 off )
{
    if (m_aErrCode)
        return;
    for (sal_uInt32 i = off; i;)
    {
        if ((i + sizeof(sal_uInt32)) > GetSize())
        {
            m_aErrCode = ERRCODE_BASIC_INTERNAL_ERROR;
            m_sErrMsg = "BACKCHAIN";
            break;
        }
        auto ip = m_aBuf.begin() + i;
        i = ip[0] | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24);
        write(ip, GetSize());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
