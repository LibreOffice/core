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

#include <drawinglayer/attribute/sdrobjectattribute3d.hxx>
#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <rtl/instance.hxx>


namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdr3DObjectAttribute
        {
        public:
            // 3D object attribute definitions
            css::drawing::NormalsKind              maNormalsKind;              // normals type (0..2)
            css::drawing::TextureProjectionMode    maTextureProjectionX;       // texture projection type X (0..2)
            css::drawing::TextureProjectionMode    maTextureProjectionY;       // texture projection type Y (0..2)
            css::drawing::TextureKind2             maTextureKind;              // texture kind (see uno API)
            css::drawing::TextureMode              maTextureMode;              // texture kind (see uno API)
            MaterialAttribute3D                    maMaterial;                 // object, specular and emissive colors, SpecularIntensity

            // bitfield
            bool                                   mbNormalsInvert : 1;        // invert normals
            bool                                   mbDoubleSided : 1;          // surfaces are double sided
            bool                                   mbShadow3D : 1;             // display shadow in 3D (if on), params for that are at scene
            bool                                   mbTextureFilter : 1;        // filter texture to make more smooth
            bool                                   mbReducedLineGeometry : 1;  // use reduced line geometry (object specific)

            ImpSdr3DObjectAttribute(
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
                bool bReducedLineGeometry)
            :   maNormalsKind(aNormalsKind),
                maTextureProjectionX(aTextureProjectionX),
                maTextureProjectionY(aTextureProjectionY),
                maTextureKind(aTextureKind),
                maTextureMode(aTextureMode),
                maMaterial(rMaterial),
                mbNormalsInvert(bNormalsInvert),
                mbDoubleSided(bDoubleSided),
                mbShadow3D(bShadow3D),
                mbTextureFilter(bTextureFilter),
                mbReducedLineGeometry(bReducedLineGeometry)
            {
            }

            ImpSdr3DObjectAttribute()
            :   maNormalsKind(css::drawing::NormalsKind_SPECIFIC),
                maTextureProjectionX(css::drawing::TextureProjectionMode_OBJECTSPECIFIC),
                maTextureProjectionY(css::drawing::TextureProjectionMode_OBJECTSPECIFIC),
                maTextureKind(css::drawing::TextureKind2_LUMINANCE),
                maTextureMode(css::drawing::TextureMode_REPLACE),
                maMaterial(),
                mbNormalsInvert(false),
                mbDoubleSided(false),
                mbShadow3D(false),
                mbTextureFilter(false),
                mbReducedLineGeometry(false)
            {
            }

            // data read access
            css::drawing::NormalsKind getNormalsKind() const { return maNormalsKind; }
            css::drawing::TextureProjectionMode getTextureProjectionX() const { return maTextureProjectionX; }
            css::drawing::TextureProjectionMode getTextureProjectionY() const { return maTextureProjectionY; }
            css::drawing::TextureKind2 getTextureKind() const { return maTextureKind; }
            css::drawing::TextureMode getTextureMode() const { return maTextureMode; }
            const MaterialAttribute3D& getMaterial() const { return maMaterial; }
            bool getNormalsInvert() const { return mbNormalsInvert; }
            bool getDoubleSided() const { return mbDoubleSided; }
            bool getShadow3D() const { return mbShadow3D; }
            bool getTextureFilter() const { return mbTextureFilter; }
            bool getReducedLineGeometry() const { return mbReducedLineGeometry; }

            bool operator==(const ImpSdr3DObjectAttribute& rCandidate) const
            {
                return (getNormalsKind() == rCandidate.getNormalsKind()
                    && getTextureProjectionX() == rCandidate.getTextureProjectionX()
                    && getTextureProjectionY() == rCandidate.getTextureProjectionY()
                    && getTextureKind() == rCandidate.getTextureKind()
                    && getTextureMode() == rCandidate.getTextureMode()
                    && getMaterial() == rCandidate.getMaterial()
                    && getNormalsInvert() == rCandidate.getNormalsInvert()
                    && getDoubleSided() == rCandidate.getDoubleSided()
                    && getShadow3D() == rCandidate.getShadow3D()
                    && getTextureFilter() == rCandidate.getTextureFilter()
                    && getReducedLineGeometry() == rCandidate.getReducedLineGeometry());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< Sdr3DObjectAttribute::ImplType, theGlobalDefault > {};
        }

        Sdr3DObjectAttribute::Sdr3DObjectAttribute(
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
            bool bReducedLineGeometry)
        :   mpSdr3DObjectAttribute(ImpSdr3DObjectAttribute(
                aNormalsKind, aTextureProjectionX, aTextureProjectionY, aTextureKind, aTextureMode,
                rMaterial, bNormalsInvert, bDoubleSided, bShadow3D, bTextureFilter, bReducedLineGeometry))
        {
        }

        Sdr3DObjectAttribute::Sdr3DObjectAttribute(const Sdr3DObjectAttribute& rCandidate)
        :   mpSdr3DObjectAttribute(rCandidate.mpSdr3DObjectAttribute)
        {
        }

        Sdr3DObjectAttribute::Sdr3DObjectAttribute()
        :   mpSdr3DObjectAttribute(theGlobalDefault::get())
        {
        }

        Sdr3DObjectAttribute::~Sdr3DObjectAttribute()
        {
        }

        bool Sdr3DObjectAttribute::isDefault() const
        {
            return mpSdr3DObjectAttribute.same_object(theGlobalDefault::get());
        }

        Sdr3DObjectAttribute& Sdr3DObjectAttribute::operator=(const Sdr3DObjectAttribute& rCandidate)
        {
            mpSdr3DObjectAttribute = rCandidate.mpSdr3DObjectAttribute;
            return *this;
        }

        bool Sdr3DObjectAttribute::operator==(const Sdr3DObjectAttribute& rCandidate) const
        {
            return rCandidate.mpSdr3DObjectAttribute == mpSdr3DObjectAttribute;
        }

        css::drawing::NormalsKind Sdr3DObjectAttribute::getNormalsKind() const
        {
            return mpSdr3DObjectAttribute->getNormalsKind();
        }

        css::drawing::TextureProjectionMode Sdr3DObjectAttribute::getTextureProjectionX() const
        {
            return mpSdr3DObjectAttribute->getTextureProjectionX();
        }

        css::drawing::TextureProjectionMode Sdr3DObjectAttribute::getTextureProjectionY() const
        {
            return mpSdr3DObjectAttribute->getTextureProjectionY();
        }

        css::drawing::TextureKind2 Sdr3DObjectAttribute::getTextureKind() const
        {
            return mpSdr3DObjectAttribute->getTextureKind();
        }

        css::drawing::TextureMode Sdr3DObjectAttribute::getTextureMode() const
        {
            return mpSdr3DObjectAttribute->getTextureMode();
        }

        const MaterialAttribute3D& Sdr3DObjectAttribute::getMaterial() const
        {
            return mpSdr3DObjectAttribute->getMaterial();
        }

        bool Sdr3DObjectAttribute::getNormalsInvert() const
        {
            return mpSdr3DObjectAttribute->getNormalsInvert();
        }

        bool Sdr3DObjectAttribute::getDoubleSided() const
        {
            return mpSdr3DObjectAttribute->getDoubleSided();
        }

        bool Sdr3DObjectAttribute::getShadow3D() const
        {
            return mpSdr3DObjectAttribute->getShadow3D();
        }

        bool Sdr3DObjectAttribute::getTextureFilter() const
        {
            return mpSdr3DObjectAttribute->getTextureFilter();
        }

        bool Sdr3DObjectAttribute::getReducedLineGeometry() const
        {
            return mpSdr3DObjectAttribute->getReducedLineGeometry();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
