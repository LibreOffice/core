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

#include <crsrsh.hxx>
#include <ndtxt.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <IMark.hxx>
#include <swcrsr.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>

namespace
{
    struct CursorStateHelper
    {
        explicit CursorStateHelper(SwCursorShell const & rShell)
            : m_pCursor(rShell.GetCursor())
            , m_aSaveState(*m_pCursor)
        { }

        void SetCursorToMark(::sw::mark::IMark const * const pMark)
        {
            *(m_pCursor->GetPoint()) = pMark->GetMarkStart();
            if(pMark->IsExpanded())
            {
                m_pCursor->SetMark();
                *(m_pCursor->GetMark()) = pMark->GetMarkEnd();
            }
        }

        /// returns true if the Cursor had been rolled back
        bool RollbackIfIllegal()
        {
            if(m_pCursor->IsSelOvr(SwCursorSelOverFlags::CheckNodeSection
                | SwCursorSelOverFlags::Toggle))
            {
                m_pCursor->DeleteMark();
                m_pCursor->RestoreSavePos();
                return true;
            }
            return false;
        }

        SwCursor* m_pCursor;
        SwCursorSaveState m_aSaveState;
    };

    bool lcl_ReverseMarkOrderingByEnd(const ::sw::mark::IMark* pFirst,
                                      const ::sw::mark::IMark* pSecond)
    {
        return pFirst->GetMarkEnd() > pSecond->GetMarkEnd();
    }

    bool lcl_IsInvisibleBookmark(const ::sw::mark::IMark* pMark)
    {
        return IDocumentMarkAccess::GetType(*pMark) != IDocumentMarkAccess::MarkType::BOOKMARK;
    }
}

// at CurrentCursor.SPoint
::sw::mark::IMark* SwCursorShell::SetBookmark(
    const vcl::KeyCode& rCode,
    const OUString& rName,
    IDocumentMarkAccess::MarkType eMark)
{
    StartAction();
    ::sw::mark::IMark* pMark = getIDocumentMarkAccess()->makeMark(
        *GetCursor(),
        rName,
        eMark, sw::mark::InsertMode::New);
    ::sw::mark::IBookmark* pBookmark = dynamic_cast< ::sw::mark::IBookmark* >(pMark);
    if(pBookmark)
    {
        pBookmark->SetKeyCode(rCode);
        pBookmark->SetShortName(OUString());
    }
    EndAction();
    return pMark;
}
// set CurrentCursor.SPoint

// at CurrentCursor.SPoint
::sw::mark::IMark* SwCursorShell::SetBookmark2(
    const vcl::KeyCode& rCode,
    const OUString& rName,
    bool bHide,
    const OUString& rCondition)
{
    StartAction();
    ::sw::mark::IMark* pMark = getIDocumentMarkAccess()->makeMark(
        *GetCursor(),
        rName,
        IDocumentMarkAccess::MarkType::BOOKMARK, sw::mark::InsertMode::New);
    ::sw::mark::IBookmark* pBookmark = dynamic_cast< ::sw::mark::IBookmark* >(pMark);
    if (pBookmark)
    {
        pBookmark->SetKeyCode(rCode);
        pBookmark->SetShortName(OUString());
        pBookmark->Hide(bHide);
        pBookmark->SetHideCondition(rCondition);
    }
    EndAction();
    return pMark;
}

namespace sw {

bool IsMarkHidden(SwRootFrame const& rLayout, ::sw::mark::IMark const& rMark)
{
    if (!rLayout.HasMergedParas())
    {
        return false;
    }
    SwNode const& rNode(rMark.GetMarkPos().GetNode());
    SwTextNode const*const pTextNode(rNode.GetTextNode());
    if (pTextNode == nullptr)
    {   // UNO_BOOKMARK may point to table node
        return rNode.GetRedlineMergeFlag() == SwNode::Merge::Hidden;
    }
    SwTextFrame const*const pFrame(static_cast<SwTextFrame const*>(
        pTextNode->getLayoutFrame(&rLayout)));
    if (!pFrame)
    {
        return true;
    }
    if (rMark.IsExpanded())
    {
        SwTextFrame const*const pOtherFrame(static_cast<SwTextFrame const*>(
            rMark.GetOtherMarkPos().GetNode().GetTextNode()->getLayoutFrame(&rLayout)));
        return pFrame == pOtherFrame
            && pFrame->MapModelToViewPos(rMark.GetMarkPos())
                == pFrame->MapModelToViewPos(rMark.GetOtherMarkPos());
    }
    else
    {
        if (rMark.GetMarkPos().GetContentIndex() == pTextNode->Len())
        {   // at end of node: never deleted (except if node deleted)
            return pTextNode->GetRedlineMergeFlag() == SwNode::Merge::Hidden;
        }
        else
        {   // check character following mark pos
            return pFrame->MapModelToViewPos(rMark.GetMarkPos())
                == pFrame->MapModelToView(pTextNode, rMark.GetMarkPos().GetContentIndex() + 1);
        }
    }
}

} // namespace sw

