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
#ifndef _TOOLS_WINTYPES_HXX
#define _TOOLS_WINTYPES_HXX

#include <tools/solar.h>
#include <sal/types.h>

// Window-Types

typedef sal_uInt16 WindowType;
#define WINDOW_BASE                 0x0100
#define WINDOW_FIRST                (WINDOW_BASE + 0x30)
#define WINDOW_MESSBOX              (WINDOW_FIRST)
#define WINDOW_INFOBOX              (WINDOW_FIRST + 0x01)
#define WINDOW_WARNINGBOX           (WINDOW_FIRST + 0x02)
#define WINDOW_ERRORBOX             (WINDOW_FIRST + 0x03)
#define WINDOW_QUERYBOX             (WINDOW_FIRST + 0x04)
#define WINDOW_WINDOW               (WINDOW_FIRST + 0x05)
#define WINDOW_SYSWINDOW            (WINDOW_FIRST + 0x06)
#define WINDOW_WORKWINDOW           (WINDOW_FIRST + 0x07)
#define WINDOW_CONTAINER            (WINDOW_FIRST + 0x08)
#define WINDOW_FLOATINGWINDOW       (WINDOW_FIRST + 0x09)
#define WINDOW_DIALOG               (WINDOW_FIRST + 0x0a)
#define WINDOW_MODELESSDIALOG       (WINDOW_FIRST + 0x0b)
#define WINDOW_MODALDIALOG          (WINDOW_FIRST + 0x0c)
#define WINDOW_SYSTEMDIALOG         (WINDOW_FIRST + 0x0d)
#define WINDOW_PATHDIALOG           (WINDOW_FIRST + 0x0e)
#define WINDOW_FILEDIALOG           (WINDOW_FIRST + 0x0f)
#define WINDOW_PRINTERSETUPDIALOG   (WINDOW_FIRST + 0x10)
#define WINDOW_PRINTDIALOG          (WINDOW_FIRST + 0x11)
#define WINDOW_COLORDIALOG          (WINDOW_FIRST + 0x12)
#define WINDOW_FONTDIALOG           (WINDOW_FIRST + 0x13)
#define WINDOW_CONTROL              (WINDOW_FIRST + 0x14)
#define WINDOW_BUTTON               (WINDOW_FIRST + 0x15)
#define WINDOW_PUSHBUTTON           (WINDOW_FIRST + 0x16)
#define WINDOW_OKBUTTON             (WINDOW_FIRST + 0x17)
#define WINDOW_CANCELBUTTON         (WINDOW_FIRST + 0x18)
#define WINDOW_HELPBUTTON           (WINDOW_FIRST + 0x19)
#define WINDOW_IMAGEBUTTON          (WINDOW_FIRST + 0x1a)
#define WINDOW_MENUBUTTON           (WINDOW_FIRST + 0x1b)
#define WINDOW_MOREBUTTON           (WINDOW_FIRST + 0x1c)
#define WINDOW_SPINBUTTON           (WINDOW_FIRST + 0x1d)
#define WINDOW_RADIOBUTTON          (WINDOW_FIRST + 0x1e)
#define WINDOW_IMAGERADIOBUTTON     (WINDOW_FIRST + 0x1f)
#define WINDOW_CHECKBOX             (WINDOW_FIRST + 0x20)
#define WINDOW_TRISTATEBOX          (WINDOW_FIRST + 0x21)
#define WINDOW_EDIT                 (WINDOW_FIRST + 0x22)
#define WINDOW_MULTILINEEDIT        (WINDOW_FIRST + 0x23)
#define WINDOW_COMBOBOX             (WINDOW_FIRST + 0x24)
#define WINDOW_LISTBOX              (WINDOW_FIRST + 0x25)
#define WINDOW_MULTILISTBOX         (WINDOW_FIRST + 0x26)
#define WINDOW_FIXEDTEXT            (WINDOW_FIRST + 0x27)
#define WINDOW_FIXEDLINE            (WINDOW_FIRST + 0x28)
#define WINDOW_FIXEDBITMAP          (WINDOW_FIRST + 0x29)
#define WINDOW_FIXEDIMAGE           (WINDOW_FIRST + 0x2a)
#define WINDOW_GROUPBOX             (WINDOW_FIRST + 0x2c)
#define WINDOW_SCROLLBAR            (WINDOW_FIRST + 0x2d)
#define WINDOW_SCROLLBARBOX         (WINDOW_FIRST + 0x2e)
#define WINDOW_SPLITTER             (WINDOW_FIRST + 0x2f)
#define WINDOW_SPLITWINDOW          (WINDOW_FIRST + 0x30)
#define WINDOW_SPINFIELD            (WINDOW_FIRST + 0x31)
#define WINDOW_PATTERNFIELD         (WINDOW_FIRST + 0x32)
#define WINDOW_NUMERICFIELD         (WINDOW_FIRST + 0x33)
#define WINDOW_METRICFIELD          (WINDOW_FIRST + 0x34)
#define WINDOW_CURRENCYFIELD        (WINDOW_FIRST + 0x35)
#define WINDOW_DATEFIELD            (WINDOW_FIRST + 0x36)
#define WINDOW_TIMEFIELD            (WINDOW_FIRST + 0x37)
#define WINDOW_PATTERNBOX           (WINDOW_FIRST + 0x38)
#define WINDOW_NUMERICBOX           (WINDOW_FIRST + 0x39)
#define WINDOW_METRICBOX            (WINDOW_FIRST + 0x3a)
#define WINDOW_CURRENCYBOX          (WINDOW_FIRST + 0x3b)
#define WINDOW_DATEBOX              (WINDOW_FIRST + 0x3c)
#define WINDOW_TIMEBOX              (WINDOW_FIRST + 0x3d)
#define WINDOW_LONGCURRENCYFIELD    (WINDOW_FIRST + 0x3e)
#define WINDOW_LONGCURRENCYBOX      (WINDOW_FIRST + 0x3f)
#define WINDOW_TOOLBOX              (WINDOW_FIRST + 0x41)
#define WINDOW_DOCKINGWINDOW        (WINDOW_FIRST + 0x42)
#define WINDOW_STATUSBAR            (WINDOW_FIRST + 0x43)
#define WINDOW_TABPAGE              (WINDOW_FIRST + 0x44)
#define WINDOW_TABCONTROL           (WINDOW_FIRST + 0x45)
#define WINDOW_TABDIALOG            (WINDOW_FIRST + 0x46)
#define WINDOW_BORDERWINDOW         (WINDOW_FIRST + 0x47)
#define WINDOW_BUTTONDIALOG         (WINDOW_FIRST + 0x48)
#define WINDOW_SYSTEMCHILDWINDOW    (WINDOW_FIRST + 0x49)
#define WINDOW_SLIDER               (WINDOW_FIRST + 0x4a)
#define WINDOW_MENUBARWINDOW        (WINDOW_FIRST + 0x4b)
#define WINDOW_TREELISTBOX          (WINDOW_FIRST + 0x4c)
#define WINDOW_HELPTEXTWINDOW       (WINDOW_FIRST + 0x4d)
#define WINDOW_INTROWINDOW          (WINDOW_FIRST + 0x4e)
#define WINDOW_LISTBOXWINDOW        (WINDOW_FIRST + 0x4f)
#define WINDOW_DOCKINGAREA          (WINDOW_FIRST + 0x50)
#define WINDOW_RULER                (WINDOW_FIRST + 0x51)
#define WINDOW_CALCINPUTLINE        (WINDOW_FIRST + 0x52)
#define WINDOW_LAST                 (WINDOW_CALCINPUTLINE)

