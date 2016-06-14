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

#include "menubarwindow.hxx"
#include "menuitemlist.hxx"
#include "menufloatingwindow.hxx"

#include <vcl/dockingarea.hxx>
#include <vcl/settings.hxx>
#include <vcl/taskpanelist.hxx>

#include <salframe.hxx>
#include <salmenu.hxx>
#include <svdata.hxx>
#include <svids.hrc>
#include <window.h>

// document closing button
#define IID_DOCUMENTCLOSE 1

DecoToolBox::DecoToolBox( vcl::Window* pParent ) :
    ToolBox( pParent, 0 )
{
    ImplInit();
}

void DecoToolBox::ImplInit()
{
    lastSize = -1;
    calcMinSize();
}

void DecoToolBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE )
    {
        calcMinSize();
        SetBackground();
        SetImages( 0, true);
    }
}

void DecoToolBox::calcMinSize()
{
    ScopedVclPtrInstance<ToolBox> aTbx( GetParent() );
    if( GetItemCount() == 0 )
    {
        ResMgr* pResMgr = ImplGetResMgr();

        Bitmap aBitmap;
        if( pResMgr )
            aBitmap = Bitmap( ResId( SV_RESID_BITMAP_CLOSEDOC, *pResMgr ) );
        aTbx->InsertItem( IID_DOCUMENTCLOSE, Image( aBitmap ) );
    }
    else
    {
        sal_uInt16 nItems = GetItemCount();
        for( sal_uInt16 i = 0; i < nItems; i++ )
        {
            sal_uInt16 nId = GetItemId( i );
            aTbx->InsertItem( nId, GetItemImage( nId ) );
        }
    }
    aTbx->SetOutStyle( TOOLBOX_STYLE_FLAT );
    maMinSize = aTbx->CalcWindowSizePixel();

    aTbx.disposeAndClear();
}

void DecoToolBox::SetImages( long nMaxHeight, bool bForce )
{
    long border = getMinSize().Height() - maImage.GetSizePixel().Height();

    if( !nMaxHeight && lastSize != -1 )
        nMaxHeight = lastSize + border; // don't change anything if called with 0

    if( nMaxHeight < getMinSize().Height() )
        nMaxHeight = getMinSize().Height();

    if( (lastSize != nMaxHeight - border) || bForce )
    {
        lastSize = nMaxHeight - border;

        Color       aEraseColor( 255, 255, 255, 255 );
        BitmapEx    aBmpExDst( maImage.GetBitmapEx() );
        BitmapEx    aBmpExSrc( aBmpExDst );

        aEraseColor.SetTransparency( 255 );
        aBmpExDst.Erase( aEraseColor );
        aBmpExDst.SetSizePixel( Size( lastSize, lastSize ) );

        Rectangle aSrcRect( Point(0,0), maImage.GetSizePixel() );
        Rectangle aDestRect( Point((lastSize - maImage.GetSizePixel().Width())/2,
                                (lastSize - maImage.GetSizePixel().Height())/2 ),
                            maImage.GetSizePixel() );

        aBmpExDst.CopyPixel( aDestRect, aSrcRect, &aBmpExSrc );
        SetItemImage( IID_DOCUMENTCLOSE, Image( aBmpExDst ) );
    }
}

MenuBarWindow::MenuBarWindow( vcl::Window* pParent ) :
    Window( pParent, 0 ),
    aCloseBtn(VclPtr<DecoToolBox>::Create(this)),
    aFloatBtn(VclPtr<PushButton>::Create(this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE)),
    aHideBtn(VclPtr<PushButton>::Create(this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE))
{
    SetType(WINDOW_MENUBARWINDOW);
    pMenu = nullptr;
    pActivePopup = nullptr;
    nHighlightedItem = ITEMPOS_INVALID;
    nRolloveredItem = ITEMPOS_INVALID;
    mbAutoPopup = true;
    bIgnoreFirstMove = true;
    bStayActive = false;
    SetMBWHideAccel(true);
    SetMBWMenuKey(false);

    ResMgr* pResMgr = ImplGetResMgr();

    if(pResMgr)
    {
        BitmapEx aBitmap(ResId(SV_RESID_BITMAP_CLOSEDOC, *pResMgr));
        aCloseBtn->maImage = Image(aBitmap);

        aCloseBtn->SetOutStyle(TOOLBOX_STYLE_FLAT);
        aCloseBtn->SetBackground();
        aCloseBtn->SetPaintTransparent(true);
        aCloseBtn->SetParentClipMode(ParentClipMode::NoClip);

        aCloseBtn->InsertItem(IID_DOCUMENTCLOSE, aCloseBtn->maImage);
        aCloseBtn->SetSelectHdl(LINK(this, MenuBarWindow, CloseHdl));
        aCloseBtn->AddEventListener(LINK(this, MenuBarWindow, ToolboxEventHdl));
        aCloseBtn->SetQuickHelpText(IID_DOCUMENTCLOSE, ResId(SV_HELPTEXT_CLOSEDOCUMENT, *pResMgr).toString());

        aFloatBtn->SetSymbol( SymbolType::FLOAT );
        aFloatBtn->SetQuickHelpText( ResId(SV_HELPTEXT_RESTORE, *pResMgr).toString() );

        aHideBtn->SetSymbol( SymbolType::HIDE );
        aHideBtn->SetQuickHelpText( ResId(SV_HELPTEXT_MINIMIZE, *pResMgr).toString() );
    }

    ImplInitStyleSettings();

    AddEventListener(LINK(this, MenuBarWindow, ShowHideListener));
}

