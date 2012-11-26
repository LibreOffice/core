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

#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillHatchAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            HatchStyle                              meStyle;
            double                                  mfDistance;
            double                                  mfAngle;
            basegfx::BColor                         maColor;
            sal_uInt32                              mnMinimalDiscreteDistance;

            // bitfield
            bool                                    mbFillBackground : 1;

            ImpFillHatchAttribute(
                HatchStyle eStyle,
                double fDistance,
                double fAngle,
                const basegfx::BColor& rColor,
                sal_uInt32 nMinimalDiscreteDistance,
                bool bFillBackground)
            :   mnRefCount(0),
                meStyle(eStyle),
                mfDistance(fDistance),
                mfAngle(fAngle),
                maColor(rColor),
                mnMinimalDiscreteDistance(nMinimalDiscreteDistance),
                mbFillBackground(bFillBackground)
            {
            }

            // data read access
            HatchStyle getStyle() const { return meStyle; }
            double getDistance() const { return mfDistance; }
            double getAngle() const { return mfAngle; }
            const basegfx::BColor& getColor() const { return maColor; }
            sal_uInt32 getMinimalDiscreteDistance() const { return mnMinimalDiscreteDistance; }
            bool isFillBackground() const { return mbFillBackground; }

            bool operator==(const ImpFillHatchAttribute& rCandidate) const
            {
                return (getStyle() == rCandidate.getStyle()
                    && getDistance() == rCandidate.getDistance()
                    && getAngle() == rCandidate.getAngle()
                    && getColor() == rCandidate.getColor()
                    && getMinimalDiscreteDistance() == rCandidate.getMinimalDiscreteDistance()
                    && isFillBackground() == rCandidate.isFillBackground());
            }

            static ImpFillHatchAttribute* get_global_default()
            {
                static ImpFillHatchAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillHatchAttribute(
                        HATCHSTYLE_SINGLE,
                        0.0, 0.0,
                        basegfx::BColor(),
                        3, // same as VCL
                        false);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FillHatchAttribute::FillHatchAttribute(
            HatchStyle eStyle,
            double fDistance,
            double fAngle,
            const basegfx::BColor& rColor,
            sal_uInt32 nMinimalDiscreteDistance,
            bool bFillBackground)
        :   mpFillHatchAttribute(
                new ImpFillHatchAttribute(
                    eStyle,
                    fDistance,
                    fAngle,
                    rColor,
                    nMinimalDiscreteDistance,
                    bFillBackground))
        {
        }

        FillHatchAttribute::FillHatchAttribute()
        :   mpFillHatchAttribute(ImpFillHatchAttribute::get_global_default())
        {
            mpFillHatchAttribute->mnRefCount++;
        }

        FillHatchAttribute::FillHatchAttribute(const FillHatchAttribute& rCandidate)
        :   mpFillHatchAttribute(rCandidate.mpFillHatchAttribute)
        {
            mpFillHatchAttribute->mnRefCount++;
        }

        FillHatchAttribute::~FillHatchAttribute()
        {
            if(mpFillHatchAttribute->mnRefCount)
            {
                mpFillHatchAttribute->mnRefCount--;
            }
            else
            {
                delete mpFillHatchAttribute;
            }
        }

        bool FillHatchAttribute::isDefault() const
        {
            return mpFillHatchAttribute == ImpFillHatchAttribute::get_global_default();
        }

        FillHatchAttribute& FillHatchAttribute::operator=(const FillHatchAttribute& rCandidate)
        {
            if(rCandidate.mpFillHatchAttribute != mpFillHatchAttribute)
            {
                if(mpFillHatchAttribute->mnRefCount)
                {
                    mpFillHatchAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFillHatchAttribute;
                }

                mpFillHatchAttribute = rCandidate.mpFillHatchAttribute;
                mpFillHatchAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FillHatchAttribute::operator==(const FillHatchAttribute& rCandidate) const
        {
            if(rCandidate.mpFillHatchAttribute == mpFillHatchAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFillHatchAttribute == *mpFillHatchAttribute);
        }

        // data read access
        HatchStyle FillHatchAttribute::getStyle() const
        {
            return mpFillHatchAttribute->getStyle();
        }

        double FillHatchAttribute::getDistance() const
        {
            return mpFillHatchAttribute->getDistance();
        }

        double FillHatchAttribute::getAngle() const
        {
            return mpFillHatchAttribute->getAngle();
        }

        const basegfx::BColor& FillHatchAttribute::getColor() const
        {
            return mpFillHatchAttribute->getColor();
        }

        sal_uInt32 FillHatchAttribute::getMinimalDiscreteDistance() const
        {
            return mpFillHatchAttribute->getMinimalDiscreteDistance();
        }

        bool FillHatchAttribute::isFillBackground() const
        {
            return mpFillHatchAttribute->isFillBackground();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
