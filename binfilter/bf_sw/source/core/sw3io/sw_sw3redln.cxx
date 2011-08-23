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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _DOC_HXX //autogen

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include <doc.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _REDLINE_HXX //autogen
#include <redline.hxx>
#endif


#include "sw3marks.hxx"
#include "sw3imp.hxx"
namespace binfilter {


// lokaler Record in SWG_REDLINES
#define SWG_REDLINE_LCL 'R'

// lokaler Record in SWG_REDLINE
#define SWG_REDLINEDATA_LCL 'D'

/*N*/ void Sw3StringPool::SetupRedlines( SwDoc& rDoc )
/*N*/ {
/*N*/ 	const SwRedlineTbl& rRedlines = rDoc.GetRedlineTbl();
/*N*/ 	for( USHORT i=0; i<rRedlines.Count(); i++ )
/*N*/ 	{
/*?*/       const SwRedline *pRedline = rRedlines[i];
            for( USHORT j=0; j<pRedline->GetStackCount(); j++ )
                Add( pRedline->GetAuthorString(j), USHRT_MAX );
/*N*/ 	}
/*N*/ }

/*  */

// REDLINE:
// BYTE		Flags
// 			0x10 - visisble Flags
// UINT16	Anzahl REDLINEDATA
// REDLINEDTA*
//
// REDLINEDATA:
// BYTE		Flags
// BYTE		Redline-Typ
// UINT16	String-Pool-Index des Autors
// UINT32	Datum
// UINT32	Uhrzeit
// String	Kommentar

/*N*/ void Sw3IoImp::InRedline() //SW50.SDW 
/*N*/ {
/*N*/ 	OpenRec( SWG_REDLINE_LCL );
/*N*/ 
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 
/*N*/ 	UINT16 nCount;
/*N*/ 	*pStrm >> nCount;
/*N*/ 
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	SwRedlineData *pData = 0;
/*N*/ 	for( USHORT i=0; i<nCount; i++ )
/*N*/ 	{
/*N*/ 		OpenRec( SWG_REDLINEDATA_LCL );
/*N*/ 
/*N*/ 		BYTE cDFlags = OpenFlagRec();
/*N*/ 
/*N*/ 		BYTE cType;
/*N*/ 		UINT16 nStrIdx;
/*N*/ 
/*N*/ 		*pStrm 	>> cType
/*N*/ 				>> nStrIdx;
/*N*/ 		CloseFlagRec();
/*N*/ 
/*N*/ 		UINT32 nDate, nTime;
/*N*/ 		String aComment;
/*N*/ 		*pStrm 	>> nDate
/*N*/ 				>> nTime;
/*N*/ 		InString( *pStrm, aComment );
/*N*/ 
/*N*/ 		// Das oberste Element des Stack wurde als letztes geschrieben.
/*N*/ 		USHORT nAuthorIdx;
/*N*/ 		if( bNormal && !bInsert )
/*N*/ 			nAuthorIdx = pDoc->InsertRedlineAuthor(aStringPool.Find(nStrIdx));
/*N*/ 		else
/*N*/ 			nAuthorIdx = pDoc->GetRedlineAuthor();
/*N*/ 
/*N*/ 		pData =	new SwRedlineData( (SwRedlineType)cType, nAuthorIdx,
/*N*/ 								   DateTime( nDate, nTime ), aComment,
/*N*/ 								   pData );
/*N*/ 
/*N*/ 		CloseRec( SWG_REDLINEDATA_LCL );
/*N*/ 	}
/*N*/ 
/*N*/ 	// Der PaM ist erstmal egal und wird erst spaeter gesetzt
/*N*/ 	SwPosition aDummyPos( pDoc->GetNodes().GetEndOfExtras() );
/*N*/ 	BOOL bVisible = (cFlags & 0x10) != 0;
/*N*/ 	SwRedline *pRedline =
/*N*/ 		new SwRedline( pData, aDummyPos, (cFlags & 0x10) != 0,
/*N*/ 					   (cFlags & 0x20) != 0, (cFlags & 0x40) != 0 );
/*N*/ 
/*N*/ 	// Weil der PaM noch nicht gueltig ist, merken wir uns die Redline
/*N*/ 	// erstmal so und fuegen sie erst spaeter in das Dokument ein.
/*N*/ 	if( !pRedlines )
/*N*/ 		pRedlines = new Sw3Redlines;
/*N*/ 	pRedlines->Insert( pRedline, pRedlines->Count() );
/*N*/ 
/*N*/ 	CloseRec( SWG_REDLINE_LCL );
/*N*/ }


void Sw3IoImp::OutRedline( const SwRedline& rRedline )
{
    ASSERT( !IsSw31Or40Export(), "Redlines werden nicht exportiert!" );

    OpenRec( SWG_REDLINE_LCL );

    BYTE cFlags = 0x02;         // Count

    if( rRedline.IsVisible() )
        cFlags += 0x10;
    if( rRedline.IsDelLastPara() )
        cFlags += 0x20;
    if( rRedline.IsLastParaDelete() )
        cFlags += 0x40;

    USHORT i = rRedline.GetStackCount();

    *pStrm  << (BYTE)  cFlags
            << (UINT16)i;

    // Die Redline-Data-Objekte werden von hinten nach vorne geschrieben,
    // das macht das Verketten beim Einlesen leichter.
    while( i )
    {
        i--;

        OpenRec( SWG_REDLINEDATA_LCL );

        cFlags = 0x03;  // Type + StrPool-Index des Autors
        UINT16 nStrIdx = aStringPool.Find( rRedline.GetAuthorString(i),
                                           USHRT_MAX );
        *pStrm  << (BYTE)  cFlags
                << (BYTE)  rRedline.GetType( i )
                << (UINT16)nStrIdx;

        const DateTime& rDateTime = rRedline.GetTimeStamp( i );
        *pStrm  << (UINT32)rDateTime.GetDate()
                << (UINT32)rDateTime.GetTime();
        OutString( *pStrm, rRedline.GetComment( i ) );

        CloseRec( SWG_REDLINEDATA_LCL );
    }

    CloseRec( SWG_REDLINE_LCL );
}

/*  */

// REDLINES:
// REDLINE*

/*N*/ void Sw3IoImp::InRedlines() //SW50.SDW 
/*N*/ {
/*N*/ 	if( pRedlines )
/*N*/ 	{
/*N*/ 		delete pRedlines;
/*N*/ 		pRedlines = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	OpenRec( SWG_REDLINES );
/*N*/ 
/*N*/ 	while( BytesLeft() )
/*N*/ 		InRedline();
/*N*/ 
/*N*/ 	CloseRec( SWG_REDLINES );
/*N*/ }

/*N*/ void Sw3IoImp::OutRedlines( BOOL bPageStyles )
/*N*/ {
/*N*/ 	ASSERT( !IsSw31Or40Export(), "Redlines werden nicht exportiert!" );
/*N*/ 
/*N*/ 	if( !pRedlines )
/*N*/ 		return;
/*N*/ 
/*?*/ 	ASSERT( !bBlock, "In Textbausteinen darf es keine Redlines geben!" );
/*?*/ 
/*?*/ 	USHORT nArrLen = pRedlines->Count();
/*?*/ 	if( nArrLen && bPageStyles )
/*?*/ 	{
/*?*/ 		ASSERT( nCntntRedlineStart <= nArrLen,
/*?*/ 				"Mehr Redlines in Page-Styles als ueberhaupt vorhanden?" );
/*?*/ 		nArrLen = nCntntRedlineStart;
/*?*/ 	}
/*?*/ 	ASSERT( bPageStyles || nCntntRedlineStart==0,
/*?*/ 			"Wieso sind da noch Bookmarks aus Seitenvorlagen?" );
/*?*/ 	if( !nArrLen )
/*?*/ 		return;
/*?*/ 
/*?*/ 	OpenRec( SWG_REDLINES );
/*?*/ 
/*?*/ 	for( USHORT i = 0; i < nArrLen; i++ )
            OutRedline( *(*pRedlines)[i] );
/*?*/ 
/*?*/ 	CloseRec( SWG_REDLINES );
/*N*/ 
/*N*/ 	// Die Redlines eines Page-Styles muessen noch geloescht werden. Das
/*N*/ 	// darf aber noch nicht hier passieren, weil wir die Redlines noch
/*N*/ 	// beim rausschreiben der Markierungen brauchen.
/*N*/ }

/*  */

xub_StrLen lcl_sw3io_getNodeOff( const SwNodeIndex& rNdIdx, xub_StrLen nCntntIdx )
{
    // Hier tricksen wir ein wenig: Da Redlines auch auf Start- oder
    // Endnodes anfangen koennen, muessen wir die Positionen auch dann
    // Speichern, wenn Sections gespeichert werden. Um zu Unterscheiden,
    // ob der Start- oder der Endnode gemeint ist, nutzen wir die dann
    // unbenutzte Content-Position.
    const SwNode& rNode = rNdIdx.GetNode();
    if( rNode.IsCntntNode() )
    {
        if( nCntntIdx > STRING_MAXLEN52 )
            return STRING_MAXLEN52;
        else
            return nCntntIdx;
    }

    if( rNode.IsStartNode() )
        return 0;

    ASSERT( rNode.IsEndNode(), "Was ist denn das fuer ein Node?" );
    return USHRT_MAX;
}

/*N*/ void Sw3IoImp::CollectRedlines( SwPaM* pPaM, BOOL bPageOnly )
/*N*/ {
/*N*/ 	ASSERT( !IsSw31Or40Export(), "Redlines werden nicht exportiert!" );
/*N*/ 
/*N*/ 	delete pRedlines;
/*N*/ 	pRedlines = new Sw3Redlines;
/*N*/ 	nCntntRedlineStart = 0;
/*N*/ 
/*N*/ 	const SwPosition *pPaMStart = pPaM ? pPaM->Start() : 0;
/*N*/ 	const SwPosition *pPaMEnd = pPaM ? pPaM->End() : 0;
/*N*/ 	ULONG nEndOfExtras = pDoc->GetNodes().GetEndOfExtras().GetIndex();
/*N*/ 
/*N*/ 	const SwRedlineTbl& rRedlines = pDoc->GetRedlineTbl();
        USHORT i=0;
/*N*/ 	for( i=0; i<rRedlines.Count(); i++ )
/*N*/ 	{
/*?*/ 		SwRedline *pRedline = rRedlines[i];
/*?*/ 		const SwPosition *pPos = pRedline->Start();
/*?*/ 		const SwPosition *pEndPos = pRedline->End();
/*?*/ 		if( !pRedline->GetContentIdx() && *pPos == *pEndPos )
/*?*/ 		{
/*?*/ 			// "Leere" Redlines werden nicht gespeichert!
/*?*/ 			ASSERT( !this, "Redline in leer" );
/*?*/ 			continue;
/*?*/ 		}
/*?*/ 
/*?*/ 		// If the redline is within one node and starts behind the
/*?*/ 		// last position that is possible within the 5.2 version,
/*?*/ 		// it will be ignored.
/*?*/ 		if( pPos->nContent.GetIndex() > STRING_MAXLEN52 &&
/*?*/ 			pPos->nNode.GetIndex() == pEndPos->nNode.GetIndex() &&
/*?*/ 			pEndPos->nContent.GetIndex() > STRING_MAXLEN52 )
/*?*/ 			continue;
/*?*/ 
/*?*/ 		if( pDoc->IsInHeaderFooter( pPos->nNode ) )
/*?*/ 		{
/*?*/ 			pRedlines->Insert( pRedline, nCntntRedlineStart );
/*?*/ 			nCntntRedlineStart++;
/*?*/ 		}
/*?*/ 		else if( !bPageOnly )
/*?*/ 		{
/*?*/ 			// Bei gegebenen PaM alle Redlines mitnehmen, die in
/*?*/ 			// Sonderbereichen liegen oder ganz oder teilweise im PaM.
/*?*/           const SwPosition *pStart = pRedline->Start();
/*?*/           SwComparePosition eCmp =
/*?*/               bSaveAll || !pPaM || pStart->nNode.GetIndex() <= nEndOfExtras
/*?*/                       ? POS_INSIDE
/*?*/                       : ComparePosition( *pStart, *pRedline->End(),
/*?*/                                          *pPaMStart, *pPaMEnd );
/*?*/           if( POS_BEFORE != eCmp && POS_BEHIND != eCmp &&
/*?*/               POS_COLLIDE_END != eCmp && POS_COLLIDE_START != eCmp )
/*?*/               pRedlines->Insert( pRedline, pRedlines->Count() );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !pRedlines->Count() )
/*N*/ 	{
/*N*/ 		// Keine Redlines? Dann nicht weiter.
/*N*/ 		delete pRedlines;
/*N*/ 		pRedlines = 0;
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*?*/ 	ASSERT( !bBlock, "In Textbausteinen darf es keine Redlines geben!" );
/*?*/ 
/*?*/ 	Sw3Mark aMark;
/*?*/ 	USHORT nArrLen = pRedlines->Count();
/*?*/ 
/*?*/ 	if( !pRedlineMarks )
/*?*/ 		pRedlineMarks = new Sw3Marks( 2*nArrLen, 2 );
/*?*/ 
/*?*/ 	for( i = 0; i < nArrLen; i++ )
        {
/*?*/       const SwRedline* pRedline = pRedlines->GetObject( i );
/*?*/ 
/*?*/       if( i >= nCntntRedlineStart )
/*?*/           aMark.SetId( i - nCntntRedlineStart );
/*?*/       else
/*?*/           aMark.SetId( i );
/*?*/ 
/*?*/       // Bei gegebenen Redlines die Positionen auf den PaM reduzieren
/*?*/       const SwPosition *pStart = pRedline->Start();
/*?*/       const SwPosition *pEnd = pRedline->End();
/*?*/       SwComparePosition eCmp =
/*?*/           bSaveAll || !pPaM || pStart->nNode.GetIndex() <= nEndOfExtras
/*?*/               ? POS_INSIDE
/*?*/               : ComparePosition( *pStart, *pEnd, *pPaMStart, *pPaMEnd );
/*?*/ 
/*?*/       ASSERT( POS_BEFORE != eCmp && POS_BEHIND != eCmp &&
/*?*/               POS_COLLIDE_END != eCmp && POS_COLLIDE_START != eCmp,
/*?*/               "Redline mit ungeultigem Bereich" );
/*?*/ 
/*?*/       if( POS_OUTSIDE == eCmp || POS_OVERLAP_BEFORE == eCmp )
/*?*/           pStart = pPaMStart;
/*?*/       if( POS_OUTSIDE == eCmp || POS_OVERLAP_BEHIND == eCmp )
/*?*/           pEnd = pPaMEnd;
/*?*/ 
/*?*/       aMark.SetType( SW3_REDLINE_START );
/*?*/       aMark.SetNodePos( pStart->nNode.GetIndex() );
/*?*/       aMark.SetNodeOff( lcl_sw3io_getNodeOff( pStart->nNode,
/*?*/                                           pStart->nContent.GetIndex() ) );
/*?*/       pRedlineMarks->Insert( new Sw3Mark(aMark) );
/*?*/ 
/*?*/       if( pRedline->HasMark() )
/*?*/       {
/*?*/           // Wenn der PaM ein Mark besitzt, dann End die jeweils
/*?*/           // andere Position.
/*?*/           aMark.SetType( SW3_REDLINE_END );
/*?*/           aMark.SetNodePos( pEnd->nNode.GetIndex() );
/*?*/           aMark.SetNodeOff( lcl_sw3io_getNodeOff( pEnd->nNode,
/*?*/                                           pEnd->nContent.GetIndex() ) );
/*?*/           pRedlineMarks->Insert( new Sw3Mark(aMark) );
/*?*/       }
/*?*/   }
/*N*/ }

/*  */

// NODEREDLINE:
// BYTE		Flags
//			0x01 - End-Position einer Redline, sonst Start-Positiom
//			0x02 - Redline besitzt Section
// UINT16	Id der Redline (Array-Position)
// UINT16	Content-Position bzw. USHRT_MAX, wenn die Psoition dem zu
//			einem Start-Node gehoerenden ENd-Node bezeichnet.
// CONTENT?	Section (optional)

/*N*/ void Sw3IoImp::InNodeRedline( const SwNodeIndex& rNodeIdx, INT32& nCntntOff, //SW50.SDW
/*N*/ 							  BYTE nInsFirstPara )
/*N*/ {
/*N*/ 	OpenRec( SWG_NODEREDLINE );
/*N*/ 
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	UINT16 nId, nNodeOff;
/*N*/ 	*pStrm >> nId >> nNodeOff;
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	ASSERT( pRedlines && nId < pRedlines->Count(), "Ungueltige Redline-Id" );
/*N*/ 	if( !pRedlines || nId >= pRedlines->Count() )
/*N*/ 	{
/*N*/ 		Error();
/*N*/ 		CloseRec( SWG_NODEREDLINE );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwRedline *pRedline = (*pRedlines)[nId];
/*N*/ 	SwRedlineType eType = pRedline ? pRedline->GetType() : REDLINE_FLAG_MASK;
/*N*/ 	if( REDLINE_INSERT != eType && REDLINE_DELETE != eType &&
/*N*/ 		REDLINE_FORMAT != eType )
/*N*/ 	{
/*N*/ 		// Unbekannter oder noch nicht implementierter Redline-Typ:
/*N*/ 		// Da ist es besser, wir ignorieren sie, denn wir koennen ja
/*N*/ 		// sowieso nichts damit anfangen.
/*N*/ 		ASSERT( !pRedline, "nicht implementierter Redline-Typ" );
/*N*/ 		Warning();
/*N*/ 		CloseRec( SWG_NODEREDLINE );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Position setzen
/*N*/ 	SwPosition *pPos;
/*N*/ 	if( 0x10 & cFlags )
/*N*/ 	{
/*N*/ 		// bei End-Positionen Mark setzen
/*N*/ 		pRedline->SetMark();
/*N*/ 		pPos = pRedline->GetMark();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// sonst Point
/*N*/ 		pPos = pRedline->GetPoint();
/*N*/ 	}
/*N*/ 
/*N*/ 	SwNode& rNode = rNodeIdx.GetNode();
/*N*/ 
/*N*/ 	// Beim Einfuegen in den ersten Absatz sollten wir eine Content-Node
/*N*/ 	// haben, in den wir an der uebergeben Position einfuegen.
/*N*/ 	if( nInsFirstPara )
/*N*/ 	{
/*N*/ 		ASSERT( rNode.IsCntntNode(), "Content-Node beim Einf. erwartet" );
/*N*/ 		ASSERT( USHRT_MAX==nNodeOff, "End-Node-Position beim Einf. erwartet" );
/*N*/ 		nNodeOff = 0;	// Im IsCntntNode-Zweig gibts so die richtige Pos.
/*N*/ 	}
/*N*/ 
/*N*/ 	sal_Bool bInvalid = sal_False;
/*N*/ 	// Positionen koennen auch auf Start- oder End-Nodes fallen.
/*N*/ 	if( rNode.IsCntntNode() )
/*N*/ 	{
/*N*/ 		pPos->nNode = rNodeIdx;
/*N*/ 		xub_StrLen nLen = rNode.GetCntntNode()->Len();
/*N*/ 		xub_StrLen nIdx = static_cast< xub_StrLen >( nNodeOff+nCntntOff );
/*N*/ 		ASSERT( nIdx>=0 && nIdx<=nLen, "ungueltiger Cntnt-Offset" );
/*N*/ 		if( nIdx<0 || nIdx > nLen )
/*N*/ 		{
/*N*/ 			bInvalid = sal_True;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pPos->nContent.Assign( rNode.GetCntntNode(), nIdx );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( rNode.IsStartNode(), "Start-Node erwartet" );
/*N*/ 		if( USHRT_MAX==nNodeOff )
/*N*/ 		{
/*N*/ 			pPos->nNode = rNode.EndOfSectionIndex();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pPos->nNode = rNodeIdx;
/*N*/ 
/*N*/ 			ASSERT( 0==nNodeOff,
/*N*/ 					"ungeueltige Content-Position fuer Start-Node" );
/*N*/ 			if( 0 != nNodeOff )
/*N*/ 				Warning();
/*N*/ 		}
/*N*/ 		pPos->nContent.Assign( 0, 0 );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 	{
/*N*/ 		if( (bNormal && bInsert) || bPageDescs )
/*N*/ 		{
/*N*/ 			if( REDLINE_INSERT == eType )
/*N*/ 			{
/*N*/ 				// Ein einefuegter Text wird nicht angezeigt. Dann muss der
/*N*/ 				// eingefuegt Text an der entsprechenden Position eingfuegt
/*N*/ 				// werden.
/*N*/ 				if( !nInsFirstPara && !pPos->nNode.GetNode().IsCntntNode() &&
/*N*/ 					pRedline->IsDelLastPara() )
/*N*/ 				{
/*N*/ 					// Wenn das DelLastPara-Flag gsetzt ist, wird
/*N*/ 					// versucht in den Absatz davor einzufuegen, denn
/*N*/ 					// der soll ja geloescht werden.
/*N*/ 					pPos->nNode--;
/*N*/ 					SwCntntNode *pCNd = pPos->nNode.GetNode().GetCntntNode();
/*N*/ 					ASSERT( pCNd, "Kein Content-Node trotz DelLastPara" );
/*N*/ 					if( pCNd )
/*N*/ 						pPos->nContent.Assign( pCNd, pCNd->Len() );
/*N*/ 					else
/*N*/ 						pPos->nNode++;
/*N*/ 				}
/*N*/ 				const SwNode& rNewNode = pPos->nNode.GetNode();
/*N*/ 				if( rNewNode.IsCntntNode() )
/*N*/ 				{
/*N*/ 					xub_StrLen nCntntIdx = pPos->nContent.GetIndex();
/*N*/ 
/*N*/ 					InContents( pPos->nNode, nCntntIdx );
/*N*/ 
/*N*/ 					// Den Offset noch an die Verschiebung der
/*N*/ 					// Content-Position anpassen.
/*N*/ 					nCntntOff += pPos->nContent.GetIndex();
/*N*/ 					nCntntOff -= nCntntIdx;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					InContents( pPos->nNode, 0, FALSE, 0, TRUE );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				SkipRec();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwNodeIndex aStart( pDoc->GetNodes().GetEndOfRedlines() );
/*N*/ 			SwStartNode* pSttNd =
/*N*/ 				pDoc->GetNodes().MakeEmptySection( aStart,SwNormalStartNode );
/*N*/ 			aStart = *pSttNd->EndOfSectionNode();
/*N*/ 			InContents( aStart, 0, FALSE );
/*N*/ 			aStart = pSttNd->GetIndex();
/*N*/ 			pRedline->SetContentIdx( &aStart );
/*N*/ 			if( pSttNd->EndOfSectionIndex() - aStart.GetIndex() == 1 )
/*N*/ 				bInvalid = sal_True;
/*N*/ 			ASSERT( !bInvalid, "empty redline section imported" );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwRedlineTbl& rRedlineTbl = pDoc->GetRedlineTbl();
/*N*/ 	if( !bInvalid )
/*N*/ 	{
/*N*/ 		// Check for nested redlines.
/*N*/ 		sal_Bool bHasMark = pRedline->HasMark() && 
/*N*/ 							*pRedline->GetPoint() != *pRedline->GetMark();
/*N*/ 		const SwPosition *pStt = bHasMark ? pRedline->Start() : 0;
/*N*/ 		const SwPosition *pEnd = bHasMark ? pRedline->End() : 0;
/*N*/ 		sal_uInt16 nCount = rRedlineTbl.Count();
/*N*/ 		for( sal_uInt16 i=0; !bInvalid && i<nCount; i++ )
/*N*/ 		{
/*N*/ 			const SwRedline *pTest = rRedlineTbl[i];
/*N*/ 			if( pTest != pRedline )
/*N*/ 			{
/*N*/ 
/*N*/ 				// Is the ineew position is inside an existing
/*N*/ 				// redline then it is invalid.
/*N*/ 				const SwPosition *pTestStt = pTest->Start();
/*N*/ 				const SwPosition *pTestEnd = pTest->HasMark() ? pTest->End() : 0;
/*N*/ 				bInvalid = pTestEnd != 0 && *pPos > *pTestStt && *pPos < *pTestEnd;
/*N*/ 
/*N*/ 				// If the start or end of and existing redline is inside the
/*N*/ 				// redline then the new redline is invalid as well;
/*N*/ 				if( !bInvalid && bHasMark )
/*N*/ 					bInvalid = *pTestStt > *pStt && *pTestStt < *pEnd;
/*N*/ 				if( !bInvalid && bHasMark && pTestEnd != 0 )
/*N*/ 					bInvalid = *pTestEnd > *pStt && *pTestEnd < *pEnd;
/*N*/ 
/*N*/ 				ASSERT( !bInvalid, "overlapping redline" );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bNormal && !bInsert )
/*N*/ 	{
/*N*/ 		if( (0x10 & cFlags) == 0 )
/*N*/ 		{
/*N*/ 			// Die Redline darf jetzt eingefuegt werden. Das Redline-Objekt
/*N*/ 			// geht in den Besitz des Dokuments ueber.
/*N*/ 
/*N*/ 			if( !bInvalid )
/*N*/ 				bInvalid = !const_cast< SwRedlineTbl& >(rRedlineTbl).Insert(
/*N*/ 					pRedline, (BOOL)FALSE );
/*N*/ 		}
/*N*/ 		else if (bInvalid )
/*N*/ 		{
/*N*/ 			// The redline is is inserted already and has to be removed from the
/*N*/ 			// table;
/*N*/ 
/*N*/ 			sal_uInt16 nPos = rRedlineTbl.GetPos( pRedline );
/*N*/ 			ASSERT( USHRT_MAX != nPos, "inserted invalid redline not found" );
/*N*/ 			if( USHRT_MAX != nPos )
/*N*/ 				const_cast< SwRedlineTbl& >(rRedlineTbl).Remove( nPos );
/*N*/ 		}
/*N*/ 
/*N*/ 		if( bInvalid )
/*N*/ 		{
/*N*/ 			// JP 18.5.2001: Bug 87222 - then delete this invalid redline
/*N*/ 			delete pRedline;
/*N*/ 			pRedlines->Replace( 0, nId );
/*N*/ 			Warning();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_NODEREDLINE );
/*N*/ }

/*N*/ void Sw3IoImp::OutNodeRedlines( ULONG nIdx )
/*N*/ {
/*N*/ 	ASSERT( !IsSw31Or40Export(), "Redlines werden nicht exportiert!" );
/*N*/ 
/*N*/ 	if( pRedlineMarks )
/*N*/ 	{
/*?*/ 		for( USHORT nPos = 0; nPos < pRedlineMarks->Count(); ++nPos )
/*?*/ 		{
/*?*/ 			Sw3Mark *pMark = (*pRedlineMarks)[ nPos ];
/*?*/ 			if( pMark->GetNodePos() == nIdx )
/*?*/ 			{
/*?*/ 				ASSERT( pMark->GetId() < pRedlines->Count(),
/*?*/ 						"ungeuletige Redline-Id" );
/*?*/ 				Sw3MarkType eMarkType = pMark->GetType();
/*?*/ 				SwRedline *pRedline = (*pRedlines)[pMark->GetId()];
/*?*/ 				const SwNodeIndex *pContentIdx = pRedline->GetContentIdx();
/*?*/ 
/*?*/ 				OpenRec( SWG_NODEREDLINE );
/*?*/ 
/*?*/ 				BYTE cFlags = 0x04;
/*?*/ 				if( SW3_REDLINE_END == eMarkType )
/*?*/ 					cFlags |= 0x10;
/*?*/ 				else if( pContentIdx )
/*?*/ 					cFlags |= 0x20;
/*?*/ 
/*?*/ 				xub_StrLen nOffs = pMark->GetNodeOff();
/*?*/ 				if( pDoc->GetNodes()[nIdx]->IsCntntNode() &&
/*?*/ 					nOffs > STRING_MAXLEN52 )
/*?*/ 					nOffs = STRING_MAXLEN52;
/*?*/ 				*pStrm  << (BYTE)  cFlags
/*?*/ 						<< (UINT16)pMark->GetId()
/*?*/ 						<< (UINT16)nOffs;
/*?*/ 
/*?*/ 				if( pContentIdx )
/*?*/ 				{
/*?*/ 					ASSERT( pContentIdx->GetNode().EndOfSectionIndex() -
/*?*/ 						   	pContentIdx->GetIndex(),
/*?*/ 							"empty redline section exported" );
/*?*/ 					OutContents( *pContentIdx );
/*?*/ 				}
/*?*/ 
/*?*/ 				CloseRec( SWG_NODEREDLINE );
/*?*/ 
/*?*/ 				pRedlineMarks->Remove( nPos-- );
/*?*/ 				delete pMark;
/*?*/ 				if( !pRedlineMarks->Count() )
/*?*/ 				{
/*?*/ 					delete pRedlineMarks;
/*?*/ 					pRedlineMarks = NULL;
/*?*/ 					break;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else if( pMark->GetNodePos() > nIdx )
/*?*/ 				break;
/*?*/ 		}
/*N*/ 	}
/*N*/ }

 /*  */
/*N*/ namespace
/*N*/ {
/*N*/ 	void CorrRedline( SwRedline& rRedline, 
/*N*/ 					  const SwPosition& rOldPos,
/*N*/ 					  const SwPosition& rNewPos )
/*N*/ 	{
/*N*/ 		sal_uInt32 nOldNodeIdx = rOldPos.nNode.GetIndex();
/*N*/ 		if( rRedline.GetPoint()->nNode.GetIndex() == nOldNodeIdx )
/*N*/ 		{
/*N*/ 			xub_StrLen nOffset = rRedline.GetPoint()->nContent.GetIndex() - 
/*N*/ 								 rOldPos.nContent.GetIndex();
/*N*/ 			rRedline.GetPoint()->nNode = rNewPos.nNode;
/*N*/ 			rRedline.GetPoint()->nContent.Assign( 
/*N*/ 								rNewPos.nNode.GetNode().GetCntntNode(),
/*N*/ 								rNewPos.nContent.GetIndex() + nOffset );
/*N*/ 		}
/*N*/ 		if( rRedline.GetMark()->nNode.GetIndex() == nOldNodeIdx )
/*N*/ 		{
/*N*/ 			xub_StrLen nOffset = rRedline.GetMark()->nContent.GetIndex() - 
/*N*/ 								 rOldPos.nContent.GetIndex();
/*N*/ 			rRedline.GetMark()->nNode = rNewPos.nNode;
/*N*/ 			rRedline.GetMark()->nContent.Assign( 
/*N*/ 								rNewPos.nNode.GetNode().GetCntntNode(),
/*N*/ 								rNewPos.nContent.GetIndex() + nOffset );
/*N*/ 		}
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void Sw3IoImp::CleanupRedlines()
/*N*/ {
/*N*/ 	if( !pRedlines || !pRedlines->Count() )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// Bisher muss hier nur was beim Einfugen und Laden von Seiten-Vorlagen
/*N*/ 	// passieren.
/*N*/ 	if( (bNormal && bInsert) || bPageDescs )
/*N*/ 	{
/*N*/ 		// Alle geloeschten-Redlines, die angezeigt werden, muessen noch
/*N*/ 		// geloescht werden.
/*N*/ 
/*N*/ 		// In a first step, delete the redline objects that are not deletions.
/*N*/ 		sal_uInt16 i=0;
/*N*/ 		while( i < pRedlines->Count() )
/*N*/ 		{
/*N*/ 			SwRedline *pRedline = (*pRedlines)[i];
/*N*/ 			if( pRedline )
/*N*/ 			{
/*N*/ 				if( REDLINE_DELETE != pRedline->GetType() ||
/*N*/ 					pRedline->GetContentIdx() != 0 )
/*N*/ 				{
/*N*/ 					delete pRedline;
/*N*/ 					pRedlines->Remove( i );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					++i;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pRedlines->Remove( i );
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		// now process the remaining redlines
/*N*/ 		sal_uInt16 nCount = pRedlines->Count();
/*N*/ 		for( i=0; i<nCount; ++i )
/*N*/ 		{
/*N*/ 			SwRedline *pRedline = (*pRedlines)[i];
/*N*/ 			ASSERT( REDLINE_DELETE == pRedline->GetType() &&
/*N*/ 					!pRedline->GetContentIdx(), "wrong redline type" );
/*N*/ 			SwRedlineMode eOld = pDoc->GetRedlineMode();
/*N*/ 			pDoc->SetRedlineMode_intern( eOld & ~(REDLINE_ON | REDLINE_IGNORE) );
/*N*/ 			{
/*N*/ 				// If the end node of the redline is deleted (and not joined
/*N*/ 				// with the next one), then we have to make sure that none of
/*N*/ 				// the remaining redlines starts in that node.
/*N*/ 				// The end node of the redline is deleted if the PaM's point
/*N*/ 				// or if it is the PaM mark, but in this later case, the whole
/*N*/ 				// last node has to be selected.
/*N*/ 				const SwPosition& rPointPos = *pRedline->GetPoint();
/*N*/ 				const SwPosition& rMarkPos = *pRedline->GetMark();
/*N*/ 				if(	rPointPos.nNode.GetNode().IsCntntNode() &&
/*N*/ 					rMarkPos.nNode.GetNode().IsCntntNode() &&
/*N*/ 					( rPointPos.nNode.GetIndex() > rMarkPos.nNode.GetIndex() ||
/*N*/ 					  (rMarkPos.nNode.GetIndex() > rPointPos.nNode.GetIndex() &&
/*N*/ 					   rMarkPos.nContent.GetIndex() == rMarkPos.nNode.GetNode().GetCntntNode()->Len()) ) )
/*N*/ 				{
/*N*/ 					for( sal_uInt16 j= i+1; j < nCount; ++j )
/*N*/ 					{
/*N*/ 						SwRedline *pTestRedline = (*pRedlines)[j];
/*N*/ 						CorrRedline( *pTestRedline, *pRedline->End(),
/*N*/ 							   		 *pRedline->Start()	);
/*N*/ 					}
/*N*/ 				}
/*N*/ 				pDoc->DeleteAndJoin( *pRedline );
/*N*/ 				pDoc->SetRedlineMode_intern( eOld );
/*N*/ 			}
/*N*/ 			delete pRedline;
/*N*/ 		}
/*N*/ 		// Das Array wird zwar auch im CloseStreams geloscht, da wir
/*N*/ 		// die Objekte aber schon geloescht haben, ist es besser, das
/*N*/ 		// Array gleich zu loeschen.
/*N*/ 		pRedlines->Remove( 0, pRedlines->Count() );
/*N*/ 		delete pRedlines; pRedlines = 0;
/*N*/ 	}
/*N*/ }


}
