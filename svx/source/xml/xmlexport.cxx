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

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <sot/storage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xmleohlp.hxx>
#include <svx/xmlgrhlp.hxx>

#include <svx/unomodel.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut, const Reference< lang::XComponent >& xComponent )
{
    return SvxDrawingLayerExport( pModel, xOut, xComponent, "com.sun.star.comp.DrawingLayer.XMLExporter" );
}

bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut, const Reference< lang::XComponent >& xComponent, const char* pExportService )
{
    bool bDocRet = xOut.is();

    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = nullptr;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = nullptr;

    Reference< lang::XComponent > xSourceDoc( xComponent );
    try
    {
        if( !xSourceDoc.is() )
        {
            xSourceDoc = new SvxUnoDrawingModel( pModel );
            pModel->setUnoModel( Reference< XInterface >::query( xSourceDoc ) );
        }

        uno::Reference< uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );

        if( bDocRet )
        {
            uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

            ::comphelper::IEmbeddedHelper *pPersist = pModel->GetPersist();
            if( pPersist )
            {
                pObjectHelper = SvXMLEmbeddedObjectHelper::Create( *pPersist, EMBEDDEDOBJECTHELPER_MODE_WRITE );
                xObjectResolver = pObjectHelper;
            }

            pGraphicHelper = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_WRITE );
            xGraphicResolver = pGraphicHelper;

            if( bDocRet )
            {
                uno::Reference<xml::sax::XDocumentHandler>  xHandler( xWriter, uno::UNO_QUERY );

                // doc export
                uno::Reference< io::XActiveDataSource > xDocSrc( xWriter, uno::UNO_QUERY );
                xDocSrc->setOutputStream( xOut );

                uno::Sequence< uno::Any > aArgs( xObjectResolver.is() ? 3 : 2 );
                aArgs[0] <<= xHandler;
                aArgs[1] <<= xGraphicResolver;
                if( xObjectResolver.is() )
                    aArgs[2] <<= xObjectResolver;

                uno::Reference< document::XFilter > xFilter( xContext->getServiceManager()->createInstanceWithArgumentsAndContext( OUString::createFromAscii( pExportService ), aArgs, xContext ), uno::UNO_QUERY );
                if( !xFilter.is() )
                {
                    OSL_FAIL( "com.sun.star.comp.Draw.XMLExporter service missing" );
                    bDocRet = false;
                }

                if( bDocRet )
                {
                    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
                    if( xExporter.is() )
                    {
                        xExporter->setSourceDocument( xSourceDoc );

                        uno::Sequence< beans::PropertyValue > aDescriptor( 0 );
                        bDocRet = xFilter->filter( aDescriptor );
                    }
                }
            }
        }
    }
    catch(uno::Exception const&)
    {
        DBG_UNHANDLED_EXCEPTION();
        bDocRet = false;
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = nullptr;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = nullptr;

    return bDocRet;
}

bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerExport( pModel, xOut, xComponent );
}

//-

bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream, const Reference< lang::XComponent >& xComponent )
{
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent, "com.sun.star.comp.Draw.XMLOasisImporter" );
}

bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream, const Reference< lang::XComponent >& xComponent, const char* pImportService  )
{
    sal_uInt32  nRet = 0;

    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = nullptr;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = nullptr;

    Reference< lang::XComponent > xTargetDocument( xComponent );
    if( !xTargetDocument.is() )
    {
        xTargetDocument = new SvxUnoDrawingModel( pModel );
        pModel->setUnoModel( Reference< XInterface >::query( xTargetDocument ) );
    }

    Reference< frame::XModel > xTargetModel( xTargetDocument, UNO_QUERY );

    try
    {
        // Get service factory
        Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

        if ( xTargetModel.is() )
            xTargetModel->lockControllers();



        pGraphicHelper = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_READ );
        xGraphicResolver = pGraphicHelper;

        ::comphelper::IEmbeddedHelper *pPersist = pModel->GetPersist();
        if( pPersist )
        {
            pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                        *pPersist,
                                        EMBEDDEDOBJECTHELPER_MODE_READ );
            xObjectResolver = pObjectHelper;
        }



        if( 0 == nRet )
        {

            // parse
            // prepare ParserInputSrouce
            xml::sax::InputSource aParserInput;
            aParserInput.aInputStream = xInputStream;

            // get parser
            Reference< xml::sax::XParser > xParser = xml::sax::Parser::create( xContext );

            // prepare filter arguments
            Sequence<Any> aFilterArgs( 2 );
            Any *pArgs = aFilterArgs.getArray();
            *pArgs++ <<= xGraphicResolver;
            *pArgs++ <<= xObjectResolver;

            // get filter
            Reference< xml::sax::XDocumentHandler > xFilter( xContext->getServiceManager()->createInstanceWithArgumentsAndContext( OUString::createFromAscii( pImportService ), aFilterArgs, xContext), UNO_QUERY );
            DBG_ASSERT( xFilter.is(), "Can't instantiate filter component." );

            nRet = 1;
            if( xParser.is() && xFilter.is() )
            {
                // connect parser and filter
                xParser->setDocumentHandler( xFilter );

                // connect model and filter
                uno::Reference < document::XImporter > xImporter( xFilter, UNO_QUERY );
                xImporter->setTargetDocument( xTargetDocument );

                // finally, parser the stream
                xParser->parseStream( aParserInput );

                nRet = 0;
            }
        }
    }
    catch( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = nullptr;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = nullptr;

    if ( xTargetModel.is() )
        xTargetModel->unlockControllers();

    return nRet == 0;
}

bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
