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

#include <officecfg/Office/Common.hxx>
#include <vcl/errinf.hxx>
#include <sal/log.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <o3tl/string_view.hxx>
#include <editeng/outlobj.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfxsids.hrc>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <Outliner.hxx>
#include <unotools/streamwrap.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xmlgrhlp.hxx>
#include <xmloff/xmlexp.hxx>

#include <DrawDocShell.hxx>

#include <sdxmlwrp.hxx>
#include <svx/xmleohlp.hxx>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>

#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <comphelper/genericpropertyset.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <editeng/eeitem.hxx>

// include necessary for XML progress bar at load time
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <svtools/sfxecode.hxx>

#include <sddll.hxx>
#include <sderror.hxx>
#include <sdresid.hxx>
#include "sdtransform.hxx"
#include <strings.hrc>

#include <sfx2/frame.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::document;
using namespace comphelper;

#define SD_XML_READERROR ErrCode(1234)

char const sXML_export_impress_meta_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisMetaExporter";
char const sXML_export_impress_styles_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisStylesExporter";
char const sXML_export_impress_content_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisContentExporter";
char const sXML_export_impress_settings_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisSettingsExporter";

char const sXML_export_draw_meta_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisMetaExporter";
char const sXML_export_draw_styles_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisStylesExporter";
char const sXML_export_draw_content_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisContentExporter";
char const sXML_export_draw_settings_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisSettingsExporter";

char const sXML_import_impress_meta_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisMetaImporter";
char const sXML_import_impress_styles_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisStylesImporter";
char const sXML_import_impress_content_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisContentImporter";
char const sXML_import_impress_settings_oasis_service[] = "com.sun.star.comp.Impress.XMLOasisSettingsImporter";

char const sXML_import_draw_meta_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisMetaImporter";
char const sXML_import_draw_styles_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisStylesImporter";
char const sXML_import_draw_content_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisContentImporter";
char const sXML_import_draw_settings_oasis_service[] = "com.sun.star.comp.Draw.XMLOasisSettingsImporter";

// OOo
char const sXML_import_impress_meta_ooo_service[] = "com.sun.star.comp.Impress.XMLMetaImporter";
char const sXML_import_impress_styles_ooo_service[] = "com.sun.star.comp.Impress.XMLStylesImporter";
char const sXML_import_impress_content_ooo_service[] = "com.sun.star.comp.Impress.XMLContentImporter";
char const sXML_import_impress_settings_ooo_service[] = "com.sun.star.comp.Impress.XMLSettingsImporter";

char const sXML_import_draw_meta_ooo_service[] = "com.sun.star.comp.Draw.XMLMetaImporter";
char const sXML_import_draw_styles_ooo_service[] = "com.sun.star.comp.Draw.XMLStylesImporter";
char const sXML_import_draw_content_ooo_service[] = "com.sun.star.comp.Draw.XMLContentImporter";
char const sXML_import_draw_settings_ooo_service[] = "com.sun.star.comp.Draw.XMLSettingsImporter";

namespace {

struct XML_SERVICEMAP
{
    const char* mpService;
    const char* mpStream;
};

struct XML_SERVICES
{
    const char* mpMeta;
    const char* mpStyles;
    const char* mpContent;
    const char* mpSettings;
};

}

static XML_SERVICES const * getServices( bool bImport, bool bDraw, sal_uLong nStoreVer )
{
    // Expect that export always sets nStoreVer to SOFFICE_FILEFORMAT_8.
    assert(bImport || nStoreVer != SOFFICE_FILEFORMAT_60);

    static XML_SERVICES const gServices[] =
    {
        { sXML_import_impress_meta_oasis_service, sXML_import_impress_styles_oasis_service, sXML_import_impress_content_oasis_service, sXML_import_impress_settings_oasis_service },
        { sXML_import_draw_meta_oasis_service, sXML_import_draw_styles_oasis_service, sXML_import_draw_content_oasis_service, sXML_import_draw_settings_oasis_service },
        { sXML_export_impress_meta_oasis_service, sXML_export_impress_styles_oasis_service, sXML_export_impress_content_oasis_service, sXML_export_impress_settings_oasis_service },
        { sXML_export_draw_meta_oasis_service, sXML_export_draw_styles_oasis_service, sXML_export_draw_content_oasis_service, sXML_export_draw_settings_oasis_service },

        { sXML_import_impress_meta_ooo_service, sXML_import_impress_styles_ooo_service, sXML_import_impress_content_ooo_service, sXML_import_impress_settings_ooo_service },
        { sXML_import_draw_meta_ooo_service, sXML_import_draw_styles_ooo_service, sXML_import_draw_content_ooo_service, sXML_import_draw_settings_ooo_service },
    };

    return &gServices[ (bImport ? 0 : 2) + ((nStoreVer == SOFFICE_FILEFORMAT_60) ? 4 : 0) + (bDraw ? 1 : 0 ) ];
}


