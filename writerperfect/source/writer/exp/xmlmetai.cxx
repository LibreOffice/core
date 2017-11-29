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

/// Handler for <dc:language>.
class XMLDcLanguageContext : public XMLImportContext
{
public:
    XMLDcLanguageContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta);

    void SAL_CALL characters(const OUString &rChars) override;

    XMLMetaDocumentContext &mrMeta;
};

XMLDcLanguageContext::XMLDcLanguageContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta)
    : XMLImportContext(rImport), mrMeta(rMeta)
{
}

void XMLDcLanguageContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrMeta.m_aPropertyList.insert("dc:language", librevenge::RVNGString(sCharU8.getStr()));
}

/// Handler for <dc:date>.
class XMLDcDateContext : public XMLImportContext
{
public:
    XMLDcDateContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta);

    void SAL_CALL characters(const OUString &rChars) override;

    XMLMetaDocumentContext &mrMeta;
};

XMLDcDateContext::XMLDcDateContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta)
    : XMLImportContext(rImport), mrMeta(rMeta)
{
}

void XMLDcDateContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrMeta.m_aPropertyList.insert("dc:date", librevenge::RVNGString(sCharU8.getStr()));
}

/// Handler for <meta:generator>.
class XMLMetaGeneratorContext : public XMLImportContext
{
public:
    XMLMetaGeneratorContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta);

    void SAL_CALL characters(const OUString &rChars) override;

    XMLMetaDocumentContext &mrMeta;
};

XMLMetaGeneratorContext::XMLMetaGeneratorContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta)
    : XMLImportContext(rImport), mrMeta(rMeta)
{
}

void XMLMetaGeneratorContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrMeta.m_aPropertyList.insert("meta:generator", librevenge::RVNGString(sCharU8.getStr()));
}

/// Handler for <meta:initial-creator>.
class XMLMetaInitialCreatorContext : public XMLImportContext
{
public:
    XMLMetaInitialCreatorContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta);

    void SAL_CALL characters(const OUString &rChars) override;

    XMLMetaDocumentContext &mrMeta;
};

XMLMetaInitialCreatorContext::XMLMetaInitialCreatorContext(XMLImport &rImport, XMLMetaDocumentContext &rMeta)
    : XMLImportContext(rImport), mrMeta(rMeta)
{
}

void XMLMetaInitialCreatorContext::characters(const OUString &rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrMeta.m_aPropertyList.insert("meta:initial-creator", librevenge::RVNGString(sCharU8.getStr()));
}

XMLMetaDocumentContext::XMLMetaDocumentContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
    m_aPropertyList.insert("librevenge:cover-images", mrImport.GetCoverImages());
}

rtl::Reference<XMLImportContext> XMLMetaDocumentContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "dc:title")
        return new XMLDcTitleContext(mrImport, *this);
    if (rName == "dc:language")
        return new XMLDcLanguageContext(mrImport, *this);
    if (rName == "dc:date")
        return new XMLDcDateContext(mrImport, *this);
    if (rName == "meta:generator")
        return new XMLMetaGeneratorContext(mrImport, *this);
    if (rName == "meta:initial-creator")
        return new XMLMetaInitialCreatorContext(mrImport, *this);
    return nullptr;
}

void XMLMetaDocumentContext::endElement(const OUString &/*rName*/)
{
    mrImport.GetGenerator().setDocumentMetaData(m_aPropertyList);
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
