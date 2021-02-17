/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "xmlimp.hxx"

#include <string_view>

#include <initializer_list>
#include <unordered_map>

#include <com/sun/star/svg/XSVGWriter.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <comphelper/propertyvalue.hxx>
#include <rtl/uri.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/streamwrap.hxx>
#include <tools/diagnose_ex.h>

#include "xmlfmt.hxx"
#include "xmlictxt.hxx"
#include "xmlmetai.hxx"
#include "xmltext.hxx"

using namespace com::sun::star;

namespace writerperfect::exp
{
namespace
{
/// Looks up mime type for a given image extension.
OUString GetMimeType(const OUString& rExtension)
{
    static const std::unordered_map<OUString, OUString> vMimeTypes = {
        { "gif", "image/gif" },
        { "jpg", "image/jpeg" },
        { "png", "image/png" },
        { "svg", "image/svg+xml" },
    };

    auto it = vMimeTypes.find(rExtension);
    return it == vMimeTypes.end() ? OUString() : it->second;
}

/// Determines the base directory for cover images, XMP metadata, popup images.
OUString FindMediaDir(const OUString& rDocumentBaseURL,
                      const uno::Sequence<beans::PropertyValue>& rFilterData)
{
    OUString aMediaDir;

    // See if filter data contains a media directory explicitly.
    auto pProp = std::find_if(
        rFilterData.begin(), rFilterData.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "RVNGMediaDir"; });
    if (pProp != rFilterData.end())
        pProp->Value >>= aMediaDir;

    if (!aMediaDir.isEmpty())
        return aMediaDir + "/";

    // Not set explicitly, try to pick it up from the base directory.
    INetURLObject aURL(rDocumentBaseURL);
    try
    {
        aMediaDir = rtl::Uri::convertRelToAbs(rDocumentBaseURL, aURL.GetBase()) + "/";
    }
    catch (const rtl::MalformedUriException&)
    {
        DBG_UNHANDLED_EXCEPTION("writerperfect");
    }
    return aMediaDir;
}

/// Picks up a cover image from the base directory.
OUString FindCoverImage(const OUString& rDocumentBaseURL, OUString& rMimeType,
                        const uno::Sequence<beans::PropertyValue>& rFilterData)
{
    OUString aRet;

    // See if filter data contains a cover image explicitly.
    auto pProp = std::find_if(
        rFilterData.begin(), rFilterData.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "RVNGCoverImage"; });
    if (pProp != rFilterData.end())
        pProp->Value >>= aRet;

    if (!aRet.isEmpty())
    {
        INetURLObject aRetURL(aRet);
        rMimeType = GetMimeType(aRetURL.GetFileExtension());
        return aRet;
    }

    // Not set explicitly, try to pick it up from the base directory.
    if (rDocumentBaseURL.isEmpty())
        return aRet;

    static const std::initializer_list<std::u16string_view> vExtensions
        = { u"gif", u"jpg", u"png", u"svg" };

    OUString aMediaDir = FindMediaDir(rDocumentBaseURL, rFilterData);
    for (const auto& rExtension : vExtensions)
    {
        aRet = aMediaDir + "cover." + rExtension;
        if (!aRet.isEmpty())
        {
            SvFileStream aStream(aRet, StreamMode::READ);
            if (aStream.IsOpen())
            {
                rMimeType = GetMimeType(OUString(rExtension));
                // File exists.
                return aRet;
            }

            aRet.clear();
        }
    }

    return aRet;
}

