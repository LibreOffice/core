/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: postwin.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:14:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* nicht geschuetz, muss als gegenstueck zu prewin.h includet werden */

#ifdef WNT
#ifdef __cplusplus
}
#endif
#endif

#undef Region
#undef PolyPolygon
#undef Polygon
#undef Rectangle
#undef BYTE
#undef BOOL
#undef DELETE

#ifndef VCL_NEED_BASETSD
#undef INT64
#undef UINT64
#undef INT32
#undef UINT32
#endif
#undef Folder
#undef GradientStyle_RECT

/* Hilfe-Ids umbenennen */
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

// keine Yield-Definition
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

#ifdef WNT
extern "C"
{
WIN_BOOL WINAPI WIN_Rectangle( HDC hDC, int X1, int Y1, int X2, int Y2 );
WIN_BOOL WINAPI WIN_Polygon( HDC hDC, CONST POINT * ppt, int ncnt );
WIN_BOOL WINAPI WIN_PolyPolygon( HDC hDC, CONST POINT * ppt, LPINT npcnt, int ncnt );
}
#endif

// svwin.h nicht mehr includen
#define _SVWIN_H
