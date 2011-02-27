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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

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

    for (USHORT nFactory=0; nFactory<pAppData_Impl->pFactArr->Count(); ++nFactory)
    {
        if (pFact->nId ==  (*pAppData_Impl->pFactArr)[nFactory]->nId)
        {
            pAppData_Impl->pFactArr->Remove( nFactory );
        }
    }

    pAppData_Impl->pFactArr->C40_INSERT(
        SfxChildWinFactory, pFact, pAppData_Impl->pFactArr->Count() );
}

void SfxApplication::RegisterChildWindowContext_Impl( SfxModule *pMod, USHORT nId,
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
            USHORT nCount = pFactories->Count();
            for (USHORT nFactory=0; nFactory<nCount; ++nFactory)
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
        USHORT nCount = pFactories->Count();
        for (USHORT nFactory=0; nFactory<nCount; ++nFactory)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
