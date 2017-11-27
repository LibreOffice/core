/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLFMT_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLFMT_HXX

#include <map>

#include <librevenge/librevenge.h>

#include "xmlictxt.hxx"

namespace writerperfect
{
namespace exp
{

/// Handler for <office:automatic-styles>/<office:styles>.
class XMLStylesContext : public XMLImportContext
{
public:
    XMLStylesContext(XMLImport &rImport, bool bAutomatic);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentParagraphStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentTextStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentCellStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentColumnStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentRowStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentTableStyles();
    std::map<OUString, librevenge::RVNGPropertyList> &GetCurrentGraphicStyles();
private:
    std::map<OUString, librevenge::RVNGPropertyList> &m_rParagraphStyles;
    std::map<OUString, librevenge::RVNGPropertyList> &m_rTextStyles;
    std::map<OUString, librevenge::RVNGPropertyList> &m_rCellStyles;
    std::map<OUString, librevenge::RVNGPropertyList> &m_rColumnStyles;
    std::map<OUString, librevenge::RVNGPropertyList> &m_rRowStyles;
    std::map<OUString, librevenge::RVNGPropertyList> &m_rTableStyles;
    std::map<OUString, librevenge::RVNGPropertyList> &m_rGraphicStyles;
};

} // namespace exp
} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
