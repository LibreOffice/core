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

#include <svx/svdobj.hxx>
#include <init.hxx>
#include <fesh.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <doc.hxx>
#include <frmtool.hxx>
#include <swtable.hxx>
#include <viewimp.hxx>
#include <dview.hxx>
#include <flyfrm.hxx>
#include <node.hxx>
#include <pam.hxx>
#include <sectfrm.hxx>
#include <fmtpdsc.hxx>
#include <fmtsrnd.hxx>
#include <fmtcntnt.hxx>
#include <fmtfsize.hxx>
#include <tabfrm.hxx>
#include <cellfrm.hxx>
#include <flyfrms.hxx>
#include <txtfrm.hxx>
#include <mdiexp.hxx>
#include <edimp.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
#include <environmentofanchoredobject.hxx>
#include <ndtxt.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <UndoInsert.hxx>

using namespace com::sun::star;


void SwFEShell::EndAllActionAndCall()
{
    for(SwViewShell& rCurrentShell : GetRingContainer())
    {
        if( dynamic_cast<const SwCrsrShell*>( &rCurrentShell) !=  nullptr )
        {
            static_cast<SwFEShell*>(&rCurrentShell)->EndAction();
            static_cast<SwFEShell*>(&rCurrentShell)->CallChgLnk();
        }
        else
            rCurrentShell.EndAction();
    }
}

// Determine the Content's nearest to the point
Point SwFEShell::GetContentPos( const Point& rPoint, bool bNext ) const
{
    SET_CURR_SHELL( const_cast<SwViewShell*>(static_cast<SwViewShell const *>(this)) );
    return GetLayout()->GetNextPrevContentPos( rPoint, bNext );
}

const SwRect& SwFEShell::GetAnyCurRect( CurRectType eType, const Point* pPt,
                                        const uno::Reference < embed::XEmbeddedObject >& xObj ) const
{
    const SwFrm *pFrm = Imp()->HasDrawView()
                ? ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkedObjectList(),
                                      const_cast<SwViewShell*>(static_cast<SwViewShell const *>(this)))
                : nullptr;

    if( !pFrm )
    {
        if( pPt )
        {
            SwPosition aPos( *GetCrsr()->GetPoint() );
            Point aPt( *pPt );
            GetLayout()->GetCrsrOfst( &aPos, aPt );
            SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
            pFrm = pNd->getLayoutFrm( GetLayout(), pPt );
        }
        else
        {
            const bool bOldCallbackActionEnabled = GetLayout()->IsCallbackActionEnabled();
            if( bOldCallbackActionEnabled )
                GetLayout()->SetCallbackActionEnabled( false );
            pFrm = GetCurrFrm();
            if( bOldCallbackActionEnabled )
                GetLayout()->SetCallbackActionEnabled( true );
        }
    }

    if( !pFrm )
        return GetLayout()->Frm();

    bool bFrm = true;
    switch ( eType )
    {
        case RECT_PAGE_PRT:         bFrm = false; /* no break */
        case RECT_PAGE :            pFrm = pFrm->FindPageFrm();
                                    break;

        case RECT_PAGE_CALC:        pFrm->Calc(Imp()->GetShell()->GetOut());
                                    pFrm = pFrm->FindPageFrm();
                                    pFrm->Calc(Imp()->GetShell()->GetOut());
                                    break;

        case RECT_FLY_PRT_EMBEDDED: bFrm = false; /* no break */
        case RECT_FLY_EMBEDDED:     pFrm = xObj.is() ? FindFlyFrm( xObj )
                                                : pFrm->IsFlyFrm()
                                                    ? pFrm
                                                    : pFrm->FindFlyFrm();
                                    break;

        case RECT_OUTTABSECTION_PRT:
        case RECT_OUTTABSECTION :   if( pFrm->IsInTab() )
                                        pFrm = pFrm->FindTabFrm();
                                    else {
                                        OSL_FAIL( "Missing Table" );
                                    }
                                    /* no break */
        case RECT_SECTION_PRT:
        case RECT_SECTION:          if( pFrm->IsInSct() )
                                        pFrm = pFrm->FindSctFrm();
                                    else {
                                        OSL_FAIL( "Missing section" );
                                    }

                                    if( RECT_OUTTABSECTION_PRT == eType ||
                                        RECT_SECTION_PRT == eType )
                                        bFrm = false;
                                    break;

        case RECT_HEADERFOOTER_PRT: bFrm = false; /* no break */
        case RECT_HEADERFOOTER:     if( nullptr == (pFrm = pFrm->FindFooterOrHeader()) )
                                        return GetLayout()->Frm();
                                    break;

        case RECT_PAGES_AREA:       return GetLayout()->GetPagesArea();

        default:                    break;
    }
    return bFrm ? pFrm->Frm() : pFrm->Prt();
}

sal_uInt16 SwFEShell::GetPageNumber( const Point &rPoint ) const
{
    const SwFrm *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->Frm().IsInside( rPoint ) )
        pPage = pPage->GetNext();
    if ( pPage )
        return static_cast<const SwPageFrm*>(pPage)->GetPhyPageNum();
    else
        return 0;
}

