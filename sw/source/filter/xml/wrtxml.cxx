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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/frame/XModule.hpp>

#include <officecfg/Office/Common.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <vcl/errinf.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>
#include <svl/stritem.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <pam.hxx>
#include <doc.hxx>
#include <docfunc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <docstat.hxx>
#include <docsh.hxx>

#include <xmloff/shapeexport.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <swerror.h>
#include "wrtxml.hxx"
#include "zorder.hxx"
#include <strings.hrc>

#include <comphelper/documentconstants.hxx>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
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

ErrCodeMsg SwXMLWriter::Write_(const SfxItemSet* pMediumItemSet)
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    OUString aDocHierarchicalName;
    bool bNoEmbDS(false);

    if (pMediumItemSet)
    {
        const SfxUnoAnyItem* pStatusBarItem =
           pMediumItemSet->GetItem(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pStatusBarItem)
            pStatusBarItem->GetValue() >>= xStatusIndicator;
        const SfxStringItem* pDocHierarchItem =
            pMediumItemSet->GetItem(SID_DOC_HIERARCHICALNAME);
        if (pDocHierarchItem)
            aDocHierarchicalName = pDocHierarchItem->GetValue();
        const SfxBoolItem* pNoEmbDS = pMediumItemSet->GetItem(SID_NO_EMBEDDED_DS);
        if (pNoEmbDS)
            bNoEmbDS = pNoEmbDS->GetValue();
    }

    // Get service factory
    uno::Reference< uno::XComponentContext > xContext =
            comphelper::getProcessComponentContext();

    // Get data sink ...
    uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;
    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper ;
    uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    rtl::Reference<SvXMLEmbeddedObjectHelper> xObjectHelper;

    OSL_ENSURE( m_xStg.is(), "Where is my storage?" );
    xGraphicHelper = SvXMLGraphicHelper::Create( m_xStg,
                                                 SvXMLGraphicHelperMode::Write );
    xGraphicStorageHandler = xGraphicHelper.get();

    SfxObjectShell *pPersist = m_pDoc->GetPersist();
    if( pPersist )
    {
        xObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                         m_xStg, *pPersist,
                                         SvXMLEmbeddedObjectHelperMode::Write );
        xObjectResolver = xObjectHelper.get();
    }

    // create and prepare the XPropertySet that gets passed through
    // the components, and the XStatusIndicator that shows progress to
    // the user.

    // create XPropertySet with three properties for status indicator
    static comphelper::PropertyMapEntry const aInfoMap[] =
    {
        { u"ProgressRange"_ustr, 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressMax"_ustr, 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressCurrent"_ustr, 0,
              ::cppu::UnoType<sal_Int32>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { u"WrittenNumberStyles"_ustr, 0,
              cppu::UnoType<uno::Sequence<sal_Int32>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { u"UsePrettyPrinting"_ustr, 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ShowChanges"_ustr, 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"RedlineProtectionKey"_ustr, 0,
              cppu::UnoType<Sequence<sal_Int8>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BaseURI"_ustr, 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamRelPath"_ustr, 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamName"_ustr, 0,
              ::cppu::UnoType<OUString>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"AutoTextMode"_ustr, 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StyleNames"_ustr, 0,
              cppu::UnoType<Sequence<OUString>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StyleFamilies"_ustr, 0,
              cppu::UnoType<Sequence<sal_Int32>>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        // #i69627#
        { u"OutlineStyleAsNormalListStyle"_ustr, 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"TargetStorage"_ustr,0, cppu::UnoType<embed::XStorage>::get(),
              css::beans::PropertyAttribute::MAYBEVOID, 0 },
        // tdf#144532
        { u"NoEmbDataSet"_ustr, 0,
              cppu::UnoType<bool>::get(),
              beans::PropertyAttribute::MAYBEVOID, 0 },
    };
    uno::Reference< beans::XPropertySet > xInfoSet(
                comphelper::GenericPropertySet_CreateInstance(
                            new comphelper::PropertySetInfo( aInfoMap ) ) );

    xInfoSet->setPropertyValue( u"TargetStorage"_ustr, Any( m_xStg ) );

    xInfoSet->setPropertyValue(u"NoEmbDataSet"_ustr, Any(bNoEmbDS));

    if (m_bShowProgress)
    {
        // set progress range and start status indicator
        sal_Int32 nProgressRange(1000000);
        if (xStatusIndicator.is())
        {
            xStatusIndicator->start(SwResId( STR_STATSTR_SWGWRITE),
                                    nProgressRange);
        }
        xInfoSet->setPropertyValue(u"ProgressRange"_ustr, Any(nProgressRange));

        xInfoSet->setPropertyValue(u"ProgressMax"_ustr, Any(static_cast < sal_Int32 >( -1 )));
    }

    xInfoSet->setPropertyValue( u"UsePrettyPrinting"_ustr, Any(officecfg::Office::Common::Save::Document::PrettyPrinting::get()) );

    uno::Reference<lang::XComponent> const xModelComp(m_pDoc->GetDocShell()->GetModel());
    uno::Reference<drawing::XDrawPageSupplier> const xDPS(xModelComp, uno::UNO_QUERY);
    assert(xDPS.is());
    xmloff::FixZOrder(xDPS->getDrawPage(), sw::GetZOrderLayer(m_pDoc->getIDocumentDrawModelAccess()));

    // save show redline mode ...
    RedlineFlags const nOrigRedlineFlags = m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    RedlineFlags nRedlineFlags(nOrigRedlineFlags);
    bool isShowChanges;
    // TODO: ideally this would be stored per-view...
    SwRootFrame const*const pLayout(m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout());
    isShowChanges = pLayout == nullptr || !pLayout->IsHideRedlines();
    xInfoSet->setPropertyValue(u"ShowChanges"_ustr, Any(isShowChanges));
    // ... and hide redlines for export
    nRedlineFlags &= ~RedlineFlags::ShowMask;
    nRedlineFlags |= RedlineFlags::ShowInsert;
    m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags( nRedlineFlags );

    // Set base URI
    xInfoSet->setPropertyValue( u"BaseURI"_ustr, Any( GetBaseURL() ) );

    if( SfxObjectCreateMode::EMBEDDED == m_pDoc->GetDocShell()->GetCreateMode() )
    {
        const OUString aName( !aDocHierarchicalName.isEmpty()
            ? aDocHierarchicalName
            : u"dummyObjectName"_ustr );

        xInfoSet->setPropertyValue( u"StreamRelPath"_ustr, Any( aName ) );
    }

    if( m_bBlock )
    {
        xInfoSet->setPropertyValue( u"AutoTextMode"_ustr, Any(true) );
    }

    // #i69627#
    const bool bOASIS = ( SotStorage::GetVersion( m_xStg ) > SOFFICE_FILEFORMAT_60 );
    if ( bOASIS &&
         docfunc::HasOutlineStyleToBeWrittenAsNormalListStyle( *m_pDoc ) )
    {
        xInfoSet->setPropertyValue( u"OutlineStyleAsNormalListStyle"_ustr, Any( true ) );
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

    if( xGraphicStorageHandler.is() )
        nArgs++;
    if( xObjectResolver.is() )
        nArgs++;

    Sequence < Any > aFilterArgs( nArgs );
    pArgs = aFilterArgs.getArray();
    *pArgs++ <<= xInfoSet;
    if( xGraphicStorageHandler.is() )
        *pArgs++ <<= xGraphicStorageHandler;
    if( xObjectResolver.is() )
        *pArgs++ <<= xObjectResolver;
    if( xStatusIndicator.is() )
        *pArgs++ <<= xStatusIndicator;

    PutNumFormatFontsInAttrPool();
    PutEditEngFontsInAttrPool();

    // properties
    Sequence < PropertyValue > aProps( m_pOrigFileName ? 1 : 0 );
    if( m_pOrigFileName )
    {
        PropertyValue *pProps = aProps.getArray();
        pProps->Name = "FileName";
        pProps->Value <<= *m_pOrigFileName;
    }

    // export sub streams for package, else full stream into a file
    bool bWarn = false;

    // RDF metadata: export if ODF >= 1.2
    // N.B.: embedded documents have their own manifest.rdf!
    if ( bOASIS )
    {
        const uno::Reference<beans::XPropertySet> xPropSet(m_xStg,
            uno::UNO_QUERY_THROW);
        try
        {
            OUString Version;
            // ODF >= 1.2
            if ((xPropSet->getPropertyValue(u"Version"_ustr) >>= Version)
                && Version != ODFVER_010_TEXT
                && Version != ODFVER_011_TEXT)
            {
                const uno::Reference<rdf::XDocumentMetadataAccess> xDMA(
                    xModelComp, uno::UNO_QUERY_THROW);
                xDMA->storeMetadataToStorage(m_xStg);
            }
        }
        catch (beans::UnknownPropertyException &)
        { /* ignore */ }
        catch (uno::Exception &)
        {
            bWarn = true;
        }
    }

    bool bStoreMeta = ( SfxObjectCreateMode::EMBEDDED != m_pDoc->GetDocShell()->GetCreateMode() );
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
    if( !m_bOrganizerMode && !m_bBlock && bStoreMeta )
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

    if( !m_bBlock )
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

    if( !m_bOrganizerMode && !bErr )
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

    if( m_pDoc->getIDocumentLayoutAccess().GetCurrentViewShell() && m_pDoc->getIDocumentStatistics().GetDocStat().nPage > 1 &&
        !(m_bOrganizerMode || m_bBlock || bErr ||
            // sw_redlinehide: disable layout cache for now
            m_pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->HasMergedParas()))
    {
        try
        {
            uno::Reference < io::XStream > xStm = m_xStg->openStreamElement( u"layout-cache"_ustr, embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
            std::unique_ptr<SvStream> pStream = utl::UcbStreamHelper::CreateStream( xStm );
            if( !pStream->GetError() )
            {
                uno::Reference < beans::XPropertySet > xSet( xStm, UNO_QUERY );
                uno::Any aAny2;
                aAny2 <<= u"application/binary"_ustr;
                xSet->setPropertyValue(u"MediaType"_ustr, aAny2 );
                m_pDoc->WriteLayoutCache( *pStream );
            }
        }
        catch ( uno::Exception& )
        {
        }
    }

    if( xGraphicHelper )
        xGraphicHelper->dispose();
    xGraphicHelper.clear();
    xGraphicStorageHandler = nullptr;

    if( xObjectHelper )
        xObjectHelper->dispose();
    xObjectHelper.clear();
    xObjectResolver = nullptr;

    // restore redline mode
    nRedlineFlags = m_pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    nRedlineFlags &= ~RedlineFlags::ShowMask;
    nRedlineFlags |= RedlineFlags::ShowInsert;
    nRedlineFlags |= nOrigRedlineFlags & RedlineFlags::ShowMask;
    m_pDoc->getIDocumentRedlineAccess().SetRedlineFlags( nRedlineFlags );

    // tdf#115815 restore annotation ranges collapsed by hide redlines
    m_pDoc->getIDocumentMarkAccess()->restoreAnnotationMarks();

    if (xStatusIndicator.is())
    {
        xStatusIndicator->end();
    }

    if( bErr )
    {
        if( !sErrFile.isEmpty() )
            return ErrCodeMsg( ERR_WRITE_ERROR_FILE, sErrFile,
                                         DialogMask::ButtonsOk | DialogMask::MessageError );
        return ERR_SWG_WRITE_ERROR;
    }
    else if( bWarn )
    {
        if( !sWarnFile.isEmpty() )
            return ErrCodeMsg( WARN_WRITE_ERROR_FILE, sWarnFile,
                                         DialogMask::ButtonsOk | DialogMask::MessageError );
        return WARN_SWG_FEATURES_LOST;
    }

    return ERRCODE_NONE;
}

ErrCodeMsg SwXMLWriter::WriteStorage()
{
    return Write_(nullptr);
}

ErrCodeMsg SwXMLWriter::WriteMedium( SfxMedium& aTargetMedium )
{
    return Write_(&aTargetMedium.GetItemSet());
}

ErrCodeMsg SwXMLWriter::Write( SwPaM& rPaM, SfxMedium& rMed,
                               const OUString* pFileName )
{
    return IsStgWriter()
            ? static_cast<StgWriter *>(this)->Write( rPaM, rMed.GetOutputStorage(), pFileName, &rMed )
            : static_cast<Writer *>(this)->Write( rPaM, *rMed.GetOutStream(), pFileName );
}

bool SwXMLWriter::WriteThroughComponent(
    const uno::Reference<XComponent> & xComponent,
    const char* pStreamName,
    const uno::Reference<uno::XComponentContext> & rxContext,
    const char* pServiceName,
    const Sequence<Any> & rArguments,
    const Sequence<beans::PropertyValue> & rMediaDesc )
{
    OSL_ENSURE( m_xStg.is(), "Need storage!" );
    OSL_ENSURE( nullptr != pStreamName, "Need stream name!" );
    OSL_ENSURE( nullptr != pServiceName, "Need service name!" );

    SAL_INFO( "sw.filter", "SwXMLWriter::WriteThroughComponent : stream " << pStreamName );
    // open stream
    bool bRet = false;
    try
    {
        const OUString sStreamName = OUString::createFromAscii( pStreamName );
        uno::Reference<io::XStream> xStream =
                m_xStg->openStreamElement( sStreamName,
                embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

        uno::Reference <beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
        if( !xSet.is() )
            return false;

        xSet->setPropertyValue(u"MediaType"_ustr, Any(u"text/xml"_ustr) );

        // even plain stream should be encrypted in encrypted documents
        xSet->setPropertyValue( u"UseCommonStoragePasswordEncryption"_ustr, Any(true) );

        // set buffer and create outputstream
        uno::Reference< io::XOutputStream > xOutputStream = xStream->getOutputStream();

        // set Base URL
        uno::Reference< beans::XPropertySet > xInfoSet;
        if( rArguments.hasElements() )
            rArguments.getConstArray()[0] >>= xInfoSet;
        OSL_ENSURE( xInfoSet.is(), "missing property set" );
        if( xInfoSet.is() )
        {
            xInfoSet->setPropertyValue( u"StreamName"_ustr, Any( sStreamName ) );
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
    const char* pServiceName,
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
    Sequence<Any> aArgs( 1 + rArguments.getLength() );
    auto pArgs = aArgs.getArray();
    *pArgs <<= xSaxWriter;
    std::copy(rArguments.begin(), rArguments.end(), std::next(pArgs));

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

void GetXMLWriter(
    [[maybe_unused]] std::u16string_view /*rName*/, const OUString& rBaseURL, WriterRef& xRet )
{
    xRet = new SwXMLWriter( rBaseURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
