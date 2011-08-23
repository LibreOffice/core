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

//#define TF_NEWDESKTOP

#define _SDINTERN_HXX

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _CSTITEM_HXX //autogen
#include <bf_svtools/cstitem.hxx>
#endif
#ifndef _CONFIG_HXX
#include <tools/config.hxx>
#endif
#ifndef _EHDL_HXX
#include <bf_svtools/ehdl.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_STARTOPTIONS_HXX
#include <bf_svtools/startoptions.hxx>
#endif
#include <bf_svtools/itempool.hxx>
#include <bf_svtools/urihelper.hxx>
#include <bf_svtools/helpopt.hxx>
#include <vos/process.hxx>

#include "appimp.hxx"
#include "sfxtypes.hxx"
#include "appdata.hxx"
#include "docfac.hxx"
#include "app.hxx"
#include "interno.hxx"
#include "fcontnr.hxx"
#include "cfgmgr.hxx"
#include "appuno.hxx"
#include "objuno.hxx"
#include "docfile.hxx"

#ifdef WNT
#include <tools/svwin.h>
#endif
namespace binfilter {

#ifdef UNX
#define stricmp(a,b) strcmp(a,b)
#endif


//===================================================================

/*DBG_NAME(SfxAppMainIntro)
DBG_NAME(SfxAppMainSO_Init)
DBG_NAME(SfxAppMainAppRes)
DBG_NAME(SfxAppMainInit0)
DBG_NAME(SfxAppMainCreateAppWin)
DBG_NAME(SfxAppMainInit1)
DBG_NAME(SfxAppMainCfgMgr)
DBG_NAME(SfxAppMainInitController)
DBG_NAME(SfxAppMainInitException)
DBG_NAME(SfxAppMainRegisterIF)
DBG_NAME(SfxAppMainInit)
DBG_NAME(SfxAppMainLoadBasMgr)
DBG_NAME(SfxAppMainSbxInit)*/
/*DBG_NAME(SfxAppMainInitDispatcher)
DBG_NAME(SfxAppMainLoadConfig)
DBG_NAME(SfxAppMainInitAppWin)
DBG_NAME(SfxAppMainAppEvents)*/

//===================================================================

#define SFX_TEMPNAMEBASE_DIR	"soffice.tmp"
#define SFX_KEY_TEMPNAMEBASE	"Temp-Dir"

//===================================================================

#ifdef _MSC_VER
#pragma code_seg("STATICS")
#endif
static SfxVoidItem aStaticDefault(1);
#ifdef _MSC_VER
#pragma code_seg()
#endif

static SfxPoolItem* aStaticDefaults[1] =
{
    &aStaticDefault
};

#ifdef TF_POOLABLE
static SfxItemInfo __READONLY_DATA aItemInfos[] =
{
    { 0, 0 }
};
#endif

//===================================================================

typedef Link* LinkPtr;
SV_DECL_PTRARR(SfxInitLinkList, LinkPtr, 4, 4)//STRIP008 ;

/*N*/ TYPEINIT1(SfxSysChangeHint, SfxHint);
/*N*/ TYPEINIT2(SfxApplication,SfxShell,SfxBroadcaster);

//--------------------------------------------------------------------


//--------------------------------------------------------------------
/*N*/ void SfxApplication::Init
/*N*/ (
/*N*/ )
/*N*/ 
/*	[Beschreibung]

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
/*N*/ {
/*N*/ }

//--------------------------------------------------------------------

/*N*/ void SfxApplication::Exit()
/*N*/ 
/*	[Beschreibung]

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
/*N*/ 
/*N*/ {
/*N*/ }

//---------------------------------------------------------------------------

/*N*/ void SfxApplication::PreInit( )
/*N*/ {
/*N*/ }


void SfxApplication::Main( )
{
}

//-------------------------------------------------------------------------

/*N*/ SfxFilterMatcher& SfxApplication::GetFilterMatcher()
/*N*/ {
/*N*/ 	if( !pAppData_Impl->pMatcher )
/*N*/ 	{
/*N*/ 		pAppData_Impl->pMatcher = new SfxFilterMatcher( TRUE );
/*N*/         ::binfilter::SetMaybeFileHdl( STATIC_LINK(
/*N*/ 			pAppData_Impl->pMatcher, SfxFilterMatcher, MaybeFileHdl_Impl ) );
/*N*/ 	}
/*N*/ 	return *pAppData_Impl->pMatcher;
/*N*/ }

//-------------------------------------------------------------------------

}
