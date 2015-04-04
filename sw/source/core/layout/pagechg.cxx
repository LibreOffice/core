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

#include <ndole.hxx>
#include <svl/itemiter.hxx>
#include <fmtfsize.hxx>
#include <fmthdft.hxx>
#include <fmtclds.hxx>
#include <fmtpdsc.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <ftninfo.hxx>
#include <tgrditem.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <docary.hxx>

#include "viewimp.hxx"
#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include "dcontact.hxx"
#include "hints.hxx"

#include "ftnidx.hxx"
#include "bodyfrm.hxx"
#include "ftnfrm.hxx"
#include "tabfrm.hxx"
#include "txtfrm.hxx"
#include "layact.hxx"
#include "flyfrms.hxx"
#include "htmltbl.hxx"
#include "pagedesc.hxx"
#include <editeng/frmdiritem.hxx>
#include <sortedobjs.hxx>
#include <calbck.hxx>
#include <txtfly.hxx>

using namespace ::com::sun::star;

SwBodyFrm::SwBodyFrm( SwFrmFmt *pFmt, SwFrm* pSib ):
    SwLayoutFrm( pFmt, pSib )
{
    mnFrmType = FRM_BODY;
}

void SwBodyFrm::Format( const SwBorderAttrs * )
{
    // Formatting of the body is too simple, thus, it gets an own format method.
    // Borders etc. are not taken into account here.
    // With is taken from the PrtArea of the Upper, height is the height of the
    // PrtArea of the Upper minus any neighbors (for robustness).
    // The PrtArea has always the size of the frame.

    if ( !mbValidSize )
    {
        SwTwips nHeight = GetUpper()->Prt().Height();
        SwTwips nWidth = GetUpper()->Prt().Width();
        const SwFrm *pFrm = GetUpper()->Lower();
        do
        {
            if ( pFrm != this )
            {
                if( pFrm->IsVertical() )
                    nWidth -= pFrm->Frm().Width();
                else
                    nHeight -= pFrm->Frm().Height();
            }
            pFrm = pFrm->GetNext();
        } while ( pFrm );
        if ( nHeight < 0 )
            nHeight = 0;
        Frm().Height( nHeight );

        if( IsVertical() && !IsVertLR() && !IsReverse() && nWidth != Frm().Width() )
            Frm().Pos().setX(Frm().Pos().getX() + Frm().Width() - nWidth);
        Frm().Width( nWidth );
    }

    bool bNoGrid = true;
    if( GetUpper()->IsPageFrm() && static_cast<SwPageFrm*>(GetUpper())->HasGrid() )
    {
        SwTextGridItem const*const pGrid(
                GetGridItem(static_cast<SwPageFrm*>(GetUpper())));
        if( pGrid )
        {
            bNoGrid = false;
            long nSum = pGrid->GetBaseHeight() + pGrid->GetRubyHeight();
            SWRECTFN( this )
            long nSize = (Frm().*fnRect->fnGetWidth)();
            long nBorder = 0;
            if( GRID_LINES_CHARS == pGrid->GetGridType() )
            {
                //for textgrid refactor
                SwDoc *pDoc = GetFmt()->GetDoc();
                nBorder = nSize % (GetGridWidth(*pGrid, *pDoc));
                nSize -= nBorder;
                nBorder /= 2;
            }
            (Prt().*fnRect->fnSetPosX)( nBorder );
            (Prt().*fnRect->fnSetWidth)( nSize );

            // Height of body frame:
            nBorder = (Frm().*fnRect->fnGetHeight)();

            // Number of possible lines in area of body frame:
            long nNumberOfLines = nBorder / nSum;
            if( nNumberOfLines > pGrid->GetLines() )
                nNumberOfLines = pGrid->GetLines();

            // Space required for nNumberOfLines lines:
            nSize = nNumberOfLines * nSum;
            nBorder -= nSize;
            nBorder /= 2;

            // #i21774# Footnotes and centering the grid does not work together:
            const bool bAdjust = static_cast<SwPageFrm*>(GetUpper())->GetFmt()->GetDoc()->
                                        GetFtnIdxs().empty();

            (Prt().*fnRect->fnSetPosY)( bAdjust ? nBorder : 0 );
            (Prt().*fnRect->fnSetHeight)( nSize );
        }
    }
    if( bNoGrid )
    {
        Prt().Pos().setX(0);
        Prt().Pos().setY(0);
        Prt().Height( Frm().Height() );
        Prt().Width( Frm().Width() );
    }
    mbValidSize = mbValidPrtArea = true;
}

SwPageFrm::SwPageFrm( SwFrmFmt *pFmt, SwFrm* pSib, SwPageDesc *pPgDsc ) :
    SwFtnBossFrm( pFmt, pSib ),
    pSortedObjs( 0 ),
    pDesc( pPgDsc ),
    nPhyPageNum( 0 )
{
    SetDerivedVert( false );
    SetDerivedR2L( false );
    if( pDesc )
    {
        bHasGrid = true;
        SwTextGridItem const*const pGrid(GetGridItem(this));
        if( !pGrid )
            bHasGrid = false;
    }
    else
        bHasGrid = false;
    SetMaxFtnHeight( pPgDsc->GetFtnInfo().GetHeight() ?
                     pPgDsc->GetFtnInfo().GetHeight() : LONG_MAX ),
    mnFrmType = FRM_PAGE;
    bInvalidLayout = bInvalidCntnt = bInvalidSpelling = bInvalidSmartTags = bInvalidAutoCmplWrds = bInvalidWordCount = true;
    bInvalidFlyLayout = bInvalidFlyCntnt = bInvalidFlyInCnt = bFtnPage = bEndNotePage = false;

    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    const bool bBrowseMode = pSh && pSh->GetViewOptions()->getBrowseMode();
    if ( bBrowseMode )
    {
        Frm().Height( 0 );
        long nWidth = pSh->VisArea().Width();
        if ( !nWidth )
            nWidth = 5000L;     //aendert sich sowieso
        Frm().Width ( nWidth );
    }
    else
        Frm().SSize( pFmt->GetFrmSize().GetSize() );

    // create and insert body area if it is not a blank page
    SwDoc *pDoc = pFmt->GetDoc();
    if ( false == (bEmptyPage = (pFmt == pDoc->GetEmptyPageFmt())) )
    {
        bEmptyPage = false;
        Calc();                     // so that the PrtArea is correct
        SwBodyFrm *pBodyFrm = new SwBodyFrm( pDoc->GetDfltFrmFmt(), this );
        pBodyFrm->ChgSize( Prt().SSize() );
        pBodyFrm->Paste( this );
        pBodyFrm->Calc();           // so that the columns can be inserted correctly
        pBodyFrm->InvalidatePos();

        if ( bBrowseMode )
            _InvalidateSize();

        // insert header/footer,, but only if active.
        if ( pFmt->GetHeader().IsActive() )
            PrepareHeader();
        if ( pFmt->GetFooter().IsActive() )
            PrepareFooter();

        const SwFmtCol &rCol = pFmt->GetCol();
        if ( rCol.GetNumCols() > 1 )
        {
            const SwFmtCol aOld; //ChgColumns() needs an old value
            pBodyFrm->ChgColumns( aOld, rCol );
        }
    }
}

SwPageFrm::~SwPageFrm()
{
    // Cleanup the header-footer controls in the SwEditWin
    SwViewShell* pSh = getRootFrm()->GetCurrShell();
    SwWrtShell* pWrtSh = dynamic_cast< SwWrtShell* >( pSh );
    if ( pWrtSh )
    {
        SwEditWin& rEditWin = pWrtSh->GetView().GetEditWin();
        rEditWin.GetFrameControlsManager( ).RemoveControls( this );
    }

    // empty FlyContainer, deletion of the Flys is done by the anchor (in base class SwFrm)
    if ( pSortedObjs )
    {
        // Objects can be anchored at pages that are before their anchors (why ever...).
        // In such cases, we would access already freed memory.
        for ( size_t i = 0; i < pSortedObjs->size(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pSortedObjs)[i];
            pAnchoredObj->SetPageFrm( 0L );
        }
        delete pSortedObjs;
        pSortedObjs = 0; // reset to zero to prevent problems when detaching the Flys
    }

    if ( !IsEmptyPage() ) //#59184# unnessesary for empty pages
    {
        // prevent access to destroyed pages
        SwDoc *pDoc = GetFmt() ? GetFmt()->GetDoc() : NULL;
        if( pDoc && !pDoc->IsInDtor() )
        {
            if ( pSh )
            {
                SwViewShellImp *pImp = pSh->Imp();
                pImp->SetFirstVisPageInvalid();
                if ( pImp->IsAction() )
                    pImp->GetLayAction().SetAgain();
                // OD 12.02.2003 #i9719#, #105645# - retouche area of page
                // including border and shadow area.
                const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
                SwRect aRetoucheRect;
                SwPageFrm::GetBorderAndShadowBoundRect( Frm(), pSh, aRetoucheRect, IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
                pSh->AddPaintRect( aRetoucheRect );
            }
        }
    }

    // Hack to make sure code called from base ~SwFtnBossFrm does not interpret
    // this as a SwPageFrm (which it no longer is by then):
    mnFrmType = FRM_UNUSED;
}

void SwPageFrm::CheckGrid( bool bInvalidate )
{
    bool bOld = bHasGrid;
    bHasGrid = true;
    SwTextGridItem const*const pGrid(GetGridItem(this));
    bHasGrid = 0 != pGrid;
    if( bInvalidate || bOld != bHasGrid )
    {
        SwLayoutFrm* pBody = FindBodyCont();
        if( pBody )
        {
            pBody->InvalidatePrt();
            SwCntntFrm* pFrm = pBody->ContainsCntnt();
            while( pBody->IsAnLower( pFrm ) )
            {
                static_cast<SwTxtFrm*>(pFrm)->Prepare( PREP_CLEAR );
                pFrm = pFrm->GetNextCntntFrm();
            }
        }
        SetCompletePaint();
    }
}

void SwPageFrm::CheckDirection( bool bVert )
{
    sal_uInt16 nDir =
            static_cast<const SvxFrameDirectionItem&>(GetFmt()->GetFmtAttr( RES_FRAMEDIR )).GetValue();
    if( bVert )
    {
        if( FRMDIR_HORI_LEFT_TOP == nDir || FRMDIR_HORI_RIGHT_TOP == nDir )
        {
            mbVertLR = false;
            mbVertical = false;
        }
        else
        {
            const SwViewShell *pSh = getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                mbVertLR = false;
                mbVertical = false;
            }
            else
            {
                mbVertical = true;

                if(FRMDIR_VERT_TOP_RIGHT == nDir)
                    mbVertLR = false;
                    else if(FRMDIR_VERT_TOP_LEFT==nDir)
                       mbVertLR = true;
            }
        }

        mbReverse = false;
        mbInvalidVert = false;
    }
    else
    {
        if( FRMDIR_HORI_RIGHT_TOP == nDir )
            mbRightToLeft = true;
        else
            mbRightToLeft = false;
        mbInvalidR2L = false;
    }
}

