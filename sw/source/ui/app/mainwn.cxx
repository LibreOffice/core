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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <mdiexp.hxx>
#include <tools/shl.hxx>
#include <tools/string.hxx>
#ifndef _SVARRAY_HXX
#include <svl/svstdarr.hxx>
#endif
#include <sfx2/progress.hxx>
#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif
#include <docsh.hxx>
#include <swmodule.hxx>
#include "swtypes.hxx"

class SwDocShell;

struct SwProgress
{
    long nStartValue,
         nStartCount;
    SwDocShell  *pDocShell;
    SfxProgress *pProgress;
};

static SvPtrarr *pProgressContainer = 0;

static SwProgress *lcl_SwFindProgress( SwDocShell *pDocShell )
{
    for ( sal_uInt16 i = 0; i < pProgressContainer->Count(); ++i )
    {
        SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
        if ( pTmp->pDocShell == pDocShell )
            return pTmp;
    }
    return 0;
}


void StartProgress( sal_uInt16 nMessResId, long nStartValue, long nEndValue,
                    SwDocShell *pDocShell )
{
    if( !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = 0;

        if ( !pProgressContainer )
            pProgressContainer = new SvPtrarr( 2, 2 );
        else
        {
            if ( 0 != (pProgress = lcl_SwFindProgress( pDocShell )) )
                ++pProgress->nStartCount;
        }
        if ( !pProgress )
        {
            pProgress = new SwProgress;
            pProgress->pProgress = new SfxProgress( pDocShell,
                                                    SW_RESSTR(nMessResId),
                                                    nEndValue - nStartValue,
                                                    sal_False,
                                                    sal_True );
            pProgress->nStartCount = 1;
            pProgress->pDocShell = pDocShell;
            pProgressContainer->Insert( (void*)pProgress, 0 );
        }
        pProgress->nStartValue = nStartValue;
    }
}


void SetProgressState( long nPosition, SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->SetState(nPosition - pProgress->nStartValue);
    }
}


void EndProgress( SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = 0;
        sal_uInt16 i;
        for ( i = 0; i < pProgressContainer->Count(); ++i )
        {
            SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
            if ( pTmp->pDocShell == pDocShell )
            {
                pProgress = pTmp;
                break;
            }
        }

        if ( pProgress && 0 == --pProgress->nStartCount )
        {
            pProgress->pProgress->Stop();
            pProgressContainer->Remove( i );
            delete pProgress->pProgress;
            delete pProgress;
            //#112337# it may happen that the container has been removed
            //while rescheduling
            if ( pProgressContainer && !pProgressContainer->Count() )
                delete pProgressContainer, pProgressContainer = 0;
        }
    }
}


void SetProgressText( sal_uInt16 nId, SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->SetStateText( 0, SW_RESSTR(nId) );
    }
}


void RescheduleProgress( SwDocShell *pDocShell )
{
    if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
    {
        SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
        if ( pProgress )
            pProgress->pProgress->Reschedule();
    }
}


