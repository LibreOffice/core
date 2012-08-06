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

#ifndef _SDR_ATTRIBUTE_SDRSHADOWTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRSHADOWTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrShadowTextAttribute
        {
            // shadow and text attributes
            SdrShadowAttribute          maShadow;                   // shadow attributes (if used)
            SdrTextAttribute            maTextAttribute;            // text and text attributes (if used)

        public:
            SdrShadowTextAttribute(
                const SdrShadowAttribute& rShadow,
                const SdrTextAttribute& rTextAttribute);
            SdrShadowTextAttribute();
            SdrShadowTextAttribute(const SdrShadowTextAttribute& rCandidate);
            SdrShadowTextAttribute& operator=(const SdrShadowTextAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrShadowTextAttribute& rCandidate) const;

            // data access
            const SdrShadowAttribute& getShadow() const { return maShadow; }
            const SdrTextAttribute& getText() const { return maTextAttribute; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRSHADOWTEXTATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
