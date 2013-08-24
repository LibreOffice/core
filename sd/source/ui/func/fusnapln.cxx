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


#include "fusnapln.hxx"
#include <svl/aeitem.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>


#include "strings.hrc"

#include "sdattr.hxx"
#include "View.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"
#include "sdenumdef.hxx"
#include "sdresid.hxx"
#include "sdabstdlg.hxx"
#include "app.hrc"
#include <svx/svdpagv.hxx>

namespace sd {

TYPEINIT1( FuSnapLine, FuPoor );

FuSnapLine::FuSnapLine(ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView,
                       SdDrawDocument* pDoc, SfxRequest& rReq) :
    FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuSnapLine::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuSnapLine( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSnapLine::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16  nHelpLine = 0;
    sal_Bool    bCreateNew = sal_True;

    // Get index of snap line or snap point from the request.
    SFX_REQUEST_ARG (rReq, pHelpLineIndex, SfxUInt32Item, ID_VAL_INDEX, sal_False);
    if (pHelpLineIndex != NULL)
    {
        nHelpLine = static_cast<sal_uInt16>(pHelpLineIndex->GetValue());
        // Reset the argument pointer to trigger the display of the dialog.
        pArgs = NULL;
    }

    SdrPageView* pPV = mpView->GetSdrPageView();

    if (!pArgs)
    {
        SfxItemSet aNewAttr(mpViewShell->GetPool(), ATTR_SNAPLINE_START, ATTR_SNAPLINE_END);
        bool bLineExist (false);
        Point aLinePos;

        if (pHelpLineIndex == NULL)
        {
            // The index of the snap line is not provided as argument to the
            // request.  Determine it from the mouse position.

            aLinePos = static_cast<DrawViewShell*>(mpViewShell)->GetMousePos();
            static_cast<DrawViewShell*>(mpViewShell)->SetMousePosFreezed( sal_False );


            if ( aLinePos.X() >= 0 )
            {
                aLinePos = mpWindow->PixelToLogic(aLinePos);
                sal_uInt16 nHitLog = (sal_uInt16) mpWindow->PixelToLogic(Size(HITPIX,0)).Width();
                bLineExist = mpView->PickHelpLine(aLinePos, nHitLog, *mpWindow, nHelpLine, pPV);
                if ( bLineExist )
                    aLinePos = (pPV->GetHelpLines())[nHelpLine].GetPos();
                else
                    pPV = mpView->GetSdrPageView();

                pPV->LogicToPagePos(aLinePos);
            }
            else
                aLinePos = Point(0,0);
        }
        else
        {
            OSL_ASSERT(pPV!=NULL);
            aLinePos = (pPV->GetHelpLines())[nHelpLine].GetPos();
            pPV->LogicToPagePos(aLinePos);
            bLineExist = true;
        }
        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_X, aLinePos.X()));
        aNewAttr.Put(SfxUInt32Item(ATTR_SNAPLINE_Y, aLinePos.Y()));

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        AbstractSdSnapLineDlg* pDlg = pFact ? pFact->CreateSdSnapLineDlg( NULL, aNewAttr, mpView ) : 0;
        OSL_ASSERT(pDlg);
        if (!pDlg)
            return;

        if ( bLineExist )
        {
            pDlg->HideRadioGroup();

            const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];

            if ( rHelpLine.GetKind() == SDRHELPLINE_POINT )
            {
                pDlg->SetText(SD_RESSTR(STR_SNAPDLG_SETPOINT));
                pDlg->SetInputFields(sal_True, sal_True);
            }
            else
            {
                pDlg->SetText(SD_RESSTR(STR_SNAPDLG_SETLINE));

                if ( rHelpLine.GetKind() == SDRHELPLINE_VERTICAL )
                    pDlg->SetInputFields(sal_True, sal_False);
                else
                    pDlg->SetInputFields(sal_False, sal_True);
            }
            bCreateNew = sal_False;
        }
        else
            pDlg->HideDeleteBtn();

        sal_uInt16 nResult = pDlg->Execute();

        pDlg->GetAttr(aNewAttr);
        delete pDlg;

        switch( nResult )
        {
            case RET_OK:
                rReq.Done(aNewAttr);
                pArgs = rReq.GetArgs();
                break;

            case RET_SNAP_DELETE:
                // delete snap object
                if ( !bCreateNew )
                    pPV->DeleteHelpLine(nHelpLine);
                /*fall-through*/
            default:
                return;
        }
    }
    Point aHlpPos;

    aHlpPos.X() = ((const SfxUInt32Item&) pArgs->Get(ATTR_SNAPLINE_X)).GetValue();
    aHlpPos.Y() = ((const SfxUInt32Item&) pArgs->Get(ATTR_SNAPLINE_Y)).GetValue();
    pPV->PagePosToLogic(aHlpPos);

    if ( bCreateNew )
    {
        SdrHelpLineKind eKind;

        pPV = mpView->GetSdrPageView();

        switch ( (SnapKind) ((const SfxAllEnumItem&)
                 pArgs->Get(ATTR_SNAPLINE_KIND)).GetValue() )
        {
            case SK_HORIZONTAL  : eKind = SDRHELPLINE_HORIZONTAL;   break;
            case SK_VERTICAL    : eKind = SDRHELPLINE_VERTICAL;     break;
            default             : eKind = SDRHELPLINE_POINT;        break;
        }
        pPV->InsertHelpLine(SdrHelpLine(eKind, aHlpPos));
    }
    else
    {
        const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];
        pPV->SetHelpLine(nHelpLine, SdrHelpLine(rHelpLine.GetKind(), aHlpPos));
    }
}

void FuSnapLine::Activate()
{
}

void FuSnapLine::Deactivate()
{
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
