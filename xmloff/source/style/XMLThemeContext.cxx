/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <XMLThemeContext.hxx>

#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#include <sax/tools/converter.hxx>

#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace css;
using namespace xmloff::token;

XMLThemeContext::XMLThemeContext(
    SvXMLImport& rImport, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
    css::uno::Reference<css::uno::XInterface> const& xObjectWithThemeProperty)
    : SvXMLImportContext(rImport)
    , m_xObjectWithThemeProperty(xObjectWithThemeProperty)
    , mpTheme(new model::Theme)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                OUString aName = rAttribute.toString();
                mpTheme->SetName(aName);
                break;
            }
        }
    }
}

XMLThemeContext::~XMLThemeContext()
{
    if (mpTheme && mpTheme->getColorSet())
    {
        uno::Reference<beans::XPropertySet> xPropertySet(m_xObjectWithThemeProperty,
                                                         uno::UNO_QUERY);
        auto xTheme = model::theme::createXTheme(mpTheme);
        xPropertySet->setPropertyValue("Theme", uno::Any(xTheme));
    }
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLThemeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_THEME_COLORS))
    {
        return new XMLThemeColorsContext(GetImport(), xAttribs, *mpTheme);
    }

    return nullptr;
}

XMLThemeColorsContext::XMLThemeColorsContext(
    SvXMLImport& rImport, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
    model::Theme& rTheme)
    : SvXMLImportContext(rImport)
    , mrTheme(rTheme)
{
    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                OUString aName = rAttribute.toString();
                m_pColorSet.reset(new model::ColorSet(aName));
                break;
            }
        }
    }
}

XMLThemeColorsContext::~XMLThemeColorsContext()
{
    if (m_pColorSet)
        mrTheme.setColorSet(m_pColorSet);
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL XMLThemeColorsContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR))
    {
        if (m_pColorSet)
            return new XMLColorContext(GetImport(), xAttribs, m_pColorSet);
    }

    return nullptr;
}

XMLColorContext::XMLColorContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 std::shared_ptr<model::ColorSet>& rpColorSet)
    : SvXMLImportContext(rImport)
{
    OUString aName;
    ::Color aColor;

    for (const auto& rAttribute : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rAttribute.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_NAME):
            {
                aName = rAttribute.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_COLOR):
            {
                sax::Converter::convertColor(aColor, rAttribute.toView());
                break;
            }
        }
    }

    if (!aName.isEmpty())
    {
        auto eType = model::ThemeColorType::Unknown;
        if (aName == u"dark1")
            eType = model::ThemeColorType::Dark1;
        else if (aName == u"light1")
            eType = model::ThemeColorType::Light1;
        else if (aName == u"dark2")
            eType = model::ThemeColorType::Dark2;
        else if (aName == u"light2")
            eType = model::ThemeColorType::Light2;
        else if (aName == u"accent1")
            eType = model::ThemeColorType::Accent1;
        else if (aName == u"accent2")
            eType = model::ThemeColorType::Accent2;
        else if (aName == u"accent3")
            eType = model::ThemeColorType::Accent3;
        else if (aName == u"accent4")
            eType = model::ThemeColorType::Accent4;
        else if (aName == u"accent5")
            eType = model::ThemeColorType::Accent5;
        else if (aName == u"accent6")
            eType = model::ThemeColorType::Accent6;
        else if (aName == u"hyperlink")
            eType = model::ThemeColorType::Hyperlink;
        else if (aName == u"followed-hyperlink")
            eType = model::ThemeColorType::FollowedHyperlink;

        if (eType != model::ThemeColorType::Unknown)
        {
            rpColorSet->add(eType, aColor);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
