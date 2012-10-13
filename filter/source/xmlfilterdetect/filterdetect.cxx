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

#include <iostream>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <cstring>
#include "filterdetect.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/xml/XImportFilter.hpp>
#include <com/sun/star/xml/XExportFilter.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyleLoader.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <ucbhelper/content.hxx>

using rtl::OUString;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Any;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::io::XActiveDataSource;
using com::sun::star::io::XOutputStream;
using com::sun::star::beans::PropertyValue;
using com::sun::star::document::XExporter;
using com::sun::star::document::XFilter;
using com::sun::star::document::XExtendedFilterDetection;

using com::sun::star::io::XInputStream;
using com::sun::star::document::XImporter;
using com::sun::star::xml::sax::InputSource;
using com::sun::star::xml::sax::XDocumentHandler;
using com::sun::star::xml::sax::XParser;

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

namespace {

bool isXMLStream(const ::rtl::OString& aHeaderStrm)
{
    const char* p = aHeaderStrm.getStr();
    size_t n = aHeaderStrm.getLength();
    size_t i = 0;

    // Skip all preceding blank characters.
    for (i = 0; i < n; ++i, ++p)
    {
        sal_Char c = *p;
        if (c == ' ' || c == '\n' || c == '\t')
            continue;
        break;
    }

    n -= i;

    // First text must be '<?xml', else it's not a valid XML file stream.
    const char* sInitChars = "<?xml";
    const size_t nInitCharLen = std::strlen(sInitChars);
    for (i = 0; i < n; ++i, ++p)
    {
        if (i < nInitCharLen)
        {
            if (*p != sInitChars[i])
                return false;
        }
    }
    return true;
}

::rtl::OUString supportedByType( const ::rtl::OUString clipBoardFormat ,  const ::rtl::OString resultString, const ::rtl::OUString checkType)
{
    ::rtl::OUString sTypeName;
    if ( clipBoardFormat.match(OUString("doctype:")) )
    {
        ::rtl::OString tryStr = ::rtl::OUStringToOString(clipBoardFormat.copy(8),RTL_TEXTENCODING_ASCII_US).getStr();
        if (resultString.indexOf(tryStr) >= 0)
        {
            sTypeName = checkType;
        }
    }
    return sTypeName;
}

}

::rtl::OUString SAL_CALL FilterDetect::detect( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& aArguments ) throw( com::sun::star::uno::RuntimeException )
{
    ::rtl::OUString sTypeName;
    ::rtl::OUString sUrl;
    Sequence<PropertyValue > lProps ;

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInStream;
    const PropertyValue * pValue = aArguments.getConstArray();
    sal_Int32 nLength;
    ::rtl::OString resultString;

    nLength = aArguments.getLength();
    sal_Int32 location=nLength;
    for (sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "TypeName" )
        {
            location=i;
        }
        else if ( pValue[i].Name == "URL" )
        {
            pValue[i].Value >>= sUrl;
        }
        else if ( pValue[i].Name == "InputStream" )
        {
            pValue[i].Value >>= xInStream ;
        }
    }
    try
    {
        if (!xInStream.is())
        {
            ::ucbhelper::Content aContent(
                sUrl, Reference< com::sun::star::ucb::XCommandEnvironment >(),
                mxCtx);
            xInStream = aContent.openStream();
            if (!xInStream.is())
            {
                return sTypeName;
            }
        }
        com::sun::star::uno::Sequence< sal_Int8 > aData;
        /* long nBytesToRead= */ xInStream->available();
        xInStream->skipBytes (0);
        long bytestRead =xInStream->readBytes (aData,  4000);
        resultString=::rtl::OString((const sal_Char *)aData.getConstArray(),bytestRead) ;

        if (!isXMLStream(resultString))
            // This is not an XML stream.  It makes no sense to try to detect
            // a non-XML file type here.
            return ::rtl::OUString();

        // test typedetect code
        Reference <XNameAccess> xTypeCont(mxCtx->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", mxCtx), UNO_QUERY);
        Sequence < ::rtl::OUString > myTypes= xTypeCont->getElementNames();
        nLength = myTypes.getLength();

        sal_Int32 new_nlength=0;
        sal_Int32 i = 0 ;
        while ((i < nLength) && (sTypeName.isEmpty()))
        {
            Any elem = xTypeCont->getByName(myTypes[i]);
            elem >>=lProps;
            new_nlength = lProps.getLength();
            sal_Int32 j =0;
            while (j < new_nlength && (sTypeName.isEmpty()))
            {
                ::rtl::OUString tmpStr;
                lProps[j].Value >>=tmpStr;
                if ( lProps[j].Name == "ClipboardFormat" && !tmpStr.isEmpty() )
                {
                    sTypeName = supportedByType(tmpStr,resultString, myTypes[i]);
                }
                j++;
            }
            i++;
        }
    }
    catch (const Exception &)
    {
        OSL_FAIL( "An Exception occurred while opening File stream" );
    }

    if (!sTypeName.isEmpty())
    {
        if (location == aArguments.getLength())
        {
            aArguments.realloc(nLength+1);
            aArguments[location].Name = ::rtl::OUString( "TypeName" );
        }
        aArguments[location].Value <<=sTypeName;
    }

    return sTypeName;
}

// XInitialization

void SAL_CALL FilterDetect::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
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

            }
            else if ( pValue[i].Name == "UserData" )
            {

                pValue[i].Value >>= msUserData;

            }
            else if ( pValue[i].Name == "TemplateName" )
            {

              pValue[i].Value>>=msTemplateName;
            }

        }
    }
}



OUString FilterDetect_getImplementationName ()
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.comp.filters.XMLFilterDetect" ) );
}
#define SERVICE_NAME1 "com.sun.star.document.ExtendedTypeDetection"

Sequence< OUString > FilterDetect_getSupportedServiceNames()
{
    Sequence < OUString > aRet(1);
    aRet[0] = SERVICE_NAME1;
    return aRet;
}

Reference< XInterface > FilterDetect_createInstance( const Reference< XComponentContext > & context)
{
    return static_cast< cppu::OWeakObject * >( new FilterDetect( context ) );
}

// XServiceInfo
OUString SAL_CALL FilterDetect::getImplementationName(  )
    throw (RuntimeException)
{
    return FilterDetect_getImplementationName();
}
sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return rServiceName == SERVICE_NAME1;
}
Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return FilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
