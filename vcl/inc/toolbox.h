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

#ifndef INCLUDED_VCL_INC_TOOLBOX_H
#define INCLUDED_VCL_INC_TOOLBOX_H

#include <vcl/toolbox.hxx>
#include <vcl/toolkit/controllayout.hxx>

#include <vector>

#define TB_DROPDOWNARROWWIDTH   11

#define TB_MENUBUTTON_SIZE      12
#define TB_MENUBUTTON_OFFSET    2

namespace vcl { class Window; }

struct ImplToolItem
{
    VclPtr<vcl::Window> mpWindow; //don't dispose mpWindow - we get copied around
    bool                mbNonInteractiveWindow;
    void*               mpUserData;
    Image               maImage;
    Degree10            mnImageAngle;
    bool                mbMirrorMode;
    OUString            maText;
    OUString            maQuickHelpText;
    OUString            maHelpText;
    OUString            maCommandStr;
    OString             maHelpId;
    tools::Rectangle           maRect;
    tools::Rectangle           maCalcRect;
    /// Widget layout may request size; set it as the minimal size (like, the item will always have at least this size).
    Size                maMinimalItemSize;
    /// The overall horizontal item size, including one or more of [image size + textlength + dropdown arrow]
    Size                maItemSize;
    tools::Long                mnSepSize;
    tools::Long                mnDropDownArrowWidth;
    /// Size of the content (bitmap or text, without dropdown) that we have in the item.
    Size                maContentSize;
    ToolBoxItemType     meType;
    ToolBoxItemBits     mnBits;
    TriState            meState;
    ToolBoxItemId       mnId;
    bool                mbEnabled:1,
                        mbVisible:1,
                        mbEmptyBtn:1,
                        mbShowWindow:1,
                        mbBreak:1,
                        mbVisibleText:1,    // indicates if text will definitely be drawn, influences dropdown pos
                        mbExpand:1;

                        ImplToolItem();
                        ImplToolItem( ToolBoxItemId nItemId, const Image& rImage,
                                      ToolBoxItemBits nItemBits );
                        ImplToolItem( ToolBoxItemId nItemId, const OUString& rTxt,
                                      ToolBoxItemBits nItemBits );
                        ImplToolItem( ToolBoxItemId nItemId, const Image& rImage,
                                      const OUString& rTxt,
                                      ToolBoxItemBits nItemBits );

    // returns the size of an item, taking toolbox orientation into account
    // the default size is the precomputed size for standard items
    // ie those that are just ordinary buttons (no windows or text etc.)
    // bCheckMaxWidth indicates that item windows must not exceed maxWidth in which case they will be painted as buttons
    Size                GetSize( bool bHorz, bool bCheckMaxWidth, tools::Long maxWidth, const Size& rDefaultSize );

    // only useful for buttons: returns if the text or image part or both can be drawn according to current button drawing style
    void DetermineButtonDrawStyle( ButtonType eButtonType, bool& rbImage, bool& rbText ) const;

    // returns the rectangle which contains the drop down arrow
    // or an empty rect if there is none
    // bHorz denotes the toolbox alignment
    tools::Rectangle   GetDropDownRect( bool bHorz ) const;

    // returns sal_True if the toolbar item is currently clipped, which can happen for docked toolbars
    bool IsClipped() const;

    // returns sal_True if the toolbar item is currently hidden i.e. they are unchecked in the toolbar Customize menu
    bool IsItemHidden() const;

private:
    void init(ToolBoxItemId nItemId, ToolBoxItemBits nItemBits, bool bEmptyBtn);
};

namespace vcl
{

struct ToolBoxLayoutData : public ControlLayoutData
{
    std::vector< ToolBoxItemId >               m_aLineItemIds;
};

} /* namespace vcl */

struct ImplToolBoxPrivateData
{
    std::unique_ptr<vcl::ToolBoxLayoutData> m_pLayoutData;
    ToolBox::ImplToolItems          m_aItems;

    ImplToolBoxPrivateData();
    ~ImplToolBoxPrivateData();

    void ImplClearLayoutData() { m_pLayoutData.reset(); }

    // called when dropdown items are clicked
    Link<ToolBox *, void> maDropdownClickHdl;
    Timer   maDropdownTimer; // for opening dropdown items on "long click"

    // large or small buttons ?
    ToolBoxButtonSize   meButtonSize;

    // the optional custom menu
    VclPtr<PopupMenu>   mpMenu;
    ToolBoxMenuType maMenuType;

    // called when menu button is clicked and before the popup menu is executed
    Link<ToolBox *, void> maMenuButtonHdl;

    // a dummy item representing the custom menu button
    ImplToolItem   maMenubuttonItem;
    tools::Long           mnMenuButtonWidth;

    Wallpaper   maDisplayBackground;

    bool    mbIsLocked:1,           // keeps last lock state from ImplDockingWindowWrapper
            mbAssumeDocked:1,       // only used during calculations to override current floating/popup mode
            mbAssumeFloating:1,
            mbAssumePopupMode:1,
            mbKeyInputDisabled:1,   // no KEY input if all items disabled, closing/docking will be allowed though
            mbIsPaintLocked:1,      // don't allow paints
            mbMenubuttonSelected:1, // menu button is highlighted
            mbMenubuttonWasLastSelected:1, // menu button was highlighted when focus was lost
            mbNativeButtons:1,      // system supports native toolbar buttons
            mbWillUsePopupMode:1,   // this toolbox will be opened in popup mode
            mbDropDownByKeyboard:1; // tells whether a dropdown was started by key input
};

#endif // INCLUDED_VCL_INC_TOOLBOX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
