/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLTEXTFRAMECONTEXT_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLTEXTFRAMECONTEXT_HXX

#include <rtl/ref.hxx>

#include "xmlictxt.hxx"

namespace writerperfect
{
namespace exp
{

class XMLBase64ImportContext;

/// Handler for <draw:frame>.
class XMLTextFrameContext : public XMLImportContext
{
public:
    XMLTextFrameContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;

    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
};

} // namespace exp
} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
