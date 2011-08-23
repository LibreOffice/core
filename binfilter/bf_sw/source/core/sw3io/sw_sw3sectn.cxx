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


#ifndef SVTOOLS_URIHELPER_HXX
#include <bf_svtools/urihelper.hxx>
#endif
#ifndef _SVSTDARR_USHORTS_DECL
#define _SVSTDARR_USHORTS
#endif
#ifndef _LINKMGR_HXX
#include <bf_so3/linkmgr.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _SW3IMP_HXX
#include <sw3imp.hxx>
#endif
#ifndef _SW3MARKS_HXX
#include <sw3marks.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDNOTXT_HXX
#include <ndnotxt.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>
#endif
#ifndef _SWSWERROR_H
#include <swerror.h>
#endif
#include "bf_so3/staticbaseurl.hxx"
namespace binfilter {

//////////////////////////////////////////////////////////////////////////////

// Wird ein Text in einen Absatz eingefuegt, wird der erste Absatz an den
// ersten Teil angefuegt, ohne die Formate zu uebernehmen. Alle anderen
// Absaetze uebernehmen die Formate, was auch den Rest des alten Absatzes
// betrifft.
// Die Position rPos wird weitergefuehrt. Der uebergebene Offset ist
// ein Offset in einen TextNode, der an InTxtNode() uebergeben wird.
// Falls bNode1 FALSE ist, wird ein evtl. vorhandener leerer Node nicht
// gefuellt. Dadurch laesst sich diese Methode fuer mehrere Contents
// verwenden.

/*N*/ void Sw3IoImp::InContents( SwNodeIndex& rPos, xub_StrLen nOffset, BOOL bNode1,
/*N*/ 						   BYTE nInsFirstPara, BOOL bDontMove )
/*N*/ {
/*N*/ 	OpenRec( SWG_CONTENTS );
/*N*/ 
/*N*/ 	// Der aktuelle NumRange eines 3.1/40-Files muss gerettet werden, falls
/*N*/ 	// eine Section innerhalb einer Section gelesen wird (z.B. Flys)
/*N*/ 	SwNumRule* pOld40Rule;
/*N*/ 	SwPaM* pOld40Range;
/*N*/ 	BOOL bOldConvertNoNum;
/*N*/ 	if( !IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		pOld40Rule = pCurNumRule;
/*N*/ 		pOld40Range = pCurNumRange;
/*N*/ 		bOldConvertNoNum = bConvertNoNum;
/*N*/ 		pCurNumRange = NULL;
/*N*/ 		pCurNumRule = NULL;
/*N*/ 		bConvertNoNum = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// fuer die Sonderbehandlung des ersten/letzten Node beim "Datei/Einfuegen"
/*N*/ 	BYTE nSaveInsFirstPara = nInsFirstPara;
/*N*/ 
/*N*/ 	// Bei einem echten Insert muss der aktuelle Node gesplittet werden,
/*N*/ 	// Falls mehr als 1 Node eingelesen wird.
/*N*/ 	BOOL bSplit = FALSE;
/*N*/ 	SwTxtNode* pLast = NULL;
/*N*/ 	const SwStartNode *pSttNd, *pSectSttNd = 0;
/*N*/ 	SwNode* pStart = pDoc->GetNodes()[ rPos ];
/*N*/ 	if( pStart->GetStartNode() && !bDontMove )
/*N*/ 	{
/*N*/ 		pSttNd = (const SwStartNode *)pStart;
/*N*/ 		pSectSttNd = pSttNd;
/*N*/ 
/*N*/ 		// Index zeigt auf StartNode. In Contents-Bereich reingehen
/*N*/ 		// und den ersten TextNode nehmen
/*N*/ 		rPos++;
/*N*/ 		pStart = pDoc->GetNodes()[ rPos ];
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pSttNd = pStart->FindStartNode();
/*N*/ 
/*N*/ 	SwTxtNode* pNd = bNode1 ? pStart->GetTxtNode() : NULL;
/*N*/ 	SwTxtNode* pNd1 = pNd;
/*N*/ 	ASSERT( !nInsFirstPara || pNd, "Einfuegen in Nicht-Content-Node?" );
/*N*/ 	SwPosition *pEndPos = 0;
/*N*/ 	if( nInsFirstPara && pNd )
/*N*/ 	{
/*?*/ 		SwIndex aTmp( pNd, nOffset );
/*?*/ 		pEndPos = new SwPosition( rPos, aTmp );
/*N*/ 	}
/*N*/ 
/*N*/ 	UINT32 nNodes;
/*N*/ 	if( IsVersion(SWG_LAYFRAMES) )
/*N*/ 		OpenFlagRec();
/*N*/ 	if( IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		*pStrm >> nNodes;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		UINT16 nNodes16, nSectIdDummy;
/*N*/ 		if( IsVersion(SWG_LAYFRAMES) )
/*N*/ 			*pStrm >> nSectIdDummy;
/*N*/ 		*pStrm >> nNodes16;
/*N*/ 		nNodes = nNodes16;
/*N*/ 	}
/*N*/ 	if( IsVersion(SWG_LAYFRAMES) )
/*N*/ 		CloseFlagRec();
/*N*/ 
/*N*/ 	ULONG i = 0;
/*N*/ 	while( BytesLeft() )
/*N*/ 	{
/*N*/ 		SetPercentBar( pStrm->Tell() );
/*N*/ 		BYTE cType = Peek();
/*N*/ 		i++;
/*N*/ 		switch( cType )
/*N*/ 		{
/*N*/ 			case SWG_TEXTNODE:
/*N*/ 				// Der Node muss gesplittet werden,
/*N*/ 				// wenn mehrere Nodes inserted werden
/*N*/ 				if( !bSplit && bInsert && nNodes > 1 )
/*N*/ 				{
/*?*/ 					if( pNd )
/*?*/ 					{
/*?*/ 						// natuerlich nur TextNodes splitten
/*?*/ 						SwPosition aSplitPos( rPos, SwIndex( pNd, nOffset ) );
/*?*/ 						pDoc->SplitNode( aSplitPos );
/*?*/ 						pLast = pNd;
/*?*/ 						rPos--;
/*?*/ 						pNd = pDoc->GetNodes()[ rPos ]->GetTxtNode();
/*?*/ 						bSplit = TRUE;
/*?*/ 					}
/*N*/ 				}
/*N*/ 				else if( i == nNodes && pLast )
/*N*/ 				{
/*N*/ 					// der letzte Node: nimm evtl. den gesplitteten
/*?*/ 					pNd = pLast;
/*?*/ 					if( nSaveInsFirstPara && pNd && pNd->GetTxt().Len() )
/*?*/ 						nInsFirstPara = 2;
/*N*/ 				}
/*N*/ 				InTxtNode( pNd, rPos, nOffset, nInsFirstPara );
/*N*/ 				pNd = pNd1 = NULL; nOffset = 0;
/*N*/ 				nInsFirstPara = 0;
/*N*/ 				break;
/*N*/ 			case SWG_GRFNODE:
/*N*/ 				InGrfNode( rPos );
/*N*/ 				nInsFirstPara = 0;
/*N*/ 				break;
/*N*/ 			case SWG_OLENODE:
/*N*/ 				InOLENode( rPos );
/*N*/ 				nInsFirstPara = 0;
/*N*/ 				break;
/*N*/ 			case SWG_TABLE:
/*N*/ 				// JP 20.05.94: Dok. einfuegen: wird als 1.Node eine
/*N*/ 				//				Tabelle eingefuegt, dann immer splitten !!
/*N*/ 				nInsFirstPara = 0;
/*N*/ 				if( 1 == i && bInsert && pNd )
/*N*/ 				{
/*?*/ 					SwPosition aSplitPos( rPos, SwIndex( pNd, nOffset ) );
/*?*/ 					pDoc->SplitNode( aSplitPos );
/*?*/ 					nOffset = 0;	//	mit dem Offset hat sichs erledigt!
/*N*/ 				}
/*N*/ 				InTable( rPos );
/*N*/ 				break;
/*N*/ 			case SWG_SECTION:
/*N*/ 				// JP 20.05.94: Dok. einfuegen: wird als 1.Node eine
/*N*/ 				//				Section eingefuegt, dann immer splitten !!
/*N*/ 				nInsFirstPara = 0;
/*N*/ 				if( 1 == i && bInsert && pNd )
/*N*/ 				{
/*?*/ 					SwPosition aSplitPos( rPos, SwIndex( pNd, nOffset ) );
/*?*/ 					pDoc->SplitNode( aSplitPos );
/*?*/ 					nOffset = 0;	//	mit dem Offset hat sichs erledigt!
/*N*/ 				}
/*N*/ 				InSection( rPos );
/*N*/ 				break;
/*N*/ 			case SWG_REPTEXTNODE:
/*N*/ 				// keine Spezialbehandlung fuer 1. und letzten Node neoetig,
/*N*/ 				// weil der erste und letzte Knoten eines gespeicherten
/*N*/ 				// Doks nie ein RepTextNode ist.
/*N*/ 				// MIB 21.7.97: Irrtum: Beim Einfuegen von Tabellen wird
/*N*/ 				// auch immer gesplittet.
/*N*/ 				{
/*N*/ 					BOOL bJoin = pLast && i==nNodes;
/*N*/ 					InRepTxtNode( rPos );
/*N*/ 					if( bJoin )
/*N*/ 					{
/*?*/ 						rPos--;
/*?*/ 						SwTxtNode* pTxtNode = rPos.GetNode().GetTxtNode();
/*?*/ 						if( pTxtNode && pTxtNode->CanJoinNext() )
/*?*/ 							pTxtNode->JoinNext();
/*?*/ 						rPos++;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case SWG_SDRFMT:
/*N*/ 				// Keine Draw-Formate in Kopf oder Fusszeilen einfuegen oder
/*N*/ 				// wenn kein Drawing-Layer da ist!
/*?*/ 				if( (nGblFlags & SW3F_NODRAWING) || bInsIntoHdrFtr )
/*?*/ 				{
/*?*/ 					i--; // War gar kein Node
/*?*/ 					SkipRec();
/*?*/ 					break;
/*?*/ 				} // sonst weiter:
/*?*/ 			case SWG_FLYFMT:
/*?*/ 				{
/*?*/ 					// Was auch immer jetzt passiert, einen Node lesen wir
/*?*/ 					// nicht ein.
/*?*/ 					i--;
/*?*/ 
/*?*/ 					if( SwFlyStartNode != pSttNd->GetStartNodeType() )
/*?*/ 					{
/*?*/ 						ASSERT( !this,
/*?*/ 								"Verankerung an Frames ist nur fuer Fly-Frames implementiert" );
/*?*/ 						break;
/*?*/ 					}
/*?*/ 
/*?*/ 					// Rahmengebundener FlyFrame
/*?*/ 					USHORT eSave_StartNodeType = eStartNodeType;
/*?*/ 					eStartNodeType = SwFlyStartNode;
/*?*/ 					SwFrmFmt* pFmt = (SwFrmFmt*) InFormat( cType, NULL );
/*?*/ 					eStartNodeType = eSave_StartNodeType;
/*?*/ 
/*?*/ 					if( !pFmt )
/*?*/ 						break;
/*?*/ 
/*?*/ 					// Anker darin versenken
/*?*/ 					SwFmtAnchor aAnchor( pFmt->GetAnchor() );
/*?*/ 					SwPosition aPos( *pSttNd );
/*?*/ 					aAnchor.SetAnchor( &aPos );
/*?*/ 					pFmt->SetAttr( aAnchor );
/*?*/ 
/*?*/ 					// Layout-Frames im Insert Mode fuer absatzgebundene
/*?*/ 					// Flys erzeugen
/*?*/ 					if( bInsert && !nRes )
/*?*/ 						pFmt->MakeFrms();
/*?*/ 				}
/*?*/ 				break;
/*?*/ 
/*?*/ 			case SWG_NODEREDLINE:
/*?*/ 				{
/*N*/ /*?*/ 					i--;	//SW50.SDW
/*N*/ /*?*/ 					if( nSaveInsFirstPara )
/*N*/ /*?*/ 					{
/*N*/ /*?*/ 						// Hier kann es nur einen End-Index geben. Der
/*N*/ /*?*/ 						// entspricht dann der Einfuege-Position.
/*N*/ /*?*/ 						INT32 nOffs = pEndPos->nContent.GetIndex();
/*N*/ /*?*/ 						InNodeRedline( pEndPos->nNode, nOffs, nSaveInsFirstPara );
/*N*/ /*?*/ 					}
/*N*/ /*?*/ 					else
/*N*/ /*?*/ 					{
/*N*/ /*?*/ 						SwNodeIndex aIdx( *pSttNd );
/*N*/ /*?*/ 						INT32 nDummy = 0;
/*N*/ /*?*/ 						InNodeRedline( aIdx, nDummy );
/*N*/ /*?*/ 					}
/*?*/ 				}
/*?*/ 				break;
/*?*/ 
/*?*/ 			default:
/*?*/ 				// MIB 15.4.97: Wenn wir diesen Record ueberspringen, dann
/*?*/ 				// fuegen wir auch keinen Node ein. Dann sollten wir
/*?*/ 				// vielleicht auch den Node-Zaehler wir eins runterzaehlen,
/*?*/ 				// weil sonst doch das ein oder andere schiefgehen kann.
/*?*/ 				i--;
/*?*/ 				SkipRec();
/*?*/ 				break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	CloseRec( SWG_CONTENTS );
/*N*/ 	if( !IsVersion(SWG_LONGIDX) )
/*N*/ 	{
/*N*/ 		CloseNumRange40( rPos );
/*N*/ 		pCurNumRange = pOld40Range;
/*N*/ 		pCurNumRule = pOld40Rule;
/*N*/ 		bConvertNoNum = bOldConvertNoNum;
/*N*/ 	}
/*N*/ 
/*N*/ 	// MIB 8.9.97: Wenn man eine Section fuellt, sollte man eigentlich nie
/*N*/ 	// auf einem End-Node stehen koennen, der nicht zur eigenen Section
/*N*/ 	// gehoert. Fuer den Fall, dass man keine Section laed und trotzdem
/*N*/ 	// auf einem End-Node steht, lassen wir den Code erstmal drinne, aber
/*N*/ 	// es ist doc recht fraglich, wozu er da ist.
/*N*/ 	SwEndNode *pEndNd = pDoc->GetNodes()[ rPos ]->GetEndNode();
/*N*/ 	ASSERT( !pEndNd || !pSectSttNd || pEndNd->FindStartNode()==pSectSttNd,
/*N*/ 			"PaM steht auf EndNode, der nicht zur Section gehoert." );
/*N*/ 	if( pEndNd && !pSectSttNd &&
/*N*/ 		pEndNd != &pDoc->GetNodes().GetEndOfContent())
/*N*/ 	{
/*N*/ 		rPos++;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Eine Tabelle und eine Section koennen u.U. alleine
/*N*/ 	// in einer Textsection existieren, d.h. es muss ggf.
/*N*/ 	// der ueberfluessige Node entfernt werden!
/*N*/ 	if( pNd1 && !nRes )
/*N*/ 	{
/*?*/ 		SwNodeIndex aPos( *pNd1 );
/*?*/ 		// MIB 8.9.97: Mit dieser Abrage wird sichergestellt, dass beim
/*?*/ 		// Einfuegen nie der Absatz geloscht wird, in den eingefuegt wird.
/*?*/ 		if( !pCurPaM ||
/*?*/ 			pCurPaM->GetPoint()->nNode != aPos )
/*?*/ 		{
/*?*/ 			pDoc->GetNodes().Delete( aPos );
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Die aktuelle Position ggf. hinter die Section stellen,
/*N*/ 	// die gerade eingelesen wurde.
/*N*/ 	if( pSectSttNd )
/*N*/ 		rPos = pSectSttNd->EndOfSectionIndex() + 1;
/*N*/ 
/*N*/ 	delete pEndPos;
/*N*/ }

// Einfuegen einer nicht vorhandenen Contents-Section. Die Section
// wird am Textende eingefuegt und der numerische Index des StartNodes
// zurueckgeliefert.

/*N*/ SwStartNode& Sw3IoImp::InContents()
/*N*/ {
/*N*/ 	// Anlegen einer Section mit einem TextNode
/*N*/ #if 0
/*N*/ 	SwStartNode* pSttNd = pDoc->GetNodes().MakeTextSection(
/*N*/ 								pDoc->GetNodes().EndOfAutotext,
/*N*/ 								(SwStartNodeType)eStartNodeType,
/*N*/ 								(SwTxtFmtColl*) pDoc->GetDfltTxtFmtColl() );
/*N*/ 	SwIndex aStart( pSttNd->GetMyIndex() );
/*N*/ 	InContents( aStart );
/*N*/ #endif
/*N*/ // OPT: Section leer anlegen
/*N*/ 	SwNodeIndex aStart( pDoc->GetNodes().GetEndOfAutotext() );
/*N*/ 	SwStartNode* pSttNd = pDoc->GetNodes().MakeEmptySection(  aStart,
/*N*/ 									(SwStartNodeType)eStartNodeType );
/*N*/ 	aStart = *pSttNd->EndOfSectionNode();
/*N*/ 
/*N*/ 	InContents( aStart, 0, FALSE );
/*N*/ // /OPT: Section leer anlegen
/*N*/ 
/*N*/ 	return *pSttNd;
/*N*/ }

// Einen Basis-Contents-Bereich des Dokuments ausgeben

// Der PaM zeigt (was er ja muss) immer auf einen Cntnt-Node. Somit
// muss noch getestet werden, ob dieser am Tabellenanfang liegt. In
// diesem Fall wird die Tabelle ausgegeben. Auch, wenn am Anfang eine
// Section liegt, muss dies gesondert behandelt werden!

/*N*/ void Sw3IoImp::OutContents( SwPaM* pPaM )
/*N*/ {
/*N*/ 	// Gespeichert wird immer von Point bis Mark
/*N*/ 	if( *pPaM->GetPoint() > *pPaM->GetMark() )
/*N*/ 		pPaM->Exchange();
/*N*/ 	// gebe alle Bereiche des Pams in das File aus.
/*N*/ 	ULONG nCurNode = pPaM->GetPoint()->nNode.GetIndex();
/*N*/ 	ULONG nEndNode = pPaM->GetMark()->nNode.GetIndex();
/*N*/ 	xub_StrLen nCurPos	= pPaM->GetPoint()->nContent.GetIndex();
/*N*/ 	xub_StrLen nEndPos	= STRING_LEN;
/*N*/ 	SwNode* pNd1 = pDoc->GetNodes()[ nCurNode ];
/*N*/ 
/*N*/ 	// Is the node contained in a table?
/*N*/ 	const SwTableNode* pTbl = pNd1->FindTableNode();
/*N*/ 	if( pTbl )
/*N*/ 		nCurNode = pTbl->GetIndex();
/*N*/ 
/*N*/ 	// Step out of sections, in fact to the start node of the first
/*N*/ 	// section.
/*N*/ 	// #67503#: This must be done if the first text node is contained in
/*N*/ 	// a table, too.
/*N*/ 	do
/*N*/ 	{
/*N*/ 		pNd1 = pDoc->GetNodes()[ --nCurNode ]->GetSectionNode();
/*N*/ 	} while( pNd1 );
/*N*/ 	nCurNode++;
/*N*/ 
/*N*/ 	// Dieses OutContents schreibt den Top-Level
/*N*/ 	OutContents( nCurNode, nEndNode, nCurPos, nEndPos, TRUE );
/*N*/ }

// Ausgabe einer kompletten Contents-Section
// Der uebergebene Index zeigt auf den StartNode

/*N*/ void Sw3IoImp::OutContents( const SwNodeIndex& rStart )
/*N*/ {
/*N*/ 	// Der Index zeigt auf den Start-Node, also muessen wir einen
/*N*/ 	// bauen, der auf den naechsten Node zeigt
/*N*/ 	SwStartNode* pStt = pDoc->GetNodes()[ rStart ]->GetStartNode();
/*N*/ 	ASSERT( pStt, "StartNode nicht gefunden" );
/*N*/ 	if( pStt )
/*N*/ 	{
/*N*/ 		// Hole vom Node und vom letzten Node die Position in der Section
/*N*/ 		ULONG nStt = rStart.GetIndex();
/*N*/ 		ULONG nEnd = pStt->EndOfSectionIndex()-1;
/*N*/ 		// kein Bereich also kein gueltiger Node
/*N*/ 		if( nStt <= nEnd )
/*N*/ 			OutContents( nStt, nEnd, 0, STRING_LEN );
/*N*/ 	}
/*N*/ }

// Besitzt ein Text-Knoten Markierungen oder FlyFrames?
/*N*/ BOOL lcl_sw3sectn_NodeHasFlyOrMark( Sw3IoImp& rIo, ULONG nIdx )
/*N*/ {
/*N*/ 	USHORT nPos;
/*N*/ 
/*N*/ 	if( rIo.pMarks )
/*N*/ 		for( nPos = 0; nPos < rIo.pMarks->Count(); ++nPos )
/*N*/ 		{
/*N*/ 			const Sw3Mark *pMark = (*rIo.pMarks)[ nPos ];
/*N*/ 			if( pMark->GetNodePos() == nIdx )
/*N*/ 				return TRUE;
/*N*/ 			else if( pMark->GetNodePos() > nIdx )
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 	if( rIo.pFlyFrms )
/*N*/ 		for( nPos = 0; nPos < rIo.pFlyFrms->Count(); nPos++ )
/*N*/ 		{
/*N*/ 			ULONG nIdFly = (*rIo.pFlyFrms)[nPos]->GetNdIndex().GetIndex();
/*N*/ 			if( nIdFly == nIdx )
/*N*/ 				return TRUE;
/*N*/ 			else if( nIdFly > nIdx )
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }


// Ausgabe eines fest definierten Doc-Bereichs.
// Der Offset wird beim ersten TextNode angewandt.

/*N*/ void Sw3IoImp::OutContents
/*N*/ 	( ULONG nCurNode, ULONG nEndNode, xub_StrLen nCurPos, xub_StrLen nEndPos,
/*N*/ 	  BOOL bTopLevel )
/*N*/ {
/*N*/ 	// Die aktuelle NumRuke muss gerettet werden, falls ein 3.1/40-Export
/*N*/ 	// stattfindet und eine Sectioninnerhalb einer Section geschrieben
/*N*/ 	// wird (z.B. Flys)
/*N*/ 	SwNumRule* pOld40Rule;
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		pOld40Rule = pCurNumRule;
/*N*/ 		pCurNumRule = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bOldExportFlyFrmFmt;
/*N*/ 	const SwFlyFrm* pOldExportFlyFrm;
/*N*/ 	if( pExportInfo )
/*N*/ 	{
/*N*/ 		bOldExportFlyFrmFmt = pExportInfo->bFlyFrmFmt;
/*N*/ 		pOldExportFlyFrm = pExportInfo->pFlyFrm;
/*N*/ 		pExportInfo->bFlyFrmFmt = FALSE;
/*N*/ 		pExportInfo->pFlyFrm = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwStartNode* pStt = pDoc->GetNodes()[ nCurNode ]->GetStartNode();
/*N*/ 	if( pStt && pStt->GetNodeType() == ND_STARTNODE )
/*N*/ 		nCurNode++; // StartNode nicht speichern, TblNode/SectionNode wohl
/*N*/ 	else
/*N*/ 		pStt = pDoc->GetNodes()[ nCurNode ]->FindStartNode();
/*N*/ 	OpenRec( SWG_CONTENTS );
/*N*/ 	*pStrm << (BYTE)   0x04; 		// 4 Byte Daten, aber unterschiedliche
/*N*/ 
/*N*/ 	ULONG nNodes = 0;
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		*pStrm << (UINT16) IDX_NO_VALUE;	// war mal Section-Id (siehe Tbl)
/*N*/ 		OpenValuePos16( (UINT16)nNodes );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		OpenValuePos32( nNodes );
/*N*/ 	}
/*N*/ 
/*N*/ 	nNodes = OutNodes( nCurNode, nEndNode, nCurPos, nEndPos, bTopLevel );
/*N*/ 
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 		CloseValuePos16( (UINT16)nNodes );
/*N*/ 	else
/*N*/ 		CloseValuePos32( nNodes );
/*N*/ 
/*N*/ 	// Jetzt noch an den Start-Node der Section (Rahmen)gebundenen
/*N*/ 	// Rahmen und Redlines rausschreiben.
/*N*/ 	if( !IsSw31Or40Export() )
/*N*/ 	{
/*N*/ 		OutNodeFlyFrames( pStt->GetIndex() );
/*N*/ 		OutNodeRedlines( pStt->GetIndex() );
/*N*/ 		OutNodeRedlines( pStt->EndOfSectionIndex() );
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	CloseRec( SWG_CONTENTS );
/*N*/ 
/*N*/ 	if( IsSw31Or40Export() )
/*N*/ 		pCurNumRule = pOld40Rule;
/*N*/ 
/*N*/ 	if( pExportInfo )
/*N*/ 	{
/*N*/ 		pExportInfo->bFlyFrmFmt = bOldExportFlyFrmFmt;
/*N*/ 		pExportInfo->pFlyFrm = pOldExportFlyFrm;
/*N*/ 	}
/*N*/ }

/*N*/ ULONG Sw3IoImp::OutNodes( ULONG nCurNode, ULONG nEndNode,
/*N*/ 						  xub_StrLen nCurPos, xub_StrLen nEndPos,
/*N*/ 						  BOOL bTopLevel )
/*N*/ {
/*N*/ 	ULONG nNodes = 0;
/*N*/ 
/*N*/ 	ULONG nWords, nChars;
/*N*/ 	ULONG nRepNodesToWrite = 0;
/*N*/ 	SwTxtNode *pLastNode = NULL;
/*N*/ 	BOOL bFirstNode = bTopLevel;
/*N*/ 
/*N*/ 	while( nCurNode <= nEndNode && Good() )
/*N*/ 	{
/*N*/ 		SetPercentBar( nCurNode );
/*N*/ 		SwNode* pNd = pDoc->GetNodes()[ nCurNode ];
/*N*/ 
/*N*/ 		BYTE nNodeType = pNd->GetNodeType();
/*N*/ 
/*N*/ 		if( nNodeType==ND_TEXTNODE )
/*N*/ 		{
/*N*/ 			// Ist der Knoten einer Wiederholung des vorherigen?
/*N*/ 			// NIE(!!!) wenn dies der letzte zu speichernde Knoten des
/*N*/ 			// Top-Levels ist, weil dies beim Einfuegen des Doks Probleme
/*N*/ 			// bereiten kann.
/*N*/ 			SwTxtNode *pTxtNd = pNd->GetTxtNode();
/*N*/ 			if( pLastNode &&
/*N*/ 				(!bTopLevel || nCurNode!=nEndNode) &&
/*N*/ 				pLastNode->GetFmtColl() == pTxtNd->GetFmtColl() &&
/*N*/ 				pLastNode->GetCondFmtColl() == pTxtNd->GetCondFmtColl() &&
/*N*/ 				( ( 0 == pLastNode->GetNum() && 
/*N*/                     0 == pTxtNd->GetNum() ) ||
/*N*/                   ( pLastNode->GetNum() && pTxtNd->GetNum() &&
/*N*/                     ( pLastNode->GetNum()->GetLevel() ==
/*N*/                       pTxtNd->GetNum()->GetLevel() ) ) ) &&
/*N*/ 				!pTxtNd->HasHints() &&
/*N*/ 				!pTxtNd->GetpSwAttrSet() &&
/*N*/ 				pLastNode->GetTxt().Len() == pTxtNd->GetTxt().Len() &&
/*N*/ 				pLastNode->GetTxt() == pTxtNd->GetTxt() &&
/*N*/ 				!lcl_sw3sectn_NodeHasFlyOrMark( *this, nCurNode ) )
/*N*/ 			{
/*N*/ 				nRepNodesToWrite++;
/*N*/ 				nCurNode++;
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			// Den ersten Knoeten des Top-Levels duerfen wir nie als
/*N*/ 			// Ausgangsbasis einer Wiederholung benutzen, sonst gibt's beim
/*N*/ 			// Einfuegen des Doks in ein anderes Probleme, weil der
/*N*/ 			// Knoten kopiert wird.
/*N*/ 			if( !bFirstNode &&
/*N*/ 				!pTxtNd->HasHints() &&
/*N*/ 				!pTxtNd->GetpSwAttrSet() &&
/*N*/ 				!lcl_sw3sectn_NodeHasFlyOrMark( *this, nCurNode ) )
/*N*/ 				pLastNode = pTxtNd;
/*N*/ 			else
/*N*/ 				pLastNode = NULL;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pLastNode = NULL;
/*N*/ 
/*N*/ 		bFirstNode = FALSE;
/*N*/ 
/*N*/ 		if( nRepNodesToWrite>0 )
/*N*/ 		{
/*N*/ 			OutRepTxtNode( nRepNodesToWrite );
/*N*/ 			aStat.nPara += nRepNodesToWrite;
/*N*/ 			aStat.nWord += nRepNodesToWrite*(aStat.nWord-nWords);
/*N*/ 			aStat.nChar += nRepNodesToWrite*(aStat.nChar-nChars);
/*N*/ 			nRepNodesToWrite = 0;
/*N*/ 			nNodes++;
/*N*/ 		}
/*N*/ 
/*N*/ 		switch( (int)nNodeType	)
/*N*/ 		{
/*N*/ 			case ND_SECTIONNODE:
/*N*/ 				nNodes += OutSection( *pNd->GetSectionNode() );
/*N*/ 				nCurNode = pNd->GetSectionNode()->EndOfSectionIndex() + 1;
/*N*/ 				break;
/*N*/ 			case ND_TABLENODE:
/*N*/ 				OutTable( *pNd->GetTableNode() );
/*N*/ 				nCurNode = pNd->GetTableNode()->EndOfSectionIndex() + 1;
/*N*/ 				break;
/*N*/ 			case ND_TEXTNODE:
/*N*/ 				nWords = aStat.nWord;
/*N*/ 				nChars = aStat.nChar;
/*N*/ 				OutTxtNode( *pNd->GetTxtNode(), nCurPos, nEndPos, nCurNode );
/*N*/ 				nCurPos = 0; nCurNode++;
/*N*/ 				break;
/*N*/ 			case ND_GRFNODE:
/*N*/ 				OutGrfNode( *pNd->GetNoTxtNode() );
/*N*/ 				nCurPos = 0; nCurNode++;
/*N*/ 				break;
/*N*/ 			case ND_OLENODE:
/*N*/ 				OutOLENode( *pNd->GetNoTxtNode() );
/*N*/ 				nCurPos = 0; nCurNode++;
/*N*/ 				break;
/*?*/ 			case ND_ENDNODE:
/*?*/ 				// Das kann der Teil einer Section sein,
/*?*/ 				// kann einfach ignoriert werden
/*?*/ 				nNodes--; nCurNode++; break;
/*?*/ 			default:
/*?*/ 				ASSERT( !this, "Node kann nicht gespeichert werden" );
/*?*/ 				Error( ERR_SWG_WRITE_ERROR );
/*?*/ 				nCurNode = nEndNode;
/*N*/ 		}
/*N*/ 		nNodes++;
/*N*/ 	}
/*N*/ 
/*N*/ 	// falls der letzte Knoten eine Wiederholung ist:
/*N*/ 	if( nRepNodesToWrite>0 && Good() )
/*N*/ 	{
/*?*/ 		OutRepTxtNode( nRepNodesToWrite );
/*?*/ 		aStat.nPara += nRepNodesToWrite;
/*?*/ 		aStat.nWord += nRepNodesToWrite*(aStat.nWord-nWords);
/*?*/ 		aStat.nChar += nRepNodesToWrite*(aStat.nChar-nChars);
/*?*/ 		nRepNodesToWrite = 0;
/*?*/ 		nNodes++;
/*N*/ 	}
/*N*/ 
/*N*/ 	return nNodes;
/*N*/ }

// Einlesen einer "echten" Section innerhalb eines Content-Bereichs
// Auch hier wird der uebergebene Index weitergefuehrt.

/*N*/ void Sw3IoImp::InSection( SwNodeIndex& rPos )
/*N*/ {
/*N*/ 	OpenRec( SWG_SECTION );
/*N*/ 	String aName, aCond;
/*N*/ 	InString( *pStrm, aName );
/*N*/ 	InString( *pStrm, aCond );
/*N*/ 	// 0x10 - hidden
/*N*/ 	// 0x20 - protected
/*N*/ 	// 0x40 - !conditional hidden (gedreht, fuers Einlesen von alten Docs)
/*N*/ 	BYTE cFlags = OpenFlagRec();
/*N*/ 	UINT16 nType;
/*N*/ 	*pStrm >> nType;
/*N*/ 	CloseFlagRec();
/*N*/ 
/*N*/ 	// beim Insert vom Doc einen eindeutigen Namen erzeugen
/*N*/ 	if( bInsert )
/*N*/ 	{
/*?*/ 		aName = pDoc->GetUniqueSectionName( &aName );
/*?*/ 		if( !pSectionDepths )
/*?*/ 			pSectionDepths = new SvUShorts;
/*?*/ 		if( 0 == pSectionDepths->Count() )
/*?*/ 		{
/*?*/ 			pSectionDepths->Insert( 1U, 0U );
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			(*pSectionDepths)[pSectionDepths->Count()-1]++;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	SwSection aSect( (SectionType) nType, aName );
/*N*/ 	SwSectionFmt* pFmt = (SwSectionFmt*) InFormat( SWG_SECTFMT, NULL );
/*N*/ 	ULONG n = rPos.GetIndex();
/*N*/ 
/*N*/ 	SwNodeIndex aTmpIdx( pDoc->GetNodes().GetEndOfContent() );
/*N*/ 	SwSectionNode* pSectNd =
/*N*/ 		pDoc->GetNodes().InsertSection( rPos, *pFmt, aSect, &aTmpIdx, FALSE );
/*N*/ 
/*N*/ 	SwSection& rNdSection = pSectNd->GetSection();
/*N*/ 
/*N*/ 	rPos = n;
/*N*/ 	InContents( rPos );
/*N*/ 
/*N*/ 	// Link-Filenamen einlesen
/*N*/ 	if( nVersion >= SWG_FLYWRAPCHGD )
/*N*/ 	{
/*N*/ 		String aLinkFileName;
/*N*/ 		ByteString s8;
/*N*/ 
/*N*/ 		pStrm->ReadByteString( s8 );
/*N*/         aLinkFileName = ConvertStringNoDelim( s8, '\xff', ::binfilter::cTokenSeperator,
/*N*/ 											  eSrcSet );
/*N*/ 
/*N*/ 		if( aLinkFileName.Len() && FILE_LINK_SECTION == nType )
/*N*/ 		{
/*N*/             xub_StrLen nTokenPos = aLinkFileName.Search( ::binfilter::cTokenSeperator );
/*N*/ 			if( STRING_NOTFOUND != nTokenPos && nTokenPos )
/*N*/ 			{
/*N*/ 				String sURL( aLinkFileName.Copy( 0, nTokenPos ) );
/*N*/ 				aLinkFileName.Erase( 0, nTokenPos );
/*N*/ 				aLinkFileName.Insert( ::binfilter::StaticBaseUrl::SmartRelToAbs( sURL ), 0 );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		rNdSection.SetLinkFileName( aLinkFileName );
/*N*/ 	}
/*N*/ 
/*N*/ 	rNdSection.SetCondition( aCond );
/*N*/ 	if( cFlags & 0x10 )
/*?*/ 		rNdSection.SetHidden();
/*N*/ 	if( cFlags & 0x20 )
/*N*/ 		rNdSection.SetProtect();
/*N*/ 	if( cFlags & 0x40 )
/*?*/ 		rNdSection.SetCondHidden( FALSE );
/*N*/ 	if( cFlags & 0x80 )
/*?*/ 		rNdSection.SetConnectFlag( FALSE );
/*N*/ 
/*N*/ 	// ggf. Link neu verbinden aber nicht updaten
/*N*/ 	if( pSectNd->GetSection().IsLinkType() )
/*N*/ 		pSectNd->GetSection().CreateLink( CREATE_CONNECT );
/*N*/ 
/*N*/ 	// create frames
/*N*/ 	if( bInsert )
/*N*/ 	{
/*?*/ 		(*pSectionDepths)[pSectionDepths->Count()-1]--;
/*?*/ 		if( 0 == (*pSectionDepths)[pSectionDepths->Count()-1] )
/*?*/ 		{
/*?*/ 			SwSectionFmt *pSFmt = rNdSection.GetFmt();
/*?*/ 			pSFmt->DelFrms();
/*?*/ 			pSFmt->MakeFrms();
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	CloseRec( SWG_SECTION );
/*N*/ }

/*N*/ BOOL lcl_sw3io_isTOXHeaderSection( const SwStartNode& rSttNd )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 
/*N*/ 	const SwSectionNode *pSectNd = rSttNd.GetSectionNode();
/*N*/ 	if( pSectNd &&
/*N*/ 		TOX_HEADER_SECTION == pSectNd->GetSection().GetType() )
/*N*/ 	{
/*N*/ 		bRet = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*N*/ ULONG Sw3IoImp::OutTOXSection( const SwSectionNode& rNd )
/*N*/ {
/*N*/ 	const SwSection& rSect = rNd.GetSection();
/*N*/ 
/*N*/ 	ASSERT( TOX_HEADER_SECTION == rSect.GetType() ||
/*N*/ 			TOX_CONTENT_SECTION == rSect.GetType(),
/*N*/ 			"Not a TOX section" );
/*N*/ 
/*N*/ 	ULONG nStt = rNd.GetIndex() + 1;
/*N*/ 	ULONG nEnd = rNd.EndOfSectionIndex()-1;
/*N*/ 
/*N*/ 	if( nStt > nEnd )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	ULONG nNodes = 0;
/*N*/ 
/*N*/ 	// If a tox content section starts with a start node or ends with
/*N*/ 	// an end node that doesn't belong to a tox header section, an
/*N*/ 	// additional text node has to be inserted.
/*N*/ 
/*N*/ 	//                  +-- tox c   +-- tox c   +-- tox c   +-- tox c
/*N*/ 	//                 *|   text   *|+- stt     |+- tox h  *|+- tox h
/*N*/ 	//                  |           ||         *||  text    ||+ stt
/*N*/ 	//
/*N*/ 	//                  |           ||         *||  text    ||+ end
/*N*/ 	//                 *|   text   *|+- end     |+- tox h  *|+- tox h
/*N*/ 	//                  +-- tox c   +-- tox c   +-- tox c   +-- tox c
/*N*/ 	//
/*N*/ 	// node before:     no          yes         no          yes
/*N*/ 	// node behind:     no          yes         no          yes
/*N*/ 	//
/*N*/ 	// The * indicate the position of the tox marks
/*N*/ 
/*N*/ 	if( TOX_CONTENT_SECTION == rSect.GetType() )
/*N*/ 	{
/*N*/ 		const SwStartNode *pSttNd = pDoc->GetNodes()[nStt]->GetStartNode();
/*N*/ 		if( pSttNd && ( !lcl_sw3io_isTOXHeaderSection( *pSttNd ) ||
/*N*/ 						!pDoc->GetNodes()[nStt+1]->IsTxtNode() ) )
/*N*/ 		{
/*?*/ 			OutEmptyTxtNode( nStt, TRUE );
/*?*/ 			nNodes++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	nNodes += OutNodes( nStt, nEnd, 0, STRING_LEN, FALSE );
/*N*/ 
/*N*/ 	if( TOX_CONTENT_SECTION == rSect.GetType() )
/*N*/ 	{
/*N*/ 		const SwEndNode *pEndNd = pDoc->GetNodes()[nEnd]->GetEndNode();
/*N*/ 		if( pEndNd &&
/*N*/ 			( !lcl_sw3io_isTOXHeaderSection(*pEndNd->StartOfSectionNode()) ||
/*N*/ 			  !pDoc->GetNodes()[nEnd-1]->IsTxtNode() ) )
/*N*/ 		{
/*?*/ 			OutEmptyTxtNode( nEnd, TRUE );
/*?*/ 			nNodes++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// The sections start node is counted by OutNodes, but it hasn't been
/*N*/ 	// written. That for, the number of nodes must be reduced by one.
/*N*/ 	ASSERT( nNodes > 0, "empty TOX section?" );
/*N*/ 	return nNodes - 1;
/*N*/ }

/*N*/ ULONG Sw3IoImp::OutSection( const SwSectionNode& rNd )
/*N*/ {
/*N*/ 	const SwSection& rSect = rNd.GetSection();
/*N*/ 
/*N*/ 	if( TOX_HEADER_SECTION == rSect.GetType() ||
/*N*/ 		TOX_CONTENT_SECTION == rSect.GetType() )
/*N*/ 	{
/*N*/ 		return OutTOXSection( rNd );
/*N*/ 	}
/*N*/ 
/*N*/ 	OpenRec( SWG_SECTION );
/*N*/ 	OutString( *pStrm, rSect.GetName() );
/*N*/     OutString( *pStrm, rSect.GetCondition() );
/*N*/ 	// 0x02 - 2 Bytes Daten (Typ)
/*N*/ 	// 0x10 - hidden
/*N*/ 	// 0x20 - protected
/*N*/ 	// 0x40 - !conditional hidden (gedreht, fuers Einlesen von alten Docs)
/*N*/ 	BYTE cFlags = 0x02;
/*N*/ 	if( rSect.IsHidden() )
/*N*/ 		cFlags |= 0x10;
/*N*/ 	if( rSect.IsProtect() )
/*N*/ 		cFlags |= 0x20;
/*N*/ 	if( !rSect.IsCondHidden() )
/*N*/ 		cFlags |= 0x40;
/*N*/ 	if( !rSect.IsConnectFlag() )
/*N*/ 		cFlags |= 0x80;
/*N*/ 	*pStrm << (BYTE) cFlags
/*N*/ 		   << (UINT16) rSect.GetType();
/*N*/ 
/*N*/ 	SwSectionFmt* pFmt = rSect.GetFmt();
/*N*/ 	OutFormat( SWG_SECTFMT, *rSect.GetFmt() );
/*N*/ 
/*N*/ 	// Hier einen auf den StartNode draufzaehlen, sonst wird
/*N*/ 	// der Bereich rekursiv gespeichert!
/*N*/ 	ULONG nStt = rNd.GetIndex() + 1;
/*N*/ 	ULONG nEnd = rNd.EndOfSectionIndex()-1;
/*N*/ 	// kein Bereich also kein gueltiger Node
/*N*/ 	if( nStt <= nEnd )
/*N*/ 	{
/*N*/ 		if( !IsSw31Export() &&
/*N*/ 			pDoc->IsGlobalDoc() && !pDoc->IsGlblDocSaveLinks() &&
/*N*/ 			pFmt->GetGlobalDocSection() )
/*N*/ 		{
/*N*/ 			// nur eine Section mit einem DummyTextNode speichern!
/*N*/ 			OpenRec( SWG_CONTENTS );
/*N*/ 			*pStrm << (BYTE)   0x04;	// 4 Byte Daten
/*N*/ 
/*N*/ 			if( IsSw31Or40Export() )
/*N*/ 			{
/*?*/ 				*pStrm << (UINT16) IDX_NO_VALUE; // war mal Section-Id
/*?*/ 				OpenValuePos16( 0 );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				OpenValuePos32( 0 );
/*N*/ 			}
/*N*/ 
/*N*/ 			// leeren TextNode schreiben
/*N*/ 			OutEmptyTxtNode();
/*N*/ 
/*N*/ 			if( IsSw31Or40Export() )
/*?*/ 				CloseValuePos16( 1 );
/*N*/ 			else
/*N*/ 				CloseValuePos32( 1 );
/*N*/ 
/*N*/ 			CloseRec( SWG_CONTENTS );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			OutContents( nStt, nEnd, 0, STRING_LEN );
/*N*/ 	}
/*N*/ 	// Link-Filenamen schreiben
/*N*/ 	String aLinkFileName( rSect.GetLinkFileName() );
/*N*/ 	if( aLinkFileName.Len() && FILE_LINK_SECTION == rSect.GetType() )
/*N*/ 	{
/*N*/         xub_StrLen nTokenPos = aLinkFileName.Search( ::binfilter::cTokenSeperator );
/*N*/ 		if( STRING_NOTFOUND != nTokenPos && nTokenPos )
/*N*/ 		{
/*N*/ 			String sURL( aLinkFileName.Copy( 0, nTokenPos ) );
/*N*/ 			aLinkFileName.Erase( 0, nTokenPos );
/*N*/ 			aLinkFileName.Insert( ::binfilter::StaticBaseUrl::AbsToRel( sURL ), 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     ByteString s8 = ConvertStringNoDelim( aLinkFileName, ::binfilter::cTokenSeperator,
/*N*/ 										  '\xff', eSrcSet );
/*N*/ 	pStrm->WriteByteString( s8 );
/*N*/ 
/*N*/ 	CloseRec( SWG_SECTION );
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

// NEXT: sw3sectn_0a


}