/// create specific Flys for this page and format generic content
static void lcl_FormatLay( SwLayoutFrm *pLay )
{
    // format all LayoutFrms - no tables, Flys etc.

    SwFrm *pTmp = pLay->Lower();
    // first the low-level ones
    while ( pTmp )
    {
        if ( pTmp->GetType() & 0x00FF )
            ::lcl_FormatLay( static_cast<SwLayoutFrm*>(pTmp) );
        pTmp = pTmp->GetNext();
    }
    pLay->Calc();
}

/// Create Flys or register draw objects
static void lcl_MakeObjs( const SwFrmFmts &rTbl, SwPageFrm *pPage )
{
    // formats are in the special table of the document

    for ( size_t i = 0; i < rTbl.size(); ++i )
    {
        SwFrmFmt *pFmt = rTbl[i];
        const SwFmtAnchor &rAnch = pFmt->GetAnchor();
        if ( rAnch.GetPageNum() == pPage->GetPhyPageNum() )
        {
            if( rAnch.GetCntntAnchor() )
            {
                if (FLY_AT_PAGE == rAnch.GetAnchorId())
                {
                    SwFmtAnchor aAnch( rAnch );
                    aAnch.SetAnchor( 0 );
                    pFmt->SetFmtAttr( aAnch );
                }
                else
                    continue;
            }

            // is it a border or a SdrObject?
            bool bSdrObj = RES_DRAWFRMFMT == pFmt->Which();
            SdrObject *pSdrObj = 0;
            if ( bSdrObj  && 0 == (pSdrObj = pFmt->FindSdrObject()) )
            {
                OSL_FAIL( "DrawObject not found." );
                pFmt->GetDoc()->DelFrmFmt( pFmt );
                --i;
                continue;
            }
            // The object might be anchored to another page, e.g. when inserting
            // a new page due to a page descriptor change. In such cases, the
            // object needs to be moved.
            // In some cases the object is already anchored to the correct page.
            // This will be handled here and does not need to be coded extra.
            SwPageFrm *pPg = pPage->IsEmptyPage() ? static_cast<SwPageFrm*>(pPage->GetNext()) : pPage;
            if ( bSdrObj )
            {
                // OD 23.06.2003 #108784# - consider 'virtual' drawing objects
                SwDrawContact *pContact =
                            static_cast<SwDrawContact*>(::GetUserCall(pSdrObj));
                if ( pSdrObj->ISA(SwDrawVirtObj) )
                {
                    SwDrawVirtObj* pDrawVirtObj = static_cast<SwDrawVirtObj*>(pSdrObj);
                    if ( pContact )
                    {
                        pDrawVirtObj->RemoveFromWriterLayout();
                        pDrawVirtObj->RemoveFromDrawingPage();
                        pPg->AppendDrawObj( *(pContact->GetAnchoredObj( pDrawVirtObj )) );
                    }
                }
                else
                {
                    if ( pContact->GetAnchorFrm() )
                        pContact->DisconnectFromLayout( false );
                    pPg->AppendDrawObj( *(pContact->GetAnchoredObj( pSdrObj )) );
                }
            }
            else
            {
                SwIterator<SwFlyFrm,SwFmt> aIter( *pFmt );
                SwFlyFrm *pFly = aIter.First();
                if ( pFly)
                {
                    if( pFly->GetAnchorFrm() )
                        pFly->AnchorFrm()->RemoveFly( pFly );
                }
                else
                    pFly = new SwFlyLayFrm( static_cast<SwFlyFrmFmt*>(pFmt), pPg, pPg );
                pPg->AppendFly( pFly );
                ::RegistFlys( pPg, pFly );
            }
        }
    }
}

void SwPageFrm::PreparePage( bool bFtn )
{
    SetFtnPage( bFtn );

    // #i82258#
    // Due to made change on OOo 2.0 code line, method <::lcl_FormatLay(..)> has
    // the side effect, that the content of page header and footer are formatted.
    // For this formatting it is needed that the anchored objects are registered
    // at the <SwPageFrm> instance.
    // Thus, first calling <::RegistFlys(..)>, then call <::lcl_FormatLay(..)>
    ::RegistFlys( this, this );

    if ( Lower() )
    {
                ::lcl_FormatLay( this );
    }

    // Flys and draw objects that are still attached to the document.
    // Footnote pages do not have page-bound Flys!
    // There might be Flys or draw objects that want to be placed on
    // empty pages, however, the empty pages ignore that and the following
    // pages take care of them.
    if ( !bFtn && !IsEmptyPage() )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();

        if ( GetPrev() && static_cast<SwPageFrm*>(GetPrev())->IsEmptyPage() )
            lcl_MakeObjs( *pDoc->GetSpzFrmFmts(), static_cast<SwPageFrm*>(GetPrev()) );
        lcl_MakeObjs( *pDoc->GetSpzFrmFmts(), this );

        // format footer/ header
        SwLayoutFrm *pLow = static_cast<SwLayoutFrm*>(Lower());
        while ( pLow )
        {
            if ( pLow->GetType() & (FRM_HEADER|FRM_FOOTER) )
            {
                SwCntntFrm *pCntnt = pLow->ContainsCntnt();
                while ( pCntnt && pLow->IsAnLower( pCntnt ) )
                {
                    pCntnt->OptCalc();  // not the predecessors
                    pCntnt = pCntnt->GetNextCntntFrm();
                }
            }
            pLow = static_cast<SwLayoutFrm*>(pLow->GetNext());
        }
    }
}

