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
#include <vector>

class SbiParser;

// Stores all numbers big endian

class SbiBuffer {
    SbiParser* pParser;             // for error messages
    std::vector<sal_uInt8> aBuf = std::vector<sal_uInt8>(1024);
    template <class I, typename T> static I write(I it, T n)
    {
        *it++ = static_cast<sal_uInt8>(n & 0xFF);
        if constexpr (sizeof(n) > 1)
        {
            for (std::size_t i = 1; i < sizeof(n); ++i)
            {
                n >>= 8;
                *it++ = static_cast<sal_uInt8>(n & 0xFF);
            }
        }
        return it;
    }
    template <typename T> void append(T n)
    {
        aBuf.reserve(aBuf.size() + sizeof(n));
        write(std::back_inserter(aBuf), n);
    }

public:
    SbiBuffer(SbiParser* p) : pParser(p) {}
    void Patch( sal_uInt32, sal_uInt32 );
    void Chain( sal_uInt32 );
    void operator+=(sal_Int8 n) { append(n); } // save character
    void operator+=(sal_Int16 n) { append(n); } // save integer
    void operator+=(sal_uInt8 n) { append(n); } // save character
    void operator+=(sal_uInt16 n) { append(n); } // save integer
    void operator+=(sal_uInt32 n) { append(n); } // save integer
    void operator+=(sal_Int32 n) { append(n); } // save integer
    std::vector<sal_uInt8>&& GetBuffer() { return std::move(aBuf); } // pass ownership
    sal_uInt32 GetSize() const { return aBuf.size(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
