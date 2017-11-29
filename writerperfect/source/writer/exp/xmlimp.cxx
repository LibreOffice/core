/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlimp.hxx"

#include <initializer_list>
#include <unordered_map>

#include <rtl/uri.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

#include "xmlfmt.hxx"
#include "xmlictxt.hxx"
#include "xmlmetai.hxx"
#include "xmltext.hxx"

using namespace com::sun::star;

namespace writerperfect
{
namespace exp
{

namespace
{
/// Looks up mime type for a given image extension.
OUString GetMimeType(const OUString &rExtension)
{
    static const std::unordered_map<OUString, OUString> vMimeTypes =
    {
        {"gif", "image/gif"},
        {"jpg", "image/jpeg"},
        {"png", "image/png"},
        {"svg", "image/svg+xml"},
    };

    auto it = vMimeTypes.find(rExtension);
    return it == vMimeTypes.end() ? OUString() : it->second;
}

/// Picks up a cover image from the base directory.
OUString FindCoverImage(const OUString &rDocumentBaseURL, OUString &rMimeType)
{
    OUString aRet;

    if (rDocumentBaseURL.isEmpty())
        return aRet;

    INetURLObject aDocumentBaseURL(rDocumentBaseURL);

    static const std::initializer_list<OUStringLiteral> vExtensions =
    {
        "gif",
        "jpg",
        "png",
        "svg"
    };

    for (const auto &rExtension : vExtensions)
    {
        try
        {
            aRet = rtl::Uri::convertRelToAbs(rDocumentBaseURL, aDocumentBaseURL.GetBase() + ".cover-image." + rExtension);
        }
        catch (const rtl::MalformedUriException &rException)
        {
            SAL_WARN("writerfilter", "FindCoverImage: convertRelToAbs() failed:" << rException.getMessage());
        }

        if (!aRet.isEmpty())
        {
            SvFileStream aStream(aRet, StreamMode::READ);
            if (aStream.IsOpen())
            {
                rMimeType = GetMimeType(rExtension);
                // File exists.
                return aRet;
            }
            else
                aRet.clear();
        }
    }

    return aRet;
}
}

/// Handler for <office:body>.
class XMLBodyContext : public XMLImportContext
{
public:
    XMLBodyContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/) override;
};

XMLBodyContext::XMLBodyContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLBodyContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:text")
        return new XMLBodyContentContext(mrImport);
    return nullptr;
}

/// Handler for <office:document>.
class XMLOfficeDocContext : public XMLImportContext
{
public:
    XMLOfficeDocContext(XMLImport &rImport);

    rtl::Reference<XMLImportContext> CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/) override;
};

XMLOfficeDocContext::XMLOfficeDocContext(XMLImport &rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLOfficeDocContext::CreateChildContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:body")
        return new XMLBodyContext(mrImport);
    else if (rName == "office:meta")
        return new XMLMetaDocumentContext(mrImport);
    else if (rName == "office:automatic-styles")
        return new XMLStylesContext(mrImport, /*bAutomatic=*/true);
    else if (rName == "office:styles")
        return new XMLStylesContext(mrImport, /*bAutomatic=*/false);
    else if (rName == "office:font-face-decls")
        return new XMLFontFaceDeclsContext(mrImport);
    return nullptr;
}

XMLImport::XMLImport(librevenge::RVNGTextInterface &rGenerator, const OUString &rURL, const uno::Sequence<beans::PropertyValue> &/*rDescriptor*/)
    : mrGenerator(rGenerator)
{
    OUString aMimeType;
    OUString aCoverImage = FindCoverImage(rURL, aMimeType);
    if (!aCoverImage.isEmpty())
    {
        librevenge::RVNGBinaryData aBinaryData;
        SvFileStream aStream(aCoverImage, StreamMode::READ);
        SvMemoryStream aMemoryStream;
        aMemoryStream.WriteStream(aStream);
        aBinaryData.append(static_cast<const unsigned char *>(aMemoryStream.GetBuffer()), aMemoryStream.GetSize());
        librevenge::RVNGPropertyList aCoverImageProperties;
        aCoverImageProperties.insert("office:binary-data", aBinaryData);
        aCoverImageProperties.insert("librevenge:mime-type", aMimeType.toUtf8().getStr());
        maCoverImages.append(aCoverImageProperties);
    }
}

const librevenge::RVNGPropertyListVector &XMLImport::GetCoverImages()
{
    return maCoverImages;
}

rtl::Reference<XMLImportContext> XMLImport::CreateContext(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &/*xAttribs*/)
{
    if (rName == "office:document")
        return new XMLOfficeDocContext(*this);
    return nullptr;
}

librevenge::RVNGTextInterface &XMLImport::GetGenerator() const
{
    return mrGenerator;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticTextStyles()
{
    return maAutomaticTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticParagraphStyles()
{
    return maAutomaticParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticCellStyles()
{
    return maAutomaticCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticColumnStyles()
{
    return maAutomaticColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticRowStyles()
{
    return maAutomaticRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticTableStyles()
{
    return maAutomaticTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetAutomaticGraphicStyles()
{
    return maAutomaticGraphicStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetTextStyles()
{
    return maTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetParagraphStyles()
{
    return maParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetCellStyles()
{
    return maCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetColumnStyles()
{
    return maColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetRowStyles()
{
    return maRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetTableStyles()
{
    return maTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList> &XMLImport::GetGraphicStyles()
{
    return maGraphicStyles;
}

void XMLImport::startDocument()
{
    mrGenerator.startDocument(librevenge::RVNGPropertyList());
}

void XMLImport::endDocument()
{
    mrGenerator.endDocument();
}

void XMLImport::startElement(const OUString &rName, const css::uno::Reference<css::xml::sax::XAttributeList> &xAttribs)
{
    rtl::Reference<XMLImportContext> xContext;
    if (!maContexts.empty())
    {
        if (maContexts.top().is())
            xContext = maContexts.top()->CreateChildContext(rName, xAttribs);
    }
    else
        xContext = CreateContext(rName, xAttribs);

    if (xContext.is())
        xContext->startElement(rName, xAttribs);

    maContexts.push(xContext);
}

void XMLImport::endElement(const OUString &rName)
{
    if (maContexts.empty())
        return;

    if (maContexts.top().is())
        maContexts.top()->endElement(rName);

    maContexts.pop();
}

void XMLImport::characters(const OUString &rChars)
{
    if (maContexts.top().is())
        maContexts.top()->characters(rChars);
}

void XMLImport::ignorableWhitespace(const OUString &/*rWhitespaces*/)
{
}

void XMLImport::processingInstruction(const OUString &/*rTarget*/, const OUString &/*rData*/)
{
}

void XMLImport::setDocumentLocator(const css::uno::Reference<css::xml::sax::XLocator> &/*xLocator*/)
{
}

} // namespace exp
} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
