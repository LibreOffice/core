/*************************************************************************
 *
 *  $RCSfile: view.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:30:04 $
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

#pragma hdrstop

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLE_HPP_
#include <com/sun/star/accessibility/XAccessible.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTOBJECT_HPP_
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _SV_MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX //autogen
#include <vcl/decoview.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _UNDO_HXX //autogen
#include <svtools/undo.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXPTITEM_HXX //autogen
#include <svtools/ptitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif

#ifndef _SFXMSG_HXX //autogen
#include <sfx2/msg.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif
#ifndef _SVX_ZOOM_HXX //autogen
#include <svx/zoom.hxx>
#endif
#ifndef _MyEDITENG_HXX
#include <svx/editeng.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif


#ifndef VIEW_HXX
#include "view.hxx"
#endif
#ifndef DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef CONFIG_HXX
#include "config.hxx"
#endif
#ifndef TOOLBOX_HXX
#include "toolbox.hxx"
#endif
#ifndef DIALOG_HXX
#include "dialog.hxx"
#endif
#ifndef _STARMATH_HRC
#include "starmath.hrc"
#endif


#define MINWIDTH        200
#define MINHEIGHT       200
#define MINSPLIT        40
#define SPLITTERWIDTH   2

#define MINZOOM 25
#define MAXZOOM 800

#define SmViewShell
#include "smslots.hxx"

using namespace com::sun::star::accessibility;
using namespace com::sun::star;
using namespace com::sun::star::uno;

//////////////////////////////////////////////////////////////////////

SmGraphicWindow::SmGraphicWindow(SmViewShell* pShell):
    ScrollableWindow(&pShell->GetViewFrame()->GetWindow(), 0),
    pAccessible(0),
    pViewShell(pShell),
    nZoom(100),
    bIsCursorVisible(FALSE)
{
    Hide();
    const Fraction aFraction (1,1);
    SetMapMode( MapMode(MAP_100TH_MM, Point(), aFraction, aFraction));

    ApplyColorConfigValues( SM_MOD1()->GetColorConfig() );

    SetTotalSize();

    SetHelpId(HID_SMA_WIN_DOCUMENT);
    SetUniqueId(HID_SMA_WIN_DOCUMENT);
}

SmGraphicWindow::~SmGraphicWindow()
{
    if (pAccessible)
        pAccessible->ClearWin();    // make Accessible defunctional
    // Note: memory for pAccessible will be freed when the reference
    // xAccessible is released.
}

void SmGraphicWindow::StateChanged( StateChangedType eType )
{
    if ( eType == STATE_CHANGE_INITSHOW )
        Show();
    ScrollableWindow::StateChanged( eType );
}


void SmGraphicWindow::ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg )
{
    // Note: SetTextColor not necessary since the nodes that
    // get painted have the color information.
#if OSL_DEBUG_LEVEL > 1
    ColorData nVal = rColorCfg.GetColorValue(svtools::DOCCOLOR).nColor;
#endif
    SetBackground( Color( (ColorData) rColorCfg.GetColorValue(svtools::DOCCOLOR).nColor ) );
    Invalidate();
}


void SmGraphicWindow::DataChanged( const DataChangedEvent& rEvt )
{
    ApplyColorConfigValues( SM_MOD1()->GetColorConfig() );

    ScrollableWindow::DataChanged( rEvt );
}


void SmGraphicWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
//  GrabFocus();

    ScrollableWindow::MouseButtonDown(rMEvt);

    //
    // set formula-cursor and selection of edit window according to the
    // position clicked at
    //
    DBG_ASSERT(rMEvt.GetClicks() > 0, "Sm : 0 clicks");
    if ( rMEvt.IsLeft() && pViewShell->GetEditWindow() )
    {
        const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree();
        //! kann NULL sein! ZB wenn bereits beim laden des Dokuments (bevor der
        //! Parser angeworfen wurde) ins Fenster geklickt wird.
        if (!pTree)
            return;

        // get click position relativ to formula
        Point  aPos (PixelToLogic(rMEvt.GetPosPixel())
                     - GetFormulaDrawPos());

        // if it was clicked inside the formula then get the appropriate node
        const SmNode *pNode = 0;
        if (pTree->OrientedDist(aPos) <= 0)
            pNode = pTree->FindRectClosestTo(aPos);

        if (pNode)
        {   SmEditWindow  *pEdit = pViewShell->GetEditWindow();
            const SmToken  aToken (pNode->GetToken());

#ifdef notnow
            // include introducing symbols of special char and text
            // (ie '%' and '"')
            USHORT  nExtra = (aToken.eType == TSPECIAL  ||  aToken.eType == TTEXT) ? 1 : 0;

            // set selection to the beginning of the token
            ESelection  aSel (aToken.nRow - 1, aToken.nCol - 1 - nExtra);

            if (rMEvt.GetClicks() != 1)
            {   // select whole token
                // for text include terminating symbol (ie '"')
                aSel.nEndPos += aToken.aText.Len() + nExtra
                                + (aToken.eType == TTEXT ? 1 : 0);
            }
#endif
            // set selection to the beginning of the token
            ESelection  aSel (aToken.nRow - 1, aToken.nCol - 1);

            if (rMEvt.GetClicks() != 1)
                aSel.nEndPos += aToken.aText.Len();

            pEdit->SetSelection(aSel);
            SetCursor(pNode);

            // allow for immediate editing and
            //! implicitly synchronize the cursor position mark in this window
            pEdit->GrabFocus();
        }
    }
}

void SmGraphicWindow::GetFocus()
{
    ScrollableWindow::GetFocus();
    if (xAccessible.is())
    {
        uno::Any aOldValue, aNewValue;
        // aOldValue remains empty
        aNewValue <<= AccessibleStateType::FOCUSED;
        pAccessible->LaunchEvent( AccessibleEventId::STATE_CHANGED,
                aOldValue, aNewValue );
    }
}

void SmGraphicWindow::LoseFocus()
{
    ScrollableWindow::LoseFocus();
    if (xAccessible.is())
    {
        uno::Any aOldValue, aNewValue;
        aOldValue <<= AccessibleStateType::FOCUSED;
        // aNewValue remains empty
        pAccessible->LaunchEvent( AccessibleEventId::STATE_CHANGED,
                aOldValue, aNewValue );
    }
}

void SmGraphicWindow::ShowCursor(BOOL bShow)
    // shows or hides the formula-cursor depending on 'bShow' is TRUE or not
{
    BOOL  bInvert = bShow != IsCursorVisible();

    if (bInvert)
        InvertTracking(aCursorRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW);

    SetIsCursorVisible(bShow);
}


void SmGraphicWindow::SetCursor(const SmNode *pNode)
{
    const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree();

    // get appropriate rectangle
    Point aOffset (pNode->GetTopLeft() - pTree->GetTopLeft()),
          aTLPos  (GetFormulaDrawPos() + aOffset);
    aTLPos.X() -= pNode->GetItalicLeftSpace();
    Size  aSize   (pNode->GetItalicSize());
    Point aBRPos  (aTLPos.X() + aSize.Width(), aTLPos.Y() + aSize.Height());

    SetCursor(Rectangle(aTLPos, aSize));
}

void SmGraphicWindow::SetCursor(const Rectangle &rRect)
    // sets cursor to new position (rectangle) 'rRect'.
    // The old cursor will be removed, and the new one will be shown if
    // that is activated in the ConfigItem
{
    SmModule *pp = SM_MOD1();

    if (IsCursorVisible())
        ShowCursor(FALSE);      // clean up remainings of old cursor
    aCursorRect = rRect;
    if (pp->GetConfig()->IsShowFormulaCursor())
        ShowCursor(TRUE);       // draw new cursor
}

const SmNode * SmGraphicWindow::SetCursorPos(USHORT nRow, USHORT nCol)
    // looks for a VISIBLE node in the formula tree with it's token at
    // (or around) the position 'nRow', 'nCol' in the edit window
    // (row and column numbering starts with 1 there!).
    // If there is such a node the formula-cursor is set to cover that nodes
    // rectangle. If not the formula-cursor will be hidden.
    // In any case the search result is being returned.
{
    // find visible node with token at nRow, nCol
    const SmNode *pTree = pViewShell->GetDoc()->GetFormulaTree(),
                 *pNode = 0;
    if (pTree)
        pNode = pTree->FindTokenAt(nRow, nCol);

    if (pNode)
        SetCursor(pNode);
    else
        ShowCursor(FALSE);

    return pNode;
}


void SmGraphicWindow::Paint(const Rectangle&)
{
    DBG_ASSERT(pViewShell, "Sm : NULL pointer");

    SmDocShell &rDoc = *pViewShell->GetDoc();
    Point aPoint;

    rDoc.Draw(*this, aPoint);   //! modifies aPoint to be the topleft
                                //! corner of the formula
    SetFormulaDrawPos(aPoint);

    SetIsCursorVisible(FALSE);  // (old) cursor must be drawn again

    const SmEditWindow *pEdit = pViewShell->GetEditWindow();
    if (pEdit)
    {   // get new position for formula-cursor (for possible altered formula)
        USHORT  nRow, nCol;
        SmGetLeftSelectionPart(pEdit->GetSelection(), nRow, nCol);
        nRow++;
        nCol++;
        const SmNode *pFound = SetCursorPos(nRow, nCol);

        SmModule  *pp = SM_MOD1();
        if (pFound && pp->GetConfig()->IsShowFormulaCursor())
            ShowCursor(TRUE);
    }
}


void SmGraphicWindow::SetTotalSize ()
{
    SmDocShell &rDoc = *pViewShell->GetDoc();
    const Size aTmp( PixelToLogic( LogicToPixel( rDoc.GetSize() )));
    if ( aTmp != ScrollableWindow::GetTotalSize() )
        ScrollableWindow::SetTotalSize( aTmp );
}


void SmGraphicWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (! (SfxViewShell::Current() &&
           SfxViewShell::Current()->KeyInput(rKEvt)))
        ScrollableWindow::KeyInput(rKEvt);
}


void SmGraphicWindow::Command(const CommandEvent& rCEvt)
{
    BOOL bCallBase = TRUE;
    if ( !pViewShell->GetDoc()->GetProtocol().IsInPlaceActive() )
    {
        switch ( rCEvt.GetCommand() )
        {
            case COMMAND_CONTEXTMENU:
            {
                GetParent()->ToTop();
                SmResId aResId( RID_VIEWMENU );
                PopupMenu* pPopupMenu = new PopupMenu(aResId);
                pPopupMenu->SetSelectHdl(LINK(this, SmGraphicWindow, MenuSelectHdl));
                Point aPos(5, 5);
                if (rCEvt.IsMouseEvent())
                    aPos = rCEvt.GetMousePosPixel();
                DBG_ASSERT( pViewShell, "view shell missing" );

                // added for replaceability of context menus #96085, #93782
                pViewShell->GetViewFrame()->GetBindings().GetDispatcher()
                        ->ExecutePopup( aResId, this, &aPos );
                //pPopupMenu->Execute( this, aPos );

                delete pPopupMenu;
                bCallBase = FALSE;
            }
            break;

            case COMMAND_WHEEL:
            {
                const CommandWheelData* pWData = rCEvt.GetWheelData();
                if  ( pWData && COMMAND_WHEEL_ZOOM == pWData->GetMode() )
                {
                    USHORT nZoom = GetZoom();
                    if( 0L > pWData->GetDelta() )
                        nZoom -= 10;
                    else
                        nZoom += 10;
                    SetZoom( nZoom );
                    bCallBase = FALSE;
                }
            }
            break;
        }
    }
    if ( bCallBase )
        ScrollableWindow::Command (rCEvt);
}


IMPL_LINK_INLINE_START( SmGraphicWindow, MenuSelectHdl, Menu *, pMenu )
{
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetDispatcher()->Execute( pMenu->GetCurItemId() );
    return 0;
}
IMPL_LINK_INLINE_END( SmGraphicWindow, MenuSelectHdl, Menu *, pMenu )


void SmGraphicWindow::SetZoom(USHORT Factor)
{
    nZoom = Min(Max((USHORT) Factor, (USHORT) MINZOOM), (USHORT) MAXZOOM);
    Fraction   aFraction (nZoom, 100);
    SetMapMode( MapMode(MAP_100TH_MM, Point(), aFraction, aFraction) );
    SetTotalSize();
    SmViewShell *pViewSh = SmGetActiveView();
    if (pViewSh)
        pViewSh->GetViewFrame()->GetBindings().Invalidate(SID_ATTR_ZOOM);
    Invalidate();
}


void SmGraphicWindow::ZoomToFitInWindow()
{
    SmDocShell &rDoc = *pViewShell->GetDoc();

    // set defined mapmode before calling 'LogicToPixel' below
    SetMapMode(MapMode(MAP_100TH_MM));

    Size       aSize (LogicToPixel(rDoc.GetSize()));
    Size       aWindowSize (GetSizePixel());

    if (aSize.Width() > 0  &&  aSize.Height() > 0)
        SetZoom (Min ((85 * aWindowSize.Width())  / aSize.Width(),
                      (85 * aWindowSize.Height()) / aSize.Height()));
}

uno::Reference< XAccessible > SmGraphicWindow::CreateAccessible()
{
    if (!pAccessible)
    {
        pAccessible = new SmGraphicAccessible( this );
        xAccessible = pAccessible;
    }
    return xAccessible;
}

/**************************************************************************/


