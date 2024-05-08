/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "filterdetect.hxx"

#include <svtools/htmltokn.h>
#include <tools/urlobj.hxx>
#include <tools/zcodec.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <memory>

constexpr OUString WRITER_TEXT_FILTER = u"Text"_ustr;
constexpr OUString CALC_TEXT_FILTER = u"Text - txt - csv (StarCalc)"_ustr;

constexpr OUStringLiteral WEB_HTML_FILTER = u"HTML";
constexpr OUStringLiteral WRITER_HTML_FILTER = u"HTML (StarWriter)";
constexpr OUStringLiteral CALC_HTML_FILTER = u"calc_HTML_WebQuery";

constexpr OUString WRITER_DOCSERVICE = u"com.sun.star.text.TextDocument"_ustr;
constexpr OUString CALC_DOCSERVICE = u"com.sun.star.sheet.SpreadsheetDocument"_ustr;

using namespace ::com::sun::star;
using utl::MediaDescriptor;

namespace {

bool IsHTMLStream( const uno::Reference<io::XInputStream>& xInStream )
{
    std::unique_ptr<SvStream> pInStream( utl::UcbStreamHelper::CreateStream( xInStream ) );
    if ( !pInStream || pInStream->GetError() )
        // No stream
        return false;

    // Read the stream header
    pInStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );
    const sal_uInt64 nUniPos = pInStream->Tell();
    const sal_uInt16 nSize = 4096;

    OString sHeader;
    if ( nUniPos == 3 || nUniPos == 0 ) // UTF-8 or non-Unicode
        sHeader = read_uInt8s_ToOString( *pInStream, nSize );
    else // UTF-16 (nUniPos = 2)
        sHeader = OUStringToOString( read_uInt16s_ToOUString( *pInStream, nSize ), RTL_TEXTENCODING_ASCII_US );

    // Now check whether the stream begins with a known HTML tag.
    enum DetectPhase { BeforeTag, TagOpened, InTagName };
    DetectPhase dp = BeforeTag;
    /// BeforeDeclaration -> ? -> DeclarationOpened -> > -> BeforeDeclaration.
    enum DeclarationPhase
    {
        BeforeDeclaration,
        DeclarationOpened
    };
    DeclarationPhase eDeclaration = BeforeDeclaration;

    const char* pHeader = sHeader.getStr();
    const int   nLength = sHeader.getLength();
    int i = 0, nStartOfTagIndex = 0;

    for ( i = 0; i < nLength; ++i, ++pHeader )
    {
        char c = *pHeader;
        if ((c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f')
            && eDeclaration == BeforeDeclaration)
        {
            if ( dp == TagOpened )
                return false; // Invalid: Should start with a tag name
            else if ( dp == InTagName )
                break; // End of tag name reached
        }
        else if ( c == '<' )
        {
            if ( dp == BeforeTag )
                dp = TagOpened;
            else
                return false; // Invalid: Nested '<'
        }
        else if ( c == '>' )
        {
            if ( dp == InTagName )
                break; // End of tag name reached
            else if (eDeclaration == DeclarationOpened)
            {
                dp = BeforeTag;
                eDeclaration = BeforeDeclaration;
            }
            else
                return false; // Invalid: Empty tag or before '<'
        }
        else if ( c == '!' )
        {
            if ( dp == TagOpened )
                return true; // "<!" - DOCTYPE or comments block
            else
                return false; // Invalid: '!' before '<' or inside tag name
        }
        else
        {
            if ( dp == BeforeTag )
                return false; // Invalid: Should start with a tag
            else if ( dp == TagOpened )
            {
                if (c == '?' && eDeclaration == BeforeDeclaration)
                    eDeclaration = DeclarationOpened;
                else if (eDeclaration == BeforeDeclaration)
                {
                    nStartOfTagIndex = i;
                    dp = InTagName;
                }
            }
        }
    }

    // The string following '<' has to be a known HTML token.
    OString aToken = sHeader.copy( nStartOfTagIndex, i - nStartOfTagIndex );
    return GetHTMLToken( OStringToOUString( aToken.toAsciiLowerCase(), RTL_TEXTENCODING_ASCII_US ) ) != HtmlTokenId::NONE;
}
}

PlainTextFilterDetect::PlainTextFilterDetect() {}

PlainTextFilterDetect::~PlainTextFilterDetect() {}

