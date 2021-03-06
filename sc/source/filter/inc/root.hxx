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

#include "flttypes.hxx"
#include <memory>

class ScRangeName;

class RangeNameBufferWK3;
class SharedFormulaBuffer;
class ExtNameBuff;
class ExtSheetBuffer;
class ExcelToSc;

class XclImpColRowSettings;
class XclImpAutoFilterBuffer;
class ScRangeListTabs;

class XclExpChTrTabId;
class XclExpUserBViewList;

class XclImpRoot;
class XclExpRoot;

// Excel Imp~/Exp~ -

struct RootData     // -> incarnation in each case in the ImportExcel object!
{
    BiffTyp             eDateiTyp;           // fine differentiation
    std::unique_ptr<ExtSheetBuffer>         pExtSheetBuff;
    std::unique_ptr<SharedFormulaBuffer>    pShrfmlaBuff;
    std::unique_ptr<ExtNameBuff>            pExtNameBuff;
    ExcelToSc*          pFmlaConverter;
    XclImpColRowSettings* pColRowBuff;        // col/row settings 1 table

    // Biff8
    std::unique_ptr<XclImpAutoFilterBuffer> pAutoFilterBuffer;      // ranges for autofilter and advanced filter
    std::unique_ptr<ScRangeListTabs>        pPrintRanges;
    std::unique_ptr<ScRangeListTabs>        pPrintTitles;

    // extensions for export
    XclExpChTrTabId*        pTabId;             // pointer to rec list, do not destroy
    XclExpUserBViewList*    pUserBViewList;     // pointer to rec list, do not destroy

    XclImpRoot*         pIR;
    XclExpRoot*         pER;

                        RootData();       // -> exctools.cxx
                        ~RootData();            // -> exctools.cxx
};

class ExcRoot
{
protected:
    RootData*       pExcRoot;
    ExcRoot( RootData* pNexExcRoot ) : pExcRoot( pNexExcRoot ) {}
    ExcRoot( const ExcRoot& rCopy ) : pExcRoot( rCopy.pExcRoot ) {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
