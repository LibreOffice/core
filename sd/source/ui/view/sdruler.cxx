/*************************************************************************
 *
 *  $RCSfile: sdruler.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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

#pragma hdrstop

#include "sdview.hxx"
#include "drviewsh.hxx"
#include "sdwindow.hxx"
#include "sdruler.hxx"


/*************************************************************************
|*
|* Controller-Item fuer SdRuler
|*
\************************************************************************/

class SdRulerCtrlItem : public SfxControllerItem
{
    SdRuler &rRuler;

 protected:
    virtual void StateChanged( USHORT nSId, SfxItemState eState,
                                const SfxPoolItem* pItem );

 public:
    SdRulerCtrlItem(USHORT nId, SdRuler& rRlr, SfxBindings& rBind);
};

/*************************************************************************
|*
\************************************************************************/

SdRulerCtrlItem::SdRulerCtrlItem(USHORT nId, SdRuler& rRlr,
                                 SfxBindings& rBind) :
    SfxControllerItem(nId, rBind),
    rRuler(rRlr)
{
}


/*************************************************************************
|*
\************************************************************************/

void __EXPORT SdRulerCtrlItem::StateChanged( USHORT nSId,
                        SfxItemState eState, const SfxPoolItem* pState )
{
    switch( nSId )
    {
        case SID_RULER_NULL_OFFSET:
        {
            const SfxPointItem* pItem = PTR_CAST(SfxPointItem, pState);
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

SdRuler::SdRuler(SdDrawViewShell& rViewSh, Window* pParent, SdWindow* pWin,
                 USHORT nRulerFlags, SfxBindings& rBindings,
                 WinBits nWinStyle) :
    SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle),
    pDrViewShell(&rViewSh),
    pSdWin(pWin)
{
    rBindings.EnterRegistrations();
    pCtrlItem = new SdRulerCtrlItem(SID_RULER_NULL_OFFSET, *this, rBindings);
    rBindings.LeaveRegistrations();

    if ( nWinStyle & WB_HSCROLL )   bHorz = TRUE;
    else                            bHorz = FALSE;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdRuler::~SdRuler()
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

void __EXPORT SdRuler::MouseButtonDown(const MouseEvent& rMEvt)
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

void __EXPORT SdRuler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-Handler
|*
\************************************************************************/

void __EXPORT SdRuler::MouseButtonUp(const MouseEvent& rMEvt)
{
    SvxRuler::MouseButtonUp(rMEvt);
}

/*************************************************************************
|*
|* NullOffset setzen
|*
\************************************************************************/

void SdRuler::SetNullOffset(const Point& rOffset)
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

void SdRuler::Command(const CommandEvent& rCEvt)
{
    if( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
        !pDrViewShell->GetView()->IsTextEdit() )
    {
#ifdef MAC
        pDrViewShell->GetView()->BrkAction();   // Aktion wieder abbrechen
#endif

        SvxRuler::Command( rCEvt );
    }
}

/*************************************************************************
|*
|* ExtraDown
|*
\************************************************************************/

void SdRuler::ExtraDown()
{
    if( !pDrViewShell->GetView()->IsTextEdit() )
        SvxRuler::ExtraDown();
}



