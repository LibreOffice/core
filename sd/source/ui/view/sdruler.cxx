/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdruler.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 17:04:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "Ruler.hxx"

#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SVX_RULER_HXX //autogen
#include <svx/ruler.hxx>
#endif
#ifndef _SVXIDS_HXX //autogen
#include <svx/svxids.hrc>
#endif
#ifndef _SFXCTRLITEM_HXX //autogen
#include <sfx2/ctrlitem.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif


#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif

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
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pItem );

 public:
    RulerCtrlItem(USHORT nId, Ruler& rRlr, SfxBindings& rBind);
};

/*************************************************************************
|*
\************************************************************************/

RulerCtrlItem::RulerCtrlItem(USHORT _nId, Ruler& rRlr, SfxBindings& rBind)
: SfxControllerItem(_nId, rBind)
, rRuler(rRlr)
{
}


/*************************************************************************
|*
\************************************************************************/

void RulerCtrlItem::StateChanged( USHORT nSId, SfxItemState, const SfxPoolItem* pState )
{
    switch( nSId )
    {
        case SID_RULER_NULL_OFFSET:
        {
            const SfxPointItem* pItem = dynamic_cast< const SfxPointItem* >(pState);
            DBG_ASSERT(pState ? pItem != NULL : TRUE, "SfxPointItem erwartet");
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

Ruler::Ruler( DrawViewShell& rViewSh, ::Window* pParent, ::sd::Window* pWin, USHORT nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
: SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle)
, pSdWin(pWin)
, pDrViewShell(&rViewSh)
{
    rBindings.EnterRegistrations();
    pCtrlItem = new RulerCtrlItem(SID_RULER_NULL_OFFSET, *this, rBindings);
    rBindings.LeaveRegistrations();

    if ( nWinStyle & WB_HSCROLL )
    {
        bHorz = TRUE;
        SetHelpId( HID_SD_RULER_HORIZONTAL );
    }
    else
    {
        bHorz = FALSE;
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