bool SwFEShell::GetPageNumber( long nYPos, bool bAtCrsrPos, sal_uInt16& rPhyNum, sal_uInt16& rVirtNum, OUString &rDisplay) const
{
    const SwFrm *pPage;

    if ( bAtCrsrPos )                   // get page of Crsr
    {
        pPage = GetCurrFrm( false );
        if ( pPage )
            pPage = pPage->FindPageFrm();
    }
    else if ( nYPos > -1 )              // determine page via the position
    {
        pPage = GetLayout()->Lower();
        while( pPage &&  (pPage->Frm().Bottom() < nYPos ||
                            nYPos < pPage->Frm().Top() ) )
            pPage = pPage->GetNext();
    }
    else                                // first visible page
    {
        pPage = Imp()->GetFirstVisPage(GetOut());
        if ( pPage && static_cast<const SwPageFrm*>(pPage)->IsEmptyPage() )
            pPage = pPage->GetNext();
    }

    if( pPage )
    {
        rPhyNum  = static_cast<const SwPageFrm*>(pPage)->GetPhyPageNum();
        rVirtNum = static_cast<const SwPageFrm*>(pPage)->GetVirtPageNum();
        const SvxNumberType& rNum = static_cast<const SwPageFrm*>(pPage)->GetPageDesc()->GetNumType();
        rDisplay = rNum.GetNumStr( rVirtNum );
    }

    return nullptr != pPage;
}

bool SwFEShell::IsDirectlyInSection() const
{
    SwFrm* pFrm = GetCurrFrm( false );
    return pFrm && pFrm->GetUpper() && pFrm->GetUpper()->IsSctFrm();
}

FrmTypeFlags SwFEShell::GetFrmType( const Point *pPt, bool bStopAtFly ) const
{
    FrmTypeFlags nReturn = FrmTypeFlags::NONE;
    const SwFrm *pFrm;
    if ( pPt )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        pFrm = pNd->getLayoutFrm( GetLayout(), pPt );
    }
    else
        pFrm = GetCurrFrm( false );
    while ( pFrm )
    {
        switch ( pFrm->GetType() )
        {
            case FRM_COLUMN:    if( pFrm->GetUpper()->IsSctFrm() )
                                {
                                    // Check, if isn't not only a single column
                                    // from a section with footnotes at the end.
                                    if( pFrm->GetNext() || pFrm->GetPrev() )
                                        // Sectioncolumns
                                        nReturn |= ( nReturn & FrmTypeFlags::TABLE ) ?
                                            FrmTypeFlags::COLSECTOUTTAB : FrmTypeFlags::COLSECT;
                                }
                                else // only pages and frame columns
                                    nReturn |= FrmTypeFlags::COLUMN;
                                break;
            case FRM_PAGE:      nReturn |= FrmTypeFlags::PAGE;
                                if( static_cast<const SwPageFrm*>(pFrm)->IsFootnotePage() )
                                    nReturn |= FrmTypeFlags::FTNPAGE;
                                break;
            case FRM_HEADER:    nReturn |= FrmTypeFlags::HEADER;      break;
            case FRM_FOOTER:    nReturn |= FrmTypeFlags::FOOTER;      break;
            case FRM_BODY:      if( pFrm->GetUpper()->IsPageFrm() ) // not for ColumnFrms
                                    nReturn |= FrmTypeFlags::BODY;
                                break;
            case FRM_FTN:       nReturn |= FrmTypeFlags::FOOTNOTE;    break;
            case FRM_FLY:       if( static_cast<const SwFlyFrm*>(pFrm)->IsFlyLayFrm() )
                                    nReturn |= FrmTypeFlags::FLY_FREE;
                                else if ( static_cast<const SwFlyFrm*>(pFrm)->IsFlyAtCntFrm() )
                                    nReturn |= FrmTypeFlags::FLY_ATCNT;
                                else
                                {
                                    OSL_ENSURE( static_cast<const SwFlyFrm*>(pFrm)->IsFlyInCntFrm(),
                                            "New frametype?" );
                                    nReturn |= FrmTypeFlags::FLY_INCNT;
                                }
                                nReturn |= FrmTypeFlags::FLY_ANY;
                                if( bStopAtFly )
                                    return nReturn;
                                break;
            case FRM_TAB:
            case FRM_ROW:
            case FRM_CELL:      nReturn |= FrmTypeFlags::TABLE;       break;
            default:            /* do nothing */                break;
        }
        if ( pFrm->IsFlyFrm() )
            pFrm = static_cast<const SwFlyFrm*>(pFrm)->GetAnchorFrm();
        else
            pFrm = pFrm->GetUpper();
    }
    return nReturn;
}

void SwFEShell::ShGetFcs( bool bUpdate )
{
    ::SetShell( this );
    SwCrsrShell::ShGetFcs( bUpdate );

    if ( HasDrawView() )
    {
        Imp()->GetDrawView()->showMarkHandles();
        if ( Imp()->GetDrawView()->AreObjectsMarked() )
            FrameNotify( this, FLY_DRAG_START );
    }
}

void SwFEShell::ShLooseFcs()
{
    SwCrsrShell::ShLooseFcs();

    if ( HasDrawView() && Imp()->GetDrawView()->AreObjectsMarked() )
    {
        Imp()->GetDrawView()->hideMarkHandles();
        FrameNotify( this, FLY_DRAG_END );
    }
}

sal_uInt16 SwFEShell::GetPhyPageNum()
{
    SwFrm *pFrm = GetCurrFrm();
    if ( pFrm )
        return pFrm->GetPhyPageNum();
    return 0;
}

