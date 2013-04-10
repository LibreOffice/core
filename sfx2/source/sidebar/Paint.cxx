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

#include "precompiled_sfx2.hxx"

#include "Paint.hxx"
#include "Tools.hxx"
#include <com/sun/star/awt/Gradient.hpp>


using namespace ::com::sun::star;

namespace sfx2 { namespace sidebar {

Paint::Paint (void)
    : meType(NoPaint)
{
}




Paint::Paint (const Color& rColor)
    : meType(ColorPaint),
      maValue(rColor)
{
}




Paint::Paint (const Gradient& rGradient)
    : meType(GradientPaint),
      maValue(rGradient)
{
}




Paint Paint::Create (const cssu::Any& rValue)
{
    ColorData aColor (0);
    if (rValue >>= aColor)
        return Paint(Color(aColor));

    awt::Gradient aAwtGradient;
    if (rValue >>= aAwtGradient)
        return Paint(Tools::AwtToVclGradient(aAwtGradient));

    return Paint();
}




void Paint::Set (const Paint& rOther)
{
    meType = rOther.meType;
    maValue = rOther.maValue;
}




Paint::Type Paint::GetType (void) const
{
    return meType;
}




const Color& Paint::GetColor (void) const
{
    if (meType != ColorPaint)
    {
        assert(meType==ColorPaint);
        static Color aErrorColor;
        return aErrorColor;
    }
    else
        return ::boost::get<Color>(maValue);
}




const Gradient& Paint::GetGradient (void) const
{
    if (meType != GradientPaint)
    {
        assert(meType==GradientPaint);
        static Gradient aErrorGradient;
        return aErrorGradient;
    }
    else
        return ::boost::get<Gradient>(maValue);
}




Wallpaper Paint::GetWallpaper (void) const
{
    switch (meType)
    {
        case Paint::NoPaint:
        default:
            return Wallpaper();
            break;

        case Paint::ColorPaint:
            return Wallpaper(GetColor());
            break;

        case Paint::GradientPaint:
            return Wallpaper(GetGradient());
            break;
    }
}


} } // end of namespace sfx2::sidebar
