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

using namespace ::com::sun::star;
using namespace ::svgi;

sal_Bool SVGFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    uno::Reference< io::XInputStream > xInputStream;
    uno::Reference< task::XStatusIndicator > xStatus;
    const sal_Int32 nLength = rDescriptor.getLength();
    const beans::PropertyValue* pAttribs = rDescriptor.getConstArray();
    for ( sal_Int32 i=0 ; i<nLength; ++i, ++pAttribs )
    {
        if ( pAttribs->Name == "InputStream" )
        {
            pAttribs->Value >>= xInputStream;
        }
        else if ( pAttribs->Name == "StatusIndicator" )
            pAttribs->Value >>= xStatus;
    }

    OSL_ASSERT(xInputStream.is());
    if(!xInputStream.is())
        return sal_False;

    rtl::OUString sXMLImportService ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.Draw.XMLOasisImporter" ) );
    Reference < XDocumentHandler > xInternalHandler( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    uno::Reference < XImporter > xImporter(xInternalHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDstDoc);

    SVGReader aReader(mxMSF, xInputStream, xInternalHandler);
    return aReader.parseAndConvert();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