sal_uInt16 SwFEShell::GetVirtPageNum( const bool bCalcFrm )
{
    SwFrm *pFrm = GetCurrFrm( bCalcFrm );
    if ( pFrm )
        return pFrm->GetVirtPageNum();
    return 0;
}

static void lcl_SetAPageOffset( sal_uInt16 nOffset, SwPageFrm* pPage, SwFEShell* pThis )
{
    pThis->StartAllAction();
    OSL_ENSURE( pPage->FindFirstBodyContent(),
            "SwFEShell _SetAPageOffset() without ContentFrm" );

    SwFormatPageDesc aDesc( pPage->GetPageDesc() );
    aDesc.SetNumOffset( nOffset );

    SwFrm *pFrm = pThis->GetCurrFrm( false );
    if ( pFrm->IsInTab() )
        pThis->GetDoc()->SetAttr( aDesc, *pFrm->FindTabFrm()->GetFormat() );
    else
    {
        pThis->GetDoc()->getIDocumentContentOperations().InsertPoolItem( *pThis->GetCrsr(), aDesc );
    }

    pThis->EndAllAction();
}

void SwFEShell::SetNewPageOffset( sal_uInt16 nOffset )
{
    GetLayout()->SetVirtPageNum( true );
    const SwPageFrm *pPage = GetCurrFrm( false )->FindPageFrm();
    lcl_SetAPageOffset( nOffset, const_cast<SwPageFrm*>(pPage), this );
}

void SwFEShell::SetPageOffset( sal_uInt16 nOffset )
{
    const SwPageFrm *pPage = GetCurrFrm( false )->FindPageFrm();
    const SwRootFrm* pDocLayout = GetLayout();
    while ( pPage )
    {
        const SwFrm *pFlow = pPage->FindFirstBodyContent();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            const SwFormatPageDesc& rPgDesc = pFlow->GetAttrSet()->GetPageDesc();
            if ( rPgDesc.GetNumOffset() )
            {
                pDocLayout->SetVirtPageNum( true );
                lcl_SetAPageOffset( nOffset, const_cast<SwPageFrm*>(pPage), this );
                break;
            }
        }
        pPage = static_cast<const SwPageFrm*>(pPage->GetPrev());
    }
}

sal_uInt16 SwFEShell::GetPageOffset() const
{
    const SwPageFrm *pPage = GetCurrFrm()->FindPageFrm();
    while ( pPage )
    {
        const SwFrm *pFlow = pPage->FindFirstBodyContent();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrm();
            ::boost::optional<sal_uInt16> oNumOffset = pFlow->GetAttrSet()->GetPageDesc().GetNumOffset();
            if ( oNumOffset )
                return oNumOffset.get();
        }
        pPage = static_cast<const SwPageFrm*>(pPage->GetPrev());
    }
    return 0;
}

void SwFEShell::InsertLabel( const SwLabelType eType, const OUString &rText, const OUString& rSeparator,
                             const OUString& rNumberSeparator,
                             const bool bBefore, const sal_uInt16 nId,
                             const OUString& rCharacterStyle,
                             const bool bCpyBrd )
{
    // get node index of cursor position, SwDoc can do everything else itself
    SwContentFrm *pCnt = LTYPE_DRAW==eType ? nullptr : GetCurrFrm( false );
    if( LTYPE_DRAW==eType || pCnt )
    {
        StartAllAction();
        SwRewriter aRewriter(SwUndoInsertLabel::CreateRewriter(rText));
        StartUndo(UNDO_INSERTLABEL, &aRewriter);

        sal_uLong nIdx = 0;
        bool bInnerCntIsFly = false;
        SwFlyFrameFormat* pFlyFormat = nullptr;
        switch( eType )
        {
        case LTYPE_OBJECT:
        case LTYPE_FLY:
            bInnerCntIsFly = pCnt->IsInFly();
            if (bInnerCntIsFly)
            {
                // pass down index to the startnode for flys
                nIdx = pCnt->FindFlyFrm()->
                            GetFormat()->GetContent().GetContentIdx()->GetIndex();
            }
            break;
        case LTYPE_TABLE:
            if( pCnt->IsInTab() )
            {
                // pass down index to the TableNode for tables
                const SwTable& rTable = *pCnt->FindTabFrm()->GetTable();
                nIdx = rTable.GetTabSortBoxes()[ 0 ]
                            ->GetSttNd()->FindTableNode()->GetIndex();
            }
            break;
        case LTYPE_DRAW:
            if( Imp()->GetDrawView() )
            {
                SwDrawView *pDView = Imp()->GetDrawView();
                const SdrMarkList& rMrkList = pDView->GetMarkedObjectList();

                // copy marked drawing objects to
                // local list to perform the corresponding action for each object
                std::vector<SdrObject*> aDrawObjs;
                {
                    for ( size_t i = 0; i < rMrkList.GetMarkCount(); ++i )
                    {
                        SdrObject* pDrawObj = rMrkList.GetMark(i)->GetMarkedSdrObj();
                        if( pDrawObj )
                        aDrawObjs.push_back( pDrawObj );
                    }
                }
                // loop on marked drawing objects
                while ( !aDrawObjs.empty() )
                {
                    SdrObject* pDrawObj = aDrawObjs.back();
                    if ( dynamic_cast<const SwVirtFlyDrawObj*>( pDrawObj) ==  nullptr &&
                         dynamic_cast<const SwFlyDrawObj*>( pDrawObj) ==  nullptr )
                    {
                        SwFlyFrameFormat *pFormat =
                            GetDoc()->InsertDrawLabel( rText, rSeparator, rNumberSeparator, nId, rCharacterStyle, *pDrawObj );
                        if( !pFlyFormat )
                            pFlyFormat = pFormat;
                    }

                    aDrawObjs.pop_back();
                }

            }
            break;
        default:
            OSL_ENSURE( false, "Crsr neither in table nor in fly." );
        }

        if( nIdx )
        {
            pFlyFormat = GetDoc()->InsertLabel(eType, rText, rSeparator,
                                               rNumberSeparator, bBefore, nId,
                                               nIdx, rCharacterStyle, bCpyBrd);

            //if we succeeded in putting a caption on the content, and the
            //content was a frame/graphic, then set the contained element
            //to as-char anchoring because that's all msword is able to
            //do when inside a frame, and in writer for freshly captioned
            //elements it's largely irrelevent what the anchor of the contained
            //type is but making it as-char by default results in very
            //good roundtripping
            if (pFlyFormat && bInnerCntIsFly)
            {
                SwNodeIndex aAnchIdx(*pFlyFormat->GetContent().GetContentIdx(), 1);
                SwTextNode *pTextNode = aAnchIdx.GetNode().GetTextNode();

                SwFormatAnchor aAnc(FLY_AS_CHAR);
                sal_Int32 nInsertPos = bBefore ? pTextNode->Len() : 0;
                SwPosition aPos(*pTextNode, nInsertPos);

                aAnc.SetAnchor(&aPos);

                SfxItemSet aSet(makeItemSetFromFormatAnchor(GetDoc()->GetAttrPool(), aAnc));

                SwFlyFrm *pFly = GetSelectedOrCurrFlyFrm();
                SwFlyFrameFormat* pInnerFlyFormat = pFly->GetFormat();
                GetDoc()->SetFlyFrmAttr(*pInnerFlyFormat, aSet);

                //put a hard-break after the graphic to keep it separated
                //from the caption text if the outer frame is resized
                SwIndex aIdx(pTextNode, bBefore ? nInsertPos : 1);
                pTextNode->InsertText("\n", aIdx);
            }
        }

        if (pFlyFormat)
        {
            const Point aPt(GetCrsrDocPos());
            if (SwFlyFrm* pFrm = pFlyFormat->GetFrm(&aPt))
                SelectFlyFrm(*pFrm, true);
        }
        EndUndo();
        EndAllActionAndCall();
    }
}

