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

XMLStylesContext::XMLStylesContext(XMLImport &rImport, std::map<OUString, librevenge::RVNGPropertyList> &rParagraphStyles, std::map<OUString, librevenge::RVNGPropertyList> &rTextStyles)
    : XMLImportContext(rImport),
      m_rParagraphStyles(rParagraphStyles),
      m_rTextStyles(rTextStyles)
{
}

XMLImportContext *XMLStylesContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
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

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
