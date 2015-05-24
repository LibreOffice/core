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
#ifndef INCLUDED_TOOLS_WINTYPES_HXX
#define INCLUDED_TOOLS_WINTYPES_HXX

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
#define WINDOW_SCROLLWINDOW         (WINDOW_FIRST + 0x40)
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
WinBits const WB_CLIPCHILDREN =         0x00000001;
WinBits const WB_DIALOGCONTROL =        0x00000002;
WinBits const WB_NODIALOGCONTROL =      0x00000004;
WinBits const WB_BORDER =               0x00000008;
WinBits const WB_NOBORDER =             0x00000010;
WinBits const WB_SIZEABLE =             0x00000020;
WinBits const WB_3DLOOK =               0x00000040;
WinBits const WB_AUTOSIZE =             0x00000080;

// Window-Bits for SystemWindows
WinBits const WB_MOVEABLE =             0x00000100;
WinBits const WB_ROLLABLE =             0x00000200;
WinBits const WB_CLOSEABLE =            0x00000400;
WinBits const WB_STANDALONE =           0x00000800;
WinBits const WB_APP =                  0x00001000;
WinBits const WB_PINABLE =              0x00002000;
WinBits const WB_SYSTEMWINDOW =         SAL_CONST_INT64(0x40000000);
// warning: do not confuse WB_SYSTEMCHILDWINDOW with the SystemChildWindow class
//
// the SystemChildWindow class was there first and is a very specialized
// system child window type for plugged applications. The SystemChildWindow class
// explicitly should never use the WB_SYSTEMCHILDWINDOW WinBit
//
// WB_SYSTEMCHILDWINDOW on the other hand is to be used on system windows
// which should be created as system child windows with (more or less)
// normal event handling
WinBits const WB_SYSTEMCHILDWINDOW =    SAL_CONST_INT64(0x8000000000);
WinBits const WB_SIZEMOVE =             (WB_SIZEABLE | WB_MOVEABLE);

// Standard-Window-Bits for ChildWindows
WinBits const WB_TABSTOP =              0x00000100;
WinBits const WB_NOTABSTOP =            0x00000200;
WinBits const WB_GROUP =                0x00000400;
WinBits const WB_NOGROUP =              0x00000800;
WinBits const WB_HORZ =                 0x00001000;
WinBits const WB_VERT =                 0x00002000;
WinBits const WB_LEFT =                 0x00004000;
WinBits const WB_CENTER =               0x00008000;
WinBits const WB_RIGHT =                0x00010000;
WinBits const WB_TOP =                  0x00020000;
WinBits const WB_VCENTER =              0x00040000;
WinBits const WB_BOTTOM =               0x00080000;
WinBits const WB_DRAG =                 0x00100000;
WinBits const WB_SPIN =                 0x00200000;
WinBits const WB_REPEAT =               0x00400000;
WinBits const WB_NOPOINTERFOCUS =       0x00800000;
WinBits const WB_WORDBREAK =            0x01000000;
WinBits const WB_NOLABEL =              0x02000000;
WinBits const WB_SORT =                 0x04000000;
WinBits const WB_DROPDOWN =             0x08000000;
WinBits const WB_HIDE =                 SAL_CONST_INT64(0x80000000);
WinBits const WB_AUTOHSCROLL =          SAL_CONST_INT64(0x10000000);
WinBits const WB_DOCKABLE =             SAL_CONST_INT64(0x20000000);
WinBits const WB_AUTOVSCROLL =          SAL_CONST_INT64(0x40000000);
WinBits const WB_HYPHENATION =          SAL_CONST_INT64(0x800000000) | WB_WORDBREAK;
WinBits const WB_CHILDDLGCTRL =         SAL_CONST_INT64(0x100000000000);