OUString SAL_CALL PlainTextFilterDetect::detect(uno::Sequence<beans::PropertyValue>& lDescriptor)
{
    MediaDescriptor aMediaDesc(lDescriptor);

    OUString aType = aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_TYPENAME, OUString() );
    OUString aDocService = aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_DOCUMENTSERVICE, OUString() );

    if ((aType == "generic_HTML") || (aType == "calc_HTML"))
    {
        uno::Reference<io::XInputStream> xInStream(aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM], uno::UNO_QUERY);
        if (!xInStream.is() || !IsHTMLStream(xInStream))
            return OUString();

        if ((aDocService == CALC_DOCSERVICE) || (aType == "calc_HTML"))
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= OUString(CALC_HTML_FILTER);
        else if (aDocService == WRITER_DOCSERVICE)
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= OUString(WRITER_HTML_FILTER);
        else
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= OUString(WEB_HTML_FILTER);
    }

    else if (aType == "generic_Text")
    {
        uno::Reference<io::XStream> xStream(aMediaDesc[MediaDescriptor::PROP_STREAM], uno::UNO_QUERY);
        uno::Reference<io::XInputStream> xInStream(aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM], uno::UNO_QUERY);
        if (xStream.is() || xInStream.is())
        {
            ZCodec aCodecGZ;
            std::unique_ptr<SvStream> pInStream;
            if (xStream.is())
                pInStream = utl::UcbStreamHelper::CreateStream(xStream);
            else
                pInStream = utl::UcbStreamHelper::CreateStream(xInStream);
            std::unique_ptr<SvMemoryStream> pDecompressedStream(new SvMemoryStream());
            if (aCodecGZ.AttemptDecompression(*pInStream, *pDecompressedStream))
            {
                uno::Reference<io::XStream> xStreamDecompressed(new utl::OStreamWrapper(std::move(pDecompressedStream)));
                aMediaDesc[MediaDescriptor::PROP_STREAM] <<= xStreamDecompressed;
                aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM] <<= xStreamDecompressed->getInputStream();
                OUString aURL = aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_URL, OUString() );
                sal_Int32 nIdx = aURL.lastIndexOf(".gz");
                if (nIdx != -1)
                    aMediaDesc[MediaDescriptor::PROP_URL] <<= aURL.copy(0, nIdx);
            }
        }
        // Get the file name extension.
        INetURLObject aParser(aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_URL, OUString() ) );
        OUString aExt = aParser.getExtension(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset);
        aExt = aExt.toAsciiLowerCase();
        OUString aName = aParser.getName().toAsciiLowerCase();

        // Decide which filter to use based on the document service first,
        // then on extension if that's not available.

        if (aDocService == CALC_DOCSERVICE)
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= CALC_TEXT_FILTER;
        else if (aDocService == WRITER_DOCSERVICE)
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= WRITER_TEXT_FILTER;
        else if (aExt == "csv" || aExt == "tsv" || aExt == "tab" || aExt == "xls" || aName.endsWith(".csv.gz"))
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= CALC_TEXT_FILTER;
        else
            aMediaDesc[MediaDescriptor::PROP_FILTERNAME] <<= WRITER_TEXT_FILTER;
    }

    else
        // Nothing to detect.
        return OUString();

    aMediaDesc >> lDescriptor;
    return aType;
}

// XInitialization

void SAL_CALL PlainTextFilterDetect::initialize(const uno::Sequence<uno::Any>& /*aArguments*/)
{
}

OUString PlainTextFilterDetect_getImplementationName()
{
    return u"com.sun.star.comp.filters.PlainTextFilterDetect"_ustr;
}

uno::Sequence<OUString> PlainTextFilterDetect_getSupportedServiceNames()
{
    return { u"com.sun.star.document.ExtendedTypeDetection"_ustr, u"com.sun.star.comp.filters.PlainTextFilterDetect"_ustr };
}

// XServiceInfo
OUString SAL_CALL PlainTextFilterDetect::getImplementationName()
{
    return PlainTextFilterDetect_getImplementationName();
}

sal_Bool SAL_CALL PlainTextFilterDetect::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL PlainTextFilterDetect::getSupportedServiceNames()
{
    return PlainTextFilterDetect_getSupportedServiceNames();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_filters_PlainTextFilterDetect_get_implementation(css::uno::XComponentContext* ,
                                                                   css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new PlainTextFilterDetect);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
