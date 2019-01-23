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

#ifndef INCLUDED_VCL_VCLLAYOUT_HXX
#define INCLUDED_VCL_VCLLAYOUT_HXX

#include <memory>

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <tools/gen.hxx>
#include <vcl/devicecoordinate.hxx>
#include <vcl/dllapi.h>

class ImplLayoutArgs;
class PhysicalFontFace;
class SalGraphics;
struct GlyphItem;
class SalLayoutGlyphs;

namespace vcl
{
    class TextLayoutCache;
}

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
    Point&          DrawBase()                              { return maDrawBase; }
    const Point&    DrawBase() const                        { return maDrawBase; }
    Point&          DrawOffset()                            { return maDrawOffset; }
    const Point&    DrawOffset() const                      { return maDrawOffset; }
    Point           GetDrawPosition( const Point& rRelative = Point(0,0) ) const;

    virtual bool    LayoutText( ImplLayoutArgs&, const SalLayoutGlyphs* ) = 0;  // first step of layouting
    virtual void    AdjustLayout( ImplLayoutArgs& );    // adjusting after fallback etc.
    virtual void    InitFont() const {}
    virtual void    DrawText( SalGraphics& ) const = 0;

    int             GetUnitsPerPixel() const                { return mnUnitsPerPixel; }
    int             GetOrientation() const                  { return mnOrientation; }

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(DeviceCoordinate nMaxWidth, DeviceCoordinate nCharExtra, int nFactor) const = 0;
    virtual DeviceCoordinate FillDXArray( DeviceCoordinate* pDXArray ) const = 0;
    virtual DeviceCoordinate GetTextWidth() const { return FillDXArray( nullptr ); }
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const = 0;
    virtual bool    IsKashidaPosValid ( int /*nCharPos*/ ) const { return true; } // i60594

    // methods using glyph indexing
    virtual bool    GetNextGlyph(const GlyphItem** pGlyph, Point& rPos, int& nStart,
                                 const PhysicalFontFace** pFallbackFont = nullptr,
                                 int* const pFallbackLevel = nullptr) const = 0;
    virtual bool GetOutline(basegfx::B2DPolyPolygonVector&) const;
    bool GetBoundRect(tools::Rectangle&) const;

    virtual const SalLayoutGlyphs* GetGlyphs() const;

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
    int             mnOrientation;

    mutable Point   maDrawOffset;
    Point           maDrawBase;
};

#endif // INCLUDED_VCL_VCLLAYOUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