bool SwFEShell::Sort(const SwSortOptions& rOpt)
{
    if( !HasSelection() )
        return false;

    SET_CURR_SHELL( this );
    bool bRet = false;
    StartAllAction();
    if(IsTableMode())
    {
        // Sort table
        // check if Point/Mark of current Crsr are in one table
        SwFrm *pFrm = GetCurrFrm( false );
        OSL_ENSURE( pFrm->FindTabFrm(), "Crsr not in table." );

        // search boxes via the layout
        SwSelBoxes  aBoxes;
        GetTableSel(*this, aBoxes);

        // The Crsr should be removed from the deletion area.
        // Always put them behind/on the table; via the
        // document position they will always be set to the old position
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        {
            /* ParkCursor->ParkCursorTab */
            ParkCursorInTab();
        }

        // call sorting on document
        bRet = mpDoc->SortTable(aBoxes, rOpt);
    }
    else
    {
        // Sort text nothing else
        for(SwPaM& rPaM : GetCrsr()->GetRingContainer())
        {
            SwPaM* pPam = &rPaM;

            SwPosition* pStart = pPam->Start();
            SwPosition* pEnd   = pPam->End();

            SwNodeIndex aPrevIdx( pStart->nNode, -1 );
            sal_uLong nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
            const sal_Int32 nCntStt  = pStart->nContent.GetIndex();

            // Sorting
            bRet = mpDoc->SortText(*pPam, rOpt);

            // put selection again
            pPam->DeleteMark();
            pPam->GetPoint()->nNode.Assign( aPrevIdx.GetNode(), +1 );
            SwContentNode* pCNd = pPam->GetContentNode();
            sal_Int32 nLen = pCNd->Len();
            if( nLen > nCntStt )
                nLen = nCntStt;
            pPam->GetPoint()->nContent.Assign(pCNd, nLen );
            pPam->SetMark();

            pPam->GetPoint()->nNode += nOffset;
            pCNd = pPam->GetContentNode();
            pPam->GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
        }
    }

    EndAllAction();
    return bRet;
}

sal_uInt16 SwFEShell::_GetCurColNum( const SwFrm *pFrm,
                                SwGetCurColNumPara* pPara ) const
{
    sal_uInt16 nRet = 0;
    while ( pFrm )
    {
        pFrm = pFrm->GetUpper();
        if( pFrm && pFrm->IsColumnFrm() )
        {
            const SwFrm *pCurFrm = pFrm;
            do {
                ++nRet;
                pFrm = pFrm->GetPrev();
            } while ( pFrm );

            if( pPara )
            {
                // now search the format, determining the columness
                pFrm = pCurFrm->GetUpper();
                while( pFrm )
                {
                    if( ( FRM_PAGE | FRM_FLY | FRM_SECTION ) & pFrm->GetType() )
                    {
                        pPara->pFrameFormat = static_cast<const SwLayoutFrm*>(pFrm)->GetFormat();
                        pPara->pPrtRect = &pFrm->Prt();
                        pPara->pFrmRect = &pFrm->Frm();
                        break;
                    }
                    pFrm = pFrm->GetUpper();
                }
                if( !pFrm )
                {
                    pPara->pFrameFormat = nullptr;
                    pPara->pPrtRect = nullptr;
                    pPara->pFrmRect = nullptr;
                }
            }
            break;
        }
    }
    return nRet;
}

