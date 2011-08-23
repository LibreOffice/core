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

#ifndef _SFXITEMITER_HXX //autogen
#include <bf_svtools/itemiter.hxx>
#endif

#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#include "sectfrm.hxx"
#include "section.hxx"		// SwSection
#include "frmtool.hxx"		// StackHack

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#include "doc.hxx"			// SwDoc
#include "pagefrm.hxx"		// SwPageFrm
#include "txtfrm.hxx"		// SwTxtFrm
#include "fmtclds.hxx"		// SwFmtCol
#include "colfrm.hxx"		// SwColumnFrm
#include "tabfrm.hxx"		// SwTabFrm
#include "flyfrm.hxx"		// SwFlyFrm
#include "ftnfrm.hxx"		// SwFtnFrm
#include "dbg_lay.hxx"
#include "frmsh.hxx"
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <bf_svx/brshitem.hxx>
#endif
#ifndef _FMTFTNTX_HXX //autogen
#include <fmtftntx.hxx>
#endif
namespace binfilter {

/*N*/ SV_IMPL_PTRARR_SORT( SwDestroyList, SwSectionFrmPtr )

/*************************************************************************
|*
|*	SwSectionFrm::SwSectionFrm(), ~SwSectionFrm()
|*
|*	Ersterstellung		AMA 26. Nov. 97
|*	Letzte Aenderung	AMA 26. Nov. 97
|*
|*************************************************************************/
/*N*/ SwSectionFrm::SwSectionFrm( SwSection &rSect ) :
/*N*/ 	SwLayoutFrm( rSect.GetFmt() ),
/*N*/ 	SwFlowFrm( (SwFrm&)*this ),
/*N*/ 	pSection( &rSect )
/*N*/ {
/*N*/     nType = FRMC_SECTION;
/*N*/ 
/*N*/ 	CalcFtnAtEndFlag();
/*N*/ 	CalcEndAtEndFlag();
/*N*/ }

/*N*/ SwSectionFrm::SwSectionFrm( SwSectionFrm &rSect, BOOL bMaster ) :
/*N*/ 	SwLayoutFrm( rSect.GetFmt() ),
/*N*/ 	SwFlowFrm( (SwFrm&)*this ),
/*N*/ 	pSection( rSect.GetSection() )
/*N*/ {
/*N*/ 	bFtnAtEnd = rSect.IsFtnAtEnd();
/*N*/ 	bEndnAtEnd = rSect.IsEndnAtEnd();
/*N*/ 	bLockJoin = FALSE;
/*N*/     nType = FRMC_SECTION;
/*N*/ 
/*N*/ 	PROTOCOL( this, PROT_SECTION, bMaster ? ACT_CREATE_MASTER : ACT_CREATE_FOLLOW, &rSect )
/*N*/ 
/*N*/ 	if( bMaster )
/*N*/ 	{
/*N*/ 		if( rSect.IsFollow() )
/*N*/ 		{
/*N*/ 			SwSectionFrm* pMaster = rSect.FindSectionMaster();
/*N*/ 			pMaster->SetFollow( this );
/*N*/ 			bIsFollow = TRUE;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			rSect.bIsFollow = TRUE;
/*N*/ 		SetFollow( &rSect );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		bIsFollow = TRUE;
/*N*/ 		SetFollow( rSect.GetFollow() );
/*N*/ 		rSect.SetFollow( this );
/*N*/ 		if( !GetFollow() )
/*N*/ 			rSect.SimpleFormat();
/*N*/ 		if( !rSect.IsColLocked() )
/*N*/ 			rSect.InvalidateSize();
/*N*/ 	}
/*N*/ }

// NOTE: call <SwSectionFrm::Init()> directly after creation of a new section
//       frame and its insert in the layout.
/*N*/ void SwSectionFrm::Init()
/*N*/ {
/*N*/     ASSERT( GetUpper(), "SwSectionFrm::Init before insertion?!" );
/*N*/     SWRECTFN( this )
/*N*/     long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
/*N*/     (Frm().*fnRect->fnSetWidth)( nWidth );
/*N*/     (Frm().*fnRect->fnSetHeight)( 0 );
/*N*/ 
/*N*/     // #109700# LRSpace for sections
/*N*/     const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
/*N*/     (Prt().*fnRect->fnSetLeft)( rLRSpace.GetLeft() );
/*N*/     (Prt().*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
/*N*/                                  rLRSpace.GetRight() );
/*N*/     (Prt().*fnRect->fnSetHeight)( 0 );
/*N*/ 
/*N*/     const SwFmtCol &rCol = GetFmt()->GetCol();
/*N*/     if( ( rCol.GetNumCols() > 1 || IsAnyNoteAtEnd() ) && !IsInFtn() )
/*N*/ 	{
/*N*/         const SwFmtCol *pOld = Lower() ? &rCol : new SwFmtCol;
/*N*/         ChgColumns( *pOld, rCol, IsAnyNoteAtEnd() );
/*N*/         if( pOld != &rCol )
/*N*/             delete pOld;
/*N*/ 	}
/*N*/ }

/*N*/ SwSectionFrm::~SwSectionFrm()
/*N*/ {
/*N*/ 	if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
/*N*/ 	{
/*N*/ 		SwRootFrm *pRootFrm = GetFmt()->GetDoc()->GetRootFrm();
/*N*/ 		if( pRootFrm )
/*N*/ 			pRootFrm->RemoveFromList( this );
/*N*/ 		if( IsFollow() )
/*N*/ 		{
/*?*/ 			SwSectionFrm *pMaster = FindSectionMaster();
/*?*/ 			if( pMaster )
/*?*/ 			{
/*?*/ 				PROTOCOL( this, PROT_SECTION, ACT_DEL_FOLLOW, pMaster )
/*?*/ 				pMaster->SetFollow( GetFollow() );
/*?*/ 				// Ein Master greift sich immer den Platz bis zur Unterkante seines
/*?*/ 				// Uppers. Wenn er keinen Follow mehr hat, kann er diesen ggf. wieder
/*?*/ 				// freigeben, deshalb wird die Size des Masters invalidiert.
/*?*/ 				if( !GetFollow() )
/*?*/ 					pMaster->InvalidateSize();
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if( HasFollow() )
/*N*/ 		{
/*?*/ 			PROTOCOL( this, PROT_SECTION, ACT_DEL_MASTER, GetFollow() )
/*?*/ 			GetFollow()->bIsFollow = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::FindSectionMaster()
|*
|*	Ersterstellung		AMA 17. Dec. 97
|*	Letzte Aenderung	AMA 17. Dec. 97
|*
|*************************************************************************/

/*N*/ SwSectionFrm *SwSectionFrm::FindSectionMaster()
/*N*/ {
/*N*/ 	ASSERT( IsFollow(), "FindSectionMaster: !IsFollow" );
/*N*/ 	SwClientIter aIter( *(pSection->GetFmt()) );
/*N*/ 	SwClient *pLast = aIter.GoStart();
/*N*/ 	while ( pLast )
/*N*/ 	{
/*N*/ 		if ( pLast->ISA( SwFrm ) )
/*N*/ 		{
/*N*/ 			SwSectionFrm* pSect = (SwSectionFrm*)pLast;
/*N*/ 			if( pSect->GetFollow() == this )
/*N*/ 				return pSect;
/*N*/ 		}
/*N*/ 		pLast = aIter++;
/*N*/ 	}
/*?*/ 	return NULL;
/*N*/ }


/*************************************************************************
|*
|*	SwSectionFrm::DelEmpty()
|*
|*	Ersterstellung		AMA 17. Dec. 97
|*	Letzte Aenderung	AMA 17. Dec. 97
|*
|*************************************************************************/
/*N*/ void SwSectionFrm::DelEmpty( BOOL bRemove )
/*N*/ {
/*N*/ 	if( IsColLocked() )
/*N*/ 	{
/*N*/ 		ASSERT( !bRemove, "Don't delete locked SectionFrms" );
/*N*/ 		return;
/*N*/ 	}
/*N*/ 	SwFrm* pUp = GetUpper();
/*N*/ 	if( pUp )
/*N*/ 		_Cut( bRemove );
/*N*/ 	if( IsFollow() )
/*N*/ 	{
/*N*/ 		SwSectionFrm *pMaster = FindSectionMaster();
/*N*/ 		pMaster->SetFollow( GetFollow() );
/*N*/ 		// Ein Master greift sich immer den Platz bis zur Unterkante seines
/*N*/ 		// Uppers. Wenn er keinen Follow mehr hat, kann er diesen ggf. wieder
/*N*/ 		// freigeben, deshalb wird die Size des Masters invalidiert.
/*N*/ 		if( !GetFollow() && !pMaster->IsColLocked() )
/*N*/ 			pMaster->InvalidateSize();
/*N*/ 		bIsFollow = FALSE;
/*N*/ 	}
/*N*/ 	else if( HasFollow() )
/*N*/ 		GetFollow()->bIsFollow = FALSE;
/*N*/ 	pFollow = NULL;
/*N*/ 	if( pUp )
/*N*/ 	{
/*N*/ 		Frm().Height( 0 );
/*N*/ 		// Wenn wir sowieso sofort zerstoert werden, brauchen/duerfen wir
/*N*/ 		// uns gar nicht erst in die Liste eintragen
/*N*/ 		if( bRemove )
/*N*/ 		{   // Wenn wir bereits halbtot waren vor diesem DelEmpty, so
/*N*/ 			// stehen wir vermutlich auch in der Liste und muessen uns
/*N*/ 			// dort austragen
/*N*/ 			if( !pSection )
/*?*/ 				GetFmt()->GetDoc()->GetRootFrm()->RemoveFromList( this );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			GetFmt()->GetDoc()->GetRootFrm()->InsertEmptySct( this );
/*N*/ 		pSection = NULL; // damit ist allerdings eine Reanimierung quasi ausgeschlossen
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::Cut()
|*
|*	Ersterstellung		AMA 02. Dec. 97
|*	Letzte Aenderung	AMA 02. Dec. 97
|*
|*************************************************************************/
/*N*/ void SwSectionFrm::Cut()
/*N*/ {
/*N*/ 	_Cut( TRUE );
/*N*/ }

/*N*/ void SwSectionFrm::_Cut( BOOL bRemove )
/*N*/ {
/*N*/ 	ASSERT( GetUpper(), "Cut ohne Upper()." );
/*N*/ 
/*N*/ 	PROTOCOL( this, PROT_CUT, 0, GetUpper() )
/*N*/ 
/*N*/ 	SwPageFrm *pPage = FindPageFrm();
/*N*/ 	InvalidatePage( pPage );
/*N*/ 	SwFrm *pFrm = GetNext();
/*N*/ 	SwFrm* pPrepFrm = NULL;
/*N*/ 	while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
/*?*/ 		pFrm = pFrm->GetNext();
/*N*/ 	if( pFrm )
/*N*/ 	{	//Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
/*N*/ 		//berechnet der ist jetzt wo er der erste wird obsolete
/*N*/ 		pFrm->_InvalidatePrt();
/*N*/ 		pFrm->_InvalidatePos();
/*N*/ 		if( pFrm->IsSctFrm() )
/*N*/ 			pFrm = ((SwSectionFrm*)pFrm)->ContainsAny();
/*N*/ 		if ( pFrm && pFrm->IsCntntFrm() )
/*N*/ 		{
/*N*/ 			pFrm->InvalidatePage( pPage );
/*N*/ 			if( IsInFtn() && !GetIndPrev() )
/*?*/ 				pPrepFrm = pFrm;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		InvalidateNextPos();
/*N*/ 		//Einer muss die Retusche uebernehmen: Vorgaenger oder Upper
/*N*/ 		if ( 0 != (pFrm = GetPrev()) )
/*N*/ 		{	pFrm->SetRetouche();
/*N*/ 			pFrm->Prepare( PREP_WIDOWS_ORPHANS );
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
/*N*/ 		}
/*N*/ 	}
/*N*/ 	//Erst removen, dann Upper Shrinken.
/*N*/ 	SwLayoutFrm *pUp = GetUpper();
/*N*/ 	if( bRemove )
/*N*/ 	{
/*N*/ 		Remove();
/*N*/ 		if( pUp && !pUp->Lower() && pUp->IsFtnFrm() && !pUp->IsColLocked() &&
/*N*/ 			pUp->GetUpper() )
/*N*/ 		{
/*?*/ 			pUp->Cut();
/*?*/ 			delete pUp;
/*?*/ 			pUp = NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( pPrepFrm )
/*?*/ 		pPrepFrm->Prepare( PREP_FTN );
/*N*/ 	if ( pUp )
/*N*/ 	{
/*N*/         SWRECTFN( this );
/*N*/         SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/         if( nFrmHeight > 0 )
/*N*/ 		{
/*N*/ 			if( !bRemove )
/*N*/ 			{
/*N*/                 (Frm().*fnRect->fnSetHeight)( 0 );
/*N*/                 (Prt().*fnRect->fnSetHeight)( 0 );
/*N*/ 			}
/*N*/             pUp->Shrink( nFrmHeight );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::Paste()
|*
|*	Ersterstellung		AMA 04. Dec. 97
|*	Letzte Aenderung	AMA 04. Dec. 97
|*
|*************************************************************************/



/*************************************************************************
|*
|*	SwSectionFrm::HasToBreak()
|*
|*  Hier wird entschieden, ob der this-SectionFrm den uebergebenen
|*  (Section)Frm aufbrechen soll oder nicht.
|*  Zunaechst werden uebergeordnete Bereiche immer aufgebrochen,
|*  spaeter koennte man es einstellbar machen.
|*
|*	Ersterstellung		AMA 12. Dec. 97
|*	Letzte Aenderung	AMA 12. Dec. 97
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwSectionFrm::MergeNext()
|*
|*	Ersterstellung		AMA 04. Dec. 97
|*	Letzte Aenderung	AMA 04. Dec. 97
|*
|*  Verschmilzt zwei SectionFrms, falls es sich um den
|*  gleichen Bereich handelt.
|*  Notwendig kann dies sein, wenn ein (Unter-)Bereich geloescht wird, der
|*  einen anderen in zwei Teile zerlegt hatte.
|*
|*************************************************************************/

/*N*/ void SwSectionFrm::MergeNext( SwSectionFrm* pNxt )
/*N*/ {
/*N*/ 	if( !pNxt->IsJoinLocked() && GetSection() == pNxt->GetSection() )
/*N*/ 	{
/*N*/ 		PROTOCOL( this, PROT_SECTION, ACT_MERGE, pNxt )
/*N*/ 
/*N*/ 		SwFrm* pTmp = ::binfilter::SaveCntnt( pNxt );
/*N*/ 		if( pTmp )
/*N*/ 		{
/*N*/ 			SwFrm* pLast = Lower();
/*N*/ 			SwLayoutFrm* pLay = this;
/*N*/ 			if( pLast )
/*N*/ 			{
/*N*/ 				while( pLast->GetNext() )
/*N*/ 					pLast = pLast->GetNext();
/*N*/ 				if( pLast->IsColumnFrm() )
/*N*/ 				{   // Spalten jetzt mit BodyFrm
/*?*/ 					pLay = (SwLayoutFrm*)((SwLayoutFrm*)pLast)->Lower();
/*?*/ 					pLast = pLay->Lower();
/*?*/ 					if( pLast )
/*?*/ 						while( pLast->GetNext() )
/*?*/ 							pLast = pLast->GetNext();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			::binfilter::RestoreCntnt( pTmp, pLay, pLast );
/*N*/ 		}
/*N*/ 		SetFollow( pNxt->GetFollow() );
/*N*/ 		pNxt->SetFollow( NULL );
/*N*/ 		pNxt->bIsFollow = FALSE;
/*N*/ 		pNxt->Cut();
/*N*/ 		delete pNxt;
/*N*/ 		InvalidateSize();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::SplitSect()
|*
|*	Ersterstellung		AMA 29. Apr. 99
|*	Letzte Aenderung	AMA 29. Apr. 99
|*
|*  Zerteilt einen SectionFrm in zwei Teile, der zweite Teil beginnt mit dem
|*  uebergebenen Frame.
|*  Benoetigt wird dies beim Einfuegen eines inneren Bereichs, weil innerhalb
|*  von Rahmen oder Tabellenzellen das MoveFwd nicht den erwuenschten Effekt
|*  haben kann.
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwSectionFrm::MoveCntntAndDelete()
|*
|*	Ersterstellung		AMA 29. Jan 99
|*	Letzte Aenderung	AMA 29. Jan 99
|*
|*  MoveCntnt wird zur Zerstoerung eines SectionFrms wg. Aufhebung oder
|*  Verstecken des Bereichs gerufen, um den Inhalt umzuhaengen.
|*  Wenn der SectionFrm keinen anderen aufbrach, so wird der Inhalt in
|*  den Upper bewegt. Anderfalls wird der Inhalt in den anderen SectionFrm
|*  umgehaengt, dieser muss ggf. gemergt werden.
|*
|*************************************************************************/
// Wenn ein mehrspaltiger Bereich aufgehoben wird, muessen die ContentFrms
// invalidiert werden


/*N*/ #define FIRSTLEAF( pLayFrm ) ( ( pLayFrm->Lower() && pLayFrm->Lower()->IsColumnFrm() )\
/*N*/ 					? pLayFrm->GetNextLayoutLeaf() \
/*N*/ 					: pLayFrm )

/*N*/ void SwSectionFrm::MoveCntntAndDelete( SwSectionFrm* pDel, BOOL bSave )
/*N*/ {
/*N*/ 	BOOL bSize = pDel->Lower() && pDel->Lower()->IsColumnFrm();
/*N*/ 	SwFrm* pPrv = pDel->GetPrev();
/*N*/ 	SwLayoutFrm* pUp = pDel->GetUpper();
/*N*/     // OD 27.03.2003 #i12711# - initialize local pointer variables.
/*N*/     SwSectionFrm* pPrvSct = NULL;
/*N*/     SwSectionFrm* pNxtSct = NULL;
/*N*/     SwSectionFmt* pParent = static_cast<SwSectionFmt*>(pDel->GetFmt())->GetParent();
/*N*/ 	if( pDel->IsInTab() && pParent )
/*N*/ 	{
/*?*/ 		SwTabFrm *pTab = pDel->FindTabFrm();
/*?*/ 		// Wenn wir innerhalb einer Tabelle liegen, koennen wir nur Bereiche
/*?*/ 		// aufgebrochen haben, die ebenfalls innerhalb liegen, nicht etwa
/*?*/ 		// einen Bereich, der die gesamte Tabelle umfasst.
/*?*/ 		if( pTab->IsInSct() && pParent == pTab->FindSctFrm()->GetFmt() )
/*?*/ 			pParent = NULL;
/*N*/ 	}
/*N*/ 	// Wenn unser Format einen Parent besitzt, so haben wir vermutlich
/*N*/ 	// einen anderen SectionFrm aufgebrochen, dies muss geprueft werden,
/*N*/ 	// dazu besorgen wir uns zunaechst den vorhergehende und den nach-
/*N*/ 	// folgenden CntntFrm, mal sehen, ob diese in SectionFrms liegen.
/*N*/     // OD 27.03.2003 #i12711# - check, if previous and next section belonging
/*N*/     // together and can be joined, *not* only if deleted section contains content.
/*N*/     if ( pParent )
/*N*/     {
/*N*/         SwFrm* pPrvCntnt = pDel->GetPrevCntntFrm();
/*N*/         pPrvSct = pPrvCntnt ? pPrvCntnt->FindSctFrm() : NULL;
/*N*/         SwFrm* pNxtCntnt = pDel->GetNextCntntFrm();
/*N*/         pNxtSct = pNxtCntnt ? pNxtCntnt->FindSctFrm() : NULL;
/*N*/     }
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pParent = NULL;
/*N*/ 		pPrvSct = pNxtSct = NULL;
/*N*/ 	}
/*N*/ 	// Jetzt wird der Inhalt beseite gestellt und der Frame zerstoert
/*N*/ 	SwFrm *pSave = bSave ? ::binfilter::SaveCntnt( pDel ) : NULL;
/*N*/ 	BOOL bOldFtn = TRUE;
/*N*/ 	if( pSave && pUp->IsFtnFrm() )
/*N*/ 	{
/*N*/ 		bOldFtn = ((SwFtnFrm*)pUp)->IsColLocked();
/*N*/ 		((SwFtnFrm*)pUp)->ColLock();
/*N*/ 	}
/*N*/ 	pDel->DelEmpty( TRUE );
/*N*/ 	delete pDel;
/*N*/ 	if( pParent )
/*N*/ 	{	// Hier wird die geeignete Einfuegeposition gesucht
/*N*/ 		if( pNxtSct && pNxtSct->GetFmt() == pParent )
/*N*/ 		{   // Hier koennen wir uns am Anfang einfuegen
/*N*/ 			pUp = FIRSTLEAF( pNxtSct );
/*N*/ 			pPrv = NULL;
/*N*/ 			if( pPrvSct && !( pPrvSct->GetFmt() == pParent ) )
/*N*/ 				pPrvSct = NULL; // damit nicht gemergt wird
/*N*/ 		}
/*N*/ 		else if( pPrvSct && pPrvSct->GetFmt() == pParent )
/*N*/ 		{   // Wunderbar, hier koennen wir uns am Ende einfuegen
/*N*/ 			pUp = pPrvSct;
/*N*/ 			if( pUp->Lower() && pUp->Lower()->IsColumnFrm() )
/*N*/ 			{
/*N*/ 				pUp = (SwLayoutFrm*)pUp->Lower(); // Die erste Spalte
/*N*/ 				while( pUp->GetNext() )
/*N*/ 					pUp = (SwLayoutFrm*)pUp->GetNext();
/*N*/ 				pUp = (SwLayoutFrm*)pUp->Lower(); // Der Body der letzten Spalte
/*N*/ 			}
/*N*/ 			pPrv = pUp->Lower(); // damit hinter dem letzten eingefuegt wird
/*N*/ 			if( pPrv )
/*N*/ 				while( pPrv->GetNext() )
/*N*/ 					pPrv = pPrv->GetNext();
/*N*/ 			pPrvSct = NULL; // damit nicht gemergt wird
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( pSave )
/*N*/ 			{	// Folgende Situationen: Vor und hinter dem zu loeschenden Bereich
/*N*/ 				// ist entweder die Bereichsgrenze des umfassenden Bereichs oder
/*N*/ 				// es schliesst ein anderer (Geschwister-)Bereich direkt an, der
/*N*/ 				// vom gleichen Parent abgeleitet ist.
/*N*/ 				// Dann gibt es (noch) keinen Teil unseres Parents, der den Inhalt
/*N*/ 				// aufnehmen kann,also bauen wir ihn uns.
/*N*/                 pPrvSct = new SwSectionFrm( *pParent->GetSection() );
/*N*/ 				pPrvSct->InsertBehind( pUp, pPrv );
/*N*/                 pPrvSct->Init();
/*N*/                 SWRECTFN( pUp )
/*N*/                 (pPrvSct->*fnRect->fnMakePos)( pUp, pPrv, TRUE );
/*N*/ 				pUp = FIRSTLEAF( pPrvSct );
/*N*/ 				pPrv = NULL;
/*N*/ 			}
/*N*/ 			pPrvSct = NULL; // damit nicht gemergt wird
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Der Inhalt wird eingefuegt..
/*N*/ 	if( pSave )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	// jetzt koennen eventuell zwei Teile des uebergeordneten Bereich verschmelzen
/*N*/ 	if( pPrvSct && !pPrvSct->IsJoinLocked() )
/*N*/ 	{
/*N*/ 		ASSERT( pNxtSct, "MoveCntnt: No Merge" );
/*N*/ 		pPrvSct->MergeNext( pNxtSct );
/*N*/ 	}
/*N*/ }

/*N*/ void SwSectionFrm::MakeAll()
/*N*/ {
/*N*/ 	if ( IsJoinLocked() || IsColLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
/*N*/ 		return;
/*N*/ 	if( !pSection ) // Durch DelEmpty
/*N*/ 	{
/*?*/ 		ASSERT( GetFmt()->GetDoc()->GetRootFrm()->IsInDelList( this ), "SectionFrm without Section" );
/*?*/ 		if( !bValidPos )
/*?*/ 		{
/*?*/             if( GetUpper() )
/*?*/             {
/*?*/                 SWRECTFN( GetUpper() )
/*?*/                 (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), FALSE );
/*?*/ 			}
/*?*/         }
/*?*/ 		bValidSize = bValidPos = bValidPrtArea = TRUE;
/*?*/ 		return;
/*N*/ 	}
/*N*/ 	LockJoin();	//Ich lass mich nicht unterwegs vernichten.
/*N*/ 
/*N*/ 	while( GetNext() && GetNext() == GetFollow() )
/*N*/ 	{
/*?*/ 		const SwFrm* pFoll = GetFollow();
/*?*/ 		MergeNext( (SwSectionFrm*)GetNext() );
/*?*/ 		if( pFoll == GetFollow() )
/*?*/ 			break;
/*N*/ 	}
/*N*/ 
/*N*/ 	// Ein Bereich mit Follow nimmt allen Platz bis zur Unterkante des Uppers
/*N*/ 	// in Anspruch. Bewegt er sich, so kann seine Groesse zu- oder abnehmen...
/*N*/ 	if( !bValidPos && ToMaximize( FALSE ) )
/*N*/ 		bValidSize = FALSE;
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	const SwFmtCol &rCol = GetFmt()->GetCol();
/*N*/ #endif
/*N*/ 	SwLayoutFrm::MakeAll();
/*N*/ 	UnlockJoin();
/*N*/ 	if( pSection && IsSuperfluous() )
/*?*/ 		DelEmpty( FALSE );
/*N*/ }



/*N*/ void lcl_FindCntntFrm( SwCntntFrm* &rpCntntFrm, SwFtnFrm* &rpFtnFrm,
/*N*/ 	SwFrm* pFrm, BOOL &rbChkFtn )
/*N*/ {
/*N*/ 	if( pFrm )
/*N*/ 	{
/*N*/ 		while( pFrm->GetNext() )
/*N*/ 			pFrm = pFrm->GetNext();
/*N*/ 		while( !rpCntntFrm && pFrm )
/*N*/ 		{
/*N*/ 			if( pFrm->IsCntntFrm() )
/*N*/ 				rpCntntFrm = (SwCntntFrm*)pFrm;
/*N*/ 			else if( pFrm->IsLayoutFrm() )
/*N*/ 			{
/*?*/ 				if( pFrm->IsFtnFrm() )
/*?*/ 				{
/*?*/ 					if( rbChkFtn )
/*?*/ 					{
/*?*/ 						rpFtnFrm = (SwFtnFrm*)pFrm;
/*?*/ 						rbChkFtn = rpFtnFrm->GetAttr()->GetFtn().IsEndNote();
/*?*/ 					}
/*?*/ 				}
/*?*/ 				else
/*?*/ 					lcl_FindCntntFrm( rpCntntFrm, rpFtnFrm,
/*?*/ 						((SwLayoutFrm*)pFrm)->Lower(), rbChkFtn );
/*N*/ 			}
/*N*/ 			pFrm = pFrm->GetPrev();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SwCntntFrm *SwSectionFrm::FindLastCntnt( BYTE nMode )
/*N*/ {
/*N*/ 	SwCntntFrm *pRet = NULL;
/*N*/ 	SwFtnFrm *pFtnFrm = NULL;
/*N*/ 	SwSectionFrm *pSect = this;
/*N*/ 	if( nMode )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const SwSectionFmt *pFmt = IsEndnAtEnd() ? GetEndSectFmt() :
/*N*/ 	}
/*N*/ 	BOOL bFtnFound = nMode == FINDMODE_ENDNOTE;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		lcl_FindCntntFrm( pRet, pFtnFrm, pSect->Lower(), bFtnFound );
/*N*/ 		if( pRet || !pSect->IsFollow() || !nMode ||
/*N*/ 			( FINDMODE_MYLAST == nMode && this == pSect ) )
/*N*/ 			break;
/*?*/ 		pSect = pSect->FindSectionMaster();
/*N*/ 	} while( pSect );
/*N*/ 	if( ( nMode == FINDMODE_ENDNOTE ) && pFtnFrm )
/*?*/ 		pRet = pFtnFrm->ContainsCntnt();
/*N*/ 	return pRet;
/*N*/ }

/*N*/ BOOL SwSectionFrm::CalcMinDiff( SwTwips& rMinDiff ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

/*************************************************************************
 *
 * 	SwSectionFrm::CollectEndnotes(  )
 *
 *	Ersterstellung		AMA 03. Nov 99
 *	Letzte Aenderung	AMA 03. Nov 99
 *
 *  CollectEndnotes looks for endnotes in the sectionfrm and his follows,
 *  the endnotes will cut off the layout and put into the array.
 *  If the first endnote is not a master-SwFtnFrm, the whole sectionfrm
 *  contains only endnotes and it is not necessary to collect them.
 *
 *************************************************************************/


/*N*/ void lcl_ColumnRefresh( SwSectionFrm* pSect, BOOL bFollow )
/*N*/ {
/*N*/ 	while( pSect )
/*N*/ 	{
/*N*/ 		BOOL bOldLock = pSect->IsColLocked();
/*N*/ 		pSect->ColLock();
/*N*/ 		if( pSect->Lower() && pSect->Lower()->IsColumnFrm() )
/*N*/ 		{
/*?*/ 			SwColumnFrm *pCol = (SwColumnFrm*)pSect->Lower();
/*?*/ 			do
/*?*/ 			{	pCol->_InvalidateSize();
/*?*/ 				pCol->_InvalidatePos();
/*?*/ 				((SwLayoutFrm*)pCol)->Lower()->_InvalidateSize();
/*?*/ 				pCol->Calc();   // calculation of column and
/*?*/ 				((SwLayoutFrm*)pCol)->Lower()->Calc();  // body
/*?*/ 				pCol = (SwColumnFrm*)pCol->GetNext();
/*?*/ 			} while ( pCol );
/*N*/ 		}
/*N*/ 		if( !bOldLock )
/*N*/ 			pSect->ColUnlock();
/*N*/ 		if( bFollow )
/*?*/ 			pSect = pSect->GetFollow();
/*N*/ 		else
/*N*/ 			pSect = NULL;
/*N*/ 	}
/*N*/ }


/*************************************************************************
|*
|*	SwSectionFrm::_CheckClipping( BOOL bGrow, BOOL bMaximize )
|*
|*	Beschreibung:		Passt die Groesse an die Umgebung an.
|*  	Wer einen Follow oder Fussnoten besitzt, soll bis zur Unterkante
|*      des Uppers gehen (bMaximize).
|*		Niemand darf ueber den Upper hinausgehen, ggf. darf man versuchen (bGrow)
|*		seinen Upper zu growen.
|*      Wenn die Groesse veraendert werden musste, wird der Inhalt kalkuliert.
|*
|*************************************************************************/

/// OD 18.09.2002 #100522#
/// perform calculation of content, only if height has changed.
/*N*/ void SwSectionFrm::_CheckClipping( BOOL bGrow, BOOL bMaximize )
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/     long nDiff;
/*N*/     SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/ 	if( bGrow && ( !IsInFly() || !GetUpper()->IsColBodyFrm() ||
/*N*/ 				   !FindFlyFrm()->IsLocked() ) )
/*N*/ 	{
/*N*/         nDiff = -(Frm().*fnRect->fnBottomDist)( nDeadLine );
/*N*/         if( !bMaximize )
/*N*/             nDiff += Undersize();
/*N*/         if( nDiff > 0 )
/*N*/         {
/*N*/             long nAdd = GetUpper()->Grow( nDiff );
/*N*/             if( bVert && !bRev )
/*?*/                 nDeadLine -= nAdd;
/*N*/             else
/*N*/                 nDeadLine += nAdd;
/*N*/         }
/*N*/ 	}
/*N*/     nDiff = -(Frm().*fnRect->fnBottomDist)( nDeadLine );
/*N*/     SetUndersized( !bMaximize && nDiff >= 0 );
/*N*/     BOOL bCalc = ( IsUndersized() || bMaximize ) && ( nDiff ||
/*N*/                  (Prt().*fnRect->fnGetTop)() > (Frm().*fnRect->fnGetHeight)() );
/*N*/     if( !bCalc && !bGrow && IsAnyNoteAtEnd() && !IsInFtn() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwSectionFrm *pSect = this;
/*N*/ 	}
/*N*/ 	if( bCalc )
/*N*/ 	{
/*N*/         nDiff = (*fnRect->fnYDiff)( nDeadLine, (Frm().*fnRect->fnGetTop)() );
/*N*/         if( nDiff < 0 )
/*N*/         {
/*?*/             nDiff = 0;
/*?*/             nDeadLine = (Frm().*fnRect->fnGetTop)();
/*N*/         }
/*N*/         const Size aOldSz( Prt().SSize() );
/*N*/         long nTop = (this->*fnRect->fnGetTopMargin)();
/*N*/         (Frm().*fnRect->fnSetBottom)( nDeadLine );
/*N*/         nDiff = (Frm().*fnRect->fnGetHeight)();
/*N*/         if( nTop > nDiff )
/*N*/             nTop = nDiff;
/*N*/         (this->*fnRect->fnSetYMargins)( nTop, 0 );

        /// OD 18.09.2002 #100522#
        /// Determine, if height has changed.
        /// Note: In vertical layout the height equals the width value.
/*N*/         bool bHeightChanged = bVert ?
/*N*/                             (aOldSz.Width() != Prt().Width()) :
/*N*/                             (aOldSz.Height() != Prt().Height());
/*N*/ 		// Wir haben zu guter Letzt noch einmal die Hoehe geaendert,
/*N*/ 		// dann wird das innere Layout (Columns) kalkuliert und
/*N*/ 		// der Inhalt ebenfalls.
        /// OD 18.09.2002 #100522#
        /// calculate content, only if height has changed.
/*N*/         if( bHeightChanged && Lower() )
/*N*/ 		{
/*N*/ 			if( Lower()->IsColumnFrm() )
/*N*/ 			{
/*?*/ 				lcl_ColumnRefresh( this, FALSE );
/*?*/ 				::binfilter::CalcCntnt( this );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				ChgLowersProp( aOldSz );
/*N*/ 				if( !bMaximize && !IsCntntLocked() )
/*N*/ 					::binfilter::CalcCntnt( this );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwSectionFrm::SimpleFormat()
/*N*/ {
/*N*/ 	if ( IsJoinLocked() || IsColLocked() )
/*N*/ 		return;
/*N*/ 	// ASSERT( pFollow, "SimpleFormat: Follow required" );
/*N*/ 	LockJoin();
/*N*/     SWRECTFN( this )
/*N*/     if( GetPrev() || GetUpper() )
/*N*/ 	{
/*N*/         (this->*fnRect->fnMakePos)( GetUpper(), GetPrev(), FALSE );
/*N*/ 		bValidPos = TRUE;
/*N*/ 	}
/*N*/     SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
    // OD 22.10.2002 #97265# - call always method <lcl_ColumnRefresh(..)>, in
    // order to get calculated lowers, not only if there space left in its upper.
/*N*/     if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
/*N*/ 	{
/*N*/ 		const Size aOldSz( Prt().SSize() );
/*N*/         (Frm().*fnRect->fnSetBottom)( nDeadLine );
/*N*/         long nHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/         long nTop = CalcUpperSpace();
/*N*/         if( nTop > nHeight )
/*?*/             nTop = nHeight;
/*N*/         (this->*fnRect->fnSetYMargins)( nTop, 0 );
/*N*/ 	}
/*N*/ 		lcl_ColumnRefresh( this, FALSE );
/*N*/ 	UnlockJoin();
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::Format()
|*
|*	Beschreibung:		"Formatiert" den Frame; Frm und PrtArea.
|*	Ersterstellung		AMA 03. Dec. 97
|* 	Letzte Aenderung	MA 09. Oct. 98
|*
|*************************************************************************/

/*N*/ void SwSectionFrm::Format( const SwBorderAttrs *pAttr )
/*N*/ {
/*N*/ 	if( !pSection ) // Durch DelEmpty
/*N*/ 	{
/*N*/ 		ASSERT( GetFmt()->GetDoc()->GetRootFrm()->IsInDelList( this ),
/*N*/ 				 "SectionFrm without Section" );
/*N*/ 		bValidSize = bValidPos = bValidPrtArea = TRUE;
/*N*/ 		return;
/*N*/ 	}
/*N*/     SWRECTFN( this )
/*N*/ 	if ( !bValidPrtArea )
/*N*/ 	{
/*N*/ 		PROTOCOL( this, PROT_PRTAREA, 0, 0 )
/*N*/ 		bValidPrtArea = TRUE;
/*N*/ 		SwTwips nUpper = CalcUpperSpace();
/*N*/ 
/*N*/         // #109700# LRSpace for sections
/*N*/         const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
/*N*/         (this->*fnRect->fnSetXMargins)( rLRSpace.GetLeft(), rLRSpace.GetRight() );
/*N*/ 
/*N*/         if( nUpper != (this->*fnRect->fnGetTopMargin)() )
/*N*/ 		{
/*N*/ 			bValidSize = FALSE;
/*N*/ 			SwFrm* pOwn = ContainsAny();
/*N*/ 			if( pOwn )
/*N*/ 				pOwn->_InvalidatePos();
/*N*/ 		}
/*N*/         (this->*fnRect->fnSetYMargins)( nUpper, 0 );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( !bValidSize )
/*N*/ 	{
/*N*/ 		PROTOCOL_ENTER( this, PROT_SIZE, 0, 0 )
/*N*/         const long nOldHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/ 		BOOL bOldLock = IsColLocked();
/*N*/ 		ColLock();
/*N*/ 
/*N*/ 		bValidSize = TRUE;
/*N*/ 
/*N*/ 		//die Groesse wird nur dann vom Inhalt bestimmt, wenn der SectFrm
/*N*/ 		//keinen Follow hat. Anderfalls fuellt er immer den Upper bis
/*N*/ 		//zur Unterkante aus. Fuer den Textfluss ist nicht er, sondern sein
/*N*/ 		//Inhalt selbst verantwortlich.
/*N*/ 		BOOL bMaximize = ToMaximize( FALSE );
/*N*/ 
/*N*/         // Column widths have to be adjusted before calling _CheckClipping.
/*N*/         // _CheckClipping can cause the formatting of the lower frames
/*N*/         // which still have a width of 0.
/*N*/         const sal_Bool bHasColumns = Lower() && Lower()->IsColumnFrm();
/*N*/         if ( bHasColumns && Lower()->GetNext() )
/*N*/             AdjustColumns( 0, FALSE );
/*N*/ 
/*N*/ 		if( GetUpper() )
/*N*/ 		{
/*N*/             long nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
/*N*/             (aFrm.*fnRect->fnSetWidth)( nWidth );
/*N*/ 
/*N*/             // #109700# LRSpace for sections
/*N*/             const SvxLRSpaceItem& rLRSpace = GetFmt()->GetLRSpace();
/*N*/             (aPrt.*fnRect->fnSetWidth)( nWidth - rLRSpace.GetLeft() -
/*N*/                                         rLRSpace.GetRight() );
/*N*/ 
/*N*/             /// OD 15.10.2002 #103517# - allow grow in online layout
/*N*/             /// Thus, set <..IsBrowseMode()> as parameter <bGrow> on calling
/*N*/             /// method <_CheckClipping(..)>.
/*N*/             _CheckClipping( GetFmt()->GetDoc()->IsBrowseMode(), bMaximize );
/*N*/ 			bMaximize = ToMaximize( FALSE );
/*N*/ 			bValidSize = TRUE;
/*N*/ 		}
/*N*/ 
/*N*/         //Breite der Spalten pruefen und ggf. einstellen.
/*N*/         if ( bHasColumns && ! Lower()->GetNext() && bMaximize )
/*N*/             ((SwColumnFrm*)Lower())->Lower()->Calc();
/*N*/ 
/*N*/ 		if ( !bMaximize )
/*N*/ 		{
/*N*/             SwTwips nRemaining = (this->*fnRect->fnGetTopMargin)(), nDiff;
/*N*/ 			SwFrm *pFrm = pLower;
/*N*/ 			if( pFrm )
/*N*/ 			{
/*N*/ 				if( pFrm->IsColumnFrm() && pFrm->GetNext() )
/*N*/ 				{
/*N*/                     FormatWidthCols( *pAttr, nRemaining, MINLAY );
/*N*/ 					while( HasFollow() && !GetFollow()->ContainsCntnt() )
/*N*/ 					{
/*N*/ 						SwFrm* pOld = GetFollow();
/*N*/ 						GetFollow()->DelEmpty( FALSE );
/*N*/ 						if( pOld == GetFollow() )
/*N*/ 							break;
/*N*/ 					}
/*N*/ 					bMaximize = ToMaximize( FALSE );
/*N*/                     nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( pFrm->IsColumnFrm() )
/*N*/ 					{
/*N*/ 						pFrm->Calc();
/*N*/ 						pFrm = ((SwColumnFrm*)pFrm)->Lower();
/*N*/ 						pFrm->Calc();
/*N*/ 						pFrm = ((SwLayoutFrm*)pFrm)->Lower();
/*N*/ 						CalcFtnCntnt();
/*N*/ 					}
/*N*/ 					// Wenn wir in einem spaltigen Rahmen stehen und dieser
/*N*/ 					// gerade im FormatWidthCols ein CalcCntnt ruft, muss
/*N*/ 					// unser Inhalt ggf. kalkuliert werden.
/*N*/ 					if( pFrm && !pFrm->IsValid() && IsInFly() &&
/*N*/ 						FindFlyFrm()->IsColLocked() )
/*N*/ 						::binfilter::CalcCntnt( this );
/*N*/ 					nRemaining += InnerHeight();
/*N*/ 					bMaximize = HasFollow();
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/             nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
/*N*/ 			if( nDiff < 0)
/*N*/ 			{
/*N*/                 SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
/*N*/ 				{
/*N*/                     long nBottom = (Frm().*fnRect->fnGetBottom)();
/*N*/                     nBottom = (*fnRect->fnYInc)( nBottom, -nDiff );
/*N*/                     long nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
/*N*/                     if( nTmpDiff > 0 )
/*N*/ 					{
/*N*/                         nTmpDiff = GetUpper()->Grow( nTmpDiff, TRUE );
/*N*/                         nDeadLine = (*fnRect->fnYInc)( nDeadLine, nTmpDiff );
/*N*/                         nTmpDiff = (*fnRect->fnYDiff)( nBottom, nDeadLine );
/*N*/                         if( nTmpDiff > 0 )
/*N*/                             nDiff += nTmpDiff;
/*N*/ 						if( nDiff > 0 )
/*N*/ 							nDiff = 0;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( nDiff )
/*N*/ 			{
/*N*/                 long nTmp = nRemaining - (Frm().*fnRect->fnGetHeight)();
/*N*/                 long nTop = (this->*fnRect->fnGetTopMargin)();
/*N*/                 (Frm().*fnRect->fnAddBottom)( nTmp );
/*N*/                 (this->*fnRect->fnSetYMargins)( nTop, 0 );
/*N*/ 				InvalidateNextPos();
/*N*/ 				if( pLower && ( !pLower->IsColumnFrm() || !pLower->GetNext() ) )
/*N*/ 				{
/*N*/ 					// Wenn ein einspaltiger Bereich gerade den Platz geschaffen
/*N*/ 					// hat, den sich die "undersized" Absaetze gewuenscht haben,
/*N*/ 					// muessen diese invalidiert und kalkuliert werden, damit
/*N*/ 					// sie diesen ausfuellen.
/*N*/ 					pFrm = pLower;
/*N*/ 					if( pFrm->IsColumnFrm() )
/*N*/ 					{
/*N*/ 						pFrm->_InvalidateSize();
/*N*/ 						pFrm->_InvalidatePos();
/*N*/ 						pFrm->Calc();
/*N*/ 						pFrm = ((SwColumnFrm*)pFrm)->Lower();
/*N*/ 						pFrm->Calc();
/*N*/ 						pFrm = ((SwLayoutFrm*)pFrm)->Lower();
/*N*/ 						CalcFtnCntnt();
/*N*/ 					}
/*N*/ 					BOOL bUnderSz = FALSE;
/*N*/ 					while( pFrm )
/*N*/ 					{
/*N*/ 						if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
/*N*/ 						{
/*N*/ 							pFrm->Prepare( PREP_ADJUST_FRM );
/*N*/ 							bUnderSz = TRUE;
/*N*/ 						}
/*N*/ 						pFrm = pFrm->GetNext();
/*N*/ 					}
/*N*/ 					if( bUnderSz && !IsCntntLocked() )
/*N*/ 						::binfilter::CalcCntnt( this );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		//Unterkante des Uppers nicht ueberschreiten. Fuer Sections mit
/*N*/ 		//Follows die Unterkante auch nicht unterschreiten.
/*N*/ 		if ( GetUpper() )
/*N*/ 			_CheckClipping( TRUE, bMaximize );
/*N*/ 		if( !bOldLock )
/*N*/ 			ColUnlock();
/*N*/         long nDiff = nOldHeight - (Frm().*fnRect->fnGetHeight)();
/*N*/         if( nDiff > 0 )
/*N*/ 		{
/*N*/ 			if( !GetNext() )
/*N*/ 				SetRetouche(); // Dann muessen wir die Retusche selbst uebernehmen
/*N*/             if( GetUpper() && !GetUpper()->IsFooterFrm() )
/*N*/                 GetUpper()->Shrink( nDiff PHEIGHT );
/*N*/ 		}
/*N*/ 		if( IsUndersized() )
/*N*/ 			bValidPrtArea = TRUE;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::GetNextSctLeaf()
|*
|*	Beschreibung		Liefert das naechste Layoutblatt in das der	Frame
|* 		gemoved werden kann.
|* 		Neue Seiten werden nur dann erzeugt, wenn der Parameter TRUE ist.
|*	Ersterstellung		AMA 07. Jan. 98
|*	Letzte Aenderung	AMA 07. Jan. 98
|*
|*************************************************************************/


/*N*/ SwLayoutFrm *SwFrm::GetNextSctLeaf( MakePageType eMakePage )
/*N*/ {
/*N*/ 	//Achtung: Geschachtelte Bereiche werden zur Zeit nicht unterstuetzt.
/*N*/ 
/*N*/ 	PROTOCOL_ENTER( this, PROT_LEAF, ACT_NEXT_SECT, GetUpper()->FindSctFrm() )
/*N*/ 
/*N*/ 	// Abkuerzungen fuer spaltige Bereiche, wenn wir noch nicht in der letzten Spalte sind.
/*N*/ 	// Koennen wir in die naechste Spalte des Bereichs rutschen?
/*N*/ 	if( IsColBodyFrm() && GetUpper()->GetNext() )
/*?*/ 		return (SwLayoutFrm*)((SwLayoutFrm*)GetUpper()->GetNext())->Lower();
/*N*/ 	if( GetUpper()->IsColBodyFrm() && GetUpper()->GetUpper()->GetNext() )
/*?*/ 		return (SwLayoutFrm*)((SwLayoutFrm*)GetUpper()->GetUpper()->GetNext())->Lower();
/*N*/ 	// Innerhalb von Bereichen in Tabellen oder Bereichen in Kopf/Fusszeilen kann
/*N*/ 	// nur ein Spaltenwechsel erfolgen, eine der oberen Abkuerzungen haette zuschlagen muessen
/*N*/ 	if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
/*?*/ 		return 0;
/*N*/ 
/*N*/ //MA 03. Feb. 99: Warum GetUpper()? Das knallt mit Buch.sgl weil im
/*N*/ //FlyAtCnt::MakeFlyPos ein Orient der SectionFrm ist und auf diesen ein
/*N*/ //GetLeaf gerufen wird.
/*N*/ //	SwSectionFrm *pSect = GetUpper()->FindSctFrm();
/*N*/ 	SwSectionFrm *pSect = FindSctFrm();
/*N*/ 	BOOL bWrongPage = FALSE;
/*N*/ 	ASSERT( pSect, "GetNextSctLeaf: Missing SectionFrm" );
/*N*/ 
/*N*/ 	// Hier eine Abkuerzung fuer Bereiche mit Follows,
/*N*/ 	// dieser kann akzeptiert werden, wenn keine Spalten oder Seiten (ausser Dummyseiten)
/*N*/ 	// dazwischen liegen.
/*N*/ 	// Bei verketteten Rahmen und ind Fussnoten wuerde die Abkuerzung noch aufwendiger
/*N*/ 	if( pSect->HasFollow() && pSect->IsInDocBody() )
/*N*/ 	{
/*N*/ 		if( pSect->GetFollow() == pSect->GetNext() )
/*N*/ 		{
/*?*/ 			SwPageFrm *pPg = pSect->GetFollow()->FindPageFrm();
/*?*/ 			if( WrongPageDesc( pPg ) )
/*?*/ 				bWrongPage = TRUE;
/*?*/ 			else
/*?*/ 				return FIRSTLEAF( pSect->GetFollow() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwFrm* pTmp;
/*N*/ 			if( !pSect->GetUpper()->IsColBodyFrm() ||
/*N*/ 				0 == ( pTmp = pSect->GetUpper()->GetUpper()->GetNext() ) )
/*N*/ 				pTmp = pSect->FindPageFrm()->GetNext();
/*N*/ 			if( pTmp ) // ist jetzt die naechste Spalte oder Seite
/*N*/ 			{
/*N*/ 				SwFrm* pTmpX = pTmp;
/*N*/ 				if( pTmp->IsPageFrm() && ((SwPageFrm*)pTmp)->IsEmptyPage() )
/*?*/ 					pTmp = pTmp->GetNext(); // Dummyseiten ueberspringen
/*N*/ 				SwFrm *pUp = pSect->GetFollow()->GetUpper();
/*N*/ 				// pUp wird die Spalte, wenn der Follow in einer "nicht ersten" Spalte
/*N*/ 				// liegt, ansonsten die Seite:
/*N*/ 				if( !pUp->IsColBodyFrm() ||
/*N*/ 					!( pUp = pUp->GetUpper() )->GetPrev() )
/*N*/ 					pUp = pUp->FindPageFrm();
/*N*/ 				// Jetzt muessen pUp und pTmp die gleiche Seite/Spalte sein,
/*N*/ 				// sonst liegen Seiten oder Spalten zwischen Master und Follow.
/*N*/ 				if( pUp == pTmp || pUp->GetNext() == pTmpX )
/*N*/ 				{
/*N*/ 					SwPageFrm* pNxtPg = pUp->IsPageFrm() ?
/*N*/ 										(SwPageFrm*)pUp : pUp->FindPageFrm();
/*N*/ 					if( WrongPageDesc( pNxtPg ) )
/*?*/ 						bWrongPage = TRUE;
/*N*/ 					else
/*N*/ 						return FIRSTLEAF( pSect->GetFollow() );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Immer im gleichen Bereich landen: Body wieder in Body etc.
/*N*/ 	const BOOL bBody = IsInDocBody();
/*N*/ 	const BOOL bFtnPage = FindPageFrm()->IsFtnPage();
/*N*/ 
/*N*/ 	SwLayoutFrm *pLayLeaf;
/*N*/ 	// Eine Abkuerzung fuer TabFrms, damit nicht alle Zellen abgehuehnert werden
/*N*/ 	if( bWrongPage )
/*?*/ 		pLayLeaf = 0;
/*N*/ 	else if( IsTabFrm() )
/*?*/ 		pLayLeaf = ((SwTabFrm*)this)->FindLastCntnt()->GetUpper();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pLayLeaf = GetNextLayoutLeaf();
/*N*/ 		if( IsColumnFrm() )
/*N*/ 		{
/*?*/ 			while( pLayLeaf && ((SwColumnFrm*)this)->IsAnLower( pLayLeaf ) )
/*?*/ 				pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	SwLayoutFrm *pOldLayLeaf = 0;			//Damit bei neu erzeugten Seiten
/*N*/ 											//nicht wieder vom Anfang gesucht
/*N*/ 											//wird.
/*N*/ 
/*N*/ 	while( TRUE )
/*N*/ 	{
/*N*/ 		if( pLayLeaf )
/*N*/ 		{
/*N*/ 			// Ein Layoutblatt wurde gefunden, mal sehen, ob er mich aufnehmen kann,
/*N*/ 			// ob hier ein weiterer SectionFrm eingefuegt werden kann
/*N*/ 			// oder ob wir weitersuchen muessen.
/*N*/ 			SwPageFrm* pNxtPg = pLayLeaf->FindPageFrm();
/*N*/ 			if ( !bFtnPage && pNxtPg->IsFtnPage() )
/*N*/ 			{	//Wenn ich bei den Endnotenseiten angelangt bin hat sichs.
/*?*/ 				pLayLeaf = 0;
/*?*/ 				continue;
/*N*/ 			}
/*N*/ 			// Einmal InBody, immer InBody, nicht in Tabellen hinein
/*N*/ 			// und nicht in fremde Bereiche hinein
/*N*/ 			if ( (bBody && !pLayLeaf->IsInDocBody()) ||
/*N*/ 				 (IsInFtn() != pLayLeaf->IsInFtn() ) ||
/*N*/ 				 pLayLeaf->IsInTab() ||
/*N*/ 				 ( pLayLeaf->IsInSct() && ( !pSect->HasFollow()
/*N*/ 				   || pSect->GetFollow() != pLayLeaf->FindSctFrm() ) ) )
/*N*/ 			{
/*N*/ 				//Er will mich nicht; neuer Versuch, neues Glueck
/*N*/ 				pOldLayLeaf = pLayLeaf;
/*N*/ 				pLayLeaf = pLayLeaf->GetNextLayoutLeaf();
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 			if( WrongPageDesc( pNxtPg ) )
/*N*/ 			{
/*N*/ 				if( bWrongPage )
/*N*/ 					break; // there's a column between me and my right page
/*N*/ 				pLayLeaf = 0;
/*N*/ 				bWrongPage = TRUE;
/*N*/ 				pOldLayLeaf = 0;
/*N*/ 				continue;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		//Es gibt keinen passenden weiteren LayoutFrm, also muss eine
/*N*/ 		//neue Seite her, allerdings nuetzen uns innerhalb eines Rahmens
/*N*/ 		//neue Seiten nichts.
/*N*/ 		else if( !pSect->IsInFly() &&
/*N*/ 			( eMakePage == MAKEPAGE_APPEND || eMakePage == MAKEPAGE_INSERT ) )
/*N*/ 		{
/*N*/ 			InsertPage(pOldLayLeaf ? pOldLayLeaf->FindPageFrm() : FindPageFrm(),
/*N*/ 					   FALSE );
/*N*/ 			//und nochmal das ganze
/*N*/ 			pLayLeaf = pOldLayLeaf ? pOldLayLeaf : GetNextLayoutLeaf();
/*N*/ 			continue;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pLayLeaf )
/*N*/ 	{
/*N*/ 		// Das passende Layoutblatt haben wir gefunden, wenn es dort bereits einen
/*N*/ 		// Follow unseres Bereichs gibt, nehmen wir dessen erstes Layoutblatt,
/*N*/ 		// andernfalls wird es Zeit, einen Bereichsfollow zu erzeugen
/*N*/ 		SwSectionFrm* pNew;
/*N*/ 
/*N*/ 		//Dies kann entfallen, wenn bei existierenden Follows bereits abgekuerzt wurde
/*N*/ 		SwFrm* pFirst = pLayLeaf->Lower();
/*N*/ 		// Auch hier muessen zum Loeschen angemeldete SectionFrms ignoriert werden
/*N*/ 		while( pFirst && pFirst->IsSctFrm() && !((SwSectionFrm*)pFirst)->GetSection() )
/*?*/ 			pFirst = pFirst->GetNext();
/*N*/ 		if( pFirst && pFirst->IsSctFrm() &&	pSect->GetFollow() == pFirst )
/*?*/ 			pNew = pSect->GetFollow();
/*N*/ 		else if( MAKEPAGE_NOSECTION == eMakePage )
/*?*/ 			return pLayLeaf;
/*N*/ 		else
/*N*/ 		{
/*N*/             pNew = new SwSectionFrm( *pSect, FALSE );
/*N*/ 			pNew->InsertBefore( pLayLeaf, pLayLeaf->Lower() );
/*N*/             pNew->Init();
/*N*/             SWRECTFN( pNew )
/*N*/             (pNew->*fnRect->fnMakePos)( pLayLeaf, NULL, TRUE );
/*N*/ 
/*N*/ 			// Wenn unser Bereichsframe einen Nachfolger hat, so muss dieser
/*N*/ 			// umgehaengt werden hinter den neuen Follow der Bereichsframes.
/*N*/ 			SwFrm* pTmp = pSect->GetNext();
/*N*/ 			if( pTmp && pTmp != pSect->GetFollow() )
/*N*/ 			{
/*N*/ 				SwFlowFrm* pNxt;
/*N*/ 				SwCntntFrm* pNxtCntnt = NULL;
/*N*/ 				if( pTmp->IsCntntFrm() )
/*N*/ 				{
/*N*/ 					pNxt = (SwCntntFrm*)pTmp;
/*N*/ 					pNxtCntnt = (SwCntntFrm*)pTmp;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pNxtCntnt = ((SwLayoutFrm*)pTmp)->ContainsCntnt();
/*N*/ 					if( pTmp->IsSctFrm() )
/*N*/ 						pNxt = (SwSectionFrm*)pTmp;
/*N*/ 					else
/*N*/ 					{
/*?*/ 						ASSERT( pTmp->IsTabFrm(), "GetNextSctLeaf: Wrong Type" );
/*?*/ 						pNxt = (SwTabFrm*)pTmp;
/*N*/ 					}
/*N*/ 					while( !pNxtCntnt && 0 != ( pTmp = pTmp->GetNext() ) )
/*N*/ 					{
/*?*/ 						if( pTmp->IsCntntFrm() )
/*?*/ 							pNxtCntnt = (SwCntntFrm*)pTmp;
/*?*/ 						else
/*?*/ 							pNxtCntnt = ((SwLayoutFrm*)pTmp)->ContainsCntnt();
/*N*/ 					}
/*N*/ 				}
/*N*/ 				if( pNxtCntnt )
/*N*/ 				{
/*N*/ 					SwFtnBossFrm* pOldBoss = pSect->FindFtnBossFrm( TRUE );
/*N*/ 					if( pOldBoss == pNxtCntnt->FindFtnBossFrm( TRUE ) )
/*N*/ 					{
/*N*/ 						SwSaveFtnHeight aHeight( pOldBoss,
/*N*/ 							pOldBoss->Frm().Top() + pOldBoss->Frm().Height() );
/*N*/ 						pSect->GetUpper()->MoveLowerFtns( pNxtCntnt, pOldBoss,
/*N*/ 									pLayLeaf->FindFtnBossFrm( TRUE ), FALSE );
/*N*/ 					}
/*N*/ 				}
/*N*/ 				((SwFlowFrm*)pNxt)->MoveSubTree( pLayLeaf, pNew->GetNext() );
/*N*/ 			}
/*N*/ 			if( pNew->GetFollow() )
/*N*/ 				pNew->SimpleFormat();
/*N*/ 		}
/*N*/ 		// Das gesuchte Layoutblatt ist jetzt das erste des ermittelten SctFrms:
/*N*/ 		pLayLeaf = FIRSTLEAF( pNew );
/*N*/ 	}
/*N*/ 	return pLayLeaf;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::GetPrevSctLeaf()
|*
|*	Beschreibung		Liefert das vorhergehende LayoutBlatt in das der
|* 		Frame gemoved werden kann.
|*	Ersterstellung		AMA 07. Jan. 98
|*	Letzte Aenderung	AMA 07. Jan. 98
|*
|*************************************************************************/


/*N*/ SwLayoutFrm *SwFrm::GetPrevSctLeaf( MakePageType eMakeFtn )
/*N*/ {
/*N*/ 	PROTOCOL_ENTER( this, PROT_LEAF, ACT_PREV_SECT, GetUpper()->FindSctFrm() )
/*N*/ 
/*N*/ 	SwLayoutFrm* pCol;
/*N*/ 	// ColumnFrm beinhalten jetzt stets einen BodyFrm
/*N*/ 	if( IsColBodyFrm() )
/*?*/ 		pCol = GetUpper();
/*N*/ 	else if( GetUpper()->IsColBodyFrm() )
/*?*/ 		pCol = GetUpper()->GetUpper();
/*N*/ 	else
/*N*/ 		pCol = NULL;
/*N*/ 	BOOL bJump = FALSE;
/*N*/ 	if( pCol )
/*N*/ 	{
/*?*/ 		if( pCol->GetPrev() )
/*?*/ 		{
/*?*/ 			do
/*?*/ 			{
/*?*/ 				pCol = (SwLayoutFrm*)pCol->GetPrev();
/*?*/ 				// Gibt es dort Inhalt?
/*?*/ 				if( ((SwLayoutFrm*)pCol->Lower())->Lower() )
/*?*/ 				{
/*?*/ 					if( bJump )		// Haben wir eine leere Spalte uebersprungen?
/*?*/ 						SwFlowFrm::SetMoveBwdJump( TRUE );
/*?*/ 					return (SwLayoutFrm*)pCol->Lower();  // Der Spaltenbody
/*?*/ 				}
/*?*/ 				bJump = TRUE;
/*?*/ 			} while( pCol->GetPrev() );
/*?*/ 
/*?*/ 			// Hier landen wir, wenn alle Spalten leer sind,
/*?*/ 			// pCol ist jetzt die erste Spalte, wir brauchen aber den Body:
/*?*/ 			pCol = (SwLayoutFrm*)pCol->Lower();
/*?*/ 		}
/*?*/ 		else
/*?*/ 			pCol = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bJump )		// Haben wir eine leere Spalte uebersprungen?
/*?*/ 		SwFlowFrm::SetMoveBwdJump( TRUE );
/*N*/ 
/*N*/ 	// Innerhalb von Bereichen in Tabellen oder Bereichen in Kopf/Fusszeilen kann
/*N*/ 	// nur ein Spaltenwechsel erfolgen, eine der oberen Abkuerzungen haette
/*N*/ 	// zuschlagen muessen, ebenso wenn der Bereich einen pPrev hat.
/*N*/ 	// Jetzt ziehen wir sogar eine leere Spalte in Betracht...
/*N*/ 	ASSERT( FindSctFrm(), "GetNextSctLeaf: Missing SectionFrm" );
/*N*/ 	if( ( IsInTab() && !IsTabFrm() ) || FindFooterOrHeader() )
/*?*/ 		return pCol;
/*N*/ 
/*N*/ 	SwSectionFrm *pSect = FindSctFrm();
/*N*/ 	SwFrm *pPrv;
/*N*/ 	if( 0 != ( pPrv = pSect->GetIndPrev() ) )
/*N*/ 	{
/*N*/ 		// Herumlungernde, halbtote SectionFrms sollen uns nicht beirren
/*N*/ 		while( pPrv && pPrv->IsSctFrm() && !((SwSectionFrm*)pPrv)->GetSection() )
/*N*/ 			pPrv = pPrv->GetPrev();
/*N*/ 		if( pPrv )
/*?*/ 			return pCol;
/*N*/ 	}
/*N*/ 
/*N*/ 	const BOOL bBody = IsInDocBody();
/*N*/ 	const BOOL bFly  = IsInFly();
/*N*/ 
/*N*/ 	SwLayoutFrm *pLayLeaf = GetPrevLayoutLeaf();
/*N*/ 	SwLayoutFrm *pPrevLeaf = 0;
/*N*/ 
/*N*/ 	while ( pLayLeaf )
/*N*/ 	{	//In Tabellen oder Bereiche geht's niemals hinein.
/*N*/ 		if ( pLayLeaf->IsInTab() || pLayLeaf->IsInSct() )
/*N*/ 			pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
/*N*/ 		else if ( bBody && pLayLeaf->IsInDocBody() )
/*N*/ 		{
            // If there is a pLayLeaf has a lower pLayLeaf is the frame we are looking for.
            // Exception: pLayLeaf->Lower() is a zombie section frame
/*N*/             const SwFrm* pTmp = pLayLeaf->Lower();
/*N*/             // OD 11.04.2003 #108824# - consider, that the zombie section frame
/*N*/             // can have frame below it in the found layout leaf.
/*N*/             // Thus, skipping zombie section frame, if possible.
/*N*/             while ( pTmp && pTmp->IsSctFrm() &&
/*N*/                     !( static_cast<const SwSectionFrm*>(pTmp)->GetSection() ) &&
/*N*/                     pTmp->GetNext()
/*N*/                   )
/*N*/             {
/*N*/                 pTmp = pTmp->GetNext();
/*N*/             }
/*N*/             if ( pTmp &&
/*N*/                  ( !pTmp->IsSctFrm() ||
/*N*/                    ( static_cast<const SwSectionFrm*>(pTmp)->GetSection() )
/*N*/                  )
/*N*/                )
/*N*/             {
/*N*/         	    break;
/*N*/             }
/*N*/ 			pPrevLeaf = pLayLeaf;
/*N*/ 			pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
/*N*/ 			if ( pLayLeaf )
/*N*/ 				SwFlowFrm::SetMoveBwdJump( TRUE );
/*N*/ 		}
/*N*/ 		else if ( bFly )
/*?*/ 			break;	//Cntnts in Flys sollte jedes Layout-Blatt recht sein. Warum?
/*N*/ 		else
/*N*/ 			pLayLeaf = pLayLeaf->GetPrevLayoutLeaf();
/*N*/ 	}
/*N*/ 	if( !pLayLeaf )
/*N*/ 	{
/*N*/ 		if( !pPrevLeaf )
/*N*/ 			return pCol;
/*?*/ 		pLayLeaf = pPrevLeaf;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwSectionFrm* pNew = NULL;
/*N*/ 	// Zunaechst einmal an das Ende des Layoutblatts gehen
/*N*/ 	SwFrm *pTmp = pLayLeaf->Lower();
/*N*/ 	if( pTmp )
/*N*/ 	{
/*N*/ 		while( pTmp->GetNext() )
/*N*/ 			pTmp = pTmp->GetNext();
/*N*/ 		if( pTmp->IsSctFrm() )
/*N*/ 		{
/*N*/ 			// Halbtote stoeren hier nur...
/*N*/ 			while( !((SwSectionFrm*)pTmp)->GetSection() && pTmp->GetPrev() &&
/*N*/ 					pTmp->GetPrev()->IsSctFrm() )
/*?*/ 				pTmp = pTmp->GetPrev();
/*N*/ 			if( ((SwSectionFrm*)pTmp)->GetFollow() == pSect )
/*N*/ 				pNew = (SwSectionFrm*)pTmp;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( !pNew )
/*N*/ 	{
/*N*/         pNew = new SwSectionFrm( *pSect, TRUE );
/*N*/ 		pNew->InsertBefore( pLayLeaf, NULL );
/*N*/         pNew->Init();
/*N*/         SWRECTFN( pNew )
/*N*/         (pNew->*fnRect->fnMakePos)( pLayLeaf, pNew->GetPrev(), TRUE );
/*N*/ 
/*N*/ 		pLayLeaf = FIRSTLEAF( pNew );
/*N*/ 		if( !pNew->Lower() )    // einspaltige Bereiche formatieren
/*N*/ 		{
/*N*/ 			pNew->MakePos();
/*N*/ 			pLayLeaf->Format(); // damit die PrtArea fuers MoveBwd stimmt
/*N*/ 		}
/*N*/ 		else
/*?*/ 			pNew->SimpleFormat();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pLayLeaf = FIRSTLEAF( pNew );
/*N*/ 		if( pLayLeaf->IsColBodyFrm() )
/*N*/         {
/*?*/             // In existent section columns we're looking for the last not empty
/*?*/             // column.
/*?*/             SwLayoutFrm *pTmp = pLayLeaf;
/*?*/ 			while( pLayLeaf->GetUpper()->GetNext() )
/*?*/             {
/*?*/ 				pLayLeaf = (SwLayoutFrm*)((SwLayoutFrm*)pLayLeaf->GetUpper()->GetNext())->Lower();
/*?*/                 if( pLayLeaf->Lower() )
/*?*/                     pTmp = pLayLeaf;
/*?*/             }
/*?*/             // If we skipped an empty column, we've to set the jump-flag
/*?*/             if( pLayLeaf != pTmp )
/*?*/             {
/*?*/                 pLayLeaf = pTmp;
/*?*/                 SwFlowFrm::SetMoveBwdJump( TRUE );
/*?*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pLayLeaf;
/*N*/ }

/*N*/ SwTwips lcl_DeadLine( const SwFrm* pFrm )
/*N*/ {
/*N*/ 	const SwLayoutFrm* pUp = pFrm->GetUpper();
/*N*/ 	while( pUp && pUp->IsInSct() )
/*N*/ 	{
/*?*/ 		if( pUp->IsSctFrm() )
/*?*/ 			pUp = pUp->GetUpper();
/*?*/ 		// Spalten jetzt mit BodyFrm
/*?*/ 		else if( pUp->IsColBodyFrm() && pUp->GetUpper()->GetUpper()->IsSctFrm() )
/*?*/ 			pUp = pUp->GetUpper()->GetUpper();
/*?*/ 		else
/*?*/ 			break;
/*N*/ 	}
/*N*/     SWRECTFN( pFrm )
/*N*/     return pUp ? (pUp->*fnRect->fnGetPrtBottom)() :
/*N*/                  (pFrm->Frm().*fnRect->fnGetBottom)();
/*N*/ }

// SwSectionFrm::Growable(..) prueft, ob der SectionFrm noch wachsen kann,
// ggf. muss die Umgebung gefragt werden

/*N*/ BOOL SwSectionFrm::Growable() const
/*N*/ {
/*N*/     SWRECTFN( this )
/*N*/     if( (*fnRect->fnYDiff)( lcl_DeadLine( this ),
/*N*/         (Frm().*fnRect->fnGetBottom)() ) > 0 )
/*N*/ 		return TRUE;
/*N*/ 
/*N*/     return ( GetUpper() && ((SwFrm*)GetUpper())->Grow( LONG_MAX PHEIGHT, TRUE ) );
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::_Grow(), _Shrink()
|*
|*	Ersterstellung		AMA 14. Jan. 98
|*	Letzte Aenderung	AMA 14. Jan. 98
|*
|*************************************************************************/

/*N*/ SwTwips SwSectionFrm::_Grow( SwTwips nDist, BOOL bTst )
/*N*/ {
/*N*/     if ( !IsColLocked() && !HasFixSize() )
/*N*/ 	{
/*N*/         SWRECTFN( this )
/*N*/         long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/         if( nFrmHeight > 0 && nDist > (LONG_MAX - nFrmHeight) )
/*N*/             nDist = LONG_MAX - nFrmHeight;
/*N*/ 
/*N*/ 		if ( nDist <= 0L )
/*?*/ 			return 0L;
/*N*/ 
/*N*/         BOOL bInCalcCntnt = GetUpper() && IsInFly() && FindFlyFrm()->IsLocked();
/*N*/ 		if ( !Lower() || !Lower()->IsColumnFrm() || !Lower()->GetNext() ||
/*N*/ 			 GetSection()->GetFmt()->GetBalancedColumns().GetValue() )
/*N*/ 		{
/*N*/             SwTwips nGrow;
/*N*/             if( IsInFtn() )
/*?*/                 nGrow = 0;
/*N*/             else
/*N*/             {
/*N*/                 nGrow = lcl_DeadLine( this );
/*N*/                 nGrow = (*fnRect->fnYDiff)( nGrow,
/*N*/                                            (Frm().*fnRect->fnGetBottom)() );
/*N*/             }
/*N*/ 			SwTwips nSpace = nGrow;
/*N*/             if( !bInCalcCntnt && nGrow < nDist && GetUpper() )
/*N*/                 nGrow += GetUpper()->Grow( LONG_MAX PHEIGHT, TRUE );
/*N*/ 
/*N*/ 			if( nGrow > nDist )
/*N*/ 				nGrow = nDist;
/*N*/ 			if( nGrow <= 0 )
/*N*/ 			{
/*N*/ 				nGrow = 0;
/*N*/ 				if( nDist && !bTst )
/*N*/ 				{
/*N*/ 					if( bInCalcCntnt )
/*?*/ 						_InvalidateSize();
/*N*/ 					else
/*N*/ 						InvalidateSize();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if( !bTst )
/*N*/ 			{
/*N*/ 				if( bInCalcCntnt )
/*?*/ 					_InvalidateSize();
/*?*/ 				else if( nSpace < nGrow &&  nDist != nSpace + GetUpper()->
/*N*/                          Grow( nGrow - nSpace, FALSE ) )
/*N*/ 					InvalidateSize();
/*N*/ 				else
/*N*/ 				{
/*N*/ 					const SvxGraphicPosition ePos =
/*N*/ 						GetAttrSet()->GetBackground().GetGraphicPos();
/*N*/ 					if ( GPOS_RT < ePos && GPOS_TILED != ePos )
/*N*/ 					{
/*?*/ 						SetCompletePaint();
/*?*/ 						InvalidatePage();
/*N*/ 					}
/*N*/                     if( GetUpper() && GetUpper()->IsHeaderFrm() )
/*?*/                         GetUpper()->InvalidateSize();
/*N*/ 				}
/*N*/                 (Frm().*fnRect->fnAddBottom)( nGrow );
/*N*/                 long nPrtHeight = (Prt().*fnRect->fnGetHeight)() + nGrow;
/*N*/                 (Prt().*fnRect->fnSetHeight)( nPrtHeight );
/*N*/ 
/*N*/ 				if( Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
/*N*/ 				{
/*?*/ 					SwFrm* pTmp = Lower();
/*?*/ 					do
/*?*/ 					{
/*?*/ 						pTmp->_InvalidateSize();
/*?*/ 						pTmp = pTmp->GetNext();
/*?*/ 					} while ( pTmp );
/*?*/ 					_InvalidateSize();
/*N*/ 				}
/*N*/ 				if( GetNext() )
/*N*/ 				{
/*N*/ 					SwFrm *pFrm = GetNext();
/*N*/ 					while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
/*?*/ 						pFrm = pFrm->GetNext();
/*N*/ 					if( pFrm )
/*N*/ 					{
/*N*/ 						if( bInCalcCntnt )
/*?*/ 							pFrm->_InvalidatePos();
/*N*/ 						else
/*N*/ 							pFrm->InvalidatePos();
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			return nGrow;
/*N*/ 		}
/*?*/ 		if ( !bTst )
/*?*/ 		{
/*?*/ 			if( bInCalcCntnt )
/*?*/ 				_InvalidateSize();
/*?*/ 			else
/*?*/ 				InvalidateSize();
/*?*/ 		}
/*?*/ 	}
/*?*/ 	return 0L;
/*N*/ }

/*N*/ SwTwips SwSectionFrm::_Shrink( SwTwips nDist, BOOL bTst )
/*N*/ {
/*N*/     if ( Lower() && !IsColLocked() && !HasFixSize() )
/*N*/ 	{
/*N*/ 		if( ToMaximize( FALSE ) )
/*N*/ 		{
/*N*/ 			if( !bTst )
/*N*/ 				InvalidateSize();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             SWRECTFN( this )
/*N*/             long nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/             if ( nDist > nFrmHeight )
/*N*/                 nDist = nFrmHeight;
/*N*/ 
/*N*/ 			if ( Lower()->IsColumnFrm() && Lower()->GetNext() && // FtnAtEnd
/*N*/ 				 !GetSection()->GetFmt()->GetBalancedColumns().GetValue() )
/*N*/ 			{	//Bei Spaltigkeit ubernimmt das Format die Kontrolle ueber
/*?*/ 				//das Wachstum (wg. des Ausgleichs).
/*?*/ 				if ( !bTst )
/*?*/ 					InvalidateSize();
/*?*/ 				return nDist;
/*N*/ 			}
/*N*/ 			else if( !bTst )
/*N*/ 			{
/*N*/ 				const SvxGraphicPosition ePos =
/*N*/ 					GetAttrSet()->GetBackground().GetGraphicPos();
/*N*/ 				if ( GPOS_RT < ePos && GPOS_TILED != ePos )
/*N*/ 				{
/*?*/ 					SetCompletePaint();
/*?*/ 					InvalidatePage();
/*N*/ 				}
/*N*/                 (Frm().*fnRect->fnAddBottom)( -nDist );
/*N*/                 long nPrtHeight = (Prt().*fnRect->fnGetHeight)() - nDist;
/*N*/                 (Prt().*fnRect->fnSetHeight)( nPrtHeight );

/*N*/                 SwTwips nReal = 0;
/*N*/                 // We do not allow a section frame to shrink the its upper
/*N*/                 // footer frame. This is because in the calculation of a
/*N*/                 // footer frame, the content of the section frame is _not_
/*N*/                 // calculated. If there is a fly frame overlapping with the
/*N*/                 // footer frame, the section frame is not affected by this
/*N*/                 // during the calculation of the footer frame size.
/*N*/                 // The footer frame does not grow in its FormatSize function
/*N*/                 // but during the calculation of the content of the section
/*N*/                 // frame. The section frame grows until some of its text is
/*N*/                 // located on top of the fly frame. The next call of CalcCntnt
/*N*/                 // tries to shrink the section and here it would also shrink
/*N*/                 // the footer. This may not happen, because shrinking the footer
/*N*/                 // would cause the top of the section frame to overlap with the
/*N*/                 // fly frame again, this would result in a perfect loop.
/*N*/                 if( !GetUpper()->IsFooterFrm() )
/*N*/                     nReal = GetUpper()->Shrink( nDist, bTst );
/*N*/ 
/*N*/ 				if( Lower() && Lower()->IsColumnFrm() && Lower()->GetNext() )
/*N*/ 				{
/*N*/ 					SwFrm* pTmp = Lower();
/*N*/ 					do
/*N*/ 					{
/*N*/ 						pTmp->_InvalidateSize();
/*N*/ 						pTmp = pTmp->GetNext();
/*N*/ 					} while ( pTmp );
/*N*/ 				}
/*N*/ 				if( GetNext() )
/*N*/ 				{
/*N*/ 					SwFrm* pFrm = GetNext();
/*N*/ 					while( pFrm && pFrm->IsSctFrm() && !((SwSectionFrm*)pFrm)->GetSection() )
/*?*/ 						pFrm = pFrm->GetNext();
/*N*/ 					if( pFrm )
/*N*/ 						pFrm->InvalidatePos();
/*N*/ 					else
/*?*/ 						SetRetouche();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					SetRetouche();
/*N*/ 				return nDist;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0L;
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::MoveAllowed()
|*
|*	Ersterstellung		MA 08. Oct. 98
|*	Letzte Aenderung	MA 08. Oct. 98
|*
|*	Wann sind Frms innerhalb eines SectionFrms moveable?
|*  Wenn sie noch nicht in der letzten Spalte des SectionFrms sind,
|* 	wenn es einen Follow gibt,
|*  wenn der SectionFrm nicht mehr wachsen kann, wird es komplizierter,
|*  dann kommt es darauf an, ob der SectionFrm ein naechstes Layoutblatt
|*  finden kann. In (spaltigen/verketteten) Flys wird dies via GetNextLayout
|* 	geprueft, in Tabellen und in Kopf/Fusszeilen gibt es keins, im DocBody
|*  und auch im Fussnoten dagegen immer.
|*
|*  Benutzt wird diese Routine im TxtFormatter, um zu entscheiden, ob ein
|* 	(Absatz-)Follow erzeugt werden darf oder ob der Absatz zusammenhalten muss.
|*
|*************************************************************************/

/*N*/ BOOL SwSectionFrm::MoveAllowed( const SwFrm* pFrm) const
/*N*/ {
/*N*/ 	// Gibt es einen Follow oder ist der Frame nicht in der letzten Spalte?
/*N*/ 	if( HasFollow() || ( pFrm->GetUpper()->IsColBodyFrm() &&
/*N*/ 		pFrm->GetUpper()->GetUpper()->GetNext() ) )
/*N*/ 		return TRUE;
/*N*/ 	if( pFrm->IsInFtn() )
/*N*/ 	{
/*?*/ 		if( IsInFtn() )
/*?*/ 		{
/*?*/ 			if( GetUpper()->IsInSct() )
/*?*/ 			{
/*?*/ 				if( Growable() )
/*?*/ 					return FALSE;
/*?*/ 				return GetUpper()->FindSctFrm()->MoveAllowed( this );
/*?*/ 			}
/*?*/ 			else
/*?*/ 				return TRUE;
/*?*/ 		}
/*?*/ 		// The content of footnote inside a columned sectionfrm is moveable
/*?*/ 		// except in the last column
/*?*/ 		const SwLayoutFrm *pLay = pFrm->FindFtnFrm()->GetUpper()->GetUpper();
/*?*/ 		if( pLay->IsColumnFrm() && pLay->GetNext() )
/*?*/ 		{
/*?*/ 			// The first paragraph in the first footnote in the first column
/*?*/ 			// in the sectionfrm at the top of the page is not moveable,
/*?*/ 			// if the columnbody is empty.
/*?*/ 			BOOL bRet = FALSE;
/*?*/ 			if( pLay->GetIndPrev() || pFrm->GetIndPrev() ||
/*?*/ 				pFrm->FindFtnFrm()->GetPrev() )
/*?*/ 				bRet = TRUE;
/*?*/ 			else
/*?*/ 			{
/*?*/ 				SwLayoutFrm* pBody = ((SwColumnFrm*)pLay)->FindBodyCont();
/*?*/ 				if( pBody && pBody->Lower() )
/*?*/ 					bRet = TRUE;
/*?*/ 			}
/*?*/ 			if( bRet && ( IsFtnAtEnd() || !Growable() ) )
/*?*/ 				return TRUE;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	// Oder kann der Bereich noch wachsen?
/*N*/ 	if(	!IsColLocked() && Growable() )
/*N*/ 		return FALSE;
/*N*/ 	// Jetzt muss untersucht werden, ob es ein Layoutblatt gibt, in dem
/*N*/ 	// ein Bereichsfollow erzeugt werden kann.
/*N*/ 	if( IsInTab() || ( !IsInDocBody() && FindFooterOrHeader() ) )
/*?*/ 		return FALSE; // In Tabellen/Kopf/Fusszeilen geht es nicht
/*N*/ 	if( IsInFly() ) // Bei spaltigen oder verketteten Rahmen
/*?*/ 		return 0 != ((SwFrm*)GetUpper())->GetNextLeaf( MAKEPAGE_NONE );
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ SwFrm* SwFrm::_GetIndPrev()
/*N*/ {
/*N*/ 	SwFrm *pRet = NULL;
/*N*/ 	ASSERT( !pPrev && IsInSct(), "Why?" );
/*N*/ 	SwFrm* pSct = GetUpper();
/*N*/ 	if( !pSct )
/*?*/ 		return NULL;
/*N*/ 	if( pSct->IsSctFrm() )
/*N*/ 		pRet = pSct->GetIndPrev();
/*N*/ 	else if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
/*N*/ 	{   // Wir duerfen nur den Vorgaenger des SectionFrms zurueckliefern,
/*?*/ 		// wenn in keiner vorhergehenden Spalte mehr Inhalt ist
/*?*/ 		SwFrm* pCol = GetUpper()->GetUpper()->GetPrev();
/*?*/ 		while( pCol )
/*?*/ 		{
/*?*/ 			ASSERT( pCol->IsColumnFrm(), "GetIndPrev(): ColumnFrm expected" );
/*?*/ 			ASSERT( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
/*?*/ 					"GetIndPrev(): Where's the body?");
/*?*/ 			if( ((SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower())->Lower() )
/*?*/ 				return NULL;
/*?*/ 			pCol = pCol->GetPrev();
/*?*/ 		}
/*?*/ 		pRet = pSct->GetIndPrev();
/*N*/ 	}
/*N*/ 	// Scheintote SectionFrames ueberspringen wir lieber
/*N*/ 	while( pRet && pRet->IsSctFrm() && !((SwSectionFrm*)pRet)->GetSection() )
/*N*/ 		pRet = pRet->GetIndPrev();
/*N*/ 	return pRet;
/*N*/ }

/*N*/ SwFrm* SwFrm::_GetIndNext()
/*N*/ {
/*N*/ 	ASSERT( !pNext && IsInSct(), "Why?" );
/*N*/ 	SwFrm* pSct = GetUpper();
/*N*/ 	if( !pSct )
/*?*/ 		return NULL;
/*N*/ 	if( pSct->IsSctFrm() )
/*N*/ 		return pSct->GetIndNext();
/*N*/ 	if( pSct->IsColBodyFrm() && (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
/*N*/ 	{	// Wir duerfen nur den Nachfolger des SectionFrms zurueckliefern,
/*?*/ 		// wenn in keiner folgenden Spalte mehr Inhalt ist
/*?*/ 		SwFrm* pCol = GetUpper()->GetUpper()->GetNext();
/*?*/ 		while( pCol )
/*?*/ 		{
/*?*/ 			ASSERT( pCol->IsColumnFrm(), "GetIndNext(): ColumnFrm expected" );
/*?*/ 			ASSERT( pCol->GetLower() && pCol->GetLower()->IsBodyFrm(),
/*?*/ 					"GetIndNext(): Where's the body?");
/*?*/ 			if( ((SwLayoutFrm*)((SwLayoutFrm*)pCol)->Lower())->Lower() )
/*?*/ 				return NULL;
/*?*/ 			pCol = pCol->GetNext();
/*?*/ 		}
/*?*/ 		return pSct->GetIndNext();
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }



/*N*/ void SwSectionFrm::CalcFtnAtEndFlag()
/*N*/ {
/*N*/ 	SwSectionFmt *pFmt = GetSection()->GetFmt();
/*N*/ 	USHORT nVal = pFmt->GetFtnAtTxtEnd( FALSE ).GetValue();
/*N*/ 	bFtnAtEnd = FTNEND_ATPGORDOCEND != nVal;
/*N*/ 	bOwnFtnNum = FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
/*N*/ 				 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
/*N*/ 	while( !bFtnAtEnd && !bOwnFtnNum )
/*N*/ 	{
/*N*/ 		if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
/*N*/ 			pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
/*N*/ 		else
/*N*/ 			break;
/*N*/ 		nVal = pFmt->GetFtnAtTxtEnd( FALSE ).GetValue();
/*N*/ 		if( FTNEND_ATPGORDOCEND != nVal )
/*N*/ 		{
/*?*/ 			bFtnAtEnd = TRUE;
/*?*/ 			bOwnFtnNum = bOwnFtnNum ||FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
/*?*/ 						 FTNEND_ATTXTEND_OWNNUMANDFMT == nVal;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ void SwSectionFrm::CalcEndAtEndFlag()
/*N*/ {
/*N*/ 	SwSectionFmt *pFmt = GetSection()->GetFmt();
/*N*/ 	bEndnAtEnd = pFmt->GetEndAtTxtEnd( FALSE ).IsAtEnd();
/*N*/ 	while( !bEndnAtEnd )
/*N*/ 	{
/*N*/ 		if( pFmt->GetRegisteredIn()->ISA( SwSectionFmt ) )
/*N*/ 			pFmt = (SwSectionFmt*)pFmt->GetRegisteredIn();
/*N*/ 		else
/*N*/ 			break;
/*N*/ 		bEndnAtEnd = pFmt->GetEndAtTxtEnd( FALSE ).IsAtEnd();
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwSectionFrm::Modify()
|*
|*	Ersterstellung		MA 08. Oct. 98
|*	Letzte Aenderung	MA 08. Oct. 98
|*
|*************************************************************************/

/*N*/ void SwSectionFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
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
/*N*/ 			SwLayoutFrm::Modify( &aOldSet, &aNewSet );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		_UpdateAttr( pOld, pNew, nInvFlags );
/*N*/ 
/*N*/ 	if ( nInvFlags != 0 )
/*N*/ 	{
/*N*/ 		if ( nInvFlags & 0x01 )
/*N*/ 			InvalidateSize();
/*N*/ 		if ( nInvFlags & 0x10 )
/*N*/ 			SetCompletePaint();
/*N*/ 	}
/*N*/ }

/*N*/ void SwSectionFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
/*N*/ 							BYTE &rInvFlags,
/*N*/ 							SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
/*N*/ {
/*N*/ 	BOOL bClear = TRUE;
/*N*/ 	const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
/*N*/ 	switch( nWhich )
/*N*/ 	{   // Mehrspaltigkeit in Fussnoten unterdruecken...
/*N*/ 		case RES_FMT_CHG:
/*N*/ 		{
/*?*/ 			const SwFmtCol& rNewCol = GetFmt()->GetCol();
/*?*/ 			if( !IsInFtn() )
/*?*/ 			{
/*?*/ 				//Dummer Fall. Bei der Zuweisung einer Vorlage k”nnen wir uns
/*?*/ 				//nicht auf das alte Spaltenattribut verlassen. Da diese
/*?*/ 				//wenigstens anzahlgemass fuer ChgColumns vorliegen muessen,
/*?*/ 				//bleibt uns nur einen temporaeres Attribut zu basteln.
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwFmtCol aCol;
/*?*/ 			}
/*?*/ 			rInvFlags |= 0x01;
/*?*/ 			bClear = FALSE;
/*N*/ 		}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_COL:
/*N*/ 			if( !IsInFtn() )
/*N*/ 			{
/*N*/ 				ChgColumns( *(const SwFmtCol*)pOld, *(const SwFmtCol*)pNew );
/*N*/ 				rInvFlags |= 0x11;
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 		case RES_FTN_AT_TXTEND:
/*?*/ 			if( !IsInFtn() )
/*?*/ 			{
/*?*/ 				BOOL bOld = IsFtnAtEnd();
/*?*/ 				CalcFtnAtEndFlag();
/*?*/ 				if( bOld != IsFtnAtEnd() )
/*?*/ 				{
/*?*/ 					const SwFmtCol& rNewCol = GetFmt()->GetCol();
/*?*/ 					ChgColumns( rNewCol, rNewCol, TRUE );
/*?*/ 					rInvFlags |= 0x01;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			break;
/*?*/ 
/*?*/ 		case RES_END_AT_TXTEND:
/*?*/ 			if( !IsInFtn() )
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 BOOL bOld = IsEndnAtEnd();
/*?*/ 			}
/*?*/ 			break;
/*?*/ 		case RES_COLUMNBALANCE:
/*?*/ 			rInvFlags |= 0x01;
/*?*/ 			break;
/*?*/ 
/*?*/         case RES_FRAMEDIR :
/*?*/             SetDerivedR2L( sal_False );
/*?*/             CheckDirChange();
/*?*/             break;
/*M*/ 
/*M*/ 		case RES_PROTECT:
/*M*/ 			{ DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*M*/ 			}
/*M*/ 			break;
/*M*/ 
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

/*-----------------09.06.99 14:58-------------------
 * SwSectionFrm::ToMaximize(..): A follow or a ftncontainer at the end of the
 * page causes a maximal Size of the sectionframe.
 * --------------------------------------------------*/

/*N*/ BOOL SwSectionFrm::ToMaximize( BOOL bCheckFollow ) const
/*N*/ {
/*N*/ 	if( HasFollow() )
/*N*/ 	{
/*N*/ 		if( !bCheckFollow ) // Don't check superfluous follows
/*N*/ 			return TRUE;
/*?*/ 		const SwSectionFrm* pFoll = GetFollow();
/*?*/ 		while( pFoll && pFoll->IsSuperfluous() )
/*?*/ 			pFoll = pFoll->GetFollow();
/*?*/ 		if( pFoll )
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/ 	if( IsFtnAtEnd() )
/*?*/ 		return FALSE;
/*N*/ 	const SwFtnContFrm* pCont = ContainsFtnCont();
/*N*/ 	if( !IsEndnAtEnd() )
/*N*/ 		return 0 != pCont;
/*?*/ 	BOOL bRet = FALSE;
/*?*/ 	while( pCont && !bRet )
/*?*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( pCont->FindFootNote() )
/*?*/ 	}
/*?*/ 	return bRet;
/*N*/ }

/*-----------------09.06.99 15:07-------------------
 * BOOL SwSectionFrm::ContainsFtnCont()
 * checks every Column for FtnContFrms.
 * --------------------------------------------------*/

/*N*/ SwFtnContFrm* SwSectionFrm::ContainsFtnCont( const SwFtnContFrm* pCont ) const
/*N*/ {
/*N*/ 	SwFtnContFrm* pRet = NULL;
/*N*/ 	const SwLayoutFrm* pLay;
/*N*/ 	if( pCont )
/*N*/ 	{
/*?*/ 		pLay = pCont->FindFtnBossFrm( NULL );
/*?*/ 		ASSERT( IsAnLower( pLay ), "ConatainsFtnCont: Wrong FtnContainer" );
/*?*/ 		pLay = (SwLayoutFrm*)pLay->GetNext();
/*N*/ 	}
/*N*/ 	else if( Lower() && Lower()->IsColumnFrm() )
/*?*/ 		pLay = (SwLayoutFrm*)Lower();
/*N*/ 	else
/*N*/ 		pLay = NULL;
/*N*/ 	while ( !pRet && pLay )
/*N*/ 	{
/*?*/ 		if( pLay->Lower() && pLay->Lower()->GetNext() )
/*?*/ 		{
/*?*/ 			ASSERT( pLay->Lower()->GetNext()->IsFtnContFrm(),
/*?*/ 					"ToMaximize: Unexspected Frame" );
/*?*/ 			pRet = (SwFtnContFrm*)pLay->Lower()->GetNext();
/*?*/ 		}
/*?*/ 		ASSERT( !pLay->GetNext() || pLay->GetNext()->IsLayoutFrm(),
/*?*/ 				"ToMaximize: ColFrm exspected" );
/*?*/ 		pLay = (SwLayoutFrm*)pLay->GetNext();
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }

/*N*/ void SwSectionFrm::InvalidateFtnPos()
/*N*/ {
/*N*/ 	SwFtnContFrm* pCont = ContainsFtnCont( NULL );
/*N*/ 	if( pCont )
/*N*/ 	{
/*?*/ 		SwFrm *pTmp = pCont->ContainsCntnt();
/*?*/ 		if( pTmp )
/*?*/ 			pTmp->_InvalidatePos();
/*N*/ 	}
/*N*/ }

/*-----------------18.03.99 10:37-------------------
 * SwSectionFrm::Undersize() liefert den Betrag, um den der Bereich gern
 * groesser waere, wenn in ihm Undersized TxtFrms liegen, ansonsten Null.
 * Das Undersized-Flag wird ggf. korrigiert.
 * --------------------------------------------------*/

/*N*/ long SwSectionFrm::Undersize( BOOL bOverSize )
/*N*/ {
/*N*/ 	bUndersized = FALSE;
/*N*/     SWRECTFN( this )
/*N*/     long nRet = InnerHeight() - (Prt().*fnRect->fnGetHeight)();
/*N*/ 	if( nRet > 0 )
/*N*/ 		bUndersized = TRUE;
/*N*/ 	else if( !bOverSize )
/*N*/ 		nRet = 0;
/*N*/ 	return nRet;
/*N*/ }

/// OD 01.04.2003 #108446# - determine next frame for footnote/endnote formatting
/// before format of current one, because current one can move backward.
/// After moving backward to a previous page method <FindNext()> will return
/// the text frame presenting the first page footnote, if it exists. Thus, the
/// rest of the footnote/endnote container would not be formatted.
/*N*/ void SwSectionFrm::CalcFtnCntnt()
/*N*/ {
/*N*/ 	SwFtnContFrm* pCont = ContainsFtnCont();
/*N*/ 	if( pCont )
/*N*/ 	{
/*?*/ 		SwFrm* pFrm = pCont->ContainsAny();
/*?*/ 		if( pFrm )
/*?*/ 			pCont->Calc();
/*?*/ 		while( pFrm && IsAnLower( pFrm ) )
/*?*/ 		{
/*?*/ 			SwFtnFrm* pFtn = pFrm->FindFtnFrm();
/*?*/ 			if( pFtn )
/*?*/ 				pFtn->Calc();
/*?*/             // OD 01.04.2003 #108446# - determine next frame before format current frame.
/*?*/             SwFrm* pNextFrm = 0;
/*?*/             {
/*?*/                 if( pFrm->IsSctFrm() )
/*?*/                 {
/*?*/                     pNextFrm = static_cast<SwSectionFrm*>(pFrm)->ContainsAny();
/*?*/                 }
/*?*/                 if( !pNextFrm )
/*?*/                 {
/*?*/                     pNextFrm = pFrm->FindNext();
/*?*/                 }
/*?*/             }
/*?*/ 			pFrm->Calc();
/*?*/             pFrm = pNextFrm;
/*?*/ 		}
/*N*/ 	}
/*N*/ }

/* -----------------09.02.99 14:26-------------------
 * Wenn ein SectionFrm leerlaeuft, z.B. weil sein Inhalt die Seite/Spalte wechselt,
 * so wird er nicht sofort zerstoert (es koennte noch jemand auf dem Stack einen Pointer
 * auf ihn halten), sondern er traegt sich in eine Liste am RootFrm ein, die spaeter
 * abgearbeitet wird (in LayAction::Action u.a.). Seine Groesse wird auf Null gesetzt und
 * sein Zeiger auf seine Section ebenfalls. Solche zum Loeschen vorgesehene SectionFrms
 * muessen vom Layout/beim Formatieren ignoriert werden.
 *
 * Mit InsertEmptySct nimmt der RootFrm einen SectionFrm in die Liste auf,
 * mit RemoveFromList kann ein SectionFrm wieder aus der Liste entfernt werden (Dtor),
 * mit DeleteEmptySct wird die Liste abgearbeitet und die SectionFrms zerstoert
 * --------------------------------------------------*/

/*N*/ void SwRootFrm::InsertEmptySct( SwSectionFrm* pDel )
/*N*/ {
/*N*/ 	if( !pDestroy )
/*N*/ 		pDestroy = new SwDestroyList;
/*N*/ 	USHORT nPos;
/*N*/ 	if( !pDestroy->Seek_Entry( pDel, &nPos ) )
/*N*/ 		pDestroy->Insert( pDel );
/*N*/ }

/*N*/ void SwRootFrm::_DeleteEmptySct()
/*N*/ {
/*N*/ 	ASSERT( pDestroy, "Keine Liste, keine Kekse" );
/*N*/ 	while( pDestroy->Count() )
/*N*/ 	{
/*N*/ 		SwSectionFrm* pSect = (*pDestroy)[0];
/*N*/ 		pDestroy->Remove( USHORT(0) );
/*N*/ 		ASSERT( !pSect->IsColLocked() && !pSect->IsJoinLocked(),
/*N*/ 				"DeleteEmptySct: Locked SectionFrm" );
/*N*/ 		if( !pSect->Frm().HasArea() && !pSect->ContainsCntnt() )
/*N*/ 		{
/*N*/ 			SwLayoutFrm* pUp = pSect->GetUpper();
/*N*/ 			pSect->Remove();
/*N*/ 			delete pSect;
/*N*/ 			if( pUp && !pUp->Lower() )
/*N*/ 			{
/*N*/ 				if( pUp->IsPageBodyFrm() )
/*N*/ 					pUp->FindRootFrm()->SetSuperfluous();
/*N*/ 				else if( pUp->IsFtnFrm() && !pUp->IsColLocked() &&
/*N*/ 					pUp->GetUpper() )
/*N*/ 				{
/*?*/ 					pUp->Cut();
/*?*/ 					delete pUp;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*?*/ 			ASSERT( pSect->GetSection(), "DeleteEmptySct: Halbtoter SectionFrm?!" );
/*N*/ 	}
/*N*/ }

/*N*/ void SwRootFrm::_RemoveFromList( SwSectionFrm* pSct )
/*N*/ {
/*N*/ 	ASSERT( pDestroy, "Where's my list?" );
/*N*/ 	USHORT nPos;
/*N*/ 	if( pDestroy->Seek_Entry( pSct, &nPos ) )
/*?*/ 		pDestroy->Remove( nPos );
/*N*/ }

/*N*/ #ifdef DBG_UTIL

/*N*/ BOOL SwRootFrm::IsInDelList( SwSectionFrm* pSct ) const
/*N*/ {
/*N*/ 	USHORT nPos;
/*N*/ 	return ( pDestroy && pDestroy->Seek_Entry( pSct, &nPos ) );
/*N*/ }

/*N*/ #endif
}
