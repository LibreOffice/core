/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <utility>
#include <xmloff/xmlprcon.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/util/Color.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

#include <docmodel/theme/ColorSet.hxx>

namespace model
{
class Theme;
}

/// Imports the theme
class XMLThemeContext : public SvXMLImportContext
{
    css::uno::Reference<css::drawing::XDrawPage> m_xPage;
    std::shared_ptr<model::Theme> mpTheme;

public:
    XMLThemeContext(SvXMLImport& rImport,
                    css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList,
                    css::uno::Reference<css::drawing::XDrawPage> const& xPage);
    ~XMLThemeContext();

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList>& xAttribs) override;
};

/// Imports the color table of a theme
class XMLColorTableContext : public SvXMLImportContext
{
    model::Theme& mrTheme;
    std::unique_ptr<model::ColorSet> mpColorSet;
    std::vector<css::util::Color> m_aColorScheme;

public:
    XMLColorTableContext(SvXMLImport& rImport,
                         css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList,
                         model::Theme& mrTheme);
    ~XMLColorTableContext();

    css::uno::Reference<css::xml::sax::XFastContextHandler> SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttribs) override;
};

/// Imports a color for a color table
class XMLColorContext : public SvXMLImportContext
{
public:
    XMLColorContext(SvXMLImport& rImport,
                    css::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList,
                    std::unique_ptr<model::ColorSet>& rpColorSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
