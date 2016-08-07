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



#ifndef _BGFX_COLOR_BCOLORTOOLS_HXX
#define _BGFX_COLOR_BCOLORTOOLS_HXX

#include <sal/types.h>
#include <basegfx/basegfxdllapi.h>

//////////////////////////////////////////////////////////////////////////////

namespace basegfx
{
    class BColor;

    namespace tools
    {
        /// Transform from RGB to HSL
        BASEGFX_DLLPUBLIC BColor rgb2hsl(const BColor& rRGBColor);
        /// Transform from HSL to RGB
        BASEGFX_DLLPUBLIC BColor hsl2rgb(const BColor& rHSLColor);

        /// Transform from RGB to HSV
        BASEGFX_DLLPUBLIC BColor rgb2hsv(const BColor& rRGBColor);
        /// Transform from HSV to RGB
        BASEGFX_DLLPUBLIC BColor hsv2rgb(const BColor& rHSVColor);

        /// Transform from R'G'B' to YIQ (NTSC color model - Y is used in monochrome mode)
        BASEGFX_DLLPUBLIC BColor rgb2yiq(const BColor& rRGBColor);
        /// Transform from YIQ to R'G'B' (NTSC color model - Y is used in monochrome mode)
        BASEGFX_DLLPUBLIC BColor yiq2rgb(const BColor& rYIQColor);

        /// Transform from R'G'B' to Y'PbPr (the [0,1]x[-.5,.5]x[-.5,.5] equivalent of Y'CbCr (which is scaled into 8bit))
        BASEGFX_DLLPUBLIC BColor rgb2ypbpr(const BColor& rRGBColor);
        /// Transform from Y'PbPr (the [0,1]x[-.5,.5]x[-.5,.5] equivalent of Y'CbCr (which is scaled into 8bit)) into R'G'B'
        BASEGFX_DLLPUBLIC BColor ypbpr2rgb(const BColor& rYPbPrColor);

        /// Transform from CIE XYZ into Rec. 709 RGB (D65 white point)
        BASEGFX_DLLPUBLIC BColor ciexyz2rgb( const BColor& rXYZColor );
        /// Transform from Rec. 709 RGB (D65 white point) into CIE XYZ
        BASEGFX_DLLPUBLIC BColor rgb2ciexyz( const BColor& rRGBColor );

    }
} // end of namespace basegfx

#endif /* _BGFX_COLOR_BCOLORTOOLS_HXX */

//////////////////////////////////////////////////////////////////////////////
// eof
