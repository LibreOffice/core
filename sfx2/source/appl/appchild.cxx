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


#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <sfx2/app.hxx>
#include <appdata.hxx>
#include <workwin.hxx>
#include <childwinimpl.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>


void SfxApplication::RegisterChildWindow_Impl( SfxModule *pMod, std::unique_ptr<SfxChildWinFactory> pFact )
{
    if ( pMod )
    {
        pMod->RegisterChildWindow( std::move(pFact) );
        return;
    }

    if (!pImpl->pFactArr)
        pImpl->pFactArr.reset(new SfxChildWinFactArr_Impl);

    for (size_t nFactory=0; nFactory<pImpl->pFactArr->size(); ++nFactory)
    {
        if (pFact->nId ==  (*pImpl->pFactArr)[nFactory].nId)
        {
            pImpl->pFactArr->erase( pImpl->pFactArr->begin() + nFactory );
        }
    }

    pImpl->pFactArr->push_back( std::move(pFact) );
}

void SfxApplication::RegisterChildWindowContext_Impl( SfxModule *pMod, sal_uInt16 nId,
        std::unique_ptr<SfxChildWinContextFactory> pFact)
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
                    // Factory found, register Context here.
                    pF = pFac;
                    break;
                }
            }
        }
    }

    if ( !pF )
    {
        // Search for Factory in the Application
        DBG_ASSERT( pImpl, "No AppData!" );
        DBG_ASSERT( pImpl->pFactArr, "No Factories!" );

        pFactories = pImpl->pFactArr.get();
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
                    pMod->RegisterChildWindow( std::unique_ptr<SfxChildWinFactory>(pF) );
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
            pF->pArr.reset( new SfxChildWinContextArr_Impl );
        pF->pArr->push_back( std::move(pFact) );
        return;
    }

    OSL_FAIL( "No ChildWindow for this Context!" );
}


SfxChildWinFactArr_Impl& SfxApplication::GetChildWinFactories_Impl() const
{
    return ( *(pImpl->pFactArr));
}


SfxWorkWindow* SfxApplication::GetWorkWindow_Impl(const SfxViewFrame *pFrame) const
{
    if ( pFrame )
        return pFrame->GetFrame().GetWorkWindow_Impl();
    else if ( pImpl->pViewFrame )
        return pImpl->pViewFrame->GetFrame().GetWorkWindow_Impl();
    else
        return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
