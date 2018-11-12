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

#include <boost/property_tree/json_parser.hpp>

#include <PostItMgr.hxx>
#include <postithelper.hxx>

#include <AnnotationWin.hxx>
#include "frmsidebarwincontainer.hxx"
#include <accmap.hxx>

#include <SidebarWindowsConsts.hxx>
#include "AnchorOverlayObject.hxx"
#include "ShadowOverlayObject.hxx"

#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>

#include <chrdlgmodes.hxx>
#include <viewopt.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>
#include <edtwin.hxx>
#include <txtfld.hxx>
#include <txtannotationfld.hxx>
#include <ndtxt.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>
#include <SwRewriter.hxx>
#include <tools/color.hxx>
#include <unotools/datetime.hxx>

#include <swmodule.hxx>
#include <strings.hrc>
#include <cmdid.h>

#include <sfx2/request.hxx>
#include <sfx2/event.hxx>
#include <svl/srchitem.hxx>

#include <svl/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <svl/hint.hxx>

#include <svx/svdview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/outliner.hxx>
#include <editeng/outlobj.hxx>

#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <annotsh.hxx>
#include <swabstdlg.hxx>
#include <swevent.hxx>
#include <calbck.hxx>
#include <memory>

// distance between Anchor Y and initial note position
#define POSTIT_INITIAL_ANCHOR_DISTANCE      20
//distance between two postits
#define POSTIT_SPACE_BETWEEN                8
#define POSTIT_MINIMUMSIZE_WITH_META        60
#define POSTIT_SCROLL_SIDEBAR_HEIGHT        20

// if we layout more often we stop, this should never happen
#define MAX_LOOP_COUNT                      50

using namespace sw::sidebarwindows;
using namespace sw::annotation;

namespace {

    enum class CommentNotificationType { Add, Remove, Modify };

    bool comp_pos(const std::unique_ptr<SwSidebarItem>& a, const std::unique_ptr<SwSidebarItem>& b)
    {
        // sort by anchor position
        SwPosition aPosAnchorA = a->GetAnchorPosition();
        SwPosition aPosAnchorB = b->GetAnchorPosition();

        bool aAnchorAInFooter = false;
        bool aAnchorBInFooter = false;

        // is the anchor placed in Footnote or the Footer?
        if( aPosAnchorA.nNode.GetNode().FindFootnoteStartNode() || aPosAnchorA.nNode.GetNode().FindFooterStartNode() )
            aAnchorAInFooter = true;
        if( aPosAnchorB.nNode.GetNode().FindFootnoteStartNode() || aPosAnchorB.nNode.GetNode().FindFooterStartNode() )
            aAnchorBInFooter = true;

        // fdo#34800
        // if AnchorA is in footnote, and AnchorB isn't
        // we do not want to change over the position
        if( aAnchorAInFooter && !aAnchorBInFooter )
            return false;
        // if aAnchorA is not placed in a footnote, and aAnchorB is
        // force a change over
        else if( !aAnchorAInFooter && aAnchorBInFooter )
            return true;
        // If neither or both are in the footer, compare the positions.
        // Since footnotes are in Inserts section of nodes array and footers
        // in Autotext section, all footnotes precede any footers so no need
        // to check that.
        else
            return aPosAnchorA < aPosAnchorB;
    }

    /// Emits LOK notification about one addition/removal/change of a comment
    void lcl_CommentNotification(const SwView* pView, const CommentNotificationType nType, const SwSidebarItem* pItem, const sal_uInt32 nPostItId)
    {
        if (!comphelper::LibreOfficeKit::isActive())
            return;

        boost::property_tree::ptree aAnnotation;
        aAnnotation.put("action", (nType == CommentNotificationType::Add ? "Add" :
                                   (nType == CommentNotificationType::Remove ? "Remove" :
                                    (nType == CommentNotificationType::Modify ? "Modify" : "???"))));
        aAnnotation.put("id", nPostItId);
        if (nType != CommentNotificationType::Remove && pItem != nullptr)
        {
            sw::annotation::SwAnnotationWin* pWin = pItem->pPostIt.get();

            const SwPostItField* pField = pWin->GetPostItField();
            const SwRect& aRect = pWin->GetAnchorRect();
            const tools::Rectangle aSVRect(aRect.Pos().getX(),
                                    aRect.Pos().getY(),
                                    aRect.Pos().getX() + aRect.SSize().Width(),
                                    aRect.Pos().getY() + aRect.SSize().Height());
            std::vector<OString> aRects;
            for (const basegfx::B2DRange& aRange : pWin->GetAnnotationTextRanges())
            {
                const SwRect rect(aRange.getMinX(), aRange.getMinY(), aRange.getWidth(), aRange.getHeight());
                aRects.push_back(rect.SVRect().toString());
            }
            const OString sRects = comphelper::string::join("; ", aRects);

            aAnnotation.put("id", pField->GetPostItId());
            aAnnotation.put("parent", pWin->CalcParent());
            aAnnotation.put("author", pField->GetPar1().toUtf8().getStr());
            aAnnotation.put("text", pField->GetPar2().toUtf8().getStr());
            aAnnotation.put("dateTime", utl::toISO8601(pField->GetDateTime().GetUNODateTime()));
            aAnnotation.put("anchorPos", aSVRect.toString());
            aAnnotation.put("textRange", sRects.getStr());
        }

        boost::property_tree::ptree aTree;
        aTree.add_child("comment", aAnnotation);
        std::stringstream aStream;
        boost::property_tree::write_json(aStream, aTree);
        std::string aPayload = aStream.str();

        if (pView)
        {
            pView->libreOfficeKitViewCallback(LOK_CALLBACK_COMMENT, aPayload.c_str());
        }
    }

} // anonymous namespace

SwPostItMgr::SwPostItMgr(SwView* pView)
    : mpView(pView)
    , mpWrtShell(mpView->GetDocShell()->GetWrtShell())
    , mpEditWin(&mpView->GetEditWin())
    , mnEventId(nullptr)
    , mbWaitingForCalcRects(false)
    , mpActivePostIt(nullptr)
    , mbLayout(false)
    , mbLayoutHeight(0)
    , mbLayouting(false)
    , mbReadOnly(mpView->GetDocShell()->IsReadOnly())
    , mbDeleteNote(true)
    , mpAnswer(nullptr)
    , mbIsShowAnchor( false )
{
    if(!mpView->GetDrawView() )
        mpView->GetWrtShell().MakeDrawView();

    SwNoteProps aProps;
    mbIsShowAnchor = aProps.IsShowAnchor();

    //make sure we get the colour yellow always, even if not the first one of comments or redlining
    SW_MOD()->GetRedlineAuthor();

    // collect all PostIts and redline comments that exist after loading the document
    // don't check for existence for any of them, don't focus them
    AddPostIts(false,false);
    /*  this code can be used once we want redline comments in the Sidebar
    AddRedlineComments(false,false);
    */
    // we want to receive stuff like SfxHintId::DocChanged
    StartListening(*mpView->GetDocShell());
    if (!mvPostItFields.empty())
    {
        mbWaitingForCalcRects = true;
        mnEventId = Application::PostUserEvent( LINK( this, SwPostItMgr, CalcHdl) );
    }
}

SwPostItMgr::~SwPostItMgr()
{
    if ( mnEventId )
        Application::RemoveUserEvent( mnEventId );
    // forget about all our Sidebar windows
    RemoveSidebarWin();
    EndListening( *mpView->GetDocShell() );

    mPages.clear();
}

void SwPostItMgr::CheckForRemovedPostIts()
{
    IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
    bool bRemoved = false;
    auto it = mvPostItFields.begin();
    while(it != mvPostItFields.end())
    {
        if (!(*it)->UseElement(*mpWrtShell->GetLayout(), rIDRA))
        {
            EndListening(const_cast<SfxBroadcaster&>(*(*it)->GetBroadcaster()));
            std::unique_ptr<SwSidebarItem> p = std::move(*it);
            it = mvPostItFields.erase(it);
            if (GetActiveSidebarWin() == p->pPostIt)
                SetActiveSidebarWin(nullptr);
            p->pPostIt.disposeAndClear();
            bRemoved = true;
        }
        else
            ++it;
    }

    if ( bRemoved )
    {
        // make sure that no deleted items remain in page lists
        // todo: only remove deleted ones?!
        if ( mvPostItFields.empty() )
        {
            PreparePageContainer();
            PrepareView();
        }
        else
            // if postits are their make sure that page lists are not empty
            // otherwise sudden paints can cause pain (in BorderOverPageBorder)
            CalcRects();
    }
}

SwSidebarItem* SwPostItMgr::InsertItem(SfxBroadcaster* pItem, bool bCheckExistence, bool bFocus)
{
    if (bCheckExistence)
    {
        for (auto const& postItField : mvPostItFields)
        {
            if ( postItField->GetBroadcaster() == pItem )
                return nullptr;
        }
    }
    mbLayout = bFocus;

    SwSidebarItem* pAnnotationItem = nullptr;
    if (dynamic_cast< const SwFormatField *>( pItem ) !=  nullptr)
    {
        mvPostItFields.push_back(std::make_unique<SwAnnotationItem>(static_cast<SwFormatField&>(*pItem), bFocus));
        pAnnotationItem = mvPostItFields.back().get();
    }
    OSL_ENSURE(dynamic_cast< const SwFormatField *>( pItem ) !=  nullptr,"Mgr::InsertItem: seems like new stuff was added");
    StartListening(*pItem);
    return pAnnotationItem;
}

void SwPostItMgr::RemoveItem( SfxBroadcaster* pBroadcast )
{
    EndListening(*pBroadcast);
    auto i = std::find_if(mvPostItFields.begin(), mvPostItFields.end(),
        [&pBroadcast](const std::unique_ptr<SwSidebarItem>& pField) { return pField->GetBroadcaster() == pBroadcast; });
    if (i != mvPostItFields.end())
    {
        std::unique_ptr<SwSidebarItem> p = std::move(*i);
        if (GetActiveSidebarWin() == p->pPostIt)
            SetActiveSidebarWin(nullptr);
        // tdf#120487 remove from list before dispose, so comment window
        // won't be recreated due to the entry still in the list if focus
        // transferring from the pPostIt triggers relayout of postits
        mvPostItFields.erase(i);
        p->pPostIt.disposeAndClear();
    }
    mbLayout = true;
    PrepareView();
}

