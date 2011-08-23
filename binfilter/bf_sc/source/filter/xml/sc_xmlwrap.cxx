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

#ifdef _MSC_VER
#pragma hdrstop
#endif

// INCLUDE ---------------------------------------------------------------

#include <bf_sfx2/docfile.hxx>
#include <bf_sfx2/objsh.hxx>
#include <unotools/streamwrap.hxx>
#include <bf_svx/xmlgrhlp.hxx>
#include <bf_svtools/sfxecode.hxx>
#include <bf_sfx2/appuno.hxx>
#include <bf_svtools/itemset.hxx>
#include <bf_sfx2/sfxsids.hrc>

#include <com/sun/star/xml/sax/XParser.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>

#include <bf_svx/xmleohlp.hxx>
#include <rtl/logfile.hxx>
#include <bf_svtools/saveopt.hxx>

#include "document.hxx"
#include "xmlwrap.hxx"
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"
#include "globstr.hrc"
#include "scerrors.hxx"
#include "XMLExportSharedData.hxx"
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#ifndef SEQTYPE
namespace binfilter {
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#define MAP_LEN(x) x, sizeof(x) - 1

using namespace ::com::sun::star;

using rtl::OUString;

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
                    try
                    {
                        xStatusIndicator = xFactory->createStatusIndicator();
                    }
                    catch( lang::DisposedException e )
                    {
                        DBG_ERROR("Exception while trying to get a Status Indicator");
                    }
                }
            }
        }
    }
    return xStatusIndicator;
}

uno::Reference <task::XStatusIndicator> ScXMLImportWrapper::GetStatusIndicator()
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    if (pMedium)
    {
        SfxItemSet* pSet = pMedium->GetItemSet();
        if (pSet)
        {
            const SfxUnoAnyItem* pItem = static_cast<const SfxUnoAnyItem*>(pSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL));
            if (pItem)
            {
                uno::Any aAny(pItem->GetValue());
                aAny >>= xStatusIndicator;
            }
        }
    }
    return xStatusIndicator;
}

sal_uInt32 ScXMLImportWrapper::ImportFromComponent(uno::Reference<lang::XMultiServiceFactory>& xServiceFactory,
    uno::Reference<frame::XModel>& xModel, uno::Reference<uno::XInterface>& xXMLParser,
    xml::sax::InputSource& aParserInput,
    const ::rtl::OUString& sComponentName, const ::rtl::OUString& sDocName,
    const ::rtl::OUString& sOldDocName, uno::Sequence<uno::Any>& aArgs,
    sal_Bool bMustBeSuccessfull)
{
    SvStorageStreamRef xDocStream;
    if ( !pStorage && pMedium )
        pStorage = pMedium->GetStorage();

    // Get data source ...

    uno::Reference< uno::XInterface > xPipe;
    uno::Reference< io::XActiveDataSource > xSource;

    sal_Bool bEncrypted = sal_False;
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
        aParserInput.aInputStream = xDocStream->GetXInputStream();

        uno::Any aAny;
        bEncrypted = xDocStream->GetProperty(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("Encrypted") ), aAny ) &&
                        aAny.getValueType() == ::getBooleanCppuType() &&
                        *(sal_Bool *)aAny.getValue();
    }
    // #99667#; no longer necessary
