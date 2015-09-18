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

#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/window.hxx>

class Button;

/** Toolbox that holds the close button (right hand side of the menubar).

This is also used by the online update check; when an update is available, it
inserts here the button that leads to the download of the update.
*/
class DecoToolBox : public ToolBox
{
    long lastSize;
    Size maMinSize;

    using Window::ImplInit;
public:
             DecoToolBox( vcl::Window* pParent, WinBits nStyle = 0 );
    virtual ~DecoToolBox() {}

    void    ImplInit();

    void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void    SetImages( long nMaxHeight = 0, bool bForce = false );

    void    calcMinSize();
    const Size& getMinSize() { return maMinSize;}

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
        sal_uInt16    m_nId;
        Link<MenuBar::MenuBarButtonCallbackArg&,bool>  m_aSelectLink;
        Link<MenuBar::MenuBarButtonCallbackArg&,bool>  m_aHighlightLink;

        AddButtonEntry() : m_nId( 0 ) {}
    };

    Menu*           pMenu;
    PopupMenu*      pActivePopup;
    sal_uInt16      nHighlightedItem;
    sal_uInt16      nRolloveredItem;
    sal_uLong       nSaveFocusId;
    bool            mbAutoPopup;
    bool            bIgnoreFirstMove;
    bool            bStayActive;
    bool            mbHideAccel;
    bool            mbMenuKey;

    VclPtr<DecoToolBox>  aCloseBtn;
    VclPtr<PushButton>   aFloatBtn;
    VclPtr<PushButton>   aHideBtn;

    std::map< sal_uInt16, AddButtonEntry > m_aAddButtons;

    void            HighlightItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos, bool bHighlight);
    void    ChangeHighlightItem(sal_uInt16 n, bool bSelectPopupEntry, bool bAllowRestoreFocus = true, bool bDefaultToDocument = true);

    sal_uInt16      ImplFindEntry( const Point& rMousePos ) const;
    void            ImplCreatePopup( bool bPreSelectFirst );
    bool    HandleKeyEvent(const KeyEvent& rKEvent, bool bFromMenu = true);
    Rectangle       ImplGetItemRect( sal_uInt16 nPos );

    void            ImplInitStyleSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) SAL_OVERRIDE;

    DECL_LINK_TYPED( CloseHdl, ToolBox*, void );
    DECL_LINK_TYPED( ToolboxEventHdl, VclWindowEvent&, void );
    DECL_LINK_TYPED( ShowHideListener, VclWindowEvent&, void );

    void            StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    void            DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;
    void            LoseFocus() SAL_OVERRIDE;
    void            GetFocus() SAL_OVERRIDE;

public:
    explicit        MenuBarWindow( vcl::Window* pParent );
    virtual         ~MenuBarWindow();
    virtual void    dispose() SAL_OVERRIDE;

    void    ShowButtons(bool bClose, bool bFloat, bool bHide);

    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvent ) SAL_OVERRIDE;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

    void    SetFocusId(sal_uLong nId) { nSaveFocusId = nId; }
    sal_uLong GetFocusId() const { return nSaveFocusId; }

    void    SetMenu(MenuBar* pMenu);
    void    SetHeight(long nHeight);
    void    KillActivePopup();
    void    PopupClosed(Menu* pMenu);
    sal_uInt16 GetHighlightedItem() const { return nHighlightedItem; }
    virtual css::uno::Reference<css::accessibility::XAccessible> CreateAccessible() SAL_OVERRIDE;

    void    SetAutoPopup(bool bAuto) { mbAutoPopup = bAuto; }
    void    LayoutChanged();
    Size            MinCloseButtonSize();

    /// Add an arbitrary button to the menubar that will appear next to the close button.
    sal_uInt16 AddMenuBarButton(const Image&, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>&, const OUString&);
    void SetMenuBarButtonHighlightHdl(sal_uInt16 nId, const Link<MenuBar::MenuBarButtonCallbackArg&,bool>&);
    Rectangle GetMenuBarButtonRectPixel(sal_uInt16 nId);
    void RemoveMenuBarButton(sal_uInt16 nId);
    bool HandleMenuButtonEvent(sal_uInt16 i_nButtonId);
    virtual void SetMBWHideAccel (bool val) { mbHideAccel = val; }
    virtual bool GetMBWHideAccel (void) const { return mbHideAccel; }
    virtual void SetMBWMenuKey (bool val) { mbMenuKey = val; }
    virtual bool GetMBWMenuKey (void) const { return mbMenuKey; }
};

#endif // INCLUDED_VCL_SOURCE_WINDOW_MENUBARWINDOW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