void SwPageFrm::Modify( const SfxPoolItem* pOld, const SfxPoolItem * pNew )
{
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();
    sal_uInt8 nInvFlags = 0;

    if( pNew && RES_ATTRSET_CHG == pNew->Which() )
    {
        SfxItemIter aNIter( *static_cast<const SwAttrSetChg*>(pNew)->GetChgSet() );
        SfxItemIter aOIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *static_cast<const SwAttrSetChg*>(pOld) );
        SwAttrSetChg aNewSet( *static_cast<const SwAttrSetChg*>(pNew) );
        while( true )
        {
            _UpdateAttr( aOIter.GetCurItem(),
                         aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        InvalidatePage( this );
        if ( nInvFlags & 0x01 )
            _InvalidatePrt();
        if ( nInvFlags & 0x02 )
            SetCompletePaint();
        if ( nInvFlags & 0x04 && GetNext() )
            GetNext()->InvalidatePos();
        if ( nInvFlags & 0x08 )
            PrepareHeader();
        if ( nInvFlags & 0x10 )
            PrepareFooter();
        if ( nInvFlags & 0x20 )
            CheckGrid( nInvFlags & 0x40 );
    }
}


void SwPageFrm::SwClientNotify(const SwModify& rModify, const SfxHint& rHint)
{
    if(typeid(sw::PageFootnoteHint) == typeid(rHint))
    {
        // currently the savest way:
        static_cast<SwRootFrm*>(GetUpper())->SetSuperfluous();
        SetMaxFtnHeight(pDesc->GetFtnInfo().GetHeight());
        if(!GetMaxFtnHeight())
            SetMaxFtnHeight(LONG_MAX);
        SetColMaxFtnHeight();
        // here, the page might be destroyed:
        static_cast<SwRootFrm*>(GetUpper())->RemoveFtns(0, false, true);
    }
    else
        SwClient::SwClientNotify(rModify, rHint);
}

void SwPageFrm::_UpdateAttr( const SfxPoolItem *pOld, const SfxPoolItem *pNew,
                             sal_uInt8 &rInvFlags,
                             SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    bool bClear = true;
    const sal_uInt16 nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_FMT_CHG:
        {
            // If the frame format is changed, several things might also change:
            // 1. columns:
            assert(pOld && pNew); //FMT_CHG Missing Format
            const SwFmt *const pOldFmt = static_cast<const SwFmtChg*>(pOld)->pChangedFmt;
            const SwFmt *const pNewFmt = static_cast<const SwFmtChg*>(pNew)->pChangedFmt;
            assert(pOldFmt && pNewFmt); //FMT_CHG Missing Format
            const SwFmtCol &rOldCol = pOldFmt->GetCol();
            const SwFmtCol &rNewCol = pNewFmt->GetCol();
            if( rOldCol != rNewCol )
            {
                SwLayoutFrm *pB = FindBodyCont();
                assert(pB && "Page without Body.");
                pB->ChgColumns( rOldCol, rNewCol );
                rInvFlags |= 0x20;
            }

            // 2. header and footer:
            const SwFmtHeader &rOldH = pOldFmt->GetHeader();
            const SwFmtHeader &rNewH = pNewFmt->GetHeader();
            if( rOldH != rNewH )
                rInvFlags |= 0x08;

            const SwFmtFooter &rOldF = pOldFmt->GetFooter();
            const SwFmtFooter &rNewF = pNewFmt->GetFooter();
            if( rOldF != rNewF )
                rInvFlags |= 0x10;
            CheckDirChange();
        }
        // no break
        case RES_FRM_SIZE:
        {
            const SwRect aOldPageFrmRect( Frm() );
            SwViewShell *pSh = getRootFrm()->GetCurrShell();
            if( pSh && pSh->GetViewOptions()->getBrowseMode() )
            {
                mbValidSize = false;
                // OD 28.10.2002 #97265# - Don't call <SwPageFrm::MakeAll()>
                // Calculation of the page is not necessary, because its size is
                // is invalidated here and further invalidation is done in the
                // calling method <SwPageFrm::Modify(..)> and probably by calling
                // <SwLayoutFrm::Modify(..)> at the end.
                // It can also causes inconsistences, because the lowers are
                // adjusted, but not calculated, and a <SwPageFrm::MakeAll()> of
                // a next page is called. This is performed on the switch to the
                // online layout.
                //MakeAll();
            }
            else if (pNew)
            {
                const SwFmtFrmSize &rSz = nWhich == RES_FMT_CHG ?
                        static_cast<const SwFmtChg*>(pNew)->pChangedFmt->GetFrmSize() :
                        static_cast<const SwFmtFrmSize&>(*pNew);

                Frm().Height( std::max( rSz.GetHeight(), long(MINLAY) ) );
                Frm().Width ( std::max( rSz.GetWidth(),  long(MINLAY) ) );

                if ( GetUpper() )
                    static_cast<SwRootFrm*>(GetUpper())->CheckViewLayout( 0, 0 );
            }
            // cleanup Window
            if( pSh && pSh->GetWin() && aOldPageFrmRect.HasArea() )
            {
                // OD 12.02.2003 #i9719#, #105645# - consider border and shadow of
                // page frame for determine 'old' rectangle - it's used for invalidating.
                const bool bRightSidebar = (SidebarPosition() == sw::sidebarwindows::SidebarPosition::RIGHT);
                SwRect aOldRectWithBorderAndShadow;
                SwPageFrm::GetBorderAndShadowBoundRect( aOldPageFrmRect, pSh, aOldRectWithBorderAndShadow,
                    IsLeftShadowNeeded(), IsRightShadowNeeded(), bRightSidebar );
                pSh->InvalidateWindows( aOldRectWithBorderAndShadow );
            }
            rInvFlags |= 0x03;
            if ( aOldPageFrmRect.Height() != Frm().Height() )
                rInvFlags |= 0x04;
        }
        break;

        case RES_COL:
            assert(pOld && pNew); //COL Missing Format
            if (pOld && pNew)
            {
                SwLayoutFrm *pB = FindBodyCont();
                assert(pB); //page without body
                pB->ChgColumns( *static_cast<const SwFmtCol*>(pOld), *static_cast<const SwFmtCol*>(pNew) );
                rInvFlags |= 0x22;
            }
        break;

        case RES_HEADER:
            rInvFlags |= 0x08;
            break;

        case RES_FOOTER:
            rInvFlags |= 0x10;
            break;
        case RES_TEXTGRID:
            rInvFlags |= 0x60;
            break;
        case RES_FRAMEDIR :
            CheckDirChange();
            break;

        default:
            bClear = false;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/// get information from Modify
bool SwPageFrm::GetInfo( SfxPoolItem & rInfo ) const
{
    if( RES_AUTOFMT_DOCNODE == rInfo.Which() )
    {
        // a page frame exists, so use this one
        return false;
    }
    return true; // continue searching
}

void  SwPageFrm::SetPageDesc( SwPageDesc *pNew, SwFrmFmt *pFmt )
{
    pDesc = pNew;
    if ( pFmt )
        SetFrmFmt( pFmt );
}

/* determine the right PageDesc:
 *  0.  from the document for footnote and endnote pages
 *  1.  from the first BodyCntnt below a page
 *  2.  from PageDesc of the predecessor page
 *  3.  from PageDesc of the previous page if blank page
 *  3.1 from PageDesc of the next page if no predecessor exists
 *  4.  default PageDesc
 *  5.  In BrowseMode use the first paragraph or default PageDesc.
 */
SwPageDesc *SwPageFrm::FindPageDesc()
{
    // 0.
    if ( IsFtnPage() )
    {
        SwDoc *pDoc = GetFmt()->GetDoc();
        if ( IsEndNotePage() )
            return pDoc->GetEndNoteInfo().GetPageDesc( *pDoc );
        else
            return pDoc->GetFtnInfo().GetPageDesc( *pDoc );
    }

    SwPageDesc *pRet = 0;

    //5.
    const SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if( pSh && pSh->GetViewOptions()->getBrowseMode() )
    {
        SwCntntFrm *pFrm = GetUpper()->ContainsCntnt();
        while (pFrm && !pFrm->IsInDocBody())
            pFrm = pFrm->GetNextCntntFrm();
        if (pFrm)
        {
            SwFrm *pFlow = pFrm;
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            pRet = const_cast<SwPageDesc*>(pFlow->GetAttrSet()->GetPageDesc().GetPageDesc());
        }
        if ( !pRet )
            pRet = &GetFmt()->GetDoc()->GetPageDesc( 0 );
        return pRet;
    }

    SwFrm *pFlow = FindFirstBodyCntnt();
    if ( pFlow && pFlow->IsInTab() )
        pFlow = pFlow->FindTabFrm();

    //1.
    if ( pFlow )
    {
        SwFlowFrm *pTmp = SwFlowFrm::CastFlowFrm( pFlow );
        if ( !pTmp->IsFollow() )
            pRet = const_cast<SwPageDesc*>(pFlow->GetAttrSet()->GetPageDesc().GetPageDesc());
    }

    //3. und 3.1
    if ( !pRet && IsEmptyPage() )
            // FME 2008-03-03 #i81544# lijian/fme: an empty page should have
            // the same page description as its prev, just like after construction
            // of the empty page.
        pRet = GetPrev() ? static_cast<SwPageFrm*>(GetPrev())->GetPageDesc() :
               GetNext() ? static_cast<SwPageFrm*>(GetNext())->GetPageDesc() : 0;

    //2.
    if ( !pRet )
        pRet = GetPrev() ?
                    static_cast<SwPageFrm*>(GetPrev())->GetPageDesc()->GetFollow() : 0;

    //4.
    if ( !pRet )
        pRet = &GetFmt()->GetDoc()->GetPageDesc( 0 );

    OSL_ENSURE( pRet, "could not find page descriptor." );
    return pRet;
}

// Notify if the RootFrm changes its size
void AdjustSizeChgNotify( SwRootFrm *pRoot )
{
    const bool bOld = pRoot->IsSuperfluous();
    pRoot->mbCheckSuperfluous = false;
    if ( pRoot->GetCurrShell() )
    {
        for(SwViewShell& rSh : pRoot->GetCurrShell()->GetRingContainer())
        {
            if( pRoot == rSh.GetLayout() )
            {
                rSh.SizeChgNotify();
                if ( rSh.Imp() )
                    rSh.Imp()->NotifySizeChg( pRoot->Frm().SSize() );
            }
        }
    }
    pRoot->mbCheckSuperfluous = bOld;
}

inline void SetLastPage( SwPageFrm *pPage )
{
    static_cast<SwRootFrm*>(pPage->GetUpper())->mpLastPage = pPage;
}

void SwPageFrm::Cut()
{
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( !IsEmptyPage() )
    {
        if ( GetNext() )
            GetNext()->InvalidatePos();

        // move Flys whose anchor is on a different page (draw objects are not relevant here)
        if ( GetSortedObjs() )
        {
            size_t i = 0;
            while ( GetSortedObjs() && i < GetSortedObjs()->size() )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];

                if ( pAnchoredObj->ISA(SwFlyAtCntFrm) )
                {
                    SwFlyFrm* pFly = static_cast<SwFlyAtCntFrm*>(pAnchoredObj);
                    SwPageFrm *pAnchPage = pFly->GetAnchorFrm() ?
                                pFly->AnchorFrm()->FindPageFrm() : 0;
                    if ( pAnchPage && (pAnchPage != this) )
                    {
                        MoveFly( pFly, pAnchPage );
                        pFly->InvalidateSize();
                        pFly->_InvalidatePos();
                        // Do not increment index, in this case
                        continue;
                    }
                }
                ++i;
            }
        }
        // cleanup Window
        if ( pSh && pSh->GetWin() )
            pSh->InvalidateWindows( Frm() );
    }

    // decrease the root's page number
    static_cast<SwRootFrm*>(GetUpper())->DecrPhyPageNums();
    SwPageFrm *pPg = static_cast<SwPageFrm*>(GetNext());
    if ( pPg )
    {
        while ( pPg )
        {
            pPg->DecrPhyPageNum();  //inline --nPhyPageNum
            pPg = static_cast<SwPageFrm*>(pPg->GetNext());
        }
    }
    else
        ::SetLastPage( static_cast<SwPageFrm*>(GetPrev()) );

    SwFrm* pRootFrm = GetUpper();

    // cut all connections
    RemoveFromLayout();

    if ( pRootFrm )
        static_cast<SwRootFrm*>(pRootFrm)->CheckViewLayout( 0, 0 );
}

void SwPageFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    OSL_ENSURE( pParent->IsRootFrm(), "Parent is no Root." );
    OSL_ENSURE( pParent, "No parent for Paste()." );
    OSL_ENSURE( pParent != this, "I'm my own parent." );
    OSL_ENSURE( pSibling != this, "I'm my own neighbour." );
    OSL_ENSURE( !GetPrev() && !GetNext() && !GetUpper(),
            "I am still registered somewhere." );

    // insert into tree structure
    InsertBefore( static_cast<SwLayoutFrm*>(pParent), pSibling );

    // increase the root's page number
    static_cast<SwRootFrm*>(GetUpper())->IncrPhyPageNums();
    if( GetPrev() )
        SetPhyPageNum( static_cast<SwPageFrm*>(GetPrev())->GetPhyPageNum() + 1 );
    else
        SetPhyPageNum( 1 );
    SwPageFrm *pPg = static_cast<SwPageFrm*>(GetNext());
    if ( pPg )
    {
        while ( pPg )
        {
            pPg->IncrPhyPageNum();  //inline ++nPhyPageNum
            pPg->_InvalidatePos();
            pPg->InvalidateLayout();
            pPg = static_cast<SwPageFrm*>(pPg->GetNext());
        }
    }
    else
        ::SetLastPage( this );

    if( Frm().Width() != pParent->Prt().Width() )
        _InvalidateSize();

    InvalidatePos();

    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( pSh )
        pSh->SetFirstVisPageInvalid();

    getRootFrm()->CheckViewLayout( 0, 0 );
}

static void lcl_PrepFlyInCntRegister( SwCntntFrm *pFrm )
{
    pFrm->Prepare( PREP_REGISTER );
    if( pFrm->GetDrawObjs() )
    {
        for( size_t i = 0; i < pFrm->GetDrawObjs()->size(); ++i )
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
            if ( pAnchoredObj->ISA(SwFlyInCntFrm) )
            {
                SwFlyFrm* pFly = static_cast<SwFlyInCntFrm*>(pAnchoredObj);
                SwCntntFrm *pCnt = pFly->ContainsCntnt();
                while ( pCnt )
                {
                    lcl_PrepFlyInCntRegister( pCnt );
                    pCnt = pCnt->GetNextCntntFrm();
                }
            }
        }
    }
}

