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

void ScSingleRefData::InitAddress( const ScAddress& rAdr )
{
    InitAddress( rAdr.Col(), rAdr.Row(), rAdr.Tab());
}

void ScSingleRefData::InitAddress( SCCOL nColP, SCROW nRowP, SCTAB nTabP )
{
    InitFlags();
    nCol = nColP;
    nRow = nRowP;
    nTab = nTabP;
}

void ScSingleRefData::InitAddressRel( const ScAddress& rAdr, const ScAddress& rPos )
{
    SetColRel(true);
    SetRowRel(true);
    SetTabRel(true);
    SetAddress(rAdr, rPos);
}

void ScSingleRefData::SetColDeleted( bool bVal )
{
    Flags.bColDeleted = (bVal ? true : false );
}

bool ScSingleRefData::IsColDeleted() const
{
    return Flags.bColDeleted;
}

void ScSingleRefData::SetRowDeleted( bool bVal )
{
    Flags.bRowDeleted = (bVal ? true : false );
}

bool ScSingleRefData::IsRowDeleted() const
{
    return Flags.bRowDeleted;
}

void ScSingleRefData::SetTabDeleted( bool bVal )
{
    Flags.bTabDeleted = (bVal ? true : false );
}

bool ScSingleRefData::IsTabDeleted() const
{
    return Flags.bTabDeleted;
}

bool ScSingleRefData::IsDeleted() const
{
    return IsColDeleted() || IsRowDeleted() || IsTabDeleted();
}

bool ScSingleRefData::Valid() const
{
    return  nCol >= 0 && nCol <= MAXCOL &&
            nRow >= 0 && nRow <= MAXROW &&
            nTab >= 0 && nTab <= MAXTAB;
}

bool ScSingleRefData::ValidExternal() const
{
    return  nCol >= 0 && nCol <= MAXCOL &&
            nRow >= 0 && nRow <= MAXROW &&
            nTab == -1;
}

