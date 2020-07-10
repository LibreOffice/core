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

#include <sdr/attribute/sdrlineeffectstextattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>


namespace drawinglayer::attribute
    {
        class SdrLineFillEffectsTextAttribute : public SdrLineEffectsTextAttribute
        {
            // add fill and transGradient attributes
            SdrFillAttribute            maFill;                     // fill attributes (if used)
            FillGradientAttribute       maFillFloatTransGradient;   // fill float transparence gradient (if used)

        public:
            SdrLineFillEffectsTextAttribute(
                const SdrLineAttribute& rLine,
                const SdrFillAttribute& rFill,
                const SdrLineStartEndAttribute& rLineStartEnd,
                const SdrShadowAttribute& rShadow,
                const FillGradientAttribute& rFillFloatTransGradient,
                const SdrTextAttribute& rTextAttribute,
                const SdrGlowAttribute& rGlow,
                sal_Int32 nSoftEdgeRadius);
            SdrLineFillEffectsTextAttribute();
            SdrLineFillEffectsTextAttribute(const SdrLineFillEffectsTextAttribute& rCandidate);
            SdrLineFillEffectsTextAttribute& operator=(const SdrLineFillEffectsTextAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineFillEffectsTextAttribute& rCandidate) const;

            // data access
            const SdrFillAttribute& getFill() const { return maFill; }
            const FillGradientAttribute& getFillFloatTransGradient() const { return maFillFloatTransGradient; }
        };

} // end of namespace drawinglayer::attribute


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
