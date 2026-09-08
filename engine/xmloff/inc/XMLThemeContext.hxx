/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xmloff/xmlictxt.hxx>

#include <cpo/uno/XInterface.hpp>

#include <docmodel/theme/ColorSet.hxx>

namespace model
{
class Theme;
}

/// Imports the theme
class XMLThemeContext final : public SvXMLImportContext
{
    // Any UNO object that has the "Theme" property - usually XPage (master page) or XModel
    cpo::uno::Reference<cpo::uno::XInterface> m_xObjectWithThemeProperty;
    std::shared_ptr<model::Theme> mpTheme;

public:
    XMLThemeContext(SvXMLImport& rImport,
                    cpo::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList,
                    cpo::uno::Reference<cpo::uno::XInterface> const& xObjectWithThemeProperty);
    ~XMLThemeContext();

    cpo::uno::Reference<css::xml::sax::XFastContextHandler> createFastChildContext(
        sal_Int32 nElement,
        const cpo::uno::Reference<css::xml::sax::XFastAttributeList>& xAttribs) override;
};

/// Imports the theme colors of a theme
class XMLThemeColorsContext final : public SvXMLImportContext
{
    model::Theme& mrTheme;
    std::shared_ptr<model::ColorSet> m_pColorSet;

public:
    XMLThemeColorsContext(SvXMLImport& rImport,
                          cpo::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList,
                          model::Theme& mrTheme);
    ~XMLThemeColorsContext();

    cpo::uno::Reference<css::xml::sax::XFastContextHandler> createFastChildContext(
        sal_Int32 nElement,
        cpo::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttribs) override;
};

/// Imports a color for a color table
class XMLColorContext final : public SvXMLImportContext
{
public:
    XMLColorContext(SvXMLImport& rImport,
                    cpo::uno::Reference<css::xml::sax::XFastAttributeList> const& xAttrList,
                    const std::shared_ptr<model::ColorSet>& rpColorSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
