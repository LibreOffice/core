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
#include <DocumentTimerManager.hxx>

#include <doc.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <viewsh.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <fldupde.hxx>
#include <sfx2/progress.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <docfld.hxx>
#include <fldbas.hxx>
#include <vcl/scheduler.hxx>
#include <comphelper/lok.hxx>
#include <editsh.hxx>

namespace sw
{
DocumentTimerManager::DocumentTimerManager(SwDoc& i_rSwdoc)
    : m_rDoc(i_rSwdoc)
    , m_nIdleBlockCount(0)
    , m_bStartOnUnblock(false)
    , m_aDocIdle(i_rSwdoc, "sw::DocumentTimerManager m_aDocIdle")
    , m_aFireIdleJobsTimer("sw::DocumentTimerManager m_aFireIdleJobsTimer")
    , m_bWaitForLokInit(true)
{
    m_aDocIdle.SetPriority(TaskPriority::LOWEST);
    m_aDocIdle.SetInvokeHandler(LINK(this, DocumentTimerManager, DoIdleJobs));

    m_aFireIdleJobsTimer.SetPriority(TaskPriority::DEFAULT_IDLE);
    m_aFireIdleJobsTimer.SetInvokeHandler(LINK(this, DocumentTimerManager, FireIdleJobsTimeout));
    m_aFireIdleJobsTimer.SetTimeout(1000); // Enough time for LOK to render the first tiles.
}

void DocumentTimerManager::StartIdling()
{
    if (m_bWaitForLokInit && comphelper::LibreOfficeKit::isActive())
    {
        // Start the idle jobs only after a certain delay.
        m_bWaitForLokInit = false;
        StopIdling();
        m_aFireIdleJobsTimer.Start();
        return;
    }

    m_bWaitForLokInit = false;
    m_bStartOnUnblock = true;
    if (0 == m_nIdleBlockCount)
    {
        if (!m_aDocIdle.IsActive())
            m_aDocIdle.Start();
        else
            Scheduler::Wakeup();
    }
}

void DocumentTimerManager::StopIdling()
{
    m_bStartOnUnblock = false;
    m_aDocIdle.Stop();
}

void DocumentTimerManager::BlockIdling()
{
    assert(SAL_MAX_UINT32 != m_nIdleBlockCount);
    ++m_nIdleBlockCount;
}

void DocumentTimerManager::UnblockIdling()
{
    assert(0 != m_nIdleBlockCount);
    --m_nIdleBlockCount;

    if ((0 == m_nIdleBlockCount) && m_bStartOnUnblock)
    {
        if (!m_aDocIdle.IsActive())
            m_aDocIdle.Start();
        else
            Scheduler::Wakeup();
    }
}

IMPL_LINK(DocumentTimerManager, FireIdleJobsTimeout, Timer*, , void)
{
    // Now we can run the idle jobs, assuming we finished LOK initialization.
    StartIdling();
}

DocumentTimerManager::IdleJob DocumentTimerManager::GetNextIdleJob() const
{
    SwRootFrame* pTmpRoot = m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pTmpRoot &&
        !SfxProgress::GetActiveProgress( m_rDoc.GetDocShell() ) )
    {
        SwViewShell* pShell(m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell());
        for(const SwViewShell& rSh : pShell->GetRingContainer())
            if( rSh.ActionPend() )
                return IdleJob::Busy;

        if( pTmpRoot->IsNeedGrammarCheck() )
        {
            bool bIsOnlineSpell = pShell->GetViewOptions()->IsOnlineSpell();
            bool bIsAutoGrammar = false;
            SvtLinguConfig().GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bIsAutoGrammar;

            if( bIsOnlineSpell && bIsAutoGrammar && m_rDoc.StartGrammarChecking( true ) )
                return IdleJob::Grammar;
        }

        // If we're dragging re-layout doesn't occur so avoid a busy loop.
        if (!pShell->HasDrawViewDrag())
        {
            for ( auto pLayout : m_rDoc.GetAllLayouts() )
            {
                if( pLayout->IsIdleFormat() )
                    return IdleJob::Layout;
            }
        }

        SwFieldUpdateFlags nFieldUpdFlag = m_rDoc.GetDocumentSettingManager().getFieldUpdateFlags(true);
        if( ( AUTOUPD_FIELD_ONLY == nFieldUpdFlag
                    || AUTOUPD_FIELD_AND_CHARTS == nFieldUpdFlag )
                && m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().IsFieldsDirty() )
        {
            if( m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().IsInUpdateFields()
                    || m_rDoc.getIDocumentFieldsAccess().IsExpFieldsLocked() )
                return IdleJob::Busy;
            return IdleJob::Fields;
        }
    }

    return IdleJob::None;
}

IMPL_LINK_NOARG( DocumentTimerManager, DoIdleJobs, Timer*, void )
{
#ifdef TIMELOG
    static ::rtl::Logfile* pModLogFile = new ::rtl::Logfile( "First DoIdleJobs" );
#endif
    SfxCloseVetoLock lock(m_rDoc.GetDocShell());

    BlockIdling();
    StopIdling();

    IdleJob eJob = GetNextIdleJob();

    switch ( eJob )
    {
    case IdleJob::Grammar:
        m_rDoc.StartGrammarChecking();
        break;

    case IdleJob::Layout:
        for ( auto pLayout : m_rDoc.GetAllLayouts() )
            if( pLayout->IsIdleFormat() )
            {
                pLayout->GetCurrShell()->LayoutIdle();
                break;
            }
         break;

    case IdleJob::Fields:
    {
        SwViewShell* pShell( m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell() );
        SwRootFrame* pTmpRoot = m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout();

        //  Action brackets!
        m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().SetInUpdateFields( true );

        pTmpRoot->StartAllAction();

        // no jump on update of fields #i85168#
        const bool bOldLockView = pShell->IsViewLocked();
        pShell->LockView( true );

        auto pChapterFieldType = m_rDoc.getIDocumentFieldsAccess().GetSysFieldType( SwFieldIds::Chapter );
        pChapterFieldType->CallSwClientNotify(sw::LegacyModifyHint( nullptr, nullptr ));  // ChapterField
        m_rDoc.getIDocumentFieldsAccess().UpdateExpFields( nullptr, false );  // Updates ExpressionFields
        m_rDoc.getIDocumentFieldsAccess().UpdateTableFields(nullptr);  // Tables
        m_rDoc.getIDocumentFieldsAccess().UpdateRefFields();  // References

        // Validate and update the paragraph signatures.
        if (SwEditShell* pSh = m_rDoc.GetEditShell())
            pSh->ValidateAllParagraphSignatures(true);

        pTmpRoot->EndAllAction();

        pShell->LockView( bOldLockView );

        m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().SetInUpdateFields( false );
        m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().SetFieldsDirty( false );
        break;
    }

    case IdleJob::Busy:
        break;
    case IdleJob::None:
        break;
    }

    if ( IdleJob::None != eJob )
        StartIdling();
    UnblockIdling();

#ifdef TIMELOG
    if( pModLogFile && 1 != (long)pModLogFile )
        delete pModLogFile, static_cast<long&>(pModLogFile) = 1;
#endif
}

DocumentTimerManager::~DocumentTimerManager() {}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
