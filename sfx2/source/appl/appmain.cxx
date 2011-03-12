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

#define _SDINTERN_HXX

#include <stdio.h>
#include <tools/urlobj.hxx>
#include <tools/config.hxx>
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
#include "intro.hxx"
#include <sfx2/msgpool.hxx>
#include <sfx2/mnumgr.hxx>
#include <sfx2/appuno.hxx>
#include "app.hrc"
#include <sfx2/docfile.hxx>
#include "workwin.hxx"

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif


//===================================================================

DBG_NAME(SfxAppMainNewMenu)
DBG_NAME(SfxAppMainBmkMenu)
DBG_NAME(SfxAppMainWizMenu)
DBG_NAME(SfxAppMainOLEReg)
DBG_NAME(SfxAppMainCHAOSReg)

//===================================================================

#define SFX_TEMPNAMEBASE_DIR    "soffice.tmp"
#define SFX_KEY_TEMPNAMEBASE    "Temp-Dir"

//===================================================================

#ifdef TF_POOLABLE
static SfxItemInfo const aItemInfos[] =
{
    { 0, 0 }
};
#endif

//===================================================================

TYPEINIT2(SfxApplication,SfxShell,SfxBroadcaster);

//--------------------------------------------------------------------
void SfxApplication::Init
(
)

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx aus Application:a:Main() gerufen,
    bevor Execute() ausgef"uhrt wird und
    - das Intro bereits angezeigt ist,
    - das Applikationsfenster exisitiert, aber noch hidden ist,
    - die Bindings bereits existieren (Controller sind anmeldbar),
    - der Ini- und Config-Manager bereits existiert,
    - die Standard-Controller bereits exisitieren,
    - die SFx-Shells ihre Interfaces bereits registriert haben.

    [Querverweise]
    <SfxApplication::Exit()>
    <SfxApplication::OpenClients()>
*/
{
}

//--------------------------------------------------------------------

void SfxApplication::Exit()

/*  [Beschreibung]

    Diese virtuelle Methode wird vom SFx aus Application::Main() gerufen,
    nachdem Execute() beendet ist und
    - die Konfiguration (SfxConfigManager) bereits gespeichert wurde,
    - die Fensterpostionen etc. in den SfxIniManager geschrieben wurden,
    - das Applikationsfenster noch existiert, aber hidden ist
    - s"amtliche Dokumente und deren Views bereits geschlossen sind.
    - Dispatcher, Bindings etc. bereits zerst"ort sind

    [Querverweise]
    <SfxApplication::Init(int,char*[])>
*/

{
}

//---------------------------------------------------------------------------

void SfxApplication::PreInit( )
{
}

void SfxApplication::Main( )
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
