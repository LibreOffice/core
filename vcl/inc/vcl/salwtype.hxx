/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salwtype.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _SV_SALWTYPE_HXX
#define _SV_SALWTYPE_HXX

#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <vcl/sv.h>

class SalGraphics;
class SalFrame;
class SalObject;
class Window;

class ImplFontSelectData;

// ---------------
// - SalExtInput -
// ---------------

// Muessen mit den Defines in cmdevt.hxx uebereinstimmen, da diese
// nicht konvertiert werden
#define SAL_EXTTEXTINPUT_ATTR_GRAYWAVELINE          ((USHORT)0x0100)
#define SAL_EXTTEXTINPUT_ATTR_UNDERLINE             ((USHORT)0x0200)
#define SAL_EXTTEXTINPUT_ATTR_BOLDUNDERLINE         ((USHORT)0x0400)
#define SAL_EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE       ((USHORT)0x0800)
#define SAL_EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE      ((USHORT)0x1000)
#define SAL_EXTTEXTINPUT_ATTR_HIGHLIGHT             ((USHORT)0x2000)
#define SAL_EXTTEXTINPUT_ATTR_REDTEXT               ((USHORT)0x4000)
#define SAL_EXTTEXTINPUT_ATTR_HALFTONETEXT          ((USHORT)0x8000)

#define SAL_EXTTEXTINPUT_CURSOR_INVISIBLE           ((USHORT)0x0001)
#define SAL_EXTTEXTINPUT_CURSOR_OVERWRITE           ((USHORT)0x0002)

// ------------
// - SalEvent -
// ------------

#define SALEVENT_MOUSEMOVE              ((USHORT)1)
#define SALEVENT_MOUSELEAVE             ((USHORT)2)
#define SALEVENT_MOUSEBUTTONDOWN        ((USHORT)3)
#define SALEVENT_MOUSEBUTTONUP          ((USHORT)4)
#define SALEVENT_KEYINPUT               ((USHORT)5)
#define SALEVENT_KEYUP                  ((USHORT)6)
#define SALEVENT_KEYMODCHANGE           ((USHORT)7)
#define SALEVENT_PAINT                  ((USHORT)8)
#define SALEVENT_RESIZE                 ((USHORT)9)
#define SALEVENT_GETFOCUS               ((USHORT)10)
#define SALEVENT_LOSEFOCUS              ((USHORT)11)
#define SALEVENT_CLOSE                  ((USHORT)12)
#define SALEVENT_SHUTDOWN               ((USHORT)13)
#define SALEVENT_SETTINGSCHANGED        ((USHORT)14)
#define SALEVENT_VOLUMECHANGED          ((USHORT)15)
#define SALEVENT_PRINTERCHANGED         ((USHORT)16)
#define SALEVENT_DISPLAYCHANGED         ((USHORT)17)
#define SALEVENT_FONTCHANGED            ((USHORT)18)
#define SALEVENT_DATETIMECHANGED        ((USHORT)19)
#define SALEVENT_KEYBOARDCHANGED        ((USHORT)20)
#define SALEVENT_WHEELMOUSE             ((USHORT)21)
#define SALEVENT_USEREVENT              ((USHORT)22)
#define SALEVENT_MOUSEACTIVATE          ((USHORT)23)
#define SALEVENT_EXTTEXTINPUT           ((USHORT)24)
#define SALEVENT_ENDEXTTEXTINPUT        ((USHORT)25)
#define SALEVENT_EXTTEXTINPUTPOS        ((USHORT)26)
#define SALEVENT_INPUTCONTEXTCHANGE     ((USHORT)27)
#define SALEVENT_MOVE                   ((USHORT)28)
#define SALEVENT_MOVERESIZE             ((USHORT)29)
#define SALEVENT_CLOSEPOPUPS            ((USHORT)30)
#define SALEVENT_EXTERNALKEYINPUT       ((USHORT)31)
#define SALEVENT_EXTERNALKEYUP          ((USHORT)32)
#define SALEVENT_MENUCOMMAND            ((USHORT)33)
#define SALEVENT_MENUHIGHLIGHT          ((USHORT)34)
#define SALEVENT_MENUACTIVATE           ((USHORT)35)
#define SALEVENT_MENUDEACTIVATE         ((USHORT)36)
#define SALEVENT_EXTERNALMOUSEMOVE      ((USHORT)37)
#define SALEVENT_EXTERNALMOUSEBUTTONDOWN ((USHORT)38)
#define SALEVENT_EXTERNALMOUSEBUTTONUP  ((USHORT)39)
#define SALEVENT_INPUTLANGUAGECHANGE    ((USHORT)40)
#define SALEVENT_SHOWDIALOG             ((USHORT)41)
#define SALEVENT_COUNT                  ((USHORT)41)