void SwPageFrm::PrepareRegisterChg()
{
    SwCntntFrm *pFrm = FindFirstBodyCntnt();
    while( pFrm )
    {
        lcl_PrepFlyInCntRegister( pFrm );
        pFrm = pFrm->GetNextCntntFrm();
        if( !IsAnLower( pFrm ) )
            break;
    }
    if( GetSortedObjs() )
    {
        for( size_t i = 0; i < GetSortedObjs()->size(); ++i )
        {
            // #i28701#
            SwAnchoredObject* pAnchoredObj = (*GetSortedObjs())[i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                pFrm = pFly->ContainsCntnt();
                while ( pFrm )
                {
                    ::lcl_PrepFlyInCntRegister( pFrm );
                    pFrm = pFrm->GetNextCntntFrm();
                }
            }
        }
    }
}

//FIXME: provide missing documentation
/** Check all pages (starting from the given one) if they use the right frame format.
 *
 * If "wrong" pages are found, try to fix this as simple as possible.
 *
 * @param pStart        the page from where to start searching
 * @param bNotifyFields
 * @param ppPrev
 */
void SwFrm::CheckPageDescs( SwPageFrm *pStart, bool bNotifyFields, SwPageFrm** ppPrev )
{
    assert(pStart && "no starting page.");

    SwViewShell *pSh   = pStart->getRootFrm()->GetCurrShell();
    SwViewShellImp *pImp  = pSh ? pSh->Imp() : 0;

    if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
    {
        pImp->GetLayAction().SetCheckPageNum( pStart->GetPhyPageNum() );
        return;
    }

    // For the update of page numbering fields, nDocPos provides
    // the page position from where invalidation should start.
    SwTwips nDocPos  = LONG_MAX;

    SwRootFrm *pRoot = static_cast<SwRootFrm*>(pStart->GetUpper());
    SwDoc* pDoc      = pStart->GetFmt()->GetDoc();
    const bool bFtns = !pDoc->GetFtnIdxs().empty();

    SwPageFrm *pPage = pStart;
    if( pPage->GetPrev() && static_cast<SwPageFrm*>(pPage->GetPrev())->IsEmptyPage() )
        pPage = static_cast<SwPageFrm*>(pPage->GetPrev());
    while ( pPage )
    {
        // obtain PageDesc and FrmFmt
        SwPageDesc *pDesc = pPage->FindPageDesc();
        bool bCheckEmpty = pPage->IsEmptyPage();
        bool bActOdd = pPage->OnRightPage();
        bool bOdd = pPage->WannaRightPage();
        bool bFirst = pPage->OnFirstPage();
        SwFrmFmt *pFmtWish = (bOdd)
            ? pDesc->GetRightFmt(bFirst) : pDesc->GetLeftFmt(bFirst);

        if ( bActOdd != bOdd ||
             pDesc != pPage->GetPageDesc() ||        // wrong Desc
             ( pFmtWish != pPage->GetFmt()  &&       // wrong format and
               ( !pPage->IsEmptyPage() || pFmtWish ) // not blank /empty
             )
           )
        {
            // Updating a page might take a while, so check the WaitCrsr
            if( pImp )
                pImp->CheckWaitCrsr();

            // invalidate the field, starting from here
            if ( nDocPos == LONG_MAX )
                nDocPos = pPage->GetPrev() ?
                            pPage->GetPrev()->Frm().Top() : pPage->Frm().Top();

            // Cases:
            //  1. Empty page should be "normal" page -> remove empty page and take next one
            //  2. Empty page should have different descriptor -> change
            //  3. Normal page should be empty -> insert empty page if previous page
            //     is not empty, otherwise see (6).
            //  4. Normal page should have different descriptor -> change
            //  5. Normal page should have different format -> change
            //  6. No "wish" format provided -> take the "other" format (left/right) of the PageDesc

            if ( pPage->IsEmptyPage() && ( pFmtWish ||          //1.
                 ( !bOdd && !pPage->GetPrev() ) ) )
            {
                SwPageFrm *pTmp = static_cast<SwPageFrm*>(pPage->GetNext());
                pPage->Cut();
                bool bUpdatePrev = false;
                if (ppPrev && *ppPrev == pPage)
                    bUpdatePrev = true;
                delete pPage;
                if ( pStart == pPage )
                    pStart = pTmp;
                pPage = pTmp;
                if (bUpdatePrev)
                    *ppPrev = pTmp;
                continue;
            }
            else if ( pPage->IsEmptyPage() && !pFmtWish &&  //2.
                      pDesc != pPage->GetPageDesc() )
            {
                pPage->SetPageDesc( pDesc, 0 );
            }
            else if ( !pPage->IsEmptyPage() &&      //3.
                      bActOdd != bOdd &&
                      ( ( !pPage->GetPrev() && !bOdd ) ||
                        ( pPage->GetPrev() &&
                          !static_cast<SwPageFrm*>(pPage->GetPrev())->IsEmptyPage() )
                      )
                    )
            {
                if ( pPage->GetPrev() )
                    pDesc = static_cast<SwPageFrm*>(pPage->GetPrev())->GetPageDesc();
                SwPageFrm *pTmp = new SwPageFrm( pDoc->GetEmptyPageFmt(),pRoot,pDesc);
                pTmp->Paste( pRoot, pPage );
                pTmp->PreparePage( false );
                pPage = pTmp;
            }
            else if ( pPage->GetPageDesc() != pDesc )           //4.
            {
                SwPageDesc *pOld = pPage->GetPageDesc();
                pPage->SetPageDesc( pDesc, pFmtWish );
                if ( bFtns )
                {
                    // If specific values of the FtnInfo are changed, something has to happen.
                    // We try to limit the damage...
                    // If the page has no FtnCont it might be problematic.
                    // Let's hope that invalidation is enough.
                    SwFtnContFrm *pCont = pPage->FindFtnCont();
                    if ( pCont && !(pOld->GetFtnInfo() == pDesc->GetFtnInfo()) )
                        pCont->_InvalidateAll();
                }
            }
            else if ( pFmtWish && pPage->GetFmt() != pFmtWish )         //5.
            {
                pPage->SetFrmFmt( pFmtWish );
            }
            else if ( !pFmtWish )                                       //6.
            {
                // get format with inverted logic
                if (!pFmtWish)
                    pFmtWish = bOdd ? pDesc->GetLeftFmt() : pDesc->GetRightFmt();
                if ( pPage->GetFmt() != pFmtWish )
                    pPage->SetFrmFmt( pFmtWish );
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OSL_FAIL( "CheckPageDescs, missing solution" );
            }
#endif
        }
        if ( bCheckEmpty )
        {
            // It also might be that an empty page is not needed at all.
            // However, the algorithm above cannot determine that. It is not needed if the following
            // page can live without it. Do obtain that information, we need to dig deeper...
            SwPageFrm *pPg = static_cast<SwPageFrm*>(pPage->GetNext());
            if( !pPg || pPage->OnRightPage() == pPg->WannaRightPage() )
            {
                // The following page can find a FrmFmt or has no successor -> empty page not needed
                SwPageFrm *pTmp = static_cast<SwPageFrm*>(pPage->GetNext());
                pPage->Cut();
                bool bUpdatePrev = false;
                if (ppPrev && *ppPrev == pPage)
                    bUpdatePrev = true;
                delete pPage;
                if ( pStart == pPage )
                    pStart = pTmp;
                pPage = pTmp;
                if (bUpdatePrev)
                    *ppPrev = pTmp;
                continue;
            }
        }
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    }

    pRoot->SetAssertFlyPages();
    pRoot->AssertPageFlys( pStart );

    if ( bNotifyFields && (!pImp || !pImp->IsUpdateExpFlds()) )
    {
        SwDocPosUpdate aMsgHnt( nDocPos );
        pDoc->getIDocumentFieldsAccess().UpdatePageFlds( &aMsgHnt );
    }

#if OSL_DEBUG_LEVEL > 0
    //1. check if two empty pages are behind one another
    bool bEmpty = false;
    SwPageFrm *pPg = pStart;
    while ( pPg )
    {
        if ( pPg->IsEmptyPage() )
        {
            if ( bEmpty )
            {
                OSL_FAIL( "double empty pages." );
                break;  // once is enough
            }
            bEmpty = true;
        }
        else
            bEmpty = false;

        pPg = static_cast<SwPageFrm*>(pPg->GetNext());
    }
#endif
}

