/*************************************************************************
 *
 *  $RCSfile: xmlwrap.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 07:49:33 $
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
#include <svx/xmlgrhlp.hxx>
#include <svtools/sfxecode.hxx>
#ifndef _SFXFRAME_HXX
#include <sfx2/frame.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

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
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#endif

#ifndef _XMLEOHLP_HXX
#include <svx/xmleohlp.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_SAVEOPT_HXX
#include <svtools/saveopt.hxx>
#endif

#include "document.hxx"
#include "xmlwrap.hxx"
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"
#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif
#ifndef _SCERRORS_HXX
#include "scerrors.hxx"
#endif
#ifndef SC_XMLEXPORTSHAREDDATA_HXX
#include "XMLExportSharedData.hxx"
#endif

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
    const rtl::OUString& sComponentName, const rtl::OUString& sDocName,
    const rtl::OUString& sOldDocName, uno::Sequence<uno::Any>& aArgs,
    sal_Bool bMustBeSuccessfull)
{
    SvStorageStreamRef xDocStream;
    if ( !pStorage && pMedium )
        pStorage = pMedium->GetStorage();

    // Get data source ...

    uno::Reference< uno::XInterface > xPipe;
    uno::Reference< io::XActiveDataSource > xSource;

    sal_Bool bEncrypted = sal_False;
    rtl::OUString sStream(sDocName);
    if( pStorage )
    {
        if (pStorage->IsStream(sDocName))
            xDocStream = pStorage->OpenStream( sDocName,
                                  STREAM_READ | STREAM_NOCREATE );
        else if (sOldDocName.getLength() && pStorage->IsStream(sOldDocName))
        {
            xDocStream = pStorage->OpenStream( sOldDocName,
                                  STREAM_READ | STREAM_NOCREATE );
            sStream = sOldDocName;
        }
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
/*  else if ( pMedium )
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

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( aArgs.getLength() > 0 )
        aArgs.getConstArray()[0] >>= xInfoSet;
    DBG_ASSERT( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
        xInfoSet->setPropertyValue( sPropName, uno::makeAny( sStream ) );
    }

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

        /** property map for export info set */
        comphelper::PropertyMapEntry aImportInfoMap[] =
        {
            { MAP_LEN( "ProgressRange" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressMax" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "NumberStyles" ), 0, &::getCppuType((uno::Reference<container::XNameAccess> *)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
            { MAP_LEN( "PrivateData" ), 0, &::getCppuType( (uno::Reference<uno::XInterface> *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "BaseURI" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamRelPath" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { MAP_LEN( "StreamName" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
            { NULL, 0, 0, NULL, 0, 0 }
        };
        uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aImportInfoMap ) ) );

        uno::Reference<task::XStatusIndicator> xStatusIndicator(GetStatusIndicator());
        if (xStatusIndicator.is())
        {
            sal_Int32 nProgressRange(1000000);
            xStatusIndicator->start(rtl::OUString(ScGlobal::GetRscString(STR_LOAD_DOC)), nProgressRange);
            uno::Any aProgRange;
            aProgRange <<= nProgressRange;
            xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);
        }

        // Set base URI
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
        xInfoSet->setPropertyValue( sPropName,
            uno::makeAny( rtl::OUString(INetURLObject::GetBaseURL()) ) );
        if( SFX_CREATE_MODE_EMBEDDED == pObjSh->GetCreateMode() )
        {
            rtl::OUString aName( pStorage->GetName() );
            if( aName.getLength() )
            {
                sPropName = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StreamRelPath"));
                xInfoSet->setPropertyValue( sPropName, uno::makeAny( aName ) );
            }
        }

        sal_Bool bOasis = pStorage->GetVersion() > SOFFICE_FILEFORMAT_60;

        sal_uInt32 nMetaRetval(0);
        if(!bStylesOnly)
        {
            uno::Sequence<uno::Any> aMetaArgs(1);
            uno::Any* pMetaArgs = aMetaArgs.getArray();
            pMetaArgs[0] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta import start" );

            nMetaRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisMetaImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Meta.xml")), aMetaArgs,
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
        pStylesArgs[0] <<= xInfoSet;
        pStylesArgs[1] <<= xGrfContainer;
        pStylesArgs[2] <<= xStatusIndicator;
        pStylesArgs[3] <<= xObjectResolver;

        sal_uInt32 nSettingsRetval(0);
        if (!bStylesOnly)
        {
            //  Settings must be loaded first because of the printer setting,
            //  which is needed in the page styles (paper tray).

            uno::Sequence<uno::Any> aSettingsArgs(1);
            uno::Any* pSettingsArgs = aSettingsArgs.getArray();
            pSettingsArgs[0] <<= xInfoSet;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings import start" );

            nSettingsRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisSettingsImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
                sEmpty, aSettingsArgs, sal_False);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings import end" );
        }

        sal_uInt32 nStylesRetval(0);
        {
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles import start" );

            nStylesRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisStylesImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
                sEmpty, aStylesArgs, sal_True);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles import end" );
        }

        sal_uInt32 nDocRetval(0);
        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aDocArgs(4);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xInfoSet;
            pDocArgs[1] <<= xGrfContainer;
            pDocArgs[2] <<= xStatusIndicator;
            pDocArgs[3] <<= xObjectResolver;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "content import start" );

            nDocRetval = ImportFromComponent(xServiceFactory, xModel, xXMLParser, aParserInput,
                bOasis ? rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisContentImporter"))
                       : rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentImporter")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Content.xml")), aDocArgs,
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
    uno::Sequence<beans::PropertyValue>& aDescriptor, const rtl::OUString& sName,
    const rtl::OUString& sMediaType, const rtl::OUString& sComponentName,
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
        xStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType")), aAny);
        if (bPlainText)
        {
            aAny = ::cppu::bool2any(sal_False);
            xStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Compressed")), aAny);
        }
        else
        {
            aAny = ::cppu::bool2any(sal_True);
            xStream->SetProperty(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Encrypted")), aAny);
        }
        xStream->SetBufferSize( 16*1024 );
        xOut = new utl::OOutputStreamWrapper( *xStream );
    }
    // #99667#; no longer necessary
