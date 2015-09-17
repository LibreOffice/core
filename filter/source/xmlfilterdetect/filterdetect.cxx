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

#include "filterdetect.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <memory>

using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;

namespace {

OUString supportedByType( const OUString& clipBoardFormat,  const OUString& resultString, const OUString& checkType)
{
    OUString sTypeName;
    if ( clipBoardFormat.match("doctype:") )
    {
        OUString tryStr = clipBoardFormat.copy(8);
        if (resultString.indexOf(tryStr) >= 0)
        {
            sTypeName = checkType;
        }
    }
    return sTypeName;
}

}

OUString SAL_CALL FilterDetect::detect( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& aArguments ) throw( com::sun::star::uno::RuntimeException, std::exception )
{
    OUString sTypeName;
    OUString sUrl;
    Sequence<PropertyValue > lProps ;

    com::sun::star::uno::Reference< com::sun::star::io::XInputStream > xInStream;
    const PropertyValue * pValue = aArguments.getConstArray();
    sal_Int32 nLength;
    OUString resultString;

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

        std::unique_ptr< SvStream > pInStream( ::utl::UcbStreamHelper::CreateStream( xInStream ) );
        pInStream->StartReadingUnicodeText( RTL_TEXTENCODING_DONTKNOW );
        sal_Size nUniPos = pInStream->Tell();

        const sal_uInt16 nSize = 4000;
        bool  bTryUtf16 = false;

        if ( nUniPos == 0 ) // No BOM detected, try to guess UTF-16 endianness
        {
            sal_uInt16 nHeader = 0;
            pInStream->ReadUInt16( nHeader );
            if ( nHeader == 0x003C )
                bTryUtf16 = true;
            else if ( nHeader == 0x3C00 )
            {
                bTryUtf16 = true;
                pInStream->SetEndian( pInStream->GetEndian() == SvStreamEndian::LITTLE ? SvStreamEndian::BIG : SvStreamEndian::LITTLE );
            }
            pInStream->Seek( STREAM_SEEK_TO_BEGIN );
        }

        if ( nUniPos == 3 || ( nUniPos == 0 && !bTryUtf16 ) ) // UTF-8 or non-Unicode
        {
            OString const str(read_uInt8s_ToOString(*pInStream, nSize));
            resultString = OUString(str.getStr(), str.getLength(),
                RTL_TEXTENCODING_ASCII_US,
                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_DEFAULT|RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT|RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT);
        }
        else if ( nUniPos == 2 || bTryUtf16 ) // UTF-16
            resultString = read_uInt16s_ToOUString( *pInStream, nSize );

        if ( !resultString.startsWith( "<?xml" ) )
            // This is not an XML stream.  It makes no sense to try to detect
            // a non-XML file type here.
            return OUString();

        // test typedetect code
        Reference <XNameAccess> xTypeCont(mxCtx->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", mxCtx), UNO_QUERY);
        Sequence < OUString > myTypes= xTypeCont->getElementNames();
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
                OUString tmpStr;
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
            aArguments[location].Name = "TypeName";
        }
        aArguments[location].Value <<=sTypeName;
    }

    return sTypeName;
}

// XInitialization
void SAL_CALL FilterDetect::initialize( const Sequence< Any >& /*aArguments*/ )
    throw (Exception, RuntimeException, std::exception)
{
}

OUString FilterDetect_getImplementationName ()
{
    return OUString( "com.sun.star.comp.filters.XMLFilterDetect" );
}

Sequence< OUString > FilterDetect_getSupportedServiceNames()
{
    Sequence < OUString > aRet(1);
    aRet[0] = "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > FilterDetect_createInstance( const Reference< XComponentContext > & context)
{
    return static_cast< cppu::OWeakObject * >( new FilterDetect( context ) );
}

// XServiceInfo
OUString SAL_CALL FilterDetect::getImplementationName(  )
    throw (RuntimeException, std::exception)
{
    return FilterDetect_getImplementationName();
}

sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName )
    throw (RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames(  )
    throw (RuntimeException, std::exception)
{
    return FilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
