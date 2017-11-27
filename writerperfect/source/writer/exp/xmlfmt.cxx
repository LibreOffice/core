/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlfmt.hxx"

#include "txtstyli.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLStylesContext::XMLStylesContext(XMLImport &rImport, bool bAutomatic)
    : XMLImportContext(rImport),
      m_rParagraphStyles(bAutomatic ? mrImport.GetAutomaticParagraphStyles() : mrImport.GetParagraphStyles()),
      m_rTextStyles(bAutomatic ? mrImport.GetAutomaticTextStyles() : mrImport.GetTextStyles()),
      m_rCellStyles(bAutomatic ? mrImport.GetAutomaticCellStyles() : mrImport.GetCellStyles()),
      m_rColumnStyles(bAutomatic ? mrImport.GetAutomaticColumnStyles() : mrImport.GetColumnStyles()),
      m_rRowStyles(bAutomatic ? mrImport.GetAutomaticRowStyles() : mrImport.GetRowStyles()),
      m_rTableStyles(bAutomatic ? mrImport.GetAutomaticTableStyles() : mrImport.GetTableStyles()),
      m_rGraphicStyles(bAutomatic ? mrImport.GetAutomaticGraphicStyles() : mrImport.GetGraphicStyles())
{
}

rtl::Reference<XMLImportContext> XMLStylesContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "style:style")
        return new XMLStyleContext(mrImport, *this);
    return nullptr;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentParagraphStyles()
{
    return m_rParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentTextStyles()
{
    return m_rTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentCellStyles()
{
    return m_rCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentColumnStyles()
{
    return m_rColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentRowStyles()
{
    return m_rRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentTableStyles()
{
    return m_rTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLStylesContext::GetCurrentGraphicStyles()
{
    return m_rGraphicStyles;
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
