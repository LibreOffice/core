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

#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <vcl/graph.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpFillGraphicAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            Graphic                                 maGraphic;
            basegfx::B2DRange                       maGraphicRange;

            // tiling definitions, offsets in X/Y in percent for each 2nd row.
            // If both are set, Y is ignored (X has precedence)
            double                                  mfOffsetX;
            double                                  mfOffsetY;

            // bitfield
            unsigned                                mbTiling : 1;

            ImpFillGraphicAttribute(
                const Graphic& rGraphic,
                const basegfx::B2DRange& rGraphicRange,
                bool bTiling,
                double fOffsetX,
                double fOffsetY)
            :   mnRefCount(0),
                maGraphic(rGraphic),
                maGraphicRange(rGraphicRange),
                mbTiling(bTiling),
                mfOffsetX(fOffsetX),
                mfOffsetY(fOffsetY)
            {
            }

            // data read access
            const Graphic& getGraphic() const { return maGraphic; }
            const basegfx::B2DRange& getGraphicRange() const { return maGraphicRange; }
            bool getTiling() const { return mbTiling; }
            double getOffsetX() const { return mfOffsetX; }
            double getOffsetY() const { return mfOffsetY; }

            bool operator==(const ImpFillGraphicAttribute& rCandidate) const
            {
                return (getGraphic() == rCandidate.getGraphic()
                    && getGraphicRange() == rCandidate.getGraphicRange()
                    && getTiling() == rCandidate.getTiling()
                    && getOffsetX() == rCandidate.getOffsetX()
                    && getOffsetY() == rCandidate.getOffsetY());
            }

            static ImpFillGraphicAttribute* get_global_default()
            {
                static ImpFillGraphicAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpFillGraphicAttribute(
                        Graphic(),
                        basegfx::B2DRange(),
                        false,
                        0.0,
                        0.0);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        FillGraphicAttribute::FillGraphicAttribute(
            const Graphic& rGraphic,
            const basegfx::B2DRange& rGraphicRange,
            bool bTiling,
            double fOffsetX,
            double fOffsetY)
        :   mpFillGraphicAttribute(
                new ImpFillGraphicAttribute(
                    rGraphic,
                    rGraphicRange,
                    bTiling,
                    basegfx::clamp(fOffsetX, 0.0, 1.0),
                    basegfx::clamp(fOffsetY, 0.0, 1.0)))
        {
        }

        FillGraphicAttribute::FillGraphicAttribute()
        :   mpFillGraphicAttribute(ImpFillGraphicAttribute::get_global_default())
        {
            mpFillGraphicAttribute->mnRefCount++;
        }

        FillGraphicAttribute::FillGraphicAttribute(const FillGraphicAttribute& rCandidate)
        :   mpFillGraphicAttribute(rCandidate.mpFillGraphicAttribute)
        {
            mpFillGraphicAttribute->mnRefCount++;
        }

        FillGraphicAttribute::~FillGraphicAttribute()
        {
            if(mpFillGraphicAttribute->mnRefCount)
            {
                mpFillGraphicAttribute->mnRefCount--;
            }
            else
            {
                delete mpFillGraphicAttribute;
            }
        }

        bool FillGraphicAttribute::isDefault() const
        {
            return mpFillGraphicAttribute == ImpFillGraphicAttribute::get_global_default();
        }

        FillGraphicAttribute& FillGraphicAttribute::operator=(const FillGraphicAttribute& rCandidate)
        {
            if(rCandidate.mpFillGraphicAttribute != mpFillGraphicAttribute)
            {
                if(mpFillGraphicAttribute->mnRefCount)
                {
                    mpFillGraphicAttribute->mnRefCount--;
                }
                else
                {
                    delete mpFillGraphicAttribute;
                }

                mpFillGraphicAttribute = rCandidate.mpFillGraphicAttribute;
                mpFillGraphicAttribute->mnRefCount++;
            }

            return *this;
        }

        bool FillGraphicAttribute::operator==(const FillGraphicAttribute& rCandidate) const
        {
            if(rCandidate.mpFillGraphicAttribute == mpFillGraphicAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpFillGraphicAttribute == *mpFillGraphicAttribute);
        }

        const Graphic& FillGraphicAttribute::getGraphic() const
        {
            return mpFillGraphicAttribute->getGraphic();
        }

        const basegfx::B2DRange& FillGraphicAttribute::getGraphicRange() const
        {
            return mpFillGraphicAttribute->getGraphicRange();
        }

        bool FillGraphicAttribute::getTiling() const
        {
            return mpFillGraphicAttribute->getTiling();
        }

        double FillGraphicAttribute::getOffsetX() const
        {
            return mpFillGraphicAttribute->getOffsetX();
        }

        double FillGraphicAttribute::getOffsetY() const
        {
            return mpFillGraphicAttribute->getOffsetY();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
