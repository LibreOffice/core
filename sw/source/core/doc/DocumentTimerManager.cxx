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
#include <rtl/logfile.hxx>

namespace sw
{

DocumentTimerManager::DocumentTimerManager( SwDoc& i_rSwdoc ) : m_rSwdoc( i_rSwdoc ),
                                                                mbStartIdleTimer( false ),
                                                                mIdleBlockCount( 0 )
{
    maIdleTimer.SetTimeout( 600 );
    maIdleTimer.SetTimeoutHdl( LINK( this, DocumentTimerManager, DoIdleJobs) );
}

void DocumentTimerManager::StartIdling()
{
    mbStartIdleTimer = true;
    if( !mIdleBlockCount )
        maIdleTimer.Start();
}

void DocumentTimerManager::StopIdling()
{
    mbStartIdleTimer = false;
    maIdleTimer.Stop();
}

void DocumentTimerManager::BlockIdling()
{
    maIdleTimer.Stop();
    ++mIdleBlockCount;
}

void DocumentTimerManager::UnblockIdling()
{
    --mIdleBlockCount;
    if( !mIdleBlockCount && mbStartIdleTimer && !maIdleTimer.IsActive() )
        maIdleTimer.Start();
}

void DocumentTimerManager::StartBackgroundJobs() {
    // Trigger DoIdleJobs(), asynchronously.
    maIdleTimer.Start();
}

IMPL_LINK( DocumentTimerManager, DoIdleJobs, Timer *, pTimer )
{
#ifdef TIMELOG
    static ::rtl::Logfile* pModLogFile = 0;
    if( !pModLogFile )
        pModLogFile = new ::rtl::Logfile( "First DoIdleJobs" );
#endif

    SwRootFrm* pTmpRoot = m_rSwdoc.GetCurrentLayout();
    if( pTmpRoot &&
        !SfxProgress::GetActiveProgress( m_rSwdoc.GetDocShell() ) )
    {
        SwViewShell *pSh, *pStartSh;
        pSh = pStartSh = m_rSwdoc.GetCurrentViewShell();
        do {
            if( pSh->ActionPend() )
            {
                pTimer->Start();
                return 0;
            }
            pSh = (SwViewShell*)pSh->GetNext();
        } while( pSh != pStartSh );

        if( pTmpRoot->IsNeedGrammarCheck() )
        {
            bool bIsOnlineSpell = pSh->GetViewOptions()->IsOnlineSpell();
            bool bIsAutoGrammar = false;
            SvtLinguConfig().GetProperty( OUString(
                        UPN_IS_GRAMMAR_AUTO ) ) >>= bIsAutoGrammar;

            if (bIsOnlineSpell && bIsAutoGrammar)
                StartGrammarChecking( m_rSwdoc );
        }
        std::set<SwRootFrm*> aAllLayouts = m_rSwdoc.GetAllLayouts();
        std::set<SwRootFrm*>::iterator pLayIter = aAllLayouts.begin();
        for ( ;pLayIter != aAllLayouts.end();++pLayIter )
        {
            if ((*pLayIter)->IsIdleFormat())
            {
                (*pLayIter)->GetCurrShell()->LayoutIdle();

                // Defer the remaining work.
                pTimer->Start();
                return 0;
            }
        }

        SwFldUpdateFlags nFldUpdFlag = m_rSwdoc.GetDocumentSettingManager().getFieldUpdateFlags(true);
        if( ( AUTOUPD_FIELD_ONLY == nFldUpdFlag
                    || AUTOUPD_FIELD_AND_CHARTS == nFldUpdFlag ) &&
                m_rSwdoc.getIDocumentFieldsAccess().GetUpdtFlds().IsFieldsDirty()
                // If we switch the field name the Fields are not updated.
                // So the "backgorund update" should always be carried out
                /* && !pStartSh->GetViewOptions()->IsFldName()*/ )
        {
            if ( m_rSwdoc.getIDocumentFieldsAccess().GetUpdtFlds().IsInUpdateFlds() ||
                 m_rSwdoc.getIDocumentFieldsAccess().IsExpFldsLocked() )
            {
                pTimer->Start();
                return 0;
            }

            //  Action brackets!
            m_rSwdoc.getIDocumentFieldsAccess().GetUpdtFlds().SetInUpdateFlds( true );

            pTmpRoot->StartAllAction();

            // no jump on update of fields #i85168#
            const bool bOldLockView = pStartSh->IsViewLocked();
            pStartSh->LockView( true );

            m_rSwdoc.getIDocumentFieldsAccess().GetSysFldType( RES_CHAPTERFLD )->ModifyNotification( 0, 0 );    // ChapterField
            m_rSwdoc.getIDocumentFieldsAccess().UpdateExpFlds( 0, false );      // Updates ExpressionFields
            m_rSwdoc.getIDocumentFieldsAccess().UpdateTblFlds(NULL);                // Tables
            m_rSwdoc.getIDocumentFieldsAccess().UpdateRefFlds(NULL);                // References

            pTmpRoot->EndAllAction();

            pStartSh->LockView( bOldLockView );

            m_rSwdoc.getIDocumentFieldsAccess().GetUpdtFlds().SetInUpdateFlds( false );
            m_rSwdoc.getIDocumentFieldsAccess().GetUpdtFlds().SetFieldsDirty( false );
        }
    }
#ifdef TIMELOG
    if( pModLogFile && 1 != (long)pModLogFile )
        delete pModLogFile, ((long&)pModLogFile) = 1;
#endif
    return 0;
}

DocumentTimerManager::~DocumentTimerManager() {}

}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
