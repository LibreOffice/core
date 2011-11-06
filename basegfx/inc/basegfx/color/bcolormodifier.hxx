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



#ifndef _BGFX_COLOR_BCOLORMODIFIER_HXX
#define _BGFX_COLOR_BCOLORMODIFIER_HXX

#include <basegfx/color/bcolor.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    /** Descriptor for type of color modification
    */
    enum BColorModifyMode
    {
        BCOLORMODIFYMODE_REPLACE,       // replace all color with local color
        BCOLORMODIFYMODE_INTERPOLATE,   // interpolate color between given and local with local value
        BCOLORMODIFYMODE_GRAY,          // convert color to gray
        BCOLORMODIFYMODE_BLACKANDWHITE  // convert color to B&W, local value is treshhold
    };

    /** Class to hold a color, value and mode for a color modification. Color modification is
        done calling the getModifiedColor() method
    */
    class BColorModifier
    {
    protected:
        ::basegfx::BColor                           maBColor;
        double                                      mfValue;
        BColorModifyMode                            meMode;

    public:
        BColorModifier(
            const ::basegfx::BColor& rBColor,
            double fValue = 0.5,
            BColorModifyMode eMode = BCOLORMODIFYMODE_REPLACE)
        :   maBColor(rBColor),
            mfValue(fValue),
            meMode(eMode)
        {}

        // compare operator(s)
        bool operator==(const BColorModifier& rCompare) const
        {
            return (maBColor == rCompare.maBColor && mfValue == rCompare.mfValue && meMode == rCompare.meMode);
        }

        bool operator!=(const BColorModifier& rCompare) const
        {
            return !(operator==(rCompare));
        }

        // data access
        const ::basegfx::BColor& getBColor() const { return maBColor; }
        double getValue() const { return mfValue; }
        BColorModifyMode getMode() const { return meMode; }

        // compute modified color
        ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& aSourceColor) const;
    };

    /** Class to hold a stack of BColorModifiers and to get the modified color with
        applying all existing entry changes
    */
    class BColorModifierStack
    {
    protected:
        ::std::vector< BColorModifier >             maBColorModifiers;

    public:
        sal_uInt32 count() const
        {
            return maBColorModifiers.size();
        }

        const BColorModifier& getBColorModifier(sal_uInt32 nIndex) const
        {
            OSL_ENSURE(nIndex < count(), "BColorModifierStack: Access out of range (!)");
            return maBColorModifiers[nIndex];
        }

        ::basegfx::BColor getModifiedColor(const ::basegfx::BColor& rSource) const
        {
            if(count())
            {
                ::basegfx::BColor aRetval(rSource);
                ::std::vector< BColorModifier >::const_iterator aEnd(maBColorModifiers.end());

                while(aEnd != maBColorModifiers.begin())
                {
                    aRetval = (--aEnd)->getModifiedColor(aRetval);
                }

                return aRetval;
            }
            else
            {
                return rSource;
            }
        }

        void push(const BColorModifier& rNew)
        {
            maBColorModifiers.push_back(rNew);
        }

        void pop()
        {
            maBColorModifiers.pop_back();
        }
    };
} // end of namespace basegfx

#endif // _BGFX_COLOR_BCOLORMODIFIER_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
