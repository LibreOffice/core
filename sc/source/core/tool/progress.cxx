/*************************************************************************
 *
 *  $RCSfile: progress.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/progress.hxx>

#define SC_PROGRESS_CXX
#include "progress.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globstr.hrc"



ScProgress theDummyInterpretProgress;
SfxProgress*    ScProgress::pGlobalProgress = NULL;
ULONG           ScProgress::nGlobalRange = 0;
ULONG           ScProgress::nGlobalPercent = 0;
BOOL            ScProgress::bGlobalNoUserBreak = TRUE;
ScProgress*     ScProgress::pInterpretProgress = &theDummyInterpretProgress;
ScProgress*     ScProgress::pOldInterpretProgress = NULL;
ULONG           ScProgress::nInterpretProgress = 0;
BOOL            ScProgress::bAllowInterpretProgress = TRUE;
ScDocument*     ScProgress::pInterpretDoc;
BOOL            ScProgress::bIdleWasDisabled = FALSE;



ScProgress::ScProgress( SfxObjectShell* pObjSh, const String& rText,
                        ULONG nRange, BOOL bAllDocs, BOOL bWait )
{

    if ( pGlobalProgress || SfxProgress::GetActiveProgress( NULL ) )
    {
        DBG_ERROR( "ScProgress: es kann nur einen geben!" );
        pProgress = NULL;
    }
    else if ( SFX_APP()->IsDowning() )
    {
        //  kommt vor z.B. beim Speichern des Clipboard-Inhalts als OLE beim Beenden
        //  Dann wuerde ein SfxProgress wild im Speicher rummuellen
        //! Soll das so sein ???

        pProgress = NULL;
    }
    else if ( pObjSh && ( pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
                          pObjSh->GetProgress() ) )
    {
        //  #62808# no own progress for embedded objects,
        //  #73633# no second progress if the document already has one

        pProgress = NULL;
    }
    else
    {
        pProgress = new SfxProgress( pObjSh, rText, nRange, bAllDocs, bWait );
        pGlobalProgress = pProgress;
        nGlobalRange = nRange;
        nGlobalPercent = 0;
        bGlobalNoUserBreak = TRUE;
    }
}


ScProgress::ScProgress() :
        pProgress( NULL )
{   // DummyInterpret
}


ScProgress::~ScProgress()
{
    if ( pProgress )
    {
        delete pProgress;
        pGlobalProgress = NULL;
        nGlobalRange = 0;
        nGlobalPercent = 0;
        bGlobalNoUserBreak = TRUE;
    }
}


// static

void ScProgress::SetAllowInterpret( BOOL bAllow )
{   // Grundzustand: Allow, Dummy gesetzt, pOld==NULL
    if ( !bAllow && bAllowInterpretProgress )
    {   // vorherigen/Dummy merken und Dummy setzen
        pOldInterpretProgress = pInterpretProgress;
        pInterpretProgress = &theDummyInterpretProgress;
        bAllowInterpretProgress = FALSE;
    }
    else if ( bAllow && !bAllowInterpretProgress )
    {   // Dummy weg und vorherigen/Dummy setzen
        pInterpretProgress = pOldInterpretProgress;
        pOldInterpretProgress = NULL;
        bAllowInterpretProgress = TRUE;
    }
}


// static

void ScProgress::CreateInterpretProgress( ScDocument* pDoc, BOOL bWait )
{
    if ( bAllowInterpretProgress )
    {
        if ( nInterpretProgress )
            nInterpretProgress++;
        else if ( pDoc->GetAutoCalc() )
        {
            bIdleWasDisabled = pDoc->IsIdleDisabled();
            pDoc->DisableIdle( TRUE );
            pInterpretProgress = new ScProgress( pDoc->GetDocumentShell(),
                ScGlobal::GetRscString( STR_PROGRESS_CALCULATING ),
                pDoc->GetFormulaCodeInTree(), FALSE, bWait );
            pInterpretDoc = pDoc;
            nInterpretProgress = 1;
        }
    }
}


// static

void ScProgress::DeleteInterpretProgress()
{
    if ( bAllowInterpretProgress && nInterpretProgress )
    {
        if ( --nInterpretProgress == 0 )
        {
            if ( pInterpretProgress != &theDummyInterpretProgress )
            {
                delete pInterpretProgress;
                pInterpretProgress = &theDummyInterpretProgress;
                pInterpretDoc->DisableIdle( bIdleWasDisabled );
            }
            else
            {
                DBG_ASSERT( pInterpretProgress != &theDummyInterpretProgress,
                    "DeleteInterpretProgress: Dummy loeschen?!?"  )
            }
        }
    }
}



