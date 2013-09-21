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

#include "WrapPropertyPanel.hxx"
#include "PropertyPanel.hrc"

#include <cmdid.h>
#include <swtypes.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/imagemgr.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>

#include "com/sun/star/lang/IllegalArgumentException.hpp"

const char UNO_WRAPOFF[] = ".uno:WrapOff";
const char UNO_WRAPLEFT[] = ".uno:WrapLeft";
const char UNO_WRAPRIGHT[] = ".uno:WrapRight";
const char UNO_WRAPON[] = ".uno:WrapOn";
const char UNO_WRAPTHROUGH[] = ".uno:WrapThrough";
const char UNO_WRAPIDEAL[] = ".uno:WrapIdeal";

namespace sw { namespace sidebar {

WrapPropertyPanel* WrapPropertyPanel::Create (
    Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw ::com::sun::star::lang::IllegalArgumentException("no parent Window given to WrapPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to WrapPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to WrapPropertyPanel::Create", NULL, 2);

    return new WrapPropertyPanel(
        pParent,
        rxFrame,
        pBindings);
}


WrapPropertyPanel::WrapPropertyPanel(
    Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings )
    : PanelLayout(pParent, "WrapPropertyPanel", "modules/swriter/ui/sidebarwrap.ui", rxFrame)
    , mxFrame( rxFrame )
    , mpBindings(pBindings)
    // resources
    , aWrapIL(6,2)
    // controller items
    , maSwNoWrapControl(FN_FRAME_NOWRAP, *pBindings, *this)
    , maSwWrapLeftControl(FN_FRAME_WRAP, *pBindings, *this)
    , maSwWrapRightControl(FN_FRAME_WRAP_RIGHT, *pBindings, *this)
    , maSwWrapParallelControl(FN_FRAME_WRAP_LEFT, *pBindings, *this)
    , maSwWrapThroughControl(FN_FRAME_WRAPTHRU, *pBindings, *this)
    , maSwWrapIdealControl(FN_FRAME_WRAP_IDEAL, *pBindings, *this)
{
    get(mpRBNoWrap, "buttonnone");
    get(mpRBWrapLeft, "buttonbefore");
    get(mpRBWrapRight, "buttonafter");
    get(mpRBWrapParallel, "buttonparallel");
    get(mpRBWrapThrough, "buttonthrough");
    get(mpRBIdealWrap, "buttonoptimal");

    Initialize();
}


WrapPropertyPanel::~WrapPropertyPanel()
{
}


void WrapPropertyPanel::Initialize()
{
    Link aLink = LINK(this, WrapPropertyPanel, WrapTypeHdl);
    mpRBNoWrap->SetClickHdl(aLink);
    mpRBWrapLeft->SetClickHdl(aLink);
    mpRBWrapRight->SetClickHdl(aLink);
    mpRBWrapParallel->SetClickHdl(aLink);
    mpRBWrapThrough->SetClickHdl(aLink);
    mpRBIdealWrap->SetClickHdl(aLink);

    aWrapIL.AddImage( UNO_WRAPOFF,
                      ::GetImage( mxFrame, UNO_WRAPOFF, sal_False ) );
    aWrapIL.AddImage( UNO_WRAPLEFT,
                      ::GetImage( mxFrame, UNO_WRAPLEFT, sal_False ) );
    aWrapIL.AddImage( UNO_WRAPRIGHT,
                      ::GetImage( mxFrame, UNO_WRAPRIGHT, sal_False ) );
    aWrapIL.AddImage( UNO_WRAPON,
                      ::GetImage( mxFrame, UNO_WRAPON, sal_False ) );
    aWrapIL.AddImage( UNO_WRAPTHROUGH,
                      ::GetImage( mxFrame, UNO_WRAPTHROUGH, sal_False ) );
    aWrapIL.AddImage( UNO_WRAPIDEAL,
                      ::GetImage( mxFrame, UNO_WRAPIDEAL, sal_False ) );

    mpRBNoWrap->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPOFF) );
    if ( Application::GetSettings().GetLayoutRTL() )
    {
        mpRBWrapLeft->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPRIGHT) );
        mpRBWrapRight->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPLEFT) );
    }
    else
    {
        mpRBWrapLeft->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPLEFT) );
        mpRBWrapRight->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPRIGHT) );
    }
    mpRBWrapParallel->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPON) );
    mpRBWrapThrough->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPTHROUGH) );
    mpRBIdealWrap->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPIDEAL) );

    mpRBNoWrap->SetAccessibleName(mpRBNoWrap->GetQuickHelpText());
    mpRBWrapLeft->SetAccessibleName(mpRBWrapLeft->GetQuickHelpText());
    mpRBWrapRight->SetAccessibleName(mpRBWrapRight->GetQuickHelpText());
    mpRBWrapParallel->SetAccessibleName(mpRBWrapParallel->GetQuickHelpText());
    mpRBWrapThrough->SetAccessibleName(mpRBWrapThrough->GetQuickHelpText());
    mpRBIdealWrap->SetAccessibleName(mpRBIdealWrap->GetQuickHelpText());

    mpBindings->Update( FN_FRAME_NOWRAP );
    mpBindings->Update( FN_FRAME_WRAP );
    mpBindings->Update( FN_FRAME_WRAP_RIGHT );
    mpBindings->Update( FN_FRAME_WRAP_LEFT );
    mpBindings->Update( FN_FRAME_WRAPTHRU );
    mpBindings->Update( FN_FRAME_WRAP_IDEAL );
}


