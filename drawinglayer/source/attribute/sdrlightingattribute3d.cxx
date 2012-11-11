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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrLightingAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // 3D light attribute definitions
            basegfx::BColor                         maAmbientLight;
            ::std::vector< Sdr3DLightAttribute >    maLightVector;

            ImpSdrLightingAttribute(
                const basegfx::BColor& rAmbientLight,
                const ::std::vector< Sdr3DLightAttribute >& rLightVector)
            :   mnRefCount(0),
                maAmbientLight(rAmbientLight),
                maLightVector(rLightVector)
            {
            }

            // data read access
            const basegfx::BColor& getAmbientLight() const { return maAmbientLight; }
            const ::std::vector< Sdr3DLightAttribute >& getLightVector() const { return maLightVector; }

            bool operator==(const ImpSdrLightingAttribute& rCandidate) const
            {
                return (getAmbientLight() == rCandidate.getAmbientLight()
                    && getLightVector() == rCandidate.getLightVector());
            }

            static ImpSdrLightingAttribute* get_global_default()
            {
                static ImpSdrLightingAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrLightingAttribute(
                        basegfx::BColor(),
                        std::vector< Sdr3DLightAttribute >());

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrLightingAttribute::SdrLightingAttribute(
            const basegfx::BColor& rAmbientLight,
            const ::std::vector< Sdr3DLightAttribute >& rLightVector)
        :   mpSdrLightingAttribute(new ImpSdrLightingAttribute(
                rAmbientLight, rLightVector))
        {
        }

        SdrLightingAttribute::SdrLightingAttribute()
        :   mpSdrLightingAttribute(ImpSdrLightingAttribute::get_global_default())
        {
            mpSdrLightingAttribute->mnRefCount++;
        }

        SdrLightingAttribute::SdrLightingAttribute(const SdrLightingAttribute& rCandidate)
        :   mpSdrLightingAttribute(rCandidate.mpSdrLightingAttribute)
        {
            mpSdrLightingAttribute->mnRefCount++;
        }

        SdrLightingAttribute::~SdrLightingAttribute()
        {
            if(mpSdrLightingAttribute->mnRefCount)
            {
                mpSdrLightingAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrLightingAttribute;
            }
        }

        bool SdrLightingAttribute::isDefault() const
        {
            return mpSdrLightingAttribute == ImpSdrLightingAttribute::get_global_default();
        }

        SdrLightingAttribute& SdrLightingAttribute::operator=(const SdrLightingAttribute& rCandidate)
        {
            if(rCandidate.mpSdrLightingAttribute != mpSdrLightingAttribute)
            {
                if(mpSdrLightingAttribute->mnRefCount)
                {
                    mpSdrLightingAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrLightingAttribute;
                }

                mpSdrLightingAttribute = rCandidate.mpSdrLightingAttribute;
                mpSdrLightingAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrLightingAttribute::operator==(const SdrLightingAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrLightingAttribute == mpSdrLightingAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrLightingAttribute == *mpSdrLightingAttribute);
        }

        const ::std::vector< Sdr3DLightAttribute >& SdrLightingAttribute::getLightVector() const
        {
            return mpSdrLightingAttribute->getLightVector();
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

            // prepare light access. Is there a light?
            const sal_uInt32 nLightCount(mpSdrLightingAttribute->getLightVector().size());

            if(nLightCount && !rNormalInEyeCoordinates.equalZero())
            {
                // prepare normal
                basegfx::B3DVector aEyeNormal(rNormalInEyeCoordinates);
                aEyeNormal.normalize();

                for(sal_uInt32 a(0L); a < nLightCount; a++)
                {
                    const Sdr3DLightAttribute& rLight(mpSdrLightingAttribute->getLightVector()[a]);
                    const double fCosFac(rLight.getDirection().scalar(aEyeNormal));

                    if(basegfx::fTools::more(fCosFac, 0.0))
                    {
                        aRetval += ((rLight.getColor() * rColor) * fCosFac);

                        if(rLight.getSpecular())
                        {
                            // expand by (0.0, 0.0, 1.0) in Z
                            basegfx::B3DVector aSpecularNormal(rLight.getDirection().getX(), rLight.getDirection().getY(), rLight.getDirection().getZ() + 1.0);
                            aSpecularNormal.normalize();
                            double fCosFac2(aSpecularNormal.scalar(aEyeNormal));

                            if(basegfx::fTools::more(fCosFac2, 0.0))
                            {
                                fCosFac2 = pow(fCosFac2, (double)nSpecularIntensity);
                                aRetval += (rSpecular * fCosFac2);
                            }
                        }
                    }
                }
            }

            // clamp to color space before usage
            aRetval.clamp();

            return aRetval;
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
