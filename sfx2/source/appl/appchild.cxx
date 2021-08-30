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
#include <sfx2/childwin.hxx>
#include <sfx2/module.hxx>
#include <sfx2/viewfrm.hxx>


void SfxApplication::RegisterChildWindow_Impl( SfxModule *pMod, const SfxChildWinFactory& rFact )
{
    if ( pMod )
    {
        pMod->RegisterChildWindow( rFact );
        return;
    }

    for (size_t nFactory=0; nFactory<pImpl->maFactories.size(); ++nFactory)
    {
        if (rFact.nId == pImpl->maFactories[nFactory].nId)
        {
            pImpl->maFactories.erase( pImpl->maFactories.begin() + nFactory );
        }
    }

    pImpl->maFactories.push_back( rFact );
}

SfxChildWinFactory* SfxApplication::GetChildWinFactoryById(sal_uInt16 nId) const
{
    for (auto& rFactory : pImpl->maFactories)
        if (rFactory.nId == nId)
            return &rFactory;
    return nullptr;
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
