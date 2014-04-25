/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/uno/Reference.h>

#include <writerperfect/DocumentHandler.hxx>
#include <writerperfect/WPXSvInputStream.hxx>

#include <xmloff/attrlist.hxx>

#include "ImportFilterBase.hxx"

namespace writerperfect
{
namespace writer
{

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

using writerperfect::DocumentHandler;
using writerperfect::WPXSvInputStream;

ImportFilterImpl::ImportFilterImpl( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &rxContext )
    : mxContext( rxContext )
{
}

ImportFilterImpl::~ImportFilterImpl()
{
}

sal_Bool SAL_CALL ImportFilterImpl::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "ImportFilterImpl::filter");
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
    xImporter->setTargetDocument( mxDoc );

    // OO Graphics Handler: abstract class to handle document SAX messages, concrete implementation here
    // writes to in-memory target doc
    DocumentHandler xHandler(xInternalHandler);

    WPXSvInputStream input( xInputStream );

    OdtGenerator exporter(&xHandler, ODF_FLAT_XML);

    doRegisterHandlers(exporter);

    return doImportDocument(input, sFilterName, exporter);
}

void SAL_CALL ImportFilterImpl::cancel(  )
throw (RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "ImportFilterImpl::cancel");
}

// XImporter
void SAL_CALL ImportFilterImpl::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "ImportFilterImpl::setTargetDocument");
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL ImportFilterImpl::detect( com::sun::star::uno::Sequence< PropertyValue >& Descriptor )
throw( com::sun::star::uno::RuntimeException, std::exception )
{
    SAL_INFO("writerperfect", "ImportFilterImpl::detect");
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

    if ( doDetectFormat( input, sTypeName ) )
    {
        assert (!sTypeName.isEmpty());

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
void SAL_CALL ImportFilterImpl::initialize( const Sequence< Any >& aArguments )
throw (Exception, RuntimeException, std::exception)
{
    SAL_INFO("writerperfect", "ImportFilterImpl::initialize");
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

void ImportFilterImpl::doRegisterHandlers( OdtGenerator & )
{
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
