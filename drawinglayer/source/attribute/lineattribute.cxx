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

#include <drawinglayer/attribute/lineattribute.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class ImpLineAttribute
        {
        public:
            // refcounter
            sal_uInt32                              mnRefCount;

            // data definitions
            basegfx::BColor                         maColor;                // color
            double                                  mfWidth;                // absolute line width
            basegfx::B2DLineJoin                    meLineJoin;             // type of LineJoin

            ImpLineAttribute(
                const basegfx::BColor& rColor,
                double fWidth,
                basegfx::B2DLineJoin aB2DLineJoin)
            :   mnRefCount(0),
                maColor(rColor),
                mfWidth(fWidth),
                meLineJoin(aB2DLineJoin)
            {
            }

            // data read access
            const basegfx::BColor& getColor() const { return maColor; }
            double getWidth() const { return mfWidth; }
            basegfx::B2DLineJoin getLineJoin() const { return meLineJoin; }

            bool operator==(const ImpLineAttribute& rCandidate) const
            {
                return (getColor() == rCandidate.getColor()
                    && getWidth() == rCandidate.getWidth()
                    && getLineJoin() == rCandidate.getLineJoin());
            }

            static ImpLineAttribute* get_global_default()
            {
                static ImpLineAttribute* pDefault = 0;

                if(!pDefault)
                {
                    pDefault = new ImpLineAttribute(
                        basegfx::BColor(),
                        0.0,
                        basegfx::B2DLINEJOIN_ROUND);

                    // never delete; start with RefCount 1, not 0
                    pDefault->mnRefCount++;
                }

                return pDefault;
            }
        };

        LineAttribute::LineAttribute(
            const basegfx::BColor& rColor,
            double fWidth,
            basegfx::B2DLineJoin aB2DLineJoin)
        :   mpLineAttribute(new ImpLineAttribute(
                rColor, fWidth, aB2DLineJoin))
        {
        }

        LineAttribute::LineAttribute()
        :   mpLineAttribute(ImpLineAttribute::get_global_default())
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::LineAttribute(const LineAttribute& rCandidate)
        :   mpLineAttribute(rCandidate.mpLineAttribute)
        {
            mpLineAttribute->mnRefCount++;
        }

        LineAttribute::~LineAttribute()
        {
            if(mpLineAttribute->mnRefCount)
            {
                mpLineAttribute->mnRefCount--;
            }
            else
            {
                delete mpLineAttribute;
            }
        }

        bool LineAttribute::isDefault() const
        {
            return mpLineAttribute == ImpLineAttribute::get_global_default();
        }

        LineAttribute& LineAttribute::operator=(const LineAttribute& rCandidate)
        {
            if(rCandidate.mpLineAttribute != mpLineAttribute)
            {
                if(mpLineAttribute->mnRefCount)
                {
                    mpLineAttribute->mnRefCount--;
                }
                else
                {
                    delete mpLineAttribute;
                }

                mpLineAttribute = rCandidate.mpLineAttribute;
                mpLineAttribute->mnRefCount++;
            }

            return *this;
        }

        bool LineAttribute::operator==(const LineAttribute& rCandidate) const
        {
            if(rCandidate.mpLineAttribute == mpLineAttribute)
            {
                return true;
            }

            if(rCandidate.isDefault() != isDefault())
            {
                return false;
            }

            return (*rCandidate.mpLineAttribute == *mpLineAttribute);
        }

        const basegfx::BColor& LineAttribute::getColor() const
        {
            return mpLineAttribute->getColor();
        }

        double LineAttribute::getWidth() const
        {
            return mpLineAttribute->getWidth();
        }

        basegfx::B2DLineJoin LineAttribute::getLineJoin() const
        {
            return mpLineAttribute->getLineJoin();
        }

    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
