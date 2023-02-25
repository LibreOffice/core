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

#include <docmodel/uno/UnoTheme.hxx>
#include <docmodel/theme/Theme.hxx>

using namespace css;
using namespace xmloff::token;

XMLThemeContext::XMLThemeContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 css::uno::Reference<css::drawing::XDrawPage> const& xPage)
    : SvXMLImportContext(rImport)
    , m_xPage(xPage)
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
    uno::Reference<beans::XPropertySet> xPropertySet(m_xPage, uno::UNO_QUERY);
    auto xTheme = model::theme::createXTheme(mpTheme);
    xPropertySet->setPropertyValue("Theme", uno::Any(xTheme));
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLThemeContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR_TABLE))
    {
        return new XMLColorTableContext(GetImport(), xAttribs, *mpTheme);
    }

    return nullptr;
}

XMLColorTableContext::XMLColorTableContext(
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
                mpColorSet.reset(new model::ColorSet(aName));
                break;
            }
        }
    }
}

XMLColorTableContext::~XMLColorTableContext()
{
    if (mpColorSet)
        mrTheme.SetColorSet(std::move(mpColorSet));
}

uno::Reference<xml::sax::XFastContextHandler> SAL_CALL XMLColorTableContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttribs)
{
    if (nElement == XML_ELEMENT(LO_EXT, XML_COLOR))
    {
        if (mpColorSet)
            return new XMLColorContext(GetImport(), xAttribs, mpColorSet);
    }

    return nullptr;
}

XMLColorContext::XMLColorContext(SvXMLImport& rImport,
                                 const uno::Reference<xml::sax::XFastAttributeList>& xAttrList,
                                 std::unique_ptr<model::ColorSet>& rpColorSet)
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
        if (aName == u"dk1")
            eType = model::ThemeColorType::Dark1;
        else if (aName == u"lt1")
            eType = model::ThemeColorType::Light1;
        else if (aName == u"dk2")
            eType = model::ThemeColorType::Dark2;
        else if (aName == u"lt2")
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
        else if (aName == u"hlink")
            eType = model::ThemeColorType::Hyperlink;
        else if (aName == u"folHlink")
            eType = model::ThemeColorType::FollowedHyperlink;

        if (eType != model::ThemeColorType::Unknown)
        {
            rpColorSet->add(eType, aColor);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
