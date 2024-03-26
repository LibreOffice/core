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

#include <drawinglayer/attribute/sdrlightingattribute3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <drawinglayer/attribute/sdrlightattribute3d.hxx>


namespace drawinglayer::attribute
{
        class ImpSdrLightingAttribute
        {
        public:
            // 3D light attribute definitions
            basegfx::BColor                         maAmbientLight;
            std::vector< Sdr3DLightAttribute >    maLightVector;

            ImpSdrLightingAttribute(
                const basegfx::BColor& rAmbientLight,
                std::vector< Sdr3DLightAttribute >&& rLightVector)
            :   maAmbientLight(rAmbientLight),
                maLightVector(std::move(rLightVector))
            {
            }

            ImpSdrLightingAttribute()
            {
            }

            // data read access
            const basegfx::BColor& getAmbientLight() const { return maAmbientLight; }
            const std::vector< Sdr3DLightAttribute >& getLightVector() const { return maLightVector; }

            bool operator==(const ImpSdrLightingAttribute& rCandidate) const
            {
                return (getAmbientLight() == rCandidate.getAmbientLight()
                    && getLightVector() == rCandidate.getLightVector());
            }
        };

        namespace
        {
            SdrLightingAttribute::ImplType& theGlobalDefault()
            {
                static SdrLightingAttribute::ImplType SINGLETON;
                return SINGLETON;
            }
        }

        SdrLightingAttribute::SdrLightingAttribute(
            const basegfx::BColor& rAmbientLight,
            std::vector< Sdr3DLightAttribute >&& rLightVector)
        :   mpSdrLightingAttribute(ImpSdrLightingAttribute(
                rAmbientLight, std::move(rLightVector)))
        {
        }

        SdrLightingAttribute::SdrLightingAttribute()
        :   mpSdrLightingAttribute(theGlobalDefault())
        {
        }

        SdrLightingAttribute::SdrLightingAttribute(const SdrLightingAttribute&) = default;

        SdrLightingAttribute::SdrLightingAttribute(SdrLightingAttribute&&) = default;

        SdrLightingAttribute::~SdrLightingAttribute() = default;


        bool SdrLightingAttribute::isDefault() const
        {
            return mpSdrLightingAttribute.same_object(theGlobalDefault());
        }

        SdrLightingAttribute& SdrLightingAttribute::operator=(const SdrLightingAttribute&) = default;

        SdrLightingAttribute& SdrLightingAttribute::operator=(SdrLightingAttribute&&) = default;

        bool SdrLightingAttribute::operator==(const SdrLightingAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrLightingAttribute == mpSdrLightingAttribute;
        }

        const std::vector< Sdr3DLightAttribute >& SdrLightingAttribute::getLightVector() const
        {
            return mpSdrLightingAttribute->getLightVector();
        }

        const basegfx::BColor& SdrLightingAttribute::getAmbientLightColor() const
        {
            return mpSdrLightingAttribute->maAmbientLight;
        }

        // color model solver
        basegfx::BColor SdrLightingAttribute::solveColorModel(
            const basegfx::B3DVector& rNormalInEyeCoordinates,
            const basegfx::BColor& rColor, const basegfx::BColor& rSpecular,
            const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity) const
        {
            // initialize with emissive color
            basegfx::BColor aRetval(rEmission);

            // take care of global ambient light
            aRetval += mpSdrLightingAttribute->getAmbientLight() * rColor;

            const std::vector<Sdr3DLightAttribute>& rLightVector = mpSdrLightingAttribute->getLightVector();

            // prepare light access. Is there a light?
            const sal_uInt32 nLightCount(rLightVector.size());

            if(nLightCount && !rNormalInEyeCoordinates.equalZero())
            {
                // prepare normal
                basegfx::B3DVector aEyeNormal(rNormalInEyeCoordinates);
                aEyeNormal.normalize();

                for(sal_uInt32 a(0); a < nLightCount; a++)
                {
                    const Sdr3DLightAttribute& rLight(rLightVector[a]);
                    const double fCosFac(rLight.getDirection().scalar(aEyeNormal));

                    if(fCosFac > 0.0)
                    {
                        aRetval += (rLight.getColor() * rColor) * fCosFac;

                        if(rLight.getSpecular())
                        {
                            // expand by (0.0, 0.0, 1.0) in Z
                            basegfx::B3DVector aSpecularNormal(rLight.getDirection().getX(), rLight.getDirection().getY(), rLight.getDirection().getZ() + 1.0);
                            aSpecularNormal.normalize();
                            double fCosFac2(aSpecularNormal.scalar(aEyeNormal));

                            if(fCosFac2 > 0.0)
                            {
                                fCosFac2 = pow(fCosFac2, static_cast<double>(nSpecularIntensity));
                                aRetval += rSpecular * fCosFac2;
                            }
                        }
                    }
                }
            }

            // clamp to color space before usage
            aRetval.clamp();

            return aRetval;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
