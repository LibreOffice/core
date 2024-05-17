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
#include <utility>
#include <vcl/errinf.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
#include <osl/diagnose.h>
#include <o3tl/unreachable.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/xmlgrhlp.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/frame.hxx>
#include <svl/itemset.hxx>
#include <svl/stritem.hxx>
#include <sfx2/sfxsids.hrc>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/SAXParseException.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/genericpropertyset.hxx>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/rdf/XDocumentMetadataAccess.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include <sfx2/DocumentMetadataAccess.hxx>
#include <comphelper/documentconstants.hxx>
#include <xmloff/shapeexport.hxx>
#include <svx/xmleohlp.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <document.hxx>
#include <xmlwrap.hxx>
#include "xmlimprt.hxx"
#include "xmlexprt.hxx"
#include <globstr.hrc>
#include <scresid.hxx>
#include <scerrors.hxx>
#include "XMLExportSharedData.hxx"
#include <docuno.hxx>
#include <drwlayer.hxx>
#include <sheetdata.hxx>
#include "XMLCodeNameProvider.hxx"
#include <docsh.hxx>
#include <unonames.hxx>

using namespace com::sun::star;
using namespace css::uno;

ScXMLImportWrapper::ScXMLImportWrapper( ScDocShell& rDocSh, SfxMedium* pM, uno::Reference < embed::XStorage > xStor ) :
    mrDocShell(rDocSh),
    rDoc(rDocSh.GetDocument()),
    pMedium(pM),
    xStorage(std::move(xStor))
{
    OSL_ENSURE( pMedium || xStorage.is(), "ScXMLImportWrapper: Medium or Storage must be set" );
}

uno::Reference <task::XStatusIndicator> ScXMLImportWrapper::GetStatusIndicator() const
{
    uno::Reference<task::XStatusIndicator> xStatusIndicator;
    if (pMedium)
    {
        const SfxUnoAnyItem* pItem = pMedium->GetItemSet().GetItem(SID_PROGRESS_STATUSBAR_CONTROL);
        if (pItem)
            xStatusIndicator.set(pItem->GetValue(), uno::UNO_QUERY);
    }
    return xStatusIndicator;
}

