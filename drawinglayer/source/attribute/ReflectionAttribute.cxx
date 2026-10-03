/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/attribute/ReflectionAttribute.hxx>

#include <algorithm>

namespace drawinglayer::attribute
{
ReflectionAttribute::ReflectionAttribute(sal_Int32 nDistance, sal_Int32 nBlurRadius,
                                         double fStartTransparency, double fStartPosition,
                                         double fEndTransparency, double fEndPosition)
    : mnDistance(nDistance)
    , mnBlurRadius(std::max<sal_Int32>(nBlurRadius, 0))
    , mfStartTransparency(std::clamp(fStartTransparency, 0.0, 1.0))
    , mfStartPosition(std::clamp(fStartPosition, 0.0, 1.0))
    , mfEndTransparency(std::clamp(fEndTransparency, 0.0, 1.0))
    , mfEndPosition(std::clamp(fEndPosition, mfStartPosition, 1.0))
{
}

} // end of namespace drawinglayer::attribute

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
