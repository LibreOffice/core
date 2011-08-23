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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <stdio.h>
#include <tools/rcid.h>

#include <cstdarg>
#include "app.hxx"
#include "arrdecl.hxx"
#include "docfac.hxx"
#include "module.hxx"

namespace binfilter {

static SfxModuleArr_Impl* pModules=0;

/*N*/ TYPEINIT1(SfxModule, SfxShell);

/*?*/ BOOL SfxModule::QueryUnload()
/*?*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001
/*?*/ }

/*?*/ SfxModule* SfxModule::Load()
/*?*/ {
/*?*/ 	return 0;
/*?*/ }

/*?*/ void SfxModule::Free()
/*?*/ {
/*?*/ }


/*N*/ ResMgr* SfxModule::GetResMgr()
/*N*/ {
/*N*/ 	return pResMgr;
/*N*/ }

//====================================================================
/*N*/ SfxModule::SfxModule( ResMgr* pMgrP, BOOL bDummyP,
/*N*/ 					  SfxObjectFactory* pFactoryP, ... )
/*N*/ 	: pResMgr( pMgrP ), bDummy( bDummyP )
/*N*/ {
/*N*/ 	if( !bDummy )
/*N*/ 	{
/*N*/ 		SfxApplication *pApp = SFX_APP();
/*N*/       SfxModuleArr_Impl& rArr = GetModules_Impl();
/*N*/ 		SfxModule* pPtr = (SfxModule*)this;
/*N*/ 		rArr.C40_INSERT( SfxModule, pPtr, rArr.Count() );
/*N*/ 		SetPool( &pApp->GetPool() );
/*N*/ 	}
/*N*/ 	std::va_list pVarArgs;
/*N*/ 	va_start( pVarArgs, pFactoryP );
/*N*/ 	for ( SfxObjectFactory *pArg = pFactoryP; pArg;
/*N*/ 		 pArg = va_arg( pVarArgs, SfxObjectFactory* ) )
/*N*/ 		pArg->SetModule_Impl( this );
/*N*/ 	va_end(pVarArgs);
/*N*/ }

/*N*/ SfxModule::~SfxModule()
/*N*/ {
/*N*/ 	if( !bDummy )
/*N*/ 	{
/*N*/ 		if ( SFX_APP()->Get_Impl() )
/*N*/ 		{
/*N*/ 			// Das Modul wird noch vor dem DeInitialize zerst"ort, also auis dem Array entfernen
/*N*/           SfxModuleArr_Impl& rArr = GetModules_Impl();
/*N*/ 			for( USHORT nPos = rArr.Count(); nPos--; )
/*N*/ 			{
/*N*/ 				if( rArr[ nPos ] == this )
/*N*/ 				{
/*N*/ 					rArr.Remove( nPos );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		delete pResMgr;
/*N*/ 	}
/*N*/ }

/*N*/ SfxModuleArr_Impl& SfxModule::GetModules_Impl()
/*N*/ {
/*N*/     if( !pModules )
/*N*/         pModules = new SfxModuleArr_Impl;
/*N*/     return *pModules;
/*N*/ };
}
