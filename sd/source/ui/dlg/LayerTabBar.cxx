/*************************************************************************
 *
 *  $RCSfile: LayerTabBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:49:25 $
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

#include "LayerTabBar.hxx"

#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif
#ifndef _SVDPAGV_HXX //autogen
#include <svx/svdpagv.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#pragma hdrstop

#include "sdattr.hxx"
#include "app.hxx"
#include "helpids.h"
#include "app.hrc"
#include "glob.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#include "drawdoc.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "undolayer.hxx"


namespace sd {

#define SWITCH_TIMEOUT  20


/*************************************************************************
|*
|* Standard-Konstruktor
|*
\************************************************************************/

LayerTabBar::LayerTabBar(DrawViewShell* pViewSh, Window* pParent)
    : TabBar( pParent, WinBits( WB_BORDER | WB_3DLOOK | WB_SCROLL | WB_SIZEABLE ) ),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh)
{
    EnableEditMode();
    SetSizePixel(Size(0, 0));
    SetMaxPageWidth( 150 );
    SetHelpId( HID_SD_TABBAR_LAYERS );
}


LayerTabBar::LayerTabBar (
    DrawViewShell* pViewSh,
    Window* pParent,
    const ResId& rResId)
    : TabBar (pParent, rResId.aWinBits),
    DropTargetHelper( this ),
    pDrViewSh(pViewSh)
{
    EnableEditMode();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

LayerTabBar::~LayerTabBar()
{
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::Select()
{
    SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
    pDispatcher->Execute(SID_SWITCHLAYER, SFX_CALLMODE_ASYNCHRON);
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::MouseButtonDown(const MouseEvent& rMEvt)
{
    BOOL bSetPageID=FALSE;

    if (rMEvt.IsLeft() && !rMEvt.IsMod1() && !rMEvt.IsMod2())
    {
        Point aPosPixel = rMEvt.GetPosPixel();
        USHORT aLayerId = GetPageId( PixelToLogic(aPosPixel) );

        if (aLayerId == 0)
        {
            SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
            pDispatcher->Execute(SID_INSERTLAYER, SFX_CALLMODE_SYNCHRON);

            bSetPageID=TRUE;
        }
        else if (rMEvt.IsShift())
        {
            // Toggle zw. Layer sichtbar / unsichtbar
            String aName(GetPageText(aLayerId));
            SdrPageView* pPV = pDrViewSh->GetView()->GetPageViewPvNum(0);
            BOOL bVisible = pPV->IsLayerVisible(aName);
            pPV->SetLayerVisible(aName, !bVisible);
            pDrViewSh->ResetActualLayer();
        }
    }

    // If you insert a new layer you must not call TabBar::MouseButtonDown(rMEvt);
    // because you want to activate the new layer
    if( !bSetPageID )
        TabBar::MouseButtonDown(rMEvt);
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::DoubleClick()
{
    if (GetCurPageId() != 0)
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute( SID_MODIFYLAYER, SFX_CALLMODE_SYNCHRON );
    }
}


/*************************************************************************
|*
|* AcceptDrop-Event
|*
\************************************************************************/

sal_Int8 LayerTabBar::AcceptDrop( const AcceptDropEvent& rEvt )
{
    sal_Int8 nRet = DND_ACTION_NONE;

    if( rEvt.mbLeaving )
        EndSwitchPage();

    if( !pDrViewSh->GetDocSh()->IsReadOnly() )
    {
        USHORT          nPageId = SDRPAGE_NOTFOUND;
        Point           aPos( PixelToLogic( rEvt.maPosPixel ) );
        USHORT          nLayerId = pDrViewSh->GetView()->GetDoc()->GetLayerAdmin().GetLayerID( GetPageText( GetPageId( aPos ) ), FALSE );

        nRet = pDrViewSh->AcceptDrop( rEvt, *this, NULL, nPageId, nLayerId );

        SwitchPage( aPos );
    }

    return nRet;
}

/*************************************************************************
|*
|* ExecuteDrop-Event
|*
\************************************************************************/

sal_Int8 LayerTabBar::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    USHORT          nPageId = SDRPAGE_NOTFOUND;
    USHORT          nLayerId = pDrViewSh->GetView()->GetDoc()->GetLayerAdmin().GetLayerID( GetPageText( GetPageId( PixelToLogic( rEvt.maPosPixel ) ) ), FALSE );
    sal_Int8        nRet = pDrViewSh->ExecuteDrop( rEvt, *this, NULL, nPageId, nLayerId );

    EndSwitchPage();

    return nRet;

}

/*************************************************************************
|*
\************************************************************************/

void  LayerTabBar::Command(const CommandEvent& rCEvt)
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU )
    {
        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->ExecutePopup(SdResId(RID_LAYERTAB_POPUP));
    }
}


