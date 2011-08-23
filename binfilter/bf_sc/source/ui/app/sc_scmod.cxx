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

#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <comphelper/processfactory.hxx>

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"

#include <bf_svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <tools/pstm.hxx>

#include <bf_svtools/poolitem.hxx>

#include <bf_svx/itemdata.hxx>

#include <tools/date.hxx>

#include <tools/time.hxx>

#include <bf_svx/flditem.hxx>
#include <bf_svx/outliner.hxx>
#include "bf_basic/sbstar.hxx"

#include <bf_svtools/ehdl.hxx>
#include <bf_svtools/ctloptions.hxx>
#include <vcl/status.hxx>
#include <bf_sfx2/request.hxx>

#include <bf_sfx2/macrconf.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/langitem.hxx>
#include <bf_svtools/colorcfg.hxx>

#include <bf_svtools/whiter.hxx>
#include <bf_offmgr/app.hxx>
#include <vcl/msgbox.hxx>
#include <bf_offmgr/ofaids.hrc>
#include <bf_svtools/inethist.hxx>
#include <vcl/waitobj.hxx>

#define ITEMID_SPELLCHECK 0

#include "scmod.hxx"
#include "global.hxx"
#include "viewopti.hxx"
#include "docoptio.hxx"
#include "appoptio.hxx"
#include "inputopt.hxx"
#include "printopt.hxx"
#include "navicfg.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "uiitems.hxx"
#include "bf_sc.hrc"
#include "cfgids.hxx"

