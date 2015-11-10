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
#include "fuzoom.hxx"

#include <vcl/msgbox.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/zoom_def.hxx>
#include <sfx2/zoomitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svxdlg.hxx>
#include <memory>

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

rtl::Reference<FuPoor> FuScale::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuScale( pViewSh, pWin, pView, pDoc, rReq ) );
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
        std::unique_ptr<SvxZoomItem> pZoomItem;
        SvxZoomEnableFlags nZoomValues = SvxZoomEnableFlags::ALL;

        nValue = (sal_Int16) mpWindow->GetZoom();

        // zoom on page size?
        if( mpViewShell && dynamic_cast< DrawViewShell *>( mpViewShell ) !=  nullptr &&
            static_cast<DrawViewShell*>(mpViewShell)->IsZoomOnPage() )
        {
            pZoomItem.reset(new SvxZoomItem( SvxZoomType::WHOLEPAGE, nValue ));
        }
        else
        {
            pZoomItem.reset(new SvxZoomItem( SvxZoomType::PERCENT, nValue ));
        }

        // limit range
        if( mpViewShell )
        {
            if( dynamic_cast< DrawViewShell *>( mpViewShell ) !=  nullptr )
            {
                SdrPageView* pPageView = mpView->GetSdrPageView();
                if( ( pPageView && pPageView->GetObjList()->GetObjCount() == 0 ) )
                    // || ( mpView->GetMarkedObjectList().GetMarkCount() == 0 ) )
                {
                    nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
                }
            }
            else if( dynamic_cast< OutlineViewShell *>( mpViewShell ) !=  nullptr )
            {
                nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
                nZoomValues &= ~SvxZoomEnableFlags::WHOLEPAGE;
                nZoomValues &= ~SvxZoomEnableFlags::PAGEWIDTH;
            }
        }

        pZoomItem->SetValueSet( nZoomValues );
        aNewAttr.Put( *pZoomItem );

        std::unique_ptr<AbstractSvxZoomDialog> pDlg;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            pDlg.reset(pFact->CreateSvxZoomDialog(nullptr, aNewAttr));
        }

        if( pDlg )
        {
            pDlg->SetLimits( (sal_uInt16)mpWindow->GetMinZoom(), (sal_uInt16)mpWindow->GetMaxZoom() );
            sal_uInt16 nResult = pDlg->Execute();
            switch( nResult )
            {
                case RET_CANCEL:
                {
                    rReq.Ignore ();
                    return; // Cancel
                }
                default:
                {
                    rReq.Ignore ();
                }
                break;
            }

            const SfxItemSet aArgs (*(pDlg->GetOutputItemSet ()));

            pDlg.reset();

            if (!mpViewShell)
                return;

            switch (static_cast<const SvxZoomItem &>( aArgs.Get (SID_ATTR_ZOOM)).GetType ())
            {
                case SvxZoomType::PERCENT:
                {
                    nValue = static_cast<const SvxZoomItem &>( aArgs.Get (SID_ATTR_ZOOM)).GetValue ();

                    mpViewShell->SetZoom( nValue );

                    mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
                }
                break;

                case SvxZoomType::OPTIMAL:
                {
                    if( dynamic_cast< DrawViewShell *>( mpViewShell ) !=  nullptr )
                    {
                        // name confusion: SID_SIZE_ALL -> zoom onto all objects
                        // --> the program offers it as optimal
                        mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                    }
                }
                break;

                case SvxZoomType::PAGEWIDTH:
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                    break;

                case SvxZoomType::WHOLEPAGE:
                    mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                    break;
                default:
                    break;
            }
        }
    }
    else if(mpViewShell && (pArgs->Count () == 1))
    {
        const SfxUInt32Item* pScale = rReq.GetArg<SfxUInt32Item>(ID_VAL_ZOOM);
        mpViewShell->SetZoom (pScale->GetValue ());

        mpViewShell->GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
    }

}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
