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

#include <strings.hrc>
#include <cmdid.h>

#include "CommentsPanel.hxx"

#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace sw::sidebar
{
Comment::Comment(weld::Container* pParent, CommentsPanel& rCommentsPanel)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/swriter/ui/commentwidget.ui"))
    , mxContainer(mxBuilder->weld_container("Comment"))
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
    mxTextView->connect_focus_out(LINK(this, Comment, OnFocusOut));
    mxResolve->connect_toggled(LINK(this, Comment, ResolveClicked));
    mxReply->connect_clicked(LINK(this, Comment, ReplyClicked));
}

Comment::~Comment() {}

void Comment::InitControls(const SwPostItField* pPostItField)
{
    if (!pPostItField)
        return;
    msText = pPostItField->GetText();
    msAuthor = pPostItField->GetPar1();
    msInitials = pPostItField->GetInitials();
    msName = pPostItField->GetName();
    maDate = Date(pPostItField->GetDateTime().GetDate());
    maTime = tools::Time(pPostItField->GetDateTime().GetTime());
    mbResolved = pPostItField->GetResolved();

    // Format date and time according to the system locale
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocalData = aSysLocale.GetLocaleData();
    OUString sMeta;
    if (maDate.IsValidAndGregorian())
    {
        sMeta = rLocalData.getDate(maDate);
    }
    else
    {
        sMeta = SwResId(STR_NODATE);
    }
    if (mxDate->get_label() != sMeta)
    {
        mxDate->set_label(sMeta);
    }
    if (pPostItField->GetTime().GetTime() != 0)
    {
        sMeta = " " + rLocalData.getTime(pPostItField->GetTime(), false);
    }
    if (mxTime->get_label() != sMeta)
    {
        mxTime->set_label(sMeta);
    }

    mxAuthor->set_label(msAuthor);
    mxAuthor->set_tooltip_text(msAuthor);
    mxResolve->set_active(mbResolved);
    mxTextView->set_text(msText);
}

IMPL_LINK_NOARG(Comment, OnFocusOut, weld::Widget&, void) { mrCommentsPanel.EditComment(this); }

IMPL_LINK_NOARG(Comment, ResolveClicked, weld::Toggleable&, void)
{
    mrCommentsPanel.ToggleResolved(this);
}

IMPL_LINK_NOARG(Comment, ReplyClicked, weld::Button&, void) { mrCommentsPanel.ReplyComment(this); }

Thread::Thread(weld::Container* pParent)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/swriter/ui/commentsthread.ui"))
    , mxContainer(mxBuilder->weld_container("Thread"))
    , mxCommentBox(mxBuilder->weld_box("comments_box"))
    , mxText(mxBuilder->weld_label("commentedtext"))
{
    // mxContainer->set_size_request(-1, mxContainer->get_preferred_size().Height());
}

Thread::~Thread() {}

std::unique_ptr<PanelLayout> CommentsPanel::Create(weld::Widget* pParent)
{
    if (pParent == nullptr)
        throw ::com::sun::star::lang::IllegalArgumentException(
            "no parent window given to CommentsPanel::Create", nullptr, 0);
    return std::make_unique<CommentsPanel>(pParent);
}

CommentsPanel::CommentsPanel(weld::Widget* pParent)
    : PanelLayout(pParent, "CommentsPanel", "modules/swriter/ui/commentspanel.ui")
    , mpPostItMgr(nullptr)
    , mxFilterAuthor(m_xBuilder->weld_combo_box("filter_author"))
    , mxFilterTime(m_xBuilder->weld_combo_box("filter_time"))
    , mxShowTime(m_xBuilder->weld_check_button("show_time"))
    , mxShowResolved(m_xBuilder->weld_check_button("show_resolved"))
    , mxShowReference(m_xBuilder->weld_check_button("show_reference"))
    , mxSortbyPosition(m_xBuilder->weld_radio_button("sortby_position"))
    , mxSortbyTime(m_xBuilder->weld_radio_button("sortby_time"))
    , mxThreadsContainer(m_xBuilder->weld_box("comment_threads"))
{
    SwView* pView = GetActiveView();
    if (!pView)
        return;
    SwWrtShell& rSh = pView->GetWrtShell();
    mpPostItMgr = rSh.GetPostItMgr();
    populateComments();

    StartListening(*mpPostItMgr);
}

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
                deleteComment(pAnnotationWin);
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

sal_uInt32 CommentsPanel::getPostItId(sw::annotation::SwAnnotationWin* pAnnotationWin)
{
    const SwPostItField* pField = pAnnotationWin->GetPostItField();
    return pField->GetPostItId();
}

sw::annotation::SwAnnotationWin* CommentsPanel::getAnnotationWin(Comment* pComment)
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

