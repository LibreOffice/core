/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef INCLUDED_VCL_VCLEVENT_HXX
#define INCLUDED_VCL_VCLEVENT_HXX

#include <tools/link.hxx>
#include <vcl/dllapi.h>
#include <vcl/impdel.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/uno/Reference.hxx>

class Menu;

namespace com { namespace sun { namespace star {
    namespace accessibility {
        class XAccessible;
    }
}}}

#define VCLEVENT_OBJECT_DYING                  1

// VclWindowEvent:
#define VCLEVENT_WINDOW_CHILDCREATED         500    // pData = vcl::Window*
#define VCLEVENT_WINDOW_CHILDDESTROYED       501    // pData = vcl::Window*
#define VCLEVENT_WINDOW_PAINT               1000    // pData = Rectangle*
#define VCLEVENT_WINDOW_MOVE                1001
#define VCLEVENT_WINDOW_RESIZE              1002
#define VCLEVENT_WINDOW_SHOW                1003
#define VCLEVENT_WINDOW_HIDE                1004
#define VCLEVENT_WINDOW_ACTIVATE            1005
#define VCLEVENT_WINDOW_DEACTIVATE          1006    // pData = vcl::Window* = pPrevActiveWindow
#define VCLEVENT_WINDOW_CLOSE               1007
#define VCLEVENT_WINDOW_GETFOCUS            1008
#define VCLEVENT_WINDOW_LOSEFOCUS           1009
#define VCLEVENT_WINDOW_MINIMIZE            1010
#define VCLEVENT_WINDOW_NORMALIZE           1011
#define VCLEVENT_WINDOW_KEYINPUT            1012    // pData = KeyEvent*
#define VCLEVENT_WINDOW_KEYUP               1013    // pData = KeyEvent*
#define VCLEVENT_WINDOW_COMMAND             1014    // pData = CommandEvent*
#define VCLEVENT_WINDOW_MOUSEMOVE           1015    // pData = MouseEvent*
#define VCLEVENT_WINDOW_MOUSEBUTTONDOWN     1016    // pData = MouseEvent*
#define VCLEVENT_WINDOW_MOUSEBUTTONUP       1017    // pData = MouseEvent*
#define VCLEVENT_WINDOW_FRAMETITLECHANGED   1018    // pData = OUString* = oldTitle
#define VCLEVENT_APPLICATION_DATACHANGED    1019    // pData = DataChangedEvent*
#define VCLEVENT_WINDOW_ENABLED             1020
#define VCLEVENT_WINDOW_DISABLED            1021
#define VCLEVENT_WINDOW_DATACHANGED         1022    // pData = DataChangedEvent*
#define VCLEVENT_WINDOW_ZOOM                1023    // pData = ZoomEvent*
#define VCLEVENT_WINDOW_SCROLL              1024    // pData = ScrollEvent*

// VclWindowEvent
#define VCLEVENT_CONTROL_GETFOCUS           1100
#define VCLEVENT_CONTROL_LOSEFOCUS          1101
#define VCLEVENT_BUTTON_CLICK               1102
#define VCLEVENT_PUSHBUTTON_TOGGLE          1103
#define VCLEVENT_RADIOBUTTON_TOGGLE         1104
#define VCLEVENT_CHECKBOX_TOGGLE            1105
#define VCLEVENT_COMBOBOX_SELECT            1106
#define VCLEVENT_COMBOBOX_DOUBLECLICK       1107
#define VCLEVENT_LISTBOX_SELECT             1108
#define VCLEVENT_LISTBOX_DOUBLECLICK        1109
#define VCLEVENT_EDIT_MODIFY                1110
#define VCLEVENT_SCROLLBAR_SCROLL           1111
#define VCLEVENT_SCROLLBAR_ENDSCROLL        1112
#define VCLEVENT_SPINBUTTON_UP              1113
#define VCLEVENT_SPINBUTTON_DOWN            1114
#define VCLEVENT_SPINFIELD_UP               1115
#define VCLEVENT_SPINFIELD_DOWN             1116
#define VCLEVENT_SPINFIELD_FIRST            1117
#define VCLEVENT_SPINFIELD_LAST             1118
#define VCLEVENT_STATUSBAR_CLICK            1119
#define VCLEVENT_STATUSBAR_DOUBLECLICK      1120
#define VCLEVENT_TOOLBOX_CLICK              1121
#define VCLEVENT_TOOLBOX_DOUBLECLICK        1122
#define VCLEVENT_TOOLBOX_ACTIVATE           1123
#define VCLEVENT_TOOLBOX_DEACTIVATE         1124
#define VCLEVENT_TOOLBOX_HIGHLIGHT          1125
#define VCLEVENT_TOOLBOX_SELECT             1126
// Resort later...
#define VCLEVENT_LISTBOX_SCROLLED           1127
#define VCLEVENT_COMBOBOX_SCROLLED          1128
#define VCLEVENT_EDIT_SELECTIONCHANGED      1129
#define VCLEVENT_DROPDOWN_OPEN              1130
#define VCLEVENT_DROPDOWN_CLOSE             1131

