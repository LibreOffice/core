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
using com::sun::star::io::XInputStream;
using com::sun::star::beans::PropertyValue;
using com::sun::star::ucb::XCommandEnvironment;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::XDocumentHandler;

//                                 W     o     r     d     P     r     o
static const sal_Int8 header[] = { 0x57, 0x6f, 0x72, 0x64, 0x50, 0x72, 0x6f };

bool LotusWordProImportFilter::importImpl( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    OUString sURL;
    for (const PropertyValue& rValue : aDescriptor)
    {
        //Note, we should attempt to use InputStream here first!
        if ( rValue.Name == "URL" )
            rValue.Value >>= sURL;
    }

    SvFileStream inputStream( sURL, StreamMode::READ );
    if (!inputStream.good())
         return false;

    // An XML import service: what we push sax messages to...
    uno::Reference< XDocumentHandler > xInternalHandler(
        mxContext->getServiceManager()->createInstanceWithContext( "com.sun.star.comp.Writer.XMLImporter", mxContext ), UNO_QUERY );
    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    if (xImporter.is())
        xImporter->setTargetDocument(mxDoc);

    return ( ReadWordproFile( inputStream, xInternalHandler) == 0 );

}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportLWP(SvStream &rStream)
{
    uno::Reference< XDocumentHandler > xHandler;
    return ReadWordproFile(rStream, xHandler) == 0;
}

sal_Bool SAL_CALL LotusWordProImportFilter::filter( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    return importImpl ( aDescriptor );
}
void SAL_CALL LotusWordProImportFilter::cancel(  )
{
}

// XImporter
void SAL_CALL LotusWordProImportFilter::setTargetDocument( const uno::Reference< css::lang::XComponent >& xDoc )
{
    mxDoc = xDoc;
}

// XExtendedFilterDetection
OUString SAL_CALL LotusWordProImportFilter::detect( css::uno::Sequence< PropertyValue >& Descriptor )
{
    OUString sTypeName( "writer_LotusWordPro_Document" );
    OUString sURL;
    uno::Reference < XInputStream > xInputStream;
    for (const PropertyValue& rValue : std::as_const(Descriptor))
    {
        if ( rValue.Name == "TypeName" )
            rValue.Value >>= sTypeName;
        else if ( rValue.Name == "InputStream" )
            rValue.Value >>= xInputStream;
        else if ( rValue.Name == "URL" )
            rValue.Value >>= sURL;
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
    if ( ( nLen != xInputStream->readBytes( aData, nLen ) )
            || ( memcmp( static_cast<void const *>(header), static_cast<void const *>(aData.getConstArray()), nLen ) != 0 ) )
        sTypeName.clear();

    return sTypeName;
}

// XInitialization
void SAL_CALL LotusWordProImportFilter::initialize( const Sequence< Any >& /*aArguments*/ )
{
}

// XServiceInfo
OUString SAL_CALL LotusWordProImportFilter::getImplementationName()
{
    return "com.sun.star.comp.Writer.LotusWordProImportFilter";
}

sal_Bool SAL_CALL LotusWordProImportFilter::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence<OUString> SAL_CALL LotusWordProImportFilter::getSupportedServiceNames()
{
    return { "com.sun.star.document.ImportFilter", "com.sun.star.document.ExtendedTypeDetection" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
LotusWordProImportFilter_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new LotusWordProImportFilter(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
