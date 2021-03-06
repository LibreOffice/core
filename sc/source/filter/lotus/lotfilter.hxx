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

#include <map>
#include <decl.h>
#include <optab.h>
#include <patattr.hxx>
#include <lotattr.hxx>
#include <lotrange.hxx>
#include <lotfntbf.hxx>
#include <flttypes.hxx>
#include <namebuff.hxx>

class SvxHorJustifyItem;
class FormCache;

struct LotusContext
{
    static const sal_uInt16 nBOF = 0x0000;
    static OPCODE_FKT pOpFkt[ FKT_LIMIT ];
    static OPCODE_FKT pOpFkt123[ FKT_LIMIT123 ]; // -> optab.cxx; table of possible Opcodes

    WKTYP            eTyp;          // type of file being processed
    bool             bEOF;          // shows end of file
    rtl_TextEncoding eCharset;
    ScDocument&      rDoc;          // reference to access document
    std::map<sal_uInt16, ScPatternAttr> aLotusPatternPool;

    std::unique_ptr<SvxHorJustifyItem> xAttrRight, xAttrLeft, xAttrCenter, xAttrRepeat, xAttrStandard;

    std::unique_ptr<FormCache> xValueFormCache; // -> initialized in memory.cxx

    LotusRangeList      maRangeNames;
    Lotus123Typ         eFirstType;
    Lotus123Typ         eActType;
    ScRange             aActRange;
    std::unique_ptr<RangeNameBufferWK3> pRngNmBffWK3;
    LotusFontBuffer     maFontBuff;
    LotAttrTable        maAttrTable;

    LotusContext(ScDocument& rDocP, rtl_TextEncoding eQ);
    ~LotusContext();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
