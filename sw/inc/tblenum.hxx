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

#include <o3tl/typed_flags_set.hxx>

// For changing table columns/rows widths/heights.
enum class TableChgWidthHeightType : sal_uInt16
{
    ColLeft      = 0,
    ColRight     = 1,
    RowBottom    = 3,
    CellLeft     = 4,
    CellRight    = 5,
    CellTop      = 6,
    CellBottom   = 7,
    InvalidPos   = 0x0f,

    // The following can be "or"ed in.
    BiggerMode       = 0x8000, // Box becomes larger -> else smaller.
};
namespace o3tl {
    template<> struct typed_flags<TableChgWidthHeightType> : is_typed_flags<TableChgWidthHeightType, 0x800f> {};
}
constexpr TableChgWidthHeightType extractPosition(TableChgWidthHeightType e) {
    return static_cast<TableChgWidthHeightType>(static_cast<sal_uInt16>(e) & 0xf);
}

enum class TableChgMode
{
    FixedWidthChangeAbs,          // Table fixed width, change neighbour.
    FixedWidthChangeProp,         // Table fixed width, change all neighbours.
    VarWidthChangeAbs           // Table variable, change all neighbours.
};

enum class SplitTable_HeadlineOption
{
    NONE = 0,          // Leave everything in place.
    BorderCopy,        // Copy border of the previous line.
    ContentCopy,         // Copy 1st line with all contents.
    BoxAttrCopy,       // Copy box attributes of 1st line.
    BoxAttrAllCopy     // Copy box attributes and paragraph styles of 1st line.
};

enum class TableMergeErr
{
    Ok,
    NoSelection,
    TooComplex
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