#include "msgpool.hxx"
#include "scresid.hxx"
#include "teamdlg.hxx"
#include "formdata.hxx"
#include "tpusrlst.hxx"
#include "tpcalc.hxx"
#include "tpprint.hxx"
#include "detfunc.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#define SC_IDLE_MIN		150
#define SC_IDLE_MAX		3000
#define SC_IDLE_STEP	75
#define SC_IDLE_COUNT	50
namespace binfilter {

static USHORT nIdleCount = 0;

//------------------------------------------------------------------

/*N*/ ScModule::ScModule( SfxObjectFactory* pFact ) :
/*N*/ 	ScModuleDummy( SFX_APP()->CreateResManager( "bf_sc" ), FALSE, pFact ), //STRIP005
/*N*/ 	bIsWaterCan( FALSE ),
/*N*/ 	bIsInEditCommand( FALSE ),
/*N*/ 	pRefInputHandler( NULL ),
/*N*/ 	pViewCfg( NULL ),
/*N*/ 	pDocCfg( NULL ),
/*N*/ 	pAppCfg( NULL ),
/*N*/ 	pInputCfg( NULL ),
/*N*/ 	pPrintCfg( NULL ),
/*N*/ 	pNavipiCfg( NULL ),
/*N*/ 	pColorConfig( NULL ),
/*N*/ 	pCTLOptions( NULL ),
/*N*/ 	pTeamDlg( NULL ),
/*N*/ 	nCurRefDlgId( 0 ),
/*N*/ 	pErrorHdl( NULL ),
/*N*/ 	pSvxErrorHdl( NULL ),
/*N*/ 	pMessagePool( NULL ),
/*N*/ 	pFormEditData( NULL )
/*N*/ {
/*N*/ 	//	im ctor ist der ResManager (DLL-Daten) noch nicht initialisiert!
/*N*/
/*N*/ 	SetName(String::CreateFromAscii(RTL_CONSTASCII_STRINGPARAM("StarCalc")));		// fuer Basic
/*N*/
/*N*/ 	ResetDragObject();
/*N*/
/*N*/ 	//	InputHandler braucht nicht mehr angelegt zu werden
/*N*/
/*N*/ 	//	ErrorHandler anlegen - war in Init()
/*N*/ 	//	zwischen OfficeApplication::Init und ScGlobal::Init
/*N*/ //	pSvxErrorHdl = new SvxErrorHandler();
/*N*/ 	pErrorHdl	 = new SfxErrorHandler( RID_ERRHDLSC,
/*N*/ 										ERRCODE_AREA_SC,
/*N*/ 										ERRCODE_AREA_APP2-1,
/*N*/ 										GetResMgr() );
/*N*/
/*N*/ 	aSpellTimer.SetTimeout(10);
/*N*/ 	aSpellTimer.SetTimeoutHdl( LINK( this, ScModule, SpellTimerHdl ) );
/*N*/ 	aIdleTimer.SetTimeout(SC_IDLE_MIN);
/*N*/ 	aIdleTimer.SetTimeoutHdl( LINK( this, ScModule, IdleHandler ) );
/*N*/ 	aIdleTimer.Start();
/*N*/
/*N*/ 	pMessagePool = new ScMessagePool;
/*N*/ 	pMessagePool->FreezeIdRanges();
/*N*/ 	SetPool( pMessagePool );
/*N*/ 	ScGlobal::InitTextHeight( pMessagePool );
/*N*/
/*N*/ 	StartListening( *SFX_APP() );		// for SFX_HINT_DEINITIALIZING
/*N*/ }

/*N*/ ScModule::~ScModule()
/*N*/ {
/*N*/ 	DELETEZ( pMessagePool );
/*N*/
/*N*/ 	DELETEZ( pFormEditData );
/*N*/
/*N*/ 	delete pErrorHdl;
/*N*/ //	delete pSvxErrorHdl;
/*N*/
/*N*/ 	ScGlobal::Clear();		// ruft auch ScDocumentPool::DeleteVersionMaps();
/*N*/
/*N*/ 	DeleteCfg();			// wurde mal aus Exit() gerufen
/*N*/ }

//------------------------------------------------------------------

/*N*/ void ScModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
/*N*/ {
/*N*/ 	if ( rHint.ISA(SfxSimpleHint) )
/*N*/ 	{
/*N*/         ULONG nHintId = ((SfxSimpleHint&)rHint).GetId();
/*N*/         if ( nHintId == SFX_HINT_DEINITIALIZING )
/*N*/ 		{
/*N*/ 			//	ConfigItems must be removed before ConfigManager
/*N*/ 			DeleteCfg();
/*N*/ 		}
/*N*/ 		else if ( nHintId == SFX_HINT_COLORS_CHANGED || nHintId == SFX_HINT_ACCESSIBILITY_CHANGED )
/*N*/ 		{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if ( ScDetectiveFunc::IsColorsInitialized() )
/*N*/ 		}
/*N*/         else if ( nHintId == SFX_HINT_CTL_SETTINGS_CHANGED )
/*N*/         {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/         }
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------

/*N*/ void ScModule::DeleteCfg()
/*N*/ {
/*N*/ 	DELETEZ( pViewCfg ); // Speichern passiert vor Exit() automatisch
/*N*/ 	DELETEZ( pDocCfg );
/*N*/ 	DELETEZ( pAppCfg );
/*N*/ 	DELETEZ( pInputCfg );
/*N*/ 	DELETEZ( pPrintCfg );
/*N*/ 	DELETEZ( pNavipiCfg );
/*N*/
/*N*/ 	if ( pColorConfig )
/*N*/ 	{
/*N*/ 	    EndListening(*pColorConfig);
/*N*/ 		DELETEZ( pColorConfig );
/*N*/ 	}
/*N*/ 	if ( pCTLOptions )
/*N*/ 	{
/*N*/ 	    EndListening(*pCTLOptions);
/*N*/ 		DELETEZ( pCTLOptions );
/*N*/ 	}
/*N*/ }

//------------------------------------------------------------------

/*N*/ SfxModule* ScModule::Load()
/*N*/ {
/*N*/ 	return this;					// ist schon geladen
/*N*/ }

/*N*/ SfxModule* ScModuleDummy::Load()
/*N*/ {
/*N*/ 	return NULL;					// Dummy zum Linken der DLL, wird nicht gerufen
/*N*/ }

//------------------------------------------------------------------

#define TEXT_WIDTH(s)	rStatusBar.GetTextWidth((s))

/*N*/ void ScModule::FillStatusBar(StatusBar& rStatusBar)
/*N*/ {
/*N*/ }

#undef TEXT_WIDTH

//------------------------------------------------------------------
//
//		von der Applikation verschoben:
//
//------------------------------------------------------------------

/*N*/ void ScModule::ResetDragObject()
/*N*/ {
/*N*/ 	aDragData.aLinkDoc.Erase();
/*N*/ 	aDragData.aLinkTable.Erase();
/*N*/ 	aDragData.aLinkArea.Erase();
/*N*/ 	aDragData.pJumpLocalDoc = NULL;
/*N*/ 	aDragData.aJumpTarget.Erase();
/*N*/ 	aDragData.aJumpText.Erase();
/*N*/ }

//------------------------------------------------------------------

/*N*/ const ScViewOptions& ScModule::GetViewOptions()
/*N*/ {
/*N*/ 	if ( !pViewCfg )
/*N*/ 		pViewCfg = new ScViewCfg;
/*N*/
/*N*/ 	return *pViewCfg;
/*N*/ }


/*N*/ const ScDocOptions& ScModule::GetDocOptions()
/*N*/ {
/*N*/ 	if ( !pDocCfg )
/*N*/ 		pDocCfg = new ScDocCfg;
/*N*/
/*N*/ 	return *pDocCfg;
/*N*/ }

#ifndef	LRU_MAX
#define LRU_MAX 10
#endif


/*N*/ void ScModule::RecentFunctionsChanged()
/*N*/ {
/*?*/	DBG_BF_ASSERT(0, "STRIP");
/*N*/ }

/*N*/ void ScModule::SetAppOptions( const ScAppOptions& rOpt )
/*N*/ {
/*N*/ 	if ( !pAppCfg )
/*N*/ 		pAppCfg = new ScAppCfg;
/*N*/
/*N*/ 	pAppCfg->SetOptions( rOpt );
/*N*/ }

/*N*/ void global_InitAppOptions()
/*N*/ {
/*N*/ 	SC_MOD()->GetAppOptions();
/*N*/ }

/*N*/ const ScAppOptions& ScModule::GetAppOptions()
/*N*/ {
/*N*/ 	if ( !pAppCfg )
/*N*/ 		pAppCfg = new ScAppCfg;
/*N*/
/*N*/ 	return *pAppCfg;
/*N*/ }

/*N*/ void ScModule::SetInputOptions( const ScInputOptions& rOpt )
/*N*/ {
/*N*/ 	if ( !pInputCfg )
/*N*/ 		pInputCfg = new ScInputCfg;
/*N*/
/*N*/ 	pInputCfg->SetOptions( rOpt );
/*N*/ }

/*N*/ const ScInputOptions& ScModule::GetInputOptions()
/*N*/ {
/*N*/ 	if ( !pInputCfg )
/*N*/ 		pInputCfg = new ScInputCfg;
/*N*/
/*N*/ 	return *pInputCfg;
/*N*/ }

/*N*/ void ScModule::SetPrintOptions( const ScPrintOptions& rOpt )
/*N*/ {
/*N*/ 	if ( !pPrintCfg )
/*N*/ 		pPrintCfg = new ScPrintCfg;
/*N*/
/*N*/ 	pPrintCfg->SetOptions( rOpt );
/*N*/ }

/*N*/ const ScPrintOptions& ScModule::GetPrintOptions()
/*N*/ {
/*N*/ 	if ( !pPrintCfg )
/*N*/ 		pPrintCfg = new ScPrintCfg;
/*N*/
/*N*/ 	return *pPrintCfg;
/*N*/ }

        ColorConfig& ScModule::GetColorConfig()
/*N*/ {
/*N*/ 	if ( !pColorConfig )
/*N*/ 	{
/*N*/         pColorConfig = new ColorConfig;
/*N*/ 	    StartListening(*pColorConfig);
/*N*/ 	}
/*N*/
/*N*/ 	return *pColorConfig;
/*N*/ }

/*N*/ SvtCTLOptions& ScModule::GetCTLOptions()
/*N*/ {
/*N*/ 	if ( !pCTLOptions )
/*N*/ 	{
/*N*/ 		pCTLOptions = new SvtCTLOptions;
/*N*/ 		StartListening(*pCTLOptions);
/*N*/ 	}
/*N*/
/*N*/ 	return *pCTLOptions;
/*N*/ }

/*N*/ USHORT ScModule::GetOptDigitLanguage()
/*N*/ {
/*N*/ 	SvtCTLOptions::TextNumerals eNumerals = GetCTLOptions().GetCTLTextNumerals();
/*N*/ 	return ( eNumerals == SvtCTLOptions::NUMERALS_ARABIC ) ? LANGUAGE_ENGLISH_US :
/*N*/ 		   ( eNumerals == SvtCTLOptions::NUMERALS_HINDI)   ? LANGUAGE_ARABIC_SAUDI_ARABIA :
/*N*/ 															 LANGUAGE_SYSTEM;
/*N*/ }

//------------------------------------------------------------------
//
//					Idle / Online-Spelling
//
//------------------------------------------------------------------

/*N*/ void ScModule::AnythingChanged()
/*N*/ {
/*N*/ 	ULONG nOldTime = aIdleTimer.GetTimeout();
/*N*/ 	if ( nOldTime != SC_IDLE_MIN )
/*N*/ 		aIdleTimer.SetTimeout( SC_IDLE_MIN );
/*N*/
/*N*/ 	nIdleCount = 0;
/*N*/ }

/*N*/ IMPL_LINK( ScModule, IdleHandler, Timer*, pTimer )
/*N*/ {
/*N*/ 	if ( Application::AnyInput( INPUT_MOUSEANDKEYBOARD ) )
/*N*/ 	{
/*N*/ 		aIdleTimer.Start();			// Timeout unveraendert
/*N*/ 		return 0;
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bMore = FALSE;
/*N*/ 	ScDocShell* pDocSh = PTR_CAST( ScDocShell, SfxObjectShell::Current() );
/*N*/ 	if ( pDocSh )
/*N*/ 	{
/*?*/ 		ScDocument* pDoc = pDocSh->GetDocument();
/*?*/ 		if ( pDoc->IsLoadingDone() )
/*?*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 BOOL bLinks = pDoc->IdleCheckLinks();
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	ULONG nOldTime = aIdleTimer.GetTimeout();
/*N*/ 	ULONG nNewTime = nOldTime;
/*N*/ 	if ( bMore )
/*N*/ 	{
/*?*/ 		nNewTime = SC_IDLE_MIN;
/*?*/ 		nIdleCount = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//	SC_IDLE_COUNT mal mit initialem Timeout, dann hochzaehlen
/*N*/
/*N*/ 		if ( nIdleCount < SC_IDLE_COUNT )
/*N*/ 			++nIdleCount;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nNewTime += SC_IDLE_STEP;
/*N*/ 			if ( nNewTime > SC_IDLE_MAX )
/*N*/ 				nNewTime = SC_IDLE_MAX;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( nNewTime != nOldTime )
/*N*/ 		aIdleTimer.SetTimeout( nNewTime );
/*N*/
/*N*/ 	aIdleTimer.Start();
/*N*/ 	return 0;
/*N*/ }

IMPL_LINK( ScModule, SpellTimerHdl, Timer*, pTimer )
{
    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if ( Application::AnyInput( INPUT_KEYBOARD ) )
    return 0;
}

/*N*/ IMPL_LINK( ScModule, CalcFieldValueHdl, EditFieldInfo*, pInfo )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if (pInfo)
/*N*/ 	return 0;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
