/*************************************************************************
 *
 *  $RCSfile: address.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $  $Date: 2004-06-04 10:00:39 $
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

#ifndef SC_ADDRESS_HXX
#define SC_ADDRESS_HXX

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <limits>

class ScDocument;

#if 0
// Temporary conversion environment,
// defines SC_ROWLIMIT_TYPECONVERSION_NOCONVPASS
// and declares class SCROW,SCCOL,SCTAB,SCSIZE
#include "rowlimit_noconv.hxx"
#endif

#ifdef SC_ROWLIMIT_TYPECONVERSION_NOCONVPASS

#define SC_ROWLIMIT_TEMP_HACK 1

typedef SCROW    SCsROW;
typedef SCCOL    SCsCOL;
typedef SCTAB    SCsTAB;
typedef SCCOLROW SCsCOLROW;

#undef min
#undef max
const SCROW  SCROW_MAX    = USHRT_MAX;
const SCCOL  SCCOL_MAX    = USHRT_MAX;
const SCTAB  SCTAB_MAX    = USHRT_MAX;
const SCCOL  SCCOLROW_MAX = USHRT_MAX;
const SCSIZE SCSIZE_MAX   = ::std::numeric_limits<size_t>::max();

#define SC_ROWLIMIT_MORE_THAN_32K 0
#define MAXROWCOUNT_DEFINE 32000
#define MAXCOLCOUNT_DEFINE 256

const size_t      MAXROWCOUNT    = MAXROWCOUNT_DEFINE;
const size_t      MAXCOLCOUNT    = MAXCOLCOUNT_DEFINE;
const size_t      MAXTABCOUNT    = 256;
const size_t      MAXCOLROWCOUNT = MAXROWCOUNT;

const SCROW       MAXROW         = static_cast<int>(MAXROWCOUNT) - 1;
const SCCOL       MAXCOL         = static_cast<int>(MAXCOLCOUNT) - 1;
const SCTAB       MAXTAB         = static_cast<int>(MAXTABCOUNT) - 1;
const SCCOLROW    MAXCOLROW      = MAXROW;

#else // ! SC_ROWLIMIT_TYPECONVERSION_NOCONVPASS

// The typedefs
typedef sal_Int32 SCROW;
typedef sal_Int16 SCCOL;
typedef sal_Int16 SCTAB;
typedef sal_Int32 SCCOLROW;     // a type capable of holding either SCCOL or SCROW

// temporarily signed typedefs
typedef sal_Int32 SCsROW;
typedef sal_Int16 SCsCOL;
typedef sal_Int16 SCsTAB;
typedef sal_Int32 SCsCOLROW;

// size_t typedef to be able to find places where code was changed from USHORT
// to size_t and is used to read/write from/to streams.
typedef size_t SCSIZE;
// resolve ambiguity, TODO: move to tools/solar.h, or even better use ::std::min/max
inline size_t Min( size_t a, size_t b) { return a<b ? a : b; }
inline size_t Max( size_t a, size_t b) { return a<b ? a : b; }

// Maximum possible value of data type, NOT maximum row value.
// MSC confuses numeric_limit max() with macro max() if vcl/wintypes.hxx is
// included, we should not be using those stupid macros anyway.
#undef min
#undef max
const SCROW    SCROW_MAX    = ::std::numeric_limits<SCROW>::max();
const SCCOL    SCCOL_MAX    = ::std::numeric_limits<SCCOL>::max();
const SCTAB    SCTAB_MAX    = ::std::numeric_limits<SCTAB>::max();
const SCCOLROW SCCOLROW_MAX = ::std::numeric_limits<SCCOLROW>::max();
const SCSIZE   SCSIZE_MAX   = ::std::numeric_limits<SCSIZE>::max();

// A define to handle critical sections we hopefully don't need very often.
#define SC_ROWLIMIT_MORE_THAN_32K 1     /* set to 1 if we throw the switch */

// The maximum values. Defines are needed for preprocessor checks in
// bcaslot.cxx, otherwise type safe constants are preferred.
#define MAXROWCOUNT_DEFINE 65536
#define MAXCOLCOUNT_DEFINE 256

