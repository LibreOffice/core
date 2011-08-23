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

#define ITEMID_COLOR_TABLE		SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST	SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST		SID_HATCH_LIST
#define ITEMID_BITMAP_LIST		SID_BITMAP_LIST
#define ITEMID_DASH_LIST		SID_DASH_LIST
#define ITEMID_LINEEND_LIST 	SID_LINEEND_LIST


#ifndef _OFF_APP_HXX //autogen
#include <bf_offmgr/app.hxx>
#endif
#ifndef _SVX_DRAWITEM_HXX //autogen
#include <bf_svx/drawitem.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <bf_svx/svdmodel.hxx>
#endif
#ifndef _SVDOUTL_HXX
#include <bf_svx/svdoutl.hxx>
#endif

#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
namespace binfilter {

/*--------------------------------------------------------------------
    Beschreibung: Document laden
 --------------------------------------------------------------------*/


/*N*/ void  SwDocShell::InitDraw()
/*N*/ {
/*N*/ 	SdrModel *pDrDoc = pDoc->GetDrawModel();
/*N*/ 	if( pDrDoc )
/*N*/ 	{
/*N*/ 		// Listen, bzw. Tables im ItemSet der DocShell anlegen
/*N*/ 		PutItem( SvxGradientListItem( pDrDoc->GetGradientList() ) );
/*N*/ 		PutItem( SvxHatchListItem( pDrDoc->GetHatchList() ) );
/*N*/ 		PutItem( SvxBitmapListItem( pDrDoc->GetBitmapList() ) );
/*N*/ 		PutItem( SvxDashListItem( pDrDoc->GetDashList() ) );
/*N*/ 		PutItem( SvxLineEndListItem( pDrDoc->GetLineEndList() ) );
/*N*/ 
/*N*/ 		Outliner& rOutliner = pDrDoc->GetDrawOutliner();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		PutItem( SvxColorTableItem( OFF_APP()->GetStdColorTable() ));
/*N*/ }

}