// system floating window
WinBits const WB_SYSTEMFLOATWIN =       SAL_CONST_INT64(0x100000000);
WinBits const WB_INTROWIN =             SAL_CONST_INT64(0x200000000);
WinBits const WB_NOSHADOW =             SAL_CONST_INT64(0x400000000);
WinBits const WB_TOOLTIPWIN =           SAL_CONST_INT64(0x800000000);
WinBits const WB_OWNERDRAWDECORATION =  SAL_CONST_INT64(0x2000000000);
WinBits const WB_DEFAULTWIN =           SAL_CONST_INT64(0x4000000000);
WinBits const WB_NEEDSFOCUS =           SAL_CONST_INT64(0x1000000000);
WinBits const WB_POPUP =                SAL_CONST_INT64(0x20000000);

WinBits const WB_HSCROLL =              WB_HORZ;
WinBits const WB_VSCROLL =              WB_VERT;
WinBits const WB_TOPIMAGE =             WB_TOP;

// Window-Bits for PushButtons
WinBits const WB_DEFBUTTON =            0x10000000;
WinBits const WB_NOLIGHTBORDER =        0x20000000;
WinBits const WB_RECTSTYLE =            0x08000000;
WinBits const WB_SMALLSTYLE =           0x04000000;
WinBits const WB_TOGGLE =               SAL_CONST_INT64(0x1000000000);
WinBits const WB_BEVELBUTTON =          SAL_CONST_INT64(0x2000000000);
WinBits const WB_FLATBUTTON =           SAL_CONST_INT64(0x4000000000);

// Window-Bits for FixedText
WinBits const WB_PATHELLIPSIS =         0x00100000;
WinBits const WB_EXTRAOFFSET =          0x02000000;
WinBits const WB_NOMULTILINE =          0x10000000;
WinBits const WB_INFO =                 0x20000000;

// Window-Bits for CheckBox
WinBits const WB_CBLINESTYLE =          SAL_CONST_INT64(0x2000000000);
WinBits const WB_EARLYTOGGLE =          SAL_CONST_INT64(0x4000000000);

// Window-Bits for Edit
WinBits const WB_PASSWORD =             0x01000000;
WinBits const WB_READONLY =             0x02000000;
WinBits const WB_NOHIDESELECTION =      SAL_CONST_INT64(0x1000000000);
WinBits const WB_FORCECTRLBACKGROUND =  0x80000000;

// Window-Bits for MultiLineEdit
WinBits const WB_IGNORETAB =            0x20000000;

// Window-Bits for ListBox and MultiListBox
WinBits const WB_SIMPLEMODE =           0x20000000;

// Window-Bits for FixedBitmap
WinBits const WB_FAST =                 0x04000000;
WinBits const WB_SCALE =                0x08000000;
WinBits const WB_TOPLEFTVISIBLE =       0x10000000;

// Window-Bits for ToolBox
WinBits const WB_LINESPACING =          0x01000000;
WinBits const WB_SCROLL =               0x02000000;
WinBits const WB_FORCETABCYCLE =        0x04000000;

// Window-Bits for DockingWindows
WinBits const WB_DOCKBORDER =           0x00001000;

// Window-Bits for SplitWindow
WinBits const WB_NOSPLITDRAW =          0x01000000;
WinBits const WB_FLATSPLITDRAW =        0x02000000;

// Window-Bits for MessageBoxen
WinBits const WB_OK =                   0x00100000;
WinBits const WB_OK_CANCEL =            0x00200000;
WinBits const WB_YES_NO =               0x00400000;
WinBits const WB_YES_NO_CANCEL =        0x00800000;
WinBits const WB_RETRY_CANCEL =         0x01000000;
WinBits const WB_DEF_OK =               0x02000000;
WinBits const WB_DEF_CANCEL =           0x04000000;
WinBits const WB_DEF_RETRY =            0x08000000;
WinBits const WB_DEF_YES =              SAL_CONST_INT64(0x10000000);
WinBits const WB_DEF_NO =               SAL_CONST_INT64(0x20000000);
WinBits const WB_ABORT_RETRY_IGNORE =   SAL_CONST_INT64(0x1000000000);
WinBits const WB_DEF_IGNORE =           SAL_CONST_INT64(0x2000000000);

