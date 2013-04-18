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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <o3tl/cow_wrapper.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class ImpMaterialAttribute3D;
}}

namespace basegfx {
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC MaterialAttribute3D
        {
        public:
            typedef o3tl::cow_wrapper< ImpMaterialAttribute3D > ImplType;

        private:
            ImplType mpMaterialAttribute3D;

        public:
            // constructors/destructor
            MaterialAttribute3D(
                const basegfx::BColor& rColor,
                const basegfx::BColor& rSpecular,
                const basegfx::BColor& rEmission,
                sal_uInt16 nSpecularIntensity);
            MaterialAttribute3D(const basegfx::BColor& rColor);
            MaterialAttribute3D();
            MaterialAttribute3D(const MaterialAttribute3D& rCandidate);
            ~MaterialAttribute3D();

            // assignment operator
            MaterialAttribute3D& operator=(const MaterialAttribute3D& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const MaterialAttribute3D& rCandidate) const;

            // data read access
            const basegfx::BColor& getColor() const;
            const basegfx::BColor& getSpecular() const;
            const basegfx::BColor& getEmission() const;
            sal_uInt16 getSpecularIntensity() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
