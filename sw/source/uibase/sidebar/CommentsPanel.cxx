/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>
#include <sfx2/bindings.hxx>

#include <PostItMgr.hxx>
#include <postithelper.hxx>
#include <AnnotationWin.hxx>
#include <fmtfld.hxx>
#include <docufld.hxx>
#include <txtfld.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <MarkManager.hxx>
#include <ndtxt.hxx>
#include <swmodule.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustring.hxx>
#include <xmloff/xmlmetae.hxx>
#include <tools/datetimeutils.hxx>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <tools/date.hxx>
#include <tools/datetime.hxx>
#include <tools/time.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <tools/link.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editeng.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/event.hxx>

#include <strings.hrc>
#include <cmdid.h>

#include "CommentsPanel.hxx"
#include <annotationmark.hxx>
#include <pam.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{
Comment::Comment(weld::Container* pParent, CommentsPanel& rCommentsPanel)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/swriter/ui/commentwidget.ui"))
    , mxContainer(mxBuilder->weld_container("Comment"))
    , mxExpander(mxBuilder->weld_expander("expander"))
    , mxAuthor(mxBuilder->weld_label("authorlabel"))
    , mxDate(mxBuilder->weld_label("datelabel"))
    , mxTime(mxBuilder->weld_label("timelabel"))
    , mxReply(mxBuilder->weld_button("replybutton"))
    , mxResolve(mxBuilder->weld_check_button("resolvebutton"))
    , mxTextView(mxBuilder->weld_text_view("textview"))
    , mrCommentsPanel(rCommentsPanel)
    , maDate(Date::EMPTY)
    , maTime(tools::Time::EMPTY)
    , mbResolved(false)
{
    mxTextView->set_editable(false);
    mxTextView->set_tooltip_text(SwResId(STR_COMMENT_VIEW_MODE));
    mxTextView->connect_focus_out(LINK(this, Comment, OnFocusOut));
    mxResolve->connect_toggled(LINK(this, Comment, ResolveClicked));
    mxReply->connect_clicked(LINK(this, Comment, ReplyClicked));
    mxExpander->connect_mouse_press(LINK(this, Comment, ContextMenuHdl));
}

IMPL_LINK(Comment, ContextMenuHdl, const MouseEvent&, rMEvt, bool)
{
    if (rMEvt.IsRight())
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(
            mxExpander.get(), u"modules/swriter/ui/commentcontextmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xPopMenu(xBuilder->weld_menu(u"contextmenu"_ustr));
        Point aPos = rMEvt.GetPosPixel();
        OUString sId
            = xPopMenu->popup_at_rect(mxExpander.get(), tools::Rectangle(aPos, Size(1, 1)));

        if (sId == "edit")
        {
            this->makeEditable();
            getTextView()->set_tooltip_text(SwResId(STR_COMMENT_EDIT_MODE));
        }
        else if (sId == "reply")
            mrCommentsPanel.ReplyComment(this);
        else if (sId == "delete")
            mrCommentsPanel.DeleteComment(this);
        else if (sId == "toggle_resolved")
            mrCommentsPanel.ToggleResolved(this);
        else if (sId == "delete_thread")
            mrCommentsPanel.DeleteThread(this);
        else if (sId == "resolve_thread")
            mrCommentsPanel.ResolveThread(this);
        return true;
    }
    return false;
}

OUString CommentsPanel::getReferenceText(const SwTextNode* pTextNode,
                                         const sw::mark::AnnotationMark* pMark)
{
    if (!pMark)
        return OUString();
    const SwPosition& rStart = pMark->GetMarkStart();
    const SwPosition& rEnd = pMark->GetMarkEnd();
    OUString sReferenceText = pTextNode->GetText().copy(
        rStart.GetContentIndex(), rEnd.GetContentIndex() - rStart.GetContentIndex() - 1);
    return sReferenceText;
}

Comment::~Comment() {}