#define VCLEVENT_TOOLBOX_ITEMADDED          1132        // pData = itempos
#define VCLEVENT_TOOLBOX_ITEMREMOVED        1133        // pData = itempos
#define VCLEVENT_TOOLBOX_ALLITEMSCHANGED    1134
#define VCLEVENT_TOOLBOX_HIGHLIGHTOFF       1135        // pData = itempos
#define VCLEVENT_WINDOW_MENUBARADDED        1136        // pData = pMenuBar
#define VCLEVENT_TABPAGE_ACTIVATE           1137        // pData = pageid
#define VCLEVENT_TABPAGE_DEACTIVATE         1138        // pData = pageid
// free
// free
#define VCLEVENT_TABBAR_PAGESELECTED        1141        // pData = pageid
#define VCLEVENT_TABBAR_PAGEACTIVATED       1142        // pData = pageid
#define VCLEVENT_TABBAR_PAGEDEACTIVATED     1143        // pData = pageid
#define VCLEVENT_TABBAR_PAGEINSERTED        1144        // pData = pageid
#define VCLEVENT_TABBAR_PAGEREMOVED         1145        // pData = pageid
#define VCLEVENT_TABBAR_PAGEMOVED           1146        // pData = Pair( pagepos_old, pagepos_new )
#define VCLEVENT_TABBAR_PAGETEXTCHANGED     1147        // pData = pageid
#define VCLEVENT_COMBOBOX_DESELECT          1148
#define VCLEVENT_TOOLBOX_ITEMTEXTCHANGED    1149        // pData = itempos
#define VCLEVENT_TABPAGE_INSERTED           1150        // pData = pageid
#define VCLEVENT_TABPAGE_REMOVED            1151        // pData = pageid
#define VCLEVENT_TABPAGE_REMOVEDALL         1152
#define VCLEVENT_LISTBOX_ITEMADDED          1153        // pData = itempos
#define VCLEVENT_LISTBOX_ITEMREMOVED        1154        // pData = itempos, -1=ALL
#define VCLEVENT_COMBOBOX_ITEMADDED         1155        // pData = itempos
#define VCLEVENT_COMBOBOX_ITEMREMOVED       1156        // pData = itempos, -1=ALL
// free                                     1157
#define VCLEVENT_WINDOW_MENUBARREMOVED      1158        // pData = pMenuBar
#define VCLEVENT_STATUSBAR_ITEMADDED        1159        // pData = itemid
#define VCLEVENT_STATUSBAR_ITEMREMOVED      1160        // pData = itemid
#define VCLEVENT_STATUSBAR_ALLITEMSREMOVED  1161
#define VCLEVENT_STATUSBAR_SHOWITEM         1162        // pData = itemid
#define VCLEVENT_STATUSBAR_HIDEITEM         1163        // pData = itemid
#define VCLEVENT_STATUSBAR_SHOWALLITEMS     1164
// free
#define VCLEVENT_STATUSBAR_DRAWITEM         1166        // pData = itemid
#define VCLEVENT_STATUSBAR_NAMECHANGED      1167        // pData = itemid
#define VCLEVENT_TOOLBOX_ITEMENABLED        1168        // pData = itempos
#define VCLEVENT_TOOLBOX_ITEMDISABLED       1169        // pData = itempos
#define VCLEVENT_TABPAGE_PAGETEXTCHANGED    1170        // pData = pageid
#define VCLEVENT_ROADMAP_ITEMSELECTED       1171
#define VCLEVENT_TOOLBOX_FORMATCHANGED      1172        // request new layout
#define VCLEVENT_COMBOBOX_SETTEXT           1173
// #i92103#
#define VCLEVENT_ITEM_EXPANDED              1174
#define VCLEVENT_ITEM_COLLAPSED             1175
#define VCLEVENT_DROPDOWN_PRE_OPEN          1176
#define VCLEVENT_DROPDOWN_SELECT            1177
#define VCLEVENT_LISTBOX_FOCUSITEMCHANGED   1180

