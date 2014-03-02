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


#include "svgfilter.hxx"
#include "svgreader.hxx"

#include "rtl/ref.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Type.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include <unotools/mediadescriptor.hxx>
#include <tools/zcodec.hxx>

using namespace ::com::sun::star;
using namespace ::svgi;

sal_Bool SVGFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    utl::MediaDescriptor aMediaDescriptor(rDescriptor);
    uno::Reference<io::XInputStream> xInputStream;
    uno::Reference<task::XStatusIndicator> xStatus;

    xInputStream = uno::Reference<io::XInputStream>(aMediaDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY);
    xStatus = uno::Reference<task::XStatusIndicator>(aMediaDescriptor[utl::MediaDescriptor::PROP_STATUSINDICATOR()], UNO_QUERY);

    if (isStreamGZip(xInputStream))
    {
        uno::Reference<io::XSeekable> xSeek(xInputStream, uno::UNO_QUERY);
        if (!xSeek.is())
            return sal_False;
        xSeek->seek(0);

        boost::scoped_ptr<SvStream> aStream(utl::UcbStreamHelper::CreateStream(xInputStream, true ));
        if(!aStream.get())
            return sal_False;

        SvStream* pMemoryStream = new SvMemoryStream;
        GZCodec aCodec;
        aCodec.BeginCompression();
        aCodec.Decompress(*aStream.get(), *pMemoryStream);
        aCodec.EndCompression();
        pMemoryStream->Seek(STREAM_SEEK_TO_BEGIN);
        uno::Reference<io::XInputStream> xDecompressedInput(new utl::OSeekableInputStreamWrapper(pMemoryStream, true));
        if (!xDecompressedInput.is())
            return sal_False;
        xInputStream = xDecompressedInput;
    }
    else
    {
        uno::Reference<io::XSeekable> xSeek(xInputStream, uno::UNO_QUERY);
        if (xSeek.is())
            xSeek->seek(0);
    }

    OSL_ASSERT(xInputStream.is());
    if(!xInputStream.is())
        return sal_False;

    OUString sXMLImportService ( "com.sun.star.comp.Draw.XMLOasisImporter" );
    Reference < XDocumentHandler > xInternalHandler( mxContext->getServiceManager()->createInstanceWithContext( sXMLImportService, mxContext ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDstDoc);

    SVGReader aReader(mxContext, xInputStream, xInternalHandler);
    return aReader.parseAndConvert();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