void SwPostItMgr::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( const SfxEventHint* pSfxEventHint = dynamic_cast<const SfxEventHint*>(&rHint) )
    {
        if ( pSfxEventHint->GetEventId() == SfxEventHintId::SwEventLayoutFinished )
        {
            if ( !mbWaitingForCalcRects && !mvPostItFields.empty())
            {
                mbWaitingForCalcRects = true;
                mnEventId = Application::PostUserEvent( LINK( this, SwPostItMgr, CalcHdl) );
            }
        }
    }
    else if ( const SwFormatFieldHint * pFormatHint = dynamic_cast<const SwFormatFieldHint*>(&rHint) )
    {
        SwFormatField* pField = const_cast <SwFormatField*>( pFormatHint->GetField() );
        switch ( pFormatHint->Which() )
        {
            case SwFormatFieldHintWhich::INSERTED :
            {
                if (!pField)
                {
                    AddPostIts();
                    break;
                }
                // get field to be inserted from hint
                if ( pField->IsFieldInDoc() )
                {
                    bool bEmpty = !HasNotes();
                    SwSidebarItem* pItem = InsertItem( pField, true, false );

                    if (bEmpty && !mvPostItFields.empty())
                        PrepareView(true);

                    // True until the layout of this post it finishes
                    if (pItem)
                        pItem->bPendingLayout = true;
                }
                else
                {
                    OSL_FAIL("Inserted field not in document!" );
                }
                break;
            }
            case SwFormatFieldHintWhich::REMOVED:
            {
                if (mbDeleteNote)
                {
                    if (!pField)
                    {
                        CheckForRemovedPostIts();
                        break;
                    }
                    RemoveItem(pField);

                    // If LOK has disabled tiled annotations, emit annotation callbacks
                    if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
                    {
                        SwPostItField* pPostItField = static_cast<SwPostItField*>(pField->GetField());
                        lcl_CommentNotification(mpView, CommentNotificationType::Remove, nullptr, pPostItField->GetPostItId());
                    }
                }
                break;
            }
            case SwFormatFieldHintWhich::FOCUS:
            {
                if (pFormatHint->GetView()== mpView)
                    Focus(rBC);
                break;
            }
            case SwFormatFieldHintWhich::CHANGED:
            {
                SwFormatField* pFormatField = dynamic_cast<SwFormatField*>(&rBC);
                for (auto const& postItField : mvPostItFields)
                {
                    if ( pFormatField == postItField->GetBroadcaster() )
                    {
                        if (postItField->pPostIt)
                        {
                            postItField->pPostIt->SetPostItText();
                            mbLayout = true;
                        }

                        // If LOK has disabled tiled annotations, emit annotation callbacks
                        if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
                        {
                            lcl_CommentNotification(mpView, CommentNotificationType::Modify, postItField.get(), 0);
                        }
                        break;
                    }
                }
                break;
            }

            case SwFormatFieldHintWhich::LANGUAGE:
            {
                SwFormatField* pFormatField = dynamic_cast<SwFormatField*>(&rBC);
                for (auto const& postItField : mvPostItFields)
                {
                    if ( pFormatField == postItField->GetBroadcaster() )
                    {
                        if (postItField->pPostIt)
                        {
                            const SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( postItField->GetFormatField().GetField()->GetLanguage() );
                            sal_uInt16 nLangWhichId = 0;
                            switch (nScriptType)
                            {
                            case SvtScriptType::LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
                            case SvtScriptType::ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                            case SvtScriptType::COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                            default: break;
                            }
                            postItField->pPostIt->SetLanguage(
                                SvxLanguageItem(
                                postItField->GetFormatField().GetField()->GetLanguage(),
                                nLangWhichId) );
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    else
    {
        SfxHintId nId = rHint.GetId();
        switch ( nId )
        {
            case SfxHintId::ModeChanged:
            {
                if ( mbReadOnly != mpView->GetDocShell()->IsReadOnly() )
                {
                    mbReadOnly = !mbReadOnly;
                    SetReadOnlyState();
                    mbLayout = true;
                }
                break;
            }
            case SfxHintId::DocChanged:
            {
                if ( mpView->GetDocShell() == &rBC )
                {
                    if ( !mbWaitingForCalcRects && !mvPostItFields.empty())
                    {
                        mbWaitingForCalcRects = true;
                        mnEventId = Application::PostUserEvent( LINK( this, SwPostItMgr, CalcHdl) );
                    }
                }
                break;
            }
            case SfxHintId::SwSplitNodeOperation:
            {
                // if we are in a SplitNode/Cut operation, do not delete note and then add again, as this will flicker
                mbDeleteNote = !mbDeleteNote;
                break;
            }
            case SfxHintId::Dying:
            {
                if ( mpView->GetDocShell() != &rBC )
                {
                    // field to be removed is the broadcaster
                    OSL_FAIL("Notification for removed SwFormatField was not sent!");
                    RemoveItem(&rBC);
                }
                break;
            }
            default: break;
        }
    }
}

void SwPostItMgr::Focus(SfxBroadcaster& rBC)
{
    if (!mpWrtShell->GetViewOptions()->IsPostIts())
    {
        SfxRequest aRequest(mpView->GetViewFrame(), SID_TOGGLE_NOTES);
        mpView->ExecViewOptions(aRequest);
    }

    for (auto const& postItField : mvPostItFields)
    {
        // field to get the focus is the broadcaster
        if ( &rBC == postItField->GetBroadcaster() )
        {
            if (postItField->pPostIt)
            {
                postItField->pPostIt->GrabFocus();
                MakeVisible(postItField->pPostIt);
            }
            else
            {
                // when the layout algorithm starts, this postit is created and receives focus
                postItField->bFocus = true;
            }
        }
    }
}

bool SwPostItMgr::CalcRects()
{
    if ( mnEventId )
    {
        // if CalcRects() was forced and an event is still pending: remove it
        // it is superfluous and also may cause reentrance problems if triggered while layouting
        Application::RemoveUserEvent( mnEventId );
        mnEventId = nullptr;
    }

    bool bChange = false;
    bool bRepair = false;
    PreparePageContainer();
    if ( !mvPostItFields.empty() )
    {
        IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
        for (auto const& pItem : mvPostItFields)
        {
            if (!pItem->UseElement(*mpWrtShell->GetLayout(), rIDRA))
            {
                OSL_FAIL("PostIt is not in doc or other wrong use");
                bRepair = true;
                continue;
            }
            const SwRect aOldAnchorRect( pItem->maLayoutInfo.mPosition );
            const SwPostItHelper::SwLayoutStatus eOldLayoutStatus = pItem->mLayoutStatus;
            const sal_uLong nOldStartNodeIdx( pItem->maLayoutInfo.mnStartNodeIdx );
            const sal_Int32 nOldStartContent( pItem->maLayoutInfo.mnStartContent );
            {
                // update layout information
                const SwTextAnnotationField* pTextAnnotationField =
                    dynamic_cast< const SwTextAnnotationField* >( pItem->GetFormatField().GetTextField() );
                const ::sw::mark::IMark* pAnnotationMark =
                    pTextAnnotationField != nullptr ? pTextAnnotationField->GetAnnotationMark() : nullptr;
                if ( pAnnotationMark != nullptr )
                {
                    pItem->mLayoutStatus =
                        SwPostItHelper::getLayoutInfos(
                            pItem->maLayoutInfo,
                            pItem->GetAnchorPosition(),
                            &pAnnotationMark->GetMarkStart() );
                }
                else
                {
                    pItem->mLayoutStatus =
                        SwPostItHelper::getLayoutInfos( pItem->maLayoutInfo, pItem->GetAnchorPosition() );
                }
            }
            bChange = bChange
                      || pItem->maLayoutInfo.mPosition != aOldAnchorRect
                      || pItem->mLayoutStatus != eOldLayoutStatus
                      || pItem->maLayoutInfo.mnStartNodeIdx != nOldStartNodeIdx
                      || pItem->maLayoutInfo.mnStartContent != nOldStartContent;
        }

        // show notes in right order in navigator
        //prevent Anchors during layout to overlap, e.g. when moving a frame
        if (mvPostItFields.size()>1 )
            std::stable_sort(mvPostItFields.begin(), mvPostItFields.end(), comp_pos);

        // sort the items into the right page vector, so layout can be done by page
        for (auto const& pItem : mvPostItFields)
        {
            if( SwPostItHelper::INVISIBLE == pItem->mLayoutStatus )
            {
                if (pItem->pPostIt)
                    pItem->pPostIt->HideNote();
                continue;
            }

            if( SwPostItHelper::HIDDEN == pItem->mLayoutStatus )
            {
                if (!mpWrtShell->GetViewOptions()->IsShowHiddenChar())
                {
                    if (pItem->pPostIt)
                        pItem->pPostIt->HideNote();
                    continue;
                }
            }

            const unsigned long aPageNum = pItem->maLayoutInfo.mnPageNumber;
            if (aPageNum > mPages.size())
            {
                const unsigned long nNumberOfPages = mPages.size();
                mPages.reserve(aPageNum);
                for (unsigned long j=0; j<aPageNum - nNumberOfPages; ++j)
                    mPages.emplace_back( new SwPostItPageItem());
            }
            mPages[aPageNum-1]->mvSidebarItems.push_back(pItem.get());
            mPages[aPageNum-1]->mPageRect = pItem->maLayoutInfo.mPageFrame;
            mPages[aPageNum-1]->eSidebarPosition = pItem->maLayoutInfo.meSidebarPosition;
        }

        if (!bChange && mpWrtShell->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE))
        {
            long nLayoutHeight = SwPostItHelper::getLayoutHeight( mpWrtShell->GetLayout() );
            if( nLayoutHeight > mbLayoutHeight )
            {
                if (mPages[0]->bScrollbar || HasScrollbars())
                    bChange = true;
            }
            else if( nLayoutHeight < mbLayoutHeight )
            {
                if (mPages[0]->bScrollbar || !BorderOverPageBorder(1))
                    bChange = true;
            }
        }
    }

    if ( bRepair )
        CheckForRemovedPostIts();

    mbLayoutHeight = SwPostItHelper::getLayoutHeight( mpWrtShell->GetLayout() );
    mbWaitingForCalcRects = false;
    return bChange;
}

bool SwPostItMgr::HasScrollbars() const
{
    for (auto const& postItField : mvPostItFields)
    {
        if (postItField->bShow && postItField->pPostIt && postItField->pPostIt->HasScrollbar())
            return true;
    }
    return false;
}

void SwPostItMgr::PreparePageContainer()
{
    // we do not just delete the SwPostItPageItem, so offset/scrollbar is not lost
    long lPageSize = mpWrtShell->GetNumPages();
    long lContainerSize = mPages.size();

    if (lContainerSize < lPageSize)
    {
        mPages.reserve(lPageSize);
        for (long i=0; i<lPageSize - lContainerSize;i++)
            mPages.emplace_back( new SwPostItPageItem());
    }
    else if (lContainerSize > lPageSize)
    {
        for (int i=mPages.size()-1; i >= lPageSize;--i)
        {
            mPages.pop_back();
        }
    }
    // only clear the list, DO NOT delete the objects itself
    for (auto const& page : mPages)
    {
        page->mvSidebarItems.clear();
        if (mvPostItFields.empty())
            page->bScrollbar = false;
    }
}

void SwPostItMgr::LayoutPostIts()
{
    bool bEnableMapMode = comphelper::LibreOfficeKit::isActive() && !mpEditWin->IsMapModeEnabled();
    if (bEnableMapMode)
        mpEditWin->EnableMapMode();

    if ( !mvPostItFields.empty() && !mbWaitingForCalcRects )
    {
        mbLayouting = true;

        //loop over all pages and do the layout
        // - create SwPostIt if necessary
        // - place SwPostIts on their initial position
        // - calculate necessary height for all PostIts together
        bool bUpdate = false;
        for (std::unique_ptr<SwPostItPageItem>& pPage : mPages)
        {
            // only layout if there are notes on this page
            if (!pPage->mvSidebarItems.empty())
            {
                std::vector<SwAnnotationWin*> aVisiblePostItList;
                unsigned long           lNeededHeight = 0;
                long                    mlPageBorder = 0;
                long                    mlPageEnd = 0;

                for (auto const& pItem : pPage->mvSidebarItems)
                {
                    VclPtr<SwAnnotationWin> pPostIt = pItem->pPostIt;

                    if (pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT )
                    {
                        // x value for notes positioning
                        mlPageBorder = mpEditWin->LogicToPixel( Point( pPage->mPageRect.Left(), 0)).X() - GetSidebarWidth(true);// - GetSidebarBorderWidth(true);
                        //bending point
                        mlPageEnd =
                            mpWrtShell->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE)
                            ? pItem->maLayoutInfo.mPagePrtArea.Left()
                            : pPage->mPageRect.Left() + 350;
                    }
                    else if (pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT )
                    {
                        // x value for notes positioning
                        mlPageBorder = mpEditWin->LogicToPixel( Point(pPage->mPageRect.Right(), 0)).X() + GetSidebarBorderWidth(true);
                        //bending point
                        mlPageEnd =
                            mpWrtShell->getIDocumentSettingAccess().get(DocumentSettingId::BROWSE_MODE)
                            ? pItem->maLayoutInfo.mPagePrtArea.Right() :
                            pPage->mPageRect.Right() - 350;
                    }

                    if (pItem->bShow)
                    {
                        long Y = mpEditWin->LogicToPixel( Point(0,pItem->maLayoutInfo.mPosition.Bottom())).Y();
                        long aPostItHeight = 0;
                        if (!pPostIt)
                        {
                            pPostIt = pItem->GetSidebarWindow( mpView->GetEditWin(),
                                                              *this );
                            pPostIt->InitControls();
                            pPostIt->SetReadonly(mbReadOnly);
                            pItem->pPostIt = pPostIt;
                            if (mpAnswer)
                            {
                                if (static_cast<bool>(pPostIt->CalcParent())) //do we really have another note in front of this one
                                    pPostIt.get()->InitAnswer(mpAnswer);
                                delete mpAnswer;
                                mpAnswer = nullptr;
                            }
                        }

                        pPostIt->SetChangeTracking(
                            pItem->mLayoutStatus,
                            GetColorAnchor(pItem->maLayoutInfo.mRedlineAuthor));
                        pPostIt->SetSidebarPosition(pPage->eSidebarPosition);
                        pPostIt->SetFollow(static_cast<bool>(pPostIt->CalcParent()));
                        aPostItHeight = ( pPostIt->GetPostItTextHeight() < pPostIt->GetMinimumSizeWithoutMeta()
                                          ? pPostIt->GetMinimumSizeWithoutMeta()
                                          : pPostIt->GetPostItTextHeight() )
                                        + pPostIt->GetMetaHeight();
                        pPostIt->SetPosSizePixelRect( mlPageBorder ,
                                                      Y - GetInitialAnchorDistance(),
                                                      GetSidebarWidth(true),
                                                      aPostItHeight,
                                                      pItem->maLayoutInfo.mPosition,
                                                      mlPageEnd );
                        pPostIt->ChangeSidebarItem( *pItem );

                        if (pItem->bFocus)
                        {
                            mbLayout = true;
                            pPostIt->GrabFocus();
                            pItem->bFocus = false;
                        }
                        // only the visible postits are used for the final layout
                        aVisiblePostItList.push_back(pPostIt);
                        lNeededHeight += pPostIt->IsFollow() ? aPostItHeight : aPostItHeight+GetSpaceBetween();
                    }
                    else // we don't want to see it
                    {
                        if (pPostIt)
                            pPostIt->HideNote();
                    }
                }

                if ((!aVisiblePostItList.empty()) && ShowNotes())
                {
                    bool bOldScrollbar = pPage->bScrollbar;
                    if (ShowNotes())
                        pPage->bScrollbar = LayoutByPage(aVisiblePostItList, pPage->mPageRect.SVRect(), lNeededHeight);
                    else
                        pPage->bScrollbar = false;
                    if (!pPage->bScrollbar)
                    {
                        pPage->lOffset = 0;
                    }
                    else
                    {
                        //when we changed our zoom level, the offset value can be to big, so lets check for the largest possible zoom value
                        long aAvailableHeight = mpEditWin->LogicToPixel(Size(0,pPage->mPageRect.Height())).Height() - 2 * GetSidebarScrollerHeight();
                        long lOffset = -1 * GetScrollSize() * (aVisiblePostItList.size() - aAvailableHeight / GetScrollSize());
                        if (pPage->lOffset < lOffset)
                            pPage->lOffset = lOffset;
                    }
                    bUpdate = (bOldScrollbar != pPage->bScrollbar) || bUpdate;
                    const long aSidebarheight = pPage->bScrollbar ? mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height() : 0;
                    /*
                                       TODO
                                       - enlarge all notes till GetNextBorder(), as we resized to average value before
                                       */
                    //lets hide the ones which overlap the page
                    for (auto const& visiblePostIt : aVisiblePostItList)
                    {
                        if (pPage->lOffset != 0)
                            visiblePostIt->TranslateTopPosition(pPage->lOffset);

                        bool bBottom  = mpEditWin->PixelToLogic(Point(0,visiblePostIt->VirtualPos().Y()+visiblePostIt->VirtualSize().Height())).Y() <= (pPage->mPageRect.Bottom()-aSidebarheight);
                        bool bTop = mpEditWin->PixelToLogic(Point(0,visiblePostIt->VirtualPos().Y())).Y() >= (pPage->mPageRect.Top()+aSidebarheight);
                        if ( bBottom && bTop )
                        {
                            // When tiled rendering, make sure that only the
                            // view that has the comment focus emits callbacks,
                            // so the editing view jumps to the comment, but
                            // not the others.
                            bool bTiledPainting = comphelper::LibreOfficeKit::isTiledPainting();
                            if (!bTiledPainting)
                                // No focus -> disable callbacks.
                                comphelper::LibreOfficeKit::setTiledPainting(!visiblePostIt->HasChildPathFocus());
                            visiblePostIt->ShowNote();
                            if (!bTiledPainting)
                            {
                                comphelper::LibreOfficeKit::setTiledPainting(bTiledPainting);
                                visiblePostIt->InvalidateControl();
                            }
                        }
                        else
                        {
                            if (mpEditWin->PixelToLogic(Point(0,visiblePostIt->VirtualPos().Y())).Y() < (pPage->mPageRect.Top()+aSidebarheight))
                            {
                                if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT )
                                    visiblePostIt->ShowAnchorOnly(Point( pPage->mPageRect.Left(),
                                                                pPage->mPageRect.Top()));
                                else if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT )
                                    visiblePostIt->ShowAnchorOnly(Point( pPage->mPageRect.Right(),
                                                                pPage->mPageRect.Top()));
                            }
                            else
                            {
                                if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT )
                                    visiblePostIt->ShowAnchorOnly(Point(pPage->mPageRect.Left(),
                                                               pPage->mPageRect.Bottom()));
                                else if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT )
                                    visiblePostIt->ShowAnchorOnly(Point(pPage->mPageRect.Right(),
                                                               pPage->mPageRect.Bottom()));
                            }
                            OSL_ENSURE(pPage->bScrollbar,"SwPostItMgr::LayoutByPage(): note overlaps, but bScrollbar is not true");
                        }
                    }
                }
                else
                {
                    for (auto const& visiblePostIt : aVisiblePostItList)
                    {
                        visiblePostIt->SetPosAndSize();
                    }

                    bool bOldScrollbar = pPage->bScrollbar;
                    pPage->bScrollbar = false;
                    bUpdate = (bOldScrollbar != pPage->bScrollbar) || bUpdate;
                }

                for (auto const& visiblePostIt : aVisiblePostItList)
                {
                    if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
                    {
                        if (visiblePostIt->GetSidebarItem().bPendingLayout)
                            lcl_CommentNotification(mpView, CommentNotificationType::Add, &visiblePostIt->GetSidebarItem(), 0);
                        else if (visiblePostIt->IsAnchorRectChanged())
                        {
                            lcl_CommentNotification(mpView, CommentNotificationType::Modify, &visiblePostIt->GetSidebarItem(), 0);
                            visiblePostIt->ResetAnchorRectChanged();
                        }
                    }

                    // Layout for this post it finished now
                    visiblePostIt->GetSidebarItem().bPendingLayout = false;
                }
            }
            else
            {
                if (pPage->bScrollbar)
                    bUpdate = true;
                pPage->bScrollbar = false;
            }
        }

        if (!ShowNotes())
        {       // we do not want to see the notes anymore -> Options-Writer-View-Notes
            IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
            bool bRepair = false;
            for (auto const& postItField : mvPostItFields)
            {
                if (!postItField->UseElement(*mpWrtShell->GetLayout(), rIDRA))
                {
                    OSL_FAIL("PostIt is not in doc!");
                    bRepair = true;
                    continue;
                }

                if (postItField->pPostIt)
                {
                    postItField->pPostIt->HideNote();
                    if (postItField->pPostIt->HasChildPathFocus())
                    {
                        SetActiveSidebarWin(nullptr);
                        postItField->pPostIt->GrabFocusToDocument();
                    }
                }
            }

            if ( bRepair )
                CheckForRemovedPostIts();
        }

        // notes scrollbar is otherwise not drawn correctly for some cases
        // scrollbar area is enough
        if (bUpdate)
            mpEditWin->Invalidate(); /*This is a super expensive relayout and render of the entire page*/

        mbLayouting = false;
    }

    if (bEnableMapMode)
        mpEditWin->EnableMapMode(false);
}

