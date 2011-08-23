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


#ifndef _SVDOBJ_HXX //autogen
#include <bf_svx/svdobj.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

#ifndef _GLOBDOC_HRC
#include "globdoc.hrc"
#endif

#ifndef _SWWDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _SWGLOBDOCSH_HXX
#include <globdoc.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _INIT_HXX
#include <init.hxx>
#endif
#ifndef _DOBJFAC_HXX
#include <dobjfac.hxx>
#endif
#ifndef _CFGID_H
#include <cfgid.h>
#endif
namespace binfilter {

/*************************************************************************
|*
|* Init
|*
\************************************************************************/

/*N*/ void SwDLL::Init()
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDLL" );
/*N*/
/*N*/ 	// called directly after loading the DLL
/*N*/ 	// do whatever you want, you may use Sw-DLL too
/*N*/
/*N*/ 	// the SdModule must be created
/*N*/ 	SwModuleDummy** ppShlPtr = (SwModuleDummy**) GetAppData(BF_SHL_WRITER);
/*N*/
/*N*/ 	SvFactory* pDocFact     = (SvFactory*)(*ppShlPtr)->pSwDocShellFactory;
/*N*/ 	SvFactory* pWDocFact    = (SvFactory*)(*ppShlPtr)->pSwWebDocShellFactory;
/*N*/ 	SvFactory* pGlobDocFact = (SvFactory*)(*ppShlPtr)->pSwGlobalDocShellFactory;
/*N*/ 	delete (*ppShlPtr);
/*N*/     SwModule* pModule = new SwModule( pWDocFact, pDocFact, pGlobDocFact );
/*N*/ 	(*ppShlPtr) = pModule;
/*N*/ 	(*ppShlPtr)->pSwDocShellFactory    		= pDocFact    ;
/*N*/ 	(*ppShlPtr)->pSwWebDocShellFactory     	= pWDocFact   ;
/*N*/ 	(*ppShlPtr)->pSwGlobalDocShellFactory 	= pGlobDocFact;
/*N*/
/*N*/ 	SdrObjFactory::InsertMakeObjectHdl( LINK( &aSwObjectFactory, SwObjectFactory, MakeObject ) );
/*N*/
/*N*/ 	RTL_LOGFILE_CONTEXT_TRACE( aLog, "Init Core/UI/Filter" );
/*N*/
/*N*/ 	//Initialisierung der Statics
/*N*/ 	::binfilter::_InitCore();
/*N*/ 	::binfilter::_InitFilter();
///*N*/   ::binfilter::_InitUI();
/*N*/
/*N*/ 	pModule->InitAttrPool();
/*N*/ 	//jetzt darf das SwModule seinen Pool anlegen
/*N*/ }

/*************************************************************************
|*
|* Exit
|*
\************************************************************************/

/*N*/ void SwDLL::Exit()
/*N*/ {
/*N*/ 	// called directly befor unloading the DLL
/*N*/ 	// do whatever you want, Sw-DLL is accessible
/*N*/
/*N*/ 	// der Pool muss vor den statics geloescht werden
/*N*/ 	SW_MOD()->RemoveAttrPool();
/*N*/
///*N*/   ::binfilter::_FinitUI();
/*N*/ 	::binfilter::_FinitFilter();
/*N*/ 	::binfilter::_FinitCore();
/*N*/ 	// Objekt-Factory austragen
/*N*/ 	SdrObjFactory::RemoveMakeObjectHdl(LINK(&aSwObjectFactory, SwObjectFactory, MakeObject ));
/*N*/    // the SwModule must be destroyed
/*N*/ 	SwModuleDummy** ppShlPtr = (SwModuleDummy**) GetAppData(BF_SHL_WRITER);
/*N*/ 	delete (*ppShlPtr);
/*N*/ 	(*ppShlPtr) = NULL;
/*N*/ }

}
