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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINEATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINEATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <basegfx/vector/b2enums.hxx>
#include <com/sun/star/drawing/LineCap.hpp>
#include <com/sun/star/drawing/LineDash.hpp>
#include <o3tl/cow_wrapper.hxx>
#include <vector>


// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer::attribute {
    class ImpSdrLineAttribute;
}


namespace drawinglayer::attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLineAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrLineAttribute > ImplType;

        private:
            ImplType mpSdrLineAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLineAttribute(
                basegfx::B2DLineJoin eJoin,
                double fWidth,
                double fTransparence,
                const basegfx::BColor& rColor,
                css::drawing::LineCap eCap,
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen,
                css::drawing::LineDash eDash);
            SdrLineAttribute();
            SdrLineAttribute(const SdrLineAttribute&);
            SdrLineAttribute(SdrLineAttribute&&);
            SdrLineAttribute& operator=(const SdrLineAttribute&);
            SdrLineAttribute& operator=(SdrLineAttribute&&);
            ~SdrLineAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineAttribute& rCandidate) const;

            // data access
            basegfx::B2DLineJoin getJoin() const;
            double getWidth() const;
            double getTransparence() const;
            const basegfx::BColor& getColor() const;
            const ::std::vector< double >& getDotDashArray() const;
            double getFullDotDashLen() const;
            css::drawing::LineCap getCap() const;
            css::drawing::LineDash getDash() const;
        };

} // end of namespace drawinglayer::attribute


#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLINEATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
