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

#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpSdrLineAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // line definitions
            basegfx::B2DLineJoin                    meJoin;             // B2DLINEJOIN_* defines
            double                                  mfWidth;            // 1/100th mm, 0.0==hair
            double                                  mfTransparence;     // [0.0 .. 1.0], 0.0==no transp.
            basegfx::BColor                         maColor;            // color of line
            com::sun::star::drawing::LineCap        meCap;              // BUTT, ROUND, or SQUARE
            ::std::vector< double >                 maDotDashArray;     // array of double which defines the dot-dash pattern
            double                                  mfFullDotDashLen;   // sum of maDotDashArray (for convenience)

            ImpSdrLineAttribute(
                basegfx::B2DLineJoin eJoin,
                double fWidth,
                double fTransparence,
                const basegfx::BColor& rColor,
                com::sun::star::drawing::LineCap eCap,
                const ::std::vector< double >& rDotDashArray,
                double fFullDotDashLen)
            :   mnRefCount(0),
                meJoin(eJoin),
                mfWidth(fWidth),
                mfTransparence(fTransparence),
                maColor(rColor),
                meCap(eCap),
                maDotDashArray(rDotDashArray),
                mfFullDotDashLen(fFullDotDashLen)
            {
            }

            ImpSdrLineAttribute(const basegfx::BColor& rColor)
            :   mnRefCount(0),
                meJoin(basegfx::B2DLINEJOIN_NONE),
                mfWidth(0.0),
                mfTransparence(0.0),
                maColor(rColor),
                meCap(com::sun::star::drawing::LineCap_BUTT),
                maDotDashArray(),
                mfFullDotDashLen(0.0)
            {
            }

            // data read access
            basegfx::B2DLineJoin getJoin() const { return meJoin; }
            double getWidth() const { return mfWidth; }
            double getTransparence() const { return mfTransparence; }
            const basegfx::BColor& getColor() const { return maColor; }
            com::sun::star::drawing::LineCap getCap() const { return meCap; }
            const ::std::vector< double >& getDotDashArray() const { return maDotDashArray; }
            double getFullDotDashLen() const { return mfFullDotDashLen; }

            bool operator==(const ImpSdrLineAttribute& rCandidate) const
            {
                return (getJoin() == rCandidate.getJoin()
                    && getWidth() == rCandidate.getWidth()
                    && getTransparence() == rCandidate.getTransparence()
                    && getColor() == rCandidate.getColor()
                    && getCap() == rCandidate.getCap()
                    && getDotDashArray() == rCandidate.getDotDashArray());
            }

            static ImpSdrLineAttribute* get_global_default()
            {
                static ImpSdrLineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpSdrLineAttribute(
                        basegfx::B2DLINEJOIN_ROUND,
                        0.0,
                        0.0,
                        basegfx::BColor(),
                        com::sun::star::drawing::LineCap_BUTT,
                        std::vector< double >(),
                        0.0);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        SdrLineAttribute::SdrLineAttribute(
            basegfx::B2DLineJoin eJoin,
            double fWidth,
            double fTransparence,
            const basegfx::BColor& rColor,
            com::sun::star::drawing::LineCap eCap,
            const ::std::vector< double >& rDotDashArray,
            double fFullDotDashLen)
        :   mpSdrLineAttribute(
                new ImpSdrLineAttribute(
                    eJoin,
                    fWidth,
                    fTransparence,
                    rColor,
                    eCap,
                    rDotDashArray,
                    fFullDotDashLen))
        {
        }

        SdrLineAttribute::SdrLineAttribute(
            const basegfx::BColor& rColor)
        :   mpSdrLineAttribute(
                new ImpSdrLineAttribute(
                    rColor))
        {
        }

        SdrLineAttribute::SdrLineAttribute()
        :   mpSdrLineAttribute(ImpSdrLineAttribute::get_global_default())
        {
            mpSdrLineAttribute->mnRefCount++;
        }

        SdrLineAttribute::SdrLineAttribute(const SdrLineAttribute& rCandidate)
        :   mpSdrLineAttribute(rCandidate.mpSdrLineAttribute)
        {
            mpSdrLineAttribute->mnRefCount++;
        }

        SdrLineAttribute::~SdrLineAttribute()
        {
            if(mpSdrLineAttribute->mnRefCount)
            {
                mpSdrLineAttribute->mnRefCount--;
            }
            else
            {
                delete mpSdrLineAttribute;
            }
        }

        bool SdrLineAttribute::isDefault() const
        {
            return mpSdrLineAttribute == ImpSdrLineAttribute::get_global_default();
        }

        SdrLineAttribute& SdrLineAttribute::operator=(const SdrLineAttribute& rCandidate)
        {
            if(rCandidate.mpSdrLineAttribute != mpSdrLineAttribute)
            {
                if(mpSdrLineAttribute->mnRefCount)
                {
                    mpSdrLineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpSdrLineAttribute;
                }

                mpSdrLineAttribute = rCandidate.mpSdrLineAttribute;
                mpSdrLineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool SdrLineAttribute::operator==(const SdrLineAttribute& rCandidate) const
        {
            if(rCandidate.mpSdrLineAttribute == mpSdrLineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpSdrLineAttribute == *mpSdrLineAttribute);
        }

        basegfx::B2DLineJoin SdrLineAttribute::getJoin() const
        {
            return mpSdrLineAttribute->getJoin();
        }

        double SdrLineAttribute::getWidth() const
        {
            return mpSdrLineAttribute->getWidth();
        }

        double SdrLineAttribute::getTransparence() const
        {
            return mpSdrLineAttribute->getTransparence();
        }

        const basegfx::BColor& SdrLineAttribute::getColor() const
        {
            return mpSdrLineAttribute->getColor();
        }

        const ::std::vector< double >& SdrLineAttribute::getDotDashArray() const
        {
            return mpSdrLineAttribute->getDotDashArray();
        }

        double SdrLineAttribute::getFullDotDashLen() const
        {
            return mpSdrLineAttribute->getFullDotDashLen();
        }

        bool SdrLineAttribute::isDashed() const
        {
            return (0L != getDotDashArray().size());
        }

        com::sun::star::drawing::LineCap SdrLineAttribute::getCap() const
        {
            return mpSdrLineAttribute->getCap();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
