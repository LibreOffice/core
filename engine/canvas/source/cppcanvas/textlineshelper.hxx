/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cpo/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <RenderState.hxx>
#include <basegfx/vector/b2dsize.hxx>

namespace cppcanvastools
{
struct TextLineInfo;
}
namespace vclcanvas
{
class Canvas;
}

namespace cppcanvas
{
struct OutDevState;

class TextLinesHelper
{
    basegfx::B2DPolyPolygon mxOverline;
    basegfx::B2DPolyPolygon mxUnderline;
    basegfx::B2DPolyPolygon mxStrikeout;

    bool mbIsOverlineColorSet;
    std::optional<::Color> maOverlineColor;

    bool mbIsUnderlineColorSet;
    std::optional<::Color> maUnderlineColor;

    bool mbOverlineWaveline;
    bool mbUnderlineWaveline;

    bool mbOverlineWavelineBold;
    bool mbUnderlineWavelineBold;

public:
    TextLinesHelper(const OutDevState& rState);

    /** Init textlines with specified linewidth and TextLineInfo.
     */
    void init(double nLineWidth, const cppcanvastools::TextLineInfo& rLineInfo);

    /** Fill the textlines with colors.
        OutDevState::textUnderlineColor.

        @param rRenderState
        Used to invoke XCanvas::fillPolyPolygon.

        @param bNormalText
        Use overline color and underline color if the value is true, ignore those
        colors otherwise ( typical case is to render the shadow ).
     */
    void render(vclcanvas::Canvas& rCanvas, const vclcanvas::ViewState& rViewState,
                const ::vclcanvas::RenderState& rRenderState, bool bNormalText) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