SwPageFrm *SwFrm::InsertPage( SwPageFrm *pPrevPage, bool bFtn )
{
    SwRootFrm *pRoot = static_cast<SwRootFrm*>(pPrevPage->GetUpper());
    SwPageFrm *pSibling = static_cast<SwPageFrm*>(pPrevPage->GetNext());
    SwPageDesc *pDesc = 0;

    // insert right (odd) or left (even) page?
    bool bNextOdd = !pPrevPage->OnRightPage();
    bool bWishedOdd = bNextOdd;

    // Which PageDesc is relevant?
    // For CntntFrm take the one from format if provided,
    // otherwise from the Follow of the PrevPage
    if ( IsFlowFrm() && !SwFlowFrm::CastFlowFrm( this )->IsFollow() )
    {   SwFmtPageDesc &rDesc = (SwFmtPageDesc&)GetAttrSet()->GetPageDesc();
        pDesc = rDesc.GetPageDesc();
        if ( rDesc.GetNumOffset() )
        {
            ::boost::optional<sal_uInt16> oNumOffset = rDesc.GetNumOffset();
            bWishedOdd = oNumOffset && (oNumOffset.get() % 2) != 0;
            // use the opportunity to set the flag at root
            pRoot->SetVirtPageNum( true );
        }
    }
    if ( !pDesc )
        pDesc = pPrevPage->GetPageDesc()->GetFollow();

    assert(pDesc && "Missing PageDesc");
    if( !(bWishedOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
        bWishedOdd = !bWishedOdd;
    bool const bWishedFirst = pDesc != pPrevPage->GetPageDesc();

    SwDoc *pDoc = pPrevPage->GetFmt()->GetDoc();
    bool bCheckPages = false;
    // If there is no FrmFmt for this page, create an empty page.
    if( bWishedOdd != bNextOdd )
    {
        SwFrmFmt *const pEmptyFmt = pDoc->GetEmptyPageFmt();
        SwPageDesc *pTmpDesc = pPrevPage->GetPageDesc();
        SwPageFrm *pPage = new SwPageFrm(pEmptyFmt, pRoot, pTmpDesc);
        pPage->Paste( pRoot, pSibling );
        pPage->PreparePage( bFtn );
        // If the sibling has no body text, destroy it as long as it is no footnote page.
        if ( pSibling && !pSibling->IsFtnPage() &&
             !pSibling->FindFirstBodyCntnt() )
        {
            SwPageFrm *pDel = pSibling;
            pSibling = static_cast<SwPageFrm*>(pSibling->GetNext());
            if ( !pDoc->GetFtnIdxs().empty() )
                pRoot->RemoveFtns( pDel, true );
            pDel->Cut();
            delete pDel;
        }
        else
            bCheckPages = true;
    }
    SwFrmFmt *const pFmt( (bWishedOdd)
            ? pDesc->GetRightFmt(bWishedFirst)
            : pDesc->GetLeftFmt(bWishedFirst) );
    assert(pFmt);
    SwPageFrm *pPage = new SwPageFrm( pFmt, pRoot, pDesc );
    pPage->Paste( pRoot, pSibling );
    pPage->PreparePage( bFtn );
    // If the sibling has no body text, destroy it as long as it is no footnote page.
    if ( pSibling && !pSibling->IsFtnPage() &&
         !pSibling->FindFirstBodyCntnt() )
    {
        SwPageFrm *pDel = pSibling;
        pSibling = static_cast<SwPageFrm*>(pSibling->GetNext());
        if ( !pDoc->GetFtnIdxs().empty() )
            pRoot->RemoveFtns( pDel, true );
        pDel->Cut();
        delete pDel;
    }
    else
        bCheckPages = true;

    if ( pSibling )
    {
        if ( bCheckPages )
        {
            CheckPageDescs( pSibling, false );
            SwViewShell *pSh = getRootFrm()->GetCurrShell();
            SwViewShellImp *pImp = pSh ? pSh->Imp() : 0;
            if ( pImp && pImp->IsAction() && !pImp->GetLayAction().IsCheckPages() )
            {
                const sal_uInt16 nNum = pImp->GetLayAction().GetCheckPageNum();
                if ( nNum == pPrevPage->GetPhyPageNum() + 1 )
                    pImp->GetLayAction().SetCheckPageNumDirect(
                                                    pSibling->GetPhyPageNum() );
                return pPage;
            }
        }
        else
            pRoot->AssertPageFlys( pSibling );
    }

    // For the update of page numbering fields, nDocPos provides
    // the page position from where invalidation should start.
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( !pSh || !pSh->Imp()->IsUpdateExpFlds() )
    {
        SwDocPosUpdate aMsgHnt( pPrevPage->Frm().Top() );
        pDoc->getIDocumentFieldsAccess().UpdatePageFlds( &aMsgHnt );
    }
    return pPage;
}

sw::sidebarwindows::SidebarPosition SwPageFrm::SidebarPosition() const
{
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if( !pSh || pSh->GetViewOptions()->getBrowseMode() )
    {
        return sw::sidebarwindows::SidebarPosition::RIGHT;
    }
    else
    {
        const bool bLTR = getRootFrm()->IsLeftToRightViewLayout();
        const bool bBookMode = pSh->GetViewOptions()->IsViewLayoutBookMode();
        const bool bRightSidebar = bLTR ? (!bBookMode || OnRightPage()) : (bBookMode && !OnRightPage());

        return bRightSidebar
               ? sw::sidebarwindows::SidebarPosition::RIGHT
               : sw::sidebarwindows::SidebarPosition::LEFT;
    }
}

SwTwips SwRootFrm::GrowFrm( SwTwips nDist, bool bTst, bool )
{
    if ( !bTst )
        Frm().SSize().Height() += nDist;
    return nDist;
}

SwTwips SwRootFrm::ShrinkFrm( SwTwips nDist, bool bTst, bool )
{
    OSL_ENSURE( nDist >= 0, "nDist < 0." );
    OSL_ENSURE( nDist <= Frm().Height(), "nDist > als aktuelle Groesse." );

    if ( !bTst )
        Frm().SSize().Height() -= nDist;
    return nDist;
}

/// remove pages that are not needed at all
void SwRootFrm::RemoveSuperfluous()
{
    // A page is empty if the body text area has no CntntFrm, but not if there
    // is at least one Fly or one footnote attached to the page. Two runs are
    // needed: one for endnote pages and one for the pages of the body text.

    if ( !IsSuperfluous() )
        return;
    mbCheckSuperfluous = false;

    SwPageFrm *pPage = GetLastPage();
    long nDocPos = LONG_MAX;

    // Check the corresponding last page if it is empty and stop loop at the last non-empty page.
    do
    {
        bool bExistEssentialObjs = ( 0 != pPage->GetSortedObjs() );
        if ( bExistEssentialObjs )
        {
            // Only because the page has Flys does not mean that it is needed. If all Flys are
            // attached to generic content it is also superfluous (checking DocBody should be enough)
            // OD 19.06.2003 #108784# - consider that drawing objects in
            // header/footer are supported now.
            bool bOnlySuperfluosObjs = true;
            SwSortedObjs &rObjs = *pPage->GetSortedObjs();
            for ( size_t i = 0; bOnlySuperfluosObjs && i < rObjs.size(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = rObjs[i];
                // OD 2004-01-19 #110582# - do not consider hidden objects
                if ( pPage->GetFmt()->GetDoc()->getIDocumentDrawModelAccess().IsVisibleLayerId(
                                    pAnchoredObj->GetDrawObj()->GetLayer() ) &&
                     !pAnchoredObj->GetAnchorFrm()->FindFooterOrHeader() )
                {
                    bOnlySuperfluosObjs = false;
                }
            }
            bExistEssentialObjs = !bOnlySuperfluosObjs;
        }

        // OD 19.06.2003 #108784# - optimization: check first, if essential objects
        // exists.
        const SwLayoutFrm* pBody = 0;
        if ( bExistEssentialObjs ||
             pPage->FindFtnCont() ||
             ( 0 != ( pBody = pPage->FindBodyCont() ) &&
                ( pBody->ContainsCntnt() ||
                    // #i47580#
                    // Do not delete page if there's an empty tabframe
                    // left. I think it might be correct to use ContainsAny()
                    // instead of ContainsCntnt() to cover the empty-table-case,
                    // but I'm not fully sure, since ContainsAny() also returns
                    // SectionFrames. Therefore I prefer to do it the safe way:
                  ( pBody->Lower() && pBody->Lower()->IsTabFrm() ) ) ) )
        {
            if ( pPage->IsFtnPage() )
            {
                while ( pPage->IsFtnPage() )
                {
                    pPage = static_cast<SwPageFrm*>(pPage->GetPrev());
                    OSL_ENSURE( pPage, "only endnote pages remain." );
                }
                continue;
            }
            else
                pPage = 0;
        }

        if ( pPage )
        {
            SwPageFrm *pEmpty = pPage;
            pPage = static_cast<SwPageFrm*>(pPage->GetPrev());
            if ( !GetFmt()->GetDoc()->GetFtnIdxs().empty() )
                RemoveFtns( pEmpty, true );
            pEmpty->Cut();
            delete pEmpty;
            nDocPos = pPage ? pPage->Frm().Top() : 0;
        }
    } while ( pPage );

    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    if ( nDocPos != LONG_MAX &&
         (!pSh || !pSh->Imp()->IsUpdateExpFlds()) )
    {
        SwDocPosUpdate aMsgHnt( nDocPos );
        GetFmt()->GetDoc()->getIDocumentFieldsAccess().UpdatePageFlds( &aMsgHnt );
    }
}

/// Ensures that enough pages exist, so that all page bound frames and draw objects can be placed
void SwRootFrm::AssertFlyPages()
{
    if ( !IsAssertFlyPages() )
        return;
    mbAssertFlyPages = false;

    SwDoc *pDoc = GetFmt()->GetDoc();
    const SwFrmFmts *pTbl = pDoc->GetSpzFrmFmts();

    // what page targets the "last" Fly?
    sal_uInt16 nMaxPg = 0;

    for ( size_t i = 0; i < pTbl->size(); ++i )
    {
        const SwFmtAnchor &rAnch = (*pTbl)[i]->GetAnchor();
        if ( !rAnch.GetCntntAnchor() && nMaxPg < rAnch.GetPageNum() )
            nMaxPg = rAnch.GetPageNum();
    }
    // How many pages exist at the moment?
    SwPageFrm *pPage = static_cast<SwPageFrm*>(Lower());
    while ( pPage && pPage->GetNext() &&
            !static_cast<SwPageFrm*>(pPage->GetNext())->IsFtnPage() )
    {
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    }

    if ( nMaxPg > pPage->GetPhyPageNum() )
    {
        // Continue pages based on the rules of the PageDesc after the last page.
        bool bOdd = (pPage->GetPhyPageNum() % 2) != 0;
        SwPageDesc *pDesc = pPage->GetPageDesc();
        SwFrm *pSibling = pPage->GetNext();
        for ( sal_uInt16 i = pPage->GetPhyPageNum(); i < nMaxPg; ++i  )
        {
            if ( !(bOdd ? pDesc->GetRightFmt() : pDesc->GetLeftFmt()) )
            {
                // Insert empty page (but Flys will be stored in the next page)
                pPage = new SwPageFrm( pDoc->GetEmptyPageFmt(), this, pDesc );
                pPage->Paste( this, pSibling );
                pPage->PreparePage( false );
                bOdd = !bOdd;
                ++i;
            }
            pPage = new
                    SwPageFrm( (bOdd ? pDesc->GetRightFmt() :
                                       pDesc->GetLeftFmt()), this, pDesc );
            pPage->Paste( this, pSibling );
            pPage->PreparePage( false );
            bOdd = !bOdd;
            pDesc = pDesc->GetFollow();
        }
        // If the endnote pages are now corrupt, destroy them.
        if ( !pDoc->GetFtnIdxs().empty() )
        {
            pPage = static_cast<SwPageFrm*>(Lower());
            while ( pPage && !pPage->IsFtnPage() )
                pPage = static_cast<SwPageFrm*>(pPage->GetNext());

            if ( pPage )
            {
                SwPageDesc *pTmpDesc = pPage->FindPageDesc();
                bOdd = pPage->OnRightPage();
                if ( pPage->GetFmt() !=
                     (bOdd ? pTmpDesc->GetRightFmt() : pTmpDesc->GetLeftFmt()) )
                    RemoveFtns( pPage, false, true );
            }
        }
    }
}

/// Ensure that after the given page all page-bound objects are located on the correct page
void SwRootFrm::AssertPageFlys( SwPageFrm *pPage )
{
    while ( pPage )
    {
        if (pPage->GetSortedObjs())
        {
            size_t i = 0;
            while ( pPage->GetSortedObjs() && i< pPage->GetSortedObjs()->size() )
            {
                // #i28701#
                SwFrmFmt& rFmt = (*pPage->GetSortedObjs())[i]->GetFrmFmt();
                const SwFmtAnchor &rAnch = rFmt.GetAnchor();
                const sal_uInt16 nPg = rAnch.GetPageNum();
                if ((rAnch.GetAnchorId() == FLY_AT_PAGE) &&
                     nPg != pPage->GetPhyPageNum() )
                {
                    // If on the wrong page, check if previous page is empty
                    if( nPg && !(pPage->GetPhyPageNum()-1 == nPg &&
                        static_cast<SwPageFrm*>(pPage->GetPrev())->IsEmptyPage()) )
                    {
                        // It can move by itself. Just send a modify to its anchor attribute.
#if OSL_DEBUG_LEVEL > 1
                        const size_t nCnt = pPage->GetSortedObjs()->size();
                        rFmt.NotifyClients( 0, (SwFmtAnchor*)&rAnch );
                        OSL_ENSURE( !pPage->GetSortedObjs() ||
                                nCnt != pPage->GetSortedObjs()->size(),
                                "Object couldn't be reattached!" );
#else
                        rFmt.NotifyClients( 0, const_cast<SwFmtAnchor*>(&rAnch) );
#endif
                        // Do not increment index, in this case
                        continue;
                    }
                }
                ++i;
            }
        }
        pPage = static_cast<SwPageFrm*>(pPage->GetNext());
    }
}

Size SwRootFrm::ChgSize( const Size& aNewSize )
{
    Frm().SSize() = aNewSize;
    _InvalidatePrt();
    mbFixSize = false;
    return Frm().SSize();
}

void SwRootFrm::MakeAll()
{
    if ( !mbValidPos )
    {   mbValidPos = true;
        maFrm.Pos().setX(DOCUMENTBORDER);
        maFrm.Pos().setY(DOCUMENTBORDER);
    }
    if ( !mbValidPrtArea )
    {   mbValidPrtArea = true;
        maPrt.Pos().setX(0);
        maPrt.Pos().setY(0);
        maPrt.SSize( maFrm.SSize() );
    }
    if ( !mbValidSize )
        // SSize is set by the pages (Cut/Paste).
        mbValidSize = true;
}

void SwRootFrm::ImplInvalidateBrowseWidth()
{
    mbBrowseWidthValid = false;
    SwFrm *pPg = Lower();
    while ( pPg )
    {
        pPg->InvalidateSize();
        pPg = pPg->GetNext();
    }
}

void SwRootFrm::ImplCalcBrowseWidth()
{
    OSL_ENSURE( GetCurrShell() && GetCurrShell()->GetViewOptions()->getBrowseMode(),
            "CalcBrowseWidth and not in BrowseView" );

    // The (minimal) with is determined from borders, tables and paint objects.
    // It is calculated based on the attributes. Thus, it is not relevant how wide they are
    // currently but only how wide they want to be.
    // Frames and paint objects inside other objects (frames, tables) do not count.
    // Borders and columns are not taken into account.

    SwFrm *pFrm = ContainsCntnt();
    while ( pFrm && !pFrm->IsInDocBody() )
        pFrm = static_cast<SwCntntFrm*>(pFrm)->GetNextCntntFrm();
    if ( !pFrm )
        return;

    mbBrowseWidthValid = true;
    SwViewShell *pSh = getRootFrm()->GetCurrShell();
    mnBrowseWidth = pSh
                    ? MINLAY + 2 * pSh->GetOut()->
                                PixelToLogic( pSh->GetBrowseBorder() ).Width()
                    : 5000;
    do
    {
        if ( pFrm->IsInTab() )
            pFrm = pFrm->FindTabFrm();

        if ( pFrm->IsTabFrm() &&
             !static_cast<SwLayoutFrm*>(pFrm)->GetFmt()->GetFrmSize().GetWidthPercent() )
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), pFrm );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            const SwFmtHoriOrient &rHori = rAttrs.GetAttrSet().GetHoriOrient();
            long nWidth = rAttrs.GetSize().Width();
            if ( nWidth < USHRT_MAX-2000 && //-2k, because USHRT_MAX gets missing while trying to resize!
                 text::HoriOrientation::FULL != rHori.GetHoriOrient() )
            {
                const SwHTMLTableLayout *pLayoutInfo =
                    static_cast<const SwTabFrm *>(pFrm)->GetTable()
                                            ->GetHTMLTableLayout();
                if ( pLayoutInfo )
                    nWidth = std::min( nWidth, pLayoutInfo->GetBrowseWidthMin() );

                switch ( rHori.GetHoriOrient() )
                {
                    case text::HoriOrientation::NONE:
                        // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                        nWidth += rAttrs.CalcLeft( pFrm ) + rAttrs.CalcRight( pFrm );
                        break;
                    case text::HoriOrientation::LEFT_AND_WIDTH:
                        nWidth += rAttrs.CalcLeft( pFrm );
                        break;
                    default:
                        break;
                }
                mnBrowseWidth = std::max( mnBrowseWidth, nWidth );
            }
        }
        else if ( pFrm->GetDrawObjs() )
        {
            for ( size_t i = 0; i < pFrm->GetDrawObjs()->size(); ++i )
            {
                // #i28701#
                SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                const SwFrmFmt& rFmt = pAnchoredObj->GetFrmFmt();
                const bool bFly = pAnchoredObj->ISA(SwFlyFrm);
                if ((bFly && (FAR_AWAY == pAnchoredObj->GetObjRect().Width()))
                    || rFmt.GetFrmSize().GetWidthPercent())
                {
                    continue;
                }

                long nWidth = 0;
                switch ( rFmt.GetAnchor().GetAnchorId() )
                {
                    case FLY_AS_CHAR:
                        nWidth = bFly ? rFmt.GetFrmSize().GetWidth() :
                                        pAnchoredObj->GetObjRect().Width();
                        break;
                    case FLY_AT_PARA:
                        {
                            // #i33170#
                            // Reactivated old code because
                            // nWidth = pAnchoredObj->GetObjRect().Right()
                            // gives wrong results for objects that are still
                            // at position FAR_AWAY.
                            if ( bFly )
                            {
                                nWidth = rFmt.GetFrmSize().GetWidth();
                                const SwFmtHoriOrient &rHori = rFmt.GetHoriOrient();
                                switch ( rHori.GetHoriOrient() )
                                {
                                    case text::HoriOrientation::NONE:
                                        nWidth += rHori.GetPos();
                                        break;
                                    case text::HoriOrientation::INSIDE:
                                    case text::HoriOrientation::LEFT:
                                        if ( text::RelOrientation::PRINT_AREA == rHori.GetRelationOrient() )
                                            nWidth += pFrm->Prt().Left();
                                        break;
                                    default:
                                        break;
                                }
                            }
                            else
                                // Paint objects to not have attributes and
                                // are defined by their current size
                                nWidth = pAnchoredObj->GetObjRect().Right() -
                                         pAnchoredObj->GetDrawObj()->GetAnchorPos().X();
                        }
                        break;
                    default:    /* do nothing */;
                }
                mnBrowseWidth = std::max( mnBrowseWidth, nWidth );
            }
        }
        pFrm = pFrm->FindNextCnt();
    } while ( pFrm );
}

