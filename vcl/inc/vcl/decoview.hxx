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

#ifndef _SV_DECOVIEW_HXX
#define _SV_DECOVIEW_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/symbol.hxx>

class Rectangle;
class Point;
class Color;
class OutputDevice;

// ------------------------
// - DecorationView-Types -
// ------------------------

// Flags for DrawSymbol()
#define SYMBOL_DRAW_MONO                    ((sal_uInt16)0x0001)
#define SYMBOL_DRAW_DISABLE                 ((sal_uInt16)0x0002)

// Flags for DrawFrame()
#define FRAME_DRAW_IN                       ((sal_uInt16)0x0001)
#define FRAME_DRAW_OUT                      ((sal_uInt16)0x0002)
#define FRAME_DRAW_GROUP                    ((sal_uInt16)0x0003)
#define FRAME_DRAW_DOUBLEIN                 ((sal_uInt16)0x0004)
#define FRAME_DRAW_DOUBLEOUT                ((sal_uInt16)0x0005)
#define FRAME_DRAW_NWF                      ((sal_uInt16)0x0006)
#define FRAME_DRAW_MENU                     ((sal_uInt16)0x0010)
#define FRAME_DRAW_WINDOWBORDER             ((sal_uInt16)0x0020)
#define FRAME_DRAW_BORDERWINDOWBORDER       ((sal_uInt16)0x0040)
#define FRAME_DRAW_MONO                     ((sal_uInt16)0x1000)
#define FRAME_DRAW_NODRAW                   ((sal_uInt16)0x8000)
#define FRAME_DRAW_STYLE                    ((sal_uInt16)0x000F)

// Flags for DrawHighlightFrame()
#define FRAME_HIGHLIGHT_IN                  ((sal_uInt16)0x0001)
#define FRAME_HIGHLIGHT_OUT                 ((sal_uInt16)0x0002)
#define FRAME_HIGHLIGHT_TESTBACKGROUND      ((sal_uInt16)0x4000)
#define FRAME_HIGHLIGHT_STYLE               ((sal_uInt16)0x000F)

// Flags for DrawButton()
#define BUTTON_DRAW_DEFAULT                 ((sal_uInt16)0x0001)
#define BUTTON_DRAW_NOLIGHTBORDER           ((sal_uInt16)0x0002)
#define BUTTON_DRAW_PRESSED                 ((sal_uInt16)0x0004)
#define BUTTON_DRAW_CHECKED                 ((sal_uInt16)0x0008)
#define BUTTON_DRAW_DONTKNOW                ((sal_uInt16)0x0010)
#define BUTTON_DRAW_MONO                    ((sal_uInt16)0x0020)
#define BUTTON_DRAW_NOFILL                  ((sal_uInt16)0x0040)
#define BUTTON_DRAW_DISABLED                ((sal_uInt16)0x0080)
#define BUTTON_DRAW_HIGHLIGHT               ((sal_uInt16)0x0100)
#define BUTTON_DRAW_FLAT                    ((sal_uInt16)0x0200)
#define BUTTON_DRAW_NOLEFTLIGHTBORDER       ((sal_uInt16)0x1000)
#define BUTTON_DRAW_NOTEXT                  ((sal_uInt16)0x2000)
#define BUTTON_DRAW_NOIMAGE                 ((sal_uInt16)0x4000)

// ------------------
// - DecorationView -
// ------------------

class VCL_DLLPUBLIC DecorationView
{
private:
    OutputDevice*       mpOutDev;

public:
                        DecorationView( OutputDevice* pOutDev )
                            { mpOutDev = pOutDev; }

    void                DrawSymbol( const Rectangle& rRect, SymbolType eType,
                                    const Color& rColor, sal_uInt16 nStyle = 0 );
    void                DrawFrame( const Rectangle& rRect,
                                   const Color& rLeftTopColor,
                                   const Color& rRightBottomColor );
    void                DrawHighlightFrame( const Rectangle& rRect,
                                            sal_uInt16 nStyle = FRAME_HIGHLIGHT_OUT );
    Rectangle           DrawFrame( const Rectangle& rRect, sal_uInt16 nStyle = FRAME_DRAW_OUT );
    Rectangle           DrawButton( const Rectangle& rRect, sal_uInt16 nStyle );
    void                DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical = true );
};

#endif // _SV_DECOVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
