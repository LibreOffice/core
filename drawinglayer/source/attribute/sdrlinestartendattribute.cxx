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

#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrLineStartEndAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // line arrow definitions
            basegfx::B2DPolyPolygon                 maStartPolyPolygon;     // start Line PolyPolygon
            basegfx::B2DPolyPolygon                 maEndPolyPolygon;       // end Line PolyPolygon
            double                                  mfStartWidth;           // 1/100th mm
            double                                  mfEndWidth;             // 1/100th mm

            // bitfield
            unsigned                                mbStartActive : 1L;     // start of Line is active
            unsigned                                mbEndActive : 1L;       // end of Line is active
            unsigned                                mbStartCentered : 1L;   // Line is centered on line start point
            unsigned                                mbEndCentered : 1L;     // Line is centered on line end point

            ImpSdrLineStartEndAttribute(
                const basegfx::B2DPolyPolygon& rStartPolyPolygon,
                const basegfx::B2DPolyPolygon& rEndPolyPolygon,
                double fStartWidth,
                double fEndWidth,
                bool bStartActive,
                bool bEndActive,
                bool bStartCentered,
                bool bEndCentered)
            :   mnRefCount(0),
                maStartPolyPolygon(rStartPolyPolygon),
                maEndPolyPolygon(rEndPolyPolygon),
                mfStartWidth(fStartWidth),
                mfEndWidth(fEndWidth),
                mbStartActive(bStartActive),
                mbEndActive(bEndActive),
                mbStartCentered(bStartCentered),
                mbEndCentered(bEndCentered)
            {
            }

            // data read access
            const basegfx::B2DPolyPolygon& getStartPolyPolygon() const { return maStartPolyPolygon; }
            const basegfx::B2DPolyPolygon& getEndPolyPolygon() const { return maEndPolyPolygon; }
            double getStartWidth() const { return mfStartWidth; }
            double getEndWidth() const { return mfEndWidth; }
            bool isStartActive() const { return mbStartActive; }
            bool isEndActive() const { return mbEndActive; }
            bool isStartCentered() const { return mbStartCentered; }
            bool isEndCentered() const { return mbEndCentered; }

            bool operator==(const ImpSdrLineStartEndAttribute& rCandidate) const
            {
                return (getStartPolyPolygon() == rCandidate.getStartPolyPolygon()
                    && getEndPolyPolygon() == rCandidate.getEndPolyPolygon()
                    && getStartWidth() == rCandidate.getStartWidth()
                    && getEndWidth() == rCandidate.getEndWidth()
                    && isStartActive() == rCandidate.isStartActive()
                    && isEndActive() == rCandidate.isEndActive()
                    && isStartCentered() == rCandidate.isStartCentered()
                    && isEndCentered() == rCandidate.isEndCentered());
            }

            static ImpSdrLineStartEndAttribute* get_global_default()
            {
                static ImpSdrLineStartEndAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrLineStartEndAttribute(
                        basegfx::B2DPolyPolygon(),
                        basegfx::B2DPolyPolygon(),
                        0.0,
                        0.0,
                        false,
                        false,
                        false,
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrLineStartEndAttribute::SdrLineStartEndAttribute(
            const basegfx::B2DPolyPolygon& rStartPolyPolygon,
            const basegfx::B2DPolyPolygon& rEndPolyPolygon,
            double fStartWidth,
            double fEndWidth,
            bool bStartActive,
            bool bEndActive,
            bool bStartCentered,
            bool bEndCentered)
        :   mpSdrLineStartEndAttribute(new ImpSdrLineStartEndAttribute(
                rStartPolyPolygon, rEndPolyPolygon, fStartWidth, fEndWidth, bStartActive, bEndActive, bStartCentered, bEndCentered))
        {
        }

        SdrLineStartEndAttribute::SdrLineStartEndAttribute()
        :   mpSdrLineStartEndAttribute(ImpSdrLineStartEndAttribute::get_global_default())
        {
            mpSdrLineStartEndAttribute->mnRefCount++;
        }

        SdrLineStartEndAttribute::SdrLineStartEndAttribute(const SdrLineStartEndAttribute& rCandidate)
        :   mpSdrLineStartEndAttribute(rCandidate.mpSdrLineStartEndAttribute)
        {
            mpSdrLineStartEndAttribute->mnRefCount++;
        }

        SdrLineStartEndAttribute::~SdrLineStartEndAttribute()
        {
            if(mpSdrLineStartEndAttribute->mnRefCount)
            {
                mpSdrLineStartEndAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrLineStartEndAttribute;
            }
        }

        bool SdrLineStartEndAttribute::isDefault() const
        {
            return mpSdrLineStartEndAttribute == ImpSdrLineStartEndAttribute::get_global_default();
        }

        SdrLineStartEndAttribute& SdrLineStartEndAttribute::operator=(const SdrLineStartEndAttribute& rCandidate)
        {
            if(rCandidate.mpSdrLineStartEndAttribute != mpSdrLineStartEndAttribute)
            {
                if(mpSdrLineStartEndAttribute->mnRefCount)
                {
                    mpSdrLineStartEndAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrLineStartEndAttribute;
                }

                mpSdrLineStartEndAttribute = rCandidate.mpSdrLineStartEndAttribute;
                mpSdrLineStartEndAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrLineStartEndAttribute::operator==(const SdrLineStartEndAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrLineStartEndAttribute == mpSdrLineStartEndAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrLineStartEndAttribute == *mpSdrLineStartEndAttribute);
        }

        const basegfx::B2DPolyPolygon& SdrLineStartEndAttribute::getStartPolyPolygon() const
        {
            return mpSdrLineStartEndAttribute->getStartPolyPolygon();
        }

        const basegfx::B2DPolyPolygon& SdrLineStartEndAttribute::getEndPolyPolygon() const
        {
            return mpSdrLineStartEndAttribute->getEndPolyPolygon();
        }

        double SdrLineStartEndAttribute::getStartWidth() const
        {
            return mpSdrLineStartEndAttribute->getStartWidth();
        }

        double SdrLineStartEndAttribute::getEndWidth() const
        {
            return mpSdrLineStartEndAttribute->getEndWidth();
        }

        bool SdrLineStartEndAttribute::isStartActive() const
        {
            return mpSdrLineStartEndAttribute->isStartActive();
        }

        bool SdrLineStartEndAttribute::isEndActive() const
        {
            return mpSdrLineStartEndAttribute->isEndActive();
        }

        bool SdrLineStartEndAttribute::isStartCentered() const
        {
            return mpSdrLineStartEndAttribute->isStartCentered();
        }

        bool SdrLineStartEndAttribute::isEndCentered() const
        {
            return mpSdrLineStartEndAttribute->isEndCentered();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
