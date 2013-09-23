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

#ifndef _SV_SALWTYPE_HXX
#define _SV_SALWTYPE_HXX

#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>

class SalGraphics;
class SalFrame;
class SalObject;
class Window;

class FontSelectPattern;

// ------------
// - SalEvent -
// ------------

#define SALEVENT_MOUSEMOVE              ((sal_uInt16)1)
#define SALEVENT_MOUSELEAVE             ((sal_uInt16)2)
#define SALEVENT_MOUSEBUTTONDOWN        ((sal_uInt16)3)
#define SALEVENT_MOUSEBUTTONUP          ((sal_uInt16)4)
#define SALEVENT_KEYINPUT               ((sal_uInt16)5)
#define SALEVENT_KEYUP                  ((sal_uInt16)6)
#define SALEVENT_KEYMODCHANGE           ((sal_uInt16)7)
#define SALEVENT_PAINT                  ((sal_uInt16)8)
#define SALEVENT_RESIZE                 ((sal_uInt16)9)
#define SALEVENT_GETFOCUS               ((sal_uInt16)10)
#define SALEVENT_LOSEFOCUS              ((sal_uInt16)11)
#define SALEVENT_CLOSE                  ((sal_uInt16)12)
#define SALEVENT_SHUTDOWN               ((sal_uInt16)13)
#define SALEVENT_SETTINGSCHANGED        ((sal_uInt16)14)
#define SALEVENT_VOLUMECHANGED          ((sal_uInt16)15)
#define SALEVENT_PRINTERCHANGED         ((sal_uInt16)16)
#define SALEVENT_DISPLAYCHANGED         ((sal_uInt16)17)
#define SALEVENT_FONTCHANGED            ((sal_uInt16)18)
#define SALEVENT_DATETIMECHANGED        ((sal_uInt16)19)
#define SALEVENT_KEYBOARDCHANGED        ((sal_uInt16)20)
#define SALEVENT_WHEELMOUSE             ((sal_uInt16)21)
#define SALEVENT_USEREVENT              ((sal_uInt16)22)
#define SALEVENT_MOUSEACTIVATE          ((sal_uInt16)23)
#define SALEVENT_EXTTEXTINPUT           ((sal_uInt16)24)
#define SALEVENT_ENDEXTTEXTINPUT        ((sal_uInt16)25)
#define SALEVENT_EXTTEXTINPUTPOS        ((sal_uInt16)26)
#define SALEVENT_INPUTCONTEXTCHANGE     ((sal_uInt16)27)
#define SALEVENT_MOVE                   ((sal_uInt16)28)
#define SALEVENT_MOVERESIZE             ((sal_uInt16)29)
#define SALEVENT_CLOSEPOPUPS            ((sal_uInt16)30)
#define SALEVENT_EXTERNALKEYINPUT       ((sal_uInt16)31)
#define SALEVENT_EXTERNALKEYUP          ((sal_uInt16)32)
#define SALEVENT_MENUCOMMAND            ((sal_uInt16)33)
#define SALEVENT_MENUHIGHLIGHT          ((sal_uInt16)34)
#define SALEVENT_MENUACTIVATE           ((sal_uInt16)35)
#define SALEVENT_MENUDEACTIVATE         ((sal_uInt16)36)
#define SALEVENT_EXTERNALMOUSEMOVE      ((sal_uInt16)37)
#define SALEVENT_EXTERNALMOUSEBUTTONDOWN ((sal_uInt16)38)
#define SALEVENT_EXTERNALMOUSEBUTTONUP  ((sal_uInt16)39)
#define SALEVENT_INPUTLANGUAGECHANGE    ((sal_uInt16)40)
#define SALEVENT_SHOWDIALOG             ((sal_uInt16)41)
#define SALEVENT_MENUBUTTONCOMMAND      ((sal_uInt16)42)
#define SALEVENT_SURROUNDINGTEXTREQUEST ((sal_uInt16)43)
#define SALEVENT_SURROUNDINGTEXTSELECTIONCHANGE ((sal_uInt16)44)
#define SALEVENT_STARTRECONVERSION      ((sal_uInt16)45)
#define SALEVENT_EXTERNALZOOM           ((sal_uInt16)46)
#define SALEVENT_EXTERNALSCROLL         ((sal_uInt16)47)
#define SALEVENT_QUERYCHARPOSITION      ((sal_uInt16)48)

