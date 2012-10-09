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

#include <drawinglayer/attribute/sdrlightattribute3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/vector/b3dvector.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdr3DLightAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // 3D light attribute definitions
            basegfx::BColor                         maColor;
            basegfx::B3DVector                      maDirection;

            // bitfield
            unsigned                                mbSpecular : 1;

            ImpSdr3DLightAttribute(
                const basegfx::BColor& rColor,
                const basegfx::B3DVector& rDirection,
                bool bSpecular)
            :   mnRefCount(0),
                maColor(rColor),
                maDirection(rDirection),
                mbSpecular(bSpecular)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            const basegfx::B3DVector& getDirection() const { return maDirection; }
            bool getSpecular() const { return mbSpecular; }

            bool operator==(const ImpSdr3DLightAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getDirection() == rCandidate.getDirection()
                    && getSpecular() == rCandidate.getSpecular());
            }

            static ImpSdr3DLightAttribute* get_global_default()
            {
                static ImpSdr3DLightAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdr3DLightAttribute(
                        basegfx::BColor(),
                        basegfx::B3DVector(),
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        Sdr3DLightAttribute::Sdr3DLightAttribute(
            const basegfx::BColor& rColor,
            const basegfx::B3DVector& rDirection,
            bool bSpecular)
        :   mpSdr3DLightAttribute(new ImpSdr3DLightAttribute(
                rColor, rDirection, bSpecular))
        {
        }

        Sdr3DLightAttribute::Sdr3DLightAttribute(const Sdr3DLightAttribute& rCandidate)
        :   mpSdr3DLightAttribute(rCandidate.mpSdr3DLightAttribute)
        {
            mpSdr3DLightAttribute->mnRefCount++;
        }

        Sdr3DLightAttribute::~Sdr3DLightAttribute()
        {
            if(mpSdr3DLightAttribute->mnRefCount)
            {
                mpSdr3DLightAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdr3DLightAttribute;
            }
        }

        bool Sdr3DLightAttribute::isDefault() const
        {
            return mpSdr3DLightAttribute == ImpSdr3DLightAttribute::get_global_default();
        }

        Sdr3DLightAttribute& Sdr3DLightAttribute::operator=(const Sdr3DLightAttribute& rCandidate)
        {
            if(rCandidate.mpSdr3DLightAttribute != mpSdr3DLightAttribute)
            {
                if(mpSdr3DLightAttribute->mnRefCount)
                {
                    mpSdr3DLightAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdr3DLightAttribute;
                }

                mpSdr3DLightAttribute = rCandidate.mpSdr3DLightAttribute;
                mpSdr3DLightAttribute->mnRefCount++;
            }

            return *this;
        }

        bool Sdr3DLightAttribute::operator==(const Sdr3DLightAttribute& rCandidate) const
        {
            if(rCandidate.mpSdr3DLightAttribute == mpSdr3DLightAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdr3DLightAttribute == *mpSdr3DLightAttribute);
        }

        const basegfx::BColor& Sdr3DLightAttribute::getColor() const
        {
            return mpSdr3DLightAttribute->getColor();
        }

        const basegfx::B3DVector& Sdr3DLightAttribute::getDirection() const
        {
            return mpSdr3DLightAttribute->getDirection();
        }

        bool Sdr3DLightAttribute::getSpecular() const
        {
            return mpSdr3DLightAttribute->getSpecular();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
