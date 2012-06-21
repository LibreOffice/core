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

#ifndef _VCL_DTSETENUM_HXX
#define _VCL_DTSETENUM_HXX

enum DtSetEnum
{
    /* settings for mouse */
    MouseOptions = 1,
    DoubleClickTime,
    DoubleClickWidth,
    DoubleClickHeight,
    StartDragWidth,
    StartDragHeight,
    DragMoveCode,
    DragCopyCode,
    DragLinkCode,
    ContextMenuCode,
    ContextMenuClicks,
    ContextMenuDown,
    ScrollRepeat,
    ButtonStartRepeat,
    ButtonRepeat,
    ActionDelay,
    MenuDelay,
    Follow,
    MiddleButtonAction,
    /* settings for keyboard */
    KeyboardOptions=64,
    /* style settings */
    StyleOptions = 128,
    BorderSize,
    TitleHeight,
    FloatTitleHeight,
    TearOffTitleHeight,
    MenuBarHeight,
    ScrollBarSize,
    SpinSize,
    SplitSize,
    IconHorzSpace,
    IconVertSpace,
    CursorSize,
    CursorBlinkTime,
    ScreenZoom,
    ScreenFontZoom,
    LogoDisplayTime,
    DragFullOptions,
    AnimationOptions,
    SelectionOptions,
    DisplayOptions,
    AntialiasingMinPixelHeight,
    /* style colors */
    AllTextColors, /* convenience, sets all control text colors */
    AllBackColors, /* convenience, sets all control background colors */
    ThreeDColor,
    FaceColor,
    CheckedColor,
    LightColor,
    LightBorderColor,
    ShadowColor,
    DarkShadowColor,
    ButtonTextColor,
    RadioCheckTextColor,
    GroupTextColor,
    LabelTextColor,
    InfoTextColor,
    WindowColor,
    WindowTextColor,
    DialogColor,
    DialogTextColor,
    WorkspaceColor,
    FieldColor,
    FieldTextColor,
    ActiveColor,
    ActiveColor2,
    ActiveTextColor,
    ActiveBorderColor,
    DeactiveColor,
    DeactiveColor2,
    DeactiveTextColor,
    DeactiveBorderColor,
    HighlightColor,
    HighlightTextColor,
    DisableColor,
    HelpColor,
    HelpTextColor,
    MenuColor,
    MenuBarColor,
    MenuTextColor,
    MenuHighlightColor,
    MenuHighlightTextColor,
    LinkColor,
    VisitedLinkColor,
    HighlightLinkColor,
    HighContrastMode,
    /* style fonts */
    UIFont, /* convenience, sets all fonts but TitleFont and FloatTitleFont */
    AppFont,
    HelpFont,
    TitleFont,
    FloatTitleFont,
    MenuFont,
    ToolFont,
    GroupFont,
    LabelFont,
    InfoFont,
    RadioCheckFont,
    PushButtonFont,
    FieldFont,
    IconFont,
    /* style numeric styles */
    RadioButtonStyle,
    CheckBoxStyle,
    PushButtonStyle,
    TabControlStyle,
    /* toolbar style */
    ToolbarIconSize

};

#endif // _VCL_DTSETENUM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
