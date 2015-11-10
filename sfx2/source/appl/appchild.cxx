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


#include <svl/whiter.hxx>
#include <svl/eitem.hxx>

#include <sfx2/app.hxx>
#include "appdata.hxx"
#include "workwin.hxx"
#include "childwinimpl.hxx"
#include <sfx2/childwin.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include "sfxtypes.hxx"
#include <sfx2/module.hxx>
#include <sfx2/sfxsids.hrc>




void SfxApplication::RegisterChildWindow_Impl( SfxModule *pMod, SfxChildWinFactory *pFact )
{
    if ( pMod )
    {
        pMod->RegisterChildWindow( pFact );
        return;
    }

    if (!pAppData_Impl->pFactArr)
        pAppData_Impl->pFactArr = new SfxChildWinFactArr_Impl;

    for (size_t nFactory=0; nFactory<pAppData_Impl->pFactArr->size(); ++nFactory)
    {
        if (pFact->nId ==  (*pAppData_Impl->pFactArr)[nFactory].nId)
        {
            pAppData_Impl->pFactArr->erase( pAppData_Impl->pFactArr->begin() + nFactory );
        }
    }

    pAppData_Impl->pFactArr->push_back( pFact );
}

void SfxApplication::RegisterChildWindowContext_Impl( SfxModule *pMod, sal_uInt16 nId,
        SfxChildWinContextFactory *pFact)
{
    SfxChildWinFactArr_Impl *pFactories;
    SfxChildWinFactory *pF = nullptr;
    if ( pMod )
    {
        // Abandon Module, search there for ChildwindowFactory
        pFactories = pMod->GetChildWinFactories_Impl();
        if ( pFactories )
        {
            sal_uInt16 nCount = pFactories->size();
            for (sal_uInt16 nFactory=0; nFactory<nCount; ++nFactory)
            {
                SfxChildWinFactory *pFac = &(*pFactories)[nFactory];
                if ( nId == pFac->nId )
                {
                    // Factory found, registrer Context here.
                    pF = pFac;
                    break;
                }
            }
        }
    }

    if ( !pF )
    {
        // Search for Factory in the Application
        DBG_ASSERT( pAppData_Impl, "No AppData!" );
        DBG_ASSERT( pAppData_Impl->pFactArr, "No Factories!" );

        pFactories = pAppData_Impl->pFactArr;
        sal_uInt16 nCount = pFactories->size();
        for (sal_uInt16 nFactory=0; nFactory<nCount; ++nFactory)
        {
            SfxChildWinFactory *pFac = &(*pFactories)[nFactory];
            if ( nId == pFac->nId )
            {
                if ( pMod )
                {
                    // If the context of a module has been registered, then the
                    // ChildWindowFactory must also be available there,
                    // else the ContextFactory would have be unsubscribed on
                    // DLL-exit
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
        pF->pArr->push_back( pFact );
        return;
    }

    OSL_FAIL( "No ChildWindow for this Context!" );
}



SfxChildWinFactArr_Impl& SfxApplication::GetChildWinFactories_Impl() const
{
    return ( *(pAppData_Impl->pFactArr));
}


SfxWorkWindow* SfxApplication::GetWorkWindow_Impl(const SfxViewFrame *pFrame) const
{
    if ( pFrame )
        return pFrame->GetFrame().GetWorkWindow_Impl();
    else if ( pAppData_Impl->pViewFrame )
        return pAppData_Impl->pViewFrame->GetFrame().GetWorkWindow_Impl();
    else
        return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
