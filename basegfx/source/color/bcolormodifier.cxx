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
#include "precompiled_basegfx.hxx"

#include <basegfx/color/bcolormodifier.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    ::basegfx::BColor BColorModifier::getModifiedColor(const ::basegfx::BColor& aSourceColor) const
    {
        switch(meMode)
        {
            case BCOLORMODIFYMODE_INTERPOLATE :
            {
                return interpolate(maBColor, aSourceColor, mfValue);
            }
            case BCOLORMODIFYMODE_GRAY :
            {
                const double fLuminance(aSourceColor.luminance());
                return ::basegfx::BColor(fLuminance, fLuminance, fLuminance);
            }
            case BCOLORMODIFYMODE_BLACKANDWHITE :
            {
                const double fLuminance(aSourceColor.luminance());

                if(fLuminance < mfValue)
                {
                    return ::basegfx::BColor::getEmptyBColor();
                }
                else
                {
                    return ::basegfx::BColor(1.0, 1.0, 1.0);
                }
            }
            case BCOLORMODIFYMODE_INVERT :
            {
                return ::basegfx::BColor(1.0 - aSourceColor.getRed(), 1.0 - aSourceColor.getGreen(), 1.0 - aSourceColor.getBlue());
            }
            case BCOLORMODIFYMODE_LUMINANCE_TO_ALPHA:
            {
                const double fAlpha(1.0 - ((aSourceColor.getRed() * 0.2125) + (aSourceColor.getGreen() * 0.7154) + (aSourceColor.getBlue() * 0.0721)));
                return ::basegfx::BColor(fAlpha, fAlpha, fAlpha);
            }
            default : // BCOLORMODIFYMODE_REPLACE
            {
                return maBColor;
            }
        }
    }
} // end of namespace basegfx

//////////////////////////////////////////////////////////////////////////////
// eof
