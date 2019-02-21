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

#include <memory>
#include <map>

#include <osl/module.hxx>
#include <tools/svlibrary.h>
#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/itemset.hxx>

#include <DrawDocShell.hxx>

#include <pres.hxx>
#include <sdfilter.hxx>


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;


SdFilter::SdFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell )
:   mxModel( rDocShell.GetModel() )
,   mrMedium( rMedium )
,   mrDocShell( rDocShell )
,   mrDocument( *rDocShell.GetDoc() )
,   mbIsDraw( rDocShell.GetDocumentType() == DocumentType::Draw )
{
}

SdFilter::~SdFilter()
{
}

OUString SdFilter::ImplGetFullLibraryName( const OUString& rLibraryName )
{
    return OUString(SVLIBRARY("?")).replaceFirst( "?", rLibraryName );
}

#ifndef DISABLE_DYNLOADING

typedef std::map<OUString, std::unique_ptr<osl::Module>> SdModuleMap;
static SdModuleMap g_SdModuleMap;

extern "C" { static void thisModule() {} }

oslGenericFunction SdFilter::GetLibrarySymbol( const OUString& rLibraryName, const OUString &rFnSymbol )
{
    osl::Module *pMod = nullptr;
    auto it = g_SdModuleMap.find(rLibraryName);
    if (it != g_SdModuleMap.end())
        pMod = it->second.get();

    if (!pMod)
    {
        pMod = new osl::Module;
        if (pMod->loadRelative(&thisModule, ImplGetFullLibraryName(rLibraryName),
                               SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY))
            g_SdModuleMap[rLibraryName] = std::unique_ptr<osl::Module>(pMod);
        else
        {
            delete pMod;
            pMod = nullptr;
        }
    }
    if (!pMod)
        return nullptr;
    else
        return pMod->getFunctionSymbol(rFnSymbol);
}

void SdFilter::Preload()
{
    (void)GetLibrarySymbol("sdfilt", "ImportPPT");
    (void)GetLibrarySymbol("icg", "ImportCGM");
}

#endif

void SdFilter::CreateStatusIndicator()
{
    // The status indicator must be retrieved from the provided medium arguments
    const SfxUnoAnyItem* pStatusBarItem = static_cast<const SfxUnoAnyItem*>(
            mrMedium.GetItemSet()->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );

    if ( pStatusBarItem )
        pStatusBarItem->GetValue() >>= mxStatusIndicator;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
