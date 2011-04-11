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

#ifndef _SDR_ATTRIBUTE_SDRLINESHADOWTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRLINESHADOWTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/sdr/attribute/sdrshadowtextattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineShadowTextAttribute : public SdrShadowTextAttribute
        {
            // line, shadow, lineStartEnd and text attributes
            SdrLineAttribute                maLine;                     // line attributes (if used)
            SdrLineStartEndAttribute        maLineStartEnd;             // line start end (if used)

        public:
            SdrLineShadowTextAttribute(
                const SdrLineAttribute& rLine,
                const SdrLineStartEndAttribute& rLineStartEnd,
                const SdrShadowAttribute& rShadow,
                const SdrTextAttribute& rTextAttribute);
            SdrLineShadowTextAttribute();
            SdrLineShadowTextAttribute(const SdrLineShadowTextAttribute& rCandidate);
            SdrLineShadowTextAttribute& operator=(const SdrLineShadowTextAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineShadowTextAttribute& rCandidate) const;

            // data access
            const SdrLineAttribute& getLine() const { return maLine; }
            const SdrLineStartEndAttribute& getLineStartEnd() const { return maLineStartEnd; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRLINESHADOWTEXTATTRIBUTE_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
