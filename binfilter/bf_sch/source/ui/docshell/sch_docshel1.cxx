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

#ifndef _SVX_SVXIDS_HRC
#define ITEMID_FONTLIST 		SID_ATTR_CHAR_FONTLIST
#define ITEMID_NUMBERINFO		SID_ATTR_NUMBERFORMAT_INFO
#define ITEMID_COLOR_TABLE		SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST	SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST		SID_HATCH_LIST
#define ITEMID_BITMAP_LIST		SID_BITMAP_LIST
#define ITEMID_DASH_LIST		SID_DASH_LIST
#define ITEMID_LINEEND_LIST 	SID_LINEEND_LIST
#include <bf_svx/svxids.hrc>
#endif

#include <bf_sfx2/app.hxx>

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "docshell.hxx"

namespace binfilter {

/*************************************************************************
|*
|*  Zeichnen der DocShell (mittels der Hilfsklasse SchViewSpecial)
|*
\************************************************************************/




/*************************************************************************
|*
|*
|*
\************************************************************************/


/*************************************************************************
|*
|*	Bitmap der StyleFamily zurueckgeben (zunaechst Dummy)
|*
\************************************************************************/

/*N*/ SfxStyleSheetBasePool* SchChartDocShell::GetStyleSheetPool() throw()
/*N*/ {
/*N*/ 	return (SfxStyleSheetBasePool*)pChDoc->GetStyleSheetPool();
/*N*/ }

/*************************************************************************
|*
|*	Vorlage einfuegen (zunaechst nur Dummy)
|*
\************************************************************************/


/*************************************************************************
|*
|*	Vorlage entfernen (zunaechst nur Dummy)
|*
\************************************************************************/


/*************************************************************************
|*
|*	Chart updaten
|*
\************************************************************************/


/*************************************************************************
|*
|*
|*
\************************************************************************/

/*N*/ Rectangle SchChartDocShell::GetVisArea(USHORT nAspect) const throw()
/*N*/ {
/*N*/ 	return SfxInPlaceObject::GetVisArea((nAspect == ASPECT_THUMBNAIL) ?
/*N*/ 										 ASPECT_CONTENT : nAspect);
/*N*/ }

/*************************************************************************
|*
|* VisArea fuer InPlace setzen
|*
\************************************************************************/

/*N*/ void SchChartDocShell::SetVisArea(const Rectangle& rRect) throw()
/*N*/ {
/*N*/ 	Rectangle aRect(rRect);
/*N*/ 	aRect.SetPos(Point(0, 0));
/*N*/ 
/*N*/ 	if( GetCreateMode() == SFX_CREATE_MODE_EMBEDDED )
/*N*/ 	{
/*N*/ 		// sets the modified flag
/*N*/ 		SfxInPlaceObject::SetVisArea( rRect );
/*N*/ 
/*N*/         // #107434# propagate modified status to changed status at model
/*N*/         if( pChDoc && IsModified() )
/*N*/             pChDoc->SetChanged( TRUE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         // does not set the modified flag
/*?*/ 		SvEmbeddedObject::SetVisArea( rRect );
/*N*/ 	}
/*N*/ 
/*N*/     if( GetCreateMode() != SFX_CREATE_MODE_INTERNAL )
/*N*/ 	{
/*N*/ 		Size aSize;
/*N*/ 
/*N*/ 		if (rRect.IsEmpty ())
/*N*/ 			aSize = Size ();
/*N*/ 		else
/*N*/ 			aSize = GetVisArea(ASPECT_CONTENT).GetSize();
/*N*/ 
/*N*/ 		SdrPage* pPage = pChDoc->GetPage(0);
/*N*/ 
/*N*/ 		if(aSize.Width() > 0 && aSize.Height() > 0)
/*N*/ 		{
/*N*/ 			if( !pPage || aSize != pPage->GetSize() )
/*N*/ 			{
/*N*/ 				pChDoc->ResizePage(aSize);
/*N*/ 
/*N*/                 // notify the controller
/*N*/                 // note: the controller is only available in in-place
/*N*/                 // Broadcast( SchVisAreaChangedHint( rRect ) );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* OLE-Status
|*
\************************************************************************/

/*N*/ ULONG SchChartDocShell::GetMiscStatus() const throw()
/*N*/ {
/*N*/ 	// Chart soll stets selber resizen
/*N*/ 	ULONG nStatus = SfxInPlaceObject::GetMiscStatus() |
/*N*/ 					SVOBJ_MISCSTATUS_SERVERRESIZE     |
/*N*/ 					SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE;
/*N*/ 
/*N*/ 	return nStatus;
/*N*/ }

/*N*/ void SchChartDocShell::SetModified( BOOL bModified ) throw()
/*N*/ {
/*N*/ 	if( IsEnableSetModified() )
/*N*/ 	{
/*N*/ 		SfxInPlaceObject::SetModified( bModified );
/*N*/ 		DBG_ASSERT( pChDoc, "Invalid ChartModel" );
/*N*/ 		if( ! pChDoc->IsLockedBuild())
/*N*/ 			Broadcast( SfxSimpleHint( SFX_HINT_DOCCHANGED ) );
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
