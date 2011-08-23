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


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <bf_svx/protitem.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif

#ifndef _PAGEFRM_HXX //autogen
#include <pagefrm.hxx>
#endif
#ifndef _FTNFRM_HXX //autogen
#include <ftnfrm.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _FLYFRM_HXX //autogen
#include <flyfrm.hxx>
#endif
#ifndef _TABFRM_HXX //autogen
#include <tabfrm.hxx>
#endif
#ifndef _TXTFRM_HXX //autogen
#include <txtfrm.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _CRSTATE_HXX //autogen
#include <crstate.hxx>
#endif
#ifndef _FRMTOOL_HXX //autogen
#include <frmtool.hxx>
#endif
#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
namespace binfilter {


//Fuer SwFlyFrm::GetCrsrOfst


/*************************************************************************
|*
|*	SwLayoutFrm::GetCrsrOfst()
|*
|*	Beschreibung:		Sucht denjenigen CntntFrm, innerhalb dessen
|* 						PrtArea der Point liegt.
|*	Ersterstellung		MA 20. Jul. 92
|*	Letzte Aenderung	MA 23. May. 95
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwPageFrm::GetCrsrOfst()
|*
|*	Beschreibung:		Sucht die Seite, innerhalb der der gesuchte Point
|*						liegt.
|*	Ersterstellung		MA 20. Jul. 92
|*	Letzte Aenderung	MA 18. Jul. 96
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwRootFrm::GetCrsrOfst()
|*
|*	Beschreibung:		Reicht Primaer den Aufruf an die erste Seite weiter.
|*						Wenn der 'reingereichte Point veraendert wird,
|*						so wird FALSE zurueckgegeben.
|*	Ersterstellung		MA 01. Jun. 92
|*	Letzte Aenderung	MA 30. Nov. 94
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwCellFrm::GetCrsrOfst()
|*
|*	Beschreibung		Wenn es sich um eine Cntnt-tragende Cell handelt wird
|* 						der Crsr notfalls mit Gewalt in einen der CntntFrms
|* 						gesetzt.
|* 						In geschuetzte Zellen gibt es hier keinen Eingang.
|*	Ersterstellung		MA 04. Jun. 93
|*	Letzte Aenderung	MA 23. May. 95
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwFlyFrm::GetCrsrOfst()
|*
|*	Ersterstellung		MA 15. Dec. 92
|*	Letzte Aenderung	MA 23. May. 95
|*
|*************************************************************************/
//Problem: Wenn zwei Flys genau gleich gross sind und auf derselben
//Position stehen, so liegt jeder innerhalb des anderen.
//Da jeweils geprueft wird, ob der Point nicht zufaellig innerhalb eines
//anderen Flys liegt, der sich vollstaendig innerhalb des aktuellen befindet
//und ggf. ein rekursiver Aufruf erfolgt wuerde o.g. Situation zu einer
//endlosen Rekursion fuehren.
//Mit der Hilfsklasse SwCrsrOszControl unterbinden wir die Rekursion. Das
//GetCrsrOfst entscheidet sich bei einer Rekursion fuer denjenigen der
//am weitesten oben liegt.


/*************************************************************************
|*
|*	  Beschreibung		Layoutabhaengiges Cursortravelling
|*	  Ersterstellung	MA 23. Jul. 92
|*	  Letzte Aenderung	MA 06. Sep. 93
|*
|*************************************************************************/


/*N*/ const SwCntntFrm *lcl_GetNxtCnt( const SwCntntFrm* pCnt )
/*N*/ {
/*N*/ 	return pCnt->GetNextCntntFrm();
/*N*/ }


/*N*/ typedef const SwCntntFrm *(*GetNxtPrvCnt)( const SwCntntFrm* );

//Frame in wiederholter Headline?
/*N*/ FASTBOOL lcl_IsInRepeatedHeadline( const SwFrm *pFrm,
/*N*/ 									const SwTabFrm** ppTFrm = 0 )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }


