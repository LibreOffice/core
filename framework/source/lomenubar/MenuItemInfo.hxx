/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * A LibreOffice extension to send the menubar structure through DBusMenu
 *
 * Copyright 2011 Canonical, Ltd.
 * Authors:
 *     Alberto Ruiz <alberto.ruiz@codethink.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the the GNU Lesser General Public License version 3, as published by the Free
 * Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranties of MERCHANTABILITY,
 * SATISFACTORY QUALITY or FITNESS FOR A PARTICULAR PURPOSE.  See the applicable
 * version of the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with this program. If not, see <http://www.gnu.org/licenses/>
 *
 */

#ifndef __MENU_ITEM_INFO_HXX__
#define __MENU_ITEM_INFO_HXX__
#include <glib.h>

// This is used in a hash table with commandurls as keys
class MenuItemInfo {
    gchar*            m_label;         //Label text in UTF-8 with tildes subst by underscores
    gint              m_check_state;
    gchar*            m_check_type;
    gboolean          m_is_enabled;
    gboolean          m_is_visible;

  public:
    MenuItemInfo ();
    ~MenuItemInfo ();

    //Setters
    void setLabel (gchar* label);
    void setEnabled (gboolean is_enabled);
    void setCheckState (gint check_state);
    void setCheckType (const gchar* check_type);
    void setVisible (gboolean is_visible);

    //Getters
    gchar*       getLabel ();
    gboolean     getEnabled ();
    gint         getCheckState ();
    const gchar* getCheckType ();
    gboolean     getVisible ();
};
#endif // __MENU_ITEM_INFO_HXX__
