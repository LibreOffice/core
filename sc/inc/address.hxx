/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
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

#ifndef INCLUDED_SC_INC_ADDRESS_HXX
#define INCLUDED_SC_INC_ADDRESS_HXX

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <array>
#include <limits>
#include <ostream>

#include "scdllapi.h"
#include "types.hxx"
#include <formula/grammar.hxx>

#include <o3tl/typed_flags_set.hxx>
#include <o3tl/underlyingenumvalue.hxx>

namespace com { namespace sun { namespace star {
    namespace sheet {
        struct ExternalLinkInfo;
    }
}}}

namespace com { namespace sun { namespace star { namespace uno { template <typename > class Sequence; } } } }

class ScDocument;

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
// Maximum tiled rendering values
const SCROW       MAXTILEDROW    = 500000;
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

#define MAXROW_30   8191

[[nodiscard]] inline bool ValidCol( SCCOL nCol )
{
    return nCol >= 0 && nCol <= MAXCOL;
}

[[nodiscard]] inline bool ValidRow( SCROW nRow )
{
    return nRow >= 0 && nRow <= MAXROW;
}

[[nodiscard]] inline bool ValidTab( SCTAB nTab )
{
    return nTab >= 0 && nTab <= MAXTAB;
}

[[nodiscard]] inline bool ValidTab( SCTAB nTab, SCTAB nMaxTab )
{
    return nTab >= 0 && nTab <= nMaxTab;
}

[[nodiscard]] inline bool ValidColRow( SCCOL nCol, SCROW nRow )
{
    return ValidCol( nCol) && ValidRow( nRow);
}

[[nodiscard]] inline bool ValidColRowTab( SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    return ValidCol( nCol) && ValidRow( nRow) && ValidTab( nTab);
}

[[nodiscard]] inline SCCOL SanitizeCol( SCCOL nCol )
{
    return nCol < 0 ? 0 : std::min(nCol, MAXCOL);
}

[[nodiscard]] inline SCROW SanitizeRow( SCROW nRow )
{
    return nRow < 0 ? 0 : std::min(nRow, MAXROW);
}

[[nodiscard]] inline SCTAB SanitizeTab( SCTAB nTab )
{
    return nTab < 0 ? 0 : std::min(nTab, MAXTAB);
}

// The result of ConvertRef() is a bit group of the following:
enum class ScRefFlags : sal_uInt16
{
    ZERO          = 0x0000,
    COL_ABS       = 0x0001,
    ROW_ABS       = 0x0002,
    TAB_ABS       = 0x0004,
    TAB_3D        = 0x0008,
    COL2_ABS      = 0x0010,
    ROW2_ABS      = 0x0020,
    TAB2_ABS      = 0x0040,
    TAB2_3D       = 0x0080,
    ROW_VALID     = 0x0100,
    COL_VALID     = 0x0200,
    TAB_VALID     = 0x0400,
    // BITS for convenience
    BITS          = COL_ABS | ROW_ABS | TAB_ABS | TAB_3D
                    | ROW_VALID | COL_VALID | TAB_VALID,
    // somewhat cheesy kludge to force the display of the document name even for
    // local references.  Requires TAB_3D to be valid
    FORCE_DOC     = 0x0800,
    ROW2_VALID    = 0x1000,
    COL2_VALID    = 0x2000,
    TAB2_VALID    = 0x4000,
    VALID         = 0x8000,

    TAB_ABS_3D    = TAB_ABS | TAB_3D,

    ADDR_ABS      = VALID | COL_ABS | ROW_ABS | TAB_ABS,

    RANGE_ABS     = ADDR_ABS | COL2_ABS | ROW2_ABS | TAB2_ABS,

    ADDR_ABS_3D   = ADDR_ABS | TAB_3D,
    RANGE_ABS_3D  = RANGE_ABS | TAB_3D
};

namespace o3tl
{
    template<> struct typed_flags<ScRefFlags> : is_typed_flags<ScRefFlags, 0xffff> {};
}
inline void applyStartToEndFlags(ScRefFlags &target,const ScRefFlags source)
{
    target |= ScRefFlags(o3tl::underlyingEnumValue(source) << 4);
}
inline void applyStartToEndFlags(ScRefFlags &target)
{
    target |= ScRefFlags(o3tl::underlyingEnumValue(target) << 4);
}

