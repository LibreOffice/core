/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            DBG_ASSERT(pState ? pItem != NULL : true, "SfxPointItem erwartet");
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
        bHorz = true;
        SetHelpId( HID_SD_RULER_HORIZONTAL );
    }
    else
    {
        bHorz = false;
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

