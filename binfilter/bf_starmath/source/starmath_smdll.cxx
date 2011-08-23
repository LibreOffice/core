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

#ifndef _SVX_SVXIDS_HRC //autogen
#include <bf_svx/svxids.hrc>
#endif

#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif

#ifndef _STARMATH_HRC
#include <starmath.hrc>
#endif
namespace binfilter {

BOOL SmDLL::bInitialized = FALSE;

/*************************************************************************
|*
|* Initialisierung
|*
\************************************************************************/
/*N*/ void SmDLL::Init()
/*N*/ {
/*N*/     if ( bInitialized )
/*N*/         return;
/*N*/ 
/*N*/     bInitialized = TRUE;
/*N*/ 
/*N*/ 	// called directly after loading the DLL
/*N*/ 	// do whatever you want, you may use Sd-DLL too
/*N*/ 
/*N*/ 	// the SdModule must be created
/*N*/ 
/*N*/ 	SmModuleDummy** ppShlPtr = (SmModuleDummy**) GetAppData(BF_SHL_SM);
/*N*/ 
/*N*/     SvFactory* pFact = PTR_CAST(SvFactory,(*ppShlPtr)->pSmDocShellFactory);
/*N*/     delete (*ppShlPtr);
/*N*/     (*ppShlPtr) = new SmModule(pFact);
/*N*/     (*ppShlPtr)->pSmDocShellFactory = pFact;
/*N*/ 
/*N*/ 	String aResDll(C2S("sm"));
/*N*/ 
/*N*/ 	SfxModule *p = SM_MOD1();
/*N*/ 	SmModule *pp = (SmModule *) p;
/*N*/ 
}

/*************************************************************************
|*
|* Deinitialisierung
|*
\************************************************************************/
/*N*/ void SmDLL::Exit()
/*N*/ {
/*N*/ 	// the SdModule must be destroyed
/*N*/ 	SmModuleDummy** ppShlPtr = (SmModuleDummy**) GetAppData(BF_SHL_SM);
/*N*/ 	delete (*ppShlPtr);
/*N*/ 	(*ppShlPtr) = NULL;
/*N*/ 
/*N*/ 	*GetAppData(BF_SHL_SM) = 0;
/*N*/ }


}