SdXMLFilter::SdXMLFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell, SdXMLFilterMode eFilterMode, sal_uLong nStoreVer ) :
    SdFilter( rMedium, rDocShell ), meFilterMode( eFilterMode ), mnStoreVer( nStoreVer )
{
}

SdXMLFilter::~SdXMLFilter()
{
}

namespace
{

ErrCodeMsg ReadThroughComponent(
    const Reference<io::XInputStream>& xInputStream,
    const Reference<XComponent>& xModelComponent,
    const OUString& rStreamName,
    Reference<uno::XComponentContext> const & rxContext,
    const char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessful,
    bool bEncrypted )
{
    DBG_ASSERT(xInputStream.is(), "input stream missing");
    DBG_ASSERT(xModelComponent.is(), "document missing");
    DBG_ASSERT(rxContext.is(), "factory missing");
    DBG_ASSERT(nullptr != pFilterName,"I need a service name for the component!");

    SAL_INFO( "sd.filter", "ReadThroughComponent" );

    // prepare ParserInputSource
    xml::sax::InputSource aParserInput;
    aParserInput.sSystemId = rName;
    aParserInput.aInputStream = xInputStream;

    // get filter
    OUString aFilterName(OUString::createFromAscii(pFilterName));
    // the underlying SvXMLImport implements XFastParser, XImporter, XFastDocumentHandler
    Reference< XInterface > xFilter(
        rxContext->getServiceManager()->createInstanceWithArgumentsAndContext(aFilterName, rFilterArguments, rxContext),
        UNO_QUERY );
    SAL_WARN_IF(!xFilter.is(), "sd.filter", "Can't instantiate filter component: " << aFilterName);
    if( !xFilter.is() )
        return SD_XML_READERROR;
    Reference< xml::sax::XFastParser > xFastParser(xFilter, UNO_QUERY);
    Reference< xml::sax::XDocumentHandler > xDocumentHandler;
    if (!xFastParser)
        xDocumentHandler.set(xFilter, UNO_QUERY);
    if (!xFastParser && !xDocumentHandler)
    {
        SAL_WARN("sd", "service does not implement XFastParser or XDocumentHandler");
        assert(false);
        return SD_XML_READERROR;
    }
    SAL_INFO( "sd.filter", "" << pFilterName << " created" );

    // connect model and filter
    Reference < XImporter > xImporter( xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );

    // finally, parser the stream
    SAL_INFO( "sd.filter", "parsing stream" );
    try
    {
        if (xFastParser)
            xFastParser->parseStream( aParserInput );
        else
        {
            Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(rxContext);
            // connect parser and filter
            xParser->setDocumentHandler( xDocumentHandler );
            xParser->parseStream( aParserInput );
        }
    }
    catch (const xml::sax::SAXParseException& r)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        // sax parser sends wrapped exceptions,
        // try to find the original one
        xml::sax::SAXException aSaxEx = *static_cast<xml::sax::SAXException const *>(&r);
        bool bTryChild = true;

        while( bTryChild )
        {
            xml::sax::SAXException aTmp;
            if ( aSaxEx.WrappedException >>= aTmp )
                aSaxEx = std::move(aTmp);
            else
                bTryChild = false;
        }

        packages::zip::ZipIOException aBrokenPackage;
        if ( aSaxEx.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

        SAL_WARN( "sd.filter", "SAX parse exception caught while importing: " << exceptionToString(ex));

        OUString sErr =  OUString::number( r.LineNumber ) +
            "," + OUString::number( r.ColumnNumber );

        if (!rStreamName.isEmpty())
        {
            return ErrCodeMsg(
                            (bMustBeSuccessful ? ERR_FORMAT_FILE_ROWCOL
                                                    : WARN_FORMAT_FILE_ROWCOL),
                            rStreamName, sErr,
                            DialogMask::ButtonsOk | DialogMask::MessageError );
        }
        else
        {
            DBG_ASSERT( bMustBeSuccessful, "Warnings are not supported" );
            return ErrCodeMsg( ERR_FORMAT_ROWCOL, sErr,
                             DialogMask::ButtonsOk | DialogMask::MessageError );
        }
    }
    catch (const xml::sax::SAXException& r)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        packages::zip::ZipIOException aBrokenPackage;
        if ( r.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;

        if( bEncrypted )
            return ERRCODE_SFX_WRONGPASSWORD;

        SAL_WARN( "sd.filter", "SAX exception caught while importing: " << exceptionToString(ex));
        return SD_XML_READERROR;
    }
    catch (const packages::zip::ZipIOException&)
    {
        TOOLS_WARN_EXCEPTION( "sd.filter", "Zip exception caught while importing");
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const io::IOException&)
    {
        TOOLS_WARN_EXCEPTION( "sd.filter", "IO exception caught while importing");
        return SD_XML_READERROR;
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sd.filter", "uno exception caught while importing");
        return SD_XML_READERROR;
    }

    // success!
    return ERRCODE_NONE;
}

ErrCodeMsg ReadThroughComponent(
    const uno::Reference < embed::XStorage >& xStorage,
    const Reference<XComponent>& xModelComponent,
    const char* pStreamName,
    Reference<uno::XComponentContext> const & rxContext,
    const char* pFilterName,
    const Sequence<Any>& rFilterArguments,
    const OUString& rName,
    bool bMustBeSuccessful )
{
    DBG_ASSERT(xStorage.is(), "Need storage!");
    DBG_ASSERT(nullptr != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    bool bContainsStream = false;
    try
    {
        bContainsStream = xStorage->isStreamElement(sStreamName);
    }
    catch (const container::NoSuchElementException&)
    {
    }

    if (!bContainsStream )
    {
        // stream name not found! return immediately with OK signal
        return ERRCODE_NONE;
    }

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( rFilterArguments.hasElements() )
        rFilterArguments.getConstArray()[0] >>= xInfoSet;
    DBG_ASSERT( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        xInfoSet->setPropertyValue( u"StreamName"_ustr, Any( sStreamName ) );
    }

    try
    {
        // get input stream
        Reference <io::XStream> xStream =
                xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );
        Reference <beans::XPropertySet > xProps( xStream, uno::UNO_QUERY );
        if ( !xStream.is() || ! xProps.is() )
            return SD_XML_READERROR;

        Any aAny = xProps->getPropertyValue( u"Encrypted"_ustr );

        bool bEncrypted = false;
        aAny >>= bEncrypted;

        Reference <io::XInputStream> xInputStream = xStream->getInputStream();

        // read from the stream
        return ReadThroughComponent(
            xInputStream, xModelComponent, sStreamName, rxContext,
            pFilterName, rFilterArguments,
            rName, bMustBeSuccessful, bEncrypted );
    }
    catch (const packages::WrongPasswordException&)
    {
        return ERRCODE_SFX_WRONGPASSWORD;
    }
    catch (const packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const uno::Exception&)
    {}

    return SD_XML_READERROR;
}

}

