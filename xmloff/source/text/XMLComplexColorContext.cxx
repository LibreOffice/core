/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sal/log.hxx>
#include <sax/tools/converter.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlprhdl.hxx>
#include <XMLComplexColorContext.hxx>

using namespace css;
using namespace xmloff::token;

SvXMLEnumMapEntry<sal_Int16> const pXML_ThemeColor_Enum[] = { { XML_NONE, -1 },
                                                              { XML_DARK1, 0 },
                                                              { XML_LIGHT1, 1 },
                                                              { XML_DARK2, 2 },
                                                              { XML_LIGHT2, 3 },
                                                              { XML_ACCENT1, 4 },
                                                              { XML_ACCENT2, 5 },
                                                              { XML_ACCENT3, 6 },
                                                              { XML_ACCENT4, 7 },
                                                              { XML_ACCENT5, 8 },
                                                              { XML_ACCENT6, 9 },
                                                              { XML_HYPERLINK, 10 },
                                                              { XML_FOLLOWED_HYPERLINK, 11 },
                                                              { XML_TOKEN_INVALID, 0 } };

XMLComplexColorContext::XMLComplexColorContext(
    SvXMLImport& rImport, sal_Int32 nElement,
    const uno::Reference<xml::sax::XFastAttributeList>& xAttrList, const XMLPropertyState& rProp,
    std::vector<XMLPropertyState>& rProps)
    : XMLElementPropertyContext(rImport, nElement, rProp, rProps)
    , mnRootElement(nElement)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_THEME_TYPE):
            {
                sal_Int16 nValue = -1;
                if (SvXMLUnitConverter::convertEnum(nValue, aIter.toView(), pXML_ThemeColor_Enum))
                {
                    maComplexColor.setSchemeColor(model::convertToThemeColorType(nValue));
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_COLOR_TYPE):
            {
                const OUString aValue = aIter.toString();
                if (aValue == u"theme")
                    maComplexColor.setType(model::ColorType::Scheme);
                // TODO - handle other color types
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                break;
        }
    }
}

css::uno::Reference<css::xml::sax::XFastContextHandler>
XMLComplexColorContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttrList)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_TRANSFORMATION))
    {
        auto eTransformationType = model::TransformationType::Undefined;
        sal_Int16 nTransformationValue = 0;
        for (auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(LO_EXT, XML_TYPE):
                {
                    const OUString aValue = aIter.toString();
                    if (aValue == u"tint")
                        eTransformationType = model::TransformationType::Tint;
                    else if (aValue == u"shade")
                        eTransformationType = model::TransformationType::Shade;
                    else if (aValue == u"lumoff")
                        eTransformationType = model::TransformationType::LumOff;
                    else if (aValue == u"lummod")
                        eTransformationType = model::TransformationType::LumMod;
                    break;
                }
                case XML_ELEMENT(LO_EXT, XML_VALUE):
                {
                    sal_Int32 nValue;
                    if (::sax::Converter::convertNumber(nValue, aIter.toView(), SHRT_MIN, SHRT_MAX))
                        nTransformationValue = static_cast<sal_Int16>(nValue);
                    break;
                }
                default:
                    XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                    break;
            }
        }
        maComplexColor.addTransformation({ eTransformationType, nTransformationValue });
        return this;
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void XMLComplexColorContext::endFastElement(sal_Int32 nElement)
{
    if (nElement == mnRootElement)
    {
        if (maComplexColor.getSchemeType() != model::ThemeColorType::Unknown)
        {
            aProp.maValue <<= model::color::createXComplexColor(maComplexColor);
            SetInsert(true);
        }
    }
    XMLElementPropertyContext::endFastElement(nElement);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
