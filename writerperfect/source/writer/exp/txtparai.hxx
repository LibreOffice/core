/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_TXTPARAI_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_TXTPARAI_HXX

#include <map>

#include "xmlictxt.hxx"

namespace writerperfect
{
namespace exp
{

/// Handler for <text:p>/<text:h>.
class XMLParaContext : public XMLImportContext
{
public:
    XMLParaContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;

private:
    OUString m_aStyleName;
    /// List of properties spans should inherit from this paragraph.
    librevenge::RVNGPropertyList m_aTextPropertyList;
};

/// Shared child context factory for paragraph and span contexts.
rtl::Reference<XMLImportContext> CreateParagraphOrSpanChildContext(XMLImport &rImport, const OUString &rName, const librevenge::RVNGPropertyList &rTextPropertyList);

/// Looks for rName in rAutomaticStyles (and failing that, in rNamedStyles) and
/// fills rPropertyList based on that.
void FillStyles(const OUString &rName,
                std::map<OUString, librevenge::RVNGPropertyList> &rAutomaticStyles,
                std::map<OUString, librevenge::RVNGPropertyList> &rNamedStyles,
                librevenge::RVNGPropertyList &rPropertyList);

} // namespace exp
} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