// Count values
const SCROW       MAXROWCOUNT    = MAXROWCOUNT_DEFINE;
const SCCOL       MAXCOLCOUNT    = MAXCOLCOUNT_DEFINE;
const SCTAB       MAXTABCOUNT    = 256;
const SCCOLROW    MAXCOLROWCOUNT = MAXROWCOUNT;
// Maximum values
const SCROW       MAXROW         = MAXROWCOUNT - 1;
const SCCOL       MAXCOL         = MAXCOLCOUNT - 1;
const SCTAB       MAXTAB         = MAXTABCOUNT - 1;
const SCCOLROW    MAXCOLROW      = MAXROW;

#endif // SC_ROWLIMIT_TYPECONVERSION_NOCONVPASS


// Special values
const SCTAB SC_TAB_APPEND     = SCTAB_MAX;
const SCTAB TABLEID_DOC       = SCTAB_MAX;  // entire document, e.g. protect
const SCROW SCROWS32K         = 32000;
const SCCOL SCCOL_REPEAT_NONE = SCCOL_MAX;
const SCROW SCROW_REPEAT_NONE = SCROW_MAX;
// REPEAT_NONE


// We hope to get rid of the binary file format. If not, these are the places
// we'd have to investigate because variable types changed. Just place code in
// #if SC_ROWLIMIT_STREAM_ACCESS for now.
#define SC_ROWLIMIT_STREAM_ACCESS 0
// usage:
//#if SC_ROWLIMIT_STREAM_ACCESS
//#error address types changed!
//... code ...
//#endif // SC_ROWLIMIT_STREAM_ACCESS


// For future reference, place in code where more than 64k rows would need a
// special handling:
// #if SC_ROWLIMIT_MORE_THAN_64K
// #error row limit 64k
// #endif
#define SC_ROWLIMIT_MORE_THAN_64K 0     /* set to 1 if we throw the switch */
const SCROW SCROWS64K = 65536;

// Undef code that uses ScAddress bitfield stream accessing methods.
#ifdef SC_ROWLIMIT_TYPECONVERSION_NOCONVPASS
#define SC_ADDRESS_BITS_USED 0
#else
#define SC_ADDRESS_BITS_USED 1
#endif
// If we throw the 64k switch this has to be the consequence.
#if SC_ROWLIMIT_MORE_THAN_64K
#undef SC_ADDRESS_BITS_USED
#define SC_ADDRESS_BITS_USED 0
#endif

// === old stuff defines =====================================================

#ifdef WIN
// Under 16bit Windows rows still had to be limited to 8192.
// (define manually for testing)
#define SC_LIMIT_ROWS
#endif

#define MAXROW_30   8191
#define MAXROW_40   31999

#ifdef SC_LIMIT_ROWS
#undef MAXROWCOUNT_DEFINE
#define MAXROWCOUNT_DEFINE 8192
const SCROW W16MAXROWCOUNT = MAXROWCOUNT_DEFINE;
const SCROW W16MAXROW = W16MAXROWCOUNT - 1;
#define MAXROWCOUNT W16MAXROWCOUNT
#define MAXROW      W16MAXROW
#endif

#define VALIDCOL(nCol)                  (ValidCol(nCol))
#define VALIDROW(nRow)                  (ValidRow(nRow))
#define VALIDTAB(nTab)                  (ValidTab(nTab))
#define VALIDCOLROW(nCol,nRow)          (ValidColRow(nCol,nRow))
#define VALIDCOLROWTAB(nCol,nRow,nTab)  (ValidColRowTab(nCol,nRow,nTab))

// === old stuff defines end =================================================

inline bool ValidCol( SCCOL nCol )
{
    return static_cast<SCCOL>(0) <= nCol && nCol <= MAXCOL;
}

inline bool ValidRow( SCROW nRow )
{
    return static_cast<SCROW>(0) <= nRow && nRow <= MAXROW;
}

inline bool ValidTab( SCTAB nTab )
{
    return static_cast<SCTAB>(0) <= nTab && nTab <= MAXTAB;
}

