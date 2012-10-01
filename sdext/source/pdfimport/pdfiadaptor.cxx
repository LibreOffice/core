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


#include "pdfiadaptor.hxx"
#include "filterdet.hxx"
#include "saxemitter.hxx"
#include "odfemitter.hxx"
#include "inc/wrapper.hxx"
#include "inc/contentsink.hxx"
#include "tree/pdfiprocessor.hxx"

#include <osl/file.h>
#include <osl/thread.h>
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/io/XSeekable.hpp>


#include <boost/shared_ptr.hpp>

using namespace com::sun::star;


namespace pdfi
{

PDFIHybridAdaptor::PDFIHybridAdaptor( const uno::Reference< uno::XComponentContext >& xContext ) :
    PDFIHybridAdaptorBase( m_aMutex ),
    m_xContext( xContext ),
    m_xModel()
{
}

// XFilter
sal_Bool SAL_CALL PDFIHybridAdaptor::filter( const uno::Sequence< beans::PropertyValue >& rFilterData ) throw( uno::RuntimeException )
{
    sal_Bool bRet = sal_False;
    if( m_xModel.is() )
    {
        uno::Reference< io::XStream > xSubStream;
        rtl::OUString aPwd;
        const beans::PropertyValue* pAttribs = rFilterData.getConstArray();
        sal_Int32 nAttribs = rFilterData.getLength();
        sal_Int32 nPwPos = -1;
        for( sal_Int32 i = 0; i < nAttribs; i++ )
        {
            #if OSL_DEBUG_LEVEL > 1
            rtl::OUString aVal( RTL_CONSTASCII_USTRINGPARAM( "<no string>" ) );
            pAttribs[i].Value >>= aVal;
            OSL_TRACE( "filter: Attrib: %s = %s\n",
                       rtl::OUStringToOString( pAttribs[i].Name, RTL_TEXTENCODING_UTF8 ).getStr(),
                       rtl::OUStringToOString( aVal, RTL_TEXTENCODING_UTF8 ).getStr() );
            #endif
            if ( pAttribs[i].Name == "EmbeddedSubstream" )
                pAttribs[i].Value >>= xSubStream;
            else if ( pAttribs[i].Name == "Password" )
            {
                nPwPos = i;
                pAttribs[i].Value >>= aPwd;
            }
        }
        bool bAddPwdProp = false;
        if( ! xSubStream.is() )
        {
            uno::Reference< io::XInputStream > xInput;
            for( sal_Int32 i = 0; i < nAttribs; i++ )
            {
                if ( pAttribs[i].Name == "InputStream" )
                {
                    pAttribs[i].Value >>= xInput;
                    break;
                }
            }
            if( xInput.is() )
            {
                // TODO(P2): extracting hybrid substream twice - once during detection, second time here
                uno::Reference< io::XSeekable > xSeek( xInput, uno::UNO_QUERY );
                if( xSeek.is() )
                    xSeek->seek( 0 );
                oslFileHandle aFile = NULL;
                sal_uInt64 nWritten = 0;
                rtl::OUString aURL;
                if( osl_createTempFile( NULL, &aFile, &aURL.pData ) == osl_File_E_None )
                {
                    OSL_TRACE( "created temp file %s", rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8 ).getStr() );
                    const sal_Int32 nBufSize = 4096;
                    uno::Sequence<sal_Int8> aBuf(nBufSize);
                    // copy the bytes
                    sal_Int32 nBytes;
                    do
                    {
                        nBytes = xInput->readBytes( aBuf, nBufSize );
                        if( nBytes > 0 )
                        {
                            osl_writeFile( aFile, aBuf.getConstArray(), nBytes, &nWritten );
                            if( static_cast<sal_Int32>(nWritten) != nBytes )
                            {
                                xInput.clear();
                                break;
                            }
                        }
                    } while( nBytes == nBufSize );
                    osl_closeFile( aFile );
                    if( xInput.is() )
                    {
                        rtl::OUString aEmbedMimetype;
                        rtl::OUString aOrgPwd( aPwd );
                        xSubStream = getAdditionalStream( aURL, aEmbedMimetype, aPwd, m_xContext, rFilterData, true );
                        if( aOrgPwd != aPwd )
                            bAddPwdProp = true;
                    }
                    osl_removeFile( aURL.pData );
                }
                else
                    xSubStream.clear();
            }
        }
        if( xSubStream.is() )
        {
            uno::Sequence< uno::Any > aArgs( 2 );
            aArgs[0] <<= m_xModel;
            aArgs[1] <<= xSubStream;

            OSL_TRACE( "try to instantiate subfilter" );
            uno::Reference< document::XFilter > xSubFilter;
            try {
                xSubFilter = uno::Reference<document::XFilter>(
                    m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.OwnSubFilter" ) ),
                        aArgs,
                        m_xContext ),
                    uno::UNO_QUERY );
            }
            catch(const uno::Exception& e)
            {
                (void)e;
                OSL_TRACE( "subfilter exception: %s\n",
                           OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr() );
            }

            OSL_TRACE( "subfilter: %p", xSubFilter.get() );
            if( xSubFilter.is() )
            {
                if( bAddPwdProp )
                {
                    uno::Sequence<beans::PropertyValue> aFilterData( rFilterData );
                    if( nPwPos == -1 )
                    {
                        nPwPos = aFilterData.getLength();
                        aFilterData.realloc( nPwPos+1 );
                        aFilterData[nPwPos].Name = rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM( "Password" ) );
                    }
                    aFilterData[nPwPos].Value <<= aPwd;
                    bRet = xSubFilter->filter( aFilterData );
                }
                else
                    bRet = xSubFilter->filter( rFilterData );
            }
        }
        #if OSL_DEBUG_LEVEL > 1
        else
            OSL_TRACE( "PDFIAdaptor::filter: no embedded substream set" );
        #endif
    }
    #if OSL_DEBUG_LEVEL > 1
    else
        OSL_TRACE( "PDFIAdaptor::filter: no model set" );
    #endif

    return bRet;
}

