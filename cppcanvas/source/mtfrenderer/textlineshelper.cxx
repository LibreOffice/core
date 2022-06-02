/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/StrokeAttributes.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <outdevstate.hxx>
#include <utility>
#include "textlineshelper.hxx"
#include "mtftools.hxx"

using namespace ::com::sun::star;

namespace
{
void initLineStyleWaveline(sal_uInt32 nLineStyle, bool& bIsWaveline, bool& bIsBold)
{
    bIsWaveline = nLineStyle == LINESTYLE_DOUBLEWAVE || nLineStyle == LINESTYLE_SMALLWAVE
                  || nLineStyle == LINESTYLE_BOLDWAVE || nLineStyle == LINESTYLE_WAVE;
    bIsBold = nLineStyle == LINESTYLE_BOLDWAVE;
}
}

namespace cppcanvas::internal
{
TextLinesHelper::TextLinesHelper(CanvasSharedPtr xCanvas, const OutDevState& rState)
    : mpCanvas(std::move(xCanvas))
    , mbIsOverlineColorSet(rState.isTextOverlineColorSet)
    , maOverlineColor(rState.textOverlineColor)
    , mbIsUnderlineColorSet(rState.isTextLineColorSet)
    , maUnderlineColor(rState.textLineColor)
    , mbOverlineWaveline(false)
    , mbUnderlineWaveline(false)
    , mbOverlineWavelineBold(false)
    , mbUnderlineWavelineBold(false)
{
}

void TextLinesHelper::init(double nLineWidth, const tools::TextLineInfo& rLineInfo)
{
    ::basegfx::B2DRange aRange; // default is empty.
    ::basegfx::B2DPolyPolygon aOverline, aUnderline, aStrikeout;
    tools::createTextLinesPolyPolygon(0.0, nLineWidth, rLineInfo, aOverline, aUnderline,
                                      aStrikeout);

    mxOverline.clear();
    mxUnderline.clear();
    mxStrikeout.clear();

    uno::Reference<rendering::XGraphicDevice> xDevice = mpCanvas->getUNOCanvas()->getDevice();

    if (aOverline.count())
    {
        aRange.expand(::basegfx::utils::getRange(aOverline));
        mxOverline = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(xDevice, aOverline);
    }

    if (aUnderline.count())
    {
        aRange.expand(::basegfx::utils::getRange(aUnderline));
        mxUnderline = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(xDevice, aUnderline);
    }

    if (aStrikeout.count())
    {
        aRange.expand(::basegfx::utils::getRange(aStrikeout));
        mxStrikeout = ::basegfx::unotools::xPolyPolygonFromB2DPolyPolygon(xDevice, aStrikeout);
    }

    maOverallSize = aRange.getRange();

    initLineStyleWaveline(rLineInfo.mnOverlineStyle, mbOverlineWaveline, mbOverlineWavelineBold);

    initLineStyleWaveline(rLineInfo.mnUnderlineStyle, mbUnderlineWaveline, mbUnderlineWavelineBold);
}

void TextLinesHelper::render(const rendering::RenderState& rRenderState, bool bNormalText) const
{
    const rendering::ViewState& rViewState(mpCanvas->getViewState());
    const uno::Reference<rendering::XCanvas>& xCanvas(mpCanvas->getUNOCanvas());
    rendering::StrokeAttributes aStrokeAttributes;
    aStrokeAttributes.JoinType = rendering::PathJoinType::ROUND;

    if (mxOverline.is())
    {
        rendering::RenderState aLocalState(rRenderState);
        if (bNormalText && mbIsOverlineColorSet)
            aLocalState.DeviceColor = maOverlineColor;

        if (mbOverlineWaveline)
        {
            aStrokeAttributes.StrokeWidth = mbOverlineWavelineBold ? 2.0 : 1.0;
            xCanvas->strokePolyPolygon(mxOverline, rViewState, aLocalState, aStrokeAttributes);
        }
        else
            xCanvas->fillPolyPolygon(mxOverline, rViewState, aLocalState);
    }

    if (mxUnderline.is())
    {
        rendering::RenderState aLocalState(rRenderState);
        if (bNormalText && mbIsUnderlineColorSet)
            aLocalState.DeviceColor = maUnderlineColor;
        if (mbUnderlineWaveline)
        {
            aStrokeAttributes.StrokeWidth = mbUnderlineWavelineBold ? 2.0 : 1.0;
            xCanvas->strokePolyPolygon(mxUnderline, rViewState, aLocalState, aStrokeAttributes);
        }
        else
            xCanvas->fillPolyPolygon(mxUnderline, rViewState, aLocalState);
    }

    if (mxStrikeout.is())
        xCanvas->fillPolyPolygon(mxStrikeout, rViewState, rRenderState);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
