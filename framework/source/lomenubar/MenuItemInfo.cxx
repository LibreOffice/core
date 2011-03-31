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

#include "MenuItemInfo.hxx"
#include <libdbusmenu-gtk/menuitem.h>

MenuItemInfo::MenuItemInfo ()
{
    label = NULL;
    check_state = DBUSMENU_MENUITEM_TOGGLE_STATE_UNKNOWN;
    check_type  = (gchar*)DBUSMENU_MENUITEM_TOGGLE_CHECK;

    is_visible = TRUE;
    is_enabled = TRUE;
}

MenuItemInfo::~MenuItemInfo ()
{
    if (label)
        g_free(label);
}

//Setters
void
MenuItemInfo::setLabel (gchar* label)
{
    this->label = g_strdup (label);
}

void
MenuItemInfo::setEnabled (gboolean is_enabled)
{
    this->is_enabled = is_enabled;
}

void
MenuItemInfo::setCheckState (gint check_state)
{
    this->check_state = check_state;
}

void
MenuItemInfo::setCheckType (const gchar* check_type)
{
    this->check_type = (gchar*)check_type;
}

void
MenuItemInfo::setVisible (gboolean is_visible)
{
    this->is_visible = is_visible;
}

//Getters
gchar*
MenuItemInfo::getLabel ()
{
    return label;
}

gboolean
MenuItemInfo::getEnabled ()
{
    return is_enabled;
}

gint
MenuItemInfo::getCheckState ()
{
    return check_state;
}

const gchar*
MenuItemInfo::getCheckType ()
{
    return check_type;
}

gboolean
MenuItemInfo::getVisible ()
{
    return is_visible;
}
