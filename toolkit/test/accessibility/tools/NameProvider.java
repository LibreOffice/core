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

package tools;

import java.util.HashMap;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.AccessibleRelationType;


/** Provide names for several accessibility constants groups.
*/
public class NameProvider
{
    /** Return the name of the specified state.
        @param nStateId
            Id of the state for which to return its name.  This is one of
            the ids listed in the <type>AccessibleStateType</const>
            constants group.
        @return
            Returns the name of the specified state or an empty string if an
            invalid / unknown state id was given.
     */
    public static String getStateName (int nStateId)
    {
        return maStateMap.get (Integer.valueOf(nStateId));
    }


    /** Return the name of the specified event.
        @param nEventId
            Id of the event type for which to return its name.  This is one
            of the ids listed in the <type>AccessibleEventId</const>
            constants group.
        @return
            Returns the name of the specified event type or an empty string
            if an invalid / unknown event id was given.
     */
    public static String getEventName (int nEventId)
    {
        return maEventMap.get (Integer.valueOf(nEventId));
    }


    /** Return the name of the specified role.
        @param nRole
            Id of the role for which to return its name.  This is one of
            the ids listed in the <type>AccessibleRole</const>
            constants group.
        @return
            Returns the name of the specified role or an empty string if an
            invalid / unknown role id was given.
     */
    public static String getRoleName (int nRole)
    {
        return maRoleMap.get (Integer.valueOf(nRole));
    }


    /** Return the name of the specified relation.
        @param nRelation
            Id of the relation for which to return its name.  This is one of
            the ids listed in the <type>AccessibleRelationType</const>
            constants group.
        @return
            Returns the name of the specified relation type or an empty
            string if an invalid / unknown role id was given.
     */
    public static String getRelationName (int nRelation)
    {
        return maRelationMap.get (Integer.valueOf(nRelation));
    }


    private static HashMap<Integer, String> maStateMap = new HashMap<Integer, String>();
    private static HashMap<Integer, String> maEventMap = new HashMap<Integer, String>();
    private static HashMap<Integer, String> maRoleMap = new HashMap<Integer, String>();
    private static HashMap<Integer, String> maRelationMap = new HashMap<Integer, String>();

