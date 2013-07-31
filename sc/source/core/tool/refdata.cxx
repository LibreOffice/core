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

sal_uInt8 ScSingleRefData::FlagValue() const
{
    return mnFlagValue;
}

void ScSingleRefData::SetAbsCol( SCCOL nVal )
{
    Flags.bColRel = false;
    nCol = nVal;
}

void ScSingleRefData::SetRelCol( SCCOL nVal )
{
    Flags.bColRel = true;
    nRelCol = nVal;
}

void ScSingleRefData::IncCol( SCCOL nInc )
{
    if (Flags.bColRel)
        nRelCol += nInc;
    else
        nCol += nInc;
}

void ScSingleRefData::SetAbsRow( SCROW nVal )
{
    Flags.bRowRel = false;
    nRow = nVal;
}

void ScSingleRefData::SetRelRow( SCROW nVal )
{
    Flags.bRowRel = true;
    nRelRow = nVal;
}

void ScSingleRefData::IncRow( SCROW nInc )
{
    if (Flags.bRowRel)
        nRelRow += nInc;
    else
        nRow += nInc;
}

void ScSingleRefData::SetAbsTab( SCTAB nVal )
{
    Flags.bTabRel = false;
    nTab = nVal;
}

void ScSingleRefData::SetRelTab( SCTAB nVal )
{
    Flags.bTabRel = true;
    nRelTab = nVal;
}

void ScSingleRefData::IncTab( SCTAB nInc )
{
    if (Flags.bTabRel)
        nRelTab += nInc;
    else
        nTab += nInc;
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
    return ColValid() && RowValid() && TabValid();
}

bool ScSingleRefData::ColValid() const
{
    if (Flags.bColRel)
    {
        if (nRelCol < -MAXCOL || MAXCOL < nRelCol)
            return false;
    }
    else
    {
        if (nCol < 0 || MAXCOL < nCol)
            return false;
    }

    return true;
}

bool ScSingleRefData::RowValid() const
{
    if (Flags.bRowRel)
    {
        if (nRelRow < -MAXROW || MAXROW < nRelRow)
            return false;
    }
    else
    {
        if (nRow < 0 || MAXROW < nRow)
            return false;
    }

    return true;
}

bool ScSingleRefData::TabValid() const
{
    if (Flags.bTabRel)
    {
        if (nRelTab < -MAXTAB || MAXTAB < nRelTab)
            return false;
    }
    else
    {
        if (nTab < 0 || MAXTAB < nTab)
            return false;
    }

    return true;
}

bool ScSingleRefData::ValidExternal() const
{
    return ColValid() && RowValid() && nTab == -1;
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

SCROW ScSingleRefData::Row() const
{
    if (Flags.bRowDeleted)
        return -1;
    return Flags.bRowRel ? nRelRow : nRow;
}

SCCOL ScSingleRefData::Col() const
{
    if (Flags.bColDeleted)
        return -1;
    return Flags.bColRel ? nRelCol : nCol;
}

SCTAB ScSingleRefData::Tab() const
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
void ScSingleRefData::Dump( int nIndent ) const
{
    std::string aIndent;
    for (int i = 0; i < nIndent; ++i)
        aIndent += "  ";

    cout << aIndent << "address type column: " << (IsColRel()?"relative":"absolute")
         << "  row : " << (IsRowRel()?"relative":"absolute") << "  sheet: "
         << (IsTabRel()?"relative":"absolute") << endl;
    cout << aIndent << "deleted column: " << (IsColDeleted()?"yes":"no")
         << "  row : " << (IsRowDeleted()?"yes":"no") << "  sheet: "
         << (IsTabDeleted()?"yes":"no") << endl;
    cout << aIndent << "absolute pos column: " << nCol << "  row: " << nRow << "  sheet: " << nTab << endl;
    cout << aIndent << "relative pos column: " << nRelCol << "  row: " << nRelRow << "  sheet: " << nRelTab << endl;
    cout << aIndent << "3d ref: " << (IsFlag3D()?"yes":"no") << endl;
}
#endif

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
    return Ref1.ValidExternal() && Ref2.ColValid() && Ref2.RowValid() && Ref1.Tab() <= Ref2.Tab();
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
void ScComplexRefData::Dump( int nIndent ) const
{
    std::string aIndent;
    for (int i = 0; i < nIndent; ++i)
        aIndent += "  ";

    cout << aIndent << "ref 1" << endl;
    Ref1.Dump(nIndent+1);
    cout << aIndent << "ref 2" << endl;
    Ref2.Dump(nIndent+1);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
