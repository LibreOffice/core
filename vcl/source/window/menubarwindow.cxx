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
#include <sal/log.hxx>

#include <salframe.hxx>
#include <salmenu.hxx>
#include <svdata.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <window.h>
#include "bufferdevice.hxx"
#include <menubarvalue.hxx>

// document closing button
#define IID_DOCUMENTCLOSE 1

DecoToolBox::DecoToolBox( vcl::Window* pParent ) :
    ToolBox( pParent, 0 ),
    lastSize(-1)
{
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
        aTbx->InsertItem(ToolBoxItemId(IID_DOCUMENTCLOSE), Image(StockImage::Yes, SV_RESID_BITMAP_CLOSEDOC));
    }
    else
    {
        ImplToolItems::size_type nItems = GetItemCount();
        for( ImplToolItems::size_type i = 0; i < nItems; i++ )
        {
            ToolBoxItemId nId = GetItemId( i );
            aTbx->InsertItem( nId, GetItemImage( nId ) );
        }
    }
    maMinSize = aTbx->CalcWindowSizePixel();

    aTbx.disposeAndClear();
}

void DecoToolBox::SetImages( tools::Long nMaxHeight, bool bForce )
{
    tools::Long border = getMinSize().Height() - maImage.GetSizePixel().Height();

    if( !nMaxHeight && lastSize != -1 )
        nMaxHeight = lastSize + border; // don't change anything if called with 0

    if( nMaxHeight < getMinSize().Height() )
        nMaxHeight = getMinSize().Height();

    if( (lastSize == nMaxHeight - border) && !bForce )
        return;

    lastSize = nMaxHeight - border;

    Color       aEraseColor( ColorTransparency, 255, 255, 255, 255 );
    BitmapEx    aBmpExDst( maImage.GetBitmapEx() );
    BitmapEx    aBmpExSrc( aBmpExDst );

    aEraseColor.SetAlpha( 0 );
    aBmpExDst.Erase( aEraseColor );
    aBmpExDst.Scale( Size( lastSize, lastSize ) );

    tools::Rectangle aSrcRect( Point(0,0), maImage.GetSizePixel() );
    tools::Rectangle aDestRect( Point((lastSize - maImage.GetSizePixel().Width())/2,
                            (lastSize - maImage.GetSizePixel().Height())/2 ),
                        maImage.GetSizePixel() );

    aBmpExDst.CopyPixel( aDestRect, aSrcRect, aBmpExSrc );
    SetItemImage( ToolBoxItemId(IID_DOCUMENTCLOSE), Image( aBmpExDst ) );

}

MenuBarWindow::MenuBarWindow( vcl::Window* pParent ) :
    Window( pParent, 0 ),
    m_aCloseBtn(VclPtr<DecoToolBox>::Create(this)),
    m_aFloatBtn(VclPtr<PushButton>::Create(this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE)),
    m_aHideBtn(VclPtr<PushButton>::Create(this, WB_NOPOINTERFOCUS | WB_SMALLSTYLE | WB_RECTSTYLE))
{
    SetType(WindowType::MENUBARWINDOW);
    m_pMenu = nullptr;
    m_pActivePopup = nullptr;
    m_nHighlightedItem = ITEMPOS_INVALID;
    m_nRolloveredItem = ITEMPOS_INVALID;
    mbAutoPopup = true;
    m_bIgnoreFirstMove = true;
    SetMBWHideAccel(ImplGetSVData()->maNWFData.mbAutoAccel);
    SetMBWMenuKey(false);

    m_aCloseBtn->maImage = Image(StockImage::Yes, SV_RESID_BITMAP_CLOSEDOC);

    m_aCloseBtn->SetBackground();
    m_aCloseBtn->SetPaintTransparent(true);
    m_aCloseBtn->SetParentClipMode(ParentClipMode::NoClip);

    m_aCloseBtn->InsertItem(ToolBoxItemId(IID_DOCUMENTCLOSE), m_aCloseBtn->maImage);
    m_aCloseBtn->SetSelectHdl(LINK(this, MenuBarWindow, CloseHdl));
    m_aCloseBtn->AddEventListener(LINK(this, MenuBarWindow, ToolboxEventHdl));
    m_aCloseBtn->SetQuickHelpText(ToolBoxItemId(IID_DOCUMENTCLOSE), VclResId(SV_HELPTEXT_CLOSEDOCUMENT));

    m_aFloatBtn->SetSymbol( SymbolType::FLOAT );
    m_aFloatBtn->SetQuickHelpText(VclResId(SV_HELPTEXT_RESTORE));

    m_aHideBtn->SetSymbol( SymbolType::HIDE );
    m_aHideBtn->SetQuickHelpText(VclResId(SV_HELPTEXT_MINIMIZE));

    ImplInitStyleSettings();

    AddEventListener(LINK(this, MenuBarWindow, ShowHideListener));
}

MenuBarWindow::~MenuBarWindow()
{
    disposeOnce();
}

