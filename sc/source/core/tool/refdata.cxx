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
    mnCol = nColP;
    mnRow = nRowP;
    mnTab = nTabP;
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
    mnCol = nVal;
}

void ScSingleRefData::SetRelCol( SCCOL nVal )
{
    Flags.bColRel = true;
    mnCol = nVal;
}

void ScSingleRefData::IncCol( SCCOL nInc )
{
    mnCol += nInc;
}

void ScSingleRefData::SetAbsRow( SCROW nVal )
{
    Flags.bRowRel = false;
    mnRow = nVal;
}

void ScSingleRefData::SetRelRow( SCROW nVal )
{
    Flags.bRowRel = true;
    mnRow = nVal;
}

void ScSingleRefData::IncRow( SCROW nInc )
{
    mnRow += nInc;
}

void ScSingleRefData::SetAbsTab( SCTAB nVal )
{
    Flags.bTabRel = false;
    mnTab = nVal;
}

void ScSingleRefData::SetRelTab( SCTAB nVal )
{
    Flags.bTabRel = true;
    mnTab = nVal;
}

void ScSingleRefData::IncTab( SCTAB nInc )
{
    mnTab += nInc;
}

void ScSingleRefData::SetColDeleted( bool bVal )
{
    Flags.bColDeleted = bVal;
}

bool ScSingleRefData::IsColDeleted() const
{
    return Flags.bColDeleted;
}

void ScSingleRefData::SetRowDeleted( bool bVal )
{
    Flags.bRowDeleted = bVal;
}

bool ScSingleRefData::IsRowDeleted() const
{
    return Flags.bRowDeleted;
}

void ScSingleRefData::SetTabDeleted( bool bVal )
{
    Flags.bTabDeleted = bVal;
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
        if (mnCol < -MAXCOL || MAXCOL < mnCol)
            return false;
    }
    else
    {
        if (mnCol < 0 || MAXCOL < mnCol)
            return false;
    }

    return true;
}

bool ScSingleRefData::RowValid() const
{
    if (Flags.bRowRel)
    {
        if (mnRow < -MAXROW || MAXROW < mnRow)
            return false;
    }
    else
    {
        if (mnRow < 0 || MAXROW < mnRow)
            return false;
    }

    return true;
}

bool ScSingleRefData::TabValid() const
{
    if (Flags.bTabRel)
    {
        if (mnTab < -MAXTAB || MAXTAB < mnTab)
            return false;
    }
    else
    {
        if (mnTab < 0 || MAXTAB < mnTab)
            return false;
    }

    return true;
}

bool ScSingleRefData::ValidExternal() const
{
    return ColValid() && RowValid() && mnTab == -1;
}

ScAddress ScSingleRefData::toAbs( const ScAddress& rPos ) const
{
    SCCOL nRetCol = Flags.bColRel ? mnCol + rPos.Col() : mnCol;
    SCROW nRetRow = Flags.bRowRel ? mnRow + rPos.Row() : mnRow;
    SCTAB nRetTab = Flags.bTabRel ? mnTab + rPos.Tab() : mnTab;

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
        mnCol = rAddr.Col() - rPos.Col();
    else
        mnCol = rAddr.Col();

    if (Flags.bRowRel)
        mnRow = rAddr.Row() - rPos.Row();
    else
        mnRow = rAddr.Row();

    if (Flags.bTabRel)
        mnTab = rAddr.Tab() - rPos.Tab();
    else
        mnTab = rAddr.Tab();
}

SCROW ScSingleRefData::Row() const
{
    if (Flags.bRowDeleted)
        return -1;
    return mnRow;
}

SCCOL ScSingleRefData::Col() const
{
    if (Flags.bColDeleted)
        return -1;
    return mnCol;
}

SCTAB ScSingleRefData::Tab() const
{
    if (Flags.bTabDeleted)
        return -1;
    return mnTab;
}

bool ScSingleRefData::operator==( const ScSingleRefData& r ) const
{
    return mnFlagValue == r.mnFlagValue && mnCol == r.mnCol && mnRow == r.mnRow && mnTab == r.mnTab;
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
    cout << aIndent << "column: " << mnCol << "  row: " << mnRow << "  sheet: " << mnTab << endl;
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
