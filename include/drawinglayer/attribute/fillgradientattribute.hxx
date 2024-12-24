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
#include <com/sun/star/awt/GradientStyle.hpp>

namespace basegfx
{
class BColorStops;
}

namespace drawinglayer::attribute
{
class ImpFillGradientAttribute;

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
       prepare an instance of basegfx::BColorStops with the
       StartColor at 0.0 and the EndColor at 1.0.

       A rigid correction/input data will be done by the constructor,
       including to sort the ColorStops by offset and removing invalid
       entries (see sortAndCorrect)

       To access e.g. the StartColor, use getColorStops().front(), and
       getColorStops().back(), accordingly, for EndColor. The existence
       of at least one entry is guaranteed, so no need to check before
       accessing using of front()/back() calls. If only one color entry
       exists, start == end color is assumed, so not really a gradient
       but (existing) fallbacks to filled polygon can trigger.
    */
    /// constructors/assignmentoperator/destructor
    FillGradientAttribute(css::awt::GradientStyle eStyle, double fBorder, double fOffsetX,
                          double fOffsetY, double fAngle, const basegfx::BColorStops& rColorStops,
                          sal_uInt16 nSteps = 0);
    FillGradientAttribute();
    FillGradientAttribute(const FillGradientAttribute&);
    FillGradientAttribute(FillGradientAttribute&&);
    FillGradientAttribute& operator=(const FillGradientAttribute&);
    FillGradientAttribute& operator=(FillGradientAttribute&&);
    ~FillGradientAttribute();

    // checks if the incarnation is default constructed
    bool isDefault() const;

    // MCGR: Check if rendering cannot be handled by old vcl stuff
    // due to various restrictions, based on local parameters. There
    // may be even more reasons on caller's side, e.g. a
    // ViewTransformation that uses shear/rotate/mirror (what VCL
    // cannot do at all)
    bool cannotBeHandledByVCL() const;

    // check if local and given FillGradientAttribute are identical,
    // do take care of exceptions (e.g. no offset for linear/axial, ...)
    bool sameDefinitionThanAlpha(const FillGradientAttribute& rAlpha) const;

    // compare operator
    bool operator==(const FillGradientAttribute& rCandidate) const;

    // data read access
    css::awt::GradientStyle getStyle() const;
    double getBorder() const;
    double getOffsetX() const;
    double getOffsetY() const;
    double getAngle() const;
    const basegfx::BColorStops& getColorStops() const;
    sal_uInt16 getSteps() const;
};

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
