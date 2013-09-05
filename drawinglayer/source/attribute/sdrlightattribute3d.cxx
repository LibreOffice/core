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
#include <rtl/instance.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdr3DLightAttribute
        {
        public:
            // 3D light attribute definitions
            basegfx::BColor                         maColor;
            basegfx::B3DVector                      maDirection;

            // bitfield
            unsigned                                mbSpecular : 1;

            ImpSdr3DLightAttribute(
                const basegfx::BColor& rColor,
                const basegfx::B3DVector& rDirection,
                bool bSpecular)
            :   maColor(rColor),
                maDirection(rDirection),
                mbSpecular(bSpecular)
            {
            }

            ImpSdr3DLightAttribute()
            :   maColor(basegfx::BColor()),
                maDirection(basegfx::B3DVector()),
                mbSpecular(false)
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
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< Sdr3DLightAttribute::ImplType, theGlobalDefault > {};
        }

        Sdr3DLightAttribute::Sdr3DLightAttribute(
            const basegfx::BColor& rColor,
            const basegfx::B3DVector& rDirection,
            bool bSpecular)
        :   mpSdr3DLightAttribute(ImpSdr3DLightAttribute(
                rColor, rDirection, bSpecular))
        {
        }

        Sdr3DLightAttribute::Sdr3DLightAttribute(const Sdr3DLightAttribute& rCandidate)
        :   mpSdr3DLightAttribute(rCandidate.mpSdr3DLightAttribute)
        {
        }

        Sdr3DLightAttribute::~Sdr3DLightAttribute()
        {
        }

        Sdr3DLightAttribute& Sdr3DLightAttribute::operator=(const Sdr3DLightAttribute& rCandidate)
        {
            mpSdr3DLightAttribute = rCandidate.mpSdr3DLightAttribute;
            return *this;
        }

        bool Sdr3DLightAttribute::operator==(const Sdr3DLightAttribute& rCandidate) const
        {
            return rCandidate.mpSdr3DLightAttribute == mpSdr3DLightAttribute;
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
