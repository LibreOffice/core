/*************************************************************************
 *
 *  $RCSfile: xmlexport.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:32:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_SAXPARSEEXCEPTION_HPP_
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XFILTER_HPP_
#include <com/sun/star/document/XFilter.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _UTL_STREAM_WRAPPER_HXX_
#include <unotools/streamwrap.hxx>
#endif

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svdmodel.hxx>
#endif

#ifndef _XMLEOHLP_HXX
#include <xmleohlp.hxx>
#endif
#ifndef _XMLGRHLP_HXX
#include <xmlgrhlp.hxx>
#endif

#include "unomodel.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

sal_Bool SvxDrawingLayerExport( SdrModel* pModel, uno::Reference<io::XOutputStream> xOut, Reference< lang::XComponent > xComponent )
{
    return SvxDrawingLayerExport( pModel, xOut, xComponent, "com.sun.star.comp.DrawingLayer.XMLExporter" );
}

sal_Bool SvxDrawingLayerExport( SdrModel* pModel, uno::Reference<io::XOutputStream> xOut, Reference< lang::XComponent > xComponent, const char* pExportService )
{
    sal_Bool bDocRet = xOut.is();

    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    try
    {
        if( !xComponent.is() )
        {
            xComponent = new SvxUnoDrawingModel( pModel );
            pModel->setUnoModel( Reference< XInterface >::query( xComponent ) );
        }

        uno::Reference< lang::XMultiServiceFactory> xServiceFactory( ::comphelper::getProcessServiceFactory() );
        if( !xServiceFactory.is() )
        {
            DBG_ERROR( "got no service manager" );
            bDocRet = sal_False;
        }

        if( bDocRet )
        {
            uno::Reference< uno::XInterface > xWriter( xServiceFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" ) ) ) );
            if( !xWriter.is() )
            {
                DBG_ERROR( "com.sun.star.xml.sax.Writer service missing" );
                bDocRet = sal_False;
            }

            // init resolver
            SvPersist *pPersist = pModel->GetPersist();
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
                    DBG_ERROR( "com.sun.star.comp.Draw.XMLExporter service missing" );
                    bDocRet = sal_False;
                }

                if( bDocRet )
                {
                    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
                    if( xExporter.is() )
                    {
                        xExporter->setSourceDocument( xComponent );

                        uno::Sequence< beans::PropertyValue > aDescriptor( 0 );
                        bDocRet = xFilter->filter( aDescriptor );
                    }
                }
            }
        }
    }
    catch(uno::Exception e)
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "uno Exception caught while exporting:\n" );
        aError += ByteString( String( e.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
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

sal_Bool SvxDrawingLayerExport( SdrModel* pModel, uno::Reference<io::XOutputStream> xOut )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerExport( pModel, xOut, xComponent );
}

//-////////////////////////////////////////////////////////////////////

sal_Bool SvxDrawingLayerImport( SdrModel* pModel, uno::Reference<io::XInputStream> xInputStream, Reference< lang::XComponent > xComponent )
{
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent, "com.sun.star.comp.Draw.XMLImporter" );
}

sal_Bool SvxDrawingLayerImport( SdrModel* pModel, uno::Reference<io::XInputStream> xInputStream, Reference< lang::XComponent > xComponent, const char* pImportService  )
{
    sal_uInt32  nRet = 0;

    Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;

    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    if( !xComponent.is() )
    {
        xComponent = new SvxUnoDrawingModel( pModel );
        pModel->setUnoModel( Reference< XInterface >::query( xComponent ) );
    }

    Reference< frame::XModel > xModel( xComponent, UNO_QUERY );

    try
    {
        // Get service factory
        Reference< lang::XMultiServiceFactory > xServiceFactory = comphelper::getProcessServiceFactory();
        DBG_ASSERT( xServiceFactory.is(), "XMLReader::Read: got no service manager" );

        if( !xServiceFactory.is() )
            nRet = 1;

        if( 0 == nRet )
        {
            xModel->lockControllers();

            // -------------------------------------

            pGraphicHelper = SvXMLGraphicHelper::Create( GRAPHICHELPER_MODE_READ );
            xGraphicResolver = pGraphicHelper;

            SvPersist *pPersist = pModel->GetPersist();
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
            Reference< xml::sax::XParser > xParser( xServiceFactory->createInstance( OUString::createFromAscii("com.sun.star.xml.sax.Parser") ), UNO_QUERY );
            DBG_ASSERT( xParser.is(), "Can't create parser" );

            // prepare filter arguments
            Sequence<Any> aFilterArgs( 2 );
            Any *pArgs = aFilterArgs.getArray();
            *pArgs++ <<= xGraphicResolver;
            *pArgs++ <<= xObjectResolver;

            // get filter
            Reference< xml::sax::XDocumentHandler > xFilter( xServiceFactory->createInstanceWithArguments( OUString::createFromAscii( pImportService ), aFilterArgs), UNO_QUERY );
            DBG_ASSERT( xFilter.is(), "Can't instantiate filter component." );

            if( !xParser.is() || !xFilter.is() )
            {
                nRet = 1;
            }
            else
            {
                // connect parser and filter
                xParser->setDocumentHandler( xFilter );

                // connect model and filter
                uno::Reference < document::XImporter > xImporter( xFilter, UNO_QUERY );
                xImporter->setTargetDocument( xComponent );

                // finally, parser the stream
                xParser->parseStream( aParserInput );
            }
        }
    }
    catch( xml::sax::SAXParseException& r )
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "SAX parse exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
    }
    catch( xml::sax::SAXException& r )
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "SAX exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
    }
    catch( io::IOException& r )
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "IO exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
    }
    catch( uno::Exception& r )
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "uno exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    if( xModel.is() )
        xModel->unlockControllers();

    return nRet == 0;
}

sal_Bool SvxDrawingLayerImport( SdrModel* pModel, uno::Reference<io::XInputStream> xInputStream )
{
    Reference< lang::XComponent > xComponent;
    return SvxDrawingLayerImport( pModel, xInputStream, xComponent );
}
