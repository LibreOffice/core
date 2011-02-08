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

#include "Ruler.hxx"
#include <svl/ptitem.hxx>
#include <svx/ruler.hxx>
#ifndef _SVXIDS_HXX //autogen
#include <svx/svxids.hrc>
#endif
#include <sfx2/ctrlitem.hxx>
#include <sfx2/bindings.hxx>


#include "View.hxx"
#include "DrawViewShell.hxx"
#include "Window.hxx"

#include "helpids.h"

namespace sd {

/*************************************************************************
|*
|* Controller-Item fuer Ruler
|*
\************************************************************************/

class RulerCtrlItem : public SfxControllerItem
{
    Ruler &rRuler;

 protected:
    virtual void StateChanged( sal_uInt16 nSId, SfxItemState eState,
                                const SfxPoolItem* pItem );

 public:
    RulerCtrlItem(sal_uInt16 nId, Ruler& rRlr, SfxBindings& rBind);
};

/*************************************************************************
|*
\************************************************************************/

RulerCtrlItem::RulerCtrlItem(sal_uInt16 _nId, Ruler& rRlr, SfxBindings& rBind)
: SfxControllerItem(_nId, rBind)
, rRuler(rRlr)
{
}


/*************************************************************************
|*
\************************************************************************/

void RulerCtrlItem::StateChanged( sal_uInt16 nSId, SfxItemState, const SfxPoolItem* pState )
{
    switch( nSId )
    {
        case SID_RULER_NULL_OFFSET:
        {
            const SfxPointItem* pItem = dynamic_cast< const SfxPointItem* >(pState);
            DBG_ASSERT(pState ? pItem != NULL : sal_True, "SfxPointItem erwartet");
            if ( pItem )
                rRuler.SetNullOffset(pItem->GetValue());
        }
        break;
    }
}


/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

Ruler::Ruler( DrawViewShell& rViewSh, ::Window* pParent, ::sd::Window* pWin, sal_uInt16 nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
: SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle)
, pSdWin(pWin)
, pDrViewShell(&rViewSh)
{
    rBindings.EnterRegistrations();
    pCtrlItem = new RulerCtrlItem(SID_RULER_NULL_OFFSET, *this, rBindings);
    rBindings.LeaveRegistrations();

    if ( nWinStyle & WB_HSCROLL )
    {
        bHorz = sal_True;
        SetHelpId( HID_SD_RULER_HORIZONTAL );
    }
    else
    {
        bHorz = sal_False;
        SetHelpId( HID_SD_RULER_VERTICAL );
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

Ruler::~Ruler()
{
    SfxBindings& rBindings = pCtrlItem->GetBindings();
    rBindings.EnterRegistrations();
    delete pCtrlItem;
    rBindings.LeaveRegistrations();
}

/*************************************************************************
|*
|* MouseButtonDown-Handler
|*
\************************************************************************/

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

/*************************************************************************
|*
|* MouseMove-Handler
|*
\************************************************************************/

void Ruler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-Handler
|*
\************************************************************************/

void Ruler::MouseButtonUp(const MouseEvent& rMEvt)
{
    SvxRuler::MouseButtonUp(rMEvt);
}

/*************************************************************************
|*
|* NullOffset setzen
|*
\************************************************************************/

void Ruler::SetNullOffset(const Point& rOffset)
{
    long nOffset;

    if ( bHorz )    nOffset = rOffset.X();
    else            nOffset = rOffset.Y();

    SetNullOffsetLogic(nOffset);
}

/*************************************************************************
|*
|* Command event
|*
\************************************************************************/

void Ruler::Command(const CommandEvent& rCEvt)
{
    if( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
        !pDrViewShell->GetView()->IsTextEdit() )
    {
        SvxRuler::Command( rCEvt );
    }
}

/*************************************************************************
|*
|* ExtraDown
|*
\************************************************************************/

void Ruler::ExtraDown()
{
    if( !pDrViewShell->GetView()->IsTextEdit() )
        SvxRuler::ExtraDown();
}

} // end of namespace sd