ErrCodeMsg ScXMLImportWrapper::ImportFromComponent(const uno::Reference<uno::XComponentContext>& xContext,
    const uno::Reference<frame::XModel>& xModel,
    xml::sax::InputSource& aParserInput,
    const OUString& sComponentName, const OUString& sDocName,
    const uno::Sequence<uno::Any>& aArgs,
    bool bMustBeSuccessful)
{
    uno::Reference < io::XStream > xDocStream;
    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetStorage();

    bool bEncrypted = false;
    OUString sStream(sDocName);
    if( xStorage.is() )
    {
        try
        {
            if ( xStorage->hasByName(sDocName) && xStorage->isStreamElement( sDocName) )
                xDocStream = xStorage->openStreamElement( sDocName, embed::ElementModes::READ );
            else
                return ERRCODE_NONE;

            aParserInput.aInputStream = xDocStream->getInputStream();
            uno::Reference < beans::XPropertySet > xSet( xDocStream, uno::UNO_QUERY );

            uno::Any aAny = xSet->getPropertyValue(u"Encrypted"_ustr);
            aAny >>= bEncrypted;
        }
        catch( const packages::WrongPasswordException& )
        {
            return ERRCODE_SFX_WRONGPASSWORD;
        }
        catch( const packages::zip::ZipIOException& )
        {
            return ERRCODE_IO_BROKENPACKAGE;
        }
        catch( const uno::Exception& )
        {
            return SCERR_IMPORT_UNKNOWN;
        }
    }
    else
        return SCERR_IMPORT_UNKNOWN;

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( aArgs.hasElements() )
        aArgs.getConstArray()[0] >>= xInfoSet;
    OSL_ENSURE( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        xInfoSet->setPropertyValue( u"StreamName"_ustr, uno::Any( sStream ) );
    }

    ErrCodeMsg nReturn = ERRCODE_NONE;
    rDoc.SetRangeOverflowType(ERRCODE_NONE);   // is modified by the importer if limits are exceeded

    uno::Reference<XInterface> xImportInterface =
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            sComponentName, aArgs, xContext );
    SAL_WARN_IF( !xImportInterface, "sc", "can't get Calc importer " << sComponentName );
    uno::Reference<document::XImporter> xImporter( xImportInterface, uno::UNO_QUERY );
    if (xImporter.is())
        xImporter->setTargetDocument( xModel );

    ScXMLImport* pImporterImpl = dynamic_cast<ScXMLImport*>(xImporter.get());
    if (pImporterImpl)
        pImporterImpl->SetPostProcessData(&maPostProcessData);

    // connect parser and filter
    try
    {
        // xImportInterface is either ScXMLImport or an XMLTransformer subclass.
        // ScXMLImport implements XFastParser, but XMLTransformer only implements XExtendedDocumentHandler

        uno::Reference< xml::sax::XFastParser > xFastParser(xImportInterface, uno::UNO_QUERY);
        if (xFastParser)
            xFastParser->parseStream( aParserInput );
        else
        {
            uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(xContext);
            uno::Reference<css::xml::sax::XDocumentHandler> xDocumentHandler(xImportInterface, uno::UNO_QUERY);
            xParser->setDocumentHandler( xDocumentHandler );
            xParser->parseStream( aParserInput );
        }
    }
    catch( const xml::sax::SAXParseException& r )
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
                aSaxEx = aTmp;
            else
                bTryChild = false;
        }

        packages::zip::ZipIOException aBrokenPackage;
        if ( aSaxEx.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;
        else if( bEncrypted )
            nReturn = ERRCODE_SFX_WRONGPASSWORD;
        else
        {
            SAL_WARN("sc.filter", "SAX parse exception caught while importing: " << exceptionToString(ex));

            OUString sErr = OUString::number( r.LineNumber ) +
                          "," +
                          OUString::number( r.ColumnNumber );

            if( !sDocName.isEmpty() )
            {
                nReturn = ErrCodeMsg(
                                (bMustBeSuccessful ? SCERR_IMPORT_FILE_ROWCOL
                                                        : SCWARN_IMPORT_FILE_ROWCOL),
                                sDocName, sErr,
                                DialogMask::ButtonsOk | DialogMask::MessageError );
            }
            else
            {
                OSL_ENSURE( bMustBeSuccessful, "Warnings are not supported" );
                nReturn = ErrCodeMsg( SCERR_IMPORT_FORMAT_ROWCOL, sErr,
                                 DialogMask::ButtonsOk | DialogMask::MessageError );
            }
        }
    }
    catch( const xml::sax::SAXException& r )
    {
        css::uno::Any ex( cppu::getCaughtException() );
        packages::zip::ZipIOException aBrokenPackage;
        if ( r.WrappedException >>= aBrokenPackage )
            return ERRCODE_IO_BROKENPACKAGE;
        else if( bEncrypted )
            nReturn = ERRCODE_SFX_WRONGPASSWORD;
        else
        {
            SAL_WARN("sc.filter", "SAX exception caught while importing: " << exceptionToString(ex));

            nReturn = SCERR_IMPORT_FORMAT;
        }
    }
    catch( const packages::zip::ZipIOException& )
    {
        TOOLS_WARN_EXCEPTION("sc.filter", "Zip exception caught while importing");

        nReturn = ERRCODE_IO_BROKENPACKAGE;
    }
    catch( const io::IOException& )
    {
        TOOLS_WARN_EXCEPTION("sc.filter", "IO exception caught while importing");

        nReturn = SCERR_IMPORT_OPEN;
    }
    catch( const uno::Exception& )
    {
        TOOLS_WARN_EXCEPTION("sc.filter", "uno exception caught while importing");

        nReturn = SCERR_IMPORT_UNKNOWN;
    }

    // #i31130# Can't use getImplementation here to get the ScXMLImport from xDocHandler,
    // because when OOo 1.x files are loaded, xDocHandler is the OOo2OasisTransformer.
    // So the overflow warning ErrorCode is now stored in the document.
    // Export works differently, there getImplementation still works.

    if (rDoc.HasRangeOverflow() && !nReturn)
        nReturn = rDoc.GetRangeOverflowType();

    // success!
    return nReturn;
}

