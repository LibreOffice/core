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

package org.openoffice.java.accessibility;

import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;

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
        javax.accessibility.AccessibleRole.COLUMN_HEADER,
        javax.accessibility.AccessibleRole.CANVAS,
        javax.accessibility.AccessibleRole.CHECK_BOX,
        javax.accessibility.AccessibleRole.CHECK_BOX,   // CHECK_MENU_ITEM
        javax.accessibility.AccessibleRole.COLOR_CHOOSER,
        javax.accessibility.AccessibleRole.COMBO_BOX,
        javax.accessibility.AccessibleRole.DATE_EDITOR,
        javax.accessibility.AccessibleRole.DESKTOP_ICON,
        javax.accessibility.AccessibleRole.DESKTOP_PANE,
        javax.accessibility.AccessibleRole.DIRECTORY_PANE,
        javax.accessibility.AccessibleRole.DIALOG,
        javax.accessibility.AccessibleRole.CANVAS, // DOCUMENT
        javax.accessibility.AccessibleRole.PANEL,  // EMBEDDED_OBJECT
        javax.accessibility.AccessibleRole.PANEL,  // ENDNOTE
        javax.accessibility.AccessibleRole.FILE_CHOOSER,
        javax.accessibility.AccessibleRole.FILLER,
        javax.accessibility.AccessibleRole.FONT_CHOOSER,
        javax.accessibility.AccessibleRole.FOOTER,
        javax.accessibility.AccessibleRole.PANEL,  // FOOTNOTE
        javax.accessibility.AccessibleRole.FRAME,
        javax.accessibility.AccessibleRole.GLASS_PANE,
        javax.accessibility.AccessibleRole.PANEL,  // GRAPHIC
        javax.accessibility.AccessibleRole.GROUP_BOX,
        javax.accessibility.AccessibleRole.HEADER,
        javax.accessibility.AccessibleRole.TEXT,   // HEADING
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
        javax.accessibility.AccessibleRole.PARAGRAPH,
        javax.accessibility.AccessibleRole.PASSWORD_TEXT,
        javax.accessibility.AccessibleRole.POPUP_MENU,
        javax.accessibility.AccessibleRole.PUSH_BUTTON,
        javax.accessibility.AccessibleRole.PROGRESS_BAR,
        javax.accessibility.AccessibleRole.RADIO_BUTTON,
        javax.accessibility.AccessibleRole.RADIO_BUTTON,  // RADIO_MENU_ITEM
        javax.accessibility.AccessibleRole.ROW_HEADER,
        javax.accessibility.AccessibleRole.ROOT_PANE,
        javax.accessibility.AccessibleRole.SCROLL_BAR,
        javax.accessibility.AccessibleRole.SCROLL_PANE,
        javax.accessibility.AccessibleRole.CANVAS,  // SHAPE
        javax.accessibility.AccessibleRole.SEPARATOR,
        javax.accessibility.AccessibleRole.SLIDER,
        javax.accessibility.AccessibleRole.SPIN_BOX,
        javax.accessibility.AccessibleRole.SPLIT_PANE,
        javax.accessibility.AccessibleRole.STATUS_BAR,
        javax.accessibility.AccessibleRole.TABLE,
        javax.accessibility.AccessibleRole.LABEL,  // TABLE_CELL - required by ZoomText
        javax.accessibility.AccessibleRole.TEXT,
        javax.accessibility.AccessibleRole.PANEL,  // TEXT_FRAME
        javax.accessibility.AccessibleRole.TOGGLE_BUTTON,
        javax.accessibility.AccessibleRole.TOOL_BAR,
        javax.accessibility.AccessibleRole.TOOL_TIP,
        javax.accessibility.AccessibleRole.TREE,
        javax.accessibility.AccessibleRole.VIEWPORT,
        javax.accessibility.AccessibleRole.WINDOW,
    javax.accessibility.AccessibleRole.RADIO_BUTTON,  // BUTTON_DROPDOWN
    javax.accessibility.AccessibleRole.RADIO_BUTTON,  // BUTTON_MENU
    javax.accessibility.AccessibleRole.PANEL,  // CAPTION
    javax.accessibility.AccessibleRole.PANEL,  // CHART
    javax.accessibility.AccessibleRole.EDITBAR,
    javax.accessibility.AccessibleRole.PANEL,  // FORM
    javax.accessibility.AccessibleRole.PANEL,  // IMAGE_MAP
    javax.accessibility.AccessibleRole.PANEL,  // NOTE
    javax.accessibility.AccessibleRole.PANEL,  // PAGE
    javax.accessibility.AccessibleRole.RULER,
    javax.accessibility.AccessibleRole.PANEL,  // SECTION
    javax.accessibility.AccessibleRole.LABEL,  // TREE_ITEM
    javax.accessibility.AccessibleRole.TABLE   // TREE_TABLE
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
                        XAccessibleContext unoAccessibleContext = unoAccessible.getAccessibleContext();
                        if (unoAccessibleContext != null) {
                            return getAccessibleRole(unoAccessibleContext.getAccessibleRole());
                        }
                        return null;
        } catch (com.sun.star.uno.RuntimeException e) {
            return javax.accessibility.AccessibleRole.UNKNOWN;
        }
    }
}
