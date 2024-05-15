/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xmloff/XMLComplexColorExport.hxx>

#include <sal/config.h>

#include <docmodel/uno/UnoComplexColor.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlexp.hxx>
#include <array>

using namespace css;
using namespace ::xmloff::token;

XMLComplexColorExport::XMLComplexColorExport(SvXMLExport& rExport)
    : mrExport(rExport)
{
}

namespace
{
constexpr const std::array<XMLTokenEnum, 12> constThemeColorTypeToToken{
    XML_DARK1,   XML_LIGHT1,  XML_DARK2,   XML_LIGHT2,  XML_ACCENT1,   XML_ACCENT2,
    XML_ACCENT3, XML_ACCENT4, XML_ACCENT5, XML_ACCENT6, XML_HYPERLINK, XML_FOLLOWED_HYPERLINK
};
}

void XMLComplexColorExport::doExport(model::ComplexColor const& rComplexColor, sal_uInt16 nPrefix,
                                     const OUString& rLocalName)
{
    auto eThemeType = rComplexColor.getThemeColorType();
    if (eThemeType == model::ThemeColorType::Unknown)
        return;

    XMLTokenEnum nToken = constThemeColorTypeToToken[sal_Int16(eThemeType)];
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_THEME_TYPE, nToken);
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_COLOR_TYPE, u"theme"_ustr);
    SvXMLElementExport aComplexColorElement(mrExport, nPrefix, rLocalName, true, true);

    for (auto const& rTransform : rComplexColor.getTransformations())
    {
        OUString aType;
        switch (rTransform.meType)
        {
            case model::TransformationType::Tint:
                aType = "tint";
                break;
            case model::TransformationType::Shade:
                aType = "shade";
                break;
            case model::TransformationType::LumMod:
                aType = "lummod";
                break;
            case model::TransformationType::LumOff:
                aType = "lumoff";
                break;
            default:
                break;
        }
        if (!aType.isEmpty())
        {
            mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_TYPE, aType);
            mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_VALUE,
                                  OUString::number(rTransform.mnValue));
            SvXMLElementExport aTransformElement(mrExport, XML_NAMESPACE_LO_EXT, XML_TRANSFORMATION,
                                                 true, true);
        }
    }
}

void XMLComplexColorExport::exportComplexColor(model::ComplexColor const& rComplexColor,
                                               sal_uInt16 nPrefix, XMLTokenEnum nToken)
{
    doExport(rComplexColor, nPrefix, GetXMLToken(nToken));
}

void XMLComplexColorExport::exportXML(const uno::Any& rAny, sal_uInt16 nPrefix,
                                      const OUString& rLocalName)
{
    uno::Reference<util::XComplexColor> xComplexColor;
    rAny >>= xComplexColor;
    if (!xComplexColor.is())
        return;

    model::ComplexColor aComplexColor = model::color::getFromXComplexColor(xComplexColor);
    doExport(aComplexColor, nPrefix, rLocalName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