SmGraphicController::SmGraphicController(SmGraphicWindow &rSmGraphic,
                        USHORT          nId,
                        SfxBindings     &rBindings) :
    rGraphic(rSmGraphic),
    SfxControllerItem(nId, rBindings)
{
}


void SmGraphicController::StateChanged(USHORT nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    rGraphic.SetTotalSize();
    rGraphic.Invalidate();
    SfxControllerItem::StateChanged (nSID, eState, pState);
}


/**************************************************************************/


SmEditController::SmEditController(SmEditWindow &rSmEdit,
                     USHORT       nId,
                     SfxBindings  &rBindings) :
    rEdit(rSmEdit),
    SfxControllerItem(nId, rBindings)
{
}


#if OSL_DEBUG_LEVEL > 1
SmEditController::~SmEditController()
{
}
#endif


void SmEditController::StateChanged(USHORT nSID, SfxItemState eState, const SfxPoolItem* pState)
{
    const SfxStringItem *pItem = PTR_CAST(SfxStringItem, pState);

    if ((pItem != NULL) && (rEdit.GetText() != pItem->GetValue()))
        rEdit.SetText(pItem->GetValue());
    SfxControllerItem::StateChanged (nSID, eState, pState);
}


/**************************************************************************/

SmCmdBoxWindow::SmCmdBoxWindow(SfxBindings *pBindings, SfxChildWindow *pChildWindow,
                               Window *pParent) :
    SfxDockingWindow(pBindings, pChildWindow, pParent, SmResId(RID_CMDBOXWINDOW)),
    aEdit       (*this),
    aController (aEdit, SID_TEXT, *pBindings),
    bExiting    (FALSE)
{
    Hide ();
    aGrabTimer.SetTimeout (1000);
    aGrabTimer.SetTimeoutHdl (LINK (this, SmCmdBoxWindow, UpdateTimeoutHdl));
    aGrabTimer.Start ();
}


