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

#ifndef INCLUDED_SC_INC_BIGRANGE_HXX
#define INCLUDED_SC_INC_BIGRANGE_HXX

#include "address.hxx"
#include <algorithm>

static const sal_Int32 nInt32Min = 0x80000000;
static const sal_Int32 nInt32Max = 0x7fffffff;

class ScDocument;

class ScBigAddress
{
    sal_Int32   nRow;
    sal_Int32   nCol;
    sal_Int32   nTab;

public:
            ScBigAddress() : nRow(0), nCol(0), nTab(0) {}
            ScBigAddress( sal_Int32 nColP, sal_Int32 nRowP, sal_Int32 nTabP )
                : nRow( nRowP ), nCol( nColP ), nTab( nTabP ) {}
            ScBigAddress( const ScBigAddress& r )
                : nRow( r.nRow ), nCol( r.nCol ), nTab( r.nTab ) {}
            ScBigAddress( ScBigAddress&& ) = default;
            ScBigAddress( const ScAddress& r )
                : nRow( r.Row() ), nCol( r.Col() ), nTab( r.Tab() ) {}

    sal_Int32   Col() const { return nCol; }
    sal_Int32   Row() const { return nRow; }
    sal_Int32   Tab() const { return nTab; }

    void    Set( sal_Int32 nColP, sal_Int32 nRowP, sal_Int32 nTabP )
                { nCol = nColP; nRow = nRowP; nTab = nTabP; }
    void    SetCol( sal_Int32 nColP ) { nCol = nColP; }
    void    SetRow( sal_Int32 nRowP ) { nRow = nRowP; }
    void    SetTab( sal_Int32 nTabP ) { nTab = nTabP; }
    void    IncCol( sal_Int32 n = 1 ) { nCol += n; }
    void    IncRow( sal_Int32 n = 1 ) { nRow += n; }
    void    IncTab( sal_Int32 n = 1 ) { nTab += n; }

    void    GetVars( sal_Int32& nColP, sal_Int32& nRowP, sal_Int32& nTabP ) const
                { nColP = nCol; nRowP = nRow; nTabP = nTab; }

    bool IsValid( const ScDocument* pDoc ) const;
    inline ScAddress    MakeAddress() const;

    ScBigAddress&   operator=( const ScBigAddress& r )
                    { nCol = r.nCol; nRow = r.nRow; nTab = r.nTab; return *this; }
    ScBigAddress&   operator=( ScBigAddress&& ) = default;
    ScBigAddress&   operator=( const ScAddress& r )
                    { nCol = r.Col(); nRow = r.Row(); nTab = r.Tab(); return *this; }
    bool            operator==( const ScBigAddress& r ) const
                    { return nCol == r.nCol && nRow == r.nRow && nTab == r.nTab; }
    bool            operator!=( const ScBigAddress& r ) const
                    { return !operator==( r ); }
};

inline ScAddress ScBigAddress::MakeAddress() const
{
    SCCOL nColA;
    SCROW nRowA;
    SCTAB nTabA;

    if ( nCol < 0 )
        nColA = 0;
    else if ( nCol > MAXCOL )
        nColA = MAXCOL;
    else
        nColA = static_cast<SCCOL>(nCol);

    if ( nRow < 0 )
        nRowA = 0;
    else if ( nRow > MAXROW )
        nRowA = MAXROW;
    else
        nRowA = static_cast<SCROW>(nRow);

    if ( nTab < 0 )
        nTabA = 0;
    else if ( nTab > MAXTAB )
        nTabA = MAXTAB;
    else
        nTabA = static_cast<SCTAB>(nTab);

    return ScAddress( nColA, nRowA, nTabA );
}

class ScBigRange
{
public:

    ScBigAddress    aStart;
    ScBigAddress    aEnd;

                    ScBigRange() : aStart(), aEnd() {}
                    ScBigRange( const ScBigRange& r )
                        : aStart( r.aStart ), aEnd( r.aEnd ) {}
                    ScBigRange( ScBigRange&& ) = default;
                    ScBigRange( const ScRange& r )
                        : aStart( r.aStart ), aEnd( r.aEnd ) {}
                    ScBigRange( sal_Int32 nCol1, sal_Int32 nRow1, sal_Int32 nTab1,
                            sal_Int32 nCol2, sal_Int32 nRow2, sal_Int32 nTab2 )
                        : aStart( nCol1, nRow1, nTab1 ),
                        aEnd( nCol2, nRow2, nTab2 ) {}

    void    Set( sal_Int32 nCol1, sal_Int32 nRow1, sal_Int32 nTab1,
                     sal_Int32 nCol2, sal_Int32 nRow2, sal_Int32 nTab2 )
                { aStart.Set( nCol1, nRow1, nTab1 );
                  aEnd.Set( nCol2, nRow2, nTab2 ); }

    void    GetVars( sal_Int32& nCol1, sal_Int32& nRow1, sal_Int32& nTab1,
                     sal_Int32& nCol2, sal_Int32& nRow2, sal_Int32& nTab2 ) const
                { aStart.GetVars( nCol1, nRow1, nTab1 );
                  aEnd.GetVars( nCol2, nRow2, nTab2 ); }

    bool    IsValid( const ScDocument* pDoc ) const
                { return aStart.IsValid( pDoc ) && aEnd.IsValid( pDoc ); }
    ScRange  MakeRange() const
                    { return ScRange( aStart.MakeAddress(),
                        aEnd.MakeAddress() ); }

    inline bool In( const ScBigAddress& ) const;    ///< is Address& in range?
    inline bool In( const ScBigRange& ) const;      ///< is Range& in range?
    inline bool Intersects( const ScBigRange& ) const;  ///< do two ranges overlap?

    ScBigRange&     operator=( const ScBigRange& r )
                        { aStart = r.aStart; aEnd = r.aEnd; return *this; }
    ScBigRange&     operator=( ScBigRange&& ) = default;
    bool            operator==( const ScBigRange& r ) const
                        { return (aStart == r.aStart) && (aEnd == r.aEnd); }
    bool            operator!=( const ScBigRange& r ) const
                        { return !operator==( r ); }
};

inline bool ScBigRange::In( const ScBigAddress& rAddr ) const
{
    return
        aStart.Col() <= rAddr.Col() && rAddr.Col() <= aEnd.Col() &&
        aStart.Row() <= rAddr.Row() && rAddr.Row() <= aEnd.Row() &&
        aStart.Tab() <= rAddr.Tab() && rAddr.Tab() <= aEnd.Tab();
}

inline bool ScBigRange::In( const ScBigRange& r ) const
{
    return
        aStart.Col() <= r.aStart.Col() && r.aEnd.Col() <= aEnd.Col() &&
        aStart.Row() <= r.aStart.Row() && r.aEnd.Row() <= aEnd.Row() &&
        aStart.Tab() <= r.aStart.Tab() && r.aEnd.Tab() <= aEnd.Tab();
}

inline bool ScBigRange::Intersects( const ScBigRange& r ) const
{
    return !(
        std::min( aEnd.Col(), r.aEnd.Col() ) < std::max( aStart.Col(), r.aStart.Col() )
     || std::min( aEnd.Row(), r.aEnd.Row() ) < std::max( aStart.Row(), r.aStart.Row() )
     || std::min( aEnd.Tab(), r.aEnd.Tab() ) < std::max( aStart.Tab(), r.aStart.Tab() )
        );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
