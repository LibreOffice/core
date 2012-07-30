/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef GTKSALMENU_HXX
#define GTKSALMENU_HXX

#include <vcl/sv.h>
#include <vcl/bitmap.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/salmenu.h>
#include <gio/gio.h>


class GtkSalMenu : public SalMenu
{
private:
    sal_Bool    mbMenuBar;

public:
    Menu*                   mpVCLMenu;
    const GtkSalFrame*      mpFrame;
    GMenuModel*             mpParentMenuModel;
    GMenuModel*             mpMenuModel;
    gchar*                  aDBusMenubarPath;
    GDBusConnection*        pSessionBus;

    GtkSalMenu( sal_Bool bMenuBar );
    virtual ~GtkSalMenu();

    virtual sal_Bool VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                                        // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );
};

class GtkSalMenuItem : public SalMenuItem
{
public:
    GtkSalMenuItem( const SalItemParams* );
    virtual ~GtkSalMenuItem();

    sal_uInt16          mnId;                 // Item ID
    Menu*               mpVCLMenu;            // VCL Menu into which this MenuItem is inserted
    GtkSalMenu*         mpParentMenu;         // The menu in which this menu item is inserted
    GtkSalMenu*         mpSubMenu;            // Sub menu of this item (if defined)
    GMenuItem*          mpMenuItem;           // The GMenuItem
};

#endif // GTKSALMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
