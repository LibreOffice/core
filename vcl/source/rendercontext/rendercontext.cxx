/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <tools/color.hxx>

#include <vcl/RenderContext.hxx>
#include <vcl/virdev.hxx>

RenderContext::RenderContext()
    : maFillColor(COL_WHITE)
    , maTextColor(COL_BLACK)
    , maTextLineColor(COL_TRANSPARENT)
    , mnTextLayoutMode(ComplexTextLayoutFlags)
    , meTextLanguage(LANGUAGE_SYSTEM)
    , meRasterOp(RasterOp::OverPaint)
    , mnOutOffOrigX(0)
    , mnOutOffLogicX(0)
    , mnOutOffOrigY(0)
    , mnOutOffLogicY(0)
    , mbNewFont(false)
    , mpMap(false)
    , mnDPIX(0)
    , mnDPIY(0)
    , mnOutOffX(0)
    , mnOutOffY(0)
    , mnOutWidth(0)
    , mnOutHeight(0)
    , mbLineColor(true)
    , mbFillColor(true)
    , mbInitLineColor(true)
    , mbInitFillColor(true)
    , mbInitTextColor(true)
    , mpAlphaVDev(nullptr)
{
}

void RenderContext2::SetRasterOp(RasterOp eRasterOp)
{
    if (meRasterOp != eRasterOp)
    {
        meRasterOp = eRasterOp;
        mbInitLineColor = mbInitFillColor = true;

        if (mpGraphics || AcquireGraphics())
        {
            assert(mpGraphics);
            mpGraphics->SetXORMode((RasterOp::Invert == meRasterOp)
                                       || (RasterOp::Xor == meRasterOp),
                                   RasterOp::Invert == meRasterOp);
        }
    }

    if (mpAlphaVDev)
        mpAlphaVDev->SetRasterOp(eRasterOp);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
