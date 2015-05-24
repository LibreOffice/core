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


#include <sdr/attribute/sdrformtextoutlineattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <rtl/instance.hxx>


namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFormTextOutlineAttribute
        {
        public:
            // one set of attributes for FormText (FontWork) outline visualisation
            LineAttribute                       maLineAttribute;
            StrokeAttribute                     maStrokeAttribute;
            sal_uInt8                           mnTransparence;

            ImpSdrFormTextOutlineAttribute(
                const LineAttribute& rLineAttribute,
                const StrokeAttribute& rStrokeAttribute,
                sal_uInt8 nTransparence)
            :   maLineAttribute(rLineAttribute),
                maStrokeAttribute(rStrokeAttribute),
                mnTransparence(nTransparence)
            {
            }

            ImpSdrFormTextOutlineAttribute()
            : maLineAttribute(),
              maStrokeAttribute(),
              mnTransparence(0)
            {
            }

            // data read access
            const LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }
            sal_uInt8 getTransparence() const { return mnTransparence; }

            // compare operator
            bool operator==(const ImpSdrFormTextOutlineAttribute& rCandidate) const
            {
                return (getLineAttribute() == rCandidate.getLineAttribute()
                    && getStrokeAttribute() == rCandidate.getStrokeAttribute()
                    && getTransparence() == rCandidate.getTransparence());
            }
        };

        namespace
        {
            struct theGlobalDefault :
                public rtl::Static< SdrFormTextOutlineAttribute::ImplType, theGlobalDefault > {};
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute(
            const LineAttribute& rLineAttribute,
            const StrokeAttribute& rStrokeAttribute,
            sal_uInt8 nTransparence)
        :   mpSdrFormTextOutlineAttribute(
                ImpSdrFormTextOutlineAttribute(
                    rLineAttribute, rStrokeAttribute, nTransparence))
        {
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute()
        :   mpSdrFormTextOutlineAttribute(theGlobalDefault::get())
        {
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute(const SdrFormTextOutlineAttribute& rCandidate)
        :   mpSdrFormTextOutlineAttribute(rCandidate.mpSdrFormTextOutlineAttribute)
        {
        }

        SdrFormTextOutlineAttribute::~SdrFormTextOutlineAttribute()
        {
        }

        bool SdrFormTextOutlineAttribute::isDefault() const
        {
            return mpSdrFormTextOutlineAttribute.same_object(theGlobalDefault::get());
        }

        SdrFormTextOutlineAttribute& SdrFormTextOutlineAttribute::operator=(const SdrFormTextOutlineAttribute& rCandidate)
        {
            mpSdrFormTextOutlineAttribute = rCandidate.mpSdrFormTextOutlineAttribute;
            return *this;
        }

        bool SdrFormTextOutlineAttribute::operator==(const SdrFormTextOutlineAttribute& rCandidate) const
        {
            // tdf#87509 default attr is always != non-default attr, even with same values
            if(rCandidate.isDefault() != isDefault())
                return false;

            return rCandidate.mpSdrFormTextOutlineAttribute == mpSdrFormTextOutlineAttribute;
        }

        const LineAttribute& SdrFormTextOutlineAttribute::getLineAttribute() const
        {
            return mpSdrFormTextOutlineAttribute->getLineAttribute();
        }

        const StrokeAttribute& SdrFormTextOutlineAttribute::getStrokeAttribute() const
        {
            return mpSdrFormTextOutlineAttribute->getStrokeAttribute();
        }

        sal_uInt8 SdrFormTextOutlineAttribute::getTransparence() const
        {
            return mpSdrFormTextOutlineAttribute->getTransparence();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
