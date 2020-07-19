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

#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <sdr/attribute/sdrtextattribute.hxx>
#include <drawinglayer/attribute/sdrglowattribute.hxx>


namespace drawinglayer::attribute
    {
        class SdrEffectsTextAttribute
        {
            // shadow and text attributes
            SdrShadowAttribute          maShadow;                   // shadow attributes (if used)
            SdrTextAttribute            maTextAttribute;            // text and text attributes (if used)

            // glow effect
            SdrGlowAttribute            maGlow;

            // soft edge
            sal_Int32 mnSoftEdgeRadius = 0;

        public:
            SdrEffectsTextAttribute(
                const SdrShadowAttribute& rShadow,
                const SdrTextAttribute& rTextAttribute,
                const SdrGlowAttribute& rGlow,
                sal_Int32 nSoftEdgeRadius);
            SdrEffectsTextAttribute();
            SdrEffectsTextAttribute(const SdrEffectsTextAttribute& rCandidate);
            SdrEffectsTextAttribute& operator=(const SdrEffectsTextAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrEffectsTextAttribute& rCandidate) const;

            // data access
            const SdrShadowAttribute& getShadow() const { return maShadow; }
            const SdrTextAttribute& getText() const { return maTextAttribute; }
            const SdrGlowAttribute& getGlow() const { return maGlow; }
            sal_Int32 getSoftEdgeRadius() const { return mnSoftEdgeRadius; }
        };

} // end of namespace drawinglayer::attribute


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
