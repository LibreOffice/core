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


#include <sdr/attribute/sdrlinefilleffectstextattribute.hxx>
#include <utility>


namespace drawinglayer::attribute
{
        SdrLineFillEffectsTextAttribute::SdrLineFillEffectsTextAttribute(
            const SdrLineAttribute& rLine,
            SdrFillAttribute aFill,
            const SdrLineStartEndAttribute& rLineStartEnd,
            const SdrShadowAttribute& rShadow,
            FillGradientAttribute aFillFloatTransGradient,
            const SdrTextAttribute& rTextAttribute,
            const SdrGlowAttribute& rGlow,
            const SdrGlowTextAttribute& rGlowText,
            sal_Int32 nSoftEdgeRadius)
        :   SdrLineEffectsTextAttribute(rLine, rLineStartEnd, rShadow, rTextAttribute, rGlow, rGlowText, nSoftEdgeRadius),
            maFill(std::move(aFill)),
            maFillFloatTransGradient(std::move(aFillFloatTransGradient))
        {
        }

        SdrLineFillEffectsTextAttribute::SdrLineFillEffectsTextAttribute()
        {
        }

        SdrLineFillEffectsTextAttribute::SdrLineFillEffectsTextAttribute(const SdrLineFillEffectsTextAttribute& rCandidate)
        :   SdrLineEffectsTextAttribute(rCandidate),
            maFill(rCandidate.getFill()),
            maFillFloatTransGradient(rCandidate.getFillFloatTransGradient())
        {
        }

        SdrLineFillEffectsTextAttribute& SdrLineFillEffectsTextAttribute::operator=(const SdrLineFillEffectsTextAttribute& rCandidate)
        {
            SdrLineEffectsTextAttribute::operator=(rCandidate);
            maFill = rCandidate.getFill();
            maFillFloatTransGradient = rCandidate.getFillFloatTransGradient();

            return *this;
        }

        bool SdrLineFillEffectsTextAttribute::isDefault() const
        {
            return (SdrLineEffectsTextAttribute::isDefault()
                && getFill().isDefault()
                && getFillFloatTransGradient().isDefault());
        }

        bool SdrLineFillEffectsTextAttribute::operator==(const SdrLineFillEffectsTextAttribute& rCandidate) const
        {
            return(SdrLineEffectsTextAttribute::operator==(rCandidate)
                && getFill() == rCandidate.getFill()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient());
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
