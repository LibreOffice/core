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

#include "Ruler.hxx"
#include <svl/ptitem.hxx>
#include <svx/ruler.hxx>
#include <svx/svxids.hrc>
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>

#include "View.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"

#include "helpids.h"

namespace sd {

/**
 * Controller-Item for ruler
 */
class RulerCtrlItem : public SfxControllerItem
{
    Ruler &rRuler;

 protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pItem ) override;

 public:
    RulerCtrlItem(sal_uInt16 nId, Ruler& rRlr, SfxBindings& rBind);
};

RulerCtrlItem::RulerCtrlItem(sal_uInt16 _nId, Ruler& rRlr, SfxBindings& rBind)
: SfxControllerItem(_nId, rBind)
, rRuler(rRlr)
{
}

void RulerCtrlItem::StateChanged( sal_uInt16 nSId, SfxItemState, const SfxPoolItem* pState )
{
    switch( nSId )
    {
        case SID_RULER_NULL_OFFSET:
        {
            const SfxPointItem* pItem = dynamic_cast< const SfxPointItem* >(pState);
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SfxPointItem expected");
            if ( pItem )
                rRuler.SetNullOffset(pItem->GetValue());
        }
        break;
    }
}

Ruler::Ruler( DrawViewShell& rViewSh, vcl::Window* pParent, ::sd::Window* pWin, SvxRulerSupportFlags nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
    : SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle)
    , pSdView(nullptr)
    , pSdWin(pWin)
    , pDrViewShell(&rViewSh)
{
    rBindings.EnterRegistrations();
    pCtrlItem = new RulerCtrlItem(SID_RULER_NULL_OFFSET, *this, rBindings);
    rBindings.LeaveRegistrations();

    if ( nWinStyle & WB_HSCROLL )
    {
        bHorz = true;
        SetHelpId( HID_SD_RULER_HORIZONTAL );
    }
    else
    {
        bHorz = false;
        SetHelpId( HID_SD_RULER_VERTICAL );
    }
}

Ruler::~Ruler()
{
    disposeOnce();
}

void Ruler::dispose()
{
    SfxBindings& rBindings = pCtrlItem->GetBindings();
    rBindings.EnterRegistrations();
    DELETEZ( pCtrlItem );
    rBindings.LeaveRegistrations();
    pSdWin.clear();
    SvxRuler::dispose();
}

void Ruler::MouseButtonDown(const MouseEvent& rMEvt)
{
    Point aMPos = rMEvt.GetPosPixel();
    RulerType eType = GetType(aMPos);

    if ( !pDrViewShell->GetView()->IsTextEdit() &&
        rMEvt.IsLeft() && rMEvt.GetClicks() == 1 &&
        (eType == RULER_TYPE_DONTKNOW || eType == RULER_TYPE_OUTSIDE) )
    {
        pDrViewShell->StartRulerDrag(*this, rMEvt);
    }
    else
        SvxRuler::MouseButtonDown(rMEvt);
}

void Ruler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
}

void Ruler::MouseButtonUp(const MouseEvent& rMEvt)
{
    SvxRuler::MouseButtonUp(rMEvt);
}

void Ruler::SetNullOffset(const Point& rOffset)
{
    long nOffset;

    if ( bHorz )    nOffset = rOffset.X();
    else            nOffset = rOffset.Y();

    SetNullOffsetLogic(nOffset);
}

void Ruler::Command(const CommandEvent& rCEvt)
{
    if( rCEvt.GetCommand() == CommandEventId::ContextMenu &&
        !pDrViewShell->GetView()->IsTextEdit() )
    {
        SvxRuler::Command( rCEvt );
    }
}

void Ruler::ExtraDown()
{
    if( !pDrViewShell->GetView()->IsTextEdit() )
        SvxRuler::ExtraDown();
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