SmCmdBoxWindow::~SmCmdBoxWindow ()
{
    bExiting = TRUE;
    aGrabTimer.Stop ();
}


SmViewShell * SmCmdBoxWindow::GetView()
{
    SfxViewShell *pView = GetBindings().GetDispatcher()->GetFrame()->GetViewShell();
    return PTR_CAST(SmViewShell, pView);
}

void SmCmdBoxWindow::Resize()
{
    Rectangle aRect = Rectangle(Point(0, 0), GetOutputSizePixel());

    if (! IsFloatingMode())
    {
        switch (GetAlignment())
        {
            case SFX_ALIGN_TOP:     aRect.Bottom()--;   break;
            case SFX_ALIGN_BOTTOM:  aRect.Top()++;      break;
            case SFX_ALIGN_LEFT:    aRect.Right()--;    break;
            case SFX_ALIGN_RIGHT:   aRect.Left()++;     break;
        }
    }

    DecorationView aView(this);
    aRect.Left() += 8; aRect.Top()   += 8;
    aRect.Right()-= 8; aRect.Bottom()-= 8;
    aRect = aView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );

    aEdit.SetPosSizePixel(aRect.TopLeft(), aRect.GetSize());
    SfxDockingWindow::Resize();
    Invalidate();
}


void SmCmdBoxWindow::Paint(const Rectangle& rRect)
{
    Rectangle aRect = Rectangle(Point(0, 0), GetOutputSizePixel());
    DecorationView aView(this);

    if (! IsFloatingMode())
    {
        Point aFrom, aTo;
        switch (GetAlignment())
        {
            case SFX_ALIGN_TOP:
                aFrom = aRect.BottomLeft(); aTo = aRect.BottomRight();
                aRect.Bottom()--;
                break;

            case SFX_ALIGN_BOTTOM:
                aFrom = aRect.TopLeft(); aTo = aRect.TopRight();
                aRect.Top()++;
                break;

            case SFX_ALIGN_LEFT:
                aFrom = aRect.TopRight(); aTo = aRect.BottomRight();
                aRect.Right()--;
                break;

            case SFX_ALIGN_RIGHT:
                aFrom = aRect.TopLeft(); aTo = aRect.BottomLeft();
                aRect.Left()++;
                break;
        }
        DrawLine( aFrom, aTo );
        aView.DrawFrame(aRect, FRAME_DRAW_OUT);
    }
    aRect.Left() += 8; aRect.Top()   += 8;
    aRect.Right()-= 8; aRect.Bottom()-= 8;
    aRect = aView.DrawFrame( aRect, FRAME_DRAW_DOUBLEIN );
}


Size SmCmdBoxWindow::CalcDockingSize(SfxChildAlignment eAlign)
{
    switch (eAlign)
    {
        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
            return Size();
/*
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
*/
    }
    return SfxDockingWindow::CalcDockingSize(eAlign);
}


IMPL_LINK(SmCmdBoxWindow, UpdateTimeoutHdl, Timer *, pTimer)
{
    aGrabTimer.Stop ();
    Grab();
    return 0;
}


void SmCmdBoxWindow::Grab ()
{
    GrabFocus ();
    aEdit.GrabFocus ();
}


SfxChildAlignment SmCmdBoxWindow::CheckAlignment(SfxChildAlignment eActual,
                                             SfxChildAlignment eWish)
{
    switch (eWish)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_NOALIGNMENT:
            return eWish;
    }

    return eActual;
}


void SmCmdBoxWindow::StateChanged( StateChangedType nStateChange )
{
    if (STATE_CHANGE_INITSHOW == nStateChange)
    {
        Resize();   // #98848# avoid SmEditWindow not being painted correctly

        // set initial position of window in floating mode
        if (TRUE == IsFloatingMode())
            AdjustPosition();   //! don't change pos in docking-mode !
    }

    SfxDockingWindow::StateChanged( nStateChange );
}


