/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* KeynoteImportFilter: Sets up the filter, and calls OdpExporter
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

#include <xmloff/attrlist.hxx>

#include <libetonyek/libetonyek.h>
#include <libodfgen/libodfgen.hxx>

#include "common/DocumentHandler.hxx"
#include "common/WPXSvStream.hxx"
#include "KeynoteImportFilter.hxx"

#include <iostream>

using namespace ::com::sun::star::uno;
using com::sun::star::uno::Reference;
using com::sun::star::io::XInputStream;
using com::sun::star::io::XSeekable;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;


sal_Bool SAL_CALL KeynoteImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::filter");
    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
    }
    if ( !xInputStream.is() )
    {
        OSL_ASSERT( 0 );
        return sal_False;
    }

    // An XML import service: what we push sax messages to..
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Draw.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument( mxDoc );

    // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    WPXSvInputStream input( xInputStream );

    OdpGenerator exporter(&xHandler, ODF_FLAT_XML);
    bool tmpParseResult = libetonyek::KEYDocument::parse(&input, &exporter);
    return tmpParseResult;
}

void SAL_CALL KeynoteImportFilter::cancel(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::cancel");
}

// XImporter
void SAL_CALL KeynoteImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::setTargetDocument");
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL KeynoteImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
throw( com::sun::star::uno::RuntimeException )
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::detect");
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

    if (!xInputStream.is())
        return OUString();

    WPXSvInputStream input( xInputStream );

    if (libetonyek::KEYDocument::isSupported(&input))
        sTypeName = "impress_AppleKeynote";

    if (!sTypeName.isEmpty())
    {
        if ( location == nLength )
        {
            Descriptor.realloc(nLength+1);
            Descriptor[location].Name = "TypeName";
        }

        Descriptor[location].Value <<=sTypeName;
    }
    return sTypeName;
}


// XInitialization
void SAL_CALL KeynoteImportFilter::initialize( const Sequence< Any >& aArguments )
throw (Exception, RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::initialize");
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
OUString KeynoteImportFilter_getImplementationName ()
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_getImplementationName");
    return OUString ( "org.libreoffice.comp.Impress.KeynoteImportFilter" );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
sal_Bool SAL_CALL KeynoteImportFilter_supportsService( const OUString &ServiceName )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_supportsService");
    return ( ServiceName == SERVICE_NAME1 || ServiceName == SERVICE_NAME2 );
}
Sequence< OUString > SAL_CALL KeynoteImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_getSupportedServiceNames");
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  OUString ( SERVICE_NAME1 );
    pArray[1] =  OUString ( SERVICE_NAME2 );
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL KeynoteImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    SAL_INFO("writerperfect", "KeynoteImportFilter_createInstance");
    return (cppu::OWeakObject *) new KeynoteImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL KeynoteImportFilter::getImplementationName(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::getImplementationName");
    return KeynoteImportFilter_getImplementationName();
}
sal_Bool SAL_CALL KeynoteImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::supportsService");
    return KeynoteImportFilter_supportsService( rServiceName );
}
Sequence< OUString > SAL_CALL KeynoteImportFilter::getSupportedServiceNames(  )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "KeynoteImportFilter::getSupportedServiceNames");
    return KeynoteImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
