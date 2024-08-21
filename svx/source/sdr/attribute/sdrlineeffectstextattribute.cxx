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


#include <sdr/attribute/sdrlineeffectstextattribute.hxx>
#include <utility>


namespace drawinglayer::attribute
{
        SdrLineEffectsTextAttribute::SdrLineEffectsTextAttribute(
            SdrLineAttribute aLine,
            SdrLineStartEndAttribute aLineStartEnd,
            const SdrShadowAttribute& rShadow,
            const SdrTextAttribute& rTextAttribute,
            const SdrGlowAttribute& rGlow,
            const SdrGlowTextAttribute& rGlowText,
            sal_Int32 nSoftEdgeRadius)
        :   SdrEffectsTextAttribute(rShadow, rTextAttribute, rGlow, rGlowText, nSoftEdgeRadius),
            maLine(std::move(aLine)),
            maLineStartEnd(std::move(aLineStartEnd))
        {
        }

        SdrLineEffectsTextAttribute::SdrLineEffectsTextAttribute()
        {
        }

        SdrLineEffectsTextAttribute::SdrLineEffectsTextAttribute(const SdrLineEffectsTextAttribute& rCandidate)
        :   SdrEffectsTextAttribute(rCandidate),
            maLine(rCandidate.getLine()),
            maLineStartEnd(rCandidate.getLineStartEnd())
        {
        }

        SdrLineEffectsTextAttribute& SdrLineEffectsTextAttribute::operator=(const SdrLineEffectsTextAttribute& rCandidate)
        {
            SdrEffectsTextAttribute::operator=(rCandidate);
            maLine = rCandidate.getLine();
            maLineStartEnd = rCandidate.getLineStartEnd();

            return *this;
        }

        bool SdrLineEffectsTextAttribute::isDefault() const
        {
            return(SdrEffectsTextAttribute::isDefault()
                && getLine().isDefault()
                && getLineStartEnd().isDefault());
        }

        bool SdrLineEffectsTextAttribute::operator==(const SdrLineEffectsTextAttribute& rCandidate) const
        {
            return(SdrEffectsTextAttribute::operator==(rCandidate)
                && getLine() == rCandidate.getLine()
                && getLineStartEnd() == rCandidate.getLineStartEnd());
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