inline bool ValidTab( SCTAB nTab, SCTAB nMaxTab )
{
    return static_cast<SCTAB>(0) <= nTab && nTab <= nMaxTab;
}

inline bool ValidColRow( SCCOL nCol, SCROW nRow )
{
    return ValidCol( nCol) && ValidRow( nRow);
}

inline bool ValidColRowTab( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    return ValidCol( nCol) && ValidRow( nRow) && ValidTab( nTab);
}

// === ScAddress =============================================================

// The old cell address is combined in one UINT32:
// +---+---+-------+
// |Tab|Col|  Row  |
// +---+---+-------+
// For speed reasons access isn't done by shifting bits but by using platform
// dependent casts, which unfortunately also leads to aliasing problems when
// not using gcc -fno-strict-aliasing

// The result of ConvertRef() is a bit group of the following:

#define SCA_COL_ABSOLUTE    0x01
#define SCA_ROW_ABSOLUTE    0x02
#define SCA_TAB_ABSOLUTE    0x04
#define SCA_TAB_3D          0x08
#define SCA_COL2_ABSOLUTE   0x10
#define SCA_ROW2_ABSOLUTE   0x20
#define SCA_TAB2_ABSOLUTE   0x40
#define SCA_TAB2_3D         0x80
#define SCA_VALID_ROW       0x0100
#define SCA_VALID_COL       0x0200
#define SCA_VALID_TAB       0x0400
#define SCA_VALID_ROW2      0x1000
#define SCA_VALID_COL2      0x2000
#define SCA_VALID_TAB2      0x4000
#define SCA_VALID           0x8000

#define SCA_ABS               SCA_VALID \
                            | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE

#define SCR_ABS               SCA_ABS \
                            | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE

#define SCA_ABS_3D          SCA_ABS | SCA_TAB_3D
#define SCR_ABS_3D          SCR_ABS | SCA_TAB_3D

// === ScAddressOldBitfieldConverter =========================================
// TODO: to be removed if binary file format is stripped

#if SC_ADDRESS_BITS_USED

class ScAddressOldBitfieldConverter
{
private:
    UINT32 nAddress;
    inline UINT16* RowPos();
    inline BYTE* ColPos();
    inline BYTE* TabPos();
    inline const UINT16* RowPos() const;
    inline const BYTE* ColPos() const;
    inline const BYTE* TabPos() const;

public:
    inline ScAddressOldBitfieldConverter() : nAddress(0) {}
    inline explicit ScAddressOldBitfieldConverter( UINT32 n ) : nAddress(n) {}
    inline explicit ScAddressOldBitfieldConverter( SvStream& rStream )
    { rStream >> nAddress; }
    inline ScAddressOldBitfieldConverter( SCCOL nCol, SCROW nRow, SCTAB nTab )
    { Set( nCol, nRow, nTab); }
    inline void Set( SCCOL nCol, SCROW nRow, SCTAB nTab );
    inline void Get( SCCOL& nCol, SCROW& nRow, SCTAB& nTab ) const;
    inline UINT32 GetBitfield() const { return nAddress; }
};

#ifdef __LITTLEENDIAN

inline USHORT* ScAddressOldBitfieldConverter::RowPos() { return (USHORT*) &nAddress; }
inline BYTE*   ScAddressOldBitfieldConverter::ColPos() { return (BYTE*) &nAddress + 2; }
inline BYTE*   ScAddressOldBitfieldConverter::TabPos() { return (BYTE*) &nAddress + 3; }
inline const USHORT* ScAddressOldBitfieldConverter::RowPos() const { return (USHORT*) &nAddress; }
inline const BYTE*   ScAddressOldBitfieldConverter::ColPos() const { return (BYTE*) &nAddress + 2; }
inline const BYTE*   ScAddressOldBitfieldConverter::TabPos() const { return (BYTE*) &nAddress + 3; }

#else