void MenuBarWindow::dispose()
{
    m_aCloseBtn->RemoveEventListener(LINK(this, MenuBarWindow, ToolboxEventHdl));
    RemoveEventListener(LINK(this, MenuBarWindow, ShowHideListener));

    mpParentPopup.disposeAndClear();
    m_aHideBtn.disposeAndClear();
    m_aFloatBtn.disposeAndClear();
    m_aCloseBtn.disposeAndClear();
    m_pMenu.clear();
    m_pActivePopup.clear();
    m_xSaveFocusId.clear();

    Window::dispose();
}

void MenuBarWindow::SetMenu( MenuBar* pMen )
{
    m_pMenu = pMen;
    KillActivePopup();
    m_nHighlightedItem = ITEMPOS_INVALID;
    if (pMen)
    {
        m_aCloseBtn->ShowItem(ToolBoxItemId(IID_DOCUMENTCLOSE), pMen->HasCloseButton());
        m_aCloseBtn->Show(pMen->HasCloseButton() || !m_aAddButtons.empty());
        m_aFloatBtn->Show(pMen->HasFloatButton());
        m_aHideBtn->Show(pMen->HasHideButton());
    }
    Invalidate();

    // show and connect native menubar
    if( m_pMenu && m_pMenu->ImplGetSalMenu() )
    {
        if( m_pMenu->ImplGetSalMenu()->VisibleMenuBar() )
            ImplGetFrame()->SetMenu( m_pMenu->ImplGetSalMenu() );

        m_pMenu->ImplGetSalMenu()->SetFrame( ImplGetFrame() );
        m_pMenu->ImplGetSalMenu()->ShowMenuBar(true);
    }
}

void MenuBarWindow::SetHeight(tools::Long nHeight)
{
    setPosSizePixel(0, 0, 0, nHeight, PosSizeFlags::Height);
}

void MenuBarWindow::ShowButtons( bool bClose, bool bFloat, bool bHide )
{
    m_aCloseBtn->ShowItem(ToolBoxItemId(IID_DOCUMENTCLOSE), bClose);
    m_aCloseBtn->Show(bClose || !m_aAddButtons.empty());
    if (m_pMenu->mpSalMenu)
        m_pMenu->mpSalMenu->ShowCloseButton(bClose);
    m_aFloatBtn->Show( bFloat );
    m_aHideBtn->Show( bHide );
    Resize();
}

Size const & MenuBarWindow::MinCloseButtonSize() const
{
    return m_aCloseBtn->getMinSize();
}

IMPL_LINK_NOARG(MenuBarWindow, CloseHdl, ToolBox *, void)
{
    if( ! m_pMenu )
        return;

    if( m_aCloseBtn->GetCurItemId() == ToolBoxItemId(IID_DOCUMENTCLOSE) )
    {
        // #i106052# call close hdl asynchronously to ease handler implementation
        // this avoids still being in the handler while the DecoToolBox already
        // gets destroyed
        Application::PostUserEvent(m_pMenu->GetCloseButtonClickHdl());
    }
    else
    {
        std::map<sal_uInt16,AddButtonEntry>::iterator it = m_aAddButtons.find(sal_uInt16(m_aCloseBtn->GetCurItemId()));
        if( it != m_aAddButtons.end() )
        {
            MenuBarButtonCallbackArg aArg;
            aArg.nId = it->first;
            aArg.bHighlight = (sal_uInt16(m_aCloseBtn->GetHighlightItemId()) == it->first);
            it->second.m_aSelectLink.Call( aArg );
        }
    }
}

IMPL_LINK( MenuBarWindow, ToolboxEventHdl, VclWindowEvent&, rEvent, void )
{
    if( ! m_pMenu )
        return;

    MenuBarButtonCallbackArg aArg;
    aArg.nId = 0xffff;
    aArg.bHighlight = (rEvent.GetId() == VclEventId::ToolboxHighlight);
    if( rEvent.GetId() == VclEventId::ToolboxHighlight )
        aArg.nId =sal_uInt16(m_aCloseBtn->GetHighlightItemId());
    else if( rEvent.GetId() == VclEventId::ToolboxHighlightOff )
    {
        auto nPos = static_cast<ToolBox::ImplToolItems::size_type>(reinterpret_cast<sal_IntPtr>(rEvent.GetData()));
        aArg.nId = sal_uInt16(m_aCloseBtn->GetItemId(nPos));
    }
    std::map< sal_uInt16, AddButtonEntry >::iterator it = m_aAddButtons.find( aArg.nId );
    if( it != m_aAddButtons.end() )
    {
        it->second.m_aHighlightLink.Call( aArg );
    }
}

IMPL_LINK( MenuBarWindow, ShowHideListener, VclWindowEvent&, rEvent, void )
{
    if( ! m_pMenu )
        return;

    if( rEvent.GetId() == VclEventId::WindowShow )
        m_pMenu->ImplCallEventListeners( VclEventId::MenuShow, ITEMPOS_INVALID );
    else if( rEvent.GetId() == VclEventId::WindowHide )
        m_pMenu->ImplCallEventListeners( VclEventId::MenuHide, ITEMPOS_INVALID );
}