bool SwPostItMgr::BorderOverPageBorder(unsigned long aPage) const
{
    if ( mPages[aPage-1]->mvSidebarItems.empty() )
    {
        OSL_FAIL("Notes SidePane painted but no rects and page lists calculated!");
        return false;
    }

    auto aItem = mPages[aPage-1]->mvSidebarItems.end();
    --aItem;
    OSL_ENSURE ((*aItem)->pPostIt,"BorderOverPageBorder: NULL postIt, should never happen");
    if ((*aItem)->pPostIt)
    {
        const long aSidebarheight = mPages[aPage-1]->bScrollbar ? mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height() : 0;
        const long aEndValue = mpEditWin->PixelToLogic(Point(0,(*aItem)->pPostIt->GetPosPixel().Y()+(*aItem)->pPostIt->GetSizePixel().Height())).Y();
        return aEndValue <= mPages[aPage-1]->mPageRect.Bottom()-aSidebarheight;
    }
    else
        return false;
}

void SwPostItMgr::DrawNotesForPage(OutputDevice *pOutDev, sal_uInt32 nPage)
{
    assert(nPage < mPages.size());
    if (nPage >= mPages.size())
        return;
    for (auto const& pItem : mPages[nPage]->mvSidebarItems)
    {
        SwAnnotationWin* pPostIt = pItem->pPostIt;
        if (!pPostIt)
            continue;
        Point aPoint(mpEditWin->PixelToLogic(pPostIt->GetPosPixel()));
        Size aSize(pPostIt->PixelToLogic(pPostIt->GetSizePixel()));
        pPostIt->Draw(pOutDev, aPoint, aSize, DrawFlags::NONE);
    }
}