inline USHORT* ScAddressOldBitfieldConverter::RowPos() { return (USHORT*) ((BYTE*) &nAddress + 2 ); }
inline BYTE*   ScAddressOldBitfieldConverter::ColPos() { return (BYTE*) &nAddress + 1; }
inline BYTE*   ScAddressOldBitfieldConverter::TabPos() { return (BYTE*) &nAddress; }
inline const USHORT* ScAddressOldBitfieldConverter::RowPos() const { return (USHORT*) ((BYTE*) &nAddress + 2 ); }
inline const BYTE*   ScAddressOldBitfieldConverter::ColPos() const { return (BYTE*) &nAddress + 1; }
inline const BYTE*   ScAddressOldBitfieldConverter::TabPos() const { return (BYTE*) &nAddress; }

#endif

inline void ScAddressOldBitfieldConverter::Set( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    DBG_ASSERT( nCol <= 0xff && nRow <= 0xffff && nTab <= 0xff,
            "ScAddress UINT32 conversion doesn't fit");
//  nAddress = ((UINT32) ((((BYTE) nTab) << 8 ) + (BYTE) nCol ) << 16 )
//           | (UINT32) (UINT16) nRow;
// Shifting: 5 mov, 2 xor, 2 shl, 1 add, 1 and, 1 or
// Casting: 7 mov, less code and faster
    *ColPos() = (BYTE) nCol;
    *RowPos() = (UINT16) nRow;
    *TabPos() = (BYTE) nTab;
}

inline void ScAddressOldBitfieldConverter::Get( SCCOL& nCol, SCROW& nRow, SCTAB& nTab ) const
{
    nCol = *ColPos();
    nRow = *RowPos();
    nTab = *TabPos();
}

#endif  // SC_ADDRESS_BITS_USED

// === ScAddress =============================================================

class ScAddress
{
private:
    SCROW   nRow;
    SCCOL   nCol;
    SCTAB   nTab;

public:

    enum Uninitialized      { UNINITIALIZED };
    enum InitializeInvalid  { INITIALIZE_INVALID };

    inline ScAddress() : nRow(0), nCol(0), nTab(0) {}
    inline ScAddress( SCCOL nColP, SCROW nRowP, SCTAB nTabP )
        : nRow(nRowP), nCol(nColP), nTab(nTabP)
        {}
    /** Yes, it is what it seems to be: Uninitialized. May be used for
        performance reasons if it is initialized by other means. */
    inline ScAddress( Uninitialized ) {}
    inline ScAddress( InitializeInvalid )
        : nRow(-1), nCol(-1), nTab(-1) {}
    inline ScAddress( const ScAddress& r )
        : nRow(r.nRow), nCol(r.nCol), nTab(r.nTab)
        {}
    inline ScAddress& operator=( const ScAddress& r );

#if SC_ADDRESS_BITS_USED
    // TODO: TO BE REMOVED
    inline explicit ScAddress( UINT32 nOldAddress )
    { ScAddressOldBitfieldConverter( nOldAddress).Get( nCol, nRow, nTab); }
    // TODO: TO BE REMOVED
    inline ScAddress& operator=( UINT32 nOldAddress )
    {
        ScAddressOldBitfieldConverter( nOldAddress).Get( nCol, nRow, nTab);
        return *this;
    }
    // TODO: TO BE REMOVED
    inline void Set( const ScAddressOldBitfieldConverter& rBC )
    { rBC.Get( nCol, nRow, nTab); }
    // TODO: TO  BE REMOVED
    inline operator UINT32() const
    { return ScAddressOldBitfieldConverter( nCol, nRow, nTab).GetBitfield(); }
#endif // SC_ADDRESS_BITS_USED