/*  else if ( pMedium )
    {
        xOut = pMedium->GetDataSink();
    }*/

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( aArgs.getLength() > 0 )
        aArgs.getConstArray()[0] >>= xInfoSet;
    DBG_ASSERT( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        rtl::OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
        xInfoSet->setPropertyValue( sPropName, uno::makeAny( sName ) );
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
    RTL_LOGFILE_CONTEXT_AUTHOR ( aLog, "sc", "sb99857", "ScXMLImportWrapper::Export" );

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
        { MAP_LEN( "ProgressRange" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "ProgressMax" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "ProgressCurrent" ), 0, &::getCppuType((sal_Int32*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "WrittenNumberStyles" ), 0, &::getCppuType((uno::Sequence<sal_Int32>*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "UsePrettyPrinting" ), 0, &::getCppuType((sal_Bool*)0), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0},
        { MAP_LEN( "BaseURI" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamRelPath" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
        { MAP_LEN( "StreamName" ), 0, &::getCppuType( (rtl::OUString *)0 ), ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },
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
            xStatusIndicator->start(rtl::OUString(ScGlobal::GetRscString(STR_SAVE_DOC)), nProgressRange);
        uno::Any aProgRange;
        aProgRange <<= nProgressRange;
        xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ProgressRange")), aProgRange);

        SvtSaveOptions aSaveOpt;
        sal_Bool bUsePrettyPrinting(aSaveOpt.IsPrettyPrinting());
        uno::Any aUsePrettyPrinting;
        aUsePrettyPrinting <<= bUsePrettyPrinting;
        xInfoSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting")), aUsePrettyPrinting);

        sal_Bool bMetaRet(pObjSh->GetCreateMode() == SFX_CREATE_MODE_EMBEDDED);
        sal_Bool bStylesRet (sal_False);
        sal_Bool bDocRet(sal_False);
        sal_Bool bSettingsRet(sal_False);
        ScMySharedData* pSharedData = NULL;

        sal_Bool bOasis = pStorage->GetVersion() > SOFFICE_FILEFORMAT_60;

        // meta export
        if (!bStylesOnly && !bMetaRet)
        {
            uno::Sequence<uno::Any> aMetaArgs(3);
            uno::Any* pMetaArgs = aMetaArgs.getArray();
            pMetaArgs[0] <<= xInfoSet;
            pMetaArgs[1] <<= xHandler;
            pMetaArgs[2] <<= xStatusIndicator;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "meta export start" );

            bMetaRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("meta.xml")),
                sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisMetaExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLMetaExporter")),
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
            pStylesArgs[0] <<= xInfoSet;
            pStylesArgs[1] <<= xGrfContainer;
            pStylesArgs[2] <<= xStatusIndicator;
            pStylesArgs[3] <<= xHandler;
            pStylesArgs[4] <<= xObjectResolver;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles export start" );

            bStylesRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("styles.xml")),
                sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisStylesExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLStylesExporter")),
                sal_False, aStylesArgs, pSharedData);

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "styles export end" );
        }

        // content export

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aDocArgs(5);
            uno::Any* pDocArgs = aDocArgs.getArray();
            pDocArgs[0] <<= xInfoSet;
            pDocArgs[1] <<= xGrfContainer;
            pDocArgs[2] <<= xStatusIndicator;
            pDocArgs[3] <<= xHandler;
            pDocArgs[4] <<= xObjectResolver;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "content export start" );

            bDocRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("content.xml")),
                sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisContentExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLContentExporter")),
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
            pSettingsArgs[0] <<= xInfoSet;
            pSettingsArgs[1] <<= xHandler;
            pSettingsArgs[2] <<= xStatusIndicator;

            RTL_LOGFILE_CONTEXT_TRACE( aLog, "settings export start" );

            bSettingsRet = ExportToComponent(xServiceFactory, xModel, xWriter, aDescriptor,
                rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("settings.xml")),
                sTextMediaType,
                bOasis ? rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLOasisSettingsExporter"))
                       : rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Calc.XMLSettingsExporter")),
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



