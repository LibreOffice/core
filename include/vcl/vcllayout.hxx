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

#pragma once

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <tools/gen.hxx>
#include <tools/degree.hxx>

#include <vcl/devicecoordinate.hxx>
#include <vcl/glyphitem.hxx>
#include <vcl/dllapi.h>

class LogicalFontInstance;
namespace vcl::text { class ImplLayoutArgs; }
namespace vcl::font { class PhysicalFontFace; }
class SalGraphics;
class GlyphItem;

// all positions/widths are in font units
// one exception: drawposition is in pixel units

// Unfortunately there is little documentation to help implementors of
// new classes derived from SalLayout ("layout engines"), and the code
// and data structures are far from obvious.

// For instance, I *think* the important virtual functions in the
// layout engines are called in this order:

// * InitFont()
// * LayoutText()
// * AdjustLayout(), any number of times (but presumably
// usually not at all or just once)
// * Optionally, DrawText()

// Functions that just return information like GetTexWidth() and
// FillDXArray() are called after LayoutText() and before DrawText().

// Another important questions is which parts of an ImplLayoutArgs can
// be changed by callers between LayoutText() and AdjustLayout()
// calls. It probably makes sense only if one assumes that the "string
// related inputs" part are not changed after LayoutText().

// But why use the same ImplLayoutArgs structure as parameter for both
// LayoutText() and AdjustLayout() in the first place? And why
// duplicate some of the fields in both SalLayout and ImplLayoutArgs
// (mnMinCharPos, mnEndCharPos, mnLayoutFlags==mnFlags,
// mnOrientation)? Lost in history...

class VCL_DLLPUBLIC SalLayout
{
public:
    virtual         ~SalLayout();
    // used by upper layers
    DevicePoint&    DrawBase()                              { return maDrawBase; }
    const DevicePoint& DrawBase() const                     { return maDrawBase; }
    Point&          DrawOffset()                            { return maDrawOffset; }
    const Point&    DrawOffset() const                      { return maDrawOffset; }
    DevicePoint     GetDrawPosition( const DevicePoint& rRelative = DevicePoint(0,0) ) const;

    virtual bool    LayoutText( vcl::text::ImplLayoutArgs&, const SalLayoutGlyphsImpl* ) = 0;  // first step of layouting
    virtual void    AdjustLayout( vcl::text::ImplLayoutArgs& );    // adjusting after fallback etc.
    virtual void    InitFont() const {}
    virtual void    DrawText( SalGraphics& ) const = 0;

    int             GetUnitsPerPixel() const                { return mnUnitsPerPixel; }
    Degree10        GetOrientation() const                  { return mnOrientation; }

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const = 0;
    virtual DeviceCoordinate FillDXArray( std::vector<DeviceCoordinate>* pDXArray ) const = 0;
    virtual DeviceCoordinate GetTextWidth() const { return FillDXArray( nullptr ); }
    virtual void    GetCaretPositions( int nArraySize, sal_Int32* pCaretXArray ) const = 0;
    virtual bool    IsKashidaPosValid ( int /*nCharPos*/ ) const { return true; } // i60594

    // methods using glyph indexing
    virtual bool    GetNextGlyph(const GlyphItem** pGlyph, DevicePoint& rPos, int& nStart,
                                 const LogicalFontInstance** ppGlyphFont = nullptr,
                                 const vcl::font::PhysicalFontFace** pFallbackFont = nullptr) const = 0;
    virtual bool GetOutline(basegfx::B2DPolyPolygonVector&) const;
    bool GetBoundRect(tools::Rectangle&) const;

    virtual SalLayoutGlyphs GetGlyphs() const;

protected:
    // used by layout engines
    SalLayout();

private:
    SalLayout(const SalLayout&) = delete;
    SalLayout& operator=(const SalLayout&) = delete;

protected:
    int             mnMinCharPos;
    int             mnEndCharPos;

    int             mnUnitsPerPixel;
    Degree10        mnOrientation;

    mutable Point   maDrawOffset;
    DevicePoint     maDrawBase;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
