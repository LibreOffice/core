/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLTBLI_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLTBLI_HXX

#include <librevenge/librevenge.h>

#include "xmlictxt.hxx"

namespace writerperfect::exp
{
/// Handler for <table:table>.
class XMLTableContext : public XMLImportContext
{
public:
    XMLTableContext(XMLImport& rImport, bool bTopLevel = false);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;

    void SAL_CALL
    startElement(const OUString& rName,
                 const css::uno::Reference<css::xml::sax::XAttributeList>& xAttribs) override;
    void SAL_CALL endElement(const OUString& rName) override;

private:
    bool m_bTableOpened = false;
    /// If the context is a direct child of XMLBodyContentContext.
    /// Only direct child of XMLBodyContentContext has to handle page span.
    bool m_bTopLevel;
    librevenge::RVNGPropertyList m_aPropertyList;
    librevenge::RVNGPropertyListVector m_aColumns;
};

} // namespace writerperfect::exp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
