/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pfuncache.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:43:32 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_PFUNCACHE_HXX
#define SC_PFUNCACHE_HXX

#include <vector>

#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

class ScDocShell;
class ScMarkData;


/** Possible types of selection for print functions */

enum ScPrintSelectionMode
{
    SC_PRINTSEL_INVALID,
    SC_PRINTSEL_DOCUMENT,
    SC_PRINTSEL_CURSOR,
    SC_PRINTSEL_RANGE
};


/** Stores the selection in the ScPrintFuncCache so it is only used
    for the same selection again. */

class ScPrintSelectionStatus
{
    ScPrintSelectionMode    eMode;
    ScRangeList             aRanges;

public:
            ScPrintSelectionStatus() : eMode(SC_PRINTSEL_INVALID) {}
            ~ScPrintSelectionStatus() {}

    void    SetMode(ScPrintSelectionMode eNew)  { eMode = eNew; }
    void    SetRanges(const ScRangeList& rNew)  { aRanges = rNew; }

    BOOL    operator==(const ScPrintSelectionStatus& rOther) const
            { return eMode == rOther.eMode && aRanges == rOther.aRanges; }
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

    BOOL    IsSameSelection( const ScPrintSelectionStatus& rStatus ) const;

    void    InitLocations( const ScMarkData& rMark, OutputDevice* pDev );
    bool    FindLocation( const ScAddress& rCell, ScPrintPageLocation& rLocation ) const;

    long    GetPageCount() const                { return nTotalPages; }
    long    GetFirstAttr( SCTAB nTab ) const    { return nFirstAttr[nTab]; }
    SCTAB   GetTabForPage( long nPage ) const;
    long    GetTabStart( SCTAB nTab ) const;
    long    GetDisplayStart( SCTAB nTab ) const;
};

#endif