void MenuBarWindow::ImplCreatePopup( bool bPreSelectFirst )
{
    MenuItemData* pItemData = m_pMenu ? m_pMenu->GetItemList()->GetDataFromPos( m_nHighlightedItem ) : nullptr;
    if ( !pItemData )
        return;

    m_bIgnoreFirstMove = true;
    if ( m_pActivePopup && ( m_pActivePopup != pItemData->pSubMenu ) )
    {
        KillActivePopup();
    }
    if ( !(pItemData->bEnabled && pItemData->pSubMenu && ( m_nHighlightedItem != ITEMPOS_INVALID ) &&
         ( pItemData->pSubMenu != m_pActivePopup )) )
        return;

    m_pActivePopup = pItemData->pSubMenu.get();
    tools::Long nX = 0;
    MenuItemData* pData = nullptr;
    for ( sal_uLong n = 0; n < m_nHighlightedItem; n++ )
    {
        pData = m_pMenu->GetItemList()->GetDataFromPos( n );
        nX += pData->aSz.Width();
    }
    pData = m_pMenu->pItemList->GetDataFromPos( m_nHighlightedItem );
    Point aItemTopLeft( nX, 0 );
    Point aItemBottomRight( aItemTopLeft );
    aItemBottomRight.AdjustX(pData->aSz.Width() );

    if (pData->bHiddenOnGUI)
    {
        mpParentPopup.disposeAndClear();
        mpParentPopup = VclPtr<PopupMenu>::Create();
        m_pActivePopup = mpParentPopup.get();

        for (sal_uInt16 i = m_nHighlightedItem; i < m_pMenu->GetItemCount(); ++i)
        {
            sal_uInt16 nId = m_pMenu->GetItemId(i);

            MenuItemData* pParentItemData = m_pMenu->GetItemList()->GetData(nId);
            assert(pParentItemData);
            mpParentPopup->InsertItem(nId, pParentItemData->aText, pParentItemData->nBits, pParentItemData->sIdent);
            mpParentPopup->SetHelpId(nId, pParentItemData->aHelpId);
            mpParentPopup->SetHelpText(nId, pParentItemData->aHelpText);
            mpParentPopup->SetAccelKey(nId, pParentItemData->aAccelKey);
            mpParentPopup->SetItemCommand(nId, pParentItemData->aCommandStr);
            mpParentPopup->SetHelpCommand(nId, pParentItemData->aHelpCommandStr);

            PopupMenu* pPopup = m_pMenu->GetPopupMenu(nId);
            mpParentPopup->SetPopupMenu(nId, pPopup);
        }
    }
    // the menu bar could have height 0 in fullscreen mode:
    // so do not use always WindowHeight, as ItemHeight < WindowHeight.
    if ( GetSizePixel().Height() )
    {
        // #107747# give menuitems the height of the menubar
        aItemBottomRight.AdjustY(GetOutputSizePixel().Height()-1 );
    }

    // ImplExecute is not modal...
    // #99071# do not grab the focus, otherwise it will be restored to the menubar
    // when the frame is reactivated later
    //GrabFocus();
    m_pActivePopup->ImplExecute( this, tools::Rectangle( aItemTopLeft, aItemBottomRight ), FloatWinPopupFlags::Down | FloatWinPopupFlags::NoHorzPlacement, m_pMenu, bPreSelectFirst );
    // does not have a window, if aborted before or if there are no entries
    if ( m_pActivePopup->ImplGetFloatingWindow() )
        m_pActivePopup->ImplGetFloatingWindow()->AddPopupModeWindow( this );
    else
        m_pActivePopup = nullptr;
}

void MenuBarWindow::KillActivePopup()
{
    if ( !m_pActivePopup )
        return;

    if( m_pActivePopup->pWindow )
        if( static_cast<FloatingWindow *>(m_pActivePopup->pWindow.get())->IsInCleanUp() )
            return; // kill it later

    if ( m_pActivePopup->bInCallback )
        m_pActivePopup->bCanceled = true;

    m_pActivePopup->bInCallback = true;
    m_pActivePopup->Deactivate();
    m_pActivePopup->bInCallback = false;
    // check for pActivePopup, if stopped by deactivate...
    if ( m_pActivePopup->ImplGetWindow() )
    {
        if (mpParentPopup)
        {
            for (sal_uInt16 i = 0; i < mpParentPopup->GetItemCount(); ++i)
            {
                sal_uInt16 nId = mpParentPopup->GetItemId(i);
                MenuItemData* pParentItemData = mpParentPopup->GetItemList()->GetData(nId);
                assert(pParentItemData);
                pParentItemData->pSubMenu = nullptr;
            }
        }
        m_pActivePopup->ImplGetFloatingWindow()->StopExecute();
        m_pActivePopup->ImplGetFloatingWindow()->doShutdown();
        m_pActivePopup->pWindow.disposeAndClear();
    }
    m_pActivePopup = nullptr;
}