//PresObjKind::Outlines in master pages are the preview of the outline styles
//numbering format. Since fdo#78151 toggling bullets on and off changes
//the style they are a preview of, previously toggling bullets on and off
//would only affect the preview paragraph itself without an effect on the
//style. i.e.  previews of numbering which don't match the real numbering
//they are supposed to be a preview of.
//
//But there exist documents which were saved previous to that modification
//so here we detect such cases and fix them up to ensure the previews
//numbering level matches that of the outline level it previews
static void fixupOutlinePlaceholderNumberingDepths(SdDrawDocument* pDoc)
{
    for (sal_uInt16 i = 0; i < pDoc->GetMasterSdPageCount(PageKind::Standard); ++i)
    {
        SdPage *pMasterPage = pDoc->GetMasterSdPage(i, PageKind::Standard);
        SdrObject* pMasterOutline = pMasterPage->GetPresObj(PresObjKind::Outline);
        if (!pMasterOutline)
            continue;
        OutlinerParaObject* pOutlParaObj = pMasterOutline->GetOutlinerParaObject();
        if (!pOutlParaObj)
            continue;
        SdOutliner* pOutliner = pDoc->GetInternalOutliner();
        pOutliner->Clear();
        pOutliner->SetText(*pOutlParaObj);
        bool bInconsistent = false;
        const sal_Int32 nParaCount = pOutliner->GetParagraphCount();
        for (sal_Int32 j = 0; j < nParaCount; ++j)
        {
            //Make sure the depth of the paragraph matches that of the outline style it previews
            const sal_Int16 nExpectedDepth = j;
            if (nExpectedDepth != pOutliner->GetDepth(j))
            {
                Paragraph* p = pOutliner->GetParagraph(j);
                pOutliner->SetDepth(p, nExpectedDepth);
                bInconsistent = true;
            }

            //If the preview has hard-coded bullets/numbering then they must
            //be stripped to reveal the true underlying styles attributes
            SfxItemSet aAttrs(pOutliner->GetParaAttribs(j));
            if (aAttrs.GetItemState(EE_PARA_NUMBULLET) == SfxItemState::SET)
            {
                aAttrs.ClearItem(EE_PARA_NUMBULLET);
                pOutliner->SetParaAttribs(j, aAttrs);
                bInconsistent = true;
            }

        }
        if (bInconsistent)
        {
            SAL_WARN("sd.filter", "Fixing inconsistent outline numbering placeholder preview");
            pMasterOutline->SetOutlinerParaObject(pOutliner->CreateParaObject(0, nParaCount));
        }
        pOutliner->Clear();
    }
}