bool ScXMLImportWrapper::Import( ImportFlags nMode, ErrCodeMsg& rError )
{
    uno::Reference<uno::XComponentContext> xContext = comphelper::getProcessComponentContext();

    xml::sax::InputSource aParserInput;
    if (pMedium)
        aParserInput.sSystemId = pMedium->GetName();

    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetStorage();

    // get filter
    uno::Reference<frame::XModel> xModel = mrDocShell.GetModel();

    /** property map for export info set */
    static comphelper::PropertyMapEntry const aImportInfoMap[] =
    {
        { u"ProgressRange"_ustr, 0, ::cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressMax"_ustr, 0, ::cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressCurrent"_ustr, 0, ::cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"NumberStyles"_ustr, 0, cppu::UnoType<container::XNameAccess>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"PrivateData"_ustr, 0, cppu::UnoType<uno::XInterface>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamRelPath"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamName"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"BuildId"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"VBACompatibilityMode"_ustr, 0, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"ScriptConfiguration"_ustr, 0, cppu::UnoType<container::XNameAccess>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"OrganizerMode"_ustr, 0, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"SourceStorage"_ustr, 0, cppu::UnoType<embed::XStorage>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { SC_UNO_ODS_LOCK_SOLAR_MUTEX, 0, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { SC_UNO_ODS_IMPORT_STYLES, 0, cppu::UnoType<bool>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
    };
    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aImportInfoMap ) ) );

    // No need to lock solar mutex when calling from the wrapper.
    xInfoSet->setPropertyValue(SC_UNO_ODS_LOCK_SOLAR_MUTEX, uno::Any(false));

    // ---- get BuildId from parent container if available

    uno::Reference< container::XChild > xChild( xModel, uno::UNO_QUERY );
    if( xChild.is() )
    {
        uno::Reference< beans::XPropertySet > xParentSet( xChild->getParent(), uno::UNO_QUERY );
        if( xParentSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xParentSet->getPropertySetInfo() );
            OUString sPropName(u"BuildId"_ustr );
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(sPropName) )
            {
                xInfoSet->setPropertyValue( sPropName, xParentSet->getPropertyValue(sPropName) );
            }
        }
    }

    uno::Reference<task::XStatusIndicator> xStatusIndicator = GetStatusIndicator();
    if (xStatusIndicator.is())
    {
        sal_Int32 nProgressRange(1000000);
        xStatusIndicator->start(SvxResId(RID_SVXSTR_DOC_LOAD), nProgressRange);
        xInfoSet->setPropertyValue(u"ProgressRange"_ustr, uno::Any(nProgressRange));
    }

    // Set base URI
    OSL_ENSURE( pMedium, "There is no medium to get MediaDescriptor from!" );
    OUString aBaseURL = pMedium ? pMedium->GetBaseURL() : OUString();
    // needed for relative URLs, but in clipboard copy/paste there may be none
    SAL_INFO_IF(aBaseURL.isEmpty(), "sc.filter", "ScXMLImportWrapper: no base URL");
    OUString sPropName(u"BaseURI"_ustr);
    xInfoSet->setPropertyValue( sPropName, uno::Any( aBaseURL ) );

    // TODO/LATER: do not do it for embedded links
    OUString aName;
    if (SfxObjectCreateMode::EMBEDDED == mrDocShell.GetCreateMode())
    {
        if ( pMedium )
        {
            const SfxStringItem* pDocHierarchItem =
                pMedium->GetItemSet().GetItem(SID_DOC_HIERARCHICALNAME);
            if ( pDocHierarchItem )
                aName = pDocHierarchItem->GetValue();
        }
        else
            aName = "dummyObjectName";

        if( !aName.isEmpty() )
        {
            sPropName = "StreamRelPath";
            xInfoSet->setPropertyValue( sPropName, uno::Any( aName ) );
        }
    }

    if (mrDocShell.GetCreateMode() == SfxObjectCreateMode::ORGANIZER)
        xInfoSet->setPropertyValue(u"OrganizerMode"_ustr, uno::Any(true));

    xInfoSet->setPropertyValue( u"SourceStorage"_ustr, uno::Any( xStorage ) );

    bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );

    if ((nMode & ImportFlags::Metadata) && bOasis)
    {
        // RDF metadata: ODF >= 1.2
        try
        {
            const uno::Reference< rdf::XDocumentMetadataAccess > xDMA(
                xModel, uno::UNO_QUERY_THROW );
            const uno::Reference< rdf::XURI > xBaseURI(
                ::sfx2::createBaseURI( xContext, xModel, aBaseURL, aName ) );
            uno::Reference<task::XInteractionHandler> xHandler =
                mrDocShell.GetMedium()->GetInteractionHandler();
            xDMA->loadMetadataFromStorage( xStorage, xBaseURI, xHandler );
        }
        catch ( const lang::WrappedTargetException & e)
        {
            ucb::InteractiveAugmentedIOException iaioe;
            if ( e.TargetException >>= iaioe )
            {
                rError = SCERR_IMPORT_UNKNOWN;
            }
            else
            {
                rError = SCWARN_IMPORT_FEATURES_LOST;
            }
        }
        catch ( const uno::Exception &)
        {
            rError = SCWARN_IMPORT_FEATURES_LOST;
        }
    }

    // #i103539#: always read meta.xml for generator
    ErrCodeMsg nMetaRetval(ERRCODE_NONE);
    if (nMode & ImportFlags::Metadata)
    {
        uno::Sequence<uno::Any> aMetaArgs { Any(xInfoSet) };

        SAL_INFO( "sc.filter", "meta import start" );

        nMetaRetval = ImportFromComponent(
                                xContext, xModel, aParserInput,
                                bOasis ? u"com.sun.star.comp.Calc.XMLOasisMetaImporter"_ustr
                                : u"com.sun.star.comp.Calc.XMLMetaImporter"_ustr,
                                u"meta.xml"_ustr, aMetaArgs, false);

        SAL_INFO( "sc.filter", "meta import end" );
    }

    rtl::Reference<SvXMLGraphicHelper> xGraphicHelper;
    uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;

    uno::Reference< document::XEmbeddedObjectResolver > xObjectResolver;
    rtl::Reference<SvXMLEmbeddedObjectHelper> xObjectHelper;

    if( xStorage.is() )
    {
        xGraphicHelper = SvXMLGraphicHelper::Create( xStorage, SvXMLGraphicHelperMode::Read );
        xGraphicStorageHandler = xGraphicHelper.get();

        xObjectHelper = SvXMLEmbeddedObjectHelper::Create(xStorage, mrDocShell, SvXMLEmbeddedObjectHelperMode::Read);
        xObjectResolver = xObjectHelper.get();
    }
    uno::Sequence<uno::Any> aStylesArgs
    {
        Any(xInfoSet),
        Any(xGraphicStorageHandler),
        Any(xStatusIndicator),
        Any(xObjectResolver)
    };

    ErrCodeMsg nSettingsRetval(ERRCODE_NONE);
    if (nMode & ImportFlags::Settings)
    {
        //  Settings must be loaded first because of the printer setting,
        //  which is needed in the page styles (paper tray).

        uno::Sequence<uno::Any> aSettingsArgs { Any(xInfoSet) };

        SAL_INFO( "sc.filter", "settings import start" );

        nSettingsRetval = ImportFromComponent(
                            xContext, xModel, aParserInput,
                            bOasis ? u"com.sun.star.comp.Calc.XMLOasisSettingsImporter"_ustr
                                   : u"com.sun.star.comp.Calc.XMLSettingsImporter"_ustr,
                            u"settings.xml"_ustr, aSettingsArgs, false);

        SAL_INFO( "sc.filter", "settings import end" );
    }

    ErrCodeMsg nStylesRetval(ERRCODE_NONE);
    if (nMode & ImportFlags::Styles)
    {
        SAL_INFO( "sc.filter", "styles import start" );

        nStylesRetval = ImportFromComponent(xContext, xModel, aParserInput,
            bOasis ? u"com.sun.star.comp.Calc.XMLOasisStylesImporter"_ustr
                   : u"com.sun.star.comp.Calc.XMLStylesImporter"_ustr,
            u"styles.xml"_ustr,
            aStylesArgs, true);

        SAL_INFO( "sc.filter", "styles import end" );
    }

    ErrCodeMsg nDocRetval(ERRCODE_NONE);
    if (nMode & ImportFlags::Content)
    {
        if (mrDocShell.GetCreateMode() == SfxObjectCreateMode::INTERNAL)
            // We only need to import content for external link cache document.
            xInfoSet->setPropertyValue(SC_UNO_ODS_IMPORT_STYLES, uno::Any(false));

        uno::Sequence<uno::Any> aDocArgs
        {
            Any(xInfoSet),
            Any(xGraphicStorageHandler),
            Any(xStatusIndicator),
            Any(xObjectResolver)
        };

        SAL_INFO( "sc.filter", "content import start" );

        nDocRetval = ImportFromComponent(xContext, xModel, aParserInput,
            bOasis ? u"com.sun.star.comp.Calc.XMLOasisContentImporter"_ustr
                   : u"com.sun.star.comp.Calc.XMLContentImporter"_ustr,
            u"content.xml"_ustr,
            aDocArgs,
            true);

        SAL_INFO( "sc.filter", "content import end" );
    }
    if( xGraphicHelper.is() )
        xGraphicHelper->dispose();
    xGraphicHelper.clear();

    if( xObjectHelper.is() )
        xObjectHelper->dispose();
    xObjectHelper.clear();

    if (xStatusIndicator.is())
        xStatusIndicator->end();

    if (nDocRetval)
        rError = nDocRetval;
    else if (nStylesRetval)
        rError = nStylesRetval;
    else if (nMetaRetval)
        rError = nMetaRetval;
    else if (nSettingsRetval)
        rError = nSettingsRetval;

    bool bRet = !rError.IsError();

    ::svx::DropUnusedNamedItems(xModel);

    // set BuildId on XModel for later OLE object loading
    if( xInfoSet.is() )
    {
        uno::Reference< beans::XPropertySet > xModelSet( xModel, uno::UNO_QUERY );
        if( xModelSet.is() )
        {
            uno::Reference< beans::XPropertySetInfo > xModelSetInfo( xModelSet->getPropertySetInfo() );
            OUString sBuildPropName(u"BuildId"_ustr );
            if( xModelSetInfo.is() && xModelSetInfo->hasPropertyByName(sBuildPropName) )
            {
                xModelSet->setPropertyValue( sBuildPropName, xInfoSet->getPropertyValue(sBuildPropName) );
            }
        }

        // Set Code Names
        uno::Any aAny = xInfoSet->getPropertyValue(u"ScriptConfiguration"_ustr);
        uno::Reference <container::XNameAccess> xCodeNameAccess;
        if( aAny >>= xCodeNameAccess )
            XMLCodeNameProvider::set( xCodeNameAccess, &rDoc );

        // VBA compatibility
        bool bVBACompat = false;
        if ( (xInfoSet->getPropertyValue(u"VBACompatibilityMode"_ustr) >>= bVBACompat) && bVBACompat )
        {
            /*  Set library container to VBA compatibility mode, this
                forces loading the Basic project, which in turn creates the
                VBA Globals object and does all related initialization. */
            if ( xModelSet.is() ) try
            {
                uno::Reference< script::vba::XVBACompatibility > xVBACompat( xModelSet->getPropertyValue(
                    u"BasicLibraries"_ustr ), uno::UNO_QUERY_THROW );
                xVBACompat->setVBACompatibilityMode( true );
            }
            catch( const uno::Exception& )
            {
            }
        }
    }

    // Don't test bStylesRetval and bMetaRetval, because it could be an older file which not contain such streams
    return bRet;//!bStylesOnly ? bDocRetval : bStylesRetval;
}

