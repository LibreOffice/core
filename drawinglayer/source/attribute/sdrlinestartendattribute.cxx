/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
