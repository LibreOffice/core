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


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif

#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <bf_offmgr/app.hxx>
#endif
#ifndef _EHDL_HXX //autogen
#include <bf_svtools/ehdl.hxx>
#endif
#ifndef _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_STRINGSDTOR
#include <bf_svtools/svstdarr.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SCANNER_XSCANNERMANAGER_HPP_
#include <com/sun/star/scanner/XScannerManager.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _DOBJFAC_HXX
#include <dobjfac.hxx>
#endif
#ifndef _INIT_HXX
#include <init.hxx>
#endif
#ifndef _SWWDOCSH_HXX //autogen
#include <wdocsh.hxx>
#endif
#ifndef _SWGLOBDOCSH_HXX //autogen
#include <globdoc.hxx>
#endif
#ifndef _USRPREF_HXX //autogen
#include <usrpref.hxx>
#endif
#ifndef _UINUMS_HXX //autogen
#include <uinums.hxx>
#endif
#ifndef _PRTOPT_HXX //autogen
#include <prtopt.hxx>
#endif
#ifndef _MODOPT_HXX //autogen
#include <modcfg.hxx>
#endif
#ifndef _FONTCFG_HXX //autogen
#include <fontcfg.hxx>
#endif
#ifndef _SFX_EVENTCONF_HXX
#include <bf_sfx2/evntconf.hxx>
#endif
#ifndef _SFX_APPUNO_HXX //autogen
#include <bf_sfx2/appuno.hxx>
#endif
#ifndef _SWATRSET_HXX //autogen
#include <swatrset.hxx>
#endif
#ifndef _DLELSTNR_HXX_
#include <dlelstnr.hxx>
#endif
#ifndef _BARCFG_HXX
#include <barcfg.hxx>
#endif
// OD 14.02.2003 #107424#
#ifndef INCLUDED_SVTOOLS_COLORCFG_HXX
#include <bf_svtools/colorcfg.hxx>
#endif

#include "glshell.hxx"

#include <bf_svtools/moduleoptions.hxx>