// MOUSELEAVE must send, when the pointer leave the client area and
// the mouse is not captured
// MOUSEMOVE, MOUSELEAVE, MOUSEBUTTONDOWN and MOUSEBUTTONUP
// MAC: Ctrl+Button is MOUSE_RIGHT
struct SalMouseEvent
{
    ULONG           mnTime;         // Time in ms, when event is created
    long            mnX;            // X-Position (Pixel, TopLeft-Output)
    long            mnY;            // Y-Position (Pixel, TopLeft-Output)
    USHORT          mnButton;       // 0-MouseMove/MouseLeave, MOUSE_LEFT, MOUSE_RIGHT, MOUSE_MIDDLE
    USHORT          mnCode;         // SV-ModifierCode (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)
};

// KEYINPUT and KEYUP
struct SalKeyEvent
{
    ULONG           mnTime;         // Time in ms, when event is created
    USHORT          mnCode;         // SV-KeyCode (KEY_xxx | KEY_SHIFT | KEY_MOD1 | KEY_MOD2)
    USHORT          mnCharCode;     // SV-CharCode
    USHORT          mnRepeat;       // Repeat-Count (KeyInputs-1)
};

// MENUEVENT
struct SalMenuEvent
{
    USHORT          mnId;           // Menu item ID
    void*           mpMenu;         // pointer to VCL menu (class Menu)
};

