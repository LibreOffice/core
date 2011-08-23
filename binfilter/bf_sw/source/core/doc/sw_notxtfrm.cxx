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



#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _FMTSRND_HXX
#include <fmtsrnd.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _FESH_HXX
#include <fesh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _NOTXTFRM_HXX
#include <notxtfrm.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif

#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
namespace binfilter {

#define DEFTEXTSIZE  12

// OD 25.09.2002 #99739# - insert declaration of global methods <SwAlignRect>
//     and <SwAlignGrtRect>.
//     Methods are implemented in /core/layout/paintfrm.cxx
// OD 24.01.2003 #106593# - no longer needed, included in <frmtool.hxx>
//extern void MA_FASTCALL SwAlignRect( SwRect &rRect, ViewShell *pSh );
//extern void SwAlignGrfRect( SwRect *pGrfRect, const OutputDevice &rOut );

//Zum asynchronen (erstmaligem) anfordern von Grafiken


extern void ClrContourCache( const SdrObject *pObj ); // TxtFly.Cxx











/*************************************************************************
|*
|*	  SwGrfFrm::SwGrfFrm(ViewShell * const,SwGrfNode *)
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 03. Mar. 93
|*
*************************************************************************/


/*N*/ SwNoTxtFrm::SwNoTxtFrm(SwNoTxtNode * const pNode)
/*N*/ 	: SwCntntFrm(pNode)
/*N*/ {
/*N*/ 	InitCtor();
/*N*/ }

// Initialisierung: z.Zt. Eintragen des Frames im Cache


/*N*/ void SwNoTxtFrm::InitCtor()
/*N*/ {
/*N*/     nType = FRMC_NOTXT;
/*N*/ 	// Das Gewicht der Grafik ist 0, wenn sie noch nicht
/*N*/ 	// gelesen ist, < 0, wenn ein Lesefehler auftrat und
/*N*/ 	// Ersatzdarstellung angewendet werden musste und >0,
/*N*/ 	// wenn sie zur Verfuegung steht.
/*N*/ 	nWeight = 0;
/*N*/ }

/*************************************************************************
|*
|*	  SwNoTxtNode::MakeFrm()
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 03. Mar. 93
|*
*************************************************************************/


/*N*/ SwCntntFrm *SwNoTxtNode::MakeFrm()
/*N*/ {
/*N*/ 	return new SwNoTxtFrm(this);
/*N*/ }

/*************************************************************************
|*
|*	  SwNoTxtFrm::~SwNoTxtFrm()
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 30. Apr. 96
|*
*************************************************************************/

/*N*/ SwNoTxtFrm::~SwNoTxtFrm()
/*N*/ {
/*N*/ 	StopAnimation();
/*N*/ }

/*************************************************************************
|*
|*	  void SwNoTxtFrm::Modify( SwHint * pOld, SwHint * pNew )
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	JP 05.03.91
|*
*************************************************************************/






/*************************************************************************
|*
|*	  void SwNoTxtFrm::Paint()
|*
|*	  Beschreibung
|*	  Ersterstellung	JP 05.03.91
|*	  Letzte Aenderung	MA 10. Jan. 97
|*
*************************************************************************/

/*N*/ void SwNoTxtFrm::Paint( const SwRect &rRect ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*************************************************************************
|*
|*    void lcl_CalcRect( Point & aPt, Size & aDim,
|*                   USHORT nMirror )
|*
|*    Beschreibung      Errechne die Position und die Groesse der Grafik im
|*                      Frame, entsprechen der aktuellen Grafik-Attribute
|*
|*    Parameter         Point&  die Position im Frame  ( auch Return-Wert )
|*                      Size&   die Groesse der Grafik ( auch Return-Wert )
|*                      MirrorGrf   akt. Spiegelungs-Attribut
|*    Ersterstellung    JP 04.03.91
|*    Letzte Aenderung  JP 31.08.94
|*
*************************************************************************/



/*************************************************************************
|*
|*	  void SwNoTxtFrm::GetGrfArea()
|*
|*	  Beschreibung		Errechne die Position und die Groesse der Bitmap
|*						innerhalb des uebergebenem Rechtecks.
|*
|*	  Ersterstellung	JP 03.09.91
|*	  Letzte Aenderung	MA 11. Oct. 94
|*
*************************************************************************/


/*************************************************************************
|*
|*	  Size SwNoTxtFrm::GetSize()
|*
|*	  Beschreibung		Gebe die Groesse des umgebenen FLys und
|*						damit die der Grafik zurueck.
|*	  Ersterstellung	JP 04.03.91
|*	  Letzte Aenderung	JP 31.08.94
|*
*************************************************************************/


/*N*/ const Size& SwNoTxtFrm::GetSize() const
/*N*/ {
/*N*/ 	// gebe die Groesse des Frames zurueck
/*N*/ 	const SwFrm *pFly = FindFlyFrm();
/*N*/ 	if( !pFly )
/*?*/ 		pFly = this;
/*N*/ 	return pFly->Prt().SSize();
/*N*/ }

/*************************************************************************
|*
|*	  SwNoTxtFrm::MakeAll()
|*
|*	  Ersterstellung	MA 29. Nov. 96
|*	  Letzte Aenderung	MA 29. Nov. 96
|*
*************************************************************************/


/*N*/ void SwNoTxtFrm::MakeAll()
/*N*/ {
/*N*/ 	SwCntntNotify aNotify( this );
/*N*/ 	SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
/*N*/ 	const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 
/*N*/ 	while ( !bValidPos || !bValidSize || !bValidPrtArea )
/*N*/ 	{
/*N*/ 		MakePos();
/*N*/ 
/*N*/ 		if ( !bValidSize )
/*N*/ 			Frm().Width( GetUpper()->Prt().Width() );
/*N*/ 
/*N*/ 		MakePrtArea( rAttrs );
/*N*/ 
/*N*/ 		if ( !bValidSize )
/*N*/ 		{	bValidSize = TRUE;
/*N*/ 			Format();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  SwNoTxtFrm::Format()
|*
|*	  Beschreibung		Errechne die Groesse der Bitmap, wenn noetig
|*	  Ersterstellung	JP 11.03.91
|*	  Letzte Aenderung	MA 13. Mar. 96
|*
*************************************************************************/


/*M*/ void SwNoTxtFrm::Format( const SwBorderAttrs * )
/*M*/ {
/*M*/ 	const Size aNewSize( GetSize() );
/*M*/ 
/*M*/ 	// hat sich die Hoehe geaendert?
/*M*/ 	SwTwips nChgHght = IsVertical() ? 
/*M*/ 		(SwTwips)(aNewSize.Width() - Prt().Width()) :
/*M*/ 		(SwTwips)(aNewSize.Height() - Prt().Height());
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/ 	if( nChgHght > 0)
/*M*/         Grow( nChgHght );
/*M*/ 	else if( nChgHght < 0)
/*M*/         Shrink( Min(Prt().Height(), -nChgHght) );
/*M*/ #else
/*M*/ 	const SzPtr pVar = pVARSIZE;
/*M*/ 	if( nChgHght > 0)
/*M*/ 		Grow( nChgHght, pVar );
/*M*/ 	else if( nChgHght < 0)
/*M*/ 		Shrink( Min(Prt().Height(), -nChgHght), pVar );
/*M*/ #endif
/*M*/ }

/*************************************************************************
|*
|*	  SwNoTxtFrm::GetCharRect()
|*
|*	  Beschreibung
|*	  Ersterstellung	SS 29-Apr-1991
|*	  Letzte Aenderung	MA 10. Oct. 94
|*
|*************************************************************************/


/*N*/ BOOL SwNoTxtFrm::GetCharRect( SwRect &rRect, const SwPosition& rPos,
/*N*/ 							  SwCrsrMoveState *pCMS ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ BOOL SwNoTxtFrm::GetCrsrOfst(SwPosition* pPos, Point& aPoint,
/*N*/ 							const SwCrsrMoveState* ) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ #define CLEARCACHE( pNd ) {\
/*N*/ 	(pNd)->GetGrfObj().ReleaseFromCache();\
/*N*/ 	SwFlyFrm* pFly = FindFlyFrm();\
/*N*/ 	if( pFly && pFly->GetFmt()->GetSurround().IsContour() )\
/*N*/ 	{\
/*N*/ 		ClrContourCache( pFly->GetVirtDrawObj() );\
/*N*/ 		pFly->NotifyBackground( FindPageFrm(), Prt(), PREP_FLY_ATTR_CHG );\
/*N*/ 	}\
/*N*/ }

/*N*/ void SwNoTxtFrm::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pNew ? pNew->Which() : pOld ? pOld->Which() : 0;
/*N*/ 
/*N*/ 	if ( RES_GRAPHIC_PIECE_ARRIVED != nWhich &&
/*N*/ 		 RES_GRAPHIC_ARRIVED != nWhich &&
/*N*/ 		 RES_GRF_REREAD_AND_INCACHE != nWhich )
/*N*/ 		SwCntntFrm::Modify( pOld, pNew );
/*N*/ 
/*N*/ 	FASTBOOL bCompletePaint = TRUE;
/*N*/ 
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 	case RES_OBJECTDYING:
/*N*/ 		break;
/*N*/ 
/*?*/ 	case RES_GRF_REREAD_AND_INCACHE:
/*?*/ 		if( ND_GRFNODE == GetNode()->GetNodeType() )
/*?*/ 		{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	bCompletePaint = FALSE;
/*?*/ 		}
/*?*/ 		break;
/*?*/ 
/*N*/ 	case RES_UPDATE_ATTR:
/*N*/ 	case RES_FMT_CHG:
/*N*/ 		CLEARCACHE( (SwGrfNode*) GetNode() )
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_ATTRSET_CHG:
/*N*/ 		{
                USHORT n;
/*N*/ 			for( n = RES_GRFATR_BEGIN; n < RES_GRFATR_END; ++n )
/*N*/ 				if( SFX_ITEM_SET == ((SwAttrSetChg*)pOld)->GetChgSet()->
/*N*/ 								GetItemState( n, FALSE ))
/*N*/ 				{
/*N*/ 					CLEARCACHE( (SwGrfNode*) GetNode() )
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			if( RES_GRFATR_END == n )			// not found
/*N*/ 				return ;
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	case RES_GRAPHIC_PIECE_ARRIVED:
/*N*/ 	case RES_GRAPHIC_ARRIVED:
/*N*/ 		if ( GetNode()->GetNodeType() == ND_GRFNODE )
/*N*/ 		{
/*N*/ 			bCompletePaint = FALSE;
/*N*/ 			SwGrfNode* pNd = (SwGrfNode*) GetNode();
/*N*/ 
/*N*/ 			CLEARCACHE( pNd )
/*N*/ 
/*N*/ 			SwRect aRect( Frm() );
/*N*/ 
/*N*/ 			ViewShell *pVSh = 0;
/*N*/ 			pNd->GetDoc()->GetEditShell( &pVSh );
/*N*/             if( !pVSh )
/*N*/ 				break;
/*N*/ 
/*N*/ 			ViewShell *pSh = pVSh;
/*N*/ 			do {
/*N*/ 				SET_CURR_SHELL( pSh );
/*N*/ 				if( pSh->IsPreView() )
/*N*/ 				{
/*?*/ 					if( pSh->GetWin() )
/*?*/                       DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ::binfilter::RepaintPagePreview( pSh, aRect );
/*?*/ 				}
/*?*/ 				else if ( pSh->VisArea().IsOver( aRect ) &&
/*?*/ 					 OUTDEV_WINDOW == pSh->GetOut()->GetOutDevType() )
/*?*/ 				{
/*?*/                     // OD 27.11.2002 #105519# - invalidate instead of painting
/*?*/                     pSh->GetWin()->Invalidate( aRect.SVRect() );
/*?*/ 				}
/*N*/ 
/*N*/ 				pSh = (ViewShell *)pSh->GetNext();
/*N*/ 			} while( pSh != pVSh );
/*N*/ 		}
/*N*/ 		break;
/*N*/ 
/*N*/ 	default:
/*?*/ 		if( !pNew || RES_GRFATR_BEGIN > nWhich || nWhich >= RES_GRFATR_END )
/*?*/ 			return;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bCompletePaint )
/*N*/ 	{
/*N*/ 		InvalidatePrt();
/*N*/ 		SetCompletePaint();
/*N*/ 	}
/*N*/ }

/*N*/ void SwNoTxtFrm::StopAnimation( OutputDevice* pOut ) const
/*N*/ {
/*N*/ 	//animierte Grafiken anhalten
/*N*/ 	SwGrfNode* pGrfNd = (SwGrfNode*)GetNode()->GetGrfNode();
/*N*/ 	if( pGrfNd && pGrfNd->IsAnimated() )
/*N*/ 		pGrfNd->GetGrfObj().StopAnimation( pOut, long(this) );
/*N*/ }

 
/*N*/ BOOL SwNoTxtFrm::HasAnimation() const
/*N*/ {
/*N*/ 	const SwGrfNode* pGrfNd = GetNode()->GetGrfNode();
/*N*/ 	return pGrfNd && pGrfNd->IsAnimated();
/*N*/ }



}