//Ueberspringen geschuetzter Tabellenzellen. Optional auch
//Ueberspringen von wiederholten Headlines.
//MA 26. Jan. 98: Chg auch andere Geschuetzte Bereiche ueberspringen.
/*N*/ const SwCntntFrm * MA_FASTCALL lcl_MissProtectedFrames( const SwCntntFrm *pCnt,
/*N*/ 													   GetNxtPrvCnt fnNxtPrv,
/*N*/ 													   FASTBOOL bMissHeadline,
/*N*/ 													   FASTBOOL bInReadOnly )
/*N*/ {
/*N*/ 	if ( pCnt && pCnt->IsInTab() )
/*N*/ 	{
/*N*/ 		BOOL bProtect = TRUE;
/*N*/ 		while ( pCnt && bProtect )
/*N*/ 		{
/*N*/ 			const SwLayoutFrm *pCell = pCnt->GetUpper();
/*N*/ 			while ( pCell && !pCell->IsCellFrm() )
/*?*/ 				pCell = pCell->GetUpper();
/*N*/ 			if ( !pCell ||
/*N*/ 					((bInReadOnly || !pCell->GetFmt()->GetProtect().IsCntntProtected()) &&
/*N*/ 					 (!bMissHeadline || !lcl_IsInRepeatedHeadline( pCell ) )))
/*N*/ 				bProtect = FALSE;
/*N*/ 			else
/*?*/ 				pCnt = (*fnNxtPrv)( pCnt );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( !bInReadOnly )
/*N*/ 		while ( pCnt && pCnt->IsProtected() )
/*N*/ 			pCnt = (*fnNxtPrv)( pCnt );
/*N*/ 
/*N*/ 	return pCnt;
/*N*/ }




/*************************************************************************
|*
|*	SwRootFrm::GetCurrPage()
|*
|*	Beschreibung:		Liefert die Nummer der aktuellen Seite.
|*			Wenn die Methode einen PaM bekommt, so ist die aktuelle Seite
|*			diejenige in der der PaM sitzt. Anderfalls ist die aktuelle
|*			Seite die erste Seite innerhalb der VisibleArea.
|*			Es wird nur auf den vorhandenen Seiten gearbeitet!
|*	Ersterstellung		MA 20. May. 92
|*	Letzte Aenderung	MA 09. Oct. 97
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwRootFrm::SetCurrPage()
|*
|*	Beschreibung:		Liefert einen PaM der am Anfang der gewuenschten
|*			Seite sitzt.
|*			Formatiert wird soweit notwendig
|*			Liefert Null, wenn die Operation nicht moeglich ist.
|*			Der PaM sitzt in der letzten Seite, wenn die Seitenzahl zu gross
|*			gewaehlt wurde.
|*	Ersterstellung		MA 20. May. 92
|*	Letzte Aenderung	MA 09. Oct. 97
|*
|*************************************************************************/

/*************************************************************************
|*
|*	  SwCntntFrm::StartxxPage(), EndxxPage()
|*
|*	  Beschreibung		Cursor an Anfang/Ende der aktuellen/vorherigen/
|*		naechsten Seite. Alle sechs Methoden rufen GetFrmInPage() mit der
|*		entsprechenden Parametrisierung.
|*		Zwei Parameter steuern die Richtung: einer bestimmt die Seite, der
|*		andere Anfang/Ende.
|*		Fuer die Bestimmung der Seite und des Cntnt (Anfang/Ende) werden
|*		die im folgenden definierten Funktionen benutzt.
|*	  Ersterstellung	MA 15. Oct. 92
|*	  Letzte Aenderung	MA 28. Feb. 93
|*
|*************************************************************************/





//Jetzt koennen auch die Funktionspointer initalisiert werden;
//sie sind in cshtyp.hxx declariert.

//Liefert den ersten/den letzten Contentframe (gesteuert ueber
//den Parameter fnPosPage) in der
//aktuellen/vorhergehenden/folgenden Seite (gesteuert durch den
//Parameter fnWhichPage).

/*************************************************************************
|*
|*	SwLayoutFrm::GetCntntPos()
|*
|*	Beschreibung		Es wird der nachstliegende Cntnt zum uebergebenen
|* 						gesucht. Betrachtet werden die vorhergehende, die
|* 						aktuelle und die folgende Seite.
|* 						Wenn kein Inhalt gefunden wird, so wird der Bereich
 * 						erweitert bis einer gefunden wird.
|* 						Zurueckgegeben wird die 'Semantisch richtige' Position
|* 						innerhalb der PrtArea des gefundenen CntntFrm
|*	Ersterstellung		MA 15. Jul. 92
|*	Letzte Aenderung	MA 09. Jan. 97
|*
|*************************************************************************/
/*N*/ ULONG CalcDiff( const Point &rPt1, const Point &rPt2 )
/*N*/ {
/*N*/ 	//Jetzt die Entfernung zwischen den beiden Punkten berechnen.
/*N*/ 	//'Delta' X^2 + 'Delta'Y^2 = 'Entfernung'^2
/*N*/ 	sal_uInt32 dX = Max( rPt1.X(), rPt2.X() ) -
/*N*/ 			   Min( rPt1.X(), rPt2.X() ),
/*N*/ 		  dY = Max( rPt1.Y(), rPt2.Y() ) -
/*N*/ 			   Min( rPt1.Y(), rPt2.Y() );
/*N*/ 	BigInt dX1( dX ), dY1( dY );
/*N*/ 	dX1 *= dX1; dY1 *= dY1;
/*N*/ 	return ::binfilter::SqRt( dX1 + dY1 );
/*N*/ }

// lcl_Inside ueberprueft, ob der Punkt innerhalb des Seitenteils liegt, in dem
// auch der CntntFrame liegt. Als Seitenteile gelten in diesem Zusammenhang
// Kopfzeile, Seitenbody, Fusszeile und FussnotenContainer.
// Dies dient dazu, dass ein CntntFrm, der im "richtigen" Seitenteil liegt,
// eher akzeptiert wird als ein anderer, der nicht dort liegt, auch wenn
// dessen Abstand zum Punkt geringer ist.

/*N*/ const SwLayoutFrm* lcl_Inside( const SwCntntFrm *pCnt, Point& rPt )
/*N*/ {
/*N*/ 	const SwLayoutFrm* pUp = pCnt->GetUpper();
/*N*/ 	while( pUp )
/*N*/ 	{
/*N*/ 		if( pUp->IsPageBodyFrm() || pUp->IsFooterFrm() || pUp->IsHeaderFrm() )
/*N*/ 		{
/*N*/ 			if( rPt.Y() >= pUp->Frm().Top() && rPt.Y() <= pUp->Frm().Bottom() )
/*?*/ 				return pUp;
/*N*/ 			return NULL;
/*N*/ 		}
/*N*/ 		if( pUp->IsFtnContFrm() )
/*?*/ 			return pUp->Frm().IsInside( rPt ) ? pUp : NULL;
/*N*/ 		pUp = pUp->GetUpper();
/*N*/ 	}
/*?*/ 	return NULL;
/*N*/ }

//Fuer MSC keine Optimierung mit e (enable register...) hier, sonst gibts
//einen Bug (ID: 2857)
/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize("e",off)
/*N*/ #endif

/*N*/ const SwCntntFrm *SwLayoutFrm::GetCntntPos( Point& rPoint,
/*N*/ 											const BOOL bDontLeave,
/*N*/ 											const BOOL bBodyOnly,
/*N*/ 											const BOOL bCalc,
/*N*/ 											const SwCrsrMoveState *pCMS,
/*N*/ 											const BOOL bDefaultExpand ) const
/*N*/ {
/*N*/ 	//Ersten CntntFrm ermitteln.
/*N*/ 	const SwLayoutFrm *pStart = (!bDontLeave && bDefaultExpand && GetPrev()) ?
/*N*/ 									(SwLayoutFrm*)GetPrev() : this;
/*N*/ 	const SwCntntFrm *pCntnt = pStart->ContainsCntnt();
/*N*/ 
/*N*/ 	if ( !pCntnt && (GetPrev() && !bDontLeave) )
/*N*/ 		pCntnt = ContainsCntnt();
/*N*/ 
/*N*/ 	if ( bBodyOnly && pCntnt && !pCntnt->IsInDocBody() )
/*N*/ 		while ( pCntnt && !pCntnt->IsInDocBody() )
/*N*/ 			pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 
/*N*/ 	const SwCntntFrm *pActual= pCntnt;
/*N*/ 	const SwLayoutFrm *pInside = NULL;
/*N*/ 	USHORT nMaxPage = GetPhyPageNum() + (bDefaultExpand ? 1 : 0);
/*N*/ 	Point aPoint = rPoint;
/*N*/ 	ULONG nDistance = ULONG_MAX;
/*N*/ 
/*N*/ 	while ( TRUE ) 	//Sicherheitsschleifchen, damit immer einer gefunden wird.
/*N*/ 	{
/*N*/ 		while ( pCntnt &&
/*N*/ 				((!bDontLeave || IsAnLower( pCntnt )) &&
/*N*/ 				(pCntnt->GetPhyPageNum() <= nMaxPage)) )
/*N*/ 		{
/*N*/ 			if ( ( bCalc || pCntnt->Frm().Width() ) &&
/*N*/ 				 ( !bBodyOnly || pCntnt->IsInDocBody() ) )
/*N*/ 			{
/*N*/ 				//Wenn der Cntnt in einem geschuetzen Bereich (Zelle, Ftn, Section)
/*N*/ 				//liegt, wird der nachste Cntnt der nicht geschuetzt ist gesucht.
/*N*/ 				const SwCntntFrm *pComp = pCntnt;
/*N*/ 				pCntnt = ::binfilter::lcl_MissProtectedFrames( pCntnt, lcl_GetNxtCnt, FALSE,
/*N*/ 										pCMS ? pCMS->bSetInReadOnly : FALSE );
/*N*/ 				if ( pComp != pCntnt )
/*N*/ 					continue;
/*N*/ 
/*N*/ 				if ( !pCntnt->IsTxtFrm() || !((SwTxtFrm*)pCntnt)->IsHiddenNow() )
/*N*/ 				{
/*N*/ 					if ( bCalc )
/*N*/ 						pCntnt->Calc();
/*N*/ 
/*N*/ 					SwRect aCntFrm( pCntnt->UnionFrm() );
/*N*/ 					if ( aCntFrm.IsInside( rPoint ) )
/*N*/ 					{
/*N*/ 						pActual = pCntnt;
/*N*/ 						aPoint = rPoint;
/*N*/ 						break;
/*N*/ 					}
/*N*/ 					//Die Strecke von rPoint zum dichtesten Punkt von pCntnt wird
/*N*/ 					//jetzt berechnet.
/*N*/ 					Point aCntntPoint( rPoint );
/*N*/ 
/*N*/ 					//Erst die Vertikale Position einstellen
/*N*/ 					if ( aCntFrm.Top() > aCntntPoint.Y() )
/*N*/ 						aCntntPoint.Y() = aCntFrm.Top();
/*N*/ 					else if ( aCntFrm.Bottom() < aCntntPoint.Y() )
/*N*/ 						aCntntPoint.Y() = aCntFrm.Bottom();
/*N*/ 
/*N*/ 					//Jetzt die Horizontale Position
/*N*/ 					if ( aCntFrm.Left() > aCntntPoint.X() )
/*N*/ 						aCntntPoint.X() = aCntFrm.Left();
/*N*/ 					else if ( aCntFrm.Right() < aCntntPoint.X() )
/*N*/ 						aCntntPoint.X() = aCntFrm.Right();
/*N*/ 
/*N*/ 					// pInside ist ein Seitenbereich, in dem der Punkt liegt,
/*N*/ 					// sobald pInside!=0 ist, werden nur noch Frames akzeptiert,
/*N*/ 					// die innerhalb liegen.
/*N*/ 					if( !pInside || ( pInside->IsAnLower( pCntnt ) &&
/*N*/ 						( !pCntnt->IsInFtn() || pInside->IsFtnContFrm() ) ) )
/*N*/ 					{
/*N*/ 						const ULONG nDiff = ::binfilter::CalcDiff( aCntntPoint, rPoint );
/*N*/ 						BOOL bBetter = nDiff < nDistance;  // Dichter dran
/*N*/ 						if( !pInside )
/*N*/ 						{
/*N*/ 							pInside = lcl_Inside( pCntnt, rPoint );
/*N*/ 							if( pInside )  // Im "richtigen" Seitenteil
/*N*/ 								bBetter = TRUE;
/*N*/ 						}
/*N*/ 						if( bBetter )
/*N*/ 						{
/*N*/ 							aPoint = aCntntPoint;
/*N*/ 							nDistance = nDiff;
/*N*/ 							pActual = pCntnt;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 			if ( bBodyOnly )
/*N*/ 				while ( pCntnt && !pCntnt->IsInDocBody() )
/*N*/ 					pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 		}
/*N*/ 		if ( !pActual )
/*N*/ 		{	//Wenn noch keiner gefunden wurde muss der Suchbereich erweitert
/*N*/ 			//werden, irgenwann muessen wir einen Finden!
/*N*/ 			//MA 09. Jan. 97: Opt fuer viele leere Seiten, wenn wir nur im
/*N*/ 			//Body suchen, koennen wir den Suchbereich gleich in einem
/*N*/ 			//Schritt hinreichend erweitern.
/*N*/ 			if ( bBodyOnly )
/*N*/ 			{
/*N*/ 				while ( !pCntnt && pStart->GetPrev() )
/*N*/ 				{
/*N*/ 					++nMaxPage;
/*N*/ 					if( !pStart->GetPrev()->IsLayoutFrm() )
/*N*/ 						return 0;
/*N*/ 					pStart = (SwLayoutFrm*)pStart->GetPrev();
/*N*/ 					pCntnt = pStart->IsInDocBody()
/*N*/ 								? pStart->ContainsCntnt()
/*N*/ 								: pStart->FindPageFrm()->FindFirstBodyCntnt();
/*N*/ 				}
/*N*/ 				if ( !pCntnt )	//irgendwann muessen wir mit irgendeinem Anfangen!
/*N*/ 				{
/*N*/ 					pCntnt = pStart->FindPageFrm()->GetUpper()->ContainsCntnt();
/*N*/ 					while ( pCntnt && !pCntnt->IsInDocBody() )
/*N*/ 						pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 					if ( !pCntnt )
/*N*/ 						return 0;	//Es gibt noch keine Dokumentinhalt!
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				++nMaxPage;
/*N*/ 				if ( pStart->GetPrev() )
/*N*/ 				{
/*N*/ 					if( !pStart->GetPrev()->IsLayoutFrm() )
/*N*/ 						return 0;
/*N*/ 					pStart = (SwLayoutFrm*)pStart->GetPrev();
/*N*/ 					pCntnt = pStart->ContainsCntnt();
/*N*/ 				}
/*N*/ 				else //irgendwann muessen wir mit irgendeinem Anfangen!
/*N*/ 					pCntnt = pStart->FindPageFrm()->GetUpper()->ContainsCntnt();
/*N*/ 			}
/*N*/ 			pActual = pCntnt;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	ASSERT( pActual, "Keinen Cntnt gefunden." );
/*N*/ 	if ( bBodyOnly )
/*N*/ 		ASSERT( pActual->IsInDocBody(), "Cnt nicht im Body." );
/*N*/ #endif
/*N*/ 
/*N*/ 	//Spezialfall fuer das selektieren von Tabellen, nicht in wiederholte
/*N*/ 	//TblHedlines.
/*N*/ 	if ( pActual->IsInTab() && pCMS && pCMS->eState == MV_TBLSEL )
/*N*/ 	{
/*N*/ 		const SwTabFrm *pTab = pActual->FindTabFrm();
/*N*/ 		if ( pTab->IsFollow() && pTab->GetTable()->IsHeadlineRepeat() &&
/*N*/ 			 ((SwLayoutFrm*)pTab->Lower())->IsAnLower( pActual ) )
/*N*/ 		{
/*N*/ 			((SwCrsrMoveState*)pCMS)->bStop = TRUE;
/*N*/ 			return 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Jetzt noch eine kleine Korrektur beim ersten/letzten
/*N*/ 	Size aActualSize( pActual->Prt().SSize() );
/*N*/ 	if ( aActualSize.Height() > pActual->GetUpper()->Prt().Height() )
/*N*/ 		aActualSize.Height() = pActual->GetUpper()->Prt().Height();
/*N*/ 
/*N*/     SWRECTFN( pActual )
/*N*/ 	if ( !pActual->GetPrev() &&
/*N*/ 		 (*fnRect->fnYDiff)( (pActual->*fnRect->fnGetPrtTop)(),
/*N*/                               bVert ? rPoint.X() : rPoint.Y() ) > 0 )
/*N*/     {
/*N*/         aPoint.Y() = pActual->Frm().Top() + pActual->Prt().Top();
/*N*/         aPoint.X() = pActual->Frm().Left() +
/*N*/                         ( pActual->IsRightToLeft() || bVert ?
/*N*/                           pActual->Prt().Right() :
/*N*/                           pActual->Prt().Left() );
/*N*/     }
/*N*/ 	else if ( !pActual->GetNext() &&
/*N*/               (*fnRect->fnYDiff)( (pActual->*fnRect->fnGetPrtBottom)(),
/*N*/                                    bVert ? rPoint.X() : rPoint.Y() ) < 0 )
/*N*/     {
/*N*/         aPoint.Y() = pActual->Frm().Top() + pActual->Prt().Bottom();
/*N*/         aPoint.X() = pActual->Frm().Left() +
/*N*/                         ( pActual->IsRightToLeft() || bVert ?
/*N*/                           pActual->Prt().Left() :
/*N*/                           pActual->Prt().Right() );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Und den Point in die PrtArea bringen
/*N*/ 	if ( bCalc )
/*N*/ 		pActual->Calc();
/*N*/ 	const SwRect aRect( pActual->Frm().Pos() + pActual->Prt().Pos(),
/*N*/ 						aActualSize );
/*N*/ 	if ( aPoint.Y() < aRect.Top() )
/*N*/ 		aPoint.Y() = aRect.Top();
/*N*/ 	else if ( aPoint.Y() > aRect.Bottom() )
/*N*/ 		aPoint.Y() = aRect.Bottom();
/*N*/ 	if ( aPoint.X() < aRect.Left() )
/*N*/ 		aPoint.X() = aRect.Left();
/*N*/ 	else if ( aPoint.X() > aRect.Right() )
/*N*/ 		aPoint.X() = aRect.Right();
/*N*/ 	rPoint = aPoint;
/*N*/ 	return pActual;
/*N*/ }

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize("",on)
/*N*/ #endif

/*************************************************************************
|*
|*	SwPageFrm::GetCntntPosition()
|*
|*	Beschreibung		Analog zu SwLayoutFrm::GetCntntPos().
|* 						Spezialisiert fuer Felder in Rahmen.
|*
|*	Ersterstellung		MA 22. Mar. 95
|*	Letzte Aenderung	MA 07. Nov. 95
|*
|*************************************************************************/
/*N*/ void SwPageFrm::GetCntntPosition( const Point &rPt, SwPosition &rPos ) const
/*N*/ {
/*N*/ 	//Ersten CntntFrm ermitteln.
/*N*/ 	const SwCntntFrm *pCntnt = ContainsCntnt();
/*N*/ 	if ( pCntnt )
/*N*/ 	{
/*N*/ 		//Einen weiter zurueck schauen (falls moeglich).
/*N*/ 		const SwCntntFrm *pTmp = pCntnt->GetPrevCntntFrm();
/*N*/ 		while ( pTmp && !pTmp->IsInDocBody() )
/*?*/ 			pTmp = pTmp->GetPrevCntntFrm();
/*N*/ 		if ( pTmp )
/*?*/ 			pCntnt = pTmp;
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pCntnt = GetUpper()->ContainsCntnt();
/*N*/ 
/*N*/ 	const SwCntntFrm *pAct = pCntnt;
/*N*/ 	Point aAct 		 = rPt;
/*N*/ 	ULONG nDist		 = ULONG_MAX;
/*N*/ 
/*N*/ 	while ( pCntnt )
/*N*/ 	{
/*N*/ 		SwRect aCntFrm( pCntnt->UnionFrm() );
/*N*/ 		if ( aCntFrm.IsInside( rPt ) )
/*N*/ 		{
/*N*/ 			//dichter gehts nimmer.
/*?*/ 			pAct = pCntnt;
/*?*/ 			break;
/*N*/ 		}
/*N*/ 
/*N*/ 		//Die Strecke von rPt zum dichtesten Punkt von pCntnt berechnen.
/*N*/ 		Point aPoint( rPt );
/*N*/ 
/*N*/ 		//Erst die vertikale Position einstellen
/*N*/ 		if ( aCntFrm.Top() > rPt.Y() )
/*N*/ 			aPoint.Y() = aCntFrm.Top();
/*N*/ 		else if ( aCntFrm.Bottom() < rPt.Y() )
/*N*/ 			aPoint.Y() = aCntFrm.Bottom();
/*N*/ 
/*N*/ 		//Jetzt die horizontale Position
/*N*/ 		if ( aCntFrm.Left() > rPt.X() )
/*N*/ 			aPoint.X() = aCntFrm.Left();
/*N*/ 		else if ( aCntFrm.Right() < rPt.X() )
/*N*/ 			aPoint.X() = aCntFrm.Right();
/*N*/ 
/*N*/ 		const ULONG nDiff = ::binfilter::CalcDiff( aPoint, rPt );
/*N*/ 		if ( nDiff < nDist )
/*N*/ 		{
/*N*/ 			aAct	= aPoint;
/*N*/ 			nDist	= nDiff;
/*N*/ 			pAct	= pCntnt;
/*N*/ 		}
/*N*/ 		else if ( aCntFrm.Top() > Frm().Bottom() )
/*N*/ 			//Dichter wirds im Sinne der Felder nicht mehr!
/*N*/ 			break;
/*N*/ 
/*N*/ 		pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 		while ( pCntnt && !pCntnt->IsInDocBody() )
/*N*/ 			pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 	}
/*N*/ 
/*N*/ 	//Und den Point in die PrtArea bringen
/*N*/ 	const SwRect aRect( pAct->Frm().Pos() + pAct->Prt().Pos(), pAct->Prt().SSize() );
/*N*/ 	if ( aAct.Y() < aRect.Top() )
/*N*/ 		aAct.Y() = aRect.Top();
/*N*/ 	else if ( aAct.Y() > aRect.Bottom() )
/*N*/ 		aAct.Y() = aRect.Bottom();
/*N*/ 	if ( aAct.X() < aRect.Left() )
/*?*/ 		aAct.X() = aRect.Left();
/*N*/ 	else if ( aAct.X() > aRect.Right() )
/*N*/ 		aAct.X() = aRect.Right();
/*N*/ 
/*N*/ 	if( !pAct->IsValid() )
/*N*/ 	{
/*N*/ 		// CntntFrm nicht formatiert -> immer auf Node-Anfang
/*?*/ 		SwCntntNode* pCNd = (SwCntntNode*)pAct->GetNode();
/*?*/ 		ASSERT( pCNd, "Wo ist mein CntntNode?" );
/*?*/ 		rPos.nNode = *pCNd;
/*?*/ 		rPos.nContent.Assign( pCNd, 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwCrsrMoveState aTmpState( MV_SETONLYTEXT );
/*N*/ 		pAct->GetCrsrOfst( &rPos, aAct, &aTmpState );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::GetNextPrevCntntPos()
|*
|*	Beschreibung		Es wird der naechstliegende Cntnt zum uebergebenen
|* 						Point gesucht. Es wird nur im BodyText gesucht.
|*	Ersterstellung		MA 15. Jul. 92
|*	Letzte Aenderung	JP 11.10.2001
|*
|*************************************************************************/

//!!!!! Es wird nur der vertikal naechstliegende gesucht.
//JP 11.10.2001: only in tables we try to find the right column - Bug 72294

/*************************************************************************
|*
|*	SwRootFrm::GetPagePos()
|*
|*	Beschreibung:	Liefert die absolute Dokumentpositon der gewuenschten
|*			Seite.
|*			Formatiert wird nur soweit notwendig und nur dann wenn bFormat=TRUE
|*			Liefert Null, wenn die Operation nicht moeglich ist.
|*			Die Pos ist die der letzten Seite, wenn die Seitenzahl zu gross
|*			gewaehlt wurde.
|*	Ersterstellung		MA 01. Jun. 92
|*	Letzte Aenderung	MA 09. Oct. 97
|*
|*************************************************************************/

/** get page frame by phyiscal page number

    OD 14.01.2003 #103492#

    @return pointer to the page frame with the given physical page number
*/

/*************************************************************************
|*
|*  SwRootFrm::IsDummyPage(USHORT)
|*
|*  Description: Returns TRUE, when the given physical pagenumber does't exist
|*               or this page is an empty page.
|*************************************************************************/


/*************************************************************************
|*
|*	  SwFrm::IsProtected()
|*
|*	  Beschreibung		Ist der Frm bzw. die Section in der er steht
|* 						geschuetzt?
|* 						Auch Fly in Fly in ... und Fussnoten
|*
|*	  Ersterstellung	MA 28. Jul. 93
|*	  Letzte Aenderung	MA 06. Nov. 97
|*
|*************************************************************************/
/*N*/ BOOL SwFrm::IsProtected() const
/*N*/ {
/*N*/ 	//Der Frm kann in Rahmen, Zellen oder Bereichen geschuetzt sein.
/*N*/ 	//Geht auch FlyFrms rekursiv hoch. Geht auch von Fussnoten zum Anker.
/*N*/ 	const SwFrm *pFrm = this;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		if ( pFrm->IsCntntFrm() )
/*N*/ 		{
/*N*/ 			if ( ((SwCntntFrm*)pFrm)->GetNode() &&
/*N*/                  ((SwCntntFrm*)pFrm)->GetNode()->IsInProtectSect() )
/*N*/ 				return TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/         {
/*N*/             if ( ((SwLayoutFrm*)pFrm)->GetFmt() &&
/*N*/                  ((SwLayoutFrm*)pFrm)->GetFmt()->
/*N*/                  GetProtect().IsCntntProtected() )
/*N*/ 			return TRUE;
/*N*/         }
/*N*/ 		if ( pFrm->IsFlyFrm() )
/*N*/ 		{
/*N*/ 			//Der Schutz des Inhaltes kann bei Verkettung vom Master der Kette
/*N*/ 			//vorgegeben werden.
/*N*/ 			if ( ((SwFlyFrm*)pFrm)->GetPrevLink() )
/*N*/ 			{
/*N*/ 				SwFlyFrm *pMaster = (SwFlyFrm*)pFrm;
/*N*/ 				do
/*N*/ 				{	pMaster = pMaster->GetPrevLink();
/*N*/ 				} while ( pMaster->GetPrevLink() );
/*N*/ 				if ( pMaster->IsProtected() )
/*N*/ 					return TRUE;
/*N*/ 			}
/*N*/ 			pFrm = ((SwFlyFrm*)pFrm)->GetAnchor();
/*N*/ 		}
/*N*/ 		else if ( pFrm->IsFtnFrm() )
/*N*/ 			pFrm = ((SwFtnFrm*)pFrm)->GetRef();
/*N*/ 		else
/*N*/ 			pFrm = pFrm->GetUpper();
/*N*/ 
/*N*/ 	} while ( pFrm );
/*N*/ 
/*N*/ 	return FALSE;
/*N*/ }

/*************************************************************************
|*
|*	  SwFrm::GetPhyPageNum()
|*	  Beschreibung:		Liefert die physikalische Seitennummer
|*
|*	  Ersterstellung	OK 06.07.93 08:35
|*	  Letzte Aenderung	MA 30. Nov. 94
|*
|*************************************************************************/
/*N*/ USHORT SwFrm::GetPhyPageNum() const
/*N*/ {
/*N*/ 	const SwPageFrm *pPage = FindPageFrm();
/*N*/ 	return pPage ? pPage->GetPhyPageNum() : 0;
/*N*/ }

/*-----------------26.02.01 11:25-------------------
 * SwFrm::WannaRightPage()
 * decides if the page want to be a rightpage or not.
 * If the first content of the page has a page descriptor,
 * we take the follow of the page descriptor of the last not empty page.
 * If this descriptor allows only right(left) pages and the page
 * isn't an empty page then it wanna be such right(left) page.
 * If the descriptor allows right and left pages, we look for a number offset
 * in the first content. If there is one, odd number results right pages,
 * even number results left pages.
 * If there is no number offset, we take the physical page number instead,
 * but a previous empty page don't count.
 * --------------------------------------------------*/

/*N*/ BOOL SwFrm::WannaRightPage() const
/*N*/ {
/*N*/ 	const SwPageFrm *pPage = FindPageFrm();
/*N*/ 	if ( !pPage || !pPage->GetUpper() )
/*?*/ 		return TRUE;
/*N*/ 
/*N*/ 	const SwFrm *pFlow = pPage->FindFirstBodyCntnt();
/*N*/ 	SwPageDesc *pDesc = 0;
/*N*/ 	USHORT nPgNum = 0;
/*N*/ 	if ( pFlow )
/*N*/ 	{
/*N*/ 		if ( pFlow->IsInTab() )
/*N*/ 			pFlow = pFlow->FindTabFrm();
/*N*/ 		const SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pFlow );
/*N*/ 		if ( !pTmp->IsFollow() )
/*N*/ 		{
/*N*/ 			const SwFmtPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
/*N*/ 			pDesc = (SwPageDesc*)rPgDesc.GetPageDesc();
/*N*/ 			nPgNum = rPgDesc.GetNumOffset();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !pDesc )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPrv = (SwPageFrm*)pPage->GetPrev();
/*N*/ 		if( pPrv && pPrv->IsEmptyPage() )
/*N*/ 			pPrv = (SwPageFrm*)pPrv->GetPrev();
/*N*/ 		if( pPrv )
/*N*/ 			pDesc = pPrv->GetPageDesc()->GetFollow();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwDoc* pDoc = pPage->GetFmt()->GetDoc();
/*N*/ 			pDesc = (SwPageDesc*)&pDoc->GetPageDesc( 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	ASSERT( pDesc, "No pagedescriptor" );
/*N*/ 	BOOL bOdd;
/*N*/ 	if( nPgNum )
/*N*/ 		bOdd = nPgNum % 2 ? TRUE : FALSE;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bOdd = pPage->OnRightPage();
/*N*/ 		if( pPage->GetPrev() && ((SwPageFrm*)pPage->GetPrev())->IsEmptyPage() )
/*N*/ 			bOdd = !bOdd;
/*N*/ 	}
/*N*/ 	if( !pPage->IsEmptyPage() )
/*N*/ 	{
/*N*/ 		if( !pDesc->GetRightFmt() )
/*N*/ 			bOdd = FALSE;
/*N*/ 		else if( !pDesc->GetLeftFmt() )
/*N*/ 			bOdd = TRUE;
/*N*/ 	}
/*N*/ 	return bOdd;
/*N*/ }

/*************************************************************************
|*
|*	  SwFrm::GetVirtPageNum()
|*	  Beschreibung:		Liefert die virtuelle Seitennummer mit Offset
|*
|*	  Ersterstellung	OK 06.07.93 08:35
|*	  Letzte Aenderung	MA 30. Nov. 94
|*
|*************************************************************************/
/*N*/ USHORT SwFrm::GetVirtPageNum() const
/*N*/ {
/*N*/ 	const SwPageFrm *pPage = FindPageFrm();
/*N*/ 	if ( !pPage || !pPage->GetUpper() )
/*?*/ 		return 0;
/*N*/ 
/*N*/ 	USHORT nPhyPage = pPage->GetPhyPageNum();
/*N*/ 	if ( !((SwRootFrm*)pPage->GetUpper())->IsVirtPageNum() )
/*N*/ 		return nPhyPage;
/*N*/ 
/*N*/ 	//Den am naechsten stehenden Absatz mit virtueller Seitennummer suchen.
/*N*/ 	//Da das rueckwaertsuchen insgesamt sehr viel Zeit verschlingt suchen
/*N*/ 	//wir jetzt gezielt ueber die Abhaengigkeiten.
/*N*/ 	//von den PageDescs bekommen wir die Attribute, von den Attributen
/*N*/ 	//wiederum bekommen wir die Absaetze.
/*N*/ 	const SwPageFrm *pVirtPage = 0;
/*N*/ 	const SwFrm *pFrm = 0;
/*N*/ 	const SfxItemPool &rPool = pPage->GetFmt()->GetDoc()->GetAttrPool();
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	USHORT nMaxItems = rPool.GetItemCount( RES_PAGEDESC );
/*N*/ 	for( USHORT n = 0; n < nMaxItems; ++n )
/*N*/ 	{
/*N*/ 		if( 0 == (pItem = rPool.GetItem( RES_PAGEDESC, n ) ))
/*N*/ 			continue;
/*N*/ 
/*N*/ 		const SwFmtPageDesc *pDesc = (SwFmtPageDesc*)pItem;
/*N*/ 		if ( pDesc->GetNumOffset() && pDesc->GetDefinedIn() )
/*N*/ 		{
/*N*/ 			const SwModify *pMod = pDesc->GetDefinedIn();
/*N*/ 			SwVirtPageNumInfo aInfo( pPage );
/*N*/ 			pMod->GetInfo( aInfo );
/*N*/ 			if ( aInfo.GetPage() )
/*N*/ 			{
/*N*/ 				if( !pVirtPage || ( pVirtPage && aInfo.GetPage()->
/*N*/ 					GetPhyPageNum() > pVirtPage->GetPhyPageNum() ) )
/*N*/ 				{
/*N*/ 					pVirtPage = aInfo.GetPage();
/*N*/ 					pFrm = aInfo.GetFrm();
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( pFrm )
/*N*/ 		return nPhyPage - pFrm->GetPhyPageNum() +
/*N*/ 			   pFrm->GetAttrSet()->GetPageDesc().GetNumOffset();
/*N*/ 	return nPhyPage;
/*N*/ }

/*************************************************************************
|*
|*	SwRootFrm::MakeTblCrsrs()
|*
|*	Ersterstellung		MA 14. May. 93
|*	Letzte Aenderung	MA 02. Feb. 94
|*
|*************************************************************************/
//Ermitteln und einstellen derjenigen Zellen die von der Selektion
//eingeschlossen sind.



/*************************************************************************
|*
|*	SwRootFrm::CalcFrmRects
|*
|*	Ersterstellung		MA 24. Aug. 92
|*	Letzte Aenderung	MA 24. Aug. 93
|*
|*************************************************************************/

/*
 * nun koennen folgende Situationen auftreten:
 *	1. Start und Ende liegen in einer Bildschirm - Zeile und im
 * 	   gleichen Node
 *		-> aus Start und End ein Rectangle, dann Ok
 *	2. Start und Ende liegen in einem Frame (dadurch im gleichen Node!)
 *		-> Start nach rechts, End nach links erweitern,
 *		   und bei mehr als 2 Bildschirm - Zeilen, das dazwischen
 *		   liegende berechnen
 * 	3. Start und Ende liegen in verschiedenen Frames
 *		-> Start nach rechts erweitern, bis Frame-Ende Rect berechnen
 *		   Ende nach links erweitern, bis Frame-Start Rect berechnen
 *		   und bei mehr als 2 Frames von allen dazwischen liegenden
 * 		   Frames die PrtArea dazu.
 *	4. Wenn es sich um eine Tabellenselektion handelt wird fuer jeden
 * 	   PaM im Ring der CellFrm besorgt, dessen PrtArea wird zu den
 * 	   Rechtecken addiert.
 *
 * Grosser Umbau wg. der FlyFrm; denn diese muessen ausgespart werden.
 * Ausnahmen: - Der Fly in dem die Selektion stattfindet (wenn sie in einem Fly
 *				stattfindet).
 * 			  - Die Flys, die vom Text unterlaufen werden.
 * Arbeitsweise: Zuerst wird eine SwRegion mit der Root initialisiert.
 * 				 Aus der Region werden die zu invertierenden Bereiche
 * 				 ausgestantzt. Die Region wird Komprimiert und letztlich
 * 				 invertiert. Damit liegen dann die zu invertierenden
 * 				 Rechtecke vor.
 * 				 Am Ende werden die Flys aus der Region ausgestanzt.
 */




}
