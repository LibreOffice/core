/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* EBookImportFilter: Sets up the filter, and calls DocumentCollector
 * to do the actual filtering
 *
 * This file is part of the LibreOffice project.
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
#include <cppuhelper/supportsservice.hxx>

#include <xmloff/attrlist.hxx>
#include <ucbhelper/content.hxx>

#include <libe-book/libe-book.h>

#include <libodfgen/libodfgen.hxx>

#include "common/DocumentHandler.hxx"
#include "common/WPXSvStream.hxx"
#include "EBookImportFilter.hxx"

#include <iostream>

using namespace ::com::sun::star::uno;
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
using com::sun::star::ucb::XCommandEnvironment;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;


sal_Bool SAL_CALL EBookImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException)
{
    SAL_INFO("writerperfect", "EBookImportFilter::importImpl");

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue *pValue = aDescriptor.getConstArray();
    Reference < XInputStream > xInputStream;
    rtl::OUString sFilterName;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "FilterName" )
            pValue[i].Value >>= sFilterName;
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
    }
    if ( sFilterName.isEmpty() )
    {
        OSL_ASSERT( false );
        return sal_False;
    }
    if ( !xInputStream.is() )
    {
        OSL_ASSERT( false );
        return sal_False;
    }

    // An XML import service: what we push sax messages to..
    Reference < XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.comp.Writer.XMLOasisImporter", mxContext),
        css::uno::UNO_QUERY_THROW);

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    // OO Document Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    WPXSvInputStream input( xInputStream );

    OdtGenerator collector(&xHandler, ODF_FLAT_XML);

    if (sFilterName == "FictionBook 2")
        return libebook::FB2Document::parse(&input, &collector);
    else if (sFilterName == "PalmDoc")
        return libebook::PDBDocument::parse(&input, &collector);
    else if (sFilterName == "Plucker eBook")
        return libebook::PLKRDocument::parse(&input, &collector);
    else if (sFilterName == "eReader eBook")
        return libebook::PMLDocument::parse(&input, &collector);
    else if (sFilterName == "TealDoc")
        return libebook::TDDocument::parse(&input, &collector);
    else if (sFilterName == "zTXT")
        return libebook::ZTXTDocument::parse(&input, &collector);

    return sal_False;
}

sal_Bool SAL_CALL EBookImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "EBookImportFilter::filter");
    return importImpl ( aDescriptor );
}
void SAL_CALL EBookImportFilter::cancel(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "EBookImportFilter::cancel");
}

// XImporter
void SAL_CALL EBookImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "EBookImportFilter::getTargetDocument");
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL EBookImportFilter::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
throw( com::sun::star::uno::RuntimeException, std::exception )
{
    SAL_INFO("writerperfect", "EBookImportFilter::detect");

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

    if (libebook::FB2Document::isSupported(&input))
        sTypeName = "writer_FictionBook_2";
    else if (libebook::PDBDocument::isSupported(&input))
        sTypeName = "writer_PalmDoc";
    else if (libebook::PLKRDocument::isSupported(&input))
        sTypeName = "writer_Plucker_eBook";
    else if (libebook::PMLDocument::isSupported(&input))
        sTypeName = "writer_eReader_eBook";
    else if (libebook::TDDocument::isSupported(&input))
        sTypeName = "writer_TealDoc";
    else if (libebook::ZTXTDocument::isSupported(&input))
        sTypeName = "writer_zTXT";

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
void SAL_CALL EBookImportFilter::initialize( const Sequence< Any >& aArguments )
throw (Exception, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "EBookImportFilter::initialize");
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
OUString EBookImportFilter_getImplementationName ()
throw (RuntimeException)
{
    return OUString (  "org.libreoffice.comp.Writer.EBookImportFilter"  );
}

Sequence< OUString > SAL_CALL EBookImportFilter_getSupportedServiceNames(  )
throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString *pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1

Reference< XInterface > SAL_CALL EBookImportFilter_createInstance( const Reference< XComponentContext > & rContext)
throw( Exception )
{
    return (cppu::OWeakObject *) new EBookImportFilter( rContext );
}

// XServiceInfo
OUString SAL_CALL EBookImportFilter::getImplementationName(  )
throw (RuntimeException, std::exception)
{
    return EBookImportFilter_getImplementationName();
}
sal_Bool SAL_CALL EBookImportFilter::supportsService( const OUString &rServiceName )
throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}
Sequence< OUString > SAL_CALL EBookImportFilter::getSupportedServiceNames(  )
throw (RuntimeException, std::exception)
{
    return EBookImportFilter_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
