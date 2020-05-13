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


#include <sdr/attribute/sdreffectstextattribute.hxx>


namespace drawinglayer::attribute
{
        SdrEffectsTextAttribute::SdrEffectsTextAttribute(
            const SdrShadowAttribute& rShadow,
            const SdrTextAttribute& rTextAttribute,
            const SdrGlowAttribute& rGlow,
            sal_Int32 nSoftEdgeRadius)
        :   maShadow(rShadow),
            maTextAttribute(rTextAttribute),
            maGlow(rGlow),
            mnSoftEdgeRadius(nSoftEdgeRadius)
        {
        }

        SdrEffectsTextAttribute::SdrEffectsTextAttribute()
        :   maShadow(),
            maTextAttribute()
        {
        }

        SdrEffectsTextAttribute::SdrEffectsTextAttribute(const SdrEffectsTextAttribute& rCandidate)
        :   maShadow(rCandidate.getShadow()),
            maTextAttribute(rCandidate.getText()),
            maGlow(rCandidate.maGlow),
            mnSoftEdgeRadius(rCandidate.mnSoftEdgeRadius)
        {
        }

        SdrEffectsTextAttribute& SdrEffectsTextAttribute::operator=(const SdrEffectsTextAttribute& rCandidate)
        {
            maShadow = rCandidate.getShadow();
            maTextAttribute = rCandidate.getText();
            maGlow = rCandidate.maGlow;
            mnSoftEdgeRadius = rCandidate.mnSoftEdgeRadius;

            return *this;
        }

        bool SdrEffectsTextAttribute::isDefault() const
        {
            return (getShadow().isDefault()
                && getText().isDefault() && maGlow.isDefault() && getSoftEdgeRadius() == 0);
        }

        bool SdrEffectsTextAttribute::operator==(const SdrEffectsTextAttribute& rCandidate) const
        {
            return (getShadow() == rCandidate.getShadow()
                && getText() == rCandidate.getText()
                && getGlow() == rCandidate.getGlow()
                && getSoftEdgeRadius() == rCandidate.getSoftEdgeRadius());
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
