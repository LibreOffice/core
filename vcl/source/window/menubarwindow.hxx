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

#ifndef INCLUDED_VCL_SOURCE_WINDOW_MENUBARWINDOW_HXX
#define INCLUDED_VCL_SOURCE_WINDOW_MENUBARWINDOW_HXX

#include "menuwindow.hxx"

#include <vcl/toolkit/button.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/window.hxx>

#include <map>

class Button;

/** Toolbox that holds the close button (right hand side of the menubar).

This is also used by the online update check; when an update is available, it
inserts here the button that leads to the download of the update.
*/
class DecoToolBox : public ToolBox
{
    tools::Long lastSize;
    Size maMinSize;

public:
    explicit DecoToolBox(vcl::Window* pParent);

    void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void    SetImages( tools::Long nMaxHeight, bool bForce = false );

    void    calcMinSize();
    const Size& getMinSize() const { return maMinSize;}

    Image   maImage;
};


/** Class that implements the actual window of the menu bar.
*/
class MenuBarWindow : public vcl::Window, public MenuWindow
{
    friend class MenuBar;
    friend class Menu;

private:
    struct AddButtonEntry
    {
        Link<MenuBar::MenuBarButtonCallbackArg&,bool>  m_aSelectLink;
        Link<MenuBar::MenuBarButtonCallbackArg&,bool>  m_aHighlightLink;
    };

    VclPtr<Menu>           m_pMenu;
    VclPtr<PopupMenu>      m_pActivePopup;
    VclPtr<PopupMenu>      mpParentPopup;
    sal_uInt16      m_nHighlightedItem;
    sal_uInt16      m_nRolloveredItem;
    VclPtr<vcl::Window> m_xSaveFocusId;
    bool            mbAutoPopup;
    bool            m_bIgnoreFirstMove;
    bool            mbHideAccel;
    bool            mbMenuKey;

    VclPtr<DecoToolBox>  m_aCloseBtn;
    VclPtr<PushButton>   m_aFloatBtn;
    VclPtr<PushButton>   m_aHideBtn;

    std::map< sal_uInt16, AddButtonEntry > m_aAddButtons;

    void            HighlightItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos);
    void    ChangeHighlightItem(sal_uInt16 n, bool bSelectPopupEntry, bool bAllowRestoreFocus = true, bool bDefaultToDocument = true);

    sal_uInt16      ImplFindEntry( const Point& rMousePos ) const;
    void            ImplCreatePopup( bool bPreSelectFirst );
    bool    HandleKeyEvent(const KeyEvent& rKEvent, bool bFromMenu = true);
    tools::Rectangle       ImplGetItemRect( sal_uInt16 nPos );

    void            ImplInitStyleSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    DECL_LINK( CloseHdl, ToolBox*, void );
    DECL_LINK( ToolboxEventHdl, VclWindowEvent&, void );
    DECL_LINK( ShowHideListener, VclWindowEvent&, void );

    void            StateChanged( StateChangedType nType ) override;
    void            DataChanged( const DataChangedEvent& rDCEvt ) override;
    void            LoseFocus() override;
    void            GetFocus() override;

public:
    explicit        MenuBarWindow( vcl::Window* pParent );
    virtual         ~MenuBarWindow() override;
    virtual void    dispose() override;

    void    ShowButtons(bool bClose, bool bFloat, bool bHide);

    virtual void    MouseMove( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput( const KeyEvent& rKEvent ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Resize() override;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;

    void    SetMenu(MenuBar* pMenu);
    void    SetHeight(tools::Long nHeight);
    void    KillActivePopup();
    void    PopupClosed(Menu const * pMenu);
    sal_uInt16 GetHighlightedItem() const { return m_nHighlightedItem; }
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() override;

    void    SetAutoPopup(bool bAuto) { mbAutoPopup = bAuto; }
    void    LayoutChanged();
    Size const & MinCloseButtonSize();

    /// Add an arbitrary button to the menubar that will appear next to the close button.
    sal_uInt16 AddMenuBarButton(const Image&, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>&, const OUString&);
    void SetMenuBarButtonHighlightHdl(sal_uInt16 nId, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>&);
    tools::Rectangle GetMenuBarButtonRectPixel(sal_uInt16 nId);
    void RemoveMenuBarButton(sal_uInt16 nId);
    bool HandleMenuButtonEvent(sal_uInt16 i_nButtonId);
    void SetMBWHideAccel(bool val) { mbHideAccel = val; }
    bool GetMBWHideAccel() const { return mbHideAccel; }
    void SetMBWMenuKey(bool val) { mbMenuKey = val; }
    bool GetMBWMenuKey() const { return mbMenuKey; }
    bool CanGetFocus() const;
};

#endif // INCLUDED_VCL_SOURCE_WINDOW_MENUBARWINDOW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
