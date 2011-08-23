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

#define ITEMID_SEARCH           SID_SEARCH_ITEM

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include <bf_svx/svxids.hrc>
#include <bf_sfx2/app.hxx>
#include <vcl/status.hxx>
#include <bf_svx/svdobj.hxx>

#define _SCH_DLL                 // fuer SD_MOD()
#include "schmod.hxx"
#include "schresid.hxx"

#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"

namespace binfilter {

/*N*/ TYPEINIT1( SchModuleDummy, SfxModule );
/*N*/ TYPEINIT1( SchModule, SchModuleDummy );

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

/*N*/ SchModule::SchModule(SvFactory* pObjFact) :
/*N*/ 	SchModuleDummy(SFX_APP()->CreateResManager("bf_sch"), FALSE, pObjFact),	//STRIP005
/*N*/ 	pXOutDevPool( NULL ),
/*N*/ 	pDragData( NULL ),
/*N*/ 	pClipboardData( NULL ),
/*N*/ 	mpTransferDragDrop( NULL ),
/*N*/ 	mpTransferClipboard( NULL ),
/*N*/ 	mpTransferSelectionClipbd( NULL ),
/*N*/ 	pChartOptions( NULL )
/*N*/ {
/*N*/ 	pSchObjFactory = new SchObjFactory();
/*N*/
/*N*/ 	if (!pSchObjFactory->GetInserted())
/*N*/ 	{
/*N*/ 		SdrObjFactory::InsertMakeObjectHdl(LINK(pSchObjFactory, SchObjFactory, MakeObject));
/*N*/ 		SdrObjFactory::InsertMakeUserDataHdl(LINK(pSchObjFactory, SchObjFactory, MakeUserData));
/*N*/
/*N*/ 		pSchObjFactory->SetInserted(TRUE);
/*N*/ 	}
/*N*/
/*N*/ 	// #62493# this factory exists already
/*N*/ 	//pE3dFactory = new E3dObjFactory();
/*N*/ 	//SdrObjFactory::InsertMakeObjectHdl(LINK(pE3dFactory, E3dObjFactory, MakeObject));
/*N*/
/*N*/ 	SetName( String( RTL_CONSTASCII_USTRINGPARAM( "StarChart" )) );
/*N*/
/*N*/ 	// establish listener, so that the module is informed
/*N*/ 	// right before the application is going to be shut down
/*N*/ 	StartListening( *SFX_APP() );
/*N*/ }

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

/*N*/ SchModule::~SchModule()
/*N*/ {
/*N*/ 	if (pSchObjFactory)
/*N*/ 	{
/*N*/ 		if (pSchObjFactory->GetInserted())
/*N*/ 		{
/*N*/ 			SdrObjFactory::RemoveMakeObjectHdl(LINK(pSchObjFactory, SchObjFactory, MakeObject));
/*N*/ 			SdrObjFactory::RemoveMakeUserDataHdl(LINK(pSchObjFactory, SchObjFactory, MakeUserData));
/*N*/ 		}
/*N*/
/*N*/ 		delete pSchObjFactory;
/*N*/ 	}
/*N*/ }


/*?*/ SfxModule *SchModuleDummy::Load()
/*?*/ {
/*?*/ 	return NULL;
/*?*/ }

/*N*/ SchOptions* SchModule::GetSchOptions()
/*N*/ {
/*N*/ 	if( ! pChartOptions )
/*N*/ 		pChartOptions = new SchOptions();
/*N*/
/*N*/ 	return pChartOptions;
/*N*/ }

/*************************************************************************
|*
|* get notifications
|*
\************************************************************************/

/*N*/ void SchModule::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
/*N*/ {
/*N*/ 	if( rHint.ISA( SfxSimpleHint ) &&
/*N*/ 		( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_DEINITIALIZING )
/*N*/ 	{
/*N*/ 		delete pChartOptions, pChartOptions = NULL;
/*N*/ 	}
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