// Window-Bits

typedef sal_Int64 WinBits;

// Window-Bits for Window
#define WB_CLIPCHILDREN         ((WinBits)0x00000001)
#define WB_DIALOGCONTROL        ((WinBits)0x00000002)
#define WB_NODIALOGCONTROL      ((WinBits)0x00000004)
#define WB_BORDER               ((WinBits)0x00000008)
#define WB_NOBORDER             ((WinBits)0x00000010)
#define WB_SIZEABLE             ((WinBits)0x00000020)
#define WB_3DLOOK               ((WinBits)0x00000040)
#define WB_AUTOSIZE             ((WinBits)0x00000080)

// Window-Bits for SystemWindows
#define WB_MOVEABLE             ((WinBits)0x00000100)
#define WB_ROLLABLE             ((WinBits)0x00000200)
#define WB_CLOSEABLE            ((WinBits)0x00000400)
#define WB_STANDALONE           ((WinBits)0x00000800)
#define WB_APP                  ((WinBits)0x00001000)
#define WB_PINABLE              ((WinBits)0x00002000)
#define WB_SYSTEMWINDOW         ((WinBits)SAL_CONST_INT64(0x40000000))
// warning: do not confuse WB_SYSTEMCHILDWINDOW with the SystemChildWindow class
//
// the SystemChildWindow class was there first and is a very specialized
// sytem child window type for plugged applications. The SystemChildWindow class
// explicitly should never use the WB_SYSTEMCHILDWINDOW WinBit
//
// WB_SYSTEMCHILDWINDOW on the other hand is to be used on system windows
// which should be created as system child windows with (more or less)
// normal event handling
#define WB_SYSTEMCHILDWINDOW    ((WinBits)SAL_CONST_INT64(0x8000000000))
#define WB_SIZEMOVE             (WB_SIZEABLE | WB_MOVEABLE)