// MOUSELEAVE must send, when the pointer leave the client area and
// the mouse is not captured
// MOUSEMOVE, MOUSELEAVE, MOUSEBUTTONDOWN and MOUSEBUTTONUP
// MAC: Ctrl+Button is MOUSE_RIGHT
struct SalMouseEvent
{
    sal_uLong       mnTime;         // Time in ms, when event is created
    long            mnX;            // X-Position (Pixel, TopLeft-Output)
    long            mnY;            // Y-Position (Pixel, TopLeft-Output)
    sal_uInt16      mnButton;       // 0-MouseMove/MouseLeave, MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE
    sal_uInt16      mnCode;         // SV-Modifiercode (KEY_SHIFT|KEY_MOD1|KEY_MOD2|MOUSE_LEFT|MOUSE_MIDDLE|MOUSE_RIGHT)
};

// KEYINPUT and KEYUP
struct SalKeyEvent
{
    sal_uLong       mnTime;         // Time in ms, when event is created
    sal_uInt16      mnCode;         // SV-KeyCode (KEY_xxx | KEY_SHIFT | KEY_MOD1 | KEY_MOD2)
    sal_uInt16      mnCharCode;     // SV-CharCode
    sal_uInt16      mnRepeat;       // Repeat-Count (KeyInputs-1)
};

// MENUEVENT
struct SalMenuEvent
{
    sal_uInt16      mnId;           // Menu item ID
    void*           mpMenu;         // pointer to VCL menu (class Menu)

    SalMenuEvent() : mnId( 0 ), mpMenu( NULL ) {}
    SalMenuEvent( sal_uInt16 i_nId, void* i_pMenu )
    : mnId( i_nId ), mpMenu( i_pMenu ) {}
};

// KEYMODCHANGE
struct SalKeyModEvent
{
    sal_uLong       mnTime;         // Time in ms, when event is created
    sal_uInt16      mnCode;         // SV-Modifiercode (KEY_SHIFT|KEY_MOD1|KEY_MOD2)
    sal_uInt16      mnModKeyCode;   // extended Modifier (MODKEY_LEFT,MODKEY_RIGHT,MODKEY_PRESS,MODKEY_RELEASE)
};

// PAINT
struct SalPaintEvent
{
    long            mnBoundX;           // BoundRect - X
    long            mnBoundY;           // BoundRect - Y
    long            mnBoundWidth;       // BoundRect - Width
    long            mnBoundHeight;      // BoundRect - Height
    bool            mbImmediateUpdate;  // set to true to force an immediate update

    SalPaintEvent( long x, long y, long w, long h, bool bImmediate = false ) :
        mnBoundX( x ), mnBoundY( y ),
        mnBoundWidth( w ), mnBoundHeight( h ),
        mbImmediateUpdate( bImmediate )
    {}
};

// USEREVENT
// pEvent == pData

// WHEELMOUSE
#define SAL_WHEELMOUSE_EVENT_PAGESCROLL     ((sal_uLong)0xFFFFFFFF)
struct SalWheelMouseEvent
{
    sal_uLong       mnTime;         // Time in ms, when event is created
    long            mnX;            // X-Position (Pixel, TopLeft-Output)
    long            mnY;            // Y-Position (Pixel, TopLeft-Output)
    long            mnDelta;        // Number of rotations
    long            mnNotchDelta;   // Number of fixed rotations
    sal_uLong       mnScrollLines;  // Actual number of lines to scroll
    sal_uInt16      mnCode;         // SV-Modifiercode (KEY_SHIFT|KEY_MOD1|KEY_MOD2|MOUSE_LEFT|MOUSE_MIDDLE|MOUSE_RIGHT)
    sal_Bool        mbHorz;         // Horizontal
    sal_Bool        mbDeltaIsPixel; // delta value is a pixel value (on mac)

    SalWheelMouseEvent()
    : mnTime( 0 ), mnX( 0 ), mnY( 0 ), mnDelta( 0 ), mnNotchDelta( 0 ), mnScrollLines( 0 ), mnCode( 0 ), mbHorz( sal_False ), mbDeltaIsPixel( sal_False )
    {}
};

// MOUSEACTIVATE
struct SalMouseActivateEvent
{
    long            mnX;            // X-Position (Pixel, TopLeft-Output)
    long            mnY;            // Y-Position (Pixel, TopLeft-Output)
};

