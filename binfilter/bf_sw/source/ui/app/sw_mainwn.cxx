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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#ifndef _SFX_PROGRESS_HXX //autogen
#include <bf_sfx2/progress.hxx>
#endif

#ifndef _SWDOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#include "swtypes.hxx"
namespace binfilter {

class SwDocShell;

struct SwProgress
{
    long nStartValue,
         nStartCount;
    SwDocShell  *pDocShell;
    SfxProgress *pProgress;
};

static SvPtrarr *pProgressContainer = 0;

/*N*/ SwProgress *lcl_SwFindProgress( SwDocShell *pDocShell )
/*N*/ {
/*N*/ 	for ( USHORT i = 0; i < pProgressContainer->Count(); ++i )
/*N*/ 	{
/*N*/ 		SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
/*N*/ 		if ( pTmp->pDocShell == pDocShell )
/*N*/ 			return pTmp;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }


/*N*/ void StartProgress( USHORT nMessResId, long nStartValue, long nEndValue,
/*N*/ 					SwDocShell *pDocShell )
/*N*/ {
/*N*/ 	if( !SW_MOD()->IsEmbeddedLoadSave() )
/*N*/ 	{
/*N*/ 		SwProgress *pProgress = 0;
/*N*/ 
/*N*/ 		if ( !pProgressContainer )
/*N*/ 			pProgressContainer = new SvPtrarr( 2, 2 );
/*N*/ 		else
/*N*/ 		{
/*?*/ 			if ( 0 != (pProgress = lcl_SwFindProgress( pDocShell )) )
/*?*/ 				++pProgress->nStartCount;
/*N*/ 		}
/*N*/ 		if ( !pProgress )
/*N*/ 		{
/*N*/ 			pProgress = new SwProgress;
/*N*/ 			pProgress->pProgress = new SfxProgress( pDocShell,
/*N*/ 													SW_RESSTR(nMessResId),
/*N*/ 													nEndValue - nStartValue,
/*N*/ 													FALSE,
/*N*/ 													TRUE );
/*N*/ 			pProgress->nStartCount = 1;
/*N*/ 			pProgress->pDocShell = pDocShell;
/*N*/ 			pProgressContainer->Insert( (void*)pProgress, 0 );
/*N*/ 		}
/*N*/ 		pProgress->nStartValue = nStartValue;
/*N*/ 	}
/*N*/ }


/*N*/ void SetProgressState( long nPosition, SwDocShell *pDocShell )
/*N*/ {
/*N*/ 	if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
/*N*/ 	{
/*N*/ 		SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
/*N*/ 		if ( pProgress )
/*N*/ 			pProgress->pProgress->SetState(nPosition - pProgress->nStartValue);
/*N*/ 	}
/*N*/ }


/*N*/ void EndProgress( SwDocShell *pDocShell )
/*N*/ {
/*N*/ 	if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
/*N*/ 	{
/*N*/ 		SwProgress *pProgress = 0;
/*N*/ 		USHORT i;
/*N*/ 		for ( i = 0; i < pProgressContainer->Count(); ++i )
/*N*/ 		{
/*N*/ 			SwProgress *pTmp = (SwProgress*)(*pProgressContainer)[i];
/*N*/ 			if ( pTmp->pDocShell == pDocShell )
/*N*/ 			{
/*N*/ 				pProgress = pTmp;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if ( pProgress && 0 == --pProgress->nStartCount )
/*N*/ 		{
/*N*/ 			pProgress->pProgress->Stop();
/*N*/ 			pProgressContainer->Remove( i );
/*N*/ 			delete pProgress->pProgress;
/*N*/ 			delete pProgress;
/*N*/ 			if ( !pProgressContainer->Count() )
/*N*/ 				delete pProgressContainer, pProgressContainer = 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ }




/*N*/ void RescheduleProgress( SwDocShell *pDocShell )
/*N*/ {
/*N*/ 	if( pProgressContainer && !SW_MOD()->IsEmbeddedLoadSave() )
/*N*/ 	{
/*N*/ 		SwProgress *pProgress = lcl_SwFindProgress( pDocShell );
/*N*/ 		if ( pProgress )
/*N*/ 			pProgress->pProgress->Reschedule();
/*N*/ 	}
/*N*/ }

}
