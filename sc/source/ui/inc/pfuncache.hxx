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

#include <vector>
#include <tools/gen.hxx>
#include <rangelst.hxx>
#include <printopt.hxx>

class ScDocShell;
class ScMarkData;
class OutputDevice;

/** Possible types of selection for print functions */

enum class ScPrintSelectionMode
{
    Invalid,
    Document,
    Cursor,
    Range,
    RangeExclusivelyOleAndDrawObjects
};

/** Stores the selection in the ScPrintFuncCache so it is only used
    for the same selection again. */

class ScPrintSelectionStatus
{
    ScPrintSelectionMode    eMode;
    ScRangeList             aRanges;
    ScPrintOptions          aOptions;

public:
            ScPrintSelectionStatus() : eMode(ScPrintSelectionMode::Invalid) {}

    void    SetMode(ScPrintSelectionMode eNew)  { eMode = eNew; }
    void    SetRanges(const ScRangeList& rNew)  { aRanges = rNew; }
    void    SetOptions(const ScPrintOptions& rNew) { aOptions = rNew; }

    bool    operator==(const ScPrintSelectionStatus& rOther) const
            { return eMode == rOther.eMode && aRanges == rOther.aRanges && aOptions == rOther.aOptions; }

    ScPrintSelectionMode GetMode() const { return eMode; }
    const ScPrintOptions& GetOptions() const { return aOptions; }
};

/** The range that is printed on a page (excluding repeated columns/rows),
    and its position on the page, used to find hyperlink targets. */

struct ScPrintPageLocation
{
    tools::Long        nPage;
    ScRange     aCellRange;
    tools::Rectangle   aRectangle;     // pixels

    ScPrintPageLocation() :
        nPage(-1) {}            // default: invalid

    ScPrintPageLocation( tools::Long nP, const ScRange& rRange, const tools::Rectangle& rRect ) :
        nPage(nP), aCellRange(rRange), aRectangle(rRect) {}
};

/** Stores the data for printing that is needed from several sheets,
    so it doesn't have to be calculated for rendering each page. */

class ScPrintFuncCache
{
    ScPrintSelectionStatus  aSelection;
    ScDocShell*             pDocSh;
    tools::Long                    nTotalPages;
    std::vector<tools::Long>       nPages;
    std::vector<tools::Long>       nFirstAttr;
    std::vector<ScPrintPageLocation> aLocations;
    bool                    bLocInitialized;
    Size                    aPrintPageSize;          // print page size in Print dialog
    bool                    bPrintPageLandscape;     // print page orientation in Print dialog
    bool                    bUsePrintDialogSetting;  // use Print dialog setting

public:
            ScPrintFuncCache(ScDocShell* pD, const ScMarkData& rMark, ScPrintSelectionStatus aStatus,
                             Size aPageSize = {}, bool bLandscape = false, bool bUse = false);
            ~ScPrintFuncCache();

    bool    IsSameSelection( const ScPrintSelectionStatus& rStatus ) const;

    void    InitLocations( const ScMarkData& rMark, OutputDevice* pDev );
    bool    FindLocation( const ScAddress& rCell, ScPrintPageLocation& rLocation ) const;

    tools::Long    GetPageCount() const                { return nTotalPages; }
    tools::Long    GetFirstAttr( SCTAB nTab ) const    { return nFirstAttr[nTab]; }
    SCTAB   GetTabForPage( tools::Long nPage ) const;
    tools::Long    GetTabStart( SCTAB nTab ) const;
    tools::Long    GetDisplayStart( SCTAB nTab ) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
