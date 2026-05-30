/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLTextListContext.hxx"

#include "XMLTextListItemContext.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect::exp
{
XMLTextListContext::XMLTextListContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextListContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "text:list-item")
        return new XMLTextListItemContext(GetImport());
    return nullptr;
}

void XMLTextListContext::startElement(
    const OUString& /*rName*/,
    const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    // TODO we should find a way to know if we're in the case of ordered or unordered list
    // in second case, we should call "openUnorderedListLevel"
    GetImport().GetGenerator().openOrderedListLevel(librevenge::RVNGPropertyList());
}

void XMLTextListContext::endElement(const OUString& /*rName*/)
{
    // TODO we should find a way to know if we're in the case of ordered or unordered list
    // in second case, we should call "closeUnorderedListLevel"
    GetImport().GetGenerator().closeOrderedListLevel();
}

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
