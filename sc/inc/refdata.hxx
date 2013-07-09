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

#ifndef SC_REFDATA_HXX
#define SC_REFDATA_HXX

#include "global.hxx"
#include "address.hxx"
#include "scdllapi.h"


/// Ref-Flags for old (until release 3.1) documents

struct OldSingleRefBools
{
    sal_uInt8    bRelCol;    ///< Flag values (see further down), 2 bits each in file format
    sal_uInt8    bRelRow;
    sal_uInt8    bRelTab;
    sal_uInt8    bOldFlag3D; ///< two sal_Bool flags (see further down)
};

/// Single reference (one address) into the sheet
struct SC_DLLPUBLIC ScSingleRefData
{
    SCsCOL  nCol;       ///< Absolute values
    SCsROW  nRow;
    SCsTAB  nTab;
    SCsCOL  nRelCol;    ///< Values relative to the position
    SCsROW  nRelRow;
    SCsTAB  nRelTab;

    union
    {
        sal_Bool bFlags;
        struct
        {
            sal_Bool    bColRel     :1;
            sal_Bool    bColDeleted :1;
            sal_Bool    bRowRel     :1;
            sal_Bool    bRowDeleted :1;
            sal_Bool    bTabRel     :1;
            sal_Bool    bTabDeleted :1;
            sal_Bool    bFlag3D     :1;     ///< 3D-Ref
            sal_Bool    bRelName    :1;     ///< Reference derived from RangeName with relative values
        }Flags;
    };

    /// No default ctor, because used in ScRawToken union, set InitFlags!
    inline  void InitFlags() { bFlags = 0; }    ///< all FALSE
    /// InitAddress: InitFlags and set address
    inline  void InitAddress( const ScAddress& rAdr );
    inline  void InitAddress( SCCOL nCol, SCROW nRow, SCTAB nTab );
    /// InitAddressRel: InitFlags and set address, everything relative to rPos
    inline  void InitAddressRel( const ScAddress& rAdr, const ScAddress& rPos );
    inline  void SetColRel( sal_Bool bVal ) { Flags.bColRel = (bVal ? sal_True : false ); }
    inline  sal_Bool IsColRel() const       { return Flags.bColRel; }
    inline  void SetRowRel( sal_Bool bVal ) { Flags.bRowRel = (bVal ? sal_True : false ); }
    inline  sal_Bool IsRowRel() const       { return Flags.bRowRel; }
    inline  void SetTabRel( sal_Bool bVal ) { Flags.bTabRel = (bVal ? sal_True : false ); }
    inline  sal_Bool IsTabRel() const       { return Flags.bTabRel; }

    inline  void SetColDeleted( sal_Bool bVal ) { Flags.bColDeleted = (bVal ? sal_True : false ); }
    inline  sal_Bool IsColDeleted() const       { return Flags.bColDeleted; }
    inline  void SetRowDeleted( sal_Bool bVal ) { Flags.bRowDeleted = (bVal ? sal_True : false ); }
    inline  sal_Bool IsRowDeleted() const       { return Flags.bRowDeleted; }
    inline  void SetTabDeleted( sal_Bool bVal ) { Flags.bTabDeleted = (bVal ? sal_True : false ); }
    inline  sal_Bool IsTabDeleted() const       { return Flags.bTabDeleted; }
    sal_Bool IsDeleted() const;

    inline  void SetFlag3D( sal_Bool bVal ) { Flags.bFlag3D = (bVal ? sal_True : false ); }
    inline  sal_Bool IsFlag3D() const       { return Flags.bFlag3D; }
    inline  void SetRelName( sal_Bool bVal )    { Flags.bRelName = (bVal ? sal_True : false ); }
    inline  sal_Bool IsRelName() const          { return Flags.bRelName; }

    inline  sal_Bool Valid() const;
    /// In external references nTab is -1
    inline  bool ValidExternal() const;

    ScAddress toAbs( const ScAddress& rPos ) const;

