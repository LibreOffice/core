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

#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>
#include <rtl/instance.hxx>



namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrSceneAttribute
        {
        public:
            // 3D scene attribute definitions
            double                         mfDistance;
            double                         mfShadowSlant;
            css::drawing::ProjectionMode   maProjectionMode;
            css::drawing::ShadeMode        maShadeMode;

            // bitfield
            bool                           mbTwoSidedLighting : 1;

        public:
            ImpSdrSceneAttribute(
                double fDistance,
                double fShadowSlant,
                css::drawing::ProjectionMode aProjectionMode,
                css::drawing::ShadeMode aShadeMode,
                bool bTwoSidedLighting)
            :   mfDistance(fDistance),
                mfShadowSlant(fShadowSlant),
                maProjectionMode(aProjectionMode),
                maShadeMode(aShadeMode),
                mbTwoSidedLighting(bTwoSidedLighting)
            {
            }

            ImpSdrSceneAttribute()
            :   mfDistance(0.0),
                mfShadowSlant(0.0),
                maProjectionMode(css::drawing::ProjectionMode_PARALLEL),
                maShadeMode(css::drawing::ShadeMode_FLAT),
                mbTwoSidedLighting(false)
            {
            }

            // data read access
            double getDistance() const { return mfDistance; }
            double getShadowSlant() const { return mfShadowSlant; }
            css::drawing::ProjectionMode getProjectionMode() const { return maProjectionMode; }
            css::drawing::ShadeMode getShadeMode() const { return maShadeMode; }
            bool getTwoSidedLighting() const { return mbTwoSidedLighting; }

            bool operator==(const ImpSdrSceneAttribute& rCandidate) const
            {
                return (getDistance() == rCandidate.getDistance()
                    && getShadowSlant() == rCandidate.getShadowSlant()
                    && getProjectionMode() == rCandidate.getProjectionMode()
                    && getShadeMode() == rCandidate.getShadeMode()
                    && getTwoSidedLighting() == rCandidate.getTwoSidedLighting());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrSceneAttribute::ImplType, theGlobalDefault > {};
        }

        SdrSceneAttribute::SdrSceneAttribute(
            double fDistance,
            double fShadowSlant,
            css::drawing::ProjectionMode aProjectionMode,
            css::drawing::ShadeMode aShadeMode,
            bool bTwoSidedLighting)
        :   mpSdrSceneAttribute(ImpSdrSceneAttribute(
                fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting))
        {
        }

        SdrSceneAttribute::SdrSceneAttribute()
        :   mpSdrSceneAttribute(theGlobalDefault::get())
        {
        }

        SdrSceneAttribute::SdrSceneAttribute(const SdrSceneAttribute& rCandidate)
        :   mpSdrSceneAttribute(rCandidate.mpSdrSceneAttribute)
        {
        }

        SdrSceneAttribute::~SdrSceneAttribute()
        {
        }

        bool SdrSceneAttribute::isDefault() const
        {
            return mpSdrSceneAttribute.same_object(theGlobalDefault::get());
        }

        SdrSceneAttribute& SdrSceneAttribute::operator=(const SdrSceneAttribute& rCandidate)
        {
            mpSdrSceneAttribute = rCandidate.mpSdrSceneAttribute;
            return *this;
        }

        bool SdrSceneAttribute::operator==(const SdrSceneAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrSceneAttribute == mpSdrSceneAttribute;
        }

        double SdrSceneAttribute::getShadowSlant() const
        {
            return mpSdrSceneAttribute->getShadowSlant();
        }

        css::drawing::ProjectionMode SdrSceneAttribute::getProjectionMode() const
        {
            return mpSdrSceneAttribute->getProjectionMode();
        }

        css::drawing::ShadeMode SdrSceneAttribute::getShadeMode() const
        {
            return mpSdrSceneAttribute->getShadeMode();
        }

        bool SdrSceneAttribute::getTwoSidedLighting() const
        {
            return mpSdrSceneAttribute->getTwoSidedLighting();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
