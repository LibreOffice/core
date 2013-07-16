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

#include "refdata.hxx"

bool ScSingleRefData::IsDeleted() const
{
    return IsColDeleted() || IsRowDeleted() || IsTabDeleted();
}

void ScSingleRefData::CalcRelFromAbs( const ScAddress& rPos )
{
    nRelCol = nCol - rPos.Col();
    nRelRow = nRow - rPos.Row();
    nRelTab = nTab - rPos.Tab();
}

ScAddress ScSingleRefData::toAbs( const ScAddress& rPos ) const
{
    SCCOL nRetCol = Flags.bColRel ? nRelCol + rPos.Col() : nCol;
    SCROW nRetRow = Flags.bRowRel ? nRelRow + rPos.Row() : nRow;
    SCTAB nRetTab = Flags.bTabRel ? nRelTab + rPos.Tab() : nTab;

    if (!ValidCol(nRetCol) || !ValidRow(nRetRow) || !ValidTab(nRetTab))
        return ScAddress(ScAddress::INITIALIZE_INVALID);

    return ScAddress(nRetCol, nRetRow, nRetTab);
}

void ScSingleRefData::SetAddress( const ScAddress& rAddr, const ScAddress& rPos )
{
    if (Flags.bColRel)
        nRelCol = rAddr.Col() - rPos.Col();
    else
        nCol = rAddr.Col();

    if (Flags.bRowRel)
        nRelRow = rAddr.Row() - rPos.Row();
    else
        nRow = rAddr.Row();

    if (Flags.bTabRel)
        nRelTab = rAddr.Tab() - rPos.Tab();
    else
        nTab = rAddr.Tab();
}

void ScSingleRefData::CalcAbsIfRel( const ScAddress& rPos )
{
    if ( Flags.bColRel )
    {
        nCol = nRelCol + rPos.Col();
        if ( !ValidCol( nCol ) )
            Flags.bColDeleted = sal_True;
    }
    if ( Flags.bRowRel )
    {
        nRow = nRelRow + rPos.Row();
        if ( !ValidRow( nRow ) )
            Flags.bRowDeleted = sal_True;
    }
    if ( Flags.bTabRel )
    {
        nTab = nRelTab + rPos.Tab();
        if ( !ValidTab( nTab ) )
            Flags.bTabDeleted = sal_True;
    }
}



bool ScSingleRefData::operator==( const ScSingleRefData& r ) const
{
    return mnFlagValue == r.mnFlagValue &&
        (Flags.bColRel ? nRelCol == r.nRelCol : nCol == r.nCol) &&
        (Flags.bRowRel ? nRelRow == r.nRelRow : nRow == r.nRow) &&
        (Flags.bTabRel ? nRelTab == r.nRelTab : nTab == r.nTab);
}

bool ScSingleRefData::operator!=( const ScSingleRefData& r ) const
{
    return !operator==(r);
}

static void lcl_putInOrder( ScSingleRefData & rRef1, ScSingleRefData & rRef2 )
{
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    SCTAB nTab1, nTab2;
    bool bTmp;
    sal_uInt8 nRelState1, nRelState2;
    if ( rRef1.Flags.bRelName )
        nRelState1 =
            ((rRef1.Flags.bTabRel & 0x01) << 2)
            | ((rRef1.Flags.bRowRel & 0x01) << 1)
            | ((rRef1.Flags.bColRel & 0x01));
    else
        nRelState1 = 0;
    if ( rRef2.Flags.bRelName )
        nRelState2 =
            ((rRef2.Flags.bTabRel & 0x01) << 2)
            | ((rRef2.Flags.bRowRel & 0x01) << 1)
            | ((rRef2.Flags.bColRel & 0x01));
    else
        nRelState2 = 0;
    if ( (nCol1 = rRef1.nCol) > (nCol2 = rRef2.nCol) )
    {
        rRef1.nCol = nCol2;
        rRef2.nCol = nCol1;
        nCol1 = rRef1.nRelCol;
        rRef1.nRelCol = rRef2.nRelCol;
        rRef2.nRelCol = nCol1;
        if ( rRef1.Flags.bRelName && rRef1.Flags.bColRel )
            nRelState2 |= 1;
        else
            nRelState2 &= ~1;
        if ( rRef2.Flags.bRelName && rRef2.Flags.bColRel )
            nRelState1 |= 1;
        else
            nRelState1 &= ~1;
        bTmp = rRef1.Flags.bColRel;
        rRef1.Flags.bColRel = rRef2.Flags.bColRel;
        rRef2.Flags.bColRel = bTmp;
        bTmp = rRef1.Flags.bColDeleted;
        rRef1.Flags.bColDeleted = rRef2.Flags.bColDeleted;
        rRef2.Flags.bColDeleted = bTmp;
    }
    if ( (nRow1 = rRef1.nRow) > (nRow2 = rRef2.nRow) )
    {
        rRef1.nRow = nRow2;
        rRef2.nRow = nRow1;
        nRow1 = rRef1.nRelRow;
        rRef1.nRelRow = rRef2.nRelRow;
        rRef2.nRelRow = nRow1;
        if ( rRef1.Flags.bRelName && rRef1.Flags.bRowRel )
            nRelState2 |= 2;
        else
            nRelState2 &= ~2;
        if ( rRef2.Flags.bRelName && rRef2.Flags.bRowRel )
            nRelState1 |= 2;
        else
            nRelState1 &= ~2;
        bTmp = rRef1.Flags.bRowRel;
        rRef1.Flags.bRowRel = rRef2.Flags.bRowRel;
        rRef2.Flags.bRowRel = bTmp;
        bTmp = rRef1.Flags.bRowDeleted;
        rRef1.Flags.bRowDeleted = rRef2.Flags.bRowDeleted;
        rRef2.Flags.bRowDeleted = bTmp;
    }
    if ( (nTab1 = rRef1.nTab) > (nTab2 = rRef2.nTab) )
    {
        rRef1.nTab = nTab2;
        rRef2.nTab = nTab1;
        nTab1 = rRef1.nRelTab;
        rRef1.nRelTab = rRef2.nRelTab;
        rRef2.nRelTab = nTab1;
        if ( rRef1.Flags.bRelName && rRef1.Flags.bTabRel )
            nRelState2 |= 4;
        else
            nRelState2 &= ~4;
        if ( rRef2.Flags.bRelName && rRef2.Flags.bTabRel )
            nRelState1 |= 4;
        else
            nRelState1 &= ~4;
        bTmp = rRef1.Flags.bTabRel;
        rRef1.Flags.bTabRel = rRef2.Flags.bTabRel;
        rRef2.Flags.bTabRel = bTmp;
        bTmp = rRef1.Flags.bTabDeleted;
        rRef1.Flags.bTabDeleted = rRef2.Flags.bTabDeleted;
        rRef2.Flags.bTabDeleted = bTmp;
    }
    rRef1.Flags.bRelName = ( nRelState1 ? sal_True : false );
    rRef2.Flags.bRelName = ( nRelState2 ? sal_True : false );
}


