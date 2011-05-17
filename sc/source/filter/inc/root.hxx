/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_ROOT_HXX
#define SC_ROOT_HXX

#include <tools/solar.h>
#include "global.hxx"
#include "address.hxx"
#include "flttypes.hxx"
#include "filter.hxx"
#include "excdefs.hxx"

class ScRangeName;

class NameBuffer;
class RangeNameBufferWK3;
class ShrfmlaBuffer;
class ExtNameBuff;
class ExtSheetBuffer;
class ExcelToSc;

class XclImpColRowSettings;
class XclImpAutoFilterBuffer;
class XclImpPivotCacheList;
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
    ShrfmlaBuffer*      pShrfmlaBuff;
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
};

extern LOTUS_ROOT*      pLotusRoot; // -> Inkarn. in filter.cxx

// ----------------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