IMPL_LINK(WrapPropertyPanel, WrapTypeHdl, void *, EMPTYARG)
{
    sal_uInt16 nSlot = 0;
    if ( mpRBWrapLeft->IsChecked() )
    {
        nSlot = FN_FRAME_WRAP_LEFT;
    }
    else if( mpRBWrapRight->IsChecked() )
    {
        nSlot = FN_FRAME_WRAP_RIGHT;
    }
    else if ( mpRBWrapParallel->IsChecked() )
    {
        nSlot = FN_FRAME_WRAP;
    }
    else if( mpRBWrapThrough->IsChecked() )
    {
        nSlot = FN_FRAME_WRAPTHRU;
    }
    else if( mpRBIdealWrap->IsChecked() )
    {
        nSlot = FN_FRAME_WRAP_IDEAL;
    }
    else
    {
        nSlot = FN_FRAME_NOWRAP;
    }
    SfxBoolItem bStateItem( nSlot, sal_True );
    mpBindings->GetDispatcher()->Execute( nSlot, SFX_CALLMODE_RECORD, &bStateItem, 0L );

    return 0;
}


void WrapPropertyPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    if ( eState == SFX_ITEM_AVAILABLE &&
        pState->ISA(SfxBoolItem) )
    {
        //Set Radio Button enable
        mpRBNoWrap->Enable(true);
        mpRBWrapLeft->Enable(true);
        mpRBWrapRight->Enable(true);
        mpRBWrapParallel->Enable(true);
        mpRBWrapThrough->Enable(true);
        mpRBIdealWrap->Enable(true);

        const SfxBoolItem* pBoolItem = static_cast< const SfxBoolItem* >( pState );
        switch( nSId )
        {
        case FN_FRAME_WRAP_RIGHT:
            mpRBWrapRight->Check( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP_LEFT:
            mpRBWrapLeft->Check( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAPTHRU:
            mpRBWrapThrough->Check( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP_IDEAL:
            mpRBIdealWrap->Check( pBoolItem->GetValue() );
            break;
        case FN_FRAME_WRAP:
            mpRBWrapParallel->Check( pBoolItem->GetValue() );
            break;
        case FN_FRAME_NOWRAP:
        default:
            mpRBNoWrap->Check( pBoolItem->GetValue() );
            break;
        }
    }
    else
    {
        mpRBNoWrap->Enable(false);
        mpRBWrapLeft->Enable(false);
        mpRBWrapRight->Enable(false);
        mpRBWrapParallel->Enable(false);
        mpRBWrapThrough->Enable(false);
        mpRBIdealWrap->Enable(false);

        mpRBNoWrap->Check( sal_False );
        mpRBWrapLeft->Check( sal_False );
        mpRBWrapRight->Check( sal_False );
        mpRBWrapParallel->Check( sal_False );
        mpRBWrapThrough->Check( sal_False );
        mpRBIdealWrap->Check( sal_False );
    }
}

} } // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
