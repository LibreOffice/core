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

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/gen.hxx>
#include <tools/degree.hxx>

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
    basegfx::B2DPoint& DrawBase()                           { return maDrawBase; }
    const basegfx::B2DPoint& DrawBase() const               { return maDrawBase; }
    Point&          DrawOffset()                            { return maDrawOffset; }
    const Point&    DrawOffset() const                      { return maDrawOffset; }
    basegfx::B2DPoint GetDrawPosition( const basegfx::B2DPoint& rRelative = basegfx::B2DPoint(0,0) ) const;

    virtual bool    LayoutText( vcl::text::ImplLayoutArgs&, const SalLayoutGlyphsImpl* ) = 0;  // first step of layouting
    virtual void    AdjustLayout( vcl::text::ImplLayoutArgs& );    // adjusting after fallback etc.
    virtual void    InitFont() const {}
    virtual void    DrawText( SalGraphics& ) const = 0;

    Degree10        GetOrientation() const                  { return mnOrientation; }

    void            SetSubpixelPositioning(bool bSubpixelPositioning)
    {
        mbSubpixelPositioning = bSubpixelPositioning;
    }

    bool            GetSubpixelPositioning() const
    {
        return mbSubpixelPositioning;
    }

    // methods using string indexing
    virtual sal_Int32 GetTextBreak(double nMaxWidth, double nCharExtra, int nFactor) const = 0;
    virtual double  FillDXArray( std::vector<double>* pDXArray, const OUString& rStr ) const = 0;
    virtual double  GetTextWidth() const { return FillDXArray( nullptr, {} ); }
    virtual void    GetCaretPositions( std::vector<double>& rCaretPositions, const OUString& rStr ) const = 0;
    virtual bool    IsKashidaPosValid ( int /*nCharPos*/, int /*nNextCharPos*/ ) const = 0; // i60594

    // methods using glyph indexing
    virtual bool    GetNextGlyph(const GlyphItem** pGlyph, basegfx::B2DPoint& rPos, int& nStart,
                                 const LogicalFontInstance** ppGlyphFont = nullptr) const = 0;
    virtual bool GetOutline(basegfx::B2DPolyPolygonVector&) const;
    bool GetBoundRect(basegfx::B2DRectangle&) const;

    static tools::Rectangle BoundRect2Rectangle(basegfx::B2DRectangle&);

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
    LanguageTag     maLanguageTag;

    Degree10        mnOrientation;

    mutable Point   maDrawOffset;
    basegfx::B2DPoint maDrawBase;

    bool            mbSubpixelPositioning;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
