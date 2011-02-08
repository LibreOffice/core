/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "refdata.hxx"


void ScSingleRefData::CalcRelFromAbs( const ScAddress& rPos )
{
    nRelCol = nCol - rPos.Col();
    nRelRow = nRow - rPos.Row();
    nRelTab = nTab - rPos.Tab();
}


void ScSingleRefData::SmartRelAbs( const ScAddress& rPos )
{
    if ( Flags.bColRel )
        nCol = nRelCol + rPos.Col();
    else
        nRelCol = nCol - rPos.Col();

    if ( Flags.bRowRel )
        nRow = nRelRow + rPos.Row();
    else
        nRelRow = nRow - rPos.Row();

    if ( Flags.bTabRel )
        nTab = nRelTab + rPos.Tab();
    else
        nRelTab = nTab - rPos.Tab();
}


void ScSingleRefData::CalcAbsIfRel( const ScAddress& rPos )
{
    if ( Flags.bColRel )
    {
        nCol = nRelCol + rPos.Col();
        if ( !VALIDCOL( nCol ) )
            Flags.bColDeleted = sal_True;
    }
    if ( Flags.bRowRel )
    {
        nRow = nRelRow + rPos.Row();
        if ( !VALIDROW( nRow ) )
            Flags.bRowDeleted = sal_True;
    }
    if ( Flags.bTabRel )
    {
        nTab = nRelTab + rPos.Tab();
        if ( !VALIDTAB( nTab ) )
            Flags.bTabDeleted = sal_True;
    }
}

