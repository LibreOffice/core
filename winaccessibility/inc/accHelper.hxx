/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef __ACCHELPER_HXX
#define __ACCHELPER_HXX

#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include "UAccCOM2.h"

enum DIRECTION
{
    FIRST_CHILD=0,
    LAST_CHILD=-1,
    BEFORE_CHILD=1 ,
    AFTER_CHILD=2
};

#define CHILDID_SELF             0
#define UNO_MSAA_UNMAPPING       0x00000000

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility::AccessibleRole;
using namespace com::sun::star::accessibility::AccessibleStateType;

//Role mapping table,left side is UNO role, right side is MSAA role
const short ROLE_TABLE[][2] =
    {
        {UNKNOWN,                  IA2_ROLE_UNKNOWN},
        {ALERT ,                   ROLE_SYSTEM_DIALOG},
        {COLUMN_HEADER ,           ROLE_SYSTEM_COLUMNHEADER},
        //{CANVAS ,                  ROLE_SYSTEM_CLIENT},
        {CANVAS ,                  IA2_ROLE_CANVAS},
        {CHECK_BOX ,               ROLE_SYSTEM_CHECKBUTTON},
        {CHECK_MENU_ITEM ,         IA2_ROLE_CHECK_MENU_ITEM},
        {COLOR_CHOOSER,            IA2_ROLE_COLOR_CHOOSER},
        {COMBO_BOX ,               ROLE_SYSTEM_COMBOBOX},
        {DATE_EDITOR ,             IA2_ROLE_DATE_EDITOR},
        {DESKTOP_ICON ,            IA2_ROLE_DESKTOP_ICON},
        {DESKTOP_PANE,             IA2_ROLE_DESKTOP_PANE},
        {DIRECTORY_PANE,           IA2_ROLE_DIRECTORY_PANE},
        {DIALOG,                   ROLE_SYSTEM_DIALOG},
        {DOCUMENT,                 ROLE_SYSTEM_DOCUMENT},
        {EMBEDDED_OBJECT ,         IA2_ROLE_EMBEDDED_OBJECT },
        {END_NOTE ,                IA2_ROLE_ENDNOTE },
        {FILE_CHOOSER ,            IA2_ROLE_FILE_CHOOSER },
        {FILLER,                   ROLE_SYSTEM_WHITESPACE},
        {FONT_CHOOSER,             IA2_ROLE_FONT_CHOOSER},
        {FOOTER,                   IA2_ROLE_FOOTER},
        {FOOTNOTE,                 IA2_ROLE_FOOTNOTE},
        //{FRAME,                      IA2_ROLE_FRAME},
        {FRAME,                    ROLE_SYSTEM_DIALOG},
        {GLASS_PANE ,              IA2_ROLE_GLASS_PANE},
        {GRAPHIC ,                 ROLE_SYSTEM_GRAPHIC},
        {GROUP_BOX,                ROLE_SYSTEM_GROUPING},
        {HEADER ,                  IA2_ROLE_HEADER},
        {HEADING ,                 IA2_ROLE_HEADING},
        {HYPER_LINK ,              ROLE_SYSTEM_TEXT},
        {ICON ,                    IA2_ROLE_ICON},
        {INTERNAL_FRAME,           IA2_ROLE_INTERNAL_FRAME},
        {LABEL,                    ROLE_SYSTEM_STATICTEXT},
        {LAYERED_PANE ,            IA2_ROLE_LAYERED_PANE},
        {LIST ,                    ROLE_SYSTEM_LIST},
        {LIST_ITEM ,               ROLE_SYSTEM_LISTITEM},
        //{MENU ,                    ROLE_SYSTEM_MENUPOPUP},
        {MENU,                ROLE_SYSTEM_MENUITEM},
        {MENU_BAR,                 ROLE_SYSTEM_MENUBAR},
        {MENU_ITEM,                ROLE_SYSTEM_MENUITEM},
        {OPTION_PANE ,             IA2_ROLE_OPTION_PANE},
        {PAGE_TAB,                 ROLE_SYSTEM_PAGETAB},
        {PAGE_TAB_LIST,            ROLE_SYSTEM_PAGETABLIST},
        {PANEL,                    IA2_ROLE_OPTION_PANE},
        {PARAGRAPH,                IA2_ROLE_PARAGRAPH},
        {PASSWORD_TEXT,            ROLE_SYSTEM_TEXT},
        {POPUP_MENU,               ROLE_SYSTEM_MENUPOPUP},
        {PUSH_BUTTON,              ROLE_SYSTEM_PUSHBUTTON},
        {PROGRESS_BAR,             ROLE_SYSTEM_PROGRESSBAR},
        {RADIO_BUTTON,             ROLE_SYSTEM_RADIOBUTTON},
        {RADIO_MENU_ITEM,          IA2_ROLE_RADIO_MENU_ITEM},
        {ROW_HEADER ,              ROLE_SYSTEM_ROWHEADER},
        {ROOT_PANE,                IA2_ROLE_ROOT_PANE},
        {SCROLL_BAR ,              ROLE_SYSTEM_SCROLLBAR},
        {SCROLL_PANE ,             IA2_ROLE_SCROLL_PANE},
        {SHAPE,                    IA2_ROLE_SHAPE},
        {SEPARATOR ,               ROLE_SYSTEM_SEPARATOR},
        {SLIDER ,                  ROLE_SYSTEM_SLIDER},
        {SPIN_BOX ,                ROLE_SYSTEM_SPINBUTTON},
        {SPLIT_PANE,               IA2_ROLE_SPLIT_PANE},
        {STATUS_BAR,               ROLE_SYSTEM_STATUSBAR},
        {TABLE,                    ROLE_SYSTEM_TABLE},
        {TABLE_CELL ,              ROLE_SYSTEM_CELL},
        {TEXT,                     ROLE_SYSTEM_TEXT},
        {TEXT_FRAME ,              IA2_ROLE_TEXT_FRAME},
        //for change toggle button to push button for jaws
        {TOGGLE_BUTTON,            ROLE_SYSTEM_PUSHBUTTON},

        {TOOL_BAR,                 ROLE_SYSTEM_TOOLBAR},
        {TOOL_TIP,                 ROLE_SYSTEM_TOOLTIP},
        {TREE ,                    ROLE_SYSTEM_OUTLINE},
        {VIEW_PORT ,               IA2_ROLE_VIEW_PORT},
        {WINDOW,                   ROLE_SYSTEM_WINDOW},
        {BUTTON_DROPDOWN,  ROLE_SYSTEM_BUTTONDROPDOWN},
        {BUTTON_MENU,             ROLE_SYSTEM_BUTTONMENU},
        {CAPTION,                   IA2_ROLE_CAPTION},
        {CHART,                     IA2_ROLE_SHAPE},
        {EDIT_BAR,                  IA2_ROLE_EDITBAR},
        {FORM,                      IA2_ROLE_FORM},
        {IMAGE_MAP ,              IA2_ROLE_IMAGE_MAP},
        {NOTE,                      IA2_ROLE_NOTE},
        {PAGE,                      IA2_ROLE_PAGE},
        {RULER ,                  IA2_ROLE_RULER},
        {SECTION,                   IA2_ROLE_SECTION},
        {TREE_ITEM ,              ROLE_SYSTEM_OUTLINEITEM},
        {TREE_TABLE,                ROLE_SYSTEM_OUTLINE}
    };

struct ltstr4
{
    bool operator()(const long  s1, const long  s2) const
    {
        return long(s1)<long(s2);
    }
};

#endif
