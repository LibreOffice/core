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
    const sal_uInt16 nBOF;
    WKTYP            eTyp;   // Typ der gerade in bearbeitung befindlichen Datei
    bool             bEOF;           // zeigt Ende der Datei
    rtl_TextEncoding eCharVon;
    ScDocument*      pDoc;           // Aufhaenger zum Dokumentzugriff
    static OPCODE_FKT pOpFkt[ FKT_LIMIT ];
    static OPCODE_FKT pOpFkt123[ FKT_LIMIT123 ]; // -> optab.cxx, Table of possible Opcodes
    LOTUS_ROOT*      pLotusRoot;
    std::map<sal_uInt16, ScPatternAttr> aLotusPatternPool;

    sal_uInt8        nDefaultFormat; // -> op.cpp, Standard-Zellenformat

    SvxHorJustifyItem *pAttrRight, *pAttrLeft, *pAttrCenter, *pAttrRepeat, *pAttrStandard;
    ScProtectionAttr* pAttrUnprot;
    SfxUInt32Item**   pAttrValForms;

    FormCache*       pValueFormCache; // -> in memory.cxx initialisiert

    LotusContext()
        : nBOF(0x0000)
        , eTyp(eWK_UNKNOWN)
        , bEOF(false)
        , eCharVon(RTL_TEXTENCODING_DONTKNOW)
        , pDoc(0)
        , pLotusRoot(NULL)

        , nDefaultFormat(0)
        , pAttrRight(NULL)
        , pAttrLeft(NULL)
        , pAttrCenter(NULL)
        , pAttrRepeat(NULL)
        , pAttrStandard(NULL)
        , pAttrUnprot(NULL)
        , pAttrValForms(NULL)
        , pValueFormCache(NULL)
    {
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