void MenuBarWindow::PopupClosed( Menu const * pPopup )
{
    if ( pPopup == m_pActivePopup )
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
    if ( ( nEntry != ITEMPOS_INVALID ) && !m_pActivePopup )
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
        if ( m_nRolloveredItem != ITEMPOS_INVALID && m_nRolloveredItem != m_nHighlightedItem )
        {
            // there is a spurious MouseMove generated after a menu is launched from the keyboard, hence this...
            if (m_nHighlightedItem != ITEMPOS_INVALID)
            {
                bool hide = GetMBWHideAccel();
                SetMBWHideAccel(ImplGetSVData()->maNWFData.mbAutoAccel);
                Invalidate(); //HighlightItem( nRolloveredItem, false );
                SetMBWHideAccel(hide);
            }
            else
                Invalidate(); //HighlightItem( nRolloveredItem, false );
        }

        m_nRolloveredItem = ITEMPOS_INVALID;
        return;
    }

    sal_uInt16 nEntry = ImplFindEntry( rMEvt.GetPosPixel() );
    if ( m_nHighlightedItem == ITEMPOS_INVALID )
    {
        if ( m_nRolloveredItem != nEntry  )
        {
            if ( m_nRolloveredItem != ITEMPOS_INVALID )
                Invalidate(); //HighlightItem( nRolloveredItem, false );

            m_nRolloveredItem = nEntry;
            Invalidate(); //HighlightItem( nRolloveredItem, true );
        }
        return;
    }
    m_nRolloveredItem = nEntry;

    if( m_bIgnoreFirstMove )
    {
        m_bIgnoreFirstMove = false;
        return;
    }

    if ( ( nEntry != ITEMPOS_INVALID )
       && ( nEntry != m_nHighlightedItem ) )
        ChangeHighlightItem( nEntry, false );
}

void MenuBarWindow::ChangeHighlightItem( sal_uInt16 n, bool bSelectEntry, bool bAllowRestoreFocus, bool bDefaultToDocument)
{
    if( ! m_pMenu )
        return;

    if (n == ITEMPOS_INVALID)
        SetMBWHideAccel(ImplGetSVData()->maNWFData.mbAutoAccel);

    // #57934# close active popup if applicable, as TH's background storage works.
    MenuItemData* pNextData = m_pMenu->pItemList->GetDataFromPos( n );
    if ( m_pActivePopup && m_pActivePopup->ImplGetWindow() && ( !pNextData || ( m_pActivePopup != pNextData->pSubMenu ) ) )
        KillActivePopup(); // pActivePopup when applicable without pWin, if Rescheduled in  Activate()

    // activate menubar only ones per cycle...
    bool bJustActivated = false;
    if ( ( m_nHighlightedItem == ITEMPOS_INVALID ) && ( n != ITEMPOS_INVALID ) )
    {
        ImplGetSVData()->mpWinData->mbNoDeactivate = true;
        // #105406# avoid saving the focus when we already have the focus
        bool bNoSaveFocus = (this == ImplGetSVData()->mpWinData->mpFocusWin.get());

        if( m_xSaveFocusId != nullptr )
        {
            if (!ImplGetSVData()->mpWinData->mbNoSaveFocus)
            {
                 m_xSaveFocusId = nullptr;
                 if( !bNoSaveFocus )
                    m_xSaveFocusId = Window::SaveFocus(); // only save focus when initially activated
            }
            else {
                ; // do nothing: we 're activated again from taskpanelist, focus was already saved
            }
        }
        else
        {
            if( !bNoSaveFocus )
                m_xSaveFocusId = Window::SaveFocus(); // only save focus when initially activated
        }
        m_pMenu->bInCallback = true;  // set here if Activate overridden
        m_pMenu->Activate();
        m_pMenu->bInCallback = false;
        bJustActivated = true;
    }
    else if ( ( m_nHighlightedItem != ITEMPOS_INVALID ) && ( n == ITEMPOS_INVALID ) )
    {
        m_pMenu->bInCallback = true;
        m_pMenu->Deactivate();
        m_pMenu->bInCallback = false;
        ImplGetSVData()->mpWinData->mbNoDeactivate = false;
        if (!ImplGetSVData()->mpWinData->mbNoSaveFocus)
        {
            VclPtr<vcl::Window> xTempFocusId;
            if (m_xSaveFocusId && !m_xSaveFocusId->isDisposed())
                xTempFocusId = m_xSaveFocusId;
            m_xSaveFocusId = nullptr;

            if (bAllowRestoreFocus)
            {
                // tdf#115227 the popup is already killed, so temporarily set us as the
                // focus window, so we could avoid sending superfluous activate events
                // to top window listeners.
                if (xTempFocusId || bDefaultToDocument)
                    ImplGetSVData()->mpWinData->mpFocusWin = this;

                // #105406# restore focus to document if we could not save focus before
                if (!xTempFocusId && bDefaultToDocument)
                    GrabFocusToDocument();
                else
                    Window::EndSaveFocus(xTempFocusId);
            }
        }
    }

    if ( m_nHighlightedItem != ITEMPOS_INVALID )
    {
        if ( m_nHighlightedItem != m_nRolloveredItem )
            Invalidate(); //HighlightItem( nHighlightedItem, false );

        m_pMenu->ImplCallEventListeners( VclEventId::MenuDehighlight, m_nHighlightedItem );
    }

    m_nHighlightedItem = n;
    SAL_WARN_IF( ( m_nHighlightedItem != ITEMPOS_INVALID ) && !m_pMenu->ImplIsVisible( m_nHighlightedItem ), "vcl", "ChangeHighlightItem: Not visible!" );
    if ( m_nHighlightedItem != ITEMPOS_INVALID )
        Invalidate(); //HighlightItem( nHighlightedItem, true );
    else if ( m_nRolloveredItem != ITEMPOS_INVALID )
        Invalidate(); //HighlightItem( nRolloveredItem, true );
    m_pMenu->ImplCallHighlight(m_nHighlightedItem);

    if( mbAutoPopup )
        ImplCreatePopup( bSelectEntry );

    // #58935# #73659# Focus, if no popup underneath...
    if ( bJustActivated && !m_pActivePopup )
        GrabFocus();
}

