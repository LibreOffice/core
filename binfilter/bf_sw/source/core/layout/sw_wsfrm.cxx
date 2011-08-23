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

#ifndef _OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <bf_svtools/itemiter.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <bf_svx/brshitem.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _FMTORNT_HXX
#include <fmtornt.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _VIEWIMP_HXX
#include <viewimp.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _FMTFSIZE_HXX
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX
#include <fmtpdsc.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX
#include <fmtftn.hxx>
#endif
#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>
#endif
#ifndef _FLYFRMS_HXX
#include <flyfrms.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _BODYFRM_HXX
#include <bodyfrm.hxx>
#endif
#ifndef _DBG_LAY_HXX
#include <dbg_lay.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <bf_svx/frmdiritem.hxx>
#endif
namespace binfilter {




/*************************************************************************
|*
|*	SwFrm::SwFrm()
|*
|*	Ersterstellung		AK 12-Feb-1991
|*	Letzte Aenderung	MA 05. Apr. 94
|*
|*************************************************************************/

/*N*/ SwFrm::SwFrm( SwModify *pMod ) :
/*N*/ 	SwClient( pMod ),
/*N*/ 	pPrev( 0 ),
/*N*/ 	pNext( 0 ),
/*N*/ 	pUpper( 0 ),
/*N*/ 	pDrawObjs( 0 )
/*N*/ #ifdef DBG_UTIL
/*N*/ 	, nFrmId( SwFrm::nLastFrmId++ )
/*N*/ #endif
/*N*/ {
/*N*/ #ifdef DBG_UTIL
/*N*/     bFlag01 = bFlag02 = bFlag03 = bFlag04 = bFlag05 = 0;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	static USHORT nStopAt = USHRT_MAX;
/*N*/ 	if ( nFrmId == nStopAt )
/*N*/ 	{
/*N*/ 		int bla = 5;
/*N*/ 	}
/*N*/ #endif
/*N*/ #endif
/*N*/
/*N*/ 	ASSERT( pMod, "Kein Frameformat uebergeben." );
/*N*/     bInvalidR2L = bInvalidVert = 1;
/*N*/     bDerivedR2L = bDerivedVert = bRightToLeft = bVertical = bReverse = 0;
/*N*/     bValidPos = bValidPrtArea = bValidSize = bValidLineNum = bRetouche =
/*N*/     bFixSize = bColLocked = FALSE;
/*N*/     bCompletePaint = bInfInvalid = TRUE;
/*N*/ }

/*N*/ void SwFrm::CheckDir( UINT16 nDir, BOOL bVert, BOOL bOnlyBiDi, BOOL bBrowse )
/*N*/ {
/*N*/     if( FRMDIR_ENVIRONMENT == nDir || ( bVert && bOnlyBiDi ) )
/*N*/     {
/*N*/         bDerivedVert = 1;
/*N*/         if( FRMDIR_ENVIRONMENT == nDir )
/*N*/             bDerivedR2L = 1;
/*N*/         SetDirFlags( bVert );
/*N*/     }
/*N*/     else if( bVert )
/*N*/     {
/*?*/         bInvalidVert = 0;
/*?*/         if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir
/*?*/             || bBrowse )
/*?*/             bVertical = 0;
/*?*/         else
/*?*/             bVertical = 1;
/*?*/     }
/*?*/     else
/*?*/     {
/*?*/         bInvalidR2L = 0;
/*?*/         if( FRMDIR_HORI_RIGHT_TOP == nDir )
/*?*/             bRightToLeft = 1;
/*?*/         else
/*?*/             bRightToLeft = 0;
/*N*/     }
/*N*/ }

