/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lspcitem.hxx>

#include <drawdoc.hxx>
#include <fmtornt.hxx>
#include <fmthdft.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include <docary.hxx>
#include <lineinfo.hxx>
#include <swmodule.hxx>
#include "pagefrm.hxx"
#include "colfrm.hxx"
#include "fesh.hxx"
#include "viewimp.hxx"
#include "viewopt.hxx"
#include "dflyobj.hxx"
#include "dcontact.hxx"
#include "frmtool.hxx"
#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "ftnfrm.hxx"
#include "txtfrm.hxx"
#include "notxtfrm.hxx"
#include "flyfrms.hxx"
#include "layact.hxx"
#include "pagedesc.hxx"
#include "section.hxx"
#include "sectfrm.hxx"
#include "node2lay.hxx"
#include "ndole.hxx"
#include "hints.hxx"
#include <layhelp.hxx>
#include <laycache.hxx>
#include <rootfrm.hxx>
#include <paratr.hxx>
#include <sortedobjs.hxx>
#include <objectformatter.hxx>
#include <calbck.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentTimerAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>

//UUUU
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>

using namespace ::com::sun::star;

bool bObjsDirect = true;
bool bDontCreateObjects = false;
bool bSetCompletePaintOnInvalidate = false;

sal_uInt8 StackHack::nCnt = 0;
bool StackHack::bLocked = false;

SwFrmNotify::SwFrmNotify( SwFrm *pF ) :
    pFrm( pF ),
    aFrm( pF->Frm() ),
    aPrt( pF->Prt() ),
    bInvaKeep( false ),
    bValidSize( pF->GetValidSizeFlag() ),
    mbFrmDeleted( false )     // #i49383#
{
    if ( pF->IsTextFrm() )
    {
        mnFlyAnchorOfst = static_cast<SwTextFrm*>(pF)->GetBaseOfstForFly( true );
        mnFlyAnchorOfstNoWrap = static_cast<SwTextFrm*>(pF)->GetBaseOfstForFly( false );
    }
    else
    {
        mnFlyAnchorOfst = 0;
        mnFlyAnchorOfstNoWrap = 0;
    }

    bHadFollow = pF->IsContentFrm() && static_cast<SwContentFrm*>(pF)->GetFollow();
}

SwFrmNotify::~SwFrmNotify()
{
    // #i49383#
    if ( mbFrmDeleted )
    {
        return;
    }

    SWRECTFN( pFrm )
    const bool bAbsP = POS_DIFF( aFrm, pFrm->Frm() );
    const bool bChgWidth =
            (aFrm.*fnRect->fnGetWidth)() != (pFrm->Frm().*fnRect->fnGetWidth)();
    const bool bChgHeight =
            (aFrm.*fnRect->fnGetHeight)()!=(pFrm->Frm().*fnRect->fnGetHeight)();
    const bool bChgFlyBasePos = pFrm->IsTextFrm() &&
       ( ( mnFlyAnchorOfst != static_cast<SwTextFrm*>(pFrm)->GetBaseOfstForFly( true ) ) ||
         ( mnFlyAnchorOfstNoWrap != static_cast<SwTextFrm*>(pFrm)->GetBaseOfstForFly( false ) ) );

    if ( pFrm->IsFlowFrm() && !pFrm->IsInFootnote() )
    {
        SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );

        if ( !pFlow->IsFollow() )
        {
            if ( !pFrm->GetIndPrev() )
            {
                if ( bInvaKeep )
                {
                    SwFrm *pPre = pFrm->FindPrev();
                    if ( pPre && pPre->IsFlowFrm() )
                    {
                        // 1. pPre wants to keep with me:
                        bool bInvalidPrePos = SwFlowFrm::CastFlowFrm( pPre )->IsKeep( *pPre->GetAttrSet() ) && pPre->GetIndPrev();

                        // 2. pPre is a table and the last row wants to keep with me:
                        if ( !bInvalidPrePos && pPre->IsTabFrm() )
                        {
                            SwTabFrm* pPreTab = static_cast<SwTabFrm*>(pPre);
                            if ( pPreTab->GetFormat()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::TABLE_ROW_KEEP) )
                            {
                                SwRowFrm* pLastRow = static_cast<SwRowFrm*>(pPreTab->GetLastLower());
                                if ( pLastRow && pLastRow->ShouldRowKeepWithNext() )
                                    bInvalidPrePos = true;
                            }
                        }

                        if ( bInvalidPrePos )
                            pPre->InvalidatePos();
                    }
                }
            }
            else if ( !pFlow->HasFollow() )
            {
                long nOldHeight = (aFrm.*fnRect->fnGetHeight)();
                long nNewHeight = (pFrm->Frm().*fnRect->fnGetHeight)();
                if( (nOldHeight > nNewHeight) || (!nOldHeight && nNewHeight) )
                    pFlow->CheckKeep();
            }
        }
    }

    if ( bAbsP )
    {
        pFrm->SetCompletePaint();

        SwFrm* pNxt = pFrm->GetIndNext();
        // #121888# - skip empty section frames
        while ( pNxt &&
                pNxt->IsSctFrm() && !static_cast<SwSectionFrm*>(pNxt)->GetSection() )
        {
            pNxt = pNxt->GetIndNext();
        }

        if ( pNxt )
            pNxt->InvalidatePos();
        else
        {
            // #104100# - correct condition for setting retouche
            // flag for vertical layout.
            if( pFrm->IsRetoucheFrm() &&
                (aFrm.*fnRect->fnTopDist)( (pFrm->Frm().*fnRect->fnGetTop)() ) > 0 )
            {
                pFrm->SetRetouche();
            }

            // A fresh follow frame does not have to be invalidated, because
            // it is already formatted:
            if ( bHadFollow || !pFrm->IsContentFrm() || !static_cast<SwContentFrm*>(pFrm)->GetFollow() )
            {
                if ( !pFrm->IsTabFrm() || !static_cast<SwTabFrm*>(pFrm)->GetFollow() )
                    pFrm->InvalidateNextPos();
            }
        }
    }

    //For each resize of the background graphics is a repaint necessary.
    const bool bPrtWidth =
            (aPrt.*fnRect->fnGetWidth)() != (pFrm->Prt().*fnRect->fnGetWidth)();
    const bool bPrtHeight =
            (aPrt.*fnRect->fnGetHeight)()!=(pFrm->Prt().*fnRect->fnGetHeight)();
    if ( bPrtWidth || bPrtHeight )
    {
        //UUUU
        bool bUseNewFillProperties(false);
        if (pFrm->supportsFullDrawingLayerFillAttributeSet())
        {
            drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFillAttributes(pFrm->getSdrAllFillAttributesHelper());
            if(aFillAttributes.get() && aFillAttributes->isUsed())
            {
                bUseNewFillProperties = true;
                //UUUU use SetCompletePaint if needed
                if(aFillAttributes->needCompleteRepaint())
                {
                    pFrm->SetCompletePaint();
                }
            }
        }
        if (!bUseNewFillProperties)
        {
            const SvxGraphicPosition ePos = pFrm->GetAttrSet()->GetBackground().GetGraphicPos();
            if(GPOS_NONE != ePos && GPOS_TILED != ePos)
                pFrm->SetCompletePaint();
        }
    }
    else
    {
        // #97597# - consider case that *only* margins between
        // frame and printing area has changed. Then, frame has to be repainted,
        // in order to force paint of the margin areas.
        if ( !bAbsP && (bChgWidth || bChgHeight) )
        {
            pFrm->SetCompletePaint();
        }
    }

    const bool bPrtP = POS_DIFF( aPrt, pFrm->Prt() );
    if ( bAbsP || bPrtP || bChgWidth || bChgHeight ||
         bPrtWidth || bPrtHeight || bChgFlyBasePos )
    {
        if( pFrm->IsAccessibleFrm() )
        {
            SwRootFrm *pRootFrm = pFrm->getRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
                pRootFrm->GetCurrShell() )
            {
                pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aFrm );
            }
        }

        // Notification of anchored objects
        if ( pFrm->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pFrm->GetDrawObjs();
            SwPageFrm* pPageFrm = 0;
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                // OD 2004-03-31 #i26791# - no general distinction between
                // Writer fly frames and drawing objects
                bool bNotify = false;
                bool bNotifySize = false;
                SwAnchoredObject* pObj = rObjs[i];
                SwContact* pContact = ::GetUserCall( pObj->GetDrawObj() );
                // #115759#
                const bool bAnchoredAsChar = pContact->ObjAnchoredAsChar();
                if ( !bAnchoredAsChar )
                {
                    // Notify object, which aren't anchored as-character:

                    // always notify objects, if frame position has changed
                    // or if the object is to-page|to-fly anchored.
                    if ( bAbsP ||
                         pContact->ObjAnchoredAtPage() ||
                         pContact->ObjAnchoredAtFly() )
                    {
                        bNotify = true;

                        // assure that to-fly anchored Writer fly frames are
                        // registered at the correct page frame, if frame
                        // position has changed.
                        if ( bAbsP && pContact->ObjAnchoredAtFly() &&
                             pObj->ISA(SwFlyFrm) )
                        {
                            // determine to-fly anchored Writer fly frame
                            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
                            // determine page frame of to-fly anchored
                            // Writer fly frame
                            SwPageFrm* pFlyPageFrm = pFlyFrm->FindPageFrm();
                            // determine page frame, if needed.
                            if ( !pPageFrm )
                            {
                                pPageFrm = pFrm->FindPageFrm();
                            }
                            if ( pPageFrm != pFlyPageFrm )
                            {
                                OSL_ENSURE( pFlyPageFrm, "~SwFrmNotify: Fly from Nowhere" );
                                if( pFlyPageFrm )
                                    pFlyPageFrm->MoveFly( pFlyFrm, pPageFrm );
                                else
                                    pPageFrm->AppendFlyToPage( pFlyFrm );
                            }
                        }
                    }
                    // otherwise the objects are notified in dependence to
                    // its positioning and alignment
                    else
                    {
                        const SwFormatVertOrient& rVert =
                                        pContact->GetFormat()->GetVertOrient();
                        if ( ( rVert.GetVertOrient() == text::VertOrientation::CENTER ||
                               rVert.GetVertOrient() == text::VertOrientation::BOTTOM ||
                               rVert.GetRelationOrient() == text::RelOrientation::PRINT_AREA ) &&
                             ( bChgHeight || bPrtHeight ) )
                        {
                            bNotify = true;
                        }
                        if ( !bNotify )
                        {
                            const SwFormatHoriOrient& rHori =
                                        pContact->GetFormat()->GetHoriOrient();
                            if ( ( rHori.GetHoriOrient() != text::HoriOrientation::NONE ||
                                   rHori.GetRelationOrient()== text::RelOrientation::PRINT_AREA ||
                                   rHori.GetRelationOrient()== text::RelOrientation::FRAME ) &&
                                 ( bChgWidth || bPrtWidth || bChgFlyBasePos ) )
                            {
                                bNotify = true;
                            }
                        }
                    }
                }
                else if ( bPrtWidth )
                {
                    // Notify as-character anchored objects, if printing area
                    // width has changed.
                    bNotify = true;
                    bNotifySize = true;
                }

                // perform notification via the corresponding invalidations
                if ( bNotify )
                {
                    if ( pObj->ISA(SwFlyFrm) )
                    {
                        SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
                        if ( bNotifySize )
                            pFlyFrm->_InvalidateSize();
                        // #115759# - no invalidation of
                        // position for as-character anchored objects.
                        if ( !bAnchoredAsChar )
                        {
                            pFlyFrm->_InvalidatePos();
                        }
                        pFlyFrm->_Invalidate();
                    }
                    else if ( pObj->ISA(SwAnchoredDrawObject) )
                    {
                        // #115759# - no invalidation of
                        // position for as-character anchored objects.
                        if ( !bAnchoredAsChar )
                        {
                            pObj->InvalidateObjPos();
                        }
                    }
                    else
                    {
                        OSL_FAIL( "<SwContentNotify::~SwContentNotify()> - unknown anchored object type." );
                    }
                }
            }
        }
    }
    else if( pFrm->IsTextFrm() && bValidSize != pFrm->GetValidSizeFlag() )
    {
        SwRootFrm *pRootFrm = pFrm->getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->InvalidateAccessibleFrmContent( pFrm );
        }
    }

    // #i9046# Automatic frame width
    SwFlyFrm* pFly = 0;
    // #i35879# Do not trust the inf flags. pFrm does not
    // necessarily have to have an upper!
    if ( !pFrm->IsFlyFrm() && 0 != ( pFly = pFrm->ImplFindFlyFrm() ) )
    {
        // #i61999#
        // no invalidation of columned Writer fly frames, because automatic
        // width doesn't make sense for such Writer fly frames.
        if ( pFly->Lower() && !pFly->Lower()->IsColumnFrm() )
        {
            const SwFormatFrmSize &rFrmSz = pFly->GetFormat()->GetFrmSize();

            // This could be optimized. Basically the fly frame only has to
            // be invalidated, if the first line of pFrm (if pFrm is a content
            // frame, for other frame types its the print area) has changed its
            // size and pFrm was responsible for the current width of pFly. On
            // the other hand, this is only rarely used and re-calculation of
            // the fly frame does not cause too much trouble. So we keep it this
            // way:
            if ( ATT_FIX_SIZE != rFrmSz.GetWidthSizeType() )
            {
                // #i50668#, #i50998# - invalidation of position
                // of as-character anchored fly frames not needed and can cause
                // layout loops
                if ( !pFly->ISA(SwFlyInCntFrm) )
                {
                    pFly->InvalidatePos();
                }
                pFly->InvalidateSize();
            }
        }
    }
}

SwLayNotify::SwLayNotify( SwLayoutFrm *pLayFrm ) :
    SwFrmNotify( pLayFrm ),
    bLowersComplete( false )
{
}

// OD 2004-05-11 #i28701# - local method to invalidate the position of all
// frames inclusive its floating screen objects, which are lowers of the given
// layout frame
static void lcl_InvalidatePosOfLowers( SwLayoutFrm& _rLayoutFrm )
{
    if( _rLayoutFrm.IsFlyFrm() && _rLayoutFrm.GetDrawObjs() )
    {
        _rLayoutFrm.InvalidateObjs( true, false );
    }

    SwFrm* pLowerFrm = _rLayoutFrm.Lower();
    while ( pLowerFrm )
    {
        pLowerFrm->InvalidatePos();
        if ( pLowerFrm->IsTextFrm() )
        {
            static_cast<SwTextFrm*>(pLowerFrm)->Prepare( PREP_POS_CHGD );
        }
        else if ( pLowerFrm->IsTabFrm() )
        {
            pLowerFrm->InvalidatePrt();
        }

        pLowerFrm->InvalidateObjs( true, false );

        pLowerFrm = pLowerFrm->GetNext();
    }
}

