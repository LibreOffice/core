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

    /// No default ctor, because used in ScRawToken union, set InitFlags!
    inline  void InitFlags() { mnFlagValue = 0; }    ///< all FALSE
    /// InitAddress: InitFlags and set address
    inline  void InitAddress( const ScAddress& rAdr );
    inline  void InitAddress( SCCOL nCol, SCROW nRow, SCTAB nTab );
    /// InitAddressRel: InitFlags and set address, everything relative to rPos
    inline  void InitAddressRel( const ScAddress& rAdr, const ScAddress& rPos );
    inline  void SetColRel( bool bVal ) { Flags.bColRel = (bVal ? true : false ); }
    inline  bool IsColRel() const{ return Flags.bColRel; }
    inline  void SetRowRel( bool bVal ) { Flags.bRowRel = (bVal ? true : false ); }
    inline  bool IsRowRel() const { return Flags.bRowRel; }
    inline  void SetTabRel( bool bVal ) { Flags.bTabRel = (bVal ? true : false ); }
    inline  bool IsTabRel() const       { return Flags.bTabRel; }

    inline  void SetColDeleted( bool bVal ) { Flags.bColDeleted = (bVal ? true : false ); }
    inline  bool IsColDeleted() const       { return Flags.bColDeleted; }
    inline  void SetRowDeleted( bool bVal ) { Flags.bRowDeleted = (bVal ? true : false ); }
    inline  bool IsRowDeleted() const       { return Flags.bRowDeleted; }
    inline  void SetTabDeleted( bool bVal ) { Flags.bTabDeleted = (bVal ? true : false ); }
    inline  bool IsTabDeleted() const       { return Flags.bTabDeleted; }
    bool IsDeleted() const;

    inline  void SetFlag3D( bool bVal ) { Flags.bFlag3D = (bVal ? true : false ); }
    inline  bool IsFlag3D() const       { return Flags.bFlag3D; }
    inline  void SetRelName( bool bVal )    { Flags.bRelName = (bVal ? true : false ); }
    inline  bool IsRelName() const          { return Flags.bRelName; }

    inline  bool Valid() const;
    /// In external references nTab is -1
    inline  bool ValidExternal() const;

    ScAddress toAbs( const ScAddress& rPos ) const;

            void CalcRelFromAbs( const ScAddress& rPos );
            void CalcAbsIfRel( const ScAddress& rPos );
            bool operator==( const ScSingleRefData& ) const;
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
    SetColRel( true );
    SetRowRel( true );
    SetTabRel( true );
    CalcRelFromAbs( rPos );
}

inline bool ScSingleRefData::Valid() const
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

    inline void InitFlags()
        { Ref1.InitFlags(); Ref2.InitFlags(); }
    inline void InitRange( const ScRange& rRange )
        {
            Ref1.InitAddress( rRange.aStart );
            Ref2.InitAddress( rRange.aEnd );
        }
    inline void InitRangeRel( const ScRange& rRange, const ScAddress& rPos )
        {
            Ref1.InitAddressRel( rRange.aStart, rPos );
            Ref2.InitAddressRel( rRange.aEnd, rPos );
        }
    inline void InitRange( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2 )
        {
            Ref1.InitAddress( nCol1, nRow1, nTab1 );
            Ref2.InitAddress( nCol2, nRow2, nTab2 );
        }
    inline void CalcRelFromAbs( const ScAddress& rPos )
        { Ref1.CalcRelFromAbs( rPos ); Ref2.CalcRelFromAbs( rPos ); }
    inline void CalcAbsIfRel( const ScAddress& rPos )
        { Ref1.CalcAbsIfRel( rPos ); Ref2.CalcAbsIfRel( rPos ); }
    inline bool IsDeleted() const
        { return Ref1.IsDeleted() || Ref2.IsDeleted(); }
    inline bool Valid() const
        { return Ref1.Valid() && Ref2.Valid(); }
    /** In external references nTab is -1 for the start tab and -1 for the end
        tab if one sheet, or >=0 if more than one sheets. */
    inline  bool ValidExternal() const;

    ScRange toAbs( const ScAddress& rPos ) const;

    /// Absolute references have to be up-to-date when calling this!
    void PutInOrder();
    inline bool operator==( const ScComplexRefData& r ) const
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
