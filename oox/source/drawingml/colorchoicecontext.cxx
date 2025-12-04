/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <oox/drawingml/drawingmltypes.hxx>
#include <drawingml/colorchoicecontext.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/drawingml/color.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <unordered_map>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>

namespace oox::drawingml {

namespace
{

constexpr auto constSystemColorMap = frozen::make_unordered_map<sal_Int32, model::SystemColorType>
({
    { XML_scrollBar, model::SystemColorType::ScrollBar },
    { XML_background, model::SystemColorType::Background },
    { XML_activeCaption, model::SystemColorType::ActiveCaption },
    { XML_inactiveCaption, model::SystemColorType::InactiveCaption },
    { XML_menu, model::SystemColorType::Menu },
    { XML_window, model::SystemColorType::Window },
    { XML_windowFrame, model::SystemColorType::WindowFrame },
    { XML_menuText, model::SystemColorType::MenuText },
    { XML_windowText, model::SystemColorType::WindowText },
    { XML_captionText, model::SystemColorType::CaptionText },
    { XML_activeBorder, model::SystemColorType::ActiveBorder },
    { XML_inactiveBorder, model::SystemColorType::InactiveBorder },
    { XML_appWorkspace, model::SystemColorType::AppWorkspace },
    { XML_highlight, model::SystemColorType::Highlight },
    { XML_highlightText, model::SystemColorType::HighlightText },
    { XML_btnFace, model::SystemColorType::ButtonFace },
    { XML_btnShadow, model::SystemColorType::ButtonShadow },
    { XML_grayText, model::SystemColorType::GrayText },
    { XML_btnText, model::SystemColorType::ButtonText },
    { XML_inactiveCaptionText, model::SystemColorType::InactiveCaptionText },
    { XML_btnHighlight, model::SystemColorType::ButtonHighlight },
    { XML_3dDkShadow, model::SystemColorType::DarkShadow3D },
    { XML_3dLight, model::SystemColorType::Light3D },
    { XML_infoText, model::SystemColorType::InfoText },
    { XML_infoBk, model::SystemColorType::InfoBack },
    { XML_hotLight, model::SystemColorType::HotLight },
    { XML_gradientActiveCaption, model::SystemColorType::GradientActiveCaption },
    { XML_gradientInactiveCaption, model::SystemColorType::GradientInactiveCaption },
    { XML_menuHighlight, model::SystemColorType::MenuHighlight },
    { XML_menuBar, model::SystemColorType::MenuBar }
});

}

ColorValueContext::ColorValueContext(ContextHandler2Helper const & rParent, Color* pColor, model::ComplexColor* pComplexColor)
    : ContextHandler2(rParent)
    , mpColor(pColor)
    , mpComplexColor(pComplexColor)
{
}

void ColorValueContext::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case A_TOKEN(scrgbClr):
        {
            if (mpColor)
            {
                mpColor->setScrgbClr(
                    rAttribs.getInteger( XML_r, 0 ),
                    rAttribs.getInteger( XML_g, 0 ),
                    rAttribs.getInteger( XML_b, 0 ) );
            }
            if (mpComplexColor)
            {
                mpComplexColor->setCRGB(
                    rAttribs.getInteger(XML_r, 0),
                    rAttribs.getInteger(XML_g, 0),
                    rAttribs.getInteger(XML_b, 0));
            }
        }
        break;

        case A_TOKEN(srgbClr):
        {
            if (mpColor) mpColor->setSrgbClr(rAttribs.getIntegerHex(XML_val, 0));
            if (mpComplexColor)
            {
                mpComplexColor->setRGB(rAttribs.getIntegerHex(XML_val, 0));
            }
        }
        break;

        case A_TOKEN(hslClr):
        {
            if (mpColor)
            {
                mpColor->setHslClr(
                    rAttribs.getInteger(XML_hue, 0),
                    rAttribs.getInteger(XML_sat, 0),
                    rAttribs.getInteger(XML_lum, 0));
            }

            if (mpComplexColor)
            {
                mpComplexColor->setHSL(
                    rAttribs.getInteger(XML_hue, 0),
                    rAttribs.getInteger(XML_sat, 0),
                    rAttribs.getInteger(XML_lum, 0));
            }
        }
        break;

        case A_TOKEN(sysClr):
        {
            sal_Int32 nToken = rAttribs.getToken(XML_val, XML_TOKEN_INVALID);
            sal_Int32 nLastColor = rAttribs.getIntegerHex(XML_lastClr, -1);

            if (mpColor) mpColor->setSysClr(nToken, nLastColor);

            if (mpComplexColor)
            {
                auto aIterator = constSystemColorMap.find(nToken);
                if (aIterator != constSystemColorMap.end())
                {
                    auto const& aPair = *aIterator;
                    model::SystemColorType eType = aPair.second;
                    mpComplexColor->setSystemColor(eType, nLastColor);
                }
            }
        }
        break;

        case A_TOKEN(schemeClr):
        {
            auto nToken = rAttribs.getToken(XML_val, XML_TOKEN_INVALID);
            if (mpColor) mpColor->setSchemeClr(nToken);
            std::optional<OUString> sSchemeName = rAttribs.getString(XML_val);
            if (sSchemeName.has_value())
            {
                if (mpColor) mpColor->setSchemeName(*sSchemeName);

                if (mpComplexColor)
                {
                    if (nToken == XML_phClr)
                    {
                        mpComplexColor->setThemePlaceholder();
                    }
                    else
                    {
                        model::ThemeColorType eType = schemeNameToThemeColorType(*sSchemeName);
                        mpComplexColor->setThemeColor(eType);
                    }
                }
            }
        }
        break;