SwLayNotify::~SwLayNotify()
{
    // #i49383#
    if ( mbFrmDeleted )
    {
        return;
    }

    SwLayoutFrm *pLay = GetLay();
    SWRECTFN( pLay )
    bool bNotify = false;
    if ( pLay->Prt().SSize() != aPrt.SSize() )
    {
        if ( !IsLowersComplete() )
        {
            bool bInvaPercent;

            if ( pLay->IsRowFrm() )
            {
                bInvaPercent = true;
                long nNew = (pLay->Prt().*fnRect->fnGetHeight)();
                if( nNew != (aPrt.*fnRect->fnGetHeight)() )
                     static_cast<SwRowFrm*>(pLay)->AdjustCells( nNew, true);
                if( (pLay->Prt().*fnRect->fnGetWidth)()
                    != (aPrt.*fnRect->fnGetWidth)() )
                     static_cast<SwRowFrm*>(pLay)->AdjustCells( 0, false );
            }
            else
            {
                //Proportional adoption of the internal.
                //1. If the formatted is no Fly
                //2. If he contains no columns
                //3. If the Fly has a fixed hight and the columns
                //   are next to be.
                //   Hoehe danebenliegen.
                //4. Never at SectionFrms.
                bool bLow;
                if( pLay->IsFlyFrm() )
                {
                    if ( pLay->Lower() )
                    {
                        bLow = !pLay->Lower()->IsColumnFrm() ||
                            (pLay->Lower()->Frm().*fnRect->fnGetHeight)()
                             != (pLay->Prt().*fnRect->fnGetHeight)();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsSctFrm() )
                {
                    if ( pLay->Lower() )
                    {
                        if( pLay->Lower()->IsColumnFrm() && pLay->Lower()->GetNext() )
                            bLow = pLay->Lower()->Frm().Height() != pLay->Prt().Height();
                        else
                            bLow = pLay->Prt().Width() != aPrt.Width();
                    }
                    else
                        bLow = false;
                }
                else if( pLay->IsFooterFrm() && !pLay->HasFixSize() )
                    bLow = pLay->Prt().Width() != aPrt.Width();
                else
                    bLow = true;
                bInvaPercent = bLow;
                if ( bLow )
                {
                    pLay->ChgLowersProp( aPrt.SSize() );
                }
                // If the PrtArea has been extended, it might be possible that the chain of parts
                // can take another frame. As a result, the "possible right one" needs to be
                // invalidated. This only pays off if this or its Uppers are moveable sections.
                // A PrtArea has been extended if width or height are larger than before.
                if ( (pLay->Prt().Height() > aPrt.Height() ||
                      pLay->Prt().Width()  > aPrt.Width()) &&
                     (pLay->IsMoveable() || pLay->IsFlyFrm()) )
                {
                    SwFrm *pTmpFrm = pLay->Lower();
                    if ( pTmpFrm && pTmpFrm->IsFlowFrm() )
                    {
                        while ( pTmpFrm->GetNext() )
                            pTmpFrm = pTmpFrm->GetNext();
                        pTmpFrm->InvalidateNextPos();
                    }
                }
            }
            bNotify = true;
            //EXPENSIVE!! But how we do it more elegant?
            if( bInvaPercent )
                pLay->InvaPercentLowers( pLay->Prt().Height() - aPrt.Height() );
        }
        if ( pLay->IsTabFrm() )
            //So that _only_ the shadow is drawn while resizing.
            static_cast<SwTabFrm*>(pLay)->SetComplete();
        else
        {
            const SwViewShell *pSh = pLay->getRootFrm()->GetCurrShell();
            if( !( pSh && pSh->GetViewOptions()->getBrowseMode() ) ||
                  !(pLay->GetType() & (FRM_BODY | FRM_PAGE)) )
            //Thereby the subordinates are retouched clean.
            //Example problem: Take the Flys with the handles and downsize.
            //Not for body and page, otherwise it flickers when loading HTML.
            pLay->SetCompletePaint();
        }
    }
    //Notify Lower if the position has changed.
    const bool bPrtPos = POS_DIFF( aPrt, pLay->Prt() );
    const bool bPos = bPrtPos || POS_DIFF( aFrm, pLay->Frm() );
    const bool bSize = pLay->Frm().SSize() != aFrm.SSize();

    if ( bPos && pLay->Lower() && !IsLowersComplete() )
        pLay->Lower()->InvalidatePos();

    if ( bPrtPos )
        pLay->SetCompletePaint();

    //Inform the Follower if the SSize has changed.
    if ( bSize )
    {
        if( pLay->GetNext() )
        {
            if ( pLay->GetNext()->IsLayoutFrm() )
                pLay->GetNext()->_InvalidatePos();
            else
                pLay->GetNext()->InvalidatePos();
        }
        else if( pLay->IsSctFrm() )
            pLay->InvalidateNextPos();
    }
    if ( !IsLowersComplete() &&
         !(pLay->GetType()&(FRM_FLY|FRM_SECTION) &&
            pLay->Lower() && pLay->Lower()->IsColumnFrm()) &&
         (bPos || bNotify) &&
         !(pLay->GetType() & (FRM_ROW|FRM_TAB|FRM_FTNCONT|FRM_PAGE|FRM_ROOT)))
    {
        // #i44016# - force unlock of position of lower objects.
        // #i43913# - no unlock of position of objects,
        // if <pLay> is a cell frame, and its table frame resp. its parent table
        // frame is locked.
        // #i47458# - force unlock of position of lower objects,
        // only if position of layout frame has changed.
        bool bUnlockPosOfObjs( bPos );
        if ( bUnlockPosOfObjs && pLay->IsCellFrm() )
        {
            SwTabFrm* pTabFrm( pLay->FindTabFrm() );
            if ( pTabFrm &&
                 ( pTabFrm->IsJoinLocked() ||
                   ( pTabFrm->IsFollow() &&
                     pTabFrm->FindMaster()->IsJoinLocked() ) ) )
            {
                bUnlockPosOfObjs = false;
            }
        }
        // #i49383# - check for footnote frame, if unlock
        // of position of lower objects is allowed.
        else if ( bUnlockPosOfObjs && pLay->IsFootnoteFrm() )
        {
            bUnlockPosOfObjs = static_cast<SwFootnoteFrm*>(pLay)->IsUnlockPosOfLowerObjs();
        }
        // #i51303# - no unlock of object positions for sections
        else if ( bUnlockPosOfObjs && pLay->IsSctFrm() )
        {
            bUnlockPosOfObjs = false;
        }
        pLay->NotifyLowerObjs( bUnlockPosOfObjs );
    }
    if ( bPos && pLay->IsFootnoteFrm() && pLay->Lower() )
    {
        // OD 2004-05-11 #i28701#
        ::lcl_InvalidatePosOfLowers( *pLay );
    }
    if( ( bPos || bSize ) && pLay->IsFlyFrm() && static_cast<SwFlyFrm*>(pLay)->GetAnchorFrm()
          && static_cast<SwFlyFrm*>(pLay)->GetAnchorFrm()->IsFlyFrm() )
        static_cast<SwFlyFrm*>(pLay)->AnchorFrm()->InvalidateSize();
}

SwFlyNotify::SwFlyNotify( SwFlyFrm *pFlyFrm ) :
    SwLayNotify( pFlyFrm ),
    // #115759# - keep correct page frame - the page frame
    // the Writer fly frame is currently registered at.
    pOldPage( pFlyFrm->GetPageFrm() ),
    aFrmAndSpace( pFlyFrm->GetObjRectWithSpaces() )
{
}

SwFlyNotify::~SwFlyNotify()
{
    // #i49383#
    if ( mbFrmDeleted )
    {
        return;
    }

    SwFlyFrm *pFly = GetFly();
    if ( pFly->IsNotifyBack() )
    {
        SwViewShell *pSh = pFly->getRootFrm()->GetCurrShell();
        SwViewShellImp *pImp = pSh ? pSh->Imp() : 0;
        if ( !pImp || !pImp->IsAction() || !pImp->GetLayAction().IsAgain() )
        {
            //If in the LayAction the IsAgain is set it can be
            //that the old page is destroyed in the meantime!
            ::Notify( pFly, pOldPage, aFrmAndSpace, &aPrt );
            // #i35640# - additional notify anchor text frame,
            // if Writer fly frame has changed its page
            if ( pFly->GetAnchorFrm()->IsTextFrm() &&
                 pFly->GetPageFrm() != pOldPage )
            {
                pFly->AnchorFrm()->Prepare( PREP_FLY_LEAVE );
            }
        }
        pFly->ResetNotifyBack();
    }

    //Have the size or the position changed,
    //so should the view know this.
    SWRECTFN( pFly )
    const bool bPosChgd = POS_DIFF( aFrm, pFly->Frm() );
    const bool bFrmChgd = pFly->Frm().SSize() != aFrm.SSize();
    const bool bPrtChgd = aPrt != pFly->Prt();
    if ( bPosChgd || bFrmChgd || bPrtChgd )
    {
        pFly->NotifyDrawObj();
    }
    if ( bPosChgd && aFrm.Pos().X() != FAR_AWAY )
    {
        // OD 2004-05-10 #i28701# - no direct move of lower Writer fly frames.
        // reason: New positioning and alignment (e.g. to-paragraph anchored,
        // but aligned at page) are introduced.
        // <SwLayNotify::~SwLayNotify()> takes care of invalidation of lower
        // floating screen objects by calling method <SwLayoutFrm::NotifyLowerObjs()>.

        if ( pFly->IsFlyAtCntFrm() )
        {
            SwFrm *pNxt = pFly->AnchorFrm()->FindNext();
            if ( pNxt )
            {
                pNxt->InvalidatePos();
            }
        }

        // #i26945# - notify anchor.
        // Needed for negative positioned Writer fly frames
        if ( pFly->GetAnchorFrm()->IsTextFrm() )
        {
            pFly->AnchorFrm()->Prepare( PREP_FLY_LEAVE );
        }
    }

    // OD 2004-05-13 #i28701#
    // #i45180# - no adjustment of layout process flags and
    // further notifications/invalidations, if format is called by grow/shrink
    if ( pFly->ConsiderObjWrapInfluenceOnObjPos() &&
         ( !pFly->ISA(SwFlyFreeFrm) ||
           !static_cast<SwFlyFreeFrm*>(pFly)->IsNoMoveOnCheckClip() ) )
    {
        // #i54138# - suppress restart of the layout process
        // on changed frame height.
        // Note: It doesn't seem to be necessary and can cause layout loops.
        if ( bPosChgd )
        {
            // indicate a restart of the layout process
            pFly->SetRestartLayoutProcess( true );
        }
        else
        {
            // lock position
            pFly->LockPosition();

            if ( !pFly->ConsiderForTextWrap() )
            {
                // indicate that object has to be considered for text wrap
                pFly->SetConsiderForTextWrap( true );
                // invalidate 'background' in order to allow its 'background'
                // to wrap around it.
                pFly->NotifyBackground( pFly->GetPageFrm(),
                                        pFly->GetObjRectWithSpaces(),
                                        PREP_FLY_ARRIVE );
                // invalidate position of anchor frame in order to force
                // a re-format of the anchor frame, which also causes a
                // re-format of the invalid previous frames of the anchor frame.
                pFly->AnchorFrm()->InvalidatePos();
            }
        }
    }
}

SwContentFrm *SwContentNotify::GetCnt()
{
    return static_cast<SwContentFrm*>(pFrm);
}

SwContentNotify::SwContentNotify( SwContentFrm *pContentFrm ) :
    SwFrmNotify( pContentFrm ),
    // OD 08.01.2004 #i11859#
    mbChkHeightOfLastLine( false ),
    mnHeightOfLastLine( 0L ),
    // OD 2004-02-26 #i25029#
    mbInvalidatePrevPrtArea( false ),
    mbBordersJoinedWithPrev( false )
{
    // OD 08.01.2004 #i11859#
    if ( pContentFrm->IsTextFrm() )
    {
        SwTextFrm* pTextFrm = static_cast<SwTextFrm*>(pContentFrm);
        if ( !pTextFrm->GetTextNode()->getIDocumentSettingAccess()->get(DocumentSettingId::OLD_LINE_SPACING) )
        {
            const SwAttrSet* pSet = pTextFrm->GetAttrSet();
            const SvxLineSpacingItem &rSpace = pSet->GetLineSpacing();
            if ( rSpace.GetInterLineSpaceRule() == SVX_INTER_LINE_SPACE_PROP )
            {
                mbChkHeightOfLastLine = true;
                mnHeightOfLastLine = pTextFrm->GetHeightOfLastLine();
            }
        }
    }
}

SwContentNotify::~SwContentNotify()
{
    // #i49383#
    if ( mbFrmDeleted )
    {
        return;
    }

    SwContentFrm *pCnt = GetCnt();
    if ( bSetCompletePaintOnInvalidate )
        pCnt->SetCompletePaint();

    SWRECTFN( pCnt )
    if ( pCnt->IsInTab() && ( POS_DIFF( pCnt->Frm(), aFrm ) ||
                             pCnt->Frm().SSize() != aFrm.SSize()))
    {
        SwLayoutFrm* pCell = pCnt->GetUpper();
        while( !pCell->IsCellFrm() && pCell->GetUpper() )
            pCell = pCell->GetUpper();
        OSL_ENSURE( pCell->IsCellFrm(), "Where's my cell?" );
        if ( text::VertOrientation::NONE != pCell->GetFormat()->GetVertOrient().GetVertOrient() )
            pCell->InvalidatePrt(); //for the vertical align.
    }

    // OD 2004-02-26 #i25029#
    if ( mbInvalidatePrevPrtArea && mbBordersJoinedWithPrev &&
         pCnt->IsTextFrm() &&
         !pCnt->IsFollow() && !pCnt->GetIndPrev() )
    {
        // determine previous frame
        SwFrm* pPrevFrm = pCnt->FindPrev();
        // skip empty section frames and hidden text frames
        {
            while ( pPrevFrm &&
                    ( ( pPrevFrm->IsSctFrm() &&
                        !static_cast<SwSectionFrm*>(pPrevFrm)->GetSection() ) ||
                      ( pPrevFrm->IsTextFrm() &&
                        static_cast<SwTextFrm*>(pPrevFrm)->IsHiddenNow() ) ) )
            {
                pPrevFrm = pPrevFrm->FindPrev();
            }
        }

        // Invalidate printing area of found previous frame
        if ( pPrevFrm )
        {
            if ( pPrevFrm->IsSctFrm() )
            {
                if ( pCnt->IsInSct() )
                {
                    // Note: found previous frame is a section frame and
                    //       <pCnt> is also inside a section.
                    //       Thus due to <mbBordersJoinedWithPrev>,
                    //       <pCnt> had joined its borders/shadow with the
                    //       last content of the found section.
                    // Invalidate printing area of last content in found section.
                    SwFrm* pLstContentOfSctFrm =
                            static_cast<SwSectionFrm*>(pPrevFrm)->FindLastContent();
                    if ( pLstContentOfSctFrm )
                    {
                        pLstContentOfSctFrm->InvalidatePrt();
                    }
                }
            }
            else
            {
                pPrevFrm->InvalidatePrt();
            }
        }
    }

    const bool bFirst = (aFrm.*fnRect->fnGetWidth)() == 0;

    if ( pCnt->IsNoTextFrm() )
    {
        //Active PlugIn's or OLE-Objects should know something of the change
        //thereby they move their window appropriate.
        SwViewShell *pSh  = pCnt->getRootFrm()->GetCurrShell();
        if ( pSh )
        {
            SwOLENode *pNd;
            if ( 0 != (pNd = pCnt->GetNode()->GetOLENode()) &&
                 (pNd->GetOLEObj().IsOleRef() ||
                  pNd->IsOLESizeInvalid()) )
            {
                const bool bNoTextFrmPrtAreaChanged =
                        ( aPrt.SSize().Width() != 0 &&
                          aPrt.SSize().Height() != 0 ) &&
                        aPrt.SSize() != pCnt->Prt().SSize();
                OSL_ENSURE( pCnt->IsInFly(), "OLE not in FlyFrm" );
                SwFlyFrm *pFly = pCnt->FindFlyFrm();
                svt::EmbeddedObjectRef& xObj = pNd->GetOLEObj().GetObject();
                SwFEShell *pFESh = 0;
                for(SwViewShell& rCurrentShell : pSh->GetRingContainer())
                {   if ( rCurrentShell.ISA( SwCrsrShell ) )
                    {
                        pFESh = static_cast<SwFEShell*>(&rCurrentShell);
                        // #108369#: Here used to be the condition if (!bFirst).
                        // I think this should mean "do not call CalcAndSetScale"
                        // if the frame is formatted for the first time.
                        // Unfortunately this is not valid anymore since the
                        // SwNoTextFrm already gets a width during CalcLowerPreps.
                        // Nevertheless, the indention of !bFirst seemed to be
                        // to assure that the OLE objects have already been notified
                        // if necessary before calling CalcAndSetScale.
                        // So I replaced !bFirst by !IsOLESizeInvalid. There is
                        // one additional problem specific to the word import:
                        // The layout is calculated _before_ calling PrtOLENotify,
                        // and the OLE objects are not invalidated during import.
                        // Therefore I added the condition !IsUpdateExpField,
                        // have a look at the occurrence of CalcLayout in
                        // uiview/view.cxx.
                        if ( !pNd->IsOLESizeInvalid() &&
                             !pSh->GetDoc()->getIDocumentState().IsUpdateExpField() )
                            pFESh->CalcAndSetScale( xObj,
                                                    &pFly->Prt(), &pFly->Frm(),
                                                    bNoTextFrmPrtAreaChanged );
                    }
                }

                if ( pFESh && pNd->IsOLESizeInvalid() )
                {
                    pNd->SetOLESizeInvalid( false );
                    pFESh->CalcAndSetScale( xObj ); // create client
                }
            }
            //dito animated graphics
            if ( Frm().HasArea() && static_cast<SwNoTextFrm*>(pCnt)->HasAnimation() )
            {
                static_cast<SwNoTextFrm*>(pCnt)->StopAnimation();
                pSh->InvalidateWindows( Frm() );
            }
        }
    }

    if ( bFirst )
    {
        pCnt->SetRetouche();    //fix(13870)

        SwDoc *pDoc = pCnt->GetNode()->GetDoc();
        if ( !pDoc->GetSpzFrameFormats()->empty() &&
             pDoc->DoesContainAtPageObjWithContentAnchor() && !pDoc->getIDocumentState().IsNewDoc() )
        {
            // If certain import filters for foreign file format import
            // AT_PAGE anchored objects, the corresponding page number is
            // typically not known. In this case the content position is
            // stored at which the anchored object is found in the
            // imported document.
            // When this content is formatted it is the time at which
            // the page is known. Thus, this data can be corrected now.

            const SwPageFrm *pPage = 0;
            SwNodeIndex *pIdx  = 0;
            SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();

            for ( size_t i = 0; i < pTable->size(); ++i )
            {
                SwFrameFormat *pFormat = (*pTable)[i];
                const SwFormatAnchor &rAnch = pFormat->GetAnchor();
                if ( FLY_AT_PAGE != rAnch.GetAnchorId() ||
                     rAnch.GetContentAnchor() == 0 )
                {
                    continue;
                }

                if ( !pIdx )
                {
                    pIdx = new SwNodeIndex( *pCnt->GetNode() );
                }
                if ( rAnch.GetContentAnchor()->nNode == *pIdx )
                {
                    OSL_FAIL( "<SwContentNotify::~SwContentNotify()> - to page anchored object with content position." );
                    if ( !pPage )
                    {
                        pPage = pCnt->FindPageFrm();
                    }
                    SwFormatAnchor aAnch( rAnch );
                    aAnch.SetAnchor( 0 );
                    aAnch.SetPageNum( pPage->GetPhyPageNum() );
                    pFormat->SetFormatAttr( aAnch );
                    if ( RES_DRAWFRMFMT != pFormat->Which() )
                    {
                        pFormat->MakeFrms();
                    }
                }
            }
            delete pIdx;
        }
    }

    // OD 12.01.2004 #i11859# - invalidate printing area of following frame,
    //  if height of last line has changed.
    if ( pCnt->IsTextFrm() && mbChkHeightOfLastLine )
    {
        if ( mnHeightOfLastLine != static_cast<SwTextFrm*>(pCnt)->GetHeightOfLastLine() )
        {
            pCnt->InvalidateNextPrtArea();
        }
    }

    // #i44049#
    if ( pCnt->IsTextFrm() && POS_DIFF( aFrm, pCnt->Frm() ) )
    {
        pCnt->InvalidateObjs( true );
    }

    // #i43255# - move code to invalidate at-character
    // anchored objects due to a change of its anchor character from
    // method <SwTextFrm::Format(..)>.
    if ( pCnt->IsTextFrm() )
    {
        SwTextFrm* pMasterFrm = pCnt->IsFollow()
                               ? static_cast<SwTextFrm*>(pCnt)->FindMaster()
                               : static_cast<SwTextFrm*>(pCnt);
        if ( pMasterFrm && !pMasterFrm->IsFlyLock() &&
             pMasterFrm->GetDrawObjs() )
        {
            SwSortedObjs* pObjs = pMasterFrm->GetDrawObjs();
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                SwAnchoredObject* pAnchoredObj = (*pObjs)[i];
                if ( pAnchoredObj->GetFrameFormat().GetAnchor().GetAnchorId()
                        == FLY_AT_CHAR )
                {
                    pAnchoredObj->CheckCharRectAndTopOfLine( !pMasterFrm->IsEmpty() );
                }
            }
        }
    }
}