static int ImplGetTopDockingAreaHeight( vcl::Window const *pWindow )
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
                    if ( pChildWin->GetType() == WindowType::DOCKINGAREA )
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
        tools::Rectangle aRect(Point(), rSize);
        rRenderContext.DrawLine(aRect.BottomLeft(), aRect.BottomRight());
    }
}

void MenuBarWindow::HighlightItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos)
{
    if (!m_pMenu)
        return;

    tools::Long nX = 0;
    size_t nCount = m_pMenu->pItemList->size();

    Size aOutputSize = GetOutputSizePixel();
    aOutputSize.AdjustWidth( -(m_aCloseBtn->GetSizePixel().Width()) );

    for (size_t n = 0; n < nCount; n++)
    {
        MenuItemData* pData = m_pMenu->pItemList->GetDataFromPos( n );
        if (n == nPos)
        {
            if (pData->eType != MenuItemType::SEPARATOR)
            {
                // #107747# give menuitems the height of the menubar
                tools::Rectangle aRect(Point(nX, 1), Size(pData->aSz.Width(), aOutputSize.Height() - 2));
                rRenderContext.Push(vcl::PushFlags::CLIPREGION);
                rRenderContext.IntersectClipRegion(aRect);
                bool bRollover, bHighlight;
                if (!ImplGetSVData()->maNWFData.mbRolloverMenubar)
                {
                    bHighlight = true;
                    bRollover = nPos != m_nHighlightedItem;
                }
                else
                {
                    bRollover = nPos == m_nRolloveredItem;
                    bHighlight = nPos == m_nHighlightedItem;
                }
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
                        tools::Rectangle aBgRegion(Point(), aOutputSize);
                        rRenderContext.DrawNativeControl(ControlType::Menubar, ControlPart::Entire, aBgRegion,
                                                         ControlState::ENABLED, aControlValue, OUString());
                    }

                    ImplAddNWFSeparator(rRenderContext, aOutputSize, aControlValue);

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

                m_pMenu->ImplPaint(rRenderContext, aOutputSize, 0, 0, pData, bHighlight, false, bRollover);
            }
            return;
        }

        nX += pData->aSz.Width();
    }
}

