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

#include "crsrsh.hxx"
#include "ndtxt.hxx"
#include <docary.hxx>
#include "IMark.hxx"
#include "callnk.hxx"
#include "swcrsr.hxx"
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>

using namespace std;

namespace
{
    struct CursorStateHelper
    {
        explicit CursorStateHelper(SwCursorShell const & rShell)
            : m_pCursor(rShell.GetSwCursor())
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

    bool lcl_ReverseMarkOrderingByEnd(const IDocumentMarkAccess::pMark_t& rpFirst,
        const IDocumentMarkAccess::pMark_t& rpSecond)
    {
        return rpFirst->GetMarkEnd() > rpSecond->GetMarkEnd();
    }

    bool lcl_IsInvisibleBookmark(const IDocumentMarkAccess::pMark_t& pMark)
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

bool SwCursorShell::GotoMark(const ::sw::mark::IMark* const pMark, bool bAtStart)
{
    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    if ( bAtStart )
        *(aCursorSt.m_pCursor)->GetPoint() = pMark->GetMarkStart();
    else
        *(aCursorSt.m_pCursor)->GetPoint() = pMark->GetMarkEnd();

    if(aCursorSt.RollbackIfIllegal()) return false;

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

bool SwCursorShell::GotoMark(const ::sw::mark::IMark* const pMark)
{
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
    IDocumentMarkAccess::container_t vCandidates;
    remove_copy_if(
        upper_bound( // finds the first that is starting after
            pMarkAccess->getBookmarksBegin(),
            pMarkAccess->getBookmarksEnd(),
            *GetCursor()->GetPoint(),
            sw::mark::CompareIMarkStartsAfter()),
        pMarkAccess->getBookmarksEnd(),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);

    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    IDocumentMarkAccess::const_iterator_t ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        aCursorSt.SetCursorToMark(ppMark->get());
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
    IDocumentMarkAccess::container_t vCandidates;
    remove_copy_if(
        pMarkAccess->getBookmarksBegin(),
        upper_bound(
            pMarkAccess->getBookmarksBegin(),
            pMarkAccess->getBookmarksEnd(),
            *GetCursor()->GetPoint(),
            sw::mark::CompareIMarkStartsAfter()),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);
    sort(
        vCandidates.begin(),
        vCandidates.end(),
        &lcl_ReverseMarkOrderingByEnd);

    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    IDocumentMarkAccess::const_iterator_t ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        // ignoring those not ending before the Cursor
        // (we were only able to eliminate those starting
        // behind the Cursor by the upper_bound(..)
        // above)
        if(!(**ppMark).EndsBefore(*GetCursor()->GetPoint()))
            continue;
        aCursorSt.SetCursorToMark(ppMark->get());
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
    SwPosition pos(*GetCursor()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkFor(pos);
}

::sw::mark::IFieldmark* SwCursorShell::GetFieldmarkAfter()
{
    SwPosition pos(*GetCursor()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkAfter(pos);
}

::sw::mark::IFieldmark* SwCursorShell::GetFieldmarkBefore()
{
    SwPosition pos(*GetCursor()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkBefore(pos);
}

bool SwCursorShell::GotoFieldmark(::sw::mark::IFieldmark const * const pMark)
{
    if(pMark==nullptr) return false;

    // watch Cursor-Moves
    CursorStateHelper aCursorSt(*this);
    aCursorSt.SetCursorToMark(pMark);
    ++aCursorSt.m_pCursor->GetPoint()->nContent;
    --aCursorSt.m_pCursor->GetMark()->nContent;

    if(aCursorSt.RollbackIfIllegal()) return false;

    UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
