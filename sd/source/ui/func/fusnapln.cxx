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

#include <fusnapln.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svxids.hrc>

#include <strings.hrc>
#include <sdattr.hrc>

#include <View.hxx>
#include <ViewShell.hxx>
#include <DrawViewShell.hxx>
#include <Window.hxx>
#include <sdenumdef.hxx>
#include <sdresid.hxx>
#include <sdabstdlg.hxx>
#include <svx/svdpagv.hxx>

namespace sd {


FuSnapLine::FuSnapLine(ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView,
                       SdDrawDocument& rDoc, SfxRequest& rReq) :
    FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuSnapLine::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuSnapLine( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuSnapLine::DoExecute( SfxRequest& rReq )
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    sal_uInt16  nHelpLine = 0;
    bool    bCreateNew = true;

    // Get index of snap line or snap point from the request.
    const SfxUInt32Item* pHelpLineIndex = rReq.GetArg<SfxUInt32Item>(ID_VAL_INDEX);
    if (pHelpLineIndex != nullptr)
    {
        nHelpLine = static_cast<sal_uInt16>(pHelpLineIndex->GetValue());
        // Reset the argument pointer to trigger the display of the dialog.
        pArgs = nullptr;
    }

    SdrPageView* pPV = mpView->GetSdrPageView();

    if (!pArgs)
    {
        SfxItemSetFixed<ATTR_SNAPLINE_START, ATTR_SNAPLINE_END> aNewAttr(mrViewShell.GetPool());
        bool bLineExist (false);
        Point aLinePos;

        if (pHelpLineIndex == nullptr)
        {
            // The index of the snap line is not provided as argument to the
            // request.  Determine it from the mouse position.

            aLinePos = static_cast<DrawViewShell*>(&mrViewShell)->GetMousePos();

            if ( aLinePos.X() >= 0 )
            {
                aLinePos = mpWindow->PixelToLogic(aLinePos);
                sal_uInt16 nHitLog = static_cast<sal_uInt16>(mpWindow->PixelToLogic(Size(HITPIX,0)).Width());
                bLineExist = mpView->PickHelpLine(aLinePos, nHitLog, *mpWindow->GetOutDev(), nHelpLine, pPV);
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
            assert(pPV!=nullptr);
            aLinePos = (pPV->GetHelpLines())[nHelpLine].GetPos();
            pPV->LogicToPagePos(aLinePos);
            bLineExist = true;
        }
        aNewAttr.Put(SfxInt32Item(ATTR_SNAPLINE_X, aLinePos.X()));
        aNewAttr.Put(SfxInt32Item(ATTR_SNAPLINE_Y, aLinePos.Y()));

        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        vcl::Window* pWin = mrViewShell.GetActiveWindow();
        ScopedVclPtr<AbstractSdSnapLineDlg> pDlg( pFact->CreateSdSnapLineDlg(pWin ? pWin->GetFrameWeld() : nullptr, aNewAttr, mpView) );

        if ( bLineExist )
        {
            pDlg->HideRadioGroup();

            const SdrHelpLine& rHelpLine = (pPV->GetHelpLines())[nHelpLine];

            if ( rHelpLine.GetKind() == SdrHelpLineKind::Point )
            {
                pDlg->SetText(SdResId(STR_SNAPDLG_SETPOINT));
                pDlg->SetInputFields(true, true);
            }
            else
            {
                pDlg->SetText(SdResId(STR_SNAPDLG_SETLINE));

                if ( rHelpLine.GetKind() == SdrHelpLineKind::Vertical )
                    pDlg->SetInputFields(true, false);
                else
                    pDlg->SetInputFields(false, true);
            }
            bCreateNew = false;
        }
        else
            pDlg->HideDeleteBtn();

        sal_uInt16 nResult = pDlg->Execute();

        pDlg->GetAttr(aNewAttr);
        pDlg.disposeAndClear();

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
                [[fallthrough]];
            default:
                return;
        }
    }
    Point aHlpPos;

    aHlpPos.setX( pArgs->Get(ATTR_SNAPLINE_X).GetValue() );
    aHlpPos.setY( pArgs->Get(ATTR_SNAPLINE_Y).GetValue() );
    pPV->PagePosToLogic(aHlpPos);

    if ( bCreateNew )
    {
        SdrHelpLineKind eKind;

        pPV = mpView->GetSdrPageView();

        switch ( static_cast<SnapKind>(pArgs->Get(ATTR_SNAPLINE_KIND).GetValue()) )
        {
            case SnapKind::Horizontal  : eKind = SdrHelpLineKind::Horizontal;   break;
            case SnapKind::Vertical    : eKind = SdrHelpLineKind::Vertical;     break;
            default                    : eKind = SdrHelpLineKind::Point;        break;
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
