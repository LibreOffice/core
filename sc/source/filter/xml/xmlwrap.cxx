/*************************************************************************
 *
 *  $RCSfile: xmlwrap.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-27 16:06:29 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <rsc/rscsfx.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#include <vos/xception.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/streamwrap.hxx>
#include <xmloff/xmlkywd.hxx>
#include <svx/xmlgrhlp.hxx>

#include <com/sun/star/xml/sax/XErrorHandler.hpp>
#include <com/sun/star/xml/sax/XEntityResolver.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/XDTDHandler.hpp>
#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#include <com/sun/star/frame/XModel.hpp>
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
#endif

#include "document.hxx"
#include "xmlwrap.hxx"
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"

using namespace com::sun::star;

// -----------------------------------------------------------------------

ScXMLImportWrapper::ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, SvStorage* pS) :
    rDoc(rD),
    pMedium(pM),
    pStorage(pS)
{
    DBG_ASSERT( pMedium || pStorage, "ScXMLImportWrapper: Medium or Storage must be set" );
}

sal_Bool ScXMLImportWrapper::Import()
{
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( !xServiceFactory.is() )
        return sal_False;

    // Get data source ...

    uno::Reference< io::XActiveDataSource > xSource;
    uno::Reference< uno::XInterface > xPipe;

    xml::sax::InputSource aParserInput;
    if (pMedium)
        aParserInput.sSystemId = OUString(pMedium->GetName());

    SvStorageStreamRef xDocStream;
    if ( !pStorage && pMedium )
        pStorage = pMedium->GetStorage();

    if( pStorage )
    {
        OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
        xDocStream = pStorage->OpenStream( sDocName,
                                  STREAM_READ | STREAM_NOCREATE );
        xDocStream->SetBufferSize( 16*1024 );
        aParserInput.aInputStream = new utl::OInputStreamWrapper( *xDocStream );
    }
    else if ( pMedium )
    {
        // if there is a medium and if this medium has a load environment,
        // we get an active data source from the medium.
        pMedium->GetInStream()->Seek( 0 );
        xSource = pMedium->GetDataSource();
        DBG_ASSERT( xSource.is(), "got no data source from medium" );
        if( !xSource.is() )
            return sal_False;

        // get a pipe for connecting the data source to the parser
        xPipe = xServiceFactory->createInstance(
                OUString::createFromAscii("com.sun.star.io.Pipe") );
        DBG_ASSERT( xPipe.is(),
                "XMLReader::Read: com.sun.star.io.Pipe service missing" );
        if( !xPipe.is() )
            return sal_False;

        // connect pipe's output stream to the data source
        uno::Reference<io::XOutputStream> xPipeOutput( xPipe, uno::UNO_QUERY );
        xSource->setOutputStream( xPipeOutput );

        aParserInput.aInputStream =
            uno::Reference< io::XInputStream >( xPipe, uno::UNO_QUERY );
    }
    else
        return sal_False;

    // get parser
    uno::Reference<uno::XInterface> xXMLParser =
        xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" )) );
    DBG_ASSERT( xXMLParser.is(), "com.sun.star.xml.sax.Parser service missing" );
    if( !xXMLParser.is() )
        return sal_False;


    sal_uInt16 nStyleFamilyMask(0);
//  USHORT nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
    sal_Bool bLoadDoc(sal_True);
//  BOOL bInsert;
//  if( aOpt.IsFmtsOnly() )
//  {
//      bLoadDoc = FALSE;
//      bInsert = aOpt.IsMerge();
//      nStyleFamilyMask = 0U;
//      if( aOpt.IsFrmFmts() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_FRAME;
//      if( aOpt.IsPageDescs() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_PAGE;
//      if( aOpt.IsTxtFmts() )
//          nStyleFamilyMask |= (SFX_STYLE_FAMILY_CHAR|SFX_STYLE_FAMILY_PARA);
//      if( aOpt.IsNumRules() )
//          nStyleFamilyMask |= SFX_STYLE_FAMILY_PSEUDO;
//  }
//  else
//  {
//      bLoadDoc = TRUE;
//      bInsert = bInsertMode;
//      nStyleFamilyMask = SFX_STYLE_FAMILY_ALL;
//  }
//  aOpt.ResetAllFmtsOnly();

    // get filter
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    if ( pObjSh )
    {
        SvXMLGraphicHelper* pGraphicHelper = NULL;
        uno::Reference< document::XGraphicObjectResolver > xGrfContainer;

        uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
        SvXMLEmbeddedObjectHelper *pObjectHelper = NULL;

        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();


        if( pStorage )
        {
            pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_READ );
            xGrfContainer = pGraphicHelper;

            SvPersist *pPersist = pObjSh;
            if( pPersist )
            {
                pObjectHelper = SvXMLEmbeddedObjectHelper::Create(*pStorage, *pPersist, EMBEDDEDOBJECTHELPER_MODE_READ, sal_False );
                xObjectResolver = pObjectHelper;
            }
        }

        uno::Reference<task::XStatusIndicator> xStatusIndicator;
        if (xModel.is())
        {
            uno::Reference<frame::XController> xController( xModel->getCurrentController());
            if( xController.is())
            {
                uno::Reference<frame::XFrame> xFrame( xController->getFrame());
                if( xFrame.is())
                {
                    uno::Reference<task::XStatusIndicatorFactory> xFactory( xFrame, uno::UNO_QUERY );
                    if( xFactory.is())
                    {
                        xStatusIndicator = xFactory->createStatusIndicator();
                    }
                }
            }
        }

        uno::Sequence<uno::Any> aArgs(3);
        uno::Any* pArgs = aArgs.getArray();
        pArgs[0] <<= xGrfContainer;
        pArgs[1] <<= xStatusIndicator;
        pArgs[2] <<= xObjectResolver;

        uno::Reference<xml::sax::XDocumentHandler> xDocHandler(
            xServiceFactory->createInstanceWithArguments(
                OUString::createFromAscii( "com.sun.star.office.sax.importer.Calc" ), aArgs ),
            uno::UNO_QUERY );
        DBG_ASSERT( xDocHandler.is(), "can't get Calc importer" );
        uno::Reference<document::XImporter> xImporter( xDocHandler, uno::UNO_QUERY );
        uno::Reference<lang::XComponent> xComponent( xModel, uno::UNO_QUERY );
        if (xImporter.is())
            xImporter->setTargetDocument( xComponent );

        // connect parser and filter
        uno::Reference<xml::sax::XParser> xParser( xXMLParser, uno::UNO_QUERY );
        xParser->setDocumentHandler( xDocHandler );

        // parse
        if( xSource.is() )
        {
            uno::Reference<io::XActiveDataControl> xSourceControl( xSource, uno::UNO_QUERY );
            if( xSourceControl.is() )
                xSourceControl->start();
        }

        sal_Bool bRetval(sal_True);

        try
        {
            xParser->parseStream( aParserInput );
        }
        catch( xml::sax::SAXParseException e )
        {
            bRetval = sal_False;
        }
        catch( xml::sax::SAXException e )
        {
            bRetval = sal_False;
        }
        catch( io::IOException e )
        {
            bRetval = sal_False;
        }

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );

        if( pObjectHelper )
            SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );

        return bRetval;
    }
    return sal_False;
}

uno::Reference <task::XStatusIndicator> ScXMLImportWrapper::GetStatusIndicator(
    uno::Reference < frame::XModel> & rModel)
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    if (rModel.is())
    {
        uno::Reference<frame::XController> xController( rModel->getCurrentController());
        if( xController.is())
        {
            uno::Reference<frame::XFrame> xFrame( xController->getFrame());
            if( xFrame.is())
            {
                uno::Reference<task::XStatusIndicatorFactory> xFactory( xFrame, uno::UNO_QUERY );
                if( xFactory.is())
                {
                    return xFactory->createStatusIndicator();
                }
            }
        }
    }
    return xStatusIndicator;
}

sal_Bool ScXMLImportWrapper::Export()
{
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( !xServiceFactory.is() )
        return sal_False;

    uno::Reference<uno::XInterface> xWriter =
        xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Writer" )) );
    DBG_ASSERT( xWriter.is(), "com.sun.star.xml.sax.Writer service missing" );
    if(!xWriter.is())
        return sal_False;

    if ( !pStorage && pMedium )
        pStorage = pMedium->GetOutputStorage( sal_True );

    uno::Reference<xml::sax::XDocumentHandler> xHandler( xWriter, uno::UNO_QUERY );

    OUString sFileName;
    if (pMedium)
        sFileName = pMedium->GetName();
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    uno::Sequence<beans::PropertyValue> aDescriptor(1);
    beans::PropertyValue* pProps = aDescriptor.getArray();
    pProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
    pProps[0].Value <<= sFileName;
    if ( pObjSh && pStorage)
    {
        pObjSh->UpdateDocInfoForSave();     // update information

        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();

        sal_Bool bMetaRet = sal_False;
        sal_Bool bDocRet = sal_False;

        // meta export

        {
            uno::Reference<io::XOutputStream> xMetaOut;
            SvStorageStreamRef xMetaStream;

            if( pStorage )
            {
                OUString sMetaName( RTL_CONSTASCII_USTRINGPARAM( "Meta.xml" ) );
                xMetaStream = pStorage->OpenStream( sMetaName,
                                          STREAM_WRITE | STREAM_SHARE_DENYWRITE );
                uno::Any aAny = ::cppu::bool2any(sal_False);
                xMetaStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed")), aAny);
                xMetaStream->SetBufferSize( 16*1024 );
                xMetaOut = new utl::OOutputStreamWrapper( *xMetaStream );
            }
            else if ( pMedium )
            {
                xMetaOut = pMedium->GetDataSink();
            }

            uno::Reference<io::XActiveDataSource> xMetaSrc( xWriter, uno::UNO_QUERY );
            xMetaSrc->setOutputStream( xMetaOut );

            uno::Sequence<uno::Any> aMetaArgs(1);
            uno::Any* pMetaArgs = aMetaArgs.getArray();
            pMetaArgs[0] <<= xHandler;

            uno::Reference<document::XFilter> xMetaFilter(
                xServiceFactory->createInstanceWithArguments(
                    OUString::createFromAscii( "com.sun.star.office.sax.exporter.MetaInformation" ), aMetaArgs ),
                uno::UNO_QUERY );
            DBG_ASSERT( xMetaFilter.is(), "can't get Meta exporter" );
            uno::Reference<document::XExporter> xMetaExporter( xMetaFilter, uno::UNO_QUERY );
            uno::Reference<lang::XComponent> xMetaComponent( xModel, uno::UNO_QUERY );
            if (xMetaExporter.is())
                xMetaExporter->setSourceDocument( xMetaComponent );

            if ( xMetaFilter.is() )
            {
                bMetaRet = xMetaFilter->filter( aDescriptor );

                if (xMetaStream.Is())
                    xMetaStream->Commit();
            }
        }

        // doc export

        {
            uno::Reference<io::XOutputStream> xDocOut;
            SvStorageStreamRef xDocStream;

            if( pStorage )
            {
                OUString sDocName( RTL_CONSTASCII_USTRINGPARAM( "Content.xml" ) );
                xDocStream = pStorage->OpenStream( sDocName,
                                          STREAM_WRITE | STREAM_SHARE_DENYWRITE );
                rtl::OUString sType(RTL_CONSTASCII_USTRINGPARAM("text/xml"));
                uno::Any aAny; aAny <<= sType;
                xDocStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);
                xDocStream->SetBufferSize( 16*1024 );
                xDocOut = new utl::OOutputStreamWrapper( *xDocStream );
            }
            else if ( pMedium )
            {
                xDocOut = pMedium->GetDataSink();
            }

            uno::Reference<io::XActiveDataSource> xDocSrc( xWriter, uno::UNO_QUERY );
            xDocSrc->setOutputStream( xDocOut );

            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

            uno::Reference< document::XGraphicObjectResolver > xGrfContainer;
            SvXMLGraphicHelper* pGraphicHelper;

            if( pStorage )
            {
                pGraphicHelper = SvXMLGraphicHelper::Create( *pStorage, GRAPHICHELPER_MODE_WRITE, FALSE );
                xGrfContainer = pGraphicHelper;
            }

            SvPersist *pPersist = pObjSh;
            if( pPersist )
            {
                pObjectHelper = SvXMLEmbeddedObjectHelper::Create( *pStorage, *pPersist, EMBEDDEDOBJECTHELPER_MODE_WRITE, sal_False );
                xObjectResolver = pObjectHelper;
            }

            uno::Sequence<uno::Any> aDocArgs(4);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xGrfContainer;
            pDocArgs[1] <<= GetStatusIndicator(xModel);
            pDocArgs[2] <<= xHandler;
            pDocArgs[3] <<= xObjectResolver;

            uno::Reference<document::XFilter> xDocFilter(
                xServiceFactory->createInstanceWithArguments(
                    OUString::createFromAscii( "com.sun.star.office.sax.exporter.Calc" ), aDocArgs ),
                uno::UNO_QUERY );
            DBG_ASSERT( xDocFilter.is(), "can't get Calc exporter" );
            uno::Reference<document::XExporter> xDocExporter( xDocFilter, uno::UNO_QUERY );
            uno::Reference<lang::XComponent> xDocComponent( xModel, uno::UNO_QUERY );
            if (xDocExporter.is())
                xDocExporter->setSourceDocument( xDocComponent );

            if ( xDocFilter.is() )
            {
                bDocRet = xDocFilter->filter( aDescriptor );

                if (xDocStream.Is())
                    xDocStream->Commit();
            }

            if( pGraphicHelper )
                SvXMLGraphicHelper::Destroy( pGraphicHelper );

            if( pObjectHelper )
                SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
        }

        return bDocRet && bMetaRet;
    }

    // later: give string descriptor as parameter for doc type

    return sal_False;
}



