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

#include <drawinglayer/attribute/sdrsceneattribute3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrSceneAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // 3D scene attribute definitions
            double                                      mfDistance;
            double                                      mfShadowSlant;
            ::com::sun::star::drawing::ProjectionMode   maProjectionMode;
            ::com::sun::star::drawing::ShadeMode        maShadeMode;

            // bitfield
            unsigned                                    mbTwoSidedLighting : 1;

        public:
            ImpSdrSceneAttribute(
                double fDistance,
                double fShadowSlant,
                ::com::sun::star::drawing::ProjectionMode aProjectionMode,
                ::com::sun::star::drawing::ShadeMode aShadeMode,
                bool bTwoSidedLighting)
            :   mnRefCount(0),
                mfDistance(fDistance),
                mfShadowSlant(fShadowSlant),
                maProjectionMode(aProjectionMode),
                maShadeMode(aShadeMode),
                mbTwoSidedLighting(bTwoSidedLighting)
            {
            }

            // data read access
            double getDistance() const { return mfDistance; }
            double getShadowSlant() const { return mfShadowSlant; }
            ::com::sun::star::drawing::ProjectionMode getProjectionMode() const { return maProjectionMode; }
            ::com::sun::star::drawing::ShadeMode getShadeMode() const { return maShadeMode; }
            bool getTwoSidedLighting() const { return mbTwoSidedLighting; }

            bool operator==(const ImpSdrSceneAttribute& rCandidate) const
            {
                return (getDistance() == rCandidate.getDistance()
                    && getShadowSlant() == rCandidate.getShadowSlant()
                    && getProjectionMode() == rCandidate.getProjectionMode()
                    && getShadeMode() == rCandidate.getShadeMode()
                    && getTwoSidedLighting() == rCandidate.getTwoSidedLighting());
            }

            static ImpSdrSceneAttribute* get_global_default()
            {
                static ImpSdrSceneAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrSceneAttribute(
                        0.0, 0.0,
                        ::com::sun::star::drawing::ProjectionMode_PARALLEL,
                        ::com::sun::star::drawing::ShadeMode_FLAT,
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrSceneAttribute::SdrSceneAttribute(
            double fDistance,
            double fShadowSlant,
            ::com::sun::star::drawing::ProjectionMode aProjectionMode,
            ::com::sun::star::drawing::ShadeMode aShadeMode,
            bool bTwoSidedLighting)
        :   mpSdrSceneAttribute(new ImpSdrSceneAttribute(
                fDistance, fShadowSlant, aProjectionMode, aShadeMode, bTwoSidedLighting))
        {
        }

        SdrSceneAttribute::SdrSceneAttribute()
        :   mpSdrSceneAttribute(ImpSdrSceneAttribute::get_global_default())
        {
            mpSdrSceneAttribute->mnRefCount++;
        }

        SdrSceneAttribute::SdrSceneAttribute(const SdrSceneAttribute& rCandidate)
        :   mpSdrSceneAttribute(rCandidate.mpSdrSceneAttribute)
        {
            mpSdrSceneAttribute->mnRefCount++;
        }

        SdrSceneAttribute::~SdrSceneAttribute()
        {
            if(mpSdrSceneAttribute->mnRefCount)
            {
                mpSdrSceneAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrSceneAttribute;
            }
        }

        bool SdrSceneAttribute::isDefault() const
        {
            return mpSdrSceneAttribute == ImpSdrSceneAttribute::get_global_default();
        }

        SdrSceneAttribute& SdrSceneAttribute::operator=(const SdrSceneAttribute& rCandidate)
        {
            if(rCandidate.mpSdrSceneAttribute != mpSdrSceneAttribute)
            {
                if(mpSdrSceneAttribute->mnRefCount)
                {
                    mpSdrSceneAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrSceneAttribute;
                }

                mpSdrSceneAttribute = rCandidate.mpSdrSceneAttribute;
                mpSdrSceneAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrSceneAttribute::operator==(const SdrSceneAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrSceneAttribute == mpSdrSceneAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrSceneAttribute == *mpSdrSceneAttribute);
        }

        double SdrSceneAttribute::getShadowSlant() const
        {
            return mpSdrSceneAttribute->getShadowSlant();
        }

        ::com::sun::star::drawing::ProjectionMode SdrSceneAttribute::getProjectionMode() const
        {
            return mpSdrSceneAttribute->getProjectionMode();
        }

        ::com::sun::star::drawing::ShadeMode SdrSceneAttribute::getShadeMode() const
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