#include <app.hrc>
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#endif
namespace binfilter {
ResMgr *pSwResMgr = 0;
sal_Bool 	bNoInterrupt 	= sal_False;

#ifndef PROFILE
// Code zum Initialisieren von Statics im eigenen Code-Segment
#ifdef _MSC_VER
#pragma code_seg( "SWSTATICS" )
#endif
#endif

#ifndef PROFILE
#ifdef _MSC_VER
#pragma code_seg()
#endif
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::scanner;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

#define C2S(cChar) String::CreateFromAscii(cChar)

/*N*/ TYPEINIT1( SwModuleDummy, SfxModule );
/*N*/ TYPEINIT1( SwModule, SwModuleDummy );


//************************************************************************

/*N*/ SwModule::SwModule( SvFactory* pFact,
/*N*/ 					SvFactory* pWebFact,
/*N*/ 					SvFactory* pGlobalFact )
/*N*/ 	: SwModuleDummy( SFX_APP()->CreateResManager( "bf_sw" ), sal_False, pFact,	//STRIP005
/*N*/ 					 pWebFact, pGlobalFact ),
/*N*/ 	pModuleConfig(0),
/*N*/ 	pView(0),
/*N*/ 	pChapterNumRules(0),
/*N*/ 	pStdFontConfig(0),
/*N*/ 	pPrtOpt(0),
/*N*/ 	pWebPrtOpt(0),
/*N*/ 	pWebUsrPref(0),
/*N*/ 	pUsrPref(0),
/*N*/ 	pDBConfig(0),
/*N*/     pColorConfig(0),
/*N*/     pCTLOptions(0),
/*N*/ 	pAttrPool(0),
/*N*/ 	bAuthorInitialised(sal_False),
/*N*/ 	bEmbeddedLoadSave( sal_False )
/*N*/ {
/*N*/ 	SetName( String::CreateFromAscii("StarWriter") );
/*N*/ 	pSwResMgr = GetResMgr();
/*N*/ 	pErrorHdl = new SfxErrorHandler( RID_SW_ERRHDL,
/*N*/ 									 ERRCODE_AREA_SW,
/*N*/ 									 ERRCODE_AREA_SW_END,
/*N*/ 									 pSwResMgr );
/*N*/
/*N*/ 	SfxEventConfiguration::RegisterEvent(SW_EVENT_MAIL_MERGE, SW_RES(STR_PRINT_MERGE_MACRO), String::CreateFromAscii("OnMailMerge"));
/*N*/ 	SfxEventConfiguration::RegisterEvent(SW_EVENT_MAIL_MERGE_END, SW_RES(STR_PRINT_MERGE_MACRO), String::CreateFromAscii("OnMailMergeFinished"));
/*N*/ 	SfxEventConfiguration::RegisterEvent(SW_EVENT_PAGE_COUNT, SW_RES(STR_PAGE_COUNT_MACRO), String::CreateFromAscii("OnPageCountChange"));
/*N*/ 	pModuleConfig = new SwModuleOptions;
/*N*/
/*N*/ 	pStdFontConfig = new SwStdFontConfig;
/*N*/
/*N*/ 	pAuthorNames = new SvStringsDtor(5, 1);	// Alle Redlining-Autoren
/*N*/
/*N*/ 	StartListening( *OFF_APP() );
/*N*/
/*N*/ 	Reference< XMultiServiceFactory > xMgr( ::legacy_binfilters::getLegacyProcessServiceFactory() );
/*N*/ 	if( xMgr.is() )
/*N*/ 	{
/*N*/ 		m_xScannerManager = Reference< XScannerManager >(
/*N*/ 						xMgr->createInstance( OUString::createFromAscii( "com.sun.star.scanner.ScannerManager" ) ),
/*N*/ 						UNO_QUERY );
/*N*/
/*N*/ //		if( m_xScannerManager.is() )
/*N*/ //		{
/*N*/ //			m_xScannerListener = Reference< lang::XEventListener >(
/*N*/ //										OWeakObject* ( new ScannerEventListener( this ) ), UNO_QUERY );
/*N*/ //		}
/*N*/ 	}
/*N*/
/*N*/     // OD 14.02.2003 #107424# - init color configuration
/*N*/     // member <pColorConfig> is created and the color configuration is applied
/*N*/     // at the view options.
/*N*/     GetColorConfig();
/*N*/ }

//************************************************************************

/*N*/ SwModule::~SwModule()
/*N*/ {
/*N*/ 	SetPool(0);
/*N*/ 	delete pAttrPool;
/*N*/ 	delete pErrorHdl;
/*N*/ 	EndListening( *OFF_APP() );
/*N*/ }

/*************************************************************************
|*
|* Modul laden (nur Attrappe fuer das Linken der DLL)
|*
\************************************************************************/

/*N*/ SfxModule* SwModuleDummy::Load()
/*N*/ {
/*N*/ 	return (NULL);
/*N*/ }

/*N*/ SwModuleDummy::~SwModuleDummy()
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Modul laden
|*
\************************************************************************/

/*N*/ SfxModule* SwModule::Load()
/*N*/ {
/*N*/ 	return (this);
/*N*/ }

/* -----------------20.04.99 10:46-------------------
 *
 * --------------------------------------------------*/
/*N*/ void	SwModule::InitAttrPool()
/*N*/ {
/*N*/ 	DBG_ASSERT(!pAttrPool, "Pool ist schon da!");
/*N*/ 	pAttrPool = new SwAttrPool(0);
/*N*/ 	SetPool(pAttrPool);
/*N*/ }
/* -----------------20.04.99 10:46-------------------
 *
 * --------------------------------------------------*/
/*N*/ void	SwModule::RemoveAttrPool()
/*N*/ {
/*N*/ 	SetPool(0);
/*N*/ 	DELETEZ(pAttrPool);
/*N*/ }

}