void Comment::InitControls(const SwPostItField* pPostItField)
{
    if (!pPostItField)
        return;
    msText = pPostItField->GetText();
    msAuthor = pPostItField->GetPar1();
    maDate = Date(pPostItField->GetDateTime());
    maTime = tools::Time(pPostItField->GetDateTime());
    mbResolved = pPostItField->GetResolved();

    OUString sDate = sw::sidebar::CommentsPanel::FormatDate(maDate);
    OUString sTime = sw::sidebar::CommentsPanel::FormatTime(maTime);
    if (mxDate->get_label() != sDate)
    {
        mxDate->set_label(sDate);
    }
    if (mxTime->get_label() != sTime)
    {
        mxTime->set_label(sTime);
    }

    mxAuthor->set_label(msAuthor);
    mxAuthor->set_tooltip_text(msAuthor);
    mxResolve->set_active(mbResolved);
    mxTextView->set_text(msText);
}

IMPL_LINK_NOARG(Comment, OnFocusOut, weld::Widget&, void)
{
    mrCommentsPanel.EditComment(this);
    getTextView()->set_tooltip_text(SwResId(STR_COMMENT_VIEW_MODE));
}

IMPL_LINK_NOARG(Comment, ResolveClicked, weld::Toggleable&, void)
{
    mrCommentsPanel.ToggleResolved(this);
    mrCommentsPanel.ShowResolvedHdl(mrCommentsPanel.getShowResolved());
}

IMPL_LINK_NOARG(Comment, ReplyClicked, weld::Button&, void) { mrCommentsPanel.ReplyComment(this); }

Thread::Thread(weld::Container* pParent)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/swriter/ui/commentsthread.ui"))
    , mxContainer(mxBuilder->weld_container("Thread"))
    , mxExpander(mxBuilder->weld_expander("expander"))
    , mxCommentBox(mxBuilder->weld_box("comments_box"))
{
    // mxContainer->set_size_request(-1, mxContainer->get_preferred_size().Height());
}

Thread::~Thread() {}

std::unique_ptr<PanelLayout> CommentsPanel::Create(weld::Widget* pParent)
{
    if (pParent == nullptr)
        throw css::lang::IllegalArgumentException("no parent window given to CommentsPanel::Create",
                                                  nullptr, 0);
    return std::make_unique<CommentsPanel>(pParent);
}

CommentsPanel::CommentsPanel(weld::Widget* pParent)
    : PanelLayout(pParent, "CommentsPanel", "modules/swriter/ui/commentspanel.ui")
    , mpDoc(nullptr)
    , mpPostItMgr(nullptr)
    , mxFilterAuthor(m_xBuilder->weld_combo_box("filter_author"))
    , mxFilterDate(new SvtCalendarBox(m_xBuilder->weld_menu_button("filter_date"), true))
    , mxResetDate(m_xBuilder->weld_button("reset"))
    , mxShowTime(m_xBuilder->weld_check_button("show_time"))
    , mxShowResolved(m_xBuilder->weld_check_button("show_resolved"))
    , mxShowReference(m_xBuilder->weld_check_button("show_reference"))
    , mxSortbyPosition(m_xBuilder->weld_radio_button("sortby_position"))
    , mxSortbyTime(m_xBuilder->weld_radio_button("sortby_time"))
    , mxThreadsContainer(m_xBuilder->weld_box("comment_threads"))
{
    mxFilterAuthor->connect_changed(LINK(this, CommentsPanel, FilterByAuthor));
    mxFilterDate->connect_activated(LINK(this, CommentsPanel, FilterByDate));
    mxResetDate->connect_clicked(LINK(this, CommentsPanel, ResetDate));
    mxShowTime->connect_toggled(LINK(this, CommentsPanel, ShowTimeHdl));
    mxShowResolved->connect_toggled(LINK(this, CommentsPanel, ShowResolvedHdl));
    mxSortbyPosition->connect_toggled(LINK(this, CommentsPanel, SortHdl));
    mxSortbyTime->connect_toggled(LINK(this, CommentsPanel, SortHdl));

    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rSh = pView->GetWrtShell();
    mpPostItMgr = rSh.GetPostItMgr();
    mpDoc = rSh.GetDoc();
    populateComments();

    StartListening(*mpPostItMgr);
}

IMPL_LINK_NOARG(CommentsPanel, SortHdl, weld::Toggleable&, void) { populateComments(); }

