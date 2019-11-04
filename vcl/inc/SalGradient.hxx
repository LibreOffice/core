/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_SALGRADIENT_HXX
#define INCLUDED_VCL_INC_SALGRADIENT_HXX

#include <basegfx/point/b2dpoint.hxx>

struct SalGradientStop
{
    Color maColor;
    float mfOffset;

    SalGradientStop(Color const& rColor, float fOffset)
        : maColor(rColor)
        , mfOffset(fOffset)
    {
    }
};

struct SalGradient
{
    basegfx::B2DPoint maPoint1;
    basegfx::B2DPoint maPoint2;
    std::vector<SalGradientStop> maStops;
};

#endif // INCLUDED_VCL_INC_SALGRADIENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
