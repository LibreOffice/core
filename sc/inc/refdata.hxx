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

#ifndef INCLUDED_SC_INC_REFDATA_HXX
#define INCLUDED_SC_INC_REFDATA_HXX

#include "address.hxx"
#include "scdllapi.h"
#include "calcmacros.hxx"

struct ScSheetLimits;

/// Single reference (one address) into the sheet
struct SC_DLLPUBLIC ScSingleRefData
{
private:
    SCCOL mnCol;
    SCROW mnRow;
    SCTAB mnTab;

    union
    {
        sal_uInt8 mnFlagValue;
        struct
        {
            bool bColRel     :1;
            bool bColDeleted :1;
            bool bRowRel     :1;
            bool bRowDeleted :1;
            bool bTabRel     :1;
            bool bTabDeleted :1;
            bool bFlag3D     :1;     ///< 3D-Ref
            bool bRelName    :1;     ///< Reference derived from RangeName with relative values
        } Flags;
    };

public:
    /// No default ctor, because used in ScRawToken union, set InitFlags!
    void InitFlags() { mnFlagValue = 0; }    ///< all FALSE
    /// InitAddress: InitFlags and set address
    void InitAddress( const ScAddress& rAdr );
    void InitAddress( SCCOL nCol, SCROW nRow, SCTAB nTab );
    /// InitAddressRel: InitFlags and set address, everything relative to rPos
    void InitAddressRel( const ScDocument& rDoc, const ScAddress& rAdr, const ScAddress& rPos );
    /// InitFlags and set address, relative to rPos if rRef says so.
    void InitFromRefAddress( const ScDocument& rDoc, const ScRefAddress& rRef, const ScAddress& rPos );
    sal_uInt8 FlagValue() const { return mnFlagValue;}

    void SetColRel( bool bVal ) { Flags.bColRel = bVal; }
    bool IsColRel() const { return Flags.bColRel; }
    void SetRowRel( bool bVal ) { Flags.bRowRel = bVal; }
    bool IsRowRel() const { return Flags.bRowRel; }
    void SetTabRel( bool bVal ) { Flags.bTabRel = bVal; }
    bool IsTabRel() const { return Flags.bTabRel; }

    void SetAbsCol( SCCOL nVal );
    void SetRelCol( SCCOL nVal );
    void IncCol( SCCOL nInc );
    void SetAbsRow( SCROW nVal );
    void SetRelRow( SCROW nVal );
    void IncRow( SCROW nInc );
    void SetAbsTab( SCTAB nVal );
    void SetRelTab( SCTAB nVal );
    void IncTab( SCTAB nInc );

    void SetColDeleted( bool bVal );
    bool IsColDeleted() const { return Flags.bColDeleted;}
    void SetRowDeleted( bool bVal );
    bool IsRowDeleted() const { return Flags.bRowDeleted;}
    void SetTabDeleted( bool bVal );
    bool IsTabDeleted() const { return Flags.bTabDeleted;}
    bool IsDeleted() const;

    void SetFlag3D( bool bVal ) { Flags.bFlag3D = bVal; }
    bool IsFlag3D() const { return Flags.bFlag3D; }
    void SetRelName( bool bVal ) { Flags.bRelName = bVal; }
    bool IsRelName() const { return Flags.bRelName; }

    bool Valid(const ScDocument& rDoc) const;
    bool ColValid(const ScDocument& rDoc) const;
    bool RowValid(const ScDocument& rDoc) const;
    bool TabValid() const;
    /** In external references nTab is -1 if the external document was not
        loaded but the sheet was cached, or >=0 if the external document was
        loaded. */
    bool ValidExternal(const ScDocument& rDoc) const;

    ScAddress toAbs( const ScSheetLimits& rLimits, const ScAddress& rPos ) const;
    ScAddress toAbs( const ScDocument& rDoc, const ScAddress& rPos ) const;
    void SetAddress( const ScSheetLimits& rLimits, const ScAddress& rAddr, const ScAddress& rPos );
    SCROW Row() const;
    SCCOL Col() const;
    SCTAB Tab() const;

