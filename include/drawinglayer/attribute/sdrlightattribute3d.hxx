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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace basegfx {
    class BColor;
    class B3DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdr3DLightAttribute;
}}



namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC Sdr3DLightAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdr3DLightAttribute > ImplType;

        private:
            ImplType mpSdr3DLightAttribute;

        public:
            // constructors/destructor
            Sdr3DLightAttribute(
                const basegfx::BColor& rColor,
                const basegfx::B3DVector& rDirection,
                bool bSpecular);
            explicit Sdr3DLightAttribute(const basegfx::BColor& rColor);
            Sdr3DLightAttribute(const Sdr3DLightAttribute& rCandidate);
            ~Sdr3DLightAttribute();

            // assignment operator
            Sdr3DLightAttribute& operator=(const Sdr3DLightAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const Sdr3DLightAttribute& rCandidate) const;

            // data read access
            const basegfx::BColor& getColor() const;
            const basegfx::B3DVector& getDirection() const;
            bool getSpecular() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
