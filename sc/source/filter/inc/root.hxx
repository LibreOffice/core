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

#ifndef SC_ROOT_HXX
#define SC_ROOT_HXX

#include <tools/solar.h>
#include "global.hxx"
#include "address.hxx"
#include "flttypes.hxx"
#include "filter.hxx"
#include "excdefs.hxx"

class ScRangeName;

class RangeNameBufferWK3;
class SharedFormulaBuffer;
class ExtNameBuff;
class ExtSheetBuffer;
class ExcelToSc;

class XclImpColRowSettings;
class XclImpAutoFilterBuffer;
class _ScRangeListTabs;

class XclExpChTrTabId;
class XclExpUserBViewList;

class XclImpRoot;
class XclExpRoot;

// ---------------------------------------------------------- Excel Imp~/Exp~ -

struct RootData     // -> Inkarnation jeweils im ImportExcel-Objekt!
{
    BiffTyp             eDateiTyp;              // feine Differenzierung
    ExtSheetBuffer*     pExtSheetBuff;
    SharedFormulaBuffer*      pShrfmlaBuff;
    ExtNameBuff*        pExtNameBuff;
    ExcelToSc*          pFmlaConverter;
    XclImpColRowSettings* pColRowBuff;        // Col/Row-Einstellungen 1 Tabelle

    // Biff8
    XclImpAutoFilterBuffer* pAutoFilterBuffer;      // ranges for autofilter and advanced filter
    _ScRangeListTabs*       pPrintRanges;
    _ScRangeListTabs*       pPrintTitles;

    // Erweiterungen fuer Export
    XclExpChTrTabId*        pTabId;             // pointer to rec list, do not destroy
    XclExpUserBViewList*    pUserBViewList;     // pointer to rec list, do not destroy

    XclImpRoot*         pIR;
    XclExpRoot*         pER;

                        RootData( void );       // -> exctools.cxx
                        ~RootData();            // -> exctools.cxx
};

class ExcRoot
{
protected:
    RootData*       pExcRoot;
    inline          ExcRoot( RootData* pNexExcRoot ) : pExcRoot( pNexExcRoot ) {}
    inline          ExcRoot( const ExcRoot& rCopy ) : pExcRoot( rCopy.pExcRoot ) {}
};

// ---------------------------------------------------------- Lotus Imp~/Exp~ -

class LotusRangeList;
class LotusFontBuffer;
class LotAttrTable;


struct LOTUS_ROOT
{
    ScDocument*         pDoc;
    LotusRangeList*     pRangeNames;
    ScRangeName*        pScRangeName;
    CharSet             eCharsetQ;
    Lotus123Typ         eFirstType;
    Lotus123Typ         eActType;
    ScRange             aActRange;
    RangeNameBufferWK3* pRngNmBffWK3;
    LotusFontBuffer*    pFontBuff;
    LotAttrTable*       pAttrTable;

                        LOTUS_ROOT( ScDocument* pDocP, CharSet eQ );
                        ~LOTUS_ROOT();
};

extern LOTUS_ROOT*      pLotusRoot; // -> Inkarn. in filter.cxx

// ----------------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