/*	else if ( pMedium )
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
    }*/
    else
        return SCERR_IMPORT_UNKNOWN;

    sal_uInt32 nReturn(0);
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

    sal_Bool bFormatError = sal_False;
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& r )
    {
        if( bEncrypted )
            nReturn = ERRCODE_SFX_WRONGPASSWORD;
        else
        {

#ifdef DBG_UTIL
            ByteString aError( "SAX parse exception catched while importing:\n" );
            aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aError.GetBuffer() );
#endif

            String sErr( String::CreateFromInt32( r.LineNumber ));
            sErr += ',';
            sErr += String::CreateFromInt32( r.ColumnNumber );

            if( sDocName.getLength() )
            {
                nReturn = *new TwoStringErrorInfo(
                                (bMustBeSuccessfull ? SCERR_IMPORT_FILE_ROWCOL
                                                        : SCWARN_IMPORT_FILE_ROWCOL),
                                sDocName, sErr,
                                ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
            }
            else
            {
                DBG_ASSERT( bMustBeSuccessfull, "Warnings are not supported" );
                nReturn = *new StringErrorInfo( SCERR_IMPORT_FORMAT_ROWCOL, sErr,
                                 ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
            }
        }
    }
    catch( xml::sax::SAXException& r )
    {
        if( bEncrypted )
            nReturn = ERRCODE_SFX_WRONGPASSWORD;
        else
        {

#ifdef DBG_UTIL
            ByteString aError( "SAX exception catched while importing:\n" );
            aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
            DBG_ERROR( aError.GetBuffer() );
#endif
            nReturn = SCERR_IMPORT_FORMAT;
        }
    }
    catch( packages::zip::ZipIOException& r )
    {
#ifdef DBG_UTIL
        ByteString aError( "Zip exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        nReturn = ERRCODE_IO_BROKENPACKAGE;
    }
    catch( io::IOException& r )
    {
#ifdef DBG_UTIL
        ByteString aError( "IO exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        nReturn = SCERR_IMPORT_OPEN;
    }
    catch( uno::Exception& r )
    {
#ifdef DBG_UTIL
        ByteString aError( "uno exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        DBG_ERROR( aError.GetBuffer() );
#endif
        nReturn = SCERR_IMPORT_UNKNOWN;
    }

    if ( xDocHandler.is() )
    {
        ScXMLImport* pImport = static_cast<ScXMLImport*>(SvXMLImport::getImplementation(xDocHandler));

        if (pImport && pImport->HasRangeOverflow() && !nReturn)
            nReturn = pImport->GetRangeOverflowType();
    }

    // free the component
    xParser->setDocumentHandler( NULL );

    // success!
    return nReturn;
}

sal_Bool ScXMLImportWrapper::Import(sal_Bool bStylesOnly)
{
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScXMLImportWrapper::Import" );

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        ::legacy_binfilters::getLegacyProcessServiceFactory();
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
        ::rtl::OUString sEmpty;
        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();

        /** property map for export info set */
        ::comphelper::PropertyMapEntry aImportInfoMap[] =
        {
            { MAP_LEN( "ProgressRange" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressMax" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "NumberStyles" ), 0, &::getCppuType((uno::Reference<container::XNameAccess> *)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { NULL, 0, 0, NULL, 0, 0 }
        };
        uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aImportInfoMap ) ) );

        uno::Reference<task::XStatusIndicator> xStatusIndicator(GetStatusIndicator());
        if (xStatusIndicator.is())
        {
            sal_Int32 nProgressRange(1000000);
            xStatusIndicator->start(::rtl::OUString(ScGlobal::GetRscString(STR_LOAD_DOC)), nProgressRange);
            uno::Any aProgRange;
            aProgRange <<= nProgressRange;
            xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);
        }

        sal_uInt32 nMetaRetval(0);
        if(!bStylesOnly)
        {
            uno::Sequence<uno::Any> aMetaArgs(1);
            uno::Any* pMetaArgs = aMetaArgs.getArray();
            pMetaArgs[0] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta import start" );

            nMetaRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaImporter")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Meta.xml")), aMetaArgs,
                sal_False);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta import end" );
        }

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
        uno::Sequence<uno::Any> aStylesArgs(4);
        uno::Any* pStylesArgs = aStylesArgs.getArray();
        pStylesArgs[0] <<= xGrfContainer;
        pStylesArgs[1] <<= xStatusIndicator;
        pStylesArgs[2] <<= xObjectResolver;
        pStylesArgs[3] <<= xInfoSet;

        sal_uInt32 nSettingsRetval(0);
        if (!bStylesOnly)
        {
            //	Settings must be loaded first because of the printer setting,
            //	which is needed in the page styles (paper tray).

            uno::Sequence<uno::Any> aSettingsArgs(0);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings import start" );

            nSettingsRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsImporter")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
                sEmpty, aSettingsArgs, sal_False);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings import end" );
        }

        sal_uInt32 nStylesRetval(0);
        {
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles import start" );

            nStylesRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesImporter")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
                sEmpty, aStylesArgs, sal_True);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles import end" );
        }

        sal_uInt32 nDocRetval(0);
        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aDocArgs(4);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xGrfContainer;
            pDocArgs[1] <<= xStatusIndicator;
            pDocArgs[2] <<= xObjectResolver;
            pDocArgs[3] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "content import start" );

            nDocRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentImporter")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")),
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Content.xml")), aDocArgs,
                sal_True);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "content import end" );
        }
        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );

        if( pObjectHelper )
            SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );

        if (xStatusIndicator.is())
            xStatusIndicator->end();

        sal_Bool bRet(sal_False);
        if (bStylesOnly)
        {
            if (nStylesRetval)
                pStorage->SetError(nStylesRetval);
            else
                bRet = sal_True;
        }
        else
        {
            if (nDocRetval)
            {
                pStorage->SetError(nDocRetval);
                if (nDocRetval == SCWARN_IMPORT_RANGE_OVERFLOW ||
                    nDocRetval == SCWARN_IMPORT_ROW_OVERFLOW ||
                    nDocRetval == SCWARN_IMPORT_COLUMN_OVERFLOW ||
                    nDocRetval == SCWARN_IMPORT_SHEET_OVERFLOW)
                    bRet = sal_True;
            }
            else if (nStylesRetval)
                pStorage->SetError(nStylesRetval);
            else if (nMetaRetval)
                pStorage->SetError(nMetaRetval);
            else if (nSettingsRetval)
                pStorage->SetError(nSettingsRetval);
            else
                bRet = sal_True;
        }

        // Don't test bStylesRetval and bMetaRetval, because it could be an older file which not contain such streams
        return bRet;//!bStylesOnly ? bDocRetval : bStylesRetval;
    }
    return sal_False;
}

