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

// include ---------------------------------------------------------------

#include <bf_svtools/solar.hrc>
#include "dialdll.hxx"
#include "dialmgr.hxx"
#define ITEMID_BRUSH 1
#include "brshitem.hxx"


#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif



#ifndef _FILTER_HXX //autogen
#include <bf_svtools/filter.hxx>
#endif

#ifndef _WALLITEM_HXX
#include <bf_svtools/wallitem.hxx>
#endif
namespace binfilter {

// struct DialogsResMgr --------------------------------------------------
/*N*/ DialogsResMgr::DialogsResMgr()
/*N*/ :	pResMgr(0),
/*N*/ 	pGrapicFilter(0)
/*N*/ {
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	ByteString aName( "bf_svx" );		//STRIP005
/*N*/ #else
/*N*/ 	ByteString aName( "bf_svl" );		//STRIP005
/*N*/ #endif
/*N*/     pResMgr = ResMgr::CreateResMgr( aName.GetBuffer(), Application::GetSettings().GetUILocale() );
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ DialogsResMgr::~DialogsResMgr()
/*N*/ {
/*N*/ 	delete pGrapicFilter;
/*N*/ 	delete pResMgr;
/*N*/ }

// class SvxDialogDll ----------------------------------------------------
/*N*/ SvxDialogDll::SvxDialogDll()
/*N*/ {
/*N*/ 	DBG_ASSERT( !(*GetAppData(BF_SHL_SVX)), "Ctor, but pointer not null" );
/*N*/ 
/*N*/ 	(*(DialogsResMgr**)GetAppData(BF_SHL_SVX)) = new DialogsResMgr;
/*N*/ 	SvxBrushItem::InitSfxLink();  //  OV,30.06.1998
/*N*/ }

// -----------------------------------------------------------------------
/*N*/ SvxDialogDll::~SvxDialogDll()
/*N*/ {
/*N*/ 	DBG_ASSERT( (*GetAppData(BF_SHL_SVX)), "Dtor, pointer == null" );
/*N*/ 
/*N*/ 	delete (*(DialogsResMgr**)GetAppData(BF_SHL_SVX));
/*N*/ 	(*(DialogsResMgr**)GetAppData(BF_SHL_SVX)) = 0;
/*N*/ 	delete SfxBrushItemLink::Get();
/*N*/ 	(*(SfxBrushItemLink**)GetAppData(SHL_BRUSHITEM)) = 0;
/*N*/ }

// -----------------------------------------------------------------------


}
