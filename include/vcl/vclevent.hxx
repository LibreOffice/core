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
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

#include <com/sun/star/uno/Reference.hxx>

class Menu;

namespace com { namespace sun { namespace star {
    namespace accessibility {
        class XAccessible;
    }
}}}

enum class VclEventId
{
    NONE                        =    0,
    ObjectDying                 =    1,
// VclWindowEvent:
    WindowChildCreated          =  500,  // pData = vcl::Window*
    WindowChildDestroyed        =  501,  // pData = vcl::Window*
    WindowPaint                 = 1000,  // pData = Rectangle*
    WindowMove                  = 1001,
    WindowResize                = 1002,
    WindowShow                  = 1003,
    WindowHide                  = 1004,
    WindowActivate              = 1005,
    WindowDeactivate            = 1006,  // pData = vcl::Window* = pPrevActiveWindow
    WindowClose                 = 1007,
    WindowGetFocus              = 1008,
    WindowLoseFocus             = 1009,
    WindowMinimize              = 1010,
    WindowNormalize             = 1011,
    WindowKeyInput              = 1012,  // pData = KeyEvent*
    WindowKeyUp                 = 1013,  // pData = KeyEvent*
    WindowCommand               = 1014,  // pData = CommandEvent*
    WindowMouseMove             = 1015,  // pData = MouseEvent*
    WindowMouseButtonDown       = 1016,  // pData = MouseEvent*
    WindowMouseButtonUp         = 1017,  // pData = MouseEvent*
    WindowFrameTitleChanged     = 1018,  // pData = OUString* = oldTitle
    ApplicationDataChanged      = 1019,  // pData = DataChangedEvent*
    WindowEnabled               = 1020,
    WindowDisabled              = 1021,
    WindowDataChanged           = 1022,  // pData = DataChangedEvent*
    WindowZoom                  = 1023,  // pData = ZoomEvent*
    WindowScroll                = 1024,  // pData = ScrollEvent*
    ControlGetFocus             = 1100,
    ControlLoseFocus            = 1101,
    ButtonClick                 = 1102,
    PushbuttonToggle            = 1103,
    RadiobuttonToggle           = 1104,
    CheckboxToggle              = 1105,
    ComboboxSelect              = 1106,
    ComboboxDoubleClick         = 1107,
    ListboxSelect               = 1108,
    ListboxDoubleClick          = 1109,
    EditModify                  = 1110,
    ScrollbarScroll             = 1111,
    ScrollbarEndScroll          = 1112,
    SpinbuttonUp                = 1113,
    SpinbuttonDown              = 1114,
    SpinfieldUp                 = 1115,
    SpinfieldDown               = 1116,
    SpinfieldFirst              = 1117,
    SpinfieldLast               = 1118,
    StatusbarClick              = 1119,
    StatusbarDoubleClick        = 1120,
    ToolboxClick                = 1121,
    ToolboxDoubleClick          = 1122,
    ToolboxActivate             = 1123,
    ToolboxDeactivate           = 1124,
    ToolboxHighlight            = 1125,
    ToolboxSelect               = 1126,
// Resort later...
    ListboxScrolled             = 1127,
    ComboboxScrolled            = 1128,
    EditSelectionChanged        = 1129,
    DropdownOpen                = 1130,
    DropdownClose               = 1131,
    ToolboxItemAdded            = 1132,  // pData = itempos
    ToolboxItemRemoved          = 1133,  // pData = itempos
    ToolboxAllItemsChanged      = 1134,
    ToolboxHighlightOff         = 1135,  // pData = itempos
    WindowMenubarAdded          = 1136,  // pData = pMenuBar
    TabpageActivate             = 1137,  // pData = pageid
    TabpageDeactivate           = 1138,  // pData = pageid
// free
// free
    TabbarPageSelected          = 1141,  // pData = pageid
    TabbarPageActivated         = 1142,  // pData = pageid
    TabbarPageDeactivated       = 1143,  // pData = pageid
    TabbarPageInserted          = 1144,  // pData = pageid
    TabbarPageRemoved           = 1145,  // pData = pageid
    TabbarPageMoved             = 1146,  // pData = Pair( pagepos_old, pagepos_new )
    TabbarPageTextChanged       = 1147,  // pData = pageid
    ComboboxDeselect            = 1148,
    ToolboxItemTextChanged      = 1149,  // pData = itempos
    TabpageInserted             = 1150,  // pData = pageid
    TabpageRemoved              = 1151,  // pData = pageid
    TabpageRemovedAll           = 1152,
    ListboxItemAdded            = 1153,  // pData = itempos
    ListboxItemRemoved          = 1154,  // pData = itempos, -1=All
    ComboboxItemAdded           = 1155,  // pData = itempos
    ComboboxItemRemoved         = 1156,  // pData = itempos, -1=All
//free
    WindowMenubarRemoved        = 1158,  // pData = pMenuBar
    StatusbarItemAdded          = 1159,  // pData = itemid
    StatusbarItemRemoved        = 1160,  // pData = itemid
    StatusbarAllItemsRemoved    = 1161,
    StatusbarShowItem           = 1162,  // pData = itemid
    StatusbarHideItem           = 1163,  // pData = itemid
    StatusbarShowAllItems       = 1164,
// free
    StatusbarDrawItem           = 1166,  // pData = itemid
    StatusbarNameChanged        = 1167,  // pData = itemid
    ToolboxItemEnabled          = 1168,  // pData = itempos
    ToolboxItemDisabled         = 1169,  // pData = itempos
    TabpagePageTextChanged      = 1170,  // pData = pageid
    RoadmapItemSelected         = 1171,
    ToolboxFormatChanged        = 1172,  // request new layout
    ComboboxSetText             = 1173,
// #i92103#
    ItemExpanded                = 1174,
    ItemCollapsed               = 1175,
    DropdownPreOpen             = 1176,
    DropdownSelect              = 1177,
    ListboxFocusItemChanged     = 1180,

// VclMenuEvent
    MenuActivate                = 1200,
    MenuDeactivate              = 1201,
    MenuHighlight               = 1202,
    MenuSelect                  = 1203,
    MenuEnable                  = 1204,
    MenuInsertItem              = 1205,
    MenuRemoveItem              = 1206,
    MenuSubmenuActivate         = 1207,
    MenuSubmenuDeactivate       = 1208,
    MenuSubmenuChanged          = 1209,
    MenuDehighlight             = 1210,
    MenuDisable                 = 1211,
    MenuItemTextChanged         = 1212,
    MenuItemChecked             = 1213,
    MenuItemUnchecked           = 1214,
    MenuAccessibleNameChanged   = 1215,
    ToolboxItemWindowChanged    = 1216,
    ToolboxItemUpdated          = 1217,

