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

#include <hintids.hxx>

#include <bf_svx/keepitem.hxx>
#include <bf_svx/hyznitem.hxx>

#include <pagefrm.hxx>		// ChangeFtnRef
#include <dflyobj.hxx>		// SwVirtFlyDrawObj

#include <horiornt.hxx>

#include <ftnfrm.hxx>		// SwFtnFrm
#include <paratr.hxx>
#include <viewopt.hxx>		// SwViewOptions
#include <viewsh.hxx>	  	// ViewShell
#include <frmatr.hxx>
#include <flyfrms.hxx>
#include <frmsh.hxx>
#include <txtcfg.hxx>
#include <itrform2.hxx> 	// SwTxtFormatter
#include <widorp.hxx>		// Widows and Orphans
#include <txtcache.hxx>
#include <sectfrm.hxx>		// SwSectionFrm

namespace binfilter {

extern FASTBOOL IsInProgress( const SwFlyFrm *pFly );

class FormatLevel
{
    static MSHORT nLevel;
public:
    inline FormatLevel()  { ++nLevel; }
    inline ~FormatLevel() { --nLevel; }
    inline MSHORT GetLevel() const { return nLevel; }
    static sal_Bool LastLevel() { return 10 < nLevel; }
};
MSHORT FormatLevel::nLevel = 0;

/*************************************************************************
 *							ValidateTxt/Frm()
 *************************************************************************/

/*N*/ void ValidateTxt( SwFrm *pFrm )		// Freund vom Frame
/*N*/ {
/*N*/     if ( ( ! pFrm->IsVertical() &&
/*N*/              pFrm->Frm().Width() == pFrm->GetUpper()->Prt().Width() ) ||
/*N*/              pFrm->IsVertical() &&
/*N*/              pFrm->Frm().Height() == pFrm->GetUpper()->Prt().Height() )
/*N*/ 		pFrm->bValidSize = sal_True;
/*
    pFrm->bValidPrtArea = sal_True;
    //Die Position validieren um nicht unnoetige (Test-)Moves zu provozieren.
    //Dabei darf allerdings nicht eine tatsaechlich falsche Coordinate
    //validiert werden.
    if ( !pFrm->bValidPos )
    {
        //Leider muessen wir dazu die korrekte Position berechnen.
        Point aOld( pFrm->Frm().Pos() );
        pFrm->MakePos();
        if ( aOld != pFrm->Pos() )
        {
            pFrm->Frm().Pos( aOld );
            pFrm->bValidPos = sal_False;
        }
    }
*/
/*N*/ }

/*N*/ void SwTxtFrm::ValidateFrm()
/*N*/ {
/*N*/ 	// Umgebung validieren, um Oszillationen zu verhindern.
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/ 
/*M*/ 	if ( !IsInFly() && !IsInTab() )
/*N*/ 	{	//Innerhalb eines Flys nur this validieren, der Rest sollte eigentlich
/*N*/ 		//nur fuer Fussnoten notwendig sein und die gibt es innerhalb von
/*N*/ 		//Flys nicht. Fix fuer 5544
/*N*/ 		SwSectionFrm* pSct = FindSctFrm();
/*N*/ 		if( pSct )
/*N*/ 		{
/*?*/ 			if( !pSct->IsColLocked() )
/*?*/ 				pSct->ColLock();
/*?*/ 			else
/*?*/ 				pSct = NULL;
/*N*/ 		}
/*N*/ 
/*N*/ 		SwFrm *pUp = GetUpper();
/*N*/ 		pUp->Calc();
/*N*/ 		if( pSct )
/*?*/ 			pSct->ColUnlock();
/*N*/ 	}
/*N*/ 	ValidateTxt( this );
/*N*/ 
/*N*/ 	//MA: mindestens das MustFit-Flag muessen wir retten!
/*N*/ 	ASSERT( HasPara(), "ResetPreps(), missing ParaPortion." );
/*N*/ 	SwParaPortion *pPara = GetPara();
/*N*/ 	const sal_Bool bMustFit = pPara->IsPrepMustFit();
/*N*/ 	ResetPreps();
/*N*/ 	pPara->SetPrepMustFit( bMustFit );
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ }

/*************************************************************************
 *							ValidateBodyFrm()
 *************************************************************************/

// nach einem RemoveFtn muss der BodyFrm und alle innenliegenden kalkuliert
// werden, damit die DeadLine richtig sitzt.
// Erst wird nach aussen hin gesucht, beim Rueckweg werden alle kalkuliert.

/*N*/ void _ValidateBodyFrm( SwFrm *pFrm )
/*N*/ {
/*N*/    if( pFrm && !pFrm->IsCellFrm() )
/*N*/ 	{
/*N*/         if( !pFrm->IsBodyFrm() && pFrm->GetUpper() )
/*N*/ 			_ValidateBodyFrm( pFrm->GetUpper() );
/*N*/ 		if( !pFrm->IsSctFrm() )
/*N*/ 			pFrm->Calc();
/*N*/ 		else
/*N*/ 		{
/*N*/ 			sal_Bool bOld = ((SwSectionFrm*)pFrm)->IsCntntLocked();
/*N*/ 			((SwSectionFrm*)pFrm)->SetCntntLock( sal_True );
/*N*/ 			pFrm->Calc();
/*N*/ 			if( !bOld )
/*N*/ 				((SwSectionFrm*)pFrm)->SetCntntLock( sal_False );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwTxtFrm::ValidateBodyFrm()
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/ 
/*N*/      //siehe Kommtar in ValidateFrm()
/*M*/     if ( !IsInFly() && !IsInTab() &&
/*M*/          !( IsInSct() && FindSctFrm()->Lower()->IsColumnFrm() ) )
/*N*/ 		_ValidateBodyFrm( GetUpper() );
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::FindBodyFrm()
 *************************************************************************/


/*************************************************************************
 *						SwTxtFrm::FindBodyFrm()
 *************************************************************************/

/*N*/ const SwBodyFrm *SwTxtFrm::FindBodyFrm() const
/*N*/ {
/*N*/ 	if ( IsInDocBody() )
/*N*/ 	{
/*N*/ 		const SwFrm *pFrm = GetUpper();
/*N*/ 		while( pFrm && !pFrm->IsBodyFrm() )
/*N*/ 			pFrm = pFrm->GetUpper();
/*N*/ 		return (const SwBodyFrm*)pFrm;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::CalcFollow()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrm::CalcFollow( const xub_StrLen nTxtOfst )
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/ 
/*N*/     ASSERT( HasFollow(), "CalcFollow: missing Follow." );
/*N*/ 
/*N*/ 	SwTxtFrm *pFollow = GetFollow();
/*N*/ 
/*N*/ 	SwParaPortion *pPara = GetPara();
/*N*/ 	sal_Bool bFollowFld = pPara ? pPara->IsFollowField() : sal_False;
/*N*/ 
/*N*/     if( !pFollow->GetOfst() || pFollow->GetOfst() != nTxtOfst ||
/*N*/         bFollowFld || pFollow->IsFieldFollow() ||
/*N*/         ( pFollow->IsVertical() && !pFollow->Prt().Width() ) ||
/*N*/         ( ! pFollow->IsVertical() && !pFollow->Prt().Height() ) )
/*N*/ 	{
/*N*/ #ifdef DBG_UTIL
/*N*/ 		const SwFrm *pOldUp = GetUpper();
/*N*/ #endif
/*N*/ 
/*N*/         SWRECTFN ( this )
/*N*/         SwTwips nOldBottom = (GetUpper()->Frm().*fnRect->fnGetBottom)();
/*N*/         SwTwips nMyPos = (Frm().*fnRect->fnGetTop)();
/*N*/ 
/*N*/ 		const SwPageFrm *pPage = 0;
/*N*/ 		sal_Bool  bOldInvaCntnt,
/*N*/ 			  bOldInvaLayout;
/*N*/ 		if ( !IsInFly() && GetNext() )
/*N*/ 		{
/*N*/ 			pPage = FindPageFrm();
/*N*/ 			//Minimieren - sprich ggf. zuruecksetzen - der Invalidierungen s.u.
/*N*/ 			bOldInvaCntnt  = pPage->IsInvalidCntnt();
/*N*/ 			bOldInvaLayout = pPage->IsInvalidLayout();
/*N*/ 		}
/*N*/ 
/*N*/ 		pFollow->_SetOfst( nTxtOfst );
/*N*/ 		pFollow->SetFieldFollow( bFollowFld );
/*N*/ 		if( HasFtn() || pFollow->HasFtn() )
/*N*/ 		{
/*?*/ 			ValidateFrm();
/*?*/ 			ValidateBodyFrm();
/*?*/ 			if( pPara )
/*?*/ 			{
/*?*/ 				*(pPara->GetReformat()) = SwCharRange();
/*?*/ 				*(pPara->GetDelta()) = 0;
/*?*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Der Fussnotenbereich darf sich keinesfalls vergrossern.
/*N*/ 		SwSaveFtnHeight aSave( FindFtnBossFrm( sal_True ), LONG_MAX );
/*N*/ 
/*N*/         pFollow->CalcFtnFlag();
/*N*/ 		if ( !pFollow->GetNext() && !pFollow->HasFtn() )
/*N*/             nOldBottom = bVert ? 0 : LONG_MAX;
/*N*/ 
/*N*/ 		while( sal_True )
/*N*/ 		{
/*N*/ 			if( !FormatLevel::LastLevel() )
/*N*/ 			{
/*N*/ 				// Weenn der Follow in einem spaltigen Bereich oder einem
/*N*/ 				// spaltigen Rahmen steckt, muss zunaechst dieser kalkuliert
/*N*/ 				// werden, da das FormatWidthCols() nicht funktioniert, wenn
/*N*/ 				// es aus dem MakeAll des _gelockten_ Follows heraus gerufen
/*N*/ 				// wird.
/*N*/ 				SwSectionFrm* pSct = pFollow->FindSctFrm();
/*N*/ 				if( pSct && !pSct->IsAnLower( this ) )
/*N*/ 				{
/*?*/ 					if( pSct->GetFollow() )
/*?*/ 						pSct->SimpleFormat();
/*?*/                     else if( ( pSct->IsVertical() && !pSct->Frm().Width() ) ||
/*?*/                              ( ! pSct->IsVertical() && !pSct->Frm().Height() ) )
/*?*/ 						break;
/*N*/ 				}
/*N*/                 // OD 14.03.2003 #i11760# - intrinsic format of follow is controlled.
/*N*/                 if ( FollowFormatAllowed() )
/*N*/                 {
/*N*/                     // OD 14.03.2003 #i11760# - no nested format of follows, if
/*N*/                     // text frame is contained in a column frame.
/*N*/                     // Thus, forbid intrinsic format of follow.
/*N*/                     {
/*N*/                         bool bIsFollowInColumn = false;
/*N*/                         SwFrm* pFollowUpper = pFollow->GetUpper();
/*N*/                         while ( pFollowUpper )
/*N*/                         {
/*N*/                             if ( pFollowUpper->IsColumnFrm() )
/*N*/                             {
/*N*/                                 bIsFollowInColumn = true;
/*N*/                                 break;
/*N*/                             }
/*N*/                             if ( pFollowUpper->IsPageFrm() ||
/*N*/                                  pFollowUpper->IsFlyFrm() )
/*N*/                             {
/*N*/                                 break;
/*N*/                             }
/*N*/                             pFollowUpper = pFollowUpper->GetUpper();
/*N*/                         }
/*N*/                         if ( bIsFollowInColumn )
/*N*/                         {
/*N*/                             pFollow->ForbidFollowFormat();
/*N*/                         }
/*N*/                     }
/*N*/ 
/*N*/ 				pFollow->Calc();
/*N*/ 				// Der Follow merkt anhand seiner Frm().Height(), dass was schief
/*N*/ 				// gelaufen ist.
/*N*/ 				ASSERT( !pFollow->GetPrev(), "SwTxtFrm::CalcFollow: cheesy follow" );
/*N*/ 				if( pFollow->GetPrev() )
/*N*/ 				{
/*?*/ 					pFollow->Prepare( PREP_CLEAR );
/*?*/ 					pFollow->Calc();
/*?*/ 					ASSERT( !pFollow->GetPrev(), "SwTxtFrm::CalcFollow: very cheesy follow" );
/*N*/                     }
/*N*/ 
/*N*/                     // OD 14.03.2003 #i11760# - reset control flag for follow format.
/*N*/                     pFollow->AllowFollowFormat();
/*N*/ 				}
/*N*/ 
/*N*/ 				//Sicherstellen, dass der Follow gepaintet wird.
/*N*/ 				pFollow->SetCompletePaint();
/*N*/ 			}
/*N*/ 
/*N*/ 			pPara = GetPara();
/*N*/ 			//Solange der Follow wg. Orphans Zeilen angefordert, bekommt er
/*N*/ 			//diese und wird erneut formatiert, falls moeglich.
/*N*/ 			if( pPara && pPara->IsPrepWidows() )
/*N*/ 				CalcPreps();
/*N*/ 			else
/*N*/ 				break;
/*N*/ 		}
/*N*/ 
/*N*/ 		if( HasFtn() || pFollow->HasFtn() )
/*N*/ 		{
/*?*/ 			ValidateBodyFrm();
/*?*/ 			ValidateFrm();
/*?*/ 			if( pPara )
/*?*/ 			{
/*?*/ 				*(pPara->GetReformat()) = SwCharRange();
/*?*/ 				*(pPara->GetDelta()) = 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pPage )
/*N*/ 		{
/*N*/ 			if ( !bOldInvaCntnt )
/*N*/ 				pPage->ValidateCntnt();
/*N*/ 			if ( !bOldInvaLayout && !IsInSct() )
/*N*/ 				pPage->ValidateLayout();
/*N*/ 		}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 		ASSERT( pOldUp == GetUpper(), "SwTxtFrm::CalcFollow: heavy follow" );
/*N*/ #endif
/*N*/ 
/*N*/         const long nRemaining =
/*N*/                  - (GetUpper()->Frm().*fnRect->fnBottomDist)( nOldBottom );
/*N*/         if (  nRemaining > 0 && !GetUpper()->IsSctFrm() &&
/*N*/               nRemaining != ( bVert ?
/*N*/                               nMyPos - Frm().Right() :
/*N*/                               Frm().Top() - nMyPos ) )
/*N*/         {
/*N*/             UNDO_SWAP( this )
/*N*/             return sal_True;
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ 
/*N*/     return sal_False;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::AdjustFrm()
 *************************************************************************/

/*N*/ void SwTxtFrm::AdjustFrm( const SwTwips nChgHght, sal_Bool bHasToFit )
/*N*/ {
/*N*/     if( IsUndersized() )
/*N*/ 	{
/*N*/ 		if( GetOfst() && !IsFollow() ) // ein gescrollter Absatz (undersized)
/*N*/ 			return;
/*N*/ 		SetUndersized( nChgHght == 0 || bHasToFit );
/*N*/ 	}
/*N*/ 
/*N*/     // AdjustFrm is called with a swapped frame during
/*N*/     // formatting but the frame is not swapped during FormatEmpty
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/     SWRECTFN ( this )
/*N*/ 
/*N*/     // Die Size-Variable des Frames wird durch Grow inkrementiert
/*N*/ 	// oder durch Shrink dekrementiert. Wenn die Groesse
/*N*/ 	// unveraendert ist, soll nichts passieren!
/*N*/ 	if( nChgHght >= 0)
/*N*/ 	{
/*N*/         SwTwips nChgHeight = nChgHght;
/*N*/ 		if( nChgHght && !bHasToFit )
/*N*/ 		{
/*N*/ 			if( IsInFtn() && !IsInSct() )
/*N*/ 			{
/*N*/                 SwTwips nReal = Grow( nChgHght PHEIGHT, sal_True );
/*N*/ 				if( nReal < nChgHght )
/*N*/ 				{
/*?*/                     SwTwips nBot = (*fnRect->fnYInc)( (Frm().*fnRect->fnGetBottom)(),
/*?*/                                                       nChgHght - nReal );
/*?*/ 					SwFrm* pCont = FindFtnFrm()->GetUpper();
/*?*/ 
/*?*/                     if( (pCont->Frm().*fnRect->fnBottomDist)( nBot ) > 0 )
/*?*/ 					{
/*?*/                         (Frm().*fnRect->fnAddBottom)( nChgHght );
/*?*/                         if( bVert )
/*?*/                             Prt().SSize().Width() += nChgHght;
/*?*/                         else
/*?*/ 						Prt().SSize().Height() += nChgHght;
/*?*/                         UNDO_SWAP( this )
/*?*/                         return;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/             Grow( nChgHght PHEIGHT );
/*N*/ 
/*N*/ 			if ( IsInFly() )
/*N*/ 			{
/*N*/ 				//MA 06. May. 93: Wenn einer der Upper ein Fly ist, so ist es
/*N*/ 				//sehr wahrscheinlich, dass dieser Fly durch das Grow seine
/*N*/ 				//Position veraendert - also muss auch meine Position korrigiert
/*N*/ 				//werden (sonst ist die Pruefung s.u. nicht aussagekraeftig).
/*N*/ 				//Die Vorgaenger muessen berechnet werden, damit die Position
/*N*/ 				//korrekt berechnet werden kann.
/*N*/ 				if ( GetPrev() )
/*N*/ 				{
/*N*/ 					SwFrm *pPre = GetUpper()->Lower();
/*N*/ 					do
/*N*/ 					{	pPre->Calc();
/*N*/ 						pPre = pPre->GetNext();
/*N*/ 					} while ( pPre && pPre != this );
/*N*/ 				}
/*N*/ 				const Point aOldPos( Frm().Pos() );
/*N*/ 				MakePos();
/*N*/ 				if ( aOldPos != Frm().Pos() )
/*N*/ 					CalcFlys( sal_True );	//#43679# Fly in Fly in ...
/*N*/ 			}
/*N*/             nChgHeight = 0;
/*N*/         }
/*N*/ 		// Ein Grow() wird von der Layout-Seite immer akzeptiert,
/*N*/ 		// also auch, wenn die FixSize des umgebenden Layoutframes
/*N*/ 		// dies nicht zulassen sollte. Wir ueberpruefen diesen
/*N*/ 		// Fall und korrigieren die Werte.
/*N*/ 		// MA 06. May. 93: Der Frm darf allerdings auch im Notfall nicht
/*N*/ 		// weiter geschrumpft werden als es seine Groesse zulaesst.
/*N*/         SwTwips nRstHeight;
/*N*/         if ( IsVertical() )
/*N*/         {
/*?*/             ASSERT( ! IsSwapped(),"Swapped frame while calculating nRstHeight" );
/*?*/             nRstHeight = Frm().Left() + Frm().Width() -
/*?*/                          ( GetUpper()->Frm().Left() + GetUpper()->Prt().Left() );
/*N*/         }
/*N*/         else
/*N*/             nRstHeight = GetUpper()->Frm().Top()
/*N*/                        + GetUpper()->Prt().Top()
/*N*/                        + GetUpper()->Prt().Height()
/*N*/                        - Frm().Top();
/*N*/ 
/*N*/ 		//In Tabellenzellen kann ich mir evtl. noch ein wenig dazuholen, weil
/*N*/ 		//durch eine vertikale Ausrichtung auch oben noch Raum sein kann.
/*N*/ 		if ( IsInTab() )
/*N*/ 		{
/*N*/             long nAdd = (*fnRect->fnYDiff)( (GetUpper()->Lower()->Frm().*fnRect->fnGetTop)(),
/*N*/                                             (GetUpper()->*fnRect->fnGetPrtTop)() );
/*N*/             ASSERT( nAdd >= 0, "Ey" );
/*N*/ 			nRstHeight += nAdd;
/*N*/ 		}
/*N*/ 
/* ------------------------------------
 * #50964#: nRstHeight < 0 bedeutet, dass der TxtFrm komplett ausserhalb seines
 * Upper liegt. Dies kann passieren, wenn er innerhalb eines FlyAtCntFrm liegt, der
 * durch das Grow() die Seite gewechselt hat. In so einem Fall ist es falsch, der
 * folgenden Grow-Versuch durchzufuehren. Im Bugfall fuehrte dies sogar zur
 * Endlosschleife.
 * -----------------------------------*/
/*N*/         SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/         SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/         if( nRstHeight < nFrmHeight )
/*N*/ 		{
/*N*/ 			//Kann sein, dass ich die richtige Grosse habe, der Upper aber zu
/*N*/ 			//klein ist und der Upper noch Platz schaffen kann.
/*N*/             if( ( nRstHeight >= 0 || ( IsInFtn() && IsInSct() ) ) && !bHasToFit )
/*N*/                 nRstHeight += GetUpper()->Grow( nFrmHeight - nRstHeight PHEIGHT );
/*N*/             // In spaltigen Bereichen wollen wir moeglichst nicht zu gross werden, damit
/*N*/ 			// nicht ueber GetNextSctLeaf weitere Bereiche angelegt werden. Stattdessen
/*N*/ 			// schrumpfen wir und notieren bUndersized, damit FormatWidthCols die richtige
/*N*/ 			// Spaltengroesse ermitteln kann.
/*N*/             if ( nRstHeight < nFrmHeight )
/*N*/ 			{
/*N*/ 				if(	bHasToFit || !IsMoveable() ||
/*N*/ 					( IsInSct() && !FindSctFrm()->MoveAllowed(this) ) )
/*N*/ 				{
/*N*/ 					SetUndersized( sal_True );
/*N*/                     Shrink( Min( ( nFrmHeight - nRstHeight), nPrtHeight )
/*N*/                                   PHEIGHT );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					SetUndersized( sal_False );
/*N*/ 			}
/*N*/ 		}
/*N*/         else if( nChgHeight )
/*N*/         {
/*?*/             if( nRstHeight - nFrmHeight < nChgHeight )
/*?*/                 nChgHeight = nRstHeight - nFrmHeight;
/*?*/             if( nChgHeight )
/*?*/                 Grow( nChgHeight );
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/         Shrink( -nChgHght PHEIGHT );
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::AdjustFollow()
 *************************************************************************/

/* AdjustFollow erwartet folgende Situation:
 * Der SwTxtIter steht am unteren Ende des Masters, der Offset wird
 * im Follow eingestellt.
 * nOffset haelt den Offset im Textstring, ab dem der Master abschliesst
 * und der Follow beginnt. Wenn er 0 ist, wird der FolgeFrame geloescht.
 */

/*N*/ void SwTxtFrm::_AdjustFollow( SwTxtFormatter &rLine,
/*N*/ 							 const xub_StrLen nOffset, const xub_StrLen nEnd,
/*N*/ 							 const sal_uInt8 nMode )
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/     // Wir haben den Rest der Textmasse: alle Follows loeschen
/*N*/ 	// Sonderfall sind DummyPortions()
/*N*/     // - special cases are controlled by parameter <nMode>.
/*N*/ 	if( HasFollow() && !(nMode & 1) && nOffset == nEnd )
/*N*/ 	{
/*N*/ 		while( GetFollow() )
/*N*/ 		{
/*N*/ 			if( ((SwTxtFrm*)GetFollow())->IsLocked() )
/*N*/ 			{
/*?*/ 				ASSERT( sal_False, "+SwTxtFrm::JoinFrm: Follow ist locked." );
/*?*/                 UNDO_SWAP( this )
/*?*/                 return;
/*N*/ 			}
/*N*/ 			JoinFrm();
/*N*/ 		}
/*N*/         UNDO_SWAP( this )
/*N*/         return;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Tanz auf dem Vulkan: Wir formatieren eben schnell noch einmal
/*N*/ 	// die letzte Zeile fuer das QuoVadis-Geraffel. Selbstverstaendlich
/*N*/ 	// kann sich dadurch auch der Offset verschieben:
/*N*/ 	const xub_StrLen nNewOfst = ( IsInFtn() && ( !GetIndNext() || HasFollow() ) ) ?
/*N*/ 							rLine.FormatQuoVadis(nOffset) : nOffset;
/*N*/ 
/*N*/ 	if( !(nMode & 1) )
/*N*/ 	{
/*N*/ 		// Wir klauen unseren Follows Textmasse, dabei kann es passieren,
/*N*/ 		// dass wir einige Follows Joinen muessen.
/*N*/ 		while( GetFollow() && GetFollow()->GetFollow() &&
/*N*/ 			   nNewOfst >= GetFollow()->GetFollow()->GetOfst() )
/*N*/ 		{
///*?*/ 			DBG_LOOP;
/*?*/ 			JoinFrm();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Der Ofst hat sich verschoben.
/*N*/ 	if( GetFollow() )
/*N*/ 	{
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 		static sal_Bool bTest = sal_False;
/*N*/ 		if( !bTest || ( nMode & 1 ) )
/*N*/ #endif
/*N*/ 		if ( nMode )
/*N*/ 			GetFollow()->ManipOfst( 0 );
/*N*/ 
/*N*/ 		if ( CalcFollow( nNewOfst ) )	// CalcFollow erst zum Schluss, dort erfolgt ein SetOfst
/*N*/ 			rLine.SetOnceMore( sal_True );
/*N*/ 	}
/*N*/     UNDO_SWAP( this )
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::JoinFrm()
 *************************************************************************/

/*N*/ SwCntntFrm *SwTxtFrm::JoinFrm()
/*N*/ {
/*N*/ 	ASSERT( GetFollow(), "+SwTxtFrm::JoinFrm: no follow" );
/*N*/ 	SwTxtFrm  *pFoll = GetFollow();
/*N*/ 
/*N*/ 	SwTxtFrm *pNxt = pFoll->GetFollow();
/*N*/ 
/*N*/ 	// Alle Fussnoten des zu zerstoerenden Follows werden auf uns
/*N*/ 	// umgehaengt.
/*N*/ 	xub_StrLen nStart = pFoll->GetOfst();
/*N*/ 	if ( pFoll->HasFtn() )
/*N*/ 	{
/*?*/ 		const SwpHints *pHints = pFoll->GetTxtNode()->GetpSwpHints();
/*?*/ 		if( pHints )
/*?*/ 		{
/*?*/ 			SwFtnBossFrm *pFtnBoss = 0;
/*?*/ 			SwFtnBossFrm *pEndBoss = 0;
/*?*/ 			for( MSHORT i = 0; i < pHints->Count(); ++i )
/*?*/ 			{
/*?*/ 				const SwTxtAttr *pHt = (*pHints)[i];
/*?*/ 				if( RES_TXTATR_FTN==pHt->Which() && *pHt->GetStart()>=nStart )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( pHt->GetFtn().IsEndNote() )
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/     else if ( pFoll->GetValidPrtAreaFlag() ||
/*N*/               pFoll->GetValidSizeFlag() )
/*N*/     {
/*N*/ 		pFoll->CalcFtnFlag();
/*N*/ 		ASSERT( !pFoll->HasFtn(), "Missing FtnFlag." );
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	pFoll->MoveFlyInCnt( this, nStart, STRING_LEN );
/*N*/     pFoll->SetFtn( FALSE );
/*N*/ 	pFoll->Cut();
/*N*/ 	delete pFoll;
/*N*/ 	pFollow = pNxt;
/*N*/ 	return pNxt;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::SplitFrm()
 *************************************************************************/

/*N*/ SwCntntFrm *SwTxtFrm::SplitFrm( const xub_StrLen nTxtPos )
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( this )
/*N*/ 
/*N*/ 	// Durch das Paste wird ein Modify() an mich verschickt.
/*N*/ 	// Damit meine Daten nicht verschwinden, locke ich mich.
/*N*/ 	SwTxtFrmLocker aLock( this );
/*N*/ 	SwTxtFrm *pNew = (SwTxtFrm *)(GetTxtNode()->MakeFrm());
/*N*/ 	pNew->bIsFollow = sal_True;
/*N*/ 
/*N*/ 	pNew->SetFollow( GetFollow() );
/*N*/ 	SetFollow( pNew );
/*N*/ 
/*N*/ 	pNew->Paste( GetUpper(), GetNext() );
/*N*/ 
/*N*/ 	// Wenn durch unsere Aktionen Fussnoten in pNew landen,
/*N*/ 	// so muessen sie umgemeldet werden.
/*N*/ 	if ( HasFtn() )
/*N*/ 	{
/*?*/ 		const SwpHints *pHints = GetTxtNode()->GetpSwpHints();
/*?*/ 		if( pHints )
/*?*/ 		{
/*?*/ 			SwFtnBossFrm *pFtnBoss = 0;
/*?*/ 			SwFtnBossFrm *pEndBoss = 0;
/*?*/ 			for( MSHORT i = 0; i < pHints->Count(); ++i )
/*?*/ 			{
/*?*/ 				const SwTxtAttr *pHt = (*pHints)[i];
/*?*/ 				if( RES_TXTATR_FTN==pHt->Which() && *pHt->GetStart()>=nTxtPos )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( pHt->GetFtn().IsEndNote() )
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	else
/*N*/ 	{
/*N*/ 		CalcFtnFlag( nTxtPos-1 );
/*N*/ 		ASSERT( !HasFtn(), "Missing FtnFlag." );
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	MoveFlyInCnt( pNew, nTxtPos, STRING_LEN );
/*N*/ 
/*N*/ 	// Kein SetOfst oder CalcFollow, weil gleich ohnehin ein AdjustFollow folgt.
/*N*/ #ifdef USED
/*N*/ 	CalcFollow( nNewOfst );
/*N*/ #endif
/*N*/ 
/*N*/ 	pNew->ManipOfst( nTxtPos );
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ 	return pNew;
/*N*/ }


/*************************************************************************
 *						virtual SwTxtFrm::SetOfst()
 *************************************************************************/

/*N*/ void SwTxtFrm::_SetOfst( const xub_StrLen nNewOfst )
/*N*/ {
/*N*/ #ifdef DBGTXT
/*N*/ 	// Es gibt tatsaechlich einen Sonderfall, in dem ein SetOfst(0)
/*N*/ 	// zulaessig ist: bug 3496
/*N*/ 	ASSERT( nNewOfst, "!SwTxtFrm::SetOfst: missing JoinFrm()." );
/*N*/ #endif
/*N*/ 
/*N*/ 	// Die Invalidierung unseres Follows ist nicht noetig.
/*N*/ 	// Wir sind ein Follow, werden gleich formatiert und
/*N*/ 	// rufen von dort aus das SetOfst() !
/*N*/ 	nOfst = nNewOfst;
/*N*/ 	SwParaPortion *pPara = GetPara();
/*N*/ 	if( pPara )
/*N*/ 	{
/*N*/ 		SwCharRange &rReformat = *(pPara->GetReformat());
/*N*/ 		rReformat.Start() = 0;
/*N*/ 		rReformat.Len() = GetTxt().Len();
/*N*/ 		*(pPara->GetDelta()) = rReformat.Len();
/*N*/ 	}
/*N*/ 	InvalidateSize();
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::CalcPreps
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrm::CalcPreps()
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(), "SwTxtFrm::CalcPreps with swapped frame" );
/*N*/     SWRECTFN( this );
/*N*/ 
/*N*/     SwParaPortion *pPara = GetPara();
/*N*/ 	if ( !pPara )
/*N*/ 		return sal_False;
/*N*/ 	sal_Bool bPrep = pPara->IsPrep();
/*N*/ 	sal_Bool bPrepWidows = pPara->IsPrepWidows();
/*N*/ 	sal_Bool bPrepAdjust = pPara->IsPrepAdjust();
/*N*/ 	sal_Bool bPrepMustFit = pPara->IsPrepMustFit();
/*N*/ 	ResetPreps();
/*N*/ 
/*N*/ 	sal_Bool bRet = sal_False;
/*N*/ 	if( bPrep && !pPara->GetReformat()->Len() )
/*N*/ 	{
/*N*/ 		// PREP_WIDOWS bedeutet, dass im Follow die Orphans-Regel
/*N*/ 		// zuschlug.
/*N*/ 		// Es kann in unguenstigen Faellen vorkommen, dass auch ein
/*N*/ 		// PrepAdjust vorliegt (3680)!
/*N*/ 		if( bPrepWidows )
/*N*/ 		{
/*N*/ 			if( !GetFollow() )
/*N*/ 			{
/*?*/ 				ASSERT( GetFollow(), "+SwTxtFrm::CalcPreps: no credits" );
/*?*/ 				return sal_False;
/*N*/ 			}
/*N*/ 
/*N*/ 			// Wir muessen uns auf zwei Faelle einstellen:
/*N*/ 			// Wir konnten dem Follow noch ein paar Zeilen abgeben,
/*N*/ 			// -> dann muessen wir schrumpfen
/*N*/ 			// oder wir muessen auf die naechste Seite
/*N*/ 			// -> dann lassen wir unseren Frame zu gross werden.
/*N*/ 
/*N*/ 			SwTwips nChgHeight = GetParHeight();
/*N*/             if( nChgHeight >= (Prt().*fnRect->fnGetHeight)() )
/*N*/ 			{
/*N*/ 				if( bPrepMustFit )
/*N*/ 				{
/*?*/ 					GetFollow()->SetJustWidow( sal_True );
/*?*/ 					GetFollow()->Prepare( PREP_CLEAR );
/*N*/ 				}
/*N*/                 else if ( bVert )
/*N*/ 				{
/*?*/                     Frm().Width( Frm().Width() + Frm().Left() );
/*?*/                     Prt().Width( Prt().Width() + Frm().Left() );
/*?*/                     Frm().Left( 0 );
/*?*/ 					SetWidow( sal_True );
/*N*/ 				}
/*N*/                 else
/*N*/ 				{
/*N*/ 					SwTwips nTmp  = LONG_MAX - (Frm().Top()+10000);
/*N*/ 					SwTwips nDiff = nTmp - Frm().Height();
/*N*/ 					Frm().Height( nTmp );
/*N*/ 					Prt().Height( Prt().Height() + nDiff );
/*N*/ 					SetWidow( sal_True );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*?*/                 ASSERT( nChgHeight < (Prt().*fnRect->fnGetHeight)(),
/*?*/ 						"+SwTxtFrm::CalcPrep: wanna shrink" );
/*?*/ 
/*?*/                 nChgHeight = (Prt().*fnRect->fnGetHeight)() - nChgHeight;
/*?*/ 
/*?*/ 				GetFollow()->SetJustWidow( sal_True );
/*?*/ 				GetFollow()->Prepare( PREP_CLEAR );
/*?*/                 Shrink( nChgHeight PHEIGHT );
/*?*/ 				SwRect &rRepaint = *(pPara->GetRepaint());
/*?*/ 
/*?*/                 if ( bVert )
/*?*/                 {
                        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/                     SwRect aRepaint( Frm().Pos() + Prt().Pos(), Prt().SSize() );
/*?*/                 }
/*?*/                 else
/*?*/                     rRepaint.Chg( Frm().Pos() + Prt().Pos(), Prt().SSize() );
/*?*/ 
/*?*/ 				// 6792: Rrand < LRand und Repaint
/*?*/                 if( 0 >= rRepaint.Width() )
/*?*/ 					rRepaint.Width(1);
/*N*/ 			}
/*N*/ 			bRet = sal_True;
/*N*/ 		}
/*N*/ 
/*N*/ 		else if ( bPrepAdjust )
/*N*/ 		{
/*N*/ 			if ( HasFtn() )
/*N*/ 			{
/*N*/ 				if( !CalcPrepFtnAdjust() )
/*N*/ 				{
/*?*/ 					if( bPrepMustFit )
/*?*/ 					{
/*?*/ 						SwTxtLineAccess aAccess( this );
/*?*/ 						aAccess.GetPara()->SetPrepMustFit( sal_True );
/*?*/ 					}
/*?*/ 					return sal_False;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/     SWAP_IF_NOT_SWAPPED( this )
/*N*/ 
/*N*/ 			SwTxtFormatInfo aInf( this );
/*N*/ 			SwTxtFormatter aLine( this, &aInf );
/*N*/ 
/*N*/ 			WidowsAndOrphans aFrmBreak( this );
/*N*/ 			// Egal was die Attribute meinen, bei MustFit wird
/*N*/ 			// der Absatz im Notfall trotzdem gesplittet...
/*N*/ 			if( bPrepMustFit )
/*N*/ 			{
/*?*/ 				aFrmBreak.SetKeep( sal_False );
/*?*/ 				aFrmBreak.ClrOrphLines();
/*N*/ 			}
/*N*/ 			// Bevor wir FormatAdjust aufrufen muessen wir dafuer
/*N*/ 			// sorgen, dass die Zeilen, die unten raushaengen
/*N*/ 			// auch tatsaechlich abgeschnitten werden.
/*N*/ 			sal_Bool bBreak = aFrmBreak.IsBreakNow( aLine );
/*N*/ 			bRet = sal_True;
/*N*/ 			while( !bBreak && aLine.Next() )
/*N*/ 				bBreak = aFrmBreak.IsBreakNow( aLine );
/*N*/ 			if( bBreak )
/*N*/ 			{
/*N*/ 				// Es gibt Komplikationen: wenn TruncLines gerufen wird,
/*N*/ 				// veraendern sich ploetzlich die Bedingungen in
/*N*/ 				// IsInside, so dass IsBreakNow andere Ergebnisse
/*N*/ 				// liefern kann. Aus diesem Grund wird rFrmBreak bekannt
/*N*/ 				// gegeben, dass da wo rLine steht, das Ende erreicht
/*N*/ 				// ist. Mal sehen, ob's klappt ...
/*N*/ 				aLine.TruncLines();
/*N*/ 				aFrmBreak.SetRstHeight( aLine );
/*N*/ 				FormatAdjust( aLine, aFrmBreak, aInf.GetTxt().Len(), aInf.IsStop() );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( !GetFollow() )
/*N*/ 				{
/*N*/ 					FormatAdjust( aLine, aFrmBreak,
/*N*/ 								  aInf.GetTxt().Len(), aInf.IsStop() );
/*N*/ 				}
/*N*/ 				else if ( !aFrmBreak.IsKeepAlways() )
/*N*/ 				{
/*N*/ 					// Siehe Bug: 2320
/*N*/ 					// Vor dem Master wird eine Zeile geloescht, der Follow
/*N*/ 					// koennte eine Zeile abgeben.
/*N*/ 					const SwCharRange aFollowRg( GetFollow()->GetOfst(), 1 );
/*N*/ 					*(pPara->GetReformat()) += aFollowRg;
/*N*/ 					// Es soll weitergehen!
/*N*/ 					bRet = sal_False;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/             UNDO_SWAP( this )
/*N*/             // Eine letzte Ueberpruefung, falls das FormatAdjust() nichts
/*N*/ 			// brachte, muessen wir amputieren.
/*N*/ 			if( bPrepMustFit )
/*N*/ 			{
/*?*/                 const SwTwips nMust = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*?*/                 const SwTwips nIs   = (Frm().*fnRect->fnGetBottom)();
/*?*/ 
/*?*/                 if( bVert && nIs < nMust )
/*?*/                 {
/*?*/                     Shrink( nMust - nIs );
/*?*/                     if( Prt().Width() < 0 )
/*?*/                         Prt().Width( 0 );
/*?*/ 					SetUndersized( sal_True );
/*?*/                 }
/*?*/                 else if ( ! bVert && nIs > nMust )
/*?*/                 {
/*?*/                     Shrink( nIs - nMust );
/*?*/ 					if( Prt().Height() < 0 )
/*?*/ 						Prt().Height( 0 );
/*?*/ 					SetUndersized( sal_True );
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	pPara->SetPrepMustFit( bPrepMustFit );
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::FormatAdjust()
 *************************************************************************/

// Hier werden die Fussnoten und "als Zeichen"-gebundenen Objekte umgehaengt
#define CHG_OFFSET( pFrm, nNew )\
    {\
        if( pFrm->GetOfst() < nNew )\
            pFrm->MoveFlyInCnt( this, 0, nNew );\
        else if( pFrm->GetOfst() > nNew )\
            MoveFlyInCnt( pFrm, nNew, STRING_LEN );\
    }

/*N*/ void SwTxtFrm::FormatAdjust( SwTxtFormatter &rLine,
/*N*/ 							 WidowsAndOrphans &rFrmBreak,
/*N*/                              const xub_StrLen nStrLen,
/*N*/                              const sal_Bool bDummy )
/*N*/ {
/*N*/     SWAP_IF_NOT_SWAPPED( this )
/*N*/ 
/*N*/     SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
/*N*/ 
/*N*/ 	xub_StrLen nEnd = rLine.GetStart();
/*N*/ 
/*N*/ 	// Wir muessen fuer eindeutige Verhaeltnisse sorgen
/*N*/ 	// rFrmBreak.SetRstHeight( rLine );
/*N*/ 
/*N*/ 	// rLine.GetStart(): die letzte Zeile von rLine,
/*N*/ 	// ist bereits die Zeile, die nicht
/*N*/ 	// mehr passte. Ihr Anfang ist das Ende des Masters.
/*N*/ 	// @@@if( !GetFollow() && nEnd < nStrLen )
/*N*/ 	// (nEnd < nStrLen || rFrmBreak.IsBreakNow(rLine));
/*N*/ 
/*N*/ 	sal_Bool bHasToFit = pPara->IsPrepMustFit();
/*N*/ 
/*N*/ 	// Das StopFlag wird durch Fussnoten gesetzt,
/*N*/ 	// die auf die naechste Seite wollen.
/*N*/ 	sal_uInt8 nNew = ( !GetFollow() && nEnd < nStrLen &&
/*N*/ 		( rLine.IsStop() || ( bHasToFit ?
/*N*/ 		( rLine.GetLineNr() > 1 && !rFrmBreak.IsInside( rLine ) )
/*N*/ 		: rFrmBreak.IsBreakNow( rLine ) ) ) ) ? 1 : 0;
/*N*/ 	if( nNew )
/*N*/ 		SplitFrm( nEnd );
/*N*/ 
/*N*/ 	const SwFrm *pBodyFrm = (const SwFrm*)(FindBodyFrm());
/*N*/ 
/*N*/     const long nBodyHeight = pBodyFrm ? ( IsVertical() ?
/*N*/                                           pBodyFrm->Frm().Width() :
/*N*/                                           pBodyFrm->Frm().Height() ) : 0;
/*N*/ 
/*N*/ 	// Wenn die aktuellen Werte berechnet wurden, anzeigen, dass
/*N*/ 	// sie jetzt gueltig sind.
/*N*/ 	*(pPara->GetReformat()) = SwCharRange();
/*N*/ 	sal_Bool bDelta = *pPara->GetDelta() != 0;
/*N*/ 	*(pPara->GetDelta()) = 0;
/*N*/ 
/*N*/ 	if( rLine.IsStop() )
/*N*/ 	{
/*?*/ 		rLine.TruncLines( sal_True );
/*?*/ 		nNew = 1;
/*N*/ 	}
/*N*/ 
/*N*/ 	// FindBreak schneidet die letzte Zeile ab.
/*N*/ 	if( !rFrmBreak.FindBreak( this, rLine, bHasToFit ) )
/*N*/ 	{
/*N*/ 		// Wenn wir bis zum Ende durchformatiert haben, wird nEnd auf das Ende
/*N*/ 		// gesetzt. In AdjustFollow wird dadurch ggf. JoinFrm() ausgefuehrt.
/*N*/ 		// Ansonsten ist nEnd das Ende der letzten Zeile im Master.
/*N*/ 		xub_StrLen nOld = nEnd;
/*N*/ 		nEnd = rLine.GetEnd();
/*N*/ 		if( GetFollow() )
/*N*/ 		{
/*N*/ 			if( nNew && nOld < nEnd )
/*N*/ 				RemoveFtn( nOld, nEnd - nOld );
/*N*/ 			CHG_OFFSET( GetFollow(), nEnd )
/*N*/ 			if( !bDelta )
/*N*/ 				GetFollow()->ManipOfst( nEnd );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{   // Wenn wir Zeilen abgeben, darf kein Join auf den Folows gerufen werden,
/*N*/ 		// im Gegenteil, es muss ggf. sogar ein Follow erzeugt werden.
/*N*/ 		// Dies muss auch geschehen, wenn die Textmasse komplett im Master
/*N*/ 		// bleibt, denn es könnte ja ein harter Zeilenumbruch noch eine weitere
/*N*/ 		// Zeile (ohne Textmassse) notwendig machen!
/*N*/ 		nEnd = rLine.GetEnd();
/*N*/ 		if( GetFollow() )
/*N*/ 		{
/*N*/             // OD 21.03.2003 #108121# - Another case for not joining the follow:
/*N*/             // Text frame has no content, but a numbering. Then, do *not* join.
/*N*/             // Example of this case: When an empty, but numbered paragraph
/*N*/             // at the end of page is completely displaced by a fly frame.
/*N*/             // Thus, the text frame introduced a follow by a
/*N*/             // <SwTxtFrm::SplitFrm(..)> - see below. The follow then shows
/*N*/             // the numbering and must stay.
/*N*/             if ( GetFollow()->GetOfst() != nEnd ||
/*N*/                  GetFollow()->IsFieldFollow() ||
/*N*/                  ( nStrLen == 0 &&
/*N*/                    ( GetTxtNode()->GetNum() || GetTxtNode()->GetOutlineNum() ) )
/*N*/                )
/*N*/             {
/*N*/ 				nNew |= 3;
/*N*/             }
/*N*/ 			CHG_OFFSET( GetFollow(), nEnd )
/*N*/ 			GetFollow()->ManipOfst( nEnd );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/             // OD 21.03.2003 #108121# - Only split frame, if the frame contains
/*?*/             // content or contains no content, but has a numbering.
/*?*/             if ( nStrLen > 0 ||
/*?*/                  ( nStrLen == 0 &&
/*?*/                    ( GetTxtNode()->GetNum() || GetTxtNode()->GetOutlineNum() ) )
/*?*/                )
/*?*/             {
/*?*/                 SplitFrm( nEnd );
/*?*/                 nNew |= 3;
/*?*/             }
/*N*/ 		}
/*N*/ 		// Wenn sich die Resthoehe geaendert hat, z.B. durch RemoveFtn()
/*N*/ 		// dann muessen wir auffuellen, um Oszillationen zu vermeiden!
/*N*/         if( bDummy && pBodyFrm &&
/*N*/            nBodyHeight < ( IsVertical() ?
/*N*/                            pBodyFrm->Frm().Width() :
/*N*/                            pBodyFrm->Frm().Height() ) )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 			rLine.MakeDummyLine();
/*N*/ 	}
/*N*/ 
/*N*/ 	// In AdjustFrm() stellen wir uns selbst per Grow/Shrink ein,
/*N*/ 	// in AdjustFollow() stellen wir unseren FolgeFrame ein.
/*N*/ 
/*N*/ 	const SwTwips nDocPrtTop = Frm().Top() + Prt().Top();
/*N*/ 	const SwTwips nOldHeight = Prt().SSize().Height();
/*N*/ 	const SwTwips nChg = rLine.CalcBottomLine() - nDocPrtTop - nOldHeight;
/*N*/ 
/*N*/     // Vertical Formatting:
/*N*/     // The (rotated) repaint rectangle's x coordinate referes to the frame.
/*N*/     // If the frame grows (or shirks) the repaint rectangle cannot simply
/*N*/     // be rotated back after formatting, because we use the upper left point
/*N*/     // of the frame for rotation. This point changes when growing/shrinking.
/*N*/     if ( IsVertical() && nChg )
/*N*/     {
/*?*/         SwRect &rRepaint = *(pPara->GetRepaint());
/*?*/         rRepaint.Left( rRepaint.Left() - nChg );
/*?*/         rRepaint.Width( rRepaint.Width() - nChg );
/*N*/     }
/*N*/ 
/*N*/     AdjustFrm( nChg, bHasToFit );

/*
    // FME 16.07.2003 #i16930# - removed this code because it did not
    // work correctly. In SwCntntFrm::MakeAll, the frame did not move to the
    // next page, instead the print area was recalculated and
    // Prepare( PREP_POS_CHGD, (const void*)&bFormatted, FALSE ) invalidated
    // the other flags => loop

    // OD 04.04.2003 #108446# - handle special case:
    // If text frame contains no content and just has splitted, because of a
    // line stop, it has to move forward. To force this forward move without
    // unnecessary formatting of its footnotes and its follow, especially in
    // columned sections, adjust frame height to zero (0) and do not perform
    // the intrinsic format of the follow.
    // The formating method <SwCntntFrm::MakeAll()> will initiate the move forward.
    sal_Bool bForcedNoIntrinsicFollowCalc = sal_False;
    if ( nEnd == 0 &&
         rLine.IsStop() && HasFollow() && nNew == 1
       )
    {
        AdjustFrm( -Frm().SSize().Height(), bHasToFit );
        Prt().Pos().Y() = 0;
        Prt().Height( Frm().Height() );
        if ( FollowFormatAllowed() )
        {
            bForcedNoIntrinsicFollowCalc = sal_True;
            ForbidFollowFormat();
        }
    }
    else
    {
        AdjustFrm( nChg, bHasToFit );
    }
 */

/*N*/ 	if( HasFollow() || IsInFtn() )
/*N*/ 		_AdjustFollow( rLine, nEnd, nStrLen, nNew );

    // FME 16.07.2003 #i16930# - removed this code because it did not work
    // correctly
    // OD 04.04.2003 #108446# - allow intrinsic format of follow, if above
    // special case has forbit it.
/*    if ( bForcedNoIntrinsicFollowCalc )
    {
        AllowFollowFormat();
    }
 */

/*N*/     pPara->SetPrepMustFit( sal_False );
/*N*/ 
/*N*/     UNDO_SWAP( this )
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::FormatLine()
 *************************************************************************/

// bPrev zeigt an, ob Reformat.Start() wegen Prev() vorgezogen wurde.
// Man weiss sonst nicht, ob man Repaint weiter einschraenken kann oder nicht.


/*N*/ sal_Bool SwTxtFrm::FormatLine( SwTxtFormatter &rLine, const sal_Bool bPrev )
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || IsSwapped(),
/*N*/             "SwTxtFrm::FormatLine( rLine, bPrev) with unswapped frame" );
/*N*/ 	SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
/*N*/ 	// Nach rLine.FormatLine() haelt nStart den neuen Wert,
/*N*/ 	// waehrend in pOldStart der alte Offset gepflegt wird.
/*N*/ 	// Ueber diesen Weg soll das nDelta ersetzt werden.
/*N*/ 	// *pOldStart += rLine.GetCurr()->GetLen();
/*N*/ 	const SwLineLayout *pOldCur = rLine.GetCurr();
/*N*/ 	const xub_StrLen nOldLen    = pOldCur->GetLen();
/*N*/ 	const KSHORT nOldAscent = pOldCur->GetAscent();
/*N*/ 	const KSHORT nOldHeight = pOldCur->Height();
/*N*/ 	const SwTwips nOldWidth	= pOldCur->Width() + pOldCur->GetHangingMargin();
/*N*/ 	const sal_Bool bOldHyph = pOldCur->IsEndHyph();
/*N*/ 	SwTwips nOldTop = 0;
/*N*/ 	SwTwips nOldBottom;
/*N*/ 	if( rLine.GetCurr()->IsClipping() )
/*N*/ 		rLine.CalcUnclipped( nOldTop, nOldBottom );
/*N*/ 
/*N*/ 	const xub_StrLen nNewStart = rLine.FormatLine( rLine.GetStart() );
/*N*/ 
/*N*/     ASSERT( Frm().Pos().Y() + Prt().Pos().Y() == rLine.GetFirstPos(),
/*N*/ 			"SwTxtFrm::FormatLine: frame leaves orbit." );
/*N*/ 	ASSERT( rLine.GetCurr()->Height(),
/*N*/ 			"SwTxtFrm::FormatLine: line height is zero" );
/*N*/ 
/*N*/ 	// Das aktuelle Zeilenumbruchobjekt.
/*N*/ 	const SwLineLayout *pNew = rLine.GetCurr();
/*N*/ 
/*N*/ 	sal_Bool bUnChg = nOldLen == pNew->GetLen() &&
/*N*/ 				  bOldHyph == pNew->IsEndHyph();
/*N*/ 	if ( bUnChg && !bPrev )
/*N*/ 	{
/*N*/ 		// 6672: Toleranz von SLOPPY_TWIPS (5 Twips); vgl. 6922
/*N*/ 		const long nWidthDiff = nOldWidth > pNew->Width()
/*N*/ 								? nOldWidth - pNew->Width()
/*N*/ 								: pNew->Width() - nOldWidth;
/*N*/ 
/*N*/         // we only declare a line as unchanged, if its main values have not
/*N*/         // changed and it is not the last line (!paragraph end symbol!)
/*N*/ 		bUnChg = nOldHeight == pNew->Height() &&
/*N*/ 				 nOldAscent == pNew->GetAscent() &&
/*N*/                  nWidthDiff <= SLOPPY_TWIPS &&
/*N*/                  pOldCur->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	// rRepaint wird berechnet:
/*N*/ 	const SwTwips nBottom = rLine.Y() + rLine.GetLineHeight();
/*N*/ 	SwRepaint &rRepaint = *(pPara->GetRepaint());
/*N*/ 	if( bUnChg && rRepaint.Top() == rLine.Y()
/*N*/ 			   && (bPrev || nNewStart <= pPara->GetReformat()->Start())
/*N*/ 			   && ( nNewStart < GetTxtNode()->GetTxt().Len() ) )
/*N*/ 	{
/*N*/ 		rRepaint.Top( nBottom );
/*N*/ 		rRepaint.Height( 0 );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( nOldTop )
/*N*/ 		{
/*?*/ 			if( nOldTop < rRepaint.Top() )
/*?*/ 				rRepaint.Top( nOldTop );
/*?*/ 			if( !rLine.IsUnclipped() || nOldBottom > rRepaint.Bottom() )
/*?*/ 			{
/*?*/ 				rRepaint.Bottom( nOldBottom - 1 );
/*?*/ 				rLine.SetUnclipped( sal_True );
/*?*/ 			}
/*N*/ 		}
/*N*/ 		if( rLine.GetCurr()->IsClipping() && rLine.IsFlyInCntBase() )
/*N*/ 		{
/*?*/ 			SwTwips nTmpTop, nTmpBottom;
/*?*/ 			rLine.CalcUnclipped( nTmpTop, nTmpBottom );
/*?*/ 			if( nTmpTop < rRepaint.Top() )
/*?*/ 				rRepaint.Top( nTmpTop );
/*?*/ 			if( !rLine.IsUnclipped() || nTmpBottom > rRepaint.Bottom() )
/*?*/ 			{
/*?*/ 				rRepaint.Bottom( nTmpBottom - 1 );
/*?*/ 				rLine.SetUnclipped( sal_True );
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( !rLine.IsUnclipped() || nBottom > rRepaint.Bottom() )
/*N*/ 			{
/*N*/ 				rRepaint.Bottom( nBottom - 1 );
/*N*/ 				rLine.SetUnclipped( sal_False );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		SwTwips nRght = Max( nOldWidth, pNew->Width() +
/*N*/ 							 pNew->GetHangingMargin() );
/*N*/ 		ViewShell *pSh = GetShell();
/*N*/ 		const SwViewOption *pOpt = pSh ? pSh->GetViewOptions() : 0;
/*N*/ 		if( pOpt && (pOpt->IsParagraph() || pOpt->IsLineBreak()) )
/*?*/ 			nRght += ( Max( nOldAscent, pNew->GetAscent() ) );
/*N*/ 		else
/*N*/ 			nRght += ( Max( nOldAscent, pNew->GetAscent() ) / 4);
/*N*/ 		nRght += rLine.GetLeftMargin();
/*N*/ 		if( rRepaint.GetOfst() || rRepaint.GetRightOfst() < nRght )
/*N*/ 			rRepaint.SetRightOfst( nRght );
/*N*/ 
/*N*/         // Finally we enlarge the repaint rectangle if we found an underscore
/*N*/         // within our line. 40 Twips should be enough
/*N*/         const sal_Bool bHasUnderscore =
/*N*/                 ( rLine.GetInfo().GetUnderScorePos() < nNewStart );
/*N*/         if ( bHasUnderscore || rLine.GetCurr()->HasUnderscore() )
/*N*/             rRepaint.Bottom( rRepaint.Bottom() + 40 );
/*N*/ 
/*N*/         ((SwLineLayout*)rLine.GetCurr())->SetUnderscore( bHasUnderscore );
/*N*/ 	}
/*N*/ 	if( !bUnChg )
/*N*/ 		rLine.SetChanges();
/*N*/ 
/*N*/ 	// Die gute, alte nDelta-Berechnung:
/*N*/ 	*(pPara->GetDelta()) -= long(pNew->GetLen()) - long(nOldLen);
/*N*/ 
/*N*/ 	// Stop!
/*N*/ 	if( rLine.IsStop() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	// Unbedingt noch eine Zeile
/*N*/ 	if( rLine.IsNewLine() )
/*N*/ 		return sal_True;
/*N*/ 
/*N*/ 	// bis zum Ende des Strings ?
/*N*/ 	if( nNewStart >= GetTxtNode()->GetTxt().Len() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	if( rLine.GetInfo().IsShift() )
/*N*/ 		return sal_True;
/*N*/ 
/*N*/ 	// Ende des Reformats erreicht ?
/*N*/ 	const xub_StrLen nEnd = pPara->GetReformat()->Start() +
/*N*/ 						pPara->GetReformat()->Len();
/*N*/ 
/*N*/ 	if( nNewStart <= nEnd )
/*N*/ 		return sal_True;
/*N*/ 
/*N*/ 	return 0 != *(pPara->GetDelta());
/*N*/ 
/*N*/ // Dieser Bereich ist so sensibel, da behalten wir mal die alte Version:
/*N*/ #ifdef USED
/*N*/ 	// nDelta abgearbeitet ?
/*N*/ 	if( 0 == *(pPara->GetDelta()) )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	// Wenn die Zeilen ausgeglichen sind, ist alles ok.
/*N*/ 	if( bUnChg )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ #endif
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::_Format()
 *************************************************************************/

/*N*/ void SwTxtFrm::_Format( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf,
/*N*/ 						const sal_Bool bAdjust )
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || IsSwapped(),"SwTxtFrm::_Format with unswapped frame" );
/*N*/ 
/*N*/     SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
/*N*/ 	rLine.SetUnclipped( sal_False );
/*N*/ 
/*N*/ 	// Das war dem C30 zu kompliziert: aString( GetTxt() );
/*N*/ 	const XubString &rString = GetTxtNode()->GetTxt();
/*N*/ 	const xub_StrLen nStrLen = rString.Len();
/*N*/ 
/*N*/ 	SwCharRange &rReformat = *(pPara->GetReformat());
/*N*/ 	SwRepaint	&rRepaint = *(pPara->GetRepaint());
/*N*/ 	SwRepaint *pFreeze = NULL;
/*N*/ 
/*N*/ 	// Aus Performancegruenden wird in Init() rReformat auf STRING_LEN gesetzt.
/*N*/ 	// Fuer diesen Fall wird rReformat angepasst.
/*N*/ 	if( rReformat.Len() > nStrLen )
/*N*/ 		rReformat.Len() = nStrLen;
/*N*/ 
/*N*/ 	// Optimiert:
/*N*/ 	xub_StrLen nEnd = rReformat.Start() + rReformat.Len();
/*N*/ 	if( nEnd > nStrLen )
/*N*/ 	{
/*?*/ 		rReformat.Len() = nStrLen - rReformat.Start();
/*?*/ 		nEnd = nStrLen;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwTwips nOldBottom;
/*N*/ 	if( GetOfst() && !IsFollow() )
/*N*/ 	{
/*?*/ 		rLine.Bottom();
/*?*/ 		nOldBottom = rLine.Y();
/*?*/ 		rLine.Top();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nOldBottom = 0;
/*N*/ 	rLine.CharToLine( rReformat.Start() );
/*N*/ 
/*N*/ 	// Worte koennen durch Fortfall oder Einfuegen eines Space
/*N*/ 	// auf die Zeile vor der editierten hinausgezogen werden,
/*N*/ 	// deshalb muss diese ebenfalls formatiert werden.
/*N*/ 	// Optimierung: Wenn rReformat erst hinter dem ersten Wort der
/*N*/ 	// Zeile beginnt, so kann diese Zeile die vorige nicht mehr beeinflussen.
/*N*/ 	// AMA: Leider doch, Textgroessenaenderungen + FlyFrames, die Rueckwirkung
/*N*/ 	// kann im Extremfall mehrere Zeilen (Frames!!!) betreffen!
/*N*/ 
/*N*/ 	sal_Bool bPrev = rLine.GetPrev() &&
/*N*/ 					 ( FindBrk( rString, rLine.GetStart(),
/*N*/ 								rReformat.Start() + 1 ) >= rReformat.Start() ||
/*N*/ 					   rLine.GetCurr()->IsRest() );
/*N*/ 	if( bPrev )
/*N*/ 	{
/*N*/ 		while( rLine.Prev() )
/*N*/ 			if( rLine.GetCurr()->GetLen() && !rLine.GetCurr()->IsRest() )
/*N*/ 			{
/*N*/ 				if( !rLine.GetStart() )
/*N*/ 					rLine.Top(); // damit NumDone nicht durcheinander kommt
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		xub_StrLen nNew = rLine.GetStart() + rLine.GetLength();
/*N*/ 		if( nNew )
/*N*/ 		{
/*N*/ 			--nNew;
/*N*/ 			if( CH_BREAK == rString.GetChar( nNew ) )
/*N*/ 			{
/*N*/ 				++nNew;
/*N*/ 				rLine.Next();
/*N*/ 				bPrev = sal_False;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		rReformat.Len()  += rReformat.Start() - nNew;
/*N*/ 		rReformat.Start() = nNew;
/*N*/ 	}
/*N*/ 
/*N*/ 	rRepaint.SetOfst( 0 );
/*N*/ 	rRepaint.SetRightOfst( 0 );
/*N*/ 	rRepaint.Chg( Frm().Pos() + Prt().Pos(), Prt().SSize() );
/*N*/ 	if( pPara->IsMargin() )
/*?*/ 		rRepaint.Width( rRepaint.Width() + pPara->GetHangingMargin() );
/*N*/ 	rRepaint.Top( rLine.Y() );
/*N*/ 	// 6792: Rrand < LRand und Repaint
/*N*/ 	if( 0 >= rRepaint.Width() )
/*?*/ 		rRepaint.Width(1);
/*N*/ 	WidowsAndOrphans aFrmBreak( this, rInf.IsTest() ? 1 : 0 );
/*N*/ 
/*N*/ 	// rLine steht jetzt auf der ersten Zeile, die formatiert werden
/*N*/ 	// muss. Das Flag bFirst sorgt dafuer, dass nicht Next() gerufen wird.
/*N*/ 	// Das ganze sieht verdreht aus, aber es muss sichergestellt werden,
/*N*/ 	// dass bei IsBreakNow rLine auf der Zeile zum stehen kommt, die
/*N*/ 	// nicht mehr passt.
/*N*/ 	sal_Bool bFirst  = sal_True;
/*N*/ 	sal_Bool bFormat = sal_True;
/*N*/ 
/*N*/ 	// 5383: Das CharToLine() kann uns auch in den roten Bereich fuehren.
/*N*/ 	// In diesem Fall muessen wir zurueckwandern, bis die Zeile, die
/*N*/ 	// nicht mehr passt in rLine eingestellt ist. Ansonsten geht Textmasse
/*N*/ 	// verloren, weil der Ofst im Follow falsch eingestellt wird.
/*N*/ 
/*N*/ 	sal_Bool bBreak = ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 )
/*N*/ 					&& aFrmBreak.IsBreakNow( rLine );
/*N*/ 	if( bBreak )
/*N*/ 	{
/*?*/ 		sal_Bool bPrevDone = 0 != rLine.Prev();
/*?*/ 		while( bPrevDone && aFrmBreak.IsBreakNow(rLine) )
/*?*/ 			bPrevDone = 0 != rLine.Prev();
/*?*/ 		if( bPrevDone )
/*?*/ 		{
/*?*/ 			aFrmBreak.SetKeep( sal_False );
/*?*/ 			rLine.Next();
/*?*/ 		}
/*?*/ 		rLine.TruncLines();
/*?*/ 
/*?*/ 		// auf Nummer sicher:
/*?*/ 		bBreak = aFrmBreak.IsBreakNow(rLine) &&
/*?*/ 				  ( !pPara->IsPrepMustFit() || rLine.GetLineNr() > 1 );
/*N*/ 	}
/*N*/ 
/*	Bedeutung der folgenden Flags:
    Ist das Watch(End/Mid)Hyph-Flag gesetzt, so muss formatiert werden, wenn
    eine Trennung am (Zeilenende/Fly) vorliegt, sofern MaxHyph erreicht ist.
    Das Jump(End/Mid)Flag bedeutet, dass die naechste Zeile, bei der keine
    Trennung (Zeilenende/Fly) vorliegt, formatiert werden muss, da jetzt
    umgebrochen werden koennte, was vorher moeglicherweise durch MaxHyph
    verboten war.
    Watch(End/Mid)Hyph wird gesetzt, wenn die letzte formatierte Zeile eine
    Trennstelle erhalten hat, vorher aber keine hatte,
    Jump(End/Mid)Hyph, wenn eine Trennstelle verschwindet.
*/
/*N*/ 	sal_Bool bJumpEndHyph  = sal_False,
/*N*/ 		 bWatchEndHyph = sal_False,
/*N*/ 		 bJumpMidHyph  = sal_False,
/*N*/ 		 bWatchMidHyph = sal_False;
/*N*/ 
/*N*/ 	const SwAttrSet& rAttrSet = GetTxtNode()->GetSwAttrSet();
/*N*/ 	sal_Bool bMaxHyph = ( 0 !=
/*N*/ 		( rInf.MaxHyph() = rAttrSet.GetHyphenZone().GetMaxHyphens() ) );
/*N*/ 	if ( bMaxHyph )
/*N*/ 		rLine.InitCntHyph();
/*N*/ 
/*N*/ 	if( IsFollow() && IsFieldFollow() && rLine.GetStart() == GetOfst() )
/*N*/ 	{
/*N*/ 		const SwLineLayout* pLine;
/*N*/ 		{
/*?*/ 			SwTxtFrm *pMaster = FindMaster();
/*?*/ 			ASSERT( pMaster, "SwTxtFrm::Format: homeless follow" );
/*?*/ 			if( !pMaster->HasPara() )
/*?*/ 				pMaster->GetFormatted();
/*?*/ 			SwTxtSizeInfo aInf( pMaster );
/*?*/ 			SwTxtIter aMasterLine( pMaster, &aInf );
/*?*/ 			aMasterLine.Bottom();
/*?*/ 			pLine = aMasterLine.GetCurr();
/*?*/ 		}
/*?*/ 		SwLinePortion* pRest =
/*?*/ 			rLine.MakeRestPortion( pLine, GetOfst() );
/*?*/ 		if( pRest )
/*?*/ 			rInf.SetRest( pRest );
/*?*/ 		else
/*?*/ 			SetFieldFollow( sal_False );
/*N*/ 	}
/*N*/ 
/* Zum Abbruchkriterium:
 * Um zu erkennen, dass eine Zeile nicht mehr auf die Seite passt,
 * muss sie formatiert werden. Dieser Ueberhang wird z.B. in AdjustFollow
 * wieder entfernt.
 * Eine weitere Komplikation: wenn wir der Master sind, so muessen
 * wir die Zeilen durchgehen, da es ja sein kann, dass eine Zeile
 * vom Follow in den Master rutschen kann.
 */
/*N*/ 	do
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		if( bFirst )
/*N*/ 			bFirst = sal_False;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( bMaxHyph )
/*N*/ 			{
/*N*/ 				if ( rLine.GetCurr()->IsEndHyph() )
/*N*/ 					rLine.CntEndHyph()++;
/*N*/ 				else
/*N*/ 					rLine.CntEndHyph() = 0;
/*N*/ 				if ( rLine.GetCurr()->IsMidHyph() )
/*N*/ 					rLine.CntMidHyph()++;
/*N*/ 				else
/*N*/ 					rLine.CntMidHyph() = 0;
/*N*/ 			}
/*N*/ 			if( !rLine.Next() )
/*N*/ 			{
/*N*/ 				if( !bFormat )
/*N*/ 					rLine.MakeRestPortion( rLine.GetCurr(),	rLine.GetEnd() );
/*N*/ 				rLine.Insert( new SwLineLayout() );
/*N*/ 				rLine.Next();
/*N*/ 				bFormat = sal_True;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !bFormat && bMaxHyph &&
/*N*/ 			  (bWatchEndHyph || bJumpEndHyph || bWatchMidHyph || bJumpMidHyph) )
/*N*/ 		{
/*N*/ 			if ( rLine.GetCurr()->IsEndHyph() )
/*N*/ 			{
/*N*/ 				if ( bWatchEndHyph )
/*?*/ 					bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				bFormat = bJumpEndHyph;
/*N*/ 				bWatchEndHyph = sal_False;
/*N*/ 				bJumpEndHyph = sal_False;
/*N*/ 			}
/*N*/ 			if ( rLine.GetCurr()->IsMidHyph() )
/*N*/ 			{
/*?*/ 				if ( bWatchMidHyph && !bFormat )
/*?*/ 					bFormat = ( rLine.CntEndHyph() == rInf.MaxHyph() );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				bFormat = bFormat || bJumpMidHyph;
/*N*/ 				bWatchMidHyph = sal_False;
/*N*/ 				bJumpMidHyph = sal_False;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( bFormat )
/*N*/ 		{
/*N*/ 			sal_Bool bOldEndHyph = rLine.GetCurr()->IsEndHyph();
/*N*/ 			sal_Bool bOldMidHyph = rLine.GetCurr()->IsMidHyph();
/*N*/ 			bFormat = FormatLine( rLine, bPrev );
/*N*/ 			//9334: Es kann nur ein bPrev geben... (???)
/*N*/ 			bPrev = sal_False;
/*N*/ 			if ( bMaxHyph )
/*N*/ 			{
/*N*/ 				if ( rLine.GetCurr()->IsEndHyph() != bOldEndHyph )
/*N*/ 				{
/*N*/ 					bWatchEndHyph = !bOldEndHyph;
/*N*/ 					bJumpEndHyph = bOldEndHyph;
/*N*/ 				}
/*N*/ 				if ( rLine.GetCurr()->IsMidHyph() != bOldMidHyph )
/*N*/ 				{
/*?*/ 					bWatchMidHyph = !bOldMidHyph;
/*?*/ 					bJumpMidHyph = bOldMidHyph;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !rInf.IsNewLine() )
/*N*/ 		{
/*N*/ 			if( !bFormat )
/*N*/ 				 bFormat = 0 != rInf.GetRest();
/*N*/ 			if( rInf.IsStop() || rInf.GetIdx() >= nStrLen )
/*N*/ 				break;
/*N*/ 			if( !bFormat && ( !bMaxHyph || ( !bWatchEndHyph &&
/*N*/ 					!bJumpEndHyph && !bWatchMidHyph && !bJumpMidHyph ) ) )
/*N*/ 			{
/*N*/ 				if( GetFollow() )
/*N*/ 				{
/*N*/ 					while( rLine.Next() )
/*N*/ 						; //Nothing
/*N*/ 					pFreeze = new SwRepaint( rRepaint ); // to minimize painting
/*N*/ 				}
/*N*/ 				else
/*N*/ 					break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		bBreak = aFrmBreak.IsBreakNow(rLine);
/*N*/ 	}while( !bBreak );
/*N*/ 
/*N*/ 	if( pFreeze )
/*N*/ 	{
/*N*/ 		rRepaint = *pFreeze;
/*N*/ 		delete pFreeze;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !rLine.IsStop() )
/*N*/ 	{
/*N*/ 		// Wurde aller Text formatiert und gibt es noch weitere
/*N*/ 		// Zeilenobjekte, dann sind diese jetzt ueberfluessig,
/*N*/ 		// weil der Text kuerzer geworden ist.
/*N*/ 		if( rLine.GetStart() + rLine.GetLength() >= nStrLen &&
/*N*/ 			rLine.GetCurr()->GetNext() )
/*N*/ 		{
/*N*/ 			rLine.TruncLines();
/*N*/ 			rLine.SetTruncLines( sal_True );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( !rInf.IsTest() )
/*N*/ 	{
/*N*/ 		// Bei OnceMore lohnt sich kein FormatAdjust
/*N*/ 		if( bAdjust || !rLine.GetDropFmt() || !rLine.CalcOnceMore() )
/*N*/		{
/*N*/ 			FormatAdjust( rLine, aFrmBreak, nStrLen, rInf.IsStop() );
/*N*/		}
/*N*/ 		if( rRepaint.HasArea() )
/*N*/ 			SetRepaint();
/*N*/ 		rLine.SetTruncLines( sal_False );
/*N*/ 		if( nOldBottom )                    // Bei "gescollten" Absaetzen wird
/*N*/ 		{                                   // noch ueberprueft, ob durch Schrumpfen
/*?*/ 			rLine.Bottom();					// das Scrolling ueberfluessig wurde.
/*?*/ 			SwTwips nNewBottom = rLine.Y();
/*?*/ 			if( nNewBottom < nOldBottom )
/*?*/ 				_SetOfst( 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::Format()
 *************************************************************************/

/*N*/ void SwTxtFrm::FormatOnceMore( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || IsSwapped(),
/*N*/             "A frame is not swapped in SwTxtFrm::FormatOnceMore" );
/*N*/ 
/*N*/ 	SwParaPortion *pPara = rLine.GetInfo().GetParaPortion();
/*N*/ 	if( !pPara )
/*N*/ 		return;
/*N*/ 
/*N*/ 	// ggf gegen pPara
/*N*/ 	KSHORT nOld  = ((const SwTxtMargin&)rLine).GetDropHeight();
/*N*/ 	sal_Bool bShrink = sal_False,
/*N*/ 		 bGrow   = sal_False,
/*N*/ 		 bGoOn   = rLine.IsOnceMore();
/*N*/ 	sal_uInt8 nGo	 = 0;
/*N*/ 	while( bGoOn )
/*N*/ 	{
/*N*/ #ifdef DBGTXT
/*N*/ 		aDbstream << "OnceMore!" << endl;
/*N*/ #endif
/*N*/ 		++nGo;
/*N*/ 		rInf.Init();
/*N*/ 		rLine.Top();
/*N*/ 		if( !rLine.GetDropFmt() )
/*N*/ 			rLine.SetOnceMore( sal_False );
/*N*/ 		SwCharRange aRange( 0, rInf.GetTxt().Len() );
/*N*/ 		*(pPara->GetReformat()) = aRange;
/*N*/ 		_Format( rLine, rInf );
/*N*/ 
/*N*/ 		bGoOn = rLine.IsOnceMore();
/*N*/ 		if( bGoOn )
/*N*/ 		{
/*?*/ 			const KSHORT nNew = ((const SwTxtMargin&)rLine).GetDropHeight();
/*?*/ 			if( nOld == nNew )
/*?*/ 				bGoOn = sal_False;
/*?*/ 			else
/*?*/ 			{
/*?*/ 				if( nOld > nNew )
/*?*/ 					bShrink = sal_True;
/*?*/ 				else
/*?*/ 					bGrow = sal_True;
/*?*/ 
/*?*/ 				if( bShrink == bGrow || 5 < nGo )
/*?*/ 					bGoOn = sal_False;
/*?*/ 
/*?*/ 				nOld = nNew;
/*?*/ 			}
/*?*/ 
/*?*/ 			// 6107: Wenn was schief ging, muss noch einmal formatiert werden.
/*?*/ 			if( !bGoOn )
/*?*/ 			{
/*?*/ 				rInf.CtorInit( this );
/*?*/ 				rLine.CtorInit( this, &rInf );
/*?*/ 				rLine.SetDropLines( 1 );
/*?*/ 				rLine.CalcDropHeight( 1 );
/*?*/ 				SwCharRange aRange( 0, rInf.GetTxt().Len() );
/*?*/ 				*(pPara->GetReformat()) = aRange;
/*?*/ 				_Format( rLine, rInf, sal_True );
/*?*/ 				// 8047: Wir painten alles...
/*?*/ 				SetCompletePaint();
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::_Format()
 *************************************************************************/


/*N*/ void SwTxtFrm::_Format( SwParaPortion *pPara )
/*N*/ {
/*N*/ 	const xub_StrLen nStrLen = GetTxt().Len();
/*N*/ 
/*N*/ 	// AMA: Wozu soll das gut sein? Scheint mir zuoft zu einem kompletten
/*N*/ 	// Formatieren und Repainten zu fuehren???
/*N*/ //	if ( !(*pPara->GetDelta()) )
/*N*/ //		*(pPara->GetDelta()) = nStrLen;
/*N*/ //	else
/*N*/ 	if ( !nStrLen )
/*N*/ 	{
/*N*/ 		// Leere Zeilen werden nicht lange gequaelt:
/*N*/ 		// pPara wird blank geputzt
/*N*/ 		// entspricht *pPara = SwParaPortion;
/*N*/ 		sal_Bool bMustFit = pPara->IsPrepMustFit();
/*N*/ 		pPara->Truncate();
/*N*/ 		pPara->FormatReset();
/*N*/ 
/*N*/         // delete pSpaceAdd und pKanaComp
/*N*/         pPara->FinishSpaceAdd();
/*N*/         pPara->FinishKanaComp();
/*N*/ 		pPara->ResetFlags();
/*N*/ 		pPara->SetPrepMustFit( bMustFit );
/*N*/ 	}
/*N*/ 
/*N*/     ASSERT( ! IsSwapped(), "A frame is swapped before _Format" );
/*N*/ 
/*N*/     if ( IsVertical() )
/*?*/         SwapWidthAndHeight();
/*N*/ 
/*N*/     SwTxtFormatInfo aInf( this );
/*N*/ 	SwTxtFormatter	aLine( this, &aInf );
/*N*/ 
/*N*/ 	_Format( aLine, aInf );
/*N*/ 
/*N*/ 	if( aLine.IsOnceMore() )
/*N*/ 		FormatOnceMore( aLine, aInf );
/*N*/ 
/*N*/     if ( IsVertical() )
/*?*/         SwapWidthAndHeight();
/*N*/ 
/*N*/     ASSERT( ! IsSwapped(), "A frame is swapped after _Format" );
/*N*/ 
/*N*/     if( 1 < aLine.GetDropLines() )
/*N*/ 	{
/*N*/ 		if( SVX_ADJUST_LEFT != aLine.GetAdjust() &&
/*N*/ 			SVX_ADJUST_BLOCK != aLine.GetAdjust() )
/*N*/ 		{
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			aLine.CalcDropAdjust();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( aLine.IsPaintDrop() )
/*N*/ 		{
/*N*/ 			aLine.CalcDropRepaint();
/*N*/ 			aLine.SetPaintDrop( sal_False );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtFrm::Format()
 *************************************************************************/

/*
 * Format berechnet die Groesse des Textframes und ruft, wenn
 * diese feststeht, Shrink() oder Grow(), um die Framegroesse dem
 * evtl. veraenderten Platzbedarf anzupassen.
 */

/*M*/ void SwTxtFrm::Format( const SwBorderAttrs * )
/*M*/ {
///*M*/ 	DBG_LOOP;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ 	const XubString aXXX = GetTxtNode()->GetTxt();
/*M*/ 	const SwTwips nDbgY = Frm().Top();
/*M*/ 	const SwPageFrm *pDbgPage = FindPageFrm();
/*M*/ 	const MSHORT nDbgPageNr = pDbgPage->GetPhyPageNum();
/*M*/ 	// Um zu gucken, ob es einen Ftn-Bereich gibt.
/*M*/ 	const SwFrm *pDbgFtnCont = (const SwFrm*)(FindPageFrm()->FindFtnCont());
/*M*/ 
/*M*/ #ifdef DBG_UTIL
/*M*/ 	// nStopAt laesst sich vom CV bearbeiten.
/*M*/ 	static MSHORT nStopAt = 0;
/*M*/ 	if( nStopAt == GetFrmId() )
/*M*/ 	{
/*M*/ 		int i = GetFrmId();
/*M*/ 	}
/*M*/ #endif
/*M*/ #endif
/*M*/ 
/*M*/ #ifdef DEBUG_FTN
/*M*/ 	//Fussnote darf nicht auf einer Seite vor ihrer Referenz stehen.
/*M*/ 	if( IsInFtn() )
/*M*/ 	{
/*M*/ 		const SwFtnFrm *pFtn = (SwFtnFrm*)GetUpper();
/*M*/ 		const SwPageFrm *pFtnPage = pFtn->GetRef()->FindPageFrm();
/*M*/ 		const MSHORT nFtnPageNr = pFtnPage->GetPhyPageNum();
/*M*/ 		if( !IsLocked() )
/*M*/ 		{
/*M*/ 			if( nFtnPageNr > nDbgPageNr )
/*M*/ 			{
/*M*/ 				SwTxtFrmLocker aLock(this);
/*M*/ 				ASSERT( nFtnPageNr <= nDbgPageNr, "!Ftn steht vor der Referenz." );
/*M*/ 				MSHORT i = 0;
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/ #endif
/*M*/ 
/*M*/ 	MSHORT nRepeat = 0;
/*M*/ 
/*M*/     SWRECTFN( this )
/*M*/ 
/*M*/ 	do
/*M*/ 	{
/*M*/ 		// Vom Berichtsautopiloten oder ueber die BASIC-Schnittstelle kommen
/*M*/ 		// gelegentlich TxtFrms mit einer Breite <=0.
/*M*/         if( (Prt().*fnRect->fnGetWidth)() <= 0 )
/*M*/ 		{
/*M*/ 			// Wenn MustFit gesetzt ist, schrumpfen wir ggf. auf die Unterkante
/*M*/ 			// des Uppers, ansonsten nehmen wir einfach eine Standardgroesse
/*M*/ 			// von 12 Pt. ein (240 Twip).
/*M*/ 			SwTxtLineAccess aAccess( this );
/*M*/             long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*M*/ 			if( aAccess.GetPara()->IsPrepMustFit() )
/*M*/ 			{
/*M*/                 const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*M*/                 const SwTwips nDiff = - (Frm().*fnRect->fnBottomDist)( nLimit );
/*M*/                 if( nDiff > 0 )
/*M*/                     Shrink( nDiff );
/*M*/ 			}
/*M*/             else if( 240 < nFrmHeight )
/*M*/                 Shrink( nFrmHeight - 240 );
/*M*/             else if( 240 > nFrmHeight )
/*M*/                 Grow( 240 - nFrmHeight );
/*M*/             nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*M*/ 
/*M*/             long nTop = (this->*fnRect->fnGetTopMargin)();
/*M*/             if( nTop > nFrmHeight )
/*M*/                 (this->*fnRect->fnSetYMargins)( nFrmHeight, 0 );
/*M*/             else if( (Prt().*fnRect->fnGetHeight)() < 0 )
/*M*/                 (Prt().*fnRect->fnSetHeight)( 0 );
/*M*/ 			return;
/*M*/ 		}
/*M*/ 
/*M*/         sal_Bool bChkAtCnt = sal_False;
/*M*/ 		const xub_StrLen nStrLen = GetTxtNode()->GetTxt().Len();
/*M*/ 		if ( nStrLen || !FormatEmpty() )
/*M*/ 		{
/*M*/ 
/*M*/ 			SetEmpty( sal_False );
/*M*/ 		// Um nicht durch verschachtelte Formats irritiert zu werden.
/*M*/ 			FormatLevel aLevel;
/*M*/ 			if( 12 == aLevel.GetLevel() )
/*M*/ 				return;
/*M*/ 
/*M*/ 			// Die Formatinformationen duerfen u.U. nicht veraendert werden.
/*M*/ 			if( IsLocked() )
/*M*/ 				return;
/*M*/ 
/*M*/ 			// Waehrend wir formatieren, wollen wir nicht gestoert werden.
/*M*/ 			SwTxtFrmLocker aLock(this);
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ 	//MA 25. Jan. 94 Das Flag stimmt sehr haufig beim Eintritt nicht. Das muss
/*M*/ 	//			   bei naechster Gelegenheit geprueft und gefixt werden.
/*M*/ 			const sal_Bool bOldFtnFlag = HasFtn();
/*M*/ 			CalcFtnFlag();
/*M*/ 			if ( bOldFtnFlag != HasFtn() )
/*M*/ 				{int bla = 5;}
/*M*/ #endif
/*M*/ 
/*M*/ 			// 8708: Vorsicht, das Format() kann auch durch GetFormatted()
/*M*/ 			// angestossen werden.
/*M*/             if( IsHiddenNow() )
/*M*/ 			{
/*M*/                 long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*M*/                 if( nPrtHeight )
/*M*/ 				{
/*M*/ 					HideHidden();
/*M*/                     Shrink( nPrtHeight );
/*M*/ 				}
/*M*/ 				ChgThisLines();
/*M*/ 				return;
/*M*/ 			}
/*M*/ 
/*M*/ 			SwTxtLineAccess aAccess( this );
/*M*/ 			const sal_Bool bNew = !aAccess.SwTxtLineAccess::IsAvailable();
/*M*/             sal_Bool bSetOfst = sal_False;
/*M*/ 
/*M*/ 			if( CalcPreps() )
/*M*/ 				; // nothing
/*M*/ 			// Wir returnen, wenn schon formatiert wurde, nicht aber, wenn
/*M*/ 			// der TxtFrm gerade erzeugt wurde und ueberhaupt keine Format-
/*M*/ 			// informationen vorliegen.
/*M*/ 			else if( !bNew && !aAccess.GetPara()->GetReformat()->Len() )
/*M*/ 			{
/*M*/ 				if( GetTxtNode()->GetSwAttrSet().GetRegister().GetValue() )
/*M*/ 				{
/*M*/ 					aAccess.GetPara()->SetPrepAdjust( sal_True );
/*M*/ 					aAccess.GetPara()->SetPrep( sal_True );
/*M*/ 					CalcPreps();
/*M*/ 				}
/*M*/ 				SetWidow( sal_False );
/*M*/ 			}
/*M*/             else if( ( bSetOfst = ( GetOfst() && GetOfst() > GetTxtNode()->GetTxt().Len() ) ) &&
/*M*/                      IsFollow() )
/*M*/ 			{
/*M*/ 				SwTxtFrm *pMaster = FindMaster();
/*M*/ 				ASSERT( pMaster, "SwTxtFrm::Format: homeless follow" );
/*M*/                 if( pMaster )
/*M*/ 					pMaster->Prepare( PREP_FOLLOW_FOLLOWS );
/*M*/                 SwTwips nMaxY = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*M*/                 if( (Frm().*fnRect->fnOverStep)( nMaxY  ) )
/*M*/                     (this->*fnRect->fnSetLimit)( nMaxY );
/*M*/                 else if( (Frm().*fnRect->fnBottomDist)( nMaxY  ) < 0 )
/*M*/                     (Frm().*fnRect->fnAddBottom)( -(Frm().*fnRect->fnGetHeight)() );
/*M*/ 			}
/*M*/ 			else
/*M*/ 			{
/*M*/                 // bSetOfst here means that we have the "red arrow situation"
/*M*/                 if ( bSetOfst )
/*M*/                     _SetOfst( 0 );
/*M*/ 
/*M*/ 				const sal_Bool bOrphan = IsWidow();
/*M*/                 const SwFtnBossFrm* pFtnBoss = HasFtn() ? FindFtnBossFrm() : 0;
/*M*/                 SwTwips nFtnHeight;
/*M*/                 if( pFtnBoss )
/*M*/                 {
/*M*/                     const SwFtnContFrm* pCont = pFtnBoss->FindFtnCont();
/*M*/                     nFtnHeight = pCont ? (pCont->Frm().*fnRect->fnGetHeight)() : 0;
/*M*/                 }
/*M*/                 do
/*M*/                 {
/*M*/                     _Format( aAccess.GetPara() );
/*M*/                     if( pFtnBoss && nFtnHeight )
/*M*/                     {
/*M*/                         const SwFtnContFrm* pCont = pFtnBoss->FindFtnCont();
/*M*/                         SwTwips nNewHeight = pCont ? (pCont->Frm().*fnRect->fnGetHeight)() : 0;
/*M*/                         // If we lost some footnotes, we may have more space
/*M*/                         // for our main text, so we have to format again ...
/*M*/                         if( nNewHeight < nFtnHeight )
/*M*/                             nFtnHeight = nNewHeight;
/*M*/                         else
/*M*/                             break;
/*M*/                     }
/*M*/                     else
/*M*/                         break;
/*M*/                 } while ( pFtnBoss );
/*M*/ 				if( bOrphan )
/*M*/ 				{
/*M*/ 					ValidateFrm();
/*M*/ 					SetWidow( sal_False );
/*M*/ 				}
/*M*/ 				bChkAtCnt = sal_True;
/*M*/ 			}
/*M*/ 			if( IsEmptyMaster() )
/*M*/ 			{
/*M*/ 				SwFrm* pPre = GetPrev();
/*M*/ 				if( pPre &&	pPre->GetAttrSet()->GetKeep().GetValue() )
/*M*/ 					pPre->InvalidatePos();
/*M*/ 			}
/*M*/ 		}
/*M*/ 		MSHORT nMaxRepeat = 2;
/*M*/ 		if( bChkAtCnt && nRepeat < nMaxRepeat )
/*M*/ 		{
/*M*/ 			sal_Bool bRepeat = sal_False;
/*M*/ 			MSHORT nRepAdd = 0;
/*M*/ 			SwDrawObjs *pObjs;
/*M*/ 			SwTxtFrm *pMaster = IsFollow() ? FindMaster() : this;
/*M*/             if( pMaster && !pMaster->IsFlyLock() )
/*M*/ 			{
/*M*/ 				if ( 0 != (pObjs = pMaster->GetDrawObjs()) )
/*M*/ 				{
/*M*/ 					MSHORT nAutoCnt = 0;
/*M*/ 					for( MSHORT i = 0; i < pObjs->Count(); ++i )
/*M*/ 					{
/*M*/ 						SdrObject *pO = (*pObjs)[i];
/*M*/ 						if ( pO->IsWriterFlyFrame() )
/*M*/ 						{
/*M*/ 							SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*M*/ 							if( pFly->IsAutoPos() && !::binfilter::IsInProgress( pFly ) )
/*M*/ 							{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*M*/ 							}
/*M*/ 						}
/*M*/ 					}
/*M*/ 					if( nAutoCnt > 11 )
/*M*/ 						nMaxRepeat = nAutoCnt/4;
/*M*/ 				}
/*M*/ 			}
/*M*/ 			if( bRepeat )
/*M*/ 				nRepeat += nRepAdd;
/*M*/ 			else
/*M*/ 				nRepeat = 0;
/*M*/ 		}
/*M*/ 		else
/*M*/ 			nRepeat = 0;
/*M*/ 	} while( nRepeat );
/*M*/ 
/*M*/ 	ChgThisLines();
/*M*/ 
/*N*/     // the PrepMustFit should not survive a Format operation
/*N*/     SwParaPortion *pPara = GetPara();
/*N*/ 	if ( pPara )
/*N*/        	pPara->SetPrepMustFit( sal_False );
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ 	// Hier ein Instrumentarium, um ungewoehnlichen Master/Follow-Kombinationen,
/*M*/ 	// insbesondere bei Fussnoten, auf die Schliche zu kommen
/*M*/ 	if( IsFollow() || GetFollow() )
/*M*/ 	{
/*M*/ 		SwTxtFrm *pTmpFrm = IsFollow() ? FindMaster() : this;
/*M*/ 		const SwPageFrm *pTmpPage = pTmpFrm->FindPageFrm();
/*M*/ 		MSHORT nPgNr = pTmpPage->GetPhyPageNum();
/*M*/ 		MSHORT nLast;
/*M*/ 		MSHORT nDummy = 0; // nur zum Breakpoint setzen
/*M*/ 		while( pTmpFrm->GetFollow() )
/*M*/ 		{
/*M*/ 			pTmpFrm = pTmpFrm->GetFollow();
/*M*/ 			nLast = nPgNr;
/*M*/ 			pTmpPage = pTmpFrm->FindPageFrm();
/*M*/ 			nPgNr = pTmpPage->GetPhyPageNum();
/*M*/ 			if( nLast > nPgNr )
/*M*/ 				++nDummy; // schon fast eine Assertion wert
/*M*/ 			else if( nLast == nPgNr )
/*M*/ 				++nDummy; // bei Spalten voellig normal, aber sonst!?
/*M*/ 			else if( nLast < nPgNr - 1 )
/*M*/ 				++nDummy; // kann schon mal temporaer vorkommen
/*M*/ 		}
/*M*/ 	}
/*M*/ #endif
/*N*/ 
/*N*/     CalcBaseOfstForFly();
/*M*/ }

/*************************************************************************
 *						SwTxtFrm::FormatQuick()
 *************************************************************************/
// 6995:
// return sal_False: Prepare(), HasPara(), InvalidateRanges(),

/*N*/ sal_Bool SwTxtFrm::FormatQuick()
/*N*/ {
/*N*/     ASSERT( ! IsVertical() || ! IsSwapped(),
/*N*/             "SwTxtFrm::FormatQuick with swapped frame" );
/*N*/ 
///*N*/ 	DBG_LOOP;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	const XubString aXXX = GetTxtNode()->GetTxt();
/*N*/ 	const SwTwips nDbgY = Frm().Top();
/*N*/ #ifdef DBG_UTIL
/*N*/ 	// nStopAt laesst sich vom CV bearbeiten.
/*N*/ 	static MSHORT nStopAt = 0;
/*N*/ 	if( nStopAt == GetFrmId() )
/*N*/ 	{
/*?*/ 		int i = GetFrmId();
/*N*/ 	}
/*N*/ #endif
/*N*/ #endif
/*N*/ 
/*N*/ 	if( IsEmpty() && FormatEmpty() )
/*N*/ 		return sal_True;
/*N*/ 
/*N*/     // Wir sind sehr waehlerisch:
/*N*/ 	if( HasPara() || IsWidow() || IsLocked()
/*N*/         || !GetValidSizeFlag() ||
/*N*/         ( ( IsVertical() ? Prt().Width() : Prt().Height() ) && IsHiddenNow() ) )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	SwTxtLineAccess aAccess( this );
/*N*/ 	SwParaPortion *pPara = aAccess.GetPara();
/*N*/ 	if( !pPara )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/     SwFrmSwapper aSwapper( this, sal_True );
/*N*/ 
/*N*/     SwTxtFrmLocker aLock(this);
/*N*/ 	SwTxtFormatInfo aInf( this, sal_False, sal_True );
/*N*/ 	if( 0 != aInf.MaxHyph() )	// 27483: MaxHyphen beachten!
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	SwTxtFormatter	aLine( this, &aInf );
/*N*/ 
/*N*/ 	// DropCaps sind zu kompliziert...
/*N*/ 	if( aLine.GetDropFmt() )
/*N*/         return sal_False;
/*N*/ 
/*N*/ 	xub_StrLen nStart = GetOfst();
/*N*/ 	const xub_StrLen nEnd = GetFollow()
/*N*/ 					  ? GetFollow()->GetOfst() : aInf.GetTxt().Len();
/*N*/ 	do
/*N*/ 	{	//DBG_LOOP;
/*N*/ 		nStart = aLine.FormatLine( nStart );
/*N*/ 		if( aInf.IsNewLine() || (!aInf.IsStop() && nStart < nEnd) )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 			aLine.Insert( new SwLineLayout() );
/*N*/ 	} while( aLine.Next() );
/*N*/ 
/*N*/     // Last exit: die Hoehen muessen uebereinstimmen.
/*N*/ 	Point aTopLeft( Frm().Pos() );
/*N*/ 	aTopLeft += Prt().Pos();
/*N*/ 	const SwTwips nNewHeight = aLine.Y() + aLine.GetLineHeight();
/*N*/ 	const SwTwips nOldHeight = aTopLeft.Y() + Prt().Height();
/*N*/ 	if( nNewHeight != nOldHeight && !IsUndersized() )
/*N*/ 	{
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*?*/ //	Achtung: Durch FormatLevel==12 kann diese Situation auftreten, don't panic!
/*?*/ //		ASSERT( nNewHeight == nOldHeight, "!FormatQuick: rosebud" );
/*?*/ #endif
/*?*/ 		xub_StrLen nStrt = GetOfst();
/*?*/ 		_InvalidateRange( SwCharRange( nStrt, nEnd - nStrt) );
/*?*/ 		return sal_False;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pFollow && nStart != ((SwTxtFrm*)pFollow)->GetOfst() )
/*N*/ 		return sal_False; // kann z.B. durch Orphans auftreten (35083,35081)
/*N*/ 
/*N*/ 	// Geschafft, wir sind durch ...
/*N*/ 
/*N*/ 	// Repaint setzen
/*N*/ 	pPara->GetRepaint()->Pos( aTopLeft );
/*N*/ 	pPara->GetRepaint()->SSize( Prt().SSize() );
/*N*/ 
/*N*/ 	// Reformat loeschen
/*N*/ 	*(pPara->GetReformat()) = SwCharRange();
/*N*/ 	*(pPara->GetDelta()) = 0;
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