void SwPostItMgr::PaintTile(OutputDevice& rRenderContext)
{
    for (std::unique_ptr<SwSidebarItem>& pItem : mvPostItFields)
    {
        SwAnnotationWin* pPostIt = pItem->pPostIt;
        if (!pPostIt)
            continue;

        bool bEnableMapMode = !mpEditWin->IsMapModeEnabled();
        mpEditWin->EnableMapMode();
        rRenderContext.Push(PushFlags::MAPMODE);
        Point aOffset(mpEditWin->PixelToLogic(pPostIt->GetPosPixel()));
        MapMode aMapMode(rRenderContext.GetMapMode());
        aMapMode.SetOrigin(aMapMode.GetOrigin() + aOffset);
        rRenderContext.SetMapMode(aMapMode);
        Size aSize(rRenderContext.PixelToLogic(pPostIt->GetSizePixel()));
        tools::Rectangle aRectangle(Point(0, 0), aSize);

        pPostIt->PaintTile(rRenderContext, aRectangle);

        rRenderContext.Pop();
        if (bEnableMapMode)
            mpEditWin->EnableMapMode(false);
    }
}

void SwPostItMgr::Scroll(const long lScroll,const unsigned long aPage)
{
    OSL_ENSURE((lScroll % GetScrollSize() )==0,"SwPostItMgr::Scroll: scrolling by wrong value");
    // do not scroll more than necessary up or down
    if ( ((mPages[aPage-1]->lOffset == 0) && (lScroll>0)) || ( BorderOverPageBorder(aPage) && (lScroll<0)) )
        return;

    const bool bOldUp = ArrowEnabled(KEY_PAGEUP,aPage);
    const bool bOldDown = ArrowEnabled(KEY_PAGEDOWN,aPage);
    const long aSidebarheight = mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height();
    for (auto const& item : mPages[aPage-1]->mvSidebarItems)
    {
        SwAnnotationWin* pPostIt = item->pPostIt;
        // if this is an answer, we should take the normal position and not the real, slightly moved position
        pPostIt->SetVirtualPosSize(pPostIt->GetPosPixel(),pPostIt->GetSizePixel());
        pPostIt->TranslateTopPosition(lScroll);

        if (item->bShow)
        {
            bool bBottom  = mpEditWin->PixelToLogic(Point(0,pPostIt->VirtualPos().Y()+pPostIt->VirtualSize().Height())).Y() <= (mPages[aPage-1]->mPageRect.Bottom()-aSidebarheight);
            bool bTop = mpEditWin->PixelToLogic(Point(0,pPostIt->VirtualPos().Y())).Y() >=   (mPages[aPage-1]->mPageRect.Top()+aSidebarheight);
            if ( bBottom && bTop)
            {
                    pPostIt->ShowNote();
            }
            else
            {
                if ( mpEditWin->PixelToLogic(Point(0,pPostIt->VirtualPos().Y())).Y() < (mPages[aPage-1]->mPageRect.Top()+aSidebarheight))
                {
                    if (mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT)
                        pPostIt->ShowAnchorOnly(Point(mPages[aPage-1]->mPageRect.Left(),mPages[aPage-1]->mPageRect.Top()));
                    else if (mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT)
                        pPostIt->ShowAnchorOnly(Point(mPages[aPage-1]->mPageRect.Right(),mPages[aPage-1]->mPageRect.Top()));
                }
                else
                {
                    if (mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT)
                        pPostIt->ShowAnchorOnly(Point(mPages[aPage-1]->mPageRect.Left(),mPages[aPage-1]->mPageRect.Bottom()));
                    else if (mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT)
                        pPostIt->ShowAnchorOnly(Point(mPages[aPage-1]->mPageRect.Right(),mPages[aPage-1]->mPageRect.Bottom()));
                }
            }
        }
    }
    mPages[aPage-1]->lOffset += lScroll;
    if ( (bOldUp != ArrowEnabled(KEY_PAGEUP,aPage)) ||(bOldDown != ArrowEnabled(KEY_PAGEDOWN,aPage)) )
    {
        mpEditWin->Invalidate(GetBottomScrollRect(aPage));
        mpEditWin->Invalidate(GetTopScrollRect(aPage));
    }
}

void SwPostItMgr::AutoScroll(const SwAnnotationWin* pPostIt,const unsigned long aPage )
{
    // otherwise all notes are visible
    if (mPages[aPage-1]->bScrollbar)
    {
        const long aSidebarheight = mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height();
        const bool bBottom  = mpEditWin->PixelToLogic(Point(0,pPostIt->GetPosPixel().Y()+pPostIt->GetSizePixel().Height())).Y() <= (mPages[aPage-1]->mPageRect.Bottom()-aSidebarheight);
        const bool bTop = mpEditWin->PixelToLogic(Point(0,pPostIt->GetPosPixel().Y())).Y() >= (mPages[aPage-1]->mPageRect.Top()+aSidebarheight);
        if ( !(bBottom && bTop))
        {
            const long aDiff = bBottom ? mpEditWin->LogicToPixel(Point(0,mPages[aPage-1]->mPageRect.Top() + aSidebarheight)).Y() - pPostIt->GetPosPixel().Y() :
                                            mpEditWin->LogicToPixel(Point(0,mPages[aPage-1]->mPageRect.Bottom() - aSidebarheight)).Y() - (pPostIt->GetPosPixel().Y()+pPostIt->GetSizePixel().Height());
            // this just adds the missing value to get the next a* GetScrollSize() after aDiff
            // e.g aDiff= 61 POSTIT_SCROLL=50 --> lScroll = 100
            const auto nScrollSize = GetScrollSize();
            assert(nScrollSize);
            const long lScroll = bBottom ? (aDiff + ( nScrollSize - (aDiff % nScrollSize))) : (aDiff - (nScrollSize + (aDiff % nScrollSize)));
            Scroll(lScroll, aPage);
        }
    }
}

void SwPostItMgr::MakeVisible(const SwAnnotationWin* pPostIt )
{
    long aPage = -1;
    // we don't know the page yet, lets find it ourselves
    std::vector<SwPostItPageItem*>::size_type n=0;
    for (auto const& page : mPages)
    {
        for (auto const& item : page->mvSidebarItems)
        {
            if (item->pPostIt==pPostIt)
            {
                aPage = n+1;
                break;
            }
        }
        ++n;
    }
    if (aPage!=-1)
        AutoScroll(pPostIt,aPage);
    tools::Rectangle aNoteRect (Point(pPostIt->GetPosPixel().X(),pPostIt->GetPosPixel().Y()-5),pPostIt->GetSizePixel());
    if (!aNoteRect.IsEmpty())
        mpWrtShell->MakeVisible(SwRect(mpEditWin->PixelToLogic(aNoteRect)));
}

bool SwPostItMgr::ArrowEnabled(sal_uInt16 aDirection,unsigned long aPage) const
{
    switch (aDirection)
    {
        case KEY_PAGEUP:
            {
                return (mPages[aPage-1]->lOffset != 0);
            }
        case KEY_PAGEDOWN:
            {
                return (!BorderOverPageBorder(aPage));
            }
        default: return false;
    }
}

Color SwPostItMgr::GetArrowColor(sal_uInt16 aDirection,unsigned long aPage) const
{
    if (ArrowEnabled(aDirection,aPage))
    {
        if (Application::GetSettings().GetStyleSettings().GetHighContrastMode())
            return COL_WHITE;
        else
            return COL_NOTES_SIDEPANE_ARROW_ENABLED;
    }
    else
    {
        return COL_NOTES_SIDEPANE_ARROW_DISABLED;
    }
}

