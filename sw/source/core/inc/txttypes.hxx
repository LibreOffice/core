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
constexpr sal_uInt16 PORGRP_FIXMARG   = 0x0040;
//#define PORGRP_?  0x0020
constexpr sal_uInt16 PORGRP_TABNOTLFT = 0x0010;
constexpr sal_uInt16 PORGRP_TOXREF    = 0x0008;

enum class PortionType
{
    NONE        = 0x0000,
    FlyCnt      = 0x0001,

    Hole        = 0x0080,
    TempEnd     = 0x0081,
    Break       = 0x0082,
    Kern        = 0x0083,
    Arrow       = 0x0084,
    Multi       = 0x0085,
    HiddenText  = 0x0086,
    ControlChar = 0x0087,
    Bookmark    = 0x0088,

    Text              = PORGRP_TXT | 0x00,
    Lay               = PORGRP_TXT | 0x01,
    Para              = PORGRP_TXT | 0x02,
    Hanging           = PORGRP_TXT | 0x04,
    InputField        = PORGRP_TXT | 0x05,
    FieldMark         = PORGRP_TXT | 0x06,
    FieldFormCheckbox = PORGRP_TXT | 0x07,

    Drop              = PORGRP_TXT | 0x80,
    Tox               = PORGRP_TXT | PORGRP_TOXREF | 0x81,
    IsoTox            = PORGRP_TXT | PORGRP_TOXREF | 0x82,
    Ref               = PORGRP_TXT | PORGRP_TOXREF | 0x83,
    IsoRef            = PORGRP_TXT | PORGRP_TOXREF | 0x84,
    Meta              = PORGRP_TXT | PORGRP_TOXREF | 0x85,
    ContentControl    = PORGRP_TXT | PORGRP_TOXREF | 0x86,

    Expand      = PORGRP_TXT | PORGRP_EXP | 0x80,
    Blank       = PORGRP_TXT | PORGRP_EXP | 0x81,
    PostIts     = PORGRP_TXT | PORGRP_EXP | 0x82,

    Hyphen         = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x80,
    HyphenStr      = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x81,
    SoftHyphen     = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x82,
    SoftHyphenStr  = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x83,
    SoftHyphenComp = PORGRP_TXT | PORGRP_EXP | PORGRP_HYPH | 0x84,

    Field       = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x80,
    Hidden      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x81,
    QuoVadis    = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x82,
    ErgoSum     = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x83,
    Combined    = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x84,
    Footnote    = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | 0x85,

    FootnoteNum = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x80,
    Number      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x81,
    Bullet      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x82,
    GrfNum      = PORGRP_TXT | PORGRP_EXP | PORGRP_FLD | PORGRP_NUMBER | 0x83,

    Glue        = PORGRP_GLUE | 0x80,

    Margin      = PORGRP_GLUE | PORGRP_FIXMARG | 0x80,

    Fix         = PORGRP_GLUE | PORGRP_FIX | PORGRP_FIXMARG | 0x80,
    Fly         = PORGRP_GLUE | PORGRP_FIX | PORGRP_FIXMARG | 0x81,

    // Tabulator, not table
    Tab         = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x00,

    TabRight    = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x80,
    TabCenter   = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x81,
    TabDecimal  = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | PORGRP_TABNOTLFT | 0x82,

    TabLeft     = PORGRP_GLUE | PORGRP_FIX | PORGRP_TAB | PORGRP_FIXMARG | 0x00,
};

namespace sw
{
const char* PortionTypeToString(PortionType nType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