        case A_TOKEN(prstClr):
        {
            sal_Int32 nToken = rAttribs.getToken(XML_val, XML_TOKEN_INVALID);
            if (mpColor) mpColor->setPrstClr(nToken);
            if (mpComplexColor)
            {
                // TODO - just converted to RGB for now
                ::Color nRgbValue = Color::getDmlPresetColor(nToken, API_RGB_TRANSPARENT);
                mpComplexColor->setColor(nRgbValue);
            }
        }
        break;
    }
}

::oox::core::ContextHandlerRef ColorValueContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case A_TOKEN( alpha ):
        case A_TOKEN( alphaMod ):
        case A_TOKEN( alphaOff ):
        case A_TOKEN( blue ):
        case A_TOKEN( blueMod ):
        case A_TOKEN( blueOff ):
        case A_TOKEN( hue ):
        case A_TOKEN( hueMod ):
        case A_TOKEN( hueOff ):
        case A_TOKEN( lum ):
        case A_TOKEN( lumMod ):
        case A_TOKEN( lumOff ):
        case A_TOKEN( green ):
        case A_TOKEN( greenMod ):
        case A_TOKEN( greenOff ):
        case A_TOKEN( red ):
        case A_TOKEN( redMod ):
        case A_TOKEN( redOff ):
        case A_TOKEN( sat ):
        case A_TOKEN( satMod ):
        case A_TOKEN( satOff ):
        case A_TOKEN( shade ):
        case A_TOKEN( tint ):
        {
            OUString aValue = rAttribs.getStringDefaulted( XML_val);
            sal_Int32 nVal = 0;
            if (aValue.endsWith("%"))
            {
                nVal = aValue.toDouble() * PER_PERCENT;
            }
            else
                nVal = rAttribs.getInteger(XML_val, 0);
            if (mpColor) mpColor->addTransformation(nElement, nVal);
        }
        break;
        case A_TOKEN( comp ):
        case A_TOKEN( gamma ):
        case A_TOKEN( gray ):
        case A_TOKEN( inv ):
        case A_TOKEN( invGamma ):
            if (mpColor) mpColor->addTransformation( nElement );
        break;
    }

    if (mpComplexColor)
    {
        model::TransformationType eType = Color::getTransformationType(getBaseToken(nElement));

        OUString aValueString = rAttribs.getStringDefaulted(XML_val);
        sal_Int32 nValue = 0;
        if (aValueString.endsWith("%"))
            nValue = aValueString.toDouble() * PER_PERCENT;
        else
            nValue = rAttribs.getInteger(XML_val, 0);

        mpComplexColor->addTransformation({eType, sal_Int16(nValue / 10)});
    }

    return nullptr;
}

//===========
// ColorContext
//===========

ColorContext::ColorContext(ContextHandler2Helper const & rParent, Color* pColor, model::ComplexColor* pComplexColor)
    : ContextHandler2(rParent)
    , mpColor(pColor)
    , mpComplexColor(pComplexColor)
{
}

::oox::core::ContextHandlerRef ColorContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( scrgbClr ):
        case A_TOKEN( srgbClr ):
        case A_TOKEN( hslClr ):
        case A_TOKEN( sysClr ):
        case A_TOKEN( schemeClr ):
        case A_TOKEN( prstClr ):
            return new ColorValueContext(*this, mpColor, mpComplexColor);
    }
    return nullptr;
}

//===========
// VariationContext
//===========

VariationContext::VariationContext(ContextHandler2Helper const & rParent,
        Color* pColor, model::ComplexColor* pComplexColor)
    : ColorValueContext(rParent, pColor, pComplexColor)
    , mpColor(pColor)
    , mpComplexColor(pComplexColor)
{
}

::oox::core::ContextHandlerRef VariationContext::onCreateContext(
        sal_Int32 nElement, const AttributeList& )
{
    switch( nElement )
    {
        case A_TOKEN( scrgbClr ):
        case A_TOKEN( srgbClr ):
        case A_TOKEN( hslClr ):
        case A_TOKEN( sysClr ):
        case A_TOKEN( schemeClr ):
        case A_TOKEN( prstClr ):
            return new ColorValueContext(*this, mpColor, mpComplexColor);
    }
    return nullptr;
}

//===========
// ColorsContext
//===========

ColorsContext::ColorsContext(ContextHandler2Helper const& rParent, std::vector<Color>& rColors)
    : ContextHandler2(rParent)
    , mrColors(rColors)
{
}

::oox::core::ContextHandlerRef ColorsContext::onCreateContext(sal_Int32 nElement,
                                                              const AttributeList&)
{
    switch (nElement)
    {
        case A_TOKEN(scrgbClr):
        case A_TOKEN(srgbClr):
        case A_TOKEN(hslClr):
        case A_TOKEN(sysClr):
        case A_TOKEN(schemeClr):
        case A_TOKEN(prstClr):
        {
            mrColors.emplace_back();
            return new ColorValueContext(*this, &mrColors.back(), nullptr);
        }
    }
    return nullptr;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
