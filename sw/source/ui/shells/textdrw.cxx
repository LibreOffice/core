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

#include <svx/svdview.hxx>
#include <tools/urlobj.hxx>
#include <svx/fmglob.hxx>
#include <svx/svdouno.hxx>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <view.hxx>
#include <wrtsh.hxx>
#include <edtwin.hxx>
#include <swundo.hxx>
#include <basesh.hxx>

#include <poolfmt.hrc>

#include <docsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <avmedia/mediawindow.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

void SwBaseShell::InsertURLButton(const String& rURL, const String& rTarget, const String& rTxt)
{
    SwWrtShell& rSh = GetShell();

    if (!rSh.HasDrawView())
        rSh.MakeDrawView();
    SdrView *pSdrView = rSh.GetDrawView();

    // OBJ_FM_BUTTON
    pSdrView->SetDesignMode(sal_True);
    pSdrView->SetCurrentObj(OBJ_FM_BUTTON);
    pSdrView->SetEditMode(sal_False);

    Point aStartPos(rSh.GetCharRect().Pos() + Point(0, 1));

    rSh.StartAction();
    rSh.StartUndo( UNDO_UI_INSERT_URLBTN );
    if (rSh.BeginCreate(OBJ_FM_BUTTON, FmFormInventor, aStartPos))
    {
        pSdrView->SetOrtho(sal_False);
         Size aSz(GetView().GetEditWin().PixelToLogic(Size(140, 20)));
        Point aEndPos(aSz.Width(), aSz.Height());

        rSh.MoveCreate(aStartPos + aEndPos);
        rSh.EndCreate(SDRCREATE_FORCEEND);

        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = PTR_CAST(SdrUnoObj, rMarkList.GetMark(0)->GetMarkedSdrObj());
            uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

            OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
            if( !xControlModel.is() )
                return;

            uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);


            uno::Any aTmp;

            aTmp <<= OUString(rTxt);
            xPropSet->setPropertyValue( "Label", aTmp );

            SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();

            aTmp <<= OUString(URIHelper::SmartRel2Abs(aAbs, rURL));
            xPropSet->setPropertyValue( "TargetURL", aTmp );

            if( rTarget.Len() )
            {
                aTmp <<= OUString(rTarget);
                xPropSet->setPropertyValue( "TargetFrame", aTmp );
            }


            form::FormButtonType eButtonType = form::FormButtonType_URL;
            aTmp.setValue( &eButtonType, ::getCppuType((const form::FormButtonType*)0));
            xPropSet->setPropertyValue( "ButtonType", aTmp );

            if ( ::avmedia::MediaWindow::isMediaURL( rURL ) )
            {
                // #105638# OJ
                aTmp <<= sal_True;
                xPropSet->setPropertyValue( OUString( "DispatchURLInternal" ), aTmp );
            }
        }

        if (rSh.IsObjSelected())
        {
            rSh.UnSelectFrm();
        }
    }
    rSh.EndUndo( UNDO_UI_INSERT_URLBTN );
    rSh.EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
