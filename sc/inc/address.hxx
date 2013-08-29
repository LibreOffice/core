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

#ifndef SC_ADDRESS_HXX
#define SC_ADDRESS_HXX

#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/solar.h>
#include <rtl/ustrbuf.hxx>
#include <osl/endian.h>

#include <limits>
#include "scdllapi.h"
#include <formula/grammar.hxx>

#include <com/sun/star/uno/Sequence.hxx>

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct ExternalLinkInfo;
    }
}}}

class ScDocument;

// The typedefs
typedef sal_Int32 SCROW;
typedef sal_Int16 SCCOL;
typedef sal_Int16 SCTAB;
typedef sal_Int32 SCCOLROW;     ///< a type capable of holding either SCCOL or SCROW

// temporarily signed typedefs
typedef sal_Int32 SCsROW;
typedef sal_Int16 SCsCOL;
typedef sal_Int16 SCsTAB;
typedef sal_Int32 SCsCOLROW;

/** size_t typedef to be able to find places where code was changed from USHORT
    to size_t and is used to read/write from/to streams. */
typedef size_t SCSIZE;

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

// The maximum values. Defines are needed for preprocessor checks, for example
// in bcaslot.cxx, otherwise type safe constants are preferred.
#define MAXROWCOUNT_DEFINE 1048576
#define MAXCOLCOUNT_DEFINE 1024

// Count values
const SCROW       MAXROWCOUNT    = MAXROWCOUNT_DEFINE;
const SCCOL       MAXCOLCOUNT    = MAXCOLCOUNT_DEFINE;
/// limiting to 10000 for now, problem with 32 bit builds for now
const SCTAB       MAXTABCOUNT    = 10000;
const SCCOLROW    MAXCOLROWCOUNT = MAXROWCOUNT;
// Maximum values
const SCROW       MAXROW         = MAXROWCOUNT - 1;
const SCCOL       MAXCOL         = MAXCOLCOUNT - 1;
const SCTAB       MAXTAB         = MAXTABCOUNT - 1;
const SCCOLROW    MAXCOLROW      = MAXROW;
// Limit the initial tab count to prevent users to set the count too high,
// which could cause the memory usage of blank documents to exceed the
// available system memory.
const SCTAB       MAXINITTAB = 1024;
const SCTAB       MININITTAB = 1;

// Special values
const SCTAB SC_TAB_APPEND     = SCTAB_MAX;
const SCTAB TABLEID_DOC       = SCTAB_MAX;  // entire document, e.g. protect
const SCROW SCROWS32K         = 32000;
const SCCOL SCCOL_REPEAT_NONE = SCCOL_MAX;
const SCROW SCROW_REPEAT_NONE = SCROW_MAX;

// For future reference, place in code where more than 64k rows would need a
// special handling:
// #if SC_ROWLIMIT_MORE_THAN_64K
// #error row limit 64k
// #endif
#if MAXROWCOUNT_DEFINE > 65536
#define SC_ROWLIMIT_MORE_THAN_64K 1
#else
#define SC_ROWLIMIT_MORE_THAN_64K 0
#endif
const SCROW SCROWS64K = 65536;

// === old stuff defines =====================================================

#define MAXROW_30   8191

#ifdef SC_LIMIT_ROWS
#undef MAXROWCOUNT_DEFINE
#define MAXROWCOUNT_DEFINE 8192
const SCROW W16MAXROWCOUNT = MAXROWCOUNT_DEFINE;
const SCROW W16MAXROW = W16MAXROWCOUNT - 1;
#define MAXROWCOUNT W16MAXROWCOUNT
#define MAXROW      W16MAXROW
#endif

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

inline SCCOL SanitizeCol( SCCOL nCol )
{
    return nCol < 0 ? 0 : (nCol > MAXCOL ? MAXCOL : nCol);
}

inline SCROW SanitizeRow( SCROW nRow )
{
    return nRow < 0 ? 0 : (nRow > MAXROW ? MAXROW : nRow);
}

inline SCTAB SanitizeTab( SCTAB nTab )
{
    return nTab < 0 ? 0 : (nTab > MAXTAB ? MAXTAB : nTab);
}

