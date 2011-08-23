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
#ifndef _BASMGR_HXX //autogen
#include "bf_basic/basmgr.hxx"
#endif

#ifdef WIN
#define _TL_LANG_SPECIAL
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#include "appdata.hxx"
#include "plugobj.hxx"
#include "arrdecl.hxx"
#include "macrconf.hxx"

#include <bf_svtools/imgdef.hxx>

#include "frameobj.hxx"
#include "docfile.hxx"
#include "sfxtypes.hxx"
#include "appimp.hxx"
#include "fcontnr.hxx"
#include "nochaos.hxx"
#include "doctempl.hxx"
#include "dlgcont.hxx"
#include "scriptcont.hxx"
#include "misccfg.hxx"
#include "docfac.hxx"

namespace binfilter {

DECLARE_LIST( SfxFrameWindowFactoryArray_Impl, SfxFrameWindowFactory* )
SV_DECL_PTRARR(SfxInitLinkList, Link*, 2, 2)//STRIP008 ;

//-------------------------------------------------------------------------

/*?*/ void SfxApplication::Deinitialize()
/*?*/ {
/*?*/     if ( bDowning )
/*?*/         return;
/*?*/ 
/*?*/ 	StarBASIC::Stop();
/*?*/ 
/*?*/ 	bDowning = TRUE; // wegen Timer aus DecAliveCount und QueryExit
/*?*/ 
/*?*/ 	bDowning = FALSE;
/*?*/ 	DBG_ASSERT( !SfxObjectShell::GetFirst(),
/*?*/ 				"existing SfxObjectShell after Execute" );
/*?*/ 	bDowning = TRUE;
/*?*/ 
/*?*/ 	// call derived application-exit
/*?*/ 	bInExit = TRUE;
/*?*/ 	Exit();
/*?*/ 
/*?*/     // Controller u."a. freigeben
/*?*/     // dabei sollten auch restliche Komponenten ( Beamer! ) verschwinden
/*?*/ 	SfxObjectFactory::ClearAll_Impl();
//      pBasMgr = NULL;
/*?*/ 	if( pImp->pBasicLibContainer )
/*?*/ 		pImp->pBasicLibContainer->release();
/*?*/ 	if( pImp->pDialogLibContainer )
/*?*/ 		pImp->pDialogLibContainer->release();
/*?*/ 
/*?*/ 	bInExit = FALSE;
/*?*/ 
/*?*/ 	// ab hier d"urfen keine SvObjects mehr existieren
/*?*/ 	DELETEX(pAppData_Impl->pMatcher);
/*?*/ 	DELETEX(pAppData_Impl->pSfxFrameObjectFactoryPtr);
/*?*/ 
/*?*/ 	DELETEX(pAppData_Impl->pEventConfig);
/*?*/   DELETEX(pAppData_Impl->pMiscConfig);
/*?*/ 	SfxMacroConfig::Release_Impl();
/*?*/ 	DELETEX(pAppData_Impl->pInitLinkList);
/*?*/ 
/*?*/     DELETEX(pImp->pEventHdl);
/*?*/     DELETEX(pImp->pObjShells);
/*?*/ 
/*?*/ 	NoChaos::ReleaseItemPool();
/*?*/ 	pAppData_Impl->pPool = NULL;
/*?*/ }
}
