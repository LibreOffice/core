/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <xml/imagesconfiguration.hxx>
#include <services.h>

#include <xml/imagesdocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <unotools/streamwrap.hxx>
#include <tools/debug.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;


namespace framework
{

SV_IMPL_PTRARR( ImageItemListDescriptor, ImageItemDescriptorPtr );
SV_IMPL_PTRARR( ExternalImageItemListDescriptor, ExternalImageItemDescriptorPtr );
SV_IMPL_PTRARR( ImageListDescriptor, ImageListItemDescriptorPtr );

static Reference< XParser > GetSaxParser(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    //Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    //return Reference< XParser >( xServiceManager->createInstance( SERVICENAME_SAXPARSER), UNO_QUERY);
    return Reference< XParser >( xServiceFactory->createInstance( SERVICENAME_SAXPARSER), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    // #110897#
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    //Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
    //return Reference< XDocumentHandler >( xServiceManager->createInstance( SERVICENAME_SAXWRITER), UNO_QUERY) ;
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( SERVICENAME_SAXWRITER), UNO_QUERY) ;
}

// #110897#
sal_Bool ImagesConfiguration::LoadImages(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    SvStream& rInStream, ImageListsDescriptor& aItems )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );
    Reference< XInputStream > xInputStream(
                                (::cppu::OWeakObject *)new utl::OInputStreamWrapper( rInStream ),
                                UNO_QUERY );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set document handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadImagesDocumentHandler( aItems ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch( SAXException& )
    {
        return sal_False;
    }
    catch( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}


// #110897#
sal_Bool ImagesConfiguration::StoreImages(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    SvStream& rOutStream, const ImageListsDescriptor& aItems )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );

    Reference< XOutputStream > xOutputStream(
                                (::cppu::OWeakObject *)new utl::OOutputStreamWrapper( rOutStream ),
                                UNO_QUERY );

    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( xOutputStream );

    try
    {
        OWriteImagesDocumentHandler aWriteImagesDocumentHandler( aItems, xWriter );
        aWriteImagesDocumentHandler.WriteImagesDocument();
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch ( SAXException& )
    {
        return sal_False;
    }
    catch ( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

sal_Bool ImagesConfiguration::LoadImages(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rInputStream,
    ImageListsDescriptor& rItems )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );

    // connect stream to input stream to the parser
    InputSource aInputSource;

    aInputSource.aInputStream = rInputStream;

    // create namespace filter and set document handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadImagesDocumentHandler( rItems ));
    Reference< XDocumentHandler > xFilter( new SaxNamespaceFilter( xDocHandler ));

    // connect parser and filter
    xParser->setDocumentHandler( xFilter );

    try
    {
        xParser->parseStream( aInputSource );
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch( SAXException& )
    {
        return sal_False;
    }
    catch( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

sal_Bool ImagesConfiguration::StoreImages(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >& rOutputStream,
    const ImageListsDescriptor& rItems )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );

    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( rOutputStream );

    try
    {
        OWriteImagesDocumentHandler aWriteImagesDocumentHandler( rItems, xWriter );
        aWriteImagesDocumentHandler.WriteImagesDocument();
        return sal_True;
    }
    catch ( RuntimeException& )
    {
        return sal_False;
    }
    catch ( SAXException& )
    {
        return sal_False;
    }
    catch ( ::com::sun::star::io::IOException& )
    {
        return sal_False;
    }
}

}

