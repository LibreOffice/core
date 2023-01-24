/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <XMLThemeContext.hxx>

#include <xmloff/maptype.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlprcon.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlement.hxx>
#include <xmloff/xmlprhdl.hxx>

#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <sax/tools/converter.hxx>
#include <comphelper/sequence.hxx>

using namespace css;
using namespace xmloff::token;

XMLThemeContext::XMLThemeContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 css::uno::Reference<css::drawing::XDrawPage> const& xPage)
    : SvXMLImportContext(rImport)
    , m_xPage(xPage)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                m_aTheme["Name"] <<= rAttribute.toString();
                break;
            }
        }
    }
}

XMLThemeContext::~XMLThemeContext()
{
    uno::Any aTheme(m_aTheme.getAsConstPropertyValueList());
    uno::Reference<beans::XPropertySet> xPropertySet(m_xPage, uno::UNO_QUERY);
    xPropertySet->setPropertyValue("Theme", aTheme);
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLThemeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR_TABLE))
    {
        return new XMLColorTableContext(GetImport(), xAttribs, m_aTheme);
    }

    return nullptr;
}

XMLColorTableContext::XMLColorTableContext(
    SvXMLImport& rImport, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
    comphelper::SequenceAsHashMap& rTheme)
    : SvXMLImportContext(rImport)
    , m_rTheme(rTheme)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                m_rTheme["ColorSchemeName"] <<= rAttribute.toString();
                break;
            }
        }
    }
}

XMLColorTableContext::~XMLColorTableContext()
{
    m_rTheme["ColorScheme"] <<= comphelper::containerToSequence(m_aColorScheme);
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLColorTableContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR))
    {
        return new XMLColorContext(GetImport(), xAttribs, m_aColorScheme);
    }

    return nullptr;
}

XMLColorContext::XMLColorContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 std::vector<util::Color>& rColorScheme)
    : SvXMLImportContext(rImport)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_COLOR):
            {
                util::Color nColor;
                sax::Converter::convertColor(nColor, rAttribute.toView());
                rColorScheme.push_back(nColor);
                break;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