//  ScAddress
class SAL_WARN_UNUSED ScAddress
{
private:
    // Even if the fields are in the order "row, column, tab", in all (?) the ScAddress and
    // ScDocument APIs that take separate row, column, and tab parameters, the parameters are in the
    // order "column, row, tab", which matches the most common (A1) address syntax, if you ignore
    // the sheet (tab). Don't let this confuse you, like it confused me for a while.

    SCROW   nRow;
    SCCOL   nCol;
    SCTAB   nTab;

public:

    enum Uninitialized      { UNINITIALIZED };
    enum InitializeInvalid  { INITIALIZE_INVALID };

    struct Details
    {
        formula::FormulaGrammar::AddressConvention  eConv;
        SCROW       nRow;
        SCCOL       nCol;

        Details( formula::FormulaGrammar::AddressConvention eConvP, SCROW nRowP, SCCOL nColP ) :
            eConv(eConvP), nRow(nRowP), nCol(nColP)
        {}
        Details( formula::FormulaGrammar::AddressConvention eConvP, ScAddress const & rAddr ) :
            eConv(eConvP), nRow(rAddr.Row()),  nCol(rAddr.Col())
        {}
        Details( formula::FormulaGrammar::AddressConvention eConvP) :
            eConv(eConvP), nRow(0), nCol(0)
        {}
        /* Use the formula::FormulaGrammar::AddressConvention associated with rAddr::Tab() */
        Details( const ScDocument* pDoc, const ScAddress & rAddr );
    };
    SC_DLLPUBLIC static const Details detailsOOOa1;

    struct ExternalInfo
    {
        OUString    maTabName;
        sal_uInt16  mnFileId;
        bool        mbExternal;

        ExternalInfo() :
            mnFileId(0), mbExternal(false)
        {}
    };

    ScAddress() :
        nRow(0), nCol(0), nTab(0)
    {}
    ScAddress( SCCOL nColP, SCROW nRowP, SCTAB nTabP ) :
        nRow(nRowP), nCol(nColP), nTab(nTabP)
    {}
    /** Yes, it is what it seems to be: Uninitialized. May be used for
        performance reasons if it is initialized by other means. */
    ScAddress( Uninitialized )
    {}
    ScAddress( InitializeInvalid ) :
        nRow(-1), nCol(-1), nTab(-1)
    {}
    ScAddress( const ScAddress& rAddress ) :
        nRow(rAddress.nRow), nCol(rAddress.nCol), nTab(rAddress.nTab)
    {}
    inline ScAddress& operator=( const ScAddress& rAddress );

    inline void Set( SCCOL nCol, SCROW nRow, SCTAB nTab );

    SCROW Row() const
    {
        return nRow;
    }

    SCCOL Col() const
    {
        return nCol;
    }
    SCTAB Tab() const
    {
        return nTab;
    }
    void SetRow( SCROW nRowP )
    {
        nRow = nRowP;
    }
    void SetCol( SCCOL nColP )
    {
        nCol = nColP;
    }
    void SetTab( SCTAB nTabP )
    {
        nTab = nTabP;
    }
    void SetInvalid()
    {
        nRow = -1;
        nCol = -1;
        nTab = -1;
    }
    bool IsValid() const
    {
        return (nRow >= 0) && (nCol >= 0) && (nTab >= 0);
    }

    inline void PutInOrder( ScAddress& rAddress );

    void IncRow( SCROW nDelta = 1 )
    {
        nRow = sal::static_int_cast<SCROW>(nRow + nDelta);
    }
    void IncCol( SCCOL nDelta = 1 )
    {
        nCol = sal::static_int_cast<SCCOL>(nCol + nDelta);
    }
    void IncTab( SCTAB nDelta = 1 )
    {
        nTab = sal::static_int_cast<SCTAB>(nTab + nDelta);
    }
    void GetVars( SCCOL& nColP, SCROW& nRowP, SCTAB& nTabP ) const
    {
        nColP = nCol;
        nRowP = nRow;
        nTabP = nTab;
    }

