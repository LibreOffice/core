/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xmlictxt.hxx"

namespace writerperfect::exp
{
/// Handler for <office:text>.
class XMLBodyContentContext : public XMLImportContext
{
public:
    XMLBodyContentContext(XMLImport& rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(
        const OUString& rName,
        const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/) override;
    void SAL_CALL endElement(const OUString& rName) override;
};

/// Context factory for body text, section, table cell, etc.
rtl::Reference<XMLImportContext>
CreateTextChildContext(XMLImport& rImport, std::u16string_view rName, bool bTopLevel = false);

} // namespace writerperfect::exp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
