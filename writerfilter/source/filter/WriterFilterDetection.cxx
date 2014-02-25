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

#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <WriterFilterDetection.hxx>
#include <comphelper/storagehelper.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <sot/storage.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star;



WriterFilterDetection::WriterFilterDetection(
    const uno::Reference< uno::XComponentContext >& rxContext) :
    m_xContext( rxContext )
{
}


WriterFilterDetection::~WriterFilterDetection()
{
}


OUString WriterFilterDetection_getImplementationName () throw (uno::RuntimeException)
{
   return OUString ( "com.sun.star.comp.Writer.WriterFilterDetector"  );
}



OUString WriterFilterDetection::detect( uno::Sequence< beans::PropertyValue >& rDescriptor )
   throw( uno::RuntimeException, std::exception )
{
    OUString sTypeName;
    bool bWord = false;
    sal_Int32 nPropertyCount = rDescriptor.getLength();
    const beans::PropertyValue* pValues = rDescriptor.getConstArray();
    OUString sURL;
    uno::Reference < io::XStream > xStream;
    uno::Reference < io::XInputStream > xInputStream;
    for( sal_Int32 nProperty = 0; nProperty < nPropertyCount; ++nProperty )
    {
        if ( pValues[nProperty].Name == "TypeName" )
            rDescriptor[nProperty].Value >>= sTypeName;
        else if ( pValues[nProperty].Name == "URL" )
            pValues[nProperty].Value >>= sURL;
        else if ( pValues[nProperty].Name == "Stream" )
            pValues[nProperty].Value >>= xStream;
        else if ( pValues[nProperty].Name == "InputStream" )
            pValues[nProperty].Value >>= xInputStream;
    }
    bool bBinary = sTypeName == "writer_MS_Word_97" ||
                   sTypeName == "writer_MS_Word_97_Vorlage";

    try
    {
        if(bBinary)
        {
            SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xInputStream );
            if ( pStream && SotStorage::IsStorageFile(pStream) )

            {
                SotStorageRef xStg = new SotStorage( pStream, false );

                bool bTable2 = xStg->IsContained(OUString("1Table"));
                SotStorageStreamRef xRef = xStg->OpenSotStream(OUString("WordDocument"), STREAM_STD_READ | STREAM_NOCREATE );

                if(bTable2 && xStg.Is())
                {
                    xRef->Seek(2);
                    sal_Int16 nWord;
                    xRef->ReadInt16( nWord );
                    //version detection
                    bWord = nWord >= 0x6a && nWord <= 0xc1;
                }
            }
        }
        else
        {
            uno::Reference< embed::XStorage > xDocStorage;
            if ( sURL == "private:stream" )
                xDocStorage = comphelper::OStorageHelper::GetStorageFromInputStream( xInputStream );
            else
                xDocStorage = comphelper::OStorageHelper::GetStorageFromURL(
                                            sURL, embed::ElementModes::READ );
            if( xDocStorage.is() )
            {
                uno::Sequence< OUString > aNames = xDocStorage->getElementNames();
                const OUString* pNames = aNames.getConstArray();
                for(sal_Int32 nName = 0; nName < aNames.getLength(); ++nName)
                {
                    if ( pNames[nName] == "word" )
                    {
                        bWord = true;
                        if( sTypeName.isEmpty() )
                            sTypeName = "writer_MS_Word_2007";
                        break;
                    }
                }
            }
        }
    }
    catch(const uno::Exception&)
    {
        OSL_FAIL("exception while opening storage");
    }
    if( !bWord )
        sTypeName = OUString();
   return sTypeName;
}


uno::Sequence< OUString > WriterFilterDetection_getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
   uno::Sequence < OUString > aRet(1);
   OUString* pArray = aRet.getArray();
   pArray[0] = "com.sun.star.document.ExtendedTypeDetection";
   return aRet;
}


uno::Reference< uno::XInterface > WriterFilterDetection_createInstance( const uno::Reference< uno::XComponentContext >& xContext)
                throw( uno::Exception )
{
   return (cppu::OWeakObject*) new WriterFilterDetection( xContext );
}


OUString WriterFilterDetection::getImplementationName(  ) throw (uno::RuntimeException, std::exception)
{
   return WriterFilterDetection_getImplementationName();
}


sal_Bool WriterFilterDetection::supportsService( const OUString& rServiceName ) throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService( this, rServiceName );
}


uno::Sequence< OUString > WriterFilterDetection::getSupportedServiceNames(  ) throw (uno::RuntimeException, std::exception)
{
    return WriterFilterDetection_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
