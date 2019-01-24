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
#include <comphelper/lok.hxx>
#include <editeng/charhiddenitem.hxx>
#include <init.hxx>
#include <fesh.hxx>
#include <tabcol.hxx>
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

namespace
{
/**
 * This mutex is only used for the paste listeners, where the solar mutex can't
 * be used.
 */
osl::Mutex& GetPasteMutex()
{
    static osl::Mutex aMutex;
    return aMutex;
}
}

void SwFEShell::EndAllActionAndCall()
{
    for(SwViewShell& rCurrentShell : GetRingContainer())
    {
        if( dynamic_cast<const SwCursorShell*>( &rCurrentShell) !=  nullptr )
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
    SET_CURR_SHELL( const_cast<SwFEShell*>(this) );
    return GetLayout()->GetNextPrevContentPos( rPoint, bNext );
}

const SwRect& SwFEShell::GetAnyCurRect( CurRectType eType, const Point* pPt,
                                        const uno::Reference < embed::XEmbeddedObject >& xObj ) const
{
    const SwFrame *pFrame = Imp()->HasDrawView()
                ? ::GetFlyFromMarked( &Imp()->GetDrawView()->GetMarkedObjectList(),
                                      const_cast<SwFEShell*>(this))
                : nullptr;

    if( !pFrame )
    {
        if( pPt )
        {
            SwPosition aPos( *GetCursor()->GetPoint() );
            Point aPt( *pPt );
            GetLayout()->GetCursorOfst( &aPos, aPt );
            SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
            std::pair<Point, bool> const tmp(*pPt, true);
            pFrame = pNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
        }
        else
        {
            const bool bOldCallbackActionEnabled = GetLayout()->IsCallbackActionEnabled();
            if( bOldCallbackActionEnabled )
                GetLayout()->SetCallbackActionEnabled( false );
            pFrame = GetCurrFrame();
            if( bOldCallbackActionEnabled )
                GetLayout()->SetCallbackActionEnabled( true );
        }
    }

    if( !pFrame )
        return GetLayout()->getFrameArea();

    bool bFrame = true;
    switch ( eType )
    {
        case CurRectType::PagePrt: bFrame = false;
                                    [[fallthrough]];
        case CurRectType::Page :    pFrame = pFrame->FindPageFrame();
                                    break;

        case CurRectType::PageCalc:
                                {
                                    DisableCallbackAction a(const_cast<SwRootFrame&>(*pFrame->getRootFrame()));
                                    pFrame->Calc(Imp()->GetShell()->GetOut());
                                    pFrame = pFrame->FindPageFrame();
                                    pFrame->Calc(Imp()->GetShell()->GetOut());
                                }
                                    break;

        case CurRectType::FlyEmbeddedPrt:
                                    bFrame = false;
                                    [[fallthrough]];
        case CurRectType::FlyEmbedded:
        {
                                    const SwFrame *pFlyFrame = xObj.is() ? FindFlyFrame(xObj) : nullptr;
                                    pFrame = pFlyFrame ? pFlyFrame
                                                : pFrame->IsFlyFrame()
                                                    ? pFrame
                                                    : pFrame->FindFlyFrame();
                                    break;
        }
        case CurRectType::SectionOutsideTable :
                                    if( pFrame->IsInTab() )
                                        pFrame = pFrame->FindTabFrame();
                                    else {
                                        OSL_FAIL( "Missing Table" );
                                    }
                                    [[fallthrough]];
        case CurRectType::SectionPrt:
        case CurRectType::Section:
                                    if( pFrame->IsInSct() )
                                        pFrame = pFrame->FindSctFrame();
                                    else {
                                        OSL_FAIL( "Missing section" );
                                    }

                                    if( CurRectType::SectionPrt == eType )
                                        bFrame = false;
                                    break;

        case CurRectType::HeaderFooter:
                                    if( nullptr == (pFrame = pFrame->FindFooterOrHeader()) )
                                        return GetLayout()->getFrameArea();
                                    break;

        case CurRectType::PagesArea:
                                    return GetLayout()->GetPagesArea();

        default:                    break;
    }
    return bFrame ? pFrame->getFrameArea() : pFrame->getFramePrintArea();
}

sal_uInt16 SwFEShell::GetPageNumber( const Point &rPoint ) const
{
    const SwFrame *pPage = GetLayout()->Lower();
    while ( pPage && !pPage->getFrameArea().IsInside( rPoint ) )
        pPage = pPage->GetNext();
    if ( pPage )
        return static_cast<const SwPageFrame*>(pPage)->GetPhyPageNum();
    else
        return 0;
}

bool SwFEShell::GetPageNumber( long nYPos, bool bAtCursorPos, sal_uInt16& rPhyNum, sal_uInt16& rVirtNum, OUString &rDisplay) const
{
    const SwFrame *pPage;

    if ( bAtCursorPos )                   // get page of Cursor
    {
        pPage = GetCurrFrame( false );
        if ( pPage )
            pPage = pPage->FindPageFrame();
    }
    else if ( nYPos > -1 )              // determine page via the position
    {
        pPage = GetLayout()->Lower();
        while( pPage &&  (pPage->getFrameArea().Bottom() < nYPos ||
                            nYPos < pPage->getFrameArea().Top() ) )
            pPage = pPage->GetNext();
    }
    else                                // first visible page
    {
        pPage = Imp()->GetFirstVisPage(GetOut());
        if ( pPage && static_cast<const SwPageFrame*>(pPage)->IsEmptyPage() )
            pPage = pPage->GetNext();
    }

    if( pPage )
    {
        rPhyNum  = static_cast<const SwPageFrame*>(pPage)->GetPhyPageNum();
        rVirtNum = static_cast<const SwPageFrame*>(pPage)->GetVirtPageNum();
        const SvxNumberType& rNum = static_cast<const SwPageFrame*>(pPage)->GetPageDesc()->GetNumType();
        rDisplay = rNum.GetNumStr( rVirtNum );
    }

    return nullptr != pPage;
}

bool SwFEShell::IsDirectlyInSection() const
{
    SwFrame* pFrame = GetCurrFrame( false );
    return pFrame && pFrame->GetUpper() && pFrame->GetUpper()->IsSctFrame();
}

FrameTypeFlags SwFEShell::GetFrameType( const Point *pPt, bool bStopAtFly ) const
{
    FrameTypeFlags nReturn = FrameTypeFlags::NONE;
    const SwFrame *pFrame;
    if ( pPt )
    {
        SwPosition aPos( *GetCursor()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCursorOfst( &aPos, aPt );
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        std::pair<Point, bool> const tmp(*pPt, true);
        pFrame = pNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
    }
    else
        pFrame = GetCurrFrame( false );
    while ( pFrame )
    {
        switch ( pFrame->GetType() )
        {
            case SwFrameType::Column:
                                if( pFrame->GetUpper()->IsSctFrame() )
                                {
                                    // Check, if isn't not only a single column
                                    // from a section with footnotes at the end.
                                    if( pFrame->GetNext() || pFrame->GetPrev() )
                                        // Sectioncolumns
                                        nReturn |= ( nReturn & FrameTypeFlags::TABLE ) ?
                                            FrameTypeFlags::COLSECTOUTTAB : FrameTypeFlags::COLSECT;
                                }
                                else // only pages and frame columns
                                    nReturn |= FrameTypeFlags::COLUMN;
                                break;
            case SwFrameType::Page:
                                nReturn |= FrameTypeFlags::PAGE;
                                if( static_cast<const SwPageFrame*>(pFrame)->IsFootnotePage() )
                                    nReturn |= FrameTypeFlags::FTNPAGE;
                                break;
            case SwFrameType::Header:    nReturn |= FrameTypeFlags::HEADER;      break;
            case SwFrameType::Footer:    nReturn |= FrameTypeFlags::FOOTER;      break;
            case SwFrameType::Body:
                                if( pFrame->GetUpper()->IsPageFrame() ) // not for ColumnFrames
                                    nReturn |= FrameTypeFlags::BODY;
                                break;
            case SwFrameType::Ftn:       nReturn |= FrameTypeFlags::FOOTNOTE;    break;
            case SwFrameType::Fly:
                                if( static_cast<const SwFlyFrame*>(pFrame)->IsFlyLayFrame() )
                                    nReturn |= FrameTypeFlags::FLY_FREE;
                                else if ( static_cast<const SwFlyFrame*>(pFrame)->IsFlyAtContentFrame() )
                                    nReturn |= FrameTypeFlags::FLY_ATCNT;
                                else
                                {
                                    OSL_ENSURE( static_cast<const SwFlyFrame*>(pFrame)->IsFlyInContentFrame(),
                                            "New frametype?" );
                                    nReturn |= FrameTypeFlags::FLY_INCNT;
                                }
                                nReturn |= FrameTypeFlags::FLY_ANY;
                                if( bStopAtFly )
                                    return nReturn;
                                break;
            case SwFrameType::Tab:
            case SwFrameType::Row:
            case SwFrameType::Cell:      nReturn |= FrameTypeFlags::TABLE;       break;
            default:            /* do nothing */                break;
        }
        if ( pFrame->IsFlyFrame() )
            pFrame = static_cast<const SwFlyFrame*>(pFrame)->GetAnchorFrame();
        else
            pFrame = pFrame->GetUpper();
    }
    return nReturn;
}

void SwFEShell::ShellGetFocus()
{
    ::SetShell( this );
    SwCursorShell::ShellGetFocus();

    if ( HasDrawView() )
    {
        if (!comphelper::LibreOfficeKit::isActive())
            Imp()->GetDrawView()->showMarkHandles();
        if ( Imp()->GetDrawView()->AreObjectsMarked() )
            FrameNotify( this, FLY_DRAG_START );
    }
}

void SwFEShell::ShellLoseFocus()
{
    SwCursorShell::ShellLoseFocus();

    if ( HasDrawView() && Imp()->GetDrawView()->AreObjectsMarked() )
    {
        if (!comphelper::LibreOfficeKit::isActive())
            Imp()->GetDrawView()->hideMarkHandles();
        FrameNotify( this, FLY_DRAG_END );
    }
}

sal_uInt16 SwFEShell::GetPhyPageNum()
{
    SwFrame *pFrame = GetCurrFrame();
    if ( pFrame )
        return pFrame->GetPhyPageNum();
    return 0;
}

sal_uInt16 SwFEShell::GetVirtPageNum()
{
    SwFrame *pFrame = GetCurrFrame();
    if ( pFrame )
        return pFrame->GetVirtPageNum();
    return 0;
}

static void lcl_SetAPageOffset( sal_uInt16 nOffset, SwPageFrame* pPage, SwFEShell* pThis )
{
    pThis->StartAllAction();
    OSL_ENSURE( pPage->FindFirstBodyContent(),
            "SwFEShell _SetAPageOffset() without ContentFrame" );

    SwFormatPageDesc aDesc( pPage->GetPageDesc() );
    aDesc.SetNumOffset( nOffset );

    SwFrame *pFrame = pThis->GetCurrFrame( false );
    if ( pFrame->IsInTab() )
        pThis->GetDoc()->SetAttr( aDesc, *pFrame->FindTabFrame()->GetFormat() );
    else
    {
        pThis->GetDoc()->getIDocumentContentOperations().InsertPoolItem(
            *pThis->GetCursor(), aDesc, SetAttrMode::DEFAULT, pThis->GetLayout());
    }

    pThis->EndAllAction();
}

void SwFEShell::SetNewPageOffset( sal_uInt16 nOffset )
{
    GetLayout()->SetVirtPageNum( true );
    const SwPageFrame *pPage = GetCurrFrame( false )->FindPageFrame();
    lcl_SetAPageOffset( nOffset, const_cast<SwPageFrame*>(pPage), this );
}

void SwFEShell::SetPageOffset( sal_uInt16 nOffset )
{
    const SwPageFrame *pPage = GetCurrFrame( false )->FindPageFrame();
    const SwRootFrame* pDocLayout = GetLayout();
    while ( pPage )
    {
        const SwFrame *pFlow = pPage->FindFirstBodyContent();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrame();
            const SwFormatPageDesc& rPgDesc = pFlow->GetPageDescItem();
            if ( rPgDesc.GetNumOffset() )
            {
                pDocLayout->SetVirtPageNum( true );
                lcl_SetAPageOffset( nOffset, const_cast<SwPageFrame*>(pPage), this );
                break;
            }
        }
        pPage = static_cast<const SwPageFrame*>(pPage->GetPrev());
    }
}

sal_uInt16 SwFEShell::GetPageOffset() const
{
    const SwPageFrame *pPage = GetCurrFrame()->FindPageFrame();
    while ( pPage )
    {
        const SwFrame *pFlow = pPage->FindFirstBodyContent();
        if ( pFlow )
        {
            if ( pFlow->IsInTab() )
                pFlow = pFlow->FindTabFrame();
            ::boost::optional<sal_uInt16> oNumOffset = pFlow->GetPageDescItem().GetNumOffset();
            if ( oNumOffset )
                return oNumOffset.get();
        }
        pPage = static_cast<const SwPageFrame*>(pPage->GetPrev());
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
    SwContentFrame *pCnt = LTYPE_DRAW==eType ? nullptr : GetCurrFrame( false );
    if( LTYPE_DRAW!=eType && !pCnt )
        return;

    StartAllAction();
    SwRewriter aRewriter(SwUndoInsertLabel::CreateRewriter(rText));
    StartUndo(SwUndoId::INSERTLABEL, &aRewriter);

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
            nIdx = pCnt->FindFlyFrame()->
                        GetFormat()->GetContent().GetContentIdx()->GetIndex();
        }
        break;
    case LTYPE_TABLE:
        if( pCnt->IsInTab() )
        {
            // pass down index to the TableNode for tables
            const SwTable& rTable = *pCnt->FindTabFrame()->GetTable();
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
        OSL_ENSURE( false, "Cursor neither in table nor in fly." );
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
        //elements it's largely irrelevant what the anchor of the contained
        //type is but making it as-char by default results in very
        //good roundtripping
        if (pFlyFormat && bInnerCntIsFly)
        {
            SwNodeIndex aAnchIdx(*pFlyFormat->GetContent().GetContentIdx(), 1);
            SwTextNode *pTextNode = aAnchIdx.GetNode().GetTextNode();

            SwFormatAnchor aAnc(RndStdIds::FLY_AS_CHAR);
            sal_Int32 nInsertPos = bBefore ? pTextNode->Len() : 0;
            SwPosition aPos(*pTextNode, nInsertPos);

            aAnc.SetAnchor(&aPos);

            SwFlyFrame *pFly = GetSelectedOrCurrFlyFrame();
            OSL_ENSURE(pFly, "SetFlyFrameAttr, no Fly selected.");
            if (pFly)
            {
                SfxItemSet aSet(makeItemSetFromFormatAnchor(GetDoc()->GetAttrPool(), aAnc));
                SwFlyFrameFormat* pInnerFlyFormat = pFly->GetFormat();
                GetDoc()->SetFlyFrameAttr(*pInnerFlyFormat, aSet);
            }
            //put a hard-break after the graphic to keep it separated
            //from the caption text if the outer frame is resized
            const sal_Int32 nIndex = bBefore ? nInsertPos : 1;
            SwIndex aIdx(pTextNode, nIndex);
            pTextNode->InsertText("\n", aIdx);
            //set the hard-break to be hidden, otherwise it has
            //non-zero width in word and so hard-break flows on
            //the next line, pushing the caption text out of
            //the frame making the caption apparently disappear
            SvxCharHiddenItem aHidden(true, RES_CHRATR_HIDDEN);
            SfxItemSet aSet(GetDoc()->GetAttrPool(), {{aHidden.Which(), aHidden.Which()}});
            aSet.Put(aHidden);
            SwPaM aPam(*pTextNode, nIndex, *pTextNode, nIndex + 1);
            SetAttrSet(aSet, SetAttrMode::DEFAULT, &aPam);
        }
    }

    if (pFlyFormat)
    {
        const Point aPt(GetCursorDocPos());
        if (SwFlyFrame* pFrame = pFlyFormat->GetFrame(&aPt))
            SelectFlyFrame(*pFrame);
    }
    EndUndo();
    EndAllActionAndCall();

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
        // check if Point/Mark of current Cursor are in one table
        SwFrame *pFrame = GetCurrFrame( false );
        OSL_ENSURE( pFrame->FindTabFrame(), "Cursor not in table." );

        // search boxes via the layout
        SwSelBoxes  aBoxes;
        GetTableSel(*this, aBoxes);

        // The Cursor should be removed from the deletion area.
        // Always put them behind/on the table; via the
        // document position they will always be set to the old position
        while( !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();
        {
            /* ParkCursor->ParkCursorTab */
            ParkCursorInTab();
        }

        // call sorting on document
        bRet = mxDoc->SortTable(aBoxes, rOpt);
    }
    else
    {
        // Sort text nothing else
        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            SwPaM* pPam = &rPaM;

            SwPosition* pStart = pPam->Start();
            SwPosition* pEnd   = pPam->End();

            SwNodeIndex aPrevIdx( pStart->nNode, -1 );
            sal_uLong nOffset = pEnd->nNode.GetIndex() - pStart->nNode.GetIndex();
            const sal_Int32 nCntStt  = pStart->nContent.GetIndex();

            // Sorting
            bRet = mxDoc->SortText(*pPam, rOpt);

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

bool SwFEShell::IsColRightToLeft() const
{
    SwFrame* pFrame = GetCurrFrame();
    while (pFrame)
    {
        pFrame = pFrame->GetUpper();
        if (pFrame && pFrame->IsColumnFrame())
        {
            return pFrame->IsRightToLeft();
        }
    }
    return false;
}

sal_uInt16 SwFEShell::GetCurColNum_( const SwFrame *pFrame,
                                SwGetCurColNumPara* pPara )
{
    sal_uInt16 nRet = 0;
    while ( pFrame )
    {
        pFrame = pFrame->GetUpper();
        if( pFrame && pFrame->IsColumnFrame() )
        {
            const SwFrame *pCurFrame = pFrame;
            do {
                ++nRet;
                pFrame = pFrame->GetPrev();
            } while ( pFrame );

            if( pPara )
            {
                // now search the format, determining the columness
                pFrame = pCurFrame->GetUpper();
                while( pFrame )
                {
                    if( ( SwFrameType::Page | SwFrameType::Fly | SwFrameType::Section ) & pFrame->GetType() )
                    {
                        pPara->pFrameFormat = static_cast<const SwLayoutFrame*>(pFrame)->GetFormat();
                        pPara->pPrtRect = &pFrame->getFramePrintArea();
                        break;
                    }
                    pFrame = pFrame->GetUpper();
                }
                if( !pFrame )
                {
                    pPara->pFrameFormat = nullptr;
                    pPara->pPrtRect = nullptr;
                }
            }
            break;
        }
    }
    return nRet;
}

sal_uInt16 SwFEShell::GetCurColNum( SwGetCurColNumPara* pPara ) const
{
    OSL_ENSURE( GetCurrFrame(), "Cursor parked?" );
    return GetCurColNum_( GetCurrFrame(), pPara );
}

sal_uInt16 SwFEShell::GetCurOutColNum() const
{
    sal_uInt16 nRet = 0;
    SwFrame* pFrame = GetCurrFrame();
    OSL_ENSURE( pFrame, "Cursor parked?" );
    if( pFrame )
    {
        pFrame = pFrame->IsInTab() ? static_cast<SwFrame*>(pFrame->FindTabFrame())
                               : static_cast<SwFrame*>(pFrame->FindSctFrame());
        OSL_ENSURE( pFrame, "No Tab, no Sect" );
        if( pFrame )
            nRet = GetCurColNum_( pFrame, nullptr );
    }
    return nRet;
}

SwFEShell::SwFEShell( SwDoc& rDoc, vcl::Window *pWindow, const SwViewOption *pOptions )
    : SwEditShell( rDoc, pWindow, pOptions )
    , m_bCheckForOLEInCaption(false)
    , m_aPasteListeners(GetPasteMutex())
{
}

SwFEShell::SwFEShell( SwEditShell& rShell, vcl::Window *pWindow )
    : SwEditShell( rShell, pWindow )
    , m_bCheckForOLEInCaption(false)
    , m_aPasteListeners(GetPasteMutex())
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
                               const SwFormatFrameSize* pFormatFrameSize) const
{
    const SwFrame* pFrame;
    const SwFlyFrame* pFly;
    if( _opRef )
    {
        pFrame = GetCurrFrame();
        if( nullptr != ( pFly = pFrame->FindFlyFrame() ) )
            pFrame = pFly->GetAnchorFrame();
    }
    else
    {
        pFly = GetSelectedFlyFrame();
        pFrame = pFly ? pFly->GetAnchorFrame() : GetCurrFrame();
    }

    bool bWrapThrough = false;
    if ( pFly )
    {
        SwFlyFrameFormat* pFormat = const_cast<SwFlyFrameFormat*>(pFly->GetFormat());
        const SwFormatSurround& rSurround = pFormat->GetSurround();
        bWrapThrough = rSurround.GetSurround() == css::text::WrapTextMode_THROUGH;
    }

    const SwPageFrame* pPage = pFrame->FindPageFrame();
    _bMirror = _bMirror && !pPage->OnRightPage();

    Point aPos;
    bool bVertic = false;
    bool bRTL = false;
    bool bVerticalL2R = false;

    if ((RndStdIds::FLY_AT_PAGE == _nAnchorId) || (RndStdIds::FLY_AT_FLY == _nAnchorId)) // LAYER_IMPL
    {
        const SwFrame* pTmp = pFrame;
        // #i22305#
        if ((RndStdIds::FLY_AT_PAGE == _nAnchorId) ||
            ((RndStdIds::FLY_AT_FLY == _nAnchorId) && !_bFollowTextFlow))
        {
            pFrame = pPage;
        }
        else
        {
            pFrame = pFrame->FindFlyFrame();
        }
        if ( !pFrame )
            pFrame = pTmp;
        _orRect = pFrame->getFrameArea();
        SwRectFnSet aRectFnSet(pFrame);
        bRTL = pFrame->IsRightToLeft();
        if ( bRTL )
            aPos = pFrame->getFrameArea().TopRight();
        else
            aPos = aRectFnSet.GetPos(pFrame->getFrameArea());

        if( aRectFnSet.IsVert() || aRectFnSet.IsVertL2R() )
        {
            bVertic = aRectFnSet.IsVert();
            bVerticalL2R = aRectFnSet.IsVertL2R();
            _bMirror = false; // no mirroring in vertical environment
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT: aPos.AdjustY(pFrame->getFramePrintArea().Height() );
                    [[fallthrough]];
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.AdjustY(pFrame->getFramePrintArea().Top() ); break;
                default: break;
            }
        }
        else if ( _bMirror )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.AdjustX(pFrame->getFramePrintArea().Width() );
                    [[fallthrough]];
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT: aPos.AdjustX(pFrame->getFramePrintArea().Left() ); break;
                default: aPos.AdjustX(pFrame->getFrameArea().Width() );
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.AdjustX(pFrame->getFramePrintArea().Width() );
                    [[fallthrough]];
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::FRAME_LEFT: aPos.AdjustX(pFrame->getFramePrintArea().Left() -
                                               pFrame->getFrameArea().Width() ); break;
                default: break;
            }
        }
        else
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::FRAME_RIGHT:   aPos.AdjustX(pFrame->getFramePrintArea().Width() );
                    [[fallthrough]];
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.AdjustX(pFrame->getFramePrintArea().Left() ); break;
                default:break;
            }
        }

        if ( aRectFnSet.IsVert() && !aRectFnSet.IsVertL2R() )
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.AdjustX( -(pFrame->GetRightMargin()) );
                }
                break;
            }
        }
        else if ( aRectFnSet.IsVertL2R() )
        {
            switch ( _eVertRelOrient )
            {
                case text::RelOrientation::PRINT_AREA:
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.AdjustX(pFrame->GetLeftMargin() );
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
                    if ( pFrame->IsPageFrame() )
                    {
                        aPos.setY(
                            static_cast<const SwPageFrame*>(pFrame)->PrtWithoutHeaderAndFooter().Top() );
                    }
                    else
                    {
                        aPos.AdjustY(pFrame->getFramePrintArea().Top() );
                    }
                }
                break;
            }
        }
        if ( _opPercent )
            *_opPercent = pFrame->getFramePrintArea().SSize();
    }
    else
    {
        const SwFrame* pUpper = ( pFrame->IsPageFrame() || pFrame->IsFlyFrame() ) ?
                              pFrame : pFrame->GetUpper();
        SwRectFnSet aRectFnSet(pUpper);
        if ( _opPercent )
        {
            // If the size is relative from page, then full size should be counted from the page frame.
            if (pFormatFrameSize && pFormatFrameSize->GetWidthPercentRelation() == text::RelOrientation::PAGE_FRAME)
                _opPercent->setWidth(pPage->getFrameArea().Width());
            else
                _opPercent->setWidth(pUpper->getFramePrintArea().Width());

            if (pFormatFrameSize && pFormatFrameSize->GetHeightPercentRelation() == text::RelOrientation::PAGE_FRAME)
                // If the size is relative from page, then full size should be counted from the page frame.
                _opPercent->setHeight(pPage->getFrameArea().Height());
            else
                _opPercent->setHeight(pUpper->getFramePrintArea().Height());
        }

        bRTL = pFrame->IsRightToLeft();
        if ( bRTL )
            aPos = pFrame->getFrameArea().TopRight();
        else
            aPos = aRectFnSet.GetPos(pFrame->getFrameArea());
        // #i17567# - allow negative positions
        // for fly frames anchor to paragraph/to character.
        if ((_nAnchorId == RndStdIds::FLY_AT_PARA) || (_nAnchorId == RndStdIds::FLY_AT_CHAR))
        {
            // The rectangle, the fly frame can be positioned in, is determined
            // horizontally by the frame area of the horizontal environment
            // and vertically by the printing area of the vertical environment,
            // if the object follows the text flow, or by the frame area of the
            // vertical environment, if the object doesn't follow the text flow.
            // new class <SwEnvironmentOfAnchoredObject>
            objectpositioning::SwEnvironmentOfAnchoredObject aEnvOfObj(
                                                            _bFollowTextFlow );
            const SwLayoutFrame& rHoriEnvironLayFrame =
                                aEnvOfObj.GetHoriEnvironmentLayoutFrame( *pFrame );
            const SwLayoutFrame& rVertEnvironLayFrame =
                                aEnvOfObj.GetVertEnvironmentLayoutFrame( *pFrame );
            const SwRect& aHoriEnvironRect( rHoriEnvironLayFrame.getFrameArea() );
            SwRect aVertEnvironRect;
            if ( _bFollowTextFlow )
            {
                aVertEnvironRect = rVertEnvironLayFrame.getFramePrintArea();
                aVertEnvironRect.Pos() += rVertEnvironLayFrame.getFrameArea().Pos();
                // #i18732# - adjust vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( aRectFnSet.IsVert() && !aRectFnSet.IsVertL2R() )
                    {
                        aPos.setX( aVertEnvironRect.Right() );
                    }
                    else if ( aRectFnSet.IsVertL2R() )
                    {
                        aPos.setX( aVertEnvironRect.Left() );
                    }
                    else
                    {
                        aPos.setY( aVertEnvironRect.Top() );
                    }
                }
            }
            else
            {
                OSL_ENSURE( rVertEnvironLayFrame.IsPageFrame(),
                        "<SwFEShell::CalcBoundRect(..)> - not following text flow, but vertical environment *not* page!" );
                aVertEnvironRect = rVertEnvironLayFrame.getFrameArea();
                // #i18732# - adjustment vertical 'virtual' anchor position
                // (<aPos.Y()> respectively <aPos.X()>), if object is vertical aligned
                // to page areas.
                if ( _eVertRelOrient == text::RelOrientation::PAGE_FRAME || _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                {
                    if ( aRectFnSet.IsVert() && !aRectFnSet.IsVertL2R() )
                    {
                        aPos.setX( aVertEnvironRect.Right() );
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.setX(aPos.getX() - rVertEnvironLayFrame.GetRightMargin());
                        }
                    }
                    else if ( aRectFnSet.IsVertL2R() )
                    {
                        aPos.setX( aVertEnvironRect.Left() );
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.setX(aPos.getX() + rVertEnvironLayFrame.GetLeftMargin());
                        }
                    }
                    else
                    {
                        aPos.setY( aVertEnvironRect.Top() );
                        if ( _eVertRelOrient == text::RelOrientation::PAGE_PRINT_AREA )
                        {
                            aPos.setY(aPos.getY() + rVertEnvironLayFrame.GetTopMargin());
                            // add height of page header
                            const SwFrame* pTmpFrame = rVertEnvironLayFrame.Lower();
                            if ( pTmpFrame->IsHeaderFrame() )
                            {
                                aPos.setY(aPos.getY() + pTmpFrame->getFrameArea().Height());
                            }
                        }
                    }
                }
            }

            // #i22341# - adjust vertical 'virtual' anchor position
            // (<aPos.Y()> respectively <aPos.X()>), if object is anchored to
            // character and vertical aligned at character or top of line
            // <pFrame>, which is the anchor frame or the proposed anchor frame,
            // doesn't have to be a text frame (e.g. edit a to-page anchored
            // fly frame). Thus, assure this.
            const SwTextFrame* pTextFrame( dynamic_cast<const SwTextFrame*>(pFrame) );
            if ( pTextFrame &&
                 (_nAnchorId == RndStdIds::FLY_AT_CHAR) &&
                 ( _eVertRelOrient == text::RelOrientation::CHAR ||
                   _eVertRelOrient == text::RelOrientation::TEXT_LINE ) )
            {
                SwTwips nTop = 0;
                if ( _eVertRelOrient == text::RelOrientation::CHAR )
                {
                    SwRect aChRect;
                    if ( _pToCharContentPos )
                    {
                        pTextFrame->GetAutoPos( aChRect, *_pToCharContentPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultContentPos(*(pTextFrame->GetTextNodeFirst()));
                        pTextFrame->GetAutoPos( aChRect, aDefaultContentPos );
                    }
                    nTop = aRectFnSet.GetBottom(aChRect);
                }
                else
                {
                    if ( _pToCharContentPos )
                    {
                        pTextFrame->GetTopOfLine( nTop, *_pToCharContentPos );
                    }
                    else
                    {
                        // No content position provided. Thus, use a default one.
                        SwPosition aDefaultContentPos(*(pTextFrame->GetTextNodeFirst()));
                        pTextFrame->GetTopOfLine( nTop, aDefaultContentPos );
                    }
                }
                if ( aRectFnSet.IsVert() || aRectFnSet.IsVertL2R() )
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
            if ( pTextFrame &&
                 (_nAnchorId == RndStdIds::FLY_AT_CHAR) &&
                 _eHoriRelOrient == text::RelOrientation::CHAR )
            {
                SwTwips nLeft = 0;
                SwRect aChRect;
                if ( _pToCharContentPos )
                {
                    pTextFrame->GetAutoPos( aChRect, *_pToCharContentPos );
                }
                else
                {
                    // No content position provided. Thus, use a default one.
                    SwPosition aDefaultContentPos(*(pTextFrame->GetTextNodeFirst()));
                    pTextFrame->GetAutoPos( aChRect, aDefaultContentPos );
                }
                nLeft = aRectFnSet.GetLeft(aChRect);
                if ( aRectFnSet.IsVert() || aRectFnSet.IsVertL2R() )
                {
                    aPos.setY(nLeft);
                }
                else
                {
                    aPos.setX(nLeft);
                }
            }
            if ( aRectFnSet.IsVert() || aRectFnSet.IsVertL2R() )
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
            if( _opRef && pFly && pFly->IsFlyInContentFrame() )
                *_opRef = static_cast<const SwFlyInContentFrame*>( pFly )->GetRefPoint();

            _orRect = pUpper->getFrameArea();
            if( !pUpper->IsBodyFrame() )
            {
                _orRect += pUpper->getFramePrintArea().Pos();
                _orRect.SSize( pUpper->getFramePrintArea().SSize() );
                if ( pUpper->IsCellFrame() )//MA_FLY_HEIGHT
                {
                    const SwFrame* pTab = pUpper->FindTabFrame();
                    long nBottom = aRectFnSet.GetPrtBottom(*pTab->GetUpper());
                    aRectFnSet.SetBottom( _orRect, nBottom );
                }
            }
            // only use 90% of height for character bound
            {
                if( aRectFnSet.IsVert() || aRectFnSet.IsVertL2R() )
                    _orRect.Width( (_orRect.Width()*9)/10 );
                else
                    _orRect.Height( (_orRect.Height()*9)/10 );
            }
        }

        const SwTwips nBaseOfstForFly = ( pFrame->IsTextFrame() && pFly ) ?
                                        static_cast<const SwTextFrame*>(pFrame)->GetBaseOfstForFly( !bWrapThrough ) :
                                         0;
        if( aRectFnSet.IsVert() || aRectFnSet.IsVertL2R() )
        {
            bVertic = aRectFnSet.IsVert();
            bVerticalL2R = aRectFnSet.IsVertL2R();
            _bMirror = false;

            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_RIGHT:
                {
                    aPos.setY(aPos.getY() + pFrame->getFramePrintArea().Height());
                    aPos += aRectFnSet.GetPos(pFrame->getFramePrintArea());
                    break;
                }
                case text::RelOrientation::PRINT_AREA:
                {
                    aPos += aRectFnSet.GetPos(pFrame->getFramePrintArea());
                    aPos.setY(aPos.getY() + nBaseOfstForFly);
                    break;
                }
                case text::RelOrientation::PAGE_RIGHT:
                {
                    aPos.setY(pPage->getFrameArea().Top() + pPage->getFramePrintArea().Bottom());
                    break;
                }
                case text::RelOrientation::PAGE_PRINT_AREA:
                {
                    aPos.setY(pPage->getFrameArea().Top() + pPage->getFramePrintArea().Top());
                    break;
                }
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME:
                {
                    aPos.setY(pPage->getFrameArea().Top());
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
                case text::RelOrientation::FRAME_RIGHT:   aPos.setX(aPos.getX() + pFrame->getFramePrintArea().Left()); break;
                case text::RelOrientation::FRAME:
                case text::RelOrientation::FRAME_LEFT: aPos.setX(aPos.getX() + pFrame->getFrameArea().Width()); break;
                case text::RelOrientation::PRINT_AREA: aPos.setX(aPos.getX() + pFrame->getFramePrintArea().Right()); break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME: aPos.setX(pPage->getFrameArea().Right()); break;
                case text::RelOrientation::PAGE_PRINT_AREA: aPos.setX(pPage->getFrameArea().Left()
                                              + pPage->getFramePrintArea().Left()); break;
                default: break;
            }
        }
        else if ( bRTL )
        {
            switch ( _eHoriRelOrient )
            {
                case text::RelOrientation::FRAME_LEFT:
                    aPos.setX(pFrame->getFrameArea().Left() +
                               pFrame->getFramePrintArea().Left());
                    break;

                case text::RelOrientation::PRINT_AREA:
                    aPos.setX(pFrame->getFrameArea().Left() + pFrame->getFramePrintArea().Left() +
                               pFrame->getFramePrintArea().Width());
                    aPos.setX(aPos.getX() + nBaseOfstForFly);
                    break;

                case text::RelOrientation::PAGE_LEFT:
                    aPos.setX(pPage->getFrameArea().Left() + pPage->getFramePrintArea().Left());
                    break;

                case text::RelOrientation::PAGE_PRINT_AREA:
                    aPos.setX(pPage->getFrameArea().Left() + pPage->getFramePrintArea().Left() +
                               pPage->getFramePrintArea().Width());
                    break;

                case text::RelOrientation::PAGE_RIGHT:
                case text::RelOrientation::PAGE_FRAME:
                    aPos.setX(pPage->getFrameArea().Right());
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
                    aPos.AdjustX(pFrame->getFramePrintArea().Width() );
                    aPos += pFrame->getFramePrintArea().Pos();
                    break;
                case text::RelOrientation::PRINT_AREA:
                    aPos += pFrame->getFramePrintArea().Pos();
                    aPos.setX(aPos.getX() + nBaseOfstForFly);
                    break;
                case text::RelOrientation::PAGE_RIGHT:
                    aPos.setX(pPage->getFrameArea().Left() + pPage->getFramePrintArea().Right());
                    break;
                case text::RelOrientation::PAGE_PRINT_AREA:
                    aPos.setX(pPage->getFrameArea().Left() + pPage->getFramePrintArea().Left());
                    break;
                case text::RelOrientation::PAGE_LEFT:
                case text::RelOrientation::PAGE_FRAME:
                    aPos.setX(pPage->getFrameArea().Left());
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
    SwFlyFrame *pFly = GetSelectedFlyFrame();
    if ( pFly )
    {
        // #i32951# - due to issue #i28701# no format of a
        // newly inserted Writer fly frame or its anchor frame is performed
        // any more. Thus, it could be possible (e.g. on insert of a horizontal
        // line) that the anchor frame isn't formatted and its printing area
        // size is (0,0). If this is the case the printing area of the upper
        // of the anchor frame is taken.
        const SwFrame* pAnchorFrame = pFly->GetAnchorFrame();
        aRet = pAnchorFrame->getFramePrintArea().SSize();
        if ( aRet.Width() == 0 && aRet.Height() == 0 &&
             pAnchorFrame->GetUpper() )
        {
            aRet = pAnchorFrame->GetUpper()->getFramePrintArea().SSize();
        }

        SwRect aBound;
        CalcBoundRect( aBound, pFly->GetFormat()->GetAnchor().GetAnchorId());
        if ( pFly->GetAnchorFrame()->IsVertical() )
            aRet.setWidth( aBound.Width() );
        else
            aRet.setHeight( aBound.Height() );
    }
    return aRet;
}

bool SwFEShell::IsFrameVertical(const bool bEnvironment, bool& bRTL, bool& bVertL2R) const
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
            OSL_FAIL( "<SwFEShell::IsFrameVertical(..)> - missing SdrObject instance in marked object list -> This is a serious situation" );
            return bVert;
        }
        // #i26791#
        SwContact* pContact = GetUserCall( pObj );
        if ( !pContact )
        {
            OSL_FAIL( "<SwFEShell::IsFrameVertical(..)> - missing SwContact instance at marked object -> This is a serious situation" );
            return bVert;
        }
        const SwFrame* pRef = pContact->GetAnchoredObj( pObj )->GetAnchorFrame();
        if ( !pRef )
        {
            OSL_FAIL( "<SwFEShell::IsFrameVertical(..)> - missing anchor frame at marked object -> This is a serious situation" );
            return bVert;
        }

        if ( dynamic_cast<const SwVirtFlyDrawObj*>( pObj) !=  nullptr && !bEnvironment )
            pRef = static_cast<const SwVirtFlyDrawObj*>(pObj)->GetFlyFrame();

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

    SwCursorShell::ToggleHeaderFooterEdit();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