MenuBarWindow::~MenuBarWindow()
{
    disposeOnce();
}

void MenuBarWindow::dispose()
{
    aCloseBtn->RemoveEventListener(LINK(this, MenuBarWindow, ToolboxEventHdl));
    RemoveEventListener(LINK(this, MenuBarWindow, ShowHideListener));

    aHideBtn.disposeAndClear();
    aFloatBtn.disposeAndClear();
    aCloseBtn.disposeAndClear();

    Window::dispose();
}

void MenuBarWindow::SetMenu( MenuBar* pMen )
{
    pMenu = pMen;
    KillActivePopup();
    nHighlightedItem = ITEMPOS_INVALID;
    if (pMen)
    {
        aCloseBtn->ShowItem(IID_DOCUMENTCLOSE, pMen->HasCloseButton());
        aCloseBtn->Show(pMen->HasCloseButton() || !m_aAddButtons.empty());
        aFloatBtn->Show(pMen->HasFloatButton());
        aHideBtn->Show(pMen->HasHideButton());
    }
    Invalidate();

    // show and connect native menubar
    if( pMenu && pMenu->ImplGetSalMenu() )
    {
        if( pMenu->ImplGetSalMenu()->VisibleMenuBar() )
            ImplGetFrame()->SetMenu( pMenu->ImplGetSalMenu() );

        pMenu->ImplGetSalMenu()->SetFrame( ImplGetFrame() );
    }
}

void MenuBarWindow::SetHeight(long nHeight)
{
    setPosSizePixel(0, 0, 0, nHeight, PosSizeFlags::Height);
}

void MenuBarWindow::ShowButtons( bool bClose, bool bFloat, bool bHide )
{
    aCloseBtn->ShowItem(IID_DOCUMENTCLOSE, bClose);
    aCloseBtn->Show(bClose || !m_aAddButtons.empty());
    if (pMenu->mpSalMenu)
        pMenu->mpSalMenu->ShowCloseButton(bClose);
    aFloatBtn->Show( bFloat );
    aHideBtn->Show( bHide );
    Resize();
}

Size MenuBarWindow::MinCloseButtonSize()
{
    return aCloseBtn->getMinSize();
}

IMPL_LINK_NOARG_TYPED(MenuBarWindow, CloseHdl, ToolBox *, void)
{
    if( ! pMenu )
        return;

    if( aCloseBtn->GetCurItemId() == IID_DOCUMENTCLOSE )
    {
        // #i106052# call close hdl asynchronously to ease handler implementation
        // this avoids still being in the handler while the DecoToolBox already
        // gets destroyed
        Application::PostUserEvent(static_cast<MenuBar*>(pMenu)->GetCloseButtonClickHdl());
    }
    else
    {
        std::map<sal_uInt16,AddButtonEntry>::iterator it = m_aAddButtons.find(aCloseBtn->GetCurItemId());
        if( it != m_aAddButtons.end() )
        {
            MenuBar::MenuBarButtonCallbackArg aArg;
            aArg.nId = it->first;
            aArg.bHighlight = (aCloseBtn->GetHighlightItemId() == it->first);
            aArg.pMenuBar = dynamic_cast<MenuBar*>(pMenu);
            it->second.m_aSelectLink.Call( aArg );
        }
    }
}

IMPL_LINK_TYPED( MenuBarWindow, ToolboxEventHdl, VclWindowEvent&, rEvent, void )
{
    if( ! pMenu )
        return;

    MenuBar::MenuBarButtonCallbackArg aArg;
    aArg.nId = 0xffff;
    aArg.bHighlight = (rEvent.GetId() == VCLEVENT_TOOLBOX_HIGHLIGHT);
    aArg.pMenuBar = dynamic_cast<MenuBar*>(pMenu);
    if( rEvent.GetId() == VCLEVENT_TOOLBOX_HIGHLIGHT )
        aArg.nId = aCloseBtn->GetHighlightItemId();
    else if( rEvent.GetId() == VCLEVENT_TOOLBOX_HIGHLIGHTOFF )
    {
        sal_uInt16 nPos = static_cast< sal_uInt16 >(reinterpret_cast<sal_IntPtr>(rEvent.GetData()));
        aArg.nId = aCloseBtn->GetItemId(nPos);
    }
    std::map< sal_uInt16, AddButtonEntry >::iterator it = m_aAddButtons.find( aArg.nId );
    if( it != m_aAddButtons.end() )
    {
        it->second.m_aHighlightLink.Call( aArg );
    }
}

IMPL_LINK_TYPED( MenuBarWindow, ShowHideListener, VclWindowEvent&, rEvent, void )
{
    if( ! pMenu )
        return;

    if( rEvent.GetId() == VCLEVENT_WINDOW_SHOW )
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_SHOW, ITEMPOS_INVALID );
    else if( rEvent.GetId() == VCLEVENT_WINDOW_HIDE )
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_HIDE, ITEMPOS_INVALID );
}