sal_uInt16 SwFEShell::GetCurColNum( SwGetCurColNumPara* pPara ) const
{
    OSL_ENSURE( GetCurrFrm(), "Crsr parked?" );
    return _GetCurColNum( GetCurrFrm(), pPara );
}

sal_uInt16 SwFEShell::GetCurOutColNum( SwGetCurColNumPara* pPara ) const
{
    sal_uInt16 nRet = 0;
    SwFrm* pFrm = GetCurrFrm();
    OSL_ENSURE( pFrm, "Crsr parked?" );
    if( pFrm )
    {
        pFrm = pFrm->IsInTab() ? static_cast<SwFrm*>(pFrm->FindTabFrm())
                               : static_cast<SwFrm*>(pFrm->FindSctFrm());
        OSL_ENSURE( pFrm, "No Tab, no Sect" );
        if( pFrm )
            nRet = _GetCurColNum( pFrm, pPara );
    }
    return nRet;
}

SwFEShell::SwFEShell( SwDoc& rDoc, vcl::Window *pWindow, const SwViewOption *pOptions )
    : SwEditShell( rDoc, pWindow, pOptions )
    , m_bCheckForOLEInCaption(false)
{
}

SwFEShell::SwFEShell( SwEditShell& rShell, vcl::Window *pWindow )
    : SwEditShell( rShell, pWindow )
    , m_bCheckForOLEInCaption(false)
{
}

SwFEShell::~SwFEShell()
{
}

