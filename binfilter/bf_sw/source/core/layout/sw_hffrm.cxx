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

#include "pagefrm.hxx"
#include "viewsh.hxx"

#include <horiornt.hxx>

#include "doc.hxx"

#include <fmtcntnt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include "hffrm.hxx"
#include "txtfrm.hxx"
#include "sectfrm.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "frmfmt.hxx"
#include "frmsh.hxx"
#include "ndindex.hxx"
#include "hfspacingitem.hxx"
namespace binfilter {

/*N*/ extern FASTBOOL bObjsDirect;	//frmtool.cxx

/*M*/ static SwTwips lcl_GetFrmMinHeight(const SwLayoutFrm & rFrm)
/*M*/ {
/*M*/     const SwFmtFrmSize &rSz = rFrm.GetFmt()->GetFrmSize();
/*M*/     SwTwips nMinHeight;
/*M*/     
/*M*/     switch (rSz.GetSizeType())
/*M*/     { 
/*M*/     case ATT_MIN_SIZE:
/*M*/         nMinHeight = rSz.GetHeight();
/*M*/         
/*M*/         break;
/*M*/         
/*M*/     default: 
/*M*/         nMinHeight = 0;
/*M*/     }
/*M*/     
/*M*/ 
/*M*/     return nMinHeight;
/*M*/ }


/*M*/ static SwTwips lcl_CalcContentHeight(SwLayoutFrm & frm)
/*M*/ {
/*M*/     SwFrm* pFrm = frm.Lower();
/*M*/ 
/*M*/     SwTwips nRemaining = 0;
/*M*/     USHORT nNum = 0;
/*M*/     pFrm = frm.Lower();
/*M*/     while ( pFrm )
/*M*/     {
/*M*/         SwTwips nTmp;
/*M*/         
/*M*/         nTmp = pFrm->Frm().Height();
/*M*/         nRemaining += nTmp;
/*M*/         if( pFrm->IsTxtFrm() && ((SwTxtFrm*)pFrm)->IsUndersized() )
/*M*/         {
/*M*/             nTmp = ((SwTxtFrm*)pFrm)->GetParHeight()
/*M*/                 - pFrm->Prt().Height();
/*M*/             // Dieser TxtFrm waere gern ein bisschen groesser
/*M*/             nRemaining += nTmp;
/*M*/         }
/*M*/         else if( pFrm->IsSctFrm() && ((SwSectionFrm*)pFrm)->IsUndersized() )
/*M*/         {
/*M*/             nTmp = ((SwSectionFrm*)pFrm)->Undersize();
/*M*/             nRemaining += nTmp;
/*M*/         }
/*M*/         pFrm = pFrm->GetNext();
/*M*/ 
/*M*/         nNum++;
/*M*/     }
/*M*/     
/*M*/     return nRemaining;
/*M*/ }

/*M*/ static void lcl_LayoutFrmEnsureMinHeight(SwLayoutFrm & rFrm, 
/*M*/                                          const SwBorderAttrs * pAttrs)
/*M*/ {
/*M*/     SwTwips nMinHeight = lcl_GetFrmMinHeight(rFrm);
/*M*/ 
/*M*/     if (rFrm.Frm().Height() < nMinHeight)
/*M*/     {
/*M*/         rFrm.Grow(nMinHeight - rFrm.Frm().Height());
/*M*/     }
/*M*/ }

/*M*/ SwHeadFootFrm::SwHeadFootFrm( SwFrmFmt * pFmt, USHORT nTypeIn) 
/*M*/     : SwLayoutFrm(pFmt)
/*M*/ {
/*M*/     nType = nTypeIn;
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/     SetDerivedVert( FALSE );
/*M*/ #endif
/*M*/ 
/*M*/ 	const SwFmtCntnt &rCnt = pFmt->GetCntnt();
/*M*/ 
/*M*/ 	ASSERT( rCnt.GetCntntIdx(), "Kein Inhalt fuer Header." );
/*M*/ 
/*M*/ 	//Fuer Header Footer die Objekte gleich erzeugen lassen.
/*M*/ 	FASTBOOL bOld = bObjsDirect;
/*M*/ 	bObjsDirect = TRUE;
/*M*/ 	ULONG nIndex = rCnt.GetCntntIdx()->GetIndex();
/*M*/ 	::binfilter::_InsertCnt( this, pFmt->GetDoc(), ++nIndex );
/*M*/ 	bObjsDirect = bOld;
/*M*/ }

/*M*/ void SwHeadFootFrm::FormatPrt(SwTwips & nUL, const SwBorderAttrs * pAttrs)
/*M*/ {
/*M*/     if (GetEatSpacing())
/*M*/     {
        /* The minimal height of the print area is the minimal height of the
           frame without the height needed for borders and shadow. */
/*M*/         SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
/*M*/ 
/*M*/         nMinHeight -= pAttrs->CalcTop();
/*M*/         nMinHeight -= pAttrs->CalcBottom();
/*M*/         
        /* If the minimal height of the print area is negative, try to
           compensate by overlapping */
/*M*/         SwTwips nOverlap = 0;
/*M*/         if (nMinHeight < 0)
/*M*/         {
/*M*/             nOverlap = -nMinHeight;
/*M*/             nMinHeight = 0;        
/*M*/         }    
/*M*/         
        /* Calculate desired height of content. The minimal height has to be
           adhered. */
/*M*/         SwTwips nHeight;
/*M*/         
/*M*/         if ( ! HasFixSize() )
/*M*/             nHeight = lcl_CalcContentHeight(*this);
/*M*/         else
/*M*/             nHeight = nMinHeight;
/*M*/         
/*M*/         if (nHeight < nMinHeight)
/*M*/             nHeight = nMinHeight;
/*M*/         
/*M*/         /* calculate initial spacing/line space */
/*M*/         SwTwips nSpace, nLine;
/*M*/ 
/*M*/         if (IsHeaderFrm())
/*M*/         {
/*M*/             nSpace = pAttrs->CalcBottom();
/*M*/             nLine = pAttrs->CalcBottomLine();
/*M*/         }
/*M*/         else
/*M*/         {
/*M*/             nSpace = pAttrs->CalcTop();
/*M*/             nLine = pAttrs->CalcTopLine();
/*M*/         }
/*M*/         
/*M*/         /* calculate overlap and correct spacing */
/*M*/         nOverlap += nHeight - nMinHeight;
/*M*/         if (nOverlap < nSpace - nLine)
/*M*/             nSpace -= nOverlap;
/*M*/         else
/*M*/             nSpace = nLine;
/*M*/ 
/*M*/         /* calculate real vertical space between frame and print area */
/*M*/         if (IsHeaderFrm())
/*M*/             nUL = pAttrs->CalcTop() + nSpace;
/*M*/         else
/*M*/             nUL = pAttrs->CalcBottom() + nSpace;
/*M*/ 
/*M*/         /* set print area */
/*N*/         // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
/*N*/         SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );
/*M*/     
/*M*/         aPrt.Left(pAttrs->CalcLeft(this));
/*M*/ 
/*M*/         if (IsHeaderFrm())
/*M*/             aPrt.Top(pAttrs->CalcTop());
/*M*/         else
/*M*/             aPrt.Top(nSpace);
/*M*/ 
/*M*/         aPrt.Width(aFrm.Width() - nLR);
/*M*/ 
/*M*/         SwTwips nNewHeight;
/*M*/ 
/*M*/         if (nUL < aFrm.Height())
/*M*/             nNewHeight = aFrm.Height() - nUL;
/*M*/         else
/*M*/             nNewHeight = 0;
/*M*/ 
/*M*/         aPrt.Height(nNewHeight);
/*M*/         
/*M*/     }
/*M*/     else
/*M*/     {
/*M*/ 		//Position einstellen.
/*M*/ 		aPrt.Left( pAttrs->CalcLeft( this ) );
/*M*/ 		aPrt.Top ( pAttrs->CalcTop()  );
/*M*/         
/*M*/ 		//Sizes einstellen; die Groesse gibt der umgebende Frm vor, die
/*M*/ 		//die Raender werden einfach abgezogen.
/*N*/         // OD 23.01.2003 #106895# - add first parameter to <SwBorderAttrs::CalcRight(..)>
/*N*/         SwTwips nLR = pAttrs->CalcLeft( this ) + pAttrs->CalcRight( this );
/*M*/ 		aPrt.Width ( aFrm.Width() - nLR );
/*M*/ 		aPrt.Height( aFrm.Height()- nUL );
/*M*/ 
/*M*/     }
/*M*/ 
/*M*/     bValidPrtArea = TRUE;
/*M*/ }