    /**
        @param  pSheetEndPos
                If given and Parse() successfully parsed a sheet name it returns
                the end position (exclusive) behind the sheet name AND a
                following sheet name separator. This independent of whether the
                resulting reference is fully valid or not.
     */
    SC_DLLPUBLIC ScRefFlags Parse(
                    const OUString&, const ScDocument* = nullptr,
                    const Details& rDetails = detailsOOOa1,
                    ExternalInfo* pExtInfo = nullptr,
                    const css::uno::Sequence<css::sheet::ExternalLinkInfo>* pExternalLinks = nullptr,
                    sal_Int32* pSheetEndPos = nullptr,
                    const OUString* pErrRef = nullptr );

    SC_DLLPUBLIC void Format( OStringBuffer& r, ScRefFlags nFlags,
                                  const ScDocument* pDocument = nullptr,
                                  const Details& rDetails = detailsOOOa1) const;

    SC_DLLPUBLIC OUString Format( ScRefFlags nFlags,
                                  const ScDocument* pDocument = nullptr,
                                  const Details& rDetails = detailsOOOa1) const;

    /**
        @param  rErrorPos
                If FALSE is returned, the positions contain <0 or >MAX...
                values if shifted out of bounds.
        @param  pDocument
                The document for the maximum defined sheet number.
     */
    [[nodiscard]] SC_DLLPUBLIC bool Move( SCCOL nDeltaX, SCROW nDeltaY, SCTAB nDeltaZ,
            ScAddress& rErrorPos, const ScDocument* pDocument = nullptr );

    inline bool operator==( const ScAddress& rAddress ) const;
    inline bool operator!=( const ScAddress& rAddress ) const;
    inline bool operator<( const ScAddress& rAddress ) const;
    inline bool operator<=( const ScAddress& rAddress ) const;
    inline bool lessThanByRow( const ScAddress& rAddress ) const;

    inline size_t hash() const;

    /**
     * Create a human-readable string representation of the cell address.  You
     * cannot specify precise formatting with this method; use Format() if you
     * need to specify how the address needs to be formatted.
     *
     * The address string does not display sheet name.
     *
     * @return human-readable string representation of the cell address.
     */
    OUString GetColRowString() const;
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScAddress& rAddress)
{
    stream <<
        rAddress.Tab()+1 << "!"
        "R" << rAddress.Row()+1 <<
        "C" << rAddress.Col()+1;

    return stream;
}

inline void ScAddress::PutInOrder( ScAddress& rAddress )
{
    if ( rAddress.Col() < Col() )
    {
        SCCOL nTmp = rAddress.Col();
        rAddress.SetCol( Col() );
        SetCol( nTmp );
    }
    if ( rAddress.Row() < Row() )
    {
        SCROW nTmp = rAddress.Row();
        rAddress.SetRow( Row() );
        SetRow( nTmp );
    }
    if ( rAddress.Tab() < Tab() )
    {
        SCTAB nTmp = rAddress.Tab();
        rAddress.SetTab( Tab() );
        SetTab( nTmp );
    }
}

inline void ScAddress::Set( SCCOL nColP, SCROW nRowP, SCTAB nTabP )
{
    nCol = nColP;
    nRow = nRowP;
    nTab = nTabP;
}

inline ScAddress& ScAddress::operator=( const ScAddress& rAddress )
{
    nCol = rAddress.nCol;
    nRow = rAddress.nRow;
    nTab = rAddress.nTab;
    return *this;
}

inline bool ScAddress::operator==( const ScAddress& rAddress ) const
{
    return nRow == rAddress.nRow && nCol == rAddress.nCol && nTab == rAddress.nTab;
}

inline bool ScAddress::operator!=( const ScAddress& rAddress ) const
{
    return !operator==( rAddress );
}

/** Less than ordered by tab,col,row. */
inline bool ScAddress::operator<( const ScAddress& rAddress ) const
{
    if (nTab == rAddress.nTab)
    {
        if (nCol == rAddress.nCol)
            return nRow < rAddress.nRow;
        else
            return nCol < rAddress.nCol;
    }
    else
        return nTab < rAddress.nTab;
}

