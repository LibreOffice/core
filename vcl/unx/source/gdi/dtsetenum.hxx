/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dtsetenum.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
