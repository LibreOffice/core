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

#include "Tools.hxx"

#include "sfx2/sidebar/Theme.hxx"

#include "sfx2/imagemgr.hxx"
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <vcl/gradient.hxx>

#include <com/sun/star/graphic/XGraphicProvider.hpp>

#include <cstring>

using namespace css;
using namespace cssu;


namespace sfx2 { namespace sidebar {

Image Tools::GetImage (
    const ::rtl::OUString& rsImageURL,
    const ::rtl::OUString& rsHighContrastImageURL,
    const Reference<frame::XFrame>& rxFrame)
{
    if (Theme::IsHighContrastMode())
        return GetImage(rsHighContrastImageURL, rxFrame);
    else
        return GetImage(rsImageURL, rxFrame);
}




Image Tools::GetImage (
    const ::rtl::OUString& rsURL,
    const Reference<frame::XFrame>& rxFrame)
{
    if (rsURL.getLength() > 0)
    {
        static const sal_Char* sUnoCommandPrefix = ".uno:";
        static const sal_Int32 nUnoCommandPrefixLength = strlen(sUnoCommandPrefix);
        static const sal_Char* sCommandImagePrefix = "private:commandimage/";
        static const sal_Int32 nCommandImagePrefixLength = strlen(sCommandImagePrefix);

        if (rsURL.compareToAscii(sUnoCommandPrefix, nUnoCommandPrefixLength) == 0)
        {
            const Image aPanelImage (::GetImage(rxFrame, rsURL, sal_False, Theme::IsHighContrastMode()));
            return aPanelImage;
        }
        else if (rsURL.compareToAscii(sCommandImagePrefix, nCommandImagePrefixLength) == 0)
        {
            ::rtl::OUStringBuffer aCommandName;
            aCommandName.appendAscii(sUnoCommandPrefix);
            aCommandName.append(rsURL.copy(nCommandImagePrefixLength));
            const ::rtl::OUString sCommandName (aCommandName.makeStringAndClear());

            const Image aPanelImage (::GetImage(rxFrame, sCommandName, sal_False, Theme::IsHighContrastMode()));
            return aPanelImage;
        }
        else
        {
            const ::comphelper::ComponentContext aContext (::comphelper::getProcessServiceFactory());
            const Reference<graphic::XGraphicProvider> xGraphicProvider (
                aContext.createComponent("com.sun.star.graphic.GraphicProvider"),
                UNO_QUERY);
            if ( xGraphicProvider.is())
            {
                ::comphelper::NamedValueCollection aMediaProperties;
                aMediaProperties.put("URL", rsURL);
                const Reference<graphic::XGraphic> xGraphic (
                    xGraphicProvider->queryGraphic(aMediaProperties.getPropertyValues()),
                    UNO_QUERY);
                if (xGraphic.is())
                    return Image(xGraphic);
            }
        }
    }
    return Image();
}




css::awt::Gradient Tools::VclToAwtGradient (const Gradient aVclGradient)
{
    css::awt::Gradient aAwtGradient (
        awt::GradientStyle(aVclGradient.GetStyle()),
        aVclGradient.GetStartColor().GetRGBColor(),
        aVclGradient.GetEndColor().GetRGBColor(),
        aVclGradient.GetAngle(),
        aVclGradient.GetBorder(),
        aVclGradient.GetOfsX(),
        aVclGradient.GetOfsY(),
        aVclGradient.GetStartIntensity(),
        aVclGradient.GetEndIntensity(),
        aVclGradient.GetSteps());
    return aAwtGradient;
}




Gradient Tools::AwtToVclGradient (const css::awt::Gradient aAwtGradient)
{
    Gradient aVclGradient (
        GradientStyle(aAwtGradient.Style),
        aAwtGradient.StartColor,
        aAwtGradient.EndColor);
    aVclGradient.SetAngle(aAwtGradient.Angle);
    aVclGradient.SetBorder(aAwtGradient.Border);
    aVclGradient.SetOfsX(aAwtGradient.XOffset);
    aVclGradient.SetOfsY(aAwtGradient.YOffset);
    aVclGradient.SetStartIntensity(aAwtGradient.StartIntensity);
    aVclGradient.SetEndIntensity(aAwtGradient.EndIntensity);
    aVclGradient.SetSteps(aAwtGradient.StepCount);

    return aVclGradient;
}




SvBorder Tools::RectangleToSvBorder (const Rectangle aBox)
{
    return SvBorder(
        aBox.Left(),
        aBox.Top(),
        aBox.Right(),
        aBox.Bottom());
}

} } // end of namespace sfx2::sidebar
