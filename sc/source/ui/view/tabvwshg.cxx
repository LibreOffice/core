/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <gridwin.hxx>
#include <avmedia/mediawindow.hxx>

void ScTabViewShell::InsertURLButton( const OUString& rName, const OUString& rURL,
                                        const OUString& rTarget,
                                        const Point* pInsPos )
{
    // protected sheet ?

    ScViewData& rViewData = GetViewData();
    ScDocument* pDoc = rViewData.GetDocument();
    SCTAB nTab = rViewData.GetTabNo();
    if ( pDoc->IsTabProtected(nTab) )
    {
        ErrorMessage(STR_PROTECTIONERR);
        return;
    }

    MakeDrawLayer();

    ScTabView*  pView   = rViewData.GetView();
    ScDrawView* pDrView = pView->GetScDrawView();
    SdrModel*   pModel  = pDrView->GetModel();

    SdrObject* pObj = SdrObjFactory::MakeNewObject(FmFormInventor, OBJ_FM_BUTTON,
                               pDrView->GetSdrPageView()->GetPage(), pModel);
    SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( pObj );
    OSL_ENSURE( pUnoCtrl, "no SdrUnoObj");
    if( !pUnoCtrl )
        return;

    uno::Reference<awt::XControlModel> xControlModel = pUnoCtrl->GetUnoControlModel();
    OSL_ENSURE( xControlModel.is(), "UNO control without model" );
    if( !xControlModel.is() )
        return;

    uno::Reference< beans::XPropertySet > xPropSet( xControlModel, uno::UNO_QUERY );

    xPropSet->setPropertyValue("Label", uno::Any(rName) );

    OUString aTmp = INetURLObject::GetAbsURL( pDoc->GetDocumentShell()->GetMedium()->GetBaseURL(), rURL );
    xPropSet->setPropertyValue("TargetURL", uno::Any(aTmp) );

    if( !rTarget.isEmpty() )
    {
        xPropSet->setPropertyValue("TargetFrame", uno::Any(rTarget) );
    }

    form::FormButtonType eButtonType = form::FormButtonType_URL;
    xPropSet->setPropertyValue("ButtonType", uno::Any(eButtonType) );

        if ( ::avmedia::MediaWindow::isMediaURL( rURL, ""/*TODO?*/ ) )
    {
        xPropSet->setPropertyValue("DispatchURLInternal", uno::Any(true) );
    }

    Point aPos;
    if (pInsPos)
        aPos = *pInsPos;
    else
        aPos = GetInsertPos();

    // Size as in 3.1:
    Size aSize = GetActiveWin()->PixelToLogic(Size(140, 20));

    if ( pDoc->IsNegativePage(nTab) )
        aPos.X() -= aSize.Width();

    pObj->SetLogicRect(Rectangle(aPos, aSize));

    // for the old VC-Button the position/size had to be set explicitly once more
    // that seems not to be needed with UnoControls

    // do not mark when Ole
    pDrView->InsertObjectSafe( pObj, *pDrView->GetSdrPageView() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
