/*************************************************************************
 *
 *  $RCSfile: fuconuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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

#include <basctl/idetemp.hxx>
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX //autogen
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif


#pragma hdrstop

#ifndef _SVX_FMGLOB_HXX
#include <svx/fmglob.hxx>
#endif

#include <svx/dialogs.hrc>

class SbModule;


#include "app.hrc"
#include "glob.hrc"

#include "viewshel.hxx"
#include "sdview.hxx"
#include "sdwindow.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "fuconuno.hxx"
#include "res_bmp.hrc"

TYPEINIT1( FuConstUnoControl, FuConstruct );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuConstUnoControl::FuConstUnoControl(SdViewShell*   pViewSh,
                               SdWindow*        pWin,
                               SdView*          pView,
                               SdDrawDocument*  pDoc,
                               SfxRequest&      rReq)
    : FuConstruct(pViewSh, pWin, pView, pDoc, rReq)
{
    SFX_REQUEST_ARG( rReq, pInventorItem, SfxUInt32Item, SID_FM_CONTROL_INVENTOR, FALSE );
    SFX_REQUEST_ARG( rReq, pIdentifierItem, SfxUInt16Item, SID_FM_CONTROL_IDENTIFIER, FALSE );
    if( pInventorItem )
        nInventor = pInventorItem->GetValue();
    if( pIdentifierItem )
        nIdentifier = pIdentifierItem->GetValue();

    pViewShell->SwitchObjectBar(RID_DRAW_OBJ_TOOLBOX);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/
FuConstUnoControl::~FuConstUnoControl()
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/
BOOL FuConstUnoControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bReturn = FuConstruct::MouseButtonDown(rMEvt);

    if ( rMEvt.IsLeft() && !pView->IsAction() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pWindow->CaptureMouse();
        USHORT nDrgLog = USHORT ( pWindow->PixelToLogic(Size(DRGPIX,0)).Width() );
        pView->BegCreateObj(aPnt, (OutputDevice*) NULL, nDrgLog);
        bReturn = TRUE;
    }
    return bReturn;
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/
BOOL FuConstUnoControl::MouseMove(const MouseEvent& rMEvt)
{
    return FuConstruct::MouseMove(rMEvt);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/
BOOL FuConstUnoControl::MouseButtonUp(const MouseEvent& rMEvt)
{
    BOOL bReturn = FALSE;

    if ( pView->IsCreateObj() && rMEvt.IsLeft() )
    {
        Point aPnt( pWindow->PixelToLogic( rMEvt.GetPosPixel() ) );
        pView->EndCreateObj(SDRCREATE_FORCEEND);
        bReturn = TRUE;
    }

    bReturn = (FuConstruct::MouseButtonUp(rMEvt) || bReturn);

    if (!bPermanent)
        pViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SFX_CALLMODE_ASYNCHRON);

    return (bReturn);
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert TRUE, andernfalls
|* FALSE.
|*
\************************************************************************/
BOOL FuConstUnoControl::KeyInput(const KeyEvent& rKEvt)
{
    BOOL bReturn = FuConstruct::KeyInput(rKEvt);
    return(bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/
void FuConstUnoControl::Activate()
{
    pView->SetCurrentObj( nIdentifier, nInventor );

    aNewPointer = Pointer(POINTER_DRAW_RECT);
    aOldPointer = pWindow->GetPointer();
    pWindow->SetPointer( aNewPointer );

    aOldLayer = pView->GetActiveLayer();
    String aStr(SdResId(STR_LAYER_CONTROLS));
    pView->SetActiveLayer( aStr );

    FuConstruct::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/
void FuConstUnoControl::Deactivate()
{
    FuConstruct::Deactivate();
    pView->SetActiveLayer( aOldLayer );
    pWindow->SetPointer( aOldPointer );
}


