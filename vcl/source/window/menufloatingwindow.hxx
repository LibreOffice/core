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

#ifndef INCLUDED_VCL_SOURCE_WINDOW_MENUFLOATINGWINDOW_HXX
#define INCLUDED_VCL_SOURCE_WINDOW_MENUFLOATINGWINDOW_HXX

#include "menuwindow.hxx"

#include <vcl/floatwin.hxx>
#include <vcl/menu.hxx>

#define EXTRASPACEY         2
#define GUTTERBORDER        8

/** Class that implements the actual window of the floating menu.
*/
class MenuFloatingWindow : public FloatingWindow, public MenuWindow
{
    friend void Menu::ImplFillLayoutData() const;
    friend Menu::~Menu();

private:
    Menu*           pMenu;
    PopupMenu*      pActivePopup;
    Timer           aHighlightChangedTimer;
    Timer           aSubmenuCloseTimer;
    Timer           aScrollTimer;
    sal_uLong           nSaveFocusId;
    sal_uInt16          nHighlightedItem;       // highlighted/selected Item
    sal_uInt16          nMBDownPos;
    sal_uInt16          nScrollerHeight;
    sal_uInt16          nFirstEntry;
    sal_uInt16          nBorder;
    sal_uInt16          nPosInParent;
    bool            bInExecute;

    bool            bScrollMenu;
    bool            bScrollUp;
    bool            bScrollDown;
    bool            bIgnoreFirstMove;
    bool            bKeyInput;

                    DECL_LINK(PopupEnd, void *);
                    DECL_LINK( HighlightChanged, Timer* );
                    DECL_LINK(SubmenuClose, void *);
                    DECL_LINK(AutoScroll, void *);
                    DECL_LINK( ShowHideListener, VclWindowEvent* );

    virtual void    StateChanged( StateChangedType nType ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    void            InitMenuClipRegion();

protected:
    vcl::Region          ImplCalcClipRegion( bool bIncludeLogo = true ) const;
    void            ImplDrawScroller( bool bUp );
    using Window::ImplScroll;
    void            ImplScroll( const Point& rMousePos );
    void            ImplScroll( bool bUp );
    void            ImplCursorUpDown( bool bUp, bool bHomeEnd = false );
    void            ImplHighlightItem( const MouseEvent& rMEvt, bool bMBDown );
    long            ImplGetStartY() const;
    Rectangle       ImplGetItemRect( sal_uInt16 nPos );

public:
                    MenuFloatingWindow( Menu* pMenu, vcl::Window* pParent, WinBits nStyle );
                    virtual ~MenuFloatingWindow();

            void    doShutdown();

    virtual void    MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvent ) SAL_OVERRIDE;
    virtual void    Command( const CommandEvent& rCEvt ) SAL_OVERRIDE;
    virtual void    Paint( const Rectangle& rRect ) SAL_OVERRIDE;
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual void    Resize() SAL_OVERRIDE;

    void            SetFocusId( sal_uLong nId ) { nSaveFocusId = nId; }
    sal_uLong           GetFocusId() const      { return nSaveFocusId; }

    void            EnableScrollMenu( bool b );
    bool            IsScrollMenu() const        { return bScrollMenu; }
    sal_uInt16          GetScrollerHeight() const   { return nScrollerHeight; }

    void            Execute();
    void            StopExecute( sal_uLong nFocusId = 0 );
    void            EndExecute();
    void            EndExecute( sal_uInt16 nSelectId );

    PopupMenu*      GetActivePopup() const  { return pActivePopup; }
    void            KillActivePopup( PopupMenu* pThisOnly = NULL );

    void            HighlightItem( sal_uInt16 nPos, bool bHighlight );
    void            ChangeHighlightItem( sal_uInt16 n, bool bStartPopupTimer );
    sal_uInt16          GetHighlightedItem() const { return nHighlightedItem; }

    void            SetPosInParent( sal_uInt16 nPos ) { nPosInParent = nPos; }

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible() SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_SOURCE_WINDOW_MENUFLOATINGWINDOW_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
