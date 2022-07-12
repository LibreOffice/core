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

#include "address.hxx"
#include <svl/hint.hxx>

class ScHint final : public SfxHint
{
    ScAddress   aAddress;
    SCROW       nRowCount;

public:
    ScHint( SfxHintId n, const ScAddress& a, SCROW rowCount = 1 ) : SfxHint(n), aAddress(a), nRowCount( rowCount ) {}
    const ScAddress& GetStartAddress() const { return aAddress; }
    SCROW GetRowCount() const { return nRowCount; }
    ScRange GetRange() const
        { return ScRange(aAddress, ScAddress(aAddress.Col(), aAddress.Row() + nRowCount - 1, aAddress.Tab())); }
    void SetAddressTab(SCTAB nTab) { aAddress.SetTab(nTab); }
    void SetAddressCol(SCCOL nCol) { aAddress.SetCol(nCol); }
    void SetAddressRow(SCROW nRow) { aAddress.SetRow(nRow); }
};

class ScAreaChangedHint final : public SfxHint
{
private:
    ScRange aNewRange;
public:
    ScAreaChangedHint(const ScRange& rRange)
        : SfxHint(SfxHintId::ScAreaChanged), aNewRange(rRange) {}
    const ScRange&  GetRange() const { return aNewRange; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
