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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <tools/urlobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>

#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>

using namespace com::sun::star;

#include "tabvwsh.hxx"
#include "document.hxx"
#include "drawview.hxx"
#include "globstr.hrc"
#include <avmedia/mediawindow.hxx>

//------------------------------------------------------------------------

void ScTabViewShell::InsertURLButton( const String& rName, const String& rURL,
                                        const String& rTarget,
                                        const Point* pInsPos )
{
    //  Tabelle geschuetzt ?

    ScViewData* pViewData = GetViewData();
    ScDocument* pDoc = pViewData->GetDocument();
    SCTAB nTab = pViewData->GetTabNo();
    if ( pDoc->IsTabProtected(nTab) )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    MakeDrawLayer();

    ScTabView*  pView   = pViewData->GetView();
    ScDrawView* pDrView = pView->GetScDrawView();
    SdrModel*   pModel  = pDrView->GetModel();

    SdrObject* pObj = SdrObjFactory::MakeNewObject(FmFormInventor, OBJ_FM_BUTTON,
                               pDrView->GetSdrPageView()->GetPage(), pModel);
    SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, pObj);

    uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
    DBG_ASSERT( xControlModel.is(), "UNO-Control ohne Model" );
    if( !xControlModel.is() )
        return;

    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );
    uno::Any aAny;

    aAny <<= rtl::OUString(rName);
    xPropSet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Label" )), aAny );

    ::rtl::OUString aTmp = INetURLObject::GetAbsURL( pDoc->GetDocumentShell()->GetMedium()->GetBaseURL(), rURL );
    aAny <<= aTmp;
    xPropSet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TargetURL" )), aAny );

    if( rTarget.Len() )
    {
        aAny <<= rtl::OUString(rTarget);
        xPropSet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "TargetFrame" )), aAny );
    }

    form::FormButtonType eButtonType = form::FormButtonType_URL;
    aAny <<= eButtonType;
    xPropSet->setPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ButtonType" )), aAny );

        if ( ::avmedia::MediaWindow::isMediaURL( rURL ) )
    {
        aAny <<= sal_True;
        xPropSet->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DispatchURLInternal" )), aAny );
    }

    Point aPos;
    if (pInsPos)
        aPos = *pInsPos;
    else
        aPos = GetInsertPos();

    // Groesse wie in 3.1:
    Size aSize = GetActiveWin()->PixelToLogic(Size(140, 20));

    if ( pDoc->IsNegativePage(nTab) )
        aPos.X() -= aSize.Width();

    pObj->SetLogicRect(Rectangle(aPos, aSize));

    //  am alten VC-Button musste die Position/Groesse nochmal explizit
    //  gesetzt werden - das scheint mit UnoControls nicht noetig zu sein

    //  nicht markieren wenn Ole
    pDrView->InsertObjectSafe( pObj, *pDrView->GetSdrPageView() );
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
