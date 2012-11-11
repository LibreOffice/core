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

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

sal_Bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut, const Reference< lang::XComponent >& xComponent )
{
    return SvxDrawingLayerExport( pModel, xOut, xComponent, "com.sun.star.comp.DrawingLayer.XMLExporter" );
}

sal_Bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut, const Reference< lang::XComponent >& xComponent, const char* pExportService )
{
    sal_Bool bDocRet = xOut.is();

    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    Reference< lang::XComponent > xSourceDoc( xComponent );
    try
    {
        if( !xSourceDoc.is() )
        {
            xSourceDoc = new SvxUnoDrawingModel( pModel );
            pModel->setUnoModel( Reference< XInterface >::query( xSourceDoc ) );
        }

        uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
        uno::Reference< uno::XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
        if( !xServiceFactory.is() )
        {
            OSL_FAIL( "got no service manager" );
            bDocRet = sal_False;
        }

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

                uno::Reference< document::XFilter > xFilter( xServiceFactory->createInstanceWithArguments( OUString::createFromAscii( pExportService ), aArgs ), uno::UNO_QUERY );
                if( !xFilter.is() )
                {
                    OSL_FAIL( "com.sun.star.comp.Draw.XMLExporter service missing" );
                    bDocRet = sal_False;
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
        bDocRet = sal_False;
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    return bDocRet;
}

sal_Bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerExport( pModel, xOut, xComponent );
}

//-////////////////////////////////////////////////////////////////////

sal_Bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream, const Reference< lang::XComponent >& xComponent )
{
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent, "com.sun.star.comp.Draw.XMLOasisImporter" );
}

sal_Bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream, const Reference< lang::XComponent >& xComponent, const char* pImportService  )
{
    sal_uInt32  nRet = 0;

    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

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
        Reference< lang::XMultiServiceFactory > xServiceFactory = comphelper::getProcessServiceFactory();
        Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();
        DBG_ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );

        if( !xServiceFactory.is() )
            nRet = 1;

        if( 0 == nRet )
        {
            if ( xTargetModel.is() )
                xTargetModel->lockControllers();

            // -------------------------------------

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
        }

        // -------------------------------------

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
            Reference< xml::sax::XDocumentHandler > xFilter( xServiceFactory->createInstanceWithArguments( OUString::createFromAscii( pImportService ), aFilterArgs), UNO_QUERY );
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
    xGraphicResolver = 0;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    if ( xTargetModel.is() )
        xTargetModel->unlockControllers();

    return nRet == 0;
}

sal_Bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
