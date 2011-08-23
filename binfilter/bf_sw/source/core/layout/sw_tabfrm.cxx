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

#include "pagefrm.hxx"
#include "viewsh.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"
#include "viewimp.hxx"
#include "swtable.hxx"
#include "dflyobj.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "hints.hxx"
#include "dbg_lay.hxx"

#include <ftnidx.hxx>

#ifndef _SFXITEMITER_HXX //autogen
#include <bf_svtools/itemiter.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <bf_svx/keepitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "cellfrm.hxx"
#include "flyfrms.hxx"
#include "txtfrm.hxx"		//HasFtn()
#include "htmltbl.hxx"
#include "frmsh.hxx"
#include "sectfrm.hxx"	//SwSectionFrm
namespace binfilter {

/*N*/ extern void AppendObjs( const SwSpzFrmFmts *pTbl, ULONG nIndex,
/*N*/ 						SwFrm *pFrm, SwPageFrm *pPage );

/*************************************************************************
|*
|*	SwTabFrm::SwTabFrm(), ~SwTabFrm()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 30. May. 96
|*
|*************************************************************************/
/*N*/ SwTabFrm::SwTabFrm( SwTable &rTab ):
/*N*/ 	SwLayoutFrm( rTab.GetFrmFmt() ),
/*N*/ 	SwFlowFrm( (SwFrm&)*this ),
/*N*/ 	pTable( &rTab )
/*N*/ {
/*N*/ 	bComplete = bCalcLowers = bONECalcLowers = bLowersFormatted = bLockBackMove =
/*N*/ 	bResizeHTMLTable = FALSE;
/*N*/     BFIXHEIGHT = FALSE;     //Nicht nochmal auf die Importfilter hereinfallen.
/*N*/     nType = FRMC_TAB;
/*N*/ 
/*N*/ 	//Gleich die Zeilen erzeugen und einfuegen.
/*N*/ 	const SwTableLines &rLines = rTab.GetTabLines();
/*N*/ 	SwFrm *pPrev = 0;
/*N*/ 	for ( USHORT i = 0; i < rLines.Count(); ++i )
/*N*/ 	{
/*N*/ 		SwRowFrm *pNew = new SwRowFrm( *rLines[i] );
/*N*/ 		if( pNew->Lower() )
/*N*/ 		{
/*N*/ 			pNew->InsertBehind( this, pPrev );
/*N*/ 			pPrev = pNew;
/*N*/ 		}
/*N*/ 		else
/*?*/ 			delete pNew;
/*N*/ 	}
/*N*/ }

/*N*/ SwTabFrm::SwTabFrm( SwTabFrm &rTab ) :
/*N*/ 	SwLayoutFrm( rTab.GetFmt() ),
/*N*/ 	SwFlowFrm( (SwFrm&)*this ),
/*N*/ 	pTable( rTab.GetTable() )
/*N*/ {
/*N*/ 	bIsFollow = TRUE;
/*N*/ 	bLockJoin = bComplete = bONECalcLowers = bCalcLowers = bLowersFormatted = bLockBackMove =
/*N*/ 	bResizeHTMLTable = FALSE;
/*N*/     BFIXHEIGHT = FALSE;     //Nicht nochmal auf die Importfilter hereinfallen.
/*N*/     nType = FRMC_TAB;
/*N*/ 
/*N*/ 	SetFollow( rTab.GetFollow() );
/*N*/ 	rTab.SetFollow( this );
/*N*/ }

/*N*/ SwTabFrm::~SwTabFrm()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::JoinAndDelFollows()
|*
|*	Ersterstellung		MA 30. May. 96
|*	Letzte Aenderung	MA 30. May. 96
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwTabFrm::RegistFlys()
|*
|*	Ersterstellung		MA 08. Jul. 93
|*	Letzte Aenderung	MA 27. Jan. 99
|*
|*************************************************************************/
/*N*/ void SwTabFrm::RegistFlys()
/*N*/ {
/*N*/ 	ASSERT( Lower() && Lower()->IsRowFrm(), "Keine Zeilen." );
/*N*/ 
/*N*/ 	SwPageFrm *pPage = FindPageFrm();
/*N*/ 	if ( pPage )
/*N*/ 	{
/*N*/ 		SwRowFrm *pRow = (SwRowFrm*)Lower();
/*N*/ 		do
/*N*/ 		{	pRow->RegistFlys( pPage );
/*N*/ 			pRow = (SwRowFrm*)pRow->GetNext();
/*N*/ 		} while ( pRow );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::Split(), Join()
|*
|*	Ersterstellung		MA 03. Jun. 93
|*	Letzte Aenderung	MA 03. Sep. 96
|*
|*************************************************************************/
/*N*/ SwTwips SwTabFrm::Split( const SwTwips nCutPos )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/     ASSERT( bVert ? nCutPos >= Frm().Left()
/*N*/             && nCutPos <= Frm().Left() + Frm().Width() :
/*N*/             nCutPos >= Frm().Top() && nCutPos <= Frm().Bottom(),
/*N*/             "SplitLine out of table." );
/*N*/ 
/*N*/ 	//Um die Positionen der Zellen mit der CutPos zu vergleichen muessen sie
/*N*/ 	//ausgehend von der Tabelle nacheinander berechnet werden. Sie koennen
/*N*/ 	//wg. Positionsaenderungen der Tabelle durchaus ungueltig sein.
/*N*/ 
/*N*/ 	SwFrm *pRow = Lower();
/*N*/ 	if( !pRow )
/*?*/ 		return 0;
/*N*/     SwTwips nCut = (*fnRect->fnYDiff)( nCutPos, (Frm().*fnRect->fnGetTop)() );
/*N*/     nCut -= (this->*fnRect->fnGetTopMargin)();
/*N*/     SwTwips nRowPos = (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/ 	const BOOL bRepeat	= GetTable()->IsHeadlineRepeat();
/*N*/ 	pRow = pRow->GetNext();
/*N*/ 	if( pRow && bRepeat )
/*N*/ 	{
/*N*/         nRowPos += (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/ 		pRow = pRow->GetNext();
/*N*/ 	}
/*N*/ 	// No break before the first row and, in case of repeated headlines,
/*N*/ 	// before the the second row.
/*N*/ 	if( !pRow )
/*?*/ 		return 0;
/*N*/ 
/*N*/     while( pRow && nCut >= ( nRowPos + (pRow->Frm().*fnRect->fnGetHeight)() ) )
/*N*/ 	{
/*N*/         nRowPos += (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/ 		pRow = pRow->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !pRow )
/*N*/ 	{
/*?*/ #if OSL_DEBUG_LEVEL > 1
/*?*/ 		ASSERT( FALSE, "Tablesplit out of rows?" );
/*?*/ #endif
/*?*/ 		pRow = Lower();
/*?*/ 		while ( pRow && pRow->GetNext() )
/*?*/ 			pRow = pRow->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	//Wenn es bereits einen Follow gibt so geht's dort hinein andernfalls
/*N*/ 	//muss eben einer erzeugt werden.
/*N*/ 	FASTBOOL bNewFollow;
/*N*/ 	SwTabFrm *pFoll;
/*N*/ 	if ( GetFollow() )
/*N*/ 	{
/*N*/ 		pFoll = GetFollow();
/*N*/ 		bNewFollow = FALSE;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bNewFollow = TRUE;
/*N*/ 		pFoll = new SwTabFrm( *this );
/*N*/ 		pFoll->InsertBehind( GetUpper(), this );
/*N*/ 
/*N*/ 		if( bRepeat )
/*N*/ 		{	//Ueberschrift wiederholen.
/*N*/ 			ASSERT( GetTable()->GetTabLines()[0], "Table ohne Zeilen?" );
/*N*/ 			bDontCreateObjects = TRUE;				//frmtool
/*N*/ 			SwRowFrm *pHeadline = new SwRowFrm(
/*N*/ 									*GetTable()->GetTabLines()[0] );
/*N*/ 			bDontCreateObjects = FALSE;
/*N*/ 			pHeadline->InsertBefore( pFoll, 0 );
/*N*/ 
/*N*/ 			SwPageFrm *pPage = pHeadline->FindPageFrm();
/*N*/ 			const SwSpzFrmFmts *pTbl = GetFmt()->GetDoc()->GetSpzFrmFmts();
/*N*/ 			if( pTbl->Count() )
/*N*/ 			{
/*N*/ 				ULONG nIndex;
/*N*/ 				SwCntntFrm* pFrm = pHeadline->ContainsCntnt();
/*N*/ 				while( pFrm )
/*N*/ 				{
/*N*/ 					nIndex = pFrm->GetNode()->GetIndex();
/*N*/ 					AppendObjs( pTbl, nIndex, pFrm, pPage );
/*N*/ 					pFrm = pFrm->GetNextCntntFrm();
/*N*/ 					if( !pHeadline->IsAnLower( pFrm ) )
/*N*/ 						break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SwTwips nRet = 0;
/*N*/ 	SwFrm *pNxt;
/*N*/ 
/*N*/ 	//Optimierung beim neuen Follow braucht's kein Paste und dann kann
/*N*/ 	//das Optimierte Insert verwendet werden (nur dann treten gluecklicher weise
/*N*/ 	//auch groessere Mengen von Rows auf).
/*N*/ 	if ( bNewFollow )
/*N*/ 	{
/*N*/ 		SwFrm *pPrv = GetTable()->IsHeadlineRepeat() ? pFoll->Lower() : 0;
/*N*/ 		while ( pRow )
/*N*/ 		{
/*N*/ 			pNxt = pRow->GetNext();
/*N*/             nRet += (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/ 			pRow->Remove();
/*N*/ 			pRow->InsertBehind( pFoll, pPrv );
/*N*/ 			pRow->_InvalidateAll();
/*N*/ 			pPrv = pRow;
/*N*/ 			pRow = pNxt;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwFrm *pPrv = pFoll->Lower();
/*N*/ 		if ( pPrv && GetTable()->IsHeadlineRepeat() )
/*N*/ 			pPrv = pPrv->GetNext();
/*N*/ 		while ( pRow )
/*N*/ 		{
/*N*/ 			pNxt = pRow->GetNext();
/*N*/             nRet += (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/             pRow->Remove();
/*N*/ 			pRow->Paste( pFoll, pPrv );
/*N*/             pRow->CheckDirChange();
/*N*/ 			pRow = pNxt;
/*N*/ 		}
/*N*/ 	}
/*N*/     ASSERT( !bNewFollow || !(pFoll->Frm().*fnRect->fnGetHeight)(),
/*N*/             "Dont care about Performance");
/*N*/     Shrink( nRet );
/*N*/ 	return nRet;
/*N*/ }

/*N*/ SwTwips SwTabFrm::Join()
/*N*/ {
/*N*/ 	SwTabFrm *pFoll = GetFollow();
/*N*/ 	SwTwips nHeight = 0;	//Gesamthoehe der eingefuegten Zeilen als Return.
/*N*/ 
/*N*/ 	if ( !pFoll->IsJoinLocked() )
/*N*/ 	{
/*N*/         SWRECTFN( this )
/*N*/ 		pFoll->Cut();	//Erst ausschneiden um unuetze Benachrichtigungen zu
/*N*/ 						//minimieren.
/*N*/ 
/*N*/ 		SwFrm *pRow = pFoll->Lower(),
/*N*/ 			  *pNxt;
/*N*/ 
/*N*/ 		if ( pRow && GetTable()->IsHeadlineRepeat() )
/*N*/ 			pRow = pRow->GetNext();
/*N*/ 
/*N*/ 		SwFrm *pPrv = Lower();
/*N*/ 		while ( pPrv && pPrv->GetNext() )
/*N*/ 			pPrv = pPrv->GetNext();
/*N*/ 		while ( pRow )
/*N*/ 		{
/*N*/ 			pNxt = pRow->GetNext();
/*N*/             nHeight += (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/ 			pRow->Remove();
/*N*/ 			pRow->_InvalidateAll();
/*N*/ 			pRow->InsertBehind( this, pPrv );
/*N*/             pRow->CheckDirChange();
/*N*/ 			pPrv = pRow;
/*N*/ 			pRow = pNxt;
/*N*/ 		}
/*N*/ 		SetFollow( pFoll->GetFollow() );
/*N*/ 		delete pFoll;
/*N*/         Grow( nHeight PHEIGHT );
/*N*/ 	}
/*N*/ 	return nHeight;
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::MakeAll()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 10. Apr. 97
|*
|*************************************************************************/
/*N*/ void MA_FASTCALL SwInvalidatePositions( SwFrm *pFrm, long nBottom )
/*N*/ {
    // LONG_MAX == nBottom means we have to calculate all
/*N*/     BOOL bAll = LONG_MAX == nBottom;
/*N*/     SWRECTFN( pFrm )
/*N*/ 	do
/*N*/ 	{	pFrm->_InvalidatePos();
/*N*/ 		pFrm->_InvalidateSize();
/*N*/ 		if( pFrm->IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			if ( ((SwLayoutFrm*)pFrm)->Lower() )
/*N*/ 				::binfilter::SwInvalidatePositions( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFrm->Prepare( PREP_ADJUST_FRM );
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/     } while ( pFrm &&
/*N*/               ( bAll ||
/*N*/               (*fnRect->fnYDiff)( (pFrm->Frm().*fnRect->fnGetTop)(), nBottom ) < 0 ) );
/*N*/ }

/*N*/ BOOL MA_FASTCALL lcl_CalcLowers( SwLayoutFrm *pLay, long nBottom )
/*N*/ {
/*N*/     // LONG_MAX == nBottom means we have to calculate all
/*N*/     BOOL bAll = LONG_MAX == nBottom;
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	SwCntntFrm *pCnt = pLay->ContainsCntnt();
/*N*/     SWRECTFN( pLay )
/*N*/ 	while ( pCnt && pLay->GetUpper()->IsAnLower( pCnt ) )
/*N*/ 	{
/*N*/ 		bRet |= !pCnt->IsValid();
/*N*/ 		pCnt->CalcFlys( FALSE );
/*N*/ 		pCnt->Calc();
/*N*/ 		pCnt->GetUpper()->Calc();
/*N*/         if( ! bAll && (*fnRect->fnYDiff)((pCnt->Frm().*fnRect->fnGetTop)(), nBottom) > 0 )
/*N*/ 			break;
/*N*/ 		pCnt = pCnt->GetNextCntntFrm();
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL MA_FASTCALL lcl_InnerCalcLayout( SwFrm *pFrm, long nBottom )
/*N*/ {
/*N*/     // LONG_MAX == nBottom means we have to calculate all
/*N*/     BOOL bAll = LONG_MAX == nBottom;
/*N*/ 	BOOL bRet = FALSE;
/*N*/     const SwFrm* pOldUp = pFrm->GetUpper();
/*N*/     SWRECTFN( pFrm )
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if( pFrm->IsLayoutFrm() )
/*N*/ 		{
/*N*/ 			bRet |= !pFrm->IsValid();
/*N*/ 			pFrm->Calc();
/*N*/ 			if( ((SwLayoutFrm*)pFrm)->Lower() )
/*N*/ 				bRet |= lcl_InnerCalcLayout( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
/*N*/ 		}
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/     } while( pFrm &&
/*N*/             ( bAll ||
/*N*/               (*fnRect->fnYDiff)((pFrm->Frm().*fnRect->fnGetTop)(), nBottom) < 0 )
/*N*/             && pFrm->GetUpper() == pOldUp );
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void MA_FASTCALL lcl_CalcLayout( SwLayoutFrm *pLay, long nBottom )
/*N*/ {
/*N*/ 	BOOL bCheck = TRUE;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		while( lcl_InnerCalcLayout( pLay, nBottom ) )
/*N*/ 			bCheck = TRUE;
/*N*/ 		if( bCheck )
/*N*/ 		{
/*N*/ 			bCheck = FALSE;
/*N*/ 			if( lcl_CalcLowers( pLay, nBottom ) )
/*N*/ 				continue;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	} while( TRUE );
/*N*/ }

/*N*/ void MA_FASTCALL lcl_FirstTabCalc( SwTabFrm *pTab )
/*N*/ {
/*N*/     SWRECTFN( pTab )
/*N*/ 	if ( !pTab->IsFollow() && !pTab->GetTable()->IsTblComplex() )
/*N*/ 	{
/*N*/ 		SwLayoutFrm *pRow = (SwLayoutFrm*)pTab->Lower();
/*N*/ 		do
/*N*/ 		{
/*N*/ 			SwLayoutFrm *pCell = (SwLayoutFrm*)pRow->Lower();
/*N*/ 			SwFrm *pCnt = pCell->Lower();
/*N*/ 			pCnt->Calc();
/*N*/             const long nCellHeight = (pCell->Frm().*fnRect->fnGetHeight)();
/*N*/             const long nCellY      = (pCell->Frm().*fnRect->fnGetTop)()-1;
/*N*/             const long nCntHeight  = (pCnt->Frm().*fnRect->fnGetHeight)();
/*N*/             const long nCntY       = (pCnt->Frm().*fnRect->fnGetTop)()-1;
/*N*/ 			if ( 0 != (pCell = (SwLayoutFrm*)pCell->GetNext()) )
/*N*/ 				do
/*N*/                 {   (pCell->Frm().*fnRect->fnSetTopAndHeight)
/*N*/                                                         ( nCellY, nCellHeight );
/*N*/                     (pCell->Prt().*fnRect->fnSetHeight)( nCellHeight );
/*N*/ 					pCell->_InvalidateAll();
/*N*/ 
/*N*/ 					pCnt = pCell->Lower();
/*N*/                     (pCnt->Frm().*fnRect->fnSetTopAndHeight)(nCntY, nCntHeight);
/*N*/                     (pCnt->Prt().*fnRect->fnSetHeight)( nCntHeight );
/*N*/ 					pCnt->_InvalidateAll();
/*N*/ 
/*N*/ 					pCell = (SwLayoutFrm*)pCell->GetNext();
/*N*/ 				} while ( pCell );
/*N*/ 
/*N*/             SwTwips nRowTop = (pRow->Frm().*fnRect->fnGetTop)();
/*N*/             SwTwips nUpBot = (pTab->GetUpper()->Frm().*fnRect->fnGetBottom)();
/*N*/             if( (*fnRect->fnYDiff)( nUpBot, nRowTop ) < 0 )
/*N*/                 break;
/*N*/ 			pRow = (SwLayoutFrm*)pRow->GetNext();
/*N*/ 
/*N*/ 		} while ( pRow );
/*N*/ 	}
/*N*/ 	SwFrm *pUp = pTab->GetUpper();
/*N*/     long nBottom = (pUp->*fnRect->fnGetPrtBottom)();
/*N*/ 	if ( pTab->GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/         nBottom += pUp->Grow( LONG_MAX, TRUE );
/*N*/ 	lcl_CalcLowers( (SwLayoutFrm*)pTab->Lower(), nBottom );
/*N*/ }

/*N*/ void MA_FASTCALL lcl_Recalc( SwTabFrm *pTab,
/*N*/ 							 SwLayoutFrm *pFirstRow,
/*N*/ 							 SwLayNotify &rNotify )
/*N*/ {
/*N*/ 	if ( pTab->Lower() )
/*N*/ 	{
/*N*/         SWRECTFN( pTab )
/*N*/         const SwTwips nOldHeight = (pTab->Frm().*fnRect->fnGetHeight)();
/*N*/         const SwTwips nOldWidth  = (pTab->Frm().*fnRect->fnGetWidth)();
/*N*/ 		if ( !pFirstRow )
/*N*/ 		{
/*N*/ 			pFirstRow = (SwLayoutFrm*)pTab->Lower();
/*N*/ 			rNotify.SetLowersComplete( TRUE );
/*N*/ 		}
/*N*/ 		::binfilter::SwInvalidatePositions( pFirstRow, LONG_MAX );
/*N*/ 		::binfilter::lcl_CalcLayout( pFirstRow, LONG_MAX );
/*N*/         SwTwips nNew = (pTab->Frm().*fnRect->fnGetHeight)();
/*N*/         if ( nOldHeight < nNew )
/*N*/             rNotify.AddHeightOfst( nNew - nOldHeight );
/*N*/         else if ( nOldHeight > nNew )
/*?*/             rNotify.SubtractHeightOfst( nOldHeight - nNew );
/*N*/         nNew = (pTab->Frm().*fnRect->fnGetWidth)();
/*N*/         if ( nOldWidth < nNew )
/*N*/             rNotify.AddWidthOfst( nNew - nOldWidth );
/*N*/         else if ( nOldWidth > nNew )
/*?*/             rNotify.SubtractWidthOfst( nOldWidth - nNew );
/*N*/ 	}
/*N*/ }

/*N*/ #define KEEPTAB ( !GetFollow() && !IsFollow() )

/*N*/ void SwTabFrm::MakeAll()
/*N*/ {
/*N*/ 	if ( IsJoinLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
/*N*/ 		return;
/*N*/ 
/*N*/ 	PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )
/*N*/ 
/*N*/     LockJoin(); //Ich lass mich nicht unterwegs vernichten.
/*N*/ 	SwLayNotify aNotify( this );	//uebernimmt im DTor die Benachrichtigung
/*N*/     // If pos is invalid, we have to call a SetInvaKeep at aNotify.
/*N*/     // Otherwise the keep atribute would not work in front of a table.
/*N*/     const BOOL bOldValidPos = GetValidPosFlag();
/*N*/ 
/*N*/ 	//Wenn mein direkter Nachbar gleichzeitig mein Follow ist
/*N*/ 	//verleibe ich mir das Teil ein.
/*N*/     // OD 09.04.2003 #108698# - join all follows, which are placed on the
/*N*/     // same page/column.
/*N*/     // OD 29.04.2003 #109213# - join follow, only if join for the follow
/*N*/     // is not locked. Otherwise, join will not be performed and this loop
/*N*/     // will be endless.
/*N*/     while ( GetNext() && GetNext() == GetFollow() &&
/*N*/             !GetFollow()->IsJoinLocked()
/*N*/           )
/*N*/     {
/*N*/         aNotify.AddHeightOfst( Join() );
/*N*/     }
/*N*/ 
/*N*/ 	if ( bResizeHTMLTable )	//Optimiertes Zusammenspiel mit Grow/Shrink des Inhaltes
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	BOOL bMakePage	= TRUE;		//solange TRUE kann eine neue Seite
/*N*/ 								//angelegt werden (genau einmal)
/*N*/ 	BOOL bMovedBwd	= FALSE;	//Wird TRUE wenn der Frame zurueckfliesst
/*N*/ 	BOOL bMovedFwd	= FALSE;	//solange FALSE kann der Frm zurueck-
/*N*/ 								//fliessen (solange, bis er einmal
/*N*/ 								//vorwaerts ge'moved wurde).
/*N*/ 	BOOL bSplit		= FALSE;	//Wird TRUE wenn der Frm gesplittet wurde.
/*N*/ 	BOOL bFtnsInDoc = 0 != GetFmt()->GetDoc()->GetFtnIdxs().Count();
/*N*/ 	BOOL bMoveable;
/*N*/ 	const BOOL bRepeat	= GetTable()->IsHeadlineRepeat();
/*N*/ 	const BOOL bFly		= IsInFly();
/*N*/ 
/*N*/ 	SwBorderAttrAccess  *pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
/*N*/ 	const SwBorderAttrs *pAttrs = pAccess->Get();
/*N*/ 
/*N*/ 	const BOOL bKeep = IsKeep( *pAttrs );
/*N*/ 	const BOOL bDontSplit = !IsFollow() && !GetFmt()->GetLayoutSplit().GetValue();
/*N*/ 
/*N*/ 	if ( bDontSplit )
/*N*/ 		while ( GetFollow() )
/*N*/ 			aNotify.AddHeightOfst( Join() );
/*N*/ 
/*N*/ 	//Einen Frischling moven wir gleich schon einmal vorwaerts...
/*N*/ 	if ( !Frm().Top() && IsFollow() )
/*N*/ 	{
/*N*/ 		SwFrm *pPre = GetPrev();
/*N*/ 		if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
/*N*/ 		{
/*N*/ 			if ( !MoveFwd( bMakePage, FALSE ) )
/*N*/ 				bMakePage = FALSE;
/*N*/ 			bMovedFwd = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     SWRECTFN( this )
/*N*/ 
/*N*/ 	while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	{
/*N*/ 		if ( TRUE == (bMoveable = IsMoveable()) )
/*N*/ 			if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bMovedBwd ) )
/*N*/ 			{
/*N*/ 				bMovedFwd = TRUE;
/*N*/ 				bCalcLowers = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/         Point aOldPos( (Frm().*fnRect->fnGetPos)() );
/*N*/ 		MakePos();
/*N*/         if ( aOldPos != (Frm().*fnRect->fnGetPos)() )
/*N*/ 		{
/*N*/             if ( aOldPos.Y() != (Frm().*fnRect->fnGetTop)() )
/*N*/ 			{
/*N*/ 				SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
/*N*/ 				if( pLayout )
/*N*/ 				{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 				}
/*N*/ 
/*N*/ 				bValidPrtArea = FALSE;
/*N*/ 				aNotify.SetLowersComplete( FALSE );
/*N*/ 			}
/*N*/ 			SwFrm *pPre;
/*N*/ 			if ( bKeep || (0 != (pPre = FindPrev()) &&
/*N*/ 						   pPre->GetAttrSet()->GetKeep().GetValue()) )
/*N*/ 			{
/*N*/ 				bCalcLowers = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Wir muessen die Hoehe der ersten Zeile kennen, denn nur wenn diese
/*N*/ 		//kleiner wird muss ggf. der Master angestossen werden um noetigenfalls
/*N*/ 		//die Zeile aufzunehmen.
/*N*/ 		long n1StLineHeight = 0;
/*N*/ 		if ( IsFollow() )
/*N*/ 		{
/*N*/ 			SwFrm *pFrm = Lower();
/*N*/ 			if ( bRepeat && pFrm )
/*N*/ 				pFrm = pFrm->GetNext();
/*N*/ 			if ( pFrm )
/*N*/                 n1StLineHeight = (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( !bValidSize || !bValidPrtArea )
/*N*/ 		{
/*N*/             const BOOL bOptLower = (Frm().*fnRect->fnGetHeight)() == 0;
/*N*/ 
/*N*/             const long nOldPrtWidth = (Prt().*fnRect->fnGetWidth)();
/*N*/             const long nOldFrmWidth = (Frm().*fnRect->fnGetWidth)();
/*N*/             const Point aOldPrtPos  = (Prt().*fnRect->fnGetPos)();
/*N*/ 			Format( pAttrs );
/*N*/ 
/*N*/ 			SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
/*N*/ 			if ( /*!bOptLower &&*/ pLayout &&
/*N*/                  ((Prt().*fnRect->fnGetWidth)() != nOldPrtWidth ||
/*N*/                   (Frm().*fnRect->fnGetWidth)() != nOldFrmWidth) )
/*N*/             {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 			}
/*N*/             if ( !bOptLower && aOldPrtPos != (Prt().*fnRect->fnGetPos)() )
/*N*/ 				aNotify.SetLowersComplete( FALSE );
/*N*/ 
/*N*/ 			if ( bOptLower )
/*N*/ 			{
/*N*/ 				//MA 24. May. 95: Optimierungsversuch!
/*N*/ 				//Ganz nigel nagel neu das Teil. Damit wir nicht n-fach
/*N*/ 				//MakeAll'en formatieren wir flugs den Inhalt.
/*N*/ 				//Das erste Format mussten wir allerdings abwarten, damit
/*N*/ 				//die Breiten Stimmen!
/*N*/ 				//MA: Fix, Kein Calc wenn evtl. noch Seitengebunde Flys
/*N*/ 				//an den Cntnt haengen (siehe frmtool.cxx, ~SwCntntNotify).
/*N*/ 				SwDoc *pDoc = GetFmt()->GetDoc();
/*N*/ 				if ( !pDoc->GetSpzFrmFmts()->Count() ||
/*N*/ 					 pDoc->IsLoaded() || pDoc->IsNewDoc() )
/*N*/ 				{
/*N*/ 					//MA 28. Nov. 95: Und wieder ein Trick, gleich mal sehen
/*N*/ 					//ob ein Rueckfluss lohnt.
/*N*/ 					if ( bMoveable && !GetPrev() )
/*N*/ 					{
/*N*/ 						GetLeaf( MAKEPAGE_NONE, FALSE ); //setzt das BackMoveJump
/*N*/ 						if ( SwFlowFrm::IsMoveBwdJump() )
/*N*/ 						{
/*N*/ 							BOOL bDummy;
/*N*/ 							SwFtnBossFrm *pOldBoss = bFtnsInDoc ?
/*N*/ 								FindFtnBossFrm( TRUE ) : 0;
/*N*/ 							const FASTBOOL bOldPrev = GetPrev() != 0;
/*N*/ 							if ( MoveBwd( bDummy ) )
/*N*/ 							{
/*N*/                                 SWREFRESHFN( this )
/*N*/ 								bMovedBwd = TRUE;
/*N*/ 								if ( bFtnsInDoc )
/*N*/ 									MoveLowerFtns( 0, pOldBoss, 0, TRUE );
/*N*/ 
/*N*/                                 long nOldTop = (Frm().*fnRect->fnGetTop)();
/*N*/ 								MakePos();
/*N*/                                 if( nOldTop != (Frm().*fnRect->fnGetTop)() )
/*N*/ 								{
/*N*/ 									SwHTMLTableLayout *pLayout =
/*N*/ 										GetTable()->GetHTMLTableLayout();
/*N*/ 									if( pLayout )
/*N*/ 									{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 									}
/*N*/ 								}
/*N*/ 
/*N*/ 								if ( bOldPrev != (0 != GetPrev()) )
/*N*/ 								{
/*N*/ 									//Abstand nicht vergessen!
/*N*/ 									bValidPrtArea = FALSE;
/*N*/ 									Format( pAttrs );
/*N*/ 								}
/*N*/ 								if ( bKeep && KEEPTAB )
/*N*/ 								{
/*?*/ 									SwFrm *pNxt = FindNextCnt();
/*?*/ 									// FindNextCnt geht ggf. in einen Bereich
/*?*/ 									// hinein, in eine Tabelle allerdings auch
/*?*/ 									if( pNxt && pNxt->IsInTab() )
/*?*/ 										pNxt = pNxt->FindTabFrm();
/*?*/ 									if ( pNxt )
/*?*/ 									{
/*?*/ 										pNxt->Calc();
/*?*/ 										if ( !GetNext() )
/*?*/ 											bValidPos = FALSE;
/*?*/ 									}
/*N*/ 								}
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					::binfilter::lcl_FirstTabCalc( this );
/*N*/ 					bValidSize = bValidPrtArea = FALSE;
/*N*/ 					Format( pAttrs );
/*N*/ 					aNotify.SetLowersComplete( TRUE );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Wenn ich der erste einer Kette bin koennte ich mal sehen ob
/*N*/ 		//ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
/*N*/ 		//Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
/*N*/ 		//geflosssen sein.
/*N*/ 		if ( !GetIndPrev() && !bMovedFwd && (bMoveable || bFly) )
/*N*/ 		{
/*N*/ 			//Bei Follows muss der Master benachrichtigt
/*N*/ 			//werden. Der Follow muss nur dann Moven, wenn er leere Blaetter
/*N*/ 			//ueberspringen muss.
/*N*/ 			if ( IsFollow() )
/*N*/ 			{
/*N*/ 				//Nur wenn die Hoehe der ersten Zeile kleiner geworder ist.
/*N*/ 				SwFrm *pFrm = Lower();
/*N*/ 				if ( bRepeat && pFrm )
/*N*/ 					pFrm = pFrm->GetNext();
/*N*/                 if(pFrm && n1StLineHeight >(pFrm->Frm().*fnRect->fnGetHeight)())
/*N*/ 				{
/*N*/ 					SwTabFrm *pMaster = (SwTabFrm*)FindMaster();
/*N*/ 					BOOL bDummy;
/*N*/ 					if ( ShouldBwdMoved( pMaster->GetUpper(), FALSE, bDummy ) )
/*N*/ 						pMaster->InvalidatePos();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			SwFtnBossFrm *pOldBoss = bFtnsInDoc ? FindFtnBossFrm( TRUE ) : 0;
/*N*/ 			BOOL bReformat;
/*N*/ 			if ( MoveBwd( bReformat ) )
/*N*/ 			{
/*N*/                 SWREFRESHFN( this )
/*N*/ 				bMovedBwd = TRUE;
/*N*/ 				aNotify.SetLowersComplete( FALSE );
/*N*/ 				if ( bFtnsInDoc )
/*N*/ 					MoveLowerFtns( 0, pOldBoss, 0, TRUE );
/*N*/ 				if ( bReformat || bKeep )
/*N*/ 				{
/*N*/                     long nOldTop = (Frm().*fnRect->fnGetTop)();
/*N*/                     MakePos();
/*N*/                     if( nOldTop != (Frm().*fnRect->fnGetTop)() )
/*N*/ 					{
/*N*/ 						SwHTMLTableLayout *pLayout =
/*N*/ 							GetTable()->GetHTMLTableLayout();
/*N*/ 						if( pLayout )
/*N*/ 						{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 						}
/*N*/ 
/*N*/ 						bValidPrtArea = FALSE;
/*N*/ 						Format( pAttrs );
/*N*/ 					}
/*N*/ 					::binfilter::lcl_Recalc( this, 0, aNotify );
/*N*/ 					bLowersFormatted = TRUE;
/*N*/ 					if ( bKeep && KEEPTAB )
/*N*/ 					{
/*N*/ 						SwFrm *pNxt = FindNextCnt();
/*N*/ 						if( pNxt && pNxt->IsInTab() )
/*N*/ 							pNxt = pNxt->FindTabFrm();
/*N*/ 						if ( pNxt )
/*N*/ 						{
/*N*/ 							pNxt->Calc();
/*N*/ 							if ( !GetNext() )
/*N*/ 								bValidPos = FALSE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Wieder ein Wert ungueltig? - dann nochmal das ganze...
/*N*/ 		if ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 			continue;
/*N*/ 
/*N*/         // check, if calculation of table frame is ready.
/*N*/ 
/*N*/         /// OD 23.10.2002 #103517# - Local variable <nDistanceToUpperPrtBottom>
/*N*/         ///     Introduce local variable and init it with the distance from the
/*N*/         ///     table frame bottom to the bottom of the upper printing area.
/*N*/         /// Note: negative values denotes the situation that table frame doesn't
/*N*/         ///     fit in its upper.
/*N*/         SwTwips nDistanceToUpperPrtBottom =
/*N*/                 (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
/*N*/ 
/*N*/         /// OD 23.10.2002 #103517# - In online layout try to grow upper of table
/*N*/         /// frame, if table frame doesn't fit in its upper.
/*N*/         if ( nDistanceToUpperPrtBottom < 0 &&
/*N*/              GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/         {
/*N*/             if ( GetUpper()->Grow( -nDistanceToUpperPrtBottom ) )
/*N*/             {
/*N*/                 // upper is grown --> recalculate <nDistanceToUpperPrtBottom>
/*N*/                 nDistanceToUpperPrtBottom =
/*N*/                     (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         if( nDistanceToUpperPrtBottom >= 0)
/*N*/ 		{
/*N*/             // OD 23.10.2002 - translate german commentary
/*N*/             // If there is space left in the upper printing area, join as for trial
/*N*/             // at least one further row of an existing follow.
/*N*/ 			if ( !bSplit && GetFollow() )
/*N*/ 			{
/*N*/ 				BOOL bDummy;
/*N*/ 				if ( GetFollow()->ShouldBwdMoved( GetUpper(), FALSE, bDummy ) )
/*N*/ 				{
/*N*/ 					SwFrm *pTmp = GetUpper();
/*N*/                     SwTwips nDeadLine = (pTmp->*fnRect->fnGetPrtBottom)();
/*N*/ 					if ( GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/                         nDeadLine += pTmp->Grow( LONG_MAX, TRUE );
/*N*/                     if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
/*N*/ 					{
/*N*/ 						SwFrm *pRow = GetFollow()->Lower();
/*N*/ 						if ( bRepeat )
/*N*/ 							pRow = pRow->GetNext();
/*N*/                         const SwTwips nOld = (Frm().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/ 						const BOOL bMoveFtns = bFtnsInDoc && pRow &&
/*N*/ 											   !GetFollow()->IsJoinLocked();
/*N*/ 
/*N*/ 						SwFtnBossFrm *pOldBoss;
/*N*/ 						if ( bMoveFtns )
/*N*/ 							pOldBoss = pRow->FindFtnBossFrm( TRUE );
/*N*/ 
/*N*/ 						//fix(8680): Row kann 0 werden.
/*N*/ 						if ( !pRow || !pRow->GetNext() )
/*N*/ 							//Der Follow wird leer und damit ueberfluessig.
/*N*/ 							aNotify.AddHeightOfst( Join() );
/*N*/ 						else
/*N*/ 						{
/*N*/ 							pRow->Cut();
/*N*/ 							pRow->Paste( this );
/*N*/                             aNotify.AddHeightOfst(
/*N*/                                         (pRow->Frm().*fnRect->fnGetHeight)() );
/*N*/ 						}
/*N*/ 						//Die Fussnoten verschieben!
/*N*/ 						if ( pRow && bMoveFtns )
/*N*/ 							if ( ((SwLayoutFrm*)pRow)->MoveLowerFtns(
/*N*/ 								 0, pOldBoss, FindFtnBossFrm( TRUE ), TRUE ) )
/*N*/ 								GetUpper()->Calc();
/*N*/ 
/*N*/                         if ( pRow && nOld != (Frm().*fnRect->fnGetHeight)() )
/*N*/ 							::binfilter::lcl_Recalc( this, (SwLayoutFrm*)pRow, aNotify );
/*N*/ 						continue;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( bKeep && KEEPTAB )
/*N*/ 			{
/*N*/ 				SwFrm *pNxt = FindNextCnt();
/*N*/ 				if( pNxt && pNxt->IsInTab() )
/*N*/ 					pNxt = pNxt->FindTabFrm();
/*N*/ 				if ( pNxt )
/*N*/ 					pNxt->Calc();
/*N*/ 			}
/*N*/ 			if ( IsValid() )
/*N*/ 			{
/*N*/ 				if ( bCalcLowers )
/*N*/ 				{
/*N*/ 					::binfilter::lcl_Recalc( this, 0, aNotify );
/*N*/ 					bLowersFormatted = TRUE;
/*N*/ 					bCalcLowers = FALSE;
/*N*/ 				}
/*N*/ 				else if ( bONECalcLowers )
/*N*/ 				{
/*N*/ 					lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
/*N*/ 					bONECalcLowers = FALSE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 
/*N*/ 		//Ich passe nicht mehr in meinen Uebergeordneten, also ist es jetzt
/*N*/ 		//an der Zeit moeglichst konstruktive Veranderungen vorzunehmen
/*N*/ 
/*N*/ 		//Wenn ich den uebergeordneten Frm nicht verlassen darf, habe
/*N*/ 		//ich ein Problem; Frei nach Artur Dent tun wir das einzige das man
/*N*/ 		//mit einen nicht loesbaren Problem tun kann: wir ignorieren es - und
/*N*/ 		//zwar mit aller Kraft.
/*N*/ 		if ( !bMoveable )
/*N*/ 		{
/*N*/ 			if ( bCalcLowers && IsValid() )
/*N*/ 			{
/*N*/ 				lcl_Recalc( this, 0, aNotify );
/*N*/ 				bLowersFormatted = TRUE;
/*N*/ 				bCalcLowers = FALSE;
/*N*/ 			}
/*N*/ 			else if ( bONECalcLowers )
/*N*/ 			{
/*N*/ 				lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
/*N*/ 				bONECalcLowers = FALSE;
/*N*/ 			}
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 
/*N*/         if ( bCalcLowers && IsValid() )
/*N*/ 		{
/*N*/ 			::binfilter::lcl_Recalc( this, 0, aNotify );
/*N*/ 			bLowersFormatted = TRUE;
/*N*/ 			bCalcLowers = FALSE;
/*N*/             if( !IsValid() )
/*N*/                 continue;
/*N*/ 		}
/*N*/ 
/*N*/ 		//Der erste Versuch muss natuerlich das Aufspalten der Tabelle sein.
/*N*/ 		//Das funktioniert natuerlich nur dann, wenn die Tabelle mehr als eine
/*N*/ 		//Zeile enthaelt und wenn die Unterkante des Upper unter der ersten
/*N*/ 		//Zeile liegt.
/*N*/ 		SwFrm *pIndPrev = GetIndPrev();
/*N*/ 		if ( Lower()->GetNext() && (!bDontSplit || !pIndPrev) )
/*N*/ 		{
/*N*/ 			//Damit der Schatten nicht extra herausgerechnet werden muss,
/*N*/ 			//lassen wir das Spiel gleich wenn es ein HeadlineRepeat gibt und
/*N*/ 			//nur noch eine nicht Headline Zeile vorhanden ist.
/*N*/ 			if ( !bRepeat || Lower()->GetNext()->GetNext() )
/*N*/ 			{
/*N*/                 SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/                 if( IsInSct() )
/*N*/                     nDeadLine = (*fnRect->fnYInc)( nDeadLine,
/*N*/                                         GetUpper()->Grow( LONG_MAX, TRUE ) );
/*N*/ 				::binfilter::lcl_CalcLayout( (SwLayoutFrm*)Lower(), nDeadLine );
/*N*/ 				bLowersFormatted = TRUE;
/*N*/ 				aNotify.SetLowersComplete( TRUE );
/*N*/                 if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
/*N*/ 					continue;
/*N*/ 
/*N*/                 SwTwips nBreakLine = (Frm().*fnRect->fnGetTop)();
/*N*/                 nBreakLine = (*fnRect->fnYInc)( nBreakLine,
/*N*/                         (this->*fnRect->fnGetTopMargin)() +
/*N*/                         (Lower()->Frm().*fnRect->fnGetHeight)() +
/*N*/                         ( bRepeat ?
/*N*/                           (Lower()->GetNext()->Frm().*fnRect->fnGetHeight)()
/*N*/                           : 0 ) );
/*N*/                 if( (*fnRect->fnYDiff)(nDeadLine, nBreakLine) >=0 || !pIndPrev )
/*N*/ 				{
/*N*/ 					aNotify.SubtractHeightOfst( Split( nDeadLine ) );
/*N*/ 					if ( aNotify.GetHeightOfst() < 0 )
/*N*/ 						aNotify.ResetHeightOfst();
/*N*/ 					aNotify.SetLowersComplete( FALSE );
/*N*/ 					bSplit = TRUE;
/*N*/ 					//Damit es nicht zu Oszillationen kommt, muss der
/*N*/ 					//Follow gleich gueltig gemacht werden.
/*N*/ 					if ( GetFollow() )
/*N*/ 					{
/*N*/                         SWRECTFN( GetFollow() )
/*N*/ 
/*N*/                         static BYTE nStack = 0;
/*N*/                         if ( !StackHack::IsLocked() && nStack < 4 )
/*N*/ 						{
/*N*/                             ++nStack;
/*N*/ 							StackHack aHack;
/*N*/                             delete pAccess;
/*N*/ 							GetFollow()->MakeAll();
/*N*/                             pAccess= new SwBorderAttrAccess( SwFrm::GetCache(),
/*N*/                                                              this );
/*N*/                             pAttrs = pAccess->Get();
/*N*/ 							((SwTabFrm*)GetFollow())->SetLowersFormatted(FALSE);
/*N*/ 							::binfilter::lcl_CalcLayout((SwLayoutFrm*)GetFollow()->Lower(),
/*N*/ 								(GetFollow()->GetUpper()->Frm().*fnRect->fnGetBottom)() );
/*N*/ 							if ( !GetFollow()->GetFollow() )
/*N*/ 							{
/*N*/ 								SwFrm *pNxt = ((SwFrm*)GetFollow())->FindNext();
/*N*/ 								if ( pNxt )
/*N*/ 									pNxt->Calc();
/*N*/ 							}
/*N*/                             --nStack;
/*N*/ 						}
/*N*/ 						else if ( GetFollow() == GetNext() )
/*N*/ 							((SwTabFrm*)GetFollow())->MoveFwd( TRUE, FALSE );
/*N*/ 						ViewShell *pSh;
/*N*/ 						if ( 0 != (pSh = GetShell()) )
/*N*/ 							pSh->Imp()->ResetScroll();
/*N*/ 					}
/*N*/ 					continue;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( IsInSct() && bMovedFwd && bMakePage && GetUpper()->IsColBodyFrm() &&
/*N*/ 			GetUpper()->GetUpper()->GetUpper()->IsSctFrm() &&
/*N*/ 			( GetUpper()->GetUpper()->GetPrev() || GetIndPrev() ) &&
/*N*/ 			((SwSectionFrm*)GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
/*N*/ 			bMovedFwd = FALSE;
/*N*/ 
/*N*/ 		//Mal sehen ob ich irgenwo Platz finde...
/*N*/ 		if ( !bMovedFwd && !MoveFwd( bMakePage, FALSE ) )
/*N*/ 			bMakePage = FALSE;
/*N*/         SWREFRESHFN( this )
/*N*/ 		bMovedFwd = bCalcLowers = TRUE;
/*N*/ 		aNotify.SetLowersComplete( FALSE );
/*N*/ 		if ( IsFollow() )
/*N*/ 		{	//Um Oszillationen zu vermeiden sollte kein ungueltiger Master
/*N*/ 			//zurueckbleiben.
/*N*/ 			SwTabFrm *pTab = FindMaster();
/*N*/ 			if ( pTab->GetUpper() )
/*N*/ 				pTab->GetUpper()->Calc();
/*N*/ 			pTab->Calc();
/*N*/ 			pTab->SetLowersFormatted( FALSE );
/*N*/ 		}
/*N*/ 
/*N*/ 		//Wenn mein direkter Nachbar jetzt gleichzeitig mein Follow ist
/*N*/ 		//verleibe ich mir das Teil ein.
/*N*/ 		if ( GetNext() && GetNext() == GetFollow() )
/*N*/ 			aNotify.AddHeightOfst( Join() );
/*N*/ 
/*N*/ 		if ( bMovedBwd && GetUpper() )
/*N*/ 			//Beim zurueckfliessen wurde der Upper angeregt sich vollstaendig
/*N*/ 			//zu Painten, dass koennen wir uns jetzt nach dem hin und her
/*N*/ 			//fliessen sparen.
/*N*/ 			GetUpper()->ResetCompletePaint();
/*N*/ 
/*N*/         if ( bCalcLowers && IsValid() )
/*N*/ 		{
/*N*/ 			::binfilter::lcl_Recalc( this, 0, aNotify );
/*N*/ 			bLowersFormatted = TRUE;
/*N*/ 			bCalcLowers = FALSE;
/*N*/ 		}
/*N*/ 
/*N*/ 	} //while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 
/*N*/ 	//Wenn mein direkter Vorgaenger jetzt mein Master ist, so kann er mich
/*N*/ 	//bei der nachstbesten Gelegenheit vernichten.
/*N*/ 	if ( IsFollow() )
/*N*/ 	{
/*N*/ 		SwFrm *pPre = GetPrev();
/*N*/ 		if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
/*N*/ 			pPre->InvalidatePos();
/*N*/ 	}
/*N*/ 
/*N*/ 	bCalcLowers = bONECalcLowers = FALSE;
/*N*/ 	delete pAccess;
/*N*/ 	UnlockJoin();
/*N*/     if ( bMovedFwd || bMovedBwd || ! bOldValidPos )
/*N*/ 		aNotify.SetInvaKeep();
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::CalcFlyOffsets()
|*
|*	Beschreibung:		Berechnet die Offsets, die durch FlyFrames
|*						entstehen.
|*	Ersterstellung		MA/MIB 14. Apr. 99
|*	Letzte Aenderung
|*
|*************************************************************************/
/*N*/ BOOL SwTabFrm::CalcFlyOffsets( SwTwips& rUpper,
/*N*/ 							   long& rLeftOffset,
/*N*/ 							   long& rRightOffset ) const
/*N*/ {
/*N*/ 	BOOL bInvalidatePrtArea = FALSE;
/*N*/ 	const SwPageFrm *pPage = FindPageFrm();
/*N*/ 	const SwFlyFrm* pMyFly = FindFlyFrm();
/*N*/ 	if ( pPage->GetSortedObjs() )
/*N*/ 	{
/*N*/         SWRECTFN( this )
/*N*/         long nPrtPos = (Frm().*fnRect->fnGetTop)();
/*N*/         nPrtPos = (*fnRect->fnYInc)( nPrtPos, rUpper );
/*N*/ 		SwRect aRect( Frm() );
/*N*/         long nYDiff = (*fnRect->fnYDiff)( (Prt().*fnRect->fnGetTop)(), rUpper );
/*N*/         if( nYDiff > 0 )
/*N*/             (aRect.*fnRect->fnAddBottom)( -nYDiff );
/*N*/ 		for ( USHORT i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
/*N*/ 		{
/*N*/ 			SdrObject *pObj = (*pPage->GetSortedObjs())[i];
/*N*/ 			if ( pObj->IsWriterFlyFrame() )
/*N*/ 			{
/*N*/ 				SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 				const SwRect aFlyRect = pFly->AddSpacesToFrm();
/*N*/                 if ( WEIT_WECH != (pFly->Frm().*fnRect->fnGetTop)() &&
/*N*/ 					 pFly->IsFlyAtCntFrm() && aFlyRect.IsOver( aRect ) &&
                     // OD 25.02.2003 #i9040# - use '<=' instead of '<'
/*N*/ 					 (*fnRect->fnYDiff)(
/*N*/                             (pFly->GetAnchor()->Frm().*fnRect->fnGetBottom)(),
/*N*/                             (Frm().*fnRect->fnGetTop)() ) <= 0 &&
/*N*/ 					 !IsAnLower( pFly ) && !pFly->IsAnLower( this ) &&
/*N*/ 					 ( !pMyFly || pMyFly->IsAnLower( pFly ) ) &&
/*N*/ 					 pPage->GetPhyPageNum() >=
/*N*/                      pFly->GetAnchor()->FindPageFrm()->GetPhyPageNum() &&
/*N*/                      // anchor should be in same page body/header/footer
/*N*/                      ( pFly->GetAnchor()->FindFooterOrHeader() ==
/*N*/                        FindFooterOrHeader() ) )
/*N*/ 				{
/*N*/ 					const SwFmtSurround   &rSur = pFly->GetFmt()->GetSurround();
/*N*/ 					const SwFmtHoriOrient &rHori= pFly->GetFmt()->GetHoriOrient();
/*N*/ 					if ( SURROUND_NONE == rSur.GetSurround() )
/*N*/ 					{
/*?*/                         long nBottom = (aFlyRect.*fnRect->fnGetBottom)();
/*?*/                         if( (*fnRect->fnYDiff)( nPrtPos, nBottom ) < 0 )
/*?*/                             nPrtPos = nBottom;
/*?*/ 						bInvalidatePrtArea = TRUE;
/*N*/ 					}
/*N*/ 					if ( (SURROUND_RIGHT	== rSur.GetSurround() ||
/*N*/ 						  SURROUND_PARALLEL == rSur.GetSurround())&&
/*N*/ 						 HORI_LEFT == rHori.GetHoriOrient() )
/*N*/ 					{
/*N*/                         const long nWidth = (*fnRect->fnXDiff)(
/*N*/                             (aFlyRect.*fnRect->fnGetRight)(),
/*?*/                             (pFly->GetAnchor()->Frm().*fnRect->fnGetLeft)() );
/*?*/ 						rLeftOffset = Max( rLeftOffset, nWidth );
/*?*/ 						bInvalidatePrtArea = TRUE;
/*N*/ 					}
/*N*/ 					if ( (SURROUND_LEFT		== rSur.GetSurround() ||
/*N*/ 						  SURROUND_PARALLEL == rSur.GetSurround())&&
/*N*/ 						 HORI_RIGHT == rHori.GetHoriOrient() )
/*N*/ 					{
/*?*/                         const long nWidth = (*fnRect->fnXDiff)(
/*?*/                             (pFly->GetAnchor()->Frm().*fnRect->fnGetRight)(),
/*?*/                             (aFlyRect.*fnRect->fnGetLeft)() );
/*?*/ 						rRightOffset = Max( rRightOffset, nWidth );
/*?*/ 						bInvalidatePrtArea = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/         rUpper = (*fnRect->fnYDiff)( nPrtPos, (Frm().*fnRect->fnGetTop)() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return bInvalidatePrtArea;
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::Format()
|*
|*	Beschreibung:		"Formatiert" den Frame; Frm und PrtArea
|*						Die Fixsize wird hier nicht eingestellt.
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 18. Jun. 97
|*
|*************************************************************************/
/*M*/ void SwTabFrm::Format( const SwBorderAttrs *pAttrs )
/*M*/ {
/*M*/ 	ASSERT( pAttrs, "TabFrm::Format, pAttrs ist 0." );
/*M*/ 
/*M*/     SWRECTFN( this )
/*M*/ 	if ( !bValidSize )
/*M*/     {
/*M*/         long nDiff = (GetUpper()->Prt().*fnRect->fnGetWidth)() -
/*M*/                      (Frm().*fnRect->fnGetWidth)();
/*M*/         if( nDiff )
/*M*/             (aFrm.*fnRect->fnAddRight)( nDiff );
/*M*/     }
/*M*/ 
/*M*/ 	//VarSize ist immer die Hoehe.
/*M*/ 	//Fuer den oberen/unteren Rand gelten die selben Regeln wie fuer
/*M*/ 	//cntfrms (sie MakePrtArea() von diesen).
/*M*/ 
/*M*/ 	SwTwips nUpper = CalcUpperSpace( pAttrs );
/*M*/ 
/*M*/ 	//Wir wollen Rahmen ausweichen. Zwei Moeglichkeiten:
/*M*/ 	//1. Es gibt Rahmen mit SurroundNone, diesen wird vollsaendig ausgewichen
/*M*/ 	//2. Es gibt Rahmen mit Umlauf nur rechts bzw. nur links und diese sind
/*M*/ 	//   rechts bzw. links ausgerichtet, diese geben ein Minimum fuer die
/*M*/ 	//	 Raender vor.
/*M*/ 	long nTmpRight = -1000000,
/*M*/ 		 nLeftOffset  = 0;
/*M*/ 	if( CalcFlyOffsets( nUpper, nLeftOffset, nTmpRight ) )
/*M*/ 		bValidPrtArea = FALSE;
/*M*/ 	long nRightOffset = Max( 0L, nTmpRight );
/*M*/ 
/*M*/ 	SwTwips nLower = pAttrs->CalcBottomLine();
/*M*/ 
/*M*/ 	if ( !bValidPrtArea )
/*M*/ 	{	bValidPrtArea = TRUE;
/*M*/ 
/*M*/ 		//Die Breite der PrtArea wird vom FrmFmt vorgegeben, die Raender
/*M*/ 		//sind entsprechend einzustellen.
/*M*/ 		//Mindestraender werden von Umrandung und Schatten vorgegeben.
/*M*/ 		//Die Rander werden so eingestellt, dass die PrtArea nach dem
/*M*/ 		//angegebenen Adjustment im Frm ausgerichtet wird.
/*M*/ 		//Wenn das Adjustment 0 ist, so werden die Rander anhand des
/*M*/ 		//Randattributes eingestellt.
/*N*/ 
/*N*/         const SwTwips nOldHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/         const SwTwips nMax = (aFrm.*fnRect->fnGetWidth)();
/*N*/ 
/*N*/         // OD 14.03.2003 #i9040# - adjust variable names.
/*N*/         const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
/*N*/         const SwTwips nRightLine = pAttrs->CalcRightLine();
/*N*/ 
/*M*/ 		//Die Breite ist evtl. eine Prozentangabe. Wenn die Tabelle irgendwo
/*M*/ 		//'drinsteckt bezieht sie sich auf die Umgebung. Ist es der Body, so
/*M*/ 		//bezieht sie sich in der BrowseView auf die Bildschirmbreite.
/*M*/ 		const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
/*N*/         // OD 14.03.2003 #i9040# - adjust variable name.
/*N*/         const SwTwips nWishedTableWidth = CalcRel( rSz, TRUE );
/*M*/ 
/*M*/ 		BOOL bCheckBrowseWidth = FALSE;
/*M*/ 
/*N*/         // OD 14.03.2003 #i9040# - insert new variables for left/right spacing.
/*N*/         SwTwips nLeftSpacing  = 0;
/*N*/         SwTwips nRightSpacing = 0;
/*N*/         switch ( GetFmt()->GetHoriOrient().GetHoriOrient() )
/*N*/ 		{
/*N*/ 			case HORI_LEFT:
/*N*/ 				{
/*N*/                     // left indent:
/*N*/                     nLeftSpacing = nLeftLine + nLeftOffset;
/*N*/                     // OD 06.03.2003 #i9040# - correct calculation of right indent:
/*N*/                     // - Consider right indent given by right line attributes.
/*N*/                     // - Consider negative right indent.
/*N*/                     // wished right indent determined by wished table width and
/*N*/                     // left offset given by surround fly frames on the left:
/*N*/                     const SwTwips nWishRight = nMax - nWishedTableWidth - nLeftOffset;
/*N*/                     if ( nRightOffset > 0 )
/*N*/                     {
/*N*/                         // surrounding fly frames on the right
/*N*/                         // -> right indent is maximun of given right offset
/*N*/                         //    and wished right offset.
/*N*/                         nRightSpacing = nRightLine + Max( nRightOffset, nWishRight );
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         // no surrounding fly frames on the right
/*N*/                         // If intrinsic right indent (intrinsic means not considering
/*N*/                         // determined left indent) is negative,
/*N*/                         //      then hold this intrinsic indent,
/*N*/                         //      otherwise non negative wished right indent is hold.
/*N*/                         nRightSpacing = nRightLine +
/*N*/                                         ( ( (nWishRight+nLeftOffset) < 0 ) ?
/*N*/                                             (nWishRight+nLeftOffset) :
/*N*/                                             Max( 0L, nWishRight ) );
/*N*/                     }
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case HORI_RIGHT:
/*N*/ 				{
/*N*/                     // right indent:
/*N*/                     nRightSpacing = nRightLine + nRightOffset;
/*N*/                     // OD 06.03.2003 #i9040# - correct calculation of left indent:
/*N*/                     // - Consider left indent given by left line attributes.
/*N*/                     // - Consider negative left indent.
/*N*/                     // wished left indent determined by wished table width and
/*N*/                     // right offset given by surrounding fyl frames on the right:
/*N*/                     const SwTwips nWishLeft = nMax - nWishedTableWidth - nRightOffset;
/*N*/                     if ( nLeftOffset > 0 )
/*N*/                     {
/*N*/                         // surrounding fly frames on the left
/*N*/                         // -> right indent is maximun of given left offset
/*N*/                         //    and wished left offset.
/*N*/                         nLeftSpacing = nLeftLine + Max( nLeftOffset, nWishLeft );
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         // no surrounding fly frames on the left
/*N*/                         // If intrinsic left indent (intrinsic = not considering
/*N*/                         // determined right indent) is negative,
/*N*/                         //      then hold this intrinsic indent,
/*N*/                         //      otherwise non negative wished left indent is hold.
/*N*/                         nLeftSpacing = nLeftLine +
/*N*/                                        ( ( (nWishLeft+nRightOffset) < 0 ) ?
/*N*/                                            (nWishLeft+nRightOffset) :
/*N*/                                            Max( 0L, nWishLeft ) );
/*N*/                     }
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case HORI_CENTER:
/*N*/ 				{
/*N*/                     // OD 07.03.2003 #i9040# - consider left/right line attribute.
/*N*/                     // OD 10.03.2003 #i9040# -
/*N*/                     const SwTwips nCenterSpacing = ( nMax - nWishedTableWidth ) / 2;
/*N*/                     nLeftSpacing = nLeftLine +
/*N*/                                    ( (nLeftOffset > 0) ?
/*N*/                                      Max( nCenterSpacing, nLeftOffset ) :
/*N*/                                      nCenterSpacing );
/*N*/                     nRightSpacing = nRightLine +
/*N*/                                     ( (nRightOffset > 0) ?
/*N*/                                       Max( nCenterSpacing, nRightOffset ) :
/*N*/                                       nCenterSpacing );
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case HORI_FULL:
/*N*/ 					//Das Teil dehnt sich ueber die gesamte Breite aus.
/*N*/ 					//Nur die fuer die Umrandung benoetigten Freiraeume
/*N*/ 					//werden beruecksichtigt.
/*N*/ 					//Die Attributwerte von LRSpace werden bewusst missachtet!
/*N*/ 					bCheckBrowseWidth = TRUE;
/*N*/                     nLeftSpacing  = nLeftLine + nLeftOffset;
/*N*/                     nRightSpacing = nRightLine + nRightOffset;
/*N*/ 				break;
/*N*/ 			case HORI_NONE:
/*N*/ 				{
/*N*/ 					//Die Raender werden vom Randattribut bestimmt.
/*N*/                     nLeftSpacing = pAttrs->CalcLeft( this );
/*N*/ 					if( nLeftOffset )
/*N*/ 					{
/*N*/                         // OD 07.03.2003 #i9040# - surround fly frames only, if
/*N*/                         // they overlap with the table.
/*N*/                         // Thus, take maximun of left spacing and left offset.
/*N*/                         // OD 10.03.2003 #i9040# - consider left line attribute.
/*N*/                         nLeftSpacing = Max( nLeftSpacing, ( nLeftOffset + nLeftLine ) );
/*N*/ 					}
/*N*/                     // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
/*N*/                     nRightSpacing = pAttrs->CalcRight( this );
/*N*/ 					if( nRightOffset )
/*N*/ 					{
/*N*/                         // OD 07.03.2003 #i9040# - surround fly frames only, if
/*N*/                         // they overlap with the table.
/*N*/                         // Thus, take maximun of right spacing and right offset.
/*N*/                         // OD 10.03.2003 #i9040# - consider right line attribute.
/*N*/                         nRightSpacing = Max( nRightSpacing, ( nRightOffset + nRightLine ) );
/*N*/ 					}
/*N*/                     // OD 10.03.2003 #i9040# - do not hold wished table width.
                    /*
                    if ( !pAttrs->GetLRSpace().GetRight() )
                        nRight = Max( nRight, nMax - (nWish + nLeft + nRight));
                   */
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			case HORI_LEFT_AND_WIDTH:
/*N*/                 {
/*N*/ 					//Linker Rand und die Breite zaehlen (Word-Spezialitaet)
/*N*/                     // OD 10.03.2003 #i9040# - no width alignment in online mode.
/*N*/                     //bCheckBrowseWidth = TRUE;
/*N*/                     nLeftSpacing = pAttrs->CalcLeft( this );
/*N*/ 					if( nLeftOffset )
/*N*/ 					{
/*N*/                         // OD 10.03.2003 #i9040# - surround fly frames only, if
/*N*/                         // they overlap with the table.
/*N*/                         // Thus, take maximun of right spacing and right offset.
/*N*/                         // OD 10.03.2003 #i9040# - consider left line attribute.
/*N*/                         nLeftSpacing = Max( nLeftSpacing, ( pAttrs->CalcLeftLine() + nLeftOffset ) );
/*N*/ 					}
/*N*/                     // OD 10.03.2003 #i9040# - consider right and left line attribute.
/*N*/                     const SwTwips nWishRight =
/*N*/                             nMax - (nLeftSpacing-pAttrs->CalcLeftLine()) - nWishedTableWidth;
/*N*/                     nRightSpacing = nRightLine +
/*N*/                                     ( (nRightOffset > 0) ?
/*N*/                                       Max( nWishRight, nRightOffset ) :
/*N*/                                       nWishRight );
/*N*/                 }
/*N*/ 				break;
/*N*/ 			default:
/*N*/ 				ASSERT( FALSE, "Ungueltige orientation fuer Table." );
/*N*/ 		}
/*N*/         (this->*fnRect->fnSetYMargins)( nUpper, nLower );
/*N*/         if( (nMax - MINLAY) < (nLeftSpacing + nRightSpacing) )
/*N*/             (this->*fnRect->fnSetXMargins)( 0, 0 );
/*N*/ 		else
/*N*/             (this->*fnRect->fnSetXMargins)( nLeftSpacing, nRightSpacing );
/*N*/ 
/*N*/         ViewShell *pSh;
/*N*/ 		if ( bCheckBrowseWidth && GetFmt()->GetDoc()->IsBrowseMode() &&
/*N*/ 			 GetUpper()->IsPageBodyFrm() &&  // nur PageBodyFrms, nicht etwa ColBodyFrms
/*N*/ 			 0 != (pSh = GetShell()) && pSh->VisArea().Width() )
/*N*/ 		{
/*N*/ 			//Nicht ueber die Kante des sichbaren Bereiches hinausragen.
/*N*/ 			//Die Seite kann breiter sein, weil es Objekte mit "ueberbreite"
/*N*/ 			//geben kann (RootFrm::ImplCalcBrowseWidth())
/*N*/ 			const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
/*N*/ 			long nWidth = pSh->VisArea().Width() - 2 * aBorder.Width();
/*N*/ 			nWidth -= Prt().Left();
/*N*/ 			nWidth -= pAttrs->CalcRightLine();
/*N*/ 			Prt().Width( Min( nWidth, Prt().Width() ) );
/*N*/ 		}
/*N*/ 
/*N*/         if ( nOldHeight != (Prt().*fnRect->fnGetHeight)() )
/*N*/ 			bValidSize = FALSE;
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bValidSize )
/*N*/ 	{
/*N*/ 		bValidSize = TRUE;
/*N*/ 
/*N*/ 		//Die Groesse wird durch den Inhalt plus den Raendern bestimmt.
/*N*/ 		SwTwips nRemaining = 0, nDiff;
/*N*/ 		SwFrm *pFrm = pLower;
/*N*/ 		while ( pFrm )
/*N*/         {
/*N*/             nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/             pFrm = pFrm->GetNext();
/*N*/ 		}
/*N*/ 		//Jetzt noch die Raender addieren
/*N*/ 		nRemaining += nUpper + nLower;
/*N*/ 
/*N*/         nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
/*N*/         if ( nDiff > 0 )
/*N*/             Shrink( nDiff PHEIGHT );
/*N*/ 		else if ( nDiff < 0 )
/*N*/             Grow( -nDiff PHEIGHT );
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|*	SwTabFrm::GrowFrm()
|*
|*	Ersterstellung		MA 12. Mar. 93
|*	Letzte Aenderung	MA 23. Sep. 96
|*
|*************************************************************************/
/*N*/ SwTwips SwTabFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/     SwTwips nHeight =(Frm().*fnRect->fnGetHeight)();
/*N*/     if( nHeight > 0 && nDist > ( LONG_MAX - nHeight ) )
/*N*/         nDist = LONG_MAX - nHeight;
/*N*/ 
/*N*/ 	//Tabelle waechst immer (sie kann ja ggf. aufgespalten werden).
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/ 		if ( GetUpper() )
/*N*/ 		{
/*N*/ 			//Der Upper wird nur soweit wie notwendig gegrowed. In nReal wird erstmal
/*N*/ 			//die bereits zur Verfuegung stehende Strecke bereitgestellt.
/*N*/             SwTwips nReal = (GetUpper()->Prt().*fnRect->fnGetHeight)();
/*N*/ 			SwFrm *pFrm = GetUpper()->Lower();
/*N*/ 			while ( pFrm )
/*N*/             {   nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 				pFrm = pFrm->GetNext();
/*N*/ 			}
/*N*/ 
/*N*/ 			SwRect aOldFrm( Frm() );
/*N*/             nHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/             (Frm().*fnRect->fnSetHeight)( nHeight + nDist );
/*N*/             if( IsVertical() && !IsReverse() )
/*N*/                 Frm().Pos().X() -= nDist;
/*N*/ 			if ( nReal < nDist )
/*N*/                 GetUpper()->Grow( nDist - (nReal>0 ? nReal : 0), bTst, bInfo );
/*N*/ 
/*N*/ 			SwRootFrm *pRootFrm = FindRootFrm();
/*N*/ 			if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 				pRootFrm->GetCurrShell() )
/*N*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
/*N*/ 			}
/*N*/         }
/*N*/ 		else
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/ 		if ( GetNext() )
/*N*/ 		{
/*N*/ 			GetNext()->_InvalidatePos();
/*N*/ 			if ( GetNext()->IsCntntFrm() )
/*N*/ 				GetNext()->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		_InvalidateAll();
/*N*/ 		InvalidatePage( pPage );
/*N*/ 		SetComplete();
/*N*/ 
/*N*/ 		const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
/*N*/ 		if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
/*N*/ 			SetCompletePaint();
/*N*/ 	}
/*N*/ 	return nDist;
/*N*/ }
/*************************************************************************
|*
|*	  SwTabFrm::Modify()
|*
|*	  Ersterstellung	MA 14. Mar. 93
|*	  Letzte Aenderung	MA 06. Dec. 96
|*
|*************************************************************************/
/*N*/ void SwTabFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
/*N*/ {
/*N*/ 	BYTE nInvFlags = 0;
/*N*/ 	BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
/*N*/ 
/*N*/ 	if( bAttrSetChg )
/*N*/ 	{
/*N*/ 		SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
/*N*/ 		SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
/*N*/ 		SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
/*N*/ 		SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
/*N*/ 		while( TRUE )
/*N*/ 		{
/*N*/ 			_UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
/*N*/ 						 (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
/*N*/ 						 &aOldSet, &aNewSet );
/*N*/ 			if( aNIter.IsAtEnd() )
/*N*/ 				break;
/*N*/ 			aNIter.NextItem();
/*N*/ 			aOIter.NextItem();
/*N*/ 		}
/*N*/ 		if ( aOldSet.Count() || aNewSet.Count() )
/*N*/ 			SwLayoutFrm::Modify( &aOldSet, &aNewSet );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		_UpdateAttr( pOld, pNew, nInvFlags );
/*N*/ 
/*N*/ 	if ( nInvFlags != 0 )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/ 		InvalidatePage( pPage );
/*N*/ //		if ( nInvFlags & 0x01 )
/*N*/ //			SetCompletePaint();
/*N*/ 		if ( nInvFlags & 0x02 )
/*N*/ 			_InvalidatePrt();
/*N*/ 		if ( nInvFlags & 0x40 )
/*N*/ 			_InvalidatePos();
/*N*/ 		SwFrm *pTmp;
/*N*/ 		if ( 0 != (pTmp = GetIndNext()) )
/*N*/ 		{
/*N*/ 			if ( nInvFlags & 0x04 )
/*N*/ 			{
/*N*/ 				pTmp->_InvalidatePrt();
/*N*/ 				if ( pTmp->IsCntntFrm() )
/*N*/ 					pTmp->InvalidatePage( pPage );
/*N*/ 			}
/*N*/ 			if ( nInvFlags & 0x10 )
/*N*/ 				pTmp->SetCompletePaint();
/*N*/ 		}
/*N*/ 		if ( nInvFlags & 0x08 && 0 != (pTmp = GetPrev()) )
/*N*/ 		{
/*N*/ 			pTmp->_InvalidatePrt();
/*N*/ 			if ( pTmp->IsCntntFrm() )
/*N*/ 				pTmp->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		if ( nInvFlags & 0x20  )
/*N*/ 		{
/*N*/ 			if ( pPage && pPage->GetUpper() && !IsFollow() )
/*N*/ 				((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
/*N*/ 		}
/*N*/ 		if ( nInvFlags & 0x80 )
/*N*/ 			InvalidateNextPos();
/*N*/ 	}
/*N*/ }

/*N*/ void SwTabFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
/*N*/ 							BYTE &rInvFlags,
/*N*/ 							SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
/*N*/ {
/*N*/ 	BOOL bClear = TRUE;
/*N*/ 	const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 		case RES_TBLHEADLINECHG:
/*N*/ 			//Es wird getoggelt.
/*?*/ 			if ( IsFollow() )
/*?*/ 			{
/*?*/ 				if ( GetTable()->IsHeadlineRepeat() )
/*?*/ 				{
/*?*/ 					bDontCreateObjects = TRUE;			//frmtool
/*?*/ 					SwFrm *pRow = new SwRowFrm( *GetTable()->GetTabLines()[0] );
/*?*/ 					bDontCreateObjects = FALSE;
/*?*/ 					pRow->Paste( this, Lower() );
/*?*/ 				}
/*?*/ 				else if ( Lower() )
/*?*/ 				{
/*?*/ 					SwFrm *pLow = Lower();
/*?*/ 					pLow->Cut();
/*?*/ 					delete pLow;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else if ( !HasFollow() )
/*?*/ 				rInvFlags |= 0x02;
/*?*/ 			break;
/*?*/ 
/*N*/ 		case RES_FRM_SIZE:
/*N*/ 		case RES_HORI_ORIENT:
/*N*/ 			rInvFlags |= 0x22;
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_PAGEDESC:						//Attributaenderung (an/aus)
/*N*/ 			if ( IsInDocBody() )
/*N*/ 			{
/*N*/ 				rInvFlags |= 0x40;
/*N*/ 				SwPageFrm *pPage = FindPageFrm();
/*N*/ 				if ( !GetPrev() )
/*N*/ 					CheckPageDescs( pPage );
/*N*/ 				if ( pPage && GetFmt()->GetPageDesc().GetNumOffset() )
/*N*/ 					((SwRootFrm*)pPage->GetUpper())->SetVirtPageNum( TRUE );
/*N*/ 				SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
/*N*/ 				GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_BREAK:
/*N*/ 			rInvFlags |= 0xC0;
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_LAYOUT_SPLIT:
/*?*/ 			if ( !IsFollow() )
/*?*/ 				rInvFlags |= 0x40;
/*?*/ 			break;
/*N*/         case RES_FRAMEDIR :
/*?*/             SetDerivedR2L( sal_False );
/*?*/             CheckDirChange();
/*?*/             break;
/*N*/ 		case RES_UL_SPACE:
/*N*/ 			rInvFlags |= 0x1C;
/*N*/ 			/* kein Break hier */
/*N*/ 
/*N*/ 		default:
/*N*/ 			bClear = FALSE;
/*N*/ 	}
/*N*/ 	if ( bClear )
/*N*/ 	{
/*N*/ 		if ( pOldSet || pNewSet )
/*N*/ 		{
/*N*/ 			if ( pOldSet )
/*N*/ 				pOldSet->ClearItem( nWhich );
/*N*/ 			if ( pNewSet )
/*N*/ 				pNewSet->ClearItem( nWhich );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			SwLayoutFrm::Modify( pOld, pNew );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  SwTabFrm::GetInfo()
|*
|*	  Ersterstellung	MA 06. Dec. 96
|*	  Letzte Aenderung	MA 26. Jun. 98
|*
|*************************************************************************/
/*N*/ BOOL SwTabFrm::GetInfo( SfxPoolItem &rHnt ) const
/*N*/ {
/*N*/ 	if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() )
/*N*/ 	{
/*N*/ 		SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
/*N*/ 		const SwPageFrm *pPage = FindPageFrm();
/*N*/ 		if ( pPage  )
/*N*/ 		{
/*N*/ 			if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
/*N*/ 			{
/*N*/ 				//Das sollte er sein (kann allenfalls temporaer anders sein,
/*N*/ 				//					  sollte uns das beunruhigen?)
/*N*/ 				rInfo.SetInfo( pPage, this );
/*N*/ 				return FALSE;
/*N*/ 			}
/*N*/ 			if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
/*N*/ 				 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
/*N*/ 			{
/*N*/ 				//Das koennte er sein.
/*N*/ 				rInfo.SetInfo( pPage, this );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*************************************************************************
|*
|*	  SwTabFrm::FindLastCntnt()
|*
|*	  Ersterstellung	MA 13. Apr. 93
|*	  Letzte Aenderung	MA 15. May. 98
|*
|*************************************************************************/
/*N*/ SwCntntFrm *SwTabFrm::FindLastCntnt()
/*N*/ {
/*N*/ 	SwFrm *pRet = pLower;
/*N*/ 	while ( pRet && !pRet->IsCntntFrm() )
/*N*/ 	{
/*N*/ 		SwFrm *pOld = pRet;
/*N*/ 
/*N*/         SwFrm *pTmp = pRet;             // To skip empty section frames
/*N*/ 		while ( pRet->GetNext() )
/*N*/         {
/*N*/ 			pRet = pRet->GetNext();
/*N*/             if( !pRet->IsSctFrm() || ((SwSectionFrm*)pRet)->GetSection() )
/*N*/                 pTmp = pRet;
/*N*/         }
/*N*/         pRet = pTmp;
/*N*/ 
/*N*/ 		if ( pRet->GetLower() )
/*N*/ 			pRet = pRet->GetLower();
/*N*/ 		if ( pRet == pOld )
/*N*/ 		{	// Wenn am Ende der letzten Zelle ein spaltiger Bereich steht,
/*?*/ 			// der eine leere letzte Spalte hat, muessen wir noch die anderen
/*?*/ 			// Spalten abklappern, dies erledigt SwSectionFrm::FindLastCntnt
/*?*/ 			if( pRet->IsColBodyFrm() )
/*?*/ 			{
/*?*/ #ifdef DBG_UTIL
/*?*/ 				SwSectionFrm* pSect = pRet->FindSctFrm();
/*?*/ 				ASSERT( pSect, "Wo kommt denn die Spalte her?")
/*?*/ 				ASSERT( IsAnLower( pSect ), "Gespaltene Zelle?" );
/*?*/ #endif
/*?*/ 				return pRet->FindSctFrm()->FindLastCntnt();
/*?*/ 			}
/*?*/ 			return 0;	//Hier geht es nicht weiter. Inkonsistenter Zustand
/*?*/ 						//der Tabelle (z.B. Undo TextToTable).
/*N*/ 		}
/*N*/ 	}
/*N*/ //	ASSERT( pRet && pRet->IsCntntFrm(), "Letzter Lower von Tab kein Cnt." );
/*N*/ 	if ( pRet ) //#50235#
/*N*/ 		while ( pRet->GetNext() )
/*N*/ 			pRet = pRet->GetNext();
/*N*/ 	if( pRet->IsSctFrm() )
/*?*/ 		pRet = ((SwSectionFrm*)pRet)->FindLastCntnt();
/*N*/ 	ASSERT( pRet && pRet->IsCntntFrm(), "Letzter Lower von Tab kein Cnt." );
/*N*/ 	return (SwCntntFrm*)pRet;
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::GetLeaf()
|*
|*	Ersterstellung		MA 19. Mar. 93
|*	Letzte Aenderung	MA 25. Apr. 95
|*
|*************************************************************************/
/*N*/ SwLayoutFrm *SwTabFrm::GetLeaf( MakePageType eMakePage, BOOL bFwd )
/*N*/ {
/*N*/ 	SwLayoutFrm *pRet;
/*N*/ 	if ( bFwd )
/*N*/ 	{
/*?*/ 		pRet = GetNextLeaf( eMakePage );
/*?*/ 		while ( IsAnLower( pRet ) )
/*?*/ 			pRet = pRet->GetNextLeaf( eMakePage );
/*?*/ 	}
/*N*/ 	else
/*N*/ 		pRet = GetPrevLeaf();
/*N*/ 	if ( pRet )
/*N*/ 		pRet->Calc();
/*N*/ 	return pRet;
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::ShouldBwdMoved()
|*
|* 	Beschreibung		Returnwert sagt ob der Frm verschoben werden sollte
|*	Ersterstellung		MA 10. Jul. 95
|*	Letzte Aenderung	MA 04. Mar. 97
|*
|*************************************************************************/
/*N*/ BOOL SwTabFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL bHead, BOOL &rReformat )
/*N*/ {
/*N*/ 	rReformat = FALSE;
/*N*/ 	if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()) )
/*N*/ 	{
/*N*/ 		//Das zurueckfliessen von Frm's ist leider etwas Zeitintensiv.
/*N*/ 		//Der haufigste Fall ist der, dass dort wo der Frm hinfliessen
/*N*/ 		//moechte die FixSize die gleiche ist, die der Frm selbst hat.
/*N*/ 		//In diesem Fall kann einfach geprueft werden, ob der Frm genug
/*N*/ 		//Platz fuer seine VarSize findet, ist dies nicht der Fall kann
/*N*/ 		//gleich auf das Verschieben verzichtet werden.
/*N*/ 		//Die Pruefung, ob der Frm genug Platz findet fuehrt er selbst
/*N*/ 		//durch, dabei wird beruecksichtigt, dass er sich moeglicherweise
/*N*/ 		//aufspalten kann.
/*N*/ 		//Wenn jedoch die FixSize eine andere ist oder Flys im Spiel sind
/*N*/ 		//(an der alten oder neuen Position) hat alle Prueferei keinen Sinn
/*N*/ 		//der Frm muss dann halt Probehalber verschoben werden (Wenn ueberhaupt
/*N*/ 		//etwas Platz zur Verfuegung steht).
/*N*/ 
/*N*/ 		//Die FixSize der Umgebungen in denen Tabellen herumlungern ist immer
/*N*/ 		//Die Breite.
/*N*/ 
/*N*/ 		SwPageFrm *pOldPage = FindPageFrm(),
/*N*/ 				  *pNewPage = pNewUpper->FindPageFrm();
/*N*/ 		BOOL bMoveAnyway = FALSE;
/*N*/ 		SwTwips nSpace = 0;
/*N*/ 
/*N*/         SWRECTFN( this )
/*N*/         if ( !SwFlowFrm::IsMoveBwdJump() )
/*N*/         {
/*N*/ 
/*N*/             long nOldWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
/*N*/             SWRECTFNX( pNewUpper );
/*N*/             long nNewWidth = (pNewUpper->Prt().*fnRectX->fnGetWidth)();
/*N*/             if( Abs( nNewWidth - nOldWidth ) < 2 )
/*N*/             {
/*N*/                 if( FALSE ==
/*N*/                     ( bMoveAnyway = BwdMoveNecessary( pOldPage, Frm() ) > 1 ) )
/*N*/                 {
/*N*/                     SwRect aRect( pNewUpper->Prt() );
/*N*/                     aRect.Pos() += pNewUpper->Frm().Pos();
/*N*/                     const SwFrm *pPrevFrm = pNewUpper->Lower();
/*N*/                     while ( pPrevFrm )
/*N*/                     {
/*N*/                         (aRect.*fnRectX->fnSetTop)( (pPrevFrm->Frm().*fnRectX->
/*N*/                                                     fnGetBottom)() );
/*N*/                         pPrevFrm = pPrevFrm->GetNext();
/*N*/                     }
/*N*/                     bMoveAnyway = BwdMoveNecessary( pNewPage, aRect) > 1;
/*N*/                     nSpace = (aRect.*fnRectX->fnGetHeight)();
/*N*/                     if ( GetFmt()->GetDoc()->IsBrowseMode() )
/*N*/                         nSpace += pNewUpper->Grow( LONG_MAX, TRUE );
/*N*/                 }
/*N*/             }
/*N*/             else if( !bLockBackMove )
/*N*/                 bMoveAnyway = TRUE;
/*N*/ 		}
/*N*/         else if( !bLockBackMove )
/*N*/ 			bMoveAnyway = TRUE;
/*N*/ 
/*N*/ 		if ( bMoveAnyway )
/*N*/ 			return rReformat = TRUE;
/*N*/ 		else if ( !bLockBackMove )
/*N*/ 		{	const BOOL bRepeat = GetTable()->IsHeadlineRepeat();
/*N*/ 			SwTwips nHeight = bRepeat && Lower()->GetNext() ?
/*N*/                     (Lower()->GetNext()->Frm().*fnRect->fnGetHeight)()
/*N*/                     : (Lower()->Frm().*fnRect->fnGetHeight)();
/*N*/ 			if ( bHead && bRepeat && Lower()->GetNext() )
/*N*/                 nHeight += (Lower()->Frm().*fnRect->fnGetHeight)();
/*N*/ 			return nHeight <= nSpace;
/*N*/ 		}
/*N*/ 	}
/*?*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::Cut()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 09. Sep. 98
|*
|*************************************************************************/
/*N*/ void SwTabFrm::Cut()
/*N*/ {
/*N*/ 	ASSERT( GetUpper(), "Cut ohne Upper()." );
/*N*/ 
/*N*/ 	SwPageFrm *pPage = FindPageFrm();
/*N*/ 	InvalidatePage( pPage );
/*N*/ 	SwFrm *pFrm = GetNext();
/*N*/ 	if( pFrm )
/*N*/ 	{	//Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
/*N*/ 		//berechnet der ist jetzt wo er der erste wird obsolete
/*N*/ 		pFrm->_InvalidatePrt();
/*N*/ 		pFrm->_InvalidatePos();
/*N*/ 		if ( pFrm->IsCntntFrm() )
/*N*/ 			pFrm->InvalidatePage( pPage );
/*N*/ 		if( IsInSct() && !GetPrev() )
/*N*/ 		{
/*?*/ 			SwSectionFrm* pSct = FindSctFrm();
/*?*/ 			if( !pSct->IsFollow() )
/*?*/ 			{
/*?*/ 				pSct->_InvalidatePrt();
/*?*/ 				pSct->InvalidatePage( pPage );
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		InvalidateNextPos();
/*N*/ 		//Einer muss die Retusche uebernehmen: Vorgaenger oder Upper
/*N*/ 		if ( 0 != (pFrm = GetPrev()) )
/*N*/ 		{	pFrm->SetRetouche();
/*N*/ 			pFrm->Prepare( PREP_WIDOWS_ORPHANS );
/*N*/ 			pFrm->_InvalidatePos();
/*N*/ 			if ( pFrm->IsCntntFrm() )
/*N*/ 				pFrm->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		//Wenn ich der einzige FlowFrm in meinem Upper bin (war), so muss
/*N*/ 		//er die Retouche uebernehmen.
/*N*/ 		//Ausserdem kann eine Leerseite entstanden sein.
/*N*/ 		else
/*N*/ 		{	SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
/*N*/ 			pRoot->SetSuperfluous();
/*N*/ 			GetUpper()->SetCompletePaint();
/*N*/ 			if( IsInSct() )
/*N*/ 			{
/*?*/ 				SwSectionFrm* pSct = FindSctFrm();
/*?*/ 				if( !pSct->IsFollow() )
/*?*/ 				{
/*?*/ 					pSct->_InvalidatePrt();
/*?*/ 					pSct->InvalidatePage( pPage );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Erst removen, dann Upper Shrinken.
/*N*/ 	SwLayoutFrm *pUp = GetUpper();
/*N*/     SWRECTFN( this )
/*N*/ 	Remove();
/*N*/ 	if ( pUp )
/*N*/ 	{
/*N*/ 		ASSERT( !pUp->IsFtnFrm(), "Tabelle in Fussnote." );
/*N*/ 		SwSectionFrm *pSct = 0;
/*N*/ 		if( !pUp->Lower() && pUp->IsInSct() &&
/*N*/ 			!(pSct = pUp->FindSctFrm())->ContainsCntnt() )
/*N*/ 		{
/*?*/ 			if ( pUp->GetUpper() )
/*?*/ 			{
/*?*/ 				pSct->DelEmpty( FALSE );
/*?*/ 				pSct->_InvalidateSize();
/*?*/ 			}
/*N*/ 		}
/*N*/         else if( (Frm().*fnRect->fnGetHeight)() )
/*N*/         {
/*N*/             // OD 24.02.2003 #104992# - unlock section the table frame was in.
/*N*/             // Otherwise, the section will not shrink.
/*N*/             // The section will be locked in this situation, if table is
/*N*/             // converted to text and the table was the only content in the
/*N*/             // section beside a footnote.
/*N*/             // Note: lock state will be restored.
/*N*/             bool bOldLock;
/*N*/             if ( pSct )
/*N*/             {
/*N*/                 bOldLock = pSct->IsColLocked() ? true : false;
/*N*/                 pSct->ColUnlock();
/*N*/             }
/*N*/             pUp->Shrink( Frm().Height() PHEIGHT );
/*N*/             // OD 24.02.2003 #104992# - restore section lock state.
/*N*/             if ( pSct )
/*N*/             {
/*N*/                 if ( bOldLock )
/*N*/                 {
/*N*/                     pSct->ColLock();
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pPage && !IsFollow() && pPage->GetUpper() )
/*N*/ 		((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
/*N*/ }

/*************************************************************************
|*
|*	SwTabFrm::Paste()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 09. Sep. 98
|*
|*************************************************************************/
/*N*/ void SwTabFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
/*N*/ {
/*N*/ 	ASSERT( pParent, "Kein Parent fuer Paste." );
/*N*/ 	ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
/*N*/ 	ASSERT( pParent != this, "Bin selbst der Parent." );
/*N*/ 	ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
/*N*/ 	ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
/*N*/ 			"Bin noch irgendwo angemeldet." );
/*N*/ 
/*N*/ 	//In den Baum einhaengen.
/*N*/ 	InsertBefore( (SwLayoutFrm*)pParent, pSibling );
/*N*/ 
/*N*/ 	_InvalidateAll();
/*N*/ 	SwPageFrm *pPage = FindPageFrm();
/*N*/ 	InvalidatePage( pPage );
/*N*/ 
/*N*/ 	if ( GetNext() )
/*N*/ 	{
/*N*/ 		GetNext()->_InvalidatePos();
/*N*/ 		GetNext()->_InvalidatePrt();
/*N*/ 		if ( GetNext()->IsCntntFrm() )
/*N*/ 			GetNext()->InvalidatePage( pPage );
/*N*/ 	}
/*N*/ 
/*N*/     SWRECTFN( this )
/*N*/     if( (Frm().*fnRect->fnGetHeight)() )
/*N*/         pParent->Grow( (Frm().*fnRect->fnGetHeight)() );
/*N*/ 
/*N*/     if( (Frm().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)() )
/*N*/ 		Prepare( PREP_FIXSIZE_CHG );
/*N*/ 	if ( GetPrev() )
/*N*/ 	{
/*N*/ 		if ( !IsFollow() )
/*N*/ 		{
/*N*/ 			GetPrev()->InvalidateSize();
/*N*/ 			if ( GetPrev()->IsCntntFrm() )
/*N*/ 				GetPrev()->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( GetNext() )
/*N*/ 		//Bei CntntFrm's gilt es den Abstand zum Vorgaenger/Nachfolger
/*N*/ 		//zu beachten. Faelle (beide treten immer gleichzeitig auf):
/*N*/ 		//a) Der Cntnt wird der erste einer Kette
/*N*/ 		//b) Der neue Nachfolger war vorher der erste einer Kette
/*N*/ 		GetNext()->_InvalidatePrt();
/*N*/ 
/*N*/ 	if ( pPage && !IsFollow() )
/*N*/ 	{
/*N*/ 		if ( pPage->GetUpper() )
/*N*/ 			((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
/*N*/ 
/*N*/ 		if ( !GetPrev() )//Mindestens fuer HTML mit Tabelle am Anfang notwendig.
/*N*/ 		{
/*N*/ 			const SwPageDesc *pDesc = GetFmt()->GetPageDesc().GetPageDesc();
/*N*/ 			if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
/*N*/ 				 (!pDesc && pPage->GetPageDesc() != &GetFmt()->GetDoc()->GetPageDesc(0)) )
/*N*/ 				CheckPageDescs( pPage, TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*  SwTabFrm::Prepare()
|*
|*  Created        AMA 01/10/02
|*  Last Change    AMA 01/10/02
|*
|*************************************************************************/
/*N*/ void SwTabFrm::Prepare( const PrepareHint eHint, const void *, BOOL )
/*N*/ {
/*N*/     if( PREP_BOSS_CHGD == eHint )
/*N*/         CheckDirChange();
/*N*/ }

/*************************************************************************
|*
|*	SwRowFrm::SwRowFrm(), ~SwRowFrm()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 30. May. 96
|*
|*************************************************************************/
/*N*/ SwRowFrm::SwRowFrm( const SwTableLine &rLine ):
/*N*/ 	SwLayoutFrm( rLine.GetFrmFmt() ),
/*N*/ 	pTabLine( &rLine )
/*N*/ {
/*N*/     nType = FRMC_ROW;
/*N*/ 
/*N*/ 	//Gleich die Boxen erzeugen und einfuegen.
/*N*/ 	const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
/*N*/ 	SwFrm *pPrev = 0;
/*N*/ 	for ( USHORT i = 0; i < rBoxes.Count(); ++i )
/*N*/ 	{
/*N*/ 		SwCellFrm *pNew = new SwCellFrm( *rBoxes[i] );
/*N*/ 		pNew->InsertBehind( this, pPrev );
/*N*/ 		pPrev = pNew;
/*N*/ 	}
/*N*/ }

/*N*/ SwRowFrm::~SwRowFrm()
/*N*/ {
/*N*/ 	SwModify* pMod = GetFmt();
/*N*/ 	if( pMod )
/*N*/ 	{
/*N*/ 		pMod->Remove( this );			// austragen,
/*N*/ 		if( !pMod->GetDepends() )
/*?*/ 			delete pMod;				// und loeschen
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwRowFrm::RegistFlys()
|*
|*	Ersterstellung		MA 08. Jul. 93
|*	Letzte Aenderung	MA 08. Jul. 93
|*
|*************************************************************************/
/*N*/ void SwRowFrm::RegistFlys( SwPageFrm *pPage )
/*N*/ {
/*N*/ 	::binfilter::RegistFlys( pPage ? pPage : FindPageFrm(), this );
/*N*/ }

/*************************************************************************
|*
|*	  SwRowFrm::Modify()
|*
|*	  Ersterstellung	MA 12. Nov. 97
|*	  Letzte Aenderung	MA 12. Nov. 97
|*
|*************************************************************************/
/*N*/ void SwRowFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
/*N*/ {
/*N*/ 	BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
/*N*/ 	const SfxPoolItem *pItem = 0;
/*N*/ 
/*N*/ 	if( bAttrSetChg )
/*N*/ 		((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_FRM_SIZE, FALSE, &pItem);
/*N*/ 	else if ( RES_FRM_SIZE == pNew->Which() )
/*?*/ 		pItem = pNew;
/*N*/ 
/*N*/ 	if ( pItem )
/*N*/ 	{
/*N*/ 		SwTabFrm *pTab = FindTabFrm();
/*N*/ 		if ( pTab && pTab->IsFollow() &&
/*N*/ 			 (!GetPrev() ||
/*N*/ 			  (pTab->GetTable()->IsHeadlineRepeat() && !GetPrev()->GetPrev())))
/*N*/ 		{
/*?*/ 			pTab->FindMaster()->InvalidatePos();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwLayoutFrm::Modify( pOld, pNew );
/*N*/ }



/*************************************************************************
|*
|*	SwRowFrm::MakeAll()
|*
|*	Ersterstellung		MA 01. Mar. 94
|*	Letzte Aenderung	MA 01. Mar. 94
|*
|*************************************************************************/
/*N*/ void SwRowFrm::MakeAll()
/*N*/ {
/*N*/ 	if ( !GetNext() )
/*N*/ 		bValidSize = FALSE;
/*N*/ 	SwLayoutFrm::MakeAll();
/*N*/ }

/*************************************************************************
|*
|*	SwRowFrm::Format()
|*
|*	Ersterstellung		MA 13. Mar. 93
|*	Letzte Aenderung	MA 20. Jun. 96
|*
|*************************************************************************/
/*M*/ long MA_FASTCALL CalcHeightWidthFlys( const SwFrm *pFrm )
/*M*/ {
/*M*/     SWRECTFN( pFrm )
/*M*/ 	long nHeight = 0;
/*M*/ 	const SwFrm* pTmp = pFrm->IsSctFrm() ?
/*M*/ 			((SwSectionFrm*)pFrm)->ContainsCntnt() : pFrm;
/*M*/ 	while( pTmp )
/*M*/ 	{
/*M*/ 		if ( pTmp->GetDrawObjs() )
/*M*/ 		{
/*M*/ 			for ( USHORT i = 0; i < pTmp->GetDrawObjs()->Count(); ++i )
/*M*/ 			{
/*M*/ 				const SdrObject *pO = (*pTmp->GetDrawObjs())[i];
/*M*/ 				if ( pO->IsWriterFlyFrame() )
/*M*/ 				{
/*M*/ 					const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*M*/                     if( !pFly->IsFlyInCntFrm() && pFly->Frm().Top()!=WEIT_WECH )
/*M*/ 					{
/*M*/ 						const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
/*M*/ 						if( !rSz.GetHeightPercent() )
/*M*/                         {
/*M*/                             const SwTwips nFlyWidth =
/*M*/                                 (pFly->Frm().*fnRect->fnGetHeight)() +
/*M*/                                     ( bVert ?
/*M*/                                       pFly->GetCurRelPos().X() :
/*M*/                                       pFly->GetCurRelPos().Y() );
/*M*/ 
/*M*/                             const SwTwips nFrmDiff =
/*M*/                                 (*fnRect->fnYDiff)(
/*M*/                                     (pTmp->Frm().*fnRect->fnGetTop)(),
/*M*/                                     (pFrm->Frm().*fnRect->fnGetTop)() );
/*M*/ 
/*M*/                             nHeight = Max( nHeight, nFlyWidth + nFrmDiff -
/*M*/                                             (pFrm->Frm().*fnRect->fnGetHeight)() );
/*M*/                         }
/*M*/ 					}
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/ 		if( !pFrm->IsSctFrm() )
/*M*/ 			break;
/*M*/ 		pTmp = pTmp->FindNextCnt();
/*M*/ 		if( !((SwSectionFrm*)pFrm)->IsAnLower( pTmp ) )
/*M*/ 			break;
/*M*/ 	}
/*M*/ 	return nHeight;
/*M*/ }

/*N*/ SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwLayoutFrm *pRow );

/*N*/ SwTwips MA_FASTCALL lcl_CalcMinCellHeight( SwLayoutFrm *pCell,
/*N*/ 								  const SwBorderAttrs *pAttrs = 0 )
/*N*/ {
/*N*/     SWRECTFN( pCell )
/*N*/ 	SwTwips nHeight = 0;
/*N*/ 	SwFrm *pLow = pCell->Lower();
/*N*/ 	if ( pLow )
/*N*/ 	{
/*N*/ 		long nFlyAdd = 0;
/*N*/ 		while ( pLow )
/*N*/ 		{
/*N*/ 			if( pLow->IsCntntFrm() || pLow->IsSctFrm() )
/*N*/ 			{
/*N*/                 long nLowHeight = (pLow->Frm().*fnRect->fnGetHeight)();
/*N*/                 nHeight += nLowHeight;
/*N*/                 nFlyAdd = Max( 0L, nFlyAdd - nLowHeight );
/*N*/ 				nFlyAdd = Max( nFlyAdd, ::binfilter::CalcHeightWidthFlys( pLow ) );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nHeight += ::binfilter::lcl_CalcMinRowHeight( (SwLayoutFrm*)pLow );
/*N*/ 
/*N*/ 			pLow = pLow->GetNext();
/*N*/ 		}
/*N*/ 		if ( nFlyAdd )
/*N*/ 			nHeight += nFlyAdd;
/*N*/ 	}
/*N*/ 	//Der Border will natuerlich auch mitspielen, er kann leider nicht
/*N*/ 	//aus PrtArea und Frm errechnet werden, da diese in beliebiger
/*N*/ 	//Kombination ungueltig sein koennen.
/*N*/ 	if ( pAttrs )
/*N*/ 		nHeight += pAttrs->CalcTop() + pAttrs->CalcBottom();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
/*N*/ 		const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 		nHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();
/*N*/ 	}
/*N*/ 	return nHeight;
/*N*/ }

/*N*/ SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwLayoutFrm *pRow )
/*N*/ {
/*N*/     SWRECTFN( pRow )
/*N*/     if ( pRow->HasFixSize() )
/*N*/         return (pRow->Frm().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/ 	SwTwips nHeight = 0;
/*N*/ 	SwLayoutFrm *pLow = (SwLayoutFrm*)pRow->Lower();
/*N*/ 	while ( pLow )
/*N*/ 	{
/*N*/ 		SwTwips nTmp = ::binfilter::lcl_CalcMinCellHeight( pLow );
/*N*/ 		if ( nTmp > nHeight )
/*N*/ 			nHeight = nTmp;
/*N*/ 		pLow = (SwLayoutFrm*)pLow->GetNext();
/*N*/ 	}
/*N*/ 	const SwFmtFrmSize &rSz = pRow->GetFmt()->GetFrmSize();
/*N*/ 	if ( rSz.GetSizeType() == ATT_MIN_SIZE )
/*N*/ 		nHeight = Max( nHeight, rSz.GetHeight() );
/*N*/ 	return nHeight;
/*N*/ }

/*N*/ void SwRowFrm::Format( const SwBorderAttrs *pAttrs )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/ 	ASSERT( pAttrs, "SwRowFrm::Format ohne Attrs." );
/*N*/ 
/*N*/     const BOOL bFix = BFIXHEIGHT;
/*N*/ 
/*N*/ 	if ( !bValidPrtArea )
/*N*/ 	{
/*N*/ 		//RowFrms haben keine Umrandung usw. also entspricht die PrtArea immer
/*N*/ 		//dem Frm.
/*N*/ 		bValidPrtArea = TRUE;
/*N*/ 		aPrt.Left( 0 );
/*N*/ 		aPrt.Top( 0 );
/*N*/ 		aPrt.Width ( aFrm.Width() );
/*N*/ 		aPrt.Height( aFrm.Height() );
/*N*/ 	}
/*N*/ 
/*N*/ 	while ( !bValidSize )
/*N*/ 	{
/*N*/ 		bValidSize = TRUE;
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/         if ( HasFixSize() )
/*N*/ 		{
/*N*/ 			const SwFmtFrmSize &rFrmSize = GetFmt()->GetFrmSize();
/*N*/ 			ASSERT( rFrmSize.GetSize().Height() > 0, "Hat ihn" );
/*N*/ 		}
/*N*/ #endif
/*N*/         const SwTwips nDiff = (Frm().*fnRect->fnGetHeight)() - (HasFixSize() ?
/*N*/                                                 pAttrs->GetSize().Height() :
/*N*/                                                 ::binfilter::lcl_CalcMinRowHeight( this ));
/*N*/         if ( nDiff )
/*N*/ 		{
/*N*/             BFIXHEIGHT = FALSE;
/*N*/ 			if ( nDiff > 0 )
/*N*/                 Shrink( nDiff PHEIGHT, FALSE, TRUE );
/*N*/ 			else if ( nDiff < 0 )
/*N*/                 Grow( -nDiff PHEIGHT );
/*N*/             BFIXHEIGHT = bFix;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !GetNext() )
/*N*/ 	{
/*N*/ 		//Der letzte fuellt den verbleibenden Raum im Upper aus.
/*N*/         SwTwips nDiff = (GetUpper()->Prt().*fnRect->fnGetHeight)();
/*N*/ 		SwFrm *pSibling = GetUpper()->Lower();
/*N*/ 		do
/*N*/         {   nDiff -= (pSibling->Frm().*fnRect->fnGetHeight)();
/*N*/ 			pSibling = pSibling->GetNext();
/*N*/ 		} while ( pSibling );
/*N*/ 		if ( nDiff > 0 )
/*N*/ 		{
/*N*/             BFIXHEIGHT = FALSE;
/*N*/             Grow( nDiff PHEIGHT );
/*N*/             BFIXHEIGHT = bFix;
/*N*/ 			bValidSize = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwRowFrm::AdjustCells()
|*
|*	Ersterstellung		MA 10. Aug. 93
|*	Letzte Aenderung	MA 16. Dec. 96
|*
|*************************************************************************/
/*N*/ void SwRowFrm::AdjustCells( const SwTwips nHeight, const BOOL bHeight )
/*N*/ {
/*N*/ 	SwFrm *pFrm = Lower();
/*N*/ 	if ( bHeight )
/*N*/ 	{
/*N*/ 		SwRootFrm *pRootFrm = 0;
/*N*/         SWRECTFN( this )
/*N*/ 		while ( pFrm )
/*N*/         {
/*N*/             long nDiff = nHeight - (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/             if( nDiff )
/*N*/ 			{
/*N*/ 				SwRect aOldFrm( pFrm->Frm() );
/*N*/                 (pFrm->Frm().*fnRect->fnAddBottom)( nDiff );
/*N*/ 				if( !pRootFrm )
/*N*/ 					pRootFrm = FindRootFrm();
/*N*/ 				if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 					pRootFrm->GetCurrShell() )
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aOldFrm );
/*N*/ 				}
/*N*/ 				pFrm->_InvalidatePrt();
/*N*/ 			}
/*N*/ 			pFrm = pFrm->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	while ( pFrm )
/*N*/ 		{
/*N*/ 			pFrm->_InvalidateAll();
/*N*/ 			pFrm = pFrm->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	InvalidatePage();
/*N*/ }

/*************************************************************************
|*
|*	SwRowFrm::Cut()
|*
|*	Ersterstellung		MA 12. Nov. 97
|*	Letzte Aenderung	MA 12. Nov. 97
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwRowFrm::GrowFrm()
|*
|*	Ersterstellung		MA 15. Mar. 93
|*	Letzte Aenderung	MA 05. May. 94
|*
|*************************************************************************/


/*N*/ SwTwips SwRowFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/     const SwTwips nReal = SwLayoutFrm::GrowFrm( nDist, bTst, bInfo);
/*N*/ 
/*N*/ 	//Hoehe der Zellen auf den neuesten Stand bringen.
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/         SWRECTFN( this )
/*N*/         AdjustCells( (Prt().*fnRect->fnGetHeight)() + nReal, TRUE );
/*N*/ 		if ( nReal )
/*N*/ 			SetCompletePaint();
/*N*/ 	}
/*N*/ 	return nReal;
/*N*/ }
/*************************************************************************
|*
|*	SwRowFrm::ShrinkFrm()
|*
|*	Ersterstellung		MA 15. Mar. 93
|*	Letzte Aenderung	MA 20. Jun. 96
|*
|*************************************************************************/
/*N*/ SwTwips SwRowFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/     if( HasFixSize() )
/*N*/ 	{
/*M*/                 AdjustCells( (Prt().*fnRect->fnGetHeight)(), TRUE );
/*N*/ 		return 0L;
/*N*/ 	}
/*N*/ 
/*N*/ 	//bInfo wird ggf. vom SwRowFrm::Format auf TRUE gesetzt, hier muss dann
/*N*/ 	//entsprechend reagiert werden
/*N*/ 	const BOOL bShrinkAnyway = bInfo;
/*N*/ 
/*N*/ 	//Nur soweit Shrinken, wie es der Inhalt der groessten Zelle zulaesst.
/*N*/ 	SwTwips nRealDist = nDist;
/*N*/ 	{
/*N*/ 		const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
/*N*/         SwTwips nMinHeight = rSz.GetSizeType() == ATT_MIN_SIZE ?
/*N*/                              rSz.GetHeight() : 0;
/*N*/         SwLayoutFrm *pCell = (SwLayoutFrm*)Lower();
/*N*/         if( nMinHeight < (Frm().*fnRect->fnGetHeight)() )
/*N*/ 		{
/*N*/ 			SwLayoutFrm *pCell = (SwLayoutFrm*)Lower();
/*N*/ 			while ( pCell )
/*N*/ 			{
/*N*/ 				SwTwips nAct = ::binfilter::lcl_CalcMinCellHeight( pCell );
/*N*/ 				if ( nAct > nMinHeight )
/*N*/ 					nMinHeight = nAct;
/*N*/                 if ( nMinHeight >= (Frm().*fnRect->fnGetHeight)() )
/*N*/ 					break;
/*N*/ 				pCell = (SwLayoutFrm*)pCell->GetNext();
/*N*/ 			}
/*N*/ 		}
/*N*/         if ( ((Frm().*fnRect->fnGetHeight)() - nRealDist) < nMinHeight )
/*N*/             nRealDist = (Frm().*fnRect->fnGetHeight)() - nMinHeight;
/*N*/ 	}
/*N*/ 	if ( nRealDist < 0 )
/*?*/ 		nRealDist = 0;
/*N*/ 
/*N*/ 	SwTwips nReal = nRealDist;
/*N*/ 	if ( nReal )
/*N*/ 	{
/*N*/ 		if ( !bTst )
/*N*/         {
/*N*/             SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/             (Frm().*fnRect->fnSetHeight)( nHeight - nReal );
/*N*/             if( IsVertical() && !bRev )
/*?*/                 Frm().Pos().X() += nReal;
/*N*/         }
/*N*/ 
/*N*/         SwTwips nTmp = GetUpper()->Shrink( nReal, bTst );
/*N*/ 		if ( !bShrinkAnyway && !GetNext() && nTmp != nReal )
/*N*/ 		{
/*N*/ 			//Der letzte bekommt den Rest im Upper und nimmt deshalb
/*N*/ 			//ggf. Ruecksichten (sonst: Endlosschleife)
/*N*/ 			if ( !bTst )
/*N*/             {
/*N*/                 nReal -= nTmp;
/*N*/                 SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/                 (Frm().*fnRect->fnSetHeight)( nHeight + nReal );
/*N*/                 if( IsVertical() && !bRev )
/*?*/                     Frm().Pos().X() -= nReal;
/*N*/             }
/*N*/ 			nReal = nTmp;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Geeignet invalidieren und die Hoehe der Zellen auf den neuesten
/*N*/ 	//Stand bringen.
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/ 		if ( nReal )
/*N*/ 		{
/*N*/ 			if ( GetNext() )
/*?*/ 				GetNext()->_InvalidatePos();
/*N*/ 			_InvalidateAll();
/*N*/ 			SetCompletePaint();
/*N*/ 
/*N*/ 			SwTabFrm *pTab = FindTabFrm();
/*N*/ 			if ( pTab->IsFollow() &&
/*N*/ 				 (!GetPrev() ||
/*N*/ 				  (pTab->GetTable()->IsHeadlineRepeat() && !GetPrev()->GetPrev())))
/*N*/ 			{
/*?*/ 				pTab->FindMaster()->InvalidatePos();
/*N*/ 			}
/*N*/ 		}
/*N*/         AdjustCells( (Prt().*fnRect->fnGetHeight)() - nReal, TRUE );
/*N*/ 	}
/*N*/ 	return nReal;
/*N*/ }


/*************************************************************************
|*
|*	SwCellFrm::SwCellFrm(), ~SwCellFrm()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 30. May. 96
|*
|*************************************************************************/
/*N*/ SwCellFrm::SwCellFrm( const SwTableBox &rBox ) :
/*N*/ 	SwLayoutFrm( rBox.GetFrmFmt() ),
/*N*/ 	pTabBox( &rBox )
/*N*/ {
/*N*/     nType = FRMC_CELL;
/*N*/ 
/*N*/ 	//Wenn ein StartIdx vorhanden ist, so werden CntntFrms in der Zelle
/*N*/ 	//angelegt, andernfalls muessen Rows vorhanden sein und diese werden
/*N*/ 	//angelegt.
/*N*/ 	if ( rBox.GetSttIdx() )
/*N*/ 	{
/*N*/ 		ULONG nIndex = rBox.GetSttIdx();
/*N*/ 		::binfilter::_InsertCnt( this, rBox.GetFrmFmt()->GetDoc(), ++nIndex );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	const SwTableLines &rLines = rBox.GetTabLines();
/*N*/ 		SwFrm *pPrev = 0;
/*N*/ 		for ( USHORT i = 0; i < rLines.Count(); ++i )
/*N*/ 		{
/*N*/ 			SwRowFrm *pNew = new SwRowFrm( *rLines[i] );
/*N*/ 			pNew->InsertBehind( this, pPrev );
/*N*/ 			pPrev = pNew;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SwCellFrm::~SwCellFrm()
/*N*/ {
/*N*/ 	SwModify* pMod = GetFmt();
/*N*/ 	if( pMod )
/*N*/ 	{
/*N*/ 		// At this stage the lower frames aren't destroyed already,
/*N*/ 		// therfor we have to do a recursive dispose.
/*N*/ 		SwRootFrm *pRootFrm = FindRootFrm();
/*N*/ 		if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 			pRootFrm->GetCurrShell() )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pRootFrm->GetCurrShell()->Imp()->DisposeAccessibleFrm( this, sal_True );
/*N*/ 		}
/*N*/ 		pMod->Remove( this );			// austragen,
/*N*/         pMod->Remove( this );           // austragen,
/*N*/ 		if( !pMod->GetDepends() )
/*?*/ 			delete pMod;				// und loeschen
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwCellFrm::Format()
|*
|*	Ersterstellung		MA 09. Mar. 93
|*	Letzte Aenderung	MA 29. Jan. 98
|*
|*************************************************************************/
/*N*/ BOOL lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, BOOL bInva )
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	SwFrm *pFrm = pLay->Lower();
/*N*/     SwPageFrm* pPg = NULL;
/*N*/     SWRECTFN( pLay )
/*N*/ 	while ( pFrm )
/*N*/ 	{
/*N*/         long nFrmTop = (pFrm->Frm().*fnRect->fnGetTop)();
/*N*/         if( nFrmTop != lYStart )
/*N*/ 		{
/*N*/ 			bRet = TRUE;
/*N*/             const long lDiff = (*fnRect->fnYDiff)( lYStart, nFrmTop );
/*N*/             const long lDiffX = lYStart - nFrmTop;
/*N*/             (pFrm->Frm().*fnRect->fnSubTop)( -lDiff );
/*N*/             (pFrm->Frm().*fnRect->fnAddBottom)( lDiff );
/*N*/ 			pFrm->SetCompletePaint();
/*N*/ 			if ( !pFrm->GetNext() )
/*N*/ 				pFrm->SetRetouche();
/*N*/ 			if( bInva )
/*N*/ 				pFrm->Prepare( PREP_POS_CHGD );
/*N*/ 			if ( pFrm->IsLayoutFrm() && ((SwLayoutFrm*)pFrm)->Lower() )
/*N*/ 				lcl_ArrangeLowers( (SwLayoutFrm*)pFrm,
/*N*/                     (((SwLayoutFrm*)pFrm)->Lower()->Frm().*fnRect->fnGetTop)()
/*N*/                     + lDiffX, bInva );
/*N*/ 			if ( pFrm->GetDrawObjs() )
/*N*/ 			{
/*N*/ 				for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
/*N*/ 				{
/*N*/ 					SdrObject *pO = (*pFrm->GetDrawObjs())[i];
/*N*/ 					if ( pO->IsWriterFlyFrame() )
/*N*/ 					{
/*N*/ 						SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/                         if( WEIT_WECH != pFly->Frm().Top() )
/*N*/                         {
/*N*/                             (pFly->Frm().*fnRect->fnSubTop)( -lDiff );
/*N*/                             (pFly->Frm().*fnRect->fnAddBottom)( lDiff );
/*N*/                         }
/*N*/                         pFly->GetVirtDrawObj()->_SetRectsDirty();
/*N*/ 						if ( pFly->IsFlyInCntFrm() )
/*N*/ 							((SwFlyInCntFrm*)pFly)->AddRefOfst( lDiff );
/*N*/                         else
/*N*/                         {
/*N*/                             if( !pPg )
/*N*/                                 pPg = pLay->FindPageFrm();
/*N*/                             SwPageFrm* pOld = pFly->FindPageFrm();
/*N*/                             if( pPg != pOld )
/*N*/                                 pOld->MoveFly( pFly, pPg );
/*N*/                             if( pFly->IsAutoPos() )
/*?*/                                 ((SwFlyAtCntFrm*)pFly)->AddLastCharY( lDiff );
/*N*/                         }
/*N*/                         if( ::binfilter::lcl_ArrangeLowers( pFly,
/*N*/                             (pFly->*fnRect->fnGetPrtTop)(), bInva ) )
/*N*/ 							pFly->SetCompletePaint();
/*N*/ 					}
/*N*/ 					else
/*N*/                     {
/*N*/                         // OD 30.06.2003 #108784# - consider 'virtual' drawing
/*N*/                         // objects.
/*N*/                         if ( pO->ISA(SwDrawVirtObj) )
/*N*/                         {
/*N*/                             SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pO);
/*N*/                             pDrawVirtObj->SetAnchorPos( pFrm->GetFrmAnchorPos( ::binfilter::HasWrap( pO ) ) );
/*N*/                             pDrawVirtObj->AdjustRelativePosToReference();
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             pO->SetAnchorPos( pFrm->GetFrmAnchorPos( ::binfilter::HasWrap( pO ) ) );
/*N*/                             // OD 30.06.2003 #108784# - correct relative position
/*N*/                             // of 'virtual' drawing objects.
/*N*/                             SwDrawContact* pDrawContact =
/*N*/                                 static_cast<SwDrawContact*>(pO->GetUserCall());
/*N*/                             if ( pDrawContact )
/*N*/                             {
/*N*/                                 pDrawContact->CorrectRelativePosOfVirtObjs();
/*N*/                             }
/*N*/                         }
/*N*/                     }
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/         // Columns and cells are ordered horizontal, not vertical
/*N*/         if( !pFrm->IsColumnFrm() && !pFrm->IsCellFrm() )
/*N*/             lYStart = (*fnRect->fnYInc)( lYStart,
/*N*/                                         (pFrm->Frm().*fnRect->fnGetHeight)() );
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*N*/ void SwCellFrm::Format( const SwBorderAttrs *pAttrs )
/*N*/ {
/*N*/ 	ASSERT( pAttrs, "CellFrm::Format, pAttrs ist 0." );
/*N*/     SWRECTFN( this )
/*N*/ 	if ( !bValidPrtArea )
/*N*/ 	{
/*N*/ 		bValidPrtArea = TRUE;
/*N*/ 
/*N*/         //Position einstellen.
/*N*/         long nLeftSpace = pAttrs->CalcLeft( this );
/*N*/         // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
/*N*/         long nRightSpace = pAttrs->CalcRight( this );
/*N*/         (this->*fnRect->fnSetXMargins)( nLeftSpace, nRightSpace );
/*N*/         long nTopSpace = pAttrs->CalcTop();
/*N*/         long nBottomSpace = pAttrs->CalcBottom();
/*N*/         (this->*fnRect->fnSetYMargins)( nTopSpace, nBottomSpace );
/*N*/ 	}
/*N*/ 	long nRemaining = ::binfilter::lcl_CalcMinCellHeight( this, pAttrs );
/*N*/ 	if ( !bValidSize )
/*N*/ 	{
/*N*/ 		bValidSize = TRUE;
/*N*/ 
/*N*/ 		//Die VarSize der CellFrms ist immer die Breite.
/*N*/ 		//Tatsaechlich ist die Breite jedoch nicht Variabel, sie wird durch das
/*N*/ 		//Format vorgegeben. Dieser Vorgegebene Wert muss aber nun wiederum
/*N*/ 		//nicht der tatsaechlichen Breite entsprechen. Die Breite wird auf
/*N*/ 		//Basis des Attributes errechnet, der Wert im Attribut passt zu dem
/*N*/ 		//gewuenschten Wert des TabFrms. Anpassungen die dort vorgenommen
/*N*/ 		//wurden werden hier Proportional beruecksichtigt.
/*N*/ 		//Wenn die Celle keinen Nachbarn mehr hat beruecksichtigt sie nicht
/*N*/ 		//die Attribute, sonder greift sich einfach den Rest des
/*N*/ 		//Uppers.
/*N*/ 		SwTwips nWidth;
/*N*/ 		if ( GetNext() )
/*N*/ 		{
/*N*/ 			const SwTabFrm *pTab = FindTabFrm();
/*N*/ 			SwTwips nWish = pTab->GetFmt()->GetFrmSize().GetWidth();
/*N*/ 			nWidth = pAttrs->GetSize().Width();
/*N*/ 
/*N*/ 			ASSERT( nWish, "Tabelle ohne Breite?" );
/*N*/ 			ASSERT( nWidth <= nWish, "Zelle breiter als Tabelle." );
/*N*/ 			ASSERT( nWidth > 0, "Box without width" );
/*N*/ 
/*N*/             long nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
/*N*/             if ( nWish != nPrtWidth )
/*N*/ 			{
/*N*/                 nWidth *= nPrtWidth;
/*N*/ 				nWidth /= nWish;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			ASSERT( pAttrs->GetSize().Width() > 0, "Box without width" );
/*N*/             nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
/*N*/ 			SwFrm *pPre = GetUpper()->Lower();
/*N*/ 			while ( pPre != this )
/*N*/             {   nWidth -= (pPre->Frm().*fnRect->fnGetWidth)();
/*N*/ 				pPre = pPre->GetNext();
/*N*/ 			}
/*N*/ 		}
/*N*/         const long nDiff = nWidth - (Frm().*fnRect->fnGetWidth)();
/*N*/         if( IsNeighbourFrm() && IsRightToLeft() )
/*N*/             (Frm().*fnRect->fnSubLeft)( nDiff );
/*N*/         else
/*N*/             (Frm().*fnRect->fnAddRight)( nDiff );
/*N*/         (Prt().*fnRect->fnAddRight)( nDiff );
/*N*/ 
/*N*/ 		//Jetzt die Hoehe einstellen, sie wird vom Inhalt und den Raendern
/*N*/ 		//bestimmt.
/*N*/         const long nDiffHeight = nRemaining - (Frm().*fnRect->fnGetHeight)();
/*N*/ 		if ( nDiffHeight )
/*N*/ 		{
/*N*/ 			if ( nDiffHeight > 0 )
/*N*/ 			{
/*N*/ 				//Wieder validieren wenn kein Wachstum stattgefunden hat.
/*N*/ 				//Invalidiert wird durch AdjustCells von der Row.
/*N*/                 if ( !Grow( nDiffHeight PHEIGHT ) )
/*N*/ 					bValidSize = bValidPrtArea = TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				//Nur dann invalidiert lassen, wenn tatsaechlich
/*N*/ 				//geshrinkt wurde; das kann abgelehnt werden, weil alle
/*N*/ 				//nebeneinanderliegenden Zellen gleichgross sein muessen.
/*N*/                 if ( !Shrink( -nDiffHeight PHEIGHT ) )
/*N*/ 					bValidSize = bValidPrtArea = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	const SwFmtVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();
/*N*/ 	if ( VERT_NONE != rOri.GetVertOrient() )
/*N*/ 	{
/*N*/ 		if ( !Lower()->IsCntntFrm() && !Lower()->IsSctFrm() )
/*N*/ 		{
/*N*/ 			//ASSERT fuer HTML-Import!
/*N*/ 			ASSERT( !this, "VAlign an Zelle ohne Inhalt" );
/*N*/ 			return;
/*N*/ 		}
/*N*/ 		BOOL bVertDir = TRUE;
/*N*/ 		//Keine Ausrichtung wenn Rahmen mit Umlauf in die Zelle ragen.
/*N*/ 		SwPageFrm *pPg = FindPageFrm();
/*N*/ 		if ( pPg->GetSortedObjs() )
/*N*/ 		{
/*N*/ 			SwRect aRect( Prt() ); aRect += Frm().Pos();
/*N*/ 			for ( USHORT i = 0; i < pPg->GetSortedObjs()->Count(); ++i )
/*N*/ 			{
/*N*/ 				const SdrObject *pObj = (*pPg->GetSortedObjs())[i];
/*N*/ 				SwRect aTmp( pObj->GetBoundRect() );
/*N*/ 				if ( aTmp.IsOver( aRect ) )
/*N*/ 				{
/*N*/ 					SdrObjUserCall *pUserCall;
/*N*/ 					const SwFmtSurround &rSur = ((SwContact*)
/*N*/ 							   (pUserCall=GetUserCall(pObj)))->GetFmt()->GetSurround();
/*N*/ 					if ( SURROUND_THROUGHT != rSur.GetSurround() )
/*N*/ 					{
/*N*/ 						const SwFrm *pAnch;
/*N*/ 						if ( pObj->IsWriterFlyFrame() )
/*N*/ 						{
/*N*/ 							const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 							if ( pFly->IsAnLower( this ) )
/*N*/ 								continue;
/*N*/ 							pAnch = pFly->GetAnchor();
/*N*/ 						}
/*N*/ 						else
/*N*/ 							pAnch = ((SwDrawContact*)pUserCall)->GetAnchor();
/*N*/ 						if ( !IsAnLower( pAnch ) )
/*N*/ 						{
/*N*/ 							bVertDir = FALSE;
/*N*/ 							break;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/         long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/         if( ( bVertDir && ( nRemaining -= (pAttrs->CalcTop() +
/*N*/                             pAttrs->CalcBottom())) < nPrtHeight ) ||
/*N*/             (Lower()->Frm().*fnRect->fnGetTop)() !=
/*N*/             (this->*fnRect->fnGetPrtTop)() )
/*N*/ 		{
/*N*/ 			long lTopOfst = 0,
/*N*/                     nDiff = (Prt().*fnRect->fnGetHeight)() - nRemaining;
/*N*/             if ( nDiff >= 0 )
/*N*/ 			{
/*N*/ 				if ( bVertDir )
/*N*/ 				{
/*N*/ 					switch ( rOri.GetVertOrient() )
/*N*/ 					{
/*N*/ 						case VERT_CENTER:	lTopOfst = nDiff / 2; break;
/*N*/ 						case VERT_BOTTOM:	lTopOfst = nDiff;	  break;
/*N*/ 					};
/*N*/ 				}
/*N*/                 long nTmp = (*fnRect->fnYInc)(
/*N*/                                     (this->*fnRect->fnGetPrtTop)(), lTopOfst );
/*N*/                 if ( lcl_ArrangeLowers( this, nTmp, !bVertDir ) )
/*N*/ 					SetCompletePaint();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//Ist noch eine alte Ausrichtung beruecksichtigt worden?
/*N*/ 		if ( Lower()->IsCntntFrm() )
/*N*/ 		{
/*N*/             const long lYStart = (this->*fnRect->fnGetPrtTop)();
/*N*/ 			lcl_ArrangeLowers( this, lYStart, TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  SwCellFrm::Modify()
|*
|*	  Ersterstellung	MA 20. Dec. 96
|*	  Letzte Aenderung	MA 20. Dec. 96
|*
|*************************************************************************/
/*N*/ void SwCellFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
/*N*/ {
/*N*/ 	BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
/*N*/ 	const SfxPoolItem *pItem = 0;
/*N*/ 
/*N*/ 	if( bAttrSetChg )
/*N*/ 		((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, FALSE, &pItem);
/*N*/ 	else if ( RES_VERT_ORIENT == pNew->Which() )
/*?*/ 		pItem = pNew;
/*N*/ 
/*N*/ 	if ( pItem )
/*N*/ 	{
/*?*/ 		BOOL bInva = TRUE;
/*?*/ 		if ( VERT_NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() &&
/*?*/ 			 Lower()->IsCntntFrm() )
/*?*/ 		{
/*?*/             SWRECTFN( this )
/*?*/             const long lYStart = (this->*fnRect->fnGetPrtTop)();
/*?*/ 			bInva = lcl_ArrangeLowers( this, lYStart, FALSE );
/*?*/ 		}
/*?*/ 		if ( bInva )
/*?*/ 		{
/*?*/ 			SetCompletePaint();
/*?*/ 			InvalidatePrt();
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*M*/ 	if( (bAttrSetChg &&
/*M*/ 		 SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_PROTECT, FALSE )) ||
/*M*/ 		RES_PROTECT == pNew->Which() )
/*M*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*M*/ 	}
/*M*/ 
/*N*/ 	SwLayoutFrm::Modify( pOld, pNew );
/*N*/ }
}
