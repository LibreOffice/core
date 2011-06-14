/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_svx.hxx"

#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute(
            const SdrLineAttribute& rLine,
            const SdrFillAttribute& rFill,
            const SdrLineStartEndAttribute& rLineStartEnd,
            const SdrShadowAttribute& rShadow,
            const FillGradientAttribute& rFillFloatTransGradient,
            const SdrTextAttribute& rTextAttribute)
        :   SdrLineShadowTextAttribute(rLine, rLineStartEnd, rShadow, rTextAttribute),
            maFill(rFill),
            maFillFloatTransGradient(rFillFloatTransGradient)
        {
        }

        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute()
        :   SdrLineShadowTextAttribute(),
            maFill(),
            maFillFloatTransGradient()
        {
        }

        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute(const SdrLineFillShadowTextAttribute& rCandidate)
        :   SdrLineShadowTextAttribute(rCandidate),
            maFill(rCandidate.getFill()),
            maFillFloatTransGradient(rCandidate.getFillFloatTransGradient())
        {
        }

        SdrLineFillShadowTextAttribute& SdrLineFillShadowTextAttribute::operator=(const SdrLineFillShadowTextAttribute& rCandidate)
        {
            SdrLineShadowTextAttribute::operator=(rCandidate);
            maFill = rCandidate.getFill();
            maFillFloatTransGradient = rCandidate.getFillFloatTransGradient();

            return *this;
        }

        bool SdrLineFillShadowTextAttribute::isDefault() const
        {
            return (SdrLineShadowTextAttribute::isDefault()
                && getFill().isDefault()
                && getFillFloatTransGradient().isDefault());
        }

        bool SdrLineFillShadowTextAttribute::operator==(const SdrLineFillShadowTextAttribute& rCandidate) const
        {
            return(SdrLineShadowTextAttribute::operator==(rCandidate)
                && getFill() == rCandidate.getFill()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