bool SwPostItMgr::LayoutByPage(std::vector<SwAnnotationWin*> &aVisiblePostItList, const tools::Rectangle& rBorder, long lNeededHeight)
{
    /*** General layout idea:***/
    //  - if we have space left, we always move the current one up,
    //    otherwise the next one down
    //  - first all notes are resized
    //  - then the real layout starts

    //rBorder is the page rect
    const tools::Rectangle aBorder         = mpEditWin->LogicToPixel(rBorder);
    long            lTopBorder      = aBorder.Top() + 5;
    long            lBottomBorder   = aBorder.Bottom() - 5;
    const long      lVisibleHeight  = lBottomBorder - lTopBorder; //aBorder.GetHeight() ;
    const size_t    nPostItListSize = aVisiblePostItList.size();
    long            lTranslatePos   = 0;
    bool            bScrollbars     = false;

    // do all necessary resizings
    if (nPostItListSize > 0 && lVisibleHeight < lNeededHeight)
    {
        // ok, now we have to really resize and adding scrollbars
        const long lAverageHeight = (lVisibleHeight - nPostItListSize*GetSpaceBetween()) / nPostItListSize;
        if (lAverageHeight<GetMinimumSizeWithMeta())
        {
            bScrollbars = true;
            lTopBorder += GetSidebarScrollerHeight() + 10;
            lBottomBorder -= (GetSidebarScrollerHeight() + 10);
            for (auto const& visiblePostIt : aVisiblePostItList)
                visiblePostIt->SetSize(Size(visiblePostIt->VirtualSize().getWidth(),visiblePostIt->GetMinimumSizeWithMeta()));
        }
        else
        {
            for (auto const& visiblePostIt : aVisiblePostItList)
            {
                if ( visiblePostIt->VirtualSize().getHeight() > lAverageHeight)
                    visiblePostIt->SetSize(Size(visiblePostIt->VirtualSize().getWidth(),lAverageHeight));
            }
        }
    }

    //start the real layout so nothing overlaps anymore
    if (aVisiblePostItList.size()>1)
    {
        int loop = 0;
        bool bDone = false;
        // if no window is moved anymore we are finished
        while (!bDone)
        {
            loop++;
            bDone = true;
            long lSpaceUsed = lTopBorder + GetSpaceBetween();
            for(auto i = aVisiblePostItList.begin(); i != aVisiblePostItList.end() ; ++i)
            {
                auto aNextPostIt = i;
                ++aNextPostIt;

                if (aNextPostIt != aVisiblePostItList.end())
                {
                    lTranslatePos = ( (*i)->VirtualPos().Y() + (*i)->VirtualSize().Height()) - (*aNextPostIt)->VirtualPos().Y();
                    if (lTranslatePos > 0) // note windows overlaps the next one
                    {
                        // we are not done yet, loop at least once more
                        bDone = false;
                        // if there is space left, move the current note up
                        // it could also happen that there is no space left for the first note due to a scrollbar
                        // then we also jump into, so we move the current one up and the next one down
                        if ( (lSpaceUsed <= (*i)->VirtualPos().Y()) || (i==aVisiblePostItList.begin()))
                        {
                            // we have space left, so let's move the current one up
                            if ( ((*i)->VirtualPos().Y()- lTranslatePos - GetSpaceBetween()) > lTopBorder)
                            {
                                if ((*aNextPostIt)->IsFollow())
                                    (*i)->TranslateTopPosition(-1*(lTranslatePos+ANCHORLINE_WIDTH));
                                else
                                    (*i)->TranslateTopPosition(-1*(lTranslatePos+GetSpaceBetween()));
                            }
                            else
                            {
                                long lMoveUp = (*i)->VirtualPos().Y() - lTopBorder;
                                (*i)->TranslateTopPosition(-1* lMoveUp);
                                if ((*aNextPostIt)->IsFollow())
                                    (*aNextPostIt)->TranslateTopPosition( (lTranslatePos+ANCHORLINE_WIDTH) - lMoveUp);
                                else
                                    (*aNextPostIt)->TranslateTopPosition( (lTranslatePos+GetSpaceBetween()) - lMoveUp);
                            }
                        }
                        else
                        {
                            // no space left, left move the next one down
                            if ((*aNextPostIt)->IsFollow())
                                (*aNextPostIt)->TranslateTopPosition(lTranslatePos+ANCHORLINE_WIDTH);
                            else
                                (*aNextPostIt)->TranslateTopPosition(lTranslatePos+GetSpaceBetween());
                        }
                    }
                    else
                    {
                        // the first one could overlap the topborder instead of a second note
                        if (i==aVisiblePostItList.begin())
                        {
                            long lMoveDown = lTopBorder - (*i)->VirtualPos().Y();
                            if (lMoveDown>0)
                            {
                                bDone = false;
                                (*i)->TranslateTopPosition( lMoveDown);
                            }
                        }
                    }
                    if ( (*aNextPostIt)->IsFollow() )
                        lSpaceUsed += (*i)->VirtualSize().Height() + ANCHORLINE_WIDTH;
                    else
                        lSpaceUsed += (*i)->VirtualSize().Height() + GetSpaceBetween();
                }
                else
                {
                    //(*i) is the last visible item
                    auto aPrevPostIt = i;
                    --aPrevPostIt;
                    lTranslatePos = ( (*aPrevPostIt)->VirtualPos().Y() + (*aPrevPostIt)->VirtualSize().Height() ) - (*i)->VirtualPos().Y();
                    if (lTranslatePos > 0)
                    {
                        bDone = false;
                        if ( ((*i)->VirtualPos().Y()+ (*i)->VirtualSize().Height()+lTranslatePos) < lBottomBorder)
                        {
                            if ( (*i)->IsFollow() )
                                (*i)->TranslateTopPosition(lTranslatePos+ANCHORLINE_WIDTH);
                            else
                                (*i)->TranslateTopPosition(lTranslatePos+GetSpaceBetween());
                        }
                        else
                        {
                            (*i)->TranslateTopPosition(lBottomBorder - ((*i)->VirtualPos().Y()+ (*i)->VirtualSize().Height()) );
                        }
                    }
                    else
                    {
                        // note does not overlap, but we might be over the lower border
                        // only do this if there are no scrollbars, otherwise notes are supposed to overlap the border
                        if (!bScrollbars && ((*i)->VirtualPos().Y()+ (*i)->VirtualSize().Height() > lBottomBorder) )
                        {
                            bDone = false;
                            (*i)->TranslateTopPosition(lBottomBorder - ((*i)->VirtualPos().Y()+ (*i)->VirtualSize().Height()));
                        }
                    }
                }
            }
            // security check so we don't loop forever
            if (loop>MAX_LOOP_COUNT)
            {
                OSL_FAIL("PostItMgr::Layout(): We are looping forever");
                break;
            }
        }
    }
    else
    {
        // only one left, make sure it is not hidden at the top or bottom
        auto i = aVisiblePostItList.begin();
        lTranslatePos = lTopBorder - (*i)->VirtualPos().Y();
        if (lTranslatePos>0)
        {
            (*i)->TranslateTopPosition(lTranslatePos+GetSpaceBetween());
        }
        lTranslatePos = lBottomBorder - ((*i)->VirtualPos().Y()+ (*i)->VirtualSize().Height());
        if (lTranslatePos<0)
        {
            (*i)->TranslateTopPosition(lTranslatePos);
        }
    }
    return bScrollbars;
 }

void SwPostItMgr::AddPostIts(bool bCheckExistence, bool bFocus)
{
    bool bEmpty = mvPostItFields.empty();
    SwFieldType* pType = mpView->GetDocShell()->GetDoc()->getIDocumentFieldsAccess().GetFieldType(SwFieldIds::Postit, OUString(),false);
    SwIterator<SwFormatField,SwFieldType> aIter( *pType );
    SwFormatField* pSwFormatField = aIter.First();
    while(pSwFormatField)
    {
        if ( pSwFormatField->GetTextField())
        {
            IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
            if (pSwFormatField->IsFieldInDoc()
                && (!mpWrtShell->GetLayout()->IsHideRedlines()
                    || !sw::IsFieldDeletedInModel(rIDRA, *pSwFormatField->GetTextField())))
            {
                InsertItem(pSwFormatField,bCheckExistence,bFocus);
            }
        }
        pSwFormatField = aIter.Next();
    }

    // if we just added the first one we have to update the view for centering
    if (bEmpty && !mvPostItFields.empty())
        PrepareView(true);
}

void SwPostItMgr::RemoveSidebarWin()
{
    for (auto& postItField : mvPostItFields)
    {
        EndListening( *const_cast<SfxBroadcaster*>(postItField->GetBroadcaster()) );
        postItField->pPostIt.disposeAndClear();
        postItField.reset();
    }
    mvPostItFields.clear();

    // all postits removed, no items should be left in pages
    PreparePageContainer();
}

class FilterFunctor
{
public:
    virtual bool operator()(const SwFormatField* pField) const = 0;
    virtual ~FilterFunctor() {}
};

class IsPostitField : public FilterFunctor
{
public:
    bool operator()(const SwFormatField* pField) const override
    {
        return pField->GetField()->GetTyp()->Which() == SwFieldIds::Postit;
    }
};

class IsPostitFieldWithAuthorOf : public FilterFunctor
{
    OUString const m_sAuthor;
public:
    explicit IsPostitFieldWithAuthorOf(const OUString &rAuthor)
        : m_sAuthor(rAuthor)
    {
    }
    bool operator()(const SwFormatField* pField) const override
    {
        if (pField->GetField()->GetTyp()->Which() != SwFieldIds::Postit)
            return false;
        return static_cast<const SwPostItField*>(pField->GetField())->GetPar1() == m_sAuthor;
    }
};

class IsPostitFieldWithPostitId : public FilterFunctor
{
    sal_uInt32 const m_nPostItId;
public:
    explicit IsPostitFieldWithPostitId(sal_uInt32 nPostItId)
        : m_nPostItId(nPostItId)
        {}

    bool operator()(const SwFormatField* pField) const override
    {
        if (pField->GetField()->GetTyp()->Which() != SwFieldIds::Postit)
            return false;
        return static_cast<const SwPostItField*>(pField->GetField())->GetPostItId() == m_nPostItId;
    }
};

class IsFieldNotDeleted : public FilterFunctor
{
private:
    IDocumentRedlineAccess const& m_rIDRA;
    FilterFunctor const& m_rNext;

public:
    IsFieldNotDeleted(IDocumentRedlineAccess const& rIDRA,
            const FilterFunctor & rNext)
        : m_rIDRA(rIDRA)
        , m_rNext(rNext)
    {
    }
    bool operator()(const SwFormatField* pField) const override
    {
        if (!m_rNext(pField))
            return false;
        if (!pField->GetTextField())
            return false;
        return !sw::IsFieldDeletedInModel(m_rIDRA, *pField->GetTextField());
    }
};

//Manages the passed in vector by automatically removing entries if they are deleted
//and automatically adding entries if they appear in the document and match the
//functor.
//
//This will completely refill in the case of a "anonymous" NULL pField stating
//rather unhelpfully that "something changed" so you may process the same
//Fields more than once.
class FieldDocWatchingStack : public SfxListener
{
    std::vector<std::unique_ptr<SwSidebarItem>>& sidebarItemVector;
    std::vector<const SwFormatField*> v;
    SwDocShell& m_rDocShell;
    FilterFunctor& m_rFilter;

    virtual void Notify(SfxBroadcaster&, const SfxHint& rHint) override
    {
        const SwFormatFieldHint* pHint = dynamic_cast<const SwFormatFieldHint*>(&rHint);
        if (pHint)
        {
            bool bAllInvalidated = false;
            if (pHint->Which() == SwFormatFieldHintWhich::REMOVED)
            {
                const SwFormatField* pField = pHint->GetField();
                bAllInvalidated = pField == nullptr;
                if (!bAllInvalidated && m_rFilter(pField))
                {
                    EndListening(const_cast<SwFormatField&>(*pField));
                    v.erase(std::remove(v.begin(), v.end(), pField), v.end());
                }
            }
            else if (pHint->Which() == SwFormatFieldHintWhich::INSERTED)
            {
                const SwFormatField* pField = pHint->GetField();
                bAllInvalidated = pField == nullptr;
                if (!bAllInvalidated && m_rFilter(pField))
                {
                    StartListening(const_cast<SwFormatField&>(*pField));
                    v.push_back(pField);
                }
            }

            if (bAllInvalidated)
                FillVector();

            return;
        }
    }

public:
    FieldDocWatchingStack(std::vector<std::unique_ptr<SwSidebarItem>>& in, SwDocShell &rDocShell, FilterFunctor& rFilter)
        : sidebarItemVector(in)
        , m_rDocShell(rDocShell)
        , m_rFilter(rFilter)
    {
        FillVector();
        StartListening(m_rDocShell);
    }
    void FillVector()
    {
        EndListeningToAllFields();
        v.clear();
        v.reserve(sidebarItemVector.size());
        for (auto const& p : sidebarItemVector)
        {
            const SwFormatField& rField = p->GetFormatField();
            if (!m_rFilter(&rField))
                continue;
            StartListening(const_cast<SwFormatField&>(rField));
            v.push_back(&rField);
        }
    }
    void EndListeningToAllFields()
    {
        for (auto const& pField : v)
        {
            EndListening(const_cast<SwFormatField&>(*pField));
        }
    }
    virtual ~FieldDocWatchingStack() override
    {
        EndListeningToAllFields();
        EndListening(m_rDocShell);
    }
    const SwFormatField* pop()
    {
        if (v.empty())
            return nullptr;
        const SwFormatField* p = v.back();
        EndListening(const_cast<SwFormatField&>(*p));
        v.pop_back();
        return p;
    }
};

