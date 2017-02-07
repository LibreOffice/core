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

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XModule.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <o3tl/any.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <unotools/saveopt.hxx>
#include <svl/stritem.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <docfunc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docstat.hxx>
#include <docsh.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <swerror.h>
#include <wrtxml.hxx>
#include <statstr.hrc>

#include <comphelper/documentconstants.hxx>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

SwXMLWriter::SwXMLWriter( const OUString& rBaseURL )
{
    SetBaseURL( rBaseURL );
}

SwXMLWriter::~SwXMLWriter()
{
}

sal_uInt32 SwXMLWriter::Write_( const uno::Reference < task::XStatusIndicator >& xStatusIndicator,
                                const OUString& aDocHierarchicalName )
{
    // Get service factory
    uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

    // Get data sink ...
    uno::Reference< io::XOutputStream > xOut;
    tools::SvRef<SotStorageStream> xDocStream;
    uno::Reference< document::XGraphicObjectResolver > xGraphicResolver;
    SvXMLGraphicHelper *pGraphicHelper = nullptr;
    uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    SvXMLEmbeddedObjectHelper *pObjectHelper = nullptr;

    OSL_ENSURE( xStg.is(), "Where is my storage?" );
    pGraphicHelper = SvXMLGraphicHelper::Create( xStg,
                                                 SvXMLGraphicHelperMode::Write,
                                                 false );
    xGraphicResolver = pGraphicHelper;

    SfxObjectShell *pPersist = pDoc->GetPersist();
    if( pPersist )
    {
        pObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                         xStg, *pPersist,
                                         SvXMLEmbeddedObjectHelperMode::Write,
                                         false );
        xObjectResolver = pObjectHelper;
    }

    // create and prepare the XPropertySet that gets passed through
    // the components, and the XStatusIndicator that shows progress to
    // the user.

    // create XPropertySet with three properties for status indicator
    comphelper::PropertyMapEntry const aInfoMap[] =
    {
        { OUString("ProgressRange"), 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ProgressMax"), 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ProgressCurrent"), 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("WrittenNumberStyles"), 0,
              cppu::UnoType<uno::Sequence<sal_Int32>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("UsePrettyPrinting"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { OUString("ShowChanges"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("RedlineProtectionKey"), 0,
              cppu::UnoType<Sequence<sal_Int8>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("BaseURI"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamRelPath"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StreamName"), 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("AutoTextMode"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StyleNames"), 0,
              cppu::UnoType<Sequence<OUString>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("StyleFamilies"), 0,
              cppu::UnoType<Sequence<sal_Int32>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        // #i69627#
        { OUString("OutlineStyleAsNormalListStyle"), 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString("TargetStorage"),0, cppu::UnoType<embed::XStorage>::get(),
              css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    xInfoSet->setPropertyValue( "TargetStorage", Any( xStg ) );

    uno::Any aAny;
    if (bShowProgress)
    {
        // set progress range and start status indicator
        sal_Int32 nProgressRange(1000000);
        if (xStatusIndicator.is())
        {
            xStatusIndicator->start(SW_RESSTR( STR_STATSTR_SWGWRITE),
                                    nProgressRange);
        }
        xInfoSet->setPropertyValue("ProgressRange", Any(nProgressRange));

        xInfoSet->setPropertyValue("ProgressMax", Any(static_cast < sal_Int32 >( -1 )));
    }

    SvtSaveOptions aSaveOpt;
    xInfoSet->setPropertyValue( "UsePrettyPrinting", makeAny(aSaveOpt.IsPrettyPrinting()) );

    // save show redline mode ...
    const OUString sShowChanges("ShowChanges");
    RedlineFlags nRedlineFlags = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    xInfoSet->setPropertyValue( sShowChanges,
        makeAny( IDocumentRedlineAccess::IsShowChanges( nRedlineFlags ) ) );
    // ... and hide redlines for export
    nRedlineFlags &= ~RedlineFlags::ShowMask;
    nRedlineFlags |= RedlineFlags::ShowInsert;
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags( nRedlineFlags );

    // Set base URI
    xInfoSet->setPropertyValue( "BaseURI", makeAny( GetBaseURL() ) );

    if( SfxObjectCreateMode::EMBEDDED == pDoc->GetDocShell()->GetCreateMode() )
    {
        const OUString aName( !aDocHierarchicalName.isEmpty()
            ? aDocHierarchicalName
            : OUString( "dummyObjectName" ) );

        xInfoSet->setPropertyValue( "StreamRelPath", makeAny( aName ) );
    }

    if( bBlock )
    {
        xInfoSet->setPropertyValue( "AutoTextMode", makeAny(true) );
    }

    // #i69627#
    const bool bOASIS = ( SotStorage::GetVersion( xStg ) > SOFFICE_FILEFORMAT_60 );
    if ( bOASIS &&
         docfunc::HasOutlineStyleToBeWrittenAsNormalListStyle( *pDoc ) )
    {
        xInfoSet->setPropertyValue( "OutlineStyleAsNormalListStyle", makeAny( true ) );
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

    PutNumFormatFontsInAttrPool();
    PutEditEngFontsInAttrPool();

    // properties
    Sequence < PropertyValue > aProps( pOrigFileName ? 1 : 0 );
    if( pOrigFileName )
    {
        PropertyValue *pProps = aProps.getArray();
        pProps->Name = "FileName";
        pProps->Value <<= *pOrigFileName;
    }

    // export sub streams for package, else full stream into a file
    bool bWarn = false;

    // RDF metadata: export if ODF >= 1.2
    // N.B.: embedded documents have their own manifest.rdf!
    if ( bOASIS )
    {
        const uno::Reference<beans::XPropertySet> xPropSet(xStg,
            uno::UNO_QUERY_THROW);
        try
        {
            OUString Version;
            // ODF >= 1.2
            if ((xPropSet->getPropertyValue("Version") >>= Version)
                && Version != ODFVER_010_TEXT
                && Version != ODFVER_011_TEXT)
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
            bWarn = true;
        }
    }

    bool bStoreMeta = ( SfxObjectCreateMode::EMBEDDED != pDoc->GetDocShell()->GetCreateMode() );
    if ( !bStoreMeta )
    {
        try
        {
            Reference< frame::XModule > xModule( xModelComp, UNO_QUERY );
            if ( xModule.is() )
            {
                const OUString aModuleID = xModule->getIdentifier();
                bStoreMeta = !aModuleID.isEmpty() &&
                    ( aModuleID == "com.sun.star.sdb.FormDesign" ||
                      aModuleID == "com.sun.star.sdb.TextReportDesign" );
            }
        }
        catch( uno::Exception& )
        {}
    }

    OUString sWarnFile;
    if( !bOrganizerMode && !bBlock && bStoreMeta )
    {
        if( !WriteThroughComponent(
                xModelComp, "meta.xml", xContext,
                (bOASIS ? "com.sun.star.comp.Writer.XMLOasisMetaExporter"
                        : "com.sun.star.comp.Writer.XMLMetaExporter"),
                aEmptyArgs, aProps ) )
        {
            bWarn = true;
            sWarnFile = "meta.xml";
        }
    }

    if( !bBlock )
    {
        if( !WriteThroughComponent(
            xModelComp, "settings.xml", xContext,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisSettingsExporter"
                    : "com.sun.star.comp.Writer.XMLSettingsExporter"),
            aEmptyArgs, aProps ) )
        {
            if( !bWarn )
            {
                bWarn = true;
                sWarnFile = "settings.xml";
            }
        }
    }

    bool bErr = false;

    OUString sErrFile;
    if( !WriteThroughComponent(
            xModelComp, "styles.xml", xContext,
            (bOASIS ? "com.sun.star.comp.Writer.XMLOasisStylesExporter"
                    : "com.sun.star.comp.Writer.XMLStylesExporter"),
            aFilterArgs, aProps ) )
    {
        bErr = true;
        sErrFile = "styles.xml";
    }

    if( !bOrganizerMode && !bErr )
    {
        if( !WriteThroughComponent(
                xModelComp, "content.xml", xContext,
                (bOASIS ? "com.sun.star.comp.Writer.XMLOasisContentExporter"
                        : "com.sun.star.comp.Writer.XMLContentExporter"),
                aFilterArgs, aProps ) )
        {
            bErr = true;
            sErrFile = "content.xml";
        }
    }

    if( pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() && pDoc->getIDocumentStatistics().GetDocStat().nPage > 1 &&
        !(bOrganizerMode || bBlock || bErr) )
    {
        try
        {
            uno::Reference < io::XStream > xStm = xStg->openStreamElement( "layout-cache", embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
            SvStream* pStream = utl::UcbStreamHelper::CreateStream( xStm );
            if( !pStream->GetError() )
            {
                uno::Reference < beans::XPropertySet > xSet( xStm, UNO_QUERY );
                uno::Any aAny2;
                aAny2 <<= OUString("application/binary");
                xSet->setPropertyValue("MediaType", aAny2 );
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
    xGraphicResolver = nullptr;

    if( pObjectHelper )
        SvXMLEmbeddedObjectHelper::Destroy( pObjectHelper );
    xObjectResolver = nullptr;

    // restore redline mode
    aAny = xInfoSet->getPropertyValue( sShowChanges );
    nRedlineFlags = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    nRedlineFlags &= ~RedlineFlags::ShowMask;
    nRedlineFlags |= RedlineFlags::ShowInsert;
    if ( *o3tl::doAccess<bool>(aAny) )
        nRedlineFlags |= RedlineFlags::ShowDelete;
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags( nRedlineFlags );

    if (xStatusIndicator.is())
    {
        xStatusIndicator->end();
    }

    if( bErr )
    {
        if( !sErrFile.isEmpty() )
            return (new StringErrorInfo( ERR_WRITE_ERROR_FILE, sErrFile,
                                         ErrorHandlerFlags::ButtonsOk | ErrorHandlerFlags::MessageError ))->GetErrorCode();
        return ERR_SWG_WRITE_ERROR;
    }
    else if( bWarn )
    {
        if( !sWarnFile.isEmpty() )
            return (new StringErrorInfo( WARN_WRITE_ERROR_FILE, sWarnFile,
                                         ErrorHandlerFlags::ButtonsOk | ErrorHandlerFlags::MessageError ))->GetErrorCode();
        return WARN_SWG_FEATURES_LOST;
    }

    return 0;
}

sal_uLong SwXMLWriter::WriteStorage()
{
    return Write_( uno::Reference < task::XStatusIndicator >(), OUString() );
}

sal_uLong SwXMLWriter::WriteMedium( SfxMedium& aTargetMedium )
{
    uno::Reference < task::XStatusIndicator > xStatusIndicator;
    OUString aName;
    const SfxUnoAnyItem* pStatusBarItem = static_cast<const SfxUnoAnyItem*>(
       aTargetMedium.GetItemSet()->GetItem(SID_PROGRESS_STATUSBAR_CONTROL) );
    if ( pStatusBarItem )
        pStatusBarItem->GetValue() >>= xStatusIndicator;
    const SfxStringItem* pDocHierarchItem = static_cast<const SfxStringItem*>(
        aTargetMedium.GetItemSet()->GetItem(SID_DOC_HIERARCHICALNAME) );
    if ( pDocHierarchItem )
        aName = pDocHierarchItem->GetValue();

    return Write_( xStatusIndicator, aName );
}

sal_uLong SwXMLWriter::Write( SwPaM& rPaM, SfxMedium& rMed,
                               const OUString* pFileName )
{
    return IsStgWriter()
            ? static_cast<StgWriter *>(this)->Write( rPaM, rMed.GetOutputStorage(), pFileName, &rMed )
            : static_cast<Writer *>(this)->Write( rPaM, *rMed.GetOutStream(), pFileName );
}

bool SwXMLWriter::WriteThroughComponent(
    const uno::Reference<XComponent> & xComponent,
    const sal_Char* pStreamName,
    const uno::Reference<uno::XComponentContext> & rxContext,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<beans::PropertyValue> & rMediaDesc )
{
    OSL_ENSURE( xStg.is(), "Need storage!" );
    OSL_ENSURE( nullptr != pStreamName, "Need stream name!" );
    OSL_ENSURE( nullptr != pServiceName, "Need service name!" );

    SAL_INFO( "sw.filter", "SwXMLWriter::WriteThroughComponent : stream " << pStreamName );
    // open stream
    bool bRet = false;
    try
    {
        const OUString sStreamName = OUString::createFromAscii( pStreamName );
        uno::Reference<io::XStream> xStream =
                xStg->openStreamElement( sStreamName,
                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

        uno::Reference <beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
        if( !xSet.is() )
            return false;

        xSet->setPropertyValue("MediaType", Any(OUString("text/xml")) );

        // even plain stream should be encrypted in encrypted documents
        xSet->setPropertyValue( "UseCommonStoragePasswordEncryption", makeAny(true) );

        // set buffer and create outputstream
        uno::Reference< io::XOutputStream > xOutputStream = xStream->getOutputStream();

        // set Base URL
        uno::Reference< beans::XPropertySet > xInfoSet;
        if( rArguments.getLength() > 0 )
            rArguments.getConstArray()[0] >>= xInfoSet;
        OSL_ENSURE( xInfoSet.is(), "missing property set" );
        if( xInfoSet.is() )
        {
            xInfoSet->setPropertyValue( "StreamName", makeAny( sStreamName ) );
        }

        // write the stuff
        bRet = WriteThroughComponent(
            xOutputStream, xComponent, rxContext,
            pServiceName, rArguments, rMediaDesc );
    }
    catch ( uno::Exception& )
    {
    }

    return bRet;

}

bool SwXMLWriter::WriteThroughComponent(
    const uno::Reference<io::XOutputStream> & xOutputStream,
    const uno::Reference<XComponent> & xComponent,
    const uno::Reference<XComponentContext> & rxContext,
    const sal_Char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<PropertyValue> & rMediaDesc )
{
    OSL_ENSURE( xOutputStream.is(), "I really need an output stream!" );
    OSL_ENSURE( xComponent.is(), "Need component!" );
    OSL_ENSURE( nullptr != pServiceName, "Need component name!" );

    // get component
    uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(rxContext);
    SAL_INFO( "sw.filter", "SAX-Writer created" );
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
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pServiceName), aArgs, rxContext), UNO_QUERY);
    OSL_ENSURE( xExporter.is(),
            "can't instantiate export filter component" );
    if( !xExporter.is() )
        return false;
    SAL_INFO( "sw.filter", pServiceName << " instantiated." );
    // connect model and filter
    xExporter->setSourceDocument( xComponent );

    // filter!
    SAL_INFO( "sw.filter", "call filter()" );
    uno::Reference<XFilter> xFilter( xExporter, UNO_QUERY );
    return xFilter->filter( rMediaDesc );
}

void GetXMLWriter( const OUString& /*rName*/, const OUString& rBaseURL, WriterRef& xRet )
{
    xRet = new SwXMLWriter( rBaseURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
