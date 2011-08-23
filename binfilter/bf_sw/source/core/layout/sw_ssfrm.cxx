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

#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _DFLYOBJ_HXX
#include <dflyobj.hxx>
#endif
#ifndef _FLYFRM_HXX
#include <flyfrm.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>		// ClearPara()
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _TXTFTN_HXX
#include <txtftn.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif

#ifndef _FRMTOOL_HXX
#include <frmtool.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTS_HXX
#include <hints.hxx>		//fuer SwFmtChg
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <bf_svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <bf_svx/shaditem.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _FRMSH_HXX
#include <frmsh.hxx>
#endif
namespace binfilter {

    // No inline cause we need the function pointers
/*N*/ long SwFrm::GetTopMargin() const
/*N*/     { return Prt().Top(); }
/*N*/ long SwFrm::GetBottomMargin() const
/*N*/     { return Frm().Height() -Prt().Height() -Prt().Top(); }
/*N*/ long SwFrm::GetLeftMargin() const
/*N*/     { return Prt().Left(); }
/*N*/ long SwFrm::GetRightMargin() const
/*N*/     { return Frm().Width() - Prt().Width() - Prt().Left(); }
/*N*/ long SwFrm::GetPrtLeft() const
/*N*/     { return Frm().Left() + Prt().Left(); }
/*N*/ long SwFrm::GetPrtBottom() const
/*N*/     { return Frm().Top() + Prt().Height() + Prt().Top(); }
/*N*/ long SwFrm::GetPrtRight() const
/*N*/     { return Frm().Left() + Prt().Width() + Prt().Left(); }
/*N*/ long SwFrm::GetPrtTop() const
/*N*/     { return Frm().Top() + Prt().Top(); }

/*N*/ BOOL SwFrm::SetMinLeft( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = nDeadline - Frm().Left();
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Left( nDeadline );
/*?*/         Prt().Width( Prt().Width() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ BOOL SwFrm::SetMaxBottom( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = Frm().Top() + Frm().Height() - nDeadline;
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Height( Frm().Height() - nDiff );
/*?*/         Prt().Height( Prt().Height() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ BOOL SwFrm::SetMinTop( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = nDeadline - Frm().Top();
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Top( nDeadline );
/*?*/         Prt().Height( Prt().Height() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ BOOL SwFrm::SetMaxRight( long nDeadline )
/*N*/ {
/*N*/     SwTwips nDiff = Frm().Left() + Frm().Width() - nDeadline;
/*N*/     if( nDiff > 0 )
/*N*/     {
/*?*/         Frm().Width( Frm().Width() - nDiff );
/*?*/         Prt().Width( Prt().Width() - nDiff );
/*?*/         return TRUE;
/*N*/     }
/*N*/     return FALSE;
/*N*/ }

/*N*/ void SwFrm::MakeBelowPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().Y() += pPrv->Frm().Height();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().Y() += 1;
/*N*/ }

/*N*/ void SwFrm::MakeUpperPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().Y() -= Frm().Height();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/         aFrm.Pos().Y() += pUp->Prt().Height() - aFrm.Height();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().Y() -= 1;
/*N*/ }

/*N*/ void SwFrm::MakeLeftPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().X() -= Frm().Width();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/         aFrm.Pos().X() += pUp->Prt().Width() - aFrm.Width();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().X() -= 1;
/*N*/ }

/*N*/ void SwFrm::MakeRightPos( const SwFrm* pUp, const SwFrm* pPrv, BOOL bNotify )
/*N*/ {
/*N*/     if( pPrv )
/*N*/     {
/*N*/         aFrm.Pos( pPrv->Frm().Pos() );
/*N*/         aFrm.Pos().X() += pPrv->Frm().Width();
/*N*/     }
/*N*/     else
/*N*/     {
/*N*/         aFrm.Pos( pUp->Frm().Pos() );
/*N*/         aFrm.Pos() += pUp->Prt().Pos();
/*N*/     }
/*N*/     if( bNotify )
/*N*/         aFrm.Pos().X() += 1;
/*N*/ }

/*N*/ void SwFrm::SetTopBottomMargins( long nTop, long nBot )
/*N*/ {
/*N*/     Prt().Top( nTop );
/*N*/     Prt().Height( Frm().Height() - nTop - nBot );
/*N*/ }

/*N*/ void SwFrm::SetBottomTopMargins( long nBot, long nTop )
/*N*/ {
/*N*/     Prt().Top( nTop );
/*N*/     Prt().Height( Frm().Height() - nTop - nBot );
/*N*/ }

/*N*/ void SwFrm::SetLeftRightMargins( long nLeft, long nRight)
/*N*/ {
/*N*/     Prt().Left( nLeft );
/*N*/     Prt().Width( Frm().Width() - nLeft - nRight );
/*N*/ }

/*N*/ void SwFrm::SetRightLeftMargins( long nRight, long nLeft)
/*N*/ {
/*N*/     Prt().Left( nLeft );
/*N*/     Prt().Width( Frm().Width() - nLeft - nRight );
/*N*/ }

/*-----------------11.9.2001 11:11------------------
 * SwFrm::CheckDirChange(..)
 * checks the layout direction and
 * invalidates the lower frames rekursivly, if necessary.
 * --------------------------------------------------*/

/*N*/ void SwFrm::CheckDirChange()
/*N*/ {
/*N*/     BOOL bOldVert = GetVerticalFlag();
/*N*/     BOOL bOldRev = IsReverse();
/*N*/     BOOL bOldR2L = GetRightToLeftFlag();
/*N*/     SetInvalidVert( TRUE );
/*N*/     SetInvalidR2L( TRUE );
/*N*/     BOOL bChg = bOldR2L != IsRightToLeft();
/*N*/     if( ( IsVertical() != bOldVert ) || bChg || IsReverse() != bOldRev )
/*N*/     {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/     }
/*N*/ }

/*-----------------13.9.2002 11:11------------------
 * SwFrm::GetAnchorPos(..)
 * returns the position for anchors based on frame direction
 * --------------------------------------------------*/

/*N*/ Point SwFrm::GetFrmAnchorPos( sal_Bool bIgnoreFlysAnchoredAtThisFrame ) const
/*N*/ {
/*N*/     Point aAnchor = Frm().Pos();
/*N*/     if ( IsVertical() || IsRightToLeft() )
/*N*/         aAnchor.X() += Frm().Width();
/*N*/ 
/*N*/     if ( IsTxtFrm() )
/*N*/     {
/*N*/         SwTwips nBaseOfstForFly =
/*N*/             ((SwTxtFrm*)this)->GetBaseOfstForFly( bIgnoreFlysAnchoredAtThisFrame );
/*N*/         if ( IsVertical() )
/*N*/             aAnchor.Y() += nBaseOfstForFly;
/*N*/         else
/*N*/             aAnchor.X() += nBaseOfstForFly;
/*N*/     }
/*N*/ 
/*N*/     return aAnchor;
/*N*/ }


/*************************************************************************
|*
|*	SwFrm::~SwFrm()
|*
|*	Ersterstellung		MA 02. Mar. 94
|*	Letzte Aenderung	MA 25. Jun. 95
|*
|*************************************************************************/


/*N*/ SwFrm::~SwFrm()
/*N*/ {
/*N*/ #ifdef ACCESSIBLE_LAYOUT
/*N*/ 	// accessible objects for fly and cell frames have been already disposed
/*N*/ 	// by the destructors of the derived classes.
/*N*/ 	if( IsAccessibleFrm() && !(IsFlyFrm() || IsCellFrm()) && GetDep() )
/*N*/ 	{
/*N*/ 		SwRootFrm *pRootFrm = FindRootFrm();
/*N*/ 		if( pRootFrm && pRootFrm->IsAnyShellAccessible() )
/*N*/ 		{
/*?*/ 			ViewShell *pVSh = pRootFrm->GetCurrShell();
/*?*/ 			if( pVSh && pVSh->Imp() )
/*?*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ASSERT( !GetLower(), "Lowers should be dispose already!" );
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	if( pDrawObjs )
/*N*/ 	{
/*N*/ 		for ( USHORT i = pDrawObjs->Count(); i; )
/*N*/ 		{
/*N*/ 			SdrObject *pObj = (*pDrawObjs)[--i];
/*N*/ 			if ( pObj->IsWriterFlyFrame() )
/*N*/ 				delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/             else
/*N*/             // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
/*N*/             {
/*N*/                 if ( pObj->GetUserCall() )
/*N*/                 {
/*N*/                     ((SwDrawContact*)pObj->GetUserCall())->DisconnectObjFromLayout( pObj );
/*N*/                 }
/*N*/             }
/*N*/ 		}
/*N*/ 		if ( pDrawObjs )
/*N*/ 			delete pDrawObjs;
/*N*/ 	}
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/ 	// JP 15.10.2001: for detection of access to deleted frames
/*N*/ 	pDrawObjs = (SwDrawObjs*)0x33333333;
/*N*/ #endif
/*N*/ }

/*************************************************************************
|*
|*	  SwLayoutFrm::SetFrmFmt()
|*	  Ersterstellung	MA 22. Apr. 93
|*	  Letzte Aenderung	MA 02. Nov. 94
|*
|*************************************************************************/


/*N*/ void SwLayoutFrm::SetFrmFmt( SwFrmFmt *pNew )
/*N*/ {
/*N*/ 	if ( pNew != GetFmt() )
/*N*/ 	{
/*N*/ 		SwFmtChg aOldFmt( GetFmt() );
/*N*/ 		pNew->Add( this );
/*N*/ 		SwFmtChg aNewFmt( pNew );
/*N*/ 		Modify( &aOldFmt, &aNewFmt );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	  SwCntntFrm::SwCntntFrm(), ~SwCntntFrm()
|*
|*	  Ersterstellung	AK 15-Feb-1991
|*	  Letzte Aenderung	MA 25. Apr. 95
|*
|*************************************************************************/


/*N*/ SwCntntFrm::SwCntntFrm( SwCntntNode * const pCntnt ) :
/*N*/ 	SwFrm( pCntnt ),
/*N*/ 	SwFlowFrm( (SwFrm&)*this )
/*N*/ {
/*N*/ }



/*N*/ SwCntntFrm::~SwCntntFrm()
/*N*/ {
/*N*/ 	SwCntntNode* pCNd;
/*N*/ 	if( 0 != ( pCNd = PTR_CAST( SwCntntNode, pRegisteredIn )) &&
/*N*/ 		!pCNd->GetDoc()->IsInDtor() )
/*N*/ 	{
/*N*/ 		//Bei der Root abmelden wenn ich dort noch im Turbo stehe.
/*N*/ 		SwRootFrm *pRoot = FindRootFrm();
/*N*/ 		if( pRoot && pRoot->GetTurbo() == this )
/*N*/ 		{
/*?*/ 			pRoot->DisallowTurbo();
/*?*/ 			pRoot->ResetTurbo();
/*N*/ 		}
/*N*/         if( IsTxtFrm() && ((SwTxtFrm*)this)->HasFtn() )
/*N*/         {
/*?*/             SwTxtNode *pTxtNd = ((SwTxtFrm*)this)->GetTxtNode();
/*?*/             const SwFtnIdxs &rFtnIdxs = pCNd->GetDoc()->GetFtnIdxs();
/*?*/             USHORT nPos;
/*?*/             ULONG nIndex = pCNd->GetIndex();
/*?*/             rFtnIdxs.SeekEntry( *pTxtNd, &nPos );
/*?*/             SwTxtFtn* pTxtFtn;
/*?*/             if( nPos < rFtnIdxs.Count() )
/*?*/             {
/*?*/                 while( nPos && pTxtNd == &(rFtnIdxs[ nPos ]->GetTxtNode()) )
/*?*/                     --nPos;
/*?*/                 if( nPos || pTxtNd != &(rFtnIdxs[ nPos ]->GetTxtNode()) )
/*?*/                     ++nPos;
/*?*/             }
/*?*/             while( nPos < rFtnIdxs.Count() )
/*?*/             {
/*?*/                 DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pTxtFtn = rFtnIdxs[ nPos ];
/*?*/             }
/*N*/         }
/*N*/ 	}
/*N*/ 	if( IsTxtFrm() && ((SwTxtFrm*)this)->HasBlinkPor() )
/*?*/ 		((SwTxtFrm*)this)->ClearPara();
/*N*/ }

/*************************************************************************
|*
|*	SwLayoutFrm::~SwLayoutFrm
|*
|*	Ersterstellung		AK 28-Feb-1991
|*	Letzte Aenderung	MA 11. Jan. 95
|*
|*************************************************************************/


/*N*/ SwLayoutFrm::~SwLayoutFrm()
/*N*/ {
/*N*/ 	SwFrm *pFrm = pLower;
/*N*/ 
/*N*/ 	if( GetFmt() && !GetFmt()->GetDoc()->IsInDtor() )
/*N*/ 	{
/*N*/         while ( pFrm )
/*N*/ 		{
/*N*/ 			//Erst die Objs des Frm vernichten, denn diese koennen sich sonst nach
/*N*/ 			//dem Remove nicht mehr bei der Seite abmelden.
/*N*/ 			//Falls sich einer nicht abmeldet wollen wir nicht gleich
/*N*/ 			//endlos schleifen.
/*N*/ 
/*N*/             USHORT nCnt;
/*N*/ 			while ( pFrm->GetDrawObjs() && pFrm->GetDrawObjs()->Count() )
/*N*/ 			{
/*N*/ 				nCnt = pFrm->GetDrawObjs()->Count();
/*N*/ 				SdrObject *pObj = (*pFrm->GetDrawObjs())[0];
/*N*/ 				if ( pObj->IsWriterFlyFrame() )
/*N*/ 					delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 				else if ( pObj->GetUserCall() )
/*N*/                 {
/*N*/                     // OD 19.06.2003 #108784# - adjustments for drawing objects
/*N*/                     // in header/footer.
/*N*/                     ((SwDrawContact*)pObj->GetUserCall())->DisconnectObjFromLayout( pObj );
/*N*/                 }
/*N*/ 
/*N*/ 				if ( pFrm->GetDrawObjs() &&
/*N*/ 					 nCnt == pFrm->GetDrawObjs()->Count() )
/*N*/                 {
/*N*/ 					pFrm->GetDrawObjs()->Remove( 0 );
/*N*/                 }
/*N*/ 			}
/*N*/ 			pFrm->Remove();
/*N*/ 			delete pFrm;
/*N*/ 			pFrm = pLower;
/*N*/ 		}
/*N*/ 		//Fly's vernichten. Der letzte loescht gleich das Array.
/*N*/ 		USHORT nCnt;
/*N*/ 		while ( GetDrawObjs() && GetDrawObjs()->Count() )
/*N*/ 		{
/*N*/ 			nCnt = GetDrawObjs()->Count();
/*N*/ 			SdrObject *pObj = (*GetDrawObjs())[0];
/*N*/ 			if ( pObj->IsWriterFlyFrame() )
/*N*/ 				delete ((SwVirtFlyDrawObj*)pObj)->GetFlyFrm();
/*N*/ 			else if ( pObj->GetUserCall() )
/*N*/             {
/*N*/                 // OD 19.06.2003 #108784# - adjustments for drawing objects
/*N*/                 // in header/footer.
/*N*/                 ((SwDrawContact*)pObj->GetUserCall())->DisconnectObjFromLayout( pObj );
/*N*/             }
/*N*/ 
/*N*/ 			if ( GetDrawObjs() && nCnt == GetDrawObjs()->Count() )
/*N*/ 				GetDrawObjs()->Remove( 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		while( pFrm )
/*N*/ 		{
/*N*/ 			SwFrm *pNxt = pFrm->GetNext();
/*N*/ 			delete pFrm;
/*N*/ 			pFrm = pNxt;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::PaintArea()
|*
|*	Created		AMA 08/22/2000
|*	Last change	AMA 08/23/2000
|*
|*  The paintarea is the area, in which the content of a frame is allowed
|*  to be displayed. This region could be larger than the printarea (Prt())
|*  of the upper, it includes e.g. often the margin of the page.
|*
|*************************************************************************/

/*N*/ const SwRect SwFrm::PaintArea() const
/*N*/ {
/*N*/ 	SwRect aRect( Frm() );
/*N*/     const FASTBOOL bVert = IsVertical();
/*N*/     SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
/*N*/     long nRight = (aRect.*fnRect->fnGetRight)();
/*N*/     long nLeft  = (aRect.*fnRect->fnGetLeft)();
/*N*/ 	const SwFrm* pTmp = this;
/*N*/ 	BOOL bLeft = TRUE;
/*N*/ 	BOOL bRight = TRUE;
/*N*/ 	while( pTmp )
/*N*/ 	{
/*N*/         long nTmpRight = (pTmp->Frm().*fnRect->fnGetRight)();
/*N*/         long nTmpLeft = (pTmp->Frm().*fnRect->fnGetLeft)();
/*N*/ 		ASSERT( pTmp, "PaintArea lost in time and space" );
/*N*/ 		if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() ||
/*N*/ 			pTmp->IsCellFrm() || pTmp->IsRowFrm() || //nobody leaves a table!
/*N*/ 			pTmp->IsRootFrm() )
/*N*/ 		{
/*N*/             if( bLeft || nLeft < nTmpLeft )
/*N*/                 nLeft = nTmpLeft;
/*N*/             if( bRight || nTmpRight < nRight )
/*N*/ 				nRight = nTmpRight;
/*N*/ 			if( pTmp->IsPageFrm() || pTmp->IsFlyFrm() || pTmp->IsRootFrm() )
/*N*/ 				break;
/*N*/ 			bLeft = FALSE;
/*N*/ 			bRight = FALSE;
/*N*/ 		}
/*N*/ 		else if( pTmp->IsColumnFrm() )  // nobody enters neightbour columns
/*N*/ 		{
/*N*/             BOOL bR2L = pTmp->IsRightToLeft();
/*N*/             // the first column has _no_ influence to the left range
/*N*/             if( bR2L ? pTmp->GetNext() : pTmp->GetPrev() )
/*N*/             {
/*N*/                 if( bLeft || nLeft < nTmpLeft )
/*N*/                     nLeft = nTmpLeft;
/*N*/ 				bLeft = FALSE;
/*N*/ 			}
/*N*/              // the last column has _no_ influence to the right range
/*N*/             if( bR2L ? pTmp->GetPrev() : pTmp->GetNext() )
/*N*/             {
/*N*/ 				if( bRight || nTmpRight < nRight )
/*N*/                     nRight = nTmpRight;
/*N*/ 				bRight = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/         else if( bVert && pTmp->IsBodyFrm() )
/*N*/         {
/*?*/             // Header and footer frames have always horizontal direction and
/*?*/             // limit the body frame.
/*?*/             // A previous frame of a body frame must be a header,
/*?*/             // the next frame of a body frame may be a footnotecontainer or
/*?*/             // a footer. The footnotecontainer has the same direction like
/*?*/             // the body frame.
/*?*/             if( pTmp->GetPrev() && ( bLeft || nLeft < nTmpLeft ) )
/*?*/             {
/*?*/                 nLeft = nTmpLeft;
/*?*/                 bLeft = FALSE;
/*?*/             }
/*?*/             if( pTmp->GetNext() &&
/*?*/                 ( pTmp->GetNext()->IsFooterFrm() || pTmp->GetNext()->GetNext() )
/*?*/                 && ( bRight || nTmpRight < nRight ) )
/*?*/             {
/*?*/                 nRight = nTmpRight;
/*?*/                 bRight = FALSE;
/*?*/ 			}
/*N*/         }
/*N*/ 		pTmp = pTmp->GetUpper();
/*N*/ 	}
/*N*/     (aRect.*fnRect->fnSetLeft)( nLeft );
/*N*/     (aRect.*fnRect->fnSetRight)( nRight );
/*N*/ 	return aRect;
/*N*/ }

/*************************************************************************
|*
|*	SwFrm::UnionFrm()
|*
|*	Created		AMA 08/22/2000
|*	Last change	AMA 08/23/2000
|*
|*  The unionframe is the framearea (Frm()) of a frame expanded by the
|*  printarea, if there's a negative margin at the left or right side.
|*
|*************************************************************************/

/*N*/ const SwRect SwFrm::UnionFrm( BOOL bBorder ) const
/*N*/ {
/*N*/     BOOL bVert = IsVertical();
/*N*/     SwRectFn fnRect = bVert ? fnRectVert : fnRectHori;
/*N*/     long nLeft = (Frm().*fnRect->fnGetLeft)();
/*N*/     long nWidth = (Frm().*fnRect->fnGetWidth)();
/*N*/     long nPrtLeft = (Prt().*fnRect->fnGetLeft)();
/*N*/     long nPrtWidth = (Prt().*fnRect->fnGetWidth)();
/*N*/     if( nPrtLeft + nPrtWidth > nWidth )
/*?*/         nWidth = nPrtLeft + nPrtWidth;
/*N*/     if( nPrtLeft < 0 )
/*N*/     {
/*N*/         nLeft += nPrtLeft;
/*N*/         nWidth -= nPrtLeft;
/*N*/     }
/*N*/     SwTwips nRight = nLeft + nWidth;
/*N*/     long nAdd = 0;
/*N*/ 	if( bBorder )
/*N*/ 	{
/*N*/ 		SwBorderAttrAccess aAccess( SwFrm::GetCache(), this );
/*N*/ 		const SwBorderAttrs &rAttrs = *aAccess.Get();
/*N*/ 		const SvxBoxItem &rBox = rAttrs.GetBox();
/*N*/ 		if ( rBox.GetLeft() )
/*N*/             nLeft -= rBox.CalcLineSpace( BOX_LINE_LEFT );
/*N*/ 		else if ( rAttrs.IsBorderDist() )
/*?*/             nLeft -= rBox.GetDistance( BOX_LINE_LEFT ) + 1;
/*N*/ 		if ( rBox.GetRight() )
/*N*/             nAdd += rBox.CalcLineSpace( BOX_LINE_RIGHT );
/*N*/ 		else if ( rAttrs.IsBorderDist() )
/*?*/             nAdd += rBox.GetDistance( BOX_LINE_RIGHT ) + 1;
/*N*/ 		if( rAttrs.GetShadow().GetLocation() != SVX_SHADOW_NONE )
/*N*/ 		{
/*N*/ 			const SvxShadowItem &rShadow = rAttrs.GetShadow();
/*N*/             nLeft -= rShadow.CalcShadowSpace( SHADOW_LEFT );
/*N*/             nAdd += rShadow.CalcShadowSpace( SHADOW_RIGHT );
/*N*/ 		}
/*N*/ 	}
/*N*/     if( IsTxtFrm() && ((SwTxtFrm*)this)->HasPara() )
/*N*/     {
/*N*/         long nTmp = ((SwTxtFrm*)this)->HangingMargin();
/*N*/         if( nTmp > nAdd )
/*?*/             nAdd = nTmp;
/*N*/     }
/*N*/     nWidth = nRight + nAdd - nLeft;
/*N*/ 	SwRect aRet( Frm() );
/*N*/     (aRet.*fnRect->fnSetPosX)( nLeft );
/*N*/     (aRet.*fnRect->fnSetWidth)( nWidth );
/*N*/ 	return aRet;
/*N*/ }




}
