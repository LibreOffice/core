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
#include <basegfx/color/bcolor.hxx>
#include <vector>

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

    /* MCGR: Provide ColorSteps to the FillGradientAttribute

       This is the needed combination of offset and color:

       Offset is defined as:
       - being in the range of [0.0 .. 1.0] (unit range)
       - 0.0 being reserved for StartColor
       - 1.0 being reserved for EndColor
       - in-between offsets thus being in the range of ]0.0 .. 1.0[
       - no two equal offsets are allowed
         - this is an error, but will be ignored (maybe assert?)
       - missing 1.0 entry (EndColor) is allowed
       - at least one value (usually 0.0, StartColor) is required
         - this allows to avoid massive testing in all places where
           this data has to be accessed

       Color is defined as:
       - RGB with unit values [0.0 .. 1.0]

       These definitions are packed in a std::vector<ColorStep> ColorSteps,
       see typedef below. This array is sorted ascending by offsets, from
       lowest to highest. Since all this primitive data definition is
       read-only, this can be guaranteed by forcing/checking this in the
       constructor.
    */
    class ColorStep
    {
    private:
        double mfOffset;
        basegfx::BColor maColor;

    public:
        ColorStep(double fOffset, const basegfx::BColor& rColor)
            : mfOffset(fOffset)
            , maColor(rColor)
        {
        }

        double getOffset() const { return mfOffset; }
        const basegfx::BColor& getColor() const { return maColor; }

        bool operator<(const ColorStep& rCandidate) const
        {
            return getOffset() < rCandidate.getOffset();
        }

        bool operator==(const ColorStep& rCandidate) const
        {
            return getOffset() == rCandidate.getOffset() && getColor() == rCandidate.getColor();
        }
    };

    typedef std::vector<ColorStep> ColorSteps;

private:
    ImplType mpFillGradientAttribute;

public:
    /* MCGR: Adaptions for MultiColorGradients

       To force providing start/end colors these are still part of the
       constructor (see rStartColor/rEndColor). To also provide
       GradientSteps these need to be handed over by ColorSteps data
       if wanted/needed.

       Start/EndColor will be added to the internal ColorSteps with
       the according default offsets. A rigid correction/input data
       testing is done by the construtor, including to sort the
       ColorSteps by offset.

       To access e.g. the StartColor, use getColorSteps().front(), and
       getColorSteps().back(), accordingly, for EndColor. The existance
       of at least one entry is guaranteed, so no need to check before
       accessing using of front()/back() calls. If only one color entry
       exists, start == end color is assumed, so not really a gradient
       but (existing) fallbacks to filled polygon can trigger.
    */
    /// constructors/assignmentoperator/destructor
    FillGradientAttribute(GradientStyle eStyle, double fBorder, double fOffsetX, double fOffsetY,
                          double fAngle, const basegfx::BColor& rStartColor,
                          const basegfx::BColor& rEndColor, const ColorSteps* pColorSteps = nullptr,
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
    const ColorSteps& getColorSteps() const;
    sal_uInt16 getSteps() const;
};

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
