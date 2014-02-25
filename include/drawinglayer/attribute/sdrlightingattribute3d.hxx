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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>

#include <vector>


// predefines

namespace basegfx {
    class BColor;
    class B3DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrLightingAttribute;
    class Sdr3DLightAttribute;
}}



namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLightingAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdrLightingAttribute > ImplType;

        private:
            ImplType mpSdrLightingAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLightingAttribute(
                const basegfx::BColor& rAmbientLight,
                const ::std::vector< Sdr3DLightAttribute >& rLightVector);
            SdrLightingAttribute();
            SdrLightingAttribute(const SdrLightingAttribute& rCandidate);
            SdrLightingAttribute& operator=(const SdrLightingAttribute& rCandidate);
            ~SdrLightingAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLightingAttribute& rCandidate) const;

            // data read access
            const ::std::vector< Sdr3DLightAttribute >& getLightVector() const;

            // color model solver
            basegfx::BColor solveColorModel(
                const basegfx::B3DVector& rNormalInEyeCoordinates,
                const basegfx::BColor& rColor, const basegfx::BColor& rSpecular,
                const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