void SwRootFrm::StartAllAction()
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            if ( rSh.ISA( SwCrsrShell ) )
                static_cast<SwCrsrShell*>(&rSh)->StartAction();
            else
                rSh.StartAction();
        }
}

void SwRootFrm::EndAllAction( bool bVirDev )
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            const bool bOldEndActionByVirDev = rSh.IsEndActionByVirDev();
            rSh.SetEndActionByVirDev( bVirDev );
            if ( rSh.ISA( SwCrsrShell ) )
            {
                static_cast<SwCrsrShell*>(&rSh)->EndAction();
                static_cast<SwCrsrShell*>(&rSh)->CallChgLnk();
                if ( rSh.ISA( SwFEShell ) )
                    static_cast<SwFEShell*>(&rSh)->SetChainMarker();
            }
            else
                rSh.EndAction();
            rSh.SetEndActionByVirDev( bOldEndActionByVirDev );
        }
}

void SwRootFrm::UnoRemoveAllActions()
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            // #i84729#
            // No end action, if <SwViewShell> instance is currently in its end action.
            // Recursives calls to <::EndAction()> are not allowed.
            if ( !rSh.IsInEndAction() )
            {
                OSL_ENSURE(!rSh.GetRestoreActions(), "Restore action count is already set!");
                bool bCrsr = rSh.ISA( SwCrsrShell );
                bool bFE = rSh.ISA( SwFEShell );
                sal_uInt16 nRestore = 0;
                while( rSh.ActionCount() )
                {
                    if( bCrsr )
                    {
                        static_cast<SwCrsrShell*>(&rSh)->EndAction();
                        static_cast<SwCrsrShell*>(&rSh)->CallChgLnk();
                        if ( bFE )
                            static_cast<SwFEShell*>(&rSh)->SetChainMarker();
                    }
                    else
                        rSh.EndAction();
                    nRestore++;
                }
                rSh.SetRestoreActions(nRestore);
            }
            rSh.LockView(true);
        }
}

void SwRootFrm::UnoRestoreAllActions()
{
    if ( GetCurrShell() )
        for(SwViewShell& rSh : GetCurrShell()->GetRingContainer())
        {
            sal_uInt16 nActions = rSh.GetRestoreActions();
            while( nActions-- )
            {
                if ( rSh.ISA( SwCrsrShell ) )
                    static_cast<SwCrsrShell*>(&rSh)->StartAction();
                else
                    rSh.StartAction();
            }
            rSh.SetRestoreActions(0);
            rSh.LockView(false);
        }
}

// Helper functions for SwRootFrm::CheckViewLayout
static void lcl_MoveAllLowers( SwFrm* pFrm, const Point& rOffset );