// Standard-Window-Bits for ChildWindows
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
#define WB_HIDE                 ((WinBits)SAL_CONST_INT64(0x80000000))
#define WB_AUTOHSCROLL          ((WinBits)SAL_CONST_INT64(0x10000000))
#define WB_DOCKABLE             ((WinBits)SAL_CONST_INT64(0x20000000))
#define WB_AUTOVSCROLL          ((WinBits)SAL_CONST_INT64(0x40000000))
#define WB_HYPHENATION          (((WinBits)SAL_CONST_INT64(0x800000000)) | WB_WORDBREAK)
#define WB_CHILDDLGCTRL         ((WinBits)SAL_CONST_INT64(0x100000000000))

// system floating window
#define WB_SYSTEMFLOATWIN       ((WinBits)SAL_CONST_INT64(0x100000000))
#define WB_INTROWIN             ((WinBits)SAL_CONST_INT64(0x200000000))
#define WB_NOSHADOW             ((WinBits)SAL_CONST_INT64(0x400000000))
#define WB_TOOLTIPWIN           ((WinBits)SAL_CONST_INT64(0x800000000))
#define WB_OWNERDRAWDECORATION  ((WinBits)SAL_CONST_INT64(0x2000000000))
#define WB_DEFAULTWIN           ((WinBits)SAL_CONST_INT64(0x4000000000))
#define WB_NEEDSFOCUS           ((WinBits)SAL_CONST_INT64(0x1000000000))
#define WB_POPUP                ((WinBits)SAL_CONST_INT64(0x20000000))

#define WB_HSCROLL              WB_HORZ
#define WB_VSCROLL              WB_VERT
#define WB_TOPIMAGE             WB_TOP

// Window-Bits for PushButtons
#define WB_DEFBUTTON            ((WinBits)0x10000000)
#define WB_NOLIGHTBORDER        ((WinBits)0x20000000)
#define WB_RECTSTYLE            ((WinBits)0x08000000)
#define WB_SMALLSTYLE           ((WinBits)0x04000000)
#define WB_TOGGLE               ((WinBits)SAL_CONST_INT64(0x1000000000))
#define WB_BEVELBUTTON          ((WinBits)SAL_CONST_INT64(0x2000000000))
#define WB_FLATBUTTON           ((WinBits)SAL_CONST_INT64(0x4000000000))

// Window-Bits for FixedText
#define WB_PATHELLIPSIS         ((WinBits)0x00100000)
#define WB_EXTRAOFFSET          ((WinBits)0x02000000)
#define WB_NOMULTILINE          ((WinBits)0x10000000)
#define WB_INFO                 ((WinBits)0x20000000)

// Window-Bits for CheckBox
#define WB_CBLINESTYLE          ((WinBits)SAL_CONST_INT64(0x2000000000))
#define WB_EARLYTOGGLE          ((WinBits)SAL_CONST_INT64(0x4000000000))

// Window-Bits for Edit
#define WB_PASSWORD             ((WinBits)0x01000000)
#define WB_READONLY             ((WinBits)0x02000000)
#define WB_NOHIDESELECTION      ((WinBits)SAL_CONST_INT64(0x1000000000))
#define WB_FORCECTRLBACKGROUND  ((WinBits)0x80000000)

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
#define WB_FORCETABCYCLE        ((WinBits)0x04000000)

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
#define WB_DEF_YES              ((WinBits)SAL_CONST_INT64(0x10000000))
#define WB_DEF_NO               ((WinBits)SAL_CONST_INT64(0x20000000))
#define WB_ABORT_RETRY_IGNORE  ((WinBits)SAL_CONST_INT64(0x1000000000))
#define WB_DEF_IGNORE          ((WinBits)SAL_CONST_INT64(0x2000000000))