//UNUSED2008-05  void ScSingleRefData::OldBoolsToNewFlags( const OldSingleRefBools& rBools )
//UNUSED2008-05  {
//UNUSED2008-05      switch ( rBools.bRelCol )
//UNUSED2008-05      {
//UNUSED2008-05          case SR_DELETED :
//UNUSED2008-05              Flags.bColRel = sal_True;           // der war verlorengegangen
//UNUSED2008-05              Flags.bColDeleted = sal_True;
//UNUSED2008-05              break;
//UNUSED2008-05          case SR_ABSOLUTE :
//UNUSED2008-05              Flags.bColRel = sal_False;
//UNUSED2008-05              Flags.bColDeleted = sal_False;
//UNUSED2008-05              break;
//UNUSED2008-05          case SR_RELABS :
//UNUSED2008-05          case SR_RELATIVE :
//UNUSED2008-05          default:
//UNUSED2008-05              Flags.bColRel = sal_True;
//UNUSED2008-05              Flags.bColDeleted = sal_False;
//UNUSED2008-05      }
//UNUSED2008-05      switch ( rBools.bRelRow )
//UNUSED2008-05      {
//UNUSED2008-05          case SR_DELETED :
//UNUSED2008-05              Flags.bRowRel = sal_True;           // der war verlorengegangen
//UNUSED2008-05              Flags.bRowDeleted = sal_True;
//UNUSED2008-05              break;
//UNUSED2008-05          case SR_ABSOLUTE :
//UNUSED2008-05              Flags.bRowRel = sal_False;
//UNUSED2008-05              Flags.bRowDeleted = sal_False;
//UNUSED2008-05              break;
//UNUSED2008-05          case SR_RELABS :
//UNUSED2008-05          case SR_RELATIVE :
//UNUSED2008-05          default:
//UNUSED2008-05              Flags.bRowRel = sal_True;
//UNUSED2008-05              Flags.bRowDeleted = sal_False;
//UNUSED2008-05      }
//UNUSED2008-05      switch ( rBools.bRelTab )
//UNUSED2008-05      {
//UNUSED2008-05          case SR_DELETED :
//UNUSED2008-05              Flags.bTabRel = sal_True;           // der war verlorengegangen
//UNUSED2008-05              Flags.bTabDeleted = sal_True;
//UNUSED2008-05              break;
//UNUSED2008-05          case SR_ABSOLUTE :
//UNUSED2008-05              Flags.bTabRel = sal_False;
//UNUSED2008-05              Flags.bTabDeleted = sal_False;
//UNUSED2008-05              break;
//UNUSED2008-05          case SR_RELABS :
//UNUSED2008-05          case SR_RELATIVE :
//UNUSED2008-05          default:
//UNUSED2008-05              Flags.bTabRel = sal_True;
//UNUSED2008-05              Flags.bTabDeleted = sal_False;
//UNUSED2008-05      }
//UNUSED2008-05      Flags.bFlag3D = (rBools.bOldFlag3D & SRF_3D ? sal_True : sal_False);
//UNUSED2008-05      Flags.bRelName = (rBools.bOldFlag3D & SRF_RELNAME ? sal_True : sal_False);
//UNUSED2008-05      if ( !Flags.bFlag3D )
//UNUSED2008-05          Flags.bTabRel = sal_True;   // ist bei einigen aelteren Dokumenten nicht gesetzt
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  /*
//UNUSED2008-05   bis Release 3.1 sah Store so aus
//UNUSED2008-05
//UNUSED2008-05      sal_uInt8 n = ( ( r.bOldFlag3D & 0x03 ) << 6 )   // RelName, 3D
//UNUSED2008-05              | ( ( r.bRelTab & 0x03 ) << 4 )     // Relative, RelAbs
//UNUSED2008-05              | ( ( r.bRelRow & 0x03 ) << 2 )
//UNUSED2008-05              |   ( r.bRelCol & 0x03 );
//UNUSED2008-05
//UNUSED2008-05   bis Release 3.1 sah Load so aus
//UNUSED2008-05
//UNUSED2008-05      r.bRelCol = ( n & 0x03 );
//UNUSED2008-05      r.bRelRow = ( ( n >> 2 ) & 0x03 );
//UNUSED2008-05      r.bRelTab = ( ( n >> 4 ) & 0x03 );
//UNUSED2008-05      r.bOldFlag3D = ( ( n >> 6 ) & 0x03 );
//UNUSED2008-05
//UNUSED2008-05   bRelCol == SR_DELETED war identisch mit bRelCol == (SR_RELATIVE | SR_RELABS)
//UNUSED2008-05   leider..
//UNUSED2008-05   3.1 liest Zukunft: Deleted wird nicht unbedingt erkannt, nur wenn auch Relativ.
//UNUSED2008-05   Aber immer noch nCol > MAXCOL und gut sollte sein..
//UNUSED2008-05   */
//UNUSED2008-05
//UNUSED2008-05  sal_uInt8 ScSingleRefData::CreateStoreByteFromFlags() const
//UNUSED2008-05  {
//UNUSED2008-05      return (sal_uInt8)(
//UNUSED2008-05            ( (Flags.bRelName     & 0x01) << 7 )
//UNUSED2008-05          | ( (Flags.bFlag3D      & 0x01) << 6 )
//UNUSED2008-05          | ( (Flags.bTabDeleted  & 0x01) << 5 )
//UNUSED2008-05          | ( (Flags.bTabRel      & 0x01) << 4 )
//UNUSED2008-05          | ( (Flags.bRowDeleted  & 0x01) << 3 )
//UNUSED2008-05          | ( (Flags.bRowRel      & 0x01) << 2 )
//UNUSED2008-05          | ( (Flags.bColDeleted  & 0x01) << 1 )
//UNUSED2008-05          |   (Flags.bColRel      & 0x01)
//UNUSED2008-05          );
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  void ScSingleRefData::CreateFlagsFromLoadByte( sal_uInt8 n )
//UNUSED2008-05  {
//UNUSED2008-05      Flags.bColRel       = (n & 0x01 );
//UNUSED2008-05      Flags.bColDeleted   = ( (n >> 1) & 0x01 );
//UNUSED2008-05      Flags.bRowRel       = ( (n >> 2) & 0x01 );
//UNUSED2008-05      Flags.bRowDeleted   = ( (n >> 3) & 0x01 );
//UNUSED2008-05      Flags.bTabRel       = ( (n >> 4) & 0x01 );
//UNUSED2008-05      Flags.bTabDeleted   = ( (n >> 5) & 0x01 );
//UNUSED2008-05      Flags.bFlag3D       = ( (n >> 6) & 0x01 );
//UNUSED2008-05      Flags.bRelName      = ( (n >> 7) & 0x01 );
//UNUSED2008-05  }


sal_Bool ScSingleRefData::operator==( const ScSingleRefData& r ) const
{
    return bFlags == r.bFlags &&
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
    sal_Bool bTmp;
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
    rRef1.Flags.bRelName = ( nRelState1 ? sal_True : sal_False );
    rRef2.Flags.bRelName = ( nRelState2 ? sal_True : sal_False );
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
