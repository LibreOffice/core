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

#if defined UNX
#include <limits.h>
#else // UNX
#include <stdlib.h>
#define PATH_MAX _MAX_PATH
#endif // UNX

#include <bf_svtools/svdde.hxx>
#include <tools/urlobj.hxx>

#define _SVSTDARR_STRINGSDTOR

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include <rtl/logfile.hxx>

#include "appuno.hxx"
#include "progress.hxx"
#include "docfac.hxx"
#include "cfgmgr.hxx"
#include "fltfnc.hxx"

#include "app.hxx"
#include "appdata.hxx"
#include "interno.hxx"
#include "event.hxx"
#include "appimp.hxx"
#include "imestatuswindow.hxx"
#include "docfilt.hxx"

#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#include "bf_so3/staticbaseurl.hxx"
#include <bf_so3/ipenv.hxx>

namespace binfilter {

// Static member
SfxApplication* SfxApplication::pApp = NULL;

/*N*/ SfxApplication* SfxApplication::GetOrCreate()
/*N*/ {
/*N*/ 	::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
/*N*/ 
/*N*/ 	// SFX on demand
/*N*/     if ( !pApp )
/*N*/     {
/*N*/ 		::com::sun::star::uno::Reference < ::com::sun::star::lang::XInitialization >
/*N*/ 			xWrp(::legacy_binfilters::getLegacyProcessServiceFactory()->createInstance( DEFINE_CONST_UNICODE("com.sun.star.office.OfficeWrapper")), ::com::sun::star::uno::UNO_QUERY );
/*N*/ 			xWrp->initialize( ::com::sun::star::uno::Sequence < ::com::sun::star::uno::Any >() );
/*N*/     }
/*N*/ 
/*N*/ 	return pApp;
/*N*/ }

/*N*/ void SfxApplication::SetApp( SfxApplication* pSfxApp )
/*N*/ {
/*N*/ 	static ::osl::Mutex aProtector;
/*N*/ 	::osl::MutexGuard aGuard( aProtector );
/*N*/ 
/*N*/ 	DBG_ASSERT( !pApp, "SfxApplication already created!" );
/*N*/ 	if ( pApp )
/*?*/ 		DELETEZ( pApp );
/*N*/ 
/*N*/     pApp = pSfxApp;
/*N*/ 
/*N*/     // at the moment a bug may occur when Initialize_Impl returns FALSE, but this is only temporary because all code that may cause such a
/*N*/     // fault will be moved outside the SFX
/*N*/     pApp->Initialize_Impl();
/*N*/ }

/*N*/ SfxApplication::SfxApplication()
/*N*/ 	: _nFeatures( ULONG_MAX )
/*N*/ 	, pImp( 0 )
/*N*/ 	, pAppData_Impl( 0 )
/*N*/     , pCfgMgr( 0 )
/*N*/     , bInInit( sal_False )
/*N*/     , bInExit( sal_False )
/*N*/     , bDowning( sal_True )
/*N*/ 	, bCreatedExternal( sal_False )
/*N*/ 	, pOptions( 0 )
/*N*/ {
/*N*/ 
/*N*/ 	pImp = new SfxApplication_Impl;
/*N*/ 	pImp->bConfigLoaded = sal_False;
/*N*/ 	pImp->pEmptyMenu = 0;
/*N*/ 	pImp->nDocNo = 0;
/*N*/ 	pImp->pIntro = 0;
/*N*/ 	pImp->pObjShells = 0;
/*N*/ 	pImp->bAutoSaveNow = sal_False;
/*N*/ 	pImp->pBasicLibContainer = 0;
/*N*/ 	pImp->pDialogLibContainer = 0;
/*N*/ 	pImp->pBasicTestWin = 0;
/*N*/ 	pImp->nWarnLevel = 0;
/*N*/ 	pImp->pAutoSaveTimer = 0;
/*N*/     // Create instance of SvtSysLocale _after_ setting the locale at the application,
/*N*/     // so that it can initialize itself correctly.
/*N*/ 
/*N*/ 	pAppData_Impl = new SfxAppData_Impl( this );
/*N*/     pAppData_Impl->m_xImeStatusWindow->init();
/*N*/     pApp->PreInit();
/*N*/ 
/*N*/     pCfgMgr = new SfxConfigManager;
/*N*/ }

/*N*/ SfxApplication::~SfxApplication()
/*N*/ {
/*N*/     if ( !bDowning )
/*?*/         Deinitialize();
/*N*/ 
/*N*/ 	Broadcast( SfxSimpleHint(SFX_HINT_DYING) );
/*N*/ 
/*N*/     // better call SvFactory::DeInit, because this will remove ALL factories,
/*N*/     // but it will fail because the ConfigManager has a storage that is a SvObject
/*N*/     SfxObjectFactory::RemoveAll_Impl();
/*N*/ 
/*N*/ //    UCB_Helper::Deinitialize();
/*N*/ 
/*N*/     delete pCfgMgr;
/*N*/ 	delete pImp;
/*N*/     delete pAppData_Impl;
/*N*/     pApp = 0;
/*N*/ }

//--------------------------------------------------------------------

/*N*/ const SfxFilter* SfxApplication::GetFilter
/*N*/ (
/*N*/     const SfxObjectFactory &rFact,
/*N*/     const String &rFilterName
/*N*/     )   const
/*N*/ {
/*N*/     DBG_ASSERT( rFilterName.Search( ':' ) == STRING_NOTFOUND,
/*N*/                 "SfxApplication::GetFilter erwartet unqualifizierte Namen" );
/*N*/     return rFact.GetFilterContainer()->GetFilter4FilterName(rFilterName);
/*N*/ }

//---------------------------------------------------------------------

/*N*/ ResMgr* SfxApplication::CreateResManager( const char *pPrefix )
/*N*/ {
/*N*/     String aMgrName = String::CreateFromAscii( pPrefix );
/*N*/     return ResMgr::CreateResMgr(U2S(aMgrName));
/*N*/ }

//------------------------------------------------------------------------

/*N*/ void SfxApplication::ReleaseIndex(sal_uInt16 i)
/*N*/ {
/*N*/     pAppData_Impl->aIndexBitSet.ReleaseIndex(i-1);
/*N*/ }

//-------------------------------------------------------------------------

/*N*/ SfxObjectShellArr_Impl&     SfxApplication::GetObjectShells_Impl() const
/*N*/ {
/*N*/ 	return *pImp->pObjShells;
/*N*/ }

}
