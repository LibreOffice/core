/*************************************************************************
 *
 *  $RCSfile: refdata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: er $ $Date: 2001-02-21 18:33:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

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


// Abs-Refs muessen vorher aktualisiert werden!
// wird in refupdat.cxx mit MoveRelWrap verwendet
void ComplRefData::PutInOrder()
{
    register short n1, n2;
    register BOOL bTmp;
    BYTE nRelState1, nRelState2;
    if ( Ref1.Flags.bRelName )
        nRelState1 =
              ((Ref1.Flags.bTabRel & 0x01) << 2)
            | ((Ref1.Flags.bRowRel & 0x01) << 1)
            | ((Ref1.Flags.bColRel & 0x01));
    else
        nRelState1 = 0;
    if ( Ref2.Flags.bRelName )
        nRelState2 =
              ((Ref2.Flags.bTabRel & 0x01) << 2)
            | ((Ref2.Flags.bRowRel & 0x01) << 1)
            | ((Ref2.Flags.bColRel & 0x01));
    else
        nRelState2 = 0;
    if ( (n1 = Ref1.nCol) > (n2 = Ref2.nCol) )
    {
        Ref1.nCol = n2;
        Ref2.nCol = n1;
        n1 = Ref1.nRelCol;
        Ref1.nRelCol = Ref2.nRelCol;
        Ref2.nRelCol = n1;
        if ( Ref1.Flags.bRelName && Ref1.Flags.bColRel )
            nRelState2 |= 1;
        else
            nRelState2 &= ~1;
        if ( Ref2.Flags.bRelName && Ref2.Flags.bColRel )
            nRelState1 |= 1;
        else
            nRelState1 &= ~1;
        bTmp = Ref1.Flags.bColRel;
        Ref1.Flags.bColRel = Ref2.Flags.bColRel;
        Ref2.Flags.bColRel = bTmp;
        bTmp = Ref1.Flags.bColDeleted;
        Ref1.Flags.bColDeleted = Ref2.Flags.bColDeleted;
        Ref2.Flags.bColDeleted = bTmp;
    }
    if ( (n1 = Ref1.nRow) > (n2 = Ref2.nRow) )
    {
        Ref1.nRow = n2;
        Ref2.nRow = n1;
        n1 = Ref1.nRelRow;
        Ref1.nRelRow = Ref2.nRelRow;
        Ref2.nRelRow = n1;
        if ( Ref1.Flags.bRelName && Ref1.Flags.bRowRel )
            nRelState2 |= 2;
        else
            nRelState2 &= ~2;
        if ( Ref2.Flags.bRelName && Ref2.Flags.bRowRel )
            nRelState1 |= 2;
        else
            nRelState1 &= ~2;
        bTmp = Ref1.Flags.bRowRel;
        Ref1.Flags.bRowRel = Ref2.Flags.bRowRel;
        Ref2.Flags.bRowRel = bTmp;
        bTmp = Ref1.Flags.bRowDeleted;
        Ref1.Flags.bRowDeleted = Ref2.Flags.bRowDeleted;
        Ref2.Flags.bRowDeleted = bTmp;
    }
    if ( (n1 = Ref1.nTab) > (n2 = Ref2.nTab) )
    {
        Ref1.nTab = n2;
        Ref2.nTab = n1;
        n1 = Ref1.nRelTab;
        Ref1.nRelTab = Ref2.nRelTab;
        Ref2.nRelTab = n1;
        if ( Ref1.Flags.bRelName && Ref1.Flags.bTabRel )
            nRelState2 |= 4;
        else
            nRelState2 &= ~4;
        if ( Ref2.Flags.bRelName && Ref2.Flags.bTabRel )
            nRelState1 |= 4;
        else
            nRelState1 &= ~4;
        bTmp = Ref1.Flags.bTabRel;
        Ref1.Flags.bTabRel = Ref2.Flags.bTabRel;
        Ref2.Flags.bTabRel = bTmp;
        bTmp = Ref1.Flags.bTabDeleted;
        Ref1.Flags.bTabDeleted = Ref2.Flags.bTabDeleted;
        Ref2.Flags.bTabDeleted = bTmp;
        bTmp = Ref1.Flags.bFlag3D;
        Ref1.Flags.bFlag3D = Ref2.Flags.bFlag3D;
        Ref2.Flags.bFlag3D = bTmp;
    }
    Ref1.Flags.bRelName = ( nRelState1 ? TRUE : FALSE );
    Ref2.Flags.bRelName = ( nRelState2 ? TRUE : FALSE );
}

