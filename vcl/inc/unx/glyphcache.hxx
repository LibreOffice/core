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

#include <sal/config.h>

#include <memory>
#include <freetype/config/ftheader.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <unx/gendata.hxx>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <vcl/unx/freetypemanager.hxx>

#include <fontattributes.hxx>
#include <fontinstance.hxx>
#include <impfontmetricdata.hxx>

#include <unordered_map>

class FreetypeFontInstance;
class FreetypeFontInfo;
class FontConfigFontOptions;
namespace vcl::font
{
class PhysicalFontCollection;
}

namespace basegfx { class B2DPolyPolygon; }
namespace vcl { struct FontCapabilities; }

class VCL_DLLPUBLIC FreetypeFont final
{
public:
                            ~FreetypeFont();

    const OString&          GetFontFileName() const;
    int                     GetFontFaceIndex() const;
    int                     GetFontFaceVariation() const;
    bool                    TestFont() const { return mbFaceOk;}
    FT_Face                 GetFtFace() const;
    const FontConfigFontOptions* GetFontOptions() const;
    bool                    NeedsArtificialBold() const { return mbArtBold; }
    bool                    NeedsArtificialItalic() const { return mbArtItalic; }

    void                    GetFontMetric(ImplFontMetricDataRef const &) const;
    const unsigned char*    GetTable( const char* pName, sal_uLong* pLength ) const;
    const FontCharMapRef &  GetFontCharMap() const;
    bool                    GetFontCapabilities(vcl::FontCapabilities &) const;

    bool                    GetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const;
    bool                    GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const;
    bool                    GetAntialiasAdvice() const;

    void                    SetFontVariationsOnHBFont(hb_font_t* pHbFace) const;

    // tdf#127189 FreeType <= 2.8 will fail to render stretched horizontal brace glyphs
    // in starmath at a fairly low stretch ratio. This appears fixed in 2.9 with
    // https://git.savannah.gnu.org/cgit/freetype/freetype2.git/commit/?id=91015cb41d8f56777f93394f5a60914bc0c0f330
    // "Improve complex rendering at high ppem"
    static bool             AlmostHorizontalDrainsRenderingPool(int nRatio, const vcl::font::FontSelectPattern& rFSD);

private:
    friend class FreetypeFontInstance;
    friend class FreetypeManager;

    explicit FreetypeFont(FreetypeFontInstance&, const std::shared_ptr<FreetypeFontInfo>& rFontInfo);

    void                    ApplyGlyphTransform(bool bVertical, FT_Glyph) const;

    FreetypeFontInstance& mrFontInstance;

    // 16.16 fixed point values used for a rotated font
    tools::Long                    mnCos;
    tools::Long                    mnSin;

    int                     mnWidth;
    int                     mnPrioAntiAlias;
    std::shared_ptr<FreetypeFontInfo> mxFontInfo;
    FT_Int                  mnLoadFlags;
    double                  mfStretch;
    FT_FaceRec_*            maFaceFT;
    FT_SizeRec_*            maSizeFT;

    mutable std::unique_ptr<FontConfigFontOptions> mxFontOptions;

    bool                    mbFaceOk;
    bool                    mbArtItalic;
    bool                    mbArtBold;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
