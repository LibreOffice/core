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

#ifndef INCLUDED_SC_SOURCE_FILTER_LOTUS_FILTER_HXX
#define INCLUDED_SC_SOURCE_FILTER_LOTUS_FILTER_HXX

#include <map>
#include "decl.h"
#include "op.h"
#include "optab.h"
#include "root.hxx"

class SvxHorJustifyItem;
class ScProtectionAttr;
class SfxUInt32Item;
class FormCache;

struct LotusContext
{
    static const sal_uInt16 nBOF = 0x0000;
    WKTYP            eTyp;          // type of file being processed
    bool             bEOF;          // shows end of file
    rtl_TextEncoding eCharVon;
    ScDocument*      pDoc;          // pointer to access document
    static OPCODE_FKT pOpFkt[ FKT_LIMIT ];
    static OPCODE_FKT pOpFkt123[ FKT_LIMIT123 ]; // -> optab.cxx; table of possible Opcodes
    LOTUS_ROOT*      pLotusRoot;
    std::map<sal_uInt16, ScPatternAttr> aLotusPatternPool;

    SvxHorJustifyItem *pAttrRight, *pAttrLeft, *pAttrCenter, *pAttrRepeat, *pAttrStandard;

    FormCache*       pValueFormCache; // -> initialized in memory.cxx

    LotusContext()
        : eTyp(eWK_UNKNOWN)
        , bEOF(false)
        , eCharVon(RTL_TEXTENCODING_DONTKNOW)
        , pDoc(nullptr)
        , pLotusRoot(nullptr)

        , pAttrRight(nullptr)
        , pAttrLeft(nullptr)
        , pAttrCenter(nullptr)
        , pAttrRepeat(nullptr)
        , pAttrStandard(nullptr)
        , pValueFormCache(nullptr)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