/*N*/ void SwFrm::CheckDirection( BOOL bVert )
/*N*/ {
/*N*/     if( bVert )
/*N*/     {
/*N*/         if( !IsHeaderFrm() && !IsFooterFrm() )
/*N*/         {
/*N*/             bDerivedVert = 1;
/*N*/             SetDirFlags( bVert );
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         bDerivedR2L = 1;
/*N*/         SetDirFlags( bVert );
/*N*/     }
/*N*/ }

/*N*/ void SwSectionFrm::CheckDirection( BOOL bVert )
/*N*/ {
/*N*/     const SwFrmFmt* pFmt = GetFmt();
/*N*/     if( pFmt )
/*N*/         CheckDir(((SvxFrameDirectionItem&)pFmt->GetAttr(RES_FRAMEDIR)).GetValue(),
/*N*/                     bVert, sal_True, pFmt->GetDoc()->IsBrowseMode() );
/*N*/     else
/*?*/         SwFrm::CheckDirection( bVert );
/*N*/ }


/*N*/ void SwTabFrm::CheckDirection( BOOL bVert )
/*N*/ {
/*N*/     const SwFrmFmt* pFmt = GetFmt();
/*N*/     if( pFmt )
/*N*/         CheckDir(((SvxFrameDirectionItem&)pFmt->GetAttr(RES_FRAMEDIR)).GetValue(),
/*N*/                     bVert, sal_True, pFmt->GetDoc()->IsBrowseMode() );
/*N*/     else
/*?*/         SwFrm::CheckDirection( bVert );
/*N*/ }

/*N*/ void SwTxtFrm::CheckDirection( BOOL bVert )
/*N*/ {
/*N*/     CheckDir( GetTxtNode()->GetSwAttrSet().GetFrmDir().GetValue(), bVert,
/*N*/               sal_True, GetTxtNode()->GetDoc()->IsBrowseMode() );
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::Modify()
|*
|*	Ersterstellung		AK 01-Mar-1991
|*	Letzte Aenderung	MA 20. Jun. 96
|*
|*************************************************************************/
/*N*/ void SwFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
/*N*/ {
/*N*/ 	BYTE nInvFlags = 0;
/*N*/
/*N*/ 	if( pNew && RES_ATTRSET_CHG == pNew->Which() )
/*N*/ 	{
/*N*/ 		SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
/*N*/ 		SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
/*N*/ 		while( TRUE )
/*N*/ 		{
/*N*/ 			_UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
/*N*/ 						 (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags );
/*N*/ 			if( aNIter.IsAtEnd() )
/*N*/ 				break;
/*N*/ 			aNIter.NextItem();
/*N*/ 			aOIter.NextItem();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		_UpdateAttr( pOld, pNew, nInvFlags );
/*N*/
/*N*/ 	if ( nInvFlags != 0 )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/ 		InvalidatePage( pPage );
/*N*/ 		if ( nInvFlags & 0x01 )
/*N*/ 		{
/*N*/ 			_InvalidatePrt();
/*N*/ 			if( !GetPrev() && IsTabFrm() && IsInSct() )
/*?*/ 				FindSctFrm()->_InvalidatePrt();
/*N*/ 		}
/*N*/ 		if ( nInvFlags & 0x02 )
/*N*/ 			_InvalidateSize();
/*N*/ 		if ( nInvFlags & 0x04 )
/*N*/ 			_InvalidatePos();
/*N*/ 		if ( nInvFlags & 0x08 )
/*N*/ 			SetCompletePaint();
/*N*/ 		SwFrm *pNxt;
/*N*/ 		if ( nInvFlags & 0x30 && 0 != (pNxt = GetNext()) )
/*N*/ 		{
/*N*/ 			pNxt->InvalidatePage( pPage );
/*N*/ 			if ( nInvFlags & 0x10 )
/*?*/ 				pNxt->_InvalidatePos();
/*N*/ 			if ( nInvFlags & 0x20 )
/*N*/ 				pNxt->SetCompletePaint();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
/*N*/ 						 BYTE &rInvFlags )
/*N*/ {
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 		case RES_BOX:
/*N*/ 		case RES_SHADOW:
/*N*/ 			Prepare( PREP_FIXSIZE_CHG );
/*N*/ 			// hier kein break !
/*N*/ 		case RES_LR_SPACE:
/*N*/ 		case RES_UL_SPACE:
/*N*/ 			rInvFlags |= 0x0B;
/*N*/ 			break;
/*N*/
/*M*/ 			case RES_HEADER_FOOTER_EAT_SPACING:
/*M*/ 				rInvFlags |= 0x03;
/*M*/ 				break;
/*M*/
/*N*/ 		case RES_BACKGROUND:
/*N*/ 			rInvFlags |= 0x28;
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_KEEP:
/*N*/ 			rInvFlags |= 0x04;
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_FRM_SIZE:
/*N*/ 			ReinitializeFrmSizeAttrFlags();
/*N*/ 			rInvFlags |= 0x13;
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_FMT_CHG:
/*N*/ 			rInvFlags |= 0x0F;
/*N*/ 			break;
/*N*/
/*?*/ 		case RES_COL:
/*?*/ 			ASSERT( FALSE, "Spalten fuer neuen FrmTyp?" );
/*?*/ 			break;
/*N*/
/*N*/ 		default:
/*N*/ 			/* do Nothing */;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  SwFrm::Prepare()
|*	  Ersterstellung	MA 13. Apr. 93
|*	  Letzte Aenderung	MA 26. Jun. 96
|*
|*************************************************************************/
/*N*/ void SwFrm::Prepare( const PrepareHint, const void *, BOOL )
/*N*/ {
/*N*/ 	/* Do nothing */
/*N*/ }

/*************************************************************************
|*
|*	  SwFrm::InvalidatePage()
|*	  Beschreibung:		Invalidiert die Seite, in der der Frm gerade steht.
|*		Je nachdem ob es ein Layout, Cntnt oder FlyFrm ist wird die Seite
|*		entsprechend Invalidiert.
|*	  Ersterstellung	MA 22. Jul. 92
|*	  Letzte Aenderung	MA 14. Oct. 94
|*
|*************************************************************************/
/*N*/ void SwFrm::InvalidatePage( const SwPageFrm *pPage ) const
/*N*/ {
/*N*/ #if (OSL_DEBUG_LEVEL > 1) && defined(DBG_UTIL)
/*N*/ 	static USHORT nStop = 0;
/*N*/ 	if ( nStop == GetFrmId() )
/*N*/ 	{
/*N*/ 		int bla = 5;
/*N*/ 	}
/*N*/ #endif
/*N*/
/*N*/ 	if ( !pPage )
/*N*/ 		pPage = FindPageFrm();
/*N*/
/*N*/ 	if ( pPage && pPage->GetUpper() )
/*N*/ 	{
/*N*/ 		if ( pPage->GetFmt()->GetDoc()->IsInDtor() )
/*?*/ 			return;
/*N*/
/*N*/ 		SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
/*N*/ 		const SwFlyFrm *pFly = FindFlyFrm();
/*N*/ 		if ( IsCntntFrm() )
/*N*/ 		{
/*N*/ 			if ( pRoot->IsTurboAllowed() )
/*N*/ 			{
/*N*/ 				// JP 21.09.95: wenn sich der ContentFrame 2 mal eintragen
/*N*/ 				//				will, kann es doch eine TurboAction bleiben.
/*N*/ 				//	ODER????
/*N*/ 				if ( !pRoot->GetTurbo() || this == pRoot->GetTurbo() )
/*N*/ 					pRoot->SetTurbo( (const SwCntntFrm*)this );
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pRoot->DisallowTurbo();
/*N*/ 					//Die Seite des Turbo koennte eine andere als die meinige
/*N*/ 					//sein, deshalb muss sie invalidiert werden.
/*N*/ 					const SwFrm *pTmp = pRoot->GetTurbo();
/*N*/ 					pRoot->ResetTurbo();
/*N*/ 					pTmp->InvalidatePage();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( !pRoot->GetTurbo() )
/*N*/ 			{
/*N*/ 				if ( pFly )
/*N*/ 				{	if( !pFly->IsLocked() )
/*N*/ 					{
/*N*/ 						if ( pFly->IsFlyInCntFrm() )
/*N*/ 						{	pPage->InvalidateFlyInCnt();
/*N*/ 							((SwFlyInCntFrm*)pFly)->InvalidateCntnt();
/*N*/ 							pFly->GetAnchor()->InvalidatePage();
/*N*/ 						}
/*N*/ 						else
/*N*/ 							pPage->InvalidateFlyCntnt();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					pPage->InvalidateCntnt();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pRoot->DisallowTurbo();
/*N*/ 			if ( pFly )
/*N*/ 			{   if( !pFly->IsLocked() )
/*N*/ 				{
/*N*/ 					if ( pFly->IsFlyInCntFrm() )
/*N*/ 					{	pPage->InvalidateFlyInCnt();
/*N*/ 						((SwFlyInCntFrm*)pFly)->InvalidateLayout();
/*N*/ 						pFly->GetAnchor()->InvalidatePage();
/*N*/ 					}
/*N*/ 					else
/*N*/ 						pPage->InvalidateFlyLayout();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pPage->InvalidateLayout();
/*N*/
/*N*/ 			if ( pRoot->GetTurbo() )
/*N*/ 			{	const SwFrm *pTmp = pRoot->GetTurbo();
/*N*/ 				pRoot->ResetTurbo();
/*N*/ 				pTmp->InvalidatePage();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pRoot->SetIdleFlags();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::ChgSize()
|*
|*	Ersterstellung		AK 15-Feb-1991
|*	Letzte Aenderung	MA 18. Nov. 98
|*
|*************************************************************************/
/*N*/ void SwFrm::ChgSize( const Size& aNewSize )
/*N*/ {
/*N*/     bFixSize = TRUE;
/*N*/ 	const Size aOldSize( Frm().SSize() );
/*N*/ 	if ( aNewSize == aOldSize )
/*N*/ 		return;
/*N*/
/*N*/ 	if ( GetUpper() )
/*N*/ 	{
/*N*/         SWRECTFN2( this )
/*N*/         SwRect aNew( Point(0,0), aNewSize );
/*N*/         (aFrm.*fnRect->fnSetWidth)( (aNew.*fnRect->fnGetWidth)() );
/*N*/         long nNew = (aNew.*fnRect->fnGetHeight)();
/*N*/         long nDiff = nNew - (aFrm.*fnRect->fnGetHeight)();
/*N*/ 		if( nDiff )
/*N*/ 		{
/*N*/             if ( GetUpper()->IsFtnBossFrm() && HasFixSize() &&
/*N*/ 				 NA_GROW_SHRINK !=
/*N*/ 				 ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this ) )
/*N*/ 			{
/*N*/                 (aFrm.*fnRect->fnSetHeight)( nNew );
/*N*/                 SwTwips nReal = ((SwLayoutFrm*)this)->AdjustNeighbourhood(nDiff);
/*N*/ 				if ( nReal != nDiff )
/*N*/                     (aFrm.*fnRect->fnSetHeight)( nNew - nDiff + nReal );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/                 // OD 24.10.2002 #97265# - grow/shrink not for neighbour frames
/*N*/                 // NOTE: neighbour frames are cell and column frames.
/*N*/                 if ( !bNeighb )
/*N*/                 {
/*N*/                     if ( nDiff > 0 )
/*N*/                         Grow( nDiff );
/*N*/                     else
/*N*/                         Shrink( -nDiff );
/*N*/
/*N*/                     if ( GetUpper() && (aFrm.*fnRect->fnGetHeight)() != nNew )
/*N*/                         GetUpper()->_InvalidateSize();
/*N*/                 }
/*N*/
/*N*/                 // Auch wenn das Grow/Shrink noch nicht die gewuenschte Breite eingestellt hat,
/*N*/                 // wie z.B. beim Aufruf durch ChgColumns, um die Spaltenbreiten einzustellen,
/*N*/                 // wird die Breite jetzt gesetzt.
/*N*/                 (aFrm.*fnRect->fnSetHeight)( nNew );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/         aFrm.SSize( aNewSize );
/*N*/
/*N*/ 	if ( Frm().SSize() != aOldSize )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/ 		if ( GetNext() )
/*N*/ 		{
/*N*/ 			GetNext()->_InvalidatePos();
/*N*/ 			GetNext()->InvalidatePage( pPage );
/*N*/ 		}
/*N*/         if( IsLayoutFrm() )
/*N*/         {
/*N*/             if( IsRightToLeft() )
/*N*/                 _InvalidatePos();
/*N*/             if( ((SwLayoutFrm*)this)->Lower() )
/*N*/                 ((SwLayoutFrm*)this)->Lower()->_InvalidateSize();
/*N*/         }
/*N*/ 		_InvalidatePrt();
/*N*/ 		_InvalidateSize();
/*N*/ 		InvalidatePage( pPage );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::InsertBefore()
|*
|*	Beschreibung		SwFrm wird in eine bestehende Struktur eingefuegt
|* 						Eingefuegt wird unterhalb des Parent und entweder
|* 						vor pBehind oder am Ende der Kette wenn pBehind
|* 						leer ist.
|*	Letzte Aenderung	MA 06. Aug. 99
|*
|*************************************************************************/
/*N*/ void SwFrm::InsertBefore( SwLayoutFrm* pParent, SwFrm* pBehind )
/*N*/ {
/*N*/ 	ASSERT( pParent, "Kein Parent fuer Insert." );
/*N*/ 	ASSERT( (!pBehind || (pBehind && pParent == pBehind->GetUpper())),
/*N*/ 			"Framebaum inkonsistent." );
/*N*/
/*N*/ 	pUpper = pParent;
/*N*/ 	pNext = pBehind;
/*N*/ 	if( pBehind )
/*N*/ 	{	//Einfuegen vor pBehind.
/*N*/ 		if( 0 != (pPrev = pBehind->pPrev) )
/*N*/ 			pPrev->pNext = this;
/*N*/ 		else
/*N*/ 			pUpper->pLower = this;
/*N*/ 		pBehind->pPrev = this;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{	//Einfuegen am Ende, oder als ersten Node im Unterbaum
/*N*/ 		pPrev = pUpper->Lower();
/*N*/ 		if ( pPrev )
/*N*/ 		{
/*N*/ 			while( pPrev->pNext )
/*N*/ 				pPrev = pPrev->pNext;
/*N*/ 			pPrev->pNext = this;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pUpper->pLower = this;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::InsertBehind()
|*
|*	Beschreibung		SwFrm wird in eine bestehende Struktur eingefuegt
|* 						Eingefuegt wird unterhalb des Parent und entweder
|* 						hinter pBefore oder am Anfang der Kette wenn pBefore
|* 						leer ist.
|*	Letzte Aenderung	MA 06. Aug. 99
|*
|*************************************************************************/
/*N*/ void SwFrm::InsertBehind( SwLayoutFrm *pParent, SwFrm *pBefore )
/*N*/ {
/*N*/ 	ASSERT( pParent, "Kein Parent fuer Insert." );
/*N*/ 	ASSERT( (!pBefore || (pBefore && pParent == pBefore->GetUpper())),
/*N*/ 			"Framebaum inkonsistent." );
/*N*/
/*N*/ 	pUpper = pParent;
/*N*/ 	pPrev = pBefore;
/*N*/ 	if ( pBefore )
/*N*/ 	{
/*N*/ 		//Einfuegen hinter pBefore
/*N*/ 		if ( 0 != (pNext = pBefore->pNext) )
/*N*/ 			pNext->pPrev = this;
/*N*/ 		pBefore->pNext = this;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//Einfuegen am Anfang der Kette
/*N*/ 		pNext = pParent->Lower();
/*N*/ 		if ( pParent->Lower() )
/*N*/ 			pParent->Lower()->pPrev = this;
/*N*/ 		pParent->pLower = this;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::InsertGroup()
|*
|*	Beschreibung		Eine Kette von SwFrms wird in eine bestehende Struktur
|* 						eingefuegt
|*	Letzte Aenderung	AMA 9. Dec. 97
|*
|*  Bisher wird dies genutzt, um einen SectionFrame, der ggf. schon Geschwister
|*	mit sich bringt, in eine bestehende Struktur einzufuegen.
|*
|*  Wenn man den dritten Parameter als NULL uebergibt, entspricht
|*  diese Methode dem SwFrm::InsertBefore(..), nur eben mit Geschwistern.
|*
|*  Wenn man einen dritten Parameter uebergibt, passiert folgendes:
|*  this wird pNext von pParent,
|*  pSct wird pNext vom Letzten der this-Kette,
|*  pBehind wird vom pParent an den pSct umgehaengt.
|*	Dies dient dazu: ein SectionFrm (this) wird nicht als
|*	Kind an einen anderen SectionFrm (pParent) gehaengt, sondern pParent
|*	wird in zwei Geschwister aufgespalten (pParent+pSct) und this dazwischen
|*  eingebaut.
|*
|*************************************************************************/

/*************************************************************************
|*
|*	SwFrm::Remove()
|*
|*	Ersterstellung		AK 01-Mar-1991
|*	Letzte Aenderung	MA 07. Dec. 95
|*
|*************************************************************************/
/*N*/ void SwFrm::Remove()
/*N*/ {
/*N*/ 	ASSERT( pUpper, "Removen ohne Upper?" );
/*N*/
/*N*/ 	if( pPrev )
/*N*/ 		// einer aus der Mitte wird removed
/*N*/ 		pPrev->pNext = pNext;
/*N*/ 	else
/*N*/ 	{	// der erste in einer Folge wird removed
/*N*/ 		ASSERT( pUpper->pLower == this, "Layout inkonsistent." );
/*N*/ 		pUpper->pLower = pNext;
/*N*/ 	}
/*N*/ 	if( pNext )
/*N*/ 		pNext->pPrev = pPrev;
/*N*/
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/     // inform accessibility API
/*N*/     if ( IsInTab() )
/*N*/     {
/*N*/         SwTabFrm* pTableFrm = FindTabFrm();
/*N*/         if( pTableFrm != NULL  &&
/*N*/             pTableFrm->IsAccessibleFrm()  &&
/*N*/             pTableFrm->GetFmt() != NULL )
/*N*/         {
/*N*/             SwRootFrm *pRootFrm = pTableFrm->FindRootFrm();
/*N*/             if( pRootFrm != NULL &&
/*N*/                 pRootFrm->IsAnyShellAccessible() )
/*N*/             {
/*N*/                 ViewShell* pShell = pRootFrm->GetCurrShell();
/*N*/                 if( pShell != NULL )
/*?*/                    {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001  pShell->Imp()->DisposeAccessibleFrm( pTableFrm, sal_True );
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ #endif
/*N*/
/*N*/ 	// Verbindung kappen.
/*N*/ 	pNext  = pPrev  = 0;
/*N*/ 	pUpper = 0;
/*N*/ }
/*************************************************************************
|*
|*	SwCntntFrm::Paste()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 09. Sep. 98
|*
|*************************************************************************/
/*N*/ void SwCntntFrm::Paste( SwFrm* pParent, SwFrm* pSibling)
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
/*N*/ 	SwPageFrm *pPage = FindPageFrm();
/*N*/ 	_InvalidateAll();
/*N*/ 	InvalidatePage( pPage );
/*N*/ 
/*N*/ 	if( pPage )
/*N*/ 	{
/*N*/ 		pPage->InvalidateSpelling();
/*N*/ 		pPage->InvalidateAutoCompleteWords();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( GetNext() )
/*N*/ 	{
/*N*/ 		SwFrm* pNxt = GetNext();
/*N*/ 		pNxt->_InvalidatePrt();
/*N*/ 		pNxt->_InvalidatePos();
/*N*/ 		pNxt->InvalidatePage( pPage );
/*N*/ 		if( pNxt->IsSctFrm() )
/*?*/ 			pNxt = ((SwSectionFrm*)pNxt)->ContainsCntnt();
/*N*/ 		if( pNxt && pNxt->IsTxtFrm() && pNxt->IsInFtn() )
/*?*/ 			pNxt->Prepare( PREP_FTN, 0, FALSE );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( Frm().Height() )
/*N*/         pParent->Grow( Frm().Height() PHEIGHT );
/*N*/ 
/*N*/ 	if ( Frm().Width() != pParent->Prt().Width() )
/*N*/ 		Prepare( PREP_FIXSIZE_CHG );
/*N*/ 
/*N*/ 	if ( GetPrev() )
/*N*/ 	{
/*N*/ 		if ( IsFollow() )
/*N*/ 			//Ich bin jetzt direkter Nachfolger meines Masters geworden
/*N*/ 			((SwCntntFrm*)GetPrev())->Prepare( PREP_FOLLOW_FOLLOWS );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if ( GetPrev()->Frm().Height() !=
/*N*/ 				 GetPrev()->Prt().Height() + GetPrev()->Prt().Top() )
/*N*/ 				//Umrandung zu beruecksichtigen?
/*N*/ 				GetPrev()->_InvalidatePrt();
/*N*/             // OD 18.02.2003 #104989# - force complete paint of previous frame,
/*N*/             // if frame is inserted at the end of a section frame, in order to
/*N*/             // get subsidiary lines repainted for the section.
/*N*/             if ( pParent->IsSctFrm() && !GetNext() )
/*N*/             {
/*N*/                 // force complete paint of previous frame, if new inserted frame
/*N*/                 // in the section is the last one.
/*N*/                 GetPrev()->SetCompletePaint();
/*N*/             }
/*N*/ 			GetPrev()->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( IsInFtn() )
/*N*/ 	{
/*N*/ 		SwFrm* pFrm = GetIndPrev();
/*N*/ 		if( pFrm && pFrm->IsSctFrm() )
/*?*/ 			pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 		if( pFrm )
/*N*/ 			pFrm->Prepare( PREP_QUOVADIS, 0, FALSE );
/*N*/ 		if( !GetNext() )
/*N*/ 		{
/*N*/ 			pFrm = FindFtnFrm()->GetNext();
/*N*/ 			if( pFrm && 0 != (pFrm=((SwLayoutFrm*)pFrm)->ContainsAny()) )
/*?*/ 				pFrm->_InvalidatePrt();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	_InvalidateLineNum();
/*N*/ 	SwFrm *pNxt = FindNextCnt();
/*N*/ 	if ( pNxt  )
/*N*/ 	{
/*N*/ 		while ( pNxt && pNxt->IsInTab() )
/*N*/ 		{
/*N*/ 			if( 0 != (pNxt = pNxt->FindTabFrm()) )
/*N*/ 				pNxt = pNxt->FindNextCnt();
/*N*/ 		}
/*N*/ 		if ( pNxt )
/*N*/ 		{
/*N*/ 			pNxt->_InvalidateLineNum();
/*N*/ 			if ( pNxt != GetNext() )
/*N*/ 				pNxt->InvalidatePage();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwCntntFrm::Cut()
|*
|*	Ersterstellung		AK 14-Feb-1991
|*	Letzte Aenderung	MA 09. Sep. 98
|*
|*************************************************************************/
/*N*/ void SwCntntFrm::Cut()
/*N*/ {
/*N*/ 	ASSERT( GetUpper(), "Cut ohne Upper()." );
/*N*/ 
/*N*/ 	SwPageFrm *pPage = FindPageFrm();
/*N*/ 	InvalidatePage( pPage );
/*N*/ 	SwFrm *pFrm = GetIndPrev();
/*N*/ 	if( pFrm )
/*N*/ 	{
/*N*/ 		if( pFrm->IsSctFrm() )
/*N*/ 			pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 		if ( pFrm && pFrm->IsCntntFrm() )
/*N*/ 		{
/*N*/ 			pFrm->_InvalidatePrt();
/*N*/ 			if( IsInFtn() )
/*N*/ 				pFrm->Prepare( PREP_QUOVADIS, 0, FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwFrm *pNxt = FindNextCnt();
/*N*/ 	if ( pNxt )
/*N*/ 	{
/*N*/ 		while ( pNxt && pNxt->IsInTab() )
/*N*/ 		{
/*N*/ 			if( 0 != (pNxt = pNxt->FindTabFrm()) )
/*N*/ 				pNxt = pNxt->FindNextCnt();
/*N*/ 		}
/*N*/ 		if ( pNxt )
/*N*/ 		{
/*N*/ 			pNxt->_InvalidateLineNum();
/*N*/ 			if ( pNxt != GetNext() )
/*N*/ 				pNxt->InvalidatePage();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( 0 != (pFrm = GetIndNext()) )
/*N*/ 	{	//Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
/*N*/ 		//berechnet, der ist jetzt, wo er der erste wird obsolet bzw. anders.
/*N*/ 		pFrm->_InvalidatePrt();
/*N*/ 		pFrm->_InvalidatePos();
/*N*/ 		pFrm->InvalidatePage( pPage );
/*N*/ 		if( pFrm->IsSctFrm() )
/*N*/ 		{
/*N*/ 			pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 			if( pFrm )
/*N*/ 			{
/*N*/ 				pFrm->_InvalidatePrt();
/*N*/ 				pFrm->_InvalidatePos();
/*N*/ 				pFrm->InvalidatePage( pPage );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( pFrm && IsInFtn() )
/*?*/ 			pFrm->Prepare( PREP_ERGOSUM, 0, FALSE );
/*N*/ 		if( IsInSct() && !GetPrev() )
/*N*/ 		{
/*N*/ 			SwSectionFrm* pSct = FindSctFrm();
/*N*/ 			if( !pSct->IsFollow() )
/*N*/ 			{
/*N*/ 				pSct->_InvalidatePrt();
/*N*/ 				pSct->InvalidatePage( pPage );
/*N*/ 			}
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
/*N*/ 			pFrm->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		//Wenn ich der einzige CntntFrm in meinem Upper bin (war), so muss
/*N*/ 		//er die Retouche uebernehmen.
/*N*/ 		//Ausserdem kann eine Leerseite entstanden sein.
/*N*/ 		else
/*N*/ 		{	SwRootFrm *pRoot = FindRootFrm();
/*N*/ 			if ( pRoot )
/*N*/ 			{
/*N*/ 				pRoot->SetSuperfluous();
/*N*/ 				GetUpper()->SetCompletePaint();
/*N*/ 				GetUpper()->InvalidatePage( pPage );
/*N*/ 			}
/*N*/ 			if( IsInSct() )
/*N*/ 			{
/*N*/ 				SwSectionFrm* pSct = FindSctFrm();
/*N*/ 				if( !pSct->IsFollow() )
/*N*/ 				{
/*?*/ 					pSct->_InvalidatePrt();
/*?*/ 					pSct->InvalidatePage( pPage );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	//Erst removen, dann Upper Shrinken.
/*N*/ 	SwLayoutFrm *pUp = GetUpper();
/*N*/ 	Remove();
/*N*/ 	if ( pUp )
/*N*/ 	{
/*N*/ 		SwSectionFrm *pSct;
/*N*/ 		if ( !pUp->Lower() && ( ( pUp->IsFtnFrm() && !pUp->IsColLocked() )
/*N*/ 			|| ( pUp->IsInSct() && !(pSct = pUp->FindSctFrm())->ContainsCntnt() ) ) )
/*N*/ 		{
/*N*/ 			if ( pUp->GetUpper() )
/*N*/ 			{
/*N*/ 				if( pUp->IsFtnFrm() )
/*N*/ 				{
/*?*/ 					if( pUp->GetNext() && !pUp->GetPrev() )
/*?*/ 					{
/*?*/ 						SwFrm* pTmp = ((SwLayoutFrm*)pUp->GetNext())->ContainsAny();
/*?*/ 						if( pTmp )
/*?*/ 							pTmp->_InvalidatePrt();
/*?*/ 					}
/*?*/ 					pUp->Cut();
/*?*/ 					delete pUp;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( pSct->IsColLocked() || !pSct->IsInFtn() )
/*N*/ 					{
/*N*/ 						pSct->DelEmpty( FALSE );
/*N*/ 					// Wenn ein gelockter Bereich nicht geloescht werden darf,
/*N*/ 					// so ist zumindest seine Groesse durch das Entfernen seines
/*N*/ 					// letzten Contents ungueltig geworden.
/*N*/ 						pSct->_InvalidateSize();
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*?*/ 						pSct->DelEmpty( TRUE );
/*?*/ 						delete pSct;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/         else
/*N*/         {
/*N*/             SWRECTFN( this )
/*N*/             long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/             if( nFrmHeight )
/*N*/                 pUp->Shrink( nFrmHeight );
/*N*/         }
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::Paste()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 23. Feb. 94
|*
|*************************************************************************/
/*N*/ void SwLayoutFrm::Paste( SwFrm* pParent, SwFrm* pSibling)
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
    // OD 24.10.2002 #103517# - correct setting of variable <fnRect>
    // <fnRect> is used for the following:
    // (1) To invalidate the frame's size, if its size, which has to be the
    //      same as its upper/parent, differs from its upper's/parent's.
    // (2) To adjust/grow the frame's upper/parent, if it has a dimension in its
    //      size, which is not determined by its upper/parent.
    // Which size is which depends on the frame type and the layout direction
    // (vertical or horizontal).
    // There are the following cases:
    // (A) Header and footer frames both in vertical and in horizontal layout
    //      have to size the width to the upper/parent. A dimension in the height
    //      has to cause a adjustment/grow of the upper/parent.
    //      --> <fnRect> = fnRectHori
    // (B) Cell and column frames in vertical layout, the width has to be the
    //          same as upper/parent and a dimension in height causes adjustment/grow
    //          of the upper/parent.
    //          --> <fnRect> = fnRectHori
    //      in horizontal layout the other way around
    //          --> <fnRect> = fnRectVert
    // (C) Other frames in vertical layout, the height has to be the
    //          same as upper/parent and a dimension in width causes adjustment/grow
    //          of the upper/parent.
    //          --> <fnRect> = fnRectVert
    //      in horizontal layout the other way around
    //          --> <fnRect> = fnRectHori
    //SwRectFn fnRect = IsVertical() ? fnRectHori : fnRectVert;
/*N*/     SwRectFn fnRect;
/*N*/     if ( IsHeaderFrm() || IsFooterFrm() )
/*N*/         fnRect = fnRectHori;
/*N*/     else if ( IsCellFrm() || IsColumnFrm() )
/*N*/         fnRect = GetUpper()->IsVertical() ? fnRectHori : fnRectVert;
/*N*/     else
/*N*/         fnRect = GetUpper()->IsVertical() ? fnRectVert : fnRectHori;
/*N*/ 
/*N*/     if( (Frm().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)())
/*N*/ 		_InvalidateSize();
/*N*/ 	_InvalidatePos();
/*N*/ 	const SwPageFrm *pPage = FindPageFrm();
/*N*/ 	InvalidatePage( pPage );
/*N*/ 	SwFrm *pFrm;
/*N*/ 	if( !IsColumnFrm() )
/*N*/ 	{
/*N*/ 		if( 0 != ( pFrm = GetIndNext() ) )
/*N*/ 		{
/*N*/ 			pFrm->_InvalidatePos();
/*N*/ 			if( IsInFtn() )
/*N*/ 			{
/*N*/ 				if( pFrm->IsSctFrm() )
/*?*/ 					pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 				if( pFrm )
/*N*/ 					pFrm->Prepare( PREP_ERGOSUM, 0, FALSE );
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( IsInFtn() && 0 != ( pFrm = GetIndPrev() ) )
/*N*/ 		{
/*N*/ 			if( pFrm->IsSctFrm() )
/*?*/ 				pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 			if( pFrm )
/*N*/ 				pFrm->Prepare( PREP_QUOVADIS, 0, FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     if( (Frm().*fnRect->fnGetHeight)() )
/*N*/ 	{
/*N*/ 		// AdjustNeighbourhood wird jetzt auch in Spalten aufgerufen,
/*N*/ 		// die sich nicht in Rahmen befinden
/*N*/ 		BYTE nAdjust = GetUpper()->IsFtnBossFrm() ?
/*N*/ 				((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
/*N*/ 				: NA_GROW_SHRINK;
/*N*/         SwTwips nGrow = (Frm().*fnRect->fnGetHeight)();
/*N*/ 		if( NA_ONLY_ADJUST == nAdjust )
/*N*/ 			AdjustNeighbourhood( nGrow );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwTwips nReal = 0;
/*N*/ 			if( NA_ADJUST_GROW == nAdjust )
/*?*/ 				nReal = AdjustNeighbourhood( nGrow );
/*N*/ 			if( nReal < nGrow )
/*N*/                 nReal += pParent->Grow( nGrow - nReal );
/*N*/ 			if( NA_GROW_ADJUST == nAdjust && nReal < nGrow )
/*?*/ 				AdjustNeighbourhood( nGrow - nReal );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::Cut()
|*
|*	Ersterstellung		MA 23. Feb. 94
|*	Letzte Aenderung	MA 23. Feb. 94
|*
|*************************************************************************/
/*N*/ void SwLayoutFrm::Cut()
/*N*/ {
/*N*/ 	if ( GetNext() )
/*N*/ 		GetNext()->_InvalidatePos();
/*N*/ 
/*N*/     SWRECTFN( this )
/*N*/     SwTwips nShrink = (Frm().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/ 	//Erst removen, dann Upper Shrinken.
/*N*/ 	SwLayoutFrm *pUp = GetUpper();
/*N*/ 
/*N*/ 	// AdjustNeighbourhood wird jetzt auch in Spalten aufgerufen,
/*N*/ 	// die sich nicht in Rahmen befinden
/*N*/ 
/*N*/ 	// Remove must not be called before a AdjustNeighbourhood, but it has to
/*N*/ 	// be called before the upper-shrink-call, if the upper-shrink takes care
/*N*/ 	// of his content
/*N*/ 	if ( pUp && nShrink )
/*N*/ 	{
/*N*/ 		if( pUp->IsFtnBossFrm() )
/*N*/ 		{
/*N*/ 			BYTE nAdjust= ((SwFtnBossFrm*)pUp)->NeighbourhoodAdjustment( this );
/*N*/ 			if( NA_ONLY_ADJUST == nAdjust )
/*N*/ 				AdjustNeighbourhood( -nShrink );
/*N*/ 			else
/*N*/ 			{
/*?*/ 				SwTwips nReal = 0;
/*?*/ 				if( NA_ADJUST_GROW == nAdjust )
/*?*/ 					nReal = -AdjustNeighbourhood( -nShrink );
/*?*/ 				if( nReal < nShrink )
/*?*/ 				{
/*?*/                     SwTwips nOldHeight = (Frm().*fnRect->fnGetHeight)();
/*?*/                     (Frm().*fnRect->fnSetHeight)( 0 );
/*?*/                     nReal += pUp->Shrink( nShrink - nReal );
/*?*/                     (Frm().*fnRect->fnSetHeight)( nOldHeight );
/*?*/ 				}
/*?*/ 				if( NA_GROW_ADJUST == nAdjust && nReal < nShrink )
/*?*/ 					AdjustNeighbourhood( nReal - nShrink );
/*N*/ 			}
/*N*/ 			Remove();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			Remove();
/*N*/             pUp->Shrink( nShrink );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		Remove();
/*N*/ 
/*N*/ 	if( pUp && !pUp->Lower() )
/*N*/ 	{
/*N*/ 		pUp->SetCompletePaint();
/*N*/ 		pUp->InvalidatePage();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::Grow()
|*
|*	Ersterstellung		AK 19-Feb-1991
|*	Letzte Aenderung	MA 05. May. 94
|*
|*************************************************************************/
/*N*/ SwTwips SwFrm::Grow( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/ 	ASSERT( nDist >= 0, "Negatives Wachstum?" );
/*N*/ 
/*N*/ 	PROTOCOL_ENTER( this, bTst ? PROT_GROW_TST : PROT_GROW, 0, &nDist )
/*N*/ 
/*N*/ 	if ( nDist )
/*N*/ 	{
/*N*/         SWRECTFN( this )
/*N*/ 
/*N*/         SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/         if( nPrtHeight > 0 && nDist > (LONG_MAX - nPrtHeight) )
/*N*/             nDist = LONG_MAX - nPrtHeight;
/*N*/ 
/*N*/ 		if ( IsFlyFrm() )
/*N*/             return ((SwFlyFrm*)this)->_Grow( nDist, bTst );
/*N*/ 		else if( IsSctFrm() )
/*N*/             return ((SwSectionFrm*)this)->_Grow( nDist, bTst );
/*N*/ 		else
/*N*/ 		{
/*N*/             const SwTwips nReal = GrowFrm( nDist, bTst, bInfo );
/*N*/ 			if( !bTst )
/*N*/             {
/*N*/                 nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/                 (Prt().*fnRect->fnSetHeight)( nPrtHeight +
/*N*/                         ( IsCntntFrm() ? nDist : nReal ) );
/*N*/             }
/*N*/ 			return nReal;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0L;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::Shrink()
|*
|*	Ersterstellung		AK 14-Feb-1991
|*	Letzte Aenderung	MA 05. May. 94
|*
|*************************************************************************/
/*N*/ SwTwips SwFrm::Shrink( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/ 	ASSERT( nDist >= 0, "Negative Verkleinerung?" );
/*N*/ 
/*N*/ 	PROTOCOL_ENTER( this, bTst ? PROT_SHRINK_TST : PROT_SHRINK, 0, &nDist )
/*N*/ 
/*N*/ 	if ( nDist )
/*N*/ 	{
/*N*/ 		if ( IsFlyFrm() )
/*N*/             return ((SwFlyFrm*)this)->_Shrink( nDist, bTst );
/*N*/ 		else if( IsSctFrm() )
/*N*/             return ((SwSectionFrm*)this)->_Shrink( nDist, bTst );
/*N*/ 		else
/*N*/ 		{
/*N*/             SWRECTFN( this )
/*N*/             SwTwips nReal = (Frm().*fnRect->fnGetHeight)();
/*N*/             ShrinkFrm( nDist, bTst, bInfo );
/*N*/             nReal -= (Frm().*fnRect->fnGetHeight)();
/*N*/ 			if( !bTst )
/*N*/             {
/*N*/                 SwTwips nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/                 (Prt().*fnRect->fnSetHeight)( nPrtHeight -
/*N*/                         ( IsCntntFrm() ? nDist : nReal ) );
/*N*/             }
/*N*/ 			return nReal;
/*N*/ 		}
/*N*/ 	}
/*?*/ 	return 0L;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::AdjustNeighbourhood()
|*
|*	Beschreibung		Wenn sich die Groesse eines Frm's direkt unterhalb
|* 		eines Fussnotenbosses (Seite/Spalte) veraendert hat, so muss dieser
|*  	"Normalisiert" werden.
|* 		Es gibt dort immer einen Frame, der den "maximal moeglichen" Raum
|*		einnimmt (der Frame, der den Body.Text enhaelt) und keinen oder
|* 		mehrere Frames die den Platz einnehmen den sie halt brauchen
|* 		(Kopf-/Fussbereich, Fussnoten).
|*		Hat sich einer der Frames veraendert, so muss der Body-Text-Frame
|*		entsprechen wachsen oder schrumpfen; unabhaegig davon, dass er fix ist.
|* 		!! Ist es moeglich dies allgemeiner zu loesen, also nicht auf die
|* 		Seite beschraenkt und nicht auf einen Speziellen Frame, der den
|* 		maximalen Platz einnimmt (gesteuert ueber Attribut FrmSize)? Probleme:
|* 		Was ist wenn mehrere Frames nebeneinander stehen, die den maximalen
|* 		Platz einnehmen?
|*		Wie wird der Maximale Platz berechnet?
|* 		Wie klein duerfen diese Frames werden?
|*
|* 		Es wird auf jeden Fall nur so viel Platz genehmigt, dass ein
|* 		Minimalwert fuer die Hoehe des Bodys nicht unterschritten wird.
|*
|*	Parameter: nDiff ist der Betrag, um den Platz geschaffen werden muss
|*
|*	Ersterstellung		MA 07. May. 92
|*	Letzte Aenderung	AMA 02. Nov. 98
|*
|*************************************************************************/
/*N*/ SwTwips SwFrm::AdjustNeighbourhood( SwTwips nDiff, BOOL bTst )
/*N*/ {
/*N*/ 	PROTOCOL_ENTER( this, PROT_ADJUSTN, 0, &nDiff );
/*N*/ 
/*N*/ 	if ( !nDiff || !GetUpper()->IsFtnBossFrm() ) // nur innerhalb von Seiten/Spalten
/*?*/ 		return 0L;
/*N*/ 
/*N*/ 	FASTBOOL bBrowse = GetUpper()->GetFmt()->GetDoc()->IsBrowseMode();
/*N*/ 
/*N*/ 	//Der (Page)Body veraendert sich nur im BrowseMode, aber nicht wenn er
/*N*/ 	//Spalten enthaelt.
/*N*/ 	if ( IsPageBodyFrm() && (!bBrowse ||
/*N*/ 		  (((SwLayoutFrm*)this)->Lower() &&
/*N*/ 		   ((SwLayoutFrm*)this)->Lower()->IsColumnFrm())) )
/*N*/ 		return 0L;
/*N*/ 
/*N*/ 	//In der BrowseView kann der PageFrm selbst ersteinmal einiges von den
/*N*/ 	//Wuenschen abfangen.
/*N*/ 	long nBrowseAdd = 0;
/*N*/ 	if ( bBrowse && GetUpper()->IsPageFrm() ) // nur (Page)BodyFrms
/*N*/ 	{
/*N*/ 		ViewShell *pSh = GetShell();
/*N*/ 		SwLayoutFrm *pUp = GetUpper();
/*N*/ 		long nChg;
/*N*/ 		const long nUpPrtBottom = pUp->Frm().Height() -
/*N*/ 								  pUp->Prt().Height() - pUp->Prt().Top();
/*N*/ 		SwRect aInva( pUp->Frm() );
/*N*/ 		if ( pSh )
/*N*/ 		{
/*N*/ 			aInva.Pos().X() = pSh->VisArea().Left();
/*N*/ 			aInva.Width( pSh->VisArea().Width() );
/*N*/ 		}
/*N*/ 		if ( nDiff > 0 )
/*N*/ 		{
/*N*/ 			nChg = BROWSE_HEIGHT - pUp->Frm().Height();
/*N*/ 			nChg = Min( nDiff, nChg );
/*N*/ 
/*N*/ 			if ( !IsBodyFrm() )
/*N*/ 			{
/*?*/ 				SetCompletePaint();
/*?*/ 				if ( !pSh || pSh->VisArea().Height() >= pUp->Frm().Height() )
/*?*/ 				{
/*?*/ 					//Ersteinmal den Body verkleinern. Der waechst dann schon
/*?*/ 					//wieder.
/*?*/ 					SwFrm *pBody = ((SwFtnBossFrm*)pUp)->FindBodyCont();
/*?*/ 					const long nTmp = nChg - pBody->Prt().Height();
/*?*/ 					if ( !bTst )
/*?*/ 					{
/*?*/ 						pBody->Frm().Height(Max( 0L, pBody->Frm().Height() - nChg ));
/*?*/ 						pBody->_InvalidatePrt();
/*?*/ 						pBody->_InvalidateSize();
/*?*/ 						if ( pBody->GetNext() )
/*?*/ 							pBody->GetNext()->_InvalidatePos();
/*?*/ 						if ( !IsHeaderFrm() )
/*?*/ 							pBody->SetCompletePaint();
/*?*/ 					}
/*?*/ 					nChg = nTmp <= 0 ? 0 : nTmp;
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/ 			const long nTmp = nUpPrtBottom + 20;
/*N*/ 			aInva.Top( aInva.Bottom() - nTmp );
/*N*/ 			aInva.Height( nChg + nTmp );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			//Die Seite kann bis auf 0 schrumpfen. Die erste Seite bleibt
/*N*/ 			//mindestens so gross wie die VisArea.
/*N*/ 			nChg = nDiff;
/*N*/ 			long nInvaAdd = 0;
/*N*/ 			if ( pSh && !pUp->GetPrev() &&
/*N*/ 				 pUp->Frm().Height() + nDiff < pSh->VisArea().Height() )
/*N*/ 			{
/*?*/ 				//Das heisst aber wiederum trotzdem, das wir geeignet invalidieren
/*?*/ 				//muessen.
/*?*/ 				nChg = pSh->VisArea().Height() - pUp->Frm().Height();
/*?*/ 				nInvaAdd = -(nDiff - nChg);
/*N*/ 			}
/*N*/ 
/*N*/ 			//Invalidieren inklusive unterem Rand.
/*N*/ 			long nBorder = nUpPrtBottom + 20;
/*N*/ 			nBorder -= nChg;
/*N*/ 			aInva.Top( aInva.Bottom() - (nBorder+nInvaAdd) );
/*N*/ 			if ( !IsBodyFrm() )
/*N*/ 			{
/*?*/ 				SetCompletePaint();
/*?*/ 				if ( !IsHeaderFrm() )
/*?*/ 					((SwFtnBossFrm*)pUp)->FindBodyCont()->SetCompletePaint();
/*N*/ 			}
/*N*/ 			//Wegen der Rahmen die Seite invalidieren. Dadurch wird die Seite
/*N*/ 			//wieder entsprechend gross wenn ein Rahmen nicht passt. Das
/*N*/ 			//funktioniert anderfalls nur zufaellig fuer absatzgebundene Rahmen
/*N*/ 			//(NotifyFlys).
/*N*/ 			pUp->InvalidateSize();
/*N*/ 		}
/*N*/ 		if ( !bTst )
/*N*/ 		{
/*N*/ 			//Unabhaengig von nChg
/*N*/ 			if ( pSh && aInva.HasArea() && pUp->GetUpper() )
/*?*/ 				pSh->InvalidateWindows( aInva );
/*N*/ 		}
/*N*/ 		if ( !bTst && nChg )
/*N*/ 		{
/*N*/ 			const SwRect aOldRect( pUp->Frm() );
/*N*/ 			pUp->Frm().SSize().Height() += nChg;
/*N*/ 			pUp->Prt().SSize().Height() += nChg;
/*N*/ 			if ( pSh )
/*N*/ 				pSh->Imp()->SetFirstVisPageInvalid();
/*N*/ 
/*N*/ 			if ( GetNext() )
/*?*/ 				GetNext()->_InvalidatePos();
/*N*/ 
/*N*/ 			//Ggf. noch ein Repaint ausloesen.
/*N*/ 			const SvxGraphicPosition ePos = pUp->GetFmt()->GetBackground().GetGraphicPos();
/*N*/ 			if ( ePos != GPOS_NONE && ePos != GPOS_TILED )
/*?*/ 				pSh->InvalidateWindows( pUp->Frm() );
/*N*/ 
/*N*/ 			if ( pUp->GetUpper() )
/*N*/ 			{
/*N*/ 				if ( pUp->GetNext() )
/*N*/ 					pUp->GetNext()->InvalidatePos();
/*N*/ 
/*N*/ 				//Mies aber wahr: im Notify am ViewImp wird evtl. ein Calc
/*N*/ 				//auf die Seite und deren Lower gerufen. Die Werte sollten
/*N*/ 				//unverandert bleiben, weil der Aufrufer bereits fuer die
/*N*/ 				//Anpassung von Frm und Prt sorgen wird.
/*N*/ 				const long nOldFrmHeight = Frm().Height();
/*N*/ 				const long nOldPrtHeight = Prt().Height();
/*N*/ 				const BOOL bOldComplete = IsCompletePaint();
/*N*/ 				if ( IsBodyFrm() )
/*N*/ 					Prt().SSize().Height() = nOldFrmHeight;
/*N*/ 				((SwPageFrm*)pUp)->AdjustRootSize( CHG_CHGPAGE, &aOldRect );
/*N*/ 				Frm().SSize().Height() = nOldFrmHeight;
/*N*/ 				Prt().SSize().Height() = nOldPrtHeight;
/*N*/ 				bCompletePaint = bOldComplete;
/*N*/ 			}
/*N*/ 			if ( !IsBodyFrm() )
/*?*/ 				pUp->_InvalidateSize();
/*N*/ 			InvalidatePage( (SwPageFrm*)pUp );
/*N*/ 		}
/*N*/ 		nDiff -= nChg;
/*N*/ 		if ( !nDiff )
/*N*/ 			return nChg;
/*N*/ 		else
/*N*/ 			nBrowseAdd = nChg;
/*N*/ 	}
/*N*/ 
/*N*/ 	const SwFtnBossFrm *pBoss = (SwFtnBossFrm*)GetUpper();
/*N*/ 
/*N*/ 	SwTwips nReal = 0,
/*N*/ 			nAdd  = 0;
/*N*/ 	SwFrm *pFrm = 0;
/*N*/     SWRECTFN( this )
/*N*/ 
/*N*/ 	if( IsBodyFrm() )
/*N*/ 	{
/*N*/ 		if( IsInSct() )
/*N*/ 		{
/*?*/ 			SwSectionFrm *pSect = FindSctFrm();
/*?*/ 			if( nDiff > 0 && pSect->IsEndnAtEnd() && GetNext() &&
/*?*/ 				GetNext()->IsFtnContFrm() )
/*?*/ 			{
/*?*/ 				SwFtnContFrm* pCont = (SwFtnContFrm*)GetNext();
/*?*/ 				SwTwips nMinH = 0;
/*?*/ 				SwFtnFrm* pFtn = (SwFtnFrm*)pCont->Lower();
/*?*/ 				BOOL bFtn = FALSE;
/*?*/ 				while( pFtn )
/*?*/ 				{
/*?*/ 					if( !pFtn->GetAttr()->GetFtn().IsEndNote() )
/*?*/ 					{
/*?*/                         nMinH += (pFtn->Frm().*fnRect->fnGetHeight)();
/*?*/ 						bFtn = TRUE;
/*?*/ 					}
/*?*/ 					pFtn = (SwFtnFrm*)pFtn->GetNext();
/*?*/ 				}
/*?*/ 				if( bFtn )
/*?*/                     nMinH += (pCont->Prt().*fnRect->fnGetTop)();
/*?*/                 nReal = (pCont->Frm().*fnRect->fnGetHeight)() - nMinH;
/*?*/ 				if( nReal > nDiff )
/*?*/ 					nReal = nDiff;
/*?*/ 				if( nReal > 0 )
/*?*/ 					pFrm = GetNext();
/*?*/ 				else
/*?*/ 					nReal = 0;
/*?*/ 			}
/*?*/ 			if( !bTst && !pSect->IsColLocked() )
/*?*/ 				pSect->InvalidateSize();
/*N*/ 		}
/*N*/ 		if( !pFrm )
/*N*/ 			return nBrowseAdd;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const BOOL bFtnPage = pBoss->IsPageFrm() && ((SwPageFrm*)pBoss)->IsFtnPage();
/*N*/ 		if ( bFtnPage && !IsFtnContFrm() )
/*?*/ 			pFrm = (SwFrm*)pBoss->FindFtnCont();
/*N*/ 		if ( !pFrm )
/*N*/ 			pFrm = (SwFrm*)pBoss->FindBodyCont();
/*N*/ 
/*N*/ 		if ( !pFrm )
/*?*/ 			return 0;
/*N*/ 
/*N*/ 		//Wenn ich keinen finde eruebrigt sich alles weitere.
/*N*/         nReal = (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/         if( nReal > nDiff )
/*N*/             nReal = nDiff;
/*N*/ 		if( !bFtnPage )
/*N*/ 		{
/*N*/ 			//Minimalgrenze beachten!
/*N*/ 			if( nReal )
/*N*/ 			{
/*N*/ 				const SwTwips nMax = pBoss->GetVarSpace();
/*N*/ 				if ( nReal > nMax )
/*?*/ 					nReal = nMax;
/*N*/ 			}
/*N*/ 			if( !IsFtnContFrm() && nDiff > nReal &&
/*N*/                 pFrm->GetNext() && pFrm->GetNext()->IsFtnContFrm()
/*N*/                 && ( pFrm->GetNext()->IsVertical() == IsVertical() )
/*N*/                 )
/*N*/ 			{
/*?*/ 				//Wenn der Body nicht genuegend her gibt, kann ich noch mal
/*?*/ 				//schauen ob es eine Fussnote gibt, falls ja kann dieser
/*?*/ 				//entsprechend viel gemopst werden.
/*?*/                 const SwTwips nAddMax = (pFrm->GetNext()->Frm().*fnRect->
/*?*/                                         fnGetHeight)();
/*?*/ 				nAdd = nDiff - nReal;
/*?*/ 				if ( nAdd > nAddMax )
/*?*/ 					nAdd = nAddMax;
/*?*/ 				if ( !bTst )
/*?*/ 				{
/*?*/                     (pFrm->GetNext()->Frm().*fnRect->fnSetHeight)(nAddMax-nAdd);
/*?*/                     if( bVert && !bRev )
/*?*/                         pFrm->GetNext()->Frm().Pos().X() += nAdd;
/*?*/ 					pFrm->GetNext()->InvalidatePrt();
/*?*/ 					if ( pFrm->GetNext()->GetNext() )
/*?*/ 						pFrm->GetNext()->GetNext()->_InvalidatePos();
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bTst && nReal )
/*N*/ 	{
/*N*/         SwTwips nTmp = (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/         (pFrm->Frm().*fnRect->fnSetHeight)( nTmp - nReal );
/*N*/         if( bVert && !bRev )
/*?*/             pFrm->Frm().Pos().X() += nReal;
/*N*/ 		pFrm->InvalidatePrt();
/*N*/ 		if ( pFrm->GetNext() )
/*N*/ 			pFrm->GetNext()->_InvalidatePos();
/*N*/ 		if( nReal < 0 && pFrm->IsInSct() )
/*N*/ 		{
/*?*/ 			SwLayoutFrm* pUp = pFrm->GetUpper();
/*?*/ 			if( pUp && 0 != ( pUp = pUp->GetUpper() ) && pUp->IsSctFrm() &&
/*?*/ 				!pUp->IsColLocked() )
/*?*/ 				pUp->InvalidateSize();
/*N*/ 		}
/*N*/ 		if( ( IsHeaderFrm() || IsFooterFrm() ) && pBoss->GetDrawObjs() )
/*N*/ 		{
/*N*/ 			const SwDrawObjs &rObjs = *pBoss->GetDrawObjs();
/*N*/ 			ASSERT( pBoss->IsPageFrm(), "Header/Footer out of page?" );
/*N*/ 			SwPageFrm *pPage = (SwPageFrm*)pBoss;
/*N*/ 			for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pObj = rObjs[i];
/*N*/ 				if ( pObj->IsWriterFlyFrame() )
/*N*/ 				{
/*N*/ 					SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 					ASSERT( !pFly->IsFlyInCntFrm(), "FlyInCnt at Page?" );
/*N*/ 					const SwFmtVertOrient &rVert =
/*N*/ 										pFly->GetFmt()->GetVertOrient();
/*N*/ 				   // Wann muss invalidiert werden?
/*N*/ 				   // Wenn ein Rahmen am SeitenTextBereich ausgerichtet ist,
/*N*/ 				   // muss bei Aenderung des Headers ein TOP, MIDDLE oder NONE,
/*N*/ 				   // bei Aenderung des Footers ein BOTTOM oder MIDDLE
/*N*/ 				   // ausgerichteter Rahmen seine Position neu berechnen.
/*N*/ 					if( ( rVert.GetRelationOrient() == PRTAREA ||
/*N*/ 						  rVert.GetRelationOrient() == REL_PG_PRTAREA )	&&
/*N*/ 						((IsHeaderFrm() && rVert.GetVertOrient()!=VERT_BOTTOM) ||
/*N*/ 						 (IsFooterFrm() && rVert.GetVertOrient()!=VERT_NONE &&
/*N*/ 						  rVert.GetVertOrient() != VERT_TOP)) )
/*N*/ 					{
/*?*/ 						pFly->_InvalidatePos();
/*?*/ 						pFly->_Invalidate();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return (nBrowseAdd + nReal + nAdd);
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::ImplInvalidateSize(), ImplInvalidatePrt(), ImplInvalidatePos(),
|* 		   ImplInvalidateLineNum()
|*
|*	Ersterstellung		MA 15. Oct. 92
|*	Letzte Aenderung	MA 24. Mar. 94
|*
|*************************************************************************/
/*N*/ void SwFrm::ImplInvalidateSize()
/*N*/ {
/*N*/ 	bValidSize = FALSE;
/*N*/ 	if ( IsFlyFrm() )
/*N*/ 		((SwFlyFrm*)this)->_Invalidate();
/*N*/ 	else
/*N*/ 		InvalidatePage();
/*N*/ }

/*N*/ void SwFrm::ImplInvalidatePrt()
/*N*/ {
/*N*/ 	bValidPrtArea = FALSE;
/*N*/ 	if ( IsFlyFrm() )
/*?*/ 		((SwFlyFrm*)this)->_Invalidate();
/*N*/ 	else
/*N*/ 		InvalidatePage();
/*N*/ }

/*N*/ void SwFrm::ImplInvalidatePos()
/*N*/ {
/*N*/ 	bValidPos = FALSE;
/*N*/ 	if ( IsFlyFrm() )
/*N*/ 		((SwFlyFrm*)this)->_Invalidate();
/*N*/ 	else
/*N*/ 		InvalidatePage();
/*N*/ }

/*N*/ void SwFrm::ImplInvalidateLineNum()
/*N*/ {
/*N*/ 	bValidLineNum = FALSE;
/*N*/ 	ASSERT( IsTxtFrm(), "line numbers are implemented for text only" );
/*N*/ 	InvalidatePage();
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::ReinitializeFrmSizeAttrFlags
|*
|*	Ersterstellung		MA 15. Oct. 96
|*	Letzte Aenderung	MA 15. Oct. 96
|*
|*************************************************************************/
/*N*/ void SwFrm::ReinitializeFrmSizeAttrFlags()
/*N*/ {
/*N*/ 	const SwFmtFrmSize &rFmtSize = GetAttrSet()->GetFrmSize();
/*N*/ 	if ( ATT_VAR_SIZE == rFmtSize.GetSizeType() ||
/*N*/ 		 ATT_MIN_SIZE == rFmtSize.GetSizeType())
/*N*/ 	{
/*N*/         bFixSize = FALSE;
/*N*/ 		if ( GetType() & (FRM_HEADER | FRM_FOOTER | FRM_ROW) )
/*N*/ 		{
/*N*/ 			SwFrm *pFrm = ((SwLayoutFrm*)this)->Lower();
/*N*/ 			while ( pFrm )
/*N*/ 			{	pFrm->_InvalidateSize();
/*N*/ 				pFrm->_InvalidatePrt();
/*N*/ 				pFrm = pFrm->GetNext();
/*N*/ 			}
/*N*/ 			SwCntntFrm *pCnt = ((SwLayoutFrm*)this)->ContainsCntnt();
/*N*/ 			pCnt->InvalidatePage();
/*N*/ 			do
/*N*/ 			{   pCnt->Prepare( PREP_ADJUST_FRM );
/*N*/ 				pCnt->_InvalidateSize();
/*N*/ 				pCnt = pCnt->GetNextCntntFrm();
/*N*/ 			} while ( ((SwLayoutFrm*)this)->IsAnLower( pCnt ) );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else if ( rFmtSize.GetSizeType() == ATT_FIX_SIZE )
/*N*/     {
/*N*/         if( IsVertical() )
/*?*/ 			ChgSize( Size( rFmtSize.GetWidth(), Frm().Height()));
/*N*/         else
/*N*/ 			ChgSize( Size( Frm().Width(), rFmtSize.GetHeight()));
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwCntntFrm::GrowFrm()
|*
|*	Ersterstellung		MA 30. Jul. 92
|*	Letzte Aenderung	MA 25. Mar. 99
|*
|*************************************************************************/
/*N*/ SwTwips SwCntntFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/ 
/*N*/     SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/     if( nFrmHeight > 0 &&
/*N*/          nDist > (LONG_MAX - nFrmHeight ) )
/*N*/         nDist = LONG_MAX - nFrmHeight;
/*N*/ 
/*N*/ 	const FASTBOOL bBrowse = GetUpper()->GetFmt()->GetDoc()->IsBrowseMode();
/*N*/ 	const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
/*N*/     if( !(GetUpper()->GetType() & nType) && GetUpper()->HasFixSize() )
/*N*/ 	{
/*N*/ 		if ( !bTst )
/*N*/ 		{
/*N*/             (Frm().*fnRect->fnSetHeight)( nFrmHeight + nDist );
/*N*/             if( IsVertical() && !IsReverse() )
/*?*/                 Frm().Pos().X() -= nDist;
/*N*/ 			if ( GetNext() )
/*N*/ 				GetNext()->InvalidatePos();
/*N*/ 		}
/*N*/ 		return 0;
/*N*/ 	}
/*N*/ 
/*N*/     SwTwips nReal = (GetUpper()->Prt().*fnRect->fnGetHeight)();
/*N*/ 	SwFrm *pFrm = GetUpper()->Lower();
/*N*/     while( pFrm && nReal > 0 )
/*N*/     {   nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 		pFrm = pFrm->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/ 		//Cntnts werden immer auf den gewuenschten Wert gebracht.
/*N*/         long nOld = (Frm().*fnRect->fnGetHeight)();
/*N*/         (Frm().*fnRect->fnSetHeight)( nOld + nDist );
/*N*/         if( IsVertical() && !IsReverse() )
/*?*/             Frm().Pos().X() -= nDist;
/*N*/ 		if ( nOld && IsInTab() )
/*N*/ 		{
/*N*/ 			SwTabFrm *pTab = FindTabFrm();
/*N*/ 			if ( pTab->GetTable()->GetHTMLTableLayout() &&
/*N*/ 				 !pTab->IsJoinLocked() &&
/*N*/ 				 !pTab->GetFmt()->GetDoc()->GetDocShell()->IsReadOnly() )
/*N*/ 			{
/*?*/ 				pTab->InvalidatePos();
/*?*/ 				pTab->SetResizeHTMLTable();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	//Upper nur growen wenn notwendig.
/*M*/ 	if ( nReal < nDist )
/*M*/ 	{
/*M*/ 			if( GetUpper() )
/*M*/ 			{
/*M*/ 				 if( bTst || !GetUpper()->IsFooterFrm() )
/*M*/ 					  nReal = GetUpper()->Grow( nDist - (nReal > 0 ? nReal : 0),
/*M*/                                           bTst, bInfo );
/*M*/ 				 else
/*M*/ 				{
/*M*/ 					 nReal = 0;
/*M*/ 					GetUpper()->InvalidateSize();
/*M*/ 				}
/*M*/ 			}	
/*M*/ 			else
/*M*/ 			 nReal = 0;
/*M*/ 		 }
/*N*/ 	else
/*N*/ 		nReal = nDist;
/*N*/ 
/*N*/ 	if ( !bTst && GetNext() )
/*N*/ 		GetNext()->InvalidatePos();
/*N*/ 
/*N*/ 	return nReal;
/*N*/ }

/*************************************************************************
|*
|*	SwCntntFrm::ShrinkFrm()
|*
|*	Ersterstellung		MA 30. Jul. 92
|*	Letzte Aenderung	MA 05. May. 94
|*
|*************************************************************************/
/*N*/ SwTwips SwCntntFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/ 	ASSERT( nDist >= 0, "nDist < 0" );
/*N*/     ASSERT( nDist <= (Frm().*fnRect->fnGetHeight)(),
/*N*/ 			"nDist > als aktuelle Grosse." );
/*N*/ 
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/         SwTwips nRstHeight;
/*N*/         if( GetUpper() )
/*N*/             nRstHeight = (Frm().*fnRect->fnBottomDist)
/*N*/                          ( (GetUpper()->*fnRect->fnGetPrtBottom)() );
/*N*/         else
/*N*/             nRstHeight = 0;
/*N*/ 		if( nRstHeight < 0 )
/*N*/ 			nRstHeight = nDist + nRstHeight;
/*N*/ 		else
/*N*/ 			nRstHeight = nDist;
/*N*/         (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)() - nDist );
/*N*/         if( IsVertical() )
/*N*/             Frm().Pos().X() += nDist;
/*N*/ 		nDist = nRstHeight;
/*N*/ 		if ( IsInTab() )
/*N*/ 		{
/*N*/ 			SwTabFrm *pTab = FindTabFrm();
/*N*/ 			if ( pTab->GetTable()->GetHTMLTableLayout() &&
/*N*/ 				 !pTab->IsJoinLocked() &&
/*N*/ 				 !pTab->GetFmt()->GetDoc()->GetDocShell()->IsReadOnly() )
/*N*/ 			{
/*N*/ 				pTab->InvalidatePos();
/*N*/ 				pTab->SetResizeHTMLTable();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     SwTwips nReal;
/*N*/     if( GetUpper() && nDist > 0 )
/*N*/     {
/*N*/         if( bTst || !GetUpper()->IsFooterFrm() )
/*N*/             nReal = GetUpper()->Shrink( nDist, bTst, bInfo );
/*N*/         else
/*N*/         {
/*N*/             nReal = 0;
/*N*/ 
/*N*/             // #108745# Sorry, dear old footer friend, I'm not gonna invalidate you,
/*N*/             // if there are any objects anchored inside your content, which
/*N*/             // overlap with the shrinking frame.
/*N*/             // This may lead to a footer frame that is too big, but this is better
/*N*/             // than looping.
/*N*/             // #109722# : The fix for #108745# was too strict.
/*N*/ 
/*N*/             bool bInvalidate = true;
/*N*/             const SwRect aRect( Frm() );
/*N*/             const SwPageFrm* pPage = FindPageFrm();
/*N*/             const SwSortDrawObjs* pSorted;
/*N*/             if( pPage && ( pSorted = pPage->GetSortedObjs() ) )
/*N*/             {
/*N*/                 for ( USHORT i = 0; i < pSorted->Count(); ++i )
/*N*/                 {
/*N*/                     const SdrObject *pObj = (*pSorted)[i];
/*N*/                     const SwRect aBound( GetBoundRect( pObj ) );
/*N*/ 
/*N*/                     if( aBound.Left() > aRect.Right() )
/*N*/                         continue;
/*N*/ 
/*N*/                     if( aBound.IsOver( aRect ) )
/*N*/                     {
/*N*/                         const SwFmt* pFmt = ((SwContact*)GetUserCall(pObj))->GetFmt();
/*N*/                         if( SURROUND_THROUGHT != pFmt->GetSurround().GetSurround() )
/*N*/                         {
/*N*/                             const SwFrm* pAnchor = pObj->IsWriterFlyFrame() ?
/*N*/                                                    ( (SwVirtFlyDrawObj*)pObj )->GetFlyFrm()->GetAnchor() :
/*N*/                                                    ( (SwDrawContact*)GetUserCall(pObj) )->GetAnchor();
/*N*/ 
/*N*/                             if ( pAnchor && pAnchor->FindFooterOrHeader() == GetUpper() )
/*N*/                             {
/*N*/                                 bInvalidate = false;
/*N*/                                 break;
/*N*/                             }
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/             if ( bInvalidate )
/*N*/                 GetUpper()->InvalidateSize();
/*N*/         }
/*N*/     }
/*N*/     else
/*N*/         nReal = 0;
/*N*/ 
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/ 		//Die Position des naechsten Frm's veraendert sich auf jeden Fall.
/*N*/ 		InvalidateNextPos();
/*N*/ 
/*N*/ 		//Wenn ich keinen Nachfolger habe, so muss ich mich eben selbst um
/*N*/ 		//die Retusche kuemmern.
/*N*/ 		if ( !GetNext() )
/*N*/ 			SetRetouche();
/*N*/ 	}
/*N*/ 	return nReal;
/*N*/ }

/*************************************************************************
|*
|*	  SwCntntFrm::Modify()
|*
|*	  Beschreibung
|*	  Ersterstellung	AK 05-Mar-1991
|*	  Letzte Aenderung	MA 13. Oct. 95
|*
|*************************************************************************/
/*N*/ void SwCntntFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
/*N*/ {
/*N*/ 	BYTE nInvFlags = 0;
/*N*/ 
/*N*/ 	if( pNew && RES_ATTRSET_CHG == pNew->Which() )
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
/*N*/ 			SwFrm::Modify( &aOldSet, &aNewSet );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		_UpdateAttr( pOld, pNew, nInvFlags );
/*N*/ 
/*N*/ 	if ( nInvFlags != 0 )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/ 		InvalidatePage( pPage );
/*N*/ 		if ( nInvFlags & 0x01 )
/*N*/ 			SetCompletePaint();
/*N*/ 		if ( nInvFlags & 0x02 )
/*N*/ 			_InvalidatePos();
/*N*/ 		if ( nInvFlags & 0x04 )
/*N*/ 			_InvalidateSize();
/*N*/ 		if ( nInvFlags & 0x88 )
/*N*/ 		{
/*N*/ 			if( IsInSct() && !GetPrev() )
/*N*/ 			{
/*N*/ 				SwSectionFrm *pSect = FindSctFrm();
/*N*/ 				if( pSect->ContainsAny() == this )
/*N*/ 				{
/*N*/ 					pSect->_InvalidatePrt();
/*N*/ 					pSect->InvalidatePage( pPage );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			_InvalidatePrt();
/*N*/ 		}
/*N*/ 		SwFrm *pTmp;
/*N*/ 		if ( 0 != (pTmp = GetIndNext()) && nInvFlags & 0x10)
/*N*/ 		{
/*N*/ 			pTmp->_InvalidatePrt();
/*N*/ 			pTmp->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		if ( nInvFlags & 0x80 && pTmp )
/*N*/ 			pTmp->SetCompletePaint();
/*N*/ 		if ( nInvFlags & 0x20 && 0 != (pTmp = GetPrev()) )
/*N*/ 		{
/*N*/ 			pTmp->_InvalidatePrt();
/*N*/ 			pTmp->InvalidatePage( pPage );
/*N*/ 		}
/*N*/ 		if ( nInvFlags & 0x40 )
/*N*/ 			InvalidateNextPos();
/*N*/ 	}
/*N*/ }

/*N*/ void SwCntntFrm::_UpdateAttr( SfxPoolItem* pOld, SfxPoolItem* pNew,
/*N*/ 							  BYTE &rInvFlags,
/*N*/ 							SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
/*N*/ {
/*N*/ 	BOOL bClear = TRUE;
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ 	switch ( nWhich )
/*N*/ 	{
/*N*/ 		case RES_FMT_CHG:
/*N*/ 			rInvFlags = 0xFF;
/*N*/ 			/* kein break hier */
/*N*/ 
/*N*/ 		case RES_PAGEDESC:						//Attributaenderung (an/aus)
/*N*/ 			if ( IsInDocBody() && !IsInTab() )
/*N*/ 			{
/*N*/ 				rInvFlags |= 0x02;
/*N*/ 				SwPageFrm *pPage = FindPageFrm();
/*N*/ 				if ( !GetPrev() )
/*N*/ 					CheckPageDescs( pPage );
/*N*/ 				if ( pPage && GetAttrSet()->GetPageDesc().GetNumOffset() )
/*N*/ 					((SwRootFrm*)pPage->GetUpper())->SetVirtPageNum( TRUE );
/*N*/ 				SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
/*N*/ 				pPage->GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_UL_SPACE:
/*N*/ 			{
/*N*/ 				if( IsInFtn() && !GetIndNext() )
/*N*/ 				{
/*?*/ 					SwFrm* pNxt = FindNext();
/*?*/ 					if( pNxt )
/*?*/ 					{
/*?*/ 						SwPageFrm* pPg = pNxt->FindPageFrm();
/*?*/ 						pNxt->InvalidatePage( pPg );
/*?*/ 						pNxt->_InvalidatePrt();
/*?*/ 						if( pNxt->IsSctFrm() )
/*?*/ 						{
/*?*/ 							SwFrm* pCnt = ((SwSectionFrm*)pNxt)->ContainsAny();
/*?*/ 							if( pCnt )
/*?*/ 							{
/*?*/ 								pCnt->_InvalidatePrt();
/*?*/ 								pCnt->InvalidatePage( pPg );
/*?*/ 							}
/*?*/ 						}
/*?*/ 						pNxt->SetCompletePaint();
/*?*/ 					}
/*N*/ 				}
/*N*/ 				Prepare( PREP_UL_SPACE );	//TxtFrm muss Zeilenabst. korrigieren.
/*N*/ 				rInvFlags |= 0x80;
/*N*/ 				/* kein Break hier */
/*N*/ 			}
/*N*/ 		case RES_LR_SPACE:
/*N*/ 		case RES_BOX:
/*N*/ 		case RES_SHADOW:
/*N*/ 			Prepare( PREP_FIXSIZE_CHG );
/*N*/ 			SwFrm::Modify( pOld, pNew );
/*N*/ 			rInvFlags |= 0x30;
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_BREAK:
/*N*/ 			{
/*N*/ 				rInvFlags |= 0x42;
/*N*/ 				if( GetAttrSet()->GetDoc()->IsParaSpaceMax() ||
/*N*/ 					GetAttrSet()->GetDoc()->IsParaSpaceMaxAtPages() )
/*N*/ 				{
/*?*/ 					rInvFlags |= 0x1;
/*?*/ 					SwFrm* pNxt = FindNext();
/*?*/ 					if( pNxt )
/*?*/ 					{
/*?*/ 						SwPageFrm* pPg = pNxt->FindPageFrm();
/*?*/ 						pNxt->InvalidatePage( pPg );
/*?*/ 						pNxt->_InvalidatePrt();
/*?*/ 						if( pNxt->IsSctFrm() )
/*?*/ 						{
/*?*/ 							SwFrm* pCnt = ((SwSectionFrm*)pNxt)->ContainsAny();
/*?*/ 							if( pCnt )
/*?*/ 							{
/*?*/ 								pCnt->_InvalidatePrt();
/*?*/ 								pCnt->InvalidatePage( pPg );
/*?*/ 							}
/*?*/ 						}
/*?*/ 						pNxt->SetCompletePaint();
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_PARATR_TABSTOP:
/*N*/ 		case RES_CHRATR_PROPORTIONALFONTSIZE:
/*N*/ 		case RES_CHRATR_SHADOWED:
/*N*/ 		case RES_CHRATR_AUTOKERN:
/*N*/ 		case RES_CHRATR_UNDERLINE:
/*N*/ 		case RES_CHRATR_KERNING:
/*N*/ 		case RES_CHRATR_FONT:
/*N*/ 		case RES_CHRATR_FONTSIZE:
/*N*/ 		case RES_CHRATR_ESCAPEMENT:
/*N*/ 		case RES_CHRATR_CONTOUR:
/*N*/ 			rInvFlags |= 0x01;
/*N*/ 			break;
/*N*/ 
/*N*/ 
/*N*/ 		case RES_FRM_SIZE:
/*?*/ 			rInvFlags |= 0x01;
/*N*/ 			/* no break here */
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
/*N*/ 			SwFrm::Modify( pOld, pNew );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::SwLayoutFrm()
|*
|*	Ersterstellung		AK 14-Feb-1991
|*	Letzte Aenderung	MA 12. May. 95
|*
|*************************************************************************/
/*N*/ SwLayoutFrm::SwLayoutFrm( SwFrmFmt* pFmt ):
/*N*/ 	SwFrm( pFmt ),
/*N*/ 	pLower( 0 )
/*N*/ {
/*N*/ 	const SwFmtFrmSize &rFmtSize = pFmt->GetFrmSize();
/*N*/ 	if ( rFmtSize.GetSizeType() == ATT_FIX_SIZE )
/*N*/         BFIXHEIGHT = TRUE;
/*N*/ }

/*-----------------10.06.99 09:42-------------------
 * SwLayoutFrm::InnerHeight()
 * --------------------------------------------------*/

/*N*/ SwTwips SwLayoutFrm::InnerHeight() const
/*N*/ {
/*N*/ 	if( !Lower() )
/*N*/ 		return 0;
/*N*/ 	SwTwips nRet = 0;
/*N*/ 	const SwFrm* pCnt = Lower();
/*N*/     SWRECTFN( this )
/*N*/ 	if( pCnt->IsColumnFrm() || pCnt->IsCellFrm() )
/*N*/ 	{
/*N*/ 		do
/*N*/ 		{
/*?*/ 			SwTwips nTmp = ((SwLayoutFrm*)pCnt)->InnerHeight();
/*?*/ 			if( pCnt->GetValidPrtAreaFlag() )
/*?*/                 nTmp += (pCnt->Frm().*fnRect->fnGetHeight)() -
/*?*/                         (pCnt->Prt().*fnRect->fnGetHeight)();
/*?*/             if( nRet < nTmp )
/*?*/ 				nRet = nTmp;
/*?*/ 			pCnt = pCnt->GetNext();
/*?*/ 		} while ( pCnt );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		do
/*N*/ 		{
/*N*/             nRet += (pCnt->Frm().*fnRect->fnGetHeight)();
/*N*/ 			if( pCnt->IsCntntFrm() && ((SwTxtFrm*)pCnt)->IsUndersized() )
/*N*/                 nRet += ((SwTxtFrm*)pCnt)->GetParHeight() -
/*N*/                         (pCnt->Prt().*fnRect->fnGetHeight)();
/*N*/ 			if( pCnt->IsLayoutFrm() && !pCnt->IsTabFrm() )
/*N*/                 nRet += ((SwLayoutFrm*)pCnt)->InnerHeight() -
/*?*/                         (pCnt->Prt().*fnRect->fnGetHeight)();
/*N*/ 			pCnt = pCnt->GetNext();
/*N*/ 		} while( pCnt );
/*N*/ 
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::GrowFrm()
|*
|*	Ersterstellung		MA 30. Jul. 92
|*	Letzte Aenderung	MA 23. Sep. 96
|*
|*************************************************************************/
/*N*/ SwTwips SwLayoutFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/ 	const FASTBOOL bBrowse = GetFmt()->GetDoc()->IsBrowseMode();
/*N*/ 	const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
/*N*/     if( !(GetType() & nType) && HasFixSize() )
/*N*/ 		return 0;
/*N*/ 
/*N*/     SWRECTFN( this )
/*N*/     SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/     if ( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
/*?*/         nDist = LONG_MAX - nFrmHeight;
/*N*/ 
/*N*/     SwTwips nMin = 0;
/*N*/     if ( GetUpper() && !IsCellFrm() )
/*N*/ 	{
/*N*/         SwFrm *pFrm = GetUpper()->Lower();
/*N*/         while( pFrm )
/*N*/         {   nMin += (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/             pFrm = pFrm->GetNext();
/*N*/         }
/*N*/         nMin = (GetUpper()->Prt().*fnRect->fnGetHeight)() - nMin;
/*N*/ 		if ( nMin < 0 )
/*N*/ 			nMin = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwRect aOldFrm( Frm() );
/*N*/ 	sal_Bool bMoveAccFrm = sal_False;
/*N*/ 
/*N*/     BOOL bChgPos = IsVertical() && !IsReverse();
/*N*/ 	if ( !bTst )
/*N*/     {
/*N*/         (Frm().*fnRect->fnSetHeight)( nFrmHeight + nDist );
/*N*/         if( bChgPos )
/*?*/             Frm().Pos().X() -= nDist;
/*N*/ 		bMoveAccFrm = sal_True;
/*N*/     }
/*N*/ 
/*N*/ 	SwTwips nReal = nDist - nMin;
/*N*/ 	if ( nReal > 0 )
/*N*/ 	{
/*N*/ 		if ( GetUpper() )
/*N*/ 		{   // AdjustNeighbourhood jetzt auch in Spalten (aber nicht in Rahmen)
/*N*/ 			BYTE nAdjust = GetUpper()->IsFtnBossFrm() ?
/*N*/ 				((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
/*N*/ 				: NA_GROW_SHRINK;
/*N*/ 			if( NA_ONLY_ADJUST == nAdjust )
/*N*/ 				nReal = AdjustNeighbourhood( nReal, bTst );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				SwTwips nGrow = 0;
/*N*/ 				if( NA_ADJUST_GROW == nAdjust )
/*?*/ 					nReal += AdjustNeighbourhood( nReal - nGrow, bTst );
/*N*/ 				if( nGrow < nReal )
/*N*/                     nGrow += GetUpper()->Grow( nReal - nGrow, bTst, bInfo );
/*N*/ 				if( NA_GROW_ADJUST == nAdjust && nGrow < nReal )
/*?*/ 					nReal += AdjustNeighbourhood( nReal - nGrow, bTst );
/*N*/ 				if ( IsFtnFrm() && (nGrow != nReal) && GetNext() )
/*N*/ 				{
/*?*/ 					//Fussnoten koennen ihre Nachfolger verdraengen.
/*?*/ 					SwTwips nSpace = bTst ? 0 : -nDist;
/*?*/ 					const SwFrm *pFrm = GetUpper()->Lower();
/*?*/ 					do
/*?*/                     {   nSpace += (pFrm->Frm().*fnRect->fnGetHeight)();
/*?*/ 						pFrm = pFrm->GetNext();
/*?*/ 					} while ( pFrm != GetNext() );
/*?*/                     nSpace = (GetUpper()->Prt().*fnRect->fnGetHeight)() -nSpace;
/*?*/ 					if ( nSpace < 0 )
/*?*/ 						nSpace = 0;
/*?*/ 					nSpace += nGrow;
/*?*/ 					if ( nReal > nSpace )
/*?*/ 						nReal = nSpace;
/*?*/ 					if ( nReal && !bTst )
/*?*/ 						((SwFtnFrm*)this)->InvalidateNxtFtnCnts( FindPageFrm() );
/*N*/ 				}
/*N*/ 				else
/*N*/ 					nReal = nGrow;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			nReal = 0;
/*N*/ 
/*N*/ 		nReal += nMin;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nReal = nDist;
/*N*/ 
/*N*/ 	if ( !bTst )
/*N*/ 	{
/*N*/         if( nReal != nDist && !IsCellFrm() )
/*N*/         {
/*N*/             nDist -= nReal;
/*N*/             (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
/*N*/                                           - nDist );
/*N*/             if( bChgPos )
/*?*/                 Frm().Pos().X() += nDist;
/*N*/ 			bMoveAccFrm = sal_True;
/*N*/         }
/*N*/ 
/*N*/ 		if ( nReal )
/*N*/ 		{
/*N*/ 			SwPageFrm *pPage = FindPageFrm();
/*N*/ 			if ( GetNext() )
/*N*/ 			{
/*N*/ 				GetNext()->_InvalidatePos();
/*N*/ 				if ( GetNext()->IsCntntFrm() )
/*?*/ 					GetNext()->InvalidatePage( pPage );
/*N*/ 			}
/*N*/ 			if ( !IsPageBodyFrm() )
/*N*/ 			{
/*N*/ 				_InvalidateAll();
/*N*/ 				InvalidatePage( pPage );
/*N*/ 			}
/*N*/ 			if ( !(GetType() & 0x1823) ) //Tab, Row, FtnCont, Root, Page
/*N*/ 				NotifyFlys();
/*N*/ 
/*N*/ 			if( IsCellFrm() )
/*N*/                 InvaPercentLowers( nReal );
/*N*/ 
/*N*/ 			const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
/*N*/ 			if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
/*?*/ 				SetCompletePaint();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bMoveAccFrm && IsAccessibleFrm() )
/*N*/ 	{
/*N*/ 		SwRootFrm *pRootFrm = FindRootFrm();
/*N*/ 		if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 			pRootFrm->GetCurrShell() )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nReal;
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::ShrinkFrm()
|*
|*	Ersterstellung		MA 30. Jul. 92
|*	Letzte Aenderung	MA 25. Mar. 99
|*
|*************************************************************************/
/*N*/ SwTwips SwLayoutFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*N*/ {
/*N*/ 	const FASTBOOL bBrowse = GetFmt()->GetDoc()->IsBrowseMode();
/*N*/ 	const USHORT nType = bBrowse ? 0x2084: 0x2004; //Row+Cell, Browse mit Body
/*N*/     if( !(GetType() & nType) && HasFixSize() )
/*N*/ 		return 0;
/*N*/ 
/*N*/ 	ASSERT( nDist >= 0, "nDist < 0" );
/*N*/     SWRECTFN( this )
/*N*/     SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/     if ( nDist > nFrmHeight )
/*?*/         nDist = nFrmHeight;
/*N*/ 
/*N*/ 	SwTwips nMin = 0;
/*N*/     BOOL bChgPos = IsVertical() && !IsReverse();
/*N*/ 	if ( Lower() )
/*N*/ 	{
/*N*/         if( !Lower()->IsNeighbourFrm() )
/*N*/         {   const SwFrm *pFrm = Lower();
/*N*/             const long nTmp = (Prt().*fnRect->fnGetHeight)();
/*N*/             while( pFrm && nMin < nTmp )
/*N*/             {   nMin += (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 				pFrm = pFrm->GetNext();
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SwTwips nReal = nDist;
/*N*/     SwTwips nMinDiff = (Prt().*fnRect->fnGetHeight)() - nMin;
/*N*/     if( nReal > nMinDiff )
/*N*/         nReal = nMinDiff;
/*N*/     if( nReal <= 0 )
/*N*/ 		return nDist;
/*N*/ 
/*N*/ 	SwRect aOldFrm( Frm() );
/*N*/ 	sal_Bool bMoveAccFrm = sal_False;
/*N*/ 
/*N*/ 	SwTwips nRealDist = nReal;
/*N*/ 	if ( !bTst )
/*N*/     {
/*N*/         (Frm().*fnRect->fnSetHeight)( nFrmHeight - nReal );
/*N*/         if( bChgPos )
/*?*/             Frm().Pos().X() += nReal;
/*N*/ 		bMoveAccFrm = sal_True;
/*N*/     }
/*N*/ 
/*N*/ 	BYTE nAdjust = GetUpper() && GetUpper()->IsFtnBossFrm() ?
/*N*/ 				   ((SwFtnBossFrm*)GetUpper())->NeighbourhoodAdjustment( this )
/*N*/ 				   : NA_GROW_SHRINK;
/*N*/ 
/*N*/ 	// AdjustNeighbourhood auch in Spalten (aber nicht in Rahmen)
/*N*/ 	if( NA_ONLY_ADJUST == nAdjust )
/*N*/ 	{
/*N*/ 		if ( IsPageBodyFrm() && !bBrowse )
/*?*/ 			nReal = nDist;
/*N*/ 		else
/*N*/ 		{	nReal = AdjustNeighbourhood( -nReal, bTst );
/*N*/ 			nReal *= -1;
/*N*/ 			if ( !bTst && IsBodyFrm() && nReal < nRealDist )
/*N*/             {
/*?*/                 (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
/*?*/                                             + nRealDist - nReal );
/*?*/                 if( bChgPos )
/*?*/                     Frm().Pos().X() += nRealDist - nReal;
/*?*/ 				ASSERT( !IsAccessibleFrm(), "bMoveAccFrm has to be set!" );
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/     else if( IsColumnFrm() || IsColBodyFrm() )
/*N*/     {
/*N*/         SwTwips nTmp = GetUpper()->Shrink( nReal, bTst, bInfo );
/*N*/ 		if ( nTmp != nReal )
/*N*/ 		{
/*N*/             (Frm().*fnRect->fnSetHeight)( (Frm().*fnRect->fnGetHeight)()
/*N*/                                           + nReal - nTmp );
/*N*/             if( bChgPos )
/*?*/                 Frm().Pos().X() += nTmp - nReal;
/*N*/ 			ASSERT( !IsAccessibleFrm(), "bMoveAccFrm has to be set!" );
/*N*/ 			nReal = nTmp;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwTwips nShrink = nReal;
/*N*/         nReal = GetUpper() ? GetUpper()->Shrink( nShrink, bTst, bInfo ) : 0;
/*N*/ 		if( ( NA_GROW_ADJUST == nAdjust || NA_ADJUST_GROW == nAdjust )
/*N*/ 			&& nReal < nShrink )
/*?*/ 			AdjustNeighbourhood( nReal - nShrink );
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bMoveAccFrm && IsAccessibleFrm() )
/*N*/ 	{
/*N*/ 		SwRootFrm *pRootFrm = FindRootFrm();
/*N*/ 		if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
/*N*/ 			pRootFrm->GetCurrShell() )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( !bTst && (IsCellFrm() || IsColumnFrm() ? nReal : nRealDist) )
/*N*/ 	{
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/ 		if ( GetNext() )
/*N*/ 		{
/*N*/ 			GetNext()->_InvalidatePos();
/*N*/ 			if ( GetNext()->IsCntntFrm() )
/*N*/ 				GetNext()->InvalidatePage( pPage );
/*N*/ 			if ( IsTabFrm() )
/*N*/ 				((SwTabFrm*)this)->SetComplete();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{	if ( IsRetoucheFrm() )
/*N*/ 				SetRetouche();
/*N*/ 			if ( IsTabFrm() )
/*N*/ 			{
/*N*/ 				if( IsTabFrm() )
/*N*/ 					((SwTabFrm*)this)->SetComplete();
/*N*/ 				if ( Lower() ) 	//Kann auch im Join stehen und leer sein!
/*N*/ 					InvalidateNextPos();
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !IsBodyFrm() )
/*N*/ 		{
/*N*/ 			_InvalidateAll();
/*N*/ 			InvalidatePage( pPage );
/*N*/ 			const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
/*N*/ 			if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
/*?*/ 				SetCompletePaint();
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( !(GetType() & 0x1823) ) //Tab, Row, FtnCont, Root, Page
/*N*/ 			NotifyFlys();
/*N*/ 
/*N*/ 		if( IsCellFrm() )
/*N*/             InvaPercentLowers( nReal );
/*N*/ 
/*N*/ 		SwCntntFrm *pCnt;
/*N*/ 		if( IsFtnFrm() && !((SwFtnFrm*)this)->GetAttr()->GetFtn().IsEndNote() &&
/*N*/ 			( GetFmt()->GetDoc()->GetFtnInfo().ePos != FTNPOS_CHAPTER ||
/*N*/ 			  ( IsInSct() && FindSctFrm()->IsFtnAtEnd() ) ) &&
/*N*/ 			  0 != (pCnt = ((SwFtnFrm*)this)->GetRefFromAttr() ) )
/*N*/ 		{
/*N*/ 			if ( pCnt->IsFollow() )
/*N*/ 			{   // Wenn wir sowieso schon in einer anderen Spalte/Seite sitzen
/*?*/ 				// als der Frame mit der Referenz, dann brauchen wir nicht
/*?*/ 				// auch noch seinen Master zu invalidieren.
/*?*/ 				SwFrm *pTmp = pCnt->FindFtnBossFrm(TRUE) == FindFtnBossFrm(TRUE)
/*?*/ 							  ?  pCnt->FindMaster()->GetFrm() : pCnt;
/*?*/ 				pTmp->Prepare( PREP_ADJUST_FRM );
/*?*/ 				pTmp->InvalidateSize();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pCnt->InvalidatePos();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nReal;
/*N*/ }
/*************************************************************************
|*
|*	SwLayoutFrm::ChgLowersProp()
|*
|*	Beschreibung		Aendert die Grosse der direkt untergeordneten Frm's
|* 		die eine Fixe Groesse haben, proportional zur Groessenaenderung der
|* 		PrtArea des Frm's.
|* 		Die Variablen Frm's werden auch proportional angepasst; sie werden
|* 		sich schon wieder zurechtwachsen/-schrumpfen.
|*	Ersterstellung		MA 11.03.92
|*	Letzte Aenderung	AMA 2. Nov. 98
|*
|*************************************************************************/
/*N*/ void SwLayoutFrm::ChgLowersProp( const Size& rOldSize )
/*N*/ {
/*N*/     // no change of lower properties for root frame or if no lower exists.
/*N*/     if ( IsRootFrm() || !Lower() )
/*N*/ 		return;
/*N*/ 
/*N*/     // declare and init <SwFrm* pLowerFrm> with first lower
/*N*/     SwFrm *pLowerFrm = Lower();
/*N*/ 
/*N*/     // declare and init const booleans <bHeightChgd> and <bWidthChg>
/*N*/     const bool bHeightChgd = rOldSize.Height() != Prt().Height();
/*N*/     const bool bWidthChgd  = rOldSize.Width()  != Prt().Width();
/*N*/ 
/*N*/     // declare and init variables <bVert>, <bRev> and <fnRect>
/*N*/     SWRECTFN( this )
/*N*/ 
/*N*/     // handle special case as short cut:
/*N*/     // if method called for a body frame belonging to the flow text body
/*N*/     //  and the first lower of the body isn't a column frame (body contains real content)
/*N*/     //  and its fixed size (in vertical layout its height; in horizontal layout its
/*N*/     //      width) doesn't changed
/*N*/     //  and the body frame doesn't belong to a locked section,
/*N*/     // then only invalidate lowers that are influence by the change.
/*N*/     // "Only" the variable size (in vertical layout its width; in horizontal
/*N*/     // layout its height) of body frame has changed.
/*N*/     if ( IsBodyFrm() && IsInDocBody() &&
/*N*/          !Lower()->IsColumnFrm() &&
/*N*/          !( bVert ? bHeightChgd : bWidthChgd ) &&
/*N*/          ( !IsInSct() || !FindSctFrm()->IsColLocked() )
/*N*/        )
/*N*/ 	{
/*N*/         // Determine page frame the body frame belongs to.
/*N*/ 		SwPageFrm *pPage = FindPageFrm();
/*N*/         // Determine last lower by traveling through them using <GetNext()>.
/*N*/         // During travel check each section frame, if it will be sized to
/*N*/         // maximum. If Yes, invalidate size of section frame and set
/*N*/         // corresponding flags at the page.
/*N*/         do
/*N*/         {
/*N*/             if( pLowerFrm->IsSctFrm() &&((SwSectionFrm*)pLowerFrm)->_ToMaximize() )
/*N*/             {
/*N*/                 pLowerFrm->_InvalidateSize();
/*N*/                 pLowerFrm->InvalidatePage( pPage );
/*N*/             }
/*N*/             if( pLowerFrm->GetNext() )
/*N*/                 pLowerFrm = pLowerFrm->GetNext();
/*N*/             else
/*N*/                 break;
/*N*/         } while( TRUE );
/*N*/         // If found last lower is a section frame containing no section
/*N*/         // (section frame isn't valid and will be deleted in the future),
/*N*/         // travel backwards.
/*N*/         while( pLowerFrm->IsSctFrm() && !((SwSectionFrm*)pLowerFrm)->GetSection() &&
/*N*/                pLowerFrm->GetPrev() )
/*N*/             pLowerFrm = pLowerFrm->GetPrev();
/*N*/         // If found last lower is a section frame, set <pLowerFrm> to its last
/*N*/         // content, if the section frame is valid and is not sized to maximum.
/*N*/         // Otherwise set <pLowerFrm> to NULL - In this case body frame only
/*N*/         //      contains invalid section frames.
/*N*/         if( pLowerFrm->IsSctFrm() )
/*N*/             pLowerFrm = ((SwSectionFrm*)pLowerFrm)->GetSection() &&
/*N*/                    !((SwSectionFrm*)pLowerFrm)->ToMaximize( FALSE ) ?
/*N*/                    ((SwSectionFrm*)pLowerFrm)->FindLastCntnt() : NULL;
/*N*/ 
/*N*/         // continue with found last lower, probably the last content of a section
/*N*/         if ( pLowerFrm )
/*N*/ 		{
/*N*/             // If <pLowerFrm> is in a table frame, set <pLowerFrm> to this table
/*N*/             // frame and continue.
/*N*/             if ( pLowerFrm->IsInTab() )
/*N*/             {
/*N*/                 // OD 28.10.2002 #97265# - safeguard for setting <pLowerFrm> to
/*N*/                 // its table frame - check, if the table frame is also a lower
/*N*/                 // of the body frame, in order to assure that <pLowerFrm> is not
/*N*/                 // set to a frame, which is an *upper* of the body frame.
/*N*/                 SwFrm* pTableFrm = pLowerFrm->FindTabFrm();
/*N*/                 if ( IsAnLower( pTableFrm ) )
/*N*/                 {
/*N*/                     pLowerFrm = pTableFrm;
/*N*/                 }
/*N*/             }
/*N*/             // Check, if variable size of body frame has grown
/*N*/             // OD 28.10.2002 #97265# - correct check, if variable size has grown.
/*N*/             //SwTwips nOldHeight = bVert ? rOldSize.Height() : rOldSize.Width();
/*N*/             SwTwips nOldHeight = bVert ? rOldSize.Width() : rOldSize.Height();
/*N*/             if( nOldHeight < (Prt().*fnRect->fnGetHeight)() )
/*N*/ 			{
/*N*/                 // If variable size of body frame has grown, only found last lower
/*N*/                 // and the position of the its next have to be invalidated.
/*N*/                 pLowerFrm->_InvalidateAll();
/*N*/                 pLowerFrm->InvalidatePage( pPage );
/*N*/                 if( !pLowerFrm->IsFlowFrm() ||
/*N*/                     !SwFlowFrm::CastFlowFrm( pLowerFrm )->HasFollow() )
/*N*/                     pLowerFrm->InvalidateNextPos( TRUE );
/*N*/                 if ( pLowerFrm->IsTxtFrm() )
/*N*/                     ((SwCntntFrm*)pLowerFrm)->Prepare( PREP_ADJUST_FRM );
/*N*/                 if ( pLowerFrm->IsInSct() )
/*N*/ 				{
/*N*/                     pLowerFrm = pLowerFrm->FindSctFrm();
/*N*/                     if( IsAnLower( pLowerFrm ) )
/*N*/ 					{
/*N*/                         pLowerFrm->_InvalidateSize();
/*N*/                         pLowerFrm->InvalidatePage( pPage );
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/                 // variable size of body frame has shrinked. Thus, invalidate
/*N*/                 // all lowers not matching the new body size and the dedicated
/*N*/                 // new last lower.
/*N*/                 if( bVert )
/*N*/                 {
/*N*/                     SwTwips nBot = Frm().Left() + Prt().Left();
/*N*/                     while ( pLowerFrm->GetPrev() && pLowerFrm->Frm().Left() < nBot )
/*N*/                     {
/*N*/                         pLowerFrm->_InvalidateAll();
/*N*/                         pLowerFrm->InvalidatePage( pPage );
/*N*/                         pLowerFrm = pLowerFrm->GetPrev();
/*N*/                     }
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     SwTwips nBot = Frm().Top() + Prt().Bottom();
/*N*/                     while ( pLowerFrm->GetPrev() && pLowerFrm->Frm().Top() > nBot )
/*N*/                     {
/*N*/                         pLowerFrm->_InvalidateAll();
/*N*/                         pLowerFrm->InvalidatePage( pPage );
/*N*/                         pLowerFrm = pLowerFrm->GetPrev();
/*N*/                     }
/*N*/                 }
/*N*/                 if ( pLowerFrm )
/*N*/ 				{
/*N*/                     pLowerFrm->_InvalidateSize();
/*N*/                     pLowerFrm->InvalidatePage( pPage );
/*N*/                     if ( pLowerFrm->IsTxtFrm() )
/*N*/                         ((SwCntntFrm*)pLowerFrm)->Prepare( PREP_ADJUST_FRM );
/*N*/                     if ( pLowerFrm->IsInSct() )
/*N*/ 					{
/*N*/                         pLowerFrm = pLowerFrm->FindSctFrm();
/*N*/                         if( IsAnLower( pLowerFrm ) )
/*N*/ 						{
/*N*/                             pLowerFrm->_InvalidateSize();
/*N*/                             pLowerFrm->InvalidatePage( pPage );
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		return;
/*N*/     } // end of { special case }
/*N*/ 
/*N*/ 
/*N*/     // Invalidate page for content only once.
/*N*/     bool bInvaPageForCntnt = true;
/*N*/ 
/*N*/     // Declare booleans <bFixChgd> and <bVarChgd>, indicating for text frame
/*N*/     // adjustment, if fixed/variable size has changed.
/*N*/     bool bFixChgd, bVarChgd;
/*N*/     if( bVert == pLowerFrm->IsNeighbourFrm() )
/*N*/ 	{
/*N*/ 		bFixChgd = bWidthChgd;
/*N*/ 		bVarChgd = bHeightChgd;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bFixChgd = bHeightChgd;
/*N*/ 		bVarChgd = bWidthChgd;
/*N*/ 	}
/*N*/ 
/*N*/     // Declare const unsigned short <nFixWidth> and init it this frame types
/*N*/     // which has fixed width in vertical respectively horizontal layout.
/*N*/     // In vertical layout these are neighbour frames (cell and column frames),
/*N*/     //      header frames and footer frames.
/*N*/     // In horizontal layout these are all frames, which aren't neighbour frames.
/*N*/     const USHORT nFixWidth = bVert ? (FRM_NEIGHBOUR | FRM_HEADFOOT)
/*N*/                                    : ~FRM_NEIGHBOUR;
/*N*/ 
/*N*/     // Declare const unsigned short <nFixHeight> and init it this frame types
/*N*/     // which has fixed height in vertical respectively horizontal layout.
/*N*/     // In vertical layout these are all frames, which aren't neighbour frames,
/*N*/     //      header frames, footer frames, body frames or foot note container frames.
/*N*/     // In horizontal layout these are neighbour frames.
/*N*/     const USHORT nFixHeight= bVert ? ~(FRM_NEIGHBOUR | FRM_HEADFOOT | FRM_BODYFTNC)
/*N*/                                    : FRM_NEIGHBOUR;
/*N*/ 
/*N*/     // Travel through all lowers using <GetNext()>
/*N*/     while ( pLowerFrm )
/*N*/     {
/*N*/         if ( pLowerFrm->IsTxtFrm() )
/*N*/ 		{
/*N*/             // Text frames will only be invalidated - prepare invalidation
/*N*/ 			if ( bFixChgd )
/*N*/                 static_cast<SwCntntFrm*>(pLowerFrm)->Prepare( PREP_FIXSIZE_CHG );
/*N*/ 			if ( bVarChgd )
/*N*/                 static_cast<SwCntntFrm*>(pLowerFrm)->Prepare( PREP_ADJUST_FRM );
/*N*/ 		}
/*N*/         else
/*N*/         {
/*N*/             // If lower isn't a table, row, cell or section frame, adjust its
/*N*/             // frame size.
/*N*/             USHORT nType = pLowerFrm->GetType();
/*N*/             if ( !(nType & (FRM_TAB|FRM_ROW|FRM_CELL|FRM_SECTION)) )
/*N*/             {
/*N*/                 if ( bWidthChgd )
/*N*/                 {
/*N*/                     if( nType & nFixWidth )
/*N*/                     {
/*N*/                         // Considering previous conditions:
/*N*/                         // In vertical layout set width of column, header and
/*N*/                         // footer frames to its upper width.
/*N*/                         // In horizontal layout set width of header, footer,
/*N*/                         // foot note container, foot note, body and no-text
/*N*/                         // frames to its upper width.
/*N*/                         pLowerFrm->Frm().Width( Prt().Width() );
/*N*/                     }
/*N*/                     else if( rOldSize.Width() && !pLowerFrm->IsFtnFrm() )
/*N*/                     {
/*N*/                         // Adjust frame width proportional, if lower isn't a
/*N*/                         // foot note frame and condition <nType & nFixWidth>
/*N*/                         // isn't true.
/*N*/                         // Considering previous conditions:
/*N*/                         // In vertical layout these are foot note container,
/*N*/                         // body and no-text frames.
/*N*/                         // In horizontal layout these are column and no-text frames.
/*N*/                         // OD 24.10.2002 #97265# - <double> calculation
/*N*/                         // Perform <double> calculation of new width, if
/*N*/                         // one of the coefficients is greater than 50000
/*N*/                         SwTwips nNewWidth;
/*N*/                         if ( (pLowerFrm->Frm().Width() > 50000) ||
/*N*/                              (Prt().Width() > 50000) )
/*N*/                         {
/*N*/                             double nNewWidthTmp =
/*N*/                                 ( double(pLowerFrm->Frm().Width())
/*N*/                                   * double(Prt().Width()) )
/*N*/                                 / double(rOldSize.Width());
/*N*/                             nNewWidth = SwTwips(nNewWidthTmp);
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             nNewWidth =
/*N*/                                 (pLowerFrm->Frm().Width() * Prt().Width()) / rOldSize.Width();
/*N*/                         }
/*N*/                         pLowerFrm->Frm().Width( nNewWidth );
/*N*/                     }
/*N*/                 }
/*N*/                 if ( bHeightChgd )
/*N*/                 {
/*N*/                     if( nType & nFixHeight )
/*N*/                     {
/*N*/                         // Considering previous conditions:
/*N*/                         // In vertical layout set height of foot note and
/*N*/                         // no-text frames to its upper height.
/*N*/                         // In horizontal layout set height of column frames
/*N*/                         // to its upper height.
/*N*/                         pLowerFrm->Frm().Height( Prt().Height() );
/*N*/                     }
/*N*/                     // OD 01.10.2002 #102211#
/*N*/                     // add conditions <!pLowerFrm->IsHeaderFrm()> and
/*N*/                     // <!pLowerFrm->IsFooterFrm()> in order to avoid that
/*N*/                     // the <Grow> of header or footer are overwritten.
/*N*/                     // NOTE: Height of header/footer frame is determined by contents.
/*N*/                     else if ( rOldSize.Height() &&
/*N*/                               !pLowerFrm->IsFtnFrm() &&
/*N*/                               !pLowerFrm->IsHeaderFrm() &&
/*N*/                               !pLowerFrm->IsFooterFrm()
/*N*/                             )
/*N*/                     {
/*N*/                         // Adjust frame height proportional, if lower isn't a
/*N*/                         // foot note, a header or a footer frame and
/*N*/                         // condition <nType & nFixHeight> isn't true.
/*N*/                         // Considering previous conditions:
/*N*/                         // In vertical layout these are column, foot note container,
/*N*/                         // body and no-text frames.
/*N*/                         // In horizontal layout these are column, foot note
/*N*/                         // container, body and no-text frames.
/*N*/ 
/*N*/                         // OD 29.10.2002 #97265# - special case for page lowers
/*N*/                         // The page lowers that have to be adjusted on page height
/*N*/                         // change are the body frame and the foot note container
/*N*/                         // frame.
/*N*/                         // In vertical layout the height of both is directly
/*N*/                         // adjusted to the page height change.
/*N*/                         // In horizontal layout the height of the body frame is
/*N*/                         // directly adjsuted to the page height change and the
/*N*/                         // foot note frame height isn't touched, because its
/*N*/                         // determined by its content.
/*N*/                         // OD 31.03.2003 #108446# - apply special case for page
/*N*/                         // lowers - see description above - also for section columns.
/*N*/                         if ( IsPageFrm() ||
/*N*/                              ( IsColumnFrm() && IsInSct() )
/*N*/                            )
/*N*/                         {
/*N*/                             ASSERT( pLowerFrm->IsBodyFrm() || pLowerFrm->IsFtnContFrm(),
/*N*/                                     "ChgLowersProp - only for body or foot note container" );
/*N*/                             if ( pLowerFrm->IsBodyFrm() || pLowerFrm->IsFtnContFrm() )
/*N*/                             {
/*N*/                                 if ( IsVertical() || pLowerFrm->IsBodyFrm() )
/*N*/                                 {
/*N*/                                     SwTwips nNewHeight =
/*N*/                                             pLowerFrm->Frm().Height() +
/*N*/                                             ( Prt().Height() - rOldSize.Height() );
/*N*/                                     if ( nNewHeight < 0)
/*N*/                                     {
/*N*/                                         // OD 01.04.2003 #108446# - adjust assertion condition and text
/*N*/                                         ASSERT( !( IsPageFrm() &&
/*N*/                                                    (pLowerFrm->Frm().Height()>0) &&
/*N*/                                                    (pLowerFrm->IsValid()) ),
/*N*/                                                     "ChgLowersProg - negative height for lower.");
/*N*/                                         nNewHeight = 0;
/*N*/                                     }
/*N*/                                     pLowerFrm->Frm().Height( nNewHeight );
/*N*/                                 }
/*N*/                             }
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             SwTwips nNewHeight;
/*N*/                             // OD 24.10.2002 #97265# - <double> calculation
/*N*/                             // Perform <double> calculation of new height, if
/*N*/                             // one of the coefficients is greater than 50000
/*N*/                             if ( (pLowerFrm->Frm().Height() > 50000) ||
/*N*/                                  (Prt().Height() > 50000) )
/*N*/                             {
/*N*/                                 double nNewHeightTmp =
/*N*/                                     ( double(pLowerFrm->Frm().Height())
/*N*/                                       * double(Prt().Height()) )
/*N*/                                     / double(rOldSize.Height());
/*N*/                                 nNewHeight = SwTwips(nNewHeightTmp);
/*N*/                             }
/*N*/                             else
/*N*/                             {
/*N*/                                 nNewHeight = ( pLowerFrm->Frm().Height()
/*N*/                                              * Prt().Height() ) / rOldSize.Height();
/*N*/                             }
/*N*/                             if( !pLowerFrm->GetNext() )
/*N*/                             {
/*N*/                                 SwTwips nSum = Prt().Height();
/*N*/                                 SwFrm* pTmp = Lower();
/*N*/                                 while( pTmp->GetNext() )
/*N*/                                 {
/*N*/                                     if( !pTmp->IsFtnContFrm() || !pTmp->IsVertical() )
/*N*/                                         nSum -= pTmp->Frm().Height();
/*N*/                                     pTmp = pTmp->GetNext();
/*N*/                                 }
/*N*/                                 if( nSum - nNewHeight == 1 &&
/*N*/                                     nSum == pLowerFrm->Frm().Height() )
/*N*/                                     nNewHeight = nSum;
/*N*/                             }
/*N*/                             pLowerFrm->Frm().Height( nNewHeight );
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/             }
/*N*/         } // end of else { NOT text frame }
/*N*/ 
/*N*/         pLowerFrm->_InvalidateAll();
/*N*/         if ( bInvaPageForCntnt && pLowerFrm->IsCntntFrm() )
/*N*/         {
/*N*/             pLowerFrm->InvalidatePage();
/*N*/             bInvaPageForCntnt = false;
/*N*/         }
/*N*/ 
/*N*/         if ( !pLowerFrm->GetNext() && pLowerFrm->IsRetoucheFrm() )
/*N*/ 		{
/*N*/ 			//Wenn ein Wachstum stattgefunden hat, und die untergeordneten
/*N*/ 			//zur Retouche faehig sind (derzeit Tab, Section und Cntnt), so
/*N*/ 			//trigger ich sie an.
/*N*/ 			if ( rOldSize.Height() < Prt().SSize().Height() ||
/*N*/ 				 rOldSize.Width() < Prt().SSize().Width() )
/*N*/                 pLowerFrm->SetRetouche();
/*N*/ 		}
/*N*/         pLowerFrm = pLowerFrm->GetNext();
/*N*/ 	}
/*N*/ 
/*N*/     // Finally adjust the columns if width is set to auto
/*N*/     // Possible optimisation: execute this code earlier in this function and
/*N*/     // return???
/*N*/     if ( ( bVert && bHeightChgd || ! bVert && bWidthChgd ) &&
/*N*/            Lower()->IsColumnFrm() )
/*N*/     {
/*N*/         // get column attribute
/*N*/ 	    const SwFmtCol* pColAttr = NULL;
/*N*/         if ( IsPageBodyFrm() )
/*N*/         {
/*N*/             ASSERT( GetUpper()->IsPageFrm(), "Upper is not page frame" )
/*N*/             pColAttr = &GetUpper()->GetFmt()->GetCol();
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             ASSERT( IsFlyFrm() || IsSctFrm(), "Columns not in fly or section" )
/*N*/             pColAttr = &GetFmt()->GetCol();
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pColAttr->IsOrtho() && pColAttr->GetNumCols() > 1 )
/*N*/ 		    AdjustColumns( pColAttr, sal_False, sal_True );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::Format()
|*
|*	Beschreibung:		"Formatiert" den Frame; Frm und PrtArea.
|*						Die Fixsize wird hier nicht eingestellt.
|*	Ersterstellung		MA 28. Jul. 92
|*	Letzte Aenderung	MA 21. Mar. 95
|*
|*************************************************************************/
/*N*/ void SwLayoutFrm::Format( const SwBorderAttrs *pAttrs )
/*N*/ {
/*N*/ 	ASSERT( pAttrs, "LayoutFrm::Format, pAttrs ist 0." );
/*N*/ 
/*N*/ 	if ( bValidPrtArea && bValidSize )
/*?*/ 		return;
/*N*/ 
/*N*/     const USHORT nLeft = (USHORT)pAttrs->CalcLeft( this );
/*N*/     const USHORT nUpper = pAttrs->CalcTop();
/*N*/ 
/*N*/     const USHORT nRight = (USHORT)((SwBorderAttrs*)pAttrs)->CalcRight( this );
/*N*/     const USHORT nLower = pAttrs->CalcBottom();
/*N*/     BOOL bVert = IsVertical() && !IsPageFrm();
/*N*/     SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
/*N*/ 	if ( !bValidPrtArea )
/*N*/ 	{
/*N*/ 		bValidPrtArea = TRUE;
/*N*/         (this->*fnRect->fnSetXMargins)( nLeft, nRight );
/*N*/         (this->*fnRect->fnSetYMargins)( nUpper, nLower );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bValidSize )
/*N*/ 	{
/*N*/         if ( !HasFixSize() )
/*N*/ 		{
/*N*/             const SwTwips nBorder = nUpper + nLower;
/*N*/ 			const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
/*N*/ 			SwTwips nMinHeight = rSz.GetSizeType() == ATT_MIN_SIZE ? rSz.GetHeight() : 0;
/*N*/ 			do
/*N*/ 			{	bValidSize = TRUE;
/*N*/ 
/*N*/ 				//Die Groesse in der VarSize wird durch den Inhalt plus den
/*N*/ 				//Raendern bestimmt.
/*N*/ 				SwTwips nRemaining = 0;
/*N*/ 				SwFrm *pFrm = Lower();
/*N*/ 				while ( pFrm )
/*N*/                 {   nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 					if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
/*?*/ 					// Dieser TxtFrm waere gern ein bisschen groesser
/*?*/ 						nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
/*?*/                                       - (pFrm->Prt().*fnRect->fnGetHeight)();
/*N*/ 					else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
/*N*/ 						nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
/*N*/ 					pFrm = pFrm->GetNext();
/*N*/ 				}
/*N*/ 				nRemaining += nBorder;
/*N*/ 				nRemaining = Max( nRemaining, nMinHeight );
/*N*/                 const SwTwips nDiff = nRemaining-(Frm().*fnRect->fnGetHeight)();
/*N*/                 const long nOldLeft = (Frm().*fnRect->fnGetLeft)();
/*N*/                 const long nOldTop = (Frm().*fnRect->fnGetTop)();
/*N*/ 				if ( nDiff )
/*N*/ 				{
/*N*/ 					if ( nDiff > 0 )
/*N*/                         Grow( nDiff );
/*N*/ 					else
/*?*/                         Shrink( -nDiff );
/*N*/ 					//Schnell auf dem kurzen Dienstweg die Position updaten.
/*N*/ 					MakePos();
/*N*/ 				}
/*N*/ 				//Unterkante des Uppers nicht ueberschreiten.
/*N*/                 if ( GetUpper() && (Frm().*fnRect->fnGetHeight)() )
/*N*/ 				{
/*N*/                     const SwTwips nLimit = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/                     if( (this->*fnRect->fnSetLimit)( nLimit ) &&
/*N*/                         nOldLeft == (Frm().*fnRect->fnGetLeft)() &&
/*N*/                         nOldTop  == (Frm().*fnRect->fnGetTop)() )
/*?*/                         bValidSize = bValidPrtArea = TRUE;
/*N*/ 				}
/*N*/ 			} while ( !bValidSize );
/*N*/ 		}
/*N*/ 		else if ( GetType() & 0x0018 )
/*N*/ 		{
/*N*/ 			do
/*N*/ 			{	if ( Frm().Height() != pAttrs->GetSize().Height() )
/*N*/ 					ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
/*N*/ 				bValidSize = TRUE;
/*N*/ 				MakePos();
/*N*/ 			} while ( !bValidSize );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bValidSize = TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::InvalidatePercentLowers()
|*
|*	Ersterstellung		MA 13. Jun. 96
|*	Letzte Aenderung	MA 13. Jun. 96
|*
|*************************************************************************/
/*N*/ static void InvaPercentFlys( SwFrm *pFrm, SwTwips nDiff )
/*N*/ {
/*N*/ 	ASSERT( pFrm->GetDrawObjs(), "Can't find any Objects" );
/*N*/ 	for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = (*pFrm->GetDrawObjs())[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 			const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
/*N*/ 			if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
/*N*/             {
/*N*/                 BOOL bNotify = TRUE;
/*N*/                 // If we've a fly with more than 90% relative height...
/*N*/                 if( rSz.GetHeightPercent() > 90 && pFly->GetAnchor() &&
/*N*/                     rSz.GetHeightPercent() != 0xFF && nDiff )
/*N*/                 {
/*?*/                     const SwFrm *pRel = pFly->IsFlyLayFrm() ? pFly->GetAnchor():
/*?*/                                         pFly->GetAnchor()->GetUpper();
/*?*/                     // ... and we have already more than 90% height and we
/*?*/                     // not allow the text to go through...
/*?*/                     // then a notifycation could cause an endless loop, e.g.
/*?*/                     // 100% height and no text wrap inside a cell of a table.
/*?*/                     if( pFly->Frm().Height()*10 >
/*?*/                         ( nDiff + pRel->Prt().Height() )*9 &&
/*?*/                         pFly->GetFmt()->GetSurround().GetSurround() !=
/*?*/                         SURROUND_THROUGHT )
/*?*/                        bNotify = FALSE;
/*N*/                 }
/*N*/                 if( bNotify )
/*N*/                     pFly->InvalidateSize();
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwLayoutFrm::InvaPercentLowers( SwTwips nDiff )
/*N*/ {
/*N*/ 	if ( GetDrawObjs() )
/*N*/         ::binfilter::InvaPercentFlys( this, nDiff );
/*N*/ 
/*N*/ 	SwFrm *pFrm = ContainsCntnt();
/*N*/ 	if ( pFrm )
/*N*/ 		do
/*N*/ 		{
/*N*/ 			if ( pFrm->IsInTab() && !IsTabFrm() )
/*N*/ 			{
/*N*/ 				SwFrm *pTmp = pFrm->FindTabFrm();
/*N*/ 				ASSERT( pTmp, "Where's my TabFrm?" );
/*N*/ 				if( IsAnLower( pTmp ) )
/*N*/ 					pFrm = pTmp;
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( pFrm->IsTabFrm() )
/*N*/ 			{
/*N*/ 				const SwFmtFrmSize &rSz = ((SwLayoutFrm*)pFrm)->GetFmt()->GetFrmSize();
/*N*/ 				if ( rSz.GetWidthPercent() || rSz.GetHeightPercent() )
/*?*/ 					pFrm->InvalidatePrt();
/*N*/ 			}
/*N*/ 			else if ( pFrm->GetDrawObjs() )
/*N*/                 ::binfilter::InvaPercentFlys( pFrm, nDiff );
/*N*/ 			pFrm = pFrm->FindNextCnt();
/*N*/ 		} while ( pFrm && IsAnLower( pFrm ) ) ;
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::CalcRel()
|*
|*	Ersterstellung		MA 13. Jun. 96
|*	Letzte Aenderung	MA 10. Oct. 96
|*
|*************************************************************************/
/*N*/ long SwLayoutFrm::CalcRel( const SwFmtFrmSize &rSz, BOOL bWidth ) const
/*N*/ {
/*N*/ 	ASSERT( bWidth, "NonFlys, CalcRel: width only" );
/*N*/ 
/*N*/ 	long nRet	  = rSz.GetWidth(),
/*N*/ 		 nPercent = rSz.GetWidthPercent();
/*N*/ 
/*N*/ 	if ( nPercent )
/*N*/ 	{
/*?*/ 		const SwFrm *pRel = GetUpper();
/*?*/ 		long nRel = LONG_MAX;
/*?*/ 		const ViewShell *pSh = GetShell();
/*?*/ 		if ( pRel->IsPageBodyFrm() && GetFmt()->GetDoc()->IsBrowseMode() &&
/*?*/ 			 pSh && pSh->VisArea().Width())
/*?*/ 		{
/*?*/ 			nRel = pSh->VisArea().Width();
/*?*/ 			const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
/*?*/ 			nRel -= 2*aBorder.Width();
/*?*/ 			long nDiff = nRel - pRel->Prt().Width();
/*?*/ 			if ( nDiff > 0 )
/*?*/ 				nRel -= nDiff;
/*?*/ 		}
/*?*/ 		nRel = Min( nRel, pRel->Prt().Width() );
/*?*/ 		nRet = nRel * nPercent / 100;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ long MA_FASTCALL lcl_CalcMinColDiff( SwLayoutFrm *pLayFrm )
/*N*/ {
/*N*/ 	long nDiff = 0, nFirstDiff = 0;
/*N*/ 	SwLayoutFrm *pCol = (SwLayoutFrm*)pLayFrm->Lower();
/*N*/ 	ASSERT( pCol, "Where's the columnframe?" );
/*N*/ 	SwFrm *pFrm = pCol->Lower();
/*N*/ 	do
/*N*/     {
/*N*/         if( pFrm && pFrm->IsBodyFrm() )
/*N*/             pFrm = ((SwBodyFrm*)pFrm)->Lower();
/*N*/         if ( pFrm && pFrm->IsTxtFrm() )
/*N*/ 		{
/*N*/ 			const long nTmp = ((SwTxtFrm*)pFrm)->FirstLineHeight();
/*N*/ 			if ( nTmp != USHRT_MAX )
/*N*/ 			{
/*N*/ 				if ( pCol == pLayFrm->Lower() )
/*N*/ 					nFirstDiff = nTmp;
/*N*/ 				else
/*N*/ 					nDiff = nDiff ? Min( nDiff, nTmp ) : nTmp;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		//Leere Spalten ueberspringen!
/*N*/ 		pCol = (SwLayoutFrm*)pCol->GetNext();
/*N*/ 		while ( pCol && 0 == (pFrm = pCol->Lower()) )
/*?*/ 			pCol = (SwLayoutFrm*)pCol->GetNext();
/*N*/ 
/*N*/ 	} while ( pFrm && pCol );
/*N*/ 
/*N*/ 	return nDiff ? nDiff : nFirstDiff ? nFirstDiff : 240;
/*N*/ }

/*N*/ BOOL lcl_IsFlyHeightClipped( SwLayoutFrm *pLay )
/*N*/ {
/*N*/ 	SwFrm *pFrm = pLay->ContainsCntnt();
/*N*/ 	while ( pFrm )
/*N*/ 	{	if ( pFrm->IsInTab() )
/*?*/ 			pFrm = pFrm->FindTabFrm();
/*N*/ 
/*N*/ 		if ( pFrm->GetDrawObjs() )
/*N*/ 		{
/*N*/ 			USHORT nCnt = pFrm->GetDrawObjs()->Count();
/*N*/ 			for ( USHORT i = 0; i < nCnt; ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pO = (*pFrm->GetDrawObjs())[i];
/*N*/                 if ( pO->IsWriterFlyFrame() )
/*N*/                 {
/*N*/                     SwFlyFrm* pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/                     if( pFly->IsHeightClipped() && (!pFly->IsFlyFreeFrm() ||
/*N*/                         ((SwFlyFreeFrm*)pFly)->GetPage() ) )
/*?*/                         return TRUE;
/*N*/                 }
/*N*/ 			}
/*N*/ 		}
/*N*/ 		pFrm = pFrm->FindNextCnt();
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SwLayoutFrm::FormatWidthCols( const SwBorderAttrs &rAttrs,
/*N*/ 	const SwTwips nBorder, const SwTwips nMinHeight )
/*N*/ {
/*N*/ 	//Wenn Spalten im Spiel sind, so wird die Groesse an der
/*N*/ 	//letzten Spalte ausgerichtet.
/*N*/ 	//1. Inhalt formatieren.
/*N*/ 	//2. Hoehe der letzten Spalte ermitteln, wenn diese zu
/*N*/ 	//	 zu gross ist muss der Fly wachsen.
/*N*/ 	//	 Der Betrag um den der Fly waechst ist aber nicht etwa
/*N*/ 	//	 der Betrag des Ueberhangs, denn wir muessen davon
/*N*/ 	//	 ausgehen, dass etwas Masse zurueckfliesst und so
/*N*/ 	//	 zusaetzlicher Platz geschaffen wird.
/*N*/ 	//	 Im Ersten Ansatz ist der Betrag um den gewachsen wird
/*N*/ 	//	 der Ueberhang geteilt durch die Spaltenanzahl oder
/*N*/ 	//	 der Ueberhang selbst wenn er kleiner als die Spalten-
/*N*/ 	//	 anzahl ist.
/*N*/ 	//3. Weiter mit 1. bis zur Stabilitaet.
/*N*/ 
/*N*/ 	const SwFmtCol &rCol = rAttrs.GetAttrSet().GetCol();
/*N*/ 	const USHORT nNumCols = rCol.GetNumCols();
/*N*/ 
/*N*/ 	FASTBOOL bEnd = FALSE;
/*N*/ 	FASTBOOL bBackLock = FALSE;
/*N*/ 	SwViewImp *pImp = GetShell() ? GetShell()->Imp() : 0;
/*N*/ 	{
/*N*/ 		// Zugrunde liegender Algorithmus
/*N*/ 		// Es wird versucht, eine optimale Hoehe fuer die Spalten zu finden.
/*N*/ 		// nMinimum beginnt mit der uebergebenen Mindesthoehe und wird dann als
/*N*/ 		// Maximum der Hoehen gepflegt, bei denen noch Spalteninhalt aus einer
/*N*/ 		// Spalte herausragt.
/*N*/ 		// nMaximum beginnt bei LONG_MAX und wird als Minimum der Hoehen gepflegt,
/*N*/ 		// bei denen der Inhalt gepasst hat.
/*N*/ 		// Bei spaltigen Bereichen beginnt nMaximum bei dem maximalen Wert, den
/*N*/ 		// die Umgebung vorgibt, dies kann natuerlich ein Wert sein, bei dem noch
/*N*/ 		// Inhalt heraushaengt.
/*N*/ 		// Es werden die Spalten formatiert, wenn Inhalt heraushaengt, wird nMinimum
/*N*/ 		// ggf. angepasst, dann wird gewachsen, mindestens um nMinDiff, aber nicht ueber
/*N*/ 		// ein groesseres nMaximum hinaus. Wenn kein Inhalt heraushaengt, sondern
/*N*/ 		// noch Luft in einer Spalte ist, schrumpfen wir entsprechend, mindestens um
/*N*/ 		// nMinDiff, aber nicht unter das nMinimum.
/*N*/ 		// Abgebrochen wird, wenn kein Inhalt mehr heraushaengt und das Minimum sich auf
/*N*/ 		// weniger als ein MinDiff dem Maximum angenaehert hat oder das von der
/*N*/ 		// Umgebung vorgegebene Maximum erreicht ist und trotzdem Inhalt heraus-
/*N*/ 		// haengt.
/*N*/ 
/*N*/ 		// Kritik an der Implementation
/*N*/ 		// 1. Es kann theoretisch Situationen geben, in denen der Inhalt in einer geringeren
/*N*/ 		// Hoehe passt und in einer groesseren Hoehe nicht passt. Damit der Code robust
/*N*/ 		// gegen solche Verhaeltnisse ist, sind ein paar Abfragen bezgl. Minimum und Maximum
/*N*/ 		// drin, die wahrscheinlich niemals zuschlagen koennen.
/*N*/ 		// 2. Es wird fuer das Schrumpfen das gleiche nMinDiff benutzt wie fuer das Wachstum,
/*N*/ 		// das nMinDiff ist allerdings mehr oder weniger die kleinste erste Zeilenhoehe und
/*N*/ 		// als Mindestwert fuer das Schrumpfen nicht unbedingt optimal.
/*N*/ 
/*N*/ 		long nMinimum = nMinHeight;
/*N*/ 		long nMaximum;
/*N*/ 		BOOL bNoBalance = FALSE;
/*N*/         SWRECTFN( this )
/*N*/ 		if( IsSctFrm() )
/*N*/ 		{
/*?*/             nMaximum = (Frm().*fnRect->fnGetHeight)() - nBorder +
/*?*/                        (Frm().*fnRect->fnBottomDist)(
/*?*/                                         (GetUpper()->*fnRect->fnGetPrtBottom)() );
/*?*/             nMaximum += GetUpper()->Grow( LONG_MAX PHEIGHT, TRUE );
/*?*/ 			if( nMaximum < nMinimum )
/*?*/ 			{
/*?*/ 				if( nMaximum < 0 )
/*?*/ 					nMinimum = nMaximum = 0;
/*?*/ 				else
/*?*/ 					nMinimum = nMaximum;
/*?*/ 			}
/*?*/ 			if( nMaximum > BROWSE_HEIGHT )
/*?*/ 				nMaximum = BROWSE_HEIGHT;
/*?*/ 
/*?*/ 			bNoBalance = ((SwSectionFrm*)this)->GetSection()->GetFmt()->
/*?*/ 						 GetBalancedColumns().GetValue();
/*?*/ 			SwFrm* pAny = ContainsAny();
/*?*/ 			if( bNoBalance ||
/*?*/                 ( !(Frm().*fnRect->fnGetHeight)() && pAny ) )
/*?*/ 			{
/*?*/                 long nTop = (this->*fnRect->fnGetTopMargin)();
/*?*/                 (Frm().*fnRect->fnAddBottom)( nMaximum );
/*?*/                 if( nTop > nMaximum )
/*?*/                     nTop = nMaximum;
/*?*/                 (this->*fnRect->fnSetYMargins)( nTop, 0 );
/*?*/ 			}
/*?*/ 			if( !pAny && !((SwSectionFrm*)this)->IsFtnLock() )
/*?*/ 			{
/*?*/ 				SwFtnContFrm* pFtnCont = ((SwSectionFrm*)this)->ContainsFtnCont();
/*?*/ 				if( pFtnCont )
/*?*/ 				{
/*?*/ 					SwFrm* pFtnAny = pFtnCont->ContainsAny();
/*?*/ 					if( pFtnAny && pFtnAny->IsValid() )
/*?*/ 					{
/*?*/ 						bBackLock = TRUE;
/*?*/ 						((SwSectionFrm*)this)->SetFtnLock( TRUE );
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			nMaximum = LONG_MAX;
/*N*/ 		do
/*N*/ 		{
/*N*/ 			//Kann eine Weile dauern, deshalb hier auf Waitcrsr pruefen.
/*N*/ 			if ( pImp )
/*N*/ 				pImp->CheckWaitCrsr();
/*N*/ 
/*N*/ 			bValidSize = TRUE;
/*N*/ 			//Erstmal die Spalten formatieren, das entlastet den
/*N*/ 			//Stack ein wenig.
/*N*/ 			//Bei der Gelegenheit stellen wir auch gleich mal die
/*N*/ 			//Breiten und Hoehen der Spalten ein (so sie denn falsch sind).
/*N*/ 			SwLayoutFrm *pCol = (SwLayoutFrm*)Lower();
/*N*/             SwTwips nAvail = (Prt().*fnRect->fnGetWidth)();
/*N*/             USHORT nPrtWidth = (USHORT)nAvail;
/*N*/ 			for ( USHORT i = 0; i < nNumCols; ++i )
/*N*/ 			{
/*N*/                 SwTwips nWidth = rCol.CalcColWidth( i, nPrtWidth );
/*N*/ 				if ( i == (nNumCols - 1) ) //Dem Letzten geben wir wie
/*N*/ 					nWidth = nAvail;	   //immer den Rest.
/*N*/ 
/*N*/                 SwTwips nWidthDiff = nWidth - (pCol->Frm().*fnRect->fnGetWidth)();
/*N*/                 if( nWidthDiff )
/*N*/ 				{
/*N*/                     (pCol->Frm().*fnRect->fnAddRight)( nWidthDiff );
/*N*/ 					pCol->_InvalidatePrt();
/*N*/ 					if ( pCol->GetNext() )
/*N*/ 						pCol->GetNext()->_InvalidatePos();
/*N*/ 				}
/*N*/ 
/*N*/                 SwTwips nHeightDiff = (Prt().*fnRect->fnGetHeight)() -
/*N*/                                       (pCol->Frm().*fnRect->fnGetHeight)();
/*N*/                 if( nHeightDiff )
/*N*/ 				{
/*N*/                     (pCol->Frm().*fnRect->fnAddBottom)( nHeightDiff );
/*N*/ 					pCol->_InvalidatePrt();
/*N*/ 				}
/*N*/ 				pCol->Calc();
/*N*/ 				// ColumnFrms besitzen jetzt einen BodyFrm, der auch kalkuliert werden will
/*N*/ 				pCol->Lower()->Calc();
/*N*/ 				if( pCol->Lower()->GetNext() )
/*N*/ 					pCol->Lower()->GetNext()->Calc();  // SwFtnCont
/*N*/ 				pCol = (SwLayoutFrm*)pCol->GetNext();
/*N*/                 nAvail -= nWidth;
/*N*/ 			}
/*N*/ 
/*N*/             // OD 14.03.2003 #i11760# - adjust method call <CalcCntnt(..)>:
/*N*/             // Set 3rd parameter to true in order to forbid format of follow
/*N*/             // during format of text frames. (2nd parameter = default value.)
/*N*/             // OD 11.04.2003 #108824# - undo change of fix for #i11760# - allow
/*N*/             // follow formatting for text frames.
/*N*/             ::binfilter::CalcCntnt( this );
/*N*/ 
/*N*/ 			pCol = (SwLayoutFrm*)Lower();
/*N*/ 			ASSERT( pCol && pCol->GetNext(), ":-( Spalten auf Urlaub?");
/*N*/ 			// bMinDiff wird gesetzt, wenn es keine leere Spalte gibt
/*N*/ 			BOOL bMinDiff = TRUE;
/*N*/             // OD 28.03.2003 #108446# - check for all column content and all columns
/*N*/             while ( bMinDiff && pCol )
/*N*/             {
/*N*/                 bMinDiff = 0 != pCol->ContainsCntnt();
/*N*/                 pCol = (SwLayoutFrm*)pCol->GetNext();
/*N*/             }
/*N*/ 			pCol = (SwLayoutFrm*)Lower();
/*N*/             // OD 28.03.2003 #108446# - initialize local variable
/*N*/             SwFrm *pLow = NULL;
/*N*/ 			SwTwips nDiff = 0;
/*N*/ 			SwTwips nMaxFree = 0;
/*N*/ 			SwTwips nAllFree = LONG_MAX;
/*N*/ 			// bFoundLower wird gesetzt, wenn es mind. eine nichtleere Spalte gibt
/*N*/ 			BOOL bFoundLower = FALSE;
/*N*/ 			while( pCol )
/*N*/ 			{
/*N*/ 				SwLayoutFrm* pLay = (SwLayoutFrm*)pCol->Lower();
/*N*/                 SwTwips nInnerHeight = (pLay->Frm().*fnRect->fnGetHeight)() -
/*N*/                                        (pLay->Prt().*fnRect->fnGetHeight)();
/*N*/ 				if( pLay->Lower() )
/*N*/ 				{
/*N*/ 					bFoundLower = TRUE;
/*N*/ 					nInnerHeight += pLay->InnerHeight();
/*N*/ 				}
/*N*/ 				else if( nInnerHeight < 0 )
/*N*/ 					nInnerHeight = 0;
/*N*/ 
/*N*/ 				if( pLay->GetNext() )
/*N*/ 				{
/*N*/ 					bFoundLower = TRUE;
/*N*/ 					pLay = (SwLayoutFrm*)pLay->GetNext();
/*N*/ 					ASSERT( pLay->IsFtnContFrm(),"FtnContainer exspected" );
/*N*/ 					nInnerHeight += pLay->InnerHeight();
/*N*/                     nInnerHeight += (pLay->Frm().*fnRect->fnGetHeight)() -
/*N*/                                     (pLay->Prt().*fnRect->fnGetHeight)();
/*N*/ 				}
/*N*/                 nInnerHeight -= (pCol->Prt().*fnRect->fnGetHeight)();
/*N*/ 				if( nInnerHeight > nDiff )
/*N*/ 				{
/*N*/ 					nDiff = nInnerHeight;
/*N*/ 					nAllFree = 0;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( nMaxFree < -nInnerHeight )
/*N*/ 						nMaxFree = -nInnerHeight;
/*N*/ 					if( nAllFree > -nInnerHeight )
/*N*/ 						nAllFree = -nInnerHeight;
/*N*/ 				}
/*N*/ 				pCol = (SwLayoutFrm*)pCol->GetNext();
/*N*/ 			}
/*N*/ 
/*N*/ 			if ( bFoundLower || ( IsSctFrm() && ((SwSectionFrm*)this)->HasFollow() ) )
/*N*/ 			{
/*N*/ 				SwTwips nMinDiff = ::binfilter::lcl_CalcMinColDiff( this );
/*N*/ 				// Hier wird entschieden, ob wir wachsen muessen, naemlich wenn
/*N*/ 				// ein Spalteninhalt (nDiff) oder ein Fly herausragt.
/*N*/ 				// Bei spaltigen Bereichen wird beruecksichtigt, dass mit dem
/*N*/ 				// Besitz eines nichtleeren Follows die Groesse festgelegt ist.
/*N*/ 				if ( nDiff || ::binfilter::lcl_IsFlyHeightClipped( this ) ||
/*N*/ 					 ( IsSctFrm() && ((SwSectionFrm*)this)->CalcMinDiff( nMinDiff ) ) )
/*N*/ 				{
/*N*/                     long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/ 					// Das Minimum darf nicht kleiner sein als unsere PrtHeight,
/*N*/ 					// solange noch etwas herausragt.
/*N*/                     if( nMinimum < nPrtHeight )
/*N*/                         nMinimum = nPrtHeight;
/*N*/ 					// Es muss sichergestellt sein, dass das Maximum nicht kleiner
/*N*/ 					// als die PrtHeight ist, wenn noch etwas herausragt
/*N*/                     if( nMaximum < nPrtHeight )
/*N*/                         nMaximum = nPrtHeight;  // Robust, aber kann das ueberhaupt eintreten?
/*N*/ 					if( !nDiff ) // wenn nur Flys herausragen, wachsen wir um nMinDiff
/*N*/ 						nDiff = nMinDiff;
/*N*/ 					// Wenn wir um mehr als nMinDiff wachsen wollen, wird dies auf die
/*N*/ 					// Spalten verteilt
/*N*/ 					if ( Abs(nDiff - nMinDiff) > nNumCols && nDiff > (long)nNumCols )
/*N*/ 						nDiff /= nNumCols;
/*N*/ 
/*N*/ 					if ( bMinDiff )
/*N*/ 					{   // Wenn es keinen leeren Spalten gibt, wollen wir mind. um nMinDiff
/*N*/ 						// wachsen. Sonderfall: Wenn wir kleiner als die minimale Frmhoehe
/*N*/ 						// sind und die PrtHeight kleiner als nMinDiff ist, wachsen wir so,
/*N*/ 						// dass die PrtHeight hinterher genau nMinDiff ist.
/*N*/                         long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/                         if ( nFrmHeight > nMinHeight || nPrtHeight >= nMinDiff )
/*N*/ 							nDiff = Max( nDiff, nMinDiff );
/*N*/ 						else if( nDiff < nMinDiff )
/*N*/                             nDiff = nMinDiff - nPrtHeight + 1;
/*N*/ 					}
/*N*/ 					// nMaximum ist eine Groesse, in der der Inhalt gepasst hat,
/*N*/ 					// oder der von der Umgebung vorgegebene Wert, deshalb
/*N*/ 					// brauchen wir nicht ueber diesen Wrt hinauswachsen.
/*N*/                     if( nDiff + nPrtHeight > nMaximum )
/*N*/                         nDiff = nMaximum - nPrtHeight;
/*N*/ 				}
/*N*/ 				else if( nMaximum > nMinimum ) // Wir passen, haben wir auch noch Spielraum?
/*N*/ 				{
/*N*/                     long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
/*N*/                     if ( nMaximum < nPrtHeight )
/*N*/                         nDiff = nMaximum - nPrtHeight; // wir sind ueber eine funktionierende
/*N*/ 						// Hoehe hinausgewachsen und schrumpfen wieder auf diese zurueck,
/*N*/ 						// aber kann das ueberhaupt eintreten?
/*N*/ 					else
/*N*/ 					{   // Wir haben ein neues Maximum, eine Groesse, fuer die der Inhalt passt.
/*N*/                         nMaximum = nPrtHeight;
/*N*/ 						// Wenn der Freiraum in den Spalten groesser ist als nMinDiff und wir
/*N*/ 						// nicht dadurch wieder unter das Minimum rutschen, wollen wir ein wenig
/*N*/ 						// Luft herauslassen.
/*N*/ 						if( !bNoBalance && ( nMaxFree >= nMinDiff && (!nAllFree
/*N*/                             || nMinimum < nPrtHeight - nMinDiff ) ) )
/*N*/ 						{
/*N*/ 							nMaxFree /= nNumCols; // auf die Spalten verteilen
/*N*/ 							nDiff = nMaxFree < nMinDiff ? -nMinDiff : -nMaxFree; // mind. nMinDiff
/*N*/                             if( nPrtHeight + nDiff <= nMinimum ) // Unter das Minimum?
/*N*/ 								nDiff = ( nMinimum - nMaximum ) / 2; // dann lieber die Mitte
/*N*/ 						}
/*N*/ 						else if( nAllFree )
/*N*/ 						{
/*N*/ 							nDiff = -nAllFree;
/*N*/                             if( nPrtHeight + nDiff <= nMinimum ) // Less than minimum?
/*N*/ 								nDiff = ( nMinimum - nMaximum ) / 2; // Take the center
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if( nDiff ) // jetzt wird geschrumpft oder gewachsen..
/*N*/ 				{
/*N*/ 					Size aOldSz( Prt().SSize() );
/*N*/                     long nTop = (this->*fnRect->fnGetTopMargin)();
/*N*/                     nDiff = (Prt().*fnRect->fnGetHeight)() + nDiff + nBorder -
/*N*/                             (Frm().*fnRect->fnGetHeight)();
/*N*/                     (Frm().*fnRect->fnAddBottom)( nDiff );
/*N*/                     (this->*fnRect->fnSetYMargins)( nTop, nBorder - nTop );
/*N*/ 					ChgLowersProp( aOldSz );
/*N*/ 					NotifyFlys();
/*N*/ 
/*N*/ 					//Es muss geeignet invalidiert werden, damit
/*N*/ 					//sich die Frms huebsch ausbalancieren
/*N*/ 					//- Der jeweils erste ab der zweiten Spalte bekommt
/*N*/ 					//	ein InvalidatePos();
/*N*/ 					pCol = (SwLayoutFrm*)Lower()->GetNext();
/*N*/ 					while ( pCol )
/*N*/ 					{
/*N*/ 						pLow = pCol->Lower();
/*N*/ 						if ( pLow )
/*N*/ 							pLow->_InvalidatePos();
/*N*/ 						pCol = (SwLayoutFrm*)pCol->GetNext();
/*N*/ 					}
/*N*/ 					if( IsSctFrm() && ((SwSectionFrm*)this)->HasFollow() )
/*N*/ 					{
/*N*/ 						// Wenn wir einen Follow erzeugt haben, muessen wir
/*N*/ 						// seinem Inhalt die Chance geben, im CalcCntnt
/*N*/ 						// zurueckzufliessen
/*N*/ 						SwCntntFrm* pTmpCntnt =
/*N*/ 							((SwSectionFrm*)this)->GetFollow()->ContainsCntnt();
/*N*/ 						if( pTmpCntnt )
/*N*/ 							pTmpCntnt->_InvalidatePos();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bEnd = TRUE;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bEnd = TRUE;
/*N*/ 
/*N*/ 		} while ( !bEnd || !bValidSize );
/*N*/ 	}
/*N*/     // OD 01.04.2003 #108446# - Don't collect endnotes for sections. Thus, set
/*N*/     // 2nd parameter to <true>.
/*N*/     ::binfilter::CalcCntnt( this, true );
/*N*/ 	if( IsSctFrm() )
/*N*/ 	{
/*N*/         // OD 14.03.2003 #i11760# - adjust 2nd parameter - TRUE --> true
/*N*/         ::binfilter::CalcCntnt( this, true );
/*N*/ 		if( bBackLock )
/*N*/ 			((SwSectionFrm*)this)->SetFtnLock( FALSE );
/*N*/ 	}
/*N*/ }


/*************************************************************************
|*
|*	SwRootFrm::InvalidateAllCntnt()
|*
|*	Ersterstellung		MA 13. Feb. 98
|*	Letzte Aenderung	MA 12. Aug. 00
|*
|*************************************************************************/



/*N*/ void lcl_InvalidateAllCntnt( SwCntntFrm *pCnt, BYTE nInv );

/*N*/ void lcl_InvalidateCntnt( SwCntntFrm *pCnt, BYTE nInv )
/*N*/ {
/*N*/ 	SwCntntFrm *pLastTabCnt = NULL;
/*N*/ 	SwCntntFrm *pLastSctCnt = NULL;
/*N*/ 	while ( pCnt )
/*N*/ 	{
/*N*/ 		if( nInv & INV_SECTION )
/*N*/ 		{
/*?*/ 			if( pCnt->IsInSct() )
/*?*/ 			{
/*?*/ 				// Siehe oben bei Tabellen
/*?*/ 				if( !pLastSctCnt )
/*?*/ 				{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	pLastSctCnt = lcl_InvalidateSection( pCnt, nInv );
/*?*/ 				if( pLastSctCnt == pCnt )
/*?*/ 					pLastSctCnt = NULL;
/*?*/ 			}
/*?*/ #ifdef DBG_UTIL
/*?*/ 			else
/*?*/ 				ASSERT( !pLastSctCnt, "Where's the last SctCntnt?" );
/*?*/ #endif
/*N*/ 		}
/*N*/ 		if( nInv & INV_TABLE )
/*N*/ 		{
/*?*/ 			if( pCnt->IsInTab() )
/*?*/ 			{
/*?*/ 				// Um nicht fuer jeden CntntFrm einer Tabelle das FindTabFrm() zu rufen
/*?*/ 				// und wieder die gleiche Tabelle zu invalidieren, merken wir uns den letzten
/*?*/ 				// CntntFrm der Tabelle und reagieren erst wieder auf IsInTab(), wenn wir
/*?*/ 				// an diesem vorbei sind.
/*?*/ 				// Beim Eintritt in die Tabelle wird der LastSctCnt auf Null gesetzt,
/*?*/ 				// damit Bereiche im Innern der Tabelle richtig invalidiert werden.
/*?*/ 				// Sollte die Tabelle selbst in einem Bereich stehen, so wird an
/*?*/ 				// diesem die Invalidierung bis zu dreimal durchgefuehrt, das ist vertretbar.
/*?*/ 				if( !pLastTabCnt )
/*?*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pLastTabCnt = lcl_InvalidateTable( pCnt->FindTabFrm(), nInv );
/*?*/ 				}
/*?*/ 				if( pLastTabCnt == pCnt )
/*?*/ 				{
/*?*/ 					pLastTabCnt = NULL;
/*?*/ 					pLastSctCnt = NULL;
/*?*/ 				}
/*?*/ 			}
/*?*/ #ifdef DBG_UTIL
/*?*/ 			else
/*?*/ 				ASSERT( !pLastTabCnt, "Where's the last TabCntnt?" );
/*?*/ #endif
/*N*/ 		}
/*N*/ 
/*N*/ 		if( nInv & INV_SIZE )
/*N*/ 			pCnt->Prepare( PREP_CLEAR, 0, FALSE );
/*N*/ 		if( nInv & INV_POS )
/*?*/ 			pCnt->_InvalidatePos();
/*N*/ 		if( nInv & INV_PRTAREA )
/*?*/ 			pCnt->_InvalidatePrt();
/*N*/ 		if ( nInv & INV_LINENUM )
/*?*/ 			pCnt->InvalidateLineNum();
/*N*/ 		if ( pCnt->GetDrawObjs() )
/*N*/ 			lcl_InvalidateAllCntnt( pCnt, nInv );
/*N*/ 		pCnt = pCnt->GetNextCntntFrm();
/*N*/ 	}
/*N*/ }

/*N*/ void lcl_InvalidateAllCntnt( SwCntntFrm *pCnt, BYTE nInv )
/*N*/ {
/*N*/ 	SwDrawObjs &rObjs = *pCnt->GetDrawObjs();
/*N*/ 	for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = rObjs[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)pO)->GetFlyFrm();
/*N*/ 			if ( pFly->IsFlyInCntFrm() )
/*N*/             {
/*?*/ 				::binfilter::lcl_InvalidateCntnt( pFly->ContainsCntnt(), nInv );
/*?*/                 if( nInv & INV_DIRECTION )
/*?*/                     pFly->CheckDirChange();
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwRootFrm::InvalidateAllCntnt( BYTE nInv )
/*N*/ {
/*N*/ 	// Erst werden alle Seitengebundenen FlyFrms abgearbeitet.
/*N*/ 	SwPageFrm *pPage = (SwPageFrm*)Lower();
/*N*/ 	while( pPage )
/*N*/ 	{
/*N*/ 		pPage->InvalidateFlyLayout();
/*N*/ 		pPage->InvalidateFlyCntnt();
/*N*/ 		pPage->InvalidateFlyInCnt();
/*N*/ 		pPage->InvalidateLayout();
/*N*/ 		pPage->InvalidateCntnt();
/*N*/ 		pPage->InvalidatePage( pPage ); //Damit ggf. auch der Turbo verschwindet
/*N*/ 
/*N*/ 		if ( pPage->GetSortedObjs() )
/*N*/ 		{
/*N*/ 			const SwSortDrawObjs &rObjs = *pPage->GetSortedObjs();
/*N*/ 			for ( USHORT i = 0; i < rObjs.Count(); ++i )
/*N*/ 			{
/*N*/ 				SdrObject *pO = rObjs[i];
/*N*/ 				if ( pO->IsWriterFlyFrame() )
/*N*/                 {
/*N*/ 					::binfilter::lcl_InvalidateCntnt( ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->ContainsCntnt(),
/*N*/ 										 nInv );
/*N*/                     if( nInv & INV_DIRECTION )
/*?*/                         ((SwVirtFlyDrawObj*)pO)->GetFlyFrm()->CheckDirChange();
/*N*/                 }
/*N*/ 			}
/*N*/ 		}
/*N*/         if( nInv & INV_DIRECTION )
/*?*/             pPage->CheckDirChange();
/*N*/ 		pPage = (SwPageFrm*)(pPage->GetNext());
/*N*/ 	}
/*N*/ 
/*N*/ 	//Hier den gesamten Dokumentinhalt und die zeichengebundenen Flys.
/*N*/ 	::binfilter::lcl_InvalidateCntnt( ContainsCntnt(), nInv );
/*N*/ 
/*N*/ 	if( nInv & INV_PRTAREA )
/*N*/ 	{
/*?*/ 		ViewShell *pSh  = GetShell();
/*?*/ 		if( pSh )
/*?*/ 			pSh->InvalidateWindows( Frm() );
/*N*/ 	}
/*N*/ }

}
