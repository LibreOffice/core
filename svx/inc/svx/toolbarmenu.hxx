/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: colrctrl.hxx,v $
 * $Revision: 1.3.76.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SVX_TOOLBARMENU_HXX_
#define _SVX_TOOLBARMENU_HXX_

#include <vector>

#include <vcl/ctrl.hxx>
#include <vcl/menu.hxx>

#include <sfx2/tbxctrl.hxx>

#include "svx/svxdllapi.h"

class ToolbarMenuEntry;
typedef std::vector< ToolbarMenuEntry * > ToolbarMenuEntryVector;

class SVX_DLLPUBLIC ToolbarMenu : public SfxPopupWindow
{
public:
                    ToolbarMenu( USHORT nId,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                 Window* pParentWindow,
                                 WinBits nBits );

                    ToolbarMenu( USHORT nId,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& rFrame,
                                 Window* pParentWindow,
                                 const ResId& rResId );

                    ~ToolbarMenu();

    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvent );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();

    void            appendEntry( int nEntryId, const String& rStr, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, const Image& rImage, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, const String& rStr, const Image& rImage, MenuItemBits nItemBits = 0 );
    void            appendEntry( int nEntryId, Control* pControl, MenuItemBits nItemBits = 0 );
//  void            appendEntry( int nEntryId, const String& rStr, Control* pControl, MenuItemBits nItemBits = 0 );
    void            appendSeparator();

    void            checkEntry( int nEntryId, bool bCheck = true );
    bool            isEntryChecked( int nEntryId ) const;

    void            enableEntry( int nEntryId, bool bEnable = true );
    bool            isEntryEnabled( int nEntryId ) const;

    void            setEntryText( int nEntryId, const String& rStr );
    const String&   getEntryText( int nEntryId ) const;

    void            setEntryImage( int nEntryId, const Image& rImage );
    const Image&    getEntryImage( int nEntryId ) const;

    const Size&     getMenuSize() const { return maSize; }

    void            SetHighlightHdl( const Link& rLink )    { maHighlightHdl = rLink; }
    const Link&     GetHighlightHdl() const                 { return maHighlightHdl; }

    void            SetSelectHdl( const Link& rLink )       { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                    { return maSelectHdl; }

    int             getSelectedEntryId() const;
    int             getHighlightedEntryId() const;

private:
    ToolbarMenuEntryVector  maEntryVector;

    int mnCheckPos;
    int mnImagePos;
    int mnTextPos;

    int mnHighlightedEntry;
    int mnSelectedEntry;

    Size maSize;

    Link            maHighlightHdl;
    Link            maSelectHdl;

    void            implInit();

    void            StateChanged( StateChangedType nType );
    void            DataChanged( const DataChangedEvent& rDCEvt );

    void            initWindow();

    Size            implCalcSize();

    void            appendEntry( ToolbarMenuEntry* pEntry );

    void            implPaint( ToolbarMenuEntry* pThisOnly = NULL, bool bHighlight = false );
    void            implDrawBorder();

    void            implHighlightEntry( int nHighlightEntry, bool bHighlight );
    void            implHighlightEntry( const MouseEvent& rMEvt, bool bMBDown );

    void            implChangeHighlightEntry( int nEntry );
    void            implSelectEntry( int nSelectedEntry );

    ToolbarMenuEntry*   implCursorUpDown( bool bUp, bool bHomeEnd );
    ToolbarMenuEntry*   implGetEntry( int nEntry ) const;
    ToolbarMenuEntry*   implSearchEntry( int nEntryId ) const;
};

#endif
