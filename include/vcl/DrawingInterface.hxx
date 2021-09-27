/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <tools/fontenum.hxx>
#include <i18nlangtag/lang.h>

#include <vcl/mapmod.hxx>
#include <vcl/outdevstate.hxx>
#include <vcl/rendercontext/RasterOp.hxx>

class Color;

namespace vcl
{
class Font;

class DrawingInterface
{
public:
    virtual ~DrawingInterface() = default;

    virtual void SetFont(vcl::Font const& rNewFont) = 0;
    virtual void SetTextColor(Color const& rColor) = 0;
    virtual void SetTextFillColor(Color const& rColor) = 0;
    virtual void SetTextLineColor(Color const& rColor) = 0;
    virtual void SetLineColor(Color const& rColor) = 0;
    virtual void SetFillColor(Color const& rColor) = 0;
    virtual void SetRasterOp(RasterOp eRasterOp) = 0;
    virtual void SetMapMode(MapMode const& rNewMapMode) = 0;
    virtual void SetTextAlign(TextAlign eAlign) = 0;
    virtual void SetLayoutMode(ComplexTextLayoutFlags nTextLayoutMode) = 0;
    virtual void SetDigitLanguage(LanguageType) = 0;
    virtual void Push(PushFlags nFlags = PushFlags::ALL) = 0;
    virtual void Pop() = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
