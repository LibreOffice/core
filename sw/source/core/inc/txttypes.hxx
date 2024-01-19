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

/// Portion groups
constexpr sal_uInt16 PORGRP_TXT       = 0x8000;
constexpr sal_uInt16 PORGRP_EXP       = 0x4000;
constexpr sal_uInt16 PORGRP_FLD       = 0x2000;
constexpr sal_uInt16 PORGRP_HYPH      = 0x1000;
constexpr sal_uInt16 PORGRP_NUMBER    = 0x0800;
constexpr sal_uInt16 PORGRP_GLUE      = 0x0400;
constexpr sal_uInt16 PORGRP_FIX       = 0x0200;
constexpr sal_uInt16 PORGRP_TAB       = 0x0100;
// Small special groups
constexpr sal_uInt16 PORGRP_FIXMARG   = 0x0080;
constexpr sal_uInt16 PORGRP_TABNOTLFT = 0x0040;
constexpr sal_uInt16 PORGRP_TOXREF    = 0x0020;

// five least-significant bits are just a unique index in the group

enum class PortionType
{
    NONE        = 0x00,
    FlyCnt      = 0x01,

    Hole        = 0x10,
    TempEnd     = 0x11,
    Break       = 0x12,
    Kern        = 0x13,
    Arrow       = 0x14,
    Multi       = 0x15,
    HiddenText  = 0x16,
    ControlChar = 0x17,
    Bookmark    = PORGRP_TOXREF | 0x18, // Does it need PORGRP_TOXREF bit?

    Text              = PORGRP_TXT | 0x00,
    Lay               = PORGRP_TXT | 0x01,
    Para              = PORGRP_TXT | 0x02,
    Hanging           = PORGRP_TXT | 0x04,
    InputField        = PORGRP_TXT | 0x05,
    FieldMark         = PORGRP_TXT | 0x06,
    FieldFormCheckbox = PORGRP_TXT | 0x07,

    Drop              = PORGRP_TXT | 0x10,
    Tox               = PORGRP_TXT | PORGRP_TOXREF | 0x11,
    IsoTox            = PORGRP_TXT | PORGRP_TOXREF | 0x12,
    Ref               = PORGRP_TXT | PORGRP_TOXREF | 0x13,
    IsoRef            = PORGRP_TXT | PORGRP_TOXREF | 0x14,
    Meta              = PORGRP_TXT | PORGRP_TOXREF | 0x15, // Does it need PORGRP_TOXREF bit?
    ContentControl    = PORGRP_TXT | PORGRP_TOXREF | 0x16, // Does it need PORGRP_TOXREF bit?

    Expand      = PORGRP_TXT | PORGRP_EXP | 0x10,
    Blank       = PORGRP_TXT | PORGRP_EXP | 0x11,
    PostIts     = PORGRP_TXT | PORGRP_EXP | 0x12,

    Hyphen         = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x10,
    HyphenStr      = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x11,
    SoftHyphen     = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x12,
    SoftHyphenStr  = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x13,
    SoftHyphenComp = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x14,

    Field       = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x10,
    Hidden      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x11,
    QuoVadis    = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x12,
    ErgoSum     = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x13,
    Combined    = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x14,
    Footnote    = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x15,

    FootnoteNum = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x10,
    Number      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x11,
    Bullet      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x12,
    GrfNum      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x13,

    Glue        = PORGRP_GLUE | 0x10,

    Margin      = PORGRP_GLUE | PORGRP_FIXMARG | 0x10,

    Fix         = PORGRP_GLUE | PORGRP_FIX | PORGRP_FIXMARG | 0x10,
    Fly         = PORGRP_GLUE | PORGRP_FIX | PORGRP_FIXMARG | 0x11,

    // Tabulator, not table
    Tab         = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x00,

    TabRight    = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x10,
    TabCenter   = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x11,
    TabDecimal  = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x12,

    TabLeft     = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | 0x00,
};

namespace sw
{
const char* PortionTypeToString(PortionType nType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
