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


#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <unotools/saveopt.hxx>
#include <svl/stritem.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <docstat.hxx>
#include <docsh.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <swerror.h>
#include <wrtxml.hxx>
#include <statstr.hrc>
#include <rtl/logfile.hxx>

#include <comphelper/componentcontext.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/makesequence.hxx>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#define LOGFILE_AUTHOR "mb93740"

SwXMLWriter::SwXMLWriter( const String& rBaseURL )
{
    SetBaseURL( rBaseURL );
}


SwXMLWriter::~SwXMLWriter()
{
}


sal_uInt32 SwXMLWriter::_Write( SfxMedium* pTargetMedium )
{
    OSL_ENSURE( pTargetMedium, "No medium is provided!" );
    // Get service factory
    uno::Reference< lang::XMultiServiceFactory > xServiceFactory =
            comphelper::getProcessServiceFactory();
    OSL_ENSURE( xServiceFactory.is(),
            "SwXMLWriter::Write: got no service manager" );
    if( !xServiceFactory.is() )
        return ERR_SWG_WRITE_ERROR;

    // Get data sink ...
    uno::Reference< io::XOutputStream > xOut;
    SvStorageStreamRef xDocStream;
    uno::Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = 0;
    uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = 0;

    OSL_ENSURE( xStg.is(), "Where is my storage?" );
pGraphicHelper = SvXMLGraphicHelper::Create( xStg,
                                                 GRAPHICHELPER_MODE_WRITE,
                                                 sal_False );
    xGraphicResolver = pGraphicHelper;

    SfxObjectShell *pPersist = pDoc->GetPersist();
    if( pPersist )
    {
        pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                         xStg, *pPersist,
                                         EMBEDDEDOBJECTHELPER_MODE_WRITE,
                                         sal_False );
        xObjectResolver = pObjectHelper;
    }

    // create and prepare the XPropertySet that gets passed through
    // the components, and the XStatusIndicator that shows progress to
    // the user.

    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry aInfoMap[] =
    {
        { "ProgressRange", sizeof("ProgressRange")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ProgressMax", sizeof("ProgressMax")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ProgressCurrent", sizeof("ProgressCurrent")-1, 0,
              &::getCppuType((sal_Int32*)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "WrittenNumberStyles", sizeof("WrittenNumberStyles")-1, 0,
              &::getCppuType((uno::Sequence<sal_Int32> *)0),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "UsePrettyPrinting", sizeof("UsePrettyPrinting")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { "ShowChanges", sizeof("ShowChanges")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "RedlineProtectionKey", sizeof("RedlineProtectionKey")-1, 0,
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
              new uno::Type(::getCppuType((Sequence<sal_Int8>*)0)),
#else
              &::getCppuType((Sequence<sal_Int8>*)0),
#endif
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "BaseURI", sizeof("BaseURI")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StreamRelPath", sizeof("StreamRelPath")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StreamName", sizeof("StreamName")-1, 0,
              &::getCppuType( (OUString *)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "AutoTextMode", sizeof("AutoTextMode")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StyleNames", sizeof("StyleNames")-1, 0,
              &::getCppuType( (Sequence<OUString>*)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "StyleFamilies", sizeof("StyleFamilies")-1, 0,
              &::getCppuType( (Sequence<sal_Int32>*)0 ),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        // #i69627#
        { "OutlineStyleAsNormalListStyle", sizeof("OutlineStyleAsNormalListStyle")-1, 0,
              &::getBooleanCppuType(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { "TargetStorage", sizeof("TargetStorage")-1,0, &embed::XStorage::static_type(),
              ::com::sun::star::beans::PropertyAttribute::MAYBEVOID, 0 },

        { NULL, 0, 0, NULL, 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    const OUString sTargetStorage( RTL_CONSTASCII_USTRINGPARAM("TargetStorage") );
    xInfoSet->setPropertyValue( sTargetStorage, Any( xStg ) );

    // create XStatusIndicator
    uno::Reference<task::XStatusIndicator> xStatusIndicator;

    uno::Any aAny;
    if (bShowProgress)
    {
        // retrieve status indicator from the medium MediaDescriptor
        if ( pTargetMedium )
        {
            const SfxUnoAnyItem* pStatusBarItem = static_cast<const SfxUnoAnyItem*>(
               pTargetMedium->GetItemSet()->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );

            if ( pStatusBarItem )
                pStatusBarItem->GetValue() >>= xStatusIndicator;
        }

        // set progress range and start status indicator
        sal_Int32 nProgressRange(1000000);
        if (xStatusIndicator.is())
        {
            xStatusIndicator->start(SW_RESSTR( STR_STATSTR_SWGWRITE),
                                    nProgressRange);
        }
        aAny <<= nProgressRange;
        OUString sProgressRange(RTL_CONSTASCII_USTRINGPARAM("ProgressRange"));
        xInfoSet->setPropertyValue(sProgressRange, aAny);

        aAny <<= static_cast < sal_Int32 >( -1 );
        OUString sProgressMax(RTL_CONSTASCII_USTRINGPARAM("ProgressMax"));
        xInfoSet->setPropertyValue(sProgressMax, aAny);
    }
    SvtSaveOptions aSaveOpt;
    OUString sUsePrettyPrinting(RTL_CONSTASCII_USTRINGPARAM("UsePrettyPrinting"));
    sal_Bool bUsePrettyPrinting( aSaveOpt.IsPrettyPrinting() );
    aAny.setValue( &bUsePrettyPrinting, ::getBooleanCppuType() );
    xInfoSet->setPropertyValue( sUsePrettyPrinting, aAny );

    // save show redline mode ...
    OUString sShowChanges(RTL_CONSTASCII_USTRINGPARAM("ShowChanges"));
    sal_uInt16 nRedlineMode = pDoc->GetRedlineMode();
    sal_Bool bShowChanges( IDocumentRedlineAccess::IsShowChanges( nRedlineMode ) );
    aAny.setValue( &bShowChanges, ::getBooleanCppuType() );
    xInfoSet->setPropertyValue( sShowChanges, aAny );
    // ... and hide redlines for export
    nRedlineMode &= ~nsRedlineMode_t::REDLINE_SHOW_MASK;
    nRedlineMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT;
    pDoc->SetRedlineMode((RedlineMode_t)( nRedlineMode ));

    // Set base URI
    OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("BaseURI") );
    xInfoSet->setPropertyValue( sPropName, makeAny( ::rtl::OUString( GetBaseURL() ) ) );

    if( SFX_CREATE_MODE_EMBEDDED == pDoc->GetDocShell()->GetCreateMode() )
    {
        OUString aName;
        if ( pTargetMedium && pTargetMedium->GetItemSet() )
        {
            const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
                pTargetMedium->GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
            if ( pDocHierarchItem )
                aName = pDocHierarchItem->GetValue();
        }
        else
            aName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "dummyObjectName" ) );

        if( !aName.isEmpty() )
        {
            sPropName = OUString(RTL_CONSTASCII_USTRINGPARAM("StreamRelPath"));
            xInfoSet->setPropertyValue( sPropName, makeAny( aName ) );
        }
    }

    if( bBlock )
    {
        OUString sAutoTextMode(
                RTL_CONSTASCII_USTRINGPARAM("AutoTextMode"));
        sal_Bool bTmp = sal_True;
        Any aAny2;
        aAny2.setValue( &bTmp, ::getBooleanCppuType() );
        xInfoSet->setPropertyValue( sAutoTextMode, aAny2 );
    }

    // #i69627#
    const sal_Bool bOASIS = ( SotStorage::GetVersion( xStg ) > SOFFICE_FILEFORMAT_60 );
    if ( bOASIS &&
         docfunc::HasOutlineStyleToBeWrittenAsNormalListStyle( *pDoc ) )
    {
        OUString sOutlineStyleAsNormalListStyle(
                RTL_CONSTASCII_USTRINGPARAM("OutlineStyleAsNormalListStyle") );
        xInfoSet->setPropertyValue( sOutlineStyleAsNormalListStyle, makeAny( sal_True ) );
    }

    // filter arguments
    // - graphics + object resolver for styles + content
    // - status indicator
    // - info property set
    // - else empty
    sal_Int32 nArgs = 1;
    if( xStatusIndicator.is() )
        nArgs++;

    Sequence < Any > aEmptyArgs( nArgs );
    Any *pArgs = aEmptyArgs.getArray();
    *pArgs++ <<= xInfoSet;
    if( xStatusIndicator.is() )
        *pArgs++ <<= xStatusIndicator;

    if( xGraphicResolver.is() )
        nArgs++;
    if( xObjectResolver.is() )
        nArgs++;

    Sequence < Any > aFilterArgs( nArgs );
    pArgs = aFilterArgs.getArray();
    *pArgs++ <<= xInfoSet;
    if( xGraphicResolver.is() )
        *pArgs++ <<= xGraphicResolver;
    if( xObjectResolver.is() )
        *pArgs++ <<= xObjectResolver;
    if( xStatusIndicator.is() )
        *pArgs++ <<= xStatusIndicator;

    //Get model
    uno::Reference< lang::XComponent > xModelComp(
        pDoc->GetDocShell()->GetModel(), UNO_QUERY );
    OSL_ENSURE( xModelComp.is(), "XMLWriter::Write: got no model" );
    if( !xModelComp.is() )
        return ERR_SWG_WRITE_ERROR;

    PutNumFmtFontsInAttrPool();
    PutEditEngFontsInAttrPool();

    // properties
    Sequence < PropertyValue > aProps( pOrigFileName ? 1 : 0 );
    if( pOrigFileName )
    {
        PropertyValue *pProps = aProps.getArray();
        pProps->Name = OUString( RTL_CONSTASCII_USTRINGPARAM("FileName") );
        (pProps++)->Value <<= OUString( *pOrigFileName  );
    }

    // export sub streams for package, else full stream into a file
    sal_Bool bWarn = sal_False, bErr = sal_False;
    String sWarnFile, sErrFile;

    // RDF metadata: export if ODF >= 1.2
    // N.B.: embedded documents have their own manifest.rdf!
    if ( bOASIS )
    {
        const uno::Reference<beans::XPropertySet> xPropSet(xStg,
            uno::UNO_QUERY_THROW);
        const ::rtl::OUString VersionProp(RTL_CONSTASCII_USTRINGPARAM("Version"));
        try
        {
            ::rtl::OUString Version;
            // ODF >= 1.2
            if ((xPropSet->getPropertyValue(VersionProp) >>= Version)
                && !Version.equals(ODFVER_010_TEXT)
                && !Version.equals(ODFVER_011_TEXT))
            {
                const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(
                    xModelComp, uno::UNO_QUERY_THROW);
                xDMA->storeMetadataToStorage(xStg);
            }
        }
        catch (beans::UnknownPropertyException &)
        { /* ignore */ }
        catch (uno::Exception &)
        {
            bWarn = sal_True;
        }
    }

    sal_Bool bStoreMeta = ( SFX_CREATE_MODE_EMBEDDED != pDoc->GetDocShell()->GetCreateMode() );
    if ( !bStoreMeta )
    {
        try
        {
            Reference< frame::XModule > xModule( xModelComp, UNO_QUERY );
            if ( xModule.is() )
            {
                ::rtl::OUString aModuleID = xModule->getIdentifier();
                bStoreMeta = ( !aModuleID.isEmpty() && ( aModuleID == "com.sun.star.sdb.FormDesign" || aModuleID == "com.sun.star.sdb.TextReportDesign" ) );
            }
        }
        catch( uno::Exception& )
        {}
    }

    if( !bOrganizerMode && !bBlock && bStoreMeta )
    {
        if( !WriteThroughComponent(
                xModelComp, "meta.xml", xServiceFactory,
                (bOASIS ? "com.sun.star.comp.Writer.XMLOasisMetaExporter"
                        : "com.sun.star.comp.Writer.XMLMetaExporter"),
                aEmptyArgs, aProps, sal_True ) )
        {
            bWarn = sal_True;
            sWarnFile = String( RTL_CONSTASCII_STRINGPARAM("meta.xml"),
                                RTL_TEXTENCODING_ASCII_US );
        }
    }

    if( !bErr )
    {
        if( !bBlock )
        {
            if( !WriteThroughComponent(
                xModelComp, "settings.xml", xServiceFactory,
                (bOASIS ? "com.sun.star.comp.Writer.XMLOasisSettingsExporter"
                        : "com.sun.star.comp.Writer.XMLSettingsExporter"),
                aEmptyArgs, aProps, sal_False ) )
            {
                if( !bWarn )
                {
                    bWarn = sal_True;
                    sWarnFile = String( RTL_CONSTASCII_STRINGPARAM("settings.xml"),
                                        RTL_TEXTENCODING_ASCII_US );
                }
            }
        }
    }

    if( !WriteThroughComponent(
            xModelComp, "styles.xml", xServiceFactory,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisStylesExporter"
                    : "com.sun.star.comp.Writer.XMLStylesExporter"),
            aFilterArgs, aProps, sal_False ) )
    {
        bErr = sal_True;
        sErrFile = String( RTL_CONSTASCII_STRINGPARAM("styles.xml"),
                           RTL_TEXTENCODING_ASCII_US );
    }


    if( !bOrganizerMode && !bErr )
    {
        if( !WriteThroughComponent(
                xModelComp, "content.xml", xServiceFactory,
                (bOASIS ? "com.sun.star.comp.Writer.XMLOasisContentExporter"
                        : "com.sun.star.comp.Writer.XMLContentExporter"),
                aFilterArgs, aProps, sal_False ) )
        {
            bErr = sal_True;
            sErrFile = String( RTL_CONSTASCII_STRINGPARAM("content.xml"),
                               RTL_TEXTENCODING_ASCII_US );
        }
    }

    if( pDoc->GetCurrentViewShell() && pDoc->GetDocStat().nPage > 1 &&  //swmod 071108//swmod 071225
        !(bOrganizerMode || bBlock || bErr) )
    {
        OUString sStreamName( RTL_CONSTASCII_USTRINGPARAM("layout-cache") );
        try
        {
            uno::Reference < io::XStream > xStm = xStg->openStreamElement( sStreamName, embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
            SvStream* pStream = utl::UcbStreamHelper::CreateStream( xStm );
            if( !pStream->GetError() )
            {
                uno::Reference < beans::XPropertySet > xSet( xStm, UNO_QUERY );
                OUString aMime( RTL_CONSTASCII_USTRINGPARAM("application/binary") );
                uno::Any aAny2;
                aAny2 <<= aMime;
                xSet->setPropertyValue( rtl::OUString("MediaType"), aAny2 );
                pDoc->WriteLayoutCache( *pStream );
            }

            delete pStream;
        }
        catch ( uno::Exception& )
        {
        }
    }

    if( pGraphicHelper )
        SvXMLGraphicHelper::Destroy( pGraphicHelper );
    xGraphicResolver = 0;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = 0;

    // restore redline mode
    aAny = xInfoSet->getPropertyValue( sShowChanges );
    nRedlineMode = pDoc->GetRedlineMode();
    nRedlineMode &= ~nsRedlineMode_t::REDLINE_SHOW_MASK;
    nRedlineMode |= nsRedlineMode_t::REDLINE_SHOW_INSERT;
    if ( *(sal_Bool*)aAny.getValue() )
        nRedlineMode |= nsRedlineMode_t::REDLINE_SHOW_DELETE;
    pDoc->SetRedlineMode((RedlineMode_t)( nRedlineMode ));

    if (xStatusIndicator.is())
    {
        xStatusIndicator->end();
    }

    if( bErr )
    {
        if( sErrFile.Len() )
            return *new StringErrorInfo( ERR_WRITE_ERROR_FILE, sErrFile,
                                         ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        else
            return ERR_SWG_WRITE_ERROR;
    }
    else if( bWarn )
    {
        if( sWarnFile.Len() )
            return *new StringErrorInfo( WARN_WRITE_ERROR_FILE, sWarnFile,
                                         ERRCODE_BUTTON_OK | ERRCODE_MSG_ERROR );
        else
            return WARN_SWG_FEATURES_LOST;
    }

    return 0;
}

sal_uLong SwXMLWriter::WriteStorage()
{
    return _Write();
}

sal_uLong SwXMLWriter::WriteMedium( SfxMedium& aTargetMedium )
{
    return _Write( &aTargetMedium );
}

sal_uLong SwXMLWriter::Write( SwPaM& rPaM, SfxMedium& rMed,
                               const String* pFileName )
{
    return IsStgWriter()
            ? ((StgWriter *)this)->Write( rPaM, rMed.GetOutputStorage(), pFileName, &rMed )
            : ((Writer *)this)->Write( rPaM, *rMed.GetOutStream(), pFileName );
}

sal_Bool SwXMLWriter::WriteThroughComponent(
    const uno::Reference<XComponent> & xComponent,
    const sal_Char* pStreamName,
    const uno::Reference<lang::XMultiServiceFactory> & rFactory,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<beans::PropertyValue> & rMediaDesc,
    sal_Bool bPlainStream )
{
    OSL_ENSURE( xStg.is(), "Need storage!" );
    OSL_ENSURE( NULL != pStreamName, "Need stream name!" );
    OSL_ENSURE( NULL != pServiceName, "Need service name!" );

    RTL_LOGFILE_TRACE_AUTHOR1( "sw", LOGFILE_AUTHOR,
                               "SwXMLWriter::WriteThroughComponent : stream %s",
                               pStreamName );

    // open stream
    sal_Bool bRet = sal_False;
    try
    {
        OUString sStreamName = OUString::createFromAscii( pStreamName );
        uno::Reference<io::XStream> xStream =
                xStg->openStreamElement( sStreamName,
                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

        uno::Reference <beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
        if( !xSet.is() )
            return sal_False;

        OUString aMime( RTL_CONSTASCII_USTRINGPARAM("text/xml") );
        uno::Any aAny;
        aAny <<= aMime;
        xSet->setPropertyValue( rtl::OUString("MediaType"), aAny );

        OUString aUseCommonPassPropName( RTL_CONSTASCII_USTRINGPARAM("UseCommonStoragePasswordEncryption") );
        if( bPlainStream )
        {
            OUString aCompressPropName( RTL_CONSTASCII_USTRINGPARAM("Compressed") );
            sal_Bool bFalse = sal_False;
            aAny.setValue( &bFalse, ::getBooleanCppuType() );
            xSet->setPropertyValue( aCompressPropName, aAny );
        }

        // even plain stream should be encrypted in encrypted documents
        sal_Bool bTrue = sal_True;
        aAny.setValue( &bTrue, ::getBooleanCppuType() );
        xSet->setPropertyValue( aUseCommonPassPropName, aAny );

        // set buffer and create outputstream
        uno::Reference< io::XOutputStream > xOutputStream = xStream->getOutputStream();

        // set Base URL
        uno::Reference< beans::XPropertySet > xInfoSet;
        if( rArguments.getLength() > 0 )
            rArguments.getConstArray()[0] >>= xInfoSet;
        OSL_ENSURE( xInfoSet.is(), "missing property set" );
        if( xInfoSet.is() )
        {
            OUString sPropName( RTL_CONSTASCII_USTRINGPARAM("StreamName") );
            xInfoSet->setPropertyValue( sPropName, makeAny( sStreamName ) );
        }

        // write the stuff
        bRet = WriteThroughComponent(
            xOutputStream, xComponent, rFactory,
            pServiceName, rArguments, rMediaDesc );
    }
    catch ( uno::Exception& )
    {
    }

    return bRet;

}

sal_Bool SwXMLWriter::WriteThroughComponent(
    const uno::Reference<io::XOutputStream> & xOutputStream,
    const uno::Reference<XComponent> & xComponent,
    const uno::Reference<XMultiServiceFactory> & rFactory,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<PropertyValue> & rMediaDesc )
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( NULL != pServiceName, "Need component name!" );

    RTL_LOGFILE_CONTEXT_AUTHOR( aFilterLog, "sw", LOGFILE_AUTHOR,
                                "SwXMLWriter::WriteThroughComponent" );

    // get component
    uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(comphelper::getComponentContext(rFactory));
    RTL_LOGFILE_CONTEXT_TRACE( aFilterLog, "SAX-Writer created" );

    // connect XML writer to output stream
    xSaxWriter->setOutputStream( xOutputStream );

    // prepare arguments (prepend doc handler to given arguments)
    uno::Reference<xml::sax::XDocumentHandler> xDocHandler( xSaxWriter,UNO_QUERY);
    Sequence<Any> aArgs( 1 + rArguments.getLength() );
    aArgs[0] <<= xDocHandler;
    for(sal_Int32 i = 0; i < rArguments.getLength(); i++)
        aArgs[i+1] = rArguments[i];

    // get filter component
    uno::Reference< document::XExporter > xExporter(
        rFactory->createInstanceWithArguments(
            OUString::createFromAscii(pServiceName), aArgs), UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
            "can't instantiate export filter component" );
    if( !xExporter.is() )
        return sal_False;
    RTL_LOGFILE_CONTEXT_TRACE1( aFilterLog, "%s instantiated.", pServiceName );

    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    RTL_LOGFILE_CONTEXT_TRACE( aFilterLog, "call filter()" );
    uno::Reference<XFilter> xFilter( xExporter, UNO_QUERY );
    return xFilter->filter( rMediaDesc );
}


// -----------------------------------------------------------------------

void GetXMLWriter( const String& /*rName*/, const String& rBaseURL, WriterRef& xRet )
{
    xRet = new SwXMLWriter( rBaseURL );
}

// -----------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