void MenuBarWindow::ImplCreatePopup( bool bPreSelectFirst )
{
    MenuItemData* pItemData = pMenu ? pMenu->GetItemList()->GetDataFromPos( nHighlightedItem ) : nullptr;
    if ( pItemData )
    {
        bIgnoreFirstMove = true;
        if ( pActivePopup && ( pActivePopup != pItemData->pSubMenu ) )
        {
            KillActivePopup();
        }
        if ( pItemData->bEnabled && pItemData->pSubMenu && ( nHighlightedItem != ITEMPOS_INVALID ) &&
             ( pItemData->pSubMenu != pActivePopup ) )
        {
            pActivePopup = static_cast<PopupMenu*>(pItemData->pSubMenu);
            long nX = 0;
            MenuItemData* pData = nullptr;
            for ( sal_uLong n = 0; n < nHighlightedItem; n++ )
            {
                pData = pMenu->GetItemList()->GetDataFromPos( n );
                nX += pData->aSz.Width();
            }
            pData = pMenu->pItemList->GetDataFromPos( nHighlightedItem );
            Point aItemTopLeft( nX, 0 );
            Point aItemBottomRight( aItemTopLeft );
            aItemBottomRight.X() += pData->aSz.Width();

            // the menu bar could have height 0 in fullscreen mode:
            // so do not use always WindowHeight, as ItemHeight < WindowHeight.
            if ( GetSizePixel().Height() )
            {
                // #107747# give menuitems the height of the menubar
                aItemBottomRight.Y() += GetOutputSizePixel().Height()-1;
            }

            // ImplExecute is not modal...
            // #99071# do not grab the focus, otherwise it will be restored to the menubar
            // when the frame is reactivated later
            //GrabFocus();
            pActivePopup->ImplExecute( this, Rectangle( aItemTopLeft, aItemBottomRight ), FloatWinPopupFlags::Down | FloatWinPopupFlags::NoHorzPlacement, pMenu, bPreSelectFirst );
            // does not have a window, if aborted before or if there are no entries
            if ( pActivePopup->ImplGetFloatingWindow() )
                pActivePopup->ImplGetFloatingWindow()->AddPopupModeWindow( this );
            else
                pActivePopup = nullptr;
        }
    }
}

void MenuBarWindow::KillActivePopup()
{
    if ( pActivePopup )
    {
        if( pActivePopup->pWindow )
            if( static_cast<FloatingWindow *>(pActivePopup->pWindow.get())->IsInCleanUp() )
                return; // kill it later

        if ( pActivePopup->bInCallback )
            pActivePopup->bCanceled = true;

        pActivePopup->bInCallback = true;
        pActivePopup->Deactivate();
        pActivePopup->bInCallback = false;
        // check for pActivePopup, if stopped by deactivate...
        if ( pActivePopup->ImplGetWindow() )
        {
            pActivePopup->ImplGetFloatingWindow()->StopExecute();
            pActivePopup->ImplGetFloatingWindow()->doShutdown();
            pActivePopup->pWindow->doLazyDelete();
            pActivePopup->pWindow = nullptr;
        }
        pActivePopup = nullptr;
    }
}

void MenuBarWindow::PopupClosed( Menu* pPopup )
{
    if ( pPopup == pActivePopup )
    {
        KillActivePopup();
        ChangeHighlightItem( ITEMPOS_INVALID, false, ImplGetFrameWindow()->ImplGetFrameData()->mbHasFocus, false );
    }
}

void MenuBarWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mbAutoPopup = true;
    SetMBWMenuKey(false);
    sal_uInt16 nEntry = ImplFindEntry( rMEvt.GetPosPixel() );
    if ( ( nEntry != ITEMPOS_INVALID ) && !pActivePopup )
    {
        ChangeHighlightItem( nEntry, false );
    }
    else
    {
        KillActivePopup();
        ChangeHighlightItem( ITEMPOS_INVALID, false );
    }
}

void MenuBarWindow::MouseButtonUp( const MouseEvent& )
{
}

void MenuBarWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( rMEvt.IsSynthetic() || rMEvt.IsEnterWindow() )
        return;

    if ( rMEvt.IsLeaveWindow() )
    {
        if ( nRolloveredItem != ITEMPOS_INVALID && nRolloveredItem != nHighlightedItem )
        {
            // there is a spurious MouseMove generated after a menu is launched from the keyboard, hence this...
            if (nHighlightedItem != ITEMPOS_INVALID)
            {
                bool hide = GetMBWHideAccel();
                SetMBWHideAccel(true);
                Invalidate(); //HighlightItem( nRolloveredItem, false );
                SetMBWHideAccel(hide);
            }
            else
                Invalidate(); //HighlightItem( nRolloveredItem, false );
        }

        nRolloveredItem = ITEMPOS_INVALID;
        return;
    }

    sal_uInt16 nEntry = ImplFindEntry( rMEvt.GetPosPixel() );
    if ( nHighlightedItem == ITEMPOS_INVALID )
    {
        if ( nRolloveredItem != nEntry  )
        {
            if ( nRolloveredItem != ITEMPOS_INVALID )
                Invalidate(); //HighlightItem( nRolloveredItem, false );

            nRolloveredItem = nEntry;
            Invalidate(); //HighlightItem( nRolloveredItem, true );
        }
        return;
    }
    nRolloveredItem = nEntry;

    if( bIgnoreFirstMove )
    {
        bIgnoreFirstMove = false;
        return;
    }

    if ( ( nEntry != ITEMPOS_INVALID )
       && ( nEntry != nHighlightedItem ) )
        ChangeHighlightItem( nEntry, false );
}

