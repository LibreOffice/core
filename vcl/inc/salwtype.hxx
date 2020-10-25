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

#ifndef INCLUDED_VCL_INC_SALWTYPE_HXX
#define INCLUDED_VCL_INC_SALWTYPE_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/long.hxx>
#include <vcl/GestureEvent.hxx>

class LogicalFontInstance;
class SalGraphics;
class SalFrame;
class SalObject;
namespace vcl { class Window; }
enum class InputContextFlags;
enum class WindowStateMask;
enum class WindowStateState;
enum class ExtTextInputAttr;
enum class ModKeyFlags;

enum class SalEvent {
    NONE,
    MouseMove,
    MouseLeave,
    MouseButtonDown,
    MouseButtonUp,
    KeyInput,
    KeyUp,
    KeyModChange,
    Paint,
    Resize,
    GetFocus,
    LoseFocus,
    Close,
    Shutdown,
    SettingsChanged,
    PrinterChanged,
    DisplayChanged,
    FontChanged,
    WheelMouse,
    UserEvent,
    MouseActivate,
    ExtTextInput,
    EndExtTextInput,
    ExtTextInputPos,
    InputContextChange,
    Move,
    MoveResize,
    ClosePopups,
    ExternalKeyInput,
    ExternalKeyUp,
    MenuCommand,
    MenuHighlight,
    MenuActivate,
    MenuDeactivate,
    ExternalMouseMove,
    ExternalMouseButtonDown,
    ExternalMouseButtonUp,
    InputLanguageChange,
    ShowDialog,
    MenuButtonCommand,
    SurroundingTextRequest,
    DeleteSurroundingTextRequest,
    SurroundingTextSelectionChange,
    StartReconversion,
    QueryCharPosition,
    Swipe,
    LongPress,
    ExternalGesture,
    Gesture,
};

struct SalAbstractMouseEvent
{
    sal_uInt64 mnTime;  // Time in ms, when event is created
    tools::Long mnX;           // X-Position (Pixel, TopLeft-Output)
    tools::Long mnY;           // Y-Position (Pixel, TopLeft-Output)
    sal_uInt16 mnCode;  // SV-Modifiercode (KEY_SHIFT|KEY_MOD1|KEY_MOD2|MOUSE_LEFT|MOUSE_MIDDLE|MOUSE_RIGHT)

protected:
    SalAbstractMouseEvent() : mnTime(0), mnX(0), mnY(0), mnCode(0) {}
};

// MOUSELEAVE must send, when the pointer leave the client area and
// the mouse is not captured
// MOUSEMOVE, MOUSELEAVE, MOUSEBUTTONDOWN and MOUSEBUTTONUP
// MAC: Ctrl+Button is MOUSE_RIGHT
struct SalMouseEvent final : public SalAbstractMouseEvent
{
    sal_uInt16      mnButton;       // 0-MouseMove/MouseLeave, MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE

    SalMouseEvent() : mnButton(0) {}
};

// KEYINPUT and KEYUP
struct SalKeyEvent
{
    sal_uInt16      mnCode;         // SV-KeyCode (KEY_xxx | KEY_SHIFT | KEY_MOD1 | KEY_MOD2)
    sal_uInt16      mnCharCode;     // SV-CharCode
    sal_uInt16      mnRepeat;       // Repeat-Count (KeyInputs-1)
};

// MENUEVENT
struct SalMenuEvent
{
    sal_uInt16      mnId;           // Menu item ID
    void*           mpMenu;         // pointer to VCL menu (class Menu)

   SalMenuEvent() : mnId( 0 ), mpMenu( nullptr ) {}
   SalMenuEvent( sal_uInt16 i_nId, void* i_pMenu )
    : mnId( i_nId ), mpMenu( i_pMenu ) {}
};

// KEYMODCHANGE
struct SalKeyModEvent
{
    bool            mbDown;         // Whether the change occurred on a key down event
    sal_uInt16      mnCode;         // SV-Modifiercode (KEY_SHIFT|KEY_MOD1|KEY_MOD2)
    ModKeyFlags     mnModKeyCode;   // extended Modifier (MODKEY_LEFT,MODKEY_RIGHT,MODKEY_PRESS,MODKEY_RELEASE)
};

struct SalPaintEvent
{
    tools::Long            mnBoundX;           // BoundRect - X
    tools::Long            mnBoundY;           // BoundRect - Y
    tools::Long            mnBoundWidth;       // BoundRect - Width
    tools::Long            mnBoundHeight;      // BoundRect - Height
    bool            mbImmediateUpdate;  // set to true to force an immediate update