static void lcl_MoveAllLowerObjs( SwFrm* pFrm, const Point& rOffset )
{
    SwSortedObjs* pSortedObj = 0;
    const bool bPage = pFrm->IsPageFrm();

    if ( bPage )
        pSortedObj = static_cast<SwPageFrm*>(pFrm)->GetSortedObjs();
    else
        pSortedObj = pFrm->GetDrawObjs();

    for ( size_t i = 0; pSortedObj && i < pSortedObj->size(); ++i)
    {
        SwAnchoredObject* pAnchoredObj = (*pSortedObj)[i];

        const SwFrmFmt& rObjFmt = pAnchoredObj->GetFrmFmt();
        const SwFmtAnchor& rAnchor = rObjFmt.GetAnchor();

        // all except from the as character anchored objects are moved
        // when processing the page frame:
        const bool bAsChar = (rAnchor.GetAnchorId() == FLY_AS_CHAR);
        if ( !bPage && !bAsChar )
            continue;

        SwObjPositioningInProgress aPosInProgress( *pAnchoredObj );

        if ( pAnchoredObj->ISA(SwFlyFrm) )
        {
            SwFlyFrm* pFlyFrm( static_cast<SwFlyFrm*>(pAnchoredObj) );
            lcl_MoveAllLowers( pFlyFrm, rOffset );
            pFlyFrm->NotifyDrawObj();
            // --> let the active embedded object be moved
            if ( pFlyFrm->Lower() )
            {
                if ( pFlyFrm->Lower()->IsNoTxtFrm() )
                {
                    SwCntntFrm* pCntntFrm = static_cast<SwCntntFrm*>(pFlyFrm->Lower());
                    SwRootFrm* pRoot = pFlyFrm->Lower()->getRootFrm();
                    SwViewShell *pSh = pRoot ? pRoot->GetCurrShell() : 0;
                    if ( pSh )
                    {
                        SwOLENode* pNode = pCntntFrm->GetNode()->GetOLENode();
                        if ( pNode )
                        {
                            svt::EmbeddedObjectRef& xObj = pNode->GetOLEObj().GetObject();
                            if ( xObj.is() )
                            {
                                for(SwViewShell& rSh : pSh->GetRingContainer())
                                {
                                    SwFEShell* pFEShell = dynamic_cast< SwFEShell* >( &rSh );
                                    if ( pFEShell )
                                        pFEShell->MoveObjectIfActive( xObj, rOffset );
                                }
                            }
                        }
                    }
                }
            }
        }
        else if ( pAnchoredObj->ISA(SwAnchoredDrawObject) )
        {
            SwAnchoredDrawObject* pAnchoredDrawObj( static_cast<SwAnchoredDrawObject*>(pAnchoredObj) );

            // don't touch objects that are not yet positioned:
            const bool bNotYetPositioned = pAnchoredDrawObj->NotYetPositioned();
            if ( bNotYetPositioned )
                continue;

            const Point aCurrAnchorPos = pAnchoredDrawObj->GetDrawObj()->GetAnchorPos();
            const Point aNewAnchorPos( ( aCurrAnchorPos + rOffset ) );
            pAnchoredDrawObj->DrawObj()->SetAnchorPos( aNewAnchorPos );
            pAnchoredDrawObj->SetLastObjRect( pAnchoredDrawObj->GetObjRect().SVRect() );

            // clear contour cache
            if ( pAnchoredDrawObj->GetFrmFmt().GetSurround().IsContour() )
                ClrContourCache( pAnchoredDrawObj->GetDrawObj() );
        }
        // #i92511#
        // cache for object rectangle inclusive spaces has to be invalidated.
        pAnchoredObj->InvalidateObjRectWithSpaces();
    }
}

static void lcl_MoveAllLowers( SwFrm* pFrm, const Point& rOffset )
{
    const SwRect aFrm( pFrm->Frm() );

    // first move the current frame
    Point &rPoint = pFrm->Frm().Pos();
    if (rPoint.X() != FAR_AWAY)
        rPoint.X() += rOffset.X();
    if (rPoint.Y() != FAR_AWAY)
        rPoint.Y() += rOffset.Y();

    // Don't forget accessibility:
    if( pFrm->IsAccessibleFrm() )
    {
        SwRootFrm *pRootFrm = pFrm->getRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aFrm );
        }
    }

    // the move any objects
    lcl_MoveAllLowerObjs( pFrm, rOffset );

    // finally, for layout frames we have to call this function recursively:
    if ( pFrm->ISA(SwLayoutFrm) )
    {
        SwFrm* pLowerFrm = pFrm->GetLower();
        while ( pLowerFrm )
        {
            lcl_MoveAllLowers( pLowerFrm, rOffset );
            pLowerFrm = pLowerFrm->GetNext();
        }
    }
}

