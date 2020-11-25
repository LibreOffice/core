/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmltext.hxx"

#include "txtparai.hxx"
#include "xmltbli.hxx"
#include "XMLSectionContext.hxx"
#include "XMLTextListContext.hxx"
#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect::exp
{
XMLBodyContentContext::XMLBodyContentContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

void XMLBodyContentContext::endElement(const OUString& /*rName*/)
{
    if (GetImport().GetIsInPageSpan())
        GetImport().GetGenerator().closePageSpan();
}

rtl::Reference<XMLImportContext> XMLBodyContentContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateTextChildContext(GetImport(), rName, true);
}

rtl::Reference<XMLImportContext> CreateTextChildContext(XMLImport& rImport,
                                                        std::u16string_view rName, bool bTopLevel)
{
    if (rName == u"text:p" || rName == u"text:h")
        return new XMLParaContext(rImport, bTopLevel);
    if (rName == u"text:section")
        return new XMLSectionContext(rImport);
    if (rName == u"table:table")
        return new XMLTableContext(rImport, bTopLevel);
    if (rName == u"text:list")
        return new XMLTextListContext(rImport);
    return nullptr;
}

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
