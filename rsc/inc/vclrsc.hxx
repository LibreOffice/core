/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclrsc.hxx,v $
 * $Revision: 1.12 $
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

#ifndef _TOOLS_VCLRSC_HXX
#define _TOOLS_VCLRSC_HXX

#include <sal/types.h>
#include <tools/solar.h>

// ---------------
// - Window-Bits -
// ---------------

// from vcl/inc/wintypes.hxx
typedef sal_Int64 WinBits;

// Window-Bits fuer Window
#define WB_CLIPCHILDREN         ((WinBits)0x00000001)
#define WB_DIALOGCONTROL        ((WinBits)0x00000002)
#define WB_NODIALOGCONTROL      ((WinBits)0x00000004)
#define WB_BORDER               ((WinBits)0x00000008)
#define WB_NOBORDER             ((WinBits)0x00000010)
#define WB_SIZEABLE             ((WinBits)0x00000020)
#define WB_3DLOOK               ((WinBits)0x00000040)
#define WB_AUTOSIZE             ((WinBits)0x00000080)

// Window-Bits fuer SystemWindows
#define WB_MOVEABLE             ((WinBits)0x00000100)
#define WB_ROLLABLE             ((WinBits)0x00000200)
#define WB_CLOSEABLE            ((WinBits)0x00000400)
#define WB_STANDALONE           ((WinBits)0x00000800)
#define WB_APP                  ((WinBits)0x00001000)
#define WB_PINABLE              ((WinBits)0x00002000)
#define WB_SYSTEMWINDOW         ((WinBits)0x40000000)
#define WB_SIZEMOVE             (WB_SIZEABLE | WB_MOVEABLE)

// Standard-Window-Bits fuer ChildWindows
#define WB_TABSTOP              ((WinBits)0x00000100)
#define WB_NOTABSTOP            ((WinBits)0x00000200)
#define WB_GROUP                ((WinBits)0x00000400)
#define WB_NOGROUP              ((WinBits)0x00000800)
#define WB_HORZ                 ((WinBits)0x00001000)
#define WB_VERT                 ((WinBits)0x00002000)
#define WB_LEFT                 ((WinBits)0x00004000)
#define WB_CENTER               ((WinBits)0x00008000)
#define WB_RIGHT                ((WinBits)0x00010000)
#define WB_TOP                  ((WinBits)0x00020000)
#define WB_VCENTER              ((WinBits)0x00040000)
#define WB_BOTTOM               ((WinBits)0x00080000)
#define WB_DRAG                 ((WinBits)0x00100000)
#define WB_SPIN                 ((WinBits)0x00200000)
#define WB_REPEAT               ((WinBits)0x00400000)
#define WB_NOPOINTERFOCUS       ((WinBits)0x00800000)
#define WB_WORDBREAK            ((WinBits)0x01000000)
#define WB_NOLABEL              ((WinBits)0x02000000)
#define WB_SORT                 ((WinBits)0x04000000)
#define WB_DROPDOWN             ((WinBits)0x08000000)
#define WB_AUTOHSCROLL          ((WinBits)0x10000000)
#define WB_DOCKABLE             ((WinBits)0x20000000)
#define WB_AUTOVSCROLL          ((WinBits)0x40000000)

#define WB_HIDE                 ((WinBits)0x80000000)
#define WB_HSCROLL              WB_HORZ
#define WB_VSCROLL              WB_VERT
#define WB_TOPIMAGE             WB_TOP

// Window-Bits for PushButtons
#define WB_DEFBUTTON            ((WinBits)0x10000000)
#define WB_NOLIGHTBORDER        ((WinBits)0x20000000)
#define WB_RECTSTYLE            ((WinBits)0x08000000)
#define WB_SMALLSTYLE           ((WinBits)0x04000000)

// Window-Bits for FixedText
#define WB_INFO                 ((WinBits)0x20000000)
#define WB_PATHELLIPSIS         ((WinBits)0x00100000)

// Window-Bits for Edit
#define WB_PASSWORD             ((WinBits)0x01000000)
#define WB_READONLY             ((WinBits)0x02000000)
#define WB_NOHIDESELECTION      ((WinBits)SAL_CONST_INT64(0x1000000000))