void AppendObjs( const SwFrameFormats *pTable, sal_uLong nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage, SwDoc* doc )
{
    (void) pTable;
#if OSL_DEBUG_LEVEL > 0
    std::list<SwFrameFormat*> checkFormats;
    for ( size_t i = 0; i < pTable->size(); ++i )
    {
        SwFrameFormat *pFormat = (*pTable)[i];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetContentAnchor() &&
             (rAnch.GetContentAnchor()->nNode.GetIndex() == nIndex) )
        {
            checkFormats.push_back( pFormat );
        }
    }
#else
    (void)pTable;
#endif
    SwNode const& rNode(*doc->GetNodes()[nIndex]);
    std::vector<SwFrameFormat*> const*const pFlys(rNode.GetAnchoredFlys());
    for (size_t it = 0; pFlys && it != pFlys->size(); )
    {
        SwFrameFormat *const pFormat = (*pFlys)[it];
        const SwFormatAnchor &rAnch = pFormat->GetAnchor();
        if ( rAnch.GetContentAnchor() &&
             (rAnch.GetContentAnchor()->nNode.GetIndex() == nIndex) )
        {
#if OSL_DEBUG_LEVEL > 0
            std::list<SwFrameFormat*>::iterator checkPos = std::find( checkFormats.begin(), checkFormats.end(), pFormat );
            assert( checkPos != checkFormats.end());
            checkFormats.erase( checkPos );
#endif
            const bool bFlyAtFly = rAnch.GetAnchorId() == FLY_AT_FLY; // LAYER_IMPL
            //Is a frame or a SdrObject described?
            const bool bSdrObj = RES_DRAWFRMFMT == pFormat->Which();
            // OD 23.06.2003 #108784# - append also drawing objects anchored
            // as character.
            const bool bDrawObjInContent = bSdrObj &&
                                         (rAnch.GetAnchorId() == FLY_AS_CHAR);

            if( bFlyAtFly ||
                (rAnch.GetAnchorId() == FLY_AT_PARA) ||
                (rAnch.GetAnchorId() == FLY_AT_CHAR) ||
                bDrawObjInContent )
            {
                SdrObject* pSdrObj = 0;
                if ( bSdrObj && 0 == (pSdrObj = pFormat->FindSdrObject()) )
                {
                    OSL_ENSURE( !bSdrObj, "DrawObject not found." );
                    ++it;
                    pFormat->GetDoc()->DelFrameFormat( pFormat );
                    continue;
                }
                if ( pSdrObj )
                {
                    if ( !pSdrObj->GetPage() )
                    {
                        pFormat->getIDocumentDrawModelAccess().GetDrawModel()->GetPage(0)->
                                InsertObject(pSdrObj, pSdrObj->GetOrdNumDirect());
                    }

                    SwDrawContact* pNew =
                        static_cast<SwDrawContact*>(GetUserCall( pSdrObj ));
                    if ( !pNew->GetAnchorFrm() )
                    {
                        pFrm->AppendDrawObj( *(pNew->GetAnchoredObj( 0L )) );
                    }
                    // OD 19.06.2003 #108784# - add 'virtual' drawing object,
                    // if necessary. But control objects have to be excluded.
                    else if ( !::CheckControlLayer( pSdrObj ) &&
                              pNew->GetAnchorFrm() != pFrm &&
                              !pNew->GetDrawObjectByAnchorFrm( *pFrm ) )
                    {
                        SwDrawVirtObj* pDrawVirtObj = pNew->AddVirtObj();
                        pFrm->AppendDrawObj( *(pNew->GetAnchoredObj( pDrawVirtObj )) );

                        pDrawVirtObj->ActionChanged();
                    }

                }
                else
                {
                    SwFlyFrm *pFly;
                    if( bFlyAtFly )
                        pFly = new SwFlyLayFrm( static_cast<SwFlyFrameFormat*>(pFormat), pFrm, pFrm );
                    else
                        pFly = new SwFlyAtCntFrm( static_cast<SwFlyFrameFormat*>(pFormat), pFrm, pFrm );
                    pFly->Lock();
                    pFrm->AppendFly( pFly );
                    pFly->Unlock();
                    if ( pPage )
                        ::RegistFlys( pPage, pFly );
                }
            }
        }
        ++it;
    }
#if OSL_DEBUG_LEVEL > 0
    assert( checkFormats.empty());
#endif
}

static bool lcl_ObjConnected( const SwFrameFormat *pFormat, const SwFrm* pSib )
{
    if ( RES_FLYFRMFMT == pFormat->Which() )
    {
        SwIterator<SwFlyFrm,SwFormat> aIter( *pFormat );
        const SwRootFrm* pRoot = pSib ? pSib->getRootFrm() : 0;
        const SwFlyFrm* pTmpFrm;
        for( pTmpFrm = aIter.First(); pTmpFrm; pTmpFrm = aIter.Next() )
        {
            if(! pRoot || pRoot == pTmpFrm->getRootFrm() )
                return true;
        }
    }
    else
    {
        SwDrawContact *pContact = SwIterator<SwDrawContact,SwFormat>(*pFormat).First();
        if ( pContact )
            return pContact->GetAnchorFrm() != 0;
    }
    return false;
}

/** helper method to determine, if a <SwFrameFormat>, which has an object connected,
    is located in header or footer.

    OD 23.06.2003 #108784#
*/
static bool lcl_InHeaderOrFooter( const SwFrameFormat& _rFormat )
{
    bool bRetVal = false;

    const SwFormatAnchor& rAnch = _rFormat.GetAnchor();

    if (rAnch.GetAnchorId() != FLY_AT_PAGE)
    {
        bRetVal = _rFormat.GetDoc()->IsInHeaderFooter( rAnch.GetContentAnchor()->nNode );
    }

    return bRetVal;
}

void AppendAllObjs( const SwFrameFormats *pTable, const SwFrm* pSib )
{
    //Connecting of all Objects, which are described in the SpzTable with the
    //layout.
    //If nothing happens anymore we can stop. Then formats can still remain,
    //because we neither use character bound frames nor objects which
    //are anchored to character bounds.

    // Optimization: This code used to make a copy of pTable and erase() handled items, but using
    // vector::erase() is a bad idea for performance (especially with large mailmerge documents
    // it results in extensive repeated copying). Use another vector for marking whether the item
    // has been handled and operate on the original data without altering them.
    std::vector< bool > handled( pTable->size(), false );
    size_t handledCount = 0;

    while ( handledCount < pTable->size())
    {
        bool changed = false;
        for ( int i = 0; i < int(pTable->size()); ++i )
        {
            if( handled[ i ] )
                continue;
            SwFrameFormat *pFormat = (*pTable)[ i ];
            const SwFormatAnchor &rAnch = pFormat->GetAnchor();
            bool bRemove = false;
            if ((rAnch.GetAnchorId() == FLY_AT_PAGE) ||
                (rAnch.GetAnchorId() == FLY_AS_CHAR))
            {
                //Page bounded are already anchored, character bounded
                //I don't want here.
                bRemove = true;
            }
            else
            {
                bRemove = ::lcl_ObjConnected( pFormat, pSib );
                if  ( !bRemove || ::lcl_InHeaderOrFooter( *pFormat ) )
                {
                    // OD 23.06.2003 #108784# - correction: for objects in header
                    // or footer create frames, in spite of the fact that an connected
                    // objects already exists.
                    //Call for Flys and DrawObjs only a MakeFrms if nor
                    //no dependent exists, otherwise, or if the MakeDrms creates no
                    //dependents, remove.
                    pFormat->MakeFrms();
                    bRemove = ::lcl_ObjConnected( pFormat, pSib );
                }
            }
            if ( bRemove )
            {
                handled[ i ] = true;
                ++handledCount;
                changed = true;
            }
        }
        if( !changed )
            break;
    }
}

/** local method to set 'working' position for newly inserted frames

    OD 12.08.2003 #i17969#
*/
static void lcl_SetPos( SwFrm&             _rNewFrm,
                 const SwLayoutFrm& _rLayFrm )
{
    SWRECTFN( (&_rLayFrm) )
    (_rNewFrm.Frm().*fnRect->fnSetPos)( (_rLayFrm.Frm().*fnRect->fnGetPos)() );
    // move position by one SwTwip in text flow direction in order to get
    // notifications for a new calculated position after its formatting.
    if ( bVert )
        _rNewFrm.Frm().Pos().X() -= 1;
    else
        _rNewFrm.Frm().Pos().Y() += 1;
}