/*M*/ void SwHeadFootFrm::FormatSize(SwTwips nUL, const SwBorderAttrs * pAttrs)
/*M*/ {
/*M*/     if ( !HasFixSize() )
/*M*/     {
/*M*/         if( !IsColLocked() )
/*M*/         {
/*M*/             bValidSize = bValidPrtArea = TRUE;
/*N*/ 
/*M*/             const SwTwips nBorder = nUL;
/*M*/             SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
/*M*/             nMinHeight -= pAttrs->CalcTop();
/*M*/             nMinHeight -= pAttrs->CalcBottom();
/*M*/ 
/*M*/             if (nMinHeight < 0)
/*M*/                 nMinHeight = 0;
/*M*/         
/*M*/             ColLock();
/*M*/ 
/*M*/             SwTwips nMaxHeight = LONG_MAX;
/*M*/             SwTwips nRemaining, nOldHeight;
/*M*/             Point aOldPos;
/*M*/             
/*M*/             do
/*M*/             {
/*M*/                 nOldHeight = Prt().Height();
/*M*/                 SwFrm* pFrm = Lower();
/*M*/                 if( Frm().Pos() != aOldPos && pFrm )
/*M*/                 {
/*M*/                     pFrm->_InvalidatePos();
/*M*/                     aOldPos = Frm().Pos();
/*M*/                 }
/*M*/                 while( pFrm )
/*M*/                 {
/*M*/                     pFrm->Calc();
/*M*/                     pFrm = pFrm->GetNext();
/*M*/                 }
/*M*/                 nRemaining = 0;
/*M*/                 pFrm = Lower();
/*N*/ 
/*M*/                 while ( pFrm )
/*M*/                 {  
/*M*/                     nRemaining += pFrm->Frm().Height();
/*M*/                     
/*M*/                     if( pFrm->IsTxtFrm() && 
/*M*/                         ((SwTxtFrm*)pFrm)->IsUndersized() )
/*M*/                         // Dieser TxtFrm waere gern ein bisschen groesser
/*M*/                         nRemaining += ((SwTxtFrm*)pFrm)->GetParHeight()
/*M*/                             - pFrm->Prt().Height();
/*M*/                     else if( pFrm->IsSctFrm() && 
/*M*/                              ((SwSectionFrm*)pFrm)->IsUndersized() )
/*M*/                         nRemaining += ((SwSectionFrm*)pFrm)->Undersize();
/*M*/                     pFrm = pFrm->GetNext();
/*M*/                 }
/*M*/                 if ( nRemaining < nMinHeight )
/*M*/                     nRemaining = nMinHeight;
/*M*/ 
/*M*/                 SwTwips nDiff = nRemaining - nOldHeight;
/*M*/ 
/*M*/                 if( !nDiff )
/*M*/                     break;
/*M*/                 if( nDiff < 0 )
/*M*/                 {
/*M*/                     nMaxHeight = nOldHeight;
/*N*/ 
/*M*/                     if( nRemaining <= nMinHeight )
/*M*/                         nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
/*M*/                 }
/*M*/                 else
/*M*/                 {
/*M*/                     if (nOldHeight > nMinHeight)
/*M*/                         nMinHeight = nOldHeight;
/*M*/ 
/*M*/                     if( nRemaining >= nMaxHeight )
/*M*/                         nRemaining = ( nMaxHeight + nMinHeight + 1 ) / 2;
/*M*/                 }
/*M*/ 
/*M*/                 nDiff = nRemaining - nOldHeight;
/*M*/ 
/*M*/                 if ( nDiff )
/*M*/                 {
/*M*/                     ColUnlock();
/*M*/                     if ( nDiff > 0 )
/*M*/                     {
/*N*/ 						if ( Grow( nDiff PHEIGHT ) )
/*N*/ 						{
/*M*/                         pFrm = Lower();
/*M*/                         
/*M*/                         while ( pFrm )
/*M*/                         {
/*M*/                             if( pFrm->IsTxtFrm())
/*M*/                             {
/*M*/                                 SwTxtFrm * pTmpFrm = (SwTxtFrm*) pFrm;
/*M*/                                 if (pTmpFrm->IsUndersized() )
/*M*/                                 {                                    
/*M*/                                     pTmpFrm->InvalidateSize();
/*M*/                                     pTmpFrm->Prepare(PREP_ADJUST_FRM);
/*M*/                                 }
/*M*/                             }
                                /* #i3568# Undersized sections need to be
                                   invalidated too. */
/*N*/ 								else if (pFrm->IsSctFrm())
/*N*/ 								{
/*N*/                                     SwSectionFrm * pTmpFrm =
/*N*/ 										(SwSectionFrm*) pFrm;
/*N*/                                     if (pTmpFrm->IsUndersized() )
/*N*/                                     {
/*N*/                                         pTmpFrm->InvalidateSize();
/*N*/                                         pTmpFrm->Prepare(PREP_ADJUST_FRM);
/*N*/                                     }
/*N*/ 								}
/*M*/                             pFrm = pFrm->GetNext();
/*M*/                         }
/*N*/                         }
/*M*/                     }
/*M*/                     else
/*M*/                         Shrink( -nDiff PHEIGHT );
/*M*/                     //Schnell auf dem kurzen Dienstweg die Position updaten.
/*M*/ 
/*M*/                     MakePos();
/*M*/                     ColLock();
/*M*/                 }
/*M*/                 else
/*M*/                     break;
/*M*/                 //Unterkante des Uppers nicht ueberschreiten.
/*M*/                 if ( GetUpper() && Frm().Height() )
/*M*/                 {
/*M*/                     const SwTwips nDeadLine = GetUpper()->Frm().Top() +
/*M*/                         GetUpper()->Prt().Bottom();
/*M*/                     const SwTwips nBot = Frm().Bottom();
/*M*/                     if ( nBot > nDeadLine )
/*M*/                     {
/*M*/                         Frm().Bottom( nDeadLine );
/*M*/                         Prt().SSize().Height() = Frm().Height() - nBorder;
/*M*/                     }
/*M*/                 }
/*M*/                 bValidSize = bValidPrtArea = TRUE;
/*N*/             } while( nRemaining<=nMaxHeight && nOldHeight!=Prt().Height() );
/*M*/             ColUnlock();
/*M*/         }
/*N*/         bValidSize = bValidPrtArea = TRUE;
/*M*/     }
/*M*/     else //if ( GetType() & 0x0018 )
/*M*/     {
/*M*/         do
/*M*/         {	
/*M*/             if ( Frm().Height() != pAttrs->GetSize().Height() )
/*M*/                 ChgSize( Size( Frm().Width(), pAttrs->GetSize().Height()));
/*M*/             bValidSize = TRUE;
/*M*/             MakePos();
/*M*/         } while ( !bValidSize );
/*M*/     }
/*M*/ }