// Calculate how the pages have to be positioned
void SwRootFrm::CheckViewLayout( const SwViewOption* pViewOpt, const SwRect* pVisArea )
{
    // #i91432#
    // No calculation of page positions, if only an empty page is present.
    // This situation occurs when <SwRootFrm> instance is in construction
    // and the document contains only left pages.
    if ( Lower()->GetNext() == 0 &&
         static_cast<SwPageFrm*>(Lower())->IsEmptyPage() )
    {
        return;
    }

    if ( !pVisArea )
    {
        // no early return for bNewPage
        if ( mnViewWidth < 0 )
            mnViewWidth = 0;
    }
    else
    {
        assert(pViewOpt && "CheckViewLayout required ViewOptions");

        const sal_uInt16 nColumns =  pViewOpt->GetViewLayoutColumns();
        const bool   bBookMode = pViewOpt->IsViewLayoutBookMode();

        if ( nColumns == mnColumns && bBookMode == mbBookMode && pVisArea->Width() == mnViewWidth && !mbSidebarChanged )
            return;

        mnColumns = nColumns;
        mbBookMode = bBookMode;
        mnViewWidth = pVisArea->Width();
        mbSidebarChanged = false;
    }

    if( GetFmt()->getIDocumentSettingAccess()->get(DocumentSettingId::BROWSE_MODE ) )
    {
        mnColumns = 1;
        mbBookMode = false;
    }

    Calc();

    const bool bOldCallbackActionEnabled = IsCallbackActionEnabled();
    SetCallbackActionEnabled( false );

    maPageRects.clear();

    const long nBorder = Frm().Pos().getX();
    const long nVisWidth = mnViewWidth - 2 * nBorder;
    const long nGapBetweenPages = GAPBETWEENPAGES;

    // check how many pages fit into the first page layout row:
    SwPageFrm* pPageFrm = static_cast<SwPageFrm*>(Lower());

    // will contain the number of pages per row. 0 means that
    // the page does not fit.
    long nWidthRemain = nVisWidth;

    // after one row has been processed, these variables contain
    // the width of the row and the maxium of the page heights
    long nCurrentRowHeight = 0;
    long nCurrentRowWidth = 0;

    // these variables are used to finally set the size of the
    // root frame
    long nSumRowHeight = 0;
    SwTwips nMinPageLeft = TWIPS_MAX;
    SwTwips nMaxPageRight = 0;
    SwPageFrm* pStartOfRow = pPageFrm;
    sal_uInt16 nNumberOfPagesInRow = mbBookMode ? 1 : 0; // in book view, start with right page
    bool bFirstRow = true;

    bool bPageChanged = false;
    const bool bRTL = !IsLeftToRightViewLayout();
    const SwTwips nSidebarWidth = SwPageFrm::GetSidebarBorderWidth( GetCurrShell() );

    while ( pPageFrm )
    {
        // we consider the current page to be "start of row" if
        // 1. it is the first page in the current row or
        // 2. it is the second page in the row and the first page is an empty page in non-book view:
        const bool bStartOfRow = pPageFrm == pStartOfRow ||
                                             ( pStartOfRow->IsEmptyPage() && pPageFrm == pStartOfRow->GetNext() && !mbBookMode );

        const bool bEmptyPage = pPageFrm->IsEmptyPage() && !mbBookMode;

        // no half doc border space for first page in each row and
        long nPageWidth = 0;
        long nPageHeight = 0;

        if ( mbBookMode )
        {
            const SwFrm& rFormatPage = pPageFrm->GetFormatPage();

            nPageWidth  = rFormatPage.Frm().Width()  + nSidebarWidth + ((bStartOfRow || 1 == (pPageFrm->GetPhyPageNum()%2)) ? 0 : nGapBetweenPages);
            nPageHeight = rFormatPage.Frm().Height() + nGapBetweenPages;
        }
        else
        {
            if ( !pPageFrm->IsEmptyPage() )
            {
                nPageWidth  = pPageFrm->Frm().Width() + nSidebarWidth + (bStartOfRow ? 0 : nGapBetweenPages);
                nPageHeight = pPageFrm->Frm().Height() + nGapBetweenPages;
            }
        }

        if ( !bEmptyPage )
            ++nNumberOfPagesInRow;

        // finish current row if
        // 1. in dynamic mode the current page does not fit anymore or
        // 2. the current page exceeds the maximum number of columns
        bool bRowFinished = (0 == mnColumns && nWidthRemain < nPageWidth ) ||
                            (0 != mnColumns && mnColumns < nNumberOfPagesInRow);

        // make sure that at least one page goes to the current row:
        if ( !bRowFinished || bStartOfRow )
        {
            // current page is allowed to be in current row
            nWidthRemain = nWidthRemain - nPageWidth;

            nCurrentRowWidth = nCurrentRowWidth + nPageWidth;
            nCurrentRowHeight = std::max( nCurrentRowHeight, nPageHeight );

            pPageFrm = static_cast<SwPageFrm*>(pPageFrm->GetNext());

            if ( !pPageFrm )
                bRowFinished = true;
        }

        if ( bRowFinished )
        {
            // pPageFrm now points to the first page in the new row or null
            // pStartOfRow points to the first page in the current row

            // special centering for last row. pretend to fill the last row with virtual copies of the last page before centering:
            if ( !pPageFrm && nWidthRemain > 0 )
            {
                // find last page in current row:
                const SwPageFrm* pLastPageInCurrentRow = pStartOfRow;
                while( pLastPageInCurrentRow->GetNext() )
                    pLastPageInCurrentRow = static_cast<const SwPageFrm*>(pLastPageInCurrentRow->GetNext());

                if ( pLastPageInCurrentRow->IsEmptyPage() )
                    pLastPageInCurrentRow = static_cast<const SwPageFrm*>(pLastPageInCurrentRow->GetPrev());

                // check how many times the last page would still fit into the remaining space:
                sal_uInt16 nNumberOfVirtualPages = 0;
                const sal_uInt16 nMaxNumberOfVirtualPages = mnColumns > 0 ? mnColumns - nNumberOfPagesInRow : USHRT_MAX;
                SwTwips nRemain = nWidthRemain;
                SwTwips nVirtualPagesWidth = 0;
                SwTwips nLastPageWidth = pLastPageInCurrentRow->Frm().Width() + nSidebarWidth;

                while ( ( mnColumns > 0 || nRemain > 0 ) && nNumberOfVirtualPages < nMaxNumberOfVirtualPages )
                {
                    SwTwips nLastPageWidthWithGap = nLastPageWidth;
                    if ( !mbBookMode || ( 0 == (nNumberOfVirtualPages + nNumberOfPagesInRow) %2) )
                        nLastPageWidthWithGap += nGapBetweenPages;

                    if ( mnColumns > 0 || nLastPageWidthWithGap < nRemain )
                    {
                        ++nNumberOfVirtualPages;
                        nVirtualPagesWidth += nLastPageWidthWithGap;
                    }
                    nRemain = nRemain - nLastPageWidthWithGap;
                }

                nCurrentRowWidth = nCurrentRowWidth + nVirtualPagesWidth;
            }

            // first page in book mode is always special:
            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nCurrentRowWidth +=
                    pStartOfRow->GetFormatPage().Frm().Width() + nSidebarWidth;
            }

            // center page if possible
            long nSizeDiff = 0;
            if (nVisWidth > nCurrentRowWidth && !(GetCurrShell() && GetCurrShell()->isTiledRendering()))
                nSizeDiff = ( nVisWidth - nCurrentRowWidth ) / 2;

            // adjust positions of pages in current row
            long nX = nSizeDiff;

            const long nRowStart = nBorder + nSizeDiff;
            const long nRowEnd   = nRowStart + nCurrentRowWidth;

            if ( bFirstRow && mbBookMode )
            {
                // #i88036#
                nX += pStartOfRow->GetFormatPage().Frm().Width() + nSidebarWidth;
            }

            SwPageFrm* pEndOfRow = pPageFrm;
            SwPageFrm* pPageToAdjust = pStartOfRow;

            do
            {
                const SwPageFrm* pFormatPage = pPageToAdjust;
                if ( mbBookMode )
                    pFormatPage = &pPageToAdjust->GetFormatPage();

                const SwTwips nCurrentPageWidth = pFormatPage->Frm().Width() + (pFormatPage->IsEmptyPage() ? 0 : nSidebarWidth);
                const Point aOldPagePos = pPageToAdjust->Frm().Pos();
                const bool bLeftSidebar = pPageToAdjust->SidebarPosition() == sw::sidebarwindows::SidebarPosition::LEFT;
                const SwTwips nLeftPageAddOffset = bLeftSidebar ?
                                                   nSidebarWidth :
                                                   0;

                Point aNewPagePos( nBorder + nX, nBorder + nSumRowHeight );
                Point aNewPagePosWithLeftOffset( nBorder + nX + nLeftPageAddOffset, nBorder + nSumRowHeight );

                // RTL view layout: Calculate mirrored page position
                if ( bRTL )
                {
                    const long nXOffsetInRow = aNewPagePos.getX() - nRowStart;
                    aNewPagePos.setX(nRowEnd - nXOffsetInRow - nCurrentPageWidth);
                    aNewPagePosWithLeftOffset = aNewPagePos;
                    aNewPagePosWithLeftOffset.setX(aNewPagePosWithLeftOffset.getX() + nLeftPageAddOffset);
                }

                if ( aNewPagePosWithLeftOffset != aOldPagePos )
                {
                    lcl_MoveAllLowers( pPageToAdjust, aNewPagePosWithLeftOffset - aOldPagePos );
                    pPageToAdjust->SetCompletePaint();
                    bPageChanged = true;
                }

                // calculate area covered by the current page and store to
                // maPageRects. This is used e.g., for cursor setting
                const bool bFirstColumn = pPageToAdjust == pStartOfRow;
                const bool bLastColumn = pPageToAdjust->GetNext() == pEndOfRow;
                const bool bLastRow = !pEndOfRow;

                nMinPageLeft  = std::min( nMinPageLeft, aNewPagePos.getX() );
                nMaxPageRight = std::max( nMaxPageRight, aNewPagePos.getX() + nCurrentPageWidth);

                // border of nGapBetweenPages around the current page:
                SwRect aPageRectWithBorders( aNewPagePos.getX() - nGapBetweenPages,
                                             aNewPagePos.getY(),
                                             pPageToAdjust->Frm().SSize().Width() + nGapBetweenPages + nSidebarWidth,
                                             nCurrentRowHeight );

                static const long nOuterClickDiff = 1000000;

                // adjust borders for these special cases:
                if ( (bFirstColumn && !bRTL) || (bLastColumn && bRTL) )
                    aPageRectWithBorders.SubLeft( nOuterClickDiff );
                if ( (bLastColumn && !bRTL) || (bFirstColumn && bRTL) )
                    aPageRectWithBorders.AddRight( nOuterClickDiff );
                if ( bFirstRow )
                    aPageRectWithBorders.SubTop( nOuterClickDiff );
                if ( bLastRow )
                    aPageRectWithBorders.AddBottom( nOuterClickDiff );

                maPageRects.push_back( aPageRectWithBorders );

                nX = nX + nCurrentPageWidth;
                pPageToAdjust = static_cast<SwPageFrm*>(pPageToAdjust->GetNext());

                // distance to next page
                if ( pPageToAdjust && pPageToAdjust != pEndOfRow )
                {
                    // in book view, we add the x gap before left (even) pages:
                    if ( mbBookMode )
                    {
                        if ( 0 == (pPageToAdjust->GetPhyPageNum()%2) )
                            nX = nX + nGapBetweenPages;
                    }
                    else
                    {
                        // in non-book view, dont add x gap before
                        // 1. the last empty page in a row
                        // 2. after an empty page
                        const bool bDontAddGap = ( pPageToAdjust->IsEmptyPage() && pPageToAdjust->GetNext() == pEndOfRow ) ||
                                                 ( static_cast<SwPageFrm*>(pPageToAdjust->GetPrev())->IsEmptyPage() );

                        if  ( !bDontAddGap )
                            nX = nX + nGapBetweenPages;
                    }
                }
            }
            while (pPageToAdjust && pPageToAdjust != pEndOfRow);

            // adjust values for root frame size
            nSumRowHeight = nSumRowHeight + nCurrentRowHeight;

            // start new row:
            nCurrentRowHeight = 0;
            nCurrentRowWidth = 0;
            pStartOfRow = pEndOfRow;
            nWidthRemain = nVisWidth;
            nNumberOfPagesInRow = 0;
            bFirstRow = false;
        } // end row finished
    } // end while

    // set size of root frame:
    const Size aOldSize( Frm().SSize() );
    const Size aNewSize( nMaxPageRight - nBorder, nSumRowHeight - nGapBetweenPages );

    if ( bPageChanged || aNewSize != aOldSize )
    {
        ChgSize( aNewSize );
        ::AdjustSizeChgNotify( this );
        Calc();

        SwViewShell* pSh = GetCurrShell();

        if ( pSh && pSh->GetDoc()->GetDocShell() )
        {
            pSh->SetFirstVisPageInvalid();
            if (bOldCallbackActionEnabled)
            {
                pSh->InvalidateWindows( SwRect( 0, 0, SAL_MAX_INT32, SAL_MAX_INT32 ) );
                pSh->GetDoc()->GetDocShell()->Broadcast(SfxSimpleHint(SFX_HINT_DOCCHANGED));
            }
        }
    }

    maPagesArea.Pos( Frm().Pos() );
    maPagesArea.SSize( aNewSize );
    if ( TWIPS_MAX != nMinPageLeft )
        maPagesArea._Left( nMinPageLeft );

    SetCallbackActionEnabled( bOldCallbackActionEnabled );
}

bool SwRootFrm::IsLeftToRightViewLayout() const
{
    // Layout direction determined by layout direction of the first page.
    // #i88036#
    // Only ask a non-empty page frame for its layout direction
    const SwPageFrm& rPage =
                    dynamic_cast<const SwPageFrm&>(*Lower()).GetFormatPage();
    return !rPage.IsRightToLeft() && !rPage.IsVertical();
}

const SwPageFrm& SwPageFrm::GetFormatPage() const
{
    const SwPageFrm* pRet = this;
    if ( IsEmptyPage() )
    {
        pRet = static_cast<const SwPageFrm*>( OnRightPage() ? GetNext() : GetPrev() );
        // #i88035#
        // Typically a right empty page frame has a next non-empty page frame and
        // a left empty page frame has a previous non-empty page frame.
        // But under certain cirsumstances this assumption is not true -
        // e.g. during insertion of a left page at the end of the document right
        // after a left page in an intermediate state a right empty page does not
        // have a next page frame.
        if ( pRet == 0 )
        {
            if ( OnRightPage() )
            {
                pRet = static_cast<const SwPageFrm*>( GetPrev() );
            }
            else
            {
                pRet = static_cast<const SwPageFrm*>( GetNext() );
            }
        }
        assert(pRet &&
                "<SwPageFrm::GetFormatPage()> - inconsistent layout: empty page without previous and next page frame --> crash.");
    }
    return *pRet;
}

bool SwPageFrm::IsOverHeaderFooterArea( const Point& rPt, FrameControlType &rControl ) const
{
    long nUpperLimit = 0;
    long nLowerLimit = 0;
    const SwFrm* pFrm = Lower();
    while ( pFrm )
    {
        if ( pFrm->IsBodyFrm() )
        {
            nUpperLimit = pFrm->Frm().Top();
            nLowerLimit = pFrm->Frm().Bottom();
        }
        else if ( pFrm->IsFtnContFrm() )
            nLowerLimit = pFrm->Frm().Bottom();

        pFrm = pFrm->GetNext();
    }

    SwRect aHeaderArea( Frm().TopLeft(),
           Size( Frm().Width(), nUpperLimit - Frm().Top() ) );

    if ( aHeaderArea.IsInside( rPt ) )
    {
        rControl = Header;
        return true;
    }
    else
    {
        SwRect aFooterArea( Point( Frm().Left(), nLowerLimit ),
                Size( Frm().Width(), Frm().Bottom() - nLowerLimit ) );

        if ( aFooterArea.IsInside( rPt ) )
        {
            rControl = Footer;
            return true;
        }
    }

    return false;
}

SwTextGridItem const* GetGridItem(SwPageFrm const*const pPage)
{
    if (pPage && pPage->HasGrid())
    {
        SwTextGridItem const& rGridItem(
                pPage->GetPageDesc()->GetMaster().GetTextGrid());
        if (GRID_NONE != rGridItem.GetGridType())
        {
            return &rGridItem;
        }
    }
    return 0;
}

sal_uInt16 GetGridWidth(SwTextGridItem const& rG, SwDoc const& rDoc)
{
    return (rDoc.IsSquaredPageMode()) ? rG.GetBaseHeight() : rG.GetBaseWidth();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