inline SCTAB SanitizeTab( SCTAB nTab, SCTAB nMaxTab )
{
    return nTab < 0 ? 0 : (nTab > nMaxTab ? nMaxTab : nTab);
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
// SCA_BITS is a convience for
// (SCA_VALID_TAB | SCA_VALID_COL | SCA_VALID_ROW | SCA_TAB_3D | SCA_TAB_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_COL_ABSOLUTE)
#define SCA_BITS            0x070F
// somewhat cheesy kludge to force the display of the document name even for
// local references.  Requires TAB_3D to be valid
#define SCA_FORCE_DOC       0x0800
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

    struct Details {
        formula::FormulaGrammar::AddressConvention  eConv;
        SCROW       nRow;
        SCCOL       nCol;
        inline Details( formula::FormulaGrammar::AddressConvention eConvP, SCROW nRowP, SCCOL nColP )
            : eConv( eConvP ), nRow( nRowP ), nCol( nColP )
            {}
        inline Details( formula::FormulaGrammar::AddressConvention eConvP, ScAddress const & rAddr )
            : eConv( eConvP ), nRow( rAddr.Row() ), nCol( rAddr.Col() )
            {}
        inline Details( formula::FormulaGrammar::AddressConvention eConvP)
            : eConv( eConvP ), nRow( 0 ), nCol( 0 )
            {}
        /* Use the formula::FormulaGrammar::AddressConvention associated with rAddr::Tab() */
        Details( const ScDocument* pDoc, const ScAddress & rAddr );
    };
    SC_DLLPUBLIC static const Details detailsOOOa1;

    struct ExternalInfo
    {
        OUString maTabName;
        sal_uInt16  mnFileId;
        bool        mbExternal;

        inline ExternalInfo() : mnFileId(0), mbExternal(false) {}
    };

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

    inline void Set( SCCOL nCol, SCROW nRow, SCTAB nTab );
    inline SCROW Row() const { return nRow; }
    inline SCCOL Col() const { return nCol; }
    inline SCTAB Tab() const { return nTab; }
    inline void SetRow( SCROW nRowP ) { nRow = nRowP; }
    inline void SetCol( SCCOL nColP ) { nCol = nColP; }
    inline void SetTab( SCTAB nTabP ) { nTab = nTabP; }
    inline void SetInvalid() { nRow = -1; nCol = -1; nTab = -1; }
    inline bool IsValid() const { return (nRow >= 0) && (nCol >= 0) && (nTab >= 0); }
    inline void PutInOrder( ScAddress& r );
    inline void IncRow( SCsROW n=1 ) { nRow = sal::static_int_cast<SCROW>(nRow + n); }
    inline void IncCol( SCsCOL n=1 ) { nCol = sal::static_int_cast<SCCOL>(nCol + n); }
    inline void IncTab( SCsTAB n=1 ) { nTab = sal::static_int_cast<SCTAB>(nTab + n); }
    inline void GetVars( SCCOL& nColP, SCROW& nRowP, SCTAB& nTabP ) const
    { nColP = nCol; nRowP = nRow; nTabP = nTab; }

    SC_DLLPUBLIC sal_uInt16 Parse( const String&, ScDocument* = NULL,
                  const Details& rDetails = detailsOOOa1,
                  ExternalInfo* pExtInfo = NULL,
                  const ::com::sun::star::uno::Sequence<
                    const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks = NULL );

    SC_DLLPUBLIC OUString Format( sal_uInt16 = 0, const ScDocument* = NULL,
                 const Details& rDetails = detailsOOOa1) const;

    // The document for the maximum defined sheet number
    SC_DLLPUBLIC bool Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* =NULL );
    inline bool operator==( const ScAddress& r ) const;
    inline bool operator!=( const ScAddress& r ) const;
    inline bool operator<( const ScAddress& r ) const;
    inline bool operator<=( const ScAddress& r ) const;
    inline bool operator>( const ScAddress& r ) const;
    inline bool operator>=( const ScAddress& r ) const;

    inline size_t hash() const;

    /// "A1" or "$A$1" or R1C1 or R[1]C[1]
    String GetColRowString( bool bAbsolute = false,
                            const Details& rDetails = detailsOOOa1) const;
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

/** Same behavior as the old sal_uInt32 nAddress < r.nAddress with encoded
    tab|col|row bit fields. */
