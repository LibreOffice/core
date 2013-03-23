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

#include <drawinglayer/attribute/sdrshadowattribute.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/color/bcolor.hxx>
#include <rtl/instance.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrShadowAttribute
        {
        public:
            // shadow definitions
            basegfx::B2DVector                  maOffset;                   // shadow offset 1/100th mm
            double                              mfTransparence;             // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;                    // color of shadow

            ImpSdrShadowAttribute(
                const basegfx::B2DVector& rOffset,
                double fTransparence,
                const basegfx::BColor& rColor)
            :   maOffset(rOffset),
                mfTransparence(fTransparence),
                maColor(rColor)
            {
            }

            ImpSdrShadowAttribute()
            :   maOffset(basegfx::B2DVector()),
                mfTransparence(0.0),
                maColor(basegfx::BColor())
            {
            }

            // data read access
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }

            bool operator==(const ImpSdrShadowAttribute& rCandidate) const
            {
                return (getOffset() == rCandidate.getOffset()
                    && getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrShadowAttribute::ImplType, theGlobalDefault > {};
        }


        SdrShadowAttribute::SdrShadowAttribute(
            const basegfx::B2DVector& rOffset,
            double fTransparence,
            const basegfx::BColor& rColor)
        :   mpSdrShadowAttribute(ImpSdrShadowAttribute(
                rOffset, fTransparence, rColor))
        {
        }

        SdrShadowAttribute::SdrShadowAttribute()
        :   mpSdrShadowAttribute(theGlobalDefault::get())
        {
        }

        SdrShadowAttribute::SdrShadowAttribute(const SdrShadowAttribute& rCandidate)
        :   mpSdrShadowAttribute(rCandidate.mpSdrShadowAttribute)
        {
        }

        SdrShadowAttribute::~SdrShadowAttribute()
        {
        }

        bool SdrShadowAttribute::isDefault() const
        {
            return mpSdrShadowAttribute.same_object(theGlobalDefault::get());
        }

        SdrShadowAttribute& SdrShadowAttribute::operator=(const SdrShadowAttribute& rCandidate)
        {
            mpSdrShadowAttribute = rCandidate.mpSdrShadowAttribute;
            return *this;
        }

        bool SdrShadowAttribute::operator==(const SdrShadowAttribute& rCandidate) const
        {
            return mpSdrShadowAttribute == rCandidate.mpSdrShadowAttribute;
        }

        const basegfx::B2DVector& SdrShadowAttribute::getOffset() const
        {
            return mpSdrShadowAttribute->getOffset();
        }

        double SdrShadowAttribute::getTransparence() const
        {
            return mpSdrShadowAttribute->getTransparence();
        }

        const basegfx::BColor& SdrShadowAttribute::getColor() const
        {
            return mpSdrShadowAttribute->getColor();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
