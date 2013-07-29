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

#ifndef _SV_TOOLBOX_H
#define _SV_TOOLBOX_H

#include <tools/gen.hxx>
#include <tools/string.hxx>
#include <tools/solar.h>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/ImageListProvider.hxx>

#include <vector>

#define TB_DROPDOWNARROWWIDTH   11

#define TB_MENUBUTTON_SIZE      12
#define TB_MENUBUTTON_OFFSET    2

#define TB_SMALLIMAGESIZE       16

#define TB_LARGEIMAGESIZE            26
#define TB_LARGEIMAGESIZE_INDUSTRIAL 24
#define TB_LARGEIMAGESIZE_CRYSTAL    22
#define TB_LARGEIMAGESIZE_OXYGEN     22

class Window;

// ----------------
// - ImplToolItem -
// ----------------

struct ImplToolItem
{
    Window*             mpWindow;
    void*               mpUserData;
    Image               maImage;
    Image               maHighImage;
    long                mnImageAngle;
    bool                mbMirrorMode;
    OUString            maText;
    OUString            maQuickHelpText;
    OUString            maHelpText;
    OUString            maCommandStr;
    OString             maHelpId;
    Rectangle           maRect;
    Rectangle           maCalcRect;
    /// Widget layout may request size; set it as the minimal size (like, the item will always have at least this size).
    Size                maMinimalItemSize;
    /// The overall horizontal item size, including one or more of [image size + textlength + dropdown arrow]
    Size                maItemSize;
    long                mnSepSize;
    long                mnDropDownArrowWidth;
    /// Size of the content (bitmap or text, without dropdown) that we have in the item.
    Size                maContentSize;
    ToolBoxItemType     meType;
    ToolBoxItemBits     mnBits;
    TriState            meState;
    sal_uInt16              mnId;
    sal_Bool                mbEnabled:1,
                        mbVisible:1,
                        mbEmptyBtn:1,
                        mbShowWindow:1,
                        mbBreak:1,
                        mbVisibleText:1;    // indicates if text will definitely be drawn, influences dropdown pos

                        ImplToolItem();
                        ImplToolItem( sal_uInt16 nItemId, const Image& rImage,
                                      ToolBoxItemBits nItemBits );
                        ImplToolItem( sal_uInt16 nItemId, const OUString& rTxt,
                                      ToolBoxItemBits nItemBits );
                        ImplToolItem( sal_uInt16 nItemId, const Image& rImage,
                                      const OUString& rTxt,
                                      ToolBoxItemBits nItemBits );
                        ~ImplToolItem();

    ImplToolItem( const ImplToolItem& );
    ImplToolItem& operator=(const ImplToolItem&);

    // returns the size of a item, taking toolbox orientation into account
    // the default size is the precomputed size for standard items
    // ie those that are just ordinary buttons (no windows or text etc.)
    // bCheckMaxWidth indicates that item windows must not exceed maxWidth in which case they will be painted as buttons
    Size                GetSize( sal_Bool bHorz, sal_Bool bCheckMaxWidth, long maxWidth, const Size& rDefaultSize );

    // only useful for buttons: returns if the text or image part or both can be drawn according to current button drawing style
    void DetermineButtonDrawStyle( ButtonType eButtonType, sal_Bool& rbImage, sal_Bool& rbText ) const;

    // returns the rectangle which contains the drop down arrow
    // or an empty rect if there is none
    // bHorz denotes the toolbox alignment
    Rectangle   GetDropDownRect( sal_Bool bHorz ) const;

    // returns sal_True if the toolbar item is currently clipped, which can happen for docked toolbars
    sal_Bool IsClipped() const;

private:
    void init(sal_uInt16 nItemId, ToolBoxItemBits nItemBits, sal_Bool bEmptyBtn);
};

namespace vcl
{

struct ToolBoxLayoutData : public ControlLayoutData
{
    std::vector< sal_uInt16 >               m_aLineItemIds;
    std::vector< sal_uInt16 >               m_aLineItemPositions;
};


} /* namespace vcl */


struct ImplToolBoxPrivateData
{
    vcl::ToolBoxLayoutData*         m_pLayoutData;
    std::vector< ImplToolItem >     m_aItems;

    ImplToolBoxPrivateData();
    ~ImplToolBoxPrivateData();

    void ImplClearLayoutData() { delete m_pLayoutData; m_pLayoutData = NULL; }

    // called when dropdown items are clicked
    Link    maDropdownClickHdl;
    Timer   maDropdownTimer; // for opening dropdown items on "long click"

    // large or small buttons ?
    ToolBoxButtonSize   meButtonSize;

    // the optional custom menu
    PopupMenu*  mpMenu;
    sal_uInt16      maMenuType;
    sal_uIntPtr       mnEventId;

    // called when menu button is clicked and before the popup menu is executed
    Link        maMenuButtonHdl;

    // a dummy item representing the custom menu button
    ImplToolItem   maMenubuttonItem;
    long           mnMenuButtonWidth;

    Wallpaper   maDisplayBackground;

    // support for highcontrast
    vcl::IImageListProvider* mpImageListProvider;
    vcl::ImageListType       meImageListType;

    sal_Bool    mbIsLocked:1,           // keeps last lock state from ImplDockingWindowWrapper
            mbAssumeDocked:1,       // only used during calculations to override current floating/popup mode
            mbAssumeFloating:1,
            mbAssumePopupMode:1,
            mbKeyInputDisabled:1,   // no KEY input if all items disabled, closing/docking will be allowed though
            mbIsPaintLocked:1,      // don't allow paints
            mbMenubuttonSelected:1, // menu button is highlighted
            mbPageScroll:1,         // determines if we scroll a page at a time
            mbNativeButtons:1,      // system supports native toolbar buttons
            mbWillUsePopupMode:1,   // this toolbox will be opened in popup mode
            mbDropDownByKeyboard:1; // tells whether a dropdown was started by key input
};


#endif  // _SV_TOOLBOX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
