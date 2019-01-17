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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_TXTTYPES_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_TXTTYPES_HXX

#include <swtypes.hxx>

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

    Text        = 0x8000,
    Lay         = 0x8001,
    Para        = 0x8002,
    Hanging     = 0x8004,
    InputField  = 0x8005,

    Drop        = 0x8080,
    Tox         = 0x8089,
    IsoTox      = 0x808a,
    Ref         = 0x808b,
    IsoRef      = 0x808c,
    Meta        = 0x808d,

    Expand      = 0xc080,
    Blank       = 0xc081,
    PostIts     = 0xc082,

    Hyphen         = 0xd080,
    HyphenStr      = 0xd081,
    SoftHyphen     = 0xd082,
    SoftHyphenStr  = 0xd083,
    SoftHyphenComp = 0xd084,

    Field       = 0xe080,
    Hidden      = 0xe081,
    QuoVadis    = 0xe082,
    ErgoSum     = 0xe083,
    Combined    = 0xe084,
    Footnote    = 0xe085,

    FootnoteNum = 0xe880,
    Number      = 0xe881,
    Bullet      = 0xe882,
    GrfNum      = 0xe883,

    Glue        = 0x0480,

    Margin      = 0x04c0,

    Fix         = 0x06c0,
    Fly         = 0x06c1,

    Table       = 0x0750,

    TabRight    = 0x07d0,
    TabCenter   = 0x07d1,
    TabDecimal  = 0x07d2,

    TabLeft     = 0x0740,
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_TXTTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
