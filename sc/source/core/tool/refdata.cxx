/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: refdata.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 15:37:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include "refdata.hxx"


void SingleRefData::CalcRelFromAbs( const ScAddress& rPos )
{
    nRelCol = nCol - rPos.Col();
    nRelRow = nRow - rPos.Row();
    nRelTab = nTab - rPos.Tab();
}


void SingleRefData::SmartRelAbs( const ScAddress& rPos )
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


void SingleRefData::CalcAbsIfRel( const ScAddress& rPos )
{
    if ( Flags.bColRel )
    {
        nCol = nRelCol + rPos.Col();
        if ( !VALIDCOL( nCol ) )
            Flags.bColDeleted = TRUE;
    }
    if ( Flags.bRowRel )
    {
        nRow = nRelRow + rPos.Row();
        if ( !VALIDROW( nRow ) )
            Flags.bRowDeleted = TRUE;
    }
    if ( Flags.bTabRel )
    {
        nTab = nRelTab + rPos.Tab();
        if ( !VALIDTAB( nTab ) )
            Flags.bTabDeleted = TRUE;
    }
}


void SingleRefData::OldBoolsToNewFlags( const OldSingleRefBools& rBools )
{
    switch ( rBools.bRelCol )
    {
        case SR_DELETED :
            Flags.bColRel = TRUE;           // der war verlorengegangen
            Flags.bColDeleted = TRUE;
            break;
        case SR_ABSOLUTE :
            Flags.bColRel = FALSE;
            Flags.bColDeleted = FALSE;
            break;
        case SR_RELABS :
        case SR_RELATIVE :
        default:
            Flags.bColRel = TRUE;
            Flags.bColDeleted = FALSE;
    }
    switch ( rBools.bRelRow )
    {
        case SR_DELETED :
            Flags.bRowRel = TRUE;           // der war verlorengegangen
            Flags.bRowDeleted = TRUE;
            break;
        case SR_ABSOLUTE :
            Flags.bRowRel = FALSE;
            Flags.bRowDeleted = FALSE;
            break;
        case SR_RELABS :
        case SR_RELATIVE :
        default:
            Flags.bRowRel = TRUE;
            Flags.bRowDeleted = FALSE;
    }
    switch ( rBools.bRelTab )
    {
        case SR_DELETED :
            Flags.bTabRel = TRUE;           // der war verlorengegangen
            Flags.bTabDeleted = TRUE;
            break;
        case SR_ABSOLUTE :
            Flags.bTabRel = FALSE;
            Flags.bTabDeleted = FALSE;
            break;
        case SR_RELABS :
        case SR_RELATIVE :
        default:
            Flags.bTabRel = TRUE;
            Flags.bTabDeleted = FALSE;
    }
    Flags.bFlag3D = (rBools.bOldFlag3D & SRF_3D ? TRUE : FALSE);
    Flags.bRelName = (rBools.bOldFlag3D & SRF_RELNAME ? TRUE : FALSE);
    if ( !Flags.bFlag3D )
        Flags.bTabRel = TRUE;   // ist bei einigen aelteren Dokumenten nicht gesetzt
}


/*
 bis Release 3.1 sah Store so aus

    BYTE n = ( ( r.bOldFlag3D & 0x03 ) << 6 )   // RelName, 3D
            | ( ( r.bRelTab & 0x03 ) << 4 )     // Relative, RelAbs
            | ( ( r.bRelRow & 0x03 ) << 2 )
            |   ( r.bRelCol & 0x03 );

 bis Release 3.1 sah Load so aus

    r.bRelCol = ( n & 0x03 );
    r.bRelRow = ( ( n >> 2 ) & 0x03 );
    r.bRelTab = ( ( n >> 4 ) & 0x03 );
    r.bOldFlag3D = ( ( n >> 6 ) & 0x03 );

 bRelCol == SR_DELETED war identisch mit bRelCol == (SR_RELATIVE | SR_RELABS)
 leider..
 3.1 liest Zukunft: Deleted wird nicht unbedingt erkannt, nur wenn auch Relativ.
 Aber immer noch nCol > MAXCOL und gut sollte sein..
 */

