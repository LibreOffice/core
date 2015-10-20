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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdrSceneAttribute;
}}



namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrSceneAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrSceneAttribute > ImplType;

        private:
            ImplType mpSdrSceneAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrSceneAttribute(
                double fDistance,
                double fShadowSlant,
                css::drawing::ProjectionMode aProjectionMode,
                css::drawing::ShadeMode aShadeMode,
                bool bTwoSidedLighting);
            SdrSceneAttribute();
            SdrSceneAttribute(const SdrSceneAttribute& rCandidate);
            SdrSceneAttribute& operator=(const SdrSceneAttribute& rCandidate);
            ~SdrSceneAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrSceneAttribute& rCandidate) const;

            // data read access
            double getShadowSlant() const;
            css::drawing::ProjectionMode getProjectionMode() const;
            css::drawing::ShadeMode getShadeMode() const;
            bool getTwoSidedLighting() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
