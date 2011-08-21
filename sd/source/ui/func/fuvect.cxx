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
#include "precompiled_sd.hxx"


#include "fuvect.hxx"
#include <tools/poly.hxx>
#include <svx/svdograf.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdograf.hxx>
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

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

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
                    aStr.Append( String( SdResId( STR_UNDO_VECTORIZE ) ) );
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