void CommentsPanel::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::SwFormatField)
    {
        const SwFormatFieldHint* pFormatHint = static_cast<const SwFormatFieldHint*>(&rHint);
        const SwFormatField* pField = pFormatHint->GetField();
        switch (pFormatHint->Which())
        {
            case SwFormatFieldHintWhich::INSERTED:
            {
                if (!pField)
                {
                    break;
                }
                // get field to be inserted from hint
                if (pField->IsFieldInDoc())
                {
                    addComment(pField);
                }
                else
                {
                    OSL_FAIL("Inserted field not in document!");
                }
                break;
            }
            case SwFormatFieldHintWhich::REMOVED:
            case SwFormatFieldHintWhich::REDLINED_DELETION:
            {
                sw::annotation::SwAnnotationWin* pAnnotationWin
                    = mpPostItMgr->GetRemovedAnnotationWin(pField);
                sal_uInt32 nId = getPostItId(pAnnotationWin);
                deleteComment(nId);
                break;
            }
            case SwFormatFieldHintWhich::FOCUS:
            {
                break;
            }
            case SwFormatFieldHintWhich::CHANGED:
            case SwFormatFieldHintWhich::RESOLVED:
            {
                SwFormatField* pFormatField = dynamic_cast<SwFormatField*>(&rBC);
                SwPostItField* pPostItField = static_cast<SwPostItField*>(pFormatField->GetField());
                sw::annotation::SwAnnotationWin* pAnnotationWin
                    = mpPostItMgr->GetAnnotationWin(pPostItField);
                setResolvedStatus(pAnnotationWin);
                break;
            }
        }
    }
}

OUString CommentsPanel::FormatDate(const Date& rDate)
{
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    OUString sMeta;
    if (rDate.IsValidAndGregorian())
    {
        sMeta = rLocalData.getDate(rDate);
    }
    else
    {
        sMeta = SwResId(STR_NODATE);
    }
    return sMeta;
}

OUString CommentsPanel::FormatTime(const tools::Time& rTime)
{
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    OUString sMeta;
    if (rTime.GetTime() != 0)
    {
        sMeta = " " + rLocalData.getTime(rTime, false);
    }
    return sMeta;
}

sw::annotation::SwAnnotationWin* CommentsPanel::getRootCommentWin(const SwFormatField* pFormatField)
{
    if (!pFormatField)
        return nullptr;
    const SwPostItField* pPostItField = static_cast<const SwPostItField*>(pFormatField->GetField());
    sw::annotation::SwAnnotationWin* pAnnotationWin = mpPostItMgr->GetAnnotationWin(pPostItField);
    if (!pAnnotationWin)
        return nullptr;
    sw::annotation::SwAnnotationWin* pRootNote = pAnnotationWin->GetTopReplyNote();
    return pRootNote;
}

sal_uInt32 CommentsPanel::getPostItId(const sw::annotation::SwAnnotationWin* pAnnotationWin)
{
    const SwPostItField* pField = pAnnotationWin->GetPostItField();
    return pField->GetPostItId();
}

sw::annotation::SwAnnotationWin* CommentsPanel::getAnnotationWin(const Comment* pComment)
{
    sal_uInt32 nPostItId = 0;
    for (auto & [ rId, rComment ] : mpCommentsMap)
    {
        if (rComment.get() != pComment)
        {
            continue;
        }
        nPostItId = rId;
        break;
    }
    auto& vPostItFields = mpPostItMgr->GetPostItFields();
    SwPostItField* pPostItField = nullptr;
    for (auto& pItem : vPostItFields)
    {
        SwFormatField* pFormatField = &pItem->GetFormatField();
        SwPostItField* pField = static_cast<SwPostItField*>(pFormatField->GetField());
        if (pField->GetPostItId() == nPostItId)
        {
            pPostItField = pField;
            break;
        }
    }
    return mpPostItMgr->GetAnnotationWin(pPostItField);
}

bool CommentsPanel::comp_dateTime(SwFormatField* a, SwFormatField* b)
{
    SwPostItField* pA = static_cast<SwPostItField*>(a->GetField());
    SwPostItField* pB = static_cast<SwPostItField*>(b->GetField());

    Date aDateA(pA->GetDateTime());
    tools::Time aTimeA(pA->GetDateTime());
    Date aDateB(pB->GetDateTime());
    tools::Time aTimeB(pB->GetDateTime());

    OUString sDateTimeA = FormatTime(aTimeA) + " " + FormatDate(aDateA);
    OUString sDateTimeB = FormatTime(aTimeB) + " " + FormatDate(aDateB);

    return sDateTimeA > sDateTimeB;
}