    inline void Set( SCCOL nCol, SCROW nRow, SCTAB nTab );
    inline SCROW Row() const { return nRow; }
    inline SCCOL Col() const { return nCol; }
    inline SCTAB Tab() const { return nTab; }
    inline void SetRow( SCROW nRowP ) { nRow = nRowP; }
    inline void SetCol( SCCOL nColP ) { nCol = nColP; }
    inline void SetTab( SCTAB nTabP ) { nTab = nTabP; }
    inline void SetInvalid() { nRow = -1; nCol = -1; nTab = -1; }
    inline void PutInOrder( ScAddress& r );
    inline void IncRow( SCsROW n=1 ) { nRow += n; }
    inline void IncCol( SCsCOL n=1 ) { nCol += n; }
    inline void IncTab( SCsTAB n=1 ) { nTab += n; }
    inline void GetVars( SCCOL& nColP, SCROW& nRowP, SCTAB& nTabP ) const
    { nColP = nCol; nRowP = nRow; nTabP = nTab; }
    USHORT Parse( const String&, ScDocument* = NULL );
    void Format( String&, USHORT = 0, ScDocument* = NULL ) const;
    // The document for the maximum defined sheet number
    bool Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* =NULL );
    inline bool operator==( const ScAddress& r ) const;
    inline bool operator!=( const ScAddress& r ) const;
    inline bool operator<( const ScAddress& r ) const;
    inline bool operator<=( const ScAddress& r ) const;
    inline bool operator>( const ScAddress& r ) const;
    inline bool operator>=( const ScAddress& r ) const;
#if SC_ROWLIMIT_STREAM_ACCESS
    // TODO: TO BE REMOVED
    friend inline SvStream& operator<< ( SvStream& rStream, const ScAddress& rAdr );
    // TODO: TO BE REMOVED
    friend inline SvStream& operator>> ( SvStream& rStream, ScAddress& rAdr );
#endif

    // moved from ScTripel
    /// "(1,2,3)"
    String GetText() const;
    /// "A1" or "$A$1"
    String GetColRowString( bool bAbsolute = FALSE ) const;
};

inline void ScAddress::PutInOrder( ScAddress& r )
{
    if ( r.Col() < Col() )
    {
        SCCOL nTmp = r.Col();
        r.SetCol( Col() );
        SetCol( nTmp );
    }
    if ( r.Row() < Row() )
    {
        SCROW nTmp = r.Row();
        r.SetRow( Row() );
        SetRow( nTmp );
    }
    if ( r.Tab() < Tab() )
    {
        SCTAB nTmp = r.Tab();
        r.SetTab( Tab() );
        SetTab( nTmp );
    }
}

inline void ScAddress::Set( SCCOL nColP, SCROW nRowP, SCTAB nTabP )
{
    nCol = nColP;
    nRow = nRowP;
    nTab = nTabP;
}

inline ScAddress& ScAddress::operator=( const ScAddress& r )
{
    nCol = r.nCol;
    nRow = r.nRow;
    nTab = r.nTab;
    return *this;
}

inline bool ScAddress::operator==( const ScAddress& r ) const
{
    return nRow == r.nRow && nCol == r.nCol && nTab == r.nTab;
}

inline bool ScAddress::operator!=( const ScAddress& r ) const
{
    return !operator==( r );
}

inline bool ScAddress::operator<( const ScAddress& r ) const
{
    // Same behavior as the old UINT32 nAddress < r.nAddress with encoded
    // tab|col|row bit fields.
    if (nTab == r.nTab)
    {
        if (nCol == r.nCol)
            return nRow < r.nRow;
        else
            return nCol < r.nCol;
    }
    else
        return nTab < r.nTab;
}

inline bool ScAddress::operator<=( const ScAddress& r ) const
{
    return operator<( r ) || operator==( r );
}

inline bool ScAddress::operator>( const ScAddress& r ) const
{
    return !operator<=( r );
}

inline bool ScAddress::operator>=( const ScAddress& r ) const
{
    return !operator<( r );
}

#if SC_ADDRESS_BITS_USED && SC_ROWLIMIT_STREAM_ACCESS

inline SvStream& operator<< ( SvStream& rStream, const ScAddress& rAdr )
{
    rStream << UINT32(rAdr);
    return rStream;
}

inline SvStream& operator>> ( SvStream& rStream, ScAddress& rAdr )
{
    ScAddressOldBitfieldConverter aBC( rStream);
    rAdr.Set( aBC );
    return rStream;
}

#endif // SC_ADDRESS_BITS_USED

// === ScRange ===============================================================

