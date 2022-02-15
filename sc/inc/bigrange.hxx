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

#include "address.hxx"
#include "document.hxx"

// This is used by change tracking. References there may be located also outside of the document
// (see ScRefUpdate::Update()), and so it needs bigger range than ScAddress/ScRange.

class ScBigAddress
{
    sal_Int64   nRow;
    sal_Int64   nCol;
    sal_Int64   nTab;

public:
            ScBigAddress() : nRow(0), nCol(0), nTab(0) {}
            ScBigAddress( sal_Int64 nColP, sal_Int64 nRowP, sal_Int64 nTabP )
                : nRow( nRowP ), nCol( nColP ), nTab( nTabP ) {}
            ScBigAddress( const ScBigAddress& r )
                : nRow( r.nRow ), nCol( r.nCol ), nTab( r.nTab ) {}
            ScBigAddress( ScBigAddress&& ) = default;
            ScBigAddress( const ScAddress& r )
                : nRow( r.Row() ), nCol( r.Col() ), nTab( r.Tab() ) {}

    sal_Int64   Col() const { return nCol; }
    sal_Int64   Row() const { return nRow; }
    sal_Int64   Tab() const { return nTab; }

    void    Set( sal_Int64 nColP, sal_Int64 nRowP, sal_Int64 nTabP )
                { nCol = nColP; nRow = nRowP; nTab = nTabP; }
    void    SetCol( sal_Int64 nColP ) { nCol = nColP; }
    void    SetRow( sal_Int64 nRowP ) { nRow = nRowP; }
    void    SetTab( sal_Int64 nTabP ) { nTab = nTabP; }
    void    IncCol( sal_Int64 n = 1 ) { nCol += n; }
    void    IncRow( sal_Int64 n = 1 ) { nRow += n; }
    void    IncTab( sal_Int64 n = 1 ) { nTab += n; }

    void    GetVars( sal_Int64& nColP, sal_Int64& nRowP, sal_Int64& nTabP ) const
                { nColP = nCol; nRowP = nRow; nTabP = nTab; }

    bool IsValid( const ScDocument& rDoc ) const;
    inline ScAddress    MakeAddress( const ScDocument& rDoc ) const;

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

inline ScAddress ScBigAddress::MakeAddress( const ScDocument& rDoc ) const
{
    SCCOL nColA;
    SCROW nRowA;
    SCTAB nTabA;

    if ( nCol < 0 )
        nColA = 0;
    else if ( nCol > rDoc.MaxCol() )
        nColA = rDoc.MaxCol();
    else
        nColA = static_cast<SCCOL>(nCol);

    if ( nRow < 0 )
        nRowA = 0;
    else if ( nRow > rDoc.MaxRow() )
        nRowA = rDoc.MaxRow();
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
                    ScBigRange( sal_Int64 nCol1, sal_Int64 nRow1, sal_Int64 nTab1,
                            sal_Int64 nCol2, sal_Int64 nRow2, sal_Int64 nTab2 )
                        : aStart( nCol1, nRow1, nTab1 ),
                        aEnd( nCol2, nRow2, nTab2 ) {}

    void    Set( sal_Int64 nCol1, sal_Int64 nRow1, sal_Int64 nTab1,
                     sal_Int64 nCol2, sal_Int64 nRow2, sal_Int64 nTab2 )
                { aStart.Set( nCol1, nRow1, nTab1 );
                  aEnd.Set( nCol2, nRow2, nTab2 ); }

    void    GetVars( sal_Int64& nCol1, sal_Int64& nRow1, sal_Int64& nTab1,
                     sal_Int64& nCol2, sal_Int64& nRow2, sal_Int64& nTab2 ) const
                { aStart.GetVars( nCol1, nRow1, nTab1 );
                  aEnd.GetVars( nCol2, nRow2, nTab2 ); }

    bool    IsValid( const ScDocument& rDoc ) const
                { return aStart.IsValid( rDoc ) && aEnd.IsValid( rDoc ); }
    ScRange  MakeRange( const ScDocument& rDoc ) const
                { return ScRange( aStart.MakeAddress( rDoc ), aEnd.MakeAddress( rDoc ) ); }

    inline bool Contains( const ScBigAddress& ) const;    ///< is Address& in range?
    inline bool Contains( const ScBigRange& ) const;      ///< is Range& in range?
    inline bool Intersects( const ScBigRange& ) const;  ///< do two ranges overlap?

    ScBigRange&     operator=( const ScBigRange& r )
                        { aStart = r.aStart; aEnd = r.aEnd; return *this; }
    ScBigRange&     operator=( ScBigRange&& ) = default;
    bool            operator==( const ScBigRange& r ) const
                        { return (aStart == r.aStart) && (aEnd == r.aEnd); }
    bool            operator!=( const ScBigRange& r ) const
                        { return !operator==( r ); }

    // These are used to define whole rows/cols/tabs.
    constexpr static sal_Int64 nRangeMin = ::std::numeric_limits<sal_Int64>::min();;
    constexpr static sal_Int64 nRangeMax = ::std::numeric_limits<sal_Int64>::max();;
};

inline bool ScBigRange::Contains( const ScBigAddress& rAddr ) const
{
    return
        aStart.Col() <= rAddr.Col() && rAddr.Col() <= aEnd.Col() &&
        aStart.Row() <= rAddr.Row() && rAddr.Row() <= aEnd.Row() &&
        aStart.Tab() <= rAddr.Tab() && rAddr.Tab() <= aEnd.Tab();
}

inline bool ScBigRange::Contains( const ScBigRange& r ) const
{
    return
        aStart.Col() <= r.aStart.Col() && r.aEnd.Col() <= aEnd.Col() &&
        aStart.Row() <= r.aStart.Row() && r.aEnd.Row() <= aEnd.Row() &&
        aStart.Tab() <= r.aStart.Tab() && r.aEnd.Tab() <= aEnd.Tab();
}

inline bool ScBigRange::Intersects( const ScBigRange& r ) const
{
    return
        aStart.Col() <= r.aEnd.Col() && r.aStart.Col() <= aEnd.Col() &&
        aStart.Row() <= r.aEnd.Row() && r.aStart.Row() <= aEnd.Row() &&
        aStart.Tab() <= r.aEnd.Tab() && r.aStart.Tab() <= aEnd.Tab();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