tools::Rectangle MenuBarWindow::ImplGetItemRect( sal_uInt16 nPos ) const
{
    tools::Rectangle aRect;
    if( m_pMenu )
    {
        tools::Long nX = 0;
        size_t nCount = m_pMenu->pItemList->size();
        for ( size_t n = 0; n < nCount; n++ )
        {
            MenuItemData* pData = m_pMenu->pItemList->GetDataFromPos( n );
            if ( n == nPos )
            {
                if ( pData->eType != MenuItemType::SEPARATOR )
                    // #107747# give menuitems the height of the menubar
                    aRect = tools::Rectangle( Point( nX, 1 ), Size( pData->aSz.Width(), GetOutputSizePixel().Height()-2 ) );
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
    if (!m_pMenu)
        return false;

    if (m_pMenu->bInCallback)
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

    // no key events if native menus
    if (m_pMenu->ImplGetSalMenu() && m_pMenu->ImplGetSalMenu()->VisibleMenuBar())
    {
        return false;
    }

    if ( nCode == KEY_MENU && !rKEvent.GetKeyCode().IsShift() ) // only F10, not Shift-F10
    {
        mbAutoPopup = false;
        if ( m_nHighlightedItem == ITEMPOS_INVALID )
        {
            ChangeHighlightItem( 0, false );
            GrabFocus();
        }
        else
        {
            ChangeHighlightItem( ITEMPOS_INVALID, false );
            m_xSaveFocusId = nullptr;
        }
        bDone = true;
    }
    else if ( bFromMenu )
    {
        if ( ( nCode == KEY_LEFT ) || ( nCode == KEY_RIGHT ) ||
            ( nCode == KEY_HOME ) || ( nCode == KEY_END ) )
        {
            sal_uInt16 n = m_nHighlightedItem;
            if ( n == ITEMPOS_INVALID )
            {
                if ( nCode == KEY_LEFT)
                    n = 0;
                else
                    n = m_pMenu->GetItemCount()-1;
            }

            sal_uInt16 nLoop = n;

            if( nCode == KEY_HOME )
                { n = sal_uInt16(-1); nLoop = n+1; }
            if( nCode == KEY_END )
                { n = m_pMenu->GetItemCount(); nLoop = n-1; }

            do
            {
                if ( nCode == KEY_LEFT || nCode == KEY_END )
                {
                    if ( n )
                        n--;
                    else
                        n = m_pMenu->GetItemCount()-1;
                }
                if ( nCode == KEY_RIGHT || nCode == KEY_HOME )
                {
                    n++;
                    if ( n >= m_pMenu->GetItemCount() )
                        n = 0;
                }

                MenuItemData* pData = m_pMenu->GetItemList()->GetDataFromPos( n );
                if (pData->eType != MenuItemType::SEPARATOR &&
                    m_pMenu->ImplIsVisible(n) &&
                    !m_pMenu->ImplCurrentlyHiddenOnGUI(n))
                {
                    ChangeHighlightItem( n, true );
                    break;
                }
            } while ( n != nLoop );
            bDone = true;
        }
        else if ( nCode == KEY_RETURN )
        {
            if( m_pActivePopup ) KillActivePopup();
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
            if( m_pActivePopup )
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

    if ( !bDone && ( bFromMenu || rKEvent.GetKeyCode().IsMod2() ) )
    {
        sal_Unicode nCharCode = rKEvent.GetCharCode();
        if ( nCharCode )
        {
            size_t nEntry, nDuplicates;
            MenuItemData* pData = m_pMenu->GetItemList()->SearchItem( nCharCode, rKEvent.GetKeyCode(), nEntry, nDuplicates, m_nHighlightedItem );
            if ( pData && (nEntry != ITEMPOS_INVALID) )
            {
                mbAutoPopup = true;
                ChangeHighlightItem( nEntry, true );
                bDone = true;
            }
        }
    }

    return bDone;
}

void MenuBarWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (!m_pMenu)
        return;

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    Size aOutputSize = GetOutputSizePixel();

    // no VCL paint if native menus
    if (m_pMenu->ImplGetSalMenu() && m_pMenu->ImplGetSalMenu()->VisibleMenuBar())
        return;

    // Make sure that all actual rendering happens in one go to avoid flicker.
    vcl::BufferDevice pBuffer(this, rRenderContext);

    if (rRenderContext.IsNativeControlSupported(ControlType::Menubar, ControlPart::Entire))
    {
        MenubarValue aMenubarValue;
        aMenubarValue.maTopDockingAreaHeight = ImplGetTopDockingAreaHeight(this);

        if (!rStyleSettings.GetPersonaHeader().IsEmpty())
            Erase(*pBuffer);
        else
        {
            tools::Rectangle aCtrlRegion( Point(), aOutputSize );

            pBuffer->DrawNativeControl(ControlType::Menubar, ControlPart::Entire, aCtrlRegion,
                                       ControlState::ENABLED, aMenubarValue, OUString());
        }

        ImplAddNWFSeparator(*pBuffer, aOutputSize, aMenubarValue);
    }

    // shrink the area of the buttons
    aOutputSize.AdjustWidth( -(m_aCloseBtn->GetSizePixel().Width()) );

    pBuffer->SetFillColor(rStyleSettings.GetMenuColor());
    m_pMenu->ImplPaint(*pBuffer, aOutputSize, 0);

    if (m_nHighlightedItem != ITEMPOS_INVALID && m_pMenu && !m_pMenu->GetItemList()->GetDataFromPos(m_nHighlightedItem)->bHiddenOnGUI)
        HighlightItem(*pBuffer, m_nHighlightedItem);
    else if (m_nRolloveredItem != ITEMPOS_INVALID)
        HighlightItem(*pBuffer, m_nRolloveredItem);

    // in high contrast mode draw a separating line on the lower edge
    if (!rRenderContext.IsNativeControlSupported( ControlType::Menubar, ControlPart::Entire) &&
        rStyleSettings.GetHighContrastMode())
    {
        pBuffer->Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::MAPMODE);
        pBuffer->SetLineColor(COL_WHITE);
        pBuffer->SetMapMode(MapMode(MapUnit::MapPixel));
        Size aSize = GetSizePixel();
        pBuffer->DrawLine(Point(0, aSize.Height() - 1),
                          Point(aSize.Width() - 1, aSize.Height() - 1));
        pBuffer->Pop();
    }
}

void MenuBarWindow::Resize()
{
    Size aOutSz = GetOutputSizePixel();
    tools::Long n      = aOutSz.Height()-4;
    tools::Long nX     = aOutSz.Width()-3;
    tools::Long nY     = 2;

    if ( m_aCloseBtn->IsVisible() )
    {
        m_aCloseBtn->Hide();
        m_aCloseBtn->SetImages(n);
        Size aTbxSize( m_aCloseBtn->CalcWindowSizePixel() );
        nX -= aTbxSize.Width();
        tools::Long nTbxY = (aOutSz.Height() - aTbxSize.Height())/2;
        m_aCloseBtn->setPosSizePixel(nX, nTbxY, aTbxSize.Width(), aTbxSize.Height());
        nX -= 3;
        m_aCloseBtn->Show();
    }
    if ( m_aFloatBtn->IsVisible() )
    {
        nX -= n;
        m_aFloatBtn->setPosSizePixel( nX, nY, n, n );
    }
    if ( m_aHideBtn->IsVisible() )
    {
        nX -= n;
        m_aHideBtn->setPosSizePixel( nX, nY, n, n );
    }

    m_aFloatBtn->SetSymbol( SymbolType::FLOAT );
    m_aHideBtn->SetSymbol( SymbolType::HIDE );

    Invalidate();
}

sal_uInt16 MenuBarWindow::ImplFindEntry( const Point& rMousePos ) const
{
    if( m_pMenu )
    {
        tools::Long nX = 0;
        size_t nCount = m_pMenu->pItemList->size();
        for ( size_t n = 0; n < nCount; n++ )
        {
            MenuItemData* pData = m_pMenu->pItemList->GetDataFromPos( n );
            if ( m_pMenu->ImplIsVisible( n ) )
            {
                nX += pData->aSz.Width();
                if ( nX > rMousePos.X() )
                    return static_cast<sal_uInt16>(n);
            }
        }
    }
    return ITEMPOS_INVALID;
}

void MenuBarWindow::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nId = m_nHighlightedItem;
    if ( rHEvt.GetMode() & HelpEventMode::CONTEXT )
        ChangeHighlightItem( ITEMPOS_INVALID, true );

    tools::Rectangle aHighlightRect( ImplGetItemRect( m_nHighlightedItem ) );
    if( !ImplHandleHelpEvent( this, m_pMenu, nId, rHEvt, aHighlightRect ) )
        Window::RequestHelp( rHEvt );
}

void MenuBarWindow::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if (nType == StateChangedType::ControlForeground ||
        nType == StateChangedType::ControlBackground)
    {
        ApplySettings(*GetOutDev());
        Invalidate();
    }
    else if (nType == StateChangedType::Enable)
    {
        Invalidate();
    }
    else if(m_pMenu)
    {
        m_pMenu->ImplKillLayoutData();
    }
}

void MenuBarWindow::LayoutChanged()
{
    if (!m_pMenu)
        return;

    ApplySettings(*GetOutDev());

    // if the font was changed.
    tools::Long nHeight = m_pMenu->ImplCalcSize(this).Height();

    // depending on the native implementation or the displayable flag
    // the menubar windows is suppressed (ie, height=0)
    if (!m_pMenu->IsDisplayable() ||
        (m_pMenu->ImplGetSalMenu() && m_pMenu->ImplGetSalMenu()->VisibleMenuBar()))
    {
        nHeight = 0;
    }
    setPosSizePixel(0, 0, 0, nHeight, PosSizeFlags::Height);
    GetParent()->Resize();
    Invalidate();
    Resize();

    m_pMenu->ImplKillLayoutData();
}

void MenuBarWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    Window::ApplySettings(rRenderContext);
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    SetPointFont(rRenderContext, rStyleSettings.GetMenuFont());

    const BitmapEx& rPersonaBitmap = Application::GetSettings().GetStyleSettings().GetPersonaHeader();
    SalMenu *pNativeMenu = m_pMenu ? m_pMenu->ImplGetSalMenu() : nullptr;
    if (pNativeMenu)
        pNativeMenu->ApplyPersona();
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
    if (!(IsNativeControlSupported(ControlType::Menubar, ControlPart::MenuItem) &&
        IsNativeControlSupported(ControlType::Menubar, ControlPart::Entire)))
        return;

    AllSettings aSettings(GetSettings());
    ImplGetFrame()->UpdateSettings(aSettings); // to update persona
    StyleSettings aStyle(aSettings.GetStyleSettings());
    Color aHighlightTextColor = ImplGetSVData()->maNWFData.maMenuBarHighlightTextColor;
    if (aHighlightTextColor != COL_TRANSPARENT)
    {
        aStyle.SetMenuHighlightTextColor(aHighlightTextColor);
    }
    aSettings.SetStyleSettings(aStyle);
    GetOutDev()->SetSettings(aSettings);
}

void MenuBarWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    Window::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ApplySettings(*GetOutDev());
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
    SalMenu *pNativeMenu = m_pMenu ? m_pMenu->ImplGetSalMenu() : nullptr;
    if (pNativeMenu && pNativeMenu->TakeFocus())
        return;

    if ( m_nHighlightedItem == ITEMPOS_INVALID )
    {
        mbAutoPopup = false;    // do not open menu when activated by focus handling like taskpane cycling
        ChangeHighlightItem( 0, false );
    }
}

css::uno::Reference<css::accessibility::XAccessible> MenuBarWindow::CreateAccessible()
{
    css::uno::Reference<css::accessibility::XAccessible> xAcc;

    if (m_pMenu)
        xAcc = m_pMenu->GetAccessible();

    return xAcc;
}

sal_uInt16 MenuBarWindow::AddMenuBarButton( const Image& i_rImage, const Link<MenuBarButtonCallbackArg&,bool>& i_rLink, const OUString& i_rToolTip )
{
    // find first free button id
    sal_uInt16 nId = IID_DOCUMENTCLOSE;
    std::map< sal_uInt16, AddButtonEntry >::const_iterator it;
    do
    {
        nId++;
        it = m_aAddButtons.find( nId );
    } while( it != m_aAddButtons.end() && nId < 128 );
    SAL_WARN_IF( nId >= 128, "vcl", "too many addbuttons in menubar" );
    AddButtonEntry& rNewEntry = m_aAddButtons[nId];
    rNewEntry.m_aSelectLink = i_rLink;
    m_aCloseBtn->InsertItem(ToolBoxItemId(nId), i_rImage, ToolBoxItemBits::NONE, 0);
    m_aCloseBtn->calcMinSize();
    ShowButtons(m_aCloseBtn->IsItemVisible(ToolBoxItemId(IID_DOCUMENTCLOSE)), m_aFloatBtn->IsVisible(), m_aHideBtn->IsVisible());
    LayoutChanged();

    if( m_pMenu->mpSalMenu )
        m_pMenu->mpSalMenu->AddMenuBarButton( SalMenuButtonItem( nId, i_rImage, i_rToolTip ) );

    return nId;
}