SwPosition CommentsPanel::getAnchorPosition(SwFormatField* pField)
{
    SwTextField* pTextField = pField->GetTextField();
    SwTextNode* pTextNode = pTextField->GetpTextNode();

    SwPosition aPos(*pTextNode, pTextField->GetStart());
    return aPos;
}

bool CommentsPanel::comp_position(SwFormatField* a, SwFormatField* b)
{
    SwPosition aPosA = getAnchorPosition(a);
    SwPosition aPosB = getAnchorPosition(b);

    return aPosA < aPosB;
}

void CommentsPanel::populateComments()
{
    if (!mpCommentsMap.empty())
    {
        for (auto it = mpCommentsMap.begin(); it != mpCommentsMap.end();)
        {
            sal_uInt32 nId = it->first;
            it++;
            deleteComment(nId);
        }
    }

    if (!mpPostItMgr)
        return;
    std::vector<SwFormatField*> vFormatFields = mpPostItMgr->UpdatePostItsParentInfo();
    if (mxSortbyTime->get_active())
    {
        std::sort(vFormatFields.begin(), vFormatFields.end(),
                  [](SwFormatField* a, SwFormatField* b) {
                      return sw::sidebar::CommentsPanel::comp_dateTime(a, b);
                  });
    }
    else
    {
        std::stable_sort(vFormatFields.begin(), vFormatFields.end(),
                         [](SwFormatField* a, SwFormatField* b) {
                             return sw::sidebar::CommentsPanel::comp_position(a, b);
                         });
    }

    for (auto pFormatField : vFormatFields)
    {
        sw::annotation::SwAnnotationWin* pRootNote = getRootCommentWin(pFormatField);
        if (!pRootNote)
            continue;
        sal_uInt32 nRootId = getPostItId(pRootNote);

        if (mpThreadsMap.contains(nRootId))
        {
            if (mxSortbyPosition->get_active())
                continue;
            else
            {
                auto pThread = mpThreadsMap[nRootId].get();
                SwPostItField* pPostItField = static_cast<SwPostItField*>(pFormatField->GetField());
                sal_uInt32 nId = pPostItField->GetPostItId();
                auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
                pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                              pThread->mnComments++);
                pComment->InitControls(pPostItField);
                mpAuthorSet.insert(pComment->GetAuthor());
                mpCommentsMap[nId] = std::move(pComment);
                continue;
            }
        }

        auto pThread = std::make_unique<Thread>(mxThreadsContainer.get());
        mxThreadsContainer->reorder_child(pThread->get_widget(), mnThreads++);

        if (mxSortbyPosition->get_active())
        {
            for (sw::annotation::SwAnnotationWin* pCurrent = pRootNote;;)
            {
                sal_uInt32 nId = getPostItId(pCurrent);
                auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
                pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                              pThread->mnComments++);
                pComment->InitControls(pCurrent->GetPostItField());
                mpAuthorSet.insert(pComment->GetAuthor());
                mpCommentsMap[nId] = std::move(pComment);
                sw::annotation::SwAnnotationWin* next
                    = mpPostItMgr->GetNextPostIt(KEY_PAGEDOWN, pCurrent);
                if (!next || next->GetTopReplyNote() != pRootNote)
                    break;
                pCurrent = next;
            }
        }
        else
        {
            SwPostItField* pPostItField = static_cast<SwPostItField*>(pFormatField->GetField());
            sal_uInt32 nId = pPostItField->GetPostItId();
            auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
            pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                          pThread->mnComments++);
            pComment->InitControls(pPostItField);
            mpAuthorSet.insert(pComment->GetAuthor());
            mpCommentsMap[nId] = std::move(pComment);
        }
        mpThreadsMap[nRootId] = std::move(pThread);
        setReferenceText(nRootId);
    }
    populateAuthorComboBox();
}

