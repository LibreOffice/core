/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PostItMgr.cxx,v $
 *
 * $Revision: 1.12.34.10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "PostItMgr.hxx"
#include <postithelper.hxx>

#include <vcl/svapp.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/outdev.hxx>

#include <viewopt.hxx>

#include <view.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
#include <doc.hxx>
#include <fldbas.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>
#include <edtwin.hxx>
#include <postit.hxx>
#include <txtfld.hxx>
#include <ndtxt.hxx>
#include <redline.hxx>
#include <docary.hxx>
#include <SwRewriter.hxx>
#include <undobj.hxx>
#include <tools/color.hxx>

#include <swmodule.hxx>
#include <docvw.hrc>
#include "cmdid.h"

#include <SwRewriter.hxx>
#include <undobj.hxx>

#include <sfx2/request.hxx>
#include <sfx2/event.hxx>
#include <sfx2/srchitem.hxx>


#include <svtools/languageoptions.hxx>
#include <svtools/langtab.hxx>
#include <svtools/smplhint.hxx>

#include <svx/svdview.hxx>
#include <svx/eeitem.hxx>
#include <svx/langitem.hxx>
#include <svx/outliner.hxx>

#include <i18npool/mslangid.hxx>
#include <i18npool/lang.h>

#include "swevent.hxx"

// distance between ankor Y and initial note position
#define POSTIT_INITIAL_ANKOR_DISTANCE       20
//distance between two postits
#define POSTIT_SPACE_BETWEEN                8
#define POSTIT_MINIMUMSIZE_WITH_META        60
#define POSTIT_SCROLL_SIDEBAR_HEIGHT        20

// if we layout more often we stop, this should never happen
#define MAX_LOOP_COUNT                      50

/*
bool comp_author( const SwPostItItem* a, const SwPostItItem* b)
{
    return a->pFmtFld->GetFld()->GetPar1() < b->pFmtFld->GetFld()->GetPar1();
}

bool comp_date( const SwPostItItem* a, const SwPostItItem* b)
{
    return static_cast<SwPostItField*>(a->pFmtFld->GetFld())->GetDate()  < static_cast<SwPostItField*>(b->pFmtFld->GetFld())->GetDate();
}
*/

// if position is on the same line, sort by x (Left) position, otherwise by y(Bottom) position
// if two notes are at the same position, sort by logical node position
bool comp_pos(const SwMarginItem *a, const SwMarginItem *b)
{
    return (a->mPos.Bottom() == b->mPos.Bottom()) ?
            ( ((a->mPos.Left() == b->mPos.Left()) && (a->GetBroadCaster()->ISA(SwFmtFld) && b->GetBroadCaster()->ISA(SwFmtFld)) ) ?
                *(static_cast<SwFmtFld*>(a->GetBroadCaster())->GetTxtFld()->GetStart()) <
                *(static_cast<SwFmtFld*>(b->GetBroadCaster())->GetTxtFld()->GetStart())
                : a->mPos.Left() < b->mPos.Left() )
            : a->mPos.Bottom() < b->mPos.Bottom();
}

SwPostItMgr::SwPostItMgr(SwView* pView)
    : mpView(pView)
    , mpWrtShell(mpView->GetDocShell()->GetWrtShell())
    , mpEditWin(&mpView->GetEditWin())
    , mnEventId(0)
    , mbWaitingForCalcRects(false)
    , mpActivePostIt(0)
    , mbLayout(false)
    , mbLayoutHeight(0)
    , mbLayouting(false)
    , mbReadOnly(mpView->GetDocShell()->IsReadOnly())
    , mbDeleteNote(true)
    , mpAnswer(0)
{
    if(!mpView->GetDrawView() )
        mpView->GetWrtShell().MakeDrawView();

    SwNoteProps aProps;
    mpIsShowAnkor = aProps.IsShowAnkor();

    //make sure we get the colour yellow always, even if not the first one of comments or redlining
    SW_MOD()->GetRedlineAuthor();

    // collect all PostIts and redline comments that exist after loading the document
    // don't check for existance for any of them, don't focus them
    AddPostIts(false,false);
    /*  this code can be used once we want redline comments in the margin
    AddRedlineComments(false,false);
    */
    // we want to receive stuff like SFX_HINT_DOCCHANGED
    StartListening(*mpView->GetDocShell());
    if (!mvPostItFlds.empty())
    {
        mbWaitingForCalcRects = true;
        mnEventId = Application::PostUserEvent( LINK( this, SwPostItMgr, CalcHdl), 0 );
    }

    //#i#
    if (HasNotes() && !mpWrtShell->GetViewOptions()->IsPostIts())
        {
                SfxRequest aRequest(mpView->GetViewFrame(),FN_VIEW_NOTES);
                mpView->ExecViewOptions(aRequest);
        }

}

SwPostItMgr::~SwPostItMgr()
{
    if ( mnEventId )
        Application::RemoveUserEvent( mnEventId );
    // forget about all our margin windows
    RemoveMarginWin();
    EndListening( *mpView->GetDocShell() );

    for(std::vector<SwPostItPageItem*>::iterator i = mPages.begin(); i!= mPages.end() ; i++)
        delete (*i);
    mPages.clear();
}

void SwPostItMgr::CheckForRemovedPostIts()
{
    bool bRemoved = false;
    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end(); )
    {
        std::list<SwMarginItem*>::iterator it = i++;
        if ( !(*it)->UseElement() )
        {
            SwMarginItem* p = (*it);
            mvPostItFlds.remove(*it);
            if (GetActivePostIt() == p->pPostIt)
                 SetActivePostIt(0);
            if (p->pPostIt)
                delete p->pPostIt;
            delete p;
            bRemoved = true;
        }
    }

    if ( bRemoved )
    {
        // make sure that no deleted items remain in page lists
        // todo: only remove deleted ones?!
        if ( mvPostItFlds.empty() )
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

void SwPostItMgr::InsertItem(SfxBroadcaster* pItem, bool bCheckExistance, bool bFocus)
{
    if (bCheckExistance)
    {
        for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        {
            if ( (*i)->GetBroadCaster() == pItem )
                return;
        }
    }
    mbLayout = bFocus;
    if (pItem->ISA(SwFmtFld))
        mvPostItFlds.push_back(new SwPostItItem(static_cast<SwFmtFld*>(pItem), true, bFocus) );
    /*
    else
    if (pItem->ISA(SwRedline))
        mvPostItFlds.push_back(new SwRedCommentItem( static_cast<SwRedline*>(pItem), true, bFocus)) ;
    */
    DBG_ASSERT(pItem->ISA(SwFmtFld) /*|| pItem->ISA(SwRedline)*/,"Mgr::InsertItem: seems like new stuff was added");
    StartListening(*pItem);
}

void SwPostItMgr::RemoveItem( SfxBroadcaster* pBroadcast )
{
    EndListening(*pBroadcast);
    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ( (*i)->GetBroadCaster() == pBroadcast )
        {
            SwMarginItem* p = (*i);
            if (GetActivePostIt() == p->pPostIt)
                SetActivePostIt(0);
            mvPostItFlds.remove(*i);
            delete p->pPostIt;
            delete p;
            break;
        }
    }
    mbLayout = true;
    PrepareView();
}

