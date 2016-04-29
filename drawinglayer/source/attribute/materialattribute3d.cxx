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

#include <drawinglayer/attribute/materialattribute3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>


namespace drawinglayer
{
    namespace attribute
    {
        class ImpMaterialAttribute3D
        {
        public:
            // materialAttribute3D definitions
            basegfx::BColor                         maColor;                // object color
            basegfx::BColor                         maSpecular;             // material specular color
            basegfx::BColor                         maEmission;             // material emissive color
            sal_uInt16                              mnSpecularIntensity;    // material specular intensity [0..128]

            ImpMaterialAttribute3D(const basegfx::BColor& rColor, const basegfx::BColor& rSpecular, const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity)
            :   maColor(rColor),
                maSpecular(rSpecular),
                maEmission(rEmission),
                mnSpecularIntensity(nSpecularIntensity)
            {
            }

            explicit ImpMaterialAttribute3D(const basegfx::BColor& rColor)
            :   maColor(rColor),
                maSpecular(1.0, 1.0, 1.0),
                maEmission(),
                mnSpecularIntensity(15)
            {
            }

            ImpMaterialAttribute3D()
            :   maColor(basegfx::BColor()),
                maSpecular(basegfx::BColor()),
                maEmission(basegfx::BColor()),
                mnSpecularIntensity(0)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            const basegfx::BColor& getSpecular() const { return maSpecular; }
            const basegfx::BColor& getEmission() const { return maEmission; }
            sal_uInt16 getSpecularIntensity() const { return mnSpecularIntensity; }

            bool operator==(const ImpMaterialAttribute3D& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getSpecular() == rCandidate.getSpecular()
                    && getEmission() == rCandidate.getEmission()
                    && getSpecularIntensity() == rCandidate.getSpecularIntensity());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< MaterialAttribute3D::ImplType, theGlobalDefault > {};
        }

        MaterialAttribute3D::MaterialAttribute3D(
            const basegfx::BColor& rColor,
            const basegfx::BColor& rSpecular,
            const basegfx::BColor& rEmission,
            sal_uInt16 nSpecularIntensity)
        :   mpMaterialAttribute3D(ImpMaterialAttribute3D(
                rColor, rSpecular, rEmission, nSpecularIntensity))
        {
        }

        MaterialAttribute3D::MaterialAttribute3D(
            const basegfx::BColor& rColor)
        :   mpMaterialAttribute3D(ImpMaterialAttribute3D(rColor))
        {
        }

        MaterialAttribute3D::MaterialAttribute3D()
        :   mpMaterialAttribute3D(theGlobalDefault::get())
        {
        }

        MaterialAttribute3D::MaterialAttribute3D(const MaterialAttribute3D& rCandidate)
        :   mpMaterialAttribute3D(rCandidate.mpMaterialAttribute3D)
        {
        }

        MaterialAttribute3D::~MaterialAttribute3D()
        {
        }

        bool MaterialAttribute3D::isDefault() const
        {
            return mpMaterialAttribute3D.same_object(theGlobalDefault::get());
        }

        MaterialAttribute3D& MaterialAttribute3D::operator=(const MaterialAttribute3D& rCandidate)
        {
            mpMaterialAttribute3D = rCandidate.mpMaterialAttribute3D;
            return *this;
        }

        bool MaterialAttribute3D::operator==(const MaterialAttribute3D& rCandidate) const
        {
            return rCandidate.mpMaterialAttribute3D == mpMaterialAttribute3D;
        }

        const basegfx::BColor& MaterialAttribute3D::getColor() const
        {
            return mpMaterialAttribute3D->getColor();
        }

        const basegfx::BColor& MaterialAttribute3D::getSpecular() const
        {
            return mpMaterialAttribute3D->getSpecular();
        }

        const basegfx::BColor& MaterialAttribute3D::getEmission() const
        {
            return mpMaterialAttribute3D->getEmission();
        }

        sal_uInt16 MaterialAttribute3D::getSpecularIntensity() const
        {
            return mpMaterialAttribute3D->getSpecularIntensity();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