void _InsertCnt( SwLayoutFrm *pLay, SwDoc *pDoc,
                             sal_uLong nIndex, bool bPages, sal_uLong nEndIndex,
                             SwFrm *pPrv )
{
    pDoc->getIDocumentTimerAccess().BlockIdling();
    SwRootFrm* pLayout = pLay->getRootFrm();
    const bool bOldCallbackActionEnabled = pLayout && pLayout->IsCallbackActionEnabled();
    if( bOldCallbackActionEnabled )
        pLayout->SetCallbackActionEnabled( false );

    //In the generation of the Layout bPages=true will be handed over.
    //Then will be new pages generated all x paragraphs already times in advance.
    //On breaks and/or pagedescriptorchanges the correspondig will be generated
    //immediately.
    //The advantage is, that on one hand already a nearly realistic number of
    //pages are created, but above all there are no almost endless long chain
    //of paragraphs, which must be moved expensively until it reaches a tolarable
    //reduced level.
    //We'd like to think that 20 Paragraphs fit on one page.
    //So that it does not become in extreme situations so violent we calculate depending
    //on the node something to it.
    //If in the DocStatistik a usable given pagenumber
    //(Will be cared for while writing), so it will be presumed that this will be
    //number of pages.
    const bool bStartPercent = bPages && !nEndIndex;

    SwPageFrm *pPage = pLay->FindPageFrm();
    const SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();
    SwFrm       *pFrm = 0;
    bool   bBreakAfter   = false;

    SwActualSection *pActualSection = 0;
    SwLayHelper *pPageMaker;

    //If the layout will be created (bPages == true) we do head on the progress
    //Flys and DrawObjekte are not connected immediately, this
    //happens only at the end of the function.
    if ( bPages )
    {
        // Attention: the SwLayHelper class uses references to the content-,
        // page-, layout-frame etc. and may change them!
        pPageMaker = new SwLayHelper( pDoc, pFrm, pPrv, pPage, pLay,
                pActualSection, bBreakAfter, nIndex, 0 == nEndIndex );
        if( bStartPercent )
        {
            const sal_uLong nPageCount = pPageMaker->CalcPageCount();
            if( nPageCount )
                bObjsDirect = false;
        }
    }
    else
        pPageMaker = NULL;

    if( pLay->IsInSct() &&
        ( pLay->IsSctFrm() || pLay->GetUpper() ) ) // Hereby will newbies
            // be intercepted, of which flags could not determined yet,
            // for e.g. while inserting a table
    {
        SwSectionFrm* pSct = pLay->FindSctFrm();
        // If content will be inserted in a footnote, which in an column area,
        // the column area it is not allowed to be broken up.
        // Only if in the inner of the footnote lies an area, is this a candidate
        // for pActualSection.
        // The same applies for areas in tables, if inside the table will be
        // something inserted, it's only allowed to break up areas, which
        // lies in the inside also.
        if( ( !pLay->IsInFootnote() || pSct->IsInFootnote() ) &&
            ( !pLay->IsInTab() || pSct->IsInTab() ) )
        {
            pActualSection = new SwActualSection( 0, pSct, 0 );
            OSL_ENSURE( !pLay->Lower() || !pLay->Lower()->IsColumnFrm(),
                "_InsertCnt: Wrong Call" );
        }
    }

    //If a section is "open", the pActualSection points to an SwActualSection.
    //If the page breaks, for "open" sections a follow will created.
    //For nested sections (which have, however, not a nested layout),
    //the SwActualSection class has a member, which points to an upper(section).
    //When the "inner" section finishes, the upper will used instead.

    while( true )
    {
        SwNode *pNd = pDoc->GetNodes()[nIndex];
        if ( pNd->IsContentNode() )
        {
            SwContentNode* pNode = static_cast<SwContentNode*>(pNd);
            pFrm = pNode->MakeFrm(pLay);
            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrm->InsertBehind( pLay, pPrv );
            // #i27138#
            // notify accessibility paragraphs objects about changed
            // CONTENT_FLOWS_FROM/_TO relation.
            // Relation CONTENT_FLOWS_FROM for next paragraph will change
            // and relation CONTENT_FLOWS_TO for previous paragraph will change.
            if ( pFrm->IsTextFrm() )
            {
                SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                // no notification, if <SwViewShell> is in construction
                if ( pViewShell && !pViewShell->IsInConstructor() &&
                     pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                        dynamic_cast<SwTextFrm*>(pFrm->FindNextCnt( true )),
                        dynamic_cast<SwTextFrm*>(pFrm->FindPrevCnt( true )) );
                    // #i68958#
                    // The information flags of the text frame are validated
                    // in methods <FindNextCnt(..)> and <FindPrevCnt(..)>.
                    // The information flags have to be invalidated, because
                    // it is possible, that the one of its upper frames
                    // isn't inserted into the layout.
                    pFrm->InvalidateInfFlags();
                }
            }
            // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
            // for setting position at newly inserted frame
            lcl_SetPos( *pFrm, *pLay );
            pPrv = pFrm;

            if ( !pTable->empty() && bObjsDirect && !bDontCreateObjects )
                AppendObjs( pTable, nIndex, pFrm, pPage, pDoc );
        }
        else if ( pNd->IsTableNode() )
        {   //Should we have encountered a table?
            SwTableNode *pTableNode = static_cast<SwTableNode*>(pNd);

            // #108116# loading may produce table structures that GCLines
            // needs to clean up. To keep table formulas correct, change
            // all table formulas to internal (BOXPTR) representation.
            SwTableFormulaUpdate aMsgHint( &pTableNode->GetTable() );
            aMsgHint.eFlags = TBL_BOXPTR;
            pDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
            pTableNode->GetTable().GCLines();

            pFrm = pTableNode->MakeFrm( pLay );

            if( pPageMaker )
                pPageMaker->CheckInsert( nIndex );

            pFrm->InsertBehind( pLay, pPrv );
            // #i27138#
            // notify accessibility paragraphs objects about changed
            // CONTENT_FLOWS_FROM/_TO relation.
            // Relation CONTENT_FLOWS_FROM for next paragraph will change
            // and relation CONTENT_FLOWS_TO for previous paragraph will change.
            {
                SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                // no notification, if <SwViewShell> is in construction
                if ( pViewShell && !pViewShell->IsInConstructor() &&
                     pViewShell->GetLayout() &&
                     pViewShell->GetLayout()->IsAnyShellAccessible() )
                {
                    pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrm*>(pFrm->FindNextCnt( true )),
                            dynamic_cast<SwTextFrm*>(pFrm->FindPrevCnt( true )) );
                }
            }
            if ( bObjsDirect && !pTable->empty() )
                static_cast<SwTabFrm*>(pFrm)->RegistFlys();
            // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
            // for setting position at newly inserted frame
            lcl_SetPos( *pFrm, *pLay );

            pPrv = pFrm;
            //Set the index to the endnode of the table section.
            nIndex = pTableNode->EndOfSectionIndex();

            SwTabFrm* pTmpFrm = static_cast<SwTabFrm*>(pFrm);
            while ( pTmpFrm )
            {
                pTmpFrm->CheckDirChange();
                pTmpFrm = pTmpFrm->IsFollow() ? pTmpFrm->FindMaster() : NULL;
            }

        }
        else if ( pNd->IsSectionNode() )
        {
            SwSectionNode *pNode = static_cast<SwSectionNode*>(pNd);
            if( pNode->GetSection().CalcHiddenFlag() )
                // is hidden, skip the area
                nIndex = pNode->EndOfSectionIndex();
            else
            {
                pFrm = pNode->MakeFrm( pLay );
                pActualSection = new SwActualSection( pActualSection,
                                                static_cast<SwSectionFrm*>(pFrm), pNode );
                if ( pActualSection->GetUpper() )
                {
                    //Insert behind the Upper, the "Follow" of the Upper will be
                    //generated at the EndNode.
                    SwSectionFrm *pTmp = pActualSection->GetUpper()->GetSectionFrm();
                    pFrm->InsertBehind( pTmp->GetUpper(), pTmp );
                    // OD 25.03.2003 #108339# - direct initialization of section
                    // after insertion in the layout
                    static_cast<SwSectionFrm*>(pFrm)->Init();
                }
                else
                {
                    pFrm->InsertBehind( pLay, pPrv );
                    // OD 25.03.2003 #108339# - direct initialization of section
                    // after insertion in the layout
                    static_cast<SwSectionFrm*>(pFrm)->Init();

                    // #i33963#
                    // Do not trust the IsInFootnote flag. If we are currently
                    // building up a table, the upper of pPrv may be a cell
                    // frame, but the cell frame does not have an upper yet.
                    if( pPrv && 0 != pPrv->ImplFindFootnoteFrm() )
                    {
                        if( pPrv->IsSctFrm() )
                            pPrv = static_cast<SwSectionFrm*>(pPrv)->ContainsContent();
                        if( pPrv && pPrv->IsTextFrm() )
                            static_cast<SwTextFrm*>(pPrv)->Prepare( PREP_QUOVADIS, 0, false );
                    }
                }
                // #i27138#
                // notify accessibility paragraphs objects about changed
                // CONTENT_FLOWS_FROM/_TO relation.
                // Relation CONTENT_FLOWS_FROM for next paragraph will change
                // and relation CONTENT_FLOWS_TO for previous paragraph will change.
                {
                    SwViewShell* pViewShell( pFrm->getRootFrm()->GetCurrShell() );
                    // no notification, if <SwViewShell> is in construction
                    if ( pViewShell && !pViewShell->IsInConstructor() &&
                         pViewShell->GetLayout() &&
                         pViewShell->GetLayout()->IsAnyShellAccessible() )
                    {
                        pViewShell->InvalidateAccessibleParaFlowRelation(
                            dynamic_cast<SwTextFrm*>(pFrm->FindNextCnt( true )),
                            dynamic_cast<SwTextFrm*>(pFrm->FindPrevCnt( true )) );
                    }
                }
                pFrm->CheckDirChange();

                // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
                // for setting position at newly inserted frame
                lcl_SetPos( *pFrm, *pLay );

                // OD 20.11.2002 #105405# - no page, no invalidate.
                if ( pPage )
                {
                    // OD 18.09.2002 #100522#
                    // invalidate page in order to force format and paint of
                    // inserted section frame
                    pFrm->InvalidatePage( pPage );

                    // FME 10.11.2003 #112243#
                    // Invalidate fly content flag:
                    if ( pFrm->IsInFly() )
                        pPage->InvalidateFlyContent();

                    // OD 14.11.2002 #104684# - invalidate page content in order to
                    // force format and paint of section content.
                    pPage->InvalidateContent();
                }

                pLay = static_cast<SwLayoutFrm*>(pFrm);
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = 0;
            }
        }
        else if ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsSectionNode() )
        {
            OSL_ENSURE( pActualSection, "Sectionende ohne Anfang?" );
            OSL_ENSURE( pActualSection->GetSectionNode() == pNd->StartOfSectionNode(),
                            "Sectionende mit falschen Start Node?" );

            //Close the section, where appropriate activate the surrounding
            //section again.
            SwActualSection *pTmp = pActualSection ? pActualSection->GetUpper() : NULL;
            delete pActualSection;
            pLay = pLay->FindSctFrm();
            if ( 0 != (pActualSection = pTmp) )
            {
                //Could be, that the last SectionFrm remains empty.
                //Then now is the time to remove them.
                if ( !pLay->ContainsContent() )
                {
                    SwFrm *pTmpFrm = pLay;
                    pLay = pTmpFrm->GetUpper();
                    pPrv = pTmpFrm->GetPrev();
                    pTmpFrm->RemoveFromLayout();
                    SwFrm::DestroyFrm(pTmpFrm);
                }
                else
                {
                    pPrv = pLay;
                    pLay = pLay->GetUpper();
                }

                // new section frame
                pFrm = pActualSection->GetSectionNode()->MakeFrm( pLay );
                pFrm->InsertBehind( pLay, pPrv );
                static_cast<SwSectionFrm*>(pFrm)->Init();

                // OD 12.08.2003 #i17969# - consider horizontal/vertical layout
                // for setting position at newly inserted frame
                lcl_SetPos( *pFrm, *pLay );

                SwSectionFrm* pOuterSectionFrm = pActualSection->GetSectionFrm();

                // a follow has to be appended to the new section frame
                SwSectionFrm* pFollow = pOuterSectionFrm->GetFollow();
                if ( pFollow )
                {
                    pOuterSectionFrm->SetFollow( NULL );
                    pOuterSectionFrm->InvalidateSize();
                    static_cast<SwSectionFrm*>(pFrm)->SetFollow( pFollow );
                }

                // We don't want to leave empty parts back.
                if( ! pOuterSectionFrm->IsColLocked() &&
                    ! pOuterSectionFrm->ContainsContent() )
                {
                    pOuterSectionFrm->DelEmpty( true );
                    SwFrm::DestroyFrm(pOuterSectionFrm);
                }
                pActualSection->SetSectionFrm( static_cast<SwSectionFrm*>(pFrm) );

                pLay = static_cast<SwLayoutFrm*>(pFrm);
                if ( pLay->Lower() && pLay->Lower()->IsLayoutFrm() )
                    pLay = pLay->GetNextLayoutLeaf();
                pPrv = 0;
            }
            else
            {
                //Nothing more with sections, it goes on right behind
                //the SectionFrame.
                pPrv = pLay;
                pLay = pLay->GetUpper();
            }
        }
        else if( pNd->IsStartNode() &&
                 SwFlyStartNode == static_cast<SwStartNode*>(pNd)->GetStartNodeType() )
        {
            if ( !pTable->empty() && bObjsDirect && !bDontCreateObjects )
            {
                SwFlyFrm* pFly = pLay->FindFlyFrm();
                if( pFly )
                    AppendObjs( pTable, nIndex, pFly, pPage, pDoc );
            }
        }
        else
            // Neither Content nor table nor section,
            // so we have to be ready.
            break;

        ++nIndex;
        // Do not consider the end node. The caller (section/MakeFrms()) has to ensure that the end
        // of this area is positioned before EndIndex!
        if ( nEndIndex && nIndex >= nEndIndex )
            break;
    }

    if ( pActualSection )
    {
        // Might happen that an empty (Follow-)Section is left over.
        if ( !(pLay = pActualSection->GetSectionFrm())->ContainsContent() )
        {
            pLay->RemoveFromLayout();
            SwFrm::DestroyFrm(pLay);
        }
        delete pActualSection;
    }

    if ( bPages ) // let the Flys connect to each other
    {
        if ( !bDontCreateObjects )
            AppendAllObjs( pTable, pLayout );
        bObjsDirect = true;
    }

    if( pPageMaker )
    {
        pPageMaker->CheckFlyCache( pPage );
        delete pPageMaker;
        if( pDoc->GetLayoutCache() )
        {
#ifdef DBG_UTIL
            pDoc->GetLayoutCache()->CompareLayout( *pDoc );
#endif
            pDoc->GetLayoutCache()->ClearImpl();
        }
    }

    pDoc->getIDocumentTimerAccess().UnblockIdling();
    if( bOldCallbackActionEnabled )
        pLayout->SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

void MakeFrms( SwDoc *pDoc, const SwNodeIndex &rSttIdx,
               const SwNodeIndex &rEndIdx )
{
    bObjsDirect = false;

    SwNodeIndex aTmp( rSttIdx );
    sal_uLong nEndIdx = rEndIdx.GetIndex();
    SwNode* pNd = pDoc->GetNodes().FindPrvNxtFrmNode( aTmp,
                                            pDoc->GetNodes()[ nEndIdx-1 ]);
    if ( pNd )
    {
        bool bApres = aTmp < rSttIdx;
        SwNode2Layout aNode2Layout( *pNd, rSttIdx.GetIndex() );
        SwFrm* pFrm;
        while( 0 != (pFrm = aNode2Layout.NextFrm()) )
        {
            SwLayoutFrm *pUpper = pFrm->GetUpper();
            SwFootnoteFrm* pFootnoteFrm = pUpper->FindFootnoteFrm();
            bool bOldLock, bOldFootnote;
            if( pFootnoteFrm )
            {
                bOldFootnote = pFootnoteFrm->IsColLocked();
                pFootnoteFrm->ColLock();
            }
            else
                bOldFootnote = true;
            SwSectionFrm* pSct = pUpper->FindSctFrm();
            // Inside of footnotes only those areas are interesting that are inside of them. But
            // not the ones (e.g. column areas) in which are the footnote containers positioned.
            // #109767# Table frame is in section, insert section in cell frame.
            if( pSct && ((pFootnoteFrm && !pSct->IsInFootnote()) || pUpper->IsCellFrm()) )
                pSct = NULL;
            if( pSct )
            {   // to prevent pTmp->MoveFwd from destroying the SectionFrm
                bOldLock = pSct->IsColLocked();
                pSct->ColLock();
            }
            else
                bOldLock = true;

            // If pFrm cannot be moved, it is not possible to move it to the next page. This applies
            // also for frames (in the first column of a frame pFrm is moveable) and column
            // sections of tables (also here pFrm is moveable).
            bool bMoveNext = nEndIdx - rSttIdx.GetIndex() > 120;
            bool bAllowMove = !pFrm->IsInFly() && pFrm->IsMoveable() &&
                 (!pFrm->IsInTab() || pFrm->IsTabFrm() );
            if ( bMoveNext && bAllowMove )
            {
                SwFrm *pMove = pFrm;
                SwFrm *pPrev = pFrm->GetPrev();
                SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pMove );
                OSL_ENSURE( pTmp, "Missing FlowFrm" );

                if ( bApres )
                {
                    // The rest of this page should be empty. Thus, the following one has to move to
                    // the next page (it might also be located in the following column).
                    OSL_ENSURE( !pTmp->HasFollow(), "Follows forbidden" );
                    pPrev = pFrm;
                    // If the surrounding SectionFrm has a "next" one,
                    // so this one needs to be moved as well.
                    pMove = pFrm->GetIndNext();
                    SwColumnFrm* pCol = static_cast<SwColumnFrm*>(pFrm->FindColFrm());
                    if( pCol )
                        pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
                    do
                    {
                        if( pCol && !pMove )
                        {   // No successor so far, look into the next column
                            pMove = pCol->ContainsAny();
                            if( pCol->GetNext() )
                                pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
                            else if( pCol->IsInSct() )
                            {   // If there is no following column but we are in a column frame,
                                // there might be (page) columns outside of it.
                                pCol = static_cast<SwColumnFrm*>(pCol->FindSctFrm()->FindColFrm());
                                if( pCol )
                                    pCol = static_cast<SwColumnFrm*>(pCol->GetNext());
                            }
                            else
                                pCol = NULL;
                        }
                        // skip invalid SectionFrms
                        while( pMove && pMove->IsSctFrm() &&
                               !static_cast<SwSectionFrm*>(pMove)->GetSection() )
                            pMove = pMove->GetNext();
                    } while( !pMove && pCol );

                    if( pMove )
                    {
                        if ( pMove->IsContentFrm() )
                            pTmp = static_cast<SwContentFrm*>(pMove);
                        else if ( pMove->IsTabFrm() )
                            pTmp = static_cast<SwTabFrm*>(pMove);
                        else if ( pMove->IsSctFrm() )
                        {
                            pMove = static_cast<SwSectionFrm*>(pMove)->ContainsAny();
                            if( pMove )
                                pTmp = SwFlowFrm::CastFlowFrm( pMove );
                            else
                                pTmp = NULL;
                        }
                    }
                    else
                        pTmp = 0;
                }
                else
                {
                    OSL_ENSURE( !pTmp->IsFollow(), "Follows really forbidden" );
                    // move the _content_ of a section frame
                    if( pMove->IsSctFrm() )
                    {
                        while( pMove && pMove->IsSctFrm() &&
                               !static_cast<SwSectionFrm*>(pMove)->GetSection() )
                            pMove = pMove->GetNext();
                        if( pMove && pMove->IsSctFrm() )
                            pMove = static_cast<SwSectionFrm*>(pMove)->ContainsAny();
                        if( pMove )
                            pTmp = SwFlowFrm::CastFlowFrm( pMove );
                        else
                            pTmp = NULL;
                    }
                }

                if( pTmp )
                {
                    SwFrm* pOldUp = pTmp->GetFrm().GetUpper();
                    // MoveFwd==true means that we are still on the same page.
                    // But since we want to move if possible!
                    bool bTmpOldLock = pTmp->IsJoinLocked();
                    pTmp->LockJoin();
                    while( pTmp->MoveFwd( true, false, true ) )
                    {
                        if( pOldUp == pTmp->GetFrm().GetUpper() )
                            break;
                        pOldUp = pTmp->GetFrm().GetUpper();
                    }
                    if( !bTmpOldLock )
                        pTmp->UnlockJoin();
                }
                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(),
                              pFrm->IsInDocBody(), nEndIdx, pPrev );
            }
            else
            {
                bool bSplit;
                SwFrm* pPrv = bApres ? pFrm : pFrm->GetPrev();
                // If the section frame is inserted into another one, it must be split.
                if( pSct && rSttIdx.GetNode().IsSectionNode() )
                {
                    bSplit = pSct->SplitSect( pFrm, bApres );
                    if( !bSplit && !bApres )
                    {
                        pUpper = pSct->GetUpper();
                        pPrv = pSct->GetPrev();
                    }
                }
                else
                    bSplit = false;

                ::_InsertCnt( pUpper, pDoc, rSttIdx.GetIndex(), false,
                              nEndIdx, pPrv );
                // OD 23.06.2003 #108784# - correction: append objects doesn't
                // depend on value of <bAllowMove>
                if( !bDontCreateObjects )
                {
                    const SwFrameFormats *pTable = pDoc->GetSpzFrameFormats();
                    if( !pTable->empty() )
                        AppendAllObjs( pTable, pUpper );
                }

                // If nothing was added (e.g. a hidden section), the split must be reversed.
                if( bSplit && pSct && pSct->GetNext()
                    && pSct->GetNext()->IsSctFrm() )
                    pSct->MergeNext( static_cast<SwSectionFrm*>(pSct->GetNext()) );
                if( pFrm->IsInFly() )
                    pFrm->FindFlyFrm()->_Invalidate();
                if( pFrm->IsInTab() )
                    pFrm->InvalidateSize();
            }

            SwPageFrm *pPage = pUpper->FindPageFrm();
            SwFrm::CheckPageDescs( pPage, false );
            if( !bOldFootnote )
                pFootnoteFrm->ColUnlock();
            if( !bOldLock )
            {
                pSct->ColUnlock();
                // pSct might be empty (e.g. when inserting linked section containing further
                // sections) and can be destroyed in such cases.
                if( !pSct->ContainsContent() )
                {
                    pSct->DelEmpty( true );
                    pUpper->getRootFrm()->RemoveFromList( pSct );
                    SwFrm::DestroyFrm(pSct);
                }
            }
        }
    }

    bObjsDirect = true;
}

