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
namespace
{
/// Handler for <dc:title>.
class XMLDcTitleContext : public XMLImportContext
{
public:
    XMLDcTitleContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta);

    void SAL_CALL characters(const OUString& rChars) override;

private:
    XMLMetaDocumentContext& mrMeta;
};
}

XMLDcTitleContext::XMLDcTitleContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta)
    : XMLImportContext(rImport)
    , mrMeta(rMeta)
{
}

void XMLDcTitleContext::characters(const OUString& rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    if (!mrMeta.GetPropertyList()["dc:title"])
        mrMeta.GetPropertyList().insert("dc:title", librevenge::RVNGString(sCharU8.getStr()));
}

namespace
{
/// Handler for <dc:language>.
class XMLDcLanguageContext : public XMLImportContext
{
public:
    XMLDcLanguageContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta);

    void SAL_CALL characters(const OUString& rChars) override;

private:
    XMLMetaDocumentContext& mrMeta;
};
}

XMLDcLanguageContext::XMLDcLanguageContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta)
    : XMLImportContext(rImport)
    , mrMeta(rMeta)
{
}

void XMLDcLanguageContext::characters(const OUString& rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    if (!mrMeta.GetPropertyList()["dc:language"])
        mrMeta.GetPropertyList().insert("dc:language", librevenge::RVNGString(sCharU8.getStr()));
}

namespace
{
/// Handler for <dc:date>.
class XMLDcDateContext : public XMLImportContext
{
public:
    XMLDcDateContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta);

    void SAL_CALL characters(const OUString& rChars) override;

private:
    XMLMetaDocumentContext& mrMeta;
};
}

XMLDcDateContext::XMLDcDateContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta)
    : XMLImportContext(rImport)
    , mrMeta(rMeta)
{
}

void XMLDcDateContext::characters(const OUString& rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    if (!mrMeta.GetPropertyList()["dc:date"])
        mrMeta.GetPropertyList().insert("dc:date", librevenge::RVNGString(sCharU8.getStr()));
}

namespace
{
/// Handler for <meta:generator>.
class XMLMetaGeneratorContext : public XMLImportContext
{
public:
    XMLMetaGeneratorContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta);

    void SAL_CALL characters(const OUString& rChars) override;

private:
    XMLMetaDocumentContext& mrMeta;
};
}

XMLMetaGeneratorContext::XMLMetaGeneratorContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta)
    : XMLImportContext(rImport)
    , mrMeta(rMeta)
{
}

void XMLMetaGeneratorContext::characters(const OUString& rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    mrMeta.GetPropertyList().insert("meta:generator", librevenge::RVNGString(sCharU8.getStr()));
}

namespace
{
/// Handler for <meta:initial-creator>.
class XMLMetaInitialCreatorContext : public XMLImportContext
{
public:
    XMLMetaInitialCreatorContext(XMLImport& rImport, XMLMetaDocumentContext& rMeta);

    void SAL_CALL characters(const OUString& rChars) override;

private:
    XMLMetaDocumentContext& mrMeta;
};
}

XMLMetaInitialCreatorContext::XMLMetaInitialCreatorContext(XMLImport& rImport,
                                                           XMLMetaDocumentContext& rMeta)
    : XMLImportContext(rImport)
    , mrMeta(rMeta)
{
}

void XMLMetaInitialCreatorContext::characters(const OUString& rChars)
{
    OString sCharU8 = OUStringToOString(rChars, RTL_TEXTENCODING_UTF8);
    if (!mrMeta.GetPropertyList()["meta:initial-creator"])
        mrMeta.GetPropertyList().insert("meta:initial-creator",
                                        librevenge::RVNGString(sCharU8.getStr()));
}

XMLMetaDocumentContext::XMLMetaDocumentContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
    librevenge::RVNGPropertyList::Iter it(GetImport().GetMetaData());
    for (it.rewind(); it.next();)
        m_aPropertyList.insert(it.key(), it()->clone());
    m_aPropertyList.insert("librevenge:cover-images", GetImport().GetCoverImages());
}

