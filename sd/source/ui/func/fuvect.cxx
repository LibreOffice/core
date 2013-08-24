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

#include "fuvect.hxx"
#include <tools/poly.hxx>
#include <svx/svdograf.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdedtv.hxx>


#include "View.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdabstdlg.hxx"

namespace sd
{

TYPEINIT1( FuVectorize, FuPoor );

FuVectorize::FuVectorize (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor (pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuVectorize::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuVectorize( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuVectorize::DoExecute( SfxRequest& )
{
    const SdrMarkList& rMarkList = mpView->GetMarkedObjectList();

    if( rMarkList.GetMarkCount() == 1 )
    {
        SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

        if( pObj && pObj->ISA( SdrGrafObj ) )
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            AbstractSdVectorizeDlg* pDlg = pFact ? pFact->CreateSdVectorizeDlg( mpWindow, ( (SdrGrafObj*) pObj )->GetGraphic().GetBitmap(), mpDocSh ) : 0;
            if( pDlg && pDlg->Execute() == RET_OK )
            {
                const GDIMetaFile&  rMtf = pDlg->GetGDIMetaFile();
                SdrPageView*        pPageView = mpView->GetSdrPageView();

                if( pPageView && rMtf.GetActionSize() )
                {
                    SdrGrafObj* pVectObj = (SdrGrafObj*) pObj->Clone();
                    String      aStr( mpView->GetDescriptionOfMarkedObjects() );

                    aStr.Append( sal_Unicode(' ') );
                    aStr.Append( SD_RESSTR( STR_UNDO_VECTORIZE ) );
                    mpView->BegUndo( aStr );
                    pVectObj->SetGraphic( rMtf );
                    mpView->ReplaceObjectAtView( pObj, *pPageView, pVectObj );
                    mpView->EndUndo();
                }
            }
            delete pDlg;
        }
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
