/*************************************************************************
 *
 *  $RCSfile: xmlwrap.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: nn $ $Date: 2001-04-04 09:19:46 $
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
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif

#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
#endif

#include "document.hxx"
#include "xmlwrap.hxx"
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#define MAP_LEN(x) x, sizeof(x) - 1

using namespace com::sun::star;

// -----------------------------------------------------------------------

ScXMLImportWrapper::ScXMLImportWrapper(ScDocument& rD, SfxMedium* pM, SvStorage* pS) :
    rDoc(rD),
    pMedium(pM),
    pStorage(pS)
{
    DBG_ASSERT( pMedium || pStorage, "ScXMLImportWrapper: Medium or Storage must be set" );
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

sal_Bool ScXMLImportWrapper::ImportFromComponent(uno::Reference<lang::XMultiServiceFactory>& xServiceFactory,
    uno::Reference<frame::XModel>& xModel, uno::Reference<uno::XInterface>& xXMLParser,
    xml::sax::InputSource& aParserInput,
    const rtl::OUString& sComponentName, const rtl::OUString& sDocName,
    const rtl::OUString& sOldDocName, uno::Sequence<uno::Any>& aArgs)
{
    SvStorageStreamRef xDocStream;
    if ( !pStorage && pMedium )
        pStorage = pMedium->GetStorage();

    // Get data source ...

    uno::Reference< uno::XInterface > xPipe;
    uno::Reference< io::XActiveDataSource > xSource;

    if( pStorage )
    {
        if (pStorage->IsStream(sDocName))
            xDocStream = pStorage->OpenStream( sDocName,
                                  STREAM_READ | STREAM_NOCREATE );
        else if (sOldDocName.getLength() && pStorage->IsStream(sOldDocName))
            xDocStream = pStorage->OpenStream( sOldDocName,
                                  STREAM_READ | STREAM_NOCREATE );
        else
            return sal_False;
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

    sal_Bool bRet(sal_True);
    uno::Reference<xml::sax::XDocumentHandler> xDocHandler(
        xServiceFactory->createInstanceWithArguments(
            sComponentName, aArgs ),
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

    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException e )
    {
        bRet = sal_False;
    }
    catch( xml::sax::SAXException e )
    {
        bRet = sal_False;
    }
    catch( io::IOException e )
    {
        bRet = sal_False;
    }
    return bRet;
}

sal_Bool ScXMLImportWrapper::Import(sal_Bool bStylesOnly)
{
    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        comphelper::getProcessServiceFactory();
    DBG_ASSERT( xServiceFactory.is(), "got no service manager" );
    if( !xServiceFactory.is() )
        return sal_False;

    xml::sax::InputSource aParserInput;
    if (pMedium)
        aParserInput.sSystemId = OUString(pMedium->GetName());


    // get parser
    uno::Reference<uno::XInterface> xXMLParser =
        xServiceFactory->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.sax.Parser" )) );
    DBG_ASSERT( xXMLParser.is(), "com.sun.star.xml.sax.Parser service missing" );
    if( !xXMLParser.is() )
        return sal_False;

    // get filter
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    if ( pObjSh )
    {
        rtl::OUString sEmpty;
        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();

        if(!bStylesOnly)
        {
            uno::Sequence<uno::Any> aMetaArgs(0);

            sal_Bool bMetaRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Meta.xml")), aMetaArgs);
        }

        uno::Sequence<uno::Any> aStylesArgs(0);

        sal_Bool bStylesRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesImporter")),
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
            sEmpty, aStylesArgs);

        sal_Bool bDocRetval(sal_False);
        if (!bStylesOnly)
        {
            SvXMLGraphicHelper* pGraphicHelper = NULL;
            uno::Reference< document::XGraphicObjectResolver > xGrfContainer;

            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            SvXMLEmbeddedObjectHelper *pObjectHelper = NULL;

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

            uno::Sequence<uno::Any> aDocArgs(3);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xGrfContainer;
            pDocArgs[1] <<= GetStatusIndicator(xModel);
            pDocArgs[2] <<= xObjectResolver;

            bDocRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Content.xml")), aDocArgs);

            if( pGraphicHelper )
                SvXMLGraphicHelper::Destroy( pGraphicHelper );

            if( pObjectHelper )
                SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
        }

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aSettingsArgs(0);

            sal_Bool bSettingsRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
                sEmpty, aSettingsArgs);
        }

        // Don't test bStylesRetval and bMetaRetval, because it could be an older file which not contain such streams
        return !bStylesOnly ? bDocRetval : bStylesRetval;
    }
    return sal_False;
}

sal_Bool ScXMLImportWrapper::ExportToComponent(uno::Reference<lang::XMultiServiceFactory>& xServiceFactory,
    uno::Reference<frame::XModel>& xModel, uno::Reference<uno::XInterface>& xWriter,
    uno::Sequence<beans::PropertyValue>& aDescriptor, const rtl::OUString& sName,
    const rtl::OUString& sMediaType, const rtl::OUString& sComponentName,
    const sal_Bool bCompress, uno::Sequence<uno::Any>& aArgs, ScMySharedData*& pSharedData)
{
    sal_Bool bRet(sal_False);
    uno::Reference<io::XOutputStream> xOut;
    SvStorageStreamRef xStream;

    if( pStorage )
    {
        xStream = pStorage->OpenStream( sName,
                                STREAM_WRITE | STREAM_SHARE_DENYWRITE );
        uno::Any aAny; aAny <<= sMediaType;
        xStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);
        aAny = ::cppu::bool2any(bCompress);
        xStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed")), aAny);
        xStream->SetBufferSize( 16*1024 );
        xOut = new utl::OOutputStreamWrapper( *xStream );
    }
    else if ( pMedium )
    {
        xOut = pMedium->GetDataSink();
    }

    uno::Reference<io::XActiveDataSource> xSrc( xWriter, uno::UNO_QUERY );
    xSrc->setOutputStream( xOut );


    uno::Reference<document::XFilter> xFilter(
        xServiceFactory->createInstanceWithArguments( sComponentName , aArgs ),
            uno::UNO_QUERY );
    DBG_ASSERT( xFilter.is(), "can't get exporter" );
    uno::Reference<document::XExporter> xExporter( xFilter, uno::UNO_QUERY );
    uno::Reference<lang::XComponent> xComponent( xModel, uno::UNO_QUERY );
    if (xExporter.is())
        xExporter->setSourceDocument( xComponent );

    if ( xFilter.is() )
    {
        ScXMLExport* pExport = static_cast<ScXMLExport*>(SvXMLExport::getImplementation(xFilter));
        pExport->SetSharedData(pSharedData);
        bRet = xFilter->filter( aDescriptor );
        pSharedData = pExport->GetSharedData();

        if (xStream.Is())
            xStream->Commit();
    }
    return bRet;
}

sal_Bool ScXMLImportWrapper::Export(sal_Bool bStylesOnly)
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
    OUString sTextMediaType(RTL_CONSTASCII_USTRINGPARAM("text/xml"));
    if (pMedium)
        sFileName = pMedium->GetName();
    SfxObjectShell* pObjSh = rDoc.GetDocumentShell();
    uno::Sequence<beans::PropertyValue> aDescriptor(1);
    beans::PropertyValue* pProps = aDescriptor.getArray();
    pProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ) );
    pProps[0].Value <<= sFileName;

    /** property map for export info set */
    comphelper::PropertyMapEntry aExportInfoMap[] =
    {
        { MAP_LEN( "ProgressRange" ), 0, SEQTYPE(::getCppuType((sal_Int32*)0)),     ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
        { MAP_LEN( "ProgressMax" ), 0, SEQTYPE(::getCppuType((sal_Int32*)0)),   ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
        { MAP_LEN( "ProgressCurrent" ), 0, SEQTYPE(::getCppuType((sal_Int32*)0)),   ::com::sun::star::beans::PropertyAttribute::MAYBEVOID,     0},
        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    if ( pObjSh && pStorage)
    {
        pObjSh->UpdateDocInfoForSave();     // update information

        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();
        uno::Reference<task::XStatusIndicator> xStatusIndicator = GetStatusIndicator(xModel);
        sal_Int32 nProgressRange(1000000);
        if(xStatusIndicator.is())
            xStatusIndicator->start(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Export XML")), nProgressRange);
        uno::Any aProgRange;
        aProgRange <<= nProgressRange;
        xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);

        sal_Bool bMetaRet(sal_False);
        sal_Bool bStylesRet (sal_False);
        sal_Bool bDocRet(sal_False);
        sal_Bool bSettingsRet(sal_False);
        ScMySharedData* pSharedData = NULL;

        // meta export
        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aMetaArgs(3);
            uno::Any* pMetaArgs = aMetaArgs.getArray();
            pMetaArgs[0] <<= xHandler;
            pMetaArgs[1] <<= xStatusIndicator;
            pMetaArgs[2] <<= xInfoSet;
            bMetaRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
                sTextMediaType, rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaExporter")),
                sal_False, aMetaArgs, pSharedData);
        }

        // styles export

        {
            uno::Sequence<uno::Any> aStylesArgs(3);
            uno::Any* pStylesArgs = aStylesArgs.getArray();
            pStylesArgs[0] <<= xHandler;
            pStylesArgs[1] <<= xStatusIndicator;
            pStylesArgs[2] <<= xInfoSet;
            bStylesRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
                sTextMediaType, rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesExporter")),
                sal_True, aStylesArgs, pSharedData);
        }

        // content export

        if (!bStylesOnly)
        {
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

            uno::Sequence<uno::Any> aDocArgs(5);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xGrfContainer;
            pDocArgs[1] <<= xStatusIndicator;
            pDocArgs[2] <<= xHandler;
            pDocArgs[3] <<= xObjectResolver;
            pDocArgs[4] <<= xInfoSet;

            bDocRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("content.xml")),
                sTextMediaType, rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentExporter")),
                sal_True, aDocArgs, pSharedData);

            if( pGraphicHelper )
                SvXMLGraphicHelper::Destroy( pGraphicHelper );

            if( pObjectHelper )
                SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
        }

        // settings export

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aSettingsArgs(3);
            uno::Any* pSettingsArgs = aSettingsArgs.getArray();
            pSettingsArgs[0] <<= xHandler;
            pSettingsArgs[1] <<= xStatusIndicator;
            pSettingsArgs[2] <<= xInfoSet;
            bSettingsRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
                sTextMediaType, rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsExporter")),
                sal_True, aSettingsArgs, pSharedData);
        }

        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return bStylesRet && ((!bStylesOnly && bDocRet && bMetaRet && bSettingsRet) || bStylesOnly);
    }

    // later: give string descriptor as parameter for doc type

    return sal_False;
}