SwBorderAttrs::SwBorderAttrs(const SwModify *pMod, const SwFrm *pConstructor)
    : SwCacheObj(pMod)
    , rAttrSet(pConstructor->IsContentFrm()
                    ? static_cast<const SwContentFrm*>(pConstructor)->GetNode()->GetSwAttrSet()
                    : static_cast<const SwLayoutFrm*>(pConstructor)->GetFormat()->GetAttrSet())
    , rUL(rAttrSet.GetULSpace())
    // #i96772#
    // LRSpaceItem is copied due to the possibility that it is adjusted - see below
    , rLR(rAttrSet.GetLRSpace())
    , rBox(rAttrSet.GetBox())
    , rShadow(rAttrSet.GetShadow())
    , aFrmSize(rAttrSet.GetFrmSize().GetSize())
    , bIsLine(false)
    , bJoinedWithPrev(false)
    , bJoinedWithNext(false)
    , nTopLine(0)
    , nBottomLine(0)
    , nLeftLine(0)
    , nRightLine(0)
    , nTop(0)
    , nBottom(0)
    , nGetTopLine(0)
    , nGetBottomLine(0)
{
    // #i96772#
    const SwTextFrm* pTextFrm = dynamic_cast<const SwTextFrm*>(pConstructor);
    if ( pTextFrm )
    {
        pTextFrm->GetTextNode()->ClearLRSpaceItemDueToListLevelIndents( rLR );
    }
    else if ( pConstructor->IsNoTextFrm() )
    {
        rLR = SvxLRSpaceItem ( RES_LR_SPACE );
    }

    // Caution: The USHORTs for the cached values are not initialized by intention!

    // everything needs to be calculated at least once:
    bTopLine = bBottomLine = bLeftLine = bRightLine =
    bTop     = bBottom     = bLine   = true;

    bCacheGetLine = bCachedGetTopLine = bCachedGetBottomLine = false;
    // OD 21.05.2003 #108789# - init cache status for values <bJoinedWithPrev>
    // and <bJoinedWithNext>, which aren't initialized by default.
    bCachedJoinedWithPrev = false;
    bCachedJoinedWithNext = false;

    bBorderDist = 0 != (pConstructor->GetType() & (FRM_CELL));
}

SwBorderAttrs::~SwBorderAttrs()
{
    const_cast<SwModify *>(static_cast<SwModify const *>(pOwner))->SetInCache( false );
}

/* All calc methods calculate a safety distance in addition to the values given by the attributes.
 * This safety distance is only added when working with borders and/or shadows to prevent that
 * e.g. borders are painted over.
 */

void SwBorderAttrs::_CalcTop()
{
    nTop = CalcTopLine() + rUL.GetUpper();
    bTop = false;
}

void SwBorderAttrs::_CalcBottom()
{
    nBottom = CalcBottomLine() + rUL.GetLower();
    bBottom = false;
}

long SwBorderAttrs::CalcRight( const SwFrm* pCaller ) const
{
    long nRight=0;

    if (!pCaller->IsTextFrm() || !static_cast<const SwTextFrm*>(pCaller)->GetTextNode()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::INVERT_BORDER_SPACING)) {
    // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
    // and right border are painted on the right respectively left.
    if ( pCaller->IsCellFrm() && pCaller->IsRightToLeft() )
        nRight = CalcLeftLine();
    else
        nRight = CalcRightLine();

    }
    // for paragraphs, "left" is "before text" and "right" is "after text"
    if ( pCaller->IsTextFrm() && pCaller->IsRightToLeft() )
        nRight += rLR.GetLeft();
    else
        nRight += rLR.GetRight();

    // correction: retrieve left margin for numbering in R2L-layout
    if ( pCaller->IsTextFrm() && pCaller->IsRightToLeft() )
    {
        nRight += static_cast<const SwTextFrm*>(pCaller)->GetTextNode()->GetLeftMarginWithNum();
    }

    return nRight;
}

/// Tries to detect if this paragraph has a floating table attached.
static bool lcl_hasTabFrm(const SwTextFrm* pTextFrm)
{
    if (pTextFrm->GetDrawObjs())
    {
        const SwSortedObjs* pSortedObjs = pTextFrm->GetDrawObjs();
        if (pSortedObjs->size() > 0)
        {
            SwAnchoredObject* pObject = (*pSortedObjs)[0];
            if (pObject->IsA(TYPE(SwFlyFrm)))
            {
                SwFlyFrm* pFly = static_cast<SwFlyFrm*>(pObject);
                if (pFly->Lower() && pFly->Lower()->IsTabFrm())
                    return true;
            }
        }
    }
    return false;
}

long SwBorderAttrs::CalcLeft( const SwFrm *pCaller ) const
{
    long nLeft=0;

    if (!pCaller->IsTextFrm() || !static_cast<const SwTextFrm*>(pCaller)->GetTextNode()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::INVERT_BORDER_SPACING)) {
    // OD 23.01.2003 #106895# - for cell frame in R2L text direction the left
    // and right border are painted on the right respectively left.
    if ( pCaller->IsCellFrm() && pCaller->IsRightToLeft() )
        nLeft = CalcRightLine();
    else
        nLeft = CalcLeftLine();
    }

    // for paragraphs, "left" is "before text" and "right" is "after text"
    if ( pCaller->IsTextFrm() && pCaller->IsRightToLeft() )
        nLeft += rLR.GetRight();
    else
    {
        bool bIgnoreMargin = false;
        if (pCaller->IsTextFrm())
        {
            const SwTextFrm* pTextFrm = static_cast<const SwTextFrm*>(pCaller);
            if (pTextFrm->GetTextNode()->GetDoc()->GetDocumentSettingManager().get(DocumentSettingId::FLOATTABLE_NOMARGINS))
            {
                // If this is explicitly requested, ignore the margins next to the floating table.
                if (lcl_hasTabFrm(pTextFrm))
                    bIgnoreMargin = true;
                // TODO here we only handle the first two paragraphs, would be nice to generalize this.
                else if (pTextFrm->FindPrev() && pTextFrm->FindPrev()->IsTextFrm() && lcl_hasTabFrm(static_cast<const SwTextFrm*>(pTextFrm->FindPrev())))
                    bIgnoreMargin = true;
            }
        }
        if (!bIgnoreMargin)
            nLeft += rLR.GetLeft();
    }

    // correction: do not retrieve left margin for numbering in R2L-layout
    if ( pCaller->IsTextFrm() && !pCaller->IsRightToLeft() )
    {
        nLeft += static_cast<const SwTextFrm*>(pCaller)->GetTextNode()->GetLeftMarginWithNum();
    }

    return nLeft;
}

/* Calculated values for borders and shadows.
 * It might be that a distance is wanted even without lines. This will be
 * considered here and not by the attribute (e.g. bBorderDist for cells).
 */

void SwBorderAttrs::_CalcTopLine()
{
    nTopLine = (bBorderDist && !rBox.GetTop())
                            ? rBox.GetDistance  (SvxBoxItemLine::TOP)
                            : rBox.CalcLineSpace(SvxBoxItemLine::TOP);
    nTopLine = nTopLine + rShadow.CalcShadowSpace(SvxShadowItemSide::TOP);
    bTopLine = false;
}

void SwBorderAttrs::_CalcBottomLine()
{
    nBottomLine = (bBorderDist && !rBox.GetBottom())
                            ? rBox.GetDistance  (SvxBoxItemLine::BOTTOM)
                            : rBox.CalcLineSpace(SvxBoxItemLine::BOTTOM);
    nBottomLine = nBottomLine + rShadow.CalcShadowSpace(SvxShadowItemSide::BOTTOM);
    bBottomLine = false;
}

void SwBorderAttrs::_CalcLeftLine()
{
    nLeftLine = (bBorderDist && !rBox.GetLeft())
                            ? rBox.GetDistance  (SvxBoxItemLine::LEFT)
                            : rBox.CalcLineSpace(SvxBoxItemLine::LEFT);
    nLeftLine = nLeftLine + rShadow.CalcShadowSpace(SvxShadowItemSide::LEFT);
    bLeftLine = false;
}

void SwBorderAttrs::_CalcRightLine()
{
    nRightLine = (bBorderDist && !rBox.GetRight())
                            ? rBox.GetDistance  (SvxBoxItemLine::RIGHT)
                            : rBox.CalcLineSpace(SvxBoxItemLine::RIGHT);
    nRightLine = nRightLine + rShadow.CalcShadowSpace(SvxShadowItemSide::RIGHT);
    bRightLine = false;
}

void SwBorderAttrs::_IsLine()
{
    bIsLine = rBox.GetTop() || rBox.GetBottom() ||
              rBox.GetLeft()|| rBox.GetRight();
    bLine = false;
}

/* The borders of neighboring paragraphs are condensed by following algorithm:
 *
 * 1. No top border if the predecessor has the same top border and (3) applies.
 *    In addition, the paragraph needs to have a border at least one side (left/right/bottom).
 * 2. No bottom border if the successor has the same bottom border and (3) applies.
 *    In addition, the paragraph needs to have a border at least one side (left/right/top).
 * 3. The borders on the left and right side are identical between the current and the
 *    pre-/succeeding paragraph.
 */

inline bool CmpLines( const editeng::SvxBorderLine *pL1, const editeng::SvxBorderLine *pL2 )
{
    return ( ((pL1 && pL2) && (*pL1 == *pL2)) || (!pL1 && !pL2) );
}

// OD 21.05.2003 #108789# - change name of 1st parameter - "rAttrs" -> "rCmpAttrs"
// OD 21.05.2003 #108789# - compare <CalcRight()> and <rCmpAttrs.CalcRight()>
//          instead of only the right LR-spacing, because R2L-layout has to be
//          considered.
bool SwBorderAttrs::CmpLeftRight( const SwBorderAttrs &rCmpAttrs,
                                  const SwFrm *pCaller,
                                  const SwFrm *pCmp ) const
{
    return ( CmpLines( rCmpAttrs.GetBox().GetLeft(), GetBox().GetLeft()  ) &&
             CmpLines( rCmpAttrs.GetBox().GetRight(),GetBox().GetRight() ) &&
             CalcLeft( pCaller ) == rCmpAttrs.CalcLeft( pCmp ) &&
             // OD 21.05.2003 #108789# - compare <CalcRight> with <rCmpAttrs.CalcRight>.
             CalcRight( pCaller ) == rCmpAttrs.CalcRight( pCmp ) );
}

