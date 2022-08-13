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
#include <tools/gen.hxx>
#include <tools/long.hxx>
#include <tools/poly.hxx>

#include <vcl/dllapi.h>

namespace vcl::font
{
class VCL_DLLPUBLIC EmphasisMark
{
public:
    EmphasisMark(FontEmphasisMark eEmphasis, tools::Long nHeight, sal_Int32 nDPIY);

    tools::PolyPolygon GetShape() const { return maPolyPoly; }
    bool IsShapePolyLine() const { return mbIsPolyLine; }
    tools::Rectangle GetRect1() const { return maRect1; }
    tools::Rectangle GetRect2() const { return maRect2; }
    tools::Long GetYOffset() const { return mnYOff; }
    tools::Long GetWidth() const { return mnWidth; }

private:
    tools::PolyPolygon maPolyPoly;
    bool mbIsPolyLine;
    tools::Rectangle maRect1;
    tools::Rectangle maRect2;
    tools::Long mnYOff;
    tools::Long mnWidth;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