void SmCmdBoxWindow::AdjustPosition()
{
    Point aPt;
    const Rectangle aRect( aPt, GetParent()->GetOutputSizePixel() );
    Point aTopLeft( Point( aRect.Left(),
                           aRect.Bottom() - GetSizePixel().Height() ) );
    Point aPos( GetParent()->OutputToScreenPixel( aTopLeft ) );
    if (aPos.X() < 0)
        aPos.X() = 0;
    if (aPos.Y() < 0)
        aPos.Y() = 0;
    SetPosPixel( aPos );
}


void SmCmdBoxWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size (200, 50));
}


void SmCmdBoxWindow::GetFocus()
{
    if (!bExiting)
        aEdit.GrabFocus();
}

/**************************************************************************/


SFX_IMPL_DOCKINGWINDOW(SmCmdBoxWrapper, SID_CMDBOXWINDOW);

SmCmdBoxWrapper::SmCmdBoxWrapper(Window *pParentWindow, USHORT nId,
                                 SfxBindings *pBindings,
                                 SfxChildWinInfo *pInfo) :
    SfxChildWindow(pParentWindow, nId)
{
    pWindow = new SmCmdBoxWindow(pBindings, this, pParentWindow);

    // make window docked to the bottom initially (after first start)
    eChildAlignment = SFX_ALIGN_BOTTOM;
    ((SfxDockingWindow *)pWindow)->Initialize(pInfo);
}


#if OSL_DEBUG_LEVEL > 1
SmCmdBoxWrapper::~SmCmdBoxWrapper()
{
}
#endif


void SmCmdBoxWrapper::Grab ()
{
    ((SmCmdBoxWindow *)pWindow)->Grab ();
}


/**************************************************************************/


TYPEINIT1( SmViewShell, SfxViewShell );

SFX_IMPL_INTERFACE(SmViewShell, SfxViewShell, SmResId(0))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SmResId(RID_MATH_TOOLBOX ));
    //Dummy-Objectbar, damit es bei aktivieren nicht staendig zuppelt.
// Wegen #58705# entfernt (RID wurde nirgends verwendet)
//  SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT | SFX_VISIBILITY_SERVER,
//                              SmResId(RID_DRAW_OBJECTBAR) );

    SFX_CHILDWINDOW_REGISTRATION(SmToolBoxWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SmCmdBoxWrapper::GetChildWindowId());
}


SFX_IMPL_VIEWFACTORY(SmViewShell, SmResId(RID_VIEWNAME))
{
    SFX_VIEW_REGISTRATION(SmDocShell);
}


Size SmViewShell::GetOptimalSizePixel() const
{
    return aGraphic.LogicToPixel( ((SmViewShell*)this)->GetDoc()->GetSize() );
}


void SmViewShell::AdjustPosSizePixel(const Point &rPos, const Size &rSize)
{
    aGraphic.SetPosSizePixel(rPos, rSize);
}


void SmViewShell::InnerResizePixel(const Point &rOfs, const Size &rSize)
{
    //Ein bischen muessen wir schon raten was die Basisklasse tun wird.
    //Die Scrollbars muessen einkalkuliert werden damit sie Aussen sitzen,
    //denn genau dafuer steht ja das InnerResize
    const Size aDocSz = aGraphic.LogicToPixel( GetDoc()->GetSize() );
          Size aSize( rSize );

    SvBorder aBorder;


    if ( aSize.Width() < aDocSz.Width() )
    {
        if ( 1 == (aDocSz.Width() - aSize.Width()) )
        {
            //Einen Pixel daneben, kann ja mal passieren.
            aSize.Width() += 1;
        }
        else
        {
            aBorder.Bottom() = Application::GetSettings().GetStyleSettings().GetScrollBarSize();
            aSize.Height() += aBorder.Bottom();
        }
    }
    if ( aSize.Height() < aDocSz.Height() )
    {
        if ( 1 == (aDocSz.Height() - aSize.Height()) )
        {
            aSize.Height() += 1;
        }
        else
        {
            aBorder.Right() = Application::GetSettings().GetStyleSettings().GetScrollBarSize();
            aSize.Width() += aBorder.Right();
        }
    }
    SetBorderPixel( aBorder );
    GetGraphicWindow().SetPosSizePixel(rOfs, aSize);
    GetGraphicWindow().SetTotalSize();
}


void SmViewShell::OuterResizePixel(const Point &rOfs, const Size &rSize)
{
    SmGraphicWindow &rWin = GetGraphicWindow();
    rWin.SetPosSizePixel(rOfs, rSize);
    if (GetDoc()->IsPreview())
        rWin.ZoomToFitInWindow();
    rWin.Update();
}


void SmViewShell::QueryObjAreaPixel( Rectangle& rRect ) const
{
    rRect.SetSize( GetGraphicWindow().GetSizePixel() );
}


void SmViewShell::SetZoomFactor( const Fraction &rX, const Fraction &rY )
{
    const Fraction &rFrac = rX < rY ? rX : rY;
    GetGraphicWindow().SetZoom( (USHORT) long(rFrac * Fraction( 100, 1 )) );

    //Um Rundungsfehler zu minimieren lassen wir von der Basisklasse ggf.
    //auch die krummen Werte einstellen
    SfxViewShell::SetZoomFactor( rX, rY );
}


Size SmViewShell::GetTextLineSize(OutputDevice& rDevice, const String& rLine)
{
    String aText;
    Size   aSize(rDevice.GetTextWidth(rLine), rDevice.GetTextHeight());
    USHORT nTabs = rLine.GetTokenCount('\t');

    if (nTabs > 0)
    {
        long TabPos = rDevice.GetTextWidth('n') * 8;

        aSize.Width() = 0;

        for (USHORT i = 0; i < nTabs; i++)
        {
            if (i > 0)
                aSize.Width() = ((aSize.Width() / TabPos) + 1) * TabPos;

            aText = rLine.GetToken(i, '\t');
            aText.EraseLeadingChars('\t');
            aText.EraseTrailingChars('\t');
            aSize.Width() += rDevice.GetTextWidth(aText);
        }
    }

    return aSize;
}