rtl::Reference<XMLImportContext> XMLMetaDocumentContext::CreateChildContext(
    const OUString& rName, const css::uno::Reference<css::xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "dc:title")
        return new XMLDcTitleContext(GetImport(), *this);
    if (rName == "dc:language")
        return new XMLDcLanguageContext(GetImport(), *this);
    if (rName == "dc:date")
        return new XMLDcDateContext(GetImport(), *this);
    if (rName == "meta:generator")
        return new XMLMetaGeneratorContext(GetImport(), *this);
    if (rName == "meta:initial-creator")
        return new XMLMetaInitialCreatorContext(GetImport(), *this);
    return nullptr;
}

void XMLMetaDocumentContext::endElement(const OUString& /*rName*/)
{
    GetImport().GetGenerator().setDocumentMetaData(m_aPropertyList);
}

XMPParser::XMPParser(librevenge::RVNGPropertyList& rMetaData)
    : mrMetaData(rMetaData)
{
}

XMPParser::~XMPParser() = default;

void XMPParser::startDocument() {}

void XMPParser::endDocument()
{
    if (!mrMetaData["dc:identifier"] && !m_aIdentifier.isEmpty())
        mrMetaData.insert("dc:identifier", m_aIdentifier.toUtf8().getStr());
    if (!mrMetaData["dc:title"] && !m_aTitle.isEmpty())
        mrMetaData.insert("dc:title", m_aTitle.toUtf8().getStr());
    if (!mrMetaData["meta:initial-creator"] && !m_aCreator.isEmpty())
        mrMetaData.insert("meta:initial-creator", m_aCreator.toUtf8().getStr());
    if (!mrMetaData["dc:language"] && !m_aLanguage.isEmpty())
        mrMetaData.insert("dc:language", m_aLanguage.toUtf8().getStr());
    if (!mrMetaData["dc:date"] && !m_aDate.isEmpty())
        mrMetaData.insert("dc:date", m_aDate.toUtf8().getStr());
}

void XMPParser::startElement(const OUString& rName,
                             const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "dc:identifier")
        m_bInIdentifier = true;
    else if (rName == "dc:title")
        m_bInTitle = true;
    else if (rName == "dc:creator")
        m_bInCreator = true;
    else if (rName == "dc:language")
        m_bInLanguage = true;
    else if (rName == "dc:date")
        m_bInDate = true;
    else if (rName == "rdf:li")
    {
        if (m_bInTitle)
            m_bInTitleItem = true;
        else if (m_bInCreator)
            m_bInCreatorItem = true;
        else if (m_bInLanguage)
            m_bInLanguageItem = true;
        else if (m_bInDate)
            m_bInDateItem = true;
    }
}

void XMPParser::endElement(const OUString& rName)
{
    if (rName == "dc:identifier")
        m_bInIdentifier = false;
    else if (rName == "dc:title")
        m_bInTitle = false;
    else if (rName == "dc:creator")
        m_bInCreator = false;
    else if (rName == "dc:language")
        m_bInLanguage = false;
    else if (rName == "dc:date")
        m_bInDate = false;
    else if (rName == "rdf:li")
    {
        if (m_bInTitle)
            m_bInTitleItem = false;
        else if (m_bInCreator)
            m_bInCreatorItem = false;
        else if (m_bInLanguage)
            m_bInLanguageItem = false;
        else if (m_bInDate)
            m_bInDateItem = false;
    }
}

void XMPParser::characters(const OUString& rChars)
{
    if (m_bInIdentifier)
        m_aIdentifier += rChars;
    else if (m_bInTitleItem)
        m_aTitle += rChars;
    else if (m_bInCreatorItem)
        m_aCreator += rChars;
    else if (m_bInLanguageItem)
        m_aLanguage += rChars;
    else if (m_bInDateItem)
        m_aDate += rChars;
}

void XMPParser::ignorableWhitespace(const OUString& /*rWhitespace*/) {}

void XMPParser::processingInstruction(const OUString& /*rTarget*/, const OUString& /*rData*/) {}

void XMPParser::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& /*xLocator*/) {}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
