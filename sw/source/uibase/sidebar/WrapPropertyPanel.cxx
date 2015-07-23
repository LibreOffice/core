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
#include <svx/svxids.hrc>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sidebar/ControlFactory.hxx>
#include <sfx2/imagemgr.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/settings.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <hintids.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#define SPC_0 0
#define SPC_16 91
#define SPC_32 181
#define SPC_64 363
#define SPC_95 539
#define SPC_127 720
#define SPC_190 1077

const char UNO_WRAPOFF[] = ".uno:WrapOff";
const char UNO_WRAPLEFT[] = ".uno:WrapLeft";
const char UNO_WRAPRIGHT[] = ".uno:WrapRight";
const char UNO_WRAPON[] = ".uno:WrapOn";
const char UNO_WRAPTHROUGH[] = ".uno:WrapThrough";
const char UNO_WRAPIDEAL[] = ".uno:WrapIdeal";

namespace sw { namespace sidebar {

VclPtr<vcl::Window> WrapPropertyPanel::Create (
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings)
{
    if (pParent == NULL)
        throw ::com::sun::star::lang::IllegalArgumentException("no parent Window given to WrapPropertyPanel::Create", NULL, 0);
    if ( ! rxFrame.is())
        throw ::com::sun::star::lang::IllegalArgumentException("no XFrame given to WrapPropertyPanel::Create", NULL, 1);
    if (pBindings == NULL)
        throw ::com::sun::star::lang::IllegalArgumentException("no SfxBindings given to WrapPropertyPanel::Create", NULL, 2);

    return VclPtr<WrapPropertyPanel>::Create(
                        pParent,
                        rxFrame,
                        pBindings);
}

WrapPropertyPanel::WrapPropertyPanel(
    vcl::Window* pParent,
    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rxFrame,
    SfxBindings* pBindings )
    : PanelLayout(pParent, "WrapPropertyPanel", "modules/swriter/ui/sidebarwrap.ui", rxFrame)
    , mxFrame( rxFrame )
    , mpBindings(pBindings)
    // spacing
    , nTop(0)
    , nBottom(0)
    , nLeft(0)
    , nRight(0)
    // resources
    , aWrapIL(6,2)
    // controller items
    , maSwNoWrapControl(FN_FRAME_NOWRAP, *pBindings, *this)
    , maSwWrapLeftControl(FN_FRAME_WRAP, *pBindings, *this)
    , maSwWrapRightControl(FN_FRAME_WRAP_RIGHT, *pBindings, *this)
    , maSwWrapParallelControl(FN_FRAME_WRAP_LEFT, *pBindings, *this)
    , maSwWrapThroughControl(FN_FRAME_WRAPTHRU, *pBindings, *this)
    , maSwWrapIdealControl(FN_FRAME_WRAP_IDEAL, *pBindings, *this)
    , maSwEnableContourControl(FN_FRAME_WRAP_CONTOUR, *pBindings, *this)
    , maSwLRSpacingControl(SID_ATTR_LRSPACE, *pBindings, *this)
    , maSwULSpacingControl(SID_ATTR_ULSPACE, *pBindings, *this)
{
    get(mpRBNoWrap, "buttonnone");
    get(mpRBWrapLeft, "buttonbefore");
    get(mpRBWrapRight, "buttonafter");
    get(mpRBWrapParallel, "buttonparallel");
    get(mpRBWrapThrough, "buttonthrough");
    get(mpRBIdealWrap, "buttonoptimal");
    get(mpEnableContour, "enablecontour");
    get(mpEditContour, "editcontour");
    get(mpSpacingLB, "spacingLB");

    Initialize();
}

WrapPropertyPanel::~WrapPropertyPanel()
{
    disposeOnce();
}

void WrapPropertyPanel::dispose()
{
    mpRBNoWrap.clear();
    mpRBWrapLeft.clear();
    mpRBWrapRight.clear();
    mpRBWrapParallel.clear();
    mpRBWrapThrough.clear();
    mpRBIdealWrap.clear();
    mpEnableContour.clear();
    mpEditContour.clear();
    mpSpacingLB.clear();

    maSwNoWrapControl.dispose();
    maSwWrapLeftControl.dispose();
    maSwWrapRightControl.dispose();
    maSwWrapParallelControl.dispose();
    maSwWrapThroughControl.dispose();
    maSwWrapIdealControl.dispose();
    maSwEnableContourControl.dispose();
    maSwLRSpacingControl.dispose();
    maSwULSpacingControl.dispose();

    PanelLayout::dispose();
}

void WrapPropertyPanel::Initialize()
{
    Link<> aLink = LINK(this, WrapPropertyPanel, WrapTypeHdl);
    mpRBNoWrap->SetClickHdl(aLink);
    mpRBWrapLeft->SetClickHdl(aLink);
    mpRBWrapRight->SetClickHdl(aLink);
    mpRBWrapParallel->SetClickHdl(aLink);
    mpRBWrapThrough->SetClickHdl(aLink);
    mpRBIdealWrap->SetClickHdl(aLink);

    mpEditContour->SetClickHdl(LINK(this, WrapPropertyPanel, EditContourHdl));
    mpEnableContour->SetClickHdl(LINK(this, WrapPropertyPanel, EnableContourHdl));
    mpSpacingLB->SetSelectHdl(LINK(this, WrapPropertyPanel, SpacingLBHdl));

    aWrapIL.AddImage( UNO_WRAPOFF,
                      ::GetImage( mxFrame, UNO_WRAPOFF, false ) );
    aWrapIL.AddImage( UNO_WRAPLEFT,
                      ::GetImage( mxFrame, UNO_WRAPLEFT, false ) );
    aWrapIL.AddImage( UNO_WRAPRIGHT,
                      ::GetImage( mxFrame, UNO_WRAPRIGHT, false ) );
    aWrapIL.AddImage( UNO_WRAPON,
                      ::GetImage( mxFrame, UNO_WRAPON, false ) );
    aWrapIL.AddImage( UNO_WRAPTHROUGH,
                      ::GetImage( mxFrame, UNO_WRAPTHROUGH, false ) );
    aWrapIL.AddImage( UNO_WRAPIDEAL,
                      ::GetImage( mxFrame, UNO_WRAPIDEAL, false ) );

    mpRBNoWrap->SetModeRadioImage( aWrapIL.GetImage(UNO_WRAPOFF) );
    if ( AllSettings::GetLayoutRTL() )
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
    mpBindings->Update( FN_FRAME_WRAP_CONTOUR );
    mpBindings->Update( SID_ATTR_LRSPACE );
    mpBindings->Update( SID_ATTR_ULSPACE );

}

void WrapPropertyPanel::UpdateSpacingLB()
{
    if( (nLeft == nRight) && (nTop == nBottom) && (nLeft == nTop) )
    {
        if(nLeft == SPC_0)
            mpSpacingLB->SelectEntryPos(0);
        else if(nLeft == SPC_16)
            mpSpacingLB->SelectEntryPos(1);
        else if(nLeft == SPC_32)
            mpSpacingLB->SelectEntryPos(2);
        else if(nLeft == SPC_64)
            mpSpacingLB->SelectEntryPos(3);
        else if(nLeft == SPC_95)
            mpSpacingLB->SelectEntryPos(4);
        else if(nLeft == SPC_127)
            mpSpacingLB->SelectEntryPos(5);
        else if(nLeft == SPC_190)
            mpSpacingLB->SelectEntryPos(6);
    }
    else
    {
        mpSpacingLB->SetNoSelection();
    }
}

IMPL_LINK_NOARG(WrapPropertyPanel, EditContourHdl)
{
    SfxBoolItem aItem(SID_CONTOUR_DLG, true);
    mpBindings->GetDispatcher()->Execute(SID_CONTOUR_DLG, SfxCallMode::RECORD, &aItem, 0l);
    return 0;
}

IMPL_LINK_NOARG(WrapPropertyPanel, EnableContourHdl)
{
    bool IsContour = mpEnableContour->IsChecked();
    SfxBoolItem aItem(FN_FRAME_WRAP_CONTOUR, IsContour);
    mpBindings->GetDispatcher()->Execute(FN_FRAME_WRAP_CONTOUR, SfxCallMode::RECORD, &aItem, 0l);
    return 0;
}

IMPL_LINK_NOARG(WrapPropertyPanel, SpacingLBHdl)
{
    sal_uInt16 nPos = mpSpacingLB->GetSelectEntryPos();
    sal_uInt16 nVal = -1;
    switch(nPos)
    {
        case 0: nVal = SPC_0;break;
        case 1: nVal = SPC_16;break;
        case 2: nVal = SPC_32;break;
        case 3: nVal = SPC_64;break;
        case 4: nVal = SPC_95;break;
        case 5: nVal = SPC_127;break;
        case 6: nVal = SPC_190;break;
    }

    SvxLRSpaceItem aLRItem(nVal, nVal, 0, 0, RES_LR_SPACE);
    SvxULSpaceItem aULItem(nVal, nVal, RES_UL_SPACE);

    nTop = nBottom = nLeft = nRight = nVal;
    mpBindings->GetDispatcher()->Execute(SID_ATTR_LRSPACE, SfxCallMode::RECORD, &aLRItem, 0l);
    mpBindings->GetDispatcher()->Execute(SID_ATTR_ULSPACE, SfxCallMode::RECORD, &aULItem, 0l);

    return 0;
}

IMPL_LINK_NOARG(WrapPropertyPanel, WrapTypeHdl)
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
    SfxBoolItem bStateItem( nSlot, true );
    mpBindings->GetDispatcher()->Execute( nSlot, SfxCallMode::RECORD, &bStateItem, 0L );
    return 0;
}

void WrapPropertyPanel::UpdateEditContour()
{
    if(mpRBNoWrap->IsChecked() || mpRBWrapThrough->IsChecked())
    {
        mpEnableContour->Check( false );
        mpEnableContour->Disable();
    }
    else
    {
        mpEnableContour->Enable();
    }

}

void WrapPropertyPanel::NotifyItemUpdate(
    const sal_uInt16 nSId,
    const SfxItemState eState,
    const SfxPoolItem* pState,
    const bool bIsEnabled)
{
    (void)bIsEnabled;

    if ( eState == SfxItemState::DEFAULT &&
        pState->ISA(SfxBoolItem) )
    {
        //Set Radio Button enable
        mpRBNoWrap->Enable();
        mpRBWrapLeft->Enable();
        mpRBWrapRight->Enable();
        mpRBWrapParallel->Enable();
        mpRBWrapThrough->Enable();
        mpRBIdealWrap->Enable();
        mpEnableContour->Enable();

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
        case FN_FRAME_WRAP_CONTOUR:
            mpEnableContour->Check( pBoolItem->GetValue() );
            break;
        case FN_FRAME_NOWRAP:
            mpRBNoWrap->Check( pBoolItem->GetValue() );
            break;
        }
        UpdateEditContour();
    }
    switch(nSId)
    {
        case SID_ATTR_LRSPACE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SvxLRSpaceItem* pItem = dynamic_cast< const SvxLRSpaceItem* >(pState);
                if(pItem)
                {
                    nLeft = pItem->GetLeft();
                    nRight = pItem->GetRight();

                    UpdateSpacingLB();
                }
            }
        }
        break;
        case SID_ATTR_ULSPACE:
        {
            if(eState >= SfxItemState::DEFAULT)
            {
                const SvxULSpaceItem* pItem = dynamic_cast< const SvxULSpaceItem* >(pState);
                if(pItem)
                {
                    nTop = pItem->GetUpper();
                    nBottom = pItem->GetLower();

                    UpdateSpacingLB();
                }
            }
        }
        break;
    }
}

} } // end of namespace ::sw::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
