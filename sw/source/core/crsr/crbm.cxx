/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "crsrsh.hxx"
#include "ndtxt.hxx"
#include <docary.hxx>
#include <boost/bind.hpp>

#include "IMark.hxx"
#include "callnk.hxx"
#include "swcrsr.hxx"
#include <IDocumentMarkAccess.hxx>
#include <IDocumentSettingAccess.hxx>

using namespace std;

namespace
{
    struct CrsrStateHelper
    {
        CrsrStateHelper(SwCrsrShell& rShell)
            : m_aLink(rShell)
            , m_pCrsr(rShell.GetSwCrsr())
            , m_aSaveState(*m_pCrsr)
        { }

        void SetCrsrToMark(::sw::mark::IMark const * const pMark)
        {
            *(m_pCrsr->GetPoint()) = pMark->GetMarkStart();
            if(pMark->IsExpanded())
            {
                m_pCrsr->SetMark();
                *(m_pCrsr->GetMark()) = pMark->GetMarkEnd();
            }
        }

        /// returns true if the Cursor had been rolled back
        bool RollbackIfIllegal()
        {
            if(m_pCrsr->IsSelOvr(nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION
                | nsSwCursorSelOverFlags::SELOVER_TOGGLE))
            {
                m_pCrsr->DeleteMark();
                m_pCrsr->RestoreSavePos();
                return true;
            }
            return false;
        }

        SwCallLink m_aLink;
        SwCursor* m_pCrsr;
        SwCrsrSaveState m_aSaveState;
    };


    static bool lcl_ReverseMarkOrderingByEnd(const IDocumentMarkAccess::pMark_t& rpFirst,
        const IDocumentMarkAccess::pMark_t& rpSecond)
    {
        return rpFirst->GetMarkEnd() > rpSecond->GetMarkEnd();
    }

    static bool lcl_IsInvisibleBookmark(IDocumentMarkAccess::pMark_t pMark)
    {
        return IDocumentMarkAccess::GetType(*pMark) != IDocumentMarkAccess::BOOKMARK;
    }
}

// at CurCrsr.SPoint
::sw::mark::IMark* SwCrsrShell::SetBookmark(
    const KeyCode& rCode,
    const ::rtl::OUString& rName,
    const ::rtl::OUString& rShortName,
    IDocumentMarkAccess::MarkType eMark)
{
    StartAction();
    ::sw::mark::IMark* pMark = getIDocumentMarkAccess()->makeMark(
        *GetCrsr(),
        rName,
        eMark);
    ::sw::mark::IBookmark* pBookmark = dynamic_cast< ::sw::mark::IBookmark* >(pMark);
    if(pBookmark)
    {
        pBookmark->SetKeyCode(rCode);
        pBookmark->SetShortName(rShortName);
    }
    EndAction();
    return pMark;
}
// set CurCrsr.SPoint

bool SwCrsrShell::GotoMark(const ::sw::mark::IMark* const pMark, bool bAtStart)
{
    // watch Crsr-Moves
    CrsrStateHelper aCrsrSt(*this);
    if ( bAtStart )
        *(aCrsrSt.m_pCrsr)->GetPoint() = pMark->GetMarkStart();
    else
        *(aCrsrSt.m_pCrsr)->GetPoint() = pMark->GetMarkEnd();
    if(aCrsrSt.RollbackIfIllegal()) return false;

    UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return true;
}

bool SwCrsrShell::GotoMark(const ::sw::mark::IMark* const pMark)
{
    // watch Crsr-Moves
    CrsrStateHelper aCrsrSt(*this);
    aCrsrSt.SetCrsrToMark(pMark);

    if(aCrsrSt.RollbackIfIllegal()) return false;

    UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return true;
}

bool SwCrsrShell::GoNextBookmark()
{
    IDocumentMarkAccess* const pMarkAccess = getIDocumentMarkAccess();
    IDocumentMarkAccess::container_t vCandidates;
    remove_copy_if(
        upper_bound(
            pMarkAccess->getBookmarksBegin(),
            pMarkAccess->getBookmarksEnd(),
            *GetCrsr()->GetPoint(),
            boost::bind(&::sw::mark::IMark::StartsAfter, _2, _1)), // finds the first that is starting after
        pMarkAccess->getBookmarksEnd(),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);

    // watch Crsr-Moves
    CrsrStateHelper aCrsrSt(*this);
    IDocumentMarkAccess::const_iterator_t ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        aCrsrSt.SetCrsrToMark(ppMark->get());
        if(!aCrsrSt.RollbackIfIllegal())
            break; // found legal move
    }
    if(ppMark==vCandidates.end())
    {
        SttEndDoc(false);
        return false;
    }

    UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return true;
}

bool SwCrsrShell::GoPrevBookmark()
{
    IDocumentMarkAccess* const pMarkAccess = getIDocumentMarkAccess();
    // candidates from which to choose the mark before
    // no need to consider marks starting after rPos
    IDocumentMarkAccess::container_t vCandidates;
    remove_copy_if(
        pMarkAccess->getBookmarksBegin(),
        upper_bound(
            pMarkAccess->getBookmarksBegin(),
            pMarkAccess->getBookmarksEnd(),
            *GetCrsr()->GetPoint(),
            boost::bind(&::sw::mark::IMark::StartsAfter, _2, _1)),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);
    sort(
        vCandidates.begin(),
        vCandidates.end(),
        &lcl_ReverseMarkOrderingByEnd);

    // watch Crsr-Moves
    CrsrStateHelper aCrsrSt(*this);
    IDocumentMarkAccess::const_iterator_t ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        // ignoring those not ending before the Crsr
        // (we were only able to eliminate those starting
        // behind the Crsr by the upper_bound(..)
        // above)
        if(!(**ppMark).EndsBefore(*GetCrsr()->GetPoint()))
            continue;
        aCrsrSt.SetCrsrToMark(ppMark->get());
        if(!aCrsrSt.RollbackIfIllegal())
            break; // found legal move
    }
    if(ppMark==vCandidates.end())
    {
        SttEndDoc(true);
        return false;
    }

    UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return true;
}

bool SwCrsrShell::IsFormProtected()
{
    return getIDocumentSettingAccess()->get(IDocumentSettingAccess::PROTECT_FORM);
}

::sw::mark::IFieldmark* SwCrsrShell::GetCurrentFieldmark()
{
    // TODO: Refactor
    SwPosition pos(*GetCrsr()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkFor(pos);
}

::sw::mark::IFieldmark* SwCrsrShell::GetFieldmarkAfter()
{
    SwPosition pos(*GetCrsr()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkAfter(pos);
}

::sw::mark::IFieldmark* SwCrsrShell::GetFieldmarkBefore()
{
    SwPosition pos(*GetCrsr()->GetPoint());
    return getIDocumentMarkAccess()->getFieldmarkBefore(pos);
}

bool SwCrsrShell::GotoFieldmark(::sw::mark::IFieldmark const * const pMark)
{
    if(pMark==NULL) return false;

    // watch Crsr-Moves
    CrsrStateHelper aCrsrSt(*this);
    aCrsrSt.SetCrsrToMark(pMark);
    aCrsrSt.m_pCrsr->GetPoint()->nContent++;
    aCrsrSt.m_pCrsr->GetMark()->nContent--;
    if(aCrsrSt.RollbackIfIllegal()) return false;

    UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
