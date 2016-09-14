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
#include <set>
#include <fldupde.hxx>
#include <sfx2/progress.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <docfld.hxx>
#include <fldbas.hxx>

namespace sw
{

DocumentTimerManager::DocumentTimerManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc ),
                                                                mbStartIdleTimer( false ),
                                                                mIdleBlockCount( 0 ),
                                                                maDocIdleTimer("DocumentTimerManagerIdleTimer")
{
    maDocIdleTimer.SetPriority( SchedulerPriority::LOWEST );
    maDocIdleTimer.SetTimeoutHdl( LINK( this, DocumentTimerManager, DoIdleJobs) );
    maDocIdleTimer.SetDebugName( "sw::DocumentTimerManager maDocIdleTimer" );
}

void DocumentTimerManager::StartIdling()
{
    mbStartIdleTimer = true;
    if( !mIdleBlockCount )
    {
        maDocIdleTimer.SetTimeout( 0 );
        maDocIdleTimer.Start();
    }
}

void DocumentTimerManager::StopIdling()
{
    mbStartIdleTimer = false;
    maDocIdleTimer.Stop();
}

void DocumentTimerManager::BlockIdling()
{
    maDocIdleTimer.Stop();
    ++mIdleBlockCount;
}

void DocumentTimerManager::UnblockIdling()
{
    --mIdleBlockCount;
    if( !mIdleBlockCount && mbStartIdleTimer && !maDocIdleTimer.IsActive() )
        maDocIdleTimer.Start();
}

void DocumentTimerManager::StartBackgroundJobs()
{
    // Trigger DoIdleJobs(), asynchronously.
    if (!maDocIdleTimer.IsActive()) //fdo#73165 if the timer is already running don't restart from 0
        maDocIdleTimer.Start();
}

IMPL_LINK_TYPED( DocumentTimerManager, DoIdleJobs, Timer*, pTimer, void )
{
#ifdef TIMELOG
    static ::rtl::Logfile* pModLogFile = 0;
    if( !pModLogFile )
        pModLogFile = new ::rtl::Logfile( "First DoIdleJobs" );
#endif

    SwRootFrame* pTmpRoot = m_rDoc.getIDocumentLayoutAccess().GetCurrentLayout();
    if( pTmpRoot &&
        !SfxProgress::GetActiveProgress( m_rDoc.GetDocShell() ) )
    {
        SwViewShell* pShell(m_rDoc.getIDocumentLayoutAccess().GetCurrentViewShell());
        for(SwViewShell& rSh : pShell->GetRingContainer())
        {
            if( rSh.ActionPend() )
            {
                pTimer->SetTimeout( 2000 );
                pTimer->Start();
                return;
            }
        }

        if( pTmpRoot->IsNeedGrammarCheck() )
        {
            bool bIsOnlineSpell = pShell->GetViewOptions()->IsOnlineSpell();
            bool bIsAutoGrammar = false;
            SvtLinguConfig().GetProperty( OUString(
                        UPN_IS_GRAMMAR_AUTO ) ) >>= bIsAutoGrammar;

            if (bIsOnlineSpell && bIsAutoGrammar)
                StartGrammarChecking( m_rDoc );
        }
        std::set<SwRootFrame*> aAllLayouts = m_rDoc.GetAllLayouts();
        std::set<SwRootFrame*>::iterator pLayIter = aAllLayouts.begin();
        for ( ;pLayIter != aAllLayouts.end();++pLayIter )
        {
            if ((*pLayIter)->IsIdleFormat())
            {
                (*pLayIter)->GetCurrShell()->LayoutIdle();

                // Defer the remaining work.
                pTimer->SetTimeout( 2000 );
                pTimer->Start();
                return;
            }
        }

        SwFieldUpdateFlags nFieldUpdFlag = m_rDoc.GetDocumentSettingManager().getFieldUpdateFlags(true);
        if( ( AUTOUPD_FIELD_ONLY == nFieldUpdFlag
                    || AUTOUPD_FIELD_AND_CHARTS == nFieldUpdFlag ) &&
                m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().IsFieldsDirty()
                // If we switch the field name the Fields are not updated.
                // So the "background update" should always be carried out
                /* && !pStartSh->GetViewOptions()->IsFieldName()*/ )
        {
            if ( m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().IsInUpdateFields() ||
                 m_rDoc.getIDocumentFieldsAccess().IsExpFieldsLocked() )
            {
                pTimer->SetTimeout( 2000 );
                pTimer->Start();
                return;
            }

            //  Action brackets!
            m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().SetInUpdateFields( true );

            pTmpRoot->StartAllAction();

            // no jump on update of fields #i85168#
            const bool bOldLockView = pShell->IsViewLocked();
            pShell->LockView( true );

            m_rDoc.getIDocumentFieldsAccess().GetSysFieldType( RES_CHAPTERFLD )->ModifyNotification( nullptr, nullptr );    // ChapterField
            m_rDoc.getIDocumentFieldsAccess().UpdateExpFields( nullptr, false );      // Updates ExpressionFields
            m_rDoc.getIDocumentFieldsAccess().UpdateTableFields(nullptr);                // Tables
            m_rDoc.getIDocumentFieldsAccess().UpdateRefFields();                // References

            pTmpRoot->EndAllAction();

            pShell->LockView( bOldLockView );

            m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().SetInUpdateFields( false );
            m_rDoc.getIDocumentFieldsAccess().GetUpdateFields().SetFieldsDirty( false );
        }
    }
#ifdef TIMELOG
    if( pModLogFile && 1 != (long)pModLogFile )
        delete pModLogFile, static_cast<long&>(pModLogFile) = 1;
#endif
}

DocumentTimerManager::~DocumentTimerManager() {}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