// #i17567# - adjustments for allowing
//          negative vertical positions for fly frames anchored to paragraph/to character.
// #i22305# - adjustments for option 'Follow text flow'
//          for to frame anchored objects.
// #i22341# - adjustments for vertical alignment at top of line
//          for to character anchored objects.
void SwFEShell::CalcBoundRect( SwRect& _orRect,
                               const RndStdIds _nAnchorId,
                               const sal_Int16 _eHoriRelOrient,
                               const sal_Int16 _eVertRelOrient,
                               const SwPosition* _pToCharContentPos,
                               const bool _bFollowTextFlow,
                               bool _bMirror,
                               Point* _opRef,
                               Size* _opPercent,
                               const SwFormatFrmSize* pFormatFrmSize) const
{
    const SwFrm* pFrm;
    const SwFlyFrm* pFly;
    if( _opRef )
    {
        pFrm = GetCurrFrm();
        if( nullptr != ( pFly = pFrm->FindFlyFrm() ) )
            pFrm = pFly->GetAnchorFrm();
    }
    else
    {
        pFly = GetSelectedFlyFrm();
        pFrm = pFly ? pFly->GetAnchorFrm() : GetCurrFrm();
    }

    bool bWrapThrough = false;
    if ( pFly )
    {
        SwFlyFrameFormat* pFormat = const_cast<SwFlyFrameFormat*>(pFly->GetFormat());
        const SwFormatSurround& rSurround = pFormat->GetSurround();
        bWrapThrough = rSurround.GetSurround() == SURROUND_THROUGHT;
    }

    const SwPageFrm* pPage = pFrm->FindPageFrm();
    _bMirror = _bMirror && !pPage->OnRightPage();

    Point aPos;
    bool bVertic = false;
    bool bRTL = false;
    bool bVerticalL2R = false;

    if ((FLY_AT_PAGE == _nAnchorId) || (FLY_AT_FLY == _nAnchorId)) // LAYER_IMPL
    {
        const SwFrm* pTmp = pFrm;
        // #i22305#
        if ((FLY_AT_PAGE == _nAnchorId) ||
            ((FLY_AT_FLY == _nAnchorId) && !_bFollowTextFlow))
        {
            pFrm = pPage;
        }
        else
        {
            pFrm = pFrm->FindFlyFrm();
        }
        if ( !pFrm )
            pFrm = pTmp;
        _orRect = pFrm->Frm();
        SWRECTFN( pFrm )
        bRTL = pFrm->IsRightToLeft();
        if ( bRTL )
            aPos = pFrm->Frm().TopRight();
        else
            aPos = (pFrm->Frm().*fnRect->fnGetPos)();

        if( bVert || bVertL2R )
        {
            bVertic = bVert;
            bVerticalL2R = bVertL2R;
            _bMirror = false; // no mirroring in vertical environment
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT: aPos.Y() += pFrm->Prt().Height();
                // no break!
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.Y() += pFrm->Prt().Top(); break;
                default: break;
            }
        }
        else if ( _bMirror )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Width();
                // no break
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT: aPos.X() += pFrm->Prt().Left(); break;
                default: aPos.X() += pFrm->Frm().Width();
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Width();
                // no break!
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::FRAME_LEFT: aPos.X() += pFrm->Prt().Left() -
                                               pFrm->Frm().Width(); break;
                default: break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT:   aPos.X() += pFrm->Prt().Width();
                // no break!
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.X() += pFrm->Prt().Left(); break;
                default:break;
            }
        }

        if ( bVert && !bVertL2R )
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.X() -= pFrm->GetRightMargin();
                }
                break;
            }
        }
        else if ( bVertL2R )
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.X() += pFrm->GetLeftMargin();
                }
                break;
            }
        }
        else
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    if ( pFrm->IsPageFrm() )
                    {
                        aPos.Y() =
                            static_cast<const SwPageFrm*>(pFrm)->PrtWithoutHeaderAndFooter().Top();
                    }
                    else
                    {
                        aPos.Y() += pFrm->Prt().Top();
                    }
                }
                break;
            }
        }
        if ( _opPercent )
            *_opPercent = pFrm->Prt().SSize();
    }
    else
    {
        const SwFrm* pUpper = ( pFrm->IsPageFrm() || pFrm->IsFlyFrm() ) ?
                              pFrm : pFrm->GetUpper();
        SWRECTFN( pUpper );
        if ( _opPercent )
        {
            // If the size is relative from page, then full size should be counted from the page frame.
            if (pFormatFrmSize && pFormatFrmSize->GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
                _opPercent->setWidth(pPage->Frm().Width());
            else
                _opPercent->setWidth(pUpper->Prt().Width());

            if (pFormatFrmSize && pFormatFrmSize->GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
                // If the size is relative from page, then full size should be counted from the page frame.
                _opPercent->setHeight(pPage->Frm().Height());
            else
                _opPercent->setHeight(pUpper->Prt().Height());
        }

        bRTL = pFrm->IsRightToLeft();
        if ( bRTL )
            aPos = pFrm->Frm().TopRight();
        else
            aPos = (pFrm->Frm().*fnRect->fnGetPos)();
        // #i17567# - allow negative positions
        // for fly frames anchor to paragraph/to character.
        if ((_nAnchorId == FLY_AT_PARA) || (_nAnchorId == FLY_AT_CHAR))
        {
            // The rectangle, the fly frame can be positioned in, is determined
            // horizontally by the frame area of the horizontal environment
            // and vertically by the printing area of the vertical environment,
            // if the object follows the text flow, or by the frame area of the
            // vertical environment, if the object doesn't follow the text flow.
            // new class <SwEnvironmentOfAnchoredObject>
            objectpositioning::SwEnvironmentOfAnchoredObject aEnvOfObj(
                                                            _bFollowTextFlow );
            const SwLayoutFrm& rHoriEnvironLayFrm =
                                aEnvOfObj.GetHoriEnvironmentLayoutFrm( *pFrm );
            const SwLayoutFrm& rVertEnvironLayFrm =
                                aEnvOfObj.GetVertEnvironmentLayoutFrm( *pFrm );
            SwRect aHoriEnvironRect( rHoriEnvironLayFrm.Frm() );
            SwRect aVertEnvironRect;
            if ( _bFollowTextFlow )
            {
                aVertEnvironRect = rVertEnvironLayFrm.Prt();
                aVertEnvironRect.Pos() += rVertEnvironLayFrm.Frm().Pos();
                // #i18732# - adjust vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( bVert && !bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                    }
                    else if ( bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Left();
                    }
                    else
                    {
                        aPos.Y() = aVertEnvironRect.Top();
                    }
                }
            }
            else
            {
                OSL_ENSURE( rVertEnvironLayFrm.IsPageFrm(),
                        "<SwFEShell::CalcBoundRect(..)> - not following text flow, but vertical environment *not* page!" );
                aVertEnvironRect = rVertEnvironLayFrm.Frm();
                // #i18732# - adjustment vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( bVert && !bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Right();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.setX(aPos.getX() - rVertEnvironLayFrm.GetRightMargin());
                        }
                    }
                    else if ( bVertL2R )
                    {
                        aPos.X() = aVertEnvironRect.Left();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.setX(aPos.getX() + rVertEnvironLayFrm.GetLeftMargin());
                        }
                    }
                    else
                    {
                        aPos.Y() = aVertEnvironRect.Top();
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.setY(aPos.getY() + rVertEnvironLayFrm.GetTopMargin());
                            // add height of page header
                            const SwFrm* pTmpFrm = rVertEnvironLayFrm.Lower();
                            if ( pTmpFrm->IsHeaderFrm() )
                            {
                                aPos.setY(aPos.getY() + pTmpFrm->Frm().Height());
                            }
                        }
                    }
                }
            }

            // #i22341# - adjust vertical 'virtual' anchor position
            // (<aPos.Y()> respectively <aPos.X()>), if object is anchored to
            // character and vertical aligned at character or top of line
            // <pFrm>, which is the anchor frame or the proposed anchor frame,
            // doesn't have to be a text frame (e.g. edit a to-page anchored
            // fly frame). Thus, assure this.
            const SwTextFrm* pTextFrm( dynamic_cast<const SwTextFrm*>(pFrm) );
            if ( pTextFrm &&
                 (_nAnchorId == FLY_AT_CHAR) &&
                 ( _eVertRelOrient == text::RelOrientation::CHAR ||
                   _eVertRelOrient == text::RelOrientation::TEXT_LINE ) )
            {
                SwTwips nTop = 0L;
                if ( _eVertRelOrient == text::RelOrientation::CHAR )
                {
                    SwRect aChRect;
                    if ( _pToCharContentPos )
                    {
                        pTextFrm->GetAutoPos( aChRect, *_pToCharContentPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultContentPos( *(pTextFrm->GetTextNode()) );
                        pTextFrm->GetAutoPos( aChRect, aDefaultContentPos );
                    }
                    nTop = (aChRect.*fnRect->fnGetBottom)();
                }
                else
                {
                    if ( _pToCharContentPos )
                    {
                        pTextFrm->GetTopOfLine( nTop, *_pToCharContentPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultContentPos( *(pTextFrm->GetTextNode()) );
                        pTextFrm->GetTopOfLine( nTop, aDefaultContentPos );
                    }
                }
                if ( bVert || bVertL2R )
                {
                    aPos.setX(nTop);
                }
                else
                {
                    aPos.setY(nTop);
                }
            }

            // #i26945# - adjust horizontal 'virtual' anchor
            // position (<aPos.X()> respectively <aPos.Y()>), if object is
            // anchored to character and horizontal aligned at character.
            if ( pTextFrm &&
                 (_nAnchorId == FLY_AT_CHAR) &&
                 _eHoriRelOrient == text::RelOrientation::CHAR )
            {
                SwTwips nLeft = 0L;
                SwRect aChRect;
                if ( _pToCharContentPos )
                {
                    pTextFrm->GetAutoPos( aChRect, *_pToCharContentPos );
                }
                else
                {
                    // No content position provided. Thus, use a default one.
                    SwPosition aDefaultContentPos( *(pTextFrm->GetTextNode()) );
                    pTextFrm->GetAutoPos( aChRect, aDefaultContentPos );
                }
                nLeft = (aChRect.*fnRect->fnGetLeft)();
                if ( bVert || bVertL2R )
                {
                    aPos.setY(nLeft);
                }
                else
                {
                    aPos.setX(nLeft);
                }
            }
            if ( bVert || bVertL2R )
            {
                _orRect = SwRect( aVertEnvironRect.Left(),
                                  aHoriEnvironRect.Top(),
                                  aVertEnvironRect.Width(),
                                  aHoriEnvironRect.Height() );
            }
            else
            {
                _orRect = SwRect( aHoriEnvironRect.Left(),
                                  aVertEnvironRect.Top(),
                                  aHoriEnvironRect.Width(),
                                  aVertEnvironRect.Height() );
            }
        }
        else
        {
            if( _opRef && pFly && pFly->IsFlyInCntFrm() )
                *_opRef = static_cast<const SwFlyInCntFrm*>( pFly )->GetRefPoint();

            _orRect = pUpper->Frm();
            if( !pUpper->IsBodyFrm() )
            {
                _orRect += pUpper->Prt().Pos();
                _orRect.SSize( pUpper->Prt().SSize() );
                if ( pUpper->IsCellFrm() )//MA_FLY_HEIGHT
                {
                    const SwFrm* pTab = pUpper->FindTabFrm();
                    long nBottom = (pTab->GetUpper()->*fnRect->fnGetPrtBottom)();
                    (_orRect.*fnRect->fnSetBottom)( nBottom );
                }
            }
            // only use 90% of height for character bound
            {
                if( bVert || bVertL2R )
                    _orRect.Width( (_orRect.Width()*9)/10 );
                else
                    _orRect.Height( (_orRect.Height()*9)/10 );
            }
        }

        const SwTwips nBaseOfstForFly = ( pFrm->IsTextFrm() && pFly ) ?
                                        static_cast<const SwTextFrm*>(pFrm)->GetBaseOfstForFly( !bWrapThrough ) :
                                         0;
        if( bVert || bVertL2R )
        {
            bVertic = bVert;
            bVerticalL2R = bVertL2R;
            _bMirror = false;

            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:
                {
                    aPos.setY(aPos.getY() + pFrm->Prt().Height());
                    aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                    break;
                }
                case text::RelOrientation::PRINT_AREA:
                {
                    aPos += (pFrm->Prt().*fnRect->fnGetPos)();
                    aPos.setY(aPos.getY() + nBaseOfstForFly);
                    break;
                }
                case text::RelOrientation::PAGE_RIGHT:
                {
                    aPos.setY(pPage->Frm().Top() + pPage->Prt().Bottom());
                    break;
                }
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.setY(pPage->Frm().Top() + pPage->Prt().Top());
                    break;
                }
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME:
                {
                    aPos.setY(pPage->Frm().Top());
                    break;
                }
                case text::RelOrientation::FRAME:
                {
                    aPos.setY(aPos.getY() + nBaseOfstForFly);
                    break;
                }
                default: break;
            }
        }
        else if( _bMirror )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:   aPos.setX(aPos.getX() + pFrm->Prt().Left()); break;
                case text::RelOrientation::FRAME:
                case text::RelOrientation::FRAME_LEFT: aPos.setX(aPos.getX() + pFrm->Frm().Width()); break;
                case text::RelOrientation::PRINT_AREA: aPos.setX(aPos.getX() + pFrm->Prt().Right()); break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME: aPos.setX(pPage->Frm().Right()); break;
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.setX(pPage->Frm().Left()
                                              + pPage->Prt().Left()); break;
                default: break;
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_LEFT:
                    aPos.setX(pFrm->Frm().Left() +
                               pFrm->Prt().Left());
                    break;

                case text::RelOrientation::PRINT_AREA:
                    aPos.setX(pFrm->Frm().Left() + pFrm->Prt().Left() +
                               pFrm->Prt().Width());
                    aPos.setX(aPos.getX() + nBaseOfstForFly);
                    break;

                case text::RelOrientation::PAGE_LEFT:
                    aPos.setX(pPage->Frm().Left() + pPage->Prt().Left());
                    break;

                case text::RelOrientation::PAGE_PRINT_AREA:
                    aPos.setX(pPage->Frm().Left() + pPage->Prt().Left() +
                               pPage->Prt().Width());
                    break;

                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::PAGE_FRAME:
                    aPos.setX(pPage->Frm().Right());
                    break;

                case text::RelOrientation::FRAME:
                    aPos.setX(aPos.getX() + nBaseOfstForFly);
                    break;
                default: break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:
                    aPos.X() += pFrm->Prt().Width();
                    aPos += pFrm->Prt().Pos();
                    break;
                case text::RelOrientation::PRINT_AREA:
                    aPos += pFrm->Prt().Pos();
                    aPos.setX(aPos.getX() + nBaseOfstForFly);
                    break;
                case text::RelOrientation::PAGE_RIGHT:
                    aPos.setX(pPage->Frm().Left() + pPage->Prt().Right());
                    break;
                case text::RelOrientation::PAGE_PRINT_AREA:
                    aPos.setX(pPage->Frm().Left() + pPage->Prt().Left());
                    break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME:
                    aPos.setX(pPage->Frm().Left());
                    break;
                case text::RelOrientation::FRAME:
                    aPos.setX(aPos.getX() + nBaseOfstForFly);
                    break;
                default: break;
            }
        }

    }
    if( !_opRef )
    {
        if( bVertic && !bVerticalL2R )
            _orRect.Pos( aPos.getX() - _orRect.Width() - _orRect.Left(), _orRect.Top() - aPos.getY() );
        else if( bVerticalL2R )
            _orRect.Pos( _orRect.Left() - aPos.getX(), _orRect.Top() - aPos.getY() );
        else if ( bRTL )
            _orRect.Pos( - ( _orRect.Right() - aPos.getX() ), _orRect.Top() - aPos.getY() );
        else
            _orRect.Pos( _orRect.Left() - aPos.getX(), _orRect.Top() - aPos.getY() );
        if( _bMirror )
            _orRect.Pos( -_orRect.Right(), _orRect.Top() );
    }
}

