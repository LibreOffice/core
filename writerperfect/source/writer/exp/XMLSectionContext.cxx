/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLSectionContext.hxx"

#include "xmlimp.hxx"
#include "xmltext.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{
XMLSectionContext::XMLSectionContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLSectionContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    return CreateTextChildContext(GetImport(), rName);
}

void XMLSectionContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    GetImport().GetGenerator().openSection(librevenge::RVNGPropertyList());
}

void XMLSectionContext::endElement(const OUString& /*rName*/)
{
    GetImport().GetGenerator().closeSection();
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