class ScRange
{
public:
    ScAddress aStart, aEnd;
    inline ScRange() : aStart(), aEnd() {}
    inline ScRange( ScAddress::Uninitialized e )
        : aStart( e ), aEnd( e ) {}
    inline ScRange( ScAddress::InitializeInvalid e )
        : aStart( e ), aEnd( e ) {}
    inline ScRange( const ScAddress& s, const ScAddress& e )
        : aStart( s ), aEnd( e ) { aStart.PutInOrder( aEnd ); }
    inline ScRange( const ScRange& r ) : aStart( r.aStart ), aEnd( r.aEnd ) {}
    inline ScRange( const ScAddress& r ) : aStart( r ), aEnd( r ) {}
    inline ScRange( SCCOL nCol, SCROW nRow, SCTAB nTab )
        : aStart( nCol, nRow, nTab ), aEnd( aStart ) {}
    inline ScRange( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
             SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
        : aStart( nCol1, nRow1, nTab1 ), aEnd( nCol2, nRow2, nTab2 ) {}

    inline ScRange& operator=( const ScRange& r )
    { aStart = r.aStart; aEnd = r.aEnd; return *this; }
    inline ScRange& operator=( const ScAddress& rPos )
    { aStart = aEnd = rPos; return *this; }
    inline bool In( const ScAddress& ) const;   // is Address& in Range?
    inline bool In( const ScRange& ) const;     // is Range& in Range?
    USHORT Parse( const String&, ScDocument* = NULL );
    USHORT ParseAny( const String&, ScDocument* = NULL );
    inline void GetVars( SCCOL& nCol1, SCROW& nRow1, SCTAB& nTab1,
        SCCOL& nCol2, SCROW& nRow2, SCTAB& nTab2 ) const;
    void Format( String&, USHORT = 0, ScDocument* = NULL ) const;
    // The document for the maximum defined sheet number
    bool Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* =NULL );
    void Justify();
    void ExtendOne();
    bool Intersects( const ScRange& ) const;    // do two ranges intersect?
    inline bool operator==( const ScRange& r ) const;
    inline bool operator!=( const ScRange& r ) const;
    inline bool operator<( const ScRange& r ) const;
    inline bool operator<=( const ScRange& r ) const;
    inline bool operator>( const ScRange& r ) const;
    inline bool operator>=( const ScRange& r ) const;
#if SC_ROWLIMIT_STREAM_ACCESS
    friend inline SvStream& operator<< ( SvStream& rStream, const ScRange& rRange );
    friend inline SvStream& operator>> ( SvStream& rStream, ScRange& rRange );
#endif
};

inline void ScRange::GetVars( SCCOL& nCol1, SCROW& nRow1, SCTAB& nTab1,
        SCCOL& nCol2, SCROW& nRow2, SCTAB& nTab2 ) const
{
    aStart.GetVars( nCol1, nRow1, nTab1 );
    aEnd.GetVars( nCol2, nRow2, nTab2 );
}

inline bool ScRange::operator==( const ScRange& r ) const
{
    return ( (aStart == r.aStart) && (aEnd == r.aEnd) );
}

inline bool ScRange::operator!=( const ScRange& r ) const
{
    return !operator==( r );
}

// Sort on upper left corner, if equal then use lower right too.
inline bool ScRange::operator<( const ScRange& r ) const
{
    return aStart < r.aStart || (aStart == r.aStart && aEnd < r.aEnd) ;
}

inline bool ScRange::operator<=( const ScRange& r ) const
{
    return operator<( r ) || operator==( r );
}

inline bool ScRange::operator>( const ScRange& r ) const
{
    return !operator<=( r );
}

inline bool ScRange::operator>=( const ScRange& r ) const
{
    return !operator<( r );
}

inline bool ScRange::In( const ScAddress& rAddr ) const
{
    return
        aStart.Col() <= rAddr.Col() && rAddr.Col() <= aEnd.Col() &&
        aStart.Row() <= rAddr.Row() && rAddr.Row() <= aEnd.Row() &&
        aStart.Tab() <= rAddr.Tab() && rAddr.Tab() <= aEnd.Tab();
}

