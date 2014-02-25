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
#include <ucbhelper/content.hxx>
#include <unotools/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <boost/scoped_ptr.hpp>

using com::sun::star::io::XInputStream;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::ucb::XCommandEnvironment;

using namespace com::sun::star;
using namespace com::sun::star::beans;

namespace {

enum DetectPhase {
    BeforeTag,
    TagOpened,
    InTagName
};

bool isHTMLStream(const OString& aStreamHeader)
{
    const char* pHeader = aStreamHeader.getStr();
    const int   nLength = aStreamHeader.getLength();
    int nStartOfTagIndex = 0;
    int i = 0;

    DetectPhase dp = BeforeTag;

    for ( i = 0; i < nLength; ++i, ++pHeader )
    {
        char c = *pHeader;
        if ( c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' )
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
                nStartOfTagIndex = i;
                dp = InTagName;
            }
        }
    }

    // The string following '<' has to be a known HTML token.
    if ( GetHTMLToken( OStringToOUString( aStreamHeader.copy( nStartOfTagIndex, i - nStartOfTagIndex ),
                                                RTL_TEXTENCODING_ASCII_US ) ) != 0 )
        return true;

    return false;
}

}

OUString SAL_CALL HtmlFilterDetect::detect(Sequence<PropertyValue>& lDescriptor)
    throw (RuntimeException, std::exception)
{
    OUString sUrl;
    OUString sDocService;
    OString  resultString;
    Reference<XInputStream> xInStream;

    const PropertyValue *pValue = lDescriptor.getConstArray();
    sal_Int32 nLength  = lDescriptor.getLength();
    sal_Int32 location = nLength;

    for ( sal_Int32 i = 0; i < nLength; ++i )
    {
        if ( pValue[i].Name == utl::MediaDescriptor::PROP_URL() )
            pValue[i].Value >>= sUrl;
        else if ( pValue[i].Name == utl::MediaDescriptor::PROP_INPUTSTREAM() )
            pValue[i].Value >>= xInStream;
        else if ( pValue[i].Name == utl::MediaDescriptor::PROP_DOCUMENTSERVICE() )
        {
            location = i;
            pValue[i].Value >>= sDocService;
        }
    }

    try
    {
        if ( !xInStream.is() )
        {
            ucbhelper::Content aContent( sUrl, Reference<XCommandEnvironment>(), mxCtx );
            xInStream = aContent.openStream();
            if ( !xInStream.is() )
                return OUString();
        }

        boost::scoped_ptr<SvStream> pInStream( utl::UcbStreamHelper::CreateStream( xInStream ) );
        if ( !pInStream || pInStream->GetError() )
            return OUString();

        pInStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );
        sal_Size nUniPos = pInStream->Tell();

        const sal_uInt16 nSize = 4096;

        if ( nUniPos == 3 || nUniPos == 0 ) // UTF-8 or non-Unicode
            resultString = read_uInt8s_ToOString( *pInStream, nSize );
        else // UTF-16
            resultString = OUStringToOString( read_uInt16s_ToOUString( *pInStream, nSize ), RTL_TEXTENCODING_ASCII_US );

        if ( isHTMLStream( resultString.toAsciiLowerCase() ) )
        {
            // Some Apps/Web services use ".xls" extension to indicate that
            // the given file should be opened by a spreadsheet software
            if ( sDocService.isEmpty() )
            {
                INetURLObject aParser( sUrl );
                OUString aExt = aParser.getExtension( INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET );
                aExt = aExt.toAsciiLowerCase();

                if ( aExt == "xls" )
                {
                    if ( location == lDescriptor.getLength() )
                    {
                        lDescriptor.realloc( location + 1 );
                        lDescriptor[location].Name = utl::MediaDescriptor::PROP_DOCUMENTSERVICE();
                    }
                    lDescriptor[location].Value <<= OUString( "com.sun.star.sheet.SpreadsheetDocument" );
                }
            }
            return OUString( "generic_HTML" );
        }
    }
    catch (const Exception &)
    {
        OSL_FAIL( "An Exception occurred while opening File stream" );
    }

    return OUString(); // Failed
}

// XInitialization

void SAL_CALL HtmlFilterDetect::initialize(const Sequence<Any>& /*aArguments*/)
    throw (Exception, RuntimeException, std::exception)
{
}

OUString HtmlFilterDetect_getImplementationName()
{
    return OUString( "com.sun.star.comp.filters.HtmlFilterDetect" );
}

Sequence<OUString> HtmlFilterDetect_getSupportedServiceNames()
{
    Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ExtendedTypeDetection";
    pArray[1] = "com.sun.star.comp.filters.HtmlFilterDetect";
    return aRet;
}

Reference<XInterface> HtmlFilterDetect_createInstance(const Reference<XComponentContext>& rCtx)
{
    return (cppu::OWeakObject*) new HtmlFilterDetect( rCtx );
}

// XServiceInfo

OUString SAL_CALL HtmlFilterDetect::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return HtmlFilterDetect_getImplementationName();
}

sal_Bool SAL_CALL HtmlFilterDetect::supportsService(const OUString& rServiceName)
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}

Sequence<OUString> SAL_CALL HtmlFilterDetect::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return HtmlFilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