// Window-Bits for MultiLineEdit
#define WB_IGNORETAB            ((WinBits)0x20000000)

// Window-Bits for ListBox and MultiListBox
#define WB_SIMPLEMODE           ((WinBits)0x20000000)

// Window-Bits for FixedBitmap
#define WB_FAST                 ((WinBits)0x04000000)
#define WB_SCALE                ((WinBits)0x08000000)
#define WB_TOPLEFTVISIBLE       ((WinBits)0x10000000)

// Window-Bits for ToolBox
#define WB_LINESPACING          ((WinBits)0x01000000)
#define WB_SCROLL               ((WinBits)0x02000000)

// Window-Bits for TabControl
#define WB_SINGLELINE           ((WinBits)0x02000000)

// Window-Bits for DockingWindows
#define WB_DOCKBORDER           ((WinBits)0x00001000)

// Window-Bits for SplitWindow
#define WB_NOSPLITDRAW          ((WinBits)0x01000000)
#define WB_FLATSPLITDRAW        ((WinBits)0x02000000)

// Window-Bits for MessageBoxen
#define WB_OK                   ((WinBits)0x00100000)
#define WB_OK_CANCEL            ((WinBits)0x00200000)
#define WB_YES_NO               ((WinBits)0x00400000)
#define WB_YES_NO_CANCEL        ((WinBits)0x00800000)
#define WB_RETRY_CANCEL         ((WinBits)0x01000000)
#define WB_DEF_OK               ((WinBits)0x02000000)
#define WB_DEF_CANCEL           ((WinBits)0x04000000)
#define WB_DEF_RETRY            ((WinBits)0x08000000)
#define WB_DEF_YES              ((WinBits)0x10000000)
#define WB_DEF_NO               ((WinBits)0x20000000)
#define WB_ABORT_RETRY_IGNORE  ((WinBits)SAL_CONST_INT64(0x1000000000))
#define WB_DEF_IGNORE          ((WinBits)SAL_CONST_INT64(0x2000000000))

// Standard-WinBits
#define WB_STDWORK              (WB_SIZEMOVE | WB_CLOSEABLE)
#define WB_STDMDI               (WB_CLOSEABLE)
#define WB_STDDOCKWIN           (WB_DOCKABLE | WB_MOVEABLE | WB_CLOSEABLE)
#define WB_STDFLOATWIN          (WB_SIZEMOVE | WB_CLOSEABLE | WB_ROLLABLE)
#define WB_STDDIALOG            (WB_MOVEABLE | WB_CLOSEABLE)
#define WB_STDMODELESS          (WB_STDDIALOG)
#define WB_STDMODAL             (WB_STDDIALOG)
#define WB_STDTABDIALOG         (WB_STDDIALOG)
#define WB_STDTABCONTROL        0

// For TreeListBox
#define WB_HASBUTTONS           ((WinBits)0x00800000)
#define WB_HASLINES             ((WinBits)0x01000000)
#define WB_HASLINESATROOT       ((WinBits)0x02000000)

// --------------
// - Help-Types -
// --------------

// from vcl/inc/help.hxx
#define OOO_HELP_INDEX          ((ULONG)0xFFFFFFFF)
#define OOO_HELP_HELPONHELP     ((ULONG)0xFFFFFFFE)

// --------------
// - FieldTypes -
// --------------

// from vcl/inc/fldunit.hxx
enum FieldUnit { FUNIT_NONE, FUNIT_MM, FUNIT_CM, FUNIT_M, FUNIT_KM,
                 FUNIT_TWIP, FUNIT_POINT, FUNIT_PICA,
                 FUNIT_INCH, FUNIT_FOOT, FUNIT_MILE, FUNIT_CUSTOM,
                 FUNIT_PERCENT, FUNIT_100TH_MM };


// from vcl/inc/vclenum.hxx
#ifndef ENUM_TIMEFIELDFORMAT_DECLARED
#define ENUM_TIMEFIELDFORMAT_DECLARED

