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

#include "fuscale.hxx"

#include <svx/dialogs.hrc>

#include "app.hrc"
#include "View.hxx"
#include "Window.hxx"
#include "OutlineViewShell.hxx"
#include "drawview.hxx"
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "ViewShell.hxx"
#include "fuzoom.hxx" // because of SidArrayZoom[]

#include <vcl/msgbox.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/zoom_def.hxx>
#include <sfx2/zoomitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svxdlg.hxx>

namespace sd {

TYPEINIT1( FuScale, FuPoor );

FuScale::FuScale (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuScale::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuScale( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuScale::DoExecute( SfxRequest& rReq )
{
    sal_Int16 nValue;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSet aNewAttr( mpDoc->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM );
        SvxZoomItem* pZoomItem;
        sal_uInt16 nZoomValues = SVX_ZOOM_ENABLE_ALL;

        nValue = (sal_Int16) mpWindow->GetZoom();

        // zoom on page size?
        if( mpViewShell && mpViewShell->ISA( DrawViewShell ) &&
            static_cast<DrawViewShell*>(mpViewShell)->IsZoomOnPage() )
        {
            pZoomItem = new SvxZoomItem( SVX_ZOOM_WHOLEPAGE, nValue );
        }
        else
        {
            pZoomItem = new SvxZoomItem( SVX_ZOOM_PERCENT, nValue );
        }

        // limit range
        if( mpViewShell )
        {
            if( mpViewShell->ISA( DrawViewShell ) )
            {
                SdrPageView* pPageView = mpView->GetSdrPageView();
                if( ( pPageView && pPageView->GetObjList()->GetObjCount() == 0 ) )
                    // || ( mpView->GetMarkedObjectList().GetMarkCount() == 0 ) )
                {
                    nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
                }
            }
            else if( mpViewShell->ISA( OutlineViewShell ) )
            {
                nZoomValues &= ~SVX_ZOOM_ENABLE_OPTIMAL;
                nZoomValues &= ~SVX_ZOOM_ENABLE_WHOLEPAGE;
                nZoomValues &= ~SVX_ZOOM_ENABLE_PAGEWIDTH;
            }
        }

        pZoomItem->SetValueSet( nZoomValues );
        aNewAttr.Put( *pZoomItem );

        AbstractSvxZoomDialog* pDlg=NULL;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            pDlg = pFact->CreateSvxZoomDialog(NULL, aNewAttr);
        }

        if( pDlg )
        {
            pDlg->SetLimits( (sal_uInt16)mpWindow->GetMinZoom(), (sal_uInt16)mpWindow->GetMaxZoom() );
            sal_uInt16 nResult = pDlg->Execute();
            switch( nResult )
            {
                case RET_CANCEL:
                {
                    delete pDlg;
                    delete pZoomItem;
                    rReq.Ignore ();
                    return; // Cancel
                }
                default:
                {
                    rReq.Ignore ();
        /*
                        rReq.Done( *( pDlg->GetOutputItemSet() ) );
                        pArgs = rReq.GetArgs();*/
                }
                break;
            }

            const SfxItemSet aArgs (*(pDlg->GetOutputItemSet ()));

            delete pDlg;

            switch (((const SvxZoomItem &) aArgs.Get (SID_ATTR_ZOOM)).GetType ())
            {
                case SVX_ZOOM_PERCENT:
                {
                    nValue = ((const SvxZoomItem &) aArgs.Get (SID_ATTR_ZOOM)).GetValue ();

                    mpViewShell->SetZoom( nValue );

                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
                }
                break;

                case SVX_ZOOM_OPTIMAL:
                {
                    if( mpViewShell->ISA( DrawViewShell ) )
                    {
                        // name confusion: SID_SIZE_ALL -> zoom onto all objects
                        // --> the program offers it as optimal
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    }
                }
                break;

                case SVX_ZOOM_PAGEWIDTH:
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    break;

                case SVX_ZOOM_WHOLEPAGE:
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
                    break;
                default:
                    break;
            }
        }

        delete pZoomItem;
    }
    else if(mpViewShell && (pArgs->Count () == 1))
    {
        SFX_REQUEST_ARG (rReq, pScale, SfxUInt32Item, ID_VAL_ZOOM, sal_False);
        mpViewShell->SetZoom (pScale->GetValue ());

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
    }

}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