inline bool ScRange::In( const ScRange& r ) const
{
    return
        aStart.Col() <= r.aStart.Col() && r.aEnd.Col() <= aEnd.Col() &&
        aStart.Row() <= r.aStart.Row() && r.aEnd.Row() <= aEnd.Row() &&
        aStart.Tab() <= r.aStart.Tab() && r.aEnd.Tab() <= aEnd.Tab();
}

#if SC_ROWLIMIT_STREAM_ACCESS

inline SvStream& operator<< ( SvStream& rStream, const ScRange& rRange )
{
    rStream << rRange.aStart;
    rStream << rRange.aEnd;
    return rStream;
}

inline SvStream& operator>> ( SvStream& rStream, ScRange& rRange )
{
    rStream >> rRange.aStart;
    rStream >> rRange.aEnd;
    return rStream;
}

#endif // SC_ROWLIMIT_STREAM_ACCESS

// === ScRangePair ===========================================================

class ScRangePair
{
private:
    ScRange aRange[2];

public:
    ScRangePair() {}
    ScRangePair( const ScRangePair& r )
        { aRange[0] = r.aRange[0]; aRange[1] = r.aRange[1]; }
    ScRangePair( const ScRange& r1, const ScRange& r2 )
        {  aRange[0] = r1; aRange[1] = r2; }

    inline ScRangePair& operator= ( const ScRangePair& r );
    const ScRange&      GetRange( USHORT n ) const { return aRange[n]; }
    ScRange&            GetRange( USHORT n ) { return aRange[n]; }
    inline int operator==( const ScRangePair& ) const;
    inline int operator!=( const ScRangePair& ) const;
#if SC_ROWLIMIT_STREAM_ACCESS
    friend inline SvStream& operator<< ( SvStream&, const ScRangePair& );
    friend inline SvStream& operator>> ( SvStream&, ScRange& );
#endif
};

inline ScRangePair& ScRangePair::operator= ( const ScRangePair& r )
{
    aRange[0] = r.aRange[0];
    aRange[1] = r.aRange[1];
    return *this;
}

inline int ScRangePair::operator==( const ScRangePair& r ) const
{
    return ( (aRange[0] == r.aRange[0]) && (aRange[1] == r.aRange[1]) );
}

inline int ScRangePair::operator!=( const ScRangePair& r ) const
{
    return !operator==( r );
}

#if SC_ROWLIMIT_STREAM_ACCESS

inline SvStream& operator<< ( SvStream& rStream, const ScRangePair& rPair )
{
    rStream << rPair.GetRange(0);
    rStream << rPair.GetRange(1);
    return rStream;
}

inline SvStream& operator>> ( SvStream& rStream, ScRangePair& rPair )
{
    rStream >> rPair.GetRange(0);
    rStream >> rPair.GetRange(1);
    return rStream;
}

#endif // SC_ROWLIMIT_STREAM_ACCESS

// === ScRefAddress ==========================================================

class ScRefAddress
{
            ScAddress           aAdr;
            bool                bRelCol;
            bool                bRelRow;
            bool                bRelTab;
public:
    inline ScRefAddress() : bRelCol(false), bRelRow(false), bRelTab(false)
        {}
    inline ScRefAddress( SCCOL nCol, SCROW nRow, SCTAB nTab,
            bool bRelColP, bool bRelRowP, bool bRelTabP ) :
        aAdr(nCol, nRow, nTab),
        bRelCol(bRelColP), bRelRow(bRelRowP), bRelTab(bRelTabP)
        {}
    inline ScRefAddress( const ScAddress& rAdr,
            bool bRelColP, bool bRelRowP, bool bRelTabP ) :
        aAdr(rAdr),
        bRelCol(bRelColP), bRelRow(bRelRowP), bRelTab(bRelTabP)
        {}
    inline ScRefAddress( const ScRefAddress& rRef ) :
            aAdr(rRef.aAdr), bRelCol(rRef.bRelCol), bRelRow(rRef.bRelRow),
            bRelTab(rRef.bRelTab)
            {}

    inline  ScRefAddress&   operator=( const ScRefAddress& );

    inline  bool    IsRelCol() const { return bRelCol; }
    inline  bool    IsRelRow() const { return bRelRow; }
    inline  bool    IsRelTab() const { return bRelTab; }

