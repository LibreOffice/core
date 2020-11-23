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
#include <memory>

class SbiParser;

class SbiBuffer {
    SbiParser* pParser;             // for error messages
    std::unique_ptr<char[]>  pBuf;
    char* pCur = nullptr;
    sal_uInt32 nOff = 0;
    sal_uInt32 nSize = 0;
    short   nInc;
    bool    Check( sal_Int32 );
    template <typename T> static char* write(char* p, T n)
    {
        *p++ = static_cast<char>(n & 0xFF);
        if constexpr (sizeof(n) > 1)
        {
            for (std::size_t i = 1; i < sizeof(n); ++i)
            {
                n >>= 8;
                *p++ = static_cast<char>(n & 0xFF);
            }
        }
        return p;
    }
    template <typename T> void append(T n)
    {
        if (!Check(sizeof(n)))
            return;
        pCur = write(pCur, n);
        nOff += sizeof(n);
    }

public:
    SbiBuffer( SbiParser*, short ); // increment
    ~SbiBuffer() = default;
    void Patch( sal_uInt32, sal_uInt32 );
    void Chain( sal_uInt32 );
    void operator+=(sal_Int8 n) { append(n); } // save character
    void operator+=(sal_Int16 n) { append(n); } // save integer
    void operator+=(sal_uInt8 n) { append(n); } // save character
    void operator+=(sal_uInt16 n) { append(n); } // save integer
    void operator+=(sal_uInt32 n) { append(n); } // save integer
    void operator+=(sal_Int32 n) { append(n); } // save integer
    char*  GetBuffer();             // give out buffer (delete yourself!)
    sal_uInt32 GetSize() const { return nOff; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
