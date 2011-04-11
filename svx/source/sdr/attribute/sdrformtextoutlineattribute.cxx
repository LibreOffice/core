/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_svx.hxx"

#include <svx/sdr/attribute/sdrformtextoutlineattribute.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFormTextOutlineAttribute
        {
        public:
            // refcounter
            sal_uInt32                          mnRefCount;

            // one set of attributes for FormText (FontWork) outline visualisation
            LineAttribute                       maLineAttribute;
            StrokeAttribute                     maStrokeAttribute;
            sal_uInt8                           mnTransparence;

            ImpSdrFormTextOutlineAttribute(
                const LineAttribute& rLineAttribute,
                const StrokeAttribute& rStrokeAttribute,
                sal_uInt8 nTransparence)
            :   mnRefCount(0),
                maLineAttribute(rLineAttribute),
                maStrokeAttribute(rStrokeAttribute),
                mnTransparence(nTransparence)
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

            static ImpSdrFormTextOutlineAttribute* get_global_default()
            {
                static ImpSdrFormTextOutlineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFormTextOutlineAttribute(
                        LineAttribute(),
                        StrokeAttribute(),
                        0);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute(
            const LineAttribute& rLineAttribute,
            const StrokeAttribute& rStrokeAttribute,
            sal_uInt8 nTransparence)
        :   mpSdrFormTextOutlineAttribute(new ImpSdrFormTextOutlineAttribute(
                rLineAttribute, rStrokeAttribute, nTransparence))
        {
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute()
        :   mpSdrFormTextOutlineAttribute(ImpSdrFormTextOutlineAttribute::get_global_default())
        {
            mpSdrFormTextOutlineAttribute->mnRefCount++;
        }

        SdrFormTextOutlineAttribute::SdrFormTextOutlineAttribute(const SdrFormTextOutlineAttribute& rCandidate)
        :   mpSdrFormTextOutlineAttribute(rCandidate.mpSdrFormTextOutlineAttribute)
        {
            mpSdrFormTextOutlineAttribute->mnRefCount++;
        }

        SdrFormTextOutlineAttribute::~SdrFormTextOutlineAttribute()
        {
            if(mpSdrFormTextOutlineAttribute->mnRefCount)
            {
                mpSdrFormTextOutlineAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFormTextOutlineAttribute;
            }
        }

        bool SdrFormTextOutlineAttribute::isDefault() const
        {
            return mpSdrFormTextOutlineAttribute == ImpSdrFormTextOutlineAttribute::get_global_default();
        }

        SdrFormTextOutlineAttribute& SdrFormTextOutlineAttribute::operator=(const SdrFormTextOutlineAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFormTextOutlineAttribute != mpSdrFormTextOutlineAttribute)
            {
                if(mpSdrFormTextOutlineAttribute->mnRefCount)
                {
                    mpSdrFormTextOutlineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFormTextOutlineAttribute;
                }

                mpSdrFormTextOutlineAttribute = rCandidate.mpSdrFormTextOutlineAttribute;
                mpSdrFormTextOutlineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFormTextOutlineAttribute::operator==(const SdrFormTextOutlineAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFormTextOutlineAttribute == mpSdrFormTextOutlineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFormTextOutlineAttribute == *mpSdrFormTextOutlineAttribute);
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