void CommentsPanel::populateComments()
{
    if (!mpPostItMgr)
        return;
    std::vector<SwFormatField*> vFormatFields = mpPostItMgr->UpdatePostItsParentInfo();

    for (auto pFormatField : vFormatFields)
    {
        sw::annotation::SwAnnotationWin* pRootNote = getRootCommentWin(pFormatField);
        if (!pRootNote)
            continue;
        sal_uInt32 nPostItId = getPostItId(pRootNote);

        if (mpThreadsMap.find(nPostItId) != mpThreadsMap.end())
            continue; // Skip if root comment is already present

        auto pThread = std::make_unique<Thread>(mxThreadsContainer.get());
        mxThreadsContainer->reorder_child(pThread->get_widget(), mnThreads++);

        for (sw::annotation::SwAnnotationWin* pCurrent = pRootNote;;)
        {
            sal_uInt32 nId = getPostItId(pCurrent);
            auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
            pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                          pThread->mnComments++);
            pComment->InitControls(pCurrent->GetPostItField());
            mpCommentsMap[nId] = std::move(pComment);
            sw::annotation::SwAnnotationWin* next
                = mpPostItMgr->GetNextPostIt(KEY_PAGEDOWN, pCurrent);
            if (!next || next->GetTopReplyNote() != pRootNote)
                break;
            pCurrent = next;
        }
        mpThreadsMap[nPostItId] = std::move(pThread);
    }
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
    if (mpThreadsMap.find(nRootId) != mpThreadsMap.end())
    {
        auto& pThread = mpThreadsMap[nRootId];
        auto pComment = std::make_unique<Comment>(pThread->getCommentBoxWidget(), *this);
        pThread->getCommentBoxWidget()->reorder_child(pComment->get_widget(),
                                                      pThread->mnComments++);
        pComment->InitControls(pNote->GetPostItField());
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
        pComment->InitControls(pNote->GetPostItField());
        mpCommentsMap[nNoteId] = std::move(pComment);
    }
}

void CommentsPanel::deleteComment(sw::annotation::SwAnnotationWin* pAnnotationWin)
{
    if (!pAnnotationWin)
        return;
    sal_uInt32 nId = getPostItId(pAnnotationWin);
    SwFormatField* pFormatField = pAnnotationWin->GetFormatField();
    sw::annotation::SwAnnotationWin* pRootNote = nullptr;
    // If the root comment is deleted, the new root comment of the thread should be the next comment in the thread
    // but due to a bug `getRootCommentWin` returns root comment of some other/random thread so we completely lose
    // access to the current thread.
    if (mpThreadsMap.find(nId) != mpThreadsMap.end())
    {
        pRootNote = pAnnotationWin;
    }
    else
    {
        pRootNote = getRootCommentWin(pFormatField);
    }

    sal_uInt32 nRootId = getPostItId(pRootNote);

    if (mpThreadsMap.find(nRootId) == mpThreadsMap.end())
        return;
    auto& pComment = mpCommentsMap[nId];
    auto& pThread = mpThreadsMap[nRootId];
    if (!pComment)
        return;

    pThread->getCommentBoxWidget()->move(pComment->get_widget(), nullptr);
    mpCommentsMap.erase(nId);

    // If the last comment in the thread is deleted, delete the thread
    if (--pThread->mnComments == 0)
    {
        mxThreadsContainer->move(pThread->get_widget(), nullptr);
        if (mpThreadsMap.find(nRootId) != mpThreadsMap.end())
            mpThreadsMap.erase(nRootId);
    }
}

void CommentsPanel::setResolvedStatus(sw::annotation::SwAnnotationWin* pAnnotationWin)
{
    sal_uInt32 nId = getPostItId(pAnnotationWin);
    if (mpCommentsMap.find(nId) == mpCommentsMap.end())
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

void CommentsPanel::editComment(SwPostItField* pPostItField, Comment* pComment)
{
    OUString sText = pPostItField->GetText();
    pComment->SetCommentText(sText);
    pComment->mxTextView->set_text(sText);
}

void CommentsPanel::EditComment(Comment* pComment)
{
    if (!pComment)
        return;
    const OUString sText = pComment->mxTextView->get_text();

    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    Outliner* pOutliner = pWin->GetOutliner();
    pOutliner->Clear();
    pOutliner->SetText(sText, pOutliner->GetParagraph(0));
}

void CommentsPanel::ToggleResolved(Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ToggleResolved();
}

void CommentsPanel::ReplyComment(Comment* pComment)
{
    if (!pComment)
        return;
    sw::annotation::SwAnnotationWin* pWin = getAnnotationWin(pComment);
    pWin->ExecuteCommand(FN_REPLY);
}

CommentsPanel::~CommentsPanel() {}

void CommentsPanel::NotifyItemUpdate(const sal_uInt16 /*nSid*/, const SfxItemState /* eState */,
                                     const SfxPoolItem* pState)
{
    if (!pState) //disposed
        return;
}
}
