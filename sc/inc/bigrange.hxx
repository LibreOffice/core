/*************************************************************************
 *
 *  $RCSfile: bigrange.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_BIGRANGE_HXX
#define SC_BIGRANGE_HXX


#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif


static const INT32 nInt32Min = 0x80000000;
static const INT32 nInt32Max = 0x7fffffff;


class ScBigAddress
{
    INT32   nRow;
    INT32   nCol;
    INT32   nTab;

public:
            ScBigAddress() : nCol(0), nRow(0), nTab(0) {}
            ScBigAddress( INT32 nColP, INT32 nRowP, INT32 nTabP )
                : nCol( nColP ), nRow( nRowP ), nTab( nTabP ) {}
            ScBigAddress( const ScBigAddress& r )
                : nCol( r.nCol ), nRow( r.nRow ), nTab( r.nTab ) {}
            ScBigAddress( const ScAddress& r )
                : nCol( r.Col() ), nRow( r.Row() ), nTab( r.Tab() ) {}

    INT32   Col() const { return nCol; }
    INT32   Row() const { return nRow; }
    INT32   Tab() const { return nTab; }

    void    Set( INT32 nColP, INT32 nRowP, INT32 nTabP )
                { nCol = nColP; nRow = nRowP; nTab = nTabP; }
    void    SetCol( INT32 nColP ) { nCol = nColP; }
    void    SetRow( INT32 nRowP ) { nRow = nRowP; }
    void    SetTab( INT32 nTabP ) { nTab = nTabP; }
    void    IncCol( INT32 n = 1 ) { nCol += n; }
    void    IncRow( INT32 n = 1 ) { nRow += n; }
    void    IncTab( INT32 n = 1 ) { nTab += n; }

    void    GetVars( INT32& nColP, INT32& nRowP, INT32& nTabP ) const
                { nColP = nCol; nRowP = nRow; nTabP = nTab; }

    inline void     PutInOrder( ScBigAddress& r );
    inline BOOL     IsValid( const ScDocument* ) const;
    inline ScAddress    MakeAddress() const;

    ScBigAddress&   operator=( const ScBigAddress& r )
                    { nCol = r.nCol; nRow = r.nRow; nTab = r.nTab; return *this; }
    ScBigAddress&   operator=( const ScAddress& r )
                    { nCol = r.Col(); nRow = r.Row(); nTab = r.Tab(); return *this; }
    int             operator==( const ScBigAddress& r ) const
                    { return nCol == r.nCol && nRow == r.nRow && nTab == r.nTab; }
    int             operator!=( const ScBigAddress& r ) const
                    { return !operator==( r ); }

    friend inline SvStream& operator<< ( SvStream& rStream, const ScBigAddress& rAdr );
    friend inline SvStream& operator>> ( SvStream& rStream, ScBigAddress& rAdr );
};


inline void ScBigAddress::PutInOrder( ScBigAddress& r )
{
    INT32 nTmp;
    if ( r.nCol < nCol )
    {
        nTmp = r.nCol;
        r.nCol = nCol;
        nCol = nTmp;
    }
    if ( r.nRow < nRow )
    {
        nTmp = r.nRow;
        r.nRow = nRow;
        nRow = nTmp;
    }
    if ( r.nTab < nTab )
    {
        nTmp = r.nTab;
        r.nTab = nTab;
        nTab = nTmp;
    }
}


inline BOOL ScBigAddress::IsValid( const ScDocument* pDoc ) const
{   //! Min/Max sind ok, kennzeichnen ganze Col/Row/Tab
    return
        ((0 <= nCol && nCol <= MAXCOL)
            || nCol == nInt32Min || nCol == nInt32Max) &&
        ((0 <= nRow && nRow <= MAXROW)
            || nRow == nInt32Min || nRow == nInt32Max) &&
        ((0 <= nTab && nTab < pDoc->GetTableCount())
            || nTab == nInt32Min || nTab == nInt32Max)
        ;
}


inline ScAddress ScBigAddress::MakeAddress() const
{
    USHORT nColA, nRowA, nTabA;

    if ( nCol < 0 )
        nColA = 0;
    else if ( nCol > MAXCOL )
        nColA = MAXCOL;
    else
        nColA = (USHORT) nCol;

    if ( nRow < 0 )
        nRowA = 0;
    else if ( nRow > MAXROW )
        nRowA = MAXROW;
    else
        nRowA = (USHORT) nRow;

    if ( nTab < 0 )
        nTabA = 0;
    else if ( nTab > MAXTAB )
        nTabA = MAXTAB;
    else
        nTabA = (USHORT) nTab;

    return ScAddress( nColA, nRowA, nTabA );
}


inline SvStream& operator<< ( SvStream& rStream, const ScBigAddress& rAdr )
{
    rStream << rAdr.nCol << rAdr.nRow << rAdr.nTab;
    return rStream;
}


inline SvStream& operator>> ( SvStream& rStream, ScBigAddress& rAdr )
{
    rStream >> rAdr.nCol >> rAdr.nRow >> rAdr.nTab;
    return rStream;
}


class ScBigRange
{
public:

    ScBigAddress    aStart;
    ScBigAddress    aEnd;

                    ScBigRange() : aStart(), aEnd() {}
                    ScBigRange( const ScBigAddress& s, const ScBigAddress& e )
                        : aStart( s ), aEnd( e ) { aStart.PutInOrder( aEnd ); }
                    ScBigRange( const ScBigRange& r )
                        : aStart( r.aStart ), aEnd( r.aEnd ) {}
                    ScBigRange( const ScRange& r )
                        : aStart( r.aStart ), aEnd( r.aEnd ) {}
                    ScBigRange( const ScBigAddress& r )
                        : aStart( r ), aEnd( r ) {}
                    ScBigRange( const ScAddress& r )
                        : aStart( r ), aEnd( r ) {}
                    ScBigRange( INT32 nCol, INT32 nRow, INT32 nTab )
                        : aStart( nCol, nRow, nTab ), aEnd( aStart ) {}
                    ScBigRange( INT32 nCol1, INT32 nRow1, INT32 nTab1,
                            INT32 nCol2, INT32 nRow2, INT32 nTab2 )
                        : aStart( nCol1, nRow1, nTab1 ),
                        aEnd( nCol2, nRow2, nTab2 ) {}

    void    Set( INT32 nCol1, INT32 nRow1, INT32 nTab1,
                     INT32 nCol2, INT32 nRow2, INT32 nTab2 )
                { aStart.Set( nCol1, nRow1, nTab1 );
                    aEnd.Set( nCol2, nRow2, nTab2 ); }

    void    GetVars( INT32& nCol1, INT32& nRow1, INT32& nTab1,
                     INT32& nCol2, INT32& nRow2, INT32& nTab2 ) const
                { aStart.GetVars( nCol1, nRow1, nTab1 );
                    aEnd.GetVars( nCol2, nRow2, nTab2 ); }

    BOOL    IsValid( const ScDocument* pDoc ) const
                { return aStart.IsValid( pDoc ) && aEnd.IsValid( pDoc ); }
    inline ScRange  MakeRange() const
                    { return ScRange( aStart.MakeAddress(),
                        aEnd.MakeAddress() ); }

    inline BOOL In( const ScBigAddress& ) const;    // ist Address& in Range?
    inline BOOL In( const ScBigRange& ) const;      // ist Range& in Range?
    inline BOOL Intersects( const ScBigRange& ) const;  // ueberschneiden sich zwei Ranges?

    ScBigRange&     operator=( const ScBigRange& r )
                        { aStart = r.aStart; aEnd = r.aEnd; return *this; }
    int             operator==( const ScBigRange& r ) const
                        { return (aStart == r.aStart) && (aEnd == r.aEnd); }
    int             operator!=( const ScBigRange& r ) const
                        { return !operator==( r ); }

    friend inline SvStream& operator<< ( SvStream& rStream, const ScBigRange& rRange );
    friend inline SvStream& operator>> ( SvStream& rStream, ScBigRange& rRange );
};


inline BOOL ScBigRange::In( const ScBigAddress& rAddr ) const
{
    return
        aStart.Col() <= rAddr.Col() && rAddr.Col() <= aEnd.Col() &&
        aStart.Row() <= rAddr.Row() && rAddr.Row() <= aEnd.Row() &&
        aStart.Tab() <= rAddr.Tab() && rAddr.Tab() <= aEnd.Tab();
}


inline BOOL ScBigRange::In( const ScBigRange& r ) const
{
    return
        aStart.Col() <= r.aStart.Col() && r.aEnd.Col() <= aEnd.Col() &&
        aStart.Row() <= r.aStart.Row() && r.aEnd.Row() <= aEnd.Row() &&
        aStart.Tab() <= r.aStart.Tab() && r.aEnd.Tab() <= aEnd.Tab();
}


inline BOOL ScBigRange::Intersects( const ScBigRange& r ) const
{
    return !(
        Min( aEnd.Col(), r.aEnd.Col() ) < Max( aStart.Col(), r.aStart.Col() )
     || Min( aEnd.Row(), r.aEnd.Row() ) < Max( aStart.Row(), r.aStart.Row() )
     || Min( aEnd.Tab(), r.aEnd.Tab() ) < Max( aStart.Tab(), r.aStart.Tab() )
        );
}


inline SvStream& operator<< ( SvStream& rStream, const ScBigRange& rRange )
{
    rStream << rRange.aStart;
    rStream << rRange.aEnd;
    return rStream;
}


inline SvStream& operator>> ( SvStream& rStream, ScBigRange& rRange )
{
    rStream >> rRange.aStart;
    rStream >> rRange.aEnd;
    return rStream;
}



#endif