sal_Bool ScXMLImportWrapper::ExportToComponent(uno::Reference<lang::XMultiServiceFactory>& xServiceFactory,
    uno::Reference<frame::XModel>& xModel, uno::Reference<uno::XInterface>& xWriter,
    uno::Sequence<beans::PropertyValue>& aDescriptor, const ::rtl::OUString& sName,
    const ::rtl::OUString& sMediaType, const ::rtl::OUString& sComponentName,
    const sal_Bool bPlainText, uno::Sequence<uno::Any>& aArgs, ScMySharedData*& pSharedData)
{
    sal_Bool bRet(sal_False);
    uno::Reference<io::XOutputStream> xOut;
    SvStorageStreamRef xStream;

    if( pStorage )
    {
        // #96807#; trunc stream before use, because it could be an existing stream
        // and the new content could be shorter than the old content. In this case
        // would not all be over written by the new content and the xml file
        // would not be valid.
        xStream = pStorage->OpenStream( sName,
                                STREAM_WRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC );
        uno::Any aAny; aAny <<= sMediaType;
        xStream->SetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);
        if (bPlainText)
        {
            aAny = ::cppu::bool2any(sal_False);
            xStream->SetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed")), aAny);
        }
        else
        {
            aAny = ::cppu::bool2any(sal_True);
            xStream->SetProperty(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted")), aAny);
        }
        xStream->SetBufferSize( 16*1024 );
        xOut = new ::utl::OOutputStreamWrapper( *xStream );
    }
    // #99667#; no longer necessary
/*	else if ( pMedium )
    {
        xOut = pMedium->GetDataSink();
    }*/

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
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScXMLImportWrapper::Export" );

    uno::Reference<lang::XMultiServiceFactory> xServiceFactory =
                                        ::legacy_binfilters::getLegacyProcessServiceFactory();
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
    ::comphelper::PropertyMapEntry aExportInfoMap[] =
    {
        { MAP_LEN( "ProgressRange" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "ProgressMax" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "WrittenNumberStyles" ), 0, &::getCppuType((uno::Sequence<sal_Int32>*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "UsePrettyPrinting" ), 0, &::getCppuType((sal_Bool*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    if ( pObjSh && pStorage)
    {
        pObjSh->UpdateDocInfoForSave();		// update information

        uno::Reference<frame::XModel> xModel = pObjSh->GetModel();
        uno::Reference<task::XStatusIndicator> xStatusIndicator = GetStatusIndicator(xModel);
        sal_Int32 nProgressRange(1000000);
        if(xStatusIndicator.is())
            xStatusIndicator->start(::rtl::OUString(ScGlobal::GetRscString(STR_SAVE_DOC)), nProgressRange);
        uno::Any aProgRange;
        aProgRange <<= nProgressRange;
        xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);

        SvtSaveOptions aSaveOpt;
        sal_Bool bUsePrettyPrinting(aSaveOpt.IsPrettyPrinting());
        uno::Any aUsePrettyPrinting;
        aUsePrettyPrinting <<= bUsePrettyPrinting;
        xInfoSet->setPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting")), aUsePrettyPrinting);

        sal_Bool bMetaRet(pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
        sal_Bool bStylesRet (sal_False);
        sal_Bool bDocRet(sal_False);
        sal_Bool bSettingsRet(sal_False);
        ScMySharedData* pSharedData = NULL;

        // meta export
        if (!bStylesOnly && !bMetaRet)
        {
            uno::Sequence<uno::Any> aMetaArgs(3);
            uno::Any* pMetaArgs = aMetaArgs.getArray();
            pMetaArgs[0] <<= xHandler;
            pMetaArgs[1] <<= xStatusIndicator;
            pMetaArgs[2] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta export start" );

            bMetaRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
                sTextMediaType, ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaExporter")),
                sal_True, aMetaArgs, pSharedData);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta export end" );
        }

        uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
        SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

        uno::Reference< document::XGraphicObjectResolver > xGrfContainer;
        SvXMLGraphicHelper* pGraphicHelper = 0;

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

        // styles export

        {
            uno::Sequence<uno::Any> aStylesArgs(5);
            uno::Any* pStylesArgs = aStylesArgs.getArray();
            pStylesArgs[0] <<= xGrfContainer;
            pStylesArgs[1] <<= xStatusIndicator;
            pStylesArgs[2] <<= xHandler;
            pStylesArgs[3] <<= xObjectResolver;
            pStylesArgs[4] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles export start" );

            bStylesRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
                sTextMediaType, ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesExporter")),
                sal_False, aStylesArgs, pSharedData);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles export end" );
        }

        // content export

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aDocArgs(5);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xGrfContainer;
            pDocArgs[1] <<= xStatusIndicator;
            pDocArgs[2] <<= xHandler;
            pDocArgs[3] <<= xObjectResolver;
            pDocArgs[4] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "content export start" );

            bDocRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("content.xml")),
                sTextMediaType, ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentExporter")),
                sal_False, aDocArgs, pSharedData);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "content export end" );
        }

        if( pGraphicHelper )
            SvXMLGraphicHelper::Destroy( pGraphicHelper );

        if( pObjectHelper )
            SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );

        // settings export

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aSettingsArgs(3);
            uno::Any* pSettingsArgs = aSettingsArgs.getArray();
            pSettingsArgs[0] <<= xHandler;
            pSettingsArgs[1] <<= xStatusIndicator;
            pSettingsArgs[2] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings export start" );

            bSettingsRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
                sTextMediaType, ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsExporter")),
                sal_False, aSettingsArgs, pSharedData);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings export end" );
        }

        if (pSharedData)
            delete pSharedData;

        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return bStylesRet && ((!bStylesOnly && bDocRet && bMetaRet && bSettingsRet) || bStylesOnly);
    }

    // later: give string descriptor as parameter for doc type

    return sal_False;
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