inline bool ScAddress::operator<=( const ScAddress& rAddress ) const
{
    return operator<( rAddress ) || operator==( rAddress );
}

/** Less than ordered by tab,row,col as needed by row-wise import/export */
inline bool ScAddress::lessThanByRow( const ScAddress& rAddress ) const
{
    if (nTab == rAddress.nTab)
    {
        if (nRow == rAddress.nRow)
            return nCol < rAddress.nCol;
        else
            return nRow < rAddress.nRow;
    }
    else
        return nTab < rAddress.nTab;
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
    size_t operator()( const ScAddress & rAddress ) const
    {
        return rAddress.hash();
    }
};

inline bool ValidAddress( const ScAddress& rAddress )
{
    return ValidCol(rAddress.Col()) && ValidRow(rAddress.Row()) && ValidTab(rAddress.Tab());
}

//  ScRange
class SAL_WARN_UNUSED ScRange final
{
public:
    ScAddress aStart;
    ScAddress aEnd;

    ScRange() :
        aStart(), aEnd()
    {}

    ScRange( ScAddress::Uninitialized eUninitialized ) :
        aStart( eUninitialized ), aEnd( eUninitialized )
    {}
    ScRange( ScAddress::InitializeInvalid eInvalid ) :
        aStart( eInvalid ), aEnd( eInvalid )
    {}
    ScRange( const ScAddress& aInputStart, const ScAddress& aInputEnd ) :
        aStart( aInputStart ), aEnd( aInputEnd )
    {
        aStart.PutInOrder( aEnd );
    }
    ScRange( const ScRange& rRange ) :
        aStart( rRange.aStart ), aEnd( rRange.aEnd )
    {}
    ScRange( const ScAddress& rRange ) :
        aStart( rRange ), aEnd( rRange )
    {}
    ScRange( SCCOL nCol, SCROW nRow, SCTAB nTab ) :
        aStart( nCol, nRow, nTab ), aEnd( aStart )
    {}
    ScRange( SCCOL nCol1, SCROW nRow1, SCTAB nTab1, SCCOL nCol2, SCROW nRow2, SCTAB nTab2 ) :
        aStart( nCol1, nRow1, nTab1 ), aEnd( nCol2, nRow2, nTab2 )
    {}

    ScRange& operator=( const ScRange& rRange )
    {
        aStart = rRange.aStart;
        aEnd = rRange.aEnd;
        return *this;
    }
    ScRange& operator=( const ScAddress& rPos )
    {
        aStart = aEnd = rPos;
        return *this;
    }
    void SetInvalid()
    {
        aStart.SetInvalid();
        aEnd.SetInvalid();
    }
    bool IsValid() const
    {
        return aStart.IsValid() && aEnd.IsValid();
    }
    inline bool In( const ScAddress& ) const;   ///< is Address& in Range?
    inline bool In( const ScRange& ) const;     ///< is Range& in Range?

    SC_DLLPUBLIC ScRefFlags Parse( const OUString&, const ScDocument* = nullptr,
                                   const ScAddress::Details& rDetails = ScAddress::detailsOOOa1,
                                   ScAddress::ExternalInfo* pExtInfo = nullptr,
                                   const css::uno::Sequence<css::sheet::ExternalLinkInfo>* pExternalLinks = nullptr,
                                   const OUString* pErrRef = nullptr );