bool SwBorderAttrs::_JoinWithCmp( const SwFrm& _rCallerFrm,
                                  const SwFrm& _rCmpFrm ) const
{
    bool bReturnVal = false;

    SwBorderAttrAccess aCmpAccess( SwFrm::GetCache(), &_rCmpFrm );
    const SwBorderAttrs &rCmpAttrs = *aCmpAccess.Get();
    if ( rShadow == rCmpAttrs.GetShadow() &&
         CmpLines( rBox.GetTop(), rCmpAttrs.GetBox().GetTop() ) &&
         CmpLines( rBox.GetBottom(), rCmpAttrs.GetBox().GetBottom() ) &&
         CmpLeftRight( rCmpAttrs, &_rCallerFrm, &_rCmpFrm )
       )
    {
        bReturnVal = true;
    }

    return bReturnVal;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// previous frame. Calculated value saved in cached value <bJoinedWithPrev>
// OD 2004-02-26 #i25029# - add 2nd parameter <_pPrevFrm>
void SwBorderAttrs::_CalcJoinedWithPrev( const SwFrm& _rFrm,
                                         const SwFrm* _pPrevFrm )
{
    // set default
    bJoinedWithPrev = false;

    if ( _rFrm.IsTextFrm() )
    {
        // text frame can potentially join with previous text frame, if
        // corresponding attribute set is set at previous text frame.
        // OD 2004-02-26 #i25029# - If parameter <_pPrevFrm> is set, take this
        // one as previous frame.
        const SwFrm* pPrevFrm = _pPrevFrm ? _pPrevFrm : _rFrm.GetPrev();
        // OD 2004-02-13 #i25029# - skip hidden text frames.
        while ( pPrevFrm && pPrevFrm->IsTextFrm() &&
                static_cast<const SwTextFrm*>(pPrevFrm)->IsHiddenNow() )
        {
            pPrevFrm = pPrevFrm->GetPrev();
        }
        if ( pPrevFrm && pPrevFrm->IsTextFrm() &&
             pPrevFrm->GetAttrSet()->GetParaConnectBorder().GetValue()
           )
        {
            bJoinedWithPrev = _JoinWithCmp( _rFrm, *(pPrevFrm) );
        }
    }

    // valid cache status, if demanded
    // OD 2004-02-26 #i25029# - Do not validate cache, if parameter <_pPrevFrm>
    // is set.
    bCachedJoinedWithPrev = bCacheGetLine && !_pPrevFrm;
}

// OD 21.05.2003 #108789# - method to determine, if borders are joined with
// next frame. Calculated value saved in cached value <bJoinedWithNext>
void SwBorderAttrs::_CalcJoinedWithNext( const SwFrm& _rFrm )
{
    // set default
    bJoinedWithNext = false;

    if ( _rFrm.IsTextFrm() )
    {
        // text frame can potentially join with next text frame, if
        // corresponding attribute set is set at current text frame.
        // OD 2004-02-13 #i25029# - get next frame, but skip hidden text frames.
        const SwFrm* pNextFrm = _rFrm.GetNext();
        while ( pNextFrm && pNextFrm->IsTextFrm() &&
                static_cast<const SwTextFrm*>(pNextFrm)->IsHiddenNow() )
        {
            pNextFrm = pNextFrm->GetNext();
        }
        if ( pNextFrm && pNextFrm->IsTextFrm() &&
             _rFrm.GetAttrSet()->GetParaConnectBorder().GetValue()
           )
        {
            bJoinedWithNext = _JoinWithCmp( _rFrm, *(pNextFrm) );
        }
    }

    // valid cache status, if demanded
    bCachedJoinedWithNext = bCacheGetLine;
}

// OD 21.05.2003 #108789# - accessor for cached values <bJoinedWithPrev>
// OD 2004-02-26 #i25029# - add 2nd parameter <_pPrevFrm>, which is passed to
// method <_CalcJoindWithPrev(..)>.
bool SwBorderAttrs::JoinedWithPrev( const SwFrm& _rFrm,
                                    const SwFrm* _pPrevFrm ) const
{
    if ( !bCachedJoinedWithPrev || _pPrevFrm )
    {
        // OD 2004-02-26 #i25029# - pass <_pPrevFrm> as 2nd parameter
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithPrev( _rFrm, _pPrevFrm );
    }

    return bJoinedWithPrev;
}

bool SwBorderAttrs::JoinedWithNext( const SwFrm& _rFrm ) const
{
    if ( !bCachedJoinedWithNext )
    {
        const_cast<SwBorderAttrs*>(this)->_CalcJoinedWithNext( _rFrm );
    }

    return bJoinedWithNext;
}

// OD 2004-02-26 #i25029# - added 2nd parameter <_pPrevFrm>, which is passed to
// method <JoinedWithPrev>
void SwBorderAttrs::_GetTopLine( const SwFrm& _rFrm,
                                 const SwFrm* _pPrevFrm )
{
    sal_uInt16 nRet = CalcTopLine();

    // OD 21.05.2003 #108789# - use new method <JoinWithPrev()>
    // OD 2004-02-26 #i25029# - add 2nd parameter
    if ( JoinedWithPrev( _rFrm, _pPrevFrm ) )
    {
        nRet = 0;
    }

    bCachedGetTopLine = bCacheGetLine;

    nGetTopLine = nRet;
}

void SwBorderAttrs::_GetBottomLine( const SwFrm& _rFrm )
{
    sal_uInt16 nRet = CalcBottomLine();

    // OD 21.05.2003 #108789# - use new method <JoinWithPrev()>
    if ( JoinedWithNext( _rFrm ) )
    {
        nRet = 0;
    }

    bCachedGetBottomLine = bCacheGetLine;

    nGetBottomLine = nRet;
}

SwBorderAttrAccess::SwBorderAttrAccess( SwCache &rCach, const SwFrm *pFrm ) :
    SwCacheAccess( rCach,
                   (pFrm->IsContentFrm() ?
                      const_cast<void*>(static_cast<void const *>(static_cast<const SwContentFrm*>(pFrm)->GetNode())) :
                      const_cast<void*>(static_cast<void const *>(static_cast<const SwLayoutFrm*>(pFrm)->GetFormat()))),
                   (pFrm->IsContentFrm() ?
                      static_cast<SwModify const *>(static_cast<const SwContentFrm*>(pFrm)->GetNode())->IsInCache() :
                      static_cast<SwModify const *>(static_cast<const SwLayoutFrm*>(pFrm)->GetFormat())->IsInCache()) ),
    pConstructor( pFrm )
{
}

SwCacheObj *SwBorderAttrAccess::NewObj()
{
    const_cast<SwModify *>(static_cast<SwModify const *>(pOwner))->SetInCache( true );
    return new SwBorderAttrs( static_cast<SwModify const *>(pOwner), pConstructor );
}

SwBorderAttrs *SwBorderAttrAccess::Get()
{
    return static_cast<SwBorderAttrs*>(SwCacheAccess::Get());
}

SwOrderIter::SwOrderIter( const SwPageFrm *pPg, bool bFlys ) :
    pPage( pPg ),
    pCurrent( 0 ),
    bFlysOnly( bFlys )
{
}

const SdrObject *SwOrderIter::Top()
{
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            sal_uInt32 nTopOrd = 0;
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp >= nTopOrd )
                {
                    nTopOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

const SdrObject *SwOrderIter::Bottom()
{
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        sal_uInt32 nBotOrd = USHRT_MAX;
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nBotOrd )
                {
                    nBotOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

const SdrObject *SwOrderIter::Next()
{
    const sal_uInt32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        sal_uInt32 nOrd = USHRT_MAX;
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp > nCurOrd && nTmp < nOrd )
                {
                    nOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

const SdrObject *SwOrderIter::Prev()
{
    const sal_uInt32 nCurOrd = pCurrent ? pCurrent->GetOrdNumDirect() : 0;
    pCurrent = 0;
    if ( pPage->GetSortedObjs() )
    {
        const SwSortedObjs *pObjs = pPage->GetSortedObjs();
        if ( pObjs->size() )
        {
            sal_uInt32 nOrd = 0;
            (*pObjs)[0]->GetDrawObj()->GetOrdNum();  // force updating
            for ( size_t i = 0; i < pObjs->size(); ++i )
            {
                const SdrObject* pObj = (*pObjs)[i]->GetDrawObj();
                if ( bFlysOnly && !pObj->ISA(SwVirtFlyDrawObj) )
                    continue;
                sal_uInt32 nTmp = pObj->GetOrdNumDirect();
                if ( nTmp < nCurOrd && nTmp >= nOrd )
                {
                    nOrd = nTmp;
                    pCurrent = pObj;
                }
            }
        }
    }
    return pCurrent;
}

/// Keep and restore the substructure of a layout frame for an action.
// New algorithm:
//   Do not look at each neighbor one by one to set all pointers correctly.
//   It is sufficient to detach a part of a chain and check if another chain needs to be added
//   when attaching it again. Only the pointers necessary for the chain connection need to be
//   adjusted. The correction happens in RestoreContent(). In between all access is restricted.
//   During this action, the Flys are detached from the page.

// #115759# - 'remove' also drawing object from page and
// at-fly anchored objects from page
static void lcl_RemoveObjsFromPage( SwFrm* _pFrm )
{
    OSL_ENSURE( _pFrm->GetDrawObjs(), "no DrawObjs in lcl_RemoveObjsFromPage." );
    SwSortedObjs &rObjs = *_pFrm->GetDrawObjs();
    for ( size_t i = 0; i < rObjs.size(); ++i )
    {
        SwAnchoredObject* pObj = rObjs[i];
        // #115759# - reset member, at which the anchored
        // object orients its vertical position
        pObj->ClearVertPosOrientFrm();
        // #i43913#
        pObj->ResetLayoutProcessBools();
        // #115759# - remove also lower objects of as-character
        // anchored Writer fly frames from page
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);

            // #115759# - remove also direct lowers of Writer
            // fly frame from page
            if ( pFlyFrm->GetDrawObjs() )
            {
                ::lcl_RemoveObjsFromPage( pFlyFrm );
            }

            SwContentFrm* pCnt = pFlyFrm->ContainsContent();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_RemoveObjsFromPage( pCnt );
                pCnt = pCnt->GetNextContentFrm();
            }
            if ( pFlyFrm->IsFlyFreeFrm() )
            {
                // #i28701# - use new method <GetPageFrm()>
                pFlyFrm->GetPageFrm()->RemoveFlyFromPage( pFlyFrm );
            }
        }
        // #115759# - remove also drawing objects from page
        else if ( pObj->ISA(SwAnchoredDrawObject) )
        {
            if (pObj->GetFrameFormat().GetAnchor().GetAnchorId() != FLY_AS_CHAR)
            {
                pObj->GetPageFrm()->RemoveDrawObjFromPage(
                                *(static_cast<SwAnchoredDrawObject*>(pObj)) );
            }
        }
    }
}

SwFrm *SaveContent( SwLayoutFrm *pLay, SwFrm *pStart )
{
    if( pLay->IsSctFrm() && pLay->Lower() && pLay->Lower()->IsColumnFrm() )
        sw_RemoveFootnotes( static_cast<SwColumnFrm*>(pLay->Lower()), true, true );

    SwFrm *pSav;
    if ( 0 == (pSav = pLay->ContainsAny()) )
        return 0;

    if( pSav->IsInFootnote() && !pLay->IsInFootnote() )
    {
        do
            pSav = pSav->FindNext();
        while( pSav && pSav->IsInFootnote() );
        if( !pSav || !pLay->IsAnLower( pSav ) )
            return NULL;
    }

    // Tables should be saved as a whole, expection:
    // The contents of a section or a cell inside a table should be saved
    if ( pSav->IsInTab() && !( ( pLay->IsSctFrm() || pLay->IsCellFrm() ) && pLay->IsInTab() ) )
        while ( !pSav->IsTabFrm() )
            pSav = pSav->GetUpper();

    if( pSav->IsInSct() )
    { // search the upmost section inside of pLay
        SwFrm* pSect = pLay->FindSctFrm();
        SwFrm *pTmp = pSav;
        do
        {
            pSav = pTmp;
            pTmp = (pSav && pSav->GetUpper()) ? pSav->GetUpper()->FindSctFrm() : NULL;
        } while ( pTmp != pSect );
    }

    SwFrm *pFloat = pSav;
    if( !pStart )
        pStart = pSav;
    bool bGo = pStart == pSav;
    do
    {
        if( bGo )
            pFloat->GetUpper()->m_pLower = nullptr; // detach the chain part

        // search the end of the chain part, remove Flys on the way
        do
        {
            if( bGo )
            {
                if ( pFloat->IsContentFrm() )
                {
                    if ( pFloat->GetDrawObjs() )
                        ::lcl_RemoveObjsFromPage( static_cast<SwContentFrm*>(pFloat) );
                }
                else if ( pFloat->IsTabFrm() || pFloat->IsSctFrm() )
                {
                    SwContentFrm *pCnt = static_cast<SwLayoutFrm*>(pFloat)->ContainsContent();
                    if( pCnt )
                    {
                        do
                        {   if ( pCnt->GetDrawObjs() )
                                ::lcl_RemoveObjsFromPage( pCnt );
                            pCnt = pCnt->GetNextContentFrm();
                        } while ( pCnt && static_cast<SwLayoutFrm*>(pFloat)->IsAnLower( pCnt ) );
                    }
                }
                else {
                    OSL_ENSURE( !pFloat, "new FloatFrame?" );
                }
            }
            if ( pFloat->GetNext()  )
            {
                if( bGo )
                    pFloat->mpUpper = NULL;
                pFloat = pFloat->GetNext();
                if( !bGo && pFloat == pStart )
                {
                    bGo = true;
                    pFloat->mpPrev->mpNext = NULL;
                    pFloat->mpPrev = NULL;
                }
            }
            else
                break;

        } while ( pFloat );

        // search next chain part and connect both chains
        SwFrm *pTmp = pFloat->FindNext();
        if( bGo )
            pFloat->mpUpper = NULL;

        if( !pLay->IsInFootnote() )
            while( pTmp && pTmp->IsInFootnote() )
                pTmp = pTmp->FindNext();

        if ( !pLay->IsAnLower( pTmp ) )
            pTmp = 0;

        if ( pTmp && bGo )
        {
            pFloat->mpNext = pTmp; // connect both chains
            pFloat->mpNext->mpPrev = pFloat;
        }
        pFloat = pTmp;
        bGo = bGo || ( pStart == pFloat );
    }  while ( pFloat );

    return bGo ? pStart : NULL;
}

// #115759# - add also drawing objects to page and at-fly
// anchored objects to page
static void lcl_AddObjsToPage( SwFrm* _pFrm, SwPageFrm* _pPage )
{
    OSL_ENSURE( _pFrm->GetDrawObjs(), "no DrawObjs in lcl_AddObjsToPage." );
    SwSortedObjs &rObjs = *_pFrm->GetDrawObjs();
    for ( size_t i = 0; i < rObjs.size(); ++i )
    {
        SwAnchoredObject* pObj = rObjs[i];

        // #115759# - unlock position of anchored object
        // in order to get the object's position calculated.
        pObj->UnlockPosition();
        // #115759# - add also lower objects of as-character
        // anchored Writer fly frames from page
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm = static_cast<SwFlyFrm*>(pObj);
            if ( pObj->ISA(SwFlyFreeFrm) )
            {
                _pPage->AppendFlyToPage( pFlyFrm );
            }
            pFlyFrm->_InvalidatePos();
            pFlyFrm->_InvalidateSize();
            pFlyFrm->InvalidatePage( _pPage );

            // #115759# - add also at-fly anchored objects
            // to page
            if ( pFlyFrm->GetDrawObjs() )
            {
                ::lcl_AddObjsToPage( pFlyFrm, _pPage );
            }

            SwContentFrm *pCnt = pFlyFrm->ContainsContent();
            while ( pCnt )
            {
                if ( pCnt->GetDrawObjs() )
                    ::lcl_AddObjsToPage( pCnt, _pPage );
                pCnt = pCnt->GetNextContentFrm();
            }
        }
        // #115759# - remove also drawing objects from page
        else if ( pObj->ISA(SwAnchoredDrawObject) )
        {
            if (pObj->GetFrameFormat().GetAnchor().GetAnchorId() != FLY_AS_CHAR)
            {
                pObj->InvalidateObjPos();
                _pPage->AppendDrawObjToPage(
                                *(static_cast<SwAnchoredDrawObject*>(pObj)) );
            }
        }
    }
}

void RestoreContent( SwFrm *pSav, SwLayoutFrm *pParent, SwFrm *pSibling, bool bGrow )
{
    OSL_ENSURE( pSav && pParent, "no Save or Parent provided for RestoreContent." );
    SWRECTFN( pParent )

    // If there are already FlowFrms below the new parent, so add the chain (starting with pSav)
    // after the last one. The parts are inserted and invalidated if needed.
    // On the way, the Flys of the ContentFrms are registered at the page.

    SwPageFrm *pPage = pParent->FindPageFrm();

    if ( pPage )
        pPage->InvalidatePage( pPage );

    // determine predecessor and establish connection or initialize
    pSav->mpPrev = pSibling;
    SwFrm* pNxt;
    if ( pSibling )
    {
        pNxt = pSibling->mpNext;
        pSibling->mpNext = pSav;
        pSibling->_InvalidatePrt();
        pSibling->InvalidatePage( pPage );
        SwFlowFrm *pFlowFrm = dynamic_cast<SwFlowFrm*>(pSibling);
        if (pFlowFrm && pFlowFrm->GetFollow())
            pSibling->Prepare( PREP_CLEAR, 0, false );
    }
    else
    {   pNxt = pParent->m_pLower;
        pParent->m_pLower = pSav;
        pSav->mpUpper = pParent; // set here already, so that it is explicit when invalidating

        if ( pSav->IsContentFrm() )
            static_cast<SwContentFrm*>(pSav)->InvalidatePage( pPage );
        else
        {   // pSav might be an empty SectFrm
            SwContentFrm* pCnt = pParent->ContainsContent();
            if( pCnt )
                pCnt->InvalidatePage( pPage );
        }
    }

    // the parent needs to grow appropriately
    SwTwips nGrowVal = 0;
    SwFrm* pLast;
    do
    {   pSav->mpUpper = pParent;
        nGrowVal += (pSav->Frm().*fnRect->fnGetHeight)();
        pSav->_InvalidateAll();

        // register Flys, if TextFrms than also invalidate appropriately
        if ( pSav->IsContentFrm() )
        {
            if ( pSav->IsTextFrm() &&
                 static_cast<SwTextFrm*>(pSav)->GetCacheIdx() != USHRT_MAX )
                static_cast<SwTextFrm*>(pSav)->Init();  // I am its friend

            if ( pPage && pSav->GetDrawObjs() )
                ::lcl_AddObjsToPage( static_cast<SwContentFrm*>(pSav), pPage );
        }
        else
        {   SwContentFrm *pBlub = static_cast<SwLayoutFrm*>(pSav)->ContainsContent();
            if( pBlub )
            {
                do
                {   if ( pPage && pBlub->GetDrawObjs() )
                        ::lcl_AddObjsToPage( pBlub, pPage );
                    if( pBlub->IsTextFrm() && static_cast<SwTextFrm*>(pBlub)->HasFootnote() &&
                         static_cast<SwTextFrm*>(pBlub)->GetCacheIdx() != USHRT_MAX )
                        static_cast<SwTextFrm*>(pBlub)->Init(); // I am its friend
                    pBlub = pBlub->GetNextContentFrm();
                } while ( pBlub && static_cast<SwLayoutFrm*>(pSav)->IsAnLower( pBlub ));
            }
        }
        pLast = pSav;
        pSav = pSav->GetNext();

    } while ( pSav );

    if( pNxt )
    {
        pLast->mpNext = pNxt;
        pNxt->mpPrev = pLast;
    }

    if ( bGrow )
        pParent->Grow( nGrowVal );
}