Size SmViewShell::GetTextSize(OutputDevice& rDevice, const String& rText, long MaxWidth)
{
    Size    aSize;
    String  aLine;
    Size    TextSize;
    String  aText;
    USHORT  nLines = rText.GetTokenCount('\n');

    for (USHORT i = 0; i < nLines; i++)
    {
        aLine = rText.GetToken(i, '\n');
        aLine.EraseAllChars('\r');
        aLine.EraseLeadingChars('\n');
        aLine.EraseTrailingChars('\n');

        aSize = GetTextLineSize(rDevice, aLine);

        if (aSize.Width() > MaxWidth)
        {
            do
            {
                xub_StrLen m    = aLine.Len();
                xub_StrLen nLen = m;

                for (xub_StrLen n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine.GetChar(n);
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.Copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }

                aText = aLine.Copy(0, m);
                aLine.Erase(0, m);
                aSize = GetTextLineSize(rDevice, aText);
                TextSize.Height() += aSize.Height();
                TextSize.Width() = Max(TextSize.Width(), Min(aSize.Width(), MaxWidth));

                aLine.EraseLeadingChars(' ');
                aLine.EraseLeadingChars('\t');
                aLine.EraseLeadingChars(' ');
            }
            while (aLine.Len() > 0);
        }
        else
        {
            TextSize.Height() += aSize.Height();
            TextSize.Width() = Max(TextSize.Width(), aSize.Width());
        }
    }

    return TextSize;
}


void SmViewShell::DrawTextLine(OutputDevice& rDevice, const Point& rPosition, const String& rLine)
{
    String  aText;
    Point   aPoint (rPosition);
    USHORT  nTabs = rLine.GetTokenCount('\t');

    if (nTabs > 0)
    {
        long TabPos = rDevice.GetTextWidth('n') * 8;

        for (USHORT i = 0; i < nTabs; i++)
        {
            if (i > 0)
                aPoint.X() = ((aPoint.X() / TabPos) + 1) * TabPos;

            aText = rLine.GetToken(i, '\t');
            aText.EraseLeadingChars('\t');
            aText.EraseTrailingChars('\t');
            rDevice.DrawText(aPoint, aText);
            aPoint.X() += rDevice.GetTextWidth(aText);
        }
    }
    else
        rDevice.DrawText(aPoint, rLine);
}


void SmViewShell::DrawText(OutputDevice& rDevice, const Point& rPosition, const String& rText, USHORT MaxWidth)
{
    USHORT  nLines = rText.GetTokenCount('\n');
    Point   aPoint (rPosition);
    Size    aSize;
    String  aLine;
    String  aText;

    for (USHORT i = 0; i < nLines; i++)
    {
        aLine = rText.GetToken(i, '\n');
        aLine.EraseAllChars('\r');
        aLine.EraseLeadingChars('\n');
        aLine.EraseTrailingChars('\n');
        aSize = GetTextLineSize(rDevice, aLine);
        if (aSize.Width() > MaxWidth)
        {
            do
            {
                xub_StrLen m    = aLine.Len();
                xub_StrLen nLen = m;

                for (xub_StrLen n = 0; n < nLen; n++)
                {
                    sal_Unicode cLineChar = aLine.GetChar(n);
                    if ((cLineChar == ' ') || (cLineChar == '\t'))
                    {
                        aText = aLine.Copy(0, n);
                        if (GetTextLineSize(rDevice, aText).Width() < MaxWidth)
                            m = n;
                        else
                            break;
                    }
                }
                aText = aLine.Copy(0, m);
                aLine.Erase(0, m);

                DrawTextLine(rDevice, aPoint, aText);
                aPoint.Y() += aSize.Height();

                aLine.EraseLeadingChars(' ');
                aLine.EraseLeadingChars('\t');
                aLine.EraseLeadingChars(' ');
            }
            while (GetTextLineSize(rDevice, aLine).Width() > MaxWidth);

            // print the remaining text
            if (aLine.Len() > 0)
            {
                DrawTextLine(rDevice, aPoint, aLine);
                aPoint.Y() += aSize.Height();
            }
        }
        else
        {
            DrawTextLine(rDevice, aPoint, aLine);
            aPoint.Y() += aSize.Height();
        }
    }
}