void ScComplexRefData::PutInOrder()
{
    lcl_putInOrder( Ref1, Ref2);
}


static void lcl_adjustInOrder( ScSingleRefData & rRef1, ScSingleRefData & rRef2, bool bFirstLeader )
{
    // a1:a2:a3, bFirstLeader: rRef1==a1==r1, rRef2==a3==r2
    //                   else: rRef1==a3==r2, rRef2==a2==r1
    ScSingleRefData& r1 = (bFirstLeader ? rRef1 : rRef2);
    ScSingleRefData& r2 = (bFirstLeader ? rRef2 : rRef1);
    if (r1.Flags.bFlag3D && !r2.Flags.bFlag3D)
    {
        // [$]Sheet1.A5:A6 on Sheet2 do still refer only Sheet1.
        r2.nTab = r1.nTab;
        r2.nRelTab = r1.nRelTab;
        r2.Flags.bTabRel = r1.Flags.bTabRel;
    }
    lcl_putInOrder( rRef1, rRef2);
}


ScComplexRefData& ScComplexRefData::Extend( const ScSingleRefData & rRef, const ScAddress & rPos )
{
    CalcAbsIfRel( rPos);
    ScSingleRefData aRef = rRef;
    aRef.CalcAbsIfRel( rPos);
    bool bInherit3D = Ref1.IsFlag3D() && !Ref2.IsFlag3D();
    bool bInherit3Dtemp = bInherit3D && !rRef.IsFlag3D();
    if (aRef.nCol < Ref1.nCol || aRef.nRow < Ref1.nRow || aRef.nTab < Ref1.nTab)
    {
        lcl_adjustInOrder( Ref1, aRef, true);
        aRef = rRef;
        aRef.CalcAbsIfRel( rPos);
    }
    if (aRef.nCol > Ref2.nCol || aRef.nRow > Ref2.nRow || aRef.nTab > Ref2.nTab)
    {
        if (bInherit3D)
            Ref2.SetFlag3D( true);
        lcl_adjustInOrder( aRef, Ref2, false);
        if (bInherit3Dtemp)
            Ref2.SetFlag3D( false);
        aRef = rRef;
        aRef.CalcAbsIfRel( rPos);
    }
    // In Ref2 use absolute/relative addressing from non-extended parts if
    // equal and therefor not adjusted.
    // A$5:A5 => A$5:A$5:A5 => A$5:A5, and not A$5:A$5
    // A$6:$A5 => A$6:A$6:$A5 => A5:$A$6
    if (Ref2.nCol == aRef.nCol)
        Ref2.SetColRel( aRef.IsColRel());
    if (Ref2.nRow == aRef.nRow)
        Ref2.SetRowRel( aRef.IsRowRel());
    // $Sheet1.$A$5:$A$6 => $Sheet1.$A$5:$A$5:$A$6 => $Sheet1.$A$5:$A$6, and
    // not $Sheet1.$A$5:Sheet1.$A$6 (with invisible second 3D, but relative).
    if (Ref2.nTab == aRef.nTab)
        Ref2.SetTabRel( bInherit3Dtemp ? Ref1.IsTabRel() : aRef.IsTabRel());
    Ref2.CalcRelFromAbs( rPos);
    // Force 3D if necessary. References to other sheets always.
    if (Ref1.nTab != rPos.Tab())
        Ref1.SetFlag3D( true);
    // In the second part only if different sheet thus not inherited.
    if (Ref2.nTab != Ref1.nTab)
        Ref2.SetFlag3D( true);
    // Merge Flag3D to Ref2 in case there was nothing to inherit and/or range
    // wasn't extended as in A5:A5:Sheet1.A5 if on Sheet1.
    if (rRef.IsFlag3D())
        Ref2.SetFlag3D( true);
    return *this;
}


ScComplexRefData& ScComplexRefData::Extend( const ScComplexRefData & rRef, const ScAddress & rPos )
{
    return Extend( rRef.Ref1, rPos).Extend( rRef.Ref2, rPos);
}

ScRange ScComplexRefData::toAbs( const ScAddress& rPos ) const
{
    return ScRange(Ref1.toAbs(rPos), Ref2.toAbs(rPos));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
