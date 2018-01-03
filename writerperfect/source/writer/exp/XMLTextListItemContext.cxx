/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLTextListItemContext.hxx"

#include "XMLTextListContext.hxx"
#include "txtparai.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

XMLTextListItemContext::XMLTextListItemContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLTextListItemContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "text:p" || rName == "text:h")
        return new XMLParaContext(mrImport);
    if (rName == "text:list")
        return new XMLTextListContext(mrImport);
    return nullptr;
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