// KEYMODCHANGE
struct SalKeyModEvent
{
    ULONG           mnTime;         // Time in ms, when event is created
    USHORT          mnCode;         // SV-ModifierCode (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)
    USHORT          mnModKeyCode;   // extended Modifier (MODKEY_LEFT, MODKEY_RIGHT, MODKEY_PRESS, MODKEY_RELEASE)
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
#define SAL_WHEELMOUSE_EVENT_PAGESCROLL     ((ULONG)0xFFFFFFFF)
struct SalWheelMouseEvent
{
    ULONG           mnTime;         // Time in ms, when event is created
    long            mnX;            // X-Position (Pixel, TopLeft-Output)
    long            mnY;            // Y-Position (Pixel, TopLeft-Output)
    long            mnDelta;        // Anzahl Drehungen
    long            mnNotchDelta;   // Anzahl feste Drehungen
    ULONG           mnScrollLines;  // Aktuelle Anzahl zu scrollende Zeilen
    USHORT          mnCode;         // SV-ModifierCode (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | MOUSE_LEFT | MOUSE_MIDDLE | MOUSE_RIGHT)
    BOOL            mbHorz;         // Horizontal
};

// MOUSEACTIVATE
struct SalMouseActivateEvent
{
    long            mnX;            // X-Position (Pixel, TopLeft-Output)
    long            mnY;            // Y-Position (Pixel, TopLeft-Output)
};

#ifdef __cplusplus

// EXTTEXTINPUT
struct SalExtTextInputEvent
{
    ULONG           mnTime;         // Time in ms, when event is created
    UniString       maText;         // Text
    const USHORT*   mpTextAttr;     // Text-Attribute
    ULONG           mnCursorPos;    // Cursor-Position
    ULONG           mnDeltaStart;   // Start-Position der letzten Aenderung
    BYTE            mnCursorFlags;  // SAL_EXTTEXTINPUT_CURSOR_xxx
    BOOL            mbOnlyCursor;   // TRUE: Nur Cursor-Position wurde geaendert
};

#endif // __cplusplus

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

#ifdef __cplusplus

// INPUTCONTEXTCHANGE
struct SalInputContextChangeEvent
{
    LanguageType    meLanguage;     // Neue Sprache
};

#endif // __cplusplus

// ------------------
// - SalFrame-Types -
// ------------------

typedef long (*SALFRAMEPROC)( Window* pInst, SalFrame* pFrame,
                              USHORT nEvent, const void* pEvent );

// --------------------
// - SalObject-Events -
// --------------------

#define SALOBJ_EVENT_GETFOCUS           ((USHORT)1)
#define SALOBJ_EVENT_LOSEFOCUS          ((USHORT)2)
#define SALOBJ_EVENT_TOTOP              ((USHORT)3)
#define SALOBJ_EVENT_COUNT              ((USHORT)4)

// ------------------
// - SalObject-Types -
// ------------------

typedef long (*SALOBJECTPROC)( void* pInst, SalObject* pObject,
                               USHORT nEvent, const void* pEvent );

// -----------------
// - SalFrameState -
// -----------------

// Must be the same as in syswin.hxx
#define SAL_FRAMESTATE_MASK_X                   ((ULONG)0x00000001)
#define SAL_FRAMESTATE_MASK_Y                   ((ULONG)0x00000002)
#define SAL_FRAMESTATE_MASK_WIDTH               ((ULONG)0x00000004)
#define SAL_FRAMESTATE_MASK_HEIGHT              ((ULONG)0x00000008)
#define SAL_FRAMESTATE_MASK_MAXIMIZED_X         ((ULONG)0x00000100)
#define SAL_FRAMESTATE_MASK_MAXIMIZED_Y         ((ULONG)0x00000200)
#define SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH     ((ULONG)0x00000400)
#define SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT    ((ULONG)0x00000800)
#define SAL_FRAMESTATE_MASK_STATE               ((ULONG)0x00000010)

#define SAL_FRAMESTATE_NORMAL               ((ULONG)0x00000001)
#define SAL_FRAMESTATE_MINIMIZED            ((ULONG)0x00000002)
#define SAL_FRAMESTATE_MAXIMIZED            ((ULONG)0x00000004)
#define SAL_FRAMESTATE_ROLLUP               ((ULONG)0x00000008)
#define SAL_FRAMESTATE_MAXIMIZED_HORZ       ((ULONG)0x00000010)
#define SAL_FRAMESTATE_MAXIMIZED_VERT       ((ULONG)0x00000020)
#define SAL_FRAMESTATE_SYSTEMMASK           ((ULONG)0x0000FFFF)

struct SalFrameState
{
    ULONG           mnMask;
    long            mnX;
    long            mnY;
    long            mnWidth;
    long            mnHeight;
    long            mnMaximizedX;
    long            mnMaximizedY;
    long            mnMaximizedWidth;
    long            mnMaximizedHeight;
    ULONG           mnState;
};

// -------------------
// - SalInputContext -
// -------------------

// Muessen mit den Defines in inputctx.hxx uebereinstimmen, da diese
// nicht konvertiert werden
#define SAL_INPUTCONTEXT_TEXT               ((ULONG)0x00000001)
#define SAL_INPUTCONTEXT_EXTTEXTINPUT       ((ULONG)0x00000002)
#define SAL_INPUTCONTEXT_EXTTEXTINPUT_ON    ((ULONG)0x00000004)
#define SAL_INPUTCONTEXT_EXTTEXTINPUT_OFF   ((ULONG)0x00000008)
#define SAL_INPUTCONTEXT_CHANGELANGUAGE     ((ULONG)0x00000010)

#ifdef __cplusplus

struct SalInputContext
{
    ImplFontSelectData*     mpFont;
    LanguageType            meLanguage;
    ULONG                   mnOptions;
};

#endif // __cplusplus

// ------------------
// - SalTimer-Types -
// ------------------

typedef void (*SALTIMERPROC)();

#endif // _SV_SALWTYPE_HXX
