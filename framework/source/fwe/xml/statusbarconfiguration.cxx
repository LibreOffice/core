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
#include <framework/statusbarconfiguration.hxx>
#include <xml/statusbardocumenthandler.hxx>
#include <xml/saxnamespacefilter.hxx>
#include <services.h>

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
using namespace ::com::sun::star::container;


namespace framework
{

SV_IMPL_PTRARR( StatusBarDescriptor, StatusBarItemDescriptorPtr);

static Reference< XParser > GetSaxParser(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XParser >( xServiceFactory->createInstance( SERVICENAME_SAXPARSER), UNO_QUERY);
}

static Reference< XDocumentHandler > GetSaxWriter(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
    )
{
    return Reference< XDocumentHandler >( xServiceFactory->createInstance( SERVICENAME_SAXWRITER), UNO_QUERY) ;
}

sal_Bool StatusBarConfiguration::LoadStatusBar(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
    SvStream&, StatusBarDescriptor& )
{
    // obsolete - only support linkage of binary filters!
    return sal_True;
}

sal_Bool StatusBarConfiguration::StoreStatusBar(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&,
    SvStream&, const StatusBarDescriptor& )
{
    // obsolete - only support linkage of binary filters!
    return sal_True;
}

sal_Bool StatusBarConfiguration::LoadStatusBar(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XInputStream >& xInputStream,
    const Reference< XIndexContainer >& rStatusbarConfiguration )
{
    Reference< XParser > xParser( GetSaxParser( xServiceFactory ) );

    // connect stream to input stream to the parser
    InputSource aInputSource;
    aInputSource.aInputStream = xInputStream;

    // create namespace filter and set menudocument handler inside to support xml namespaces
    Reference< XDocumentHandler > xDocHandler( new OReadStatusBarDocumentHandler( rStatusbarConfiguration ));
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

sal_Bool StatusBarConfiguration::StoreStatusBar(
    const Reference< XMultiServiceFactory >& xServiceFactory,
    const Reference< XOutputStream >& xOutputStream,
    const Reference< XIndexAccess >& rStatusbarConfiguration )
{
    Reference< XDocumentHandler > xWriter( GetSaxWriter( xServiceFactory ) );
    Reference< ::com::sun::star::io::XActiveDataSource> xDataSource( xWriter , UNO_QUERY );
    xDataSource->setOutputStream( xOutputStream );

    try
    {
        OWriteStatusBarDocumentHandler aWriteStatusBarDocumentHandler( rStatusbarConfiguration, xWriter );
        aWriteStatusBarDocumentHandler.WriteStatusBarDocument();
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