BYTE SingleRefData::CreateStoreByteFromFlags() const
{
    return (BYTE)(
          ( (Flags.bRelName     & 0x01) << 7 )
        | ( (Flags.bFlag3D      & 0x01) << 6 )
        | ( (Flags.bTabDeleted  & 0x01) << 5 )
        | ( (Flags.bTabRel      & 0x01) << 4 )
        | ( (Flags.bRowDeleted  & 0x01) << 3 )
        | ( (Flags.bRowRel      & 0x01) << 2 )
        | ( (Flags.bColDeleted  & 0x01) << 1 )
        |   (Flags.bColRel      & 0x01)
        );
}


void SingleRefData::CreateFlagsFromLoadByte( BYTE n )
{
    Flags.bColRel       = (n & 0x01 );
    Flags.bColDeleted   = ( (n >> 1) & 0x01 );
    Flags.bRowRel       = ( (n >> 2) & 0x01 );
    Flags.bRowDeleted   = ( (n >> 3) & 0x01 );
    Flags.bTabRel       = ( (n >> 4) & 0x01 );
    Flags.bTabDeleted   = ( (n >> 5) & 0x01 );
    Flags.bFlag3D       = ( (n >> 6) & 0x01 );
    Flags.bRelName      = ( (n >> 7) & 0x01 );
}


BOOL SingleRefData::operator==( const SingleRefData& r ) const
{
    return bFlags == r.bFlags &&
        (Flags.bColRel ? nRelCol == r.nRelCol : nCol == r.nCol) &&
        (Flags.bRowRel ? nRelRow == r.nRelRow : nRow == r.nRow) &&
        (Flags.bTabRel ? nRelTab == r.nRelTab : nTab == r.nTab);
}


static void lcl_putInOrder( SingleRefData & rRef1, SingleRefData & rRef2 )
{
    SCCOL nCol1, nCol2;
    SCROW nRow1, nRow2;
    SCTAB nTab1, nTab2;
    BOOL bTmp;
    BYTE nRelState1, nRelState2;
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
    rRef1.Flags.bRelName = ( nRelState1 ? TRUE : FALSE );
    rRef2.Flags.bRelName = ( nRelState2 ? TRUE : FALSE );
}


void ComplRefData::PutInOrder()
{
    lcl_putInOrder( Ref1, Ref2);
}


static void lcl_adjustInOrder( SingleRefData & rRef1, SingleRefData & rRef2, bool bFirstLeader )
{
    // a1:a2:a3, bFirstLeader: rRef1==a1==r1, rRef2==a3==r2
    //                   else: rRef1==a3==r2, rRef2==a2==r1
    SingleRefData& r1 = (bFirstLeader ? rRef1 : rRef2);
    SingleRefData& r2 = (bFirstLeader ? rRef2 : rRef1);
    if (r1.Flags.bFlag3D && !r2.Flags.bFlag3D)
    {
        // [$]Sheet1.A5:A6 on Sheet2 do still refer only Sheet1.
        r2.nTab = r1.nTab;
        r2.nRelTab = r1.nRelTab;
        r2.Flags.bTabRel = r1.Flags.bTabRel;
    }
    lcl_putInOrder( rRef1, rRef2);
}


ComplRefData& ComplRefData::Extend( const SingleRefData & rRef, const ScAddress & rPos )
{
    CalcAbsIfRel( rPos);
    SingleRefData aRef = rRef;
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
    }
    // Force 3D if necessary. References to other sheets always.
    if (Ref1.nTab != rPos.Tab())
        Ref1.SetFlag3D( true);
    // In the second part only if different sheet thus not inherited.
    if (Ref2.nTab != Ref1.nTab)
        Ref2.SetFlag3D( true);
    // Merge Flag3D to Ref2 in case there was nothing to inherit and/or range
    // wasn't extended as in A5:A5:Sheet1.A5 if on Sheet1.
    if (!Ref1.IsFlag3D() && !Ref2.IsFlag3D() && rRef.IsFlag3D())
        Ref2.SetFlag3D( true);
    return *this;
}


ComplRefData& ComplRefData::Extend( const ComplRefData & rRef, const ScAddress & rPos )
{
    return Extend( rRef.Ref1, rPos).Extend( rRef.Ref2, rPos);
}
