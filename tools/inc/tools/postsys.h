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

/* not protected, must be included together with prestl.h! */

#if defined WNT

#undef Region
#undef PolyPolygon
#undef Polygon
#undef Rectangle
#undef DELETE

/* rename help IDs */
#define WIN_HELP_INDEX       0x0003
#define WIN_HELP_HELPONHELP  0x0004
#undef HELP_INDEX
#undef HELP_HELPONHELP

#define WIN_MOUSE_MOVED 0x0001
#undef MOUSE_MOVED

#define WIN_WB_LEFT               0
#define WIN_WB_RIGHT              1
#undef WB_LEFT
#undef WB_RIGHT

#ifdef GetObject
#undef GetObject
#ifdef UNICODE
#define WIN_GetObject   GetObjectW
#else
#define WIN_GetObject   GetObjectA
#endif
#else
#define WIN_GetObject   GetObject
#endif

#ifdef SetPrinter
#undef SetPrinter
#ifdef UNICODE
#define WIN_SetPrinter  SetPrinterW
#else
#define WIN_SetPrinter  SetPrinterA
#endif
#else
#define WIN_SetPrinter  SetPrinter
#endif

#ifdef GetPrinter
#undef GetPrinter
#ifdef UNICODE
#define WIN_GetPrinter  GetPrinterW
#else
#define WIN_GetPrinter  GetPrinterA
#endif
#else
#define WIN_GetPrinter  GetPrinter
#endif

#ifdef DrawText
#undef DrawText
#ifdef UNICODE
#define WIN_DrawText    DrawTextW
#else
#define WIN_DrawText    DrawTextA
#endif
#else
#define WIN_DrawText    DrawText
#endif

#ifdef mciSetCommand
#undef mciSetCommand
#ifdef UNICODE
#define WIN_mciSetCommand   mciSetCommandW
#else
#define WIN_mciSetCommand   mciSetCommandA
#endif
#else
#define mciSetCommand   mciSetCommand
#endif

#ifdef SetPort
#undef SetPort
#ifdef UNICODE
#define WIN_SetPort     SetPortW
#else
#define WIN_SetPort     SetPortA
#endif
#else
#define WIN_SetPort     SetPort
#endif

#ifdef CopyFile
#undef CopyFile
#ifdef UNICODE
#define WIN_CopyFile    CopyFileW
#else
#define WIN_CopyFile    CopyFileA
#endif
#else
#define WIN_CopyFile    CopyFile
#endif

#ifdef GetUserName
#undef GetUserName
#ifdef UNICODE
#define WIN_GetUserName GetUserNameW
#else
#define WIN_GetUserName GetUserNameA
#endif
#else
#define WIN_GetUserName GetUserName
#endif

#ifdef GetClassName
#undef GetClassName
#ifdef UNICODE
#define WIN_GetClassName GetClassNameW
#else
#define WIN_GetClassName GetClassNameA
#endif
#else
#define WIN_GetClassName GetClassName
#endif

#ifdef GetCharWidth
#undef GetCharWidth
#ifdef UNICODE
#define WIN_GetCharWidth GetCharWidthW
#else
#define WIN_GetCharWidth GetCharWidthA
#endif
#else
#define WIN_GetCharWidth GetCharWidth
#endif

#ifdef GetMetaFile
#undef GetMetaFile
#ifdef UNICODE
#define WIN_GetMetaFile GetMetaFileW
#else
#define WIN_GetMetaFile GetMetaFileA
#endif
#else
#define WIN_GetMetaFile GetMetaFile
#endif

#ifdef GetNumberFormat
#undef GetNumberFormat
#ifdef UNICODE
#define WIN_GetNumberFormat GetNumberFormatW
#else
#define WIN_GetNumberFormat GetNumberFormatA
#endif
#else
#define WIN_GetNumberFormat GetNumberFormat
#endif

#ifdef GetGlyphOutline
#undef GetGlyphOutline
#ifdef UNICODE
#define WIN_GetGlyphOutline GetGlyphOutlineW
#else
#define WIN_GetGlyphOutline GetGlyphOutlineA
#endif
#else
#define WIN_GetGlyphOutline GetGlyphOutline
#endif

#ifdef GetKerningPairs
#undef GetKerningPairs
#ifdef UNICODE
#define WIN_GetKerningPairs GetKerningPairsW
#else
#define WIN_GetKerningPairs GetKerningPairsA
#endif
#else
#define WIN_GetKerningPairs GetKerningPairs
#endif

#ifdef FindWindow
#undef FindWindow
#ifdef UNICODE
#define WIN_FindWindow FindWindowW
#else
#define WIN_FindWindow FindWindowA
#endif
#else
#define WIN_FindWindow FindWindow
#endif

#undef Yield

/* new StretchBlt() Modes (simpler names) */
#ifndef STRETCH_ANDSCANS
#define STRETCH_ANDSCANS        1
#endif
#ifndef STRETCH_ORSCANS
#define STRETCH_ORSCANS         2
#endif
#ifndef STRETCH_DELETESCANS
#define STRETCH_DELETESCANS     3
#endif

extern "C"
{
    BOOL WINAPI WIN_Rectangle( HDC hDC, int X1, int Y1, int X2, int Y2 );
    BOOL WINAPI WIN_Polygon( HDC hDC, CONST POINT * ppt, int ncnt );
    BOOL WINAPI WIN_PolyPolygon( HDC hDC, CONST POINT * ppt, LPINT npcnt, int ncnt );
}

#endif

// avoid conflict with Pair in tools
#undef Pair

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
