/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#ifndef INCLUDED_SC_INC_SHEETLIMITS_HXX
#define INCLUDED_SC_INC_SHEETLIMITS_HXX

#include <salhelper/simplereferenceobject.hxx>

// Because some stuff needs this info, and those objects lifetimes sometimes exceeds the lifetime
// of the ScDocument.
struct ScSheetLimits final : public salhelper::SimpleReferenceObject
{
    const SCCOL mnMaxCol; /// Maximum addressable column
    const SCROW mnMaxRow; /// Maximum addressable row

    ScSheetLimits(SCCOL nMaxCol, SCROW nMaxRow)
        : mnMaxCol(nMaxCol)
        , mnMaxRow(nMaxRow){}

              [[nodiscard]] bool ValidCol(SCCOL nCol) const
    {
        return ::ValidCol(nCol, mnMaxCol);
    }
    [[nodiscard]] bool ValidRow(SCROW nRow) const { return ::ValidRow(nRow, mnMaxRow); }
    [[nodiscard]] bool ValidColRow(SCCOL nCol, SCROW nRow) const
    {
        return ::ValidColRow(nCol, nRow, mnMaxCol, mnMaxRow);
    }
    [[nodiscard]] bool ValidColRowTab(SCCOL nCol, SCROW nRow, SCTAB nTab) const
    {
        return ::ValidColRowTab(nCol, nRow, nTab, mnMaxCol, mnMaxRow);
    }
    [[nodiscard]] bool ValidRange(const ScRange& rRange) const
    {
        return ::ValidRange(rRange, mnMaxCol, mnMaxRow);
    }
    [[nodiscard]] bool ValidAddress(const ScAddress& rAddress) const
    {
        return ::ValidAddress(rAddress, mnMaxCol, mnMaxRow);
    }
    [[nodiscard]] SCCOL SanitizeCol(SCCOL nCol) const { return ::SanitizeCol(nCol, mnMaxCol); }
    [[nodiscard]] SCROW SanitizeRow(SCROW nRow) const { return ::SanitizeRow(nRow, mnMaxRow); }

    // equivalent of MAXROWCOUNT in address.hxx
    SCROW GetMaxRowCount() const { return mnMaxRow + 1; }
    // equivalent of MAXCOLCOUNT in address.hxx
    SCCOL GetMaxColCount() const { return mnMaxCol + 1; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
