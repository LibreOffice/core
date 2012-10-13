/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/progress.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>

#define SC_PROGRESS_CXX
#include "progress.hxx"
#include "document.hxx"
#include "global.hxx"
#include "globstr.hrc"

using namespace com::sun::star;


static ScProgress theDummyInterpretProgress;
SfxProgress*    ScProgress::pGlobalProgress = NULL;
sal_uLong           ScProgress::nGlobalRange = 0;
sal_uLong           ScProgress::nGlobalPercent = 0;
sal_Bool            ScProgress::bGlobalNoUserBreak = sal_True;
ScProgress*     ScProgress::pInterpretProgress = &theDummyInterpretProgress;
ScProgress*     ScProgress::pOldInterpretProgress = NULL;
sal_uLong           ScProgress::nInterpretProgress = 0;
sal_Bool            ScProgress::bAllowInterpretProgress = sal_True;
ScDocument*     ScProgress::pInterpretDoc;
sal_Bool            ScProgress::bIdleWasDisabled = false;


static sal_Bool lcl_IsHiddenDocument( SfxObjectShell* pObjSh )
{
    if (pObjSh)
    {
        SfxMedium* pMed = pObjSh->GetMedium();
        if (pMed)
        {
            SfxItemSet* pSet = pMed->GetItemSet();
            const SfxPoolItem* pItem;
            if ( pSet && SFX_ITEM_SET == pSet->GetItemState( SID_HIDDEN, sal_True, &pItem ) &&
                        ((const SfxBoolItem*)pItem)->GetValue() )
                return sal_True;
        }
    }
    return false;
}

static bool lcl_HasControllersLocked( SfxObjectShell& rObjSh )
{
    uno::Reference<frame::XModel> xModel( rObjSh.GetBaseModel() );
    if (xModel.is())
        return xModel->hasControllersLocked();
    return false;
}

ScProgress::ScProgress( SfxObjectShell* pObjSh, const String& rText,
                        sal_uLong nRange, sal_Bool bAllDocs, sal_Bool bWait )
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
            OSL_FAIL( "ScProgress: there can be only one!" );
            pProgress = NULL;
        }
    }
    else if ( SFX_APP()->IsDowning() )
    {
        //  This happens. E.g. when saving the clipboard-content as OLE when closing the app.
        //  In this case a SfxProgress would produce dirt in memory.
        //! Should that be this way ???

        pProgress = NULL;
    }
    else if ( pObjSh && ( pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
                          pObjSh->GetProgress() ||
                          lcl_HasControllersLocked(*pObjSh) ) )
    {
        //  no own progress for embedded objects,
        //  no second progress if the document already has one

        pProgress = NULL;
    }
    else
    {
        pProgress = new SfxProgress( pObjSh, rText, nRange, bAllDocs, bWait );
        pGlobalProgress = pProgress;
        nGlobalRange = nRange;
        nGlobalPercent = 0;
        bGlobalNoUserBreak = sal_True;
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
        bGlobalNoUserBreak = sal_True;
    }
}


void ScProgress::CreateInterpretProgress( ScDocument* pDoc, sal_Bool bWait )
{
    if ( bAllowInterpretProgress )
    {
        if ( nInterpretProgress )
            nInterpretProgress++;
        else if ( pDoc->GetAutoCalc() )
        {
            nInterpretProgress = 1;
            bIdleWasDisabled = pDoc->IsIdleDisabled();
            pDoc->DisableIdle( sal_True );
            // Interpreter may be called in many circumstances, also if another
            // progress bar is active, for example while adapting row heights.
            // Keep the dummy interpret progress.
            if ( !pGlobalProgress )
                pInterpretProgress = new ScProgress( pDoc->GetDocumentShell(),
                    ScGlobal::GetRscString( STR_PROGRESS_CALCULATING ),
                    pDoc->GetFormulaCodeInTree()/MIN_NO_CODES_PER_PROGRESS_UPDATE, false, bWait );
            pInterpretDoc = pDoc;
        }
    }
}



void ScProgress::DeleteInterpretProgress()
{
    if ( bAllowInterpretProgress && nInterpretProgress )
    {
        /*  Do not decrement 'nInterpretProgress', before 'pInterpretProgress'
            is deleted. In rare cases, deletion of 'pInterpretProgress' causes
            a refresh of the sheet window which may call CreateInterpretProgress
            and DeleteInterpretProgress again (from Output::DrawStrings),
            resulting in double deletion of 'pInterpretProgress'. */
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
