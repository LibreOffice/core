/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_TEXTLINESHELPER_HXX
#define INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_TEXTLINESHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <canvasgraphichelper.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace rendering
{
class XPolyPolygon2D;
}
}
}
}

namespace cppcanvas
{
namespace tools
{
struct TextLineInfo;
}

namespace internal
{
struct OutDevState;

class TextLinesHelper
{
    const CanvasSharedPtr mpCanvas;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxOverline;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxUnderline;
    css::uno::Reference<css::rendering::XPolyPolygon2D> mxStrikeout;

    ::basegfx::B2DSize maOverallSize;

    bool mbIsOverlineColorSet;
    const css::uno::Sequence<double> maOverlineColor;

    bool mbIsUnderlineColorSet;
    const css::uno::Sequence<double> maUnderlineColor;

    bool mbOverlineWaveline;
    bool mbUnderlineWaveline;

    bool mbOverlineWavelineBold;
    bool mbUnderlineWavelineBold;

public:
    TextLinesHelper(const CanvasSharedPtr& rCanvas, const OutDevState& rState);

    ::basegfx::B2DSize const& getOverallSize() const { return maOverallSize; }

    /** Init textlines with specified linewidth and TextLineInfo.
     */
    void init(double nLineWidth, const tools::TextLineInfo& rLineInfo);

    /** Fill the textlines with colors.
        OutDevState::textUnderlineColor.

        @param rRenderState
        Used to invoke XCanvas::fillPolyPolygon.

        @param bNormalText
        Use overline color and underline color if the value is true, ignore those
        colors otherwise ( typical case is to render the shadow ).
     */
    void render(const css::rendering::RenderState& rRenderState, bool bNormalText) const;
};
}
}
#endif // INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_TEXTLINESHELPER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
