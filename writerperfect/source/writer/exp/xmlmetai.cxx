/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlmetai.hxx"

#include "xmlimp.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

/// Handler for <dc:title>.
class XMLDcTitleContext : public XMLImportContext
{
public:
    XMLDcTitleContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta);

    void SAL_CALL characters(const OUString &rChars) override;

    XMLMetaDocumentContext &mrMeta;
};

XMLDcTitleContext::XMLDcTitleContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta)
    : XMLImportContext(rImport), mrMeta(rMeta)
{
}

void XMLDcTitleContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrMeta.m_aPropertyList.insert("dc:title", librevenge::RVNGString(sCharU8.getStr()));
}

XMLMetaDocumentContext::XMLMetaDocumentContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

XMLImportContext *XMLMetaDocumentContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "dc:title")
        return new XMLDcTitleContext(mrImport, *this);
    return nullptr;
}

void XMLMetaDocumentContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().setDocumentMetaData(m_aPropertyList);
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