/// Picks up XMP metadata from the base directory.
void FindXMPMetadata(const uno::Reference<uno::XComponentContext>& xContext,
                     const OUString& rDocumentBaseURL,
                     const uno::Sequence<beans::PropertyValue>& rFilterData,
                     librevenge::RVNGPropertyList& rMetaData)
{
    // See if filter data contains metadata explicitly.
    OUString aValue;
    for (const auto& rProp : rFilterData)
    {
        if (rProp.Name == "RVNGIdentifier")
        {
            rProp.Value >>= aValue;
            if (!aValue.isEmpty())
                rMetaData.insert("dc:identifier", aValue.toUtf8().getStr());
        }
        else if (rProp.Name == "RVNGTitle")
        {
            rProp.Value >>= aValue;
            if (!aValue.isEmpty())
                rMetaData.insert("dc:title", aValue.toUtf8().getStr());
        }
        else if (rProp.Name == "RVNGInitialCreator")
        {
            rProp.Value >>= aValue;
            if (!aValue.isEmpty())
                rMetaData.insert("meta:initial-creator", aValue.toUtf8().getStr());
        }
        else if (rProp.Name == "RVNGLanguage")
        {
            rProp.Value >>= aValue;
            if (!aValue.isEmpty())
                rMetaData.insert("dc:language", aValue.toUtf8().getStr());
        }
        else if (rProp.Name == "RVNGDate")
        {
            rProp.Value >>= aValue;
            if (!aValue.isEmpty())
                rMetaData.insert("dc:date", aValue.toUtf8().getStr());
        }
    }

    // If not set explicitly, try to pick it up from the base directory.
    if (rDocumentBaseURL.isEmpty())
        return;

    OUString aMediaDir = FindMediaDir(rDocumentBaseURL, rFilterData);
    OUString aURL = aMediaDir + "metadata.xmp";
    SvFileStream aStream(aURL, StreamMode::READ);
    if (!aStream.IsOpen())
        return;

    xml::sax::InputSource aInputSource;
    uno::Reference<io::XInputStream> xStream(new utl::OStreamWrapper(aStream));
    aInputSource.aInputStream = xStream;
    uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(xContext);
    rtl::Reference<XMPParser> xXMP(new XMPParser(rMetaData));
    xParser->setDocumentHandler(xXMP);
    try
    {
        xParser->parseStream(aInputSource);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("writerperfect", "parseStream() failed");
        return;
    }
}

/// Handler for <office:body>.
class XMLBodyContext : public XMLImportContext
{
public:
    XMLBodyContext(XMLImport& rImport);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/) override;
};
}

XMLBodyContext::XMLBodyContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext>
XMLBodyContext::CreateChildContext(const OUString& rName,
                                   const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "office:text")
        return new XMLBodyContentContext(GetImport());
    return nullptr;
}

namespace
{
/// Handler for <office:document>.
class XMLOfficeDocContext : public XMLImportContext
{
public:
    XMLOfficeDocContext(XMLImport& rImport);

    rtl::Reference<XMLImportContext>
    CreateChildContext(const OUString& rName,
                       const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/) override;

    // Handles metafile for a single page.
    void HandleFixedLayoutPage(const FixedLayoutPage& rPage, bool bFirst);
};
}

XMLOfficeDocContext::XMLOfficeDocContext(XMLImport& rImport)
    : XMLImportContext(rImport)
{
}

rtl::Reference<XMLImportContext> XMLOfficeDocContext::CreateChildContext(
    const OUString& rName, const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == "office:meta")
        return new XMLMetaDocumentContext(GetImport());
    if (rName == "office:automatic-styles")
        return new XMLStylesContext(GetImport(), XMLStylesContext::StyleType_AUTOMATIC);
    if (rName == "office:styles")
        return new XMLStylesContext(GetImport(), XMLStylesContext::StyleType_NONE);
    if (rName == "office:master-styles")
        return new XMLStylesContext(GetImport(), XMLStylesContext::StyleType_NONE);
    if (rName == "office:font-face-decls")
        return new XMLFontFaceDeclsContext(GetImport());
    if (rName == "office:body")
    {
        if (GetImport().GetPageMetafiles().empty())
            return new XMLBodyContext(GetImport());

        // Ignore text from doc model in the fixed layout case, instead
        // insert the page metafiles.
        bool bFirst = true;
        for (const auto& rPage : GetImport().GetPageMetafiles())
        {
            HandleFixedLayoutPage(rPage, bFirst);
            if (bFirst)
                bFirst = false;
        }
    }
    return nullptr;
}

