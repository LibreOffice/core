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

#ifndef INCLUDED_SC_SOURCE_UI_INC_PFUNCACHE_HXX
#define INCLUDED_SC_SOURCE_UI_INC_PFUNCACHE_HXX

#include <vector>
#include <tools/gen.hxx>
#include "rangelst.hxx"
#include "printopt.hxx"

class ScDocShell;
class ScMarkData;

/** Possible types of selection for print functions */

enum ScPrintSelectionMode
{
    SC_PRINTSEL_INVALID,
    SC_PRINTSEL_DOCUMENT,
    SC_PRINTSEL_CURSOR,
    SC_PRINTSEL_RANGE,
    SC_PRINTSEL_RANGE_EXCLUSIVELY_OLE_AND_DRAW_OBJECTS
};

/** Stores the selection in the ScPrintFuncCache so it is only used
    for the same selection again. */

class ScPrintSelectionStatus
{
    ScPrintSelectionMode    eMode;
    ScRangeList             aRanges;
    ScPrintOptions          aOptions;

public:
            ScPrintSelectionStatus() : eMode(SC_PRINTSEL_INVALID) {}
            ~ScPrintSelectionStatus() {}

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
    long        nPage;
    ScRange     aCellRange;
    Rectangle   aRectangle;     // pixels

    ScPrintPageLocation() :
        nPage(-1) {}            // default: invalid

    ScPrintPageLocation( long nP, const ScRange& rRange, const Rectangle& rRect ) :
        nPage(nP), aCellRange(rRange), aRectangle(rRect) {}
};

/** Stores the data for printing that is needed from several sheets,
    so it doesn't have to be calculated for rendering each page. */

class ScPrintFuncCache
{
    ScPrintSelectionStatus  aSelection;
    ScDocShell*             pDocSh;
    long                    nTotalPages;
    std::vector<long>       nPages;
    std::vector<long>       nFirstAttr;
    std::vector<ScPrintPageLocation> aLocations;
    bool                    bLocInitialized;

public:
            ScPrintFuncCache( ScDocShell* pD, const ScMarkData& rMark,
                                const ScPrintSelectionStatus& rStatus );
            ~ScPrintFuncCache();

    bool    IsSameSelection( const ScPrintSelectionStatus& rStatus ) const;

    void    InitLocations( const ScMarkData& rMark, OutputDevice* pDev );
    bool    FindLocation( const ScAddress& rCell, ScPrintPageLocation& rLocation ) const;

    long    GetPageCount() const                { return nTotalPages; }
    long    GetFirstAttr( SCTAB nTab ) const    { return nFirstAttr[nTab]; }
    SCTAB   GetTabForPage( long nPage ) const;
    long    GetTabStart( SCTAB nTab ) const;
    long    GetDisplayStart( SCTAB nTab ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
