/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MSWorksImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * Copyright (C) 2000 by Sun Microsystems, Inc.
 * Copyright (C) 2002-2004 William Lachance (wlach@interlog.com)
 * Copyright (C) 2004 Net Integration Technologies (http://www.net-itech.com)
 * Copyright (C) 2004 Fridrich Strba <fridrich.strba@bluewin.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 */
/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <osl/diagnose.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <xmloff/attrlist.hxx>
#include <ucbhelper/content.hxx>

#include "filter/FilterInternal.hxx"
#include "filter/DocumentHandler.hxx"
#include "filter/DocumentCollector.hxx"
#include "stream/WPXSvStream.h"

#include <libwps/WPSDocument.h>
#include "MSWorksImportFilter.hxx"
#include "MSWorksCollector.hxx"

// using namespace ::rtl;
using rtl::OString;
using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::ucb::XCommandEnvironment;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

void callHandler(Reference < XDocumentHandler > xDocHandler);

sal_Bool SAL_CALL MSWorksImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    WRITER_DEBUG_MSG(("MSWorksImportFilter::importImpl: Got here!\n"));

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sURL;
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InputStream" ) ) )
        pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
        pValue[i].Value >>= sURL;
    }
    if ( !xInputStream.is() )
    {
        OSL_ASSERT( 0 );
        return sal_False;
    }
    OString sFileName;
    sFileName = OUStringToOString(sURL, RTL_TEXTENCODING_INFO_ASCII);

    // An XML import service: what we push sax messages to..
    OUString sXMLImportService ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.XMLImporter" ) );
    Reference < XDocumentHandler > xInternalHandler( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

        // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
        // writes to in-memory target doc
        DocumentHandler xHandler(xInternalHandler);

    WPXSvInputStream input( xInputStream );

    MSWorksCollector collector(&input, &xHandler);
    collector.filter();

    return true;
}

sal_Bool SAL_CALL MSWorksImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    WRITER_DEBUG_MSG(("MSWorksImportFilter::filter: Got here!\n"));
    return importImpl ( aDescriptor );
}
void SAL_CALL MSWorksImportFilter::cancel(  )
    throw (RuntimeException)
{
    WRITER_DEBUG_MSG(("MSWorksImportFilter::cancel: Got here!\n"));
}

// XImporter
void SAL_CALL MSWorksImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    WRITER_DEBUG_MSG(("MSWorksImportFilter::getTargetDocument: Got here!\n"));
    meType = FILTER_IMPORT;
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL MSWorksImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
    throw( com::sun::star::uno::RuntimeException )
{
    WRITER_DEBUG_MSG(("MSWorksImportFilter::detect: Got here!\n"));

    WPSConfidence confidence = WPS_CONFIDENCE_NONE;
    OUString sTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM ( "" ) );
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 location = nLength;
    OUString sURL;
    const PropertyValue * pValue = Descriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "TypeName" ) ) )
            location=i;
        else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "InputStream" ) ) )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "URL" ) ) )
            pValue[i].Value >>= sURL;
    }

        Reference< com::sun::star::ucb::XCommandEnvironment > xEnv;
        if (!xInputStream.is())
        {
        try
        {
            ::ucbhelper::Content aContent(sURL, xEnv);
                    xInputStream = aContent.openStream();
        }
        catch ( ... )
        {
            return ::rtl::OUString();
        }

                if (!xInputStream.is())
                    return ::rtl::OUString();
        }

    WPXSvInputStream input( xInputStream );

    if (input.atEOS())
        return ::rtl::OUString();

    confidence = WPSDocument::isFileFormatSupported(&input, false);

    if ((confidence == WPS_CONFIDENCE_EXCELLENT) || (confidence == WPS_CONFIDENCE_GOOD))
        sTypeName = OUString( RTL_CONSTASCII_USTRINGPARAM ( "writer_MS_Works_Document" ) );

    if (sTypeName.getLength())
    {
        if ( location == Descriptor.getLength() )
        {
            Descriptor.realloc(nLength+1);
            Descriptor[location].Name = ::rtl::OUString::createFromAscii( "TypeName" );
        }

        Descriptor[location].Value <<=sTypeName;
    }

    return sTypeName;
}


// XInitialization
void SAL_CALL MSWorksImportFilter::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    WRITER_DEBUG_MSG(("MSWorksImportFilter::initialize: Got here!\n"));
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue * pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name.equalsAsciiL ( RTL_CONSTASCII_STRINGPARAM ( "Type" ) ) )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}
OUString MSWorksImportFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Writer.MSWorksImportFilter" ) );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
sal_Bool SAL_CALL MSWorksImportFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return (ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME1 ) ) ||
        ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( SERVICE_NAME2 ) ) );
}
Sequence< OUString > SAL_CALL MSWorksImportFilter_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
        OUString* pArray = aRet.getArray();
        pArray[0] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME1 ) );
    pArray[1] =  OUString ( RTL_CONSTASCII_USTRINGPARAM ( SERVICE_NAME2 ) );
        return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL MSWorksImportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new MSWorksImportFilter( rSMgr );
}

// XServiceInfo
OUString SAL_CALL MSWorksImportFilter::getImplementationName(  )
    throw (RuntimeException)
{
    return MSWorksImportFilter_getImplementationName();
}
sal_Bool SAL_CALL MSWorksImportFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return MSWorksImportFilter_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL MSWorksImportFilter::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return MSWorksImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