void CommentsPanel::addComment(const SwFormatField* pField)
{
    // Get id of the note
    const SwPostItField* pPostItField = static_cast<const SwPostItField*>(pField->GetField());
    sal_uInt32 nNoteId = pPostItField->GetPostItId();

    if (mpCommentsMap.contains(nNoteId))
        return;

    // Get id of the root note
    sw::annotation::SwAnnotationWin* pRootNote = getRootCommentWin(pField);
    if (!pRootNote)
        return;
    sal_uInt32 nRootId = getPostItId(pRootNote);

    sw::annotation::SwAnnotationWin* pNote
        = mpPostItMgr->GetAnnotationWin(static_cast<const SwPostItField*>(pField->GetField()));
    // If comment is added to an existing thread
    if (mpThreadsMap.contains(nRootId))
    {
        auto& pThread = mpThreadsMap[nRootId];
        auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
        pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                      pThread->mnComments++);
        pComment->InitControls(pNote->GetPostItField());
        pComment->getTextView()->set_tooltip_text(SwResId(STR_COMMENT_EDIT_MODE));
        mpAuthorSet.insert(pComment->GetAuthor());
        mpCommentsMap[nNoteId] = std::move(pComment);
    }
    // If a new thread is created
    else
    {
        auto pThread = std::make_unique<Thread>(mxThreadsContainer.get());
        mxThreadsContainer->reorder_child(pThread->get_widget(), mnThreads++);
        auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
        pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                      pThread->mnComments++);
        mpThreadsMap[nRootId] = std::move(pThread);
        setReferenceText(nRootId);
        pComment->InitControls(pNote->GetPostItField());
        pComment->getTextView()->set_tooltip_text(SwResId(STR_COMMENT_EDIT_MODE));
        mpAuthorSet.insert(pComment->GetAuthor());
        mpCommentsMap[nNoteId] = std::move(pComment);
    }
    populateComments();
}

void CommentsPanel::deleteComment(sal_uInt32 nId)
{
    sw::annotation::SwAnnotationWin* pAnnotationWin = getAnnotationWin(mpCommentsMap[nId].get());
    SwFormatField* pFormatField = pAnnotationWin->GetFormatField();
    sw::annotation::SwAnnotationWin* pRootNote = getRootCommentWin(pFormatField);
    // // If the root comment is deleted, the new root comment of the thread should be the next comment in the thread
    // // but due to a bug `getRootCommentWin` returns root comment of some other/random thread so we completely lose
    // // access to the current thread.
    // if (mpThreadsMap.contains(nId))
    // {
    //     pRootNote = mpThreadsMap[nId];
    // }
    // else
    // {
    //     pRootNote = getRootCommentWin(pFormatField);
    // }

    sal_uInt32 nRootId = getPostItId(pRootNote);

    if (!mpThreadsMap.contains(nRootId))
    {
        SAL_WARN("sw",
                 "Comments Panel is unable to delete comment: Referenced thread does not exist!");
        return;
    }
    auto& pComment = mpCommentsMap[nId];
    auto& pThread = mpThreadsMap[nRootId];
    if (!pComment)
    {
        SAL_WARN("sw",
                 "Comments Panel is unable to delete comment: Referenced comment does not exist!");
        return;
    }

    pThread->getCommentBoxWidget()->move(pComment->get_widget(), nullptr);
    mpCommentsMap.erase(nId);

    // If the last comment in the thread is deleted, delete the thread
    if (--pThread->mnComments == 0)
    {
        mxThreadsContainer->move(pThread->get_widget(), nullptr);
        if (mpThreadsMap.contains(nRootId))
            mpThreadsMap.erase(nRootId);
        mnThreads--;
    }
}

void CommentsPanel::setResolvedStatus(const sw::annotation::SwAnnotationWin* pAnnotationWin)
{
    sal_uInt32 nId = getPostItId(pAnnotationWin);
    if (!mpCommentsMap.contains(nId))
        return;
    auto& pComment = mpCommentsMap[nId];
    if (!pComment)
        return;
    SwPostItField* pPostItField = const_cast<SwPostItField*>(pAnnotationWin->GetPostItField());
    if (pPostItField->GetResolved() == pComment->mbResolved)
    {
        editComment(pPostItField, pComment.get());
        return;
    }
    pComment->mbResolved = pPostItField->GetResolved();
    pComment->mxResolve->set_active(pComment->mbResolved);
}

void CommentsPanel::editComment(const SwPostItField* pPostItField, Comment* pComment)
{
    const OUString& sText = pPostItField->GetText();
    pComment->SetCommentText(sText);
    pComment->mxTextView->set_text(sText);
}

