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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLHATCHATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLHATCHATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace attribute {
    class ImpFillHatchAttribute;
}}


namespace drawinglayer
{
    namespace attribute
    {
        enum HatchStyle
        {
            HATCHSTYLE_SINGLE,
            HATCHSTYLE_DOUBLE,
            HATCHSTYLE_TRIPLE
        };
    } // end of namespace attribute
} // end of namespace drawinglayer


namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC FillHatchAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpFillHatchAttribute > ImplType;

        private:
            ImplType mpFillHatchAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            FillHatchAttribute(
                HatchStyle eStyle,
                double fDistance,
                double fAngle,
                const basegfx::BColor& rColor,
                sal_uInt32 nMinimalDiscreteDistance,
                bool bFillBackground);
            FillHatchAttribute();
            FillHatchAttribute(const FillHatchAttribute& rCandidate);
            FillHatchAttribute& operator=(const FillHatchAttribute& rCandidate);
            ~FillHatchAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const FillHatchAttribute& rCandidate) const;

            // data read access
            HatchStyle getStyle() const;
            double getDistance() const;
            double getAngle() const;
            const basegfx::BColor& getColor() const;

            // #i120230# If a minimal discrete distance is wanted (VCL used 3,
            // this is the default for the global instance, too), set this
            // unequal to zero. Zero means not to use it. If set bigger zero
            // (should be at least two, one leads to a full plane filled with
            // lines when Distance in discrete views is smaller than one) this
            // will be used when the discrete value is less than the given one.
            // This is used to 'emulate' old VCL behaviour which makes hatches
            // look better by not making distances as small as needed, but
            // keeping them on a minimal discrete value for more appealing
            // visualisation.
            sal_uInt32 getMinimalDiscreteDistance() const;

            bool isFillBackground() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLHATCHATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
