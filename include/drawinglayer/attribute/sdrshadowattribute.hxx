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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSHADOWATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSHADOWATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace basegfx {
    class BColor;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrShadowAttribute;
}}


namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrShadowAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrShadowAttribute > ImplType;

        private:
            ImplType mpSdrShadowAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrShadowAttribute(
                const basegfx::B2DVector& rOffset,
                double fTransparence,
                const basegfx::BColor& rColor);
            SdrShadowAttribute();
            SdrShadowAttribute(const SdrShadowAttribute& rCandidate);
            SdrShadowAttribute& operator=(const SdrShadowAttribute& rCandidate);
            ~SdrShadowAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrShadowAttribute& rCandidate) const;

            // data access
            const basegfx::B2DVector& getOffset() const;
            double getTransparence() const;
            const basegfx::BColor& getColor() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSHADOWATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
