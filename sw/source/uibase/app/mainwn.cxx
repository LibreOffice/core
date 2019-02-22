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

#include <mdiexp.hxx>
#include <sfx2/progress.hxx>
#include <docsh.hxx>
#include <swmodule.hxx>
#include <swtypes.hxx>

class SwDocShell;

struct SwProgress
{
    long nStartValue,
         nStartCount;
    SwDocShell  *pDocShell;
    std::unique_ptr<SfxProgress> pProgress;
};

static std::vector<std::unique_ptr<SwProgress>> *pProgressContainer = nullptr;

static SwProgress *lcl_SwFindProgress( SwDocShell const *pDocShell )
{
    for (auto& pTmp : *pProgressContainer)
    {
        if ( pTmp->pDocShell == pDocShell )
            return pTmp.get();
    }
    return nullptr;
}

void StartProgress( const char* pMessResId, long nStartValue, long nEndValue,
                    SwDocShell *pDocShell )
{
    if( SW_MOD()->IsEmbeddedLoadSave() )
        return;

    SwProgress *pProgress = nullptr;

    if ( !pProgressContainer )
        pProgressContainer = new std::vector<std::unique_ptr<SwProgress>>;
    else
    {
        pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            ++pProgress->nStartCount;
    }

    if ( !pProgress )
    {
        pProgress = new SwProgress;
        pProgress->pProgress.reset( new SfxProgress( pDocShell,
                                                SwResId(pMessResId),
                                                nEndValue - nStartValue ) );
        pProgress->nStartCount = 1;
        pProgress->pDocShell = pDocShell;
        pProgressContainer->insert( pProgressContainer->begin(), std::unique_ptr<SwProgress>(pProgress) );
    }
    pProgress->nStartValue = nStartValue;
}

void SetProgressState( long nPosition, SwDocShell const *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->SetState(nPosition - pProgress->nStartValue);
    }
}

void EndProgress( SwDocShell const *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = nullptr;
        std::vector<SwProgress *>::size_type i;
        for ( i = 0; i < pProgressContainer->size(); ++i )
        {
            SwProgress *pTmp = (*pProgressContainer)[i].get();
            if ( pTmp->pDocShell == pDocShell )
            {
                pProgress = pTmp;
                break;
            }
        }

        if ( pProgress && 0 == --pProgress->nStartCount )
        {
            pProgress->pProgress->Stop();
            pProgressContainer->erase( pProgressContainer->begin() + i );
            //#112337# it may happen that the container has been removed
            //while rescheduling
            if ( pProgressContainer && pProgressContainer->empty() )
            {
                delete pProgressContainer;
                pProgressContainer = nullptr;
            }
        }
    }
}

void RescheduleProgress( SwDocShell const *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            SfxProgress::Reschedule();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