// copy to new vector, otherwise RemoveItem would operate and delete stuff on mvPostItFields as well
// RemoveItem will clean up the core field and visible postit if necessary
// we cannot just delete everything as before, as postits could move into change tracking
void SwPostItMgr::Delete(const OUString& rAuthor)
{
    mpWrtShell->StartAllAction();
    if (HasActiveSidebarWin() && (GetActiveSidebarWin()->GetAuthor() == rAuthor))
    {
        SetActiveSidebarWin(nullptr);
    }
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_DELETE_AUTHOR_NOTES) + rAuthor);
    mpWrtShell->StartUndo( SwUndoId::DELETE, &aRewriter );

    IsPostitFieldWithAuthorOf aFilter(rAuthor);
    IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
    IsFieldNotDeleted aFilter2(rIDRA, aFilter);
    FieldDocWatchingStack aStack(mvPostItFields, *mpView->GetDocShell(), aFilter2);
    while (const SwFormatField* pField = aStack.pop())
    {
        if (mpWrtShell->GotoField(*pField))
            mpWrtShell->DelRight();
    }
    mpWrtShell->EndUndo();
    PrepareView();
    mpWrtShell->EndAllAction();
    mbLayout = true;
    CalcRects();
    LayoutPostIts();
}

void SwPostItMgr::Delete(sal_uInt32 nPostItId)
{
    mpWrtShell->StartAllAction();
    if (HasActiveSidebarWin() &&
        mpActivePostIt.get()->GetPostItField()->GetPostItId() == nPostItId)
    {
        SetActiveSidebarWin(nullptr);
    }
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_CONTENT_TYPE_SINGLE_POSTIT));
    mpWrtShell->StartUndo( SwUndoId::DELETE, &aRewriter );

    IsPostitFieldWithPostitId aFilter(nPostItId);
    IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
    IsFieldNotDeleted aFilter2(rIDRA, aFilter);
    FieldDocWatchingStack aStack(mvPostItFields, *mpView->GetDocShell(), aFilter2);
    const SwFormatField* pField = aStack.pop();
    if (pField && mpWrtShell->GotoField(*pField))
        mpWrtShell->DelRight();
    mpWrtShell->EndUndo();
    PrepareView();
    mpWrtShell->EndAllAction();
    mbLayout = true;
    CalcRects();
    LayoutPostIts();
}

void SwPostItMgr::Delete()
{
    mpWrtShell->StartAllAction();
    SetActiveSidebarWin(nullptr);
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_DELETE_ALL_NOTES) );
    mpWrtShell->StartUndo( SwUndoId::DELETE, &aRewriter );

    IsPostitField aFilter;
    IDocumentRedlineAccess const& rIDRA(mpWrtShell->getIDocumentRedlineAccess());
    IsFieldNotDeleted aFilter2(rIDRA, aFilter);
    FieldDocWatchingStack aStack(mvPostItFields, *mpView->GetDocShell(),
        aFilter2);
    while (const SwFormatField* pField = aStack.pop())
    {
        if (mpWrtShell->GotoField(*pField))
            mpWrtShell->DelRight();
    }

    mpWrtShell->EndUndo();
    PrepareView();
    mpWrtShell->EndAllAction();
    mbLayout = true;
    CalcRects();
    LayoutPostIts();
}

void SwPostItMgr::ExecuteFormatAllDialog(SwView& rView)
{
    if (mvPostItFields.empty())
        return;
    sw::annotation::SwAnnotationWin *pOrigActiveWin = GetActiveSidebarWin();
    sw::annotation::SwAnnotationWin *pWin = pOrigActiveWin;
    if (!pWin)
    {
        for (auto const& postItField : mvPostItFields)
        {
            pWin = postItField->pPostIt;
            if (pWin)
                break;
        }
    }
    if (!pWin)
        return;
    SetActiveSidebarWin(pWin);
    OutlinerView* pOLV = pWin->GetOutlinerView();
    SfxItemSet aEditAttr(pOLV->GetAttribs());
    SfxItemPool* pPool(SwAnnotationShell::GetAnnotationPool(rView));
    SfxItemSet aDlgAttr(*pPool, svl::Items<EE_ITEMS_START, EE_ITEMS_END>{});
    aDlgAttr.Put(aEditAttr);
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSwCharDlg(rView.GetFrameWeld(), rView, aDlgAttr, SwCharDlgMode::Ann));
    sal_uInt16 nRet = pDlg->Execute();
    if (RET_OK == nRet)
    {
        aDlgAttr.Put(*pDlg->GetOutputItemSet());
        FormatAll(aDlgAttr);
    }
    pDlg.disposeAndClear();
    SetActiveSidebarWin(pOrigActiveWin);
}

void SwPostItMgr::FormatAll(const SfxItemSet &rNewAttr)
{
    mpWrtShell->StartAllAction();
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SwResId(STR_FORMAT_ALL_NOTES) );
    mpWrtShell->StartUndo( SwUndoId::INSATTR, &aRewriter );

    for (auto const& postItField : mvPostItFields)
    {
        if (!postItField->pPostIt)
            continue;
        OutlinerView* pOLV = postItField->pPostIt->GetOutlinerView();
        //save old selection
        ESelection aOrigSel(pOLV->GetSelection());
        //select all
        Outliner *pOutliner = pOLV->GetOutliner();
        if (pOutliner)
        {
            sal_Int32 nParaCount = pOutliner->GetParagraphCount();
            if (nParaCount > 0)
                pOLV->SelectRange(0, nParaCount);
        }
        //set new char properties
        pOLV->SetAttribs(rNewAttr);
        //restore old selection
        pOLV->SetSelection(aOrigSel);
        // tdf#91596 store updated formatting in SwField
        postItField->pPostIt->UpdateData();
    }

    mpWrtShell->EndUndo();
    PrepareView();
    mpWrtShell->EndAllAction();
    mbLayout = true;
    CalcRects();
    LayoutPostIts();
}

void SwPostItMgr::Hide( const OUString& rAuthor )
{
    for (auto const& postItField : mvPostItFields)
    {
        if ( postItField->pPostIt && (postItField->pPostIt->GetAuthor() == rAuthor) )
        {
            postItField->bShow  = false;
            postItField->pPostIt->HideNote();
        }
    }

    LayoutPostIts();
}

void SwPostItMgr::Hide()
{
    for (auto const& postItField : mvPostItFields)
    {
        postItField->bShow = false;
        postItField->pPostIt->HideNote();
    }
}

void SwPostItMgr::Show()
{
    for (auto const& postItField : mvPostItFields)
    {
        postItField->bShow = true;
    }
    LayoutPostIts();
}

SwAnnotationWin* SwPostItMgr::GetSidebarWin( const SfxBroadcaster* pBroadcaster) const
{
    for (auto const& postItField : mvPostItFields)
    {
        if ( postItField->GetBroadcaster() == pBroadcaster)
            return postItField->pPostIt;
    }
    return nullptr;
}

sw::annotation::SwAnnotationWin* SwPostItMgr::GetAnnotationWin(const SwPostItField* pField) const
{
    for (auto const& postItField : mvPostItFields)
    {
        if ( postItField->GetFormatField().GetField() == pField )
            return postItField->pPostIt.get();
    }
    return nullptr;
}

sw::annotation::SwAnnotationWin* SwPostItMgr::GetAnnotationWin(const sal_uInt32 nPostItId) const
{
    for (auto const& postItField : mvPostItFields)
    {
        if ( static_cast<const SwPostItField*>(postItField->GetFormatField().GetField())->GetPostItId() == nPostItId )
            return postItField->pPostIt.get();
    }
    return nullptr;
}

SwAnnotationWin* SwPostItMgr::GetNextPostIt( sal_uInt16 aDirection,
                                          SwAnnotationWin* aPostIt )
{
    if (mvPostItFields.size()>1)
    {
        auto i = std::find_if(mvPostItFields.begin(), mvPostItFields.end(),
            [&aPostIt](const std::unique_ptr<SwSidebarItem>& pField) { return pField->pPostIt == aPostIt; });
        if (i == mvPostItFields.end())
            return nullptr;

        auto iNextPostIt = i;
        if (aDirection == KEY_PAGEUP)
        {
            if ( iNextPostIt == mvPostItFields.begin() )
            {
                return nullptr;
            }
            --iNextPostIt;
        }
        else
        {
            ++iNextPostIt;
            if ( iNextPostIt == mvPostItFields.end() )
            {
                return nullptr;
            }
        }
        // lets quit, we are back at the beginning
        if ( (*iNextPostIt)->pPostIt == aPostIt)
            return nullptr;
        return (*iNextPostIt)->pPostIt;
    }
    else
        return nullptr;
}

long SwPostItMgr::GetNextBorder()
{
    for (auto const& pPage : mPages)
    {
        for(auto b = pPage->mvSidebarItems.begin(); b!= pPage->mvSidebarItems.end(); ++b)
        {
            if ((*b)->pPostIt == mpActivePostIt)
            {
                auto aNext = b;
                ++aNext;
                bool bFollow = (aNext != pPage->mvSidebarItems.end()) && (*aNext)->pPostIt->IsFollow();
                if ( pPage->bScrollbar || bFollow )
                {
                    return -1;
                }
                else
                {
                    //if this is the last item, return the bottom border otherwise the next item
                    if (aNext == pPage->mvSidebarItems.end())
                        return mpEditWin->LogicToPixel(Point(0,pPage->mPageRect.Bottom())).Y() - GetSpaceBetween();
                    else
                        return (*aNext)->pPostIt->GetPosPixel().Y() - GetSpaceBetween();
                }
            }
        }
    }

    OSL_FAIL("SwPostItMgr::GetNextBorder(): We have to find a next border here");
    return -1;
}

