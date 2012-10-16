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

#include <drawinglayer/attribute/sdrfillattribute.hxx>
#include <basegfx/color/bcolor.hxx>
#include <drawinglayer/attribute/sdrfillgraphicattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrFillAttribute
        {
        public:
            // refcounter
            sal_uInt32                          mnRefCount;

            // fill definitions
            double                              mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                     maColor;            // fill color
            FillGradientAttribute               maGradient;         // fill gradient (if used)
            FillHatchAttribute                  maHatch;            // fill hatch (if used)
            SdrFillGraphicAttribute             maFillGraphic;      // fill graphic (if used)

        public:
            ImpSdrFillAttribute(
                double fTransparence,
                const basegfx::BColor& rColor,
                const FillGradientAttribute& rGradient,
                const FillHatchAttribute& rHatch,
                const SdrFillGraphicAttribute& rFillGraphic)
            :   mnRefCount(0),
                mfTransparence(fTransparence),
                maColor(rColor),
                maGradient(rGradient),
                maHatch(rHatch),
                maFillGraphic(rFillGraphic)
            {
            }

            // data read access
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            const FillGradientAttribute& getGradient() const { return maGradient; }
            const FillHatchAttribute& getHatch() const { return maHatch; }
            const SdrFillGraphicAttribute& getFillGraphic() const { return maFillGraphic; }

            // compare operator
            bool operator==(const ImpSdrFillAttribute& rCandidate) const
            {
                return(getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor()
                    && getGradient() == rCandidate.getGradient()
                    && getHatch() == rCandidate.getHatch()
                    && getFillGraphic() == rCandidate.getFillGraphic());
            }

            static ImpSdrFillAttribute* get_global_default()
            {
                static ImpSdrFillAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrFillAttribute(
                        0.0,
                        basegfx::BColor(),
                        FillGradientAttribute(),
                        FillHatchAttribute(),
                        SdrFillGraphicAttribute());

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrFillAttribute::SdrFillAttribute(
            double fTransparence,
            const basegfx::BColor& rColor,
            const FillGradientAttribute& rGradient,
            const FillHatchAttribute& rHatch,
            const SdrFillGraphicAttribute& rFillGraphic)
        :   mpSdrFillAttribute(
                new ImpSdrFillAttribute(
                    fTransparence, rColor, rGradient, rHatch, rFillGraphic))
        {
        }

        SdrFillAttribute::SdrFillAttribute()
        :   mpSdrFillAttribute(ImpSdrFillAttribute::get_global_default())
        {
            mpSdrFillAttribute->mnRefCount++;
        }

        SdrFillAttribute::SdrFillAttribute(const SdrFillAttribute& rCandidate)
        :   mpSdrFillAttribute(rCandidate.mpSdrFillAttribute)
        {
            mpSdrFillAttribute->mnRefCount++;
        }

        SdrFillAttribute::~SdrFillAttribute()
        {
            if(mpSdrFillAttribute->mnRefCount)
            {
                mpSdrFillAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrFillAttribute;
            }
        }

        bool SdrFillAttribute::isDefault() const
        {
            return mpSdrFillAttribute == ImpSdrFillAttribute::get_global_default();
        }

        SdrFillAttribute& SdrFillAttribute::operator=(const SdrFillAttribute& rCandidate)
        {
            if(rCandidate.mpSdrFillAttribute != mpSdrFillAttribute)
            {
                if(mpSdrFillAttribute->mnRefCount)
                {
                    mpSdrFillAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrFillAttribute;
                }

                mpSdrFillAttribute = rCandidate.mpSdrFillAttribute;
                mpSdrFillAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrFillAttribute::operator==(const SdrFillAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrFillAttribute == mpSdrFillAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrFillAttribute == *mpSdrFillAttribute);
        }

        double SdrFillAttribute::getTransparence() const
        {
            return mpSdrFillAttribute->getTransparence();
        }

        const basegfx::BColor& SdrFillAttribute::getColor() const
        {
            return mpSdrFillAttribute->getColor();
        }

        const FillGradientAttribute& SdrFillAttribute::getGradient() const
        {
            return mpSdrFillAttribute->getGradient();
        }

        const FillHatchAttribute& SdrFillAttribute::getHatch() const
        {
            return mpSdrFillAttribute->getHatch();
        }

        const SdrFillGraphicAttribute& SdrFillAttribute::getFillGraphic() const
        {
            return mpSdrFillAttribute->getFillGraphic();
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