enum TimeFieldFormat {TIMEF_NONE, TIMEF_SEC, TIMEF_100TH_SEC, TIMEF_SEC_CS, TimeFieldFormat_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// -----------------
// - KeyCode-Types -
// -----------------
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyGroup.hpp>

#define KEY_CODE        ((sal_uInt16)0x0FFF)

// Modifier-Tasten
#define KEY_SHIFT       ((sal_uInt16)0x1000)
#define KEY_MOD1        ((sal_uInt16)0x2000)
#define KEY_MOD2        ((sal_uInt16)0x4000)
#define KEY_MOD3        ((sal_uInt16)0x8000)
#define KEY_MODTYPE     ((sal_uInt16)0xF000)
#define KEY_ALLMODTYPE  ((sal_uInt16)0xF000)


// from vcl/inc/vclenum.hxx
#ifndef ENUM_KEYFUNCTYPE_DECLARED
#define ENUM_KEYFUNCTYPE_DECLARED

enum KeyFuncType { KEYFUNC_DONTKNOW, KEYFUNC_NEW, KEYFUNC_OPEN, KEYFUNC_SAVE,
                   KEYFUNC_SAVEAS, KEYFUNC_PRINT, KEYFUNC_CLOSE, KEYFUNC_QUIT,
                   KEYFUNC_CUT, KEYFUNC_COPY, KEYFUNC_PASTE, KEYFUNC_UNDO,
                   KEYFUNC_REDO, KEYFUNC_DELETE, KEYFUNC_REPEAT, KEYFUNC_FIND,
                   KEYFUNC_FINDBACKWARD, KEYFUNC_PROPERTIES, KEYFUNC_FRONT,
                   KeyFuncType_FORCE_EQUAL_SIZE=SAL_MAX_ENUM };

#endif

// ------------
// - TriState -
// ------------

// from vcl/inc/wintypes.hxx
enum TriState { STATE_NOCHECK, STATE_CHECK, STATE_DONTKNOW };

// -----------------
// - MapMode-Types -
// -----------------

// from vcl/inc/mapmod.hxx
enum MapUnit { MAP_100TH_MM, MAP_10TH_MM, MAP_MM, MAP_CM,
               MAP_1000TH_INCH, MAP_100TH_INCH, MAP_10TH_INCH, MAP_INCH,
               MAP_POINT, MAP_TWIP, MAP_PIXEL, MAP_SYSFONT, MAP_APPFONT,
               MAP_RELATIVE, MAP_REALAPPFONT };

// Das Resourcesystem benutzt den Wert 0xFFFF (freihalten)


// --------------------------
// - Bits fuer ToolBoxItems -
// --------------------------

// from vcl/inc/toolbox.hxx
typedef sal_uInt16 ToolBoxItemBits;

#define TIB_CHECKABLE           ((ToolBoxItemBits)0x0001)
#define TIB_RADIOCHECK          ((ToolBoxItemBits)0x0002)
#define TIB_AUTOCHECK           ((ToolBoxItemBits)0x0004)
#define TIB_LEFT                ((ToolBoxItemBits)0x0008)
#define TIB_AUTOSIZE            ((ToolBoxItemBits)0x0010)
#define TIB_DROPDOWN            ((ToolBoxItemBits)0x0020)
#define TIB_REPEAT              ((ToolBoxItemBits)0x0040)

enum ButtonType { BUTTON_SYMBOL, BUTTON_TEXT, BUTTON_SYMBOLTEXT };

enum ToolBoxItemType { TOOLBOXITEM_DONTKNOW, TOOLBOXITEM_BUTTON,
                       TOOLBOXITEM_SPACE, TOOLBOXITEM_SEPARATOR,
                       TOOLBOXITEM_BREAK };


// ---------------
// - BorderStyle -
// ---------------

typedef sal_uInt16 WindowBorderStyle;

#define WINDOW_BORDER_NORMAL            ((WindowBorderStyle)0x0001)
#define WINDOW_BORDER_MONO              ((WindowBorderStyle)0x0002)
#define WINDOW_BORDER_ACTIVE            ((WindowBorderStyle)0x0004)
#define WINDOW_BORDER_DOUBLEOUT         ((WindowBorderStyle)0x0008)
#define WINDOW_BORDER_MENU              ((WindowBorderStyle)0x0010)
#define WINDOW_BORDER_NOBORDER          ((WindowBorderStyle)0x1000)

// ---------------
// - WindowAlign -
// ---------------

// from vcl/inc/wintypes.hxx
enum WindowAlign { WINDOWALIGN_LEFT, WINDOWALIGN_TOP, WINDOWALIGN_RIGHT, WINDOWALIGN_BOTTOM };
enum ImageAlign { IMAGEALIGN_LEFT, IMAGEALIGN_TOP, IMAGEALIGN_RIGHT, IMAGEALIGN_BOTTOM };

// --------------
// - Menu-Types -
// --------------

// from vcl/inc/menu.hxx
enum MenuItemType { MENUITEM_DONTKNOW, MENUITEM_STRING, MENUITEM_IMAGE,
                    MENUITEM_STRINGIMAGE, MENUITEM_SEPARATOR };

typedef sal_uInt16 MenuItemBits;

#define MIB_CHECKABLE           ((MenuItemBits)0x0001)
#define MIB_RADIOCHECK          ((MenuItemBits)0x0002)
#define MIB_AUTOCHECK           ((MenuItemBits)0x0004)
#define MIB_ABOUT               ((MenuItemBits)0x0008)
#define MIB_HELP                ((MenuItemBits)0x0010)
#define MIB_POPUPSELECT         ((MenuItemBits)0x0020)

// ----------------
// - Symbol-Types -
// ----------------

// from vcl/inc/symbol.hxx
typedef sal_uInt16 SymbolType;
#define SYMBOL_DONTKNOW         ((SymbolType)0)
#define SYMBOL_IMAGE            ((SymbolType)1)
#define SYMBOL_ARROW_UP         ((SymbolType)2)
#define SYMBOL_ARROW_DOWN       ((SymbolType)3)
#define SYMBOL_ARROW_LEFT       ((SymbolType)4)
#define SYMBOL_ARROW_RIGHT      ((SymbolType)5)
#define SYMBOL_SPIN_UP          ((SymbolType)6)
#define SYMBOL_SPIN_DOWN        ((SymbolType)7)
#define SYMBOL_SPIN_LEFT        ((SymbolType)8)
#define SYMBOL_SPIN_RIGHT       ((SymbolType)9)
#define SYMBOL_FIRST            ((SymbolType)10)
#define SYMBOL_LAST             ((SymbolType)11)
#define SYMBOL_PREV             ((SymbolType)12)
#define SYMBOL_NEXT             ((SymbolType)13)
#define SYMBOL_PAGEUP           ((SymbolType)14)
#define SYMBOL_PAGEDOWN         ((SymbolType)15)
#define SYMBOL_PLAY             ((SymbolType)16)
#define SYMBOL_REVERSEPLAY      ((SymbolType)17)
#define SYMBOL_RECORD           ((SymbolType)18)
#define SYMBOL_STOP             ((SymbolType)19)
#define SYMBOL_PAUSE            ((SymbolType)20)
#define SYMBOL_WINDSTART        ((SymbolType)21)
#define SYMBOL_WINDEND          ((SymbolType)22)
#define SYMBOL_WINDBACKWARD     ((SymbolType)23)
#define SYMBOL_WINDFORWARD      ((SymbolType)24)
#define SYMBOL_CLOSE            ((SymbolType)25)
#define SYMBOL_ROLLUP           ((SymbolType)26)
#define SYMBOL_ROLLDOWN         ((SymbolType)27)
#define SYMBOL_CHECKMARK        ((SymbolType)28)
#define SYMBOL_RADIOCHECKMARK   ((SymbolType)29)
#define SYMBOL_SPIN_UPDOWN      ((SymbolType)30)
#define SYMBOL_FLOAT            ((SymbolType)31)
#define SYMBOL_DOCK             ((SymbolType)32)
#define SYMBOL_HIDE             ((SymbolType)33)
#define SYMBOL_HELP             ((SymbolType)34)
#define SYMBOL_OS2CLOSE         ((SymbolType)35)
#define SYMBOL_OS2FLOAT         ((SymbolType)36)
#define SYMBOL_OS2HIDE          ((SymbolType)37)

#define SYMBOL_NOSYMBOL         (SYMBOL_DONTKNOW)

#endif