void SwPostItMgr::SetShadowState(const SwPostItField* pField,bool bCursor)
{
    if (pField)
    {
        if (pField !=mShadowState.mpShadowField)
        {
            if (mShadowState.mpShadowField)
            {
                // reset old one if still alive
                // TODO: does not work properly if mouse and cursor was set
                sw::annotation::SwAnnotationWin* pOldPostIt =
                                    GetAnnotationWin(mShadowState.mpShadowField);
                if (pOldPostIt && pOldPostIt->Shadow() && (pOldPostIt->Shadow()->GetShadowState() != SS_EDIT))
                    pOldPostIt->SetViewState(ViewState::NORMAL);
            }
            //set new one, if it is not currently edited
            sw::annotation::SwAnnotationWin* pNewPostIt = GetAnnotationWin(pField);
            if (pNewPostIt && pNewPostIt->Shadow() && (pNewPostIt->Shadow()->GetShadowState() != SS_EDIT))
            {
                pNewPostIt->SetViewState(ViewState::VIEW);
                //remember our new field
                mShadowState.mpShadowField = pField;
                mShadowState.bCursor = false;
                mShadowState.bMouse = false;
            }
        }
        if (bCursor)
            mShadowState.bCursor = true;
        else
            mShadowState.bMouse = true;
    }
    else
    {
        if (mShadowState.mpShadowField)
        {
            if (bCursor)
                mShadowState.bCursor = false;
            else
                mShadowState.bMouse = false;
            if (!mShadowState.bCursor && !mShadowState.bMouse)
            {
                // reset old one if still alive
                sw::annotation::SwAnnotationWin* pOldPostIt = GetAnnotationWin(mShadowState.mpShadowField);
                if (pOldPostIt && pOldPostIt->Shadow() && (pOldPostIt->Shadow()->GetShadowState() != SS_EDIT))
                {
                    pOldPostIt->SetViewState(ViewState::NORMAL);
                    mShadowState.mpShadowField = nullptr;
                }
            }
        }
    }
}

void SwPostItMgr::PrepareView(bool bIgnoreCount)
{
    if (!HasNotes() || bIgnoreCount)
    {
        mpWrtShell->StartAllAction();
        SwRootFrame* pLayout = mpWrtShell->GetLayout();
        if ( pLayout )
            SwPostItHelper::setSidebarChanged( pLayout,
                mpWrtShell->getIDocumentSettingAccess().get( DocumentSettingId::BROWSE_MODE ) );
        mpWrtShell->EndAllAction();
    }
}

bool SwPostItMgr::ShowScrollbar(const unsigned long aPage) const
{
    if (mPages.size() > aPage-1)
        return (mPages[aPage-1]->bScrollbar && !mbWaitingForCalcRects);
    else
        return false;
}

bool SwPostItMgr::IsHit(const Point &aPointPixel)
{
    if (HasNotes() && ShowNotes())
    {
        const Point aPoint = mpEditWin->PixelToLogic(aPointPixel);
        const SwRootFrame* pLayout = mpWrtShell->GetLayout();
        SwRect aPageFrame;
        const unsigned long nPageNum = SwPostItHelper::getPageInfo( aPageFrame, pLayout, aPoint );
        if( nPageNum )
        {
            tools::Rectangle aRect;
            OSL_ENSURE(mPages.size()>nPageNum-1,"SwPostitMgr:: page container size wrong");
            aRect = mPages[nPageNum-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                    ? tools::Rectangle(Point(aPageFrame.Left()-GetSidebarWidth()-GetSidebarBorderWidth(),aPageFrame.Top()),Size(GetSidebarWidth(),aPageFrame.Height()))
                    : tools::Rectangle( Point(aPageFrame.Right()+GetSidebarBorderWidth(),aPageFrame.Top()) , Size(GetSidebarWidth(),aPageFrame.Height()));
            if (aRect.IsInside(aPoint))
            {
                // we hit the note's sidebar
                // lets now test for the arrow area
                if (mPages[nPageNum-1]->bScrollbar)
                    return ScrollbarHit(nPageNum,aPoint);
                else
                    return false;
            }
        }
    }
    return false;
}

vcl::Window* SwPostItMgr::IsHitSidebarWindow(const Point& rPointLogic)
{
    vcl::Window* pRet = nullptr;

    if (HasNotes() && ShowNotes())
    {
        bool bEnableMapMode = !mpEditWin->IsMapModeEnabled();
        if (bEnableMapMode)
            mpEditWin->EnableMapMode();

        for (std::unique_ptr<SwSidebarItem>& pItem : mvPostItFields)
        {
            SwAnnotationWin* pPostIt = pItem->pPostIt;
            if (!pPostIt)
                continue;

            if (pPostIt->IsHitWindow(rPointLogic))
            {
                pRet = pPostIt;
                break;
            }
        }

        if (bEnableMapMode)
            mpEditWin->EnableMapMode(false);
    }

    return pRet;
}

tools::Rectangle SwPostItMgr::GetBottomScrollRect(const unsigned long aPage) const
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointBottom = mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                         ? Point(aPageRect.Left() - GetSidebarWidth() - GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height())
                         : Point(aPageRect.Right() + GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height());
    Size aSize(GetSidebarWidth() - mpEditWin->PixelToLogic(Size(4,0)).Width(), mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height()) ;
    return tools::Rectangle(aPointBottom,aSize);
}

tools::Rectangle SwPostItMgr::GetTopScrollRect(const unsigned long aPage) const
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointTop = mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                      ? Point(aPageRect.Left() - GetSidebarWidth() -GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height())
                      : Point(aPageRect.Right() + GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height());
    Size aSize(GetSidebarWidth() - mpEditWin->PixelToLogic(Size(4,0)).Width(), mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height()) ;
    return tools::Rectangle(aPointTop,aSize);
}

//IMPORTANT: if you change the rects here, also change SwPageFrame::PaintNotesSidebar()
bool SwPostItMgr::ScrollbarHit(const unsigned long aPage,const Point &aPoint)
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointBottom = mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                         ? Point(aPageRect.Left() - GetSidebarWidth()-GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height())
                         : Point(aPageRect.Right() + GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height());

    Point aPointTop = mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                      ? Point(aPageRect.Left() - GetSidebarWidth()-GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height())
                      : Point(aPageRect.Right()+GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height());

    tools::Rectangle aRectBottom(GetBottomScrollRect(aPage));
    tools::Rectangle aRectTop(GetTopScrollRect(aPage));

    if (aRectBottom.IsInside(aPoint))
    {
        if (aPoint.X() < long((aPointBottom.X() + GetSidebarWidth()/3)))
            Scroll( GetScrollSize(),aPage);
        else
            Scroll( -1*GetScrollSize(), aPage);
        return true;
    }
    else if (aRectTop.IsInside(aPoint))
    {
        if (aPoint.X() < long((aPointTop.X() + GetSidebarWidth()/3*2)))
            Scroll(GetScrollSize(), aPage);
        else
            Scroll(-1*GetScrollSize(), aPage);
        return true;
    }
    return false;
}

void SwPostItMgr::CorrectPositions()
{
    if ( mbWaitingForCalcRects || mbLayouting || mvPostItFields.empty() )
        return;

    // find first valid note
    SwAnnotationWin *pFirstPostIt = nullptr;
    for (auto const& postItField : mvPostItFields)
    {
        pFirstPostIt = postItField->pPostIt;
        if (pFirstPostIt)
            break;
    }

    //if we have not found a valid note, forget about it and leave
    if (!pFirstPostIt)
        return;

    // yeah, I know,    if this is a left page it could be wrong, but finding the page and the note is probably not even faster than just doing it
    // check, if anchor overlay object exists.
    const long aAnchorX = pFirstPostIt->Anchor()
                          ? mpEditWin->LogicToPixel( Point(static_cast<long>(pFirstPostIt->Anchor()->GetSixthPosition().getX()),0)).X()
                          : 0;
    const long aAnchorY = pFirstPostIt->Anchor()
                          ? mpEditWin->LogicToPixel( Point(0,static_cast<long>(pFirstPostIt->Anchor()->GetSixthPosition().getY()))).Y() + 1
                          : 0;
    if (Point(aAnchorX,aAnchorY) != pFirstPostIt->GetPosPixel())
    {
        long aAnchorPosX = 0;
        long aAnchorPosY = 0;
        for (std::unique_ptr<SwPostItPageItem>& pPage : mPages)
        {
            for (auto const& item : pPage->mvSidebarItems)
            {
                // check, if anchor overlay object exists.
                if ( item->bShow && item->pPostIt && item->pPostIt->Anchor() )
                {
                    aAnchorPosX = pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                        ? mpEditWin->LogicToPixel( Point(static_cast<long>(item->pPostIt->Anchor()->GetSeventhPosition().getX()),0)).X()
                        : mpEditWin->LogicToPixel( Point(static_cast<long>(item->pPostIt->Anchor()->GetSixthPosition().getX()),0)).X();
                    aAnchorPosY = mpEditWin->LogicToPixel( Point(0,static_cast<long>(item->pPostIt->Anchor()->GetSixthPosition().getY()))).Y() + 1;
                    item->pPostIt->SetPosPixel(Point(aAnchorPosX,aAnchorPosY));
                }
            }
        }
    }
}

bool SwPostItMgr::ShowNotes() const
{
    // we only want to see notes if Options - Writer - View - Notes is ticked
    return mpWrtShell->GetViewOptions()->IsPostIts();
}

bool SwPostItMgr::HasNotes() const
{
    return !mvPostItFields.empty();
}

unsigned long SwPostItMgr::GetSidebarWidth(bool bPx) const
{
    bool bEnableMapMode = !mpWrtShell->GetOut()->IsMapModeEnabled();
    sal_uInt16 nZoom = mpWrtShell->GetViewOptions()->GetZoom();
    if (comphelper::LibreOfficeKit::isActive() && !bEnableMapMode)
    {
        // The output device is the tile and contains the real wanted scale factor.
        double fScaleX = double(mpWrtShell->GetOut()->GetMapMode().GetScaleX());
        nZoom = fScaleX * 100;
    }
    unsigned long aWidth = static_cast<unsigned long>(nZoom * 1.8);

    if (bPx)
        return aWidth;
    else
    {
        if (bEnableMapMode)
            // The output device is the window.
            mpWrtShell->GetOut()->EnableMapMode();
        long nRet = mpWrtShell->GetOut()->PixelToLogic(Size(aWidth, 0)).Width();
        if (bEnableMapMode)
            mpWrtShell->GetOut()->EnableMapMode(false);
        return nRet;
    }
}

unsigned long SwPostItMgr::GetSidebarBorderWidth(bool bPx) const
{
    if (bPx)
        return 2;
    else
        return mpWrtShell->GetOut()->PixelToLogic(Size(2,0)).Width();
}

Color SwPostItMgr::GetColorDark(std::size_t aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayNormal[] = {
            COL_AUTHOR1_NORMAL,     COL_AUTHOR2_NORMAL,     COL_AUTHOR3_NORMAL,
            COL_AUTHOR4_NORMAL,     COL_AUTHOR5_NORMAL,     COL_AUTHOR6_NORMAL,
            COL_AUTHOR7_NORMAL,     COL_AUTHOR8_NORMAL,     COL_AUTHOR9_NORMAL };

        return aArrayNormal[ aAuthorIndex % SAL_N_ELEMENTS( aArrayNormal )];
    }
    else
        return COL_WHITE;
}