bool SdXMLFilter::Import( ErrCode& nError )
{
    ErrCodeMsg nRet = ERRCODE_NONE;

    // Get service factory
    const Reference< uno::XComponentContext >& rxContext =
            comphelper::getProcessComponentContext();

    SdDrawDocument* pDoc = mrDocShell.GetDoc();
    bool const bWasUndo(pDoc->IsUndoEnabled());
    pDoc->EnableUndo(false);
    pDoc->NewOrLoadCompleted( DocCreationMode::New );
    pDoc->CreateFirstPages();
    pDoc->StopWorkStartupDelay();

    mxModel->lockControllers();

    /** property map for import info set */
    static PropertyMapEntry const aImportInfoMap[] =
    {
        // necessary properties for XML progress bar at load time
        { u"ProgressRange"_ustr,   0, cppu::UnoType<sal_Int32>::get(),               css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressMax"_ustr,     0, cppu::UnoType<sal_Int32>::get(),               css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressCurrent"_ustr, 0, cppu::UnoType<sal_Int32>::get(),               css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"Preview"_ustr,         0, cppu::UnoType<sal_Bool>::get(),                css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"PageLayouts"_ustr,     0, cppu::UnoType<container::XNameAccess>::get(),  css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"PrivateData"_ustr,     0, cppu::UnoType<XInterface>::get(),              css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BaseURI"_ustr,         0, cppu::UnoType<OUString>::get(),                css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamRelPath"_ustr,   0, cppu::UnoType<OUString>::get(),                css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamName"_ustr,      0, cppu::UnoType<OUString>::get(),                css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BuildId"_ustr,         0, cppu::UnoType<OUString>::get(),                css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"OrganizerMode"_ustr,   0, cppu::UnoType<bool>::get(),                    css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"SourceStorage"_ustr,   0, cppu::UnoType<embed::XStorage>::get(),         css::beans::PropertyAttribute::MAYBEVOID, 0 },
    };

    uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aImportInfoMap ) ) );
    xInfoSet->setPropertyValue( u"Preview"_ustr , uno::Any( mrDocShell.GetDoc()->IsStarDrawPreviewMode() ) );

    // ---- get BuildId from parent container if available

    uno::Reference< container::XChild > xChild( mxModel, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference< beans::XPropertySet > xParentSet( xChild->getParent(), uno::UNO_QUERY );
        if( xParentSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xParentSet->getPropertySetInfo() );
            OUString sPropName( u"BuildId"_ustr );
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(sPropName) )
            {
                xInfoSet->setPropertyValue( sPropName, xParentSet->getPropertyValue(sPropName) );
            }
        }
    }

    uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;
    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper;
    Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    rtl::Reference<SvXMLEmbeddedObjectHelper> xObjectHelper;

    Reference< lang::XComponent > xModelComp = mxModel;

    // try to get an XStatusIndicator from the Medium
    {
        const SfxUnoAnyItem* pItem = mrMedium.GetItemSet().GetItem(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pItem)
        {
            pItem->GetValue() >>= mxStatusIndicator;
        }

        if(mxStatusIndicator.is())
        {
            sal_Int32 nProgressRange(1000000);
            OUString aMsg(SvxResId(RID_SVXSTR_DOC_LOAD));
            mxStatusIndicator->start(aMsg, nProgressRange);

            // set ProgressRange
            uno::Any aProgRange;
            aProgRange <<= nProgressRange;
            xInfoSet->setPropertyValue( u"ProgressRange"_ustr , aProgRange);

            // set ProgressCurrent
            uno::Any aProgCurrent;
            aProgCurrent <<= sal_Int32(0);
            xInfoSet->setPropertyValue( u"ProgressCurrent"_ustr , aProgCurrent);
        }
    }

    // get the input stream (storage or stream)

    uno::Reference < embed::XStorage > xStorage = mrMedium.GetStorage();

    xInfoSet->setPropertyValue( u"SourceStorage"_ustr, Any( xStorage ) );

    if( !xStorage.is() )
        nRet = SD_XML_READERROR;

    if( ERRCODE_NONE == nRet )
    {
        xGraphicHelper = SvXMLGraphicHelper::Create( xStorage,
                                                     SvXMLGraphicHelperMode::Read );
        xGraphicStorageHandler = xGraphicHelper.get();
        xObjectHelper = SvXMLEmbeddedObjectHelper::Create(
                                    xStorage, *pDoc->GetPersist(),
                                    SvXMLEmbeddedObjectHelperMode::Read );
        xObjectResolver = xObjectHelper.get();
    }

    // Set base URI
    OUString const baseURI(mrMedium.GetBaseURL());
    // needed for relative URLs, but in clipboard copy/paste there may be none
    SAL_INFO_IF(baseURI.isEmpty(), "sd.filter", "SdXMLFilter: no base URL");
    xInfoSet->setPropertyValue(u"BaseURI"_ustr, Any(baseURI));

    if( ERRCODE_NONE == nRet && SfxObjectCreateMode::EMBEDDED == mrDocShell.GetCreateMode() )
    {
        OUString aName;
        const SfxStringItem* pDocHierarchItem =
            mrMedium.GetItemSet().GetItem(SID_DOC_HIERARCHICALNAME);
        if ( pDocHierarchItem )
            aName = pDocHierarchItem->GetValue();
        else
            aName = "dummyObjectName" ;

        if( !aName.isEmpty() )
            xInfoSet->setPropertyValue( u"StreamRelPath"_ustr, Any( aName ) );
    }

    if (SdXMLFilterMode::Organizer == meFilterMode)
        xInfoSet->setPropertyValue(u"OrganizerMode"_ustr, uno::Any(true));

    if( ERRCODE_NONE == nRet )
    {

        // prepare filter arguments
        Sequence<Any> aFilterArgs( 4 );
        Any *pArgs = aFilterArgs.getArray();
        *pArgs++ <<= xInfoSet;
        *pArgs++ <<= xGraphicStorageHandler;
        *pArgs++ <<= xObjectResolver;
        *pArgs++ <<= mxStatusIndicator;

        Sequence<Any> aEmptyArgs( 2 );
        pArgs = aEmptyArgs.getArray();
        *pArgs++ <<= xInfoSet;
        *pArgs++ <<= mxStatusIndicator;

        const OUString aName( mrMedium.GetName() );

        XML_SERVICES const * pServices = getServices( true, IsDraw(), mnStoreVer );

        ErrCodeMsg nWarn = ERRCODE_NONE;
        ErrCodeMsg nWarn2 = ERRCODE_NONE;
        // read storage streams
        // #i103539#: always read meta.xml for generator
        nWarn = ReadThroughComponent(
            xStorage, xModelComp, "meta.xml", rxContext,
            pServices->mpMeta,
            aEmptyArgs, aName, false );

        if( meFilterMode != SdXMLFilterMode::Organizer )
        {
            nWarn2 = ReadThroughComponent(
                xStorage, xModelComp, "settings.xml", rxContext,
                pServices->mpSettings,
                aFilterArgs, aName, false );
        }

        nRet = ReadThroughComponent(
            xStorage, xModelComp, "styles.xml", rxContext,
            pServices->mpStyles,
            aFilterArgs, aName, true );

        if( !nRet && (meFilterMode != SdXMLFilterMode::Organizer) )
            nRet = ReadThroughComponent(
               xStorage, xModelComp, "content.xml", rxContext,
               pServices->mpContent,
               aFilterArgs, aName, true );

        if( !nRet )
        {
            if( nWarn )
                nRet = std::move(nWarn);
            else if( nWarn2 )
                nRet = std::move(nWarn2);
        }
    }

    if( xGraphicHelper )
        xGraphicHelper->dispose();
    xGraphicHelper.clear();
    xGraphicStorageHandler = nullptr;
    if( xObjectHelper.is() )
        xObjectHelper->dispose();
    xObjectHelper.clear();
    xObjectResolver = nullptr;

    if( mxStatusIndicator.is() )
        mxStatusIndicator->end();

    if( mxModel.is() )
        mxModel->unlockControllers();

    if( nRet == ERRCODE_NONE )
        pDoc->UpdateAllLinks();

    if( nRet == ERRCODE_NONE || nRet == SD_XML_READERROR )
        ;
    else if( nRet == ERRCODE_IO_BROKENPACKAGE && xStorage.is() )
        nError = ERRCODE_IO_BROKENPACKAGE;
    else
    {
        // TODO/LATER: this is completely wrong! Filter code should never call ErrorHandler directly!
        ErrorHandler::HandleError( nRet );
        if( nRet.IsWarning() )
            nRet = ERRCODE_NONE;
    }

    // clear unused named items from item pool

    ::svx::DropUnusedNamedItems(mxModel);

    // set BuildId on XModel for later OLE object loading
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySet > xModelSet( mxModel, uno::UNO_QUERY );
        if( xModelSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xModelSetInfo( xModelSet->getPropertySetInfo() );
            static constexpr OUString sPropName( u"BuildId"_ustr );

            OUString sBuildId;
            xInfoSet->getPropertyValue(sPropName) >>= sBuildId;

            if( xModelSetInfo.is() && xModelSetInfo->hasPropertyByName(sPropName) )
            {
                xModelSet->setPropertyValue( sPropName, Any( sBuildId ) );
            }

            bool bTransform = false;

            if( nRet == ERRCODE_NONE )
            {
                if( !sBuildId.isEmpty() )
                {
                    sal_Int32 nIndex = sBuildId.indexOf('$');
                    if (sBuildId.indexOf(';') == -1 && nIndex != -1)
                    {
                        sal_Int32 nUPD = o3tl::toInt32(sBuildId.subView( 0, nIndex ));

                        if( nUPD == 300 )
                        {
                            sal_Int32 nBuildId = o3tl::toInt32(sBuildId.subView( nIndex+1 ));
                            if( (nBuildId > 0) && (nBuildId < 9316) )
                                bTransform = true; // treat OOo 3.0 beta1 as OOo 2.x
                        }
                        else if( (nUPD == 680) || ( nUPD >= 640 && nUPD <= 645 ) )
                            bTransform = true;
                    }
                }
                else
                {
                    // check for binary formats
                    std::shared_ptr<const SfxFilter> pFilter = mrMedium.GetFilter();
                    if( pFilter )
                    {
                        OUString typeName(pFilter->GetRealTypeName());
                        if( typeName.startsWith( "impress_StarImpress" ) ||
                            typeName.startsWith( "draw_StarDraw" ) )
                        {
                            bTransform = true;
                        }
                    }
                }
            }

            if( bTransform )
                TransformOOo2xDocument( pDoc );
        }
    }

    fixupOutlinePlaceholderNumberingDepths(pDoc);

    pDoc->EnableUndo(bWasUndo);
    mrDocShell.ClearUndoBuffer();
    return nRet == ERRCODE_NONE;
}

