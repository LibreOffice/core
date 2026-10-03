/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <sal/types.h>

namespace drawinglayer::attribute
{
/** The values of a reflection effect. A reflection is a vertically flipped copy of an object,
    placed under the object, and the copy becomes more transparent the further it gets from the
    object.

    The distance and the blur radius are in the units of the object geometry. A distance of 0
    makes the reflection touch the object, and a blur radius of 0 keeps the reflection sharp. The
    transparencies go from 0.0 (opaque) to 1.0 (invisible). The positions are fractions of the
    reflection height from 0.0 to 1.0, measured from the edge next to the object, and the end
    position is never before the start position. Before the start position the reflection has the
    start transparency, after the end position it has the end transparency, and in between the
    transparency changes linearly.

    A default constructed attribute is invisible along its whole height, which is the same as no
    reflection.
*/
class DRAWINGLAYER_DLLPUBLIC ReflectionAttribute
{
private:
    sal_Int32 mnDistance = 0;
    sal_Int32 mnBlurRadius = 0;
    double mfStartTransparency = 1.0;
    double mfStartPosition = 0.0;
    double mfEndTransparency = 1.0;
    double mfEndPosition = 1.0;

public:
    /// The transparencies and the start position are clamped to the range from 0.0 to 1.0, the
    /// end position to the range from the start position to 1.0, and the blur radius to 0 or more.
    ReflectionAttribute(sal_Int32 nDistance, sal_Int32 nBlurRadius, double fStartTransparency,
                        double fStartPosition, double fEndTransparency, double fEndPosition);
    ReflectionAttribute() = default;

    bool operator==(const ReflectionAttribute&) const = default;

    sal_Int32 getDistance() const { return mnDistance; }
    sal_Int32 getBlurRadius() const { return mnBlurRadius; }
    double getStartTransparency() const { return mfStartTransparency; }
    double getStartPosition() const { return mfStartPosition; }
    double getEndTransparency() const { return mfEndTransparency; }
    double getEndPosition() const { return mfEndPosition; }
    bool isDefault() const { return mfStartTransparency >= 1.0 && mfEndTransparency >= 1.0; }
};

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
