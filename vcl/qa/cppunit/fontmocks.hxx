/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <font/FontSelectPattern.hxx>
#include <font/PhysicalFontFace.hxx>
#include <font/PhysicalFontFaceCollection.hxx>
#include <fontattributes.hxx>
#include <font/LogicalFontInstance.hxx>

class TestFontInstance : public LogicalFontInstance
{
public:
    TestFontInstance(vcl::font::PhysicalFontFace const& rFontFace,
                     vcl::font::FontSelectPattern const& rFontSelectPattern)
        : LogicalFontInstance(rFontFace, rFontSelectPattern)
    {
    }

    bool GetGlyphOutline(sal_GlyphId, basegfx::B2DPolyPolygon&, bool) const override
    {
        return true;
    }

protected:
    bool ImplGetGlyphBoundRect(sal_GlyphId, tools::Rectangle&, bool) const override { return true; }
};

class TestFontFace : public vcl::font::PhysicalFontFace
{
public:
    TestFontFace(sal_uIntPtr nId)
        : vcl::font::PhysicalFontFace(FontAttributes())
        , mnFontId(nId)
    {
    }

    TestFontFace(FontAttributes const& rFontAttributes, sal_uIntPtr nId)
        : vcl::font::PhysicalFontFace(rFontAttributes)
        , mnFontId(nId)
    {
    }

    rtl::Reference<LogicalFontInstance>
    CreateFontInstance(vcl::font::FontSelectPattern const& rFontSelectPattern) const override
    {
        return new TestFontInstance(*this, rFontSelectPattern);
    }

    sal_IntPtr GetFontId() const override { return mnFontId; }
    FontCharMapRef GetFontCharMap() const override { return FontCharMap::GetDefaultMap(false); }
    bool GetFontCapabilities(vcl::FontCapabilities&) const override { return true; }

private:
    sal_IntPtr mnFontId;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