    static {
        maStateMap.put (Integer.valueOf(AccessibleStateType.INVALID), "INVALID");
        maStateMap.put (Integer.valueOf(AccessibleStateType.ACTIVE), "ACTIVE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.ARMED), "ARMED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.BUSY), "BUSY");
        maStateMap.put (Integer.valueOf(AccessibleStateType.CHECKED), "CHECKED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.DEFUNC), "DEFUNC");
        maStateMap.put (Integer.valueOf(AccessibleStateType.EDITABLE), "EDITABLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.ENABLED), "ENABLED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.EXPANDABLE), "EXPANDABLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.EXPANDED), "EXPANDED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.FOCUSABLE), "FOCUSABLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.FOCUSED), "FOCUSED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.HORIZONTAL), "HORIZONTAL");
        maStateMap.put (Integer.valueOf(AccessibleStateType.ICONIFIED), "ICONIFIED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.MODAL), "MODAL");
        maStateMap.put (Integer.valueOf(AccessibleStateType.MULTI_LINE), "MULTI_LINE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.MULTI_SELECTABLE), "MULTI_SELECTABLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.OPAQUE), "OPAQUE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.PRESSED), "PRESSED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.RESIZABLE), "RESIZABLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.SELECTABLE), "SELECTABLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.SELECTED), "SELECTED");
        maStateMap.put (Integer.valueOf(AccessibleStateType.SENSITIVE), "SENSITIVE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.SHOWING), "SHOWING");
        maStateMap.put (Integer.valueOf(AccessibleStateType.SINGLE_LINE), "SINGLE_LINE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.STALE), "STALE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.TRANSIENT), "TRANSIENT");
        maStateMap.put (Integer.valueOf(AccessibleStateType.VERTICAL), "VERTICAL");
        maStateMap.put (Integer.valueOf(AccessibleStateType.VISIBLE), "VISIBLE");
        maStateMap.put (Integer.valueOf(AccessibleStateType.MANAGES_DESCENDANTS),
            "MANAGES_DESCENDANTS");


        maEventMap.put (Integer.valueOf(0),
            "[UNKNOWN]");
        maEventMap.put (Integer.valueOf(AccessibleEventId.NAME_CHANGED),
            "NAME_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.DESCRIPTION_CHANGED),
            "DESCRIPTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.ACTION_CHANGED),
            "ACTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.STATE_CHANGED),
            "STATE_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.ACTIVE_DESCENDANT_CHANGED),
            "ACTIVE_DESCENDANT_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.BOUNDRECT_CHANGED),
            "BOUNDRECT_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.CHILD),
            "CHILD");
        maEventMap.put (Integer.valueOf(AccessibleEventId.INVALIDATE_ALL_CHILDREN),
            "INVALIDATE_ALL_CHILDREN");
        maEventMap.put (Integer.valueOf(AccessibleEventId.SELECTION_CHANGED),
            "SELECTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.VISIBLE_DATA_CHANGED),
            "VISIBLE_DATA_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.VALUE_CHANGED),
            "VALUE_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.CONTENT_FLOWS_FROM_RELATION_CHANGED),
            "CONTENT_FLOWS_FROM_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.CONTENT_FLOWS_TO_RELATION_CHANGED),
            "CONTENT_FLOWS_TO_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.CONTROLLED_BY_RELATION_CHANGED),
            "CONTROLLED_BY_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.CONTROLLER_FOR_RELATION_CHANGED),
            "CONTROLLER_FOR_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.LABEL_FOR_RELATION_CHANGED),
            "LABEL_FOR_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.LABELED_BY_RELATION_CHANGED),
            "LABELED_BY_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.MEMBER_OF_RELATION_CHANGED),
            "MEMBER_OF_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.SUB_WINDOW_OF_RELATION_CHANGED),
            "SUB_WINDOW_OF_RELATION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.CARET_CHANGED),
            "CARET_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TEXT_SELECTION_CHANGED),
            "TEXT_SELECTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TEXT_CHANGED),
            "TEXT_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TEXT_ATTRIBUTE_CHANGED),
            "TEXT_ATTRIBUTE_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.HYPERTEXT_CHANGED),
            "HYPERTEXT_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_CAPTION_CHANGED),
            "TABLE_CAPTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_COLUMN_DESCRIPTION_CHANGED),
            "TABLE_COLUMN_DESCRIPTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_COLUMN_HEADER_CHANGED),
            "TABLE_COLUMN_HEADER_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_MODEL_CHANGED),
            "TABLE_MODEL_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_ROW_DESCRIPTION_CHANGED),
            "TABLE_ROW_DESCRIPTION_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_ROW_HEADER_CHANGED),
            "TABLE_ROW_HEADER_CHANGED");
        maEventMap.put (Integer.valueOf(AccessibleEventId.TABLE_SUMMARY_CHANGED),
            "TABLE_SUMMARY_CHANGED");

        maRoleMap.put (Integer.valueOf(AccessibleRole.UNKNOWN), "UNKNOWN");
        maRoleMap.put (Integer.valueOf(AccessibleRole.UNKNOWN), "UNKNOWN");
        maRoleMap.put (Integer.valueOf(AccessibleRole.ALERT), "ALERT");
        maRoleMap.put (Integer.valueOf(AccessibleRole.COLUMN_HEADER), "COLUMN_HEADER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.CANVAS), "CANVAS");
        maRoleMap.put (Integer.valueOf(AccessibleRole.CHECK_BOX), "CHECK_BOX");
        maRoleMap.put (Integer.valueOf(AccessibleRole.CHECK_MENU_ITEM), "CHECK_MENU_ITEM");
        maRoleMap.put (Integer.valueOf(AccessibleRole.COLOR_CHOOSER), "COLOR_CHOOSER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.COMBO_BOX), "COMBO_BOX");
        maRoleMap.put (Integer.valueOf(AccessibleRole.DESKTOP_ICON), "DESKTOP_ICON");
        maRoleMap.put (Integer.valueOf(AccessibleRole.DESKTOP_PANE), "DESKTOP_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.DIRECTORY_PANE), "DIRECTORY_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.DIALOG), "DIALOG");
        maRoleMap.put (Integer.valueOf(AccessibleRole.DOCUMENT), "DOCUMENT");
        maRoleMap.put (Integer.valueOf(AccessibleRole.EMBEDDED_OBJECT), "EMBEDDED_OBJECT");
        maRoleMap.put (Integer.valueOf(AccessibleRole.END_NOTE), "END_NOTE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.FILE_CHOOSER), "FILE_CHOOSER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.FILLER), "FILLER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.FONT_CHOOSER), "FONT_CHOOSER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.FOOTER), "FOOTER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.FOOTNOTE), "FOOTNOTE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.FRAME), "FRAME");
        maRoleMap.put (Integer.valueOf(AccessibleRole.GLASS_PANE), "GLASS_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.GRAPHIC), "GRAPHIC");
        maRoleMap.put (Integer.valueOf(AccessibleRole.GROUP_BOX), "GROUP_BOX");
        maRoleMap.put (Integer.valueOf(AccessibleRole.HEADER), "HEADER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.HEADING), "HEADING");
        maRoleMap.put (Integer.valueOf(AccessibleRole.HYPER_LINK), "HYPER_LINK");
        maRoleMap.put (Integer.valueOf(AccessibleRole.ICON), "ICON");
        maRoleMap.put (Integer.valueOf(AccessibleRole.INTERNAL_FRAME), "INTERNAL_FRAME");
        maRoleMap.put (Integer.valueOf(AccessibleRole.LABEL), "LABEL");
        maRoleMap.put (Integer.valueOf(AccessibleRole.LAYERED_PANE), "LAYERED_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.LIST), "LIST");
        maRoleMap.put (Integer.valueOf(AccessibleRole.LIST_ITEM), "LIST_ITEM");
        maRoleMap.put (Integer.valueOf(AccessibleRole.MENU), "MENU");
        maRoleMap.put (Integer.valueOf(AccessibleRole.MENU_BAR), "MENU_BAR");
        maRoleMap.put (Integer.valueOf(AccessibleRole.MENU_ITEM), "MENU_ITEM");
        maRoleMap.put (Integer.valueOf(AccessibleRole.OPTION_PANE), "OPTION_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PAGE_TAB), "PAGE_TAB");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PAGE_TAB_LIST), "PAGE_TAB_LIST");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PANEL), "PANEL");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PARAGRAPH), "PARAGRAPH");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PASSWORD_TEXT), "PASSWORD_TEXT");
        maRoleMap.put (Integer.valueOf(AccessibleRole.POPUP_MENU), "POPUP_MENU");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PUSH_BUTTON), "PUSH_BUTTON");
        maRoleMap.put (Integer.valueOf(AccessibleRole.PROGRESS_BAR), "PROGRESS_BAR");
        maRoleMap.put (Integer.valueOf(AccessibleRole.RADIO_BUTTON), "RADIO_BUTTON");
        maRoleMap.put (Integer.valueOf(AccessibleRole.RADIO_MENU_ITEM), "RADIO_MENU_ITEM");
        maRoleMap.put (Integer.valueOf(AccessibleRole.ROW_HEADER), "ROW_HEADER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.ROOT_PANE), "ROOT_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SCROLL_BAR), "SCROLL_BAR");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SCROLL_PANE), "SCROLL_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SHAPE), "SHAPE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SEPARATOR), "SEPARATOR");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SLIDER), "SLIDER");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SPIN_BOX), "SPIN_BOX");
        maRoleMap.put (Integer.valueOf(AccessibleRole.SPLIT_PANE), "SPLIT_PANE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.STATUS_BAR), "STATUS_BAR");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TABLE), "TABLE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TABLE_CELL), "TABLE_CELL");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TEXT), "TEXT");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TEXT_FRAME), "TEXT_FRAME");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TOGGLE_BUTTON), "TOGGLE_BUTTON");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TOOL_BAR), "TOOL_BAR");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TOOL_TIP), "TOOL_TIP");
        maRoleMap.put (Integer.valueOf(AccessibleRole.TREE), "TREE");
        maRoleMap.put (Integer.valueOf(AccessibleRole.VIEW_PORT), "VIEW_PORT");
        maRoleMap.put (Integer.valueOf(AccessibleRole.WINDOW), "WINDOW");

        maRelationMap.put (Integer.valueOf(AccessibleRelationType.INVALID), "INVALID");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.CONTENT_FLOWS_FROM), "CONTENT_FLOWS_FROM");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.CONTENT_FLOWS_TO), "CONTENT_FLOWS_TO");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.CONTROLLED_BY), "CONTROLLED_BY");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.CONTROLLER_FOR), "CONTROLLER_FOR");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.LABEL_FOR), "LABEL_FOR");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.LABELED_BY), "LABELED_BY");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.MEMBER_OF), "MEMBER_OF");
        maRelationMap.put (Integer.valueOf(AccessibleRelationType.SUB_WINDOW_OF), "SUB_WINDOW_OF");
    }
}