    SalPaintEvent( tools::Long x, tools::Long y, tools::Long w, tools::Long h, bool bImmediate = false ) :
        mnBoundX( x ), mnBoundY( y ),
        mnBoundWidth( w ), mnBoundHeight( h ),
        mbImmediateUpdate( bImmediate )
    {}
};

#define SAL_WHEELMOUSE_EVENT_PAGESCROLL     (sal_uLong(0xFFFFFFFF))
struct SalWheelMouseEvent final : public SalAbstractMouseEvent
{
    tools::Long            mnDelta;        // Number of rotations
    tools::Long            mnNotchDelta;   // Number of fixed rotations
    double          mnScrollLines;  // Actual number of lines to scroll
    bool        mbHorz;         // Horizontal
    bool        mbDeltaIsPixel; // delta value is a pixel value (on touch devices)

    SalWheelMouseEvent()
    : mnDelta(0), mnNotchDelta(0), mnScrollLines(0), mbHorz(false), mbDeltaIsPixel(false)
    {}
};

struct SalExtTextInputEvent
{
    OUString            maText;         // Text
    const ExtTextInputAttr* mpTextAttr;     // Text-Attribute
    sal_Int32           mnCursorPos;    // Cursor-Position
    sal_uInt8           mnCursorFlags;  // EXTTEXTINPUT_CURSOR_xxx
};

struct SalExtTextInputPosEvent
{
    tools::Long            mnX;            // Cursor-X-Position to upper left corner of frame
    tools::Long            mnY;            // Cursor-Y-Position to upper left corner of frame
    tools::Long            mnWidth;        // Cursor-Width in Pixel
    tools::Long            mnHeight;       // Cursor-Height in Pixel
    tools::Long            mnExtWidth;     // Width of the PreEdit area
    bool            mbVertical;     // true if in vertical mode
    SalExtTextInputPosEvent()
        : mnX(0)
        , mnY(0)
        , mnWidth(0)
        , mnHeight(0)
        , mnExtWidth(0)
        , mbVertical(false)
    {
    }
};

struct SalInputContextChangeEvent
{
};

struct SalSurroundingTextRequestEvent
{
    OUString        maText;         // Text
    sal_uLong       mnStart;        // The beginning index of selected range
    sal_uLong       mnEnd;          // The end index of selected range
};

struct SalSurroundingTextSelectionChangeEvent
{
    sal_uLong       mnStart;        // The beginning index of selected range
    sal_uLong       mnEnd;          // The end index of selected range
};

struct SalQueryCharPositionEvent
{
    bool            mbValid;                // The data is valid or not.
    sal_uLong       mnCharPos;              // The index of character in a composition.
    bool            mbVertical;             // The text is vertical or not.
    tools::Long            mnCursorBoundX;         // The cursor bounds corresponding to the character specified by mnCharPos - X
    tools::Long            mnCursorBoundY;         // The cursor bounds corresponding to the character specified by mnCharPos - Y
    tools::Long            mnCursorBoundWidth;     // The cursor bounds corresponding to the character specified by mnCharPos - Width
    tools::Long            mnCursorBoundHeight;    // The cursor bounds corresponding to the character specified by mnCharPos - Height
};

typedef bool (*SALFRAMEPROC)( vcl::Window* pInst, SalEvent nEvent, const void* pEvent );

enum class SalObjEvent {
    GetFocus           = 1,
    LoseFocus          = 2,
    ToTop              = 3
};

struct SalFrameState
{
    WindowStateMask mnMask;
    tools::Long            mnX;
    tools::Long            mnY;
    tools::Long            mnWidth;
    tools::Long            mnHeight;
    tools::Long            mnMaximizedX;
    tools::Long            mnMaximizedY;
    tools::Long            mnMaximizedWidth;
    tools::Long            mnMaximizedHeight;
    WindowStateState mnState;
};

struct SalInputContext
{
    rtl::Reference<LogicalFontInstance> mpFont;
    InputContextFlags      mnOptions;
};

struct SalSwipeEvent
{
    double mnVelocityX;
    double mnVelocityY;
    tools::Long mnX;
    tools::Long mnY;
};

struct SalLongPressEvent
{
    tools::Long mnX;
    tools::Long mnY;
};

struct SalGestureEvent
{
    GestureEventType meEventType;
    PanningOrientation meOrientation;
    double mfOffset;
    tools::Long mnX;
    tools::Long mnY;
};

typedef void (*SALTIMERPROC)();

#endif // INCLUDED_VCL_INC_SALWTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