void MenuBarWindow::SetMenuBarButtonHighlightHdl( sal_uInt16 nId, const Link<MenuBarButtonCallbackArg&,bool>& rLink )
{
    std::map< sal_uInt16, AddButtonEntry >::iterator it = m_aAddButtons.find( nId );
    if( it != m_aAddButtons.end() )
        it->second.m_aHighlightLink = rLink;
}

tools::Rectangle MenuBarWindow::GetMenuBarButtonRectPixel( sal_uInt16 nId )
{
    tools::Rectangle aRect;
    if( m_aAddButtons.find( nId ) != m_aAddButtons.end() )
    {
        if( m_pMenu->mpSalMenu )
        {
            aRect = m_pMenu->mpSalMenu->GetMenuBarButtonRectPixel( nId, ImplGetWindowImpl()->mpFrame );
            if( aRect == tools::Rectangle( Point( -1, -1 ), Size( 1, 1 ) ) )
            {
                // system menu button is somewhere but location cannot be determined
                return tools::Rectangle();
            }
        }

        if( aRect.IsEmpty() )
        {
            aRect = m_aCloseBtn->GetItemRect(ToolBoxItemId(nId));
            Point aOffset = m_aCloseBtn->OutputToScreenPixel(Point());
            aRect.Move( aOffset.X(), aOffset.Y() );
        }
    }
    return aRect;
}

void MenuBarWindow::RemoveMenuBarButton( sal_uInt16 nId )
{
    ToolBox::ImplToolItems::size_type nPos = m_aCloseBtn->GetItemPos(ToolBoxItemId(nId));
    m_aCloseBtn->RemoveItem(nPos);
    m_aAddButtons.erase( nId );
    m_aCloseBtn->calcMinSize();
    LayoutChanged();

    if( m_pMenu->mpSalMenu )
        m_pMenu->mpSalMenu->RemoveMenuBarButton( nId );
}

bool MenuBarWindow::HandleMenuButtonEvent( sal_uInt16 i_nButtonId )
{
    std::map< sal_uInt16, AddButtonEntry >::iterator it = m_aAddButtons.find( i_nButtonId );
    if( it != m_aAddButtons.end() )
    {
        MenuBarButtonCallbackArg aArg;
        aArg.nId = it->first;
        aArg.bHighlight = true;
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
    SalMenu *pNativeMenu = m_pMenu ? m_pMenu->ImplGetSalMenu() : nullptr;
    if (pNativeMenu && pNativeMenu->VisibleMenuBar())
        return pNativeMenu->CanGetFocus();
    return GetSizePixel().Height() > 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