    inline  void    SetRelCol(bool bNewRelCol) { bRelCol = bNewRelCol; }
    inline  void    SetRelRow(bool bNewRelRow) { bRelRow = bNewRelRow; }
    inline  void    SetRelTab(bool bNewRelTab) { bRelTab = bNewRelTab; }

    inline  void    Set( const ScAddress& rAdr,
                        bool bNewRelCol, bool bNewRelRow, bool bNewRelTab );
    inline  void    Set( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                        bool bNewRelCol, bool bNewRelRow, bool bNewRelTab );

    inline  const ScAddress&    GetAddress() const { return aAdr; }
    inline  SCCOL   Col() const { return aAdr.Col(); }
    inline  SCROW   Row() const { return aAdr.Row(); }
    inline  SCTAB   Tab() const { return aAdr.Tab(); }

    inline  int     operator == ( const ScRefAddress& r ) const;
    inline  int     operator != ( const ScRefAddress& r ) const
                    { return !(operator==(r)); }

            String  GetRefString( ScDocument* pDoc, SCTAB nActTab) const;
};

inline ScRefAddress& ScRefAddress::operator=( const ScRefAddress& rRef )
{
    aAdr = rRef.aAdr;
    bRelCol = rRef.bRelCol;
    bRelRow = rRef.bRelRow;
    bRelTab = rRef.bRelTab;
    return *this;
}

inline void ScRefAddress::Set( const ScAddress& rAdr,
        bool bNewRelCol, bool bNewRelRow, bool bNewRelTab )
{
    aAdr = rAdr;
    bRelCol = bNewRelCol;
    bRelRow = bNewRelRow;
    bRelTab = bNewRelTab;
}

inline void ScRefAddress::Set( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
        bool bNewRelCol, bool bNewRelRow, bool bNewRelTab )
{
    aAdr.Set( nNewCol, nNewRow, nNewTab);
    bRelCol = bNewRelCol;
    bRelRow = bNewRelRow;
    bRelTab = bNewRelTab;
}

inline int ScRefAddress::operator==( const ScRefAddress& r ) const
{
    return aAdr == r.aAdr && bRelCol == r.bRelCol && bRelRow == r.bRelRow &&
        bRelTab == r.bRelTab;
}

// ===========================================================================
// Global functions
// ===========================================================================

// Special values for cells always broadcasting or listening (RECALCMODE_ALWAYS
// and the like).
#define BCA_BRDCST_ALWAYS ScAddress( 0, SCROW_MAX, 0 )
#define BCA_LISTEN_ALWAYS ScRange( BCA_BRDCST_ALWAYS, BCA_BRDCST_ALWAYS )

template< typename T > void PutInOrder( T& nStart, T& nEnd )
{
    if (nEnd < nStart)
    {
        T nTemp;
        nTemp = nEnd;
        nEnd = nStart;
        nStart = nTemp;
    }
}

bool ConvertSingleRef( ScDocument* pDoc, const String& rRefString,
        SCTAB nDefTab, ScRefAddress& rRefAddress);

bool ConvertDoubleRef(ScDocument* pDoc, const String& rRefString,
        SCTAB nDefTab, ScRefAddress& rStartRefAddress,
        ScRefAddress& rEndRefAddress);

/// append alpha representation of column to buffer
void ColToAlpha( rtl::OUStringBuffer& rBuffer, SCCOL nCol);

inline void ColToAlpha( String& rStr, SCCOL nCol)
{
    rtl::OUStringBuffer aBuf(2);
    ColToAlpha( aBuf, nCol);
    rStr.Append( aBuf.getStr(), static_cast<xub_StrLen>(aBuf.getLength()));
}

inline String ColToAlpha( SCCOL nCol )
{
    rtl::OUStringBuffer aBuf(2);
    ColToAlpha( aBuf, nCol);
    return aBuf.makeStringAndClear();
}

/// get column number of A..IV... string
bool AlphaToCol( SCCOL& rCol, const String& rStr);

#endif // SC_ADDRESS_HXX

