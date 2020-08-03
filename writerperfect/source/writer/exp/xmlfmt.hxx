/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <map>

#include "xmlictxt.hxx"

namespace librevenge
{
class RVNGPropertyList;
}

namespace writerperfect::exp
{
/// Handler for <office:automatic-styles>/<office:styles>.
class XMLStylesContext : public XMLImportContext
{
public:
    enum StyleType
    {
        StyleType_NONE,
        StyleType_AUTOMATIC
    };
    XMLStylesContext(XMLImport& rImport, StyleType eType);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentParagraphStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentTextStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentCellStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentColumnStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentRowStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentTableStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentGraphicStyles();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentPageLayouts();
    std::map<OUString, librevenge::RVNGPropertyList>& GetCurrentMasterStyles();

private:
    std::map<OUString, librevenge::RVNGPropertyList>& m_rParagraphStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rTextStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rCellStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rColumnStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rRowStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rTableStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rGraphicStyles;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rPageLayouts;
    std::map<OUString, librevenge::RVNGPropertyList>& m_rMasterStyles;
};

/// Handler for <office:font-face-decls>.
class XMLFontFaceDeclsContext : public XMLImportContext
{
public:
    XMLFontFaceDeclsContext(XMLImport& rImport);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
};

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
