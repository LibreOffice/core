/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

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
    rtl::OUString aURL;
    uno::Reference< io::XInputStream > xInputStream;
    uno::Reference< task::XStatusIndicator > xStatus;
    const sal_Int32 nLength = rDescriptor.getLength();
    const beans::PropertyValue* pAttribs = rDescriptor.getConstArray();
    for ( sal_Int32 i=0 ; i<nLength; ++i, ++pAttribs )
    {
        if( pAttribs->Name.equalsAscii( "InputStream" ) )
        {
            pAttribs->Value >>= xInputStream;
        }
        else if( pAttribs->Name.equalsAscii( "StatusIndicator" ) )
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