void MenuBarWindow::ChangeHighlightItem( sal_uInt16 n, bool bSelectEntry, bool bAllowRestoreFocus, bool bDefaultToDocument)
{
    if( ! pMenu )
        return;

    // always hide accelerators when updating the menu bar...
    SetMBWHideAccel(true);

    // #57934# close active popup if applicable, as TH's background storage works.
    MenuItemData* pNextData = pMenu->pItemList->GetDataFromPos( n );
    if ( pActivePopup && pActivePopup->ImplGetWindow() && ( !pNextData || ( pActivePopup != pNextData->pSubMenu ) ) )
        KillActivePopup(); // pActivePopup when applicable without pWin, if Rescheduled in  Activate()

    // activate menubar only ones per cycle...
    bool bJustActivated = false;
    if ( ( nHighlightedItem == ITEMPOS_INVALID ) && ( n != ITEMPOS_INVALID ) )
    {
        ImplGetSVData()->maWinData.mbNoDeactivate = true;
        if( !bStayActive )
        {
            // #105406# avoid saving the focus when we already have the focus
            bool bNoSaveFocus = (this == ImplGetSVData()->maWinData.mpFocusWin.get() );

            if( xSaveFocusId != nullptr )
            {
                if( !ImplGetSVData()->maWinData.mbNoSaveFocus )
                {
                    xSaveFocusId = nullptr;
                    if( !bNoSaveFocus )
                        xSaveFocusId = Window::SaveFocus(); // only save focus when initially activated
                }
                else {
                    ; // do nothing: we 're activated again from taskpanelist, focus was already saved
                }
            }
            else
            {
                if( !bNoSaveFocus )
                    xSaveFocusId = Window::SaveFocus(); // only save focus when initially activated
            }
        }
        else
            bStayActive = false;
        pMenu->bInCallback = true;  // set here if Activate overridden
        pMenu->Activate();
        pMenu->bInCallback = false;
        bJustActivated = true;
    }
    else if ( ( nHighlightedItem != ITEMPOS_INVALID ) && ( n == ITEMPOS_INVALID ) )
    {
        pMenu->bInCallback = true;
        pMenu->Deactivate();
        pMenu->bInCallback = false;
        ImplGetSVData()->maWinData.mbNoDeactivate = false;
        if( !ImplGetSVData()->maWinData.mbNoSaveFocus )
        {
            VclPtr<vcl::Window> xTempFocusId = xSaveFocusId;
            xSaveFocusId = nullptr;
            if (bAllowRestoreFocus)
                Window::EndSaveFocus(xTempFocusId);
            // #105406# restore focus to document if we could not save focus before
            if (bDefaultToDocument && xTempFocusId == nullptr && bAllowRestoreFocus)
                GrabFocusToDocument();
        }
    }

    if ( nHighlightedItem != ITEMPOS_INVALID )
    {
        if ( nHighlightedItem != nRolloveredItem )
            Invalidate(); //HighlightItem( nHighlightedItem, false );

        pMenu->ImplCallEventListeners( VCLEVENT_MENU_DEHIGHLIGHT, nHighlightedItem );
    }

    nHighlightedItem = (sal_uInt16)n;
    DBG_ASSERT( ( nHighlightedItem == ITEMPOS_INVALID ) || pMenu->ImplIsVisible( nHighlightedItem ), "ChangeHighlightItem: Not visible!" );
    if ( nHighlightedItem != ITEMPOS_INVALID )
        Invalidate(); //HighlightItem( nHighlightedItem, true );
    else if ( nRolloveredItem != ITEMPOS_INVALID )
        Invalidate(); //HighlightItem( nRolloveredItem, true );
    pMenu->ImplCallHighlight(nHighlightedItem);

    if( mbAutoPopup )
        ImplCreatePopup( bSelectEntry );

    // #58935# #73659# Focus, if no popup underneath...
    if ( bJustActivated && !pActivePopup )
        GrabFocus();
}

static int ImplGetTopDockingAreaHeight( vcl::Window *pWindow )
{
    // find docking area that is top aligned and return its height
    // note: dockingareas are direct children of the SystemWindow
    if( pWindow->ImplGetFrameWindow() )
    {
        vcl::Window *pWin = pWindow->ImplGetFrameWindow()->GetWindow( GetWindowType::FirstChild ); //mpWindowImpl->mpFirstChild;
        while( pWin )
        {
            if( pWin->IsSystemWindow() )
            {
                vcl::Window *pChildWin = pWin->GetWindow( GetWindowType::FirstChild ); //mpWindowImpl->mpFirstChild;
                while( pChildWin )
                {
                    DockingAreaWindow *pDockingArea = nullptr;
                    if ( pChildWin->GetType() == WINDOW_DOCKINGAREA )
                        pDockingArea = static_cast< DockingAreaWindow* >( pChildWin );

                    if( pDockingArea && pDockingArea->GetAlign() == WindowAlign::Top &&
                        pDockingArea->IsVisible() && pDockingArea->GetOutputSizePixel().Height() != 0 )
                    {
                        return pDockingArea->GetOutputSizePixel().Height();
                    }

                    pChildWin = pChildWin->GetWindow( GetWindowType::Next ); //mpWindowImpl->mpNext;
                }

            }

            pWin = pWin->GetWindow( GetWindowType::Next ); //mpWindowImpl->mpNext;
        }
    }
    return 0;
}

