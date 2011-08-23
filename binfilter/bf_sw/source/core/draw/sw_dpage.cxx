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

#ifdef _MSC_VER
#pragma hdrstop
#endif


#include <frmfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <drawdoc.hxx>
#include <dpage.hxx>
#include <dcontact.hxx>
#include <docsh.hxx>
#include <flyfrm.hxx>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
namespace binfilter {

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::frame;

/*?*/ SwDPage::SwDPage(SwDrawDocument& rNewModel, BOOL bMasterPage) :
/*?*/ 	FmFormPage(rNewModel, 0, bMasterPage),
/*?*/ 	pGridLst( 0 ),
/*?*/ 	rDoc(rNewModel.GetDoc())
/*?*/ {
/*?*/ }


/*N*/ SwDPage::~SwDPage()
/*N*/ {
/*N*/ 	delete pGridLst;
/*N*/ }


// #i3694#
// This GetOffset() method is not needed anymore, it even leads to errors.
//Point  SwDPage::GetOffset() const
//{
//	return Point( DOCUMENTBORDER, DOCUMENTBORDER );
//}

/*************************************************************************
|*
|*	SwDPage::ReplaceObject()
|*
|*	Ersterstellung		MA 07. Aug. 95
|*	Letzte Aenderung	MA 07. Aug. 95
|*
*************************************************************************/

/*N*/ SdrObject*  SwDPage::ReplaceObject( SdrObject* pNewObj, ULONG nObjNum )
/*N*/ {
/*N*/ 	SdrObject *pOld = GetObj( nObjNum );
/*N*/ 	ASSERT( pOld, "Oups, Object not replaced" );
/*N*/ 	SdrObjUserCall* pContact;
/*N*/ 	if ( 0 != ( pContact = GetUserCall(pOld) ) &&
/*N*/ 		 RES_DRAWFRMFMT == ((SwContact*)pContact)->GetFmt()->Which())
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 		((SwDrawContact*)pContact)->ChangeMasterObject( pNewObj );
/*N*/ 	return FmFormPage::ReplaceObject( pNewObj, nObjNum );
/*N*/ }

/*************************************************************************
|*
|*	SwDPage::GetGridFrameList()
|*
|*	Ersterstellung		MA 04. Sep. 95
|*	Letzte Aenderung	MA 15. Feb. 96
|*
*************************************************************************/




/*************************************************************************
|*
|*	String SwDPage::GetLinkData( const String& )
|*	void SwDPage::SetLinkData( const String&, const String& )
|*	void SwDPage::UpdateLinkData( const String&, const String& )
|*
|*	Ersterstellung		JP 04.09.95
|*	Letzte Aenderung	JP 04.09.95
|*
*************************************************************************/







/*
Bug 29593: QuickHelp immer an der MausPosition anzeigen (besonders unter OS/2)

                    Rectangle aRect( pObj->GetSnapRect() );
                    Point aPt( pWindow->OutputToScreenPixel( pWindow->LogicToPixel( aRect.TopLeft() )));
                    aRect.Left()   = aPt.X();
                    aRect.Top()    = aPt.Y();
                    aPt = pWindow->OutputToScreenPixel( pWindow->LogicToPixel( aRect.BottomRight() ));
                    aRect.Right()  = aPt.X();
                    aRect.Bottom() = aPt.Y();
*/
/* -----------------------------27.11.00 07:35--------------------------------

 ---------------------------------------------------------------------------*/
/*N*/ Reference< XInterface > SwDPage::createUnoPage()
/*N*/ {
/*N*/ 	Reference<XModel> xModel = rDoc.GetDocShell()->GetBaseModel();
/*N*/ 	Reference<XDrawPageSupplier> xPageSupp(xModel, UNO_QUERY);
/*N*/ 	return xPageSupp->getDrawPage();
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