// EXTTEXTINPUT
struct SalExtTextInputEvent
{
    sal_uLong           mnTime;         // Time in ms, when event is created
    OUString            maText;         // Text
    const sal_uInt16*   mpTextAttr;     // Text-Attribute
    sal_uLong           mnCursorPos;    // Cursor-Position
    sal_uLong           mnDeltaStart;   // Start-Position of last change
    sal_uInt8           mnCursorFlags;  // EXTTEXTINPUT_CURSOR_xxx
    sal_Bool            mbOnlyCursor;   // sal_True: Only Cursor-Position has been changed
};

// EXTTEXTINPUTPOS
struct SalExtTextInputPosEvent
{
    long            mnX;            // Cursor-X-Position to upper left corner of frame
    long            mnY;            // Cursor-Y-Position to upper left corner of frame
    long            mnWidth;        // Cursor-Width in Pixel
    long            mnHeight;       // Cursor-Height in Pixel
    long            mnExtWidth;     // Width of the PreEdit area
    bool            mbVertical;     // true if in vertical mode
};

// INPUTCONTEXTCHANGE
struct SalInputContextChangeEvent
{
    LanguageType    meLanguage;     // new language
};

// SURROUNDINGTEXTREQUEST
struct SalSurroundingTextRequestEvent
{
    OUString        maText;         // Text
    sal_uLong       mnStart;        // The beginning index of selected range
    sal_uLong       mnEnd;          // The end index of selected range
};

// SURROUNDINGTEXTSELECTIONCHANGE
struct SalSurroundingTextSelectionChangeEvent
{
    sal_uLong       mnStart;        // The beginning index of selected range
    sal_uLong       mnEnd;          // The end index of selected range
};

// QUERYCHARPOSITION
struct SalQueryCharPositionEvent
{
    bool            mbValid;                // The data is valid or not.
    sal_uLong       mnCharPos;              // The index of character in a composition.
    bool            mbVertical;             // The text is vertical or not.
    long            mnCursorBoundX;         // The cursor bounds corresponding to the character specified by mnCharPos - X
    long            mnCursorBoundY;         // The cursor bounds corresponding to the character specified by mnCharPos - Y
    long            mnCursorBoundWidth;     // The cursor bounds corresponding to the character specified by mnCharPos - Width
    long            mnCursorBoundHeight;    // The cursor bounds corresponding to the character specified by mnCharPos - Height
};

// ------------------
// - SalFrame-Types -
// ------------------

typedef long (*SALFRAMEPROC)( Window* pInst, SalFrame* pFrame,
                              sal_uInt16 nEvent, const void* pEvent );

// --------------------
// - SalObject-Events -
// --------------------

#define SALOBJ_EVENT_GETFOCUS           ((sal_uInt16)1)
#define SALOBJ_EVENT_LOSEFOCUS          ((sal_uInt16)2)
#define SALOBJ_EVENT_TOTOP              ((sal_uInt16)3)
#define SALOBJ_EVENT_COUNT              ((sal_uInt16)4)

// ------------------
// - SalObject-Types -
// ------------------

typedef long (*SALOBJECTPROC)( void* pInst, SalObject* pObject,
                               sal_uInt16 nEvent, const void* pEvent );

// -----------------
// - SalFrameState -
// -----------------

struct SalFrameState
{
    sal_uLong       mnMask;
    long            mnX;
    long            mnY;
    long            mnWidth;
    long            mnHeight;
    long            mnMaximizedX;
    long            mnMaximizedY;
    long            mnMaximizedWidth;
    long            mnMaximizedHeight;
    sal_uLong       mnState;
};

// -------------------
// - SalInputContext -
// -------------------

// Have to match DEFINEs in inputctx.hxx, as these are not converted
#define SAL_INPUTCONTEXT_TEXT               ((sal_uLong)0x00000001)
#define SAL_INPUTCONTEXT_EXTTEXTINPUT       ((sal_uLong)0x00000002)
#define SAL_INPUTCONTEXT_EXTTEXTINPUT_ON    ((sal_uLong)0x00000004)
#define SAL_INPUTCONTEXT_EXTTEXTINPUT_OFF   ((sal_uLong)0x00000008)
#define SAL_INPUTCONTEXT_CHANGELANGUAGE     ((sal_uLong)0x00000010)

struct SalInputContext
{
    FontSelectPattern*     mpFont;
    LanguageType           meLanguage;
    sal_uLong              mnOptions;
};

typedef void (*SALTIMERPROC)();

#endif // _SV_SALWTYPE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