/*M*/ void SwHeadFootFrm::Format(const SwBorderAttrs * pAttrs)
/*M*/ {
/*M*/ 	ASSERT( pAttrs, "SwFooterFrm::Format, pAttrs ist 0." );
/*M*/ 
/*M*/ 	if ( bValidPrtArea && bValidSize )
/*M*/ 		return;
/*M*/ 
/*M*/     if ( ! GetEatSpacing() && IsHeaderFrm())
/*M*/     {
/*M*/         SwLayoutFrm::Format(pAttrs);
/*M*/     }
/*M*/     else
/*M*/     {
/*M*/         lcl_LayoutFrmEnsureMinHeight(*this, pAttrs);
/*M*/ 
/*M*/         long nUL = pAttrs->CalcTop()  + pAttrs->CalcBottom();
/*M*/         
/*M*/         if ( !bValidPrtArea )
/*M*/             FormatPrt(nUL, pAttrs);
/*M*/         
/*M*/         if ( !bValidSize )
/*M*/             FormatSize(nUL, pAttrs);
/*M*/     }
/*M*/ }

/*M*/ SwTwips SwHeadFootFrm::GrowFrm( SwTwips nDist, BOOL bTst,  BOOL bInfo )
/*M*/ {
/*M*/     SwTwips nResult;
/*M*/ 
/*M*/     if ( IsColLocked() )
/*M*/     {
/*M*/         nResult = 0;
/*M*/     }
/*M*/     else if (! GetEatSpacing())
/*M*/     {
/*M*/         nResult = SwLayoutFrm::GrowFrm(nDist, bTst, bInfo);
/*M*/     }
/*M*/     else
/*M*/     {
/*M*/         nResult = 0;
/*M*/ 
/*M*/         SwBorderAttrAccess * pAccess = 
/*M*/             new SwBorderAttrAccess( SwFrm::GetCache(), this );
/*M*/         ASSERT(pAccess, "no border attributes");
/*M*/     
/*M*/         SwBorderAttrs * pAttrs = pAccess->Get();
/*M*/     
        /* First assume the whole amount to grow can be provided by eating 
           spacing. */
/*M*/         SwTwips nEat = nDist;
/*M*/         SwTwips nMaxEat;
/*M*/     
/*M*/         /* calculate maximum eatable spacing */
/*M*/         if (IsHeaderFrm())
/*M*/             nMaxEat = aFrm.Height() - aPrt.Bottom() - pAttrs->CalcBottomLine();
/*M*/         else
/*M*/             nMaxEat = aPrt.Top() - pAttrs->CalcTopLine();
/*M*/     
/*M*/         delete pAccess;
/*M*/     
/*M*/         if (nMaxEat < 0)
/*M*/             nMaxEat = 0;
/*M*/     
        /* If the frame is too small, eat less spacing thus letting the frame
           grow more. */
/*M*/         SwTwips nMinHeight = lcl_GetFrmMinHeight(*this);
/*M*/         SwTwips nFrameTooSmall = nMinHeight - Frm().Height();
/*M*/     
/*M*/         if (nFrameTooSmall > 0)
/*M*/             nEat -= nFrameTooSmall;
/*M*/     
/*M*/         /* No negative eating, not eating more than allowed. */
/*M*/         if (nEat < 0)
/*M*/             nEat = 0;
/*M*/         else if (nEat > nMaxEat)
/*M*/             nEat = nMaxEat;
/*M*/     
/*N*/         // OD 10.04.2003 #108719# - Notify fly frame, if header frame
/*N*/         // grows. Consider, that 'normal' grow of layout frame already notifys
/*N*/         // the fly frames.
/*N*/         sal_Bool bNotifyFlys = sal_False;
/*M*/         if (nEat > 0)
/*M*/         {        
/*M*/             if ( ! bTst)
/*M*/             {
/*M*/                 if (! IsHeaderFrm())
/*M*/                 {
/*M*/                     aPrt.Top(aPrt.Top() - nEat);
/*M*/                     aPrt.Height(aPrt.Height() - nEat);
/*M*/                 }
/*M*/ 
/*M*/                 InvalidateAll();
/*M*/             }
/*M*/         
/*M*/             nResult += nEat;
/*N*/             // OD 14.04.2003 #108719# - trigger fly frame notify.
/*N*/             if ( IsHeaderFrm() )
/*N*/             {
/*N*/                 bNotifyFlys = sal_True;
/*N*/             }
/*M*/         }
/*M*/ 
/*M*/         if (nDist - nEat > 0)
/*M*/         {
/*M*/             SwTwips nFrmGrow = 
/*M*/                 SwLayoutFrm::GrowFrm( nDist - nEat, bTst, bInfo );
/*M*/ 
/*M*/             nResult += nFrmGrow;
/*N*/             if ( nFrmGrow > 0 )
/*N*/             {
/*N*/                 bNotifyFlys = sal_False;
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         // OD 10.04.2003 #108719# - notify fly frames, if necessary and triggered.
/*N*/         if ( ( nResult > 0 ) && bNotifyFlys )
/*N*/         {
/*N*/             NotifyFlys();
/*M*/         }
/*M*/     }
/*M*/     
/*M*/ 	if ( nResult && !bTst )
/*M*/ 		SetCompletePaint();
/*M*/ 
/*M*/     return nResult;
/*M*/ }
 
/*M*/ SwTwips SwHeadFootFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
/*M*/ {
/*M*/     SwTwips nResult;
/*M*/ 
/*M*/     if ( IsColLocked() )
/*M*/     {
/*M*/         nResult = 0;
/*M*/     }
/*M*/     else if (! GetEatSpacing())
/*M*/     {
/*M*/         nResult = SwLayoutFrm::ShrinkFrm(nDist, bTst, bInfo);
/*M*/     }
/*M*/     else
/*M*/     {
/*M*/         nResult = 0;
/*M*/ 
/*M*/         SwTwips nMinHeight = lcl_GetFrmMinHeight(*this); 
/*M*/         SwTwips nOldHeight = Frm().Height();
/*M*/         SwTwips nRest = 0; // Amount to shrink by spitting out spacing
/*M*/ 
/*M*/         if ( nOldHeight >= nMinHeight )
/*M*/         {
            /* If the frame's height is bigger than its minimum height, shrink
               the frame towards its minimum height. If this is not sufficient
               to provide the shrinking requested provide the rest by spitting
               out spacing. */
/*M*/ 
/*M*/             SwTwips nBiggerThanMin = nOldHeight - nMinHeight;
/*M*/ 
/*M*/             if (nBiggerThanMin < nDist)
/*M*/             {
/*M*/                 nRest = nDist - nBiggerThanMin;
/*M*/             }
/*M*/             /* info: declaration of nRest -> else nRest = 0 */
/*M*/         }
/*M*/         else
            /* The frame cannot shrink. Provide shrinking by spitting out
               spacing. */
/*M*/             nRest = nDist;
/*M*/ 
        // OD 10.04.2003 #108719# - Notify fly frame, if header/footer frame
        // shrinks. Consider, that 'normal' shrink of layout frame already notifys
        // the fly frames.
/*N*/         sal_Bool bNotifyFlys = sal_False;
/*M*/         if (nRest > 0)
/*M*/         {
/*M*/             
/*M*/             SwBorderAttrAccess * pAccess = 
/*M*/                 new SwBorderAttrAccess( SwFrm::GetCache(), this );
/*M*/             ASSERT(pAccess, "no border attributes");
/*M*/             
/*M*/             SwBorderAttrs * pAttrs = pAccess->Get();
/*M*/ 
/*M*/             /* minimal height of print area */
/*M*/             SwTwips nMinPrtHeight = nMinHeight 
/*M*/                 - pAttrs->CalcTop()
/*M*/                 - pAttrs->CalcBottom();
/*M*/ 
/*M*/             if (nMinPrtHeight < 0)
/*M*/                 nMinPrtHeight = 0;
/*M*/ 
/*M*/             delete pAccess;
/*M*/ 
/*M*/             /* assume all shrinking can be provided */
/*M*/             SwTwips nShrink = nRest;
/*M*/ 
/*M*/             /* calculate maximum shrinking */
/*M*/             SwTwips nMaxShrink = aPrt.Height() - nMinPrtHeight;
/*M*/ 
/*M*/             /* shrink no more than maximum shrinking */
/*M*/             if (nShrink > nMaxShrink)
/*M*/             {
/*M*/                 //nRest -= nShrink - nMaxShrink;
/*M*/                 nShrink = nMaxShrink;
/*M*/             }
/*M*/ 
/*M*/             if (!bTst)
/*M*/             {
/*M*/                 if (! IsHeaderFrm() )
/*M*/                 {
/*M*/                     aPrt.Top(aPrt.Top() + nShrink);                
/*M*/                     aPrt.Height(aPrt.Height() - nShrink);
/*M*/                 }
/*M*/ 
/*M*/                 InvalidateAll();
/*M*/             }
/*M*/             nResult += nShrink;
/*N*/             // OD 14.04.2003 #108719# - trigger fly frame notify.
/*N*/             if ( IsHeaderFrm() )
/*N*/             {
/*N*/                 bNotifyFlys = sal_True;
/*N*/             }
/*M*/         }
/*M*/         
        /* The shrinking not providable by spitting out spacing has to be done
           by the frame. */
/*M*/         if (nDist - nRest > 0)
/*N*/         {
/*N*/             SwTwips nShrinkAmount = SwLayoutFrm::ShrinkFrm( nDist - nRest, bTst, bInfo );
/*N*/             nResult += nShrinkAmount;
/*N*/             if ( nShrinkAmount > 0 )
/*N*/             {
/*N*/                 bNotifyFlys = sal_False;
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         // OD 10.04.2003 #108719# - notify fly frames, if necessary.
/*N*/         if ( ( nResult > 0 ) && bNotifyFlys )
/*N*/         {
/*N*/             NotifyFlys();
/*N*/         }
/*M*/     }
/*M*/     
/*M*/     return nResult;
/*M*/ }

/*M*/ BOOL SwHeadFootFrm::GetEatSpacing() const
/*M*/ {
/*M*/     const SwFrmFmt * pFmt = GetFmt();
/*M*/     ASSERT(pFmt, "SwHeadFootFrm: no format?");
/*M*/ 
/*M*/     if (pFmt->GetHeaderAndFooterEatSpacing().GetValue())
/*M*/         return TRUE;
/*M*/ 
/*M*/     return FALSE;
/*M*/ }


/*************************************************************************
|*
|*	SwPageFrm::PrepareHeader()
|*
|*	Beschreibung		Erzeugt oder Entfernt Header
|*	Ersterstellung		MA 04. Feb. 93
|*	Letzte Aenderung	MA 12. May. 96
|*
|*************************************************************************/


/*N*/ void DelFlys( SwLayoutFrm *pFrm, SwPageFrm *pPage )
/*N*/ {
/*N*/ 	for ( int i = 0; pPage->GetSortedObjs() &&
/*N*/ 						pPage->GetSortedObjs()->Count() &&
/*N*/ 						i < (int)pPage->GetSortedObjs()->Count(); ++i )
/*N*/ 	{
/*N*/ 		SdrObject *pO = (*pPage->GetSortedObjs())[i];
/*N*/ 		if ( pO->IsWriterFlyFrame() )
/*N*/ 		{
/*N*/ 			SwVirtFlyDrawObj *pObj = (SwVirtFlyDrawObj*)pO;
/*N*/ 			if ( pFrm->IsAnLower( pObj->GetFlyFrm() ) )
/*N*/ 			{
/*?*/ 				delete pObj->GetFlyFrm();
/*?*/ 				--i;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ void SwPageFrm::PrepareHeader()
/*N*/ {
/*N*/ 	SwLayoutFrm *pLay = (SwLayoutFrm*)Lower();
/*N*/ 	if ( !pLay )
/*N*/ 		return;
/*N*/ 
/*N*/ 	const SwFmtHeader &rH = ((SwFrmFmt*)pRegisteredIn)->GetHeader();
/*N*/ 
/*N*/ 	const FASTBOOL bOn = !((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsBrowseMode() ||
/*N*/ 						  ((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsHeadInBrowse();
/*N*/ 
/*N*/ 	if ( bOn && rH.IsActive() )
/*N*/ 	{	//Header einsetzen, vorher entfernen falls vorhanden.
/*N*/ 		ASSERT( rH.GetHeaderFmt(), "FrmFmt fuer Header nicht gefunden." );
/*N*/ 
/*N*/ 		if ( pLay->GetFmt() == (SwFrmFmt*)rH.GetHeaderFmt() )
/*N*/ 			return;	//Der Footer ist bereits der richtige
/*N*/ 
/*N*/ 		if ( pLay->IsHeaderFrm() )
/*N*/ 		{	SwLayoutFrm *pDel = pLay;
/*N*/ 			pLay = (SwLayoutFrm*)pLay->GetNext();
/*N*/ 			::binfilter::DelFlys( pDel, this );
/*N*/ 			pDel->Cut();
/*N*/ 			delete pDel;
/*N*/ 		}
/*N*/ 		ASSERT( pLay, "Wohin mit dem Header?" );
/*N*/ 		SwHeaderFrm *pH = new SwHeaderFrm( (SwFrmFmt*)rH.GetHeaderFmt() );
/*N*/ 		pH->Paste( this, pLay );
/*N*/ 		if ( GetUpper() )
/*N*/ 			::binfilter::RegistFlys( this, pH );
/*N*/ 	}
/*N*/ 	else if ( pLay && pLay->IsHeaderFrm() )
/*N*/ 	{	//Header entfernen falls vorhanden.
/*N*/ 		::binfilter::DelFlys( pLay, this );
/*N*/ 		pLay->Cut();
/*N*/ 		delete pLay;
/*N*/ 	}
/*N*/ }
/*************************************************************************
|*
|*	SwPageFrm::PrepareFooter()
|*
|*	Beschreibung		Erzeugt oder Entfernt Footer
|*	Ersterstellung		MA 04. Feb. 93
|*	Letzte Aenderung	MA 12. May. 96
|*
|*************************************************************************/


/*N*/ void SwPageFrm::PrepareFooter()
/*N*/ {
/*N*/ 	SwLayoutFrm *pLay = (SwLayoutFrm*)Lower();
/*N*/ 	if ( !pLay )
/*?*/ 		return;
/*N*/ 
/*N*/ 	const SwFmtFooter &rF = ((SwFrmFmt*)pRegisteredIn)->GetFooter();
/*N*/ 	while ( pLay->GetNext() )
/*N*/ 		pLay = (SwLayoutFrm*)pLay->GetNext();
/*N*/ 
/*N*/ 	const FASTBOOL bOn = !((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsBrowseMode() ||
/*N*/ 						 ((SwFrmFmt*)pRegisteredIn)->GetDoc()->IsFootInBrowse();
/*N*/ 
/*N*/ 	if ( bOn && rF.IsActive() )
/*N*/ 	{	//Footer einsetzen, vorher entfernen falls vorhanden.
/*N*/ 		ASSERT( rF.GetFooterFmt(), "FrmFmt fuer Footer nicht gefunden." );
/*N*/ 
/*N*/ 		if ( pLay->GetFmt() == (SwFrmFmt*)rF.GetFooterFmt() )
/*?*/ 			return; //Der Footer ist bereits der richtige.
/*N*/ 
/*N*/ 		if ( pLay->IsFooterFrm() )
/*?*/ 		{	::binfilter::DelFlys( pLay, this );
/*?*/ 			pLay->Cut();
/*?*/ 			delete pLay;
/*N*/ 		}
/*N*/ 		SwFooterFrm *pF = new SwFooterFrm( (SwFrmFmt*)rF.GetFooterFmt() );
/*N*/ 		pF->Paste( this );
/*N*/ 		if ( GetUpper() )
/*N*/ 			::binfilter::RegistFlys( this, pF );
/*N*/ 	}
/*N*/ 	else if ( pLay && pLay->IsFooterFrm() )
/*N*/ 	{	//Footer entfernen falls vorhanden.
/*N*/ 		::binfilter::DelFlys( pLay, this );
/*N*/ 		ViewShell *pSh;
/*N*/ 		if ( pLay->GetPrev() && 0 != (pSh = GetShell()) &&
/*N*/ 			 pSh->VisArea().HasArea() )
/*?*/ 			pSh->InvalidateWindows( pSh->VisArea() );
/*N*/ 		pLay->Cut();
/*N*/ 		delete pLay;
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