static void ImplAddNWFSeparator(vcl::RenderContext& rRenderContext, const Size& rSize, const MenubarValue& rMenubarValue)
{
    // add a separator if
    // - we have an adjacent docking area
    // - and if toolbars would draw them as well (mbDockingAreaSeparateTB must not be set, see dockingarea.cxx)
    if (rMenubarValue.maTopDockingAreaHeight
      && !ImplGetSVData()->maNWFData.mbDockingAreaSeparateTB
      && !ImplGetSVData()->maNWFData.mbDockingAreaAvoidTBFrames)
    {
        // note: the menubar only provides the upper (dark) half of it, the rest (bright part) is drawn by the docking area

        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetSeparatorColor());
        Point aPt;
        Rectangle aRect(aPt, rSize);
        rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
    }
}

void MenuBarWindow::HighlightItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos)
{
    if (!pMenu)
        return;

    long nX = 0;
    size_t nCount = pMenu->pItemList->size();
    for (size_t n = 0; n < nCount; n++)
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if (n == nPos)
        {
            if (pData->eType != MenuItemType::SEPARATOR)
            {
                // #107747# give menuitems the height of the menubar
                Rectangle aRect = Rectangle(Point(nX, 1), Size(pData->aSz.Width(), GetOutputSizePixel().Height() - 2));
                rRenderContext.Push(PushFlags::CLIPREGION);
                rRenderContext.IntersectClipRegion(aRect);
                bool bRollover = nPos != nHighlightedItem;
                if (rRenderContext.IsNativeControlSupported(ControlType::Menubar, ControlPart::MenuItem) &&
                    rRenderContext.IsNativeControlSupported(ControlType::Menubar, ControlPart::Entire))
                {
                    // draw background (transparency)
                    MenubarValue aControlValue;
                    aControlValue.maTopDockingAreaHeight = ImplGetTopDockingAreaHeight( this );

                    if (!Application::GetSettings().GetStyleSettings().GetPersonaHeader().IsEmpty() )
                         Erase(rRenderContext);
                    else
                    {
                        Rectangle aBgRegion(Point(), GetOutputSizePixel());
                        rRenderContext.DrawNativeControl(ControlType::Menubar, ControlPart::Entire, aBgRegion,
                                                         ControlState::ENABLED, aControlValue, OUString());
                    }

                    ImplAddNWFSeparator(rRenderContext, GetOutputSizePixel(), aControlValue);

                    // draw selected item
                    ControlState nState = ControlState::ENABLED;
                    if (bRollover)
                        nState |= ControlState::ROLLOVER;
                    else
                        nState |= ControlState::SELECTED;
                    rRenderContext.DrawNativeControl(ControlType::Menubar, ControlPart::MenuItem,
                                                     aRect, nState, aControlValue, OUString() );
                }
                else
                {
                    if (bRollover)
                        rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuBarRolloverColor());
                    else
                        rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuHighlightColor());
                    rRenderContext.SetLineColor();
                    rRenderContext.DrawRect(aRect);
                }
                rRenderContext.Pop();
                pMenu->ImplPaint(rRenderContext, 0, 0, pData, true/*bHighlight*/, false, bRollover);
            }
            return;
        }

        nX += pData->aSz.Width();
    }
}

Rectangle MenuBarWindow::ImplGetItemRect( sal_uInt16 nPos )
{
    Rectangle aRect;
    if( pMenu )
    {
        long nX = 0;
        size_t nCount = pMenu->pItemList->size();
        for ( size_t n = 0; n < nCount; n++ )
        {
            MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
            if ( n == nPos )
            {
                if ( pData->eType != MenuItemType::SEPARATOR )
                    // #107747# give menuitems the height of the menubar
                    aRect = Rectangle( Point( nX, 1 ), Size( pData->aSz.Width(), GetOutputSizePixel().Height()-2 ) );
                break;
            }

            nX += pData->aSz.Width();
        }
    }
    return aRect;
}

void MenuBarWindow::KeyInput( const KeyEvent& rKEvent )
{
    if ( !HandleKeyEvent( rKEvent ) )
        Window::KeyInput( rKEvent );
}