inline bool ScAddress::operator<( const ScAddress& r ) const
{
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


inline size_t ScAddress::hash() const
{
    // Assume that there are not that many addresses with row > 2^16 AND column
    // > 2^8 AND sheet > 2^8 so we won't have too many collisions.
    if (nRow <= 0xffff)
        return (static_cast<size_t>(nTab) << 24) ^
            (static_cast<size_t>(nCol) << 16) ^ static_cast<size_t>(nRow);
    else
        return (static_cast<size_t>(nTab) << 28) ^
            (static_cast<size_t>(nCol) << 24) ^ static_cast<size_t>(nRow);
}

struct ScAddressHashFunctor
{
    size_t operator()( const ScAddress & rAdr ) const
    {
        return rAdr.hash();
    }
};

struct ScAddressEqualFunctor
{
    bool operator()( const ScAddress & rAdr1, const ScAddress & rAdr2 ) const
    {
        return rAdr1 == rAdr2;
    }
};

inline bool ValidAddress( const ScAddress& rAddr )
{
    return ValidCol(rAddr.Col()) && ValidRow(rAddr.Row()) && ValidTab(rAddr.Tab());
}

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
    inline void SetInvalid() { aStart.SetInvalid(); aEnd.SetInvalid(); }
    inline bool IsValid() const { return aStart.IsValid() && aEnd.IsValid(); }
    inline bool In( const ScAddress& ) const;   ///< is Address& in Range?
    inline bool In( const ScRange& ) const;     ///< is Range& in Range?

    SC_DLLPUBLIC sal_uInt16 Parse( const String&, ScDocument* = NULL,
                  const ScAddress::Details& rDetails = ScAddress::detailsOOOa1,
                  ScAddress::ExternalInfo* pExtInfo = NULL,
                  const ::com::sun::star::uno::Sequence<
                    const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks = NULL );

    SC_DLLPUBLIC sal_uInt16 ParseAny( const String&, ScDocument* = NULL,
                     const ScAddress::Details& rDetails = ScAddress::detailsOOOa1 );
    SC_DLLPUBLIC sal_uInt16 ParseCols( const String&, ScDocument* = NULL,
                     const ScAddress::Details& rDetails = ScAddress::detailsOOOa1 );
    SC_DLLPUBLIC sal_uInt16 ParseRows( const String&, ScDocument* = NULL,
                     const ScAddress::Details& rDetails = ScAddress::detailsOOOa1 );

    /** Parse an Excel style reference up to and including the sheet name
        separator '!', including detection of external documents and sheet
        names, and in case of MOOXML import the bracketed index is used to
        determine the actual document name passed in pExternalLinks. For
        internal references (resulting rExternDocName empty), aStart.nTab and
        aEnd.nTab are set, or -1 if sheet name not found.
        @param bOnlyAcceptSingle  If <TRUE/>, a 3D reference (Sheet1:Sheet2)
            encountered results in an error (NULL returned).
        @param pExternalLinks  pointer to ExternalLinkInfo sequence, may be
            NULL for non-filter usage, in which case indices such as [1] are
            not resolved.
        @returns
            Pointer to the position after '!' if successfully parsed, and
            rExternDocName, rStartTabName and/or rEndTabName filled if
            applicable. SCA_... flags set in nFlags.
            Or if no valid document and/or sheet header could be parsed the start
            position passed with pString.
            Or NULL if a 3D sheet header could be parsed but
            bOnlyAcceptSingle==true was given.
     */
    const sal_Unicode* Parse_XL_Header( const sal_Unicode* pString, const ScDocument* pDoc,
            String& rExternDocName, String& rStartTabName, String& rEndTabName, sal_uInt16& nFlags,
            bool bOnlyAcceptSingle,
            const ::com::sun::star::uno::Sequence<
                const ::com::sun::star::sheet::ExternalLinkInfo > * pExternalLinks = NULL );

    SC_DLLPUBLIC OUString Format(sal_uInt16 = 0, const ScDocument* = NULL,
                 const ScAddress::Details& rDetails = ScAddress::detailsOOOa1) const;

    inline void GetVars( SCCOL& nCol1, SCROW& nRow1, SCTAB& nTab1,
        SCCOL& nCol2, SCROW& nRow2, SCTAB& nTab2 ) const;
    // The document for the maximum defined sheet number
    SC_DLLPUBLIC bool Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* =NULL );
    SC_DLLPUBLIC void Justify();
    SC_DLLPUBLIC void ExtendTo( const ScRange& rRange );
    SC_DLLPUBLIC bool Intersects( const ScRange& ) const;    // do two ranges intersect?
    void PutInOrder();
    inline bool operator==( const ScRange& r ) const;
    inline bool operator!=( const ScRange& r ) const;
    inline bool operator<( const ScRange& r ) const;
    inline bool operator<=( const ScRange& r ) const;
    inline bool operator>( const ScRange& r ) const;
    inline bool operator>=( const ScRange& r ) const;

    /// Hash 2D area ignoring table number.
    inline size_t hashArea() const;
    /// Hash start column and start and end rows.
    inline size_t hashStartColumn() const;
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

