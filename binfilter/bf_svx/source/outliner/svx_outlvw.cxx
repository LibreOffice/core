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


#define _OUTLINER_CXX
#include <outleeng.hxx>










#ifndef _MyEDITVIEW_HXX
#include "editview.hxx"
#endif

namespace binfilter {


// Breite der Randzonen innerhalb derer beim D&D gescrollt wird
#define OL_SCROLL_LRBORDERWIDTHPIX	10
#define OL_SCROLL_TBBORDERWIDTHPIX	10

// Wert, um den Fensterinhalt beim D&D gescrollt wird
#define OL_SCROLL_HOROFFSET			20  /* in % von VisibleSize.Width */
#define OL_SCROLL_VEROFFSET			20  /* in % von VisibleSize.Height */

/*N*/ DBG_NAME(OutlinerView)

/*N*/ OutlinerView::~OutlinerView()
/*N*/ {
/*N*/ 	DBG_DTOR(OutlinerView,0);
/*N*/ 	delete pEditView;
/*N*/ }


/*N*/ Rectangle OutlinerView::GetVisArea() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(OutlinerView,0);
/*N*/ 	return pEditView->GetVisArea();
/*N*/ }



// MT 05/00: Wofuer dies ImpXXXScroll, sollte das nicht die EditEngine machen???


























/*N*/ void OutlinerView::Cut()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }

/*N*/ void OutlinerView::Paste()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }




// =====================================================================
// ======================   Einfache Durchreicher =======================
// ======================================================================





/*N*/ void OutlinerView::SetSelection( const ESelection& rSel )
/*N*/ {
/*N*/ 	DBG_CHKTHIS(OutlinerView,0);
/*N*/ 	pEditView->SetSelection( rSel );
/*N*/ }


/*N*/ Window* OutlinerView::GetWindow() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(OutlinerView,0);
/*N*/ 	return pEditView->GetWindow();
/*N*/ }




/*N*/ Rectangle OutlinerView::GetOutputArea() const
/*N*/ {
/*N*/ 	DBG_CHKTHIS(OutlinerView,0);
/*N*/ 	return pEditView->GetOutputArea();
/*N*/ }

/*N*/ ESelection OutlinerView::GetSelection()
/*N*/ {
/*N*/ 	DBG_CHKTHIS(OutlinerView,0);
/*N*/ 	return pEditView->GetSelection();
/*N*/ }


















/*N*/ void OutlinerView::Copy()
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ }





















#ifndef SVX_LIGHT
#endif





}