bool MenuBarWindow::HandleKeyEvent( const KeyEvent& rKEvent, bool bFromMenu )
{
    if( ! pMenu )
        return false;

    if ( pMenu->bInCallback )
        return true;    // swallow

    bool bDone = false;
    sal_uInt16 nCode = rKEvent.GetKeyCode().GetCode();

    if( GetParent() )
    {
        if( GetParent()->GetWindow( GetWindowType::Client )->IsSystemWindow() )
        {
            SystemWindow *pSysWin = static_cast<SystemWindow*>(GetParent()->GetWindow( GetWindowType::Client ));
            if( pSysWin->GetTaskPaneList() )
                if( pSysWin->GetTaskPaneList()->HandleKeyEvent( rKEvent ) )
                    return true;
        }
    }

    if ( nCode == KEY_MENU && !rKEvent.GetKeyCode().IsShift() ) // only F10, not Shift-F10
    {
        mbAutoPopup = ImplGetSVData()->maNWFData.mbOpenMenuOnF10;
        if ( nHighlightedItem == ITEMPOS_INVALID )
        {
            ChangeHighlightItem( 0, false );
            GrabFocus();
        }
        else
        {
            ChangeHighlightItem( ITEMPOS_INVALID, false );
            xSaveFocusId = nullptr;
        }
        bDone = true;
    }
    else if ( bFromMenu )
    {
        if ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) ||
            ( nCode == KEY_HOME ) || ( nCode == KEY_END ) )
        {
            sal_uInt16 n = nHighlightedItem;
            if ( n == ITEMPOS_INVALID )
            {
                if ( nCode == KEY_LEFT)
                    n = 0;
                else
                    n = pMenu->GetItemCount()-1;
            }

            // handling gtk like (aka mbOpenMenuOnF10)
            // do not highlight an item when opening a sub menu
            // unless there already was a higlighted sub menu item
            bool bWasHighlight = false;
            if( pActivePopup )
            {
                MenuFloatingWindow* pSubWindow = dynamic_cast<MenuFloatingWindow*>(pActivePopup->ImplGetWindow());
                if( pSubWindow )
                    bWasHighlight = (pSubWindow->GetHighlightedItem() != ITEMPOS_INVALID);
            }

            sal_uInt16 nLoop = n;

            if( nCode == KEY_HOME )
                { n = (sal_uInt16)-1; nLoop = n+1; }
            if( nCode == KEY_END )
                { n = pMenu->GetItemCount(); nLoop = n-1; }

            do
            {
                if ( nCode == KEY_LEFT || nCode == KEY_END )
                {
                    if ( n )
                        n--;
                    else
                        n = pMenu->GetItemCount()-1;
                }
                if ( nCode == KEY_RIGHT || nCode == KEY_HOME )
                {
                    n++;
                    if ( n >= pMenu->GetItemCount() )
                        n = 0;
                }

                MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( n );
                if ( ( pData->eType != MenuItemType::SEPARATOR ) && pMenu->ImplIsVisible( n ) )
                {
                    bool bDoSelect = true;
                    if( ImplGetSVData()->maNWFData.mbOpenMenuOnF10 )
                        bDoSelect = bWasHighlight;
                    ChangeHighlightItem( n, bDoSelect );
                    break;
                }
            } while ( n != nLoop );
            bDone = true;
        }
        else if ( nCode == KEY_RETURN )
        {
            if( pActivePopup ) KillActivePopup();
            else
                if ( !mbAutoPopup )
                {
                    ImplCreatePopup( true );
                    mbAutoPopup = true;
                }
            bDone = true;
        }
        else if ( ( nCode == KEY_UP ) || ( nCode == KEY_DOWN ) )
        {
            if ( !mbAutoPopup )
            {
                ImplCreatePopup( true );
                mbAutoPopup = true;
            }
            bDone = true;
        }
        else if ( nCode == KEY_ESCAPE || ( nCode == KEY_F6 && rKEvent.GetKeyCode().IsMod1() ) )
        {
            if( pActivePopup )
            {
                // hide the menu and remove the focus...
                mbAutoPopup = false;
                KillActivePopup();
            }

            ChangeHighlightItem( ITEMPOS_INVALID, false );

            if( nCode == KEY_F6 && rKEvent.GetKeyCode().IsMod1() )
            {
                // put focus into document
                GrabFocusToDocument();
            }

            bDone = true;
        }
    }

    bool accel = ImplGetSVData()->maNWFData.mbEnableAccel;
    if ( !bDone && ( bFromMenu || (rKEvent.GetKeyCode().IsMod2() && accel) ) )
    {
        sal_Unicode nCharCode = rKEvent.GetCharCode();
        if ( nCharCode )
        {
            sal_uInt16 nEntry, nDuplicates;
            MenuItemData* pData = pMenu->GetItemList()->SearchItem( nCharCode, rKEvent.GetKeyCode(), nEntry, nDuplicates, nHighlightedItem );
            if ( pData && (nEntry != ITEMPOS_INVALID) )
            {
                mbAutoPopup = true;
                SetMBWMenuKey(true);
                SetMBWHideAccel(true);
                Invalidate(InvalidateFlags::Update);
                ChangeHighlightItem( nEntry, true );
                bDone = true;
            }
        }
    }
    return bDone;
}

void MenuBarWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle&)
{
    if (!pMenu)
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    // no VCL paint if native menus
    if (pMenu->ImplGetSalMenu() && pMenu->ImplGetSalMenu()->VisibleMenuBar())
    {
        ImplGetFrame()->DrawMenuBar();
        return;
    }

    if (rRenderContext.IsNativeControlSupported(ControlType::Menubar, ControlPart::Entire))
    {
        MenubarValue aMenubarValue;
        aMenubarValue.maTopDockingAreaHeight = ImplGetTopDockingAreaHeight(this);

        if (!rStyleSettings.GetPersonaHeader().IsEmpty())
            Erase(rRenderContext);
        else
        {
            Point aPt;
            Rectangle aCtrlRegion( aPt, GetOutputSizePixel() );

            rRenderContext.DrawNativeControl(ControlType::Menubar, ControlPart::Entire, aCtrlRegion,
                                             ControlState::ENABLED, aMenubarValue, OUString());
        }

        ImplAddNWFSeparator(rRenderContext, GetOutputSizePixel(), aMenubarValue);
    }
    rRenderContext.SetFillColor(rStyleSettings.GetMenuColor());

    pMenu->ImplPaint(rRenderContext, 0);
    if (nHighlightedItem != ITEMPOS_INVALID)
        HighlightItem(rRenderContext, nHighlightedItem);

    // in high contrast mode draw a separating line on the lower edge
    if (!rRenderContext.IsNativeControlSupported( ControlType::Menubar, ControlPart::Entire) &&
        rStyleSettings.GetHighContrastMode())
    {
        rRenderContext.Push(PushFlags::LINECOLOR | PushFlags::MAPMODE);
        rRenderContext.SetLineColor(Color(COL_WHITE));
        rRenderContext.SetMapMode(MapMode(MAP_PIXEL));
        Size aSize = GetSizePixel();
        rRenderContext.DrawLine(Point(0, aSize.Height() - 1),
                                Point(aSize.Width() - 1, aSize.Height() - 1));
        rRenderContext.Pop();
    }
}