// VclMenuEvent
#define VCLEVENT_MENU_ACTIVATE              1200
#define VCLEVENT_MENU_DEACTIVATE            1201
#define VCLEVENT_MENU_HIGHLIGHT             1202
#define VCLEVENT_MENU_SELECT                1203
#define VCLEVENT_MENU_ENABLE                1204
#define VCLEVENT_MENU_INSERTITEM            1205
#define VCLEVENT_MENU_REMOVEITEM            1206
#define VCLEVENT_MENU_SUBMENUACTIVATE       1207
#define VCLEVENT_MENU_SUBMENUDEACTIVATE     1208
#define VCLEVENT_MENU_SUBMENUCHANGED        1209
#define VCLEVENT_MENU_DEHIGHLIGHT           1210
#define VCLEVENT_MENU_DISABLE               1211
#define VCLEVENT_MENU_ITEMTEXTCHANGED       1212
#define VCLEVENT_MENU_ITEMCHECKED           1213
#define VCLEVENT_MENU_ITEMUNCHECKED         1214
#define VCLEVENT_MENU_ACCESSIBLENAMECHANGED 1215
#define VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED  1216
#define VCLEVENT_TOOLBOX_ITEMUPDATED  1217

#define VCLEVENT_MENU_SHOW                  1250
#define VCLEVENT_MENU_HIDE                  1251

#define VCLEVENT_TOOLBOX_ITEMWINDOWCHANGED  1216
#define VCLEVENT_LISTBOX_TREEFOCUS              1220
#define VCLEVENT_LISTBOX_TREESELECT             1221
#define VCLEVENT_EDIT_CARETCHANGED  1222

#define VCLEVENT_LISTBOX_FOCUS             1224

// DockingWindow
#define VCLEVENT_WINDOW_STARTDOCKING            1227    // pData = DockingData
#define VCLEVENT_WINDOW_DOCKING                 1228
#define VCLEVENT_WINDOW_ENDDOCKING              1229    // pData = EndDockingData
#define VCLEVENT_WINDOW_PREPARETOGGLEFLOATING   1230    // pData = bool
#define VCLEVENT_WINDOW_TOGGLEFLOATING          1231
#define VCLEVENT_WINDOW_ENDPOPUPMODE            1232    // pData = EndPopupModeData

#define VCLEVENT_TOOLBOX_BUTTONSTATECHANGED     1233    // pData = itempos
#define VCLEVENT_TABLECELL_NAMECHANGED          1234    // pData = struct(Entry, Column, oldText)
#define VCLEVENT_TABLEROW_SELECT                1235
#define VCLEVENT_LISTBOX_STATEUPDATE            1236

class VCL_DLLPUBLIC VclSimpleEvent
{
private:
    sal_uLong nId;

public:
    VclSimpleEvent( sal_uLong n ) { nId = n; }
    virtual ~VclSimpleEvent() {}

    sal_uLong GetId() const { return nId; }
};

class VCL_DLLPUBLIC VclWindowEvent : public VclSimpleEvent
{
private:
    VclPtr<vcl::Window> pWindow;
    void*   pData;

public:
    VclWindowEvent( vcl::Window* pWin, sal_uLong n, void* pDat = nullptr );
    virtual ~VclWindowEvent();

    vcl::Window* GetWindow() const { return pWindow; }
    void*   GetData() const { return pData; }
};

class VCL_DLLPUBLIC VclMenuEvent : public VclSimpleEvent
{
private:
    Menu* pMenu;
    sal_uInt16 mnPos;

public:
    VclMenuEvent( Menu* pM, sal_uLong n, sal_uInt16 nPos ) : VclSimpleEvent(n) { pMenu = pM; mnPos = nPos; }
    virtual ~VclMenuEvent() {}

    Menu* GetMenu() const { return pMenu; }
    sal_uInt16 GetItemPos() const { return mnPos; }
};

class VCL_DLLPUBLIC VclAccessibleEvent: public VclSimpleEvent
{
public:
    VclAccessibleEvent( sal_uLong n, const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible );
    virtual ~VclAccessibleEvent();
    css::uno::Reference< css::accessibility::XAccessible > GetAccessible() const { return mxAccessible;}

private:
    css::uno::Reference< css::accessibility::XAccessible > mxAccessible;
};

#endif // INCLUDED_VCL_VCLEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