// set CurrentCursor.SPoint
bool SwCursorShell::GotoMark(const ::sw::mark::IMark* const pMark, bool bAtStart)
{
    if (sw::IsMarkHidden(*GetLayout(), *pMark))
    {
        return false;
    }
    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    if ( bAtStart )
        *aCursorSt.m_pCursor->GetPoint() = pMark->GetMarkStart();
    else
        *aCursorSt.m_pCursor->GetPoint() = pMark->GetMarkEnd();

    if(aCursorSt.RollbackIfIllegal()) return false;

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

bool SwCursorShell::GotoMark(const ::sw::mark::IMark* const pMark)
{
    if (sw::IsMarkHidden(*GetLayout(), *pMark))
    {
        return false;
    }
    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    aCursorSt.SetCursorToMark(pMark);

    if(aCursorSt.RollbackIfIllegal()) return false;

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

bool SwCursorShell::GoNextBookmark()
{
    IDocumentMarkAccess* pMarkAccess = getIDocumentMarkAccess();
    std::vector<::sw::mark::IMark*> vCandidates;
    remove_copy_if(
        pMarkAccess->findFirstBookmarkStartsAfter(*GetCursor()->GetPoint()),
        pMarkAccess->getBookmarksEnd(),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);

    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    auto ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        if (sw::IsMarkHidden(*GetLayout(), **ppMark))
        {
            continue;
        }
        aCursorSt.SetCursorToMark(*ppMark);
        if(!aCursorSt.RollbackIfIllegal())
            break; // found legal move
    }
    if(ppMark==vCandidates.end())
    {
        SttEndDoc(false);
        return false;
    }

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

bool SwCursorShell::GoPrevBookmark()
{
    IDocumentMarkAccess* pMarkAccess = getIDocumentMarkAccess();
    // candidates from which to choose the mark before
    // no need to consider marks starting after rPos
    std::vector<::sw::mark::IMark*> vCandidates;
    remove_copy_if(
        pMarkAccess->getBookmarksBegin(),
        pMarkAccess->findFirstBookmarkStartsAfter(*GetCursor()->GetPoint()),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);
    sort(
        vCandidates.begin(),
        vCandidates.end(),
        &lcl_ReverseMarkOrderingByEnd);

    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    auto ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        // ignoring those not ending before the Cursor
        // (we were only able to eliminate those starting
        // behind the Cursor by the upper_bound(..)
        // above)
        if(!((**ppMark).GetMarkEnd() < *GetCursor()->GetPoint()))
            continue;
        if (sw::IsMarkHidden(*GetLayout(), **ppMark))
        {
            continue;
        }
        aCursorSt.SetCursorToMark(*ppMark);
        if(!aCursorSt.RollbackIfIllegal())
            break; // found legal move
    }
    if(ppMark==vCandidates.end())
    {
        SttEndDoc(true);
        return false;
    }

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

bool SwCursorShell::IsFormProtected()
{
    return getIDocumentSettingAccess().get(DocumentSettingId::PROTECT_FORM);
}

::sw::mark::IFieldmark* SwCursorShell::GetCurrentFieldmark()
{
    // TODO: Refactor
    SwPosition pos(*GetCursor()->Start());
    return getIDocumentMarkAccess()->getInnerFieldmarkFor(pos);
}

sw::mark::IFieldmark* SwCursorShell::GetFieldmarkAfter()
{
    SwPosition pos(*GetCursor()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkAfter(pos, /*bLoop*/true);
}

sw::mark::IFieldmark* SwCursorShell::GetFieldmarkBefore()
{
    SwPosition pos(*GetCursor()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkBefore(pos, /*bLoop*/true);
}

bool SwCursorShell::GotoFieldmark(::sw::mark::IFieldmark const * const pMark, bool completeSelection)
{
    if(pMark==nullptr) return false;

    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    aCursorSt.SetCursorToMark(pMark);
    if (!completeSelection || aCursorSt.m_pCursor->HasReadonlySel(false, false))
    {
        aCursorSt.m_pCursor->GetPoint()->AdjustContent(+1);
        aCursorSt.m_pCursor->GetMark()->AdjustContent(-1);
    }

    if(aCursorSt.RollbackIfIllegal()) return false;

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
