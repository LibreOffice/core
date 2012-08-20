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


#define _SDINTERN_HXX

#include <stdio.h>
#include <svtools/ehdl.hxx>
#include <unotools/startoptions.hxx>
#include <svl/itempool.hxx>
#include <svl/urihelper.hxx>
#include <svtools/helpopt.hxx>
#include <framework/sfxhelperfunctions.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "sfxtypes.hxx"
#include "appdata.hxx"
#include <sfx2/docfac.hxx>
#include <sfx2/app.hxx>
#include "arrdecl.hxx"
#include <sfx2/dispatch.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/fcontnr.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/appuno.hxx>
#include "app.hrc"
#include <sfx2/docfile.hxx>
#include "workwin.hxx"

DBG_NAME(SfxAppMainNewMenu)
DBG_NAME(SfxAppMainBmkMenu)
DBG_NAME(SfxAppMainWizMenu)
DBG_NAME(SfxAppMainOLEReg)
DBG_NAME(SfxAppMainCHAOSReg)

//===================================================================

TYPEINIT2(SfxApplication,SfxShell,SfxBroadcaster);

//--------------------------------------------------------------------
void SfxApplication::Init
(
)

/*  [Description]

    This virtual method is called from SFx through Application::Main(),
    before Execute() is called and:
    - the Intro is already displayed,
    - the Applications window exists, but it is still hidden,
    - the Bindings already exist (Controller can be registered),
    - the Init and Config-Manager already exists,
    - the Standard-Controller already exists,
    - the SFx-Shells have alredy registered their Interfaces.

    [Cross-reference]

    <SfxApplication::Exit()>
    <SfxApplication::OpenClients()>
*/
{
}

//--------------------------------------------------------------------

void SfxApplication::Exit()

/*  [Description]

    This virtual method is called from SFx through Application::Main(),
    after Execute() has finished and
    - the configuration (SfxConfigManager) was already saved,
    - the window postions etc. in the SfxIniManager were written,
    - the Application widow still exists, but is hidden
    - all Documents and their Views already are closed.
    - Dispatcher, Bindings etc. already destroyed.

    [Cross-reference]
    <SfxApplication::Init(int,char*[])>
*/

{
}

//-------------------------------------------------------------------------

SfxFilterMatcher& SfxApplication::GetFilterMatcher()
{
    if( !pAppData_Impl->pMatcher )
    {
        pAppData_Impl->pMatcher = new SfxFilterMatcher();
        URIHelper::SetMaybeFileHdl( STATIC_LINK(
            pAppData_Impl->pMatcher, SfxFilterMatcher, MaybeFileHdl_Impl ) );
    }
    return *pAppData_Impl->pMatcher;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