/*************************************************************************
|*
\************************************************************************/
long LayerTabBar::StartRenaming()
{
    BOOL bOK = TRUE;
    String aLayerName = GetPageText( GetEditPageId() );
    String aLayoutLayer ( SdResId(STR_LAYER_LAYOUT) );
    String aControlsLayer ( SdResId(STR_LAYER_CONTROLS) );
    String aMeasureLinesLayer ( SdResId(STR_LAYER_MEASURELINES) );
    String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
    String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );

    if ( aLayerName == aLayoutLayer       || aLayerName == aControlsLayer  ||
         aLayerName == aMeasureLinesLayer ||
         aLayerName == aBackgroundLayer   || aLayerName == aBackgroundObjLayer )
    {
        // Diese Namen duerfen nicht veraendert werden
        bOK = FALSE;
    }
    else
    {
        ::sd::View* pView = pDrViewSh->GetView();

        if ( pView->IsTextEdit() )
        {
            pView->EndTextEdit();
        }
    }

    return(bOK);
}

/*************************************************************************
|*
\************************************************************************/

long LayerTabBar::AllowRenaming()
{
    BOOL bOK = TRUE;

    // Ueberpruefung auf schon vorhandene Namen
    ::sd::View* pView = pDrViewSh->GetView();
    SdDrawDocument* pDoc = pView->GetDoc();
    String aLayerName = pView->GetActiveLayer();
    SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
    String aNewName( GetEditText() );

    if ( aNewName.Len() == 0 ||
        (rLayerAdmin.GetLayer( aNewName, FALSE ) && aLayerName != aNewName) )
    {
        // Name ist schon vorhanden
        WarningBox aWarningBox( &pDrViewSh->GetViewFrame()->GetWindow(), WinBits( WB_OK ),
                                String(SdResId( STR_WARN_NAME_DUPLICATE ) ) );
        aWarningBox.Execute();
        bOK = FALSE;
    }

    if (bOK)
    {
        String aLayoutLayer ( SdResId(STR_LAYER_LAYOUT) );
        String aControlsLayer ( SdResId(STR_LAYER_CONTROLS) );
        String aMeasureLinesLayer ( SdResId(STR_LAYER_MEASURELINES) );
        String aBackgroundLayer( SdResId(STR_LAYER_BCKGRND) );
        String aBackgroundObjLayer( SdResId(STR_LAYER_BCKGRNDOBJ) );

        if ( aNewName == aLayoutLayer       || aNewName == aControlsLayer  ||
             aNewName == aMeasureLinesLayer ||
             aNewName == aBackgroundLayer   || aNewName == aBackgroundObjLayer )
        {
            // Diese Namen duerfen nicht vergeben werden
            bOK = FALSE;
        }
    }

    return(bOK);
}

/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::EndRenaming()
{
    if( !IsEditModeCanceled() )
    {
        ::sd::View* pView = pDrViewSh->GetView();
        DrawView* pDrView = PTR_CAST( DrawView, pView );

        SdDrawDocument* pDoc = pView->GetDoc();
        String aLayerName = pView->GetActiveLayer();
        SdrLayerAdmin& rLayerAdmin = pDoc->GetLayerAdmin();
        SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName, FALSE);

        if (pLayer)
        {
            String aNewName( GetEditText() );

            DBG_ASSERT( pDrView, "Rename layer undo action is only working with a SdDrawView" );
            if( pDrView )
            {
                SfxUndoManager* pManager = pDoc->GetDocSh()->GetUndoManager();
                SdLayerModifyUndoAction* pAction = new SdLayerModifyUndoAction(
                    pDoc,
                    pLayer,
                    aLayerName,
                    pDrView->IsLayerVisible(aLayerName),
                    pDrView->IsLayerLocked(aLayerName),
                    pDrView->IsLayerPrintable(aLayerName),
                    aNewName,
                    pDrView->IsLayerVisible(aLayerName),
                    pDrView->IsLayerLocked(aLayerName),
                    pDrView->IsLayerPrintable(aLayerName)
                    );
                pManager->AddUndoAction( pAction );
            }

            // Zuerst View benachrichtigen, da innerhalb von SetName() schon
            // ResetActualLayer() gerufen wird und an der View der Layer dann
            // schon bekannt sein muss.
            pView->SetActiveLayer(aNewName);
            pLayer->SetName(aNewName);
            pDoc->SetChanged(TRUE);
        }
    }
}


/*************************************************************************
|*
\************************************************************************/

void LayerTabBar::ActivatePage()
{
    if ( /*IsInSwitching*/ 1 && pDrViewSh!=NULL)
    {

        SfxDispatcher* pDispatcher = pDrViewSh->GetViewFrame()->GetDispatcher();
        pDispatcher->Execute(SID_SWITCHLAYER, SFX_CALLMODE_ASYNCHRON);
    }
}




void LayerTabBar::SendActivatePageEvent (void)
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}




void LayerTabBar::SendDeactivatePageEvent (void)
{
    CallEventListeners (VCLEVENT_TABBAR_PAGEDEACTIVATED,
        reinterpret_cast<void*>(GetCurPageId()));
}

} // end of namespace sd