    MenuShow                    = 1250,
    MenuHide                    = 1251,

    ListboxTreeFocus            = 1220,
    ListboxTreeSelect           = 1221,
    EditCaretChanged            = 1222,

    ListboxFocus                = 1224,

// DockingWindow
    WindowStartDocking          = 1227,   // pData = DockingData
    WindowDocking               = 1228,
    WindowEndDocking            = 1229,   // pData = EndDockingData
    WindowPrepareToggleFloating = 1230,   // pData = bool
    WindowToggleFloating        = 1231,
    WindowEndPopupMode          = 1232,   // pData = EndPopupModeData

    ToolboxButtonStateChanged   = 1233,   // pData = itempos
    TableCellNameChanged        = 1234,   // pData = struct(Entry, Column, oldText)
    TableRowSelect              = 1235,
    ListboxStateUpdate          = 1236,
};

class VCL_DLLPUBLIC VclSimpleEvent
{
private:
    VclEventId nId;

public:
    VclSimpleEvent( VclEventId n ) { nId = n; }
    virtual ~VclSimpleEvent() {}

    VclEventId GetId() const { return nId; }
};

class VCL_DLLPUBLIC VclWindowEvent : public VclSimpleEvent
{
private:
    VclPtr<vcl::Window> pWindow;
    void*   pData;

public:
    VclWindowEvent( vcl::Window* pWin, VclEventId n, void* pDat );
    virtual ~VclWindowEvent() override;

    vcl::Window* GetWindow() const { return pWindow; }
    void*   GetData() const { return pData; }
};

class VCL_DLLPUBLIC VclMenuEvent : public VclSimpleEvent
{
private:
    VclPtr<Menu> pMenu;
    sal_uInt16 mnPos;

    VclMenuEvent(VclMenuEvent &) = delete;
    void operator =(VclMenuEvent) = delete;

public:
    VclMenuEvent( Menu* pM, VclEventId n, sal_uInt16 nPos );
    virtual ~VclMenuEvent() override;

    Menu* GetMenu() const;
    sal_uInt16 GetItemPos() const { return mnPos; }
};

class VCL_DLLPUBLIC VclAccessibleEvent: public VclSimpleEvent
{
public:
    VclAccessibleEvent( VclEventId n, const css::uno::Reference< css::accessibility::XAccessible >& rxAccessible );
    virtual ~VclAccessibleEvent() override;
    const css::uno::Reference< css::accessibility::XAccessible >& GetAccessible() const { return mxAccessible;}

private:
    css::uno::Reference< css::accessibility::XAccessible > mxAccessible;
};

#endif // INCLUDED_VCL_VCLEVENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
