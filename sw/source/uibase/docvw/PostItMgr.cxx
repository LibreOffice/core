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

#include "PostItMgr.hxx"
#include <postithelper.hxx>

#include <AnnotationWin.hxx>
#include <SidebarWin.hxx>
#include <frmsidebarwincontainer.hxx>
#include <accmap.hxx>

#include <SidebarWindowsConsts.hxx>
#include <AnchorOverlayObject.hxx>
#include <ShadowOverlayObject.hxx>

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
#include <SwRewriter.hxx>
#include <tools/color.hxx>
#include <unotools/datetime.hxx>

#include <swmodule.hxx>
#include <annotation.hrc>
#include <utlui.hrc>
#include "cmdid.h"

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

#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include "annotsh.hxx"
#include "swabstdlg.hxx"
#include "swevent.hxx"
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

namespace {

    enum class CommentNotificationType { Add, Remove, Modify };

    bool comp_pos(const SwSidebarItem* a, const SwSidebarItem* b)
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
            sw::annotation::SwAnnotationWin* pWin = static_cast<sw::annotation::SwAnnotationWin*>((pItem)->pPostIt.get());

            const SwPostItField* pField = pWin->GetPostItField();
            const SwRect& aRect = pWin->GetAnchorRect();
            const Rectangle aSVRect(aRect.Pos().getX(),
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
    , mpFrameSidebarWinContainer( nullptr )
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

    for(std::vector<SwPostItPageItem*>::iterator i = mPages.begin(); i != mPages.end() ; ++i)
        delete (*i);
    mPages.clear();

    delete mpFrameSidebarWinContainer;
    mpFrameSidebarWinContainer = nullptr;
}

void SwPostItMgr::CheckForRemovedPostIts()
{
    bool bRemoved = false;
    for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end(); )
    {
        std::list<SwSidebarItem*>::iterator it = i++;
        if ( !(*it)->UseElement() )
        {
            SwSidebarItem* p = (*it);
            mvPostItFields.remove(*it);
            if (GetActiveSidebarWin() == p->pPostIt)
                SetActiveSidebarWin(nullptr);
            p->pPostIt.disposeAndClear();
            delete p;
            bRemoved = true;
        }
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

SwSidebarItem* SwPostItMgr::InsertItem(SfxBroadcaster* pItem, bool bCheckExistance, bool bFocus)
{
    SwSidebarItem* pAnnotationItem = nullptr;
    if (bCheckExistance)
    {
        for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        {
            if ( (*i)->GetBroadCaster() == pItem )
                return pAnnotationItem;
        }
    }
    mbLayout = bFocus;

    if (dynamic_cast< const SwFormatField *>( pItem ) !=  nullptr)
    {
        pAnnotationItem = new SwAnnotationItem(static_cast<SwFormatField&>(*pItem), bFocus);
        mvPostItFields.push_back(pAnnotationItem);
    }
    OSL_ENSURE(dynamic_cast< const SwFormatField *>( pItem ) !=  nullptr,"Mgr::InsertItem: seems like new stuff was added");
    StartListening(*pItem);
    return pAnnotationItem;
}

void SwPostItMgr::RemoveItem( SfxBroadcaster* pBroadcast )
{
    EndListening(*pBroadcast);
    for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if ( (*i)->GetBroadCaster() == pBroadcast )
        {
            SwSidebarItem* p = (*i);
            if (GetActiveSidebarWin() == p->pPostIt)
                SetActiveSidebarWin(nullptr);
            p->pPostIt.disposeAndClear();
            mvPostItFields.erase(i);
            delete p;
            break;
        }
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
                for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
                {
                    if ( pFormatField == (*i)->GetBroadCaster() )
                    {
                        if ((*i)->pPostIt)
                        {
                            (*i)->pPostIt->SetPostItText();
                            mbLayout = true;
                        }

                        // If LOK has disabled tiled annotations, emit annotation callbacks
                        if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
                        {
                            lcl_CommentNotification(mpView, CommentNotificationType::Modify, *i, 0);
                        }
                        break;
                    }
                }
                break;
            }

            case SwFormatFieldHintWhich::LANGUAGE:
            {
                SwFormatField* pFormatField = dynamic_cast<SwFormatField*>(&rBC);
                for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
                {
                    if ( pFormatField == (*i)->GetBroadCaster() )
                    {
                        if ((*i)->pPostIt)
                        {
                            const SvtScriptType nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( (*i)->GetFormatField().GetField()->GetLanguage() );
                            sal_uInt16 nLangWhichId = 0;
                            switch (nScriptType)
                            {
                            case SvtScriptType::LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
                            case SvtScriptType::ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                            case SvtScriptType::COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                            default: break;
                            }
                            (*i)->pPostIt->SetLanguage(
                                SvxLanguageItem(
                                (*i)->GetFormatField().GetField()->GetLanguage(),
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
                if ( mbReadOnly != !!(mpView->GetDocShell()->IsReadOnly()) )
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
        SfxRequest aRequest(mpView->GetViewFrame(),FN_VIEW_NOTES);
        mpView->ExecViewOptions(aRequest);
    }

    for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        // field to get the focus is the broadcaster
        if ( &rBC == (*i)->GetBroadCaster() )
        {
            if ((*i)->pPostIt)
            {
                (*i)->pPostIt->GrabFocus();
                MakeVisible((*i)->pPostIt);
            }
            else
            {
                // when the layout algorithm starts, this postit is created and receives focus
                (*i)->bFocus = true;
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
        for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        {
            SwSidebarItem* pItem = (*i);
            if ( !pItem->UseElement() )
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
            mvPostItFields.sort(comp_pos);

        // sort the items into the right page vector, so layout can be done by page
        for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        {
            SwSidebarItem* pItem = (*i);
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
                for (unsigned int j=0; j<aPageNum - nNumberOfPages; ++j)
                    mPages.push_back( new SwPostItPageItem());
            }
            mPages[aPageNum-1]->mList->push_back(pItem);
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
    for(std::list<SwSidebarItem*>::const_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if ((*i)->bShow && (*i)->pPostIt && (*i)->pPostIt->HasScrollbar())
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
        for (int i=0; i<lPageSize - lContainerSize;i++)
            mPages.push_back( new SwPostItPageItem());
    }
    else if (lContainerSize > lPageSize)
    {
        for (int i=mPages.size()-1; i >= lPageSize;--i)
        {
            delete mPages[i];
            mPages.pop_back();
        }
    }
    // only clear the list, DO NOT delete the objects itself
    for(std::vector<SwPostItPageItem*>::iterator i = mPages.begin(); i != mPages.end() ; ++i)
    {
        (*i)->mList->clear();
        if (mvPostItFields.empty())
            (*i)->bScrollbar = false;

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
        for (SwPostItPageItem* pPage : mPages)
        {
            // only layout if there are notes on this page
            if (pPage->mList->size()>0)
            {
                std::list<SwSidebarWin*>    aVisiblePostItList;
                unsigned long           lNeededHeight = 0;
                long                    mlPageBorder = 0;
                long                    mlPageEnd = 0;

                for(SwSidebarItem_iterator i = pPage->mList->begin(); i != pPage->mList->end(); ++i)
                {
                    SwSidebarItem* pItem = (*i);
                    VclPtr<SwSidebarWin> pPostIt = pItem->pPostIt;

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
                            pPostIt = (*i)->GetSidebarWindow( mpView->GetEditWin(),
                                                              *this );
                            pPostIt->InitControls();
                            pPostIt->SetReadonly(mbReadOnly);
                            pItem->pPostIt = pPostIt;
                            if (mpAnswer)
                            {
                                if (static_cast<bool>(pPostIt->CalcParent())) //do we really have another note in front of this one
                                    static_cast<sw::annotation::SwAnnotationWin*>(pPostIt.get())->InitAnswer(mpAnswer);
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
                    for(SwSidebarWin_iterator i = aVisiblePostItList.begin(); i != aVisiblePostItList.end() ; ++i)
                    {
                        if (pPage->lOffset != 0)
                            (*i)->TranslateTopPosition(pPage->lOffset);

                        bool bBottom  = mpEditWin->PixelToLogic(Point(0,(*i)->VirtualPos().Y()+(*i)->VirtualSize().Height())).Y() <= (pPage->mPageRect.Bottom()-aSidebarheight);
                        bool bTop = mpEditWin->PixelToLogic(Point(0,(*i)->VirtualPos().Y())).Y() >= (pPage->mPageRect.Top()+aSidebarheight);
                        if ( bBottom && bTop )
                        {
                            // When tiled rendering, make sure that only the
                            // view that has the comment focus emits callbacks,
                            // so the editing view jumps to the comment, but
                            // not the others.
                            bool bTiledPainting = comphelper::LibreOfficeKit::isTiledPainting();
                            if (!bTiledPainting)
                                // No focus -> disable callbacks.
                                comphelper::LibreOfficeKit::setTiledPainting(!(*i)->HasChildPathFocus());
                            (*i)->ShowNote();
                            if (!bTiledPainting)
                            {
                                comphelper::LibreOfficeKit::setTiledPainting(bTiledPainting);
                                (*i)->InvalidateControl();
                            }
                        }
                        else
                        {
                            if (mpEditWin->PixelToLogic(Point(0,(*i)->VirtualPos().Y())).Y() < (pPage->mPageRect.Top()+aSidebarheight))
                            {
                                if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT )
                                    (*i)->ShowAnchorOnly(Point( pPage->mPageRect.Left(),
                                                                pPage->mPageRect.Top()));
                                else if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT )
                                    (*i)->ShowAnchorOnly(Point( pPage->mPageRect.Right(),
                                                                pPage->mPageRect.Top()));
                            }
                            else
                            {
                                if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT )
                                    (*i)->ShowAnchorOnly(Point(pPage->mPageRect.Left(),
                                                               pPage->mPageRect.Bottom()));
                                else if ( pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::RIGHT )
                                    (*i)->ShowAnchorOnly(Point(pPage->mPageRect.Right(),
                                                               pPage->mPageRect.Bottom()));
                            }
                            OSL_ENSURE(pPage->bScrollbar,"SwPostItMgr::LayoutByPage(): note overlaps, but bScrollbar is not true");
                        }
                    }
                }
                else
                {
                    for(SwSidebarWin_iterator i = aVisiblePostItList.begin(); i != aVisiblePostItList.end() ; ++i)
                    {
                        (*i)->SetPosAndSize();
                    }

                    bool bOldScrollbar = pPage->bScrollbar;
                    pPage->bScrollbar = false;
                    bUpdate = (bOldScrollbar != pPage->bScrollbar) || bUpdate;
                }
                aVisiblePostItList.clear();
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
            bool bRepair = false;
            for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
            {
                SwSidebarItem* pItem = (*i);
                if ( !pItem->UseElement() )
                {
                    OSL_FAIL("PostIt is not in doc!");
                    bRepair = true;
                    continue;
                }

                if ((*i)->pPostIt)
                {
                    (*i)->pPostIt->HideNote();
                    if ((*i)->pPostIt->HasChildPathFocus())
                    {
                        SetActiveSidebarWin(nullptr);
                        (*i)->pPostIt->GrabFocusToDocument();
                    }

                    // Emit LOK callbacks if tiled annotation is turned off
                    if (comphelper::LibreOfficeKit::isActive() && !comphelper::LibreOfficeKit::isTiledAnnotations())
                    {
                        if ((*i)->bPendingLayout)
                            lcl_CommentNotification(mpView, CommentNotificationType::Add, *i, 0);
                    }

                    (*i)->bPendingLayout = false;
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
    if ( mPages[aPage-1]->mList->empty() )
    {
        OSL_FAIL("Notes SidePane painted but no rects and page lists calculated!");
        return false;
    }

    SwSidebarItem_iterator aItem = mPages[aPage-1]->mList->end();
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
    for(SwSidebarItem_iterator i = mPages[nPage]->mList->begin(); i != mPages[nPage]->mList->end(); ++i)
    {
        SwSidebarWin* pPostIt = (*i)->pPostIt;
        if (!pPostIt)
            continue;
        Point aPoint(mpEditWin->PixelToLogic(pPostIt->GetPosPixel()));
        Size aSize(pPostIt->PixelToLogic(pPostIt->GetSizePixel()));
        pPostIt->Draw(pOutDev, aPoint, aSize, DrawFlags::NONE);
    }
}

void SwPostItMgr::PaintTile(OutputDevice& rRenderContext, const Rectangle& /*rRect*/)
{
    for (SwSidebarItem* pItem : mvPostItFields)
    {
        SwSidebarWin* pPostIt = pItem->pPostIt;
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
        Rectangle aRectangle(Point(0, 0), aSize);

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
    for(SwSidebarItem_iterator i = mPages[aPage-1]->mList->begin(); i != mPages[aPage-1]->mList->end(); ++i)
    {
        SwSidebarWin* pPostIt = (*i)->pPostIt;
        // if this is an answer, we should take the normal position and not the real, slightly moved position
        pPostIt->SetVirtualPosSize(pPostIt->GetPosPixel(),pPostIt->GetSizePixel());
        pPostIt->TranslateTopPosition(lScroll);

        if ((*i)->bShow)
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

void SwPostItMgr::AutoScroll(const SwSidebarWin* pPostIt,const unsigned long aPage )
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
            // e.g aDiff= 61 POSTIT_SCOLL=50 --> lScroll = 100
            const long lScroll = bBottom ? (aDiff + ( GetScrollSize() - (aDiff % GetScrollSize()))) : (aDiff - (GetScrollSize() + (aDiff % GetScrollSize())));
            Scroll(lScroll, aPage);
        }
    }
}

void SwPostItMgr::MakeVisible(const SwSidebarWin* pPostIt )
{
    long aPage = -1;
    // we don't know the page yet, lets find it ourselves
    for (std::vector<SwPostItPageItem*>::size_type n=0;n<mPages.size();n++)
    {
        if (mPages[n]->mList->size()>0)
        {
            for(SwSidebarItem_iterator i = mPages[n]->mList->begin(); i != mPages[n]->mList->end(); ++i)
            {
                if ((*i)->pPostIt==pPostIt)
                {
                    aPage = n+1;
                    break;
                }
            }
        }
    }
    if (aPage!=-1)
        AutoScroll(pPostIt,aPage);
    Rectangle aNoteRect (Point(pPostIt->GetPosPixel().X(),pPostIt->GetPosPixel().Y()-5),pPostIt->GetSizePixel());
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
            return Color(COL_WHITE);
        else
            return COL_NOTES_SIDEPANE_ARROW_ENABLED;
    }
    else
    {
        return COL_NOTES_SIDEPANE_ARROW_DISABLED;
    }
}

bool SwPostItMgr::LayoutByPage(std::list<SwSidebarWin*> &aVisiblePostItList, const Rectangle& rBorder, long lNeededHeight)
{
    /*** General layout idea:***/
    //  - if we have space left, we always move the current one up,
    //    otherwise the next one down
    //  - first all notes are resized
    //  - then the real layout starts

    //rBorder is the page rect
    const Rectangle aBorder         = mpEditWin->LogicToPixel(rBorder);
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
                for(SwSidebarWin_iterator i = aVisiblePostItList.begin(); i != aVisiblePostItList.end() ; ++i)
                    (*i)->SetSize(Size((*i)->VirtualSize().getWidth(),(*i)->GetMinimumSizeWithMeta()));
        }
        else
        {
            for(SwSidebarWin_iterator i = aVisiblePostItList.begin(); i != aVisiblePostItList.end() ; ++i)
            {
                if ( (*i)->VirtualSize().getHeight() > lAverageHeight)
                    (*i)->SetSize(Size((*i)->VirtualSize().getWidth(),lAverageHeight));
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
            for(SwSidebarWin_iterator i = aVisiblePostItList.begin(); i != aVisiblePostItList.end() ; ++i)
            {
                SwSidebarWin_iterator aNextPostIt = i;
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
                    SwSidebarWin_iterator aPrevPostIt = i;
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
        SwSidebarWin_iterator i = aVisiblePostItList.begin();
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

void SwPostItMgr::AddPostIts(bool bCheckExistance, bool bFocus)
{
    bool bEmpty = mvPostItFields.empty();
    SwFieldType* pType = mpView->GetDocShell()->GetDoc()->getIDocumentFieldsAccess().GetFieldType(RES_POSTITFLD, OUString(),false);
    SwIterator<SwFormatField,SwFieldType> aIter( *pType );
    SwFormatField* pSwFormatField = aIter.First();
    while(pSwFormatField)
    {
        if ( pSwFormatField->GetTextField())
        {
            if ( pSwFormatField->IsFieldInDoc() )
                InsertItem(pSwFormatField,bCheckExistance,bFocus);
        }
        pSwFormatField = aIter.Next();
    }

    // if we just added the first one we have to update the view for centering
    if (bEmpty && !mvPostItFields.empty())
        PrepareView(true);
}

void SwPostItMgr::RemoveSidebarWin()
{
    if (!mvPostItFields.empty())
    {
        for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        {
            EndListening( *(const_cast<SfxBroadcaster*>((*i)->GetBroadCaster())) );
            (*i)->pPostIt.disposeAndClear();
            delete (*i);
        }
        mvPostItFields.clear();
    }

    // all postits removed, no items should be left in pages
    PreparePageContainer();
}

class FilterFunctor : public std::unary_function<const SwFormatField*, bool>
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
        return pField->GetField()->GetTyp()->Which() == RES_POSTITFLD;
    }
};

class IsPostitFieldWithAuthorOf : public FilterFunctor
{
    OUString m_sAuthor;
public:
    explicit IsPostitFieldWithAuthorOf(const OUString &rAuthor)
        : m_sAuthor(rAuthor)
    {
    }
    bool operator()(const SwFormatField* pField) const override
    {
        if (pField->GetField()->GetTyp()->Which() != RES_POSTITFLD)
            return false;
        return static_cast<const SwPostItField*>(pField->GetField())->GetPar1() == m_sAuthor;
    }
};

class IsPostitFieldWithPostitId : public FilterFunctor
{
    sal_uInt32 m_nPostItId;
public:
    explicit IsPostitFieldWithPostitId(sal_uInt32 nPostItId)
        : m_nPostItId(nPostItId)
        {}

    bool operator()(const SwFormatField* pField) const override
    {
        if (pField->GetField()->GetTyp()->Which() != RES_POSTITFLD)
            return false;
        return static_cast<const SwPostItField*>(pField->GetField())->GetPostItId() == m_nPostItId;
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
    std::list<SwSidebarItem*>& l;
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
    FieldDocWatchingStack(std::list<SwSidebarItem*>& in, SwDocShell &rDocShell, FilterFunctor& rFilter)
        : l(in)
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
        v.reserve(l.size());
        for(std::list<SwSidebarItem*>::iterator aI = l.begin(); aI != l.end(); ++aI)
        {
            SwSidebarItem* p = *aI;
            const SwFormatField& rField = p->GetFormatField();
            if (!m_rFilter(&rField))
                continue;
            StartListening(const_cast<SwFormatField&>(rField));
            v.push_back(&rField);
        }
    }
    void EndListeningToAllFields()
    {
        for(std::vector<const SwFormatField*>::iterator aI = v.begin(); aI != v.end(); ++aI)
        {
            const SwFormatField* pField = *aI;
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
    aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_DELETE_AUTHOR_NOTES) + rAuthor);
    mpWrtShell->StartUndo( UNDO_DELETE, &aRewriter );

    IsPostitFieldWithAuthorOf aFilter(rAuthor);
    FieldDocWatchingStack aStack(mvPostItFields, *mpView->GetDocShell(), aFilter);
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
        static_cast<sw::annotation::SwAnnotationWin*>(mpActivePostIt.get())->GetPostItField()->GetPostItId() == nPostItId)
    {
        SetActiveSidebarWin(nullptr);
    }
    SwRewriter aRewriter;
    aRewriter.AddRule(UndoArg1, SW_RESSTR(STR_CONTENT_TYPE_SINGLE_POSTIT));
    mpWrtShell->StartUndo( UNDO_DELETE, &aRewriter );

    IsPostitFieldWithPostitId aFilter(nPostItId);
    FieldDocWatchingStack aStack(mvPostItFields, *mpView->GetDocShell(), aFilter);
    const SwFormatField* pField = aStack.pop();
    if (mpWrtShell->GotoField(*pField))
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
    aRewriter.AddRule(UndoArg1, SW_RES(STR_DELETE_ALL_NOTES) );
    mpWrtShell->StartUndo( UNDO_DELETE, &aRewriter );

    IsPostitField aFilter;
    FieldDocWatchingStack aStack(mvPostItFields, *mpView->GetDocShell(),
        aFilter);
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
    sw::sidebarwindows::SwSidebarWin *pOrigActiveWin = GetActiveSidebarWin();
    sw::sidebarwindows::SwSidebarWin *pWin = pOrigActiveWin;
    if (!pWin)
    {
        for (SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end(); ++i)
        {
            pWin = (*i)->pPostIt;
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
    SfxItemSet aDlgAttr(*pPool, EE_ITEMS_START, EE_ITEMS_END);
    aDlgAttr.Put(aEditAttr);
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    ScopedVclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateSwCharDlg(rView.GetWindow(), rView, aDlgAttr, SwCharDlgMode::Ann));
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
    aRewriter.AddRule(UndoArg1, SW_RES(STR_FORMAT_ALL_NOTES) );
    mpWrtShell->StartUndo( UNDO_INSATTR, &aRewriter );

    for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if (!(*i)->pPostIt)
            continue;
        OutlinerView* pOLV = (*i)->pPostIt->GetOutlinerView();
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
    for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if ( (*i)->pPostIt && ((*i)->pPostIt->GetAuthor() == rAuthor) )
        {
            (*i)->bShow  = false;
            (*i)->pPostIt->HideNote();
        }
    }

    LayoutPostIts();
}

void SwPostItMgr::Hide()
{
    for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        (*i)->bShow = false;
        (*i)->pPostIt->HideNote();
    }
}

void SwPostItMgr::Show()
{
    for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        (*i)->bShow = true;
    }
    LayoutPostIts();
}

SwSidebarWin* SwPostItMgr::GetSidebarWin( const SfxBroadcaster* pBroadcaster) const
{
    for(const_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if ( (*i)->GetBroadCaster() == pBroadcaster)
            return (*i)->pPostIt;
    }
    return nullptr;
}

sw::annotation::SwAnnotationWin* SwPostItMgr::GetAnnotationWin(const SwPostItField* pField) const
{
    for(const_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if ( (*i)->GetFormatField().GetField() == pField )
            return dynamic_cast<sw::annotation::SwAnnotationWin*>((*i)->pPostIt.get());
    }
    return nullptr;
}

sw::annotation::SwAnnotationWin* SwPostItMgr::GetAnnotationWin(const sal_uInt32 nPostItId) const
{
    for(const_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        if ( static_cast<const SwPostItField*>((*i)->GetFormatField().GetField())->GetPostItId() == nPostItId )
            return dynamic_cast<sw::annotation::SwAnnotationWin*>((*i)->pPostIt.get());
    }
    return nullptr;
}

SwSidebarWin* SwPostItMgr::GetNextPostIt( sal_uInt16 aDirection,
                                          SwSidebarWin* aPostIt )
{
    if (mvPostItFields.size()>1)
    {
        for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        {
            if ( (*i)->pPostIt == aPostIt)
            {
                SwSidebarItem_iterator iNextPostIt  = i;
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
        }
        return nullptr;
    }
    else
        return nullptr;
}

long SwPostItMgr::GetNextBorder()
{
    for (SwPostItPageItem* pPage : mPages)
    {
        for(SwSidebarItem_iterator b = pPage->mList->begin(); b!= pPage->mList->end(); ++b)
        {
            if ((*b)->pPostIt == mpActivePostIt)
            {
                SwSidebarItem_iterator aNext = b;
                ++aNext;
                bool bFollow = (aNext != pPage->mList->end()) && (*aNext)->pPostIt->IsFollow();
                if ( pPage->bScrollbar || bFollow )
                {
                    return -1;
                }
                else
                {
                    //if this is the last item, return the bottom border otherwise the next item
                    if (aNext == pPage->mList->end())
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
            Rectangle aRect;
            OSL_ENSURE(mPages.size()>nPageNum-1,"SwPostitMgr:: page container size wrong");
            aRect = mPages[nPageNum-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                    ? Rectangle(Point(aPageFrame.Left()-GetSidebarWidth()-GetSidebarBorderWidth(),aPageFrame.Top()),Size(GetSidebarWidth(),aPageFrame.Height()))
                    : Rectangle( Point(aPageFrame.Right()+GetSidebarBorderWidth(),aPageFrame.Top()) , Size(GetSidebarWidth(),aPageFrame.Height()));
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

        for (SwSidebarItem* pItem : mvPostItFields)
        {
            SwSidebarWin* pPostIt = pItem->pPostIt;
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

Rectangle SwPostItMgr::GetBottomScrollRect(const unsigned long aPage) const
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointBottom = mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                         ? Point(aPageRect.Left() - GetSidebarWidth() - GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height())
                         : Point(aPageRect.Right() + GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height());
    Size aSize(GetSidebarWidth() - mpEditWin->PixelToLogic(Size(4,0)).Width(), mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height()) ;
    return Rectangle(aPointBottom,aSize);
}

Rectangle SwPostItMgr::GetTopScrollRect(const unsigned long aPage) const
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointTop = mPages[aPage-1]->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                      ? Point(aPageRect.Left() - GetSidebarWidth() -GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height())
                      : Point(aPageRect.Right() + GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height());
    Size aSize(GetSidebarWidth() - mpEditWin->PixelToLogic(Size(4,0)).Width(), mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height()) ;
    return Rectangle(aPointTop,aSize);
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

    Rectangle aRectBottom(GetBottomScrollRect(aPage));
    Rectangle aRectTop(GetTopScrollRect(aPage));

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
    SwSidebarWin *pFirstPostIt = nullptr;
    for(SwSidebarItem_iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
    {
        pFirstPostIt = (*i)->pPostIt;
        if (pFirstPostIt)
            break;
    }

    //if we have not found a valid note, forget about it and leave
    if (!pFirstPostIt)
        return;

    // yeah, I know,    if this is a left page it could be wrong, but finding the page and the note is probably not even faster than just doing it
    // check, if anchor overlay object exists.
    const long aAnchorX = pFirstPostIt->Anchor()
                          ? mpEditWin->LogicToPixel( Point((long)(pFirstPostIt->Anchor()->GetSixthPosition().getX()),0)).X()
                          : 0;
    const long aAnchorY = pFirstPostIt->Anchor()
                          ? mpEditWin->LogicToPixel( Point(0,(long)(pFirstPostIt->Anchor()->GetSixthPosition().getY()))).Y() + 1
                          : 0;
    if (Point(aAnchorX,aAnchorY) != pFirstPostIt->GetPosPixel())
    {
        long aAnchorPosX = 0;
        long aAnchorPosY = 0;
        for (SwPostItPageItem* pPage : mPages)
        {
            for(SwSidebarItem_iterator i = pPage->mList->begin(); i != pPage->mList->end(); ++i)
            {
                // check, if anchor overlay object exists.
                if ( (*i)->bShow && (*i)->pPostIt && (*i)->pPostIt->Anchor() )
                {
                    aAnchorPosX = pPage->eSidebarPosition == sw::sidebarwindows::SidebarPosition::LEFT
                        ? mpEditWin->LogicToPixel( Point((long)((*i)->pPostIt->Anchor()->GetSeventhPosition().getX()),0)).X()
                        : mpEditWin->LogicToPixel( Point((long)((*i)->pPostIt->Anchor()->GetSixthPosition().getX()),0)).X();
                    aAnchorPosY = mpEditWin->LogicToPixel( Point(0,(long)((*i)->pPostIt->Anchor()->GetSixthPosition().getY()))).Y() + 1;
                    (*i)->pPostIt->SetPosPixel(Point(aAnchorPosX,aAnchorPosY));
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
        double fScaleX = mpWrtShell->GetOut()->GetMapMode().GetScaleX();
        nZoom = fScaleX * 100;
    }
    unsigned long aWidth = (unsigned long)(nZoom * 1.8);

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

Color SwPostItMgr::GetColorDark(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayNormal[] = {
            COL_AUTHOR1_NORMAL,     COL_AUTHOR2_NORMAL,     COL_AUTHOR3_NORMAL,
            COL_AUTHOR4_NORMAL,     COL_AUTHOR5_NORMAL,     COL_AUTHOR6_NORMAL,
            COL_AUTHOR7_NORMAL,     COL_AUTHOR8_NORMAL,     COL_AUTHOR9_NORMAL };

        return Color( aArrayNormal[ aAuthorIndex % SAL_N_ELEMENTS( aArrayNormal )]);
    }
    else
        return Color(COL_WHITE);
}

Color SwPostItMgr::GetColorLight(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayLight[] = {
            COL_AUTHOR1_LIGHT,      COL_AUTHOR2_LIGHT,      COL_AUTHOR3_LIGHT,
            COL_AUTHOR4_LIGHT,      COL_AUTHOR5_LIGHT,      COL_AUTHOR6_LIGHT,
            COL_AUTHOR7_LIGHT,      COL_AUTHOR8_LIGHT,      COL_AUTHOR9_LIGHT };

        return Color( aArrayLight[ aAuthorIndex % SAL_N_ELEMENTS( aArrayLight )]);
    }
    else
        return Color(COL_WHITE);
}

Color SwPostItMgr::GetColorAnchor(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayAnchor[] = {
            COL_AUTHOR1_DARK,       COL_AUTHOR2_DARK,       COL_AUTHOR3_DARK,
            COL_AUTHOR4_DARK,       COL_AUTHOR5_DARK,       COL_AUTHOR6_DARK,
            COL_AUTHOR7_DARK,       COL_AUTHOR8_DARK,       COL_AUTHOR9_DARK };

        return Color( aArrayAnchor[  aAuthorIndex % SAL_N_ELEMENTS( aArrayAnchor )]);
    }
    else
        return Color(COL_WHITE);
}

void SwPostItMgr::SetActiveSidebarWin( SwSidebarWin* p)
{
    if ( p != mpActivePostIt )
    {
        // we need the temp variable so we can set mpActivePostIt before we call DeactivatePostIt
        // therefore we get a new layout in DOCCHANGED when switching from postit to document,
        // otherwise, GetActivePostIt() would still hold our old postit
        SwSidebarWin* pActive = mpActivePostIt;
        mpActivePostIt = p;
        if (pActive)
        {
            pActive->DeactivatePostIt();
            mShadowState.mpShadowField = nullptr;
        }
        if (mpActivePostIt)
        {
            mpActivePostIt->GotoPos();
            mpView->SetAnnotationMode(true);
            mpView->AttrChangedNotify(nullptr);
            mpView->SetAnnotationMode(false);
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
    for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        if ( (*i)->pPostIt )
            (*i)->pPostIt->Rescale();
}

sal_Int32 SwPostItMgr::GetInitialAnchorDistance() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return POSTIT_INITIAL_ANCHOR_DISTANCE * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwPostItMgr::GetSpaceBetween() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return ( POSTIT_SPACE_BETWEEN ) * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwPostItMgr::GetScrollSize() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return ( POSTIT_SPACE_BETWEEN + POSTIT_MINIMUMSIZE_WITH_META ) * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwPostItMgr::GetMinimumSizeWithMeta() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return POSTIT_MINIMUMSIZE_WITH_META * f.GetNumerator() / f.GetDenominator();
}

sal_Int32 SwPostItMgr::GetSidebarScrollerHeight() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return POSTIT_SCROLL_SIDEBAR_HEIGHT * f.GetNumerator() / f.GetDenominator();
}

void SwPostItMgr::SetSpellChecking()
{
    for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        if ( (*i)->pPostIt )
            (*i)->pPostIt->SetSpellChecking();
}

void SwPostItMgr::SetReadOnlyState()
{
    for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
        if ( (*i)->pPostIt )
            (*i)->pPostIt->SetReadonly( mbReadOnly );
}

void SwPostItMgr::CheckMetaText()
{
        for(std::list<SwSidebarItem*>::iterator i = mvPostItFields.begin(); i != mvPostItFields.end() ; ++i)
                if ( (*i)->pPostIt )
                       (*i)->pPostIt->CheckMetaText();

}

sal_uInt16 SwPostItMgr::Replace(SvxSearchItem* pItem)
{
    SwSidebarWin* pWin = GetActiveSidebarWin();
    sal_uInt16 aResult = pWin->GetOutlinerView()->StartSearchAndReplace( *pItem );
    if (!aResult)
        SetActiveSidebarWin(nullptr);
    return aResult;
}

sal_uInt16 SwPostItMgr::FinishSearchReplace(const css::util::SearchOptions2& rSearchOptions, bool bSrchForward)
{
    SwSidebarWin* pWin = GetActiveSidebarWin();
    SvxSearchItem aItem(SID_SEARCH_ITEM );
    aItem.SetSearchOptions(rSearchOptions);
    aItem.SetBackward(!bSrchForward);
    sal_uInt16 aResult = pWin->GetOutlinerView()->StartSearchAndReplace( aItem );
    if (!aResult)
        SetActiveSidebarWin(nullptr);
    return aResult;
}

sal_uInt16 SwPostItMgr::SearchReplace(const SwFormatField &pField, const css::util::SearchOptions2& rSearchOptions, bool bSrchForward)
{
    sal_uInt16 aResult = 0;
    SwSidebarWin* pWin = GetSidebarWin(&pField);
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
           dynamic_cast<sw::annotation::SwAnnotationWin*>(mpActivePostIt.get()) != nullptr;
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
                                          SwSidebarWin& rSidebarWin )
{
    if ( mpFrameSidebarWinContainer == nullptr )
    {
        mpFrameSidebarWinContainer = new SwFrameSidebarWinContainer();
    }

    const bool bInserted = mpFrameSidebarWinContainer->insert( rFrame, rFormatField, rSidebarWin );
    if ( bInserted &&
         mpWrtShell->GetAccessibleMap() )
    {
        mpWrtShell->GetAccessibleMap()->InvalidatePosOrSize( nullptr, nullptr, &rSidebarWin, SwRect() );
    }
}

void SwPostItMgr::DisconnectSidebarWinFromFrame( const SwFrame& rFrame,
                                               SwSidebarWin& rSidebarWin )
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
