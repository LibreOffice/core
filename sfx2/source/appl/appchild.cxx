/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#ifndef GCC
#endif
#include <svl/whiter.hxx>
#include <svl/eitem.hxx>

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "workwin.hxx"
#include <sfx2/childwin.hxx>
#include "arrdecl.hxx"
#include <sfx2/templdlg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/module.hxx>
#include <sfx2/sfxsids.hrc>

//=========================================================================


void SfxApplication::RegisterChildWindow_Impl( SfxModule *pMod, SfxChildWinFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterChildWindow( pFact );
        return;
    }

    if (!pAppData_Impl->pFactArr)
        pAppData_Impl->pFactArr = new SfxChildWinFactArr_Impl;

//#ifdef DBG_UTIL
    for (sal_uInt16 nFactory=0; nFactory<pAppData_Impl->pFactArr->Count(); ++nFactory)
    {
        if (pFact->nId ==  (*pAppData_Impl->pFactArr)[nFactory]->nId)
        {
            pAppData_Impl->pFactArr->Remove( nFactory );
//          DBG_ERROR("ChildWindow mehrfach registriert!");
//          return;
        }
    }
//#endif

    pAppData_Impl->pFactArr->C40_INSERT(
        SfxChildWinFactory, pFact, pAppData_Impl->pFactArr->Count() );
}

void SfxApplication::RegisterChildWindowContext_Impl( SfxModule *pMod, sal_uInt16 nId,
        SfxChildWinContextFactory *pFact)
{
    SfxChildWinFactArr_Impl *pFactories;
    SfxChildWinFactory *pF = NULL;
    if ( pMod )
    {
        // Modul "ubergeben, ChildwindowFactory dort suchen
        pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            sal_uInt16 nCount = pFactories->Count();
            for (sal_uInt16 nFactory=0; nFactory<nCount; ++nFactory)
            {
                SfxChildWinFactory *pFac = (*pFactories)[nFactory];
                if ( nId == pFac->nId )
                {
                    // Factory gefunden, Context dort registrieren
                    pF = pFac;
                    break;
                }
            }
        }
    }

    if ( !pF )
    {
        // Factory an der Application suchen
        DBG_ASSERT( pAppData_Impl, "Keine AppDaten!" );
        DBG_ASSERT( pAppData_Impl->pFactArr, "Keine Factories!" );

        pFactories = pAppData_Impl->pFactArr;
        sal_uInt16 nCount = pFactories->Count();
        for (sal_uInt16 nFactory=0; nFactory<nCount; ++nFactory)
        {
            SfxChildWinFactory *pFac = (*pFactories)[nFactory];
            if ( nId == pFac->nId )
            {
                if ( pMod )
                {
                    // Wenn der Context von einem Modul registriert wurde,
                    // mu\s die ChildwindowFactory auch dort zur Verf"ugung
                    // stehen, sonst m"u\ste sich die Contextfactory im DLL-Exit
                    // wieder abmelden !
                    pF = new SfxChildWinFactory( pFac->pCtor, pFac->nId,
                            pFac->nPos );
                    pMod->RegisterChildWindow( pF );
                }
                else
                    pF = pFac;
                break;
            }
        }
    }

    if ( pF )
    {
        if ( !pF->pArr )
            pF->pArr = new SfxChildWinContextArr_Impl;
        pF->pArr->C40_INSERT( SfxChildWinContextFactory, pFact, pF->pArr->Count() );
        return;
    }

    DBG_ERROR( "Kein ChildWindow fuer diesen Context!" );
}

//--------------------------------------------------------------------

SfxChildWinFactArr_Impl& SfxApplication::GetChildWinFactories_Impl() const
{
    return ( *(pAppData_Impl->pFactArr));
}

//--------------------------------------------------------------------

SfxTemplateDialog* SfxApplication::GetTemplateDialog()
{
    if ( pAppData_Impl->pViewFrame )
    {
        SfxChildWindow *pChild = pAppData_Impl->pViewFrame->GetChildWindow(SfxTemplateDialogWrapper::GetChildWindowId());
        return pChild ? (SfxTemplateDialog*) pChild->GetWindow() : 0;
    }

    return NULL;
}

//--------------------------------------------------------------------

SfxWorkWindow* SfxApplication::GetWorkWindow_Impl(const SfxViewFrame *pFrame) const
{
    if ( pFrame )
        return pFrame->GetFrame().GetWorkWindow_Impl();
    else if ( pAppData_Impl->pViewFrame )
        return pAppData_Impl->pViewFrame->GetFrame().GetWorkWindow_Impl();
    else
        return NULL;
}

