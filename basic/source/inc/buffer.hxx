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

#include <basic/sberrors.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <vector>

// Stores all numbers big endian

class SbiBuffer {
    std::vector<sal_uInt8> m_aBuf;
    ErrCode m_aErrCode;
    OUString m_sErrMsg;

    template <typename T> void append(T n);

public:
    SbiBuffer() { m_aBuf.reserve(1024); }
    void Patch( sal_uInt32, sal_uInt32 );
    void Chain( sal_uInt32 );
    void operator += (sal_Int8);        // save character
    void operator += (sal_Int16);       // save integer
    void operator += (sal_uInt8);       // save character
    void operator += (sal_uInt16);      // save integer
    void operator += (sal_uInt32);      // save integer
    void operator += (sal_Int32);       // save integer
    std::vector<sal_uInt8>&& GetBuffer() { return std::move(m_aBuf); } // pass ownership
    sal_uInt32 GetSize() const { return m_aBuf.size(); }
    const ErrCode & GetErrCode() const { return m_aErrCode; }
    const OUString & GetErrMessage() const { return m_sErrMsg; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
