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

#include "glomenu.h"
#include "gloactiongroup.h"

#include <vector>


class GtkSalMenuItem;

class GtkSalMenu : public SalMenu
{
private:
    sal_Bool                        mbMenuBar;
    Menu*                           mpVCLMenu;
    GtkSalMenu*                     mpParentSalMenu;
    const GtkSalFrame*              mpFrame;

    std::vector< GMenuModel* >      maSections;
    std::vector< GtkSalMenuItem* >  maItems;

    // DBus attributes
    gchar*                          aDBusPath;
    gchar*                          aDBusMenubarPath;
    GDBusConnection*                pSessionBus;
    sal_Int32                       mMenubarId;
    sal_Int32                       mActionGroupId;

    // GMenuModel attributes
    GMenuModel*                     mpMenuModel;
    GMenuModel*                     mpCurrentSection;

    virtual void    publishMenu( GMenuModel*, GActionGroup* );
    GtkSalMenuItem* GetSalMenuItem( sal_uInt16 nId );

public:
    GtkSalMenu( sal_Bool bMenuBar );
    virtual ~GtkSalMenu();

    virtual sal_Bool            VisibleMenuBar();   // must return TRUE to actually DISPLAY native menu bars
                                                    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void                InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void                RemoveItem( unsigned nPos );
    virtual void                SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void                SetFrame( const SalFrame* pFrame );
    virtual const GtkSalFrame*  GetFrame() const;
    virtual void                CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void                EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void                SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText );
    virtual void                SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void                SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName );
    virtual void                GetSystemMenuData( SystemMenuData* pData );
    virtual void                SetItemCommand( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& aCommandStr );
    virtual void                Freeze();

    virtual void                SetMenu( Menu* pMenu ) { mpVCLMenu = pMenu; }
    virtual Menu*               GetMenu() { return mpVCLMenu; }
    virtual GtkSalMenu*         GetParentSalMenu() { return mpParentSalMenu; }
    virtual GMenuModel*         GetMenuModel() { return mpMenuModel; }
    virtual GMenuModel*         GetCurrentSection() { return mpCurrentSection; }
    virtual unsigned            GetItemCount() { return maItems.size(); }
    virtual GtkSalMenuItem*     GetItemAtPos( unsigned nPos ) { return maItems[ nPos ]; }
};

class GtkSalMenuItem : public SalMenuItem
{
public:
    GtkSalMenuItem( const SalItemParams* );
    virtual ~GtkSalMenuItem();

    sal_uInt16          mnId;                 // Item ID
    sal_uInt16          mnPos;                // Item position
    Menu*               mpVCLMenu;            // VCL Menu into which this MenuItem is inserted
    GtkSalMenu*         mpParentMenu;         // The menu in which this menu item is inserted
    GtkSalMenu*         mpSubMenu;            // Sub menu of this item (if defined)
    GMenuModel*         mpParentSection;      // Section where this item is added.
    GLOMenuItem*        mpMenuItem;           // The GMenuItem
    GAction*            mpAction;             // The GAction associated with this item
};

#endif // GTKSALMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
