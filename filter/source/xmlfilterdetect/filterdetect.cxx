/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "filterdetect.hxx"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svl/inettype.hxx>
#include <memory>
#include <o3tl/string_view.hxx>

using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

namespace {

OUString supportedByType( std::u16string_view clipBoardFormat, std::u16string_view resultString, const OUString& checkType)
{
    OUString sTypeName;
    if ( o3tl::starts_with(clipBoardFormat, u"doctype:") )
    {
        std::u16string_view tryStr = clipBoardFormat.substr(8);
        if (resultString.find(tryStr) != std::u16string_view::npos)
        {
            sTypeName = checkType;
        }
    }
    return sTypeName;
}

bool IsMediaTypeXML( const OUString& mediaType )
{
    if (!mediaType.isEmpty())
    {
        OUString sType, sSubType;
        if (INetContentTypes::parse(mediaType, sType, sSubType)
            && sType == "application")
        {
            // RFC 3023: application/xml; don't detect text/xml
            if (sSubType == "xml")
                return true;
            // Registered media types: application/XXXX+xml
            if (sSubType.endsWith("+xml"))
                return true;
        }
    }
    return false;
}

}

OUString SAL_CALL FilterDetect::detect( css::uno::Sequence< css::beans::PropertyValue >& aArguments )
{
    OUString sUrl;
    css::uno::Reference< css::io::XInputStream > xInStream;

    sal_Int32 nLength = aArguments.getLength();
    sal_Int32 location=nLength;
    for (sal_Int32 i = 0 ; i < nLength; i++)
    {
        if (aArguments[i].Name == "TypeName")
        {
            location=i;
        }
        else if (aArguments[i].Name == "URL")
        {
            aArguments[i].Value >>= sUrl;
        }
        else if (aArguments[i].Name == "InputStream")
        {
            aArguments[i].Value >>= xInStream;
        }
    }
    try
    {
        if (!xInStream.is())
        {
            ::ucbhelper::Content aContent(
                sUrl, Reference< css::ucb::XCommandEnvironment >(),
                mxCtx);
            xInStream = aContent.openStream();
            if (!xInStream.is())
            {
                return {};
            }
        }

        std::unique_ptr< SvStream > pInStream( ::utl::UcbStreamHelper::CreateStream( xInStream ) );
        pInStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );
        sal_uInt64 const nUniPos = pInStream->Tell();

        const sal_uInt16 nSize = 4000;
        bool  bTryUtf16 = false;

        if ( nUniPos == 0 ) // No BOM detected, try to guess UTF-16 endianness
        {
            sal_uInt16 nHeader = 0;
            pInStream->ReadUInt16( nHeader );
            if ( nHeader == 0x003C )
                bTryUtf16 = true;
            else if ( nHeader == 0x3C00 )
            {
                bTryUtf16 = true;
                pInStream->SetEndian( pInStream->GetEndian() == SvStreamEndian::LITTLE ? SvStreamEndian::BIG : SvStreamEndian::LITTLE );
            }
            pInStream->Seek( STREAM_SEEK_TO_BEGIN );
        }

        OUString resultString;
        if ( nUniPos == 3 || ( nUniPos == 0 && !bTryUtf16 ) ) // UTF-8 or non-Unicode
        {
            OString const str(read_uInt8s_ToOString(*pInStream, nSize));
            resultString = OUString(str.getStr(), str.getLength(),
                RTL_TEXTENCODING_ASCII_US,
                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT|RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT|RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT);
        }
        else if ( nUniPos == 2 || bTryUtf16 ) // UTF-16
            resultString = read_uInt16s_ToOUString( *pInStream, nSize );

        if ( !resultString.startsWith( "<?xml" ) )
        {
            // Check the content type; XML declaration is optional in XML files according to XML 1.0 ch.2.8
            // (see https://www.w3.org/TR/2008/REC-xml-20081126/#sec-prolog-dtd)
            OUString sMediaType;
            try
            {
                ::ucbhelper::Content aContent(
                    sUrl, Reference< css::ucb::XCommandEnvironment >(),
                    mxCtx);
                aContent.getPropertyValue(u"MediaType"_ustr) >>= sMediaType;
                if (sMediaType.isEmpty())
                {
                    aContent.getPropertyValue(u"Content-Type"_ustr) >>= sMediaType;
                }
            }
            catch (...) {}

            if (!IsMediaTypeXML(sMediaType))
            {
                // This is not an XML stream.  It makes no sense to try to detect
                // a non-XML file type here.
                return OUString();
            }
        }

        // test typedetect code
        Reference <XNameAccess> xTypeCont(mxCtx->getServiceManager()->createInstanceWithContext(u"com.sun.star.document.TypeDetection"_ustr, mxCtx), UNO_QUERY);
        Sequence < OUString > myTypes= xTypeCont->getElementNames();
        nLength = myTypes.getLength();

        for (const OUString& myType : myTypes)
        {
            Sequence<PropertyValue> lProps;
            xTypeCont->getByName(myType) >>= lProps;
            OUString detectService, clipboardFormat;
            for (const PropertyValue& prop : lProps)
            {
                if (prop.Name == "DetectService")
                    prop.Value >>= detectService;
                else if (prop.Name == "ClipboardFormat")
                    prop.Value >>= clipboardFormat;
            }
            if (!clipboardFormat.isEmpty() && detectService == getImplementationName())
            {
                OUString sTypeName = supportedByType(clipboardFormat, resultString, myType);
                if (!sTypeName.isEmpty())
                {
                    if (location == aArguments.getLength())
                    {
                        aArguments.realloc(aArguments.getLength() + 1);
                        aArguments.getArray()[location].Name = "TypeName";
                    }
                    aArguments.getArray()[location].Value <<= sTypeName;
                    return sTypeName;
                }
            }
        }
    }
    catch (const Exception &)
    {
        TOOLS_WARN_EXCEPTION("filter.xmlfd", "An Exception occurred while opening File stream");
    }

    return {};
}

// XInitialization
void SAL_CALL FilterDetect::initialize( const Sequence< Any >& /*aArguments*/ )
{
}

// XServiceInfo
OUString SAL_CALL FilterDetect::getImplementationName(  )
{
    return u"com.sun.star.comp.filters.XMLFilterDetect"_ustr;
}

sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames(  )
{
    return { u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_XMLFilterDetect_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new FilterDetect(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
