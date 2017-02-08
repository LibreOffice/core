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
    NONE,
    ApplicationDataChanged,  // pData = DataChangedEvent*
    ButtonClick,
    CheckboxToggle,
    ComboboxDeselect,
    ComboboxDoubleClick,
    ComboboxItemAdded,       // pData = itempos
    ComboboxItemRemoved,     // pData = itempos, -1=All
    ComboboxSelect,
    ComboboxSetText,
    ControlGetFocus,
    ControlLoseFocus,
    DropdownClose,
    DropdownOpen,
    DropdownPreOpen,
    DropdownSelect,
    EditCaretChanged,
    EditModify,
    EditSelectionChanged,
    ItemCollapsed,
    ItemExpanded,
    ListboxDoubleClick,
    ListboxFocus,
    ListboxItemAdded,        // pData = itempos
    ListboxItemRemoved,      // pData = itempos, -1=All
    ListboxScrolled,
    ListboxSelect,
    ListboxStateUpdate,
    ListboxTreeFocus,
    ListboxTreeSelect,
    MenuActivate,
    MenuDeactivate,
    MenuDehighlight,
    MenuDisable,
    MenuEnable,
    MenuHide,
    MenuHighlight,
    MenuInsertItem,
    MenuItemChecked,
    MenuItemTextChanged,
    MenuItemUnchecked,
    MenuRemoveItem,
    MenuSelect,
    MenuShow,
    MenuSubmenuActivate,
    MenuSubmenuChanged,
    MenuSubmenuDeactivate,
    ObjectDying,
    PushbuttonToggle,
    RadiobuttonToggle,
    RoadmapItemSelected,
    ScrollbarEndScroll,
    ScrollbarScroll,
    SpinbuttonDown,
    SpinbuttonUp,
    SpinfieldDown,
    SpinfieldFirst,
    SpinfieldLast,
    SpinfieldUp,
    StatusbarAllItemsRemoved,
    StatusbarClick,
    StatusbarDoubleClick,
    StatusbarDrawItem,      // pData = itemid
    StatusbarHideItem,      // pData = itemid
    StatusbarItemAdded,     // pData = itemid
    StatusbarItemRemoved,   // pData = itemid
    StatusbarNameChanged,   // pData = itemid
    StatusbarShowItem,      // pData = itemid
    TabbarPageActivated,    // pData = pageid
    TabbarPageDeactivated,  // pData = pageid
    TabbarPageInserted,     // pData = pageid
    TabbarPageMoved,        // pData = Pair( pagepos_old, pagepos_new )
    TabbarPageRemoved,      // pData = pageid
    TabbarPageSelected,     // pData = pageid
    TabbarPageTextChanged,  // pData = pageid
    TableCellNameChanged,   // pData = struct(Entry, Column, oldText)
    TableRowSelect,
    TabpageActivate,        // pData = pageid
    TabpageDeactivate,      // pData = pageid
    TabpageInserted,        // pData = pageid
    TabpagePageTextChanged, // pData = pageid
    TabpageRemoved,         // pData = pageid
    TabpageRemovedAll,
    ToolboxActivate,
    ToolboxAllItemsChanged,
    ToolboxButtonStateChanged, // pData = itempos
    ToolboxClick,
    ToolboxDeactivate,
    ToolboxDoubleClick,
    ToolboxFormatChanged,   // request new layout
    ToolboxHighlight,
    ToolboxHighlightOff,    // pData = itempos
    ToolboxItemAdded,       // pData = itempos
    ToolboxItemDisabled,    // pData = itempos
    ToolboxItemEnabled,     // pData = itempos
    ToolboxItemRemoved,     // pData = itempos
    ToolboxItemTextChanged, // pData = itempos
    ToolboxItemUpdated,
    ToolboxItemWindowChanged,
    ToolboxSelect,
    WindowActivate,
    WindowChildCreated,     // pData = vcl::Window*
    WindowChildDestroyed,   // pData = vcl::Window*
    WindowClose,
    WindowCommand,          // pData = CommandEvent*
    WindowDataChanged,      // pData = DataChangedEvent*
    WindowDeactivate,       // pData = vcl::Window* = pPrevActiveWindow
    WindowDisabled,
    WindowDocking,
    WindowEnabled,
    WindowEndDocking,       // pData = EndDockingData
    WindowEndPopupMode,     // pData = EndPopupModeData
    WindowFrameTitleChanged,// pData = OUString* = oldTitle
    WindowGetFocus,
    WindowHide,
    WindowKeyInput,         // pData = KeyEvent*
    WindowKeyUp,            // pData = KeyEvent*
    WindowLoseFocus,
    WindowMenubarAdded,     // pData = pMenuBar
    WindowMenubarRemoved,   // pData = pMenuBar
    WindowMinimize,
    WindowMouseButtonDown,  // pData = MouseEvent*
    WindowMouseButtonUp,    // pData = MouseEvent*
    WindowMouseMove,        // pData = MouseEvent*
    WindowMove,
    WindowNormalize,
    WindowPaint,            // pData = Rectangle*
    WindowPrepareToggleFloating,   // pData = bool
    WindowResize,
    WindowShow,
    WindowStartDocking,     // pData = DockingData
    WindowToggleFloating,
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
