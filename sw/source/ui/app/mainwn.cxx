/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mainwn.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#pragma hdrstop


#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFX_PROGRESS_HXX //autogen
#include <sfx2/progress.hxx>
#endif
#ifndef _RESID_HXX //autogen
#include <tools/resid.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
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
    for ( USHORT i = 0; i < pProgressContainer->Count(); ++i )
    {
        SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
        if ( pTmp->pDocShell == pDocShell )
            return pTmp;
    }
    return 0;
}


void StartProgress( USHORT nMessResId, long nStartValue, long nEndValue,
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
                                                    FALSE,
                                                    TRUE );
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
        USHORT i;
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


void SetProgressText( USHORT nId, SwDocShell *pDocShell )
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


