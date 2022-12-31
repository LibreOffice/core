/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <XMLThemeColorExport.hxx>

#include <sal/config.h>

#include <docmodel/uno/UnoThemeColor.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlexp.hxx>
#include <array>

using namespace css;
using namespace ::xmloff::token;

XMLThemeColorExport::XMLThemeColorExport(SvXMLExport& rExport)
    : mrExport(rExport)
{
}

namespace
{
constexpr const std::array<XMLTokenEnum, 12> constThemeColorTypeToToken{
    XML_DK1,     XML_LT1,     XML_DK2,     XML_LT2,     XML_ACCENT1, XML_ACCENT2,
    XML_ACCENT3, XML_ACCENT4, XML_ACCENT5, XML_ACCENT6, XML_HLINK,   XML_FOLHLINK
};
}

void XMLThemeColorExport::exportXML(const uno::Any& rAny, sal_uInt16 nPrefix,
                                    const OUString& rLocalName)
{
    uno::Reference<util::XThemeColor> xThemeColor;
    rAny >>= xThemeColor;
    if (!xThemeColor.is())
        return;

    model::ThemeColor aThemeColor;
    model::theme::setFromXThemeColor(aThemeColor, xThemeColor);
    if (aThemeColor.getType() == model::ThemeColorType::Unknown)
        return;

    XMLTokenEnum nToken = constThemeColorTypeToToken[sal_Int16(aThemeColor.getType())];
    mrExport.AddAttribute(XML_NAMESPACE_LO_EXT, XML_TYPE, nToken);
    SvXMLElementExport aThemeColorElement(mrExport, nPrefix, rLocalName, true, true);

    for (auto const& rTransform : aThemeColor.getTransformations())
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
