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

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>
#include <vector>

namespace basegfx
{
class ColorStop;
class BColor;
typedef std::vector<ColorStop> ColorStops;
}

namespace drawinglayer::attribute
{
class ImpFillGradientAttribute;

enum class GradientStyle
{
    Linear,
    Axial,
    Radial,
    Elliptical,
    Square,
    Rect
};

class DRAWINGLAYER_DLLPUBLIC FillGradientAttribute
{
public:
    typedef o3tl::cow_wrapper<ImpFillGradientAttribute> ImplType;

private:
    ImplType mpFillGradientAttribute;

public:
    /* MCGR: Adaptions for MultiColorGradients

       Direct Start/EndCOlor is no longer required, instead the
       full color gradient is handed over as ColorStops vector.
       To add the former Start/EndColor in a compatible way, just
       prepare an instance of basegfx::ColorStops with the
       StartColor at 0.0 and the EndColor at 1.0.

       A rigid correction/input data will be done by the constructor,
       including to sort the ColorStops by offset and removing invalid
       entries (see sortAndCorrectColorStops)

       To access e.g. the StartColor, use getColorStops().front(), and
       getColorStops().back(), accordingly, for EndColor. The existence
       of at least one entry is guaranteed, so no need to check before
       accessing using of front()/back() calls. If only one color entry
       exists, start == end color is assumed, so not really a gradient
       but (existing) fallbacks to filled polygon can trigger.
    */
    /// constructors/assignmentoperator/destructor
    FillGradientAttribute(GradientStyle eStyle, double fBorder, double fOffsetX, double fOffsetY,
                          double fAngle, const basegfx::ColorStops& rColorStops,
                          sal_uInt16 nSteps = 0);
    FillGradientAttribute();
    FillGradientAttribute(const FillGradientAttribute&);
    FillGradientAttribute(FillGradientAttribute&&);
    FillGradientAttribute& operator=(const FillGradientAttribute&);
    FillGradientAttribute& operator=(FillGradientAttribute&&);
    ~FillGradientAttribute();

    // checks if the incarnation is default constructed
    bool isDefault() const;

    // check if it is defined by a single color, then it is no gradient at all
    bool hasSingleColor() const;

    // compare operator
    bool operator==(const FillGradientAttribute& rCandidate) const;

    // data read access
    GradientStyle getStyle() const;
    double getBorder() const;
    double getOffsetX() const;
    double getOffsetY() const;
    double getAngle() const;
    const basegfx::ColorStops& getColorStops() const;
    sal_uInt16 getSteps() const;
};

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