void MenuBarWindow::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    long n      = aOutSz.Height()-4;
    long nX     = aOutSz.Width()-3;
    long nY     = 2;

    if ( aCloseBtn->IsVisible() )
    {
        aCloseBtn->Hide();
        aCloseBtn->SetImages(n);
        Size aTbxSize( aCloseBtn->CalcWindowSizePixel() );
        nX -= aTbxSize.Width();
        long nTbxY = (aOutSz.Height() - aTbxSize.Height())/2;
        aCloseBtn->setPosSizePixel(nX, nTbxY, aTbxSize.Width(), aTbxSize.Height());
        nX -= 3;
        aCloseBtn->Show();
    }
    if ( aFloatBtn->IsVisible() )
    {
        nX -= n;
        aFloatBtn->setPosSizePixel( nX, nY, n, n );
    }
    if ( aHideBtn->IsVisible() )
    {
        nX -= n;
        aHideBtn->setPosSizePixel( nX, nY, n, n );
    }

    aFloatBtn->SetSymbol( SymbolType::FLOAT );
    aHideBtn->SetSymbol( SymbolType::HIDE );

    Invalidate();
}

sal_uInt16 MenuBarWindow::ImplFindEntry( const Point& rMousePos ) const
{
    if( pMenu )
    {
        long nX = 0;
        size_t nCount = pMenu->pItemList->size();
        for ( size_t n = 0; n < nCount; n++ )
        {
            MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
            if ( pMenu->ImplIsVisible( n ) )
            {
                nX += pData->aSz.Width();
                if ( nX > rMousePos.X() )
                    return (sal_uInt16)n;
            }
        }
    }
    return ITEMPOS_INVALID;
}

void MenuBarWindow::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nId = nHighlightedItem;
    if ( rHEvt.GetMode() & (HelpEventMode::CONTEXT | HelpEventMode::EXTENDED) )
        ChangeHighlightItem( ITEMPOS_INVALID, true );

    Rectangle aHighlightRect( ImplGetItemRect( nHighlightedItem ) );
    if( !ImplHandleHelpEvent( this, pMenu, nId, rHEvt, aHighlightRect ) )
        Window::RequestHelp( rHEvt );
}

void MenuBarWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if (nType == StateChangedType::ControlForeground ||
        nType == StateChangedType::ControlBackground)
    {
        ApplySettings(*this);
        Invalidate();
    }
    else if (nType == StateChangedType::Enable)
    {
        Invalidate();
    }
    else if(pMenu)
    {
        pMenu->ImplKillLayoutData();
    }
}

void MenuBarWindow::LayoutChanged()
{
    if (!pMenu)
        return;

    ApplySettings(*this);

    // if the font was changed.
    long nHeight = pMenu->ImplCalcSize(this).Height();

    // depending on the native implementation or the displayable flag
    // the menubar windows is suppressed (ie, height=0)
    if (!static_cast<MenuBar*>(pMenu)->IsDisplayable() ||
        (pMenu->ImplGetSalMenu() && pMenu->ImplGetSalMenu()->VisibleMenuBar()))
    {
        nHeight = 0;
    }
    setPosSizePixel(0, 0, 0, nHeight, PosSizeFlags::Height);
    GetParent()->Resize();
    Invalidate();
    Resize();

    pMenu->ImplKillLayoutData();
}

void MenuBarWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Window::ApplySettings(rRenderContext);
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    SetPointFont(rRenderContext, rStyleSettings.GetMenuFont());

    const BitmapEx& rPersonaBitmap = Application::GetSettings().GetStyleSettings().GetPersonaHeader();
    if (!rPersonaBitmap.IsEmpty())
    {
        Wallpaper aWallpaper(rPersonaBitmap);
        aWallpaper.SetStyle(WallpaperStyle::TopRight);
        aWallpaper.SetColor(Application::GetSettings().GetStyleSettings().GetWorkspaceColor());

        rRenderContext.SetBackground(aWallpaper);
        SetPaintTransparent(false);
        SetParentClipMode();
    }
    else if (rRenderContext.IsNativeControlSupported(ControlType::Menubar, ControlPart::Entire))
    {
        rRenderContext.SetBackground(); // background will be drawn by NWF
    }
    else
    {
        Wallpaper aWallpaper;
        aWallpaper.SetStyle(WallpaperStyle::ApplicationGradient);
        rRenderContext.SetBackground(aWallpaper);
        SetPaintTransparent(false);
        SetParentClipMode();
    }

    rRenderContext.SetTextColor(rStyleSettings.GetMenuBarTextColor());
    rRenderContext.SetTextFillColor();
    rRenderContext.SetLineColor();
}

void MenuBarWindow::ImplInitStyleSettings()
{
    if (IsNativeControlSupported(ControlType::Menubar, ControlPart::MenuItem) &&
        IsNativeControlSupported(ControlType::Menubar, ControlPart::Entire))
    {
        AllSettings aSettings(GetSettings());
        ImplGetFrame()->UpdateSettings(aSettings); // to update persona
        StyleSettings aStyle(aSettings.GetStyleSettings());
        Color aHighlightTextColor = ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor;
        if (aHighlightTextColor != Color(COL_TRANSPARENT))
        {
            aStyle.SetMenuHighlightTextColor(aHighlightTextColor);
        }
        aSettings.SetStyleSettings(aStyle);
        OutputDevice::SetSettings(aSettings);
    }
}

void MenuBarWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ApplySettings(*this);
        ImplInitStyleSettings();
        LayoutChanged();
    }
}

void MenuBarWindow::LoseFocus()
{
    if ( !HasChildPathFocus( true ) )
        ChangeHighlightItem( ITEMPOS_INVALID, false, false );
}

void MenuBarWindow::GetFocus()
{
    SalMenu *pNativeMenu = pMenu ? pMenu->ImplGetSalMenu() : nullptr;
    if (pNativeMenu && pNativeMenu->TakeFocus())
        return;

    if ( nHighlightedItem == ITEMPOS_INVALID )
    {
        mbAutoPopup = false;    // do not open menu when activated by focus handling like taskpane cycling
        ChangeHighlightItem( 0, false );
    }
}

css::uno::Reference<css::accessibility::XAccessible> MenuBarWindow::CreateAccessible()
{
    css::uno::Reference<css::accessibility::XAccessible> xAcc;

    if (pMenu)
        xAcc = pMenu->GetAccessible();

    return xAcc;
}

sal_uInt16 MenuBarWindow::AddMenuBarButton( const Image& i_rImage, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>& i_rLink, const OUString& i_rToolTip )
{
    // find first free button id
    sal_uInt16 nId = IID_DOCUMENTCLOSE;
    std::map< sal_uInt16, AddButtonEntry >::const_iterator it;
    do
    {
        nId++;
        it = m_aAddButtons.find( nId );
    } while( it != m_aAddButtons.end() && nId < 128 );
    DBG_ASSERT( nId < 128, "too many addbuttons in menubar" );
    AddButtonEntry& rNewEntry = m_aAddButtons[nId];
    rNewEntry.m_nId = nId;
    rNewEntry.m_aSelectLink = i_rLink;
    aCloseBtn->InsertItem(nId, i_rImage, ToolBoxItemBits::NONE, 0);
    aCloseBtn->calcMinSize();
    ShowButtons(aCloseBtn->IsItemVisible(IID_DOCUMENTCLOSE), aFloatBtn->IsVisible(), aHideBtn->IsVisible());
    LayoutChanged();

    if( pMenu->mpSalMenu )
        pMenu->mpSalMenu->AddMenuBarButton( SalMenuButtonItem( nId, i_rImage, i_rToolTip ) );

    return nId;
}

void MenuBarWindow::SetMenuBarButtonHighlightHdl( sal_uInt16 nId, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>& rLink )
{
    std::map< sal_uInt16, AddButtonEntry >::iterator it = m_aAddButtons.find( nId );
    if( it != m_aAddButtons.end() )
        it->second.m_aHighlightLink = rLink;
}

Rectangle MenuBarWindow::GetMenuBarButtonRectPixel( sal_uInt16 nId )
{
    Rectangle aRect;
    if( m_aAddButtons.find( nId ) != m_aAddButtons.end() )
    {
        if( pMenu->mpSalMenu )
        {
            aRect = pMenu->mpSalMenu->GetMenuBarButtonRectPixel( nId, ImplGetWindowImpl()->mpFrame );
            if( aRect == Rectangle( Point( -1, -1 ), Size( 1, 1 ) ) )
            {
                // system menu button is somewhere but location cannot be determined
                return Rectangle();
            }
        }

        if( aRect.IsEmpty() )
        {
            aRect = aCloseBtn->GetItemRect(nId);
            Point aOffset = aCloseBtn->OutputToScreenPixel(Point());
            aRect.Move( aOffset.X(), aOffset.Y() );
        }
    }
    return aRect;
}

void MenuBarWindow::RemoveMenuBarButton( sal_uInt16 nId )
{
    sal_uInt16 nPos = aCloseBtn->GetItemPos(nId);
    aCloseBtn->RemoveItem(nPos);
    m_aAddButtons.erase( nId );
    aCloseBtn->calcMinSize();
    LayoutChanged();

    if( pMenu->mpSalMenu )
        pMenu->mpSalMenu->RemoveMenuBarButton( nId );
}

bool MenuBarWindow::HandleMenuButtonEvent( sal_uInt16 i_nButtonId )
{
    std::map< sal_uInt16, AddButtonEntry >::iterator it = m_aAddButtons.find( i_nButtonId );
    if( it != m_aAddButtons.end() )
    {
        MenuBar::MenuBarButtonCallbackArg aArg;
        aArg.nId = it->first;
        aArg.bHighlight = true;
        aArg.pMenuBar = dynamic_cast<MenuBar*>(pMenu);
        return it->second.m_aSelectLink.Call( aArg );
    }
    return false;
}

bool MenuBarWindow::CanGetFocus() const
{
    /* #i83908# do not use the menubar if it is native or invisible
       this relies on MenuBar::ImplCreate setting the height of the menubar
       to 0 in this case
    */
    SalMenu *pNativeMenu = pMenu ? pMenu->ImplGetSalMenu() : nullptr;
    if (pNativeMenu && pNativeMenu->VisibleMenuBar())
        return pNativeMenu->CanGetFocus();
    return GetSizePixel().Height() > 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