Size SwFEShell::GetGraphicDefaultSize() const
{
    Size aRet;
    SwFlyFrm *pFly = GetSelectedFlyFrm();
    if ( pFly )
    {
        // #i32951# - due to issue #i28701# no format of a
        // newly inserted Writer fly frame or its anchor frame is performed
        // any more. Thus, it could be possible (e.g. on insert of a horizontal
        // line) that the anchor frame isn't formatted and its printing area
        // size is (0,0). If this is the case the printing area of the upper
        // of the anchor frame is taken.
        const SwFrm* pAnchorFrm = pFly->GetAnchorFrm();
        aRet = pAnchorFrm->Prt().SSize();
        if ( aRet.Width() == 0 && aRet.Height() == 0 &&
             pAnchorFrm->GetUpper() )
        {
            aRet = pAnchorFrm->GetUpper()->Prt().SSize();
        }

        SwRect aBound;
        CalcBoundRect( aBound, pFly->GetFormat()->GetAnchor().GetAnchorId());
        if ( pFly->GetAnchorFrm()->IsVertical() )
            aRet.Width() = aBound.Width();
        else
            aRet.Height() = aBound.Height();
    }
    return aRet;
}

bool SwFEShell::IsFrmVertical(const bool bEnvironment, bool& bRTL, bool& bVertL2R) const
{
    bool bVert = false;
    bRTL = false;
    bVertL2R = false;

    if ( Imp()->HasDrawView() )
    {
        const SdrMarkList &rMrkList = Imp()->GetDrawView()->GetMarkedObjectList();
        if( rMrkList.GetMarkCount() != 1 )
            return bVert;

        SdrObject* pObj = rMrkList.GetMark( 0 )->GetMarkedSdrObj();
        if ( !pObj )
        {
            OSL_FAIL( "<SwFEShell::IsFrmVertical(..)> - missing SdrObject instance in marked object list -> This is a serious situation" );
            return bVert;
        }
        // #i26791#
        SwContact* pContact = static_cast<SwContact*>(GetUserCall( pObj ));
        if ( !pContact )
        {
            OSL_FAIL( "<SwFEShell::IsFrmVertical(..)> - missing SwContact instance at marked object -> This is a serious situation" );
            return bVert;
        }
        const SwFrm* pRef = pContact->GetAnchoredObj( pObj )->GetAnchorFrm();
        if ( !pRef )
        {
            OSL_FAIL( "<SwFEShell::IsFrmVertical(..)> - missing anchor frame at marked object -> This is a serious situation" );
            return bVert;
        }

        if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr && !bEnvironment )
            pRef = static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrm();

        bVert = pRef->IsVertical();
        bRTL = pRef->IsRightToLeft();
        bVertL2R = pRef->IsVertLR();
    }

    return bVert;
}

void SwFEShell::MoveObjectIfActive( svt::EmbeddedObjectRef&, const Point& )
{
    // does not do anything, only avoids crash if the method is used for wrong shell
}

void SwFEShell::ToggleHeaderFooterEdit()
{
    // Clear objects selection
    if ( Imp()->GetDrawView()->AreObjectsMarked() )
    {
        Imp()->GetDrawView()->UnmarkAll();
        ClearMark();
    }

    SwCrsrShell::ToggleHeaderFooterEdit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