void SmViewShell::Impl_Print(
        OutputDevice &rOutDev, const SmPrintSize ePrintSize,
        Rectangle aOutRect, Point aZeroPoint )
{
    SmModule *pp = SM_MOD1();

    rOutDev.Push();
    rOutDev.SetLineColor( Color(COL_BLACK) );

    // output text on top
    if (pp->GetConfig()->IsPrintTitle())
    {
        Size aSize600 (0, 600);
        Size aSize650 (0, 650);
        Font aFont(FAMILY_DONTKNOW, aSize600);

        aFont.SetAlign(ALIGN_TOP);
        aFont.SetWeight(WEIGHT_BOLD);
        aFont.SetSize(aSize650);
        rOutDev.SetFont(aFont);

        Size aTitleSize (GetTextSize(rOutDev, GetDoc()->GetTitle(), aOutRect.GetWidth() - 200));

        aFont.SetWeight(WEIGHT_NORMAL);
        aFont.SetSize(aSize600);
        rOutDev.SetFont(aFont);

        Size aDescSize (GetTextSize(rOutDev, GetDoc()->GetComment(), aOutRect.GetWidth() - 200));

        // output rectangle
        SmModule *pp = SM_MOD1();

        if (pp->GetConfig()->IsPrintFrame())
            rOutDev.DrawRect(Rectangle(aOutRect.TopLeft(),
                               Size(aOutRect.GetWidth(), 100 + aTitleSize.Height() + 200 + aDescSize.Height() + 100)));
        aOutRect.Top() += 200;

        // output title
        aFont.SetWeight(WEIGHT_BOLD);
        aFont.SetSize(aSize650);
        rOutDev.SetFont(aFont);
        DrawText(rOutDev, Point (aOutRect.Left() + (aOutRect.GetWidth() - aTitleSize.Width())  / 2,
                                   aOutRect.Top()), GetDoc()->GetTitle(), aOutRect.GetWidth() - 200);
        aOutRect.Top() += aTitleSize.Height() + 200;

        // output description
        aFont.SetWeight(WEIGHT_NORMAL);
        aFont.SetSize(aSize600);
        rOutDev.SetFont(aFont);
        DrawText(rOutDev, Point (aOutRect.Left() + (aOutRect.GetWidth()  - aDescSize.Width())  / 2,
                                   aOutRect.Top()), GetDoc()->GetComment(), aOutRect.GetWidth() - 200);
        aOutRect.Top() += aDescSize.Height() + 300;
    }

    // output text on bottom
    if (pp->GetConfig()->IsPrintFormulaText())
    {
//        Font aFont(FAMILY_DONTKNOW, Size(0, 600));
        Font aFont;

        aFont.SetAlign(ALIGN_TOP);

        // get size
        rOutDev.SetFont(aFont);

        Size aSize (GetTextSize(rOutDev, GetDoc()->GetText(), aOutRect.GetWidth() - 200));

        aOutRect.Bottom() -= aSize.Height() + 600;

        // output rectangle
        SmModule *pp = SM_MOD1();

        if (pp->GetConfig()->IsPrintFrame())
            rOutDev.DrawRect(Rectangle(aOutRect.BottomLeft(),
                               Size(aOutRect.GetWidth(), 200 + aSize.Height() + 200)));

        DrawText(rOutDev, Point (aOutRect.Left() + (aOutRect.GetWidth()  - aSize.Width())  / 2,
                                   aOutRect.Bottom() + 300), GetDoc()->GetText(), aOutRect.GetWidth() - 200);
        aOutRect.Bottom() -= 200;
    }

    if (pp->GetConfig()->IsPrintFrame())
        rOutDev.DrawRect(aOutRect);

    aOutRect.Top()    += 100;
    aOutRect.Left()   += 100;
    aOutRect.Bottom() -= 100;
    aOutRect.Right()  -= 100;

    Size aSize (GetDoc()->GetSize());

    MapMode    OutputMapMode;
    switch (ePrintSize)
    {
        case PRINT_SIZE_NORMAL:
            OutputMapMode = MapMode(MAP_100TH_MM);
            break;

        case PRINT_SIZE_SCALED:
            if ((aSize.Width() > 0) && (aSize.Height() > 0))
            {
                Size     OutputSize (rOutDev.LogicToPixel(Size(aOutRect.GetWidth(),
                                                            aOutRect.GetHeight()), MapMode(MAP_100TH_MM)));
                Size     GraphicSize (rOutDev.LogicToPixel(aSize, MapMode(MAP_100TH_MM)));
                USHORT   nZ = (USHORT) Min((long)Fraction(OutputSize.Width()  * 100L, GraphicSize.Width()),
                                              (long)Fraction(OutputSize.Height() * 100L, GraphicSize.Height()));
                Fraction aFraction ((USHORT) Max ((USHORT) MINZOOM, Min((USHORT) MAXZOOM, (USHORT) (nZ - 10))), (USHORT) 100);

                OutputMapMode = MapMode(MAP_100TH_MM, aZeroPoint, aFraction, aFraction);
            }
            else
                OutputMapMode = MapMode(MAP_100TH_MM);
            break;

        case PRINT_SIZE_ZOOMED:
        {
            SmModule *pp = SM_MOD1();
            Fraction aFraction (pp->GetConfig()->GetPrintZoomFactor(), 100);

            OutputMapMode = MapMode(MAP_100TH_MM, aZeroPoint, aFraction, aFraction);
            break;
        }
    }

    aSize = rOutDev.PixelToLogic(rOutDev.LogicToPixel(aSize, OutputMapMode),
                                   MapMode(MAP_100TH_MM));

    Point aPos (aOutRect.Left() + (aOutRect.GetWidth()  - aSize.Width())  / 2,
                aOutRect.Top()  + (aOutRect.GetHeight() - aSize.Height()) / 2);

    aPos     = rOutDev.PixelToLogic(rOutDev.LogicToPixel(aPos, MapMode(MAP_100TH_MM)),
                                          OutputMapMode);
    aOutRect   = rOutDev.PixelToLogic(rOutDev.LogicToPixel(aOutRect, MapMode(MAP_100TH_MM)),
                                          OutputMapMode);

    rOutDev.SetMapMode(OutputMapMode);
    rOutDev.SetClipRegion(Region(aOutRect));
    GetDoc()->Draw(rOutDev, aPos);
    rOutDev.SetClipRegion();

    rOutDev.Pop();
}

USHORT SmViewShell::Print(SfxProgress &rProgress, PrintDialog *pPrintDialog)
{
    SmPrinterAccess aPrinterAccess( *GetDoc() );
    Printer *pPrinter = aPrinterAccess.GetPrinter();
    OutputDevice *pOutDev = pPrinter;

    SfxViewShell::Print (rProgress, pPrintDialog);

    pPrinter->StartPage();

    Point     aZeroPoint;
    Rectangle OutputRect( aZeroPoint, pPrinter->GetOutputSize() );

    Point   aPrtPageOffset( pPrinter->GetPageOffset() );
    Size    aPrtPaperSize ( pPrinter->GetPaperSize() );

    // set minimum top and bottom border
    if (aPrtPageOffset.Y() < 2000)
        OutputRect.Top() += 2000 - aPrtPageOffset.Y();
    if ((aPrtPaperSize.Height() - (aPrtPageOffset.Y() + OutputRect.Bottom())) < 2000)
        OutputRect.Bottom() -= 2000 - (aPrtPaperSize.Height() -
                                       (aPrtPageOffset.Y() + OutputRect.Bottom()));

    // set minimum left and right border
    if (aPrtPageOffset.X() < 2500)
        OutputRect.Left() += 2500 - aPrtPageOffset.X();
    if ((aPrtPaperSize.Width() - (aPrtPageOffset.X() + OutputRect.Right())) < 1500)
        OutputRect.Right() -= 1500 - (aPrtPaperSize.Width() -
                                      (aPrtPageOffset.X() + OutputRect.Right()));

    SmModule *pp = SM_MOD1();
    Impl_Print( *pPrinter, pp->GetConfig()->GetPrintSize(),
                 OutputRect, aZeroPoint );

    pPrinter->EndPage();

    return 0;
}


SfxPrinter* SmViewShell::GetPrinter(BOOL bCreate)
{
    SmDocShell *pDoc = GetDoc();
    if ( pDoc->HasPrinter() || bCreate )
        return pDoc->GetPrinter();
    return 0;
}


USHORT SmViewShell::SetPrinter(SfxPrinter *pNewPrinter, USHORT nDiffFlags)
{
    if ((nDiffFlags & SFX_PRINTER_PRINTER) == SFX_PRINTER_PRINTER)
        GetDoc()->SetPrinter( pNewPrinter );

    if ((nDiffFlags & SFX_PRINTER_OPTIONS) == SFX_PRINTER_OPTIONS)
    {
        SmModule *pp = SM_MOD1();
        pp->GetConfig()->ItemSetToConfig(pNewPrinter->GetOptions());
    }
    return 0;
}


SfxTabPage* SmViewShell::CreatePrintOptionsPage(Window *pParent,
                                                const SfxItemSet &rOptions)
{
    return SmPrintOptionsTabPage::Create(pParent, rOptions);
}


