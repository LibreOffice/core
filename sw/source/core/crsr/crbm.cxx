/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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


::sw::mark::IMark* SwCrsrShell::SetBookmark(
    const KeyCode& rCode,
    const OUString& rName,
    const OUString& rShortName,
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


bool SwCrsrShell::GotoMark(const ::sw::mark::IMark* const pMark, bool bAtStart)
{
    
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
            sw::mark::CompareIMarkStartsAfter()),
        pMarkAccess->getBookmarksEnd(),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);

    
    CrsrStateHelper aCrsrSt(*this);
    IDocumentMarkAccess::const_iterator_t ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        aCrsrSt.SetCrsrToMark(ppMark->get());
        if(!aCrsrSt.RollbackIfIllegal())
            break; 
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
    
    
    IDocumentMarkAccess::container_t vCandidates;
    remove_copy_if(
        pMarkAccess->getBookmarksBegin(),
        upper_bound(
            pMarkAccess->getBookmarksBegin(),
            pMarkAccess->getBookmarksEnd(),
            *GetCrsr()->GetPoint(),
            sw::mark::CompareIMarkStartsAfter()),
        back_inserter(vCandidates),
        &lcl_IsInvisibleBookmark);
    sort(
        vCandidates.begin(),
        vCandidates.end(),
        &lcl_ReverseMarkOrderingByEnd);

    
    CrsrStateHelper aCrsrSt(*this);
    IDocumentMarkAccess::const_iterator_t ppMark = vCandidates.begin();
    for(; ppMark!=vCandidates.end(); ++ppMark)
    {
        
        
        
        
        if(!(**ppMark).EndsBefore(*GetCrsr()->GetPoint()))
            continue;
        aCrsrSt.SetCrsrToMark(ppMark->get());
        if(!aCrsrSt.RollbackIfIllegal())
            break; 
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

    
    CrsrStateHelper aCrsrSt(*this);
    aCrsrSt.SetCrsrToMark(pMark);
    aCrsrSt.m_pCrsr->GetPoint()->nContent++;
    aCrsrSt.m_pCrsr->GetMark()->nContent--;

    if(aCrsrSt.RollbackIfIllegal()) return false;

    UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
