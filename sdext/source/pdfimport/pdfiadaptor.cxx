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
#include <saxemitter.hxx>
#include <odfemitter.hxx>
#include "inc/wrapper.hxx"
#include <pdfiprocessor.hxx>

#include <osl/file.h>
#include <sal/log.hxx>

#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <tools/diagnose_ex.h>

#include <memory>

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
sal_Bool SAL_CALL PDFIHybridAdaptor::filter( const uno::Sequence< beans::PropertyValue >& rFilterData )
{
    bool bRet = false;
    if( m_xModel.is() )
    {
        uno::Reference< io::XStream > xSubStream;
        OUString aPwd;
        const beans::PropertyValue* pAttribs = rFilterData.getConstArray();
        sal_Int32 nAttribs = rFilterData.getLength();
        sal_Int32 nPwPos = -1;
        for( sal_Int32 i = 0; i < nAttribs; i++ )
        {
            SAL_INFO("sdext.pdfimport", "filter: Attrib: " << pAttribs[i].Name
                    << " = " << (pAttribs[i].Value.has<OUString>()
                            ? pAttribs[i].Value.get<OUString>()
                            : OUString("<no string>"))
                    << "\n");
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
            auto pAttr = std::find_if(rFilterData.begin(), rFilterData.end(),
                [](const beans::PropertyValue& rAttr) { return rAttr.Name == "InputStream"; });
            if (pAttr != rFilterData.end())
                pAttr->Value >>= xInput;
            if( xInput.is() )
            {
                // TODO(P2): extracting hybrid substream twice - once during detection, second time here
                uno::Reference< io::XSeekable > xSeek( xInput, uno::UNO_QUERY );
                if( xSeek.is() )
                    xSeek->seek( 0 );
                oslFileHandle aFile = nullptr;
                sal_uInt64 nWritten = 0;
                OUString aURL;
                if( osl_createTempFile( nullptr, &aFile, &aURL.pData ) == osl_File_E_None )
                {
                    SAL_INFO("sdext.pdfimport", "created temp file " << aURL);
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
                        OUString aEmbedMimetype;
                        OUString aOrgPwd( aPwd );
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

            SAL_INFO("sdext.pdfimport", "try to instantiate subfilter" );
            uno::Reference< document::XFilter > xSubFilter;
            try {
                xSubFilter.set(
                    m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                        "com.sun.star.document.OwnSubFilter",
                        aArgs,
                        m_xContext ),
                    uno::UNO_QUERY );
            }
            catch(const uno::Exception&)
            {
                TOOLS_INFO_EXCEPTION("sdext.pdfimport", "subfilter");
            }

            SAL_INFO("sdext.pdfimport", "subfilter: " << xSubFilter.get() );
            if( xSubFilter.is() )
            {
                if( bAddPwdProp )
                {
                    uno::Sequence<beans::PropertyValue> aFilterData( rFilterData );
                    if( nPwPos == -1 )
                    {
                        nPwPos = aFilterData.getLength();
                        aFilterData.realloc( nPwPos+1 );
                        aFilterData[nPwPos].Name = "Password";
                    }
                    aFilterData[nPwPos].Value <<= aPwd;
                    bRet = xSubFilter->filter( aFilterData );
                }
                else
                    bRet = xSubFilter->filter( rFilterData );
            }
        }
        else
            SAL_INFO("sdext.pdfimport", "PDFIAdaptor::filter: no embedded substream set" );
    }
    else
        SAL_INFO("sdext.pdfimport", "PDFIAdaptor::filter: no model set" );

    return bRet;
}

void SAL_CALL PDFIHybridAdaptor::cancel()
{
}

//XImporter
void SAL_CALL PDFIHybridAdaptor::setTargetDocument( const uno::Reference< lang::XComponent >& xDocument )
{
    SAL_INFO("sdext.pdfimport", "PDFIAdaptor::setTargetDocument" );
    m_xModel.set( xDocument, uno::UNO_QUERY );
    if( xDocument.is() && ! m_xModel.is() )
        throw lang::IllegalArgumentException();
}

OUString PDFIHybridAdaptor::getImplementationName()
{
    return "org.libreoffice.comp.documents.HybridPDFImport";
}

sal_Bool PDFIHybridAdaptor::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> PDFIHybridAdaptor::getSupportedServiceNames()
{
    return {"com.sun.star.document.ImportFilter"};
}

PDFIRawAdaptor::PDFIRawAdaptor( OUString const & implementationName, const uno::Reference< uno::XComponentContext >& xContext ) :
    PDFIAdaptorBase( m_aMutex ),
    m_implementationName(implementationName),
    m_xContext( xContext ),
    m_xModel(),
    m_pVisitorFactory()
{
}

void PDFIRawAdaptor::setTreeVisitorFactory(const TreeVisitorFactorySharedPtr& rVisitorFactory)
{
    m_pVisitorFactory = rVisitorFactory;
}

bool PDFIRawAdaptor::parse( const uno::Reference<io::XInputStream>&       xInput,
                            const uno::Reference<task::XInteractionHandler>& xIHdl,
                            const OUString&                          rPwd,
                            const uno::Reference<task::XStatusIndicator>& xStatus,
                            const XmlEmitterSharedPtr&                    rEmitter,
                            const OUString&                          rURL,
                            const OUString&                          rFilterOptions )
{
    // container for metaformat
    auto pSink = std::make_shared<PDFIProcessor>(xStatus, m_xContext);

    bool bSuccess=false;

    if( xInput.is() )
        bSuccess = xpdf_ImportFromStream( xInput, pSink, xIHdl,
                                          rPwd, m_xContext, rFilterOptions );
    else
        bSuccess = xpdf_ImportFromFile( rURL, pSink, xIHdl,
                                        rPwd, m_xContext, rFilterOptions );

    if( bSuccess )
        pSink->emit(*rEmitter,*m_pVisitorFactory);

    return bSuccess;
}

bool PDFIRawAdaptor::odfConvert( const OUString&                          rURL,
                                 const uno::Reference<io::XOutputStream>&      xOutput,
                                 const uno::Reference<task::XStatusIndicator>& xStatus )
{
    XmlEmitterSharedPtr pEmitter = createOdfEmitter(xOutput);
    const bool bSuccess = parse(uno::Reference<io::XInputStream>(),
                                uno::Reference<task::XInteractionHandler>(),
                                OUString(),
                                xStatus,pEmitter,rURL, "");

    // tell input stream that it is no longer needed
    xOutput->closeOutput();

    return bSuccess;
}

// XImportFilter
sal_Bool SAL_CALL PDFIRawAdaptor::importer( const uno::Sequence< beans::PropertyValue >&        rSourceData,
                                            const uno::Reference< xml::sax::XDocumentHandler >& rHdl,
                                            const uno::Sequence< OUString >&               /*rUserData*/ )
{
    // get the InputStream carrying the PDF content
    uno::Reference< io::XInputStream > xInput;
    uno::Reference< task::XStatusIndicator > xStatus;
    uno::Reference< task::XInteractionHandler > xInteractionHandler;
    OUString aURL;
    OUString aPwd;
    OUString aFilterOptions;
    for( const beans::PropertyValue& rAttrib : rSourceData )
    {
        SAL_INFO("sdext.pdfimport", "importer Attrib: " << rAttrib.Name );
        if ( rAttrib.Name == "InputStream" )
            rAttrib.Value >>= xInput;
        else if ( rAttrib.Name == "URL" )
            rAttrib.Value >>= aURL;
        else if ( rAttrib.Name == "StatusIndicator" )
            rAttrib.Value >>= xStatus;
        else if ( rAttrib.Name == "InteractionHandler" )
            rAttrib.Value >>= xInteractionHandler;
        else if ( rAttrib.Name == "Password" )
            rAttrib.Value >>= aPwd;
        else if ( rAttrib.Name == "FilterOptions" )
            rAttrib.Value >>= aFilterOptions;
    }
    if( !xInput.is() )
        return false;

    XmlEmitterSharedPtr pEmitter = createSaxEmitter(rHdl);
    const bool bSuccess = parse(xInput, xInteractionHandler,
                                aPwd, xStatus, pEmitter, aURL, aFilterOptions);

    // tell input stream that it is no longer needed
    xInput->closeInput();
    xInput.clear();

    return bSuccess;
}

//XImporter
void SAL_CALL PDFIRawAdaptor::setTargetDocument( const uno::Reference< lang::XComponent >& xDocument )
{
    SAL_INFO("sdext.pdfimport", "PDFIAdaptor::setTargetDocument" );
    m_xModel.set( xDocument, uno::UNO_QUERY );
    if( xDocument.is() && ! m_xModel.is() )
        throw lang::IllegalArgumentException();
}

OUString PDFIRawAdaptor::getImplementationName()
{
    return m_implementationName;
}

sal_Bool PDFIRawAdaptor::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

css::uno::Sequence<OUString> PDFIRawAdaptor::getSupportedServiceNames()
{
    return {"com.sun.star.document.ImportFilter"};
}



extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sdext_PDFIRawAdaptor_Writer_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( "org.libreoffice.comp.documents.WriterPDFImport", context );
    pAdaptor->setTreeVisitorFactory(pdfi::createWriterTreeVisitorFactory());
    pAdaptor->acquire();
    return static_cast<cppu::OWeakObject*>(pAdaptor);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sdext_PDFIRawAdaptor_Draw_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( "org.libreoffice.comp.documents.DrawPDFImport", context );
    pAdaptor->setTreeVisitorFactory(pdfi::createDrawTreeVisitorFactory());
    pAdaptor->acquire();
    return static_cast<cppu::OWeakObject*>(pAdaptor);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sdext_PDFIRawAdaptor_Impress_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    pdfi::PDFIRawAdaptor* pAdaptor = new pdfi::PDFIRawAdaptor( "org.libreoffice.comp.documents.ImpressPDFImport", context );
    pAdaptor->setTreeVisitorFactory(pdfi::createImpressTreeVisitorFactory());
    pAdaptor->acquire();
    return static_cast<cppu::OWeakObject*>(pAdaptor);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
sdext_PDFIHybridAdaptor_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new pdfi::PDFIHybridAdaptor( context ));
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