SwPageFrm * InsertNewPage( SwPageDesc &rDesc, SwFrm *pUpper,
                          bool bOdd, bool bFirst, bool bInsertEmpty, bool bFootnote,
                          SwFrm *pSibling )
{
    SwPageFrm *pRet;
    SwDoc *pDoc = static_cast<SwLayoutFrm*>(pUpper)->GetFormat()->GetDoc();
    if (bFirst)
    {
        if (rDesc.IsFirstShared())
        {
            // We need to fallback to left or right page format, decide it now.
            // FIXME: is this still needed?
            if (bOdd)
            {
                rDesc.GetFirstMaster().SetFormatAttr( rDesc.GetMaster().GetHeader() );
                rDesc.GetFirstMaster().SetFormatAttr( rDesc.GetMaster().GetFooter() );
                // fdo#60250 copy margins for mirrored pages
                rDesc.GetFirstMaster().SetFormatAttr( rDesc.GetMaster().GetLRSpace() );
            }
            else
            {
                rDesc.GetFirstLeft().SetFormatAttr( rDesc.GetLeft().GetHeader() );
                rDesc.GetFirstLeft().SetFormatAttr( rDesc.GetLeft().GetFooter() );
                rDesc.GetFirstLeft().SetFormatAttr( rDesc.GetLeft().GetLRSpace() );
            }
        }
    }
    SwFrameFormat *pFormat(bOdd ? rDesc.GetRightFormat(bFirst) : rDesc.GetLeftFormat(bFirst));
    // If there is no FrameFormat for this page, add an empty page
    if ( !pFormat )
    {
        pFormat = bOdd ? rDesc.GetLeftFormat() : rDesc.GetRightFormat();
        OSL_ENSURE( pFormat, "Descriptor without any format?!" );
        bInsertEmpty = !bInsertEmpty;
    }
    if( bInsertEmpty )
    {
        SwPageDesc *pTmpDesc = pSibling && pSibling->GetPrev() ?
                static_cast<SwPageFrm*>(pSibling->GetPrev())->GetPageDesc() : &rDesc;
        pRet = new SwPageFrm( pDoc->GetEmptyPageFormat(), pUpper, pTmpDesc );
        pRet->Paste( pUpper, pSibling );
        pRet->PreparePage( bFootnote );
    }
    pRet = new SwPageFrm( pFormat, pUpper, &rDesc );
    pRet->Paste( pUpper, pSibling );
    pRet->PreparePage( bFootnote );
    if ( pRet->GetNext() )
        SwRootFrm::AssertPageFlys( pRet );
    return pRet;
}

/* The following two methods search the layout structure recursively and
 * register all Flys at the page that have a Frm in this structure as an anchor.
 */

static void lcl_Regist( SwPageFrm *pPage, const SwFrm *pAnch )
{
    SwSortedObjs *pObjs = const_cast<SwSortedObjs*>(pAnch->GetDrawObjs());
    for ( size_t i = 0; i < pObjs->size(); ++i )
    {
        SwAnchoredObject* pObj = (*pObjs)[i];
        if ( pObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pObj);
            // register (not if already known)
            // #i28701# - use new method <GetPageFrm()>
            SwPageFrm *pPg = pFly->IsFlyFreeFrm()
                             ? pFly->GetPageFrm() : pFly->FindPageFrm();
            if ( pPg != pPage )
            {
                if ( pPg )
                    pPg->RemoveFlyFromPage( pFly );
                pPage->AppendFlyToPage( pFly );
            }
            ::RegistFlys( pPage, pFly );
        }
        else
        {
            // #i87493#
            if ( pPage != pObj->GetPageFrm() )
            {
                // #i28701#
                if ( pObj->GetPageFrm() )
                    pObj->GetPageFrm()->RemoveDrawObjFromPage( *pObj );
                pPage->AppendDrawObjToPage( *pObj );
            }
        }

        const SwFlyFrm* pFly = pAnch->FindFlyFrm();
        if ( pFly &&
             pObj->GetDrawObj()->GetOrdNum() < pFly->GetVirtDrawObj()->GetOrdNum() &&
             pObj->GetDrawObj()->GetPage() )
        {
            //#i119945# set pFly's OrdNum to pObj's. So when pFly is removed by Undo, the original OrdNum will not be changed.
            pObj->DrawObj()->GetPage()->SetObjectOrdNum( pFly->GetVirtDrawObj()->GetOrdNumDirect(),
                                                         pObj->GetDrawObj()->GetOrdNumDirect() );
        }
    }
}

void RegistFlys( SwPageFrm *pPage, const SwLayoutFrm *pLay )
{
    if ( pLay->GetDrawObjs() )
        ::lcl_Regist( pPage, pLay );
    const SwFrm *pFrm = pLay->Lower();
    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
            ::RegistFlys( pPage, static_cast<const SwLayoutFrm*>(pFrm) );
        else if ( pFrm->GetDrawObjs() )
            ::lcl_Regist( pPage, pFrm );
        pFrm = pFrm->GetNext();
    }
}