SmEditWindow *SmViewShell::GetEditWindow()
{
    SmCmdBoxWrapper *pWrapper = (SmCmdBoxWrapper *) GetViewFrame()->
            GetChildWindow( SmCmdBoxWrapper::GetChildWindowId() );

    if (pWrapper != NULL)
    {
        SmEditWindow *pEditWin  = pWrapper->GetEditWindow();
        DBG_ASSERT( pEditWin, "SmEditWindow missing" );
        return pEditWin;
    }

    return NULL;
}


void SmViewShell::SetStatusText(const String& Text)
{
    StatusText = Text;
    GetViewFrame()->GetBindings().Invalidate(SID_TEXTSTATUS);
}


void SmViewShell::ShowError( const SmErrorDesc *pErrorDesc )
{
    DBG_ASSERT(GetDoc(), "Sm : Document missing");
    if (pErrorDesc || 0 != (pErrorDesc = GetDoc()->GetParser().GetError(0)) )
    {
        SetStatusText( pErrorDesc->Text );
        GetEditWindow()->MarkError( Point( pErrorDesc->pNode->GetColumn(),
                                           pErrorDesc->pNode->GetRow()));
    }
}


void SmViewShell::NextError()
{
    DBG_ASSERT(GetDoc(), "Sm : Document missing");
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser().NextError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}


void SmViewShell::PrevError()
{
    DBG_ASSERT(GetDoc(), "Sm : Document missing");
    const SmErrorDesc   *pErrorDesc = GetDoc()->GetParser().PrevError();

    if (pErrorDesc)
        ShowError( pErrorDesc );
}


void SmViewShell::Execute(SfxRequest& rReq)
{
    SmEditWindow *pWin = GetEditWindow();

    switch (rReq.GetSlot())
    {
        case SID_FORMULACURSOR:
        {
            SmModule *pp = SM_MOD1();

            const SfxItemSet  *pArgs = rReq.GetArgs();
            const SfxPoolItem *pItem;

            BOOL  bVal;
            if ( pArgs &&
                 SFX_ITEM_SET == pArgs->GetItemState( SID_FORMULACURSOR, FALSE, &pItem))
                bVal = ((SfxBoolItem *) pItem)->GetValue();
            else
                bVal = !pp->GetConfig()->IsShowFormulaCursor();

            pp->GetConfig()->SetShowFormulaCursor(bVal);
            GetGraphicWindow().ShowCursor(bVal);
            break;
        }
        case SID_DRAW:
            if (pWin)
            {
                GetDoc()->SetText( pWin->GetText() );
                SetStatusText(String());
                ShowError( 0 );
                GetDoc()->Resize();
            }
            break;

        case SID_ADJUST:
        case SID_FITINWINDOW:
            aGraphic.ZoomToFitInWindow();
            break;

        case SID_VIEW050:
            aGraphic.SetZoom(50);
            break;

        case SID_VIEW100:
            aGraphic.SetZoom(100);
            break;

        case SID_VIEW200:
            aGraphic.SetZoom(200);
            break;

        case SID_ZOOMIN:
            aGraphic.SetZoom(aGraphic.GetZoom() + 25);
            break;

        case SID_ZOOMOUT:
            DBG_ASSERT(aGraphic.GetZoom() >= 25, "Sm: falsches USHORT Argument");
            aGraphic.SetZoom(aGraphic.GetZoom() - 25);
            break;

        case SID_CUT:
            if (pWin)
                pWin->Cut();
            break;

        case SID_COPY:
            if (pWin)
            {
                if (pWin->IsAllSelected())
                {
                    SmViewShell *pViewSh = SmGetActiveView();
                    if (pViewSh)
                        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                                SID_COPYOBJECT, SFX_CALLMODE_STANDARD,
                                new SfxVoidItem(SID_COPYOBJECT), 0L);
                }
                else
                    pWin->Copy();
            }
            break;

        case SID_PASTE:
            {
                BOOL bCallExec = 0 == pWin;
                if( !bCallExec )
                {
                    TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                    GetEditWindow()) );

                    if( aDataHelper.GetTransferable().is() &&
                        aDataHelper.HasFormat( FORMAT_STRING ))
                        pWin->Paste();
                    else
                        bCallExec = TRUE;
                }
                if( bCallExec )
                {
                    SmViewShell *pViewSh = SmGetActiveView();
                    if( pViewSh )
                        pViewSh->GetViewFrame()->GetDispatcher()->Execute(
                            SID_PASTEOBJECT, SFX_CALLMODE_STANDARD,
                            new SfxVoidItem(SID_PASTEOBJECT), 0L);
                }
            }
            break;

        case SID_DELETE:
            if (pWin)
                pWin->Delete();
            break;

        case SID_SELECT:
            if (pWin)
                pWin->SelectAll();
            break;

        case SID_INSERTCOMMAND:
        {
            const SfxInt16Item& rItem =
                (const SfxInt16Item&)rReq.GetArgs()->Get(SID_INSERTCOMMAND);

            if (pWin)
                pWin->InsertCommand(rItem.GetValue());
            break;
        }

        case SID_INSERTTEXT:
        {
            const SfxStringItem& rItem =
                (const SfxStringItem&)rReq.GetArgs()->Get(SID_INSERTTEXT);

            if (pWin)
                pWin->InsertText(rItem.GetValue());
            break;
        }

        case SID_NEXTERR:
            NextError();
            if (pWin)
                pWin->GrabFocus();
            break;

        case SID_PREVERR:
            PrevError();
            if (pWin)
                pWin->GrabFocus();
            break;

        case SID_NEXTMARK:
            if (pWin)
            {
                pWin->SelNextMark();
                pWin->GrabFocus();
            }
            break;

        case SID_PREVMARK:
            if (pWin)
            {
                pWin->SelPrevMark();
                pWin->GrabFocus();
            }
            break;

        case SID_TEXTSTATUS:
        {
            if (rReq.GetArgs() != NULL)
            {
                const SfxStringItem& rItem =
                    (const SfxStringItem&)rReq.GetArgs()->Get(SID_TEXTSTATUS);

                SetStatusText(rItem.GetValue());
            }

            break;
        }

        case SID_GETEDITTEXT:
            if (pWin)
                if (pWin->GetText ().Len ()) GetDoc()->SetText( pWin->GetText() );
            break;

        case SID_ATTR_ZOOM:
        {
            if ( !GetDoc()->GetProtocol().IsInPlaceActive() )
            {
                SvxZoomDialog *pDlg = 0;
                const SfxItemSet *pSet = rReq.GetArgs();
                if ( !pSet )
                {
                    SfxItemSet aSet( GetDoc()->GetPool(), SID_ATTR_ZOOM, SID_ATTR_ZOOM);
                    aSet.Put( SvxZoomItem( SVX_ZOOM_PERCENT, aGraphic.GetZoom()));
                    pDlg = new SvxZoomDialog( &GetViewFrame()->GetWindow(), aSet);
                    pDlg->SetLimits( MINZOOM, MAXZOOM );
                    if( pDlg->Execute() != RET_CANCEL )
                        pSet = pDlg->GetOutputItemSet();
                }
                if ( pSet )
                {
                    const SvxZoomItem &rZoom = (const SvxZoomItem &)pSet->Get(SID_ATTR_ZOOM);
                    switch( rZoom.GetType() )
                    {
                        case SVX_ZOOM_PERCENT:
                            aGraphic.SetZoom((USHORT)rZoom.GetValue ());
                            break;

                        case SVX_ZOOM_OPTIMAL:
                            aGraphic.ZoomToFitInWindow();
                            break;

                        case SVX_ZOOM_PAGEWIDTH:
                        case SVX_ZOOM_WHOLEPAGE:
                        {
                            const MapMode aMap( MAP_100TH_MM );
                            SfxPrinter *pPrinter = GetPrinter( TRUE );
                            Point aPoint;
                            Rectangle  OutputRect(aPoint, pPrinter->GetOutputSize());
                            Size       OutputSize(pPrinter->LogicToPixel(Size(OutputRect.GetWidth(),
                                                                              OutputRect.GetHeight()), aMap));
                            Size       GraphicSize(pPrinter->LogicToPixel(GetDoc()->GetSize(), aMap));
                            USHORT     nZ = (USHORT) Min((long)Fraction(OutputSize.Width()  * 100L, GraphicSize.Width()),
                                                         (long)Fraction(OutputSize.Height() * 100L, GraphicSize.Height()));
                            aGraphic.SetZoom (nZ);
                            break;
                        }
                    }
                }
                delete pDlg;
            }
        }
        break;
    }
    rReq.Done();
}


void SmViewShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);

    SmEditWindow *pEditWin = GetEditWindow();
    for (USHORT nWh = aIter.FirstWhich(); nWh != 0; nWh = aIter.NextWhich())
    {
        switch (nWh)
        {
        case SID_CUT:
        case SID_COPY:
        case SID_DELETE:
            if (! pEditWin || ! pEditWin->IsSelected())
                rSet.DisableItem(nWh);
            break;

        case SID_PASTE:
            if( !xClipEvtLstnr.is()  &&  pEditWin)
            {
                AddRemoveClipboardListener( TRUE );
                TransferableDataHelper aDataHelper(
                        TransferableDataHelper::CreateFromSystemClipboard(
                                                        pEditWin) );

                bPasteState = aDataHelper.GetTransferable().is() &&
                 ( aDataHelper.HasFormat( FORMAT_STRING ) ||
                   aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBEDDED_OBJ ) ||
                   (aDataHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR )
                      && aDataHelper.HasFormat( SOT_FORMATSTR_ID_EMBED_SOURCE )));
            }
            if( !bPasteState )
                rSet.DisableItem( nWh );
            break;

        case SID_ATTR_ZOOM:
            rSet.Put(SvxZoomItem( SVX_ZOOM_PERCENT, aGraphic.GetZoom()));
            /* no break here */
        case SID_VIEW050:
        case SID_VIEW100:
        case SID_VIEW200:
        case SID_ADJUST:
        case SID_ZOOMIN:
        case SID_ZOOMOUT:
        case SID_FITINWINDOW:
            if ( GetDoc()->GetProtocol().IsInPlaceActive() )
                rSet.DisableItem( nWh );
            break;

        case SID_NEXTERR:
        case SID_PREVERR:
        case SID_NEXTMARK:
        case SID_PREVMARK:
        case SID_DRAW:
        case SID_SELECT:
            if (! pEditWin || pEditWin->IsEmpty())
                rSet.DisableItem(nWh);
            break;

        case SID_TEXTSTATUS:
            {
                rSet.Put(SfxStringItem(nWh, StatusText));
            }
            break;

        case SID_FORMULACURSOR:
            {
                SmModule *pp = SM_MOD1();
                rSet.Put(SfxBoolItem(nWh, pp->GetConfig()->IsShowFormulaCursor()));
            }
            break;
        }
    }
}


SmViewShell::SmViewShell(SfxViewFrame *pFrame, SfxViewShell *):
    SfxViewShell(pFrame, SFX_VIEW_DISABLE_ACCELS | SFX_VIEW_MAXIMIZE_FIRST | SFX_VIEW_HAS_PRINTOPTIONS | SFX_VIEW_CAN_PRINT),
    aGraphic(this),
    aGraphicController(aGraphic, SID_GRAPHIC, pFrame->GetBindings())
{
    pViewFrame = &pFrame->GetWindow();

    SetStatusText(String());
    SetWindow(&aGraphic);
    SfxShell::SetName(C2S("SmView"));
    SfxShell::SetUndoManager( &GetDoc()->GetEditEngine().GetUndoManager() );
    SetHelpId( HID_SMA_VIEWSHELL_DOCUMENT );
}


SmViewShell::~SmViewShell()
{
    AddRemoveClipboardListener( FALSE );

    //!! this view shell is not active anymore !!
    // Thus 'SmGetActiveView' will give a 0 pointer.
    // Thus we need to supply this view as argument
    SmEditWindow *pEditWin = GetEditWindow();
    if (pEditWin)
        pEditWin->DeleteEditView( *this );
}

void SmViewShell::Deactivate( BOOL bIsMDIActivate )
{
    SmEditWindow *pEdit = GetEditWindow();
    if ( pEdit )
        pEdit->Flush();

    SfxViewShell::Deactivate( bIsMDIActivate );
}


void SmViewShell::Activate( BOOL bIsMDIActivate )
{
    SfxViewShell::Activate( bIsMDIActivate );

    SmEditWindow *pEdit = GetEditWindow();
    if ( pEdit )
    {
        //! Since there is no way to be informed if a "drag and drop"
        //! event has taken place, we call SetText here in order to
        //! syncronize the GraphicWindow display with the text in the
        //! EditEngine.
        SmDocShell *pDoc = GetDoc();
        pDoc->SetText( pDoc->GetEditEngine().GetText( LINEEND_LF ) );

        if ( bIsMDIActivate )
            pEdit->GrabFocus();
    }
}