void SAL_CALL PDFIHybridAdaptor::cancel() throw()
{
}

//XImporter
void SAL_CALL PDFIHybridAdaptor::setTargetDocument( const uno::Reference< lang::XComponent >& xDocument ) throw( lang::IllegalArgumentException )
{
    OSL_TRACE( "PDFIAdaptor::setTargetDocument" );
    m_xModel = uno::Reference< frame::XModel >( xDocument, uno::UNO_QUERY );
    if( xDocument.is() && ! m_xModel.is() )
        throw lang::IllegalArgumentException();
}

//---------------------------------------------------------------------------------------

PDFIRawAdaptor::PDFIRawAdaptor( const uno::Reference< uno::XComponentContext >& xContext ) :
    PDFIAdaptorBase( m_aMutex ),
    m_xContext( xContext ),
    m_xModel(),
    m_pVisitorFactory(),
    m_bEnableToplevelText(false)
{
}

void PDFIRawAdaptor::setTreeVisitorFactory(const TreeVisitorFactorySharedPtr& rVisitorFactory)
{
    m_pVisitorFactory = rVisitorFactory;
}

bool PDFIRawAdaptor::parse( const uno::Reference<io::XInputStream>&       xInput,
                            const uno::Reference<task::XInteractionHandler>& xIHdl,
                            const rtl::OUString&                          rPwd,
                            const uno::Reference<task::XStatusIndicator>& xStatus,
                            const XmlEmitterSharedPtr&                    rEmitter,
                            const rtl::OUString&                          rURL )
{
    // container for metaformat
    boost::shared_ptr<PDFIProcessor> pSink(
        new PDFIProcessor(xStatus, m_xContext));

    // TEMP! TEMP!
    if( m_bEnableToplevelText )
        pSink->enableToplevelText();

    bool bSuccess=false;

    if( xInput.is() )
        bSuccess = xpdf_ImportFromStream( xInput, pSink, xIHdl, rPwd, m_xContext );
    else
        bSuccess = xpdf_ImportFromFile( rURL, pSink, xIHdl, rPwd, m_xContext );

    if( bSuccess )
        pSink->emit(*rEmitter,*m_pVisitorFactory);

    return bSuccess;
}

bool PDFIRawAdaptor::odfConvert( const rtl::OUString&                          rURL,
                                 const uno::Reference<io::XOutputStream>&      xOutput,
                                 const uno::Reference<task::XStatusIndicator>& xStatus )
{
    XmlEmitterSharedPtr pEmitter = createOdfEmitter(xOutput);
    const bool bSuccess = parse(uno::Reference<io::XInputStream>(),
                                uno::Reference<task::XInteractionHandler>(),
                                rtl::OUString(),
                                xStatus,pEmitter,rURL);

    // tell input stream that it is no longer needed
    xOutput->closeOutput();

    return bSuccess;
}

// XImportFilter
sal_Bool SAL_CALL PDFIRawAdaptor::importer( const uno::Sequence< beans::PropertyValue >&        rSourceData,
                                            const uno::Reference< xml::sax::XDocumentHandler >& rHdl,
                                            const uno::Sequence< rtl::OUString >&               /*rUserData*/ ) throw( uno::RuntimeException )
{
    // get the InputStream carrying the PDF content
    uno::Reference< io::XInputStream > xInput;
    uno::Reference< task::XStatusIndicator > xStatus;
    uno::Reference< task::XInteractionHandler > xInteractionHandler;
    rtl::OUString aURL;
    rtl::OUString aPwd;
    const beans::PropertyValue* pAttribs = rSourceData.getConstArray();
    sal_Int32 nAttribs = rSourceData.getLength();
    for( sal_Int32 i = 0; i < nAttribs; i++, pAttribs++ )
    {
        OSL_TRACE("importer Attrib: %s", OUStringToOString( pAttribs->Name, RTL_TEXTENCODING_UTF8 ).getStr() );
        if ( pAttribs->Name == "InputStream" )
            pAttribs->Value >>= xInput;
        else if ( pAttribs->Name == "URL" )
            pAttribs->Value >>= aURL;
        else if ( pAttribs->Name == "StatusIndicator" )
            pAttribs->Value >>= xStatus;
        else if ( pAttribs->Name == "InteractionHandler" )
            pAttribs->Value >>= xInteractionHandler;
        else if ( pAttribs->Name == "Password" )
            pAttribs->Value >>= aPwd;
    }
    if( !xInput.is() )
        return sal_False;

    XmlEmitterSharedPtr pEmitter = createSaxEmitter(rHdl);
    const bool bSuccess = parse(xInput,xInteractionHandler, aPwd, xStatus,pEmitter,aURL);

    // tell input stream that it is no longer needed
    xInput->closeInput();
    xInput.clear();

    return bSuccess;
}

//XImporter
void SAL_CALL PDFIRawAdaptor::setTargetDocument( const uno::Reference< lang::XComponent >& xDocument ) throw( lang::IllegalArgumentException )
{
    OSL_TRACE( "PDFIAdaptor::setTargetDocument" );
    m_xModel = uno::Reference< frame::XModel >( xDocument, uno::UNO_QUERY );
    if( xDocument.is() && ! m_xModel.is() )
        throw lang::IllegalArgumentException();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
