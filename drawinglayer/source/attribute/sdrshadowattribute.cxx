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


namespace drawinglayer::attribute
{
        class ImpSdrShadowAttribute
        {
        public:
            // shadow definitions
            basegfx::B2DVector                  maOffset;                   // shadow offset 1/100th mm
            basegfx::B2DVector                  maSize;                     // [0.0 .. 2.0]
            double                              mfTransparence;             // [0.0 .. 1.0], 0.0==no transp.
            sal_Int32                           mnBlur;                     // [0   .. 180], radius of the blur
            basegfx::BColor                     maColor;                    // color of shadow

            ImpSdrShadowAttribute(
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rSize,
                double fTransparence,
                sal_Int32 nBlur,
                const basegfx::BColor& rColor)
            :   maOffset(rOffset),
                maSize(rSize),
                mfTransparence(fTransparence),
                mnBlur(nBlur),
                maColor(rColor)
            {
            }

            ImpSdrShadowAttribute()
            :   maOffset(basegfx::B2DVector()),
                maSize(basegfx::B2DVector()),
                mfTransparence(0.0),
                mnBlur(0),
                maColor(basegfx::BColor())
            {
            }

            // data read access
            const basegfx::B2DVector& getOffset() const { return maOffset; }
            const basegfx::B2DVector& getSize() const { return maSize; }
            double getTransparence() const { return mfTransparence; }
            sal_Int32 getBlur() const { return mnBlur; }
            const basegfx::BColor& getColor() const { return maColor; }

            bool operator==(const ImpSdrShadowAttribute& rCandidate) const
            {
                return (getOffset() == rCandidate.getOffset()
                    && getSize() == rCandidate.getSize()
                    && getTransparence() == rCandidate.getTransparence()
                        && getBlur() == rCandidate.getBlur()
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
            const basegfx::B2DVector& rSize,
            double fTransparence,
            sal_Int32 nBlur,
            const basegfx::BColor& rColor)
        :   mpSdrShadowAttribute(ImpSdrShadowAttribute(
                rOffset, rSize, fTransparence,nBlur, rColor))
        {
        }

        SdrShadowAttribute::SdrShadowAttribute()
        :   mpSdrShadowAttribute(theGlobalDefault::get())
        {
        }

        SdrShadowAttribute::SdrShadowAttribute(const SdrShadowAttribute&) = default;

        SdrShadowAttribute::SdrShadowAttribute(SdrShadowAttribute&&) = default;

        SdrShadowAttribute::~SdrShadowAttribute() = default;

        bool SdrShadowAttribute::isDefault() const
        {
            return mpSdrShadowAttribute.same_object(theGlobalDefault::get());
        }

        SdrShadowAttribute& SdrShadowAttribute::operator=(const SdrShadowAttribute&) = default;

        SdrShadowAttribute& SdrShadowAttribute::operator=(SdrShadowAttribute&&) = default;

        bool SdrShadowAttribute::operator==(const SdrShadowAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return mpSdrShadowAttribute == rCandidate.mpSdrShadowAttribute;
        }

        const basegfx::B2DVector& SdrShadowAttribute::getOffset() const
        {
            return mpSdrShadowAttribute->getOffset();
        }

        const basegfx::B2DVector& SdrShadowAttribute::getSize() const
        {
            return mpSdrShadowAttribute->getSize();
        }

        double SdrShadowAttribute::getTransparence() const
        {
            return mpSdrShadowAttribute->getTransparence();
        }

        sal_Int32 SdrShadowAttribute::getBlur() const
        {
            return mpSdrShadowAttribute->getBlur();
        }

        const basegfx::BColor& SdrShadowAttribute::getColor() const
        {
            return mpSdrShadowAttribute->getColor();
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
