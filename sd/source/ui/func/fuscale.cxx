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

#include <fuscale.hxx>

#include <svx/svxids.hrc>

#include <app.hrc>
#include <View.hxx>
#include <Window.hxx>
#include <OutlineViewShell.hxx>
#include <drawdoc.hxx>
#include <DrawViewShell.hxx>
#include <ViewShell.hxx>
#include <fuzoom.hxx>

#include <svx/svdpagv.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/zoomitem.hxx>
#include <sfx2/request.hxx>
#include <svx/svxdlg.hxx>
#include <memory>

namespace sd {


FuScale::FuScale (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument& rDoc,
    SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuScale::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuScale( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuScale::DoExecute( SfxRequest& rReq )
{
    sal_Int16 nValue;

    const SfxItemSet* pArgs = rReq.GetArgs();

    if( !pArgs )
    {
        SfxItemSetFixed<SID_ATTR_ZOOM, SID_ATTR_ZOOM> aNewAttr( mrDoc.GetPool() );
        std::unique_ptr<SvxZoomItem> pZoomItem;
        SvxZoomEnableFlags nZoomValues = SvxZoomEnableFlags::ALL;

        nValue = static_cast<sal_Int16>(mpWindow->GetZoom());

        // zoom on page size?
        if( dynamic_cast< DrawViewShell *>( &mrViewShell ) &&
            static_cast<DrawViewShell*>(&mrViewShell)->IsZoomOnPage() )
        {
            pZoomItem.reset(new SvxZoomItem( SvxZoomType::WHOLEPAGE, nValue ));
        }
        else
        {
            pZoomItem.reset(new SvxZoomItem( SvxZoomType::PERCENT, nValue ));
        }

        // limit range
        if( dynamic_cast< DrawViewShell *>( &mrViewShell ) !=  nullptr )
        {
            SdrPageView* pPageView = mpView->GetSdrPageView();
            if( pPageView && pPageView->GetObjList()->GetObjCount() == 0 )
            {
                nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
            }
        }
        else if( dynamic_cast< OutlineViewShell *>( &mrViewShell ) !=  nullptr )
        {
            nZoomValues &= ~SvxZoomEnableFlags::OPTIMAL;
            nZoomValues &= ~SvxZoomEnableFlags::WHOLEPAGE;
            nZoomValues &= ~SvxZoomEnableFlags::PAGEWIDTH;
        }

        pZoomItem->SetValueSet( nZoomValues );
        aNewAttr.Put( std::move(pZoomItem) );

        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxZoomDialog> pDlg(pFact->CreateSvxZoomDialog(rReq.GetFrameWeld(), aNewAttr));
        pDlg->SetLimits( static_cast<sal_uInt16>(mpWindow->GetMinZoom()), static_cast<sal_uInt16>(mpWindow->GetMaxZoom()) );
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

        pDlg.disposeAndClear();

        switch ( aArgs.Get (SID_ATTR_ZOOM).GetType ())
        {
            case SvxZoomType::PERCENT:
            {
                nValue = aArgs.Get (SID_ATTR_ZOOM).GetValue ();

                mrViewShell.SetZoom( nValue );

                mrViewShell.GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
            }
            break;

            case SvxZoomType::OPTIMAL:
            {
                if( dynamic_cast< DrawViewShell *>( &mrViewShell ) !=  nullptr )
                {
                    // name confusion: SID_SIZE_ALL -> zoom onto all objects
                    // --> the program offers it as optimal
                    mrViewShell.GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                }
            }
            break;

            case SvxZoomType::PAGEWIDTH:
                mrViewShell.GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                break;

            case SvxZoomType::WHOLEPAGE:
                mrViewShell.GetViewFrame()->GetDispatcher()->Execute(SID_SIZE_PAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
                break;
            default:
                break;
        }
    }
    else if(pArgs->Count () == 1)
    {
        const SfxUInt32Item* pScale = rReq.GetArg<SfxUInt32Item>(ID_VAL_ZOOM);
        mrViewShell.SetZoom (pScale->GetValue ());

        mrViewShell.GetViewFrame()->GetBindings().Invalidate( SidArrayZoom );
    }

}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
