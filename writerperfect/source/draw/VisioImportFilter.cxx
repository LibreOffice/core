/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* VisioImportFilter: Sets up the filter, and calls OdgExporter
 * to do the actual filtering
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <osl/diagnose.h>
#include <rtl/tencinfo.h>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/uno/Reference.h>

#include <comphelper/componentcontext.hxx>
#include <xmloff/attrlist.hxx>

#include <libvisio/libvisio.h>
#include "filter/DocumentHandler.hxx"
#include "filter/OdgGenerator.hxx"
#include "VisioImportFilter.hxx"
#include "stream/WPXSvStream.h"

#include <iostream>

using namespace ::com::sun::star::uno;
using com::sun::star::uno::Reference;
using com::sun::star::io::XInputStream;
using com::sun::star::io::XSeekable;
using com::sun::star::uno::Sequence;
using namespace ::rtl;
using rtl::OString;
using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;


sal_Bool SAL_CALL VisioImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::filter" << std::endl;
#endif
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    OUString sURL;
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name == "URL" )
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
    OUString sXMLImportService ( "com.sun.star.comp.Draw.XMLOasisImporter" );
    Reference < XDocumentHandler > xInternalHandler( comphelper::ComponentContext( mxContext ).createComponent( sXMLImportService ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument( mxDoc );

    // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    WPXSvInputStream input( xInputStream );

    OdgGenerator exporter(&xHandler, ODF_FLAT_XML);
    bool tmpParseResult = libvisio::VisioDocument::parse(&input, &exporter);
    return tmpParseResult;
}

void SAL_CALL VisioImportFilter::cancel(  )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::cancel" << std::endl;
#endif
}

// XImporter
void SAL_CALL VisioImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::setTargetDocument" << std::endl;
#endif
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL VisioImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
throw( com::sun::star::uno::RuntimeException )
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::detect" << std::endl;
#endif
    OUString sTypeName;
    sal_Int32 nLength = Descriptor.getLength();
    sal_Int32 location = nLength;
    const PropertyValue *pValue = Descriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
            location=i;
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
    }

    WPXSvInputStream input( xInputStream );

    if (libvisio::VisioDocument::isSupported(&input))
        sTypeName = "draw_Visio_Document";

    if (!sTypeName.isEmpty())
    {
        if ( location == Descriptor.getLength() )
        {
            Descriptor.realloc(nLength+1);
            Descriptor[location].Name = "TypeName";
        }

        Descriptor[location].Value <<=sTypeName;
    }
    return sTypeName;
}


// XInitialization
void SAL_CALL VisioImportFilter::initialize( const Sequence< Any >& aArguments )
throw (Exception, RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::initialize" << std::endl;
#endif
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue *pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}
OUString VisioImportFilter_getImplementationName ()
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter_getImplementationName" << std::endl;
#endif
    return OUString ( "com.sun.star.comp.Draw.VisioImportFilter" );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
sal_Bool SAL_CALL VisioImportFilter_supportsService( const OUString &ServiceName )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter_supportsService" << std::endl;
#endif
    return ( ServiceName == SERVICE_NAME1 || ServiceName == SERVICE_NAME2 );
}
Sequence< OUString > SAL_CALL VisioImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter_getSupportedServiceNames" << std::endl;
#endif
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME1 );
    pArray[1] =  OUString ( SERVICE_NAME2 );
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL VisioImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter_createInstance" << std::endl;
#endif
    return (cppu::OWeakObject *) new VisioImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL VisioImportFilter::getImplementationName(  )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::getImplementationName" << std::endl;
#endif
    return VisioImportFilter_getImplementationName();
}
sal_Bool SAL_CALL VisioImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::supportsService" << std::endl;
#endif
    return VisioImportFilter_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL VisioImportFilter::getSupportedServiceNames(  )
throw (RuntimeException)
{
#ifdef DEBUG
    std::cerr << "VisioImportFilter::getSupportedServiceNames" << std::endl;
#endif
    return VisioImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
