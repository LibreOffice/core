/*************************************************************************
 *
 *  $RCSfile: AccessibleRoleAdapter.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 15:48:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package org.openoffice.java.accessibility;

import drafts.com.sun.star.accessibility.XAccessible;

/** This class maps the AccessibleRole(s) of the UNO accessibility API
 *  to the corresponding javax.accessibility objects.
 */
public abstract class AccessibleRoleAdapter {

    /* This array is used as a mapping between the UNO AccessibleRole
    * and the AccessibleRole objects of the Java accessibility API.
    */
    public static final javax.accessibility.AccessibleRole[] data = {
        javax.accessibility.AccessibleRole.UNKNOWN,
        javax.accessibility.AccessibleRole.ALERT,
        javax.accessibility.AccessibleRole.AWT_COMPONENT,
        javax.accessibility.AccessibleRole.COLUMN_HEADER,
        javax.accessibility.AccessibleRole.CANVAS,
        javax.accessibility.AccessibleRole.CHECK_BOX,
        javax.accessibility.AccessibleRole.COLOR_CHOOSER,
        javax.accessibility.AccessibleRole.COMBO_BOX,
        javax.accessibility.AccessibleRole.DATE_EDITOR,
        javax.accessibility.AccessibleRole.DESKTOP_ICON,
        javax.accessibility.AccessibleRole.DESKTOP_PANE,
        javax.accessibility.AccessibleRole.DIRECTORY_PANE,
        javax.accessibility.AccessibleRole.DIALOG,
        javax.accessibility.AccessibleRole.FILE_CHOOSER,
        javax.accessibility.AccessibleRole.FILLER,
        javax.accessibility.AccessibleRole.FONT_CHOOSER,
        javax.accessibility.AccessibleRole.FRAME,
        javax.accessibility.AccessibleRole.GLASS_PANE,
        javax.accessibility.AccessibleRole.GROUP_BOX,
        javax.accessibility.AccessibleRole.HYPERLINK,
        javax.accessibility.AccessibleRole.ICON,
        javax.accessibility.AccessibleRole.INTERNAL_FRAME,
        javax.accessibility.AccessibleRole.LABEL,
        javax.accessibility.AccessibleRole.LAYERED_PANE,
        javax.accessibility.AccessibleRole.LIST,
        javax.accessibility.AccessibleRole.LABEL,   // LIST_ITEM - required by Zoomtext
        javax.accessibility.AccessibleRole.MENU,
        javax.accessibility.AccessibleRole.MENU_BAR,
        javax.accessibility.AccessibleRole.MENU_ITEM,
        javax.accessibility.AccessibleRole.OPTION_PANE,
        javax.accessibility.AccessibleRole.PAGE_TAB,
        javax.accessibility.AccessibleRole.PAGE_TAB_LIST,
        javax.accessibility.AccessibleRole.PANEL,
        javax.accessibility.AccessibleRole.PASSWORD_TEXT,
        javax.accessibility.AccessibleRole.POPUP_MENU,
        javax.accessibility.AccessibleRole.PUSH_BUTTON,
        javax.accessibility.AccessibleRole.PROGRESS_BAR,
        javax.accessibility.AccessibleRole.RADIO_BUTTON,
        javax.accessibility.AccessibleRole.ROW_HEADER,
        javax.accessibility.AccessibleRole.ROOT_PANE,
        javax.accessibility.AccessibleRole.SCROLL_BAR,
        javax.accessibility.AccessibleRole.SCROLL_PANE,
        javax.accessibility.AccessibleRole.SEPARATOR,
        javax.accessibility.AccessibleRole.SLIDER,
        javax.accessibility.AccessibleRole.SPIN_BOX,
        javax.accessibility.AccessibleRole.SPLIT_PANE,
        javax.accessibility.AccessibleRole.STATUS_BAR,
        javax.accessibility.AccessibleRole.SWING_COMPONENT,
        javax.accessibility.AccessibleRole.TABLE,
        javax.accessibility.AccessibleRole.TEXT,
        javax.accessibility.AccessibleRole.TOGGLE_BUTTON,
        javax.accessibility.AccessibleRole.TOOL_BAR,
        javax.accessibility.AccessibleRole.TOOL_TIP,
        javax.accessibility.AccessibleRole.TREE,
        javax.accessibility.AccessibleRole.VIEWPORT,
        javax.accessibility.AccessibleRole.WINDOW,
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        null, // RESERVED
        javax.accessibility.AccessibleRole.CANVAS, // DOCUMENT
        javax.accessibility.AccessibleRole.PANEL,  // HEADER
        javax.accessibility.AccessibleRole.PANEL,  // FOOTER
        javax.accessibility.AccessibleRole.TEXT,   // PARAGRAPH
        javax.accessibility.AccessibleRole.TEXT,   // HEADING
        javax.accessibility.AccessibleRole.LABEL,  // TABLE_CELL - required by ZoomText
        javax.accessibility.AccessibleRole.PANEL,  // TEXT_FRAME
        javax.accessibility.AccessibleRole.PANEL,  // GRAPHIC
        javax.accessibility.AccessibleRole.PANEL,  // EMBEDDED_OBJECT
        javax.accessibility.AccessibleRole.PANEL,  // ENDNOTE
        javax.accessibility.AccessibleRole.PANEL,  // FOOTNOTE
        javax.accessibility.AccessibleRole.CANVAS  // SHAPE
    };

    public static javax.accessibility.AccessibleRole getAccessibleRole(short role) {
        if(role < data.length) {
            if(data[role] == null) {
                System.err.println("Unmapped role: " + role);
            }
            return data[role];
        }
        // FIXME: remove debug out
        System.err.println("Unmappable role: " + role);
        return null;
    }

    public static javax.accessibility.AccessibleRole getAccessibleRole(XAccessible unoAccessible) {
        try {
            return getAccessibleRole(unoAccessible.getAccessibleContext().getAccessibleRole());
        } catch (com.sun.star.uno.RuntimeException e) {
            return javax.accessibility.AccessibleRole.UNKNOWN;
        }
    }
}
