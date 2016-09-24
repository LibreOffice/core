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

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/tencinfo.h>
#include <sal/macros.h>
#include <tools/stream.hxx>
#include <vector>
#include <ucbhelper/content.hxx>
#include <xmloff/attrlist.hxx>

#include "LotusWordProImportFilter.hxx"
#include "lwpfilter.hxx"

using namespace com::sun::star;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::io::XInputStream;
using com::sun::star::beans::PropertyValue;
using com::sun::star::ucb::XCommandEnvironment;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::XDocumentHandler;

//                                 W     o     r     d     P     r     o
static const sal_Int8 header[] = { 0x57, 0x6f, 0x72, 0x64, 0x50, 0x72, 0x6f };

bool SAL_CALL LotusWordProImportFilter::importImpl( const Sequence< css::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException, std::exception)
{

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    OUString sURL;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        //Note, we should attempt to use InputStream here first!
        if ( pValue[i].Name == "URL" )
            pValue[i].Value >>= sURL;
    }

    SvFileStream inputStream( sURL, StreamMode::READ );
    if ( inputStream.IsEof() || ( inputStream.GetError() != SVSTREAM_OK ) )
         return false;

    // An XML import service: what we push sax messages to..
    OUString sXMLImportService ( "com.sun.star.comp.Writer.XMLImporter" );

    uno::Reference< XDocumentHandler > xInternalHandler( mxContext->getServiceManager()->createInstanceWithContext( sXMLImportService, mxContext ), UNO_QUERY );
    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    if (xImporter.is())
        xImporter->setTargetDocument(mxDoc);

    return ( ReadWordproFile( inputStream, xInternalHandler) == 0 );

}

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL TestImportLWP(const OUString &rURL)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    uno::Reference< XDocumentHandler > xHandler;
    return ( ReadWordproFile(aFileStream, xHandler) == 0 );
}

sal_Bool SAL_CALL LotusWordProImportFilter::filter( const Sequence< css::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException, std::exception)
{
    return importImpl ( aDescriptor );
}
void SAL_CALL LotusWordProImportFilter::cancel(  )
    throw (RuntimeException, std::exception)
{
}

// XImporter
void SAL_CALL LotusWordProImportFilter::setTargetDocument( const uno::Reference< css::lang::XComponent >& xDoc )
    throw (css::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL LotusWordProImportFilter::detect( css::uno::Sequence< PropertyValue >& Descriptor )
    throw( css::uno::RuntimeException, std::exception )
{

    OUString sTypeName( "writer_LotusWordPro_Document" );
    sal_Int32 nLength = Descriptor.getLength();
    OUString sURL;
    const PropertyValue * pValue = Descriptor.getConstArray();
    uno::Reference < XInputStream > xInputStream;
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
            pValue[i].Value >>= sTypeName;
        else if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= xInputStream;
        else if ( pValue[i].Name == "URL" )
            pValue[i].Value >>= sURL;
    }

    uno::Reference< css::ucb::XCommandEnvironment > xEnv;
    if (!xInputStream.is())
    {
        try
        {
            ::ucbhelper::Content aContent(sURL, xEnv, mxContext);
            xInputStream = aContent.openStream();
        }
        catch ( Exception& )
        {
            return OUString();
        }

        if (!xInputStream.is())
            return OUString();
    }

    Sequence< ::sal_Int8 > aData;
    sal_Int32 nLen = SAL_N_ELEMENTS( header );
    if ( !( ( nLen == xInputStream->readBytes( aData, nLen ) )
                && ( memcmp( static_cast<void const *>(header), static_cast<void const *>(aData.getConstArray()), nLen ) == 0 ) ) )
        sTypeName.clear();

    return sTypeName;
}

// XInitialization
void SAL_CALL LotusWordProImportFilter::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException, std::exception)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue * pValue = aAnySeq.getConstArray();
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

// XServiceInfo
OUString SAL_CALL LotusWordProImportFilter::getImplementationName()
    throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.Writer.LotusWordProImportFilter");
}

sal_Bool SAL_CALL LotusWordProImportFilter::supportsService(const OUString& rServiceName)
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL LotusWordProImportFilter::getSupportedServiceNames()
    throw (RuntimeException, std::exception)
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
LotusWordProImportFilter_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new LotusWordProImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