Color SwPostItMgr::GetColorLight(std::size_t aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayLight[] = {
            COL_AUTHOR1_LIGHT,      COL_AUTHOR2_LIGHT,      COL_AUTHOR3_LIGHT,
            COL_AUTHOR4_LIGHT,      COL_AUTHOR5_LIGHT,      COL_AUTHOR6_LIGHT,
            COL_AUTHOR7_LIGHT,      COL_AUTHOR8_LIGHT,      COL_AUTHOR9_LIGHT };

        return aArrayLight[ aAuthorIndex % SAL_N_ELEMENTS( aArrayLight )];
    }
    else
        return COL_WHITE;
}

Color SwPostItMgr::GetColorAnchor(std::size_t aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayAnchor[] = {
            COL_AUTHOR1_DARK,       COL_AUTHOR2_DARK,       COL_AUTHOR3_DARK,
            COL_AUTHOR4_DARK,       COL_AUTHOR5_DARK,       COL_AUTHOR6_DARK,
            COL_AUTHOR7_DARK,       COL_AUTHOR8_DARK,       COL_AUTHOR9_DARK };

        return aArrayAnchor[  aAuthorIndex % SAL_N_ELEMENTS( aArrayAnchor )];
    }
    else
        return COL_WHITE;
}

void SwPostItMgr::SetActiveSidebarWin( SwAnnotationWin* p)
{
    if ( p != mpActivePostIt )
    {
        // we need the temp variable so we can set mpActivePostIt before we call DeactivatePostIt
        // therefore we get a new layout in DOCCHANGED when switching from postit to document,
        // otherwise, GetActivePostIt() would still hold our old postit
        SwAnnotationWin* pActive = mpActivePostIt;
        mpActivePostIt = p;
        if (pActive)
        {
            pActive->DeactivatePostIt();
            mShadowState.mpShadowField = nullptr;
        }
        if (mpActivePostIt)
        {
            mpActivePostIt->GotoPos();
            mpView->AttrChangedNotify(nullptr);
            mpActivePostIt->ActivatePostIt();
        }
    }
}

IMPL_LINK_NOARG( SwPostItMgr, CalcHdl, void*, void )
{
    mnEventId = nullptr;
    if ( mbLayouting )
    {
        OSL_FAIL("Reentrance problem in Layout Manager!");
        mbWaitingForCalcRects = false;
        return;
    }

    // do not change order, even if it would seem so in the first place, we need the calcrects always
    if (CalcRects() || mbLayout)
    {
        mbLayout = false;
        LayoutPostIts();
    }
}

void SwPostItMgr::Rescale()
{
    for (auto const& postItField : mvPostItFields)
        if ( postItField->pPostIt )
            postItField->pPostIt->Rescale();
}

sal_Int32 SwPostItMgr::GetInitialAnchorDistance() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return sal_Int32(POSTIT_INITIAL_ANCHOR_DISTANCE * f);
}

sal_Int32 SwPostItMgr::GetSpaceBetween() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return sal_Int32(POSTIT_SPACE_BETWEEN * f);
}

sal_Int32 SwPostItMgr::GetScrollSize() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return sal_Int32((POSTIT_SPACE_BETWEEN + POSTIT_MINIMUMSIZE_WITH_META) * f);
}

sal_Int32 SwPostItMgr::GetMinimumSizeWithMeta() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return sal_Int32(POSTIT_MINIMUMSIZE_WITH_META * f);
}

sal_Int32 SwPostItMgr::GetSidebarScrollerHeight() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return sal_Int32(POSTIT_SCROLL_SIDEBAR_HEIGHT * f);
}

void SwPostItMgr::SetSpellChecking()
{
    for (auto const& postItField : mvPostItFields)
        if ( postItField->pPostIt )
            postItField->pPostIt->SetSpellChecking();
}

void SwPostItMgr::SetReadOnlyState()
{
    for (auto const& postItField : mvPostItFields)
        if ( postItField->pPostIt )
            postItField->pPostIt->SetReadonly( mbReadOnly );
}

void SwPostItMgr::CheckMetaText()
{
    for (auto const& postItField : mvPostItFields)
        if ( postItField->pPostIt )
            postItField->pPostIt->CheckMetaText();

}

sal_uInt16 SwPostItMgr::Replace(SvxSearchItem const * pItem)
{
    SwAnnotationWin* pWin = GetActiveSidebarWin();
    sal_uInt16 aResult = pWin->GetOutlinerView()->StartSearchAndReplace( *pItem );
    if (!aResult)
        SetActiveSidebarWin(nullptr);
    return aResult;
}

sal_uInt16 SwPostItMgr::FinishSearchReplace(const i18nutil::SearchOptions2& rSearchOptions, bool bSrchForward)
{
    SwAnnotationWin* pWin = GetActiveSidebarWin();
    SvxSearchItem aItem(SID_SEARCH_ITEM );
    aItem.SetSearchOptions(rSearchOptions);
    aItem.SetBackward(!bSrchForward);
    sal_uInt16 aResult = pWin->GetOutlinerView()->StartSearchAndReplace( aItem );
    if (!aResult)
        SetActiveSidebarWin(nullptr);
    return aResult;
}

sal_uInt16 SwPostItMgr::SearchReplace(const SwFormatField &pField, const i18nutil::SearchOptions2& rSearchOptions, bool bSrchForward)
{
    sal_uInt16 aResult = 0;
    SwAnnotationWin* pWin = GetSidebarWin(&pField);
    if (pWin)
    {
        ESelection aOldSelection = pWin->GetOutlinerView()->GetSelection();
        if (bSrchForward)
            pWin->GetOutlinerView()->SetSelection(ESelection(0,0,0,0));
        else
            pWin->GetOutlinerView()->SetSelection(
                    ESelection(EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT,EE_PARA_MAX_COUNT,EE_TEXTPOS_MAX_COUNT));
        SvxSearchItem aItem(SID_SEARCH_ITEM );
        aItem.SetSearchOptions(rSearchOptions);
        aItem.SetBackward(!bSrchForward);
        aResult = pWin->GetOutlinerView()->StartSearchAndReplace( aItem );
        if (!aResult)
            pWin->GetOutlinerView()->SetSelection(aOldSelection);
        else
        {
            SetActiveSidebarWin(pWin);
            MakeVisible(pWin);
        }
    }
    return aResult;
}

void SwPostItMgr::AssureStdModeAtShell()
{
        // deselect any drawing or frame and leave editing mode
        SdrView* pSdrView = mpWrtShell->GetDrawView();
        if ( pSdrView && pSdrView->IsTextEdit() )
        {
            bool bLockView = mpWrtShell->IsViewLocked();
            mpWrtShell->LockView( true );
            mpWrtShell->EndTextEdit();
            mpWrtShell->LockView( bLockView );
        }

        if( mpWrtShell->IsSelFrameMode() || mpWrtShell->IsObjSelected())
        {
                mpWrtShell->UnSelectFrame();
                mpWrtShell->LeaveSelFrameMode();
                mpWrtShell->GetView().LeaveDrawCreate();
                mpWrtShell->EnterStdMode();

                mpWrtShell->DrawSelChanged();
                mpView->StopShellTimer();
        }
}

bool SwPostItMgr::HasActiveSidebarWin() const
{
    return mpActivePostIt != nullptr;
}

bool SwPostItMgr::HasActiveAnnotationWin() const
{
    return HasActiveSidebarWin() &&
           mpActivePostIt != nullptr;
}

void SwPostItMgr::GrabFocusOnActiveSidebarWin()
{
    if ( HasActiveSidebarWin() )
    {
        mpActivePostIt->GrabFocus();
    }
}

void SwPostItMgr::UpdateDataOnActiveSidebarWin()
{
    if ( HasActiveSidebarWin() )
    {
        mpActivePostIt->UpdateData();
    }
}

void SwPostItMgr::DeleteActiveSidebarWin()
{
    if ( HasActiveSidebarWin() )
    {
        mpActivePostIt->Delete();
    }
}

void SwPostItMgr::HideActiveSidebarWin()
{
    if ( HasActiveSidebarWin() )
    {
        mpActivePostIt->Hide();
    }
}

void SwPostItMgr::ToggleInsModeOnActiveSidebarWin()
{
    if ( HasActiveSidebarWin() )
    {
        mpActivePostIt->ToggleInsMode();
    }
}

void SwPostItMgr::ConnectSidebarWinToFrame( const SwFrame& rFrame,
                                          const SwFormatField& rFormatField,
                                          SwAnnotationWin& rSidebarWin )
{
    if ( mpFrameSidebarWinContainer == nullptr )
    {
        mpFrameSidebarWinContainer.reset(new SwFrameSidebarWinContainer());
    }

    const bool bInserted = mpFrameSidebarWinContainer->insert( rFrame, rFormatField, rSidebarWin );
    if ( bInserted &&
         mpWrtShell->GetAccessibleMap() )
    {
        mpWrtShell->GetAccessibleMap()->InvalidatePosOrSize( nullptr, nullptr, &rSidebarWin, SwRect() );
    }
}

void SwPostItMgr::DisconnectSidebarWinFromFrame( const SwFrame& rFrame,
                                               SwAnnotationWin& rSidebarWin )
{
    if ( mpFrameSidebarWinContainer != nullptr )
    {
        const bool bRemoved = mpFrameSidebarWinContainer->remove( rFrame, rSidebarWin );
        if ( bRemoved &&
             mpWrtShell->GetAccessibleMap() )
        {
            mpWrtShell->GetAccessibleMap()->A11yDispose( nullptr, nullptr, &rSidebarWin );
        }
    }
}

bool SwPostItMgr::HasFrameConnectedSidebarWins( const SwFrame& rFrame )
{
    bool bRet( false );

    if ( mpFrameSidebarWinContainer != nullptr )
    {
        bRet = !mpFrameSidebarWinContainer->empty( rFrame );
    }

    return bRet;
}

vcl::Window* SwPostItMgr::GetSidebarWinForFrameByIndex( const SwFrame& rFrame,
                                                 const sal_Int32 nIndex )
{
    vcl::Window* pSidebarWin( nullptr );

    if ( mpFrameSidebarWinContainer != nullptr )
    {
        pSidebarWin = mpFrameSidebarWinContainer->get( rFrame, nIndex );
    }

    return pSidebarWin;
}

void SwPostItMgr::GetAllSidebarWinForFrame( const SwFrame& rFrame,
                                          std::vector< vcl::Window* >* pChildren )
{
    if ( mpFrameSidebarWinContainer != nullptr )
    {
        mpFrameSidebarWinContainer->getAll( rFrame, pChildren );
    }
}

void SwNoteProps::ImplCommit() {}
void SwNoteProps::Notify( const css::uno::Sequence< OUString >& ) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