// Standard-WinBits
#define WB_STDWORK              (WB_SIZEMOVE | WB_CLOSEABLE)
#define WB_STDDOCKWIN           (WB_DOCKABLE | WB_MOVEABLE | WB_CLOSEABLE)
#define WB_STDFLOATWIN          (WB_SIZEMOVE | WB_CLOSEABLE | WB_ROLLABLE)
#define WB_STDDIALOG            (WB_MOVEABLE | WB_CLOSEABLE)
#define WB_STDMODELESS          (WB_STDDIALOG)
#define WB_STDMODAL             (WB_STDDIALOG)
#define WB_STDTABDIALOG         (WB_STDDIALOG)
#define WB_STDTABCONTROL        0
#define WB_STDPOPUP             (WB_BORDER | WB_POPUP | WB_SYSTEMWINDOW | WB_3DLOOK | WB_DIALOGCONTROL)

// For TreeListBox
#define WB_HASBUTTONS           ((WinBits)SAL_CONST_INT64(0x000100000000))
#define WB_HASLINES             ((WinBits)SAL_CONST_INT64(0x000200000000))
#define WB_HASLINESATROOT       ((WinBits)SAL_CONST_INT64(0x000400000000))
#define WB_HASBUTTONSATROOT     ((WinBits)SAL_CONST_INT64(0x000800000000))
#define WB_NOINITIALSELECTION   ((WinBits)SAL_CONST_INT64(0x001000000000))
#define WB_HIDESELECTION        ((WinBits)SAL_CONST_INT64(0x002000000000))
#define WB_FORCE_MAKEVISIBLE    ((WinBits)SAL_CONST_INT64(0x004000000000))
// DO NOT USE: 0x008000000000, that's WB_SYSTEMCHILDWINDOW
#define WB_QUICK_SEARCH         ((WinBits)SAL_CONST_INT64(0x010000000000))

// For FileOpen Dialog
#define WB_PATH                 ((WinBits)0x00100000)
#define WB_OPEN                 ((WinBits)0x00200000)
#define WB_SAVEAS               ((WinBits)0x00400000)

// For Slider
// Window-Bits for TabControl
#define WB_SLIDERSET            ((WinBits)0x02000000)

// extended WinBits
#define WB_EXT_DOCUMENT         ((WinBits)0x00000001)
#define WB_EXT_DOCMODIFIED      ((WinBits)0x00000002)

// WindowAlign

enum WindowAlign { WINDOWALIGN_LEFT, WINDOWALIGN_TOP, WINDOWALIGN_RIGHT, WINDOWALIGN_BOTTOM };
enum ImageAlign { IMAGEALIGN_LEFT, IMAGEALIGN_TOP, IMAGEALIGN_RIGHT, IMAGEALIGN_BOTTOM,
                  IMAGEALIGN_LEFT_TOP, IMAGEALIGN_LEFT_BOTTOM, IMAGEALIGN_TOP_LEFT,
                  IMAGEALIGN_TOP_RIGHT, IMAGEALIGN_RIGHT_TOP, IMAGEALIGN_RIGHT_BOTTOM,
                  IMAGEALIGN_BOTTOM_LEFT, IMAGEALIGN_BOTTOM_RIGHT, IMAGEALIGN_CENTER };
enum SymbolAlign { SYMBOLALIGN_LEFT, SYMBOLALIGN_RIGHT };

// TriState

enum TriState { STATE_NOCHECK, STATE_CHECK, STATE_DONTKNOW };


// ButtonDialog-Types

typedef sal_uInt16 StandardButtonType;
#define BUTTON_OK           ((StandardButtonType)0)
#define BUTTON_CANCEL       ((StandardButtonType)1)
#define BUTTON_YES          ((StandardButtonType)2)
#define BUTTON_NO           ((StandardButtonType)3)
#define BUTTON_RETRY        ((StandardButtonType)4)
#define BUTTON_HELP         ((StandardButtonType)5)
#define BUTTON_CLOSE        ((StandardButtonType)6)
#define BUTTON_MORE         ((StandardButtonType)7)
#define BUTTON_IGNORE       ((StandardButtonType)8)
#define BUTTON_ABORT        ((StandardButtonType)9)
#define BUTTON_LESS         ((StandardButtonType)10)
#define BUTTON_RESET        ((StandardButtonType)11)
#define BUTTON_COUNT        12

// prominent place for ListBox window types

enum ProminentEntry { PROMINENT_TOP, PROMINENT_MIDDLE };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
