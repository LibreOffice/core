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

#include <config_features.h>

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

#include <docsh.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <avmedia/mediawindow.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

void SwBaseShell::InsertURLButton(const OUString& rURL, const OUString& rTarget, const OUString& rText)
{
    SwWrtShell& rSh = GetShell();

    if (!rSh.HasDrawView())
        rSh.MakeDrawView();
    SdrView *pSdrView = rSh.GetDrawView();

    // OBJ_FM_BUTTON
    pSdrView->SetDesignMode();
    pSdrView->SetCurrentObj(OBJ_FM_BUTTON);
    pSdrView->SetEditMode(false);

    Point aStartPos(rSh.GetCharRect().Pos() + Point(0, 1));

    rSh.StartAction();
    rSh.StartUndo( SwUndoId::UI_INSERT_URLBTN );
    if (rSh.BeginCreate(OBJ_FM_BUTTON, SdrInventor::FmForm, aStartPos))
    {
        pSdrView->SetOrtho(false);
        Size aSz(GetView().GetEditWin().PixelToLogic(Size(140, 20)));
        Point aEndPos(aSz.Width(), aSz.Height());

        rSh.MoveCreate(aStartPos + aEndPos);
        rSh.EndCreate(SdrCreateCmd::ForceEnd);

        const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();
        if (rMarkList.GetMark(0))
        {
            SdrUnoObj* pUnoCtrl = dynamic_cast<SdrUnoObj*>( rMarkList.GetMark(0)->GetMarkedSdrObj() );
            OSL_ENSURE( pUnoCtrl, "not an SdrUnoObj" );
            if (!pUnoCtrl)
                return;

            uno::Reference< awt::XControlModel >  xControlModel = pUnoCtrl->GetUnoControlModel();

            OSL_ENSURE( xControlModel.is(), "UNO-Control without Model" );
            if (!xControlModel.is())
                return;

            uno::Reference< beans::XPropertySet >  xPropSet(xControlModel, uno::UNO_QUERY);

            uno::Any aTmp;

            aTmp <<= rText;
            xPropSet->setPropertyValue( "Label", aTmp );

            SfxMedium* pMedium = rSh.GetView().GetDocShell()->GetMedium();
            INetURLObject aAbs;
            if( pMedium )
                aAbs = pMedium->GetURLObject();

            aTmp <<= URIHelper::SmartRel2Abs(aAbs, rURL);
            xPropSet->setPropertyValue( "TargetURL", aTmp );

            if( !rTarget.isEmpty() )
            {
                aTmp <<= rTarget;
                xPropSet->setPropertyValue( "TargetFrame", aTmp );
            }

            aTmp <<= form::FormButtonType_URL;
            xPropSet->setPropertyValue( "ButtonType", aTmp );

#if HAVE_FEATURE_AVMEDIA
            if ( ::avmedia::MediaWindow::isMediaURL( rURL, ""/*TODO?*/ ) )
            {
                // #105638# OJ
                aTmp <<= true;
                xPropSet->setPropertyValue("DispatchURLInternal", aTmp );
            }
#endif
        }

        if (rSh.IsObjSelected())
        {
            rSh.UnSelectFrame();
        }
    }
    rSh.EndUndo( SwUndoId::UI_INSERT_URLBTN );
    rSh.EndAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