    /** Adjust ordering (front-top-left/rear-bottom-right) to a new position. */
    static void PutInOrder( ScSingleRefData& rRef1, ScSingleRefData& rRef2, const ScAddress& rPos );

    bool operator==( const ScSingleRefData& ) const;
    bool operator!=( const ScSingleRefData& ) const;

#if DEBUG_FORMULA_COMPILER
    void Dump( int nIndent = 0 ) const;
#endif
};

/// Complex reference (a range) into the sheet
struct ScComplexRefData
{
    ScSingleRefData Ref1;
    ScSingleRefData Ref2;

    void InitFlags()
        { Ref1.InitFlags(); Ref2.InitFlags(); }
    void InitRange( const ScRange& rRange )
        {
            Ref1.InitAddress( rRange.aStart );
            Ref2.InitAddress( rRange.aEnd );
        }
    void InitRangeRel( const ScDocument& rDoc, const ScRange& rRange, const ScAddress& rPos )
        {
            Ref1.InitAddressRel( rDoc, rRange.aStart, rPos );
            Ref2.InitAddressRel( rDoc, rRange.aEnd, rPos );
        }
    void InitRange( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
        {
            Ref1.InitAddress( nCol1, nRow1, nTab1 );
            Ref2.InitAddress( nCol2, nRow2, nTab2 );
        }

    /// InitFlags and set range, relative to rPos if rRef1 and rRef2 say so.
    void InitFromRefAddresses( const ScDocument& rDoc, const ScRefAddress& rRef1, const ScRefAddress& rRef2, const ScAddress& rPos );

    bool Valid(const ScDocument& rDoc) const;

    /** In external references nTab is -1 for the start tab and -1 for the end
        tab if one sheet and the external document was not loaded but sheet was
        cached, or >=0 also if more than one sheets. */
    bool ValidExternal(const ScDocument& rDoc) const;

    /** Whether this references entire columns, A:A */
    bool IsEntireCol() const
    {
        // Both row anchors must be absolute.
        return Ref1.Row() == 0 && Ref2.Row() == MAXROW && !Ref1.IsRowRel() && !Ref2.IsRowRel();
    }

    /** Whether this references entire rows, 1:1 */
    bool IsEntireRow() const
    {
        // Both column anchors must be absolute.
        return Ref1.Col() == 0 && Ref2.Col() == MAXCOL && !Ref1.IsColRel() && !Ref2.IsColRel();
    }

    SC_DLLPUBLIC ScRange toAbs( const ScSheetLimits& rLimits, const ScAddress& rPos ) const;
    SC_DLLPUBLIC ScRange toAbs( const ScDocument& rDoc, const ScAddress& rPos ) const;

    /** Set a new range, assuming that the ordering of the range matches the
        ordering of the reference data flags already set. */
    void SetRange( const ScSheetLimits& rLimits, const ScRange& rRange, const ScAddress& rPos );

    /** Adjust ordering (front-top-left/rear-bottom-right) to a new position. */
    void PutInOrder( const ScAddress& rPos );

    bool operator==( const ScComplexRefData& r ) const
        { return Ref1 == r.Ref1 && Ref2 == r.Ref2; }
    /** Enlarge range if reference passed is not within existing range.
        ScAddress position is used to calculate absolute references from
        relative references. */
    ScComplexRefData& Extend( const ScSheetLimits& rLimits, const ScSingleRefData & rRef, const ScAddress & rPos );
    ScComplexRefData& Extend( const ScSheetLimits& rLimits, const ScComplexRefData & rRef, const ScAddress & rPos );

    /** Increment or decrement end column unless or until sticky.
        @see ScRange::IncEndColSticky()
        @return TRUE if changed. */
    bool IncEndColSticky( const ScDocument& rDoc, SCCOL nDelta, const ScAddress& rPos );

    /** Increment or decrement end row unless or until sticky.
        @see ScRange::IncEndRowSticky()
        @return TRUE if changed. */
    bool IncEndRowSticky( const ScDocument& rDoc, SCROW nDelta, const ScAddress& rPos );

    bool IsDeleted() const;

#if DEBUG_FORMULA_COMPILER
    void Dump( int nIndent = 0 ) const;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
