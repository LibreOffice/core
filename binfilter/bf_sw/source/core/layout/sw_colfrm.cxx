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

#include "cntfrm.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"

#include "hintids.hxx"

#ifndef _SVX_ULSPITEM_HXX //autogen
#include <bf_svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif

#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTFORDR_HXX //autogen
#include <fmtfordr.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#include "frmtool.hxx"
#include "colfrm.hxx"
#include "pagefrm.hxx"
#include "bodyfrm.hxx"	 // ColumnFrms jetzt mit BodyFrm
#include "rootfrm.hxx"   // wg. RemoveFtns
#include "sectfrm.hxx"	 // wg. FtnAtEnd-Flag
namespace binfilter {

// ftnfrm.cxx:
/*N*/ void lcl_RemoveFtns( SwFtnBossFrm* pBoss, BOOL bPageOnly, BOOL bEndNotes );


/*************************************************************************
|*
|*	SwColumnFrm::SwColumnFrm()
|*
|*	Ersterstellung		MA ??
|*	Letzte Aenderung	AMA 30. Oct 98
|*
|*************************************************************************/
/*N*/ SwColumnFrm::SwColumnFrm( SwFrmFmt *pFmt ):
/*N*/ 	SwFtnBossFrm( pFmt )
/*N*/ {
/*N*/     nType = FRMC_COLUMN;
/*N*/ 	SwBodyFrm* pColBody = new SwBodyFrm( pFmt->GetDoc()->GetDfltFrmFmt() );
/*N*/ 	pColBody->InsertBehind( this, 0 ); // ColumnFrms jetzt mit BodyFrm
/*N*/ 	SetMaxFtnHeight( LONG_MAX );
/*N*/ }

/*N*/ SwColumnFrm::~SwColumnFrm()
/*N*/ {
/*N*/ 	SwFrmFmt *pFmt = GetFmt();
/*N*/ 	SwDoc *pDoc;
/*N*/ 	if ( !(pDoc = pFmt->GetDoc())->IsInDtor() && pFmt->IsLastDepend() )
/*N*/ 	{
/*N*/ 		//Ich bin der einzige, weg mit dem Format.
/*N*/ 		//Vorher ummelden, damit die Basisklasse noch klarkommt.
/*N*/ 		pDoc->GetDfltFrmFmt()->Add( this );
/*N*/ 		pDoc->DelFrmFmt( pFmt );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::ChgColumns()
|*
|*	Ersterstellung		MA 11. Feb. 93
|*	Letzte Aenderung	MA 12. Oct. 98
|*
|*************************************************************************/

/*N*/ void MA_FASTCALL lcl_RemoveColumns( SwLayoutFrm *pCont, USHORT nCnt )
/*N*/ {
/*N*/ 	ASSERT( pCont && pCont->Lower() && pCont->Lower()->IsColumnFrm(),
/*N*/ 			"Keine Spalten zu entfernen." );
/*N*/ 
/*N*/ 	SwColumnFrm *pColumn = (SwColumnFrm*)pCont->Lower();
/*N*/ 	::binfilter::lcl_RemoveFtns( pColumn, TRUE, TRUE );
/*N*/ 	while ( pColumn->GetNext() )
/*N*/ 	{
/*N*/ 		ASSERT( pColumn->GetNext()->IsColumnFrm(),
/*N*/ 				"Nachbar von ColFrm kein ColFrm." );
/*N*/ 		pColumn = (SwColumnFrm*)pColumn->GetNext();
/*N*/ 	}
/*N*/ 	for ( USHORT i = 0; i < nCnt; ++i )
/*N*/ 	{
/*N*/ 		SwColumnFrm *pTmp = (SwColumnFrm*)pColumn->GetPrev();
/*N*/ 		pColumn->Cut();
/*N*/ 		delete pColumn;	//Format wird ggf. im DTor mit vernichtet.
/*N*/ 		pColumn = pTmp;
/*N*/ 	}
/*N*/ }

/*N*/ SwLayoutFrm * MA_FASTCALL lcl_FindColumns( SwLayoutFrm *pLay, USHORT nCount )
/*N*/ {
/*N*/ 	SwFrm *pCol = pLay->Lower();
/*N*/ 	if ( pLay->IsPageFrm() )
/*N*/ 		pCol = ((SwPageFrm*)pLay)->FindBodyCont()->Lower();
/*N*/ 
/*N*/ 	if ( pCol && pCol->IsColumnFrm() )
/*N*/ 	{
/*N*/ 		SwFrm *pTmp = pCol;
/*N*/ 		USHORT i;
/*N*/ 		for ( i = 0; pTmp; pTmp = pTmp->GetNext(), ++i )
/*N*/ 			/* do nothing */;
/*N*/ 		return i == nCount ? (SwLayoutFrm*)pCol : 0;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }


/*N*/ BOOL MA_FASTCALL lcl_AddColumns( SwLayoutFrm *pCont, USHORT nCount )
/*N*/ {
/*N*/ 	SwDoc *pDoc = pCont->GetFmt()->GetDoc();
/*N*/ 	const BOOL bMod = pDoc->IsModified();
/*N*/ 
/*N*/ 	//Format sollen soweit moeglich geshared werden. Wenn es also schon einen
/*N*/ 	//Nachbarn mit den selben Spalteneinstellungen gibt, so koennen die
/*N*/ 	//Spalten an die selben Formate gehaengt werden.
/*N*/ 	//Der Nachbar kann ueber das Format gesucht werden, wer der Owner des Attributes
/*N*/ 	//ist, ist allerdings vom Frametyp abhaengig.
/*N*/ 	SwLayoutFrm *pAttrOwner = pCont;
/*N*/ 	if ( pCont->IsBodyFrm() )
/*N*/ 		pAttrOwner = pCont->FindPageFrm();
/*N*/ 	SwLayoutFrm *pNeighbourCol = 0;
/*N*/ 	SwClientIter aIter( *pAttrOwner->GetFmt() );
/*N*/ 	SwLayoutFrm *pNeighbour = (SwLayoutFrm*)aIter.First( TYPE(SwLayoutFrm) );
/*N*/ 
/*N*/ 	USHORT nAdd = 0;
/*N*/ 	SwFrm *pCol = pCont->Lower();
/*N*/ 	if ( pCol && pCol->IsColumnFrm() )
/*?*/ 		for ( nAdd = 1; pCol; pCol = pCol->GetNext(), ++nAdd )
/*?*/ 			/* do nothing */;
/*N*/ 	while ( pNeighbour )
/*N*/ 	{
/*N*/ 		if ( 0 != (pNeighbourCol = lcl_FindColumns( pNeighbour, nCount+nAdd )) &&
/*N*/ 			 pNeighbourCol != pCont )
/*N*/ 			break;
/*N*/ 		pNeighbourCol = 0;
/*N*/ 		pNeighbour = (SwLayoutFrm*)aIter.Next();
/*N*/ 	}
/*N*/ 
/*N*/ 	BOOL bRet;
/*N*/ 	SwTwips nMax = pCont->IsPageBodyFrm() ?
/*N*/ 				   pCont->FindPageFrm()->GetMaxFtnHeight() : LONG_MAX;
/*N*/ 	if ( pNeighbourCol )
/*N*/ 	{
/*N*/ 		bRet = FALSE;
/*N*/ 		SwFrm *pTmp = pCont->Lower();
/*N*/ 		while ( pTmp )
/*N*/ 		{
/*?*/ 			pTmp = pTmp->GetNext();
/*?*/ 			pNeighbourCol = (SwLayoutFrm*)pNeighbourCol->GetNext();
/*N*/ 		}
/*N*/ 		for ( USHORT i = 0; i < nCount; ++i )
/*N*/ 		{
/*N*/ 			SwColumnFrm *pTmp = new SwColumnFrm( pNeighbourCol->GetFmt() );
/*N*/ 			pTmp->SetMaxFtnHeight( nMax );
/*N*/ 			pTmp->InsertBefore( pCont, NULL );
/*N*/ 			pNeighbourCol = (SwLayoutFrm*)pNeighbourCol->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bRet = TRUE;
/*N*/ 		for ( USHORT i = 0; i < nCount; ++i )
/*N*/ 		{
/*N*/ 			SwFrmFmt *pFmt = pDoc->MakeFrmFmt( aEmptyStr, pDoc->GetDfltFrmFmt());
/*N*/ 			SwColumnFrm *pTmp = new SwColumnFrm( pFmt );
/*N*/ 			pTmp->SetMaxFtnHeight( nMax );
/*N*/ 			pTmp->Paste( pCont );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bMod )
/*N*/ 		pDoc->ResetModified();
/*N*/ 	return bRet;
/*N*/ }

/*-----------------21.09.99 15:42-------------------
 * ChgColumns() adds or removes columns from a layoutframe.
 * Normally, a layoutframe with a column attribut of 1 or 0 columns contains
 * no columnframe. However, a sectionframe with "footnotes at the end" needs
 * a columnframe. If the bChgFtn-flag is set, the columnframe will be inserted
 * or remove, if necessary.
 * --------------------------------------------------*/

/*N*/ void SwLayoutFrm::ChgColumns( const SwFmtCol &rOld, const SwFmtCol &rNew,
/*N*/ 	const BOOL bChgFtn )
/*N*/ {
/*N*/ 	if ( rOld.GetNumCols() <= 1 && rNew.GetNumCols() <= 1 && !bChgFtn )
/*N*/ 		return;
/*N*/ 	USHORT nNewNum,	nOldNum = 1;
/*N*/ 	if( Lower() && Lower()->IsColumnFrm() )
/*N*/ 	{
/*N*/ 		SwFrm* pCol = Lower();
/*N*/ 		while( 0 != (pCol=pCol->GetNext()) )
/*N*/ 			++nOldNum;
/*N*/ 	}
/*N*/ 	nNewNum = rNew.GetNumCols();
/*N*/ 	if( !nNewNum )
/*N*/ 		++nNewNum;
/*N*/ 	BOOL bAtEnd;
/*N*/ 	if( IsSctFrm() )
/*?*/ 		bAtEnd = ((SwSectionFrm*)this)->IsAnyNoteAtEnd();
/*N*/ 	else
/*N*/ 		bAtEnd = FALSE;
/*N*/ 
/*N*/ 	//Einstellung der Spaltenbreiten ist nur bei neuen Formaten notwendig.
/*N*/ 	BOOL bAdjustAttributes = nOldNum != rOld.GetNumCols();
/*N*/ 
/*N*/ 	//Wenn die Spaltenanzahl unterschiedlich ist, wird der Inhalt
/*N*/ 	//gesichert und restored.
/*N*/ 	SwFrm *pSave = 0;
/*N*/ 	if( nOldNum != nNewNum || bChgFtn )
/*N*/ 	{
/*N*/ 		SwDoc *pDoc = GetFmt()->GetDoc();
/*N*/ 		ASSERT( pDoc, "FrmFmt gibt kein Dokument her." );
/*N*/ 		// SaveCntnt wuerde auch den Inhalt der Fussnotencontainer aufsaugen
/*N*/ 		// und im normalen Textfluss unterbringen.
/*N*/ 		if( IsPageBodyFrm() )
/*N*/ 			pDoc->GetRootFrm()->RemoveFtns( (SwPageFrm*)GetUpper(), TRUE, FALSE );
/*N*/ 		pSave = ::binfilter::SaveCntnt( this );
/*N*/ 
/*N*/ 		//Wenn Spalten existieren, jetzt aber eine Spaltenanzahl von
/*N*/ 		//0 oder eins gewuenscht ist, so werden die Spalten einfach vernichtet.
/*N*/ 		if ( nNewNum == 1 && !bAtEnd )
/*N*/ 		{
/*N*/ 			::binfilter::lcl_RemoveColumns( this, nOldNum );
/*N*/ 			if ( IsBodyFrm() )
/*N*/ 				SetFrmFmt( pDoc->GetDfltFrmFmt() );
/*N*/ 			else
/*?*/ 				GetFmt()->SetAttr( SwFmtFillOrder() );
/*N*/ 			if ( pSave )
/*N*/ 				::binfilter::RestoreCntnt( pSave, this, 0 );
/*N*/ 			return;
/*N*/ 		}
/*N*/ 		if ( nOldNum == 1 )
/*N*/ 		{
/*N*/ 			if ( IsBodyFrm() )
/*N*/ 				SetFrmFmt( pDoc->GetColumnContFmt() );
/*N*/ 			else
/*N*/ 				GetFmt()->SetAttr( SwFmtFillOrder( ATT_LEFT_TO_RIGHT ) );
/*N*/ 			if( !Lower() || !Lower()->IsColumnFrm() )
/*N*/ 				--nOldNum;
/*N*/ 		}
/*N*/ 		if ( nOldNum > nNewNum )
/*N*/ 		{
/*?*/ 			::binfilter::lcl_RemoveColumns( this, nOldNum - nNewNum );
/*?*/ 			bAdjustAttributes = TRUE;
/*N*/ 		}
/*N*/ 		else if( nOldNum < nNewNum )
/*N*/ 		{
/*N*/ 			USHORT nAdd = nNewNum - nOldNum;
/*N*/ 			bAdjustAttributes = ::binfilter::lcl_AddColumns( this, nAdd );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bAdjustAttributes )
/*N*/ 	{
/*N*/ 		if ( rOld.GetLineWidth() 	!= rNew.GetLineWidth() ||
/*N*/ 			 rOld.GetWishWidth()  	!= rNew.GetWishWidth() ||
/*N*/ 			 rOld.IsOrtho()			!= rNew.IsOrtho() )
/*N*/ 			bAdjustAttributes = TRUE;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			USHORT nCount = Min( rNew.GetColumns().Count(), rOld.GetColumns().Count() );
/*N*/ 			for ( USHORT i = 0; i < nCount; ++i )
/*?*/ 				if ( !(*rOld.GetColumns()[i] == *rNew.GetColumns()[i]) )
/*?*/ 				{
/*?*/ 					bAdjustAttributes = TRUE;
/*?*/ 					break;
/*?*/ 				}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Sodele, jetzt koennen die Spalten bequem eingestellt werden.
/*N*/ 	AdjustColumns( &rNew, bAdjustAttributes );
/*N*/ 
/*N*/ 	//Erst jetzt den Inhalt restaurieren. Ein frueheres Restaurieren wuerde
/*N*/ 	//unnuetzte Aktionen beim Einstellen zur Folge haben.
/*N*/ 	if ( pSave )
/*N*/ 	{
/*N*/ 		ASSERT( Lower() && Lower()->IsLayoutFrm() &&
/*N*/ 				((SwLayoutFrm*)Lower())->Lower() &&
/*N*/ 				((SwLayoutFrm*)Lower())->Lower()->IsLayoutFrm(),
/*N*/ 				"Gesucht: Spaltenbody (Tod oder Lebend)." );   // ColumnFrms jetzt mit BodyFrm
/*N*/ 		::binfilter::RestoreCntnt( pSave, (SwLayoutFrm*)((SwLayoutFrm*)Lower())->Lower(), 0 );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::AdjustColumns()
|*
|*	Ersterstellung		MA 19. Jan. 99
|*	Letzte Aenderung	MA 19. Jan. 99
|*
|*************************************************************************/

/*N*/ void SwLayoutFrm::AdjustColumns( const SwFmtCol *pAttr, BOOL bAdjustAttributes,
/*N*/                                  BOOL bAutoWidth )
/*N*/ {
/*N*/ 	if( !Lower()->GetNext() )
/*N*/ 	{
/*?*/ 		Lower()->ChgSize( Prt().SSize() );
/*?*/ 		return;
/*N*/ 	}
/*N*/ 
/*N*/     const FASTBOOL bVert = IsVertical();
/*N*/     SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
/*N*/ 
/*N*/ 	//Ist ein Pointer da, oder sollen wir die Attribute einstellen,
/*N*/ 	//so stellen wir auf jeden Fall die Spaltenbreiten ein. Andernfalls
/*N*/ 	//checken wir, ob eine Einstellung notwendig ist.
/*N*/ 	if ( !pAttr )
/*N*/ 	{
/*N*/ 		pAttr = &GetFmt()->GetCol();
/*N*/ 		if ( !bAdjustAttributes )
/*N*/ 		{
/*N*/             long nAvail = (Prt().*fnRect->fnGetWidth)();
/*N*/ 			for ( SwLayoutFrm *pCol = (SwLayoutFrm*)Lower();
/*N*/ 				  pCol;
/*N*/ 				  pCol = (SwLayoutFrm*)pCol->GetNext() )
/*N*/                 nAvail -= (pCol->Frm().*fnRect->fnGetWidth)();
/*N*/ 			if ( !nAvail )
/*N*/ 				return;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Sodele, jetzt koennen die Spalten bequem eingestellt werden.
/*N*/ 	//Die Breiten werden mitgezaehlt, damit wir dem letzten den Rest geben
/*N*/ 	//koennen.
/*N*/     SwTwips nAvail = (Prt().*fnRect->fnGetWidth)();
/*N*/     const BOOL bR2L = IsRightToLeft();
/*N*/     const BOOL bLine = pAttr->GetLineAdj() != COLADJ_NONE;
/*N*/ 	USHORT nMin = 0;
/*N*/ 	if ( bLine )
/*N*/ 		nMin = USHORT(20 + (pAttr->GetLineWidth() / 2));
/*N*/ 	SwFrm *pCol = Lower();
/*N*/     if( bR2L )
/*?*/         while( pCol->GetNext() )
/*?*/             pCol = pCol->GetNext();
/*N*/     long nGutter = 0;
/*N*/     BOOL bOrtho = bAutoWidth || ( pAttr->IsOrtho() && bAdjustAttributes &&
/*N*/                                   pAttr->GetNumCols() > 0 );
/*N*/     for ( USHORT i = 0; i < pAttr->GetNumCols();
/*N*/             pCol = bR2L ? pCol->GetPrev() : pCol->GetNext(), ++i )
/*N*/ 	{
/*N*/         if( !bOrtho )
/*N*/         {
/*N*/             const SwTwips nWidth = i == (pAttr->GetNumCols() - 1) ? nAvail :
/*N*/             pAttr->CalcColWidth( i, USHORT( (Prt().*fnRect->fnGetWidth)() ) );
/*N*/             Size aColSz = bVert ? Size( Prt().Width(), nWidth ) :
/*N*/                                   Size( nWidth, Prt().Height() );
/*N*/             pCol->ChgSize( aColSz );
/*N*/ 
/*N*/ 		// Hierdurch werden die ColumnBodyFrms von Seitenspalten angepasst und
/*N*/ 		// ihr bFixHeight-Flag wird gesetzt, damit sie nicht schrumpfen/wachsen.
/*N*/ 		// Bei Rahmenspalten hingegen soll das Flag _nicht_ gesetzt werden,
/*N*/ 		// da BodyFrms in Rahmenspalten durchaus wachsen/schrumpfen duerfen.
/*N*/             if( IsBodyFrm() )
/*N*/                 ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );
/*N*/ 
/*N*/             nAvail -= nWidth;
/*N*/         }
/*N*/ 
/*N*/         if ( bAutoWidth || bAdjustAttributes )
/*N*/ 		{
/*N*/ 			SwColumn *pC = pAttr->GetColumns()[i];
/*N*/ 			SwAttrSet* pSet = pCol->GetAttrSet();
/*N*/ 			SvxLRSpaceItem aLR( pSet->GetLRSpace() );
/*N*/ 			SvxULSpaceItem aUL( pSet->GetULSpace() );
/*N*/ 
/*N*/             {
/*N*/ 			//Damit die Trennlinien Platz finden, muessen sie hier
/*N*/ 			//Beruecksichtigung finden. Ueberall wo zwei Spalten aufeinanderstossen
/*N*/ 			//wird jeweils rechts bzw. links ein Sicherheitsabstand von 20 plus
/*N*/ 			//der halben Penbreite einkalkuliert.
/*N*/                 USHORT nRight, nLeft;
/*N*/                 if( sal_False && bR2L )
/*N*/                 {
/*?*/                     nRight = pC->GetLeft();
/*?*/                     nLeft = pC->GetRight();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     nLeft = pC->GetLeft();
/*N*/                     nRight = pC->GetRight();
/*N*/                 }
/*N*/                 if ( bLine )
/*N*/                 {
/*N*/                     if ( i == 0 )
/*N*/                     {   aLR.SetLeft ( nLeft );
/*N*/                         aLR.SetRight( Max(nRight, nMin) );
/*N*/                     }
/*N*/                     else if ( i == (pAttr->GetNumCols() - 1) )
/*N*/                     {   aLR.SetLeft ( Max(nLeft, nMin) );
/*N*/                         aLR.SetRight( nRight );
/*N*/                     }
/*N*/                     else
/*N*/                     {   aLR.SetLeft ( Max(nLeft,  nMin) );
/*N*/                         aLR.SetRight( Max(nRight, nMin) );
/*N*/                     }
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     aLR.SetLeft ( nLeft );
/*N*/                     aLR.SetRight( nRight);
/*N*/                 }
/*N*/                 aUL.SetUpper( pC->GetUpper());
/*N*/                 aUL.SetLower( pC->GetLower());
/*N*/             }
/*N*/ 
/*N*/             if ( bAdjustAttributes )
/*N*/             {
/*N*/                 ((SwLayoutFrm*)pCol)->GetFmt()->SetAttr( aLR );
/*N*/                 ((SwLayoutFrm*)pCol)->GetFmt()->SetAttr( aUL );
/*N*/             }
/*N*/ 
/*N*/             nGutter += aLR.GetLeft() + aLR.GetRight();
/*N*/ 		}
/*N*/     }
/*N*/     if( bOrtho )
/*N*/     {
/*N*/         nAvail = (Prt().*fnRect->fnGetWidth)();
/*N*/         long nInnerWidth = ( nAvail - nGutter )/ pAttr->GetNumCols();
/*N*/         pCol = Lower();
/*N*/         for( USHORT i = 0; i < pAttr->GetNumCols(); pCol = pCol->GetNext(), ++i)
/*N*/         {
/*N*/             SwTwips nWidth;
/*N*/             if( i == (pAttr->GetNumCols() - 1) )
/*N*/                 nWidth = nAvail;
/*N*/             else
/*N*/             {
/*N*/                 SvxLRSpaceItem aLR( pCol->GetAttrSet()->GetLRSpace() );
/*N*/                 nWidth = nInnerWidth + aLR.GetLeft() + aLR.GetRight();
/*N*/             }
/*N*/             if( nWidth < 0 )
/*?*/                 nWidth = 0;
/*N*/             Size aColSz = bVert ? Size( Prt().Width(), nWidth ) :
/*N*/                                   Size( nWidth, Prt().Height() );
/*N*/             pCol->ChgSize( aColSz );
/*N*/             if( IsBodyFrm() )
/*N*/                 ((SwLayoutFrm*)pCol)->Lower()->ChgSize( aColSz );
/*N*/             nAvail -= nWidth;
/*N*/         }
/*N*/     }
/*N*/ }





}