/// Sort on upper left corner, if equal then use lower right too.
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


inline size_t ScRange::hashArea() const
{
    // Assume that there are not that many ranges with identical corners so we
    // won't have too many collisions. Also assume that more lower row and
    // column numbers are used so that there are not too many conflicts with
    // the columns hashed into the values, and that start row and column
    // usually don't exceed certain values. High bits are not masked off and
    // may overlap with lower bits of other values, e.g. if start column is
    // greater than assumed.
    return
        (static_cast<size_t>(aStart.Row()) << 26) ^ // start row <= 2^6
        (static_cast<size_t>(aStart.Col()) << 21) ^ // start column <= 2^5
        (static_cast<size_t>(aEnd.Col()) << 15) ^   // end column <= 2^6
        static_cast<size_t>(aEnd.Row());            // end row <= 2^15
}


inline size_t ScRange::hashStartColumn() const
{
    // Assume that for the start row more lower row numbers are used so that
    // there are not too many conflicts with the column hashed into the higher
    // values.
    return
        (static_cast<size_t>(aStart.Col()) << 24) ^ // start column <= 2^8
        (static_cast<size_t>(aStart.Row()) << 16) ^ // start row <= 2^8
        static_cast<size_t>(aEnd.Row());
}


struct ScRangeHashAreaFunctor
{
    size_t operator()( const ScRange & rRange ) const
    {
        return rRange.hashArea();
    }
};

struct ScRangeEqualFunctor
{
    bool operator()( const ScRange & rRange1, const ScRange & rRange2 ) const
    {
        return rRange1 == rRange2;
    }
};

inline bool ValidRange( const ScRange& rRange )
{
    return ValidAddress(rRange.aStart) && ValidAddress(rRange.aEnd);
}

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
    const ScRange&      GetRange( sal_uInt16 n ) const { return aRange[n]; }
    ScRange&            GetRange( sal_uInt16 n ) { return aRange[n]; }
    inline int operator==( const ScRangePair& ) const;
    inline int operator!=( const ScRangePair& ) const;
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

            String  GetRefString( ScDocument* pDoc, SCTAB nActTab,
                                  const ScAddress::Details& rDetails = ScAddress::detailsOOOa1) const;
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
        SCTAB nDefTab, ScRefAddress& rRefAddress,
        const ScAddress::Details& rDetails = ScAddress::detailsOOOa1,
        ScAddress::ExternalInfo* pExtInfo = NULL );

bool ConvertDoubleRef(ScDocument* pDoc, const String& rRefString,
        SCTAB nDefTab, ScRefAddress& rStartRefAddress,
        ScRefAddress& rEndRefAddress,
        const ScAddress::Details& rDetails = ScAddress::detailsOOOa1,
        ScAddress::ExternalInfo* pExtInfo = NULL );

/// append alpha representation of column to buffer
SC_DLLPUBLIC void ScColToAlpha( OUStringBuffer& rBuffer, SCCOL nCol);

inline void ScColToAlpha( OUString& rStr, SCCOL nCol)
{
    OUStringBuffer aBuf(2);
    ScColToAlpha( aBuf, nCol);
    rStr += aBuf.makeStringAndClear();
}

inline OUString ScColToAlpha( SCCOL nCol )
{
    OUStringBuffer aBuf(2);
    ScColToAlpha( aBuf, nCol);
    return aBuf.makeStringAndClear();
}

/// get column number of A..IV... string
bool AlphaToCol( SCCOL& rCol, const String& rStr);

#endif // SC_ADDRESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
