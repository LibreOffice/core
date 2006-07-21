/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progress.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:39:26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



//------------------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <svtools/eitem.hxx>
#include <svtools/itemset.hxx>

#define SC_PROGRESS_CXX
#include "progress.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globstr.hrc"



static ScProgress theDummyInterpretProgress;
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


BOOL lcl_IsHiddenDocument( SfxObjectShell* pObjSh )
{
    if (pObjSh)
    {
        SfxMedium* pMed = pObjSh->GetMedium();
        if (pMed)
        {
            SfxItemSet* pSet = pMed->GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_HIDDEN, TRUE, &pItem ) &&
                        ((const SfxBoolItem*)pItem)->GetValue() )
                return TRUE;
        }
    }
    return FALSE;
}

ScProgress::ScProgress( SfxObjectShell* pObjSh, const String& rText,
                        ULONG nRange, BOOL bAllDocs, BOOL bWait )
{

    if ( pGlobalProgress || SfxProgress::GetActiveProgress( NULL ) )
    {
        if ( lcl_IsHiddenDocument(pObjSh) )
        {
            // loading a hidden document while a progress is active is possible - no error
            pProgress = NULL;
        }
        else
        {
            DBG_ERROR( "ScProgress: there can be only one!" );
            pProgress = NULL;
        }
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
            nInterpretProgress = 1;
            bIdleWasDisabled = pDoc->IsIdleDisabled();
            pDoc->DisableIdle( TRUE );
            // Interpreter may be called in many circumstances, also if another
            // progress bar is active, for example while adapting row heights.
            // Keep the dummy interpret progress.
            if ( !pGlobalProgress )
                pInterpretProgress = new ScProgress( pDoc->GetDocumentShell(),
                    ScGlobal::GetRscString( STR_PROGRESS_CALCULATING ),
                    pDoc->GetFormulaCodeInTree(), FALSE, bWait );
            pInterpretDoc = pDoc;
        }
    }
}


// static

void ScProgress::DeleteInterpretProgress()
{
    if ( bAllowInterpretProgress && nInterpretProgress )
    {
        /*  Do not decrement 'nInterpretProgress', before 'pInterpretProgress'
            is deleted. In rare cases, deletion of 'pInterpretProgress' causes
            a refresh of the sheet window which may call CreateInterpretProgress
            and DeleteInterpretProgress again (from Output::DrawStrings),
            resulting in double deletion of 'pInterpretProgress'. */
//       if ( --nInterpretProgress == 0 )
        if ( nInterpretProgress == 1 )
        {
            if ( pInterpretProgress != &theDummyInterpretProgress )
            {
                // move pointer to local temporary to avoid double deletion
                ScProgress* pTmpProgress = pInterpretProgress;
                pInterpretProgress = &theDummyInterpretProgress;
                delete pTmpProgress;
            }
            if ( pInterpretDoc )
                pInterpretDoc->DisableIdle( bIdleWasDisabled );
        }
        --nInterpretProgress;
    }
}