/// Notify the background based on the difference between old and new rectangle
void Notify( SwFlyFrm *pFly, SwPageFrm *pOld, const SwRect &rOld,
             const SwRect* pOldPrt )
{
    const SwRect aFrm( pFly->GetObjRectWithSpaces() );
    if ( rOld.Pos() != aFrm.Pos() )
    {   // changed position, invalidate old and new area
        if ( rOld.HasArea() &&
             rOld.Left()+pFly->GetFormat()->GetLRSpace().GetLeft() < FAR_AWAY )
        {
            pFly->NotifyBackground( pOld, rOld, PREP_FLY_LEAVE );
        }
        pFly->NotifyBackground( pFly->FindPageFrm(), aFrm, PREP_FLY_ARRIVE );
    }
    else if ( rOld.SSize() != aFrm.SSize() )
    {   // changed size, invalidate the area that was left or is now overlapped
        // For simplicity, we purposely invalidate a Twip even if not needed.

        SwViewShell *pSh = pFly->getRootFrm()->GetCurrShell();
        if( pSh && rOld.HasArea() )
            pSh->InvalidateWindows( rOld );

        // #i51941# - consider case that fly frame isn't
        // registered at the old page <pOld>
        SwPageFrm* pPageFrm = pFly->FindPageFrm();
        if ( pOld != pPageFrm )
        {
            pFly->NotifyBackground( pPageFrm, aFrm, PREP_FLY_ARRIVE );
        }

        if ( rOld.Left() != aFrm.Left() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Left(  std::min(aFrm.Left(), rOld.Left()) );
            aTmp.Right( std::max(aFrm.Left(), rOld.Left()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        SwTwips nOld = rOld.Right();
        SwTwips nNew = aFrm.Right();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Left(  std::min(nNew, nOld) );
            aTmp.Right( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        if ( rOld.Top() != aFrm.Top() )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Top(    std::min(aFrm.Top(), rOld.Top()) );
            aTmp.Bottom( std::max(aFrm.Top(), rOld.Top()) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
        nOld = rOld.Bottom();
        nNew = aFrm.Bottom();
        if ( nOld != nNew )
        {
            SwRect aTmp( rOld );
            aTmp.Union( aFrm );
            aTmp.Top(    std::min(nNew, nOld) );
            aTmp.Bottom( std::max(nNew, nOld) );
            pFly->NotifyBackground( pOld, aTmp, PREP_FLY_CHGD );
        }
    }
    else if ( pOldPrt && *pOldPrt != pFly->Prt() &&
              pFly->GetFormat()->GetSurround().IsContour() )
    {
        // #i24097#
        pFly->NotifyBackground( pFly->FindPageFrm(), aFrm, PREP_FLY_ARRIVE );
    }
}

static void lcl_CheckFlowBack( SwFrm* pFrm, const SwRect &rRect )
{
    SwTwips nBottom = rRect.Bottom();
    while( pFrm )
    {
        if( pFrm->IsLayoutFrm() )
        {
            if( rRect.IsOver( pFrm->Frm() ) )
                lcl_CheckFlowBack( static_cast<SwLayoutFrm*>(pFrm)->Lower(), rRect );
        }
        else if( !pFrm->GetNext() && nBottom > pFrm->Frm().Bottom() )
        {
            if( pFrm->IsContentFrm() && static_cast<SwContentFrm*>(pFrm)->HasFollow() )
                pFrm->InvalidateSize();
            else
                pFrm->InvalidateNextPos();
        }
        pFrm = pFrm->GetNext();
    }
}

static void lcl_NotifyContent( const SdrObject *pThis, SwContentFrm *pCnt,
    const SwRect &rRect, const PrepareHint eHint )
{
    if ( pCnt->IsTextFrm() )
    {
        SwRect aCntPrt( pCnt->Prt() );
        aCntPrt.Pos() += pCnt->Frm().Pos();
        if ( eHint == PREP_FLY_ATTR_CHG )
        {
            // #i35640# - use given rectangle <rRect> instead
            // of current bound rectangle
            if ( aCntPrt.IsOver( rRect ) )
                pCnt->Prepare( PREP_FLY_ATTR_CHG );
        }
        // #i23129# - only invalidate, if the text frame
        // printing area overlaps with the given rectangle.
        else if ( aCntPrt.IsOver( rRect ) )
            pCnt->Prepare( eHint, static_cast<void*>(&aCntPrt._Intersection( rRect )) );
        if ( pCnt->GetDrawObjs() )
        {
            const SwSortedObjs &rObjs = *pCnt->GetDrawObjs();
            for ( size_t i = 0; i < rObjs.size(); ++i )
            {
                SwAnchoredObject* pObj = rObjs[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pObj);
                    if ( pFly->IsFlyInCntFrm() )
                    {
                        SwContentFrm *pContent = pFly->ContainsContent();
                        while ( pContent )
                        {
                            ::lcl_NotifyContent( pThis, pContent, rRect, eHint );
                            pContent = pContent->GetNextContentFrm();
                        }
                    }
                }
            }
        }
    }
}

void Notify_Background( const SdrObject* pObj,
                        SwPageFrm* pPage,
                        const SwRect& rRect,
                        const PrepareHint eHint,
                        const bool bInva )
{
    // If the frame was positioned correctly for the first time, do not inform the old area
    if ( eHint == PREP_FLY_LEAVE && rRect.Top() == FAR_AWAY )
         return;

    SwLayoutFrm* pArea;
    SwFlyFrm *pFlyFrm = 0;
    SwFrm* pAnchor;
    if( pObj->ISA(SwVirtFlyDrawObj) )
    {
        pFlyFrm = const_cast<SwVirtFlyDrawObj*>(static_cast<const SwVirtFlyDrawObj*>(pObj))->GetFlyFrm();
        pAnchor = pFlyFrm->AnchorFrm();
    }
    else
    {
        pFlyFrm = NULL;
        pAnchor = const_cast<SwFrm*>(
                    GetUserCall(pObj)->GetAnchoredObj( pObj )->GetAnchorFrm() );
    }
    if( PREP_FLY_LEAVE != eHint && pAnchor->IsInFly() )
        pArea = pAnchor->FindFlyFrm();
    else
        pArea = pPage;
    SwContentFrm *pCnt = 0;
    if ( pArea )
    {
        if( PREP_FLY_ARRIVE != eHint )
            lcl_CheckFlowBack( pArea, rRect );

        // Only the Flys following this anchor are reacting. Thus, those do not
        // need to be processed.
        // An exception is LEAVE, since the Fly might come "from above".
        // If the anchor is positioned on the previous page, the whole page
        // needs to be processed (47722).
        // OD 2004-05-13 #i28701# - If the wrapping style has to be considered
        // on the object positioning, the complete area has to be processed,
        // because content frames before the anchor frame also have to consider
        // the object for the text wrapping.
        // #i3317# - The complete area has always been
        // processed.
        {
            pCnt = pArea->ContainsContent();
        }
    }
    SwFrm *pLastTab = 0;

    while ( pCnt && pArea && pArea->IsAnLower( pCnt ) )
    {
        ::lcl_NotifyContent( pObj, pCnt, rRect, eHint );
        if ( pCnt->IsInTab() )
        {
            SwLayoutFrm* pCell = pCnt->GetUpper();
            // #i40606# - use <GetLastBoundRect()>
            // instead of <GetCurrentBoundRect()>, because a recalculation
            // of the bounding rectangle isn't intended here.
            if ( pCell->IsCellFrm() &&
                 ( pCell->Frm().IsOver( pObj->GetLastBoundRect() ) ||
                   pCell->Frm().IsOver( rRect ) ) )
            {
                const SwFormatVertOrient &rOri = pCell->GetFormat()->GetVertOrient();
                if ( text::VertOrientation::NONE != rOri.GetVertOrient() )
                    pCell->InvalidatePrt();
            }
            SwTabFrm *pTab = pCnt->FindTabFrm();
            if ( pTab != pLastTab )
            {
                pLastTab = pTab;
                // #i40606# - use <GetLastBoundRect()>
                // instead of <GetCurrentBoundRect()>, because a recalculation
                // of the bounding rectangle isn't intended here.
                if ( pTab->Frm().IsOver( pObj->GetLastBoundRect() ) ||
                     pTab->Frm().IsOver( rRect ) )
                {
                    if ( !pFlyFrm || !pFlyFrm->IsLowerOf( pTab ) )
                        pTab->InvalidatePrt();
                }
            }
        }
        pCnt = pCnt->GetNextContentFrm();
    }
    // #128702# - make code robust
    if ( pPage && pPage->GetSortedObjs() )
    {
        pObj->GetOrdNum();
        const SwSortedObjs &rObjs = *pPage->GetSortedObjs();
        for ( size_t i = 0; i < rObjs.size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = rObjs[i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                if( pAnchoredObj->GetDrawObj() == pObj )
                    continue;
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                if ( pFly->Frm().Top() == FAR_AWAY )
                    continue;

                if ( !pFlyFrm ||
                        (!pFly->IsLowerOf( pFlyFrm ) &&
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() < pObj->GetOrdNumDirect()))
                {
                    pCnt = pFly->ContainsContent();
                    while ( pCnt )
                    {
                        ::lcl_NotifyContent( pObj, pCnt, rRect, eHint );
                        pCnt = pCnt->GetNextContentFrm();
                    }
                }
                if( pFly->IsFlyLayFrm() )
                {
                    if( pFly->Lower() && pFly->Lower()->IsColumnFrm() &&
                        pFly->Frm().Bottom() >= rRect.Top() &&
                        pFly->Frm().Top() <= rRect.Bottom() &&
                        pFly->Frm().Right() >= rRect.Left() &&
                        pFly->Frm().Left() <= rRect.Right() )
                     {
                        pFly->InvalidateSize();
                     }
                }
                // Flys above myself might sidestep if they have an automatic
                // alignment. This happens independently of my attributes since
                // this might have been changed as well.
                else if ( pFly->IsFlyAtCntFrm() &&
                        pObj->GetOrdNumDirect() <
                        pFly->GetVirtDrawObj()->GetOrdNumDirect() &&
                        pFlyFrm && !pFly->IsLowerOf( pFlyFrm ) )
                {
                    const SwFormatHoriOrient &rH = pFly->GetFormat()->GetHoriOrient();
                    if ( text::HoriOrientation::NONE != rH.GetHoriOrient()  &&
                            text::HoriOrientation::CENTER != rH.GetHoriOrient()  &&
                            ( !pFly->IsAutoPos() || text::RelOrientation::CHAR != rH.GetRelationOrient() ) &&
                            (pFly->Frm().Bottom() >= rRect.Top() &&
                            pFly->Frm().Top() <= rRect.Bottom()) )
                        pFly->InvalidatePos();
                }
            }
        }
    }
    if ( pFlyFrm && pAnchor->GetUpper() && pAnchor->IsInTab() )//MA_FLY_HEIGHT
        pAnchor->GetUpper()->InvalidateSize();

    // #i82258# - make code robust
    SwViewShell* pSh = 0;
    if ( bInva && pPage &&
        0 != (pSh = pPage->getRootFrm()->GetCurrShell()) )
    {
        pSh->InvalidateWindows( rRect );
    }
}

/// Provides the Upper of an anchor in paragraph-bound objects. If the latter
/// is a chained border or a footnote, the "virtual" Upper might be returne.
const SwFrm* GetVirtualUpper( const SwFrm* pFrm, const Point& rPos )
{
    if( pFrm->IsTextFrm() )
    {
        pFrm = pFrm->GetUpper();
        if( !pFrm->Frm().IsInside( rPos ) )
        {
            if( pFrm->IsFootnoteFrm() )
            {
                const SwFootnoteFrm* pTmp = static_cast<const SwFootnoteFrm*>(pFrm)->GetFollow();
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetFollow();
                }
            }
            else
            {
                SwFlyFrm* pTmp = const_cast<SwFlyFrm*>(pFrm->FindFlyFrm());
                while( pTmp )
                {
                    if( pTmp->Frm().IsInside( rPos ) )
                        return pTmp;
                    pTmp = pTmp->GetNextLink();
                }
            }
        }
    }
    return pFrm;
}

bool Is_Lower_Of( const SwFrm *pCurrFrm, const SdrObject* pObj )
{
    Point aPos;
    const SwFrm* pFrm;
    if( pObj->ISA(SwVirtFlyDrawObj) )
    {
        const SwFlyFrm* pFly = static_cast<const SwVirtFlyDrawObj*>(pObj )->GetFlyFrm();
        pFrm = pFly->GetAnchorFrm();
        aPos = pFly->Frm().Pos();
    }
    else
    {
        pFrm = static_cast<SwDrawContact*>(GetUserCall(pObj) )->GetAnchorFrm(pObj);
        aPos = pObj->GetCurrentBoundRect().TopLeft();
    }
    OSL_ENSURE( pFrm, "8-( Fly is lost in Space." );
    pFrm = GetVirtualUpper( pFrm, aPos );
    do
    {   if ( pFrm == pCurrFrm )
            return true;
        if( pFrm->IsFlyFrm() )
        {
            aPos = pFrm->Frm().Pos();
            pFrm = GetVirtualUpper( static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm(), aPos );
        }
        else
            pFrm = pFrm->GetUpper();
    } while ( pFrm );
    return false;
}

/// provides the area of a frame in that no Fly from another area can overlap
const SwFrm *FindKontext( const SwFrm *pFrm, sal_uInt16 nAdditionalContextType )
{
    const sal_uInt16 nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL |
                        nAdditionalContextType;
    do
    {   if ( pFrm->GetType() & nTyp )
            break;
        pFrm = pFrm->GetUpper();
    } while( pFrm );
    return pFrm;
}

bool IsFrmInSameKontext( const SwFrm *pInnerFrm, const SwFrm *pFrm )
{
    const SwFrm *pKontext = FindKontext( pInnerFrm, 0 );

    const sal_uInt16 nTyp = FRM_ROOT | FRM_HEADER   | FRM_FOOTER | FRM_FTNCONT  |
                        FRM_FTN  | FRM_FLY      |
                        FRM_TAB  | FRM_ROW      | FRM_CELL;
    do
    {   if ( pFrm->GetType() & nTyp )
        {
            if( pFrm == pKontext )
                return true;
            if( pFrm->IsCellFrm() )
                return false;
        }
        if( pFrm->IsFlyFrm() )
        {
            Point aPos( pFrm->Frm().Pos() );
            pFrm = GetVirtualUpper( static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm(), aPos );
        }
        else
            pFrm = pFrm->GetUpper();
    } while( pFrm );

    return false;
}

static SwTwips lcl_CalcCellRstHeight( SwLayoutFrm *pCell )
{
    if ( pCell->Lower()->IsContentFrm() || pCell->Lower()->IsSctFrm() )
    {
        SwFrm *pLow = pCell->Lower();
        long nHeight = 0, nFlyAdd = 0;
        do
        {
            long nLow = pLow->Frm().Height();
            if( pLow->IsTextFrm() && static_cast<SwTextFrm*>(pLow)->IsUndersized() )
                nLow += static_cast<SwTextFrm*>(pLow)->GetParHeight()-pLow->Prt().Height();
            else if( pLow->IsSctFrm() && static_cast<SwSectionFrm*>(pLow)->IsUndersized() )
                nLow += static_cast<SwSectionFrm*>(pLow)->Undersize();
            nFlyAdd = std::max( 0L, nFlyAdd - nLow );
            nFlyAdd = std::max( nFlyAdd, ::CalcHeightWithFlys( pLow ) );
            nHeight += nLow;
            pLow = pLow->GetNext();
        } while ( pLow );
        if ( nFlyAdd )
            nHeight += nFlyAdd;

        // The border cannot be calculated based on PrtArea and Frm, since both can be invalid.
        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
        const SwBorderAttrs &rAttrs = *aAccess.Get();
        nHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();

        return pCell->Frm().Height() - nHeight;
    }
    else
    {
        long nRstHeight = 0;
        SwFrm *pLow = pCell->Lower();
        while (pLow && pLow->IsLayoutFrm())
        {
            nRstHeight += ::CalcRowRstHeight(static_cast<SwLayoutFrm*>(pLow));
            pLow = pLow->GetNext();
        }
        return nRstHeight;
    }
}

SwTwips CalcRowRstHeight( SwLayoutFrm *pRow )
{
    SwTwips nRstHeight = LONG_MAX;
    SwFrm *pLow = pRow->Lower();
    while (pLow && pLow->IsLayoutFrm())
    {
        nRstHeight = std::min(nRstHeight, ::lcl_CalcCellRstHeight(static_cast<SwLayoutFrm*>(pLow)));
        pLow = pLow->GetNext();
    }
    return nRstHeight;
}

const SwFrm* FindPage( const SwRect &rRect, const SwFrm *pPage )
{
    if ( !rRect.IsOver( pPage->Frm() ) )
    {
        const SwRootFrm* pRootFrm = static_cast<const SwRootFrm*>(pPage->GetUpper());
        const SwFrm* pTmpPage = pRootFrm ? pRootFrm->GetPageAtPos( rRect.TopLeft(), &rRect.SSize(), true ) : 0;
        if ( pTmpPage )
            pPage = pTmpPage;
    }

    return pPage;
}

class SwFrmHolder : private SfxListener
{
    SwFrm* pFrm;
    bool bSet;
    virtual void Notify(  SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;
public:
    SwFrmHolder() : pFrm(0), bSet(false) {}
    void SetFrm( SwFrm* pHold );
    SwFrm* GetFrm() { return pFrm; }
    void Reset();
    bool IsSet() { return bSet; }
};

void SwFrmHolder::SetFrm( SwFrm* pHold )
{
    bSet = true;
    pFrm = pHold;
    StartListening(*pHold);
}

void SwFrmHolder::Reset()
{
    if (pFrm)
        EndListening(*pFrm);
    bSet = false;
    pFrm = 0;
}

void SwFrmHolder::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if ( pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING && &rBC == pFrm )
    {
            pFrm = 0;
    }
}

SwFrm* GetFrmOfModify( const SwRootFrm* pLayout, SwModify const& rMod, sal_uInt16 const nFrmType,
        const Point* pPoint, const SwPosition *pPos, const bool bCalcFrm )
{
    SwFrm *pMinFrm = 0, *pTmpFrm;
    SwFrmHolder aHolder;
    SwRect aCalcRect;
    bool bClientIterChanged = false;

    SwIterator<SwFrm,SwModify> aIter( rMod );
    do {
        pMinFrm = 0;
        aHolder.Reset();
        sal_uInt64 nMinDist = 0;
        bClientIterChanged = false;

        for( pTmpFrm = aIter.First(); pTmpFrm; pTmpFrm = aIter.Next() )
        {
            if( pTmpFrm->GetType() & nFrmType &&
                ( !pLayout || pLayout == pTmpFrm->getRootFrm() ) &&
                (!pTmpFrm->IsFlowFrm() ||
                 !SwFlowFrm::CastFlowFrm( pTmpFrm )->IsFollow() ))
            {
                if( pPoint )
                {
                    // watch for Frm being deleted
                    if ( pMinFrm )
                        aHolder.SetFrm( pMinFrm );
                    else
                        aHolder.Reset();

                    if( bCalcFrm )
                    {
                        // - format parent Writer
                        // fly frame, if it isn't been formatted yet.
                        // Note: The Writer fly frame could be the frame itself.
                        SwFlyFrm* pFlyFrm( pTmpFrm->FindFlyFrm() );
                        if ( pFlyFrm &&
                             pFlyFrm->Frm().Pos().X() == FAR_AWAY &&
                             pFlyFrm->Frm().Pos().Y() == FAR_AWAY )
                        {
                            SwObjectFormatter::FormatObj( *pFlyFrm );
                        }
                        pTmpFrm->Calc(pLayout ? pLayout->GetCurrShell()->GetOut() : 0);
                    }

                    // #127369#
                    // aIter.IsChanged checks if the current pTmpFrm has been deleted while
                    // it is the current iterator
                    // FrmHolder watches for deletion of the current pMinFrm
                    if( aIter.IsChanged() || ( aHolder.IsSet() && !aHolder.GetFrm() ) )
                    {
                        // restart iteration
                        bClientIterChanged = true;
                        break;
                    }

                    // for Flys go via the parent if the Fly is not yet "formatted"
                    if( !bCalcFrm && pTmpFrm->GetType() & FRM_FLY &&
                        static_cast<SwFlyFrm*>(pTmpFrm)->GetAnchorFrm() &&
                        FAR_AWAY == pTmpFrm->Frm().Pos().getX() &&
                        FAR_AWAY == pTmpFrm->Frm().Pos().getY() )
                        aCalcRect = static_cast<SwFlyFrm*>(pTmpFrm)->GetAnchorFrm()->Frm();
                    else
                        aCalcRect = pTmpFrm->Frm();

                    if ( aCalcRect.IsInside( *pPoint ) )
                    {
                        pMinFrm = pTmpFrm;
                        break;
                    }

                    // Point not in rectangle. Compare distances:
                    const Point aCalcRectCenter = aCalcRect.Center();
                    const Point aDiff = aCalcRectCenter - *pPoint;
                    const sal_uInt64 nCurrentDist = sal_Int64(aDiff.getX()) * sal_Int64(aDiff.getX()) + sal_Int64(aDiff.getY()) * sal_Int64(aDiff.getY()); // opt: no sqrt
                    if ( !pMinFrm || nCurrentDist < nMinDist )
                    {
                        pMinFrm = pTmpFrm;
                        nMinDist = nCurrentDist;
                    }
                }
                else
                {
                    // if no pPoint is provided, take the first one
                    pMinFrm = pTmpFrm;
                    break;
                }
            }
        }
    } while( bClientIterChanged );

    if( pPos && pMinFrm && pMinFrm->IsTextFrm() )
        return static_cast<SwTextFrm*>(pMinFrm)->GetFrmAtPos( *pPos );

    return pMinFrm;
}

bool IsExtraData( const SwDoc *pDoc )
{
    const SwLineNumberInfo &rInf = pDoc->GetLineNumberInfo();
    return rInf.IsPaintLineNumbers() ||
           rInf.IsCountInFlys() ||
           ((sal_Int16)SW_MOD()->GetRedlineMarkPos() != text::HoriOrientation::NONE &&
            !pDoc->getIDocumentRedlineAccess().GetRedlineTable().empty());
}

// OD 22.09.2003 #110978#
const SwRect SwPageFrm::PrtWithoutHeaderAndFooter() const
{
    SwRect aPrtWithoutHeaderFooter( Prt() );
    aPrtWithoutHeaderFooter.Pos() += Frm().Pos();

    const SwFrm* pLowerFrm = Lower();
    while ( pLowerFrm )
    {
        // Note: independent on text direction page header and page footer are
        //       always at top respectively at bottom of the page frame.
        if ( pLowerFrm->IsHeaderFrm() )
        {
            aPrtWithoutHeaderFooter.Top( aPrtWithoutHeaderFooter.Top() +
                                         pLowerFrm->Frm().Height() );
        }
        if ( pLowerFrm->IsFooterFrm() )
        {
            aPrtWithoutHeaderFooter.Bottom( aPrtWithoutHeaderFooter.Bottom() -
                                            pLowerFrm->Frm().Height() );
        }

        pLowerFrm = pLowerFrm->GetNext();
    }

    return aPrtWithoutHeaderFooter;
}

/** method to determine the spacing values of a frame

    OD 2004-03-10 #i28701#
    OD 2009-08-28 #i102458#
    Add output parameter <obIsLineSpacingProportional>
*/
void GetSpacingValuesOfFrm( const SwFrm& rFrm,
                            SwTwips& onLowerSpacing,
                            SwTwips& onLineSpacing,
                            bool& obIsLineSpacingProportional )
{
    if ( !rFrm.IsFlowFrm() )
    {
        onLowerSpacing = 0;
        onLineSpacing = 0;
    }
    else
    {
        const SvxULSpaceItem& rULSpace = rFrm.GetAttrSet()->GetULSpace();
        onLowerSpacing = rULSpace.GetLower();

        onLineSpacing = 0;
        obIsLineSpacingProportional = false;
        if ( rFrm.IsTextFrm() )
        {
            onLineSpacing = static_cast<const SwTextFrm&>(rFrm).GetLineSpace();
            obIsLineSpacingProportional =
                onLineSpacing != 0 &&
                static_cast<const SwTextFrm&>(rFrm).GetLineSpace( true ) == 0;
        }

        OSL_ENSURE( onLowerSpacing >= 0 && onLineSpacing >= 0,
                "<GetSpacingValuesOfFrm(..)> - spacing values aren't positive!" );
    }
}

/// get the content of the table cell, skipping content from nested tables
const SwContentFrm* GetCellContent( const SwLayoutFrm& rCell )
{
    const SwContentFrm* pContent = rCell.ContainsContent();
    const SwTabFrm* pTab = rCell.FindTabFrm();

    while ( pContent && rCell.IsAnLower( pContent ) )
    {
        const SwTabFrm* pTmpTab = pContent->FindTabFrm();
        if ( pTmpTab != pTab )
        {
            pContent = pTmpTab->FindLastContent();
            if ( pContent )

                pContent = pContent->FindNextCnt();

        }
        else
            break;
    }
    return pContent;
}

/// Can be used to check if a frame has been deleted
bool SwDeletionChecker::HasBeenDeleted()
{
    if ( !mpFrm || !mpRegIn )
        return false;

    SwIterator<SwFrm,SwModify> aIter(*mpRegIn);
    SwFrm* pLast = aIter.First();
    while ( pLast )
    {
        if ( pLast == mpFrm )
            return false;
        pLast = aIter.Next();
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