static bool lcl_HasValidStream(const ScDocument& rDoc)
{
    ScDocShell* pObjSh = rDoc.GetDocumentShell();
    if ( pObjSh->IsDocShared() )
        return false;                       // never copy stream from shared file

    // don't read remote file again
    // (could instead re-use medium directly in that case)
    SfxMedium* pSrcMed = rDoc.GetDocumentShell()->GetMedium();
    if ( !pSrcMed || pSrcMed->IsRemote() )
        return false;

    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; ++nTab)
        if (rDoc.IsStreamValid(nTab))
            return true;
    return false;
}

bool ScXMLImportWrapper::ExportToComponent(const uno::Reference<uno::XComponentContext>& xContext,
    const uno::Reference<frame::XModel>& xModel, const uno::Reference<xml::sax::XWriter>& xWriter,
    const uno::Sequence<beans::PropertyValue>& aDescriptor, const OUString& sName,
    const OUString& sMediaType, const OUString& sComponentName,
    const uno::Sequence<uno::Any>& aArgs, std::unique_ptr<ScMySharedData>& pSharedData)
{
    bool bRet(false);
    uno::Reference<io::XOutputStream> xOut;
    uno::Reference<io::XStream> xStream;

    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetOutputStorage();

    if( xStorage.is() )
    {
        // #96807#; trunc stream before use, because it could be an existing stream
        // and the new content could be shorter than the old content. In this case
        // would not all be over written by the new content and the xml file
        // would not be valid.
        xStream = xStorage->openStreamElement( sName, embed::ElementModes::READWRITE | embed::ElementModes::TRUNCATE );
        uno::Reference < beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
        if (xSet.is())
        {
            xSet->setPropertyValue(u"MediaType"_ustr, uno::Any(sMediaType));

            // advise storage impl to use common encryption
            xSet->setPropertyValue( u"UseCommonStoragePasswordEncryption"_ustr, uno::Any(true) );
        }

        xOut = xStream->getOutputStream();
    }

    // set Base URL
    uno::Reference< beans::XPropertySet > xInfoSet;
    if( aArgs.hasElements() )
        aArgs.getConstArray()[0] >>= xInfoSet;
    OSL_ENSURE( xInfoSet.is(), "missing property set" );
    if( xInfoSet.is() )
    {
        xInfoSet->setPropertyValue( u"StreamName"_ustr, uno::Any( sName ) );
    }

    xWriter->setOutputStream( xOut );

    uno::Reference<document::XFilter> xFilter(
        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            sComponentName , aArgs, xContext ),
        uno::UNO_QUERY );
    OSL_ENSURE( xFilter.is(), "can't get exporter" );
    uno::Reference<document::XExporter> xExporter( xFilter, uno::UNO_QUERY );
    if (xExporter.is())
        xExporter->setSourceDocument( xModel );

    if ( xFilter.is() )
    {
        ScXMLExport* pExport = static_cast<ScXMLExport*>(dynamic_cast<SvXMLExport*>(xFilter.get()));
        assert(pExport && "can only succeed");
        pExport->SetSharedData(std::move(pSharedData));

        // if there are sheets to copy, get the source stream
        if ( sName == "content.xml" && lcl_HasValidStream(rDoc) && ( pExport->getExportFlags() & SvXMLExportFlags::OASIS ) )
        {
            // old stream is still in this file's storage - open read-only

            // #i106854# use the document's storage directly, without a temporary SfxMedium
            uno::Reference<embed::XStorage> xTmpStorage = rDoc.GetDocumentShell()->GetStorage();
            uno::Reference<io::XStream> xSrcStream;
            uno::Reference<io::XInputStream> xSrcInput;

            // #i108978# If an embedded object is saved and no events are notified, don't use the stream
            // because without the ...DONE events, stream positions aren't updated.
            ScSheetSaveData* pSheetData = comphelper::getFromUnoTunnel<ScModelObj>(xModel)->GetSheetSaveData();
            if (pSheetData && pSheetData->IsInSupportedSave())
            {
                try
                {
                    if (xTmpStorage.is())
                        xSrcStream = xTmpStorage->openStreamElement( sName, embed::ElementModes::READ );
                    if (xSrcStream.is())
                        xSrcInput = xSrcStream->getInputStream();
                }
                catch ( const uno::Exception&)
                {
                    // stream not available (for example, password protected) - save normally (xSrcInput is null)
                }
            }

            pExport->SetSourceStream( xSrcInput );
            bRet = xFilter->filter( aDescriptor );
            pExport->SetSourceStream( uno::Reference<io::XInputStream>() );

            // If there was an error, reset all stream flags, so the next save attempt will use normal saving.
            // #i110692# For embedded objects, the stream may be unavailable for one save operation (m_pAntiImpl)
            // and become available again later. But after saving normally once, the stream positions aren't
            // valid anymore, so the flags also have to be reset if the stream wasn't available.
            if ( !bRet || !xSrcInput.is() )
            {
                SCTAB nTabCount = rDoc.GetTableCount();
                for (SCTAB nTab=0; nTab<nTabCount; nTab++)
                    rDoc.SetStreamValid(nTab, false);
            }
        }
        else
            bRet = xFilter->filter( aDescriptor );

        pSharedData = pExport->ReleaseSharedData();
    }

    return bRet;
}

