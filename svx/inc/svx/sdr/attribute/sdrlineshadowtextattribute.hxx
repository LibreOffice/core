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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
