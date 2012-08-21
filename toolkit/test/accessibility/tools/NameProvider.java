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
        return maStateMap.get (new Integer(nStateId));
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
        return maEventMap.get (new Integer(nEventId));
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
        return maRoleMap.get (new Integer(nRole));
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
        return maRelationMap.get (new Integer(nRelation));
    }


    private static HashMap<Integer, String> maStateMap = new HashMap<Integer, String>();
    private static HashMap<Integer, String> maEventMap = new HashMap<Integer, String>();
    private static HashMap<Integer, String> maRoleMap = new HashMap<Integer, String>();
    private static HashMap<Integer, String> maRelationMap = new HashMap<Integer, String>();

    static {
        maStateMap.put (new Integer (AccessibleStateType.INVALID), "INVALID");
        maStateMap.put (new Integer (AccessibleStateType.ACTIVE), "ACTIVE");
        maStateMap.put (new Integer (AccessibleStateType.ARMED), "ARMED");
        maStateMap.put (new Integer (AccessibleStateType.BUSY), "BUSY");
        maStateMap.put (new Integer (AccessibleStateType.CHECKED), "CHECKED");
        //        maStateMap.put (new Integer (AccessibleStateType.COLLAPSED), "COLLAPSED");
        maStateMap.put (new Integer (AccessibleStateType.DEFUNC), "DEFUNC");
        maStateMap.put (new Integer (AccessibleStateType.EDITABLE), "EDITABLE");
        maStateMap.put (new Integer (AccessibleStateType.ENABLED), "ENABLED");
        maStateMap.put (new Integer (AccessibleStateType.EXPANDABLE), "EXPANDABLE");
        maStateMap.put (new Integer (AccessibleStateType.EXPANDED), "EXPANDED");
        maStateMap.put (new Integer (AccessibleStateType.FOCUSABLE), "FOCUSABLE");
        maStateMap.put (new Integer (AccessibleStateType.FOCUSED), "FOCUSED");
        maStateMap.put (new Integer (AccessibleStateType.HORIZONTAL), "HORIZONTAL");
        maStateMap.put (new Integer (AccessibleStateType.ICONIFIED), "ICONIFIED");
        maStateMap.put (new Integer (AccessibleStateType.MODAL), "MODAL");
        maStateMap.put (new Integer (AccessibleStateType.MULTI_LINE), "MULTI_LINE");
        maStateMap.put (new Integer (AccessibleStateType.MULTI_SELECTABLE), "MULTI_SELECTABLE");
        maStateMap.put (new Integer (AccessibleStateType.OPAQUE), "OPAQUE");
        maStateMap.put (new Integer (AccessibleStateType.PRESSED), "PRESSED");
        maStateMap.put (new Integer (AccessibleStateType.RESIZABLE), "RESIZABLE");
        maStateMap.put (new Integer (AccessibleStateType.SELECTABLE), "SELECTABLE");
        maStateMap.put (new Integer (AccessibleStateType.SELECTED), "SELECTED");
        maStateMap.put (new Integer (AccessibleStateType.SENSITIVE), "SENSITIVE");
        maStateMap.put (new Integer (AccessibleStateType.SHOWING), "SHOWING");
        maStateMap.put (new Integer (AccessibleStateType.SINGLE_LINE), "SINGLE_LINE");
        maStateMap.put (new Integer (AccessibleStateType.STALE), "STALE");
        maStateMap.put (new Integer (AccessibleStateType.TRANSIENT), "TRANSIENT");
        maStateMap.put (new Integer (AccessibleStateType.VERTICAL), "VERTICAL");
        maStateMap.put (new Integer (AccessibleStateType.VISIBLE), "VISIBLE");
        maStateMap.put (new Integer (AccessibleStateType.MANAGES_DESCENDANTS),
            "MANAGES_DESCENDANTS");
        //        maStateMap.put (new Integer (AccessibleStateType.INCONSISTENT),"INCONSISTENT");


        maEventMap.put (new Integer (0),
            "[UNKNOWN]");
        maEventMap.put (new Integer (AccessibleEventId.NAME_CHANGED),
            "NAME_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.DESCRIPTION_CHANGED),
            "DESCRIPTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.ACTION_CHANGED),
            "ACTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.STATE_CHANGED),
            "STATE_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.ACTIVE_DESCENDANT_CHANGED),
            "ACTIVE_DESCENDANT_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.BOUNDRECT_CHANGED),
            "BOUNDRECT_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.CHILD),
            "CHILD");
        maEventMap.put (new Integer (AccessibleEventId.INVALIDATE_ALL_CHILDREN),
            "INVALIDATE_ALL_CHILDREN");
        maEventMap.put (new Integer (AccessibleEventId.SELECTION_CHANGED),
            "SELECTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.VISIBLE_DATA_CHANGED),
            "VISIBLE_DATA_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.VALUE_CHANGED),
            "VALUE_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.CONTENT_FLOWS_FROM_RELATION_CHANGED),
            "CONTENT_FLOWS_FROM_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.CONTENT_FLOWS_TO_RELATION_CHANGED),
            "CONTENT_FLOWS_TO_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.CONTROLLED_BY_RELATION_CHANGED),
            "CONTROLLED_BY_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.CONTROLLER_FOR_RELATION_CHANGED),
            "CONTROLLER_FOR_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.LABEL_FOR_RELATION_CHANGED),
            "LABEL_FOR_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.LABELED_BY_RELATION_CHANGED),
            "LABELED_BY_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.MEMBER_OF_RELATION_CHANGED),
            "MEMBER_OF_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.SUB_WINDOW_OF_RELATION_CHANGED),
            "SUB_WINDOW_OF_RELATION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.CARET_CHANGED),
            "CARET_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TEXT_SELECTION_CHANGED),
            "TEXT_SELECTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TEXT_CHANGED),
            "TEXT_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TEXT_ATTRIBUTE_CHANGED),
            "TEXT_ATTRIBUTE_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.HYPERTEXT_CHANGED),
            "HYPERTEXT_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_CAPTION_CHANGED),
            "TABLE_CAPTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_COLUMN_DESCRIPTION_CHANGED),
            "TABLE_COLUMN_DESCRIPTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_COLUMN_HEADER_CHANGED),
            "TABLE_COLUMN_HEADER_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_MODEL_CHANGED),
            "TABLE_MODEL_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_ROW_DESCRIPTION_CHANGED),
            "TABLE_ROW_DESCRIPTION_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_ROW_HEADER_CHANGED),
            "TABLE_ROW_HEADER_CHANGED");
        maEventMap.put (new Integer (AccessibleEventId.TABLE_SUMMARY_CHANGED),
            "TABLE_SUMMARY_CHANGED");

        maRoleMap.put (new Integer(AccessibleRole.UNKNOWN), "UNKNOWN");
        maRoleMap.put (new Integer (AccessibleRole.UNKNOWN), "UNKNOWN");
        maRoleMap.put (new Integer (AccessibleRole.ALERT), "ALERT");
        maRoleMap.put (new Integer (AccessibleRole.COLUMN_HEADER), "COLUMN_HEADER");
        maRoleMap.put (new Integer (AccessibleRole.CANVAS), "CANVAS");
        maRoleMap.put (new Integer (AccessibleRole.CHECK_BOX), "CHECK_BOX");
        maRoleMap.put (new Integer (AccessibleRole.CHECK_MENU_ITEM), "CHECK_MENU_ITEM");
        maRoleMap.put (new Integer (AccessibleRole.COLOR_CHOOSER), "COLOR_CHOOSER");
        maRoleMap.put (new Integer (AccessibleRole.COMBO_BOX), "COMBO_BOX");
        maRoleMap.put (new Integer (AccessibleRole.DESKTOP_ICON), "DESKTOP_ICON");
        maRoleMap.put (new Integer (AccessibleRole.DESKTOP_PANE), "DESKTOP_PANE");
        maRoleMap.put (new Integer (AccessibleRole.DIRECTORY_PANE), "DIRECTORY_PANE");
        maRoleMap.put (new Integer (AccessibleRole.DIALOG), "DIALOG");
        maRoleMap.put (new Integer (AccessibleRole.DOCUMENT), "DOCUMENT");
        maRoleMap.put (new Integer (AccessibleRole.EMBEDDED_OBJECT), "EMBEDDED_OBJECT");
        maRoleMap.put (new Integer (AccessibleRole.END_NOTE), "END_NOTE");
        maRoleMap.put (new Integer (AccessibleRole.FILE_CHOOSER), "FILE_CHOOSER");
        maRoleMap.put (new Integer (AccessibleRole.FILLER), "FILLER");
        maRoleMap.put (new Integer (AccessibleRole.FONT_CHOOSER), "FONT_CHOOSER");
        maRoleMap.put (new Integer (AccessibleRole.FOOTER), "FOOTER");
        maRoleMap.put (new Integer (AccessibleRole.FOOTNOTE), "FOOTNOTE");
        maRoleMap.put (new Integer (AccessibleRole.FRAME), "FRAME");
        maRoleMap.put (new Integer (AccessibleRole.GLASS_PANE), "GLASS_PANE");
        maRoleMap.put (new Integer (AccessibleRole.GRAPHIC), "GRAPHIC");
        maRoleMap.put (new Integer (AccessibleRole.GROUP_BOX), "GROUP_BOX");
        maRoleMap.put (new Integer (AccessibleRole.HEADER), "HEADER");
        maRoleMap.put (new Integer (AccessibleRole.HEADING), "HEADING");
        maRoleMap.put (new Integer (AccessibleRole.HYPER_LINK), "HYPER_LINK");
        maRoleMap.put (new Integer (AccessibleRole.ICON), "ICON");
        maRoleMap.put (new Integer (AccessibleRole.INTERNAL_FRAME), "INTERNAL_FRAME");
        maRoleMap.put (new Integer (AccessibleRole.LABEL), "LABEL");
        maRoleMap.put (new Integer (AccessibleRole.LAYERED_PANE), "LAYERED_PANE");
        maRoleMap.put (new Integer (AccessibleRole.LIST), "LIST");
        maRoleMap.put (new Integer (AccessibleRole.LIST_ITEM), "LIST_ITEM");
        maRoleMap.put (new Integer (AccessibleRole.MENU), "MENU");
        maRoleMap.put (new Integer (AccessibleRole.MENU_BAR), "MENU_BAR");
        maRoleMap.put (new Integer (AccessibleRole.MENU_ITEM), "MENU_ITEM");
        maRoleMap.put (new Integer (AccessibleRole.OPTION_PANE), "OPTION_PANE");
        maRoleMap.put (new Integer (AccessibleRole.PAGE_TAB), "PAGE_TAB");
        maRoleMap.put (new Integer (AccessibleRole.PAGE_TAB_LIST), "PAGE_TAB_LIST");
        maRoleMap.put (new Integer (AccessibleRole.PANEL), "PANEL");
        maRoleMap.put (new Integer (AccessibleRole.PARAGRAPH), "PARAGRAPH");
        maRoleMap.put (new Integer (AccessibleRole.PASSWORD_TEXT), "PASSWORD_TEXT");
        maRoleMap.put (new Integer (AccessibleRole.POPUP_MENU), "POPUP_MENU");
        maRoleMap.put (new Integer (AccessibleRole.PUSH_BUTTON), "PUSH_BUTTON");
        maRoleMap.put (new Integer (AccessibleRole.PROGRESS_BAR), "PROGRESS_BAR");
        maRoleMap.put (new Integer (AccessibleRole.RADIO_BUTTON), "RADIO_BUTTON");
        maRoleMap.put (new Integer (AccessibleRole.RADIO_MENU_ITEM), "RADIO_MENU_ITEM");
        maRoleMap.put (new Integer (AccessibleRole.ROW_HEADER), "ROW_HEADER");
        maRoleMap.put (new Integer (AccessibleRole.ROOT_PANE), "ROOT_PANE");
        maRoleMap.put (new Integer (AccessibleRole.SCROLL_BAR), "SCROLL_BAR");
        maRoleMap.put (new Integer (AccessibleRole.SCROLL_PANE), "SCROLL_PANE");
        maRoleMap.put (new Integer (AccessibleRole.SHAPE), "SHAPE");
        maRoleMap.put (new Integer (AccessibleRole.SEPARATOR), "SEPARATOR");
        maRoleMap.put (new Integer (AccessibleRole.SLIDER), "SLIDER");
        maRoleMap.put (new Integer (AccessibleRole.SPIN_BOX), "SPIN_BOX");
        maRoleMap.put (new Integer (AccessibleRole.SPLIT_PANE), "SPLIT_PANE");
        maRoleMap.put (new Integer (AccessibleRole.STATUS_BAR), "STATUS_BAR");
        maRoleMap.put (new Integer (AccessibleRole.TABLE), "TABLE");
        maRoleMap.put (new Integer (AccessibleRole.TABLE_CELL), "TABLE_CELL");
        maRoleMap.put (new Integer (AccessibleRole.TEXT), "TEXT");
        maRoleMap.put (new Integer (AccessibleRole.TEXT_FRAME), "TEXT_FRAME");
        maRoleMap.put (new Integer (AccessibleRole.TOGGLE_BUTTON), "TOGGLE_BUTTON");
        maRoleMap.put (new Integer (AccessibleRole.TOOL_BAR), "TOOL_BAR");
        maRoleMap.put (new Integer (AccessibleRole.TOOL_TIP), "TOOL_TIP");
        maRoleMap.put (new Integer (AccessibleRole.TREE), "TREE");
        maRoleMap.put (new Integer (AccessibleRole.VIEW_PORT), "VIEW_PORT");
        maRoleMap.put (new Integer (AccessibleRole.WINDOW), "WINDOW");

        maRelationMap.put (new Integer (AccessibleRelationType.INVALID), "INVALID");
        maRelationMap.put (new Integer (AccessibleRelationType.CONTENT_FLOWS_FROM), "CONTENT_FLOWS_FROM");
        maRelationMap.put (new Integer (AccessibleRelationType.CONTENT_FLOWS_TO), "CONTENT_FLOWS_TO");
        maRelationMap.put (new Integer (AccessibleRelationType.CONTROLLED_BY), "CONTROLLED_BY");
        maRelationMap.put (new Integer (AccessibleRelationType.CONTROLLER_FOR), "CONTROLLER_FOR");
        maRelationMap.put (new Integer (AccessibleRelationType.LABEL_FOR), "LABEL_FOR");
        maRelationMap.put (new Integer (AccessibleRelationType.LABELED_BY), "LABELED_BY");
        maRelationMap.put (new Integer (AccessibleRelationType.MEMBER_OF), "MEMBER_OF");
        maRelationMap.put (new Integer (AccessibleRelationType.SUB_WINDOW_OF), "SUB_WINDOW_OF");
    }
}