bool ScXMLImportWrapper::Export(bool bStylesOnly)
{
    // Prevent all broadcasting and repaints and notification of accessibility
    // during mass creation of captions, which is a major bottleneck and not
    // needed during Save.
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    bool bOldLock = bool();
    if (pDrawLayer)
    {
        bOldLock = pDrawLayer->isLocked();
        pDrawLayer->setLock(true);
    }

    rDoc.CreateAllNoteCaptions();

    if (pDrawLayer)
        pDrawLayer->setLock(bOldLock);

    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());

    uno::Reference<xml::sax::XWriter> xWriter = xml::sax::Writer::create(xContext);

    if ( !xStorage.is() && pMedium )
        xStorage = pMedium->GetOutputStorage();

    OUString sFileName;
    if (pMedium)
        sFileName = pMedium->GetName();
    ScDocShell* pObjSh = rDoc.GetDocumentShell();
    uno::Sequence<beans::PropertyValue> aDescriptor( comphelper::InitPropertySequence({
            { "FileName", uno::Any(sFileName) }
        }));

    /** property map for export info set */
    static comphelper::PropertyMapEntry const aExportInfoMap[] =
    {
        { u"ProgressRange"_ustr, 0, ::cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressMax"_ustr, 0, ::cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"ProgressCurrent"_ustr, 0, ::cppu::UnoType<sal_Int32>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"WrittenNumberStyles"_ustr, 0, cppu::UnoType<uno::Sequence<sal_Int32>>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"UsePrettyPrinting"_ustr, 0, ::cppu::UnoType<sal_Bool>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0},
        { u"BaseURI"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamRelPath"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StreamName"_ustr, 0, ::cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StyleNames"_ustr, 0, cppu::UnoType<uno::Sequence<OUString>>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"StyleFamilies"_ustr, 0, cppu::UnoType<uno::Sequence<sal_Int32>>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
        { u"TargetStorage"_ustr, 0, cppu::UnoType<embed::XStorage>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
    };
    uno::Reference< beans::XPropertySet > xInfoSet( comphelper::GenericPropertySet_CreateInstance( new comphelper::PropertySetInfo( aExportInfoMap ) ) );

    if ( pObjSh && xStorage.is() )
    {
        uno::Reference<frame::XModel> xModel(pObjSh->GetModel());
        // sorting wants to create undo actions
        assert(SfxObjectCreateMode::STANDARD != pObjSh->GetCreateMode() || rDoc.GetDrawLayer()->IsUndoEnabled());
        uno::Reference<drawing::XDrawPagesSupplier> const xDPS(xModel, uno::UNO_QUERY);
        uno::Reference<container::XIndexAccess> const xDPs(xDPS->getDrawPages());
        assert(xDPs.is());
        for (auto i = xDPs->getCount(); 0 < i; )
        {
            --i;
            uno::Reference<drawing::XShapes> const xDP(xDPs->getByIndex(i), uno::UNO_QUERY);
            assert(xDP.is());
            xmloff::FixZOrder(xDP,
                [](uno::Reference<beans::XPropertySet> const& xShape)
                {
                    sal_Int16 nLayerID(0);
                    xShape->getPropertyValue(u"LayerID"_ustr) >>= nLayerID;
                    switch (nLayerID)
                    {
                        case sal_Int16(SC_LAYER_FRONT):
                            return 1;
                        case sal_Int16(SC_LAYER_BACK):
                            return 0;
                        case sal_Int16(SC_LAYER_INTERN):
                            return 2;
                        case sal_Int16(SC_LAYER_CONTROLS):
                            return 3;
                        case sal_Int16(SC_LAYER_HIDDEN):
                            return 1; // treat as equivalent to front
                        default:
                            O3TL_UNREACHABLE;
                    }
                });
        }

        uno::Reference<task::XStatusIndicator> xStatusIndicator(GetStatusIndicator());
        sal_Int32 nProgressRange(1000000);
        if(xStatusIndicator.is())
            xStatusIndicator->start(ScResId(STR_SAVE_DOC), nProgressRange);
        xInfoSet->setPropertyValue(u"ProgressRange"_ustr, uno::Any(nProgressRange));

        bool bUsePrettyPrinting = officecfg::Office::Common::Save::Document::PrettyPrinting::get();
        xInfoSet->setPropertyValue(u"UsePrettyPrinting"_ustr, uno::Any(bUsePrettyPrinting));

        xInfoSet->setPropertyValue( u"TargetStorage"_ustr, uno::Any( xStorage ) );

        OSL_ENSURE( pMedium, "There is no medium to get MediaDescriptor from!" );
        OUString aBaseURL = pMedium ? pMedium->GetBaseURL( true ) : OUString();
        OUString sPropName(u"BaseURI"_ustr);
        xInfoSet->setPropertyValue( sPropName, uno::Any( aBaseURL ) );

        // TODO/LATER: do not do it for embedded links
        if( SfxObjectCreateMode::EMBEDDED == pObjSh->GetCreateMode() )
        {
            OUString aName(u"dummyObjectName"_ustr);
            if ( pMedium )
            {
                const SfxStringItem* pDocHierarchItem =
                    pMedium->GetItemSet().GetItem(SID_DOC_HIERARCHICALNAME);
                if ( pDocHierarchItem )
                    aName = pDocHierarchItem->GetValue();
            }

            if( !aName.isEmpty() )
            {
                sPropName = "StreamRelPath";
                xInfoSet->setPropertyValue( sPropName, uno::Any( aName ) );
            }
        }

        OUString sTextMediaType(u"text/xml"_ustr);
        bool bMetaRet(pObjSh->GetCreateMode() == SfxObjectCreateMode::EMBEDDED);
        bool bStylesRet (false);
        bool bDocRet(false);
        bool bSettingsRet(false);
        std::unique_ptr<ScMySharedData> pSharedData;

        bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );

        // RDF metadata: ODF >= 1.2
        if ( !bStylesOnly && bOasis )
        {
            const uno::Reference< beans::XPropertySet > xPropSet( xStorage, uno::UNO_QUERY_THROW );
            try
            {
                OUString aVersion;
                if (( xPropSet->getPropertyValue(u"Version"_ustr) >>= aVersion )
                    && aVersion != ODFVER_010_TEXT
                    && aVersion != ODFVER_011_TEXT )
                {
                    const uno::Reference< rdf::XDocumentMetadataAccess > xDMA(
                        xModel, uno::UNO_QUERY_THROW );
                    xDMA->storeMetadataToStorage( xStorage );
                }
            }
            catch ( const beans::UnknownPropertyException &)
            {
            }
            catch ( const uno::Exception &)
            {
            }
        }

        // meta export
        if (!bStylesOnly && !bMetaRet)
        {
            uno::Sequence<uno::Any> aMetaArgs
            {
                Any(xInfoSet),
                Any(xWriter),
                Any(xStatusIndicator)
            };

            SAL_INFO( "sc.filter", "meta export start" );

            bMetaRet = ExportToComponent(xContext, xModel, xWriter, aDescriptor,
                u"meta.xml"_ustr,
                sTextMediaType,
                bOasis ? u"com.sun.star.comp.Calc.XMLOasisMetaExporter"_ustr
                       : u"com.sun.star.comp.Calc.XMLMetaExporter"_ustr,
                aMetaArgs, pSharedData);

            SAL_INFO( "sc.filter", "meta export end" );
        }

        uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;
        rtl::Reference<SvXMLGraphicHelper> xGraphicHelper;

        if( xStorage.is() )
        {
            xGraphicHelper = SvXMLGraphicHelper::Create( xStorage, SvXMLGraphicHelperMode::Write );
            xGraphicStorageHandler = xGraphicHelper.get();
        }

        auto xObjectHelper = SvXMLEmbeddedObjectHelper::Create(
            xStorage, *pObjSh, SvXMLEmbeddedObjectHelperMode::Write);
        uno::Reference<document::XEmbeddedObjectResolver> xObjectResolver(xObjectHelper);

        // styles export

        {
            uno::Sequence<uno::Any> aStylesArgs
            {
                Any(xInfoSet),
                Any(xGraphicStorageHandler),
                Any(xStatusIndicator),
                Any(xWriter),
                Any(xObjectResolver)
            };

            SAL_INFO( "sc.filter", "styles export start" );

            bStylesRet = ExportToComponent(xContext, xModel, xWriter, aDescriptor,
                u"styles.xml"_ustr,
                sTextMediaType,
                bOasis ? u"com.sun.star.comp.Calc.XMLOasisStylesExporter"_ustr
                       : u"com.sun.star.comp.Calc.XMLStylesExporter"_ustr,
                aStylesArgs, pSharedData);

            SAL_INFO( "sc.filter", "styles export end" );
        }

        // content export

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aDocArgs
            {
                Any(xInfoSet),
                Any(xGraphicStorageHandler),
                Any(xStatusIndicator),
                Any(xWriter),
                Any(xObjectResolver)
            };

            SAL_INFO( "sc.filter", "content export start" );

            bDocRet = ExportToComponent(xContext, xModel, xWriter, aDescriptor,
                u"content.xml"_ustr,
                sTextMediaType,
                bOasis ? u"com.sun.star.comp.Calc.XMLOasisContentExporter"_ustr
                       : u"com.sun.star.comp.Calc.XMLContentExporter"_ustr,
                aDocArgs, pSharedData);

            SAL_INFO( "sc.filter", "content export end" );
        }

        if( xGraphicHelper )
            xGraphicHelper->dispose();
        xGraphicHelper.clear();

        if( xObjectHelper )
            xObjectHelper->dispose();
        xObjectHelper.clear();

        // settings export

        if (!bStylesOnly)
        {
            uno::Sequence<uno::Any> aSettingsArgs
            {
                Any(xInfoSet),
                Any(xWriter),
                Any(xStatusIndicator)
            };

            SAL_INFO( "sc.filter", "settings export start" );

            bSettingsRet = ExportToComponent(xContext, xModel, xWriter, aDescriptor,
                u"settings.xml"_ustr,
                sTextMediaType,
                bOasis ? u"com.sun.star.comp.Calc.XMLOasisSettingsExporter"_ustr
                       : u"com.sun.star.comp.Calc.XMLSettingsExporter"_ustr,
                aSettingsArgs, pSharedData);

            SAL_INFO( "sc.filter", "settings export end" );
        }

        pSharedData.reset();

        if (xStatusIndicator.is())
            xStatusIndicator->end();
        return bStylesRet && ((!bStylesOnly && bDocRet && bMetaRet && bSettingsRet) || bStylesOnly);
    }

    // later: give string descriptor as parameter for doc type

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
