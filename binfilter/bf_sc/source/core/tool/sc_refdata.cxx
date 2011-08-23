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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "refdata.hxx"
namespace binfilter {


/*N*/ void SingleRefData::CalcRelFromAbs( const ScAddress& rPos )
/*N*/ {
/*N*/ 	nRelCol = nCol - rPos.Col();
/*N*/ 	nRelRow = nRow - rPos.Row();
/*N*/ 	nRelTab = nTab - rPos.Tab();
/*N*/ }


/*N*/ void SingleRefData::SmartRelAbs( const ScAddress& rPos )
/*N*/ {
/*N*/ 	if ( Flags.bColRel )
/*N*/ 		nCol = nRelCol + rPos.Col();
/*N*/ 	else
/*N*/ 		nRelCol = nCol - rPos.Col();
/*N*/ 
/*N*/ 	if ( Flags.bRowRel )
/*N*/ 		nRow = nRelRow + rPos.Row();
/*N*/ 	else
/*N*/ 		nRelRow = nRow - rPos.Row();
/*N*/ 
/*N*/ 	if ( Flags.bTabRel )
/*N*/ 		nTab = nRelTab + rPos.Tab();
/*N*/ 	else
/*N*/ 		nRelTab = nTab - rPos.Tab();
/*N*/ }


/*N*/ void SingleRefData::CalcAbsIfRel( const ScAddress& rPos )
/*N*/ {
/*N*/ 	if ( Flags.bColRel )
/*N*/ 	{
/*N*/ 		nCol = nRelCol + rPos.Col();
/*N*/ 		if ( !VALIDCOL( nCol ) )
/*N*/ 			Flags.bColDeleted = TRUE;
/*N*/ 	}
/*N*/ 	if ( Flags.bRowRel )
/*N*/ 	{
/*N*/ 		nRow = nRelRow + rPos.Row();
/*N*/ 		if ( !VALIDROW( nRow ) )
/*N*/ 			Flags.bRowDeleted = TRUE;
/*N*/ 	}
/*N*/ 	if ( Flags.bTabRel )
/*N*/ 	{
/*N*/ 		nTab = nRelTab + rPos.Tab();
/*N*/ 		if ( !VALIDTAB( nTab ) )
/*N*/ 			Flags.bTabDeleted = TRUE;
/*N*/ 	}
/*N*/ }


/*N*/ void SingleRefData::OldBoolsToNewFlags( const OldSingleRefBools& rBools )
/*N*/ {
/*N*/ 	switch ( rBools.bRelCol )
/*N*/ 	{
/*N*/ 		case SR_DELETED :
/*?*/ 			Flags.bColRel = TRUE;			// der war verlorengegangen
/*?*/ 			Flags.bColDeleted = TRUE;
/*?*/ 			break;
/*N*/ 		case SR_ABSOLUTE :
/*N*/ 			Flags.bColRel = FALSE;
/*N*/ 			Flags.bColDeleted = FALSE;
/*N*/ 			break;
/*N*/ 		case SR_RELABS :
/*N*/ 		case SR_RELATIVE :
/*N*/ 		default:
/*N*/ 			Flags.bColRel = TRUE;
/*N*/ 			Flags.bColDeleted = FALSE;
/*N*/ 	}
/*N*/ 	switch ( rBools.bRelRow )
/*N*/ 	{
/*N*/ 		case SR_DELETED :
/*?*/ 			Flags.bRowRel = TRUE;			// der war verlorengegangen
/*?*/ 			Flags.bRowDeleted = TRUE;
/*?*/ 			break;
/*N*/ 		case SR_ABSOLUTE :
/*N*/ 			Flags.bRowRel = FALSE;
/*N*/ 			Flags.bRowDeleted = FALSE;
/*N*/ 			break;
/*N*/ 		case SR_RELABS :
/*N*/ 		case SR_RELATIVE :
/*N*/ 		default:
/*N*/ 			Flags.bRowRel = TRUE;
/*N*/ 			Flags.bRowDeleted = FALSE;
/*N*/ 	}
/*N*/ 	switch ( rBools.bRelTab )
/*N*/ 	{
/*N*/ 		case SR_DELETED :
/*?*/ 			Flags.bTabRel = TRUE;			// der war verlorengegangen
/*?*/ 			Flags.bTabDeleted = TRUE;
/*?*/ 			break;
/*N*/ 		case SR_ABSOLUTE :
/*N*/ 			Flags.bTabRel = FALSE;
/*N*/ 			Flags.bTabDeleted = FALSE;
/*N*/ 			break;
/*N*/ 		case SR_RELABS :
/*N*/ 		case SR_RELATIVE :
/*N*/ 		default:
/*N*/ 			Flags.bTabRel = TRUE;
/*N*/ 			Flags.bTabDeleted = FALSE;
/*N*/ 	}
/*N*/ 	Flags.bFlag3D = (rBools.bOldFlag3D & SRF_3D ? TRUE : FALSE);
/*N*/ 	Flags.bRelName = (rBools.bOldFlag3D & SRF_RELNAME ? TRUE : FALSE);
/*N*/ 	if ( !Flags.bFlag3D )
/*N*/ 		Flags.bTabRel = TRUE;	// ist bei einigen aelteren Dokumenten nicht gesetzt
/*N*/ }


/*
 bis Release 3.1 sah Store so aus

    BYTE n = ( ( r.bOldFlag3D & 0x03 ) << 6 )	// RelName, 3D
            | ( ( r.bRelTab & 0x03 ) << 4 )		// Relative, RelAbs
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

/*N*/ BYTE SingleRefData::CreateStoreByteFromFlags() const
/*N*/ {
/*N*/ 	return (BYTE)(
/*N*/ 		  ( (Flags.bRelName  	& 0x01) << 7 )
/*N*/ 		| ( (Flags.bFlag3D 		& 0x01) << 6 )
/*N*/ 		| ( (Flags.bTabDeleted 	& 0x01) << 5 )
/*N*/ 		| ( (Flags.bTabRel 		& 0x01) << 4 )
/*N*/ 		| ( (Flags.bRowDeleted	& 0x01) << 3 )
/*N*/ 		| ( (Flags.bRowRel 		& 0x01) << 2 )
/*N*/ 		| ( (Flags.bColDeleted	& 0x01) << 1 )
/*N*/ 		|   (Flags.bColRel 		& 0x01)
/*N*/ 		);
/*N*/ }


/*N*/ void SingleRefData::CreateFlagsFromLoadByte( BYTE n )
/*N*/ {
/*N*/ 	Flags.bColRel		= (n & 0x01 );
/*N*/ 	Flags.bColDeleted	= ( (n >> 1) & 0x01 );
/*N*/ 	Flags.bRowRel		= ( (n >> 2) & 0x01 );
/*N*/ 	Flags.bRowDeleted	= ( (n >> 3) & 0x01 );
/*N*/ 	Flags.bTabRel		= ( (n >> 4) & 0x01 );
/*N*/ 	Flags.bTabDeleted	= ( (n >> 5) & 0x01 );
/*N*/ 	Flags.bFlag3D		= ( (n >> 6) & 0x01 );
/*N*/ 	Flags.bRelName		= ( (n >> 7) & 0x01 );
/*N*/ }


/*N*/  BOOL SingleRefData::operator==( const SingleRefData& r ) const
/*N*/  {
/*N*/  	return bFlags == r.bFlags &&
/*N*/  		(Flags.bColRel ? nRelCol == r.nRelCol : nCol == r.nCol) &&
/*N*/  		(Flags.bRowRel ? nRelRow == r.nRelRow : nRow == r.nRow) &&
/*N*/  		(Flags.bTabRel ? nRelTab == r.nRelTab : nTab == r.nTab);
/*N*/  }


// Abs-Refs muessen vorher aktualisiert werden!
// wird in refupdat.cxx mit MoveRelWrap verwendet
/*N*/ void ComplRefData::PutInOrder()
/*N*/ {
/*N*/ 	register short n1, n2;
/*N*/ 	register BOOL bTmp;
/*N*/ 	BYTE nRelState1, nRelState2;
/*N*/ 	if ( Ref1.Flags.bRelName )
/*?*/ 		nRelState1 =
/*?*/ 			  ((Ref1.Flags.bTabRel & 0x01) << 2)
/*?*/ 			| ((Ref1.Flags.bRowRel & 0x01) << 1)
/*?*/ 			| ((Ref1.Flags.bColRel & 0x01));
/*N*/ 	else
/*N*/ 		nRelState1 = 0;
/*N*/ 	if ( Ref2.Flags.bRelName )
/*?*/ 		nRelState2 =
/*?*/ 			  ((Ref2.Flags.bTabRel & 0x01) << 2)
/*?*/ 			| ((Ref2.Flags.bRowRel & 0x01) << 1)
/*?*/ 			| ((Ref2.Flags.bColRel & 0x01));
/*N*/ 	else
/*N*/ 		nRelState2 = 0;
/*N*/ 	if ( (n1 = Ref1.nCol) > (n2 = Ref2.nCol) )
/*N*/ 	{
/*?*/ 		Ref1.nCol = n2;
/*?*/ 		Ref2.nCol = n1;
/*?*/ 		n1 = Ref1.nRelCol;
/*?*/ 		Ref1.nRelCol = Ref2.nRelCol;
/*?*/ 		Ref2.nRelCol = n1;
/*?*/ 		if ( Ref1.Flags.bRelName && Ref1.Flags.bColRel )
/*?*/ 			nRelState2 |= 1;
/*?*/ 		else
/*?*/ 			nRelState2 &= ~1;
/*?*/ 		if ( Ref2.Flags.bRelName && Ref2.Flags.bColRel )
/*?*/ 			nRelState1 |= 1;
/*?*/ 		else
/*?*/ 			nRelState1 &= ~1;
/*?*/ 		bTmp = Ref1.Flags.bColRel;
/*?*/ 		Ref1.Flags.bColRel = Ref2.Flags.bColRel;
/*?*/ 		Ref2.Flags.bColRel = bTmp;
/*?*/ 		bTmp = Ref1.Flags.bColDeleted;
/*?*/ 		Ref1.Flags.bColDeleted = Ref2.Flags.bColDeleted;
/*?*/ 		Ref2.Flags.bColDeleted = bTmp;
/*N*/ 	}
/*N*/ 	if ( (n1 = Ref1.nRow) > (n2 = Ref2.nRow) )
/*N*/ 	{
/*?*/ 		Ref1.nRow = n2;
/*?*/ 		Ref2.nRow = n1;
/*?*/ 		n1 = Ref1.nRelRow;
/*?*/ 		Ref1.nRelRow = Ref2.nRelRow;
/*?*/ 		Ref2.nRelRow = n1;
/*?*/ 		if ( Ref1.Flags.bRelName && Ref1.Flags.bRowRel )
/*?*/ 			nRelState2 |= 2;
/*?*/ 		else
/*?*/ 			nRelState2 &= ~2;
/*?*/ 		if ( Ref2.Flags.bRelName && Ref2.Flags.bRowRel )
/*?*/ 			nRelState1 |= 2;
/*?*/ 		else
/*?*/ 			nRelState1 &= ~2;
/*?*/ 		bTmp = Ref1.Flags.bRowRel;
/*?*/ 		Ref1.Flags.bRowRel = Ref2.Flags.bRowRel;
/*?*/ 		Ref2.Flags.bRowRel = bTmp;
/*?*/ 		bTmp = Ref1.Flags.bRowDeleted;
/*?*/ 		Ref1.Flags.bRowDeleted = Ref2.Flags.bRowDeleted;
/*?*/ 		Ref2.Flags.bRowDeleted = bTmp;
/*N*/ 	}
/*N*/ 	if ( (n1 = Ref1.nTab) > (n2 = Ref2.nTab) )
/*N*/ 	{
/*?*/ 		Ref1.nTab = n2;
/*?*/ 		Ref2.nTab = n1;
/*?*/ 		n1 = Ref1.nRelTab;
/*?*/ 		Ref1.nRelTab = Ref2.nRelTab;
/*?*/ 		Ref2.nRelTab = n1;
/*?*/ 		if ( Ref1.Flags.bRelName && Ref1.Flags.bTabRel )
/*?*/ 			nRelState2 |= 4;
/*?*/ 		else
/*?*/ 			nRelState2 &= ~4;
/*?*/ 		if ( Ref2.Flags.bRelName && Ref2.Flags.bTabRel )
/*?*/ 			nRelState1 |= 4;
/*?*/ 		else
/*?*/ 			nRelState1 &= ~4;
/*?*/ 		bTmp = Ref1.Flags.bTabRel;
/*?*/ 		Ref1.Flags.bTabRel = Ref2.Flags.bTabRel;
/*?*/ 		Ref2.Flags.bTabRel = bTmp;
/*?*/ 		bTmp = Ref1.Flags.bTabDeleted;
/*?*/ 		Ref1.Flags.bTabDeleted = Ref2.Flags.bTabDeleted;
/*?*/ 		Ref2.Flags.bTabDeleted = bTmp;
/*?*/ 		bTmp = Ref1.Flags.bFlag3D;
/*?*/ 		Ref1.Flags.bFlag3D = Ref2.Flags.bFlag3D;
/*?*/ 		Ref2.Flags.bFlag3D = bTmp;
/*N*/ 	}
/*N*/ 	Ref1.Flags.bRelName = ( nRelState1 ? TRUE : FALSE );
/*N*/ 	Ref2.Flags.bRelName = ( nRelState2 ? TRUE : FALSE );
/*N*/ }

}
