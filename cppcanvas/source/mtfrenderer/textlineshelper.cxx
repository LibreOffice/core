/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "textlineshelper.hxx"
#include "mtftools.hxx"
#include <outdevstate.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
namespace internal
{
TextLinesHelper::TextLinesHelper(const CanvasSharedPtr& rCanvas, const OutDevState& rState)
    : mpCanvas(rCanvas)
    , mbIsOverlineColorSet(rState.isTextOverlineColorSet)
    , maOverlineColor(rState.textOverlineColor)
    , mbIsUnderlineColorSet(rState.isTextLineColorSet)
    , maUnderlineColor(rState.textLineColor)
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
}

void TextLinesHelper::render(const rendering::RenderState& rRenderState, bool bNormalText) const
{
    const rendering::ViewState& rViewState(mpCanvas->getViewState());
    const uno::Reference<rendering::XCanvas>& xCanvas(mpCanvas->getUNOCanvas());

    if (mxOverline.is())
    {
        if (bNormalText && mbIsOverlineColorSet)
        {
            rendering::RenderState aLocalState(rRenderState);
            aLocalState.DeviceColor = maOverlineColor;
            xCanvas->fillPolyPolygon(mxOverline, rViewState, aLocalState);
        }
        else
            xCanvas->fillPolyPolygon(mxOverline, rViewState, rRenderState);
    }

    if (mxUnderline.is())
    {
        if (bNormalText && mbIsUnderlineColorSet)
        {
            rendering::RenderState aLocalState(rRenderState);
            aLocalState.DeviceColor = maUnderlineColor;
            xCanvas->fillPolyPolygon(mxUnderline, rViewState, aLocalState);
        }
        else
            xCanvas->fillPolyPolygon(mxUnderline, rViewState, rRenderState);
    }

    if (mxStrikeout.is())
        xCanvas->fillPolyPolygon(mxStrikeout, rViewState, rRenderState);
}
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