            void SmartRelAbs( const ScAddress& rPos );
            void CalcRelFromAbs( const ScAddress& rPos );
            void CalcAbsIfRel( const ScAddress& rPos );
            sal_Bool operator==( const ScSingleRefData& ) const;
            bool operator!=( const ScSingleRefData& ) const;
};

inline void ScSingleRefData::InitAddress( SCCOL nColP, SCROW nRowP, SCTAB nTabP )
{
    InitFlags();
    nCol = nColP;
    nRow = nRowP;
    nTab = nTabP;
}

inline void ScSingleRefData::InitAddress( const ScAddress& rAdr )
{
    InitAddress( rAdr.Col(), rAdr.Row(), rAdr.Tab());
}

inline void ScSingleRefData::InitAddressRel( const ScAddress& rAdr,
                                            const ScAddress& rPos )
{
    InitAddress( rAdr.Col(), rAdr.Row(), rAdr.Tab());
    SetColRel( sal_True );
    SetRowRel( sal_True );
    SetTabRel( sal_True );
    CalcRelFromAbs( rPos );
}

inline sal_Bool ScSingleRefData::Valid() const
{
    return  nCol >= 0 && nCol <= MAXCOL &&
            nRow >= 0 && nRow <= MAXROW &&
            nTab >= 0 && nTab <= MAXTAB;
}

inline bool ScSingleRefData::ValidExternal() const
{
    return  nCol >= 0 && nCol <= MAXCOL &&
            nRow >= 0 && nRow <= MAXROW &&
            nTab == -1;
}

/// Complex reference (a range) into the sheet
struct ScComplexRefData
{
    ScSingleRefData Ref1;
    ScSingleRefData Ref2;

    inline  void InitFlags()
        { Ref1.InitFlags(); Ref2.InitFlags(); }
    inline  void InitRange( const ScRange& rRange )
        {
            Ref1.InitAddress( rRange.aStart );
            Ref2.InitAddress( rRange.aEnd );
        }
    inline  void InitRangeRel( const ScRange& rRange, const ScAddress& rPos )
        {
            Ref1.InitAddressRel( rRange.aStart, rPos );
            Ref2.InitAddressRel( rRange.aEnd, rPos );
        }
    inline  void InitRange( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
        {
            Ref1.InitAddress( nCol1, nRow1, nTab1 );
            Ref2.InitAddress( nCol2, nRow2, nTab2 );
        }
    inline  void SmartRelAbs( const ScAddress& rPos )
        { Ref1.SmartRelAbs( rPos ); Ref2.SmartRelAbs( rPos ); }
    inline  void CalcRelFromAbs( const ScAddress& rPos )
        { Ref1.CalcRelFromAbs( rPos ); Ref2.CalcRelFromAbs( rPos ); }
    inline  void CalcAbsIfRel( const ScAddress& rPos )
        { Ref1.CalcAbsIfRel( rPos ); Ref2.CalcAbsIfRel( rPos ); }
    inline  sal_Bool IsDeleted() const
        { return Ref1.IsDeleted() || Ref2.IsDeleted(); }
    inline  sal_Bool Valid() const
        { return Ref1.Valid() && Ref2.Valid(); }
    /** In external references nTab is -1 for the start tab and -1 for the end
        tab if one sheet, or >=0 if more than one sheets. */
    inline  bool ValidExternal() const;

    /// Absolute references have to be up-to-date when calling this!
    void PutInOrder();
    inline  sal_Bool operator==( const ScComplexRefData& r ) const
        { return Ref1 == r.Ref1 && Ref2 == r.Ref2; }
    /** Enlarge range if reference passed is not within existing range.
        ScAddress position is used to calculate absolute references from
        relative references. */
    ScComplexRefData& Extend( const ScSingleRefData & rRef, const ScAddress & rPos );
    ScComplexRefData& Extend( const ScComplexRefData & rRef, const ScAddress & rPos );
};

inline bool ScComplexRefData::ValidExternal() const
{
    return Ref1.ValidExternal() &&
        Ref2.nCol >= 0 && Ref2.nCol <= MAXCOL &&
        Ref2.nRow >= 0 && Ref2.nRow <= MAXROW &&
        Ref2.nTab >= Ref1.nTab;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
