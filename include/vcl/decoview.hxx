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

#ifndef INCLUDED_VCL_DECOVIEW_HXX
#define INCLUDED_VCL_DECOVIEW_HXX

#include <vcl/dllapi.h>
#include <vcl/vclptr.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <o3tl/typed_flags_set.hxx>

class Rectangle;
class Point;
class Color;
class OutputDevice;

// Flags for DrawSymbol()
enum class DrawSymbolFlags
{
    NONE                    = 0x0000,
    Mono                    = 0x0001,
    Disable                 = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<DrawSymbolFlags> : is_typed_flags<DrawSymbolFlags, 0x03> {};
}

// Flags for DrawFrame()
enum class DrawFrameStyle
{
    NONE                     = 0x0000,
    In                       = 0x0001,
    Out                      = 0x0002,
    Group                    = 0x0003,
    DoubleIn                 = 0x0004,
    DoubleOut                = 0x0005,
    NWF                      = 0x0006,
};
enum class DrawFrameFlags
{
    NONE                     = 0x0000,
    Menu                     = 0x0010,
    WindowBorder             = 0x0020,
    BorderWindowBorder       = 0x0040,
    Mono                     = 0x1000,
    NoDraw                   = 0x8000,
};
namespace o3tl
{
    template<> struct typed_flags<DrawFrameFlags> : is_typed_flags<DrawFrameFlags, 0x9070> {};
}

// Flags for DrawHighlightFrame()
enum class DrawHighlightFrameStyle
{
    In                  = 1,
    Out                 = 2,
};

// Flags for DrawButton()
enum class DrawButtonFlags
{
    NONE                    = 0x0000,
    Default                 = 0x0001,
    NoLightBorder           = 0x0002,
    Pressed                 = 0x0004,
    Checked                 = 0x0008,
    DontKnow                = 0x0010,
    Mono                    = 0x0020,
    NoFill                  = 0x0040,
    Disabled                = 0x0080,
    Highlight               = 0x0100,
    Flat                    = 0x0200,
    NoLeftLightBorder       = 0x1000,
    NoText                  = 0x2000,
    NoImage                 = 0x4000,
};
namespace o3tl
{
    template<> struct typed_flags<DrawButtonFlags> : is_typed_flags<DrawButtonFlags, 0x73ff> {};
}

class VCL_DLLPUBLIC DecorationView
{
private:
    VclPtr<OutputDevice>  mpOutDev;

public:
    DecorationView(OutputDevice* pOutDev);

    void                DrawSymbol( const Rectangle& rRect, SymbolType eType,
                                    const Color& rColor, DrawSymbolFlags nStyle = DrawSymbolFlags::NONE );
    void                DrawFrame( const Rectangle& rRect,
                                   const Color& rLeftTopColor,
                                   const Color& rRightBottomColor );
    void                DrawHighlightFrame( const Rectangle& rRect,
                                            DrawHighlightFrameStyle nStyle = DrawHighlightFrameStyle::Out, bool bTestBackground = false );
    Rectangle           DrawFrame( const Rectangle& rRect, DrawFrameStyle nStyle = DrawFrameStyle::Out, DrawFrameFlags nFlags = DrawFrameFlags::NONE );
    Rectangle           DrawButton( const Rectangle& rRect, DrawButtonFlags nStyle );
    void                DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical = true );
    void                DrawHandle(const Rectangle& rRectangle, bool bVertical = true);
};

#endif // INCLUDED_VCL_DECOVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
