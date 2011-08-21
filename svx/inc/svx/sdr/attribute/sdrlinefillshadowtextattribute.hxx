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

#ifndef _SDR_ATTRIBUTE_SDRLINEFILLSHADOWTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRLINEFILLSHADOWTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/sdr/attribute/sdrlineshadowtextattribute.hxx>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineFillShadowTextAttribute : public SdrLineShadowTextAttribute
        {
            // add fill and transGradient attributes
            SdrFillAttribute            maFill;                     // fill attributes (if used)
            FillGradientAttribute       maFillFloatTransGradient;   // fill float transparence gradient (if used)

        public:
            SdrLineFillShadowTextAttribute(
                const SdrLineAttribute& rLine,
                const SdrFillAttribute& rFill,
                const SdrLineStartEndAttribute& rLineStartEnd,
                const SdrShadowAttribute& rShadow,
                const FillGradientAttribute& rFillFloatTransGradient,
                const SdrTextAttribute& rTextAttribute);
            SdrLineFillShadowTextAttribute();
            SdrLineFillShadowTextAttribute(const SdrLineFillShadowTextAttribute& rCandidate);
            SdrLineFillShadowTextAttribute& operator=(const SdrLineFillShadowTextAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineFillShadowTextAttribute& rCandidate) const;

            // data access
            const SdrFillAttribute& getFill() const { return maFill; }
            const FillGradientAttribute& getFillFloatTransGradient() const { return maFillFloatTransGradient; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRLINEFILLSHADOWTEXTATTRIBUTE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
