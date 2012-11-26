/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

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
            bool                                        mbTwoSidedLighting : 1;

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

        double SdrSceneAttribute::getDistance() const
        {
            return mpSdrSceneAttribute->getDistance();
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

//////////////////////////////////////////////////////////////////////////////
// eof