ScAddress ScSingleRefData::toAbs( const ScAddress& rPos ) const
{
    SCCOL nRetCol = Flags.bColRel ? nRelCol + rPos.Col() : nCol;
    SCROW nRetRow = Flags.bRowRel ? nRelRow + rPos.Row() : nRow;
    SCTAB nRetTab = Flags.bTabRel ? nRelTab + rPos.Tab() : nTab;

    ScAddress aAbs(ScAddress::INITIALIZE_INVALID);

    if (ValidCol(nRetCol))
        aAbs.SetCol(nRetCol);

    if (ValidRow(nRetRow))
        aAbs.SetRow(nRetRow);

    if (ValidTab(nRetTab))
        aAbs.SetTab(nRetTab);

    return aAbs;
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

SCROW ScSingleRefData::GetRow() const
{
    if (Flags.bRowDeleted)
        return -1;
    return Flags.bRowRel ? nRelRow : nRow;
}

SCCOL ScSingleRefData::GetCol() const
{
    if (Flags.bColDeleted)
        return -1;
    return Flags.bColRel ? nRelCol : nCol;
}

SCTAB ScSingleRefData::GetTab() const
{
    if (Flags.bTabDeleted)
        return -1;
    return Flags.bTabRel ? nRelTab : nTab;
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

#if DEBUG_FORMULA_COMPILER
void ScSingleRefData::Dump() const
{
    cout << "  address type column: " << (IsColRel()?"relative":"absolute")
        << "  row : " << (IsRowRel()?"relative":"absolute") << "  sheet: "
        << (IsTabRel()?"relative":"absolute") << endl;
    cout << "  deleted column: " << (IsColDeleted()?"yes":"no")
        << "  row : " << (IsRowDeleted()?"yes":"no") << "  sheet: "
        << (IsTabDeleted()?"yes":"no") << endl;
    cout << "  absolute pos column: " << nCol << "  row: " << nRow << "  sheet: " << nTab << endl;
    cout << "  relative pos column: " << nRelCol << "  row: " << nRelRow << "  sheet: " << nRelTab << endl;
}
#endif

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

ScComplexRefData& ScComplexRefData::Extend( const ScSingleRefData & rRef, const ScAddress & rPos )
{
    ScRange aAbsRange = toAbs(rPos);
    ScAddress aAbs = rRef.toAbs(rPos);

    if (aAbs.Col() < aAbsRange.aStart.Col())
        aAbsRange.aStart.SetCol(aAbs.Col());

    if (aAbs.Row() < aAbsRange.aStart.Row())
        aAbsRange.aStart.SetRow(aAbs.Row());

    if (aAbs.Tab() < aAbsRange.aStart.Tab())
        aAbsRange.aStart.SetTab(aAbs.Tab());

    if (aAbsRange.aEnd.Col() < aAbs.Col())
        aAbsRange.aEnd.SetCol(aAbs.Col());

    if (aAbsRange.aEnd.Row() < aAbs.Row())
        aAbsRange.aEnd.SetRow(aAbs.Row());

    if (aAbsRange.aEnd.Tab() < aAbs.Tab())
        aAbsRange.aEnd.SetTab(aAbs.Tab());

    SetRange(aAbsRange, rPos);

    return *this;
}


ScComplexRefData& ScComplexRefData::Extend( const ScComplexRefData & rRef, const ScAddress & rPos )
{
    return Extend( rRef.Ref1, rPos).Extend( rRef.Ref2, rPos);
}

bool ScComplexRefData::IsDeleted() const
{
    return Ref1.IsDeleted() || Ref2.IsDeleted();
}

bool ScComplexRefData::Valid() const
{
    return Ref1.Valid() && Ref2.Valid();
}

bool ScComplexRefData::ValidExternal() const
{
    return Ref1.ValidExternal() &&
        Ref2.nCol >= 0 && Ref2.nCol <= MAXCOL &&
        Ref2.nRow >= 0 && Ref2.nRow <= MAXROW &&
        Ref2.nTab >= Ref1.nTab;
}

ScRange ScComplexRefData::toAbs( const ScAddress& rPos ) const
{
    return ScRange(Ref1.toAbs(rPos), Ref2.toAbs(rPos));
}

void ScComplexRefData::SetRange( const ScRange& rRange, const ScAddress& rPos )
{
    Ref1.SetAddress(rRange.aStart, rPos);
    Ref2.SetAddress(rRange.aEnd, rPos);
}

#if DEBUG_FORMULA_COMPILER
void ScComplexRefData::Dump() const
{
    cout << "  ref 1" << endl;
    cout << "    address type column: " << (Ref1.IsColRel()?"relative":"absolute")
        << "  row: " << (Ref1.IsRowRel()?"relative":"absolute")
        << "  sheet: " << (Ref1.IsTabRel()?"relative":"absolute") << endl;
    cout << "    deleted column: " << (Ref1.IsColDeleted()?"yes":"no")
        << "  row: " << (Ref1.IsRowDeleted()?"yes":"no")
        << "  sheet: " << (Ref1.IsTabDeleted()?"yes":"no") << endl;
    cout << "    absolute pos column: " << Ref1.nCol << "  row: " << Ref1.nRow << "  sheet: " << Ref1.nTab << endl;
    cout << "    relative pos column: " << Ref1.nRelCol << "  row: " << Ref1.nRelRow << "  sheet: " << Ref1.nRelTab << endl;

    cout << "  ref 2" << endl;
    cout << "    address type column: " << (Ref2.IsColRel()?"relative":"absolute")
        << "  row: " << (Ref2.IsRowRel()?"relative":"absolute")
        << "  sheet: " << (Ref2.IsTabRel()?"relative":"absolute") << endl;
    cout << "    deleted column: " << (Ref2.IsColDeleted()?"yes":"no")
        << "  row: " << (Ref2.IsRowDeleted()?"yes":"no")
        << "  sheet: " << (Ref2.IsTabDeleted()?"yes":"no") << endl;
    cout << "    absolute pos column: " << Ref2.nCol << "  row: " << Ref2.nRow << "  sheet: " << Ref2.nTab << endl;
    cout << "    relative pos column: " << Ref2.nRelCol << "  row: " << Ref2.nRelRow << "  sheet: " << Ref2.nRelTab << endl;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
