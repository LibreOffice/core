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

enum class WindowType : sal_uInt16
{
    NONE                 = 0,
    FIRST                = 0x0130,
    MESSBOX              = FIRST,
    INFOBOX              ,
    WARNINGBOX           ,
    ERRORBOX             ,
    QUERYBOX             ,
    WINDOW               ,
    WORKWINDOW           ,
    CONTAINER            ,
    FLOATINGWINDOW       ,
    DIALOG               ,
    MODELESSDIALOG       ,
    MODALDIALOG          ,
    CONTROL              ,
    PUSHBUTTON           ,
    OKBUTTON             ,
    CANCELBUTTON         ,
    HELPBUTTON           ,
    IMAGEBUTTON          ,
    MENUBUTTON           ,
    MOREBUTTON           ,
    SPINBUTTON           ,
    RADIOBUTTON          ,
    CHECKBOX             ,
    TRISTATEBOX          ,
    EDIT                 ,
    MULTILINEEDIT        ,
    COMBOBOX             ,
    LISTBOX              ,
    MULTILISTBOX         ,
    FIXEDTEXT            ,
    FIXEDLINE            ,
    FIXEDBITMAP          ,
    FIXEDIMAGE           ,
    GROUPBOX             ,
    SCROLLBAR            ,
    SCROLLBARBOX         ,
    SPLITTER             ,
    SPLITWINDOW          ,
    SPINFIELD            ,
    PATTERNFIELD         ,
    NUMERICFIELD         ,
    METRICFIELD          ,
    CURRENCYFIELD        ,
    DATEFIELD            ,
    TIMEFIELD            ,
    PATTERNBOX           ,
    NUMERICBOX           ,
    METRICBOX            ,
    CURRENCYBOX          ,
    DATEBOX              ,
    TIMEBOX              ,
    LONGCURRENCYFIELD    ,
    LONGCURRENCYBOX      ,
    SCROLLWINDOW         ,
    TOOLBOX              ,
    DOCKINGWINDOW        ,
    STATUSBAR            ,
    TABPAGE              ,
    TABCONTROL           ,
    TABDIALOG            ,
    BORDERWINDOW         ,
    BUTTONDIALOG         ,
    SYSTEMCHILDWINDOW    ,
    SLIDER               ,
    MENUBARWINDOW        ,
    TREELISTBOX          ,
    HELPTEXTWINDOW       ,
    INTROWINDOW          ,
    LISTBOXWINDOW        ,
    DOCKINGAREA          ,
    RULER                ,
    CALCINPUTLINE        ,
    LAST                 = CALCINPUTLINE,
    // only used in vclxtoolkit.cxx
    TOOLKIT_FRAMEWINDOW        = 0x1000,
    TOOLKIT_SYSTEMCHILDWINDOW  = 0x1001,
};

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

// Window-Bits for SystemWindows
WinBits const WB_MOVEABLE =             0x00000100;
WinBits const WB_ROLLABLE =             0x00000200;
WinBits const WB_CLOSEABLE =            0x00000400;
WinBits const WB_STANDALONE =           0x00000800;
WinBits const WB_APP =                  0x00001000;
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
WinBits const WB_POPUP =                SAL_CONST_INT64(0x20000000);

WinBits const WB_HSCROLL =              WB_HORZ;
WinBits const WB_VSCROLL =              WB_VERT;

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

// Window-Bits for CheckBox
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
WinBits const WB_SCALE =                0x08000000;

// Window-Bits for ToolBox
WinBits const WB_LINESPACING =          0x01000000;
WinBits const WB_SCROLL =               0x02000000;

// Window-Bits for SplitWindow
WinBits const WB_NOSPLITDRAW =          0x01000000;

// Standard-WinBits
WinBits const WB_STDWORK =              WB_SIZEMOVE | WB_CLOSEABLE;
WinBits const WB_STDDOCKWIN =           WB_DOCKABLE | WB_MOVEABLE | WB_CLOSEABLE;
WinBits const WB_STDFLOATWIN =          WB_SIZEMOVE | WB_CLOSEABLE | WB_ROLLABLE;
WinBits const WB_STDDIALOG =            WB_MOVEABLE | WB_CLOSEABLE;
WinBits const WB_STDMODELESS =          WB_STDDIALOG;
WinBits const WB_STDMODAL =             WB_STDDIALOG;
WinBits const WB_STDTABCONTROL =        0;
WinBits const WB_STDPOPUP =             WB_BORDER | WB_POPUP | WB_SYSTEMWINDOW | WB_3DLOOK | WB_DIALOGCONTROL;

// For TreeListBox
WinBits const WB_HASBUTTONS =           SAL_CONST_INT64(0x000100000000);
WinBits const WB_HASLINES =             SAL_CONST_INT64(0x000200000000);
WinBits const WB_HASLINESATROOT =       SAL_CONST_INT64(0x000400000000);
WinBits const WB_HASBUTTONSATROOT =     SAL_CONST_INT64(0x000800000000);
WinBits const WB_NOINITIALSELECTION =   SAL_CONST_INT64(0x001000000000);
WinBits const WB_HIDESELECTION =        SAL_CONST_INT64(0x002000000000);
// DO NOT USE: 0x008000000000, that's WB_SYSTEMCHILDWINDOW


enum class WindowAlign { Left, Top, Right, Bottom };

enum class ImageAlign { Left, Top, Right, Bottom,
                  LeftTop, LeftBottom, TopLeft,
                  TopRight, RightTop, RightBottom,
                  BottomLeft, BottomRight, Center };

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
    Count        = 11,
};

// prominent place for ListBox window types

enum class ProminentEntry { TOP, MIDDLE };

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
