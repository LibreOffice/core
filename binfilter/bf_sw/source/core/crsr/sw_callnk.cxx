/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <com/sun/star/i18n/ScriptType.hdl>
#include <fmtcntnt.hxx>
#include <txatbase.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>

#include <horiornt.hxx>

#include <crsrsh.hxx>
#include <frmfmt.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <flyfrm.hxx>
#include <breakit.hxx>
namespace binfilter {


/*N*/ SwCallLink::SwCallLink( SwCrsrShell & rSh )
/*N*/ 	: rShell( rSh )
/*N*/ {
    // SPoint-Werte vom aktuellen Cursor merken
/*N*/ 	SwPaM* pCrsr = rShell.IsTableMode() ? rShell.GetTblCrs() : rShell.GetCrsr();
/*N*/ 	SwNode& rNd = pCrsr->GetPoint()->nNode.GetNode();
/*N*/ 	nNode = rNd.GetIndex();
/*N*/ 	nCntnt = pCrsr->GetPoint()->nContent.GetIndex();
/*N*/ 	nNdTyp = rNd.GetNodeType();
/*N*/	bHasSelection = ( *pCrsr->GetPoint() != *pCrsr->GetMark() );
/*N*/ 
/*N*/ 	if( ND_TEXTNODE & nNdTyp )
/*N*/ 		nLeftFrmPos = SwCallLink::GetFrm( (SwTxtNode&)rNd, nCntnt,
/*N*/ 											!rShell.ActionPend() );
/*N*/ 	else
/*N*/ 	{
/*?*/ 		nLeftFrmPos = 0;
/*?*/ 
/*?*/ 		// eine Sonderbehandlung fuer die SwFeShell: diese setzt beim Loeschen
/*?*/ 		// der Kopf-/Fusszeile, Fussnoten den Cursor auf NULL (Node + Content)
/*?*/ 		// steht der Cursor auf keinem CntntNode, wird sich das im NdType
/*?*/ 		// gespeichert.
/*?*/ 		if( ND_CONTENTNODE & nNdTyp )
/*?*/ 			nNdTyp = 0;
/*N*/ 	}
/*N*/ }


/*M*/ SwCallLink::~SwCallLink()
/*M*/ {
/*M*/ 	if( !nNdTyp || !rShell.bCallChgLnk )		// siehe ctor
/*M*/ 		return ;
/*M*/ 
/*M*/ 	// wird ueber Nodes getravellt, Formate ueberpruefen und im neuen
/*M*/ 	// Node wieder anmelden
/*M*/ 	SwPaM* pCurCrsr = rShell.IsTableMode() ? rShell.GetTblCrs() : rShell.GetCrsr();
/*M*/ 	SwCntntNode * pCNd = pCurCrsr->GetCntntNode();
/*M*/ 	if( !pCNd )
/*M*/ 		return;
/*M*/ 
/*M*/ 	xub_StrLen nCmp, nAktCntnt = pCurCrsr->GetPoint()->nContent.GetIndex();
/*M*/ 	USHORT nNdWhich = pCNd->GetNodeType();
/*M*/ 	ULONG nAktNode = pCurCrsr->GetPoint()->nNode.GetIndex();
/*M*/ 
/*M*/ 	// melde die Shell beim akt. Node als abhaengig an, dadurch koennen
/*M*/ 	// alle Attribut-Aenderungen ueber den Link weiter gemeldet werden.
/*M*/ 	pCNd->Add( &rShell );
/*M*/ 
/*M*/ 	if( nNdTyp != nNdWhich || nNode != nAktNode )
/*M*/ 	{
        /* immer, wenn zwischen Nodes gesprungen wird, kann es
         * vorkommen, das neue Attribute gelten; die Text-Attribute.
         * Es muesste also festgestellt werden, welche Attribute
         * jetzt gelten; das kann auch gleich der Handler machen
         */
/*M*/ 		rShell.CallChgLnk();
/*M*/ 	}
/*N*/     else if( bHasSelection != (*pCurCrsr->GetPoint() != *pCurCrsr->GetMark()) )
/*N*/     {
/*N*/         // always call change link when selection changes
/*N*/ 		rShell.CallChgLnk();
/*N*/     }
/*M*/ 	else if( rShell.aChgLnk.IsSet() && ND_TEXTNODE == nNdWhich &&
/*M*/ 			 nCntnt != nAktCntnt )
/*M*/ 	{
/*M*/ 		// nur wenn mit Left/right getravellt, dann Text-Hints pruefen
/*M*/ 		// und sich nicht der Frame geaendert hat (Spalten!)
/*M*/ 		if( nLeftFrmPos == SwCallLink::GetFrm( (SwTxtNode&)*pCNd, nAktCntnt,
/*M*/ 													!rShell.ActionPend() ) &&
/*M*/ 			(( nCmp = nCntnt ) + 1 == nAktCntnt ||			// Right
/*M*/ 			nCntnt -1 == ( nCmp = nAktCntnt )) )			// Left
/*M*/ 		{
/*M*/ 			if( nCmp == nAktCntnt && pCurCrsr->HasMark() ) // left & Sele
/*M*/ 				++nCmp;
/*M*/ 			if ( ((SwTxtNode*)pCNd)->HasHints() )
/*M*/ 			{
/*M*/ 
/*M*/ 				const SwpHints &rHts = ((SwTxtNode*)pCNd)->GetSwpHints();
/*M*/ 				USHORT n;
/*M*/ 				xub_StrLen nStart;
/*M*/ 				const xub_StrLen *pEnd;
/*M*/ 
/*M*/ 				for( n = 0; n < rHts.Count(); n++ )
/*M*/ 				{
/*M*/ 					const SwTxtAttr* pHt = rHts[ n ];
/*M*/ 					pEnd = pHt->GetEnd();
/*M*/ 					nStart = *pHt->GetStart();
/*M*/ 
/*M*/ 					// nur Start oder Start und Ende gleich, dann immer
/*M*/ 					// beim Ueberlaufen von Start callen
/*M*/ 					if( ( !pEnd || ( nStart == *pEnd ) ) &&
/*M*/ 						( nStart == nCntnt || nStart == nAktCntnt) )
/*M*/ 					{
/*M*/ 						rShell.CallChgLnk();
/*M*/ 						return;
/*M*/ 					}
/*M*/ 
/*M*/ 					// hat das Attribut einen Bereich und dieser nicht leer
/*M*/ 					else if( pEnd && nStart < *pEnd &&
/*M*/ 						// dann teste, ob ueber Start/Ende getravellt wurde
/*M*/ 						( nStart == nCmp ||
/*M*/ 							( pHt->DontExpand() ? nCmp == *pEnd-1
/*M*/ 												: nCmp == *pEnd ) ))
/*M*/ 					{
/*M*/ 						rShell.CallChgLnk();
/*M*/ 						return;
/*M*/ 					}
/*M*/ 					nStart = 0;
/*M*/ 				}
/*M*/ 			}
/*M*/ 
/*M*/ 			if( pBreakIt->xBreak.is() )
/*M*/ 			{
/*M*/ 				const String& rTxt = ((SwTxtNode*)pCNd)->GetTxt();
/*M*/ 				if( !nCmp ||
/*M*/ 					pBreakIt->xBreak->getScriptType( rTxt, nCmp )
/*M*/ 					 != pBreakIt->xBreak->getScriptType( rTxt, nCmp - 1 ))
/*M*/ 				{
/*M*/ 					rShell.CallChgLnk();
/*M*/ 					return;
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else
            /* wenn mit Home/End/.. mehr als 1 Zeichen getravellt, dann
             * immer den ChgLnk rufen, denn es kann hier nicht
             * festgestellt werden, was sich geaendert; etwas kann
             * veraendert sein.
             */
/*M*/ 			rShell.CallChgLnk();
/*M*/ 	}
/*M*/ 
/*M*/ 	const SwFrm* pFrm;
/*M*/ 	const SwFlyFrm *pFlyFrm;
/*M*/ 	if( !rShell.ActionPend() && 0 != ( pFrm = pCNd->GetFrm(0,0,FALSE) ) &&
/*M*/ 		0 != ( pFlyFrm = pFrm->FindFlyFrm() ) && !rShell.IsTableMode() )
/*M*/ 	{
/*M*/ 		const SwNodeIndex* pIndex = pFlyFrm->GetFmt()->GetCntnt().GetCntntIdx();
/*M*/ 		ASSERT( pIndex, "Fly ohne Cntnt" );
/*M*/ 		const SwNode& rStNd = pIndex->GetNode();
/*M*/ 
/*M*/ 		if( rStNd.EndOfSectionNode()->StartOfSectionIndex() > nNode ||
/*M*/ 			nNode > rStNd.EndOfSectionIndex() )
/*M*/ 			rShell.GetFlyMacroLnk().Call( (void*)pFlyFrm->GetFmt() );
/*M*/ 	}
/*M*/ }

/*N*/ long SwCallLink::GetFrm( SwTxtNode& rNd, xub_StrLen nCntPos, BOOL bCalcFrm )
/*N*/ {
/*N*/ 	SwTxtFrm* pFrm = (SwTxtFrm*)rNd.GetFrm(0,0,bCalcFrm), *pNext = pFrm;
/*N*/ 	if ( pFrm && !pFrm->IsHiddenNow() )
/*N*/ 	{
/*N*/ 		if( pFrm->HasFollow() )
/*N*/ 			while( 0 != ( pNext = (SwTxtFrm*)pFrm->GetFollow() ) &&
/*N*/ 					nCntPos >= pNext->GetOfst() )
/*N*/ 				pFrm = pNext;
/*N*/ 
/*N*/ 		return pFrm->Frm().Left();
/*N*/ 	}
/*?*/ 	return 0;
/*N*/ }

/*---------------------------------------------------------------------*/


//SwChgLinkFlag::SwChgLinkFlag( SwCrsrShell& rShell )
//    : rCrsrShell( rShell ), bOldFlag( rShell.bCallChgLnk ), nLeftFrmPos( 0 )
//{
//    rCrsrShell.bCallChgLnk = FALSE;
//    if( bOldFlag && !rCrsrShell.pTblCrsr )
//    {
//        SwNode* pNd = rCrsrShell.pCurCrsr->GetNode();
//        if( ND_TEXTNODE & pNd->GetNodeType() )
//            nLeftFrmPos = SwCallLink::GetFrm( (SwTxtNode&)*pNd,
//                    rCrsrShell.pCurCrsr->GetPoint()->nContent.GetIndex(),
//                    !rCrsrShell.ActionPend() );
//    }
//}


//SwChgLinkFlag::~SwChgLinkFlag()
//{
//    rCrsrShell.bCallChgLnk = bOldFlag;
//    if( bOldFlag && !rCrsrShell.pTblCrsr )
//    {
//        // die Spalten Ueberwachung brauchen wir immer!!!
//        SwNode* pNd = rCrsrShell.pCurCrsr->GetNode();
//        if( ND_TEXTNODE & pNd->GetNodeType() &&
//            nLeftFrmPos != SwCallLink::GetFrm( (SwTxtNode&)*pNd,
//                    rCrsrShell.pCurCrsr->GetPoint()->nContent.GetIndex(),
//                    !rCrsrShell.ActionPend() ))
//        {
//            /* immer, wenn zwischen Frames gesprungen wird, gelten
//             * neue Attribute. Es muesste also festgestellt werden, welche
//             * Attribute jetzt gelten; das kann gleich der Handler machen.
//             * Diesen direkt rufen !!!
//             */
//            rCrsrShell.aChgLnk.Call( &rCrsrShell );
//            rCrsrShell.bChgCallFlag = FALSE;        // Flag zuruecksetzen
//        }
//    }
//}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
