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
    std::vector<sal_uInt8> aBuf;
    template <typename T> void append(T n);

public:
    SbiBuffer(SbiParser* p) : pParser(p) { aBuf.reserve(1024); }
    void Patch( sal_uInt32, sal_uInt32 );
    void Chain( sal_uInt32 );
    void operator += (sal_Int8);        // save character
    void operator += (sal_Int16);       // save integer
    void operator += (sal_uInt8);       // save character
    void operator += (sal_uInt16);      // save integer
    void operator += (sal_uInt32);      // save integer
    void operator += (sal_Int32);       // save integer
    std::vector<sal_uInt8>&& GetBuffer() { return std::move(aBuf); } // pass ownership
    sal_uInt32 GetSize() const { return aBuf.size(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
