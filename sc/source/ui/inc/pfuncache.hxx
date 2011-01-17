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

#ifndef SC_PFUNCACHE_HXX
#define SC_PFUNCACHE_HXX

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

    sal_Bool    operator==(const ScPrintSelectionStatus& rOther) const
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
    long                    nPages[MAXTABCOUNT];
    long                    nFirstAttr[MAXTABCOUNT];
    std::vector<ScPrintPageLocation> aLocations;
    bool                    bLocInitialized;

public:
            ScPrintFuncCache( ScDocShell* pD, const ScMarkData& rMark,
                                const ScPrintSelectionStatus& rStatus );
            ~ScPrintFuncCache();

    sal_Bool    IsSameSelection( const ScPrintSelectionStatus& rStatus ) const;

    void    InitLocations( const ScMarkData& rMark, OutputDevice* pDev );
    bool    FindLocation( const ScAddress& rCell, ScPrintPageLocation& rLocation ) const;

    long    GetPageCount() const                { return nTotalPages; }
    long    GetFirstAttr( SCTAB nTab ) const    { return nFirstAttr[nTab]; }
    SCTAB   GetTabForPage( long nPage ) const;
    long    GetTabStart( SCTAB nTab ) const;
    long    GetDisplayStart( SCTAB nTab ) const;
};

#endif