// Standard-WinBits
WinBits const WB_STDWORK =              WB_SIZEMOVE | WB_CLOSEABLE;
WinBits const WB_STDDOCKWIN =           WB_DOCKABLE | WB_MOVEABLE | WB_CLOSEABLE;
WinBits const WB_STDFLOATWIN =          WB_SIZEMOVE | WB_CLOSEABLE | WB_ROLLABLE;
WinBits const WB_STDDIALOG =            WB_MOVEABLE | WB_CLOSEABLE;
WinBits const WB_STDMODELESS =          WB_STDDIALOG;
WinBits const WB_STDMODAL =             WB_STDDIALOG;
WinBits const WB_STDTABDIALOG =         WB_STDDIALOG;
WinBits const WB_STDTABCONTROL =        0;
WinBits const WB_STDPOPUP =             WB_BORDER | WB_POPUP | WB_SYSTEMWINDOW | WB_3DLOOK | WB_DIALOGCONTROL;

// For TreeListBox
WinBits const WB_HASBUTTONS =           SAL_CONST_INT64(0x000100000000);
WinBits const WB_HASLINES =             SAL_CONST_INT64(0x000200000000);
WinBits const WB_HASLINESATROOT =       SAL_CONST_INT64(0x000400000000);
WinBits const WB_HASBUTTONSATROOT =     SAL_CONST_INT64(0x000800000000);
WinBits const WB_NOINITIALSELECTION =   SAL_CONST_INT64(0x001000000000);
WinBits const WB_HIDESELECTION =        SAL_CONST_INT64(0x002000000000);
WinBits const WB_FORCE_MAKEVISIBLE =    SAL_CONST_INT64(0x004000000000);
// DO NOT USE: 0x008000000000, that's WB_SYSTEMCHILDWINDOW
WinBits const WB_QUICK_SEARCH =         SAL_CONST_INT64(0x010000000000);

// For FileOpen Dialog
WinBits const WB_PATH =                 0x00100000;
WinBits const WB_OPEN =                 0x00200000;
WinBits const WB_SAVEAS =               0x00400000;

// For Slider
// Window-Bits for TabControl
WinBits const WB_SLIDERSET =            0x02000000;

// extended WinBits
WinBits const WB_EXT_DOCUMENT =         0x00000001;
WinBits const WB_EXT_DOCMODIFIED =      0x00000002;

// WindowAlign

enum WindowAlign { WINDOWALIGN_LEFT, WINDOWALIGN_TOP, WINDOWALIGN_RIGHT, WINDOWALIGN_BOTTOM };
enum ImageAlign { IMAGEALIGN_LEFT, IMAGEALIGN_TOP, IMAGEALIGN_RIGHT, IMAGEALIGN_BOTTOM,
                  IMAGEALIGN_LEFT_TOP, IMAGEALIGN_LEFT_BOTTOM, IMAGEALIGN_TOP_LEFT,
                  IMAGEALIGN_TOP_RIGHT, IMAGEALIGN_RIGHT_TOP, IMAGEALIGN_RIGHT_BOTTOM,
                  IMAGEALIGN_BOTTOM_LEFT, IMAGEALIGN_BOTTOM_RIGHT, IMAGEALIGN_CENTER };
enum class SymbolAlign { LEFT, RIGHT };

// ButtonDialog-Types

enum class StandardButtonType
{
    OK           = 0,
    Cancel       = 1,
    Yes          = 2,
    No           = 3,
    Retry        = 4,
    Help         = 5,
    Close        = 6,
    More         = 7,
    Ignore       = 8,
    Abort        = 9,
    Less         = 10,
    Reset        = 11,
    Count        = 12,
};

// prominent place for ListBox window types

enum class ProminentEntry { TOP, MIDDLE };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
