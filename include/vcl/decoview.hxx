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
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/vclenum.hxx>
#include <o3tl/typed_flags_set.hxx>

namespace tools { class Rectangle; }
class Point;
class Color;

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
    Disabled                = 0x0040,
    Highlight               = 0x0100,
    Flat                    = 0x0200,
    NoLeftLightBorder       = 0x1000
};
namespace o3tl
{
    template<> struct typed_flags<DrawButtonFlags> : is_typed_flags<DrawButtonFlags, 0x137f> {};
}

class VCL_DLLPUBLIC DecorationView
{
private:
    VclPtr<OutputDevice>  mpOutDev;

public:
    DecorationView(OutputDevice* pOutDev);

    void                DrawSymbol( const tools::Rectangle& rRect, SymbolType eType,
                                    const Color& rColor, DrawSymbolFlags nStyle = DrawSymbolFlags::NONE );
    void                DrawFrame( const tools::Rectangle& rRect,
                                   const Color& rLeftTopColor,
                                   const Color& rRightBottomColor );
    void                DrawHighlightFrame( const tools::Rectangle& rRect,
                                            DrawHighlightFrameStyle nStyle );
    tools::Rectangle           DrawFrame( const tools::Rectangle& rRect, DrawFrameStyle nStyle = DrawFrameStyle::Out, DrawFrameFlags nFlags = DrawFrameFlags::NONE );
    tools::Rectangle           DrawButton( const tools::Rectangle& rRect, DrawButtonFlags nStyle );
    void                DrawSeparator( const Point& rStart, const Point& rStop, bool bVertical = true );
    void                DrawHandle(const tools::Rectangle& rRectangle);
};

#endif // INCLUDED_VCL_DECOVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
