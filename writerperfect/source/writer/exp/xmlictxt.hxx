/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLICTXT_HXX
#define INCLUDED_WRITERPERFECT_SOURCE_WRITER_EXP_XMLICTXT_HXX

#include <cppuhelper/implbase.hxx>

#include <librevenge/librevenge.h>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

namespace writerperfect
{
namespace exp
{

class XMLImport;

/// Base class for a handler of a single XML element during ODF -> librevenge conversion.
class XMLImportContext : public cppu::WeakImplHelper
    <
    css::xml::sax::XDocumentHandler
    >
{
public:
    XMLImportContext(XMLImport &rImport);

    virtual XMLImportContext *CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs);

    // XDocumentHandler
    void SAL_CALL startDocument() override;
    void SAL_CALL endDocument() override;
    void SAL_CALL startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs) override;
    void SAL_CALL endElement(const OUString &rName) override;
    void SAL_CALL characters(const OUString &rChars) override;
    void SAL_CALL ignorableWhitespace(const OUString &rWhitespaces) override;
    void SAL_CALL processingInstruction(const OUString &rTarget, const OUString &rData) override;
    void SAL_CALL setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &xLocator) override;

protected:
    XMLImport &mrImport;
};

} // namespace exp
} // namespace writerperfect

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
