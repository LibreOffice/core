/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/svdmodel.hxx>
#include <svx/xmleohlp.hxx>

#include <svx/unomodel.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace cpo::uno;

namespace
{

/** Build a graphic storage handler for the drawing layer filter.

    The single empty argument makes the handler come up without a storage of its own, so it works
    on the graphics passed through it rather than on a package.
 */
uno::Reference<document::XGraphicStorageHandler> createGraphicStorageHandler(
    const uno::Reference<cpo::uno::XComponentContext>& rContext, const OUString& rServiceName)
{
    cpo::uno::Sequence<cpo::uno::Any> aArguments{ cpo::uno::Any() };
    return uno::Reference<document::XGraphicStorageHandler>(
        rContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            rServiceName, aArguments, rContext ), uno::UNO_QUERY);
}

void disposeGraphicStorageHandler(const uno::Reference<document::XGraphicStorageHandler>& rHandler)
{
    uno::Reference<lang::XComponent> xComponent(rHandler, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
}

}

bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut, const Reference< lang::XComponent >& xComponent )
{
    return SvxDrawingLayerExport( pModel, xOut, xComponent, "com.sun.star.comp.DrawingLayer.XMLExporter" );
}

bool SvxDrawingLayerExport( SdrModel* pModel, const uno::Reference<io::XOutputStream>& xOut, const Reference< lang::XComponent >& xComponent, const char* pExportService )
{
    bool bDocRet = xOut.is();

    uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    rtl::Reference<SvXMLEmbeddedObjectHelper> xObjectHelper;
    Reference< lang::XComponent > xSourceDoc( xComponent );
    Reference< frame::XModel > xSourceModel;

    try
    {
        if( !xSourceDoc.is() )
        {
            rtl::Reference<SvxUnoDrawingModel> pDrawingModel = new SvxUnoDrawingModel( pModel );
            xSourceDoc = pDrawingModel;
            pModel->setUnoModel( pDrawingModel );
        }

        xSourceModel = Reference< frame::XModel >( xSourceDoc, UNO_QUERY );
        if ( xSourceModel.is() )
            xSourceModel->lockControllers();
        pModel->incImportExport();

        const uno::Reference< cpo::uno::XComponentContext>& xContext( ::comphelper::getProcessComponentContext() );

        if( bDocRet )
        {
            uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

            ::comphelper::IEmbeddedHelper *pPersist = pModel->GetPersist();
            if( pPersist )
            {
                xObjectHelper = SvXMLEmbeddedObjectHelper::Create( *pPersist, SvXMLEmbeddedObjectHelperMode::Write );
                xObjectResolver = xObjectHelper.get();
            }

            xGraphicStorageHandler = createGraphicStorageHandler(
                xContext, u"com.sun.star.comp.Svx.GraphicExportHelper"_ustr );

            uno::Reference<xml::sax::XDocumentHandler>  xHandler = xWriter;

            // doc export
            xWriter->setOutputStream( xOut );

            cpo::uno::Sequence< cpo::uno::Any > aArgs( xObjectResolver.is() ? 3 : 2 );
            auto pArgs = aArgs.getArray();
            pArgs[0] <<= xHandler;
            pArgs[1] <<= xGraphicStorageHandler;
            if( xObjectResolver.is() )
                pArgs[2] <<= xObjectResolver;

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

                    cpo::uno::Sequence< beans::PropertyValue > aDescriptor( 0 );
                    bDocRet = xFilter->filter( aDescriptor );
                }
            }
        }
    }
    catch(cpo::uno::Exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("svx");
        bDocRet = false;
    }

    disposeGraphicStorageHandler(xGraphicStorageHandler);
    xGraphicStorageHandler = nullptr;

    if( xObjectHelper.is() )
        xObjectHelper->dispose();

    if ( xSourceModel.is() )
        xSourceModel->unlockControllers();
    pModel->decImportExport();

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
    bool bRet = true;

    uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    rtl::Reference<SvXMLEmbeddedObjectHelper> xObjectHelper;

    Reference< lang::XComponent > xTargetDocument( xComponent );
    if( !xTargetDocument.is() )
    {
        rtl::Reference<SvxUnoDrawingModel> pDrawingModel = new SvxUnoDrawingModel( pModel );
        xTargetDocument = pDrawingModel;
        pModel->setUnoModel( pDrawingModel );
    }

    Reference< frame::XModel > xTargetModel( xTargetDocument, UNO_QUERY );

    try
    {
        // Get service factory
        const Reference< cpo::uno::XComponentContext >& xContext = comphelper::getProcessComponentContext();

        if ( xTargetModel.is() )
            xTargetModel->lockControllers();
        pModel->incImportExport();

        xGraphicStorageHandler = createGraphicStorageHandler(
            xContext, u"com.sun.star.comp.Svx.GraphicImportHelper"_ustr);

        ::comphelper::IEmbeddedHelper *pPersist = pModel->GetPersist();
        if( pPersist )
        {
            xObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                        *pPersist,
                                        SvXMLEmbeddedObjectHelperMode::Read );
            xObjectResolver = xObjectHelper.get();
        }

        // parse
        // prepare ParserInputSource
        xml::sax::InputSource aParserInput;
        aParserInput.aInputStream = xInputStream;

        // prepare filter arguments
        Sequence<Any> aFilterArgs( 2 );
        Any *pArgs = aFilterArgs.getArray();
        *pArgs++ <<= xGraphicStorageHandler;
        *pArgs++ <<= xObjectResolver;

        // get filter
        Reference< XInterface > xFilter = xContext->getServiceManager()->createInstanceWithArgumentsAndContext( OUString::createFromAscii( pImportService ), aFilterArgs, xContext);
        SAL_WARN_IF( !xFilter, "svx", "Can't instantiate filter component " << pImportService);
        uno::Reference< xml::sax::XFastParser > xFastParser( xFilter,  UNO_QUERY );
        assert(xFastParser);

        bRet = false;
        if( xFastParser.is() )
        {
            // connect model and filter
            uno::Reference < document::XImporter > xImporter( xFilter, UNO_QUERY );
            xImporter->setTargetDocument( xTargetDocument );

            // finally, parser the stream
            xFastParser->parseStream( aParserInput );

            bRet = true;
        }
    }
    catch( cpo::uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svx");
    }

    disposeGraphicStorageHandler(xGraphicStorageHandler);
    xGraphicStorageHandler = nullptr;

    if( xObjectHelper.is() )
        xObjectHelper->dispose();
    xObjectHelper.clear();
    xObjectResolver = nullptr;

    if ( xTargetModel.is() )
        xTargetModel->unlockControllers();
    pModel->decImportExport();

    return bRet;
}

bool SvxDrawingLayerImport( SdrModel* pModel, const uno::Reference<io::XInputStream>& xInputStream )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