bool IsSlideSorterPaste(::sd::DrawDocShell& rDocSh)
{
    uno::Reference<document::XDocumentProperties> xSource = rDocSh.getDocProperties();
    uno::Reference<beans::XPropertyContainer> xSourcePropertyContainer = xSource->getUserDefinedProperties();
    uno::Reference<beans::XPropertySet> xSourcePropertySet(xSourcePropertyContainer, uno::UNO_QUERY);
    if (!xSourcePropertySet)
        return false;

    const uno::Sequence<beans::Property> lProps = xSourcePropertySet->getPropertySetInfo()->getProperties();
    for (const beans::Property& rProp : lProps)
    {
        if (rProp.Name != "slidesorter")
            continue;
        uno::Any aFromSlideSorter = xSourcePropertySet->getPropertyValue("slidesorter");
        bool bFromSlideSorter(false);
        aFromSlideSorter >>= bFromSlideSorter;
        return bFromSlideSorter;
    }

    return false;
}

bool SdXMLFilter::Export()
{
    rtl::Reference<SvXMLEmbeddedObjectHelper> xObjectHelper;
    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper;
    bool                    bDocRet = false;

    if( !mxModel.is() )
    {
        SAL_WARN( "sd.filter","Got NO Model in XMLExport");
        return false;
    }

    bool bLocked = mxModel->hasControllersLocked();

    try
    {
        mxModel->lockControllers();

        uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY );

        if( !xServiceInfo.is() || !xServiceInfo->supportsService( u"com.sun.star.drawing.GenericDrawingDocument"_ustr ) )
        {
            SAL_WARN( "sd.filter", "Model is no DrawingDocument in XMLExport" );
            return false;
        }

        const uno::Reference<uno::XComponentContext>& xContext( ::comphelper::getProcessComponentContext() );

        uno::Reference< xml::sax::XWriter > xWriter = xml::sax::Writer::create( xContext );

        /** property map for export info set */
        static PropertyMapEntry const aExportInfoMap[] =
        {
            { u"ProgressRange"_ustr,    0, cppu::UnoType<sal_Int32>::get(),   css::beans::PropertyAttribute::MAYBEVOID, 0},
            { u"ProgressMax"_ustr,      0, cppu::UnoType<sal_Int32>::get(),   css::beans::PropertyAttribute::MAYBEVOID, 0},
            { u"ProgressCurrent"_ustr,  0, cppu::UnoType<sal_Int32>::get(),   css::beans::PropertyAttribute::MAYBEVOID, 0},
            { u"UsePrettyPrinting"_ustr,0, cppu::UnoType<bool>::get(),        css::beans::PropertyAttribute::MAYBEVOID, 0},
            { u"PageLayoutNames"_ustr,  0, cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::MAYBEVOID, 0},
            { u"BaseURI"_ustr,          0, cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamRelPath"_ustr,    0, cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StreamName"_ustr,       0, cppu::UnoType<OUString>::get(),    css::beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StyleNames"_ustr,       0, cppu::UnoType<Sequence<OUString>>::get(),  css::beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"StyleFamilies"_ustr,    0, cppu::UnoType<Sequence<sal_Int32>>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
            { u"TargetStorage"_ustr,    0, cppu::UnoType<embed::XStorage>::get(),     css::beans::PropertyAttribute::MAYBEVOID, 0 },
        };

        uno::Reference< beans::XPropertySet > xInfoSet( GenericPropertySet_CreateInstance( new PropertySetInfo( aExportInfoMap ) ) );

        bool bUsePrettyPrinting = officecfg::Office::Common::Save::Document::PrettyPrinting::get();
        xInfoSet->setPropertyValue( u"UsePrettyPrinting"_ustr, Any( bUsePrettyPrinting ) );

        const uno::Reference < embed::XStorage > xStorage = mrMedium.GetOutputStorage();

        // Set base URI
        OUString sPropName( u"BaseURI"_ustr );
        xInfoSet->setPropertyValue( sPropName, Any( mrMedium.GetBaseURL( true ) ) );

        xInfoSet->setPropertyValue( u"TargetStorage"_ustr, Any( xStorage ) );

        if( SfxObjectCreateMode::EMBEDDED == mrDocShell.GetCreateMode() )
        {
            OUString aName;
            const SfxStringItem* pDocHierarchItem =
                mrMedium.GetItemSet().GetItem(SID_DOC_HIERARCHICALNAME);
            if ( pDocHierarchItem )
                aName = pDocHierarchItem->GetValue();

            if( !aName.isEmpty() )
            {
                sPropName = "StreamRelPath";
                xInfoSet->setPropertyValue( sPropName, Any( aName ) );
            }
        }

        // initialize descriptor
        uno::Sequence< beans::PropertyValue > aDescriptor( 1 );
        beans::PropertyValue* pProps = aDescriptor.getArray();

        pProps[0].Name = "FileName";
        pProps[0].Value <<= mrMedium.GetName();

        {
            uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
            uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;

            // create helper for graphic and ole export if we have a storage
            if( xStorage.is() )
            {
                xObjectHelper = SvXMLEmbeddedObjectHelper::Create( xStorage, *mrDocShell.GetDoc()->GetPersist(), SvXMLEmbeddedObjectHelperMode::Write );
                xObjectResolver = xObjectHelper.get();

                xGraphicHelper = SvXMLGraphicHelper::Create( xStorage, SvXMLGraphicHelperMode::Write );
                xGraphicStorageHandler = xGraphicHelper.get();
            }

            CreateStatusIndicator();
            if(mxStatusIndicator.is())
            {
                sal_Int32 nProgressRange(1000000);
                OUString aMsg(SdResId(STR_SAVE_DOC));
                mxStatusIndicator->start(aMsg, nProgressRange);

                // set ProgressRange
                uno::Any aProgRange;
                aProgRange <<= nProgressRange;
                xInfoSet->setPropertyValue( u"ProgressRange"_ustr , aProgRange);

                // set ProgressCurrent
                uno::Any aProgCurrent;
                aProgCurrent <<= sal_Int32(0);
                xInfoSet->setPropertyValue( u"ProgressCurrent"_ustr , aProgCurrent);
            }

            XML_SERVICES const * pServiceNames = getServices( false, IsDraw(), mnStoreVer );

            XML_SERVICEMAP aServices[5]; sal_uInt16 i = 0;
            aServices[i  ].mpService = pServiceNames->mpStyles;
            aServices[i++].mpStream  = "styles.xml";

            aServices[i  ].mpService = pServiceNames->mpContent;
            aServices[i++].mpStream  = "content.xml";

            aServices[i  ].mpService = pServiceNames->mpSettings;
            aServices[i++].mpStream  = "settings.xml";

            bool bExportMeta = mrDocShell.GetCreateMode() != SfxObjectCreateMode::EMBEDDED;
            if (!bExportMeta)
            {
                // Export meta information anyway when this is a copy from the
                // slide sorter so we can distinguish that at paste time
                bExportMeta = IsSlideSorterPaste(mrDocShell);
            }
            if (bExportMeta)
            {
                aServices[i  ].mpService = pServiceNames->mpMeta;
                aServices[i++].mpStream  = "meta.xml";
            };

            aServices[i].mpService = nullptr;
            aServices[i].mpStream  = nullptr;

            XML_SERVICEMAP* pServices = aServices;
            std::vector<OUString> aEmbeddedFontNames;

            // doc export
            do
            {
                SAL_INFO( "sd.filter", "exporting substream " << pServices->mpStream );

                uno::Reference<io::XOutputStream> xDocOut;
                if( xStorage.is() )
                {
                    const OUString sDocName( OUString::createFromAscii( pServices->mpStream ) );
                    uno::Reference<io::XStream> xStream =
                            xStorage->openStreamElement( sDocName,
                            embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );

                    DBG_ASSERT(xStream.is(), "Can't create output stream in package!");
                    if( !xStream.is() )
                        return false;

                    xDocOut = xStream->getOutputStream();
                    Reference <beans::XPropertySet > xProps( xStream, uno::UNO_QUERY );
                    if( !xDocOut.is() || !xProps.is() )
                        return false;

                    xProps->setPropertyValue( u"MediaType"_ustr, Any(u"text/xml"_ustr));

                    // encrypt all streams
                    xProps->setPropertyValue( u"UseCommonStoragePasswordEncryption"_ustr,
                                              uno::Any( true ) );

                    xInfoSet->setPropertyValue( u"StreamName"_ustr, Any( sDocName ) );
                }

                xWriter->setOutputStream( xDocOut );

                uno::Sequence< uno::Any > aArgs( 2 + ( mxStatusIndicator.is() ? 1 : 0 ) + ( xGraphicStorageHandler.is() ? 1 : 0 ) + ( xObjectResolver.is() ? 1 : 0 ) );
                uno::Any* pArgs = aArgs.getArray();
                *pArgs++ <<= xInfoSet;
                if (xGraphicStorageHandler.is())
                    *pArgs++ <<= xGraphicStorageHandler;
                if (xObjectResolver.is())
                    *pArgs++ <<= xObjectResolver;
                if (mxStatusIndicator.is())
                    *pArgs++ <<= mxStatusIndicator;

                *pArgs   <<= xWriter;

                uno::Reference< document::XFilter > xFilter( xContext->getServiceManager()->createInstanceWithArgumentsAndContext( OUString::createFromAscii( pServices->mpService ), aArgs, xContext ), uno::UNO_QUERY );
                if( xFilter.is() )
                {
                    uno::Reference< document::XExporter > xExporter( xFilter, uno::UNO_QUERY );
                    if( xExporter.is() )
                    {
                        xExporter->setSourceDocument( mxModel );

                        auto pFilter = dynamic_cast<SvXMLExport*>(xFilter.get());
                        if (pFilter)
                        {
                            pFilter->setEmbeddedFontNames(aEmbeddedFontNames);
                        }
                        // outputstream will be closed by SAX parser
                        bDocRet = xFilter->filter( aDescriptor );
                        if (pFilter)
                        {
                            aEmbeddedFontNames = pFilter->getEmbeddedFontNames();
                        }
                    }
                }

                pServices++;
            }
            while( bDocRet && pServices->mpService );

            if(mxStatusIndicator.is())
                mxStatusIndicator->end();
        }
    }
    catch (const uno::Exception &)
    {
        TOOLS_WARN_EXCEPTION( "sd.filter", "uno Exception caught while exporting");
        bDocRet = false;
    }
    if ( !bLocked )
        mxModel->unlockControllers();

    if( xGraphicHelper )
        xGraphicHelper->dispose();
    xGraphicHelper.clear();

    if( xObjectHelper )
        xObjectHelper->dispose();
    xObjectHelper.clear();

    return bDocRet;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportFODP(SvStream &rStream)
{
    SdDLL::Init();

    sd::DrawDocShellRef xDocSh(new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Impress));
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new ::utl::OSeekableInputStreamWrapper(rStream));
    uno::Reference<uno::XInterface> xInterface(xMultiServiceFactory->createInstance(u"com.sun.star.comp.Writer.XmlFilterAdaptor"_ustr), uno::UNO_SET_THROW);

    css::uno::Sequence<OUString> aUserData
    {
        u"com.sun.star.comp.filter.OdfFlatXml"_ustr,
        u""_ustr,
        u"com.sun.star.comp.Impress.XMLOasisImporter"_ustr,
        u"com.sun.star.comp.Impress.XMLOasisExporter"_ustr,
        u""_ustr,
        u""_ustr,
        u"true"_ustr
    };
    uno::Sequence<beans::PropertyValue> aAdaptorArgs(comphelper::InitPropertySequence(
    {
        { "UserData", uno::Any(aUserData) },
    }));
    css::uno::Sequence<uno::Any> aOuterArgs{ uno::Any(aAdaptorArgs) };

    uno::Reference<lang::XInitialization> xInit(xInterface, uno::UNO_QUERY_THROW);
    xInit->initialize(aOuterArgs);

    uno::Reference<document::XImporter> xImporter(xInterface, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "URL", uno::Any(u"private:stream"_ustr) },
    }));
    xImporter->setTargetDocument(xModel);

    uno::Reference<document::XFilter> xFilter(xInterface, uno::UNO_QUERY_THROW);
    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = xFilter->filter(aArgs);
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportPPTX(SvStream &rStream)
{
    SdDLL::Init();

    sd::DrawDocShellRef xDocSh(new sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Impress));
    xDocSh->DoInitNew();
    uno::Reference<frame::XModel> xModel(xDocSh->GetModel());

    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(comphelper::getProcessServiceFactory());
    uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));

    uno::Reference<document::XFilter> xFilter(xMultiServiceFactory->createInstance(u"com.sun.star.comp.oox.ppt.PowerPointImport"_ustr), uno::UNO_QUERY_THROW);

    uno::Reference<document::XImporter> xImporter(xFilter, uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> aArgs(comphelper::InitPropertySequence(
    {
        { "InputStream", uno::Any(xStream) },
        { "InputMode", uno::Any(true) },
    }));
    xImporter->setTargetDocument(xModel);

    //SetLoading hack because the document properties will be re-initted
    //by the xml filter and during the init, while it's considered uninitialized,
    //setting a property will inform the document it's modified, which attempts
    //to update the properties, which throws cause the properties are uninitialized
    xDocSh->SetLoading(SfxLoadedFlags::NONE);
    bool ret = false;
    try
    {
        ret = xFilter->filter(aArgs);
    }
    catch (...)
    {
    }
    xDocSh->SetLoading(SfxLoadedFlags::ALL);

    xDocSh->DoClose();

    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