void CommentsPanel::setReferenceText(sal_uInt32 nRootId)
{
    Thread* pThread = mpThreadsMap[nRootId].get();
    Comment* pRootComment = mpCommentsMap[nRootId].get();
    sw::annotation::SwAnnotationWin* pRootNote = getAnnotationWin(pRootComment);
    SwFormatField* pField = pRootNote->GetFormatField();
    const SwPosition pAnchor = getAnchorPosition(pField);
    SwNodeIndex aIdx(pAnchor.nNode);
    SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
    sw::mark::MarkManager& rMarkManager = mpDoc->GetMarkManager();
    sw::mark::AnnotationMark* pMark = rMarkManager.getAnnotationMarkFor(pAnchor);
    OUString sText = getReferenceText(pTextNode, pMark);
    pThread->getExpander()->set_label(sText);
}

void CommentsPanel::EditComment(const Comment* pComment)
{
    if (!pComment)
        return;
    if (!pComment->mxTextView->get_editable())
        return;
    const OUString sText = pComment->mxTextView->get_text();

    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    Outliner* pOutliner = pWin->GetOutliner();
    pOutliner->Clear();
    pOutliner->SetText(sText, pOutliner->GetParagraph(0));
    pComment->mxTextView->set_editable(false);
}

void CommentsPanel::ToggleResolved(const Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ToggleResolved();
}

void CommentsPanel::ReplyComment(const Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ExecuteCommand(FN_REPLY);
}

void CommentsPanel::DeleteComment(const Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ExecuteCommand(FN_DELETE_COMMENT);
}

void CommentsPanel::DeleteThread(const Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ExecuteCommand(FN_DELETE_COMMENT_THREAD);
}

void CommentsPanel::ResolveThread(const Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ExecuteCommand(FN_RESOLVE_NOTE_THREAD);
}

void CommentsPanel::populateAuthorComboBox()
{
    mxFilterAuthor->clear();
    if (mpAuthorSet.empty())
        return;
    mxFilterAuthor->append_text("All");
    for (const OUString& rAuthor : mpAuthorSet)
    {
        mxFilterAuthor->append_text(rAuthor);
    }
    mxFilterAuthor->set_active_text("All");
}

IMPL_LINK_NOARG(CommentsPanel, FilterByAuthor, weld::ComboBox&, void)
{
    OUString sAuthor = mxFilterAuthor->get_active_text();
    if (sAuthor == "All")
    {
        for (auto & [ nId, pComment ] : mpCommentsMap)
        {
            if (mbDateSelected && mxFilterDate->get_date() != pComment->GetDate())
                continue;
            pComment->get_widget()->set_visible(true);
        }
    }
    else
    {
        for (auto & [ nId, pComment ] : mpCommentsMap)
        {
            if (sAuthor == pComment->GetAuthor())
            {
                if (mbDateSelected && mxFilterDate->get_date() != pComment->GetDate())
                    continue;
                pComment->get_widget()->set_visible(true);
            }
            else
            {
                pComment->get_widget()->set_visible(false);
            }
        }
    }
}

IMPL_LINK_NOARG(CommentsPanel, FilterByDate, SvtCalendarBox&, void)
{
    mbDateSelected = true;
    Date aDate(mxFilterDate->get_date());
    for (auto & [ nId, pComment ] : mpCommentsMap)
    {
        if (aDate == pComment->GetDate())
        {
            pComment->get_widget()->set_visible(true);
        }
        else
        {
            pComment->get_widget()->set_visible(false);
        }
    }
    FilterByAuthor(*mxFilterAuthor);
}

IMPL_LINK_NOARG(CommentsPanel, ResetDate, weld::Button&, void)
{
    mbDateSelected = false;
    FilterByAuthor(*mxFilterAuthor);
}

IMPL_LINK_NOARG(CommentsPanel, ShowTimeHdl, weld::Toggleable&, void)
{
    bool bShowTime = mxShowTime->get_active();
    for (auto & [ rId, pComment ] : mpCommentsMap)
    {
        pComment->mxTime->set_visible(bShowTime);
    }
}

IMPL_LINK_NOARG(CommentsPanel, ShowResolvedHdl, weld::Toggleable&, void)
{
    bool bShowResolved = mxShowResolved->get_active();
    for (auto & [ rId, pComment ] : mpCommentsMap)
    {
        if (pComment->mxResolve->get_active())
        {
            pComment->get_widget()->set_visible(bShowResolved);
        }
    }
}

CommentsPanel::~CommentsPanel() {}

void CommentsPanel::NotifyItemUpdate(const sal_uInt16 /*nSid*/, const SfxItemState /* eState */,
                                     const SfxPoolItem* pState)
{
    if (!pState) //disposed
        return;
}
}
