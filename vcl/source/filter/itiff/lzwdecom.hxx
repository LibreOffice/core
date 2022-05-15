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

#pragma once

#include <sal/types.h>
#include <array>

constexpr sal_Int16 MAX_TABLE_SIZE = 4096;

struct LZWTableEntry
{
    sal_uInt16 nPrevCode;
    sal_uInt16 nDataCount;
    sal_uInt8 nData;
};

class SvStream;

class LZWDecompressor
{
public:
    LZWDecompressor();

    void StartDecompression(SvStream& rIStream);

    // Returns the number of written bytes. If < nMaxCount there is
    // no more data to be unpacked or an error occurred.
    sal_uInt64 Decompress(sal_uInt8* pTarget, sal_uInt32 nMaxCount);

private:
    sal_uInt16 GetNextCode();
    void AddToTable(sal_uInt16 nPrevCode, sal_uInt16 nCodeFirstData);
    void DecompressSome();

    SvStream* m_pIStream;

    std::array<LZWTableEntry, MAX_TABLE_SIZE> m_aTable;
    sal_uInt16 m_nTableCurrentId;

    bool m_bEOIFound, m_bInvert;

    sal_uInt16 m_nOldCode;

    std::array<sal_uInt8, MAX_TABLE_SIZE> m_pOutBuf;
    sal_uInt8* m_pOutBufData;
    sal_uInt16 m_nOutBufDataLen;

    sal_uInt8 m_nInputBitsBuf;
    sal_uInt16 m_nInputBitsBufSize;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
