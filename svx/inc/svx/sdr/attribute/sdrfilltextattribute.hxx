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

#ifndef _SDR_ATTRIBUTE_SDRFILLTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRFILLTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFillTextAttribute
        {
            // shadow and text attributes
            SdrFillAttribute            maFill;                     // fill attributes (if used)
            FillGradientAttribute       maFillFloatTransGradient;   // fill float transparence gradient (if used)
            SdrTextAttribute            maTextAttribute;            // text and text attributes (if used)

        public:
            SdrFillTextAttribute(
                const SdrFillAttribute& rFill,
                const FillGradientAttribute& rFillFloatTransGradient,
                const SdrTextAttribute& rTextAttribute);
            SdrFillTextAttribute();
            SdrFillTextAttribute(const SdrFillTextAttribute& rCandidate);
            SdrFillTextAttribute& operator=(const SdrFillTextAttribute& rCandidate);

            // compare operator
            bool operator==(const SdrFillTextAttribute& rCandidate) const;

            // data access
            const SdrFillAttribute& getFill() const { return maFill; }
            const FillGradientAttribute& getFillFloatTransGradient() const { return maFillFloatTransGradient; }
            const SdrTextAttribute& getText() const { return maTextAttribute; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRFILLTEXTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
