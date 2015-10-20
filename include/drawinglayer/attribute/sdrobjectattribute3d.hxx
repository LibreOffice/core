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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <com/sun/star/drawing/NormalsKind.hpp>
#include <com/sun/star/drawing/TextureProjectionMode.hpp>
#include <com/sun/star/drawing/TextureKind2.hpp>
#include <com/sun/star/drawing/TextureMode.hpp>
#include <o3tl/cow_wrapper.hxx>


// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdr3DObjectAttribute;
    class MaterialAttribute3D;
}}



namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC Sdr3DObjectAttribute
        {
        public:
            typedef o3tl::cow_wrapper< ImpSdr3DObjectAttribute > ImplType;

        private:
            ImplType mpSdr3DObjectAttribute;

        public:
            // constructors/destructor
            Sdr3DObjectAttribute(
                css::drawing::NormalsKind  aNormalsKind,
                css::drawing::TextureProjectionMode aTextureProjectionX,
                css::drawing::TextureProjectionMode aTextureProjectionY,
                css::drawing::TextureKind2 aTextureKind,
                css::drawing::TextureMode aTextureMode,
                const MaterialAttribute3D& rMaterial,
                bool bNormalsInvert,
                bool bDoubleSided,
                bool bShadow3D,
                bool bTextureFilter,
                bool bReducedLineGeometry);
            Sdr3DObjectAttribute(const Sdr3DObjectAttribute& rCandidate);
            ~Sdr3DObjectAttribute();

            // assignment operator
            Sdr3DObjectAttribute& operator=(const Sdr3DObjectAttribute& rCandidate);

            // compare operator
            bool operator==(const Sdr3DObjectAttribute& rCandidate) const;

            // data read access
            css::drawing::NormalsKind getNormalsKind() const;
            css::drawing::TextureProjectionMode getTextureProjectionX() const;
            css::drawing::TextureProjectionMode getTextureProjectionY() const;
            css::drawing::TextureKind2 getTextureKind() const;
            css::drawing::TextureMode getTextureMode() const;
            const MaterialAttribute3D& getMaterial() const;
            bool getNormalsInvert() const;
            bool getDoubleSided() const;
            bool getShadow3D() const;
            bool getTextureFilter() const;
            bool getReducedLineGeometry() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDROBJECTATTRIBUTE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