void XMLOfficeDocContext::HandleFixedLayoutPage(const FixedLayoutPage& rPage, bool bFirst)
{
    uno::Reference<uno::XComponentContext> xCtx = GetImport().GetComponentContext();
    uno::Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(xCtx);
    if (!xSaxWriter.is())
        return;

    uno::Sequence<uno::Any> aArguments = { uno::makeAny<uno::Sequence<beans::PropertyValue>>(
        { comphelper::makePropertyValue("DTDString", false) }) };
    uno::Reference<svg::XSVGWriter> xSVGWriter(
        xCtx->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.svg.SVGWriter", aArguments, xCtx),
        uno::UNO_QUERY);
    if (!xSVGWriter.is())
        return;

    SvMemoryStream aMemoryStream;
    xSaxWriter->setOutputStream(new utl::OStreamWrapper(aMemoryStream));

    xSVGWriter->write(xSaxWriter, rPage.aMetafile);

    // Have all the info, invoke the generator.
    librevenge::RVNGPropertyList aPageProperties;
    // Pixel -> inch.
    double fWidth = rPage.aCssPixels.getWidth();
    fWidth /= 96;
    aPageProperties.insert("fo:page-width", fWidth);
    double fHeight = rPage.aCssPixels.getHeight();
    fHeight /= 96;
    aPageProperties.insert("fo:page-height", fHeight);

    if (!rPage.aChapterNames.empty())
    {
        // Name of chapters starting on this page.
        librevenge::RVNGPropertyListVector aChapterNames;
        for (const auto& rName : rPage.aChapterNames)
        {
            librevenge::RVNGPropertyList aChapter;
            aChapter.insert("librevenge:name", rName.toUtf8().getStr());
            aChapterNames.append(aChapter);
        }
        aPageProperties.insert("librevenge:chapter-names", aChapterNames);
    }

    GetImport().GetGenerator().openPageSpan(aPageProperties);
    librevenge::RVNGPropertyList aParagraphProperties;
    if (!bFirst)
        // All pages except the first one needs a page break before the page
        // metafile.
        aParagraphProperties.insert("fo:break-before", "page");
    GetImport().GetGenerator().openParagraph(aParagraphProperties);
    librevenge::RVNGPropertyList aImageProperties;
    aImageProperties.insert("librevenge:mime-type", "image/svg+xml");
    librevenge::RVNGBinaryData aBinaryData;
    aBinaryData.append(static_cast<const unsigned char*>(aMemoryStream.GetData()),
                       aMemoryStream.GetSize());
    aImageProperties.insert("office:binary-data", aBinaryData);
    GetImport().GetGenerator().insertBinaryObject(aImageProperties);
    GetImport().GetGenerator().closeParagraph();
    GetImport().GetGenerator().closePageSpan();
}

XMLImport::XMLImport(const uno::Reference<uno::XComponentContext>& xContext,
                     librevenge::RVNGTextInterface& rGenerator, const OUString& rURL,
                     const uno::Sequence<beans::PropertyValue>& rDescriptor,
                     const std::vector<FixedLayoutPage>& rPageMetafiles)
    : mrGenerator(rGenerator)
    , mxContext(xContext)
    , mbIsInPageSpan(false)
    , mrPageMetafiles(rPageMetafiles)
{
    uno::Sequence<beans::PropertyValue> aFilterData;
    auto pDescriptor = std::find_if(
        rDescriptor.begin(), rDescriptor.end(),
        [](const beans::PropertyValue& rProp) { return rProp.Name == "FilterData"; });
    if (pDescriptor != rDescriptor.end())
        pDescriptor->Value >>= aFilterData;

    maMediaDir = FindMediaDir(rURL, aFilterData);

    OUString aMimeType;
    OUString aCoverImage = FindCoverImage(rURL, aMimeType, aFilterData);
    if (!aCoverImage.isEmpty())
    {
        librevenge::RVNGBinaryData aBinaryData;
        SvFileStream aStream(aCoverImage, StreamMode::READ);
        SvMemoryStream aMemoryStream;
        aMemoryStream.WriteStream(aStream);
        aBinaryData.append(static_cast<const unsigned char*>(aMemoryStream.GetData()),
                           aMemoryStream.GetSize());
        librevenge::RVNGPropertyList aCoverImageProperties;
        aCoverImageProperties.insert("office:binary-data", aBinaryData);
        aCoverImageProperties.insert("librevenge:mime-type", aMimeType.toUtf8().getStr());
        maCoverImages.append(aCoverImageProperties);
    }

    FindXMPMetadata(mxContext, rURL, aFilterData, maMetaData);

    mxUriReferenceFactory = uri::UriReferenceFactory::create(mxContext);
}

const librevenge::RVNGPropertyListVector& XMLImport::GetCoverImages() const
{
    return maCoverImages;
}

const librevenge::RVNGPropertyList& XMLImport::GetMetaData() const { return maMetaData; }

namespace
{
/// Finds out if a file URL exists.
bool FileURLExists(const OUString& rURL)
{
    SvFileStream aStream(rURL, StreamMode::READ);
    return aStream.IsOpen();
}
}