    SC_DLLPUBLIC ScRefFlags ParseAny( const OUString&, const ScDocument*,
                                      const ScAddress::Details& rDetails = ScAddress::detailsOOOa1 );
    SC_DLLPUBLIC ScRefFlags ParseCols( const OUString&,
                                       const ScAddress::Details& rDetails = ScAddress::detailsOOOa1 );
    SC_DLLPUBLIC void ParseRows( const OUString&,
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
        @param pErrRef  pointer to "#REF!" string if to be accepted.
        @returns
            Pointer to the position after '!' if successfully parsed, and
            rExternDocName, rStartTabName and/or rEndTabName filled if
            applicable. ScRefFlags::... flags set in nFlags.
            Or if no valid document and/or sheet header could be parsed the start
            position passed with pString.
            Or NULL if a 3D sheet header could be parsed but
            bOnlyAcceptSingle==true was given.
     */
    const sal_Unicode* Parse_XL_Header( const sal_Unicode* pString, const ScDocument* pDocument,
                                        OUString& rExternDocName, OUString& rStartTabName,
                                        OUString& rEndTabName, ScRefFlags& nFlags,
                                        bool bOnlyAcceptSingle,
                                        const css::uno::Sequence<css::sheet::ExternalLinkInfo>* pExternalLinks = nullptr,
                                        const OUString* pErrRef = nullptr );

    /** Returns string with formatted cell range from aStart to aEnd,
        according to provided address convention.
        @param nFlags
            Cell reference flags
        @param pDocument
            Pointer to document which is used for example to get tab names.
        @param rDetails
            Provide information about required address convention.
            Supported address conventions are:
                CONV_OOO      'doc'#sheet.A1:sheet2.B2
                CONV_XL_A1,   [doc]sheet:sheet2!A1:B2
                CONV_XL_OOX,  [#]sheet:sheet2!A1:B2
                CONV_XL_R1C1, [doc]sheet:sheet2!R1C1:R2C2
        @param bFullAddressNotation
            If TRUE, the full address notation will be used.
            For example in case all columns are used, "A1:AMJ177" is full address notation
            and "1:177" is shortened address notation.
        @returns
            String contains formatted cell range in address convention
     */
    SC_DLLPUBLIC OUString Format( ScRefFlags nFlags = ScRefFlags::ZERO,
                                  const ScDocument* pDocument = nullptr,
                                  const ScAddress::Details& rDetails = ScAddress::detailsOOOa1,
                                  bool bFullAddressNotation = false ) const;

    inline void GetVars( SCCOL& nCol1, SCROW& nRow1, SCTAB& nTab1,
                         SCCOL& nCol2, SCROW& nRow2, SCTAB& nTab2 ) const;
    SC_DLLPUBLIC void PutInOrder();

    /**
        @param  rErrorRange
                If FALSE is returned, the positions contain <0 or >MAX...
                values if shifted out of bounds.
        @param  pDocument
                The document for the maximum defined sheet number.
     */
    [[nodiscard]] SC_DLLPUBLIC bool Move( SCCOL aDeltaX, SCROW aDeltaY, SCTAB aDeltaZ,
            ScRange& rErrorRange, const ScDocument* pDocument = nullptr );

    /** Same as Move() but with sticky end col/row anchors. */
    [[nodiscard]] SC_DLLPUBLIC bool MoveSticky( SCCOL aDeltaX, SCROW aDeltaY, SCTAB aDeltaZ,
            ScRange& rErrorRange );

    SC_DLLPUBLIC void IncColIfNotLessThan(SCCOL nStartCol, SCCOL nOffset);
    SC_DLLPUBLIC void IncRowIfNotLessThan(SCROW nStartRow, SCROW nOffset);

    SC_DLLPUBLIC void ExtendTo( const ScRange& rRange );
    SC_DLLPUBLIC bool Intersects( const ScRange& rRange ) const;    // do two ranges intersect?

    ScRange Intersection( const ScRange& rOther ) const;

    /// If maximum end column should not be adapted during reference update.
    inline bool IsEndColSticky() const;
    /// If maximum end row should not be adapted during reference update.
    inline bool IsEndRowSticky() const;

    /** Increment or decrement end column unless sticky or until it becomes
        sticky. Checks if the range encompasses at least two columns so should
        be called before adjusting the start column. */
    void IncEndColSticky( SCCOL nDelta );

    /** Increment or decrement end row unless sticky or until it becomes
        sticky. Checks if the range encompasses at least two rows so should
        be called before adjusting the start row. */
    void IncEndRowSticky( SCROW nDelta );

    inline bool operator==( const ScRange& rRange ) const;
    inline bool operator!=( const ScRange& rRange ) const;
    inline bool operator<( const ScRange& rRange ) const;
    inline bool operator<=( const ScRange& rRange ) const;

    /// Hash 2D area ignoring table number.
    inline size_t hashArea() const;
    /// Hash start column and start and end rows.
    inline size_t hashStartColumn() const;
};

// For use in SAL_DEBUG etc. Output format not guaranteed to be stable.
template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(std::basic_ostream<charT, traits> & stream, const ScRange& rRange)
{
    stream << rRange.aStart;
    if (rRange.aEnd != rRange.aStart)
    {
        stream << ":";
        if (rRange.aEnd.Tab() != rRange.aStart.Tab())
            stream << rRange.aEnd;
        else
            stream <<
                "R" << rRange.aEnd.Row()+1 <<
                "C" << rRange.aEnd.Col()+1;
    }

    return stream;
}

inline void ScRange::GetVars( SCCOL& nCol1, SCROW& nRow1, SCTAB& nTab1,
                              SCCOL& nCol2, SCROW& nRow2, SCTAB& nTab2 ) const
{
    aStart.GetVars( nCol1, nRow1, nTab1 );
    aEnd.GetVars( nCol2, nRow2, nTab2 );
}

inline bool ScRange::IsEndColSticky() const
{
    // Only in an actual column range, i.e. not if both columns are MAXCOL.
    return aEnd.Col() == MAXCOL && aStart.Col() < aEnd.Col();
}

inline bool ScRange::IsEndRowSticky() const
{
    // Only in an actual row range, i.e. not if both rows are MAXROW.
    return aEnd.Row() == MAXROW && aStart.Row() < aEnd.Row();
}

inline bool ScRange::operator==( const ScRange& rRange ) const
{
    return ( (aStart == rRange.aStart) && (aEnd == rRange.aEnd) );
}

inline bool ScRange::operator!=( const ScRange& rRange ) const
{
    return !operator==( rRange );
}

/// Sort on upper left corner tab,col,row, if equal then use lower right too.
inline bool ScRange::operator<( const ScRange& r ) const
{
    return aStart < r.aStart || (aStart == r.aStart && aEnd < r.aEnd) ;
}

inline bool ScRange::operator<=( const ScRange& rRange ) const
{
    return operator<( rRange ) || operator==( rRange );
}

inline bool ScRange::In( const ScAddress& rAddress ) const
{
    return
        aStart.Col() <= rAddress.Col() && rAddress.Col() <= aEnd.Col() &&
        aStart.Row() <= rAddress.Row() && rAddress.Row() <= aEnd.Row() &&
        aStart.Tab() <= rAddress.Tab() && rAddress.Tab() <= aEnd.Tab();
}

inline bool ScRange::In( const ScRange& rRange ) const
{
    return
        aStart.Col() <= rRange.aStart.Col() && rRange.aEnd.Col() <= aEnd.Col() &&
        aStart.Row() <= rRange.aStart.Row() && rRange.aEnd.Row() <= aEnd.Row() &&
        aStart.Tab() <= rRange.aStart.Tab() && rRange.aEnd.Tab() <= aEnd.Tab();
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
        (static_cast<size_t>(aEnd.Col())   << 15) ^ // end column <= 2^6
         static_cast<size_t>(aEnd.Row());           // end row <= 2^15
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

inline bool ValidRange( const ScRange& rRange )
{
    return ValidAddress(rRange.aStart) && ValidAddress(rRange.aEnd);
}

//  ScRangePair
class SAL_WARN_UNUSED ScRangePair final
{
private:
    std::array<ScRange,2> aRange;

public:
    ScRangePair( const ScRangePair& r )
    {
        aRange[0] = r.aRange[0];
        aRange[1] = r.aRange[1];
    }
    ScRangePair( const ScRange& rRange1, const ScRange& rRange2 )
    {
        aRange[0] = rRange1;
        aRange[1] = rRange2;
    }

    inline ScRangePair& operator= ( const ScRangePair& rRange );
    const ScRange& GetRange( sal_uInt16 n ) const
    {
        return aRange[n];
    }
    ScRange& GetRange( sal_uInt16 n )
    {
        return aRange[n];
    }
};

inline ScRangePair& ScRangePair::operator= ( const ScRangePair& rRange )
{
    aRange[0] = rRange.aRange[0];
    aRange[1] = rRange.aRange[1];
    return *this;
}

//  ScRefAddress
class SAL_WARN_UNUSED ScRefAddress
{
private:
    ScAddress           aAdr;
    bool                bRelCol;
    bool                bRelRow;
    bool                bRelTab;
public:
    ScRefAddress() :
        bRelCol(false), bRelRow(false), bRelTab(false)
    {}
    ScRefAddress( SCCOL nCol, SCROW nRow, SCTAB nTab ) :
        aAdr(nCol, nRow, nTab),
        bRelCol(false), bRelRow(false), bRelTab(false)
    {}
    ScRefAddress( const ScRefAddress& rRef ) :
        aAdr(rRef.aAdr), bRelCol(rRef.bRelCol), bRelRow(rRef.bRelRow),
        bRelTab(rRef.bRelTab)
    {}

    inline ScRefAddress& operator=( const ScRefAddress& );

    bool IsRelCol() const
    {
        return bRelCol;
    }
    bool IsRelRow() const
    {
        return bRelRow;
    }
    bool IsRelTab() const
    {
        return bRelTab;
    }

    void SetRelCol(bool bNewRelCol)
    {
        bRelCol = bNewRelCol;
    }
    void SetRelRow(bool bNewRelRow)
    {
        bRelRow = bNewRelRow;
    }
    void SetRelTab(bool bNewRelTab)
    {
        bRelTab = bNewRelTab;
    }

    inline void Set( const ScAddress& rAdr,
                     bool bNewRelCol, bool bNewRelRow, bool bNewRelTab );
    inline void Set( SCCOL nNewCol, SCROW nNewRow, SCTAB nNewTab,
                     bool bNewRelCol, bool bNewRelRow, bool bNewRelTab );

    const ScAddress& GetAddress() const
    {
        return aAdr;
    }

    SCCOL Col() const
    {
        return aAdr.Col();
    }
    SCROW Row() const
    {
        return aAdr.Row();
    }
    SCTAB Tab() const
    {
        return aAdr.Tab();
    }

    inline bool operator == ( const ScRefAddress& r ) const;

    OUString  GetRefString( const ScDocument* pDocument, SCTAB nActTab,
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

inline bool ScRefAddress::operator==( const ScRefAddress& rRefAddress ) const
{
    return aAdr == rRefAddress.aAdr &&
           bRelCol == rRefAddress.bRelCol &&
           bRelRow == rRefAddress.bRelRow &&
           bRelTab == rRefAddress.bRelTab;
}

// Global functions

// Special values for cells always broadcasting or listening (ScRecalcMode::ALWAYS
// and the like).
#define BCA_BRDCST_ALWAYS ScAddress( 0, SCROW_MAX, 0 )
#define BCA_LISTEN_ALWAYS ScRange( BCA_BRDCST_ALWAYS, BCA_BRDCST_ALWAYS )

template< typename T > void PutInOrder( T& nStart, T& nEnd )
{
    if (nEnd < nStart)
    {
        std::swap(nStart, nEnd);
    }
}

bool ConvertSingleRef( const ScDocument* pDocument, const OUString& rRefString,
                       SCTAB nDefTab, ScRefAddress& rRefAddress,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo = nullptr );

bool ConvertDoubleRef( const ScDocument* pDocument, const OUString& rRefString,
                       SCTAB nDefTab, ScRefAddress& rStartRefAddress,
                       ScRefAddress& rEndRefAddress,
                       const ScAddress::Details& rDetails,
                       ScAddress::ExternalInfo* pExtInfo = nullptr );

/// append alpha representation of column to buffer
SC_DLLPUBLIC void ScColToAlpha( OUStringBuffer& rBuffer, SCCOL nCol);

inline void ScColToAlpha( OUString& rStr, SCCOL nCol)
{
    OUStringBuffer aBuf(2);
    ScColToAlpha( aBuf, nCol);
    rStr += aBuf;
}

inline OUString ScColToAlpha( SCCOL nCol )
{
    OUStringBuffer aBuf(2);
    ScColToAlpha( aBuf, nCol);
    return aBuf.makeStringAndClear();
}

/// get column number of A..IV... string
bool AlphaToCol( SCCOL& rCol, const OUString& rStr);

#endif // INCLUDED_SC_INC_ADDRESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