void SwPostItMgr::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.IsA(TYPE(SfxEventHint) ) )
    {
        sal_uInt32 nId = ((SfxEventHint&)rHint).GetEventId();
        if ( nId == SW_EVENT_LAYOUT_FINISHED )
        {
            if ( !mbWaitingForCalcRects && !mvPostItFlds.empty())
            {
                mbWaitingForCalcRects = true;
                mnEventId = Application::PostUserEvent( LINK( this, SwPostItMgr, CalcHdl), 0 );
            }
        }
    }
    else if ( rHint.IsA(TYPE(SfxSimpleHint) ) )
    {
        sal_uInt32 nId = ((SfxSimpleHint&)rHint).GetId();
        switch ( nId )
        {
            case SFX_HINT_MODECHANGED:
            {
                if ( mbReadOnly != !!(mpView->GetDocShell()->IsReadOnly()) )
                {
                    mbReadOnly = !mbReadOnly;
                    SetReadOnlyState();
                    mbLayout = true;
                }
                break;
            }
            case SFX_HINT_DOCCHANGED:
            {
                if ( mpView->GetDocShell() == &rBC )
                {
                    if ( !mbWaitingForCalcRects && !mvPostItFlds.empty())
                    {
                        mbWaitingForCalcRects = true;
                        mnEventId = Application::PostUserEvent( LINK( this, SwPostItMgr, CalcHdl), 0 );
                    }
                }
                break;
            }
            case SFX_HINT_USER04:
            {
                // if we are in a SplitNode/Cut operation, do not delete note and then add again, as this will flicker
                mbDeleteNote = !mbDeleteNote;
                break;
            }
            case SFX_HINT_DYING:
            {
                if ( mpView->GetDocShell() != &rBC )
                {
                    // field to be removed is the broadcaster
                    DBG_ERROR("Notification for removed SwFmtFld was not sent!");
                    RemoveItem(&rBC);
                }
                break;
            }
        }
    }
    /*
    else if ( rHint.IsA(TYPE(SwRedlineHint) ) )
    {
        const SwRedlineHint rRedlineHint = static_cast<const SwRedlineHint&>(rHint);
           SwRedline* pRedline = const_cast<SwRedline*>(rRedlineHint.GetRedline());
        switch ( rRedlineHint.Which() )
        {
            case SWREDLINE_INSERTED :
            {
                bool bEmpty = !HasNotes();
                InsertItem( pRedline, true, false );
                if (bEmpty && !mvPostItFlds.empty())
                    PrepareView(true);
                break;
            }
            case SWREDLINE_REMOVED:
            {
                RemoveItem(pRedline);
                break;
            }
            case SWREDLINE_FOCUS:
            {
                                if (rRedlineHint.GetView()== mpView)
                    Focus(rBC);
                break;
            }
        }
    }
    */
    else if ( rHint.IsA(TYPE(SwFmtFldHint) ) )
    {
        const SwFmtFldHint& rFmtHint = static_cast<const SwFmtFldHint&>(rHint);
        SwFmtFld* pFld = const_cast <SwFmtFld*>( rFmtHint.GetField() );
        switch ( rFmtHint.Which() )
        {
            case SWFMTFLD_INSERTED :
            {
                if (!pFld)
                {
                    AddPostIts(true);
                    break;
                }
                // get field to be inserted from hint
                if ( pFld->IsFldInDoc() )
                {
                    bool bEmpty = !HasNotes();
                    InsertItem( pFld, true, false );
                    if (bEmpty && !mvPostItFlds.empty())
                        PrepareView(true);
                }
                else
                {
                    DBG_ERROR( "Inserted field not in document!" );
                        }
                break;
            }
            case SWFMTFLD_REMOVED:
            {
                if (mbDeleteNote)
                {
                    if (!pFld)
                    {
                        CheckForRemovedPostIts();
                        break;
                    }
                    RemoveItem(pFld);
                }
                break;
            }
            case SWFMTFLD_FOCUS:
            {
                       if (rFmtHint.GetView()== mpView)
                    Focus(rBC);
                break;
            }
            case SWFMTFLD_CHANGED:
            {
                        SwFmtFld* pFmtFld = dynamic_cast<SwFmtFld*>(&rBC);
                for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
                {
                            if ( pFmtFld == (*i)->GetBroadCaster() )
                    {
                        if ((*i)->pPostIt)
                        {
                            (*i)->pPostIt->SetPostItText();
                            mbLayout = true;
                        }
                        break;
                    }
                }
                break;
            }
            case SWFMTFLD_LANGUAGE:
            {
                        SwFmtFld* pFmtFld = dynamic_cast<SwFmtFld*>(&rBC);
                for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
                {
                                if ( pFmtFld == (*i)->GetBroadCaster() )
                    {
                        if ((*i)->pPostIt)
                        {
                            USHORT nScriptType = SvtLanguageOptions::GetScriptTypeOfLanguage( (*i)->GetFmtFld()->GetFld()->GetLanguage() );
                            USHORT nLangWhichId = 0;
                            switch (nScriptType)
                            {
                                case SCRIPTTYPE_LATIN :    nLangWhichId = EE_CHAR_LANGUAGE ; break;
                                case SCRIPTTYPE_ASIAN :    nLangWhichId = EE_CHAR_LANGUAGE_CJK; break;
                                case SCRIPTTYPE_COMPLEX :  nLangWhichId = EE_CHAR_LANGUAGE_CTL; break;
                            }
                            (*i)->pPostIt->SetLanguage(SvxLanguageItem((*i)->GetFmtFld()->GetFld()->GetLanguage(),nLangWhichId));
                        }
                        break;
                    }
                }
                break;
            }
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

    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
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
        mnEventId = 0;
    }

    bool bChange = false;
    bool bRepair = false;
    PreparePageContainer();
    if ( !mvPostItFlds.empty() )
    {
        for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        {
            SwMarginItem* pItem = (*i);
            if ( !pItem->UseElement() )
            {
                DBG_ERROR("PostIt is not in doc or other wrong use");
                bRepair = true;
                continue;
            }

            //save old rect and visible state
            SwRect aOldRect(pItem->mPos);
            SwPostItHelper::SwLayoutStatus eOldStatus = pItem->mLayoutStatus;
            std::vector< SwLayoutInfo > aInfo;
             SwPosition aPosition = pItem->GetPosition();
            pItem->mLayoutStatus = SwPostItHelper::getLayoutInfos( aInfo, aPosition );
            if( aInfo.size() )
            {
                    SwLayoutInfo& rInfo = aInfo[0];
                    pItem->mPos = rInfo.mPosition;
                    pItem->mPagePos = rInfo.mPageFrame;
                    pItem->mFramePos = rInfo.mPagePrtArea;
                    pItem->bMarginSide = rInfo.mbMarginSide;
                    pItem->mnPageNumber = rInfo.mnPageNumber;
                    pItem->mRedlineAuthor = rInfo.mRedlineAuthor;
            }
            bChange = bChange || (pItem->mPos != aOldRect) || (eOldStatus != pItem->mLayoutStatus);
        }

        // show notes in right order in navigator
        //prevent ankors during layout to overlap, e.g. when moving a frame
        Sort(SORT_POS);

        // sort the items into the right page vector, so layout can be done by page
        for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        {
            SwMarginItem* pItem = (*i);
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

            const unsigned long aPageNum = pItem->mnPageNumber;
            if (aPageNum > mPages.size())
            {
                const unsigned long nNumberOfPages = mPages.size();
                for (unsigned int j=0; j<aPageNum - nNumberOfPages; ++j)
                    mPages.push_back( new SwPostItPageItem());
            }
            mPages[aPageNum-1]->mList->push_back(pItem);
            mPages[aPageNum-1]->mPageRect = pItem->mPagePos;
            mPages[aPageNum-1]->bMarginSide = pItem->bMarginSide;
        }

        if (!bChange && mpWrtShell->getIDocumentSettingAccess()->get(IDocumentSettingAccess::BROWSE_MODE))
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
    for(std::list<SwMarginItem*>::const_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ((*i)->bShow && (*i)->pPostIt && (*i)->pPostIt->Scrollbar())
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
    else
    if (lContainerSize > lPageSize)
    {
        for (int i=mPages.size()-1; i >= lPageSize;--i)
        {
            delete mPages[i];
            mPages.pop_back();
        }
    }
    // only clear the list, DO NOT delete the objects itself
    for(std::vector<SwPostItPageItem*>::iterator i = mPages.begin(); i!= mPages.end() ; i++)
    {
        (*i)->mList->clear();
        if (mvPostItFlds.empty())
            (*i)->bScrollbar = false;

    }
}

void SwPostItMgr::LayoutPostIts()
{
    if ( !mvPostItFlds.empty() && !mbWaitingForCalcRects )
    {
        mbLayouting = true;

            //loop over all pages and do the layout
            // - create SwPostIt if neccessary
            // - place SwPostIts on their initial position
            // - calculate neccessary height for all PostIts together
            bool bUpdate = false;
            for (unsigned long n=0;n<mPages.size();n++)
            {
                // only layout if there are notes on this page
                if (mPages[n]->mList->size()>0)
                {
                    std::list<SwMarginWin*> aVisiblePostItList;
                    unsigned long           lNeededHeight = 0;
                    long                    mlPageBorder = 0;
                    long                    mlPageEnd = 0;

                    for(SwMarginItem_iterator i = mPages[n]->mList->begin(); i!= mPages[n]->mList->end(); i++)
                    {
                        SwMarginItem* pItem = (*i);
                        SwMarginWin* pPostIt = pItem->pPostIt;

                        if (mPages[n]->bMarginSide)
                        {
                            // x value for notes positioning
                            mlPageBorder = mpEditWin->LogicToPixel( Point( mPages[n]->mPageRect.Left(), 0)).X() - GetSidebarWidth(true);// - GetSidebarBorderWidth(true);
                            //bending point
                            mlPageEnd = mpWrtShell->getIDocumentSettingAccess()->get( IDocumentSettingAccess::BROWSE_MODE) ? pItem->mFramePos.Left() : mPages[n]->mPageRect.Left() + 350;
                        }
                        else
                        {
                            // x value for notes positioning
                            mlPageBorder = mpEditWin->LogicToPixel( Point(mPages[n]->mPageRect.Right(), 0)).X() + GetSidebarBorderWidth(true);
                            //bending point
                            mlPageEnd = mpWrtShell->getIDocumentSettingAccess()->get( IDocumentSettingAccess::BROWSE_MODE) ? pItem->mFramePos.Right() : mPages[n]->mPageRect.Right() - 350;
                        }

                        if (pItem->bShow)
                        {
                            long Y = mpEditWin->LogicToPixel( Point(0,pItem->mPos.Bottom())).Y();
                            long aPostItHeight = 0;
                            if (!pPostIt)
                            {
                                pPostIt = (*i)->GetMarginWindow(static_cast<Window*>(&mpView->GetEditWin()),WINDOW_CONTROL|WB_NODIALOGCONTROL,this,0);
                                pPostIt->InitControls();
                                pPostIt->SetReadonly(mbReadOnly);
                                pItem->pPostIt = pPostIt;
                                if (mpAnswer)
                                {
                                    if (pPostIt->CalcFollow()) //do we really have another note in front of this one
                                        static_cast<SwPostIt*>(pPostIt)->InitAnswer(mpAnswer);
                                    delete mpAnswer;
                                    mpAnswer = 0;
                                }
                            }

                                            if (pItem->pPostIt->ISA(SwPostIt))
                            {
                                static_cast<SwPostIt*>(pPostIt)->SetChangeTracking(pItem->mLayoutStatus,GetColorAnkor(pItem->mRedlineAuthor));
                            }
                            pPostIt->SetMarginSide(mPages[n]->bMarginSide);
                            pPostIt->SetFollow(pPostIt->CalcFollow());
                            aPostItHeight = ( pPostIt->GetPostItTextHeight() < pPostIt->GetMinimumSizeWithoutMeta() ? pPostIt->GetMinimumSizeWithoutMeta() : pPostIt->GetPostItTextHeight() ) + pPostIt->GetMetaHeight();
                            pPostIt->SetPosSizePixelRect( mlPageBorder ,Y-GetInitialAnchorDistance(), GetNoteWidth() ,aPostItHeight,pItem->mPos, mlPageEnd);

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

                    if ((aVisiblePostItList.size()>0) && ShowNotes())
                    {
                        bool bOldScrollbar = mPages[n]->bScrollbar;
                        if (ShowNotes())
                            mPages[n]->bScrollbar = LayoutByPage(aVisiblePostItList, mPages[n]->mPageRect.SVRect(), lNeededHeight);
                        else
                            mPages[n]->bScrollbar = false;
                        if (!mPages[n]->bScrollbar)
                        {
                            mPages[n]->lOffset = 0;
                        }
                        else
                        {
                            //when we changed our zoom level, the offset value can be to big, so lets check for the largest possible zoom value
                            long aAvailableHeight = mpEditWin->LogicToPixel(Size(0,mPages[n]->mPageRect.Height())).Height() - 2 * GetSidebarScrollerHeight();
                            long lOffset = -1 * GetScrollSize() * (aVisiblePostItList.size() - aAvailableHeight / GetScrollSize());
                            if (mPages[n]->lOffset < lOffset)
                                mPages[n]->lOffset = lOffset;
                        }
                        bUpdate = (bOldScrollbar != mPages[n]->bScrollbar) || bUpdate;
                        const long aSidebarheight = mPages[n]->bScrollbar ? mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height() : 0;
                        /*
                                           TODO
                                           - enlarge all notes till GetNextBorder(), as we resized to average value before
                                           */
                        //lets hide the ones which overlap the page
                        for(SwMarginWin_iterator i = aVisiblePostItList.begin(); i!= aVisiblePostItList.end() ; i++)
                        {
                            if (mPages[n]->lOffset != 0)
                                (*i)->TranslateTopPosition(mPages[n]->lOffset);

                            bool bBottom  = mpEditWin->PixelToLogic(Point(0,(*i)->VirtualPos().Y()+(*i)->VirtualSize().Height())).Y() <= (mPages[n]->mPageRect.Bottom()-aSidebarheight);
                            bool bTop = mpEditWin->PixelToLogic(Point(0,(*i)->VirtualPos().Y())).Y() >= (mPages[n]->mPageRect.Top()+aSidebarheight);
                            if ( bBottom && bTop )
                            {
                                (*i)->ShowNote();
                            }
                            else
                            {
                                if (mpEditWin->PixelToLogic(Point(0,(*i)->VirtualPos().Y())).Y() < (mPages[n]->mPageRect.Top()+aSidebarheight))
                                {
                                    if (mPages[n]->bMarginSide)
                                        (*i)->ShowAnkorOnly(Point(mPages[n]->mPageRect.Left(),mPages[n]->mPageRect.Top()));
                                    else
                                        (*i)->ShowAnkorOnly(Point(mPages[n]->mPageRect.Right(),mPages[n]->mPageRect.Top()));
                                }
                                else
                                {
                                    if (mPages[n]->bMarginSide)
                                        (*i)->ShowAnkorOnly(Point(mPages[n]->mPageRect.Left(),mPages[n]->mPageRect.Bottom()));
                                    else
                                        (*i)->ShowAnkorOnly(Point(mPages[n]->mPageRect.Right(),mPages[n]->mPageRect.Bottom()));
                                }
                                DBG_ASSERT(mPages[n]->bScrollbar,"SwPostItMgr::LayoutByPage(): note overlaps, but bScrollbar is not true");
                            }
                        }

                        // do some magic so we really see the focused note
                        for(SwMarginWin_iterator i = aVisiblePostItList.begin(); i!= aVisiblePostItList.end() ; i++)
                        {
                                            if ((*i)->HasChildPathFocus())
                            {
                                MakeVisible((*i),n+1);
                                break;
                            }
                        }
                    }
                    else
                    {
                        for(SwMarginWin_iterator i = aVisiblePostItList.begin(); i!= aVisiblePostItList.end() ; i++)
                                                                (*i)->SetPosAndSize();

                                                        bool bOldScrollbar = mPages[n]->bScrollbar;
                                                        mPages[n]->bScrollbar = false;
                                                        bUpdate = (bOldScrollbar != mPages[n]->bScrollbar) || bUpdate;
                    }
                    aVisiblePostItList.clear();
                }
                else
                {
                    bUpdate = true;
                    mPages[n]->bScrollbar = false;
                }
            }

            if (!ShowNotes())
            {       // we do not want to see the notes anymore -> Options-Writer-View-Notes
                bool bRepair = false;
                for(SwMarginItem_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
                {
                    SwMarginItem* pItem = (*i);
                    if ( !pItem->UseElement() )
                    {
                        DBG_ERROR("PostIt is not in doc!");
                        bRepair = true;
                        continue;
                    }

                    if ((*i)->pPostIt)
                    {
                        (*i)->pPostIt->HideNote();
                        if ((*i)->pPostIt->HasChildPathFocus())
                        {
                            SetActivePostIt(0);
                            (*i)->pPostIt->GrabFocusToDocument();
                        }
                    }
                }

                if ( bRepair )
                    CheckForRemovedPostIts();
            }


            // notes scrollbar is otherwise not drawn correctly for some cases
            // scrollbar area is enough
            if (bUpdate)
                mpEditWin->Invalidate();
        mbLayouting = false;
    }
}

bool SwPostItMgr::BorderOverPageBorder(unsigned long aPage) const
{
    if ( mPages[aPage-1]->mList->empty() )
    {
        DBG_ERROR("Notes SidePane painted but no rects and page lists calculated!");
        return false;
    }

    SwMarginItem_iterator aItem = mPages[aPage-1]->mList->end();
    --aItem;
    DBG_ASSERT ((*aItem)->pPostIt,"BorderOverPageBorder: NULL postIt, should never happen");
    if ((*aItem)->pPostIt)
    {
        const long aSidebarheight = mPages[aPage-1]->bScrollbar ? mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height() : 0;
        const long aEndValue = mpEditWin->PixelToLogic(Point(0,(*aItem)->pPostIt->GetPosPixel().Y()+(*aItem)->pPostIt->GetSizePixel().Height())).Y();
        return aEndValue <= mPages[aPage-1]->mPageRect.Bottom()-aSidebarheight;
    }
    else
        return false;
}

void SwPostItMgr::Scroll(const long lScroll,const unsigned long aPage)
{
    DBG_ASSERT((lScroll % GetScrollSize() )==0,"SwPostItMgr::Scroll: scrolling by wrong value");
    // do not scroll more than neccessary up or down
    if ( ((mPages[aPage-1]->lOffset == 0) && (lScroll>0)) || ( BorderOverPageBorder(aPage) && (lScroll<0)) )
        return;

    const bool bOldUp = ArrowEnabled(KEY_PAGEUP,aPage);
    const bool bOldDown = ArrowEnabled(KEY_PAGEDOWN,aPage);
    const long aSidebarheight = mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height();
    for(SwMarginItem_iterator i = mPages[aPage-1]->mList->begin(); i!= mPages[aPage-1]->mList->end(); i++)
    {
        SwMarginWin* pPostIt = (*i)->pPostIt;
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
                    if (mPages[aPage-1]->bMarginSide)
                        pPostIt->ShowAnkorOnly(Point(mPages[aPage-1]->mPageRect.Left(),mPages[aPage-1]->mPageRect.Top()));
                    else
                        pPostIt->ShowAnkorOnly(Point(mPages[aPage-1]->mPageRect.Right(),mPages[aPage-1]->mPageRect.Top()));
                }
                else
                {
                    if (mPages[aPage-1]->bMarginSide)
                        pPostIt->ShowAnkorOnly(Point(mPages[aPage-1]->mPageRect.Left(),mPages[aPage-1]->mPageRect.Bottom()));
                    else
                        pPostIt->ShowAnkorOnly(Point(mPages[aPage-1]->mPageRect.Right(),mPages[aPage-1]->mPageRect.Bottom()));
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

void SwPostItMgr::AutoScroll(const SwMarginWin* pPostIt,const unsigned long aPage )
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

void SwPostItMgr::MakeVisible(const SwMarginWin* pPostIt,long aPage )
{
    if (aPage == -1)
    {
        // we dont know the page yet, lets find it ourselves
        for (unsigned long n=0;n<mPages.size();n++)
        {
            if (mPages[n]->mList->size()>0)
            {
                for(SwMarginItem_iterator i = mPages[n]->mList->begin(); i!= mPages[n]->mList->end(); i++)
                {
                    if ((*i)->pPostIt==pPostIt)
                    {
                        aPage = n+1;
                        break;
                    }
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

bool SwPostItMgr::ArrowEnabled(USHORT aDirection,unsigned long aPage) const
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

Color SwPostItMgr::GetArrowColor(USHORT aDirection,unsigned long aPage) const
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

bool SwPostItMgr::LayoutByPage(std::list<SwMarginWin*> &aVisiblePostItList,const Rectangle aBorder, long lNeededHeight)
{
    /*** General layout idea:***/
    //  - if we have space left, we always move the current one up,
    //    otherwise the next one down
    //  - first all notes are resized
    //  - then the real layout starts
    /*************************************************************/

    //rBorder is the page rect
    const Rectangle rBorder         = mpEditWin->LogicToPixel( aBorder);
    long            lTopBorder      = rBorder.Top() + 5;
    long            lBottomBorder   = rBorder.Bottom() - 5;
    const long      lVisibleHeight  = lBottomBorder - lTopBorder; //rBorder.GetHeight() ;
    long            lSpaceUsed      = 0;
    long            lTranslatePos   = 0;
    int             loop            = 0;
    bool            bDone           = false;
    bool            bScrollbars     = false;

    // do all neccessary resizings
    if (lVisibleHeight < lNeededHeight)
    {
        // ok, now we have to really resize and adding scrollbars
        const long lAverageHeight = (lVisibleHeight - aVisiblePostItList.size()*GetSpaceBetween()) / aVisiblePostItList.size();
        if (lAverageHeight<GetMinimumSizeWithMeta())
        {
            bScrollbars = true;
            lTopBorder += GetSidebarScrollerHeight() + 10;
            lBottomBorder -= (GetSidebarScrollerHeight() + 10);
                for(SwMarginWin_iterator i = aVisiblePostItList.begin(); i!= aVisiblePostItList.end() ; i++)
                    (*i)->SetSize(Size((*i)->VirtualSize().getWidth(),(*i)->GetMinimumSizeWithMeta()));
        }
        else
        {
            for(SwMarginWin_iterator i = aVisiblePostItList.begin(); i!= aVisiblePostItList.end() ; i++)
            {
                if ( (*i)->VirtualSize().getHeight() > lAverageHeight)
                    (*i)->SetSize(Size((*i)->VirtualSize().getWidth(),lAverageHeight));
            }
        }
    }

    //start the real layout so nothing overlaps anymore
    if (aVisiblePostItList.size()>1)
    {
        // if no window is moved anymore we are finished
        while (!bDone)
        {
            loop++;
              bDone = true;
            lSpaceUsed = lTopBorder + GetSpaceBetween();
            for(SwMarginWin_iterator i = aVisiblePostItList.begin(); i!= aVisiblePostItList.end() ; i++)
            {
                SwMarginWin_iterator aNextPostIt = i;
                ++aNextPostIt;

                if (aNextPostIt !=aVisiblePostItList.end())
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
                                    (*i)->TranslateTopPosition(-1*(lTranslatePos+ANKORLINE_WIDTH));
                                else
                                    (*i)->TranslateTopPosition(-1*(lTranslatePos+GetSpaceBetween()));
                            }
                            else
                            {
                                long lMoveUp = (*i)->VirtualPos().Y() - lTopBorder;
                                (*i)->TranslateTopPosition(-1* lMoveUp);
                                if ((*aNextPostIt)->IsFollow())
                                    (*aNextPostIt)->TranslateTopPosition( (lTranslatePos+ANKORLINE_WIDTH) - lMoveUp);
                                else
                                    (*aNextPostIt)->TranslateTopPosition( (lTranslatePos+GetSpaceBetween()) - lMoveUp);
                            }
                        }
                        else
                        {
                            // no space left, left move the next one down
                            if ((*aNextPostIt)->IsFollow())
                                (*aNextPostIt)->TranslateTopPosition(lTranslatePos+ANKORLINE_WIDTH);
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
                    if (aNextPostIt !=aVisiblePostItList.end() && (*aNextPostIt)->IsFollow())
                        lSpaceUsed += (*i)->VirtualSize().Height() + ANKORLINE_WIDTH;
                    else
                        lSpaceUsed += (*i)->VirtualSize().Height() + GetSpaceBetween();
                }
                else
                {
                    //(*i) is the last visible item
                    SwMarginWin_iterator aPrevPostIt = i;
                    --aPrevPostIt;
                    //lTranslatePos = ( (*aPrevPostIt)->VirtualPos().Y() + (*aPrevPostIt)->VirtualSize().Height() + GetSpaceBetween() ) - (*i)->VirtualPos().Y();
                    lTranslatePos = ( (*aPrevPostIt)->VirtualPos().Y() + (*aPrevPostIt)->VirtualSize().Height() ) - (*i)->VirtualPos().Y();
                    if (lTranslatePos > 0)
                    {
                        bDone = false;
                        if ( ((*i)->VirtualPos().Y()+ (*i)->VirtualSize().Height()+lTranslatePos) < lBottomBorder)
                        {
                            if ( (*i)->IsFollow() )
                                (*i)->TranslateTopPosition(lTranslatePos+ANKORLINE_WIDTH);
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
                DBG_ERROR("PostItMgr::Layout(): We are looping forever");
                break;
            }
        }
    }
    else
    {
        // only one left, make sure it is not hidden at the top or bottom
        SwMarginWin_iterator i = aVisiblePostItList.begin();
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

/*
void SwPostItMgr::AddRedlineComments(bool bCheckExistance, bool bFocus)
{
    bool bEmpty = mvPostItFlds.empty();
    const SwRedlineTbl& aTable = mpView->GetDocShell()->GetDoc()->GetRedlineTbl();
    for( USHORT i = 0; i < aTable.Count(); ++i )
    {
        SwRedline* pRedline = const_cast<SwRedline*>((aTable)[i]);
        if ( pRedline->GetComment() != String(rtl::OUString::createFromAscii("")) )
            InsertItem(pRedline, bCheckExistance, bFocus);
    }
    if (bEmpty && !mvPostItFlds.empty())
        PrepareView(true);
 }
 */

void SwPostItMgr::AddPostIts(bool bCheckExistance, bool bFocus)
{
    bool bEmpty = mvPostItFlds.empty();
    SwFieldType* pType = mpView->GetDocShell()->GetDoc()->GetFldType(RES_POSTITFLD, aEmptyStr,false);
    SwClientIter aIter( *pType );
    SwClient * pFirst = aIter.GoStart();
    while(pFirst)
    {
        SwFmtFld* pSwFmtFld = static_cast<SwFmtFld*>(pFirst);
        if ( pSwFmtFld->GetTxtFld())
        {
            if ( pSwFmtFld->IsFldInDoc() )
                InsertItem(pSwFmtFld,bCheckExistance,bFocus);
        }
        pFirst = aIter++;
    }

    // if we just added the first one we have to update the view for centering
    if (bEmpty && !mvPostItFlds.empty())
        PrepareView(true);
}

void SwPostItMgr::RemoveMarginWin()
{
    if (!mvPostItFlds.empty())
    {
        for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        {
            EndListening( *((*i)->GetBroadCaster()) );
            if ((*i)->pPostIt)
                delete (*i)->pPostIt;
            delete (*i);
        }
        mvPostItFlds.clear();
    }

    // all postits removed, no items should be left in pages
    PreparePageContainer();
}

// copy to new vector, otherwise RemoveItem would operate and delete stuff on mvPostItFlds as well
// RemoveItem will clean up the core field and visible postit if neccessary
// we cannot just delete everything as before, as postits could move into change tracking
void SwPostItMgr::Delete(String aAuthor)
{
    mpWrtShell->StartAllAction();
    if ( GetActivePostIt() && (GetActivePostIt()->GetAuthor()==aAuthor) )
        SetActivePostIt(0);
    SwRewriter aRewriter;
    String aUndoString = SW_RES(STR_DELETE_AUTHOR_NOTES);
    aUndoString += aAuthor;
    aRewriter.AddRule(UNDO_ARG1, aUndoString);
    mpWrtShell->StartUndo( UNDO_DELETE, &aRewriter );

    std::vector<SwFmtFld*> aTmp;
    aTmp.reserve( mvPostItFlds.size() );
    for(std::list<SwMarginItem*>::iterator pPostIt = mvPostItFlds.begin(); pPostIt!= mvPostItFlds.end() ; pPostIt++)
    {
        if ((*pPostIt)->GetFmtFld() && ((*pPostIt)->pPostIt->GetAuthor() == aAuthor) )
            aTmp.push_back( (*pPostIt)->GetFmtFld() );
    }
    for(std::vector<SwFmtFld*>::iterator i = aTmp.begin(); i!= aTmp.end() ; i++)
    {
        mpWrtShell->GotoField( *(*i) );
        mpWrtShell->DelRight();
    }
    /*
    for(std::list<SwPostItItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end(); )
        {
            SwPostItItem* pItem = (*i);
            SwPostItField* pPostItField = static_cast<SwPostItField*>(pItem->pFmtFld->GetFld());
            if (pPostItField->GetPar1() == aAuthor)
            {
                // stop listening, we delete ourselves
                EndListening( *(pItem->pFmtFld) );

                // remove reference to yet-to-die postit
                if (pItem->pPostIt == GetActivePostIt())
                    SetActivePostIt(0);

                // delete the actual SwPostItField
                mpWrtShell->GotoField(*pItem->pFmtFld);
                mpWrtShell->DelRight();
                i = mvPostItFlds.erase(i);

                // delete visual representation
                // lazy delete doesn't work because references to document (mpFmtFld, mpView etc. may be dead before deletion)
                delete pItem->pPostIt;

                // delete struct saving the pointers
                delete pItem;
            }
            else
                ++i;
        }
    */
    mpWrtShell->EndUndo( UNDO_DELETE );
    PrepareView();
    mpWrtShell->EndAllAction();
    mbLayout = true;
    CalcRects();
    LayoutPostIts();
}

void SwPostItMgr::Delete()
{
    mpWrtShell->StartAllAction();
    SetActivePostIt(0);
    SwRewriter aRewriter;
    aRewriter.AddRule(UNDO_ARG1, SW_RES(STR_DELETE_ALL_NOTES) );
    mpWrtShell->StartUndo( UNDO_DELETE, &aRewriter );

    std::vector<SwFmtFld*> aTmp;
    aTmp.reserve( mvPostItFlds.size() );
    for(std::list<SwMarginItem*>::iterator pPostIt = mvPostItFlds.begin(); pPostIt!= mvPostItFlds.end() ; pPostIt++)
    {
        if ((*pPostIt)->GetFmtFld())
            aTmp.push_back( (*pPostIt)->GetFmtFld() );
    }
    for(std::vector<SwFmtFld*>::iterator i = aTmp.begin(); i!= aTmp.end() ; i++)
    {
        mpWrtShell->GotoField( *(*i) );
        mpWrtShell->DelRight();
    }

/*
    for(std::list<SwPostItItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        SwPostItItem* pItem = (*i);
        // stop listening, we delete ourselves
        EndListening( *(pItem->pFmtFld) );
        // delete the actual SwPostItField
        mpWrtShell->GotoField(*pItem->pFmtFld);
        mpWrtShell->DelRight();
        // delete visual representation
        delete pItem->pPostIt;
        // delete struct saving the pointers
        delete pItem;
    }
    mvPostItFlds.clear();
*/

    mpWrtShell->EndUndo( UNDO_DELETE );
    PrepareView();
    mpWrtShell->EndAllAction();
    mbLayout = true;
    CalcRects();
    LayoutPostIts();
}

void SwPostItMgr::Hide(SwPostItField* pPostItField )
{
    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ((*i)->GetFmtFld())
        {
            SwPostItField* pField = static_cast<SwPostItField*>((*i)->GetFmtFld()->GetFld());
            if (pPostItField==pField)
            {
                (*i)->bShow = false;
                (*i)->pPostIt->HideNote();
                break;
            }
        }
    }

    LayoutPostIts();
}

void SwPostItMgr::Hide( const String& rAuthor )
{
    for(SwMarginItem_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
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
    for(SwMarginItem_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        (*i)->bShow = false;
        (*i)->pPostIt->HideNote();
    }
}


void SwPostItMgr::Show()
{
    for(SwMarginItem_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        (*i)->bShow = true;
    }
    LayoutPostIts();
}

void SwPostItMgr::Sort(const short aType)
{
    if (mvPostItFlds.size()>1 )
    {
        switch (aType)
        {
            case SORT_POS:
                mvPostItFlds.sort(comp_pos);
                break;
            /*
            case SORT_AUTHOR:
                mvPostItFlds.sort(comp_author);
                break;
            case SORT_DATE:
                mvPostItFlds.sort(comp_date);
                break;
            */
        }
    }
}

SwMarginWin* SwPostItMgr::GetPostIt(SfxBroadcaster* pBroadcaster) const
{
    for(const_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ( (*i)->GetBroadCaster() == pBroadcaster)
            return (*i)->pPostIt;
    }
    return NULL;
}

SwPostIt* SwPostItMgr::GetPostIt(SwPostItField* pFld) const
{
    for(const_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ( (*i)->GetFmtFld() && ((*i)->GetFmtFld()->GetFld() == pFld) )
            return static_cast<SwPostIt*>((*i)->pPostIt);
    }
    return NULL;
}

SwMarginWin* SwPostItMgr::GetPostIt( const SfxBroadcaster* pBroadcaster) const
{
    for(const_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ( (*i)->GetBroadCaster() == pBroadcaster)
            return (*i)->pPostIt;
    }
    return NULL;
}

SwPostIt* SwPostItMgr::GetPostIt(const SwPostItField* pFld) const
{
    for(const_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
    {
        if ( (*i)->GetFmtFld() && ((*i)->GetFmtFld()->GetFld() == pFld))
            return static_cast<SwPostIt*>((*i)->pPostIt);
    }
    return NULL;
}

bool SwPostItMgr::ShowPreview(const SwField* pFld, SwFmtFld*& pFmtFld) const
{
    for (unsigned long n=0;n<mPages.size();n++)
    {
        if (mPages[n]->mList->size()>0)
        {
            for(const_iterator i = mPages[n]->mList->begin(); i!= mPages[n]->mList->end(); i++)
            {
                if ( (*i)->GetFmtFld() && ((*i)->GetFmtFld()->GetFld()==pFld) )
                {
                    pFmtFld = (*i)->GetFmtFld();
                    const long aSidebarheight = mPages[n]->bScrollbar ? mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height() : 0;
                    bool bTopPage = mpEditWin->PixelToLogic(Point(0,(*i)->pPostIt->GetPosPixel().Y())).Y() >= (mPages[n]->mPageRect.Top()+aSidebarheight);
                    bool bBottomPage  = mpEditWin->PixelToLogic(Point(0,(*i)->pPostIt->GetPosPixel().Y()+(*i)->pPostIt->GetSizePixel().Height())).Y() <= (mPages[n]->mPageRect.Bottom()-aSidebarheight);
                    const bool bTopVis = mpEditWin->PixelToLogic(Point(0,(*i)->pPostIt->GetPosPixel().Y())).Y() > mpView->GetVisArea().Top();
                    const bool bBottomVis  = mpEditWin->PixelToLogic(Point(0,(*i)->pPostIt->GetPosPixel().Y()/*+(*i)->pPostIt->GetSizePixel().Height()*/)).Y() <= mpView->GetVisArea().Bottom();
                    return !(bBottomPage && bTopPage && bBottomVis && bTopVis);
                }
            }
        }
    }
    return false;
}

SwMarginWin* SwPostItMgr::GetNextPostIt(USHORT aDirection, SwMarginWin* aPostIt)
{
    if (mvPostItFlds.size()>1)
    {
        for(SwMarginItem_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        {
            if ( (*i)->pPostIt ==aPostIt)
            {
                SwMarginItem_iterator iNextPostIt   = i;
                if (aDirection==KEY_PAGEUP)
                {
                    if ( iNextPostIt==mvPostItFlds.begin() )
                    {
                        return NULL;
                    }
                    --iNextPostIt;
                }
                else
                {
                    iNextPostIt++;
                    if ( iNextPostIt==mvPostItFlds.end() )
                    {
                        return NULL;
                    }
                }
                // lets quit, we are back at the beginng
                if ( (*iNextPostIt)->pPostIt==aPostIt)
                    return NULL;
                return (*iNextPostIt)->pPostIt;
            }
        }
        return NULL;
    }
    else
        return NULL;
}

long SwPostItMgr::GetNextBorder()
{
    for (unsigned long n=0;n<mPages.size();n++)
    {
        for(SwMarginItem_iterator b = mPages[n]->mList->begin(); b!= mPages[n]->mList->end(); b++)
        {
            if ((*b)->pPostIt == mpActivePostIt)
            {
                SwMarginItem_iterator aNext = b;
                aNext++;
                bool bFollow = (aNext == mPages[n]->mList->end()) ? false : (*aNext)->pPostIt->IsFollow();
                if ( mPages[n]->bScrollbar || bFollow )
                {
                    return -1;
                }
                else
                {
                    //if this is the last item, return the bottom border otherwise the next item
                    if (aNext == mPages[n]->mList->end())
                        return mpEditWin->LogicToPixel(Point(0,mPages[n]->mPageRect.Bottom())).Y() - GetSpaceBetween();
                    else
                        return (*aNext)->pPostIt->GetPosPixel().Y() - GetSpaceBetween();
                }
            }
        }
    }

    DBG_ERROR("SwPostItMgr::GetNextBorder(): We have to find a next border here");
    return -1;
}

void SwPostItMgr::SetShadowState(const SwPostItField* pFld,bool bCursor)
{
    if (pFld)
    {
        if (pFld !=mShadowState.mpShadowFld)
        {
            if (mShadowState.mpShadowFld)
            {
                // reset old one if still alive
                // TODO: does not work properly if mouse and cursor was set
                SwPostIt* pOldPostIt = GetPostIt(mShadowState.mpShadowFld);
                if (pOldPostIt && pOldPostIt->Shadow() && (pOldPostIt->Shadow()->GetShadowState() != SS_EDIT))
                    pOldPostIt->SetViewState(SS_NORMAL);
            }
            //set new one, if it is not currently edited
            SwPostIt* pNewPostIt = GetPostIt(pFld);
            if (pNewPostIt && pNewPostIt->Shadow() && (pNewPostIt->Shadow()->GetShadowState() != SS_EDIT))
            {
                pNewPostIt->SetViewState(SS_VIEW);
                //remember our new field
                mShadowState.mpShadowFld = pFld;
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
        if (mShadowState.mpShadowFld)
        {
            if (bCursor)
                mShadowState.bCursor = false;
            else
                mShadowState.bMouse = false;
            if (!mShadowState.bCursor && !mShadowState.bMouse)
            {
                // reset old one if still alive
                SwPostIt* pOldPostIt = GetPostIt(mShadowState.mpShadowFld);
                if (pOldPostIt && pOldPostIt->Shadow() && (pOldPostIt->Shadow()->GetShadowState() != SS_EDIT))
                {
                    pOldPostIt->SetViewState(SS_NORMAL);
                    mShadowState.mpShadowFld = 0;
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
        //mpEditWin->Invalidate(); // really not needed anymore??
        SwRootFrm* pLayout = mpWrtShell->GetLayout();
        if ( pLayout )
            SwPostItHelper::setSidebarChanged( pLayout,
                mpWrtShell->getIDocumentSettingAccess()->get( IDocumentSettingAccess::BROWSE_MODE ) );
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
        const SwRootFrm* pLayout = mpWrtShell->GetLayout();
        SwRect aPageFrm;
        const unsigned long nPageNum = SwPostItHelper::getPageInfo( aPageFrm, pLayout, aPoint );
        if( nPageNum )
        {
            Rectangle aRect;
            DBG_ASSERT(mPages.size()>nPageNum-1,"SwPostitMgr:: page container size wrong");
            aRect = mPages[nPageNum-1]->bMarginSide ? Rectangle(Point(aPageFrm.Left()-GetSidebarWidth()-GetSidebarBorderWidth(),aPageFrm.Top()),Size(GetSidebarWidth(),aPageFrm.Height())) :
                            Rectangle( Point(aPageFrm.Right()+GetSidebarBorderWidth(),aPageFrm.Top()) , Size(GetSidebarWidth(),aPageFrm.Height()));
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
Rectangle SwPostItMgr::GetBottomScrollRect(const unsigned long aPage) const
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointBottom = mPages[aPage-1]->bMarginSide ? Point(aPageRect.Left() - GetSidebarWidth() - GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height()) :
                                    Point(aPageRect.Right() + GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height());
    Size aSize(GetSidebarWidth() - mpEditWin->PixelToLogic(Size(4,0)).Width(), mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height()) ;
    return Rectangle(aPointBottom,aSize);

}

Rectangle SwPostItMgr::GetTopScrollRect(const unsigned long aPage) const
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointTop = mPages[aPage-1]->bMarginSide ?    Point(aPageRect.Left() - GetSidebarWidth() -GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height()) :
                                        Point(aPageRect.Right() + GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height());
    Size aSize(GetSidebarWidth() - mpEditWin->PixelToLogic(Size(4,0)).Width(), mpEditWin->PixelToLogic(Size(0,GetSidebarScrollerHeight())).Height()) ;
    return Rectangle(aPointTop,aSize);
}


//IMPORTANT: if you change the rects here, also change SwPageFrm::PaintNotesSidebar()
bool SwPostItMgr::ScrollbarHit(const unsigned long aPage,const Point &aPoint)
{
    SwRect aPageRect = mPages[aPage-1]->mPageRect;
    Point aPointBottom = mPages[aPage-1]->bMarginSide ? Point(aPageRect.Left() - GetSidebarWidth()-GetSidebarBorderWidth() + mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height()) :
                                    Point(aPageRect.Right() + GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Bottom()- mpEditWin->PixelToLogic(Size(0,2+GetSidebarScrollerHeight())).Height());

    Point aPointTop = mPages[aPage-1]->bMarginSide ?    Point(aPageRect.Left() - GetSidebarWidth()-GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height()) :
                                Point(aPageRect.Right()+GetSidebarBorderWidth()+ mpEditWin->PixelToLogic(Size(2,0)).Width(),aPageRect.Top() + mpEditWin->PixelToLogic(Size(0,2)).Height());

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
    else
    if (aRectTop.IsInside(aPoint))
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
   if ( mbWaitingForCalcRects || mbLayouting || mvPostItFlds.empty() )
       return;

   // find first valid note
   SwMarginWin *pFirstPostIt = 0;
   for(SwMarginItem_iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
   {
       pFirstPostIt = (*i)->pPostIt;
       if (pFirstPostIt)
            break;
   }

   //if we have not found a valid note, forget about it and leave
   if (!pFirstPostIt)
       return;

   // yeah, I know, if this is a left page it could be wrong, but finding the page and the note is probably not even faster than just doing it
   const long aAnkorX = mpEditWin->LogicToPixel( Point((long)(pFirstPostIt->Ankor()->GetSixthPosition().getX()),0)).X();
   const long aAnkorY = mpEditWin->LogicToPixel( Point(0,(long)(pFirstPostIt->Ankor()->GetSixthPosition().getY()))).Y() + 1;
   if (Point(aAnkorX,aAnkorY) != pFirstPostIt->GetPosPixel())
   {
       long aAnkorPosX = 0;
       long aAnkorPosY = 0;
       for (unsigned long n=0;n<mPages.size();n++)
       {
           for(SwMarginItem_iterator i = mPages[n]->mList->begin(); i!= mPages[n]->mList->end(); i++)
           {
               if ((*i)->bShow && (*i)->pPostIt)
               {
                    aAnkorPosX = mPages[n]->bMarginSide ?
                        mpEditWin->LogicToPixel( Point((long)((*i)->pPostIt->Ankor()->GetSeventhPosition().getX()),0)).X() :
                        mpEditWin->LogicToPixel( Point((long)((*i)->pPostIt->Ankor()->GetSixthPosition().getX()),0)).X();
                    aAnkorPosY = mpEditWin->LogicToPixel( Point(0,(long)((*i)->pPostIt->Ankor()->GetSixthPosition().getY()))).Y() + 1;
                    (*i)->pPostIt->SetPosPixel(Point(aAnkorPosX,aAnkorPosY));
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
    return !mvPostItFlds.empty();
}

unsigned long SwPostItMgr::GetSidebarWidth(bool bPx) const
{
    unsigned long aWidth = (unsigned long)(mpWrtShell->GetViewOptions()->GetZoom() * 1.8);
    if (bPx)
        return aWidth;
    else
        return mpEditWin->PixelToLogic(Size( aWidth ,0)).Width();
}

unsigned long SwPostItMgr::GetSidebarBorderWidth(bool bPx) const
{
    if (bPx)
        return 2;
    else
        return mpEditWin->PixelToLogic(Size(2,0)).Width();
}

unsigned long SwPostItMgr::GetNoteWidth()
{
    return GetSidebarWidth(true);
}

Color SwPostItMgr::GetColorDark(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayNormal[] = {
            COL_AUTHOR1_NORMAL,     COL_AUTHOR2_NORMAL,     COL_AUTHOR3_NORMAL,
            COL_AUTHOR4_NORMAL,     COL_AUTHOR5_NORMAL,     COL_AUTHOR6_NORMAL,
            COL_AUTHOR7_NORMAL,     COL_AUTHOR8_NORMAL,     COL_AUTHOR9_NORMAL };

        return Color( aArrayNormal[ aAuthorIndex % (sizeof( aArrayNormal )/ sizeof( aArrayNormal[0] ))]);
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

        return Color( aArrayLight[ aAuthorIndex % (sizeof( aArrayLight )/ sizeof( aArrayLight[0] ))]);
    }
    else
        return Color(COL_WHITE);
}

Color SwPostItMgr::GetColorAnkor(sal_uInt16 aAuthorIndex)
{
    if (!Application::GetSettings().GetStyleSettings().GetHighContrastMode())
    {
        static const Color aArrayAnkor[] = {
            COL_AUTHOR1_DARK,       COL_AUTHOR2_DARK,       COL_AUTHOR3_DARK,
            COL_AUTHOR4_DARK,       COL_AUTHOR5_DARK,       COL_AUTHOR6_DARK,
            COL_AUTHOR7_DARK,       COL_AUTHOR8_DARK,       COL_AUTHOR9_DARK };

        return Color( aArrayAnkor[  aAuthorIndex % (sizeof( aArrayAnkor )   / sizeof( aArrayAnkor[0] ))]);
    }
    else
        return Color(COL_WHITE);
}

void SwPostItMgr::SetActivePostIt( SwMarginWin* p)
{
    if ( p != mpActivePostIt )
    {
        // we need the temp variable so we can set mpActivePostIt before we call DeactivatePostIt
        // therefore we get a new layout in DOCCHANGED when switching from postit to document,
        // otherwise, GetActivePostIt() would still hold our old postit
        SwMarginWin* pActive = mpActivePostIt;
        mpActivePostIt = p;
        if (pActive)
        {
            pActive->DeactivatePostIt();
            mShadowState.mpShadowFld = 0;
        }
        if (mpActivePostIt)
        {
            mpActivePostIt->GotoPos();
            mpView->AttrChangedNotify(0);
            mpActivePostIt->ActivatePostIt();
        }
    }
}

IMPL_LINK( SwPostItMgr, CalcHdl, void*, /* pVoid*/  )
{
    mnEventId = 0;
    if ( mbLayouting )
    {
        DBG_ERROR("Reentrance problem in Layout Manager!");
        mbWaitingForCalcRects = false;
        return 0;
    }

    // do not change order, even if it would seem so in the first place, we need the calcrects always
    if (CalcRects() || mbLayout)
    {
        mbLayout = false;
        LayoutPostIts();
    }
    return 0;
}

void SwPostItMgr::Rescale()
{
    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        if ( (*i)->pPostIt )
            (*i)->pPostIt->Rescale();
}

sal_Int32 SwPostItMgr::GetInitialAnchorDistance() const
{
    const Fraction& f( mpEditWin->GetMapMode().GetScaleY() );
    return POSTIT_INITIAL_ANKOR_DISTANCE * f.GetNumerator() / f.GetDenominator();
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
    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        if ( (*i)->pPostIt )
            (*i)->pPostIt->SetSpellChecking();
}

void SwPostItMgr::SetReadOnlyState()
{
    for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
        if ( (*i)->pPostIt )
            (*i)->pPostIt->SetReadonly( mbReadOnly );
}

void SwPostItMgr::CheckMetaText()
{
        for(std::list<SwMarginItem*>::iterator i = mvPostItFlds.begin(); i!= mvPostItFlds.end() ; i++)
                if ( (*i)->pPostIt )
                       (*i)->pPostIt->CheckMetaText();

}

sal_uInt16 SwPostItMgr::Replace(SvxSearchItem* pItem)
{
    SwMarginWin* pWin = GetActivePostIt();
    sal_uInt16 aResult = pWin->View()->StartSearchAndReplace( *pItem );
    if (!aResult)
        SetActivePostIt(0);
    return aResult;
}

sal_uInt16 SwPostItMgr::FinishSearchReplace(const ::com::sun::star::util::SearchOptions& rSearchOptions, bool bSrchForward)
{
    SwMarginWin* pWin = GetActivePostIt();
    SvxSearchItem aItem(SID_SEARCH_ITEM );
    aItem.SetSearchOptions(rSearchOptions);
    aItem.SetBackward(!bSrchForward);
    sal_uInt16 aResult = pWin->View()->StartSearchAndReplace( aItem );
    if (!aResult)
        SetActivePostIt(0);
    return aResult;
}

sal_uInt16 SwPostItMgr::SearchReplace(const SwFmtFld &pFld, const ::com::sun::star::util::SearchOptions& rSearchOptions, bool bSrchForward)
{
    sal_uInt16 aResult = 0;
    SwMarginWin* pWin = GetPostIt(&pFld);
    if (pWin)
    {
        ESelection aOldSelection = pWin->View()->GetSelection();
        if (bSrchForward)
            pWin->View()->SetSelection(ESelection(0,0,0,0));
        else
            pWin->View()->SetSelection(ESelection(0xFFFF,0xFFFF,0xFFFF,0xFFFF));
        SvxSearchItem aItem(SID_SEARCH_ITEM );
        aItem.SetSearchOptions(rSearchOptions);
        aItem.SetBackward(!bSrchForward);
        aResult = pWin->View()->StartSearchAndReplace( aItem );
        if (!aResult)
            pWin->View()->SetSelection(aOldSelection);
        else
        {
            SetActivePostIt(pWin);
            MakeVisible(pWin);
        }
    }
    return aResult;
}

void SwPostItMgr::AssureStdModeAtShell()
{
    //#i103373# #i103645#
        // deselect any drawing or frame and leave editing mode
          SdrView* pSdrView = mpWrtShell->GetDrawView();
        if ( pSdrView && pSdrView->IsTextEdit() )
        {
            sal_Bool bLockView = mpWrtShell->IsViewLocked();
               mpWrtShell->LockView( sal_True );
            mpWrtShell->EndTextEdit();
                mpWrtShell->LockView( bLockView );
        }

        if( mpWrtShell->IsSelFrmMode() || mpWrtShell->IsObjSelected())
        {
                mpWrtShell->UnSelectFrm();
                mpWrtShell->LeaveSelFrmMode();
                mpWrtShell->EnterStdMode();

                mpWrtShell->DrawSelChanged();
                mpView->StopShellTimer();
        }
}