PopupState XMLImport::FillPopupData(const OUString& rURL, librevenge::RVNGPropertyList& rPropList)
{
    uno::Reference<uri::XUriReference> xUriRef;
    try
    {
        xUriRef = mxUriReferenceFactory->parse(rURL);
    }
    catch (const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("writerperfect", "XUriReference::parse() failed");
    }
    bool bAbsolute = true;
    if (xUriRef.is())
        bAbsolute = xUriRef->isAbsolute();
    if (bAbsolute)
        return PopupState::NotConsumed;

    // Default case: relative URL, popup data was in the same directory as the
    // document at insertion time.
    OUString aAbs = maMediaDir + rURL;
    if (!FileURLExists(aAbs))
        // Fallback case: relative URL, popup data was in the default media
        // directory at insertion time.
        aAbs = maMediaDir + "../" + rURL;

    if (!FileURLExists(aAbs))
        // Relative link, but points to non-existing file: don't emit that to
        // librevenge, since it will be invalid anyway.
        return PopupState::Ignore;

    SvFileStream aStream(aAbs, StreamMode::READ);
    librevenge::RVNGBinaryData aBinaryData;
    SvMemoryStream aMemoryStream;
    aMemoryStream.WriteStream(aStream);
    aBinaryData.append(static_cast<const unsigned char*>(aMemoryStream.GetData()),
                       aMemoryStream.GetSize());
    rPropList.insert("office:binary-data", aBinaryData);

    INetURLObject aAbsURL(aAbs);
    OUString aMimeType = GetMimeType(aAbsURL.GetFileExtension());
    rPropList.insert("librevenge:mime-type", aMimeType.toUtf8().getStr());

    return PopupState::Consumed;
}

const std::vector<FixedLayoutPage>& XMLImport::GetPageMetafiles() const { return mrPageMetafiles; }

const uno::Reference<uno::XComponentContext>& XMLImport::GetComponentContext() const
{
    return mxContext;
}

rtl::Reference<XMLImportContext>
XMLImport::CreateContext(std::u16string_view rName,
                         const uno::Reference<xml::sax::XAttributeList>& /*xAttribs*/)
{
    if (rName == u"office:document")
        return new XMLOfficeDocContext(*this);
    return nullptr;
}

librevenge::RVNGTextInterface& XMLImport::GetGenerator() const { return mrGenerator; }

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticTextStyles()
{
    return maAutomaticTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticParagraphStyles()
{
    return maAutomaticParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticCellStyles()
{
    return maAutomaticCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticColumnStyles()
{
    return maAutomaticColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticRowStyles()
{
    return maAutomaticRowStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticTableStyles()
{
    return maAutomaticTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetAutomaticGraphicStyles()
{
    return maAutomaticGraphicStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetTextStyles()
{
    return maTextStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetParagraphStyles()
{
    return maParagraphStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetCellStyles()
{
    return maCellStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetColumnStyles()
{
    return maColumnStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetRowStyles() { return maRowStyles; }

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetTableStyles()
{
    return maTableStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetGraphicStyles()
{
    return maGraphicStyles;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetPageLayouts()
{
    return maPageLayouts;
}

std::map<OUString, librevenge::RVNGPropertyList>& XMLImport::GetMasterStyles()
{
    return maMasterStyles;
}

void XMLImport::startDocument() { mrGenerator.startDocument(librevenge::RVNGPropertyList()); }

void XMLImport::endDocument() { mrGenerator.endDocument(); }

void XMLImport::startElement(const OUString& rName,
                             const uno::Reference<xml::sax::XAttributeList>& xAttribs)
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

void XMLImport::endElement(const OUString& rName)
{
    if (maContexts.empty())
        return;

    if (maContexts.top().is())
        maContexts.top()->endElement(rName);

    maContexts.pop();
}

void XMLImport::characters(const OUString& rChars)
{
    if (maContexts.top().is())
        maContexts.top()->characters(rChars);
}

void XMLImport::ignorableWhitespace(const OUString& /*rWhitespaces*/) {}

void XMLImport::processingInstruction(const OUString& /*rTarget*/, const OUString& /*rData*/) {}

void XMLImport::setDocumentLocator(const uno::Reference<xml::sax::XLocator>& /*xLocator*/) {}

void XMLImport::HandlePageSpan(const librevenge::RVNGPropertyList& rPropertyList)
{
    OUString sMasterPageName;
    OUString sLayoutName;

    if (rPropertyList["style:master-page-name"])
        sMasterPageName = OStringToOUString(
            rPropertyList["style:master-page-name"]->getStr().cstr(), RTL_TEXTENCODING_UTF8);
    else if (!GetIsInPageSpan())
        sMasterPageName = "Standard";

    if (sMasterPageName.getLength())
    {
        librevenge::RVNGPropertyList& rMasterPage = GetMasterStyles()[sMasterPageName];
        if (rMasterPage["style:page-layout-name"])
        {
            sLayoutName = OStringToOUString(rMasterPage["style:page-layout-name"]->getStr().cstr(),
                                            RTL_TEXTENCODING_UTF8);
        }
    }

    if (sLayoutName.getLength())
    {
        librevenge::RVNGPropertyList& rPageLayout = GetPageLayouts()[sLayoutName];

        if (GetIsInPageSpan())
            GetGenerator().closePageSpan();

        GetGenerator().openPageSpan(rPageLayout);
        mbIsInPageSpan = true;
    }
}

} // namespace writerperfect

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
