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

#include <config_features.h>

#include <sal/config.h>

#include <cassert>

#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/frame/theGlobalEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionAskLater.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/embed/XLinkageSupport.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/XEncryptionProtectedStorage.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/interaction.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/securityoptions.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/streamwrap.hxx>

#include <unotools/saveopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/docinfohelper.hxx>
#include <ucbhelper/content.hxx>
#include <sot/storage.hxx>
#include <sot/storinfo.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/string.hxx>
#include <vcl/bitmapex.hxx>
#include <svtools/embedhlp.hxx>
#include <basic/modsizeexceeded.hxx>
#include <officecfg/Office/Common.hxx>
#include <osl/file.hxx>

#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
#include "appopen.hxx"
#include "objshimp.hxx"
#include "sfxtypes.hxx"
#include "doc.hrc"
#include <sfx2/sfxsids.hrc>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include "openflag.hxx"
#include "helper.hxx"
#include <sfx2/event.hxx>
#include "fltoptint.hxx"
#include <sfx2/viewfrm.hxx>
#include "graphhelp.hxx"
#include "appbaslib.hxx"
#include "appdata.hxx"
#include <objstor.hxx>

#include "../appl/app.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::document;
using namespace ::cppu;


void impl_addToModelCollection(const css::uno::Reference< css::frame::XModel >& xModel)
{
    if (!xModel.is())
        return;

    css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::frame::XGlobalEventBroadcaster > xModelCollection =
        css::frame::theGlobalEventBroadcaster::get(xContext);
    try
    {
        xModelCollection->insert(css::uno::makeAny(xModel));
    }
    catch ( uno::Exception& )
    {
        SAL_WARN( "sfx.doc", "The document seems to be in the collection already!\n" );
    }
}


bool SfxObjectShell::Save()
{
    return SaveChildren();
}


bool SfxObjectShell::SaveAs( SfxMedium& rMedium )
{
    return SaveAsChildren( rMedium );
}


bool SfxObjectShell::QuerySlotExecutable( sal_uInt16 /*nSlotId*/ )
{
    return true;
}


bool GetEncryptionData_Impl( const SfxItemSet* pSet, uno::Sequence< beans::NamedValue >& o_rEncryptionData )
{
    bool bResult = false;
    if ( pSet )
    {
        const SfxUnoAnyItem* pEncryptionDataItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_ENCRYPTIONDATA, false);
        if ( pEncryptionDataItem )
        {
            pEncryptionDataItem->GetValue() >>= o_rEncryptionData;
            bResult = true;
        }
        else
        {
            const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_PASSWORD, false);
            if ( pPasswordItem )
            {
                OUString aPassword = pPasswordItem->GetValue();
                o_rEncryptionData = ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPassword );
                bResult = true;
            }
        }
    }

    return bResult;
}


bool SfxObjectShell::PutURLContentsToVersionStream_Impl(
                                            const OUString& aURL,
                                            const uno::Reference< embed::XStorage >& xDocStorage,
                                            const OUString& aStreamName )
{
    bool bResult = false;
    try
    {
        uno::Reference< embed::XStorage > xVersion = xDocStorage->openStorageElement(
                                                        "Versions",
                                                        embed::ElementModes::READWRITE );

        DBG_ASSERT( xVersion.is(),
                "The method must throw an exception if the storage can not be opened!\n" );
        if ( !xVersion.is() )
            throw uno::RuntimeException();

        uno::Reference< io::XStream > xVerStream = xVersion->openStreamElement(
                                                                aStreamName,
                                                                embed::ElementModes::READWRITE );
        DBG_ASSERT( xVerStream.is(), "The method must throw an exception if the storage can not be opened!\n" );
        if ( !xVerStream.is() )
            throw uno::RuntimeException();

        uno::Reference< io::XOutputStream > xOutStream = xVerStream->getOutputStream();
        uno::Reference< io::XTruncate > xTrunc( xOutStream, uno::UNO_QUERY );

        DBG_ASSERT( xTrunc.is(), "The output stream must exist and implement XTruncate interface!\n" );
        if ( !xTrunc.is() )
            throw RuntimeException();

        uno::Reference< io::XInputStream > xTmpInStream =
            ::comphelper::OStorageHelper::GetInputStreamFromURL(
                aURL, comphelper::getProcessComponentContext() );
        assert( xTmpInStream.is() );

        xTrunc->truncate();
        ::comphelper::OStorageHelper::CopyInputToOutput( xTmpInStream, xOutStream );
        xOutStream->closeOutput();

        uno::Reference< embed::XTransactedObject > xTransact( xVersion, uno::UNO_QUERY );
        DBG_ASSERT( xTransact.is(), "The storage must implement XTransacted interface!\n" );
        if ( xTransact.is() )
            xTransact->commit();

        bResult = true;
    }
    catch( uno::Exception& )
    {
        // TODO/LATER: handle the error depending on exception
        SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
    }

    return bResult;
}


OUString SfxObjectShell::CreateTempCopyOfStorage_Impl( const uno::Reference< embed::XStorage >& xStorage )
{
    OUString aTempURL = ::utl::TempFile().GetURL();

    DBG_ASSERT( !aTempURL.isEmpty(), "Can't create a temporary file!\n" );
    if ( !aTempURL.isEmpty() )
    {
        try
        {
            uno::Reference< embed::XStorage > xTempStorage =
                ::comphelper::OStorageHelper::GetStorageFromURL( aTempURL, embed::ElementModes::READWRITE );

            // the password will be transferred from the xStorage to xTempStorage by storage implementation
            xStorage->copyToStorage( xTempStorage );

            // the temporary storage was committed by the previous method and it will die by refcount
        }
        catch ( uno::Exception& )
        {
            SAL_WARN( "sfx.doc", "Creation of a storage copy is failed!" );
            ::utl::UCBContentHelper::Kill( aTempURL );

            aTempURL.clear();

            // TODO/LATER: may need error code setting based on exception
            SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
        }
    }

    return aTempURL;
}


SvGlobalName SfxObjectShell::GetClassName() const
{
    return GetFactory().GetClassId();
}

namespace {

/**
 * Chart2 does not have an Object shell, so handle this here for now
 * If we ever implement a full scale object shell in chart2 move it there
 */
SotClipboardFormatId GetChartVersion( sal_Int32 nVersion, bool bTemplate )
{
    if( nVersion == SOFFICE_FILEFORMAT_60)
    {
        return SotClipboardFormatId::STARCHART_60;
    }
    else if( nVersion == SOFFICE_FILEFORMAT_8)
    {
        if (bTemplate)
        {
            SAL_WARN("sfx.doc", "no chart template support yet");
            return SotClipboardFormatId::STARCHART_8;
        }
        else
            return SotClipboardFormatId::STARCHART_8;
    }

    SAL_WARN("sfx.doc", "unsupported version");
    return SotClipboardFormatId::NONE;
}

}


void SfxObjectShell::SetupStorage( const uno::Reference< embed::XStorage >& xStorage,
                                   sal_Int32 nVersion, bool bTemplate, bool bChart ) const
{
    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );

    if ( xProps.is() )
    {
        SvGlobalName aName;
        OUString aFullTypeName, aShortTypeName, aAppName;
        SotClipboardFormatId nClipFormat = SotClipboardFormatId::NONE;

        if(!bChart)
            FillClass( &aName, &nClipFormat, &aAppName, &aFullTypeName, &aShortTypeName, nVersion, bTemplate );
        else
            nClipFormat = GetChartVersion(nVersion, bTemplate);

        if ( nClipFormat != SotClipboardFormatId::NONE )
        {
            // basic doesn't have a ClipFormat
            // without MediaType the storage is not really usable, but currently the BasicIDE still
            // is an SfxObjectShell and so we can't take this as an error
            datatransfer::DataFlavor aDataFlavor;
            SotExchange::GetFormatDataFlavor( nClipFormat, aDataFlavor );
            if ( !aDataFlavor.MimeType.isEmpty() )
            {
                try
                {
                    xProps->setPropertyValue("MediaType", uno::makeAny( aDataFlavor.MimeType ) );
                }
                catch( uno::Exception& )
                {
                    const_cast<SfxObjectShell*>( this )->SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
                }

                SvtSaveOptions::ODFDefaultVersion nDefVersion = SvtSaveOptions::ODFVER_012;
                bool bUseSHA1InODF12 = false;
                bool bUseBlowfishInODF12 = false;

                if (!utl::ConfigManager::IsAvoidConfig())
                {
                    SvtSaveOptions aSaveOpt;
                    nDefVersion = aSaveOpt.GetODFDefaultVersion();
                    bUseSHA1InODF12 = aSaveOpt.IsUseSHA1InODF12();
                    bUseBlowfishInODF12 = aSaveOpt.IsUseBlowfishInODF12();
                }

                // the default values, that should be used for ODF1.1 and older formats
                uno::Sequence< beans::NamedValue > aEncryptionAlgs
                {
                    { "StartKeyGenerationAlgorithm", css::uno::makeAny(xml::crypto::DigestID::SHA1) },
                    { "EncryptionAlgorithm", css::uno::makeAny(xml::crypto::CipherID::BLOWFISH_CFB_8) },
                    { "ChecksumAlgorithm", css::uno::makeAny(xml::crypto::DigestID::SHA1_1K) }
                };

                if ( nDefVersion >= SvtSaveOptions::ODFVER_012 )
                {
                    try
                    {
                        // older versions can not have this property set, it exists only starting from ODF1.2
                        xProps->setPropertyValue("Version", uno::makeAny<OUString>( ODFVER_012_TEXT ) );
                    }
                    catch( uno::Exception& )
                    {
                    }

                    if ( !bUseSHA1InODF12 && nDefVersion != SvtSaveOptions::ODFVER_012_EXT_COMPAT )
                    {
                        aEncryptionAlgs[0].Value <<= xml::crypto::DigestID::SHA256;
                        aEncryptionAlgs[2].Value <<= xml::crypto::DigestID::SHA256_1K;
                    }
                    if ( !bUseBlowfishInODF12 && nDefVersion != SvtSaveOptions::ODFVER_012_EXT_COMPAT )
                        aEncryptionAlgs[1].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
                }

                try
                {
                    // set the encryption algorithms accordingly;
                    // the setting does not trigger encryption,
                    // it just provides the format for the case that contents should be encrypted
                    uno::Reference< embed::XEncryptionProtectedStorage > xEncr( xStorage, uno::UNO_QUERY_THROW );
                    xEncr->setEncryptionAlgorithms( aEncryptionAlgs );
                }
                catch( uno::Exception& )
                {
                    const_cast<SfxObjectShell*>( this )->SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
                }

            }
        }
    }
}


void SfxObjectShell::PrepareSecondTryLoad_Impl()
{
    // only for internal use
    pImpl->m_xDocStorage.clear();
    pImpl->m_bIsInit = false;
    ResetError();
}


bool SfxObjectShell::GeneralInit_Impl( const uno::Reference< embed::XStorage >& xStorage,
                                            bool bTypeMustBeSetAlready )
{
    if ( pImpl->m_bIsInit )
        return false;

    pImpl->m_bIsInit = true;
    if ( xStorage.is() )
    {
        // no notification is required the storage is set the first time
        pImpl->m_xDocStorage = xStorage;

        try {
            uno::Reference < beans::XPropertySet > xPropSet( xStorage, uno::UNO_QUERY_THROW );
            Any a = xPropSet->getPropertyValue("MediaType");
            OUString aMediaType;
            if ( !(a>>=aMediaType) || aMediaType.isEmpty() )
            {
                if ( bTypeMustBeSetAlready )
                {
                    SetError( ERRCODE_IO_BROKENPACKAGE, OSL_LOG_PREFIX );
                    return false;
                }

                SetupStorage( xStorage, SOFFICE_FILEFORMAT_CURRENT, false );
            }
        }
        catch ( uno::Exception& )
        {
            SAL_WARN( "sfx.doc", "Can't check storage's mediatype!" );
        }
    }
    else
        pImpl->m_bCreateTempStor = true;

    return true;
}


bool SfxObjectShell::InitNew( const uno::Reference< embed::XStorage >& xStorage )
{
    return GeneralInit_Impl( xStorage, false );
}


bool SfxObjectShell::Load( SfxMedium& rMedium )
{
    return GeneralInit_Impl( rMedium.GetStorage(), true );
}

void SfxObjectShell::DoInitUnitTest()
{
    pMedium = new SfxMedium;
}

bool SfxObjectShell::DoInitNew( SfxMedium* pMed )
/*  [Description]

    This from SvPersist inherited virtual method is called to initialize
    the SfxObjectShell instance from a storage (PStore! = 0) or (PStore == 0)

    Like with all Do...-methods there is a from a control, the actual
    implementation is done by the virtual method in which also the
    InitNew(SvStorate *) from the SfxObjectShell-Subclass is implemented.

    For pStore == 0 the SfxObjectShell-instance is connected to an empty
    SfxMedium, otherwise a SfxMedium, which refers to the SotStorage
    passed as a parameter.

    The object is only initialized correctly after InitNew() or Load().

    [Return value]
    true            The object has been initialized.
    false           The object could not be initialized
*/

{
    ModifyBlocker_Impl aBlock( this );
    pMedium = pMed;
    if ( !pMedium )
    {
        pMedium = new SfxMedium;
    }

    pMedium->CanDisposeStorage_Impl( true );

    if ( InitNew( pMed ? pMed->GetStorage() : uno::Reference < embed::XStorage >() ) )
    {
        // empty documents always get their macros from the user, so there is no reason to restrict access
        pImpl->aMacroMode.allowMacroExecution();
        if ( SfxObjectCreateMode::EMBEDDED == eCreateMode )
            SetTitle(SfxResId(STR_NONAME).toString());

        uno::Reference< frame::XModel >  xModel ( GetModel(), uno::UNO_QUERY );
        if ( xModel.is() )
        {
            SfxItemSet *pSet = GetMedium()->GetItemSet();
            uno::Sequence< beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pSet, aArgs );
            sal_Int32 nLength = aArgs.getLength();
            aArgs.realloc( nLength + 1 );
            aArgs[nLength].Name = "Title";
            aArgs[nLength].Value <<= OUString( GetTitle( SFX_TITLE_DETECT ) );
            xModel->attachResource( OUString(), aArgs );
            if (!utl::ConfigManager::IsAvoidConfig())
                impl_addToModelCollection(xModel);
        }

        SetInitialized_Impl( true );
        return true;
    }

    return false;
}

bool SfxObjectShell::ImportFromGeneratedStream_Impl(
                    const uno::Reference< io::XStream >& xStream,
                    const uno::Sequence< beans::PropertyValue >& rMediaDescr )
{
    if ( !xStream.is() )
        return false;

    if ( pMedium && pMedium->HasStorage_Impl() )
        pMedium->CloseStorage();

    bool bResult = false;

    try
    {
        uno::Reference< embed::XStorage > xStorage =
            ::comphelper::OStorageHelper::GetStorageFromStream( xStream );

        if ( !xStorage.is() )
            throw uno::RuntimeException();

        if ( !pMedium )
            pMedium = new SfxMedium( xStorage, OUString() );
        else
            pMedium->SetStorage_Impl( xStorage );

        SfxAllItemSet aSet( SfxGetpApp()->GetPool() );
        TransformParameters( SID_OPENDOC, rMediaDescr, aSet );
        pMedium->GetItemSet()->Put( aSet );
        pMedium->CanDisposeStorage_Impl( false );
        uno::Reference<text::XTextRange> xInsertTextRange;
        for (sal_Int32 i = 0; i < rMediaDescr.getLength(); ++i)
        {
            if (rMediaDescr[i].Name == "TextInsertModeRange")
            {
                rMediaDescr[i].Value >>= xInsertTextRange;
            }
        }

        if (xInsertTextRange.is())
        {
            bResult = InsertGeneratedStream(*pMedium, xInsertTextRange);
        }
        else
        {

            // allow the subfilter to reinit the model
            if ( pImpl->m_bIsInit )
                pImpl->m_bIsInit = false;

            if ( LoadOwnFormat( *pMedium ) )
            {
                bHasName = true;
                if ( !IsReadOnly() && IsLoadReadonly() )
                    SetReadOnlyUI();

                bResult = true;
                OSL_ENSURE( pImpl->m_xDocStorage == xStorage, "Wrong storage is used!\n" );
            }
        }

        // now the medium can be disconnected from the storage
        // the medium is not allowed to dispose the storage so CloseStorage() can be used
        pMedium->CloseStorage();
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}


bool SfxObjectShell::DoLoad( SfxMedium *pMed )
{
    ModifyBlocker_Impl aBlock( this );

    pMedium = pMed;
    pMedium->CanDisposeStorage_Impl( true );

    bool bOk = false;
    std::shared_ptr<const SfxFilter> pFilter = pMed->GetFilter();
    SfxItemSet* pSet = pMedium->GetItemSet();
    if( pImpl->nEventId == SfxEventHintId::NONE )
    {
        const SfxBoolItem* pTemplateItem = SfxItemSet::GetItem<SfxBoolItem>(pSet, SID_TEMPLATE, false);
        SetActivateEvent_Impl(
            ( pTemplateItem && pTemplateItem->GetValue() )
            ? SfxEventHintId::CreateDoc : SfxEventHintId::OpenDoc );
    }

    const SfxStringItem* pBaseItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_BASEURL, false);
    OUString aBaseURL;
    const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(pMedium->GetItemSet(), SID_DOC_SALVAGE, false);
    if( pBaseItem )
        aBaseURL = pBaseItem->GetValue();
    else
    {
        if ( pSalvageItem )
        {
            OUString aName( pMed->GetPhysicalName() );
            osl::FileBase::getFileURLFromSystemPath( aName, aBaseURL );
        }
        else
            aBaseURL = pMed->GetBaseURL();
    }
    pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, aBaseURL ) );

    pImpl->nLoadedFlags = SfxLoadedFlags::NONE;
    pImpl->bModelInitialized = false;

    //TODO/LATER: make a clear strategy how to handle "UsesStorage" etc.
    bool bOwnStorageFormat = IsOwnStorageFormat( *pMedium );
    bool bHasStorage = IsPackageStorageFormat_Impl( *pMedium );
    if ( pMedium->GetFilter() )
    {
        sal_uInt32 nError = HandleFilter( pMedium, this );
        if ( nError != ERRCODE_NONE )
            SetError( nError, OSL_LOG_PREFIX );

        if (pMedium->GetFilter()->GetFilterFlags() & SfxFilterFlags::STARTPRESENTATION)
            pSet->Put( SfxBoolItem( SID_DOC_STARTPRESENTATION, true) );
    }

    EnableSetModified( false );

    pMedium->LockOrigFileOnDemand( true, false );
    if ( GetError() == ERRCODE_NONE && bOwnStorageFormat && ( !pFilter || !( pFilter->GetFilterFlags() & SfxFilterFlags::STARONEFILTER ) ) )
    {
        uno::Reference< embed::XStorage > xStorage;
        if ( pMedium->GetError() == ERRCODE_NONE )
            xStorage = pMedium->GetStorage();

        if( xStorage.is() && pMedium->GetLastStorageCreationState() == ERRCODE_NONE )
        {
            DBG_ASSERT( pFilter, "No filter for storage found!" );

            try
            {
                bool bWarnMediaTypeFallback = false;
                const SfxBoolItem* pRepairPackageItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_REPAIRPACKAGE, false);

                // treat the package as broken if the mediatype was retrieved as a fallback
                uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );
                xStorProps->getPropertyValue("MediaTypeFallbackUsed")
                                                                    >>= bWarnMediaTypeFallback;

                if ( pRepairPackageItem && pRepairPackageItem->GetValue() )
                {
                    // the macros in repaired documents should be disabled
                    pMedium->GetItemSet()->Put( SfxUInt16Item( SID_MACROEXECMODE, document::MacroExecMode::NEVER_EXECUTE ) );

                    // the mediatype was retrieved by using fallback solution but this is a repairing mode
                    // so it is acceptable to open the document if there is no contents that required manifest.xml
                    bWarnMediaTypeFallback = false;
                }

                if ( bWarnMediaTypeFallback || !xStorage->getElementNames().getLength() )
                    SetError( ERRCODE_IO_BROKENPACKAGE, OSL_LOG_PREFIX );
            }
            catch( uno::Exception& )
            {
                // TODO/LATER: may need error code setting based on exception
                SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
            }

            // Load
            if ( !GetError() )
            {
                pImpl->nLoadedFlags = SfxLoadedFlags::NONE;
                pImpl->bModelInitialized = false;
                bOk = xStorage.is() && LoadOwnFormat( *pMed );
                if ( bOk )
                {
                    // the document loaded from template has no name
                    const SfxBoolItem* pTemplateItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_TEMPLATE, false);
                    if ( !pTemplateItem || !pTemplateItem->GetValue() )
                        bHasName = true;
                }
                else
                    SetError( ERRCODE_ABORT, OSL_LOG_PREFIX );
            }
        }
        else
            SetError( pMed->GetLastStorageCreationState(), OSL_LOG_PREFIX );
    }
    else if ( GetError() == ERRCODE_NONE && InitNew(nullptr) )
    {
        // Name vor ConvertFrom setzen, damit GetSbxObject() schon funktioniert
        bHasName = true;
        SetName( SfxResId(STR_NONAME).toString() );

        if( !bHasStorage )
            pMedium->GetInStream();
        else
            pMedium->GetStorage();

        if ( GetError() == ERRCODE_NONE )
        {
            pImpl->nLoadedFlags = SfxLoadedFlags::NONE;
            pImpl->bModelInitialized = false;
            if ( pMedium->GetFilter() && ( pMedium->GetFilter()->GetFilterFlags() & SfxFilterFlags::STARONEFILTER ) )
            {
                uno::Reference < beans::XPropertySet > xSet( GetModel(), uno::UNO_QUERY );
                OUString sLockUpdates("LockUpdates");
                bool bSetProperty = true;
                try
                {
                    xSet->setPropertyValue( sLockUpdates, makeAny( true ) );
                }
                catch(const beans::UnknownPropertyException& )
                {
                    bSetProperty = false;
                }
                bOk = ImportFrom(*pMedium, nullptr);
                if(bSetProperty)
                {
                    try
                    {
                        xSet->setPropertyValue( sLockUpdates, makeAny( false ) );
                    }
                    catch(const beans::UnknownPropertyException& )
                    {}
                }
                UpdateLinks();
                FinishedLoading();
            }
            else
            {
                bOk = ConvertFrom(*pMedium);
                InitOwnModel_Impl();
            }
        }
    }

    if ( bOk )
    {
        if ( IsReadOnlyMedium() || IsLoadReadonly() )
            SetReadOnlyUI();

        try
        {
            ::ucbhelper::Content aContent( pMedium->GetName(), utl::UCBContentHelper::getDefaultCommandEnvironment(), comphelper::getProcessComponentContext() );
            css::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                OUString aAuthor( "Author" );
                OUString aKeywords( "Keywords" );
                OUString aSubject( "Subject" );
                Any aAny;
                OUString aValue;
                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();
                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aAny = aContent.getPropertyValue( aAuthor );
                    if ( ( aAny >>= aValue ) )
                        xDocProps->setAuthor(aValue);
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    aAny = aContent.getPropertyValue( aKeywords );
                    if ( ( aAny >>= aValue ) )
                        xDocProps->setKeywords(
                          ::comphelper::string::convertCommaSeparated(aValue));
;
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aAny = aContent.getPropertyValue( aSubject );
                    if ( ( aAny >>= aValue ) ) {
                        xDocProps->setSubject(aValue);
                    }
                }
            }
        }
        catch( Exception& )
        {
        }

        // If not loaded asynchronously call FinishedLoading
        if ( !( pImpl->nLoadedFlags & SfxLoadedFlags::MAINDOCUMENT ) &&
              ( !pMedium->GetFilter() || pMedium->GetFilter()->UsesStorage() )
            )
            FinishedLoading( SfxLoadedFlags::MAINDOCUMENT );

        if( IsOwnStorageFormat(*pMed) && pMed->GetFilter() )
        {
        }
        Broadcast( SfxHint(SfxHintId::NameChanged) );

        if ( SfxObjectCreateMode::EMBEDDED != eCreateMode )
        {
            const SfxBoolItem* pAsTempItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_TEMPLATE, false);
            const SfxBoolItem* pPreviewItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_PREVIEW, false);
            const SfxBoolItem* pHiddenItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_HIDDEN, false);
            if( bOk && !pMedium->GetOrigURL().isEmpty()
            && !( pAsTempItem && pAsTempItem->GetValue() )
            && !( pPreviewItem && pPreviewItem->GetValue() )
            && !( pHiddenItem && pHiddenItem->GetValue() ) )
            {
                AddToRecentlyUsedList();
            }
        }

        const SfxBoolItem* pDdeReconnectItem = SfxItemSet::GetItem<SfxBoolItem>(pMedium->GetItemSet(), SID_DDE_RECONNECT_ONLOAD, false);

        bool bReconnectDde = true; // by default, we try to auto-connect DDE connections.
        if (pDdeReconnectItem)
            bReconnectDde = pDdeReconnectItem->GetValue();

        if (bReconnectDde)
            ReconnectDdeLinks(*this);
    }

    return bOk;
}

bool SfxObjectShell::DoLoadExternal( SfxMedium *pMed )
{
    pMedium = pMed;
    return LoadExternal(*pMedium);
}

sal_uInt32 SfxObjectShell::HandleFilter( SfxMedium* pMedium, SfxObjectShell* pDoc )
{
    sal_uInt32 nError = ERRCODE_NONE;
    SfxItemSet* pSet = pMedium->GetItemSet();
    const SfxStringItem* pOptions = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_FILE_FILTEROPTIONS, false);
    const SfxUnoAnyItem* pData = SfxItemSet::GetItem<SfxUnoAnyItem>(pSet, SID_FILTER_DATA, false);
    if ( !pData && !pOptions )
    {
        css::uno::Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        css::uno::Reference< XNameAccess > xFilterCFG;
        if( xServiceManager.is() )
        {
            xFilterCFG.set( xServiceManager->createInstance("com.sun.star.document.FilterFactory"),
                            UNO_QUERY );
        }

        if( xFilterCFG.is() )
        {
            try {
                bool bAbort = false;
                std::shared_ptr<const SfxFilter> pFilter = pMedium->GetFilter();
                Sequence < PropertyValue > aProps;
                Any aAny = xFilterCFG->getByName( pFilter->GetName() );
                if ( aAny >>= aProps )
                {
                    sal_Int32 nPropertyCount = aProps.getLength();
                    for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                        if( aProps[nProperty].Name == "UIComponent" )
                        {
                            OUString aServiceName;
                            aProps[nProperty].Value >>= aServiceName;
                            if( !aServiceName.isEmpty() )
                            {
                                css::uno::Reference< XInteractionHandler > rHandler = pMedium->GetInteractionHandler();
                                if( rHandler.is() )
                                {
                                    // we need some properties in the media descriptor, so we have to make sure that they are in
                                    Any aStreamAny;
                                    aStreamAny <<= pMedium->GetInputStream();
                                    if ( pSet->GetItemState( SID_INPUTSTREAM ) < SfxItemState::SET )
                                        pSet->Put( SfxUnoAnyItem( SID_INPUTSTREAM, aStreamAny ) );
                                    if ( pSet->GetItemState( SID_FILE_NAME ) < SfxItemState::SET )
                                        pSet->Put( SfxStringItem( SID_FILE_NAME, pMedium->GetName() ) );
                                    if ( pSet->GetItemState( SID_FILTER_NAME ) < SfxItemState::SET )
                                        pSet->Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );

                                    Sequence< PropertyValue > rProperties;
                                    TransformItems( SID_OPENDOC, *pSet, rProperties );
                                    RequestFilterOptions* pFORequest = new RequestFilterOptions( pDoc->GetModel(), rProperties );

                                    css::uno::Reference< XInteractionRequest > rRequest( pFORequest );
                                    rHandler->handle( rRequest );

                                    if ( !pFORequest->isAbort() )
                                    {
                                            SfxAllItemSet aNewParams( pDoc->GetPool() );
                                            TransformParameters( SID_OPENDOC,
                                                            pFORequest->getFilterOptions(),
                                                            aNewParams );

                                            const SfxStringItem* pFilterOptions = aNewParams.GetItem<SfxStringItem>(SID_FILE_FILTEROPTIONS, false);
                                            if ( pFilterOptions )
                                                pSet->Put( *pFilterOptions );

                                            const SfxUnoAnyItem* pFilterData = aNewParams.GetItem<SfxUnoAnyItem>(SID_FILTER_DATA, false);
                                            if ( pFilterData )
                                                pSet->Put( *pFilterData );
                                    }
                                    else
                                        bAbort = true;
                                }
                            }

                            break;
                        }
                }

                if( bAbort )
                {
                    // filter options were not entered
                    nError = ERRCODE_ABORT;
                }
            }
            catch( NoSuchElementException& )
            {
                // the filter name is unknown
                nError = ERRCODE_IO_INVALIDPARAMETER;
            }
            catch( Exception& )
            {
                nError = ERRCODE_ABORT;
            }
        }
    }

    return nError;
}


bool SfxObjectShell::IsOwnStorageFormat(const SfxMedium &rMedium)
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->IsOwnFormat() &&
             rMedium.GetFilter()->UsesStorage() &&
             rMedium.GetFilter()->GetVersion() >= SOFFICE_FILEFORMAT_60 );
}


bool SfxObjectShell::IsPackageStorageFormat_Impl(const SfxMedium &rMedium)
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->UsesStorage() &&
             rMedium.GetFilter()->GetVersion() >= SOFFICE_FILEFORMAT_60 );
}


bool SfxObjectShell::DoSave()
// DoSave is only invoked for OLE. Save your own documents in the SFX through
// DoSave_Impl order to allow for the creation of backups.
// Save in your own format again.
{
    bool bOk = false ;
    {
        ModifyBlocker_Impl aBlock( this );

        pImpl->bIsSaving = true;

        uno::Sequence< beans::NamedValue > aEncryptionData;
        if ( IsPackageStorageFormat_Impl( *GetMedium() ) )
        {
            if ( GetEncryptionData_Impl( GetMedium()->GetItemSet(), aEncryptionData ) )
            {
                try
                {
                    //TODO/MBA: GetOutputStorage?! Special mode, because it's "Save"?!
                    ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( GetMedium()->GetStorage(), aEncryptionData );
                    bOk = true;
                }
                catch( uno::Exception& )
                {
                    SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
                }

                DBG_ASSERT( bOk, "The root storage must allow to set common password!\n" );
            }
            else
                bOk = true;
#if HAVE_FEATURE_SCRIPTING
            if ( HasBasic() )
            {
                try
                {
                    // The basic and dialogs related contents are still not able to proceed with save operation ( saveTo only )
                    // so since the document storage is locked a workaround has to be used

                    uno::Reference< embed::XStorage > xTmpStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                    DBG_ASSERT( xTmpStorage.is(), "If a storage can not be created an exception must be thrown!\n" );
                    if ( !xTmpStorage.is() )
                        throw uno::RuntimeException();

                    OUString aBasicStorageName( "Basic"  );
                    OUString aDialogsStorageName( "Dialogs"  );
                    if ( GetMedium()->GetStorage()->hasByName( aBasicStorageName ) )
                        GetMedium()->GetStorage()->copyElementTo( aBasicStorageName, xTmpStorage, aBasicStorageName );
                    if ( GetMedium()->GetStorage()->hasByName( aDialogsStorageName ) )
                        GetMedium()->GetStorage()->copyElementTo( aDialogsStorageName, xTmpStorage, aDialogsStorageName );

                    GetBasicManager();

                    // disconnect from the current storage
                    pImpl->aBasicManager.setStorage( xTmpStorage );

                    // store to the current storage
                    pImpl->aBasicManager.storeLibrariesToStorage( GetMedium()->GetStorage() );

                    // connect to the current storage back
                    pImpl->aBasicManager.setStorage( GetMedium()->GetStorage() );
                }
                catch( uno::Exception& )
                {
                    SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
                    bOk = false;
                }
            }
#endif
        }

        if (bOk)
            bOk = Save();

        if (bOk)
            bOk = pMedium->Commit();
    }

    return bOk;
}

void Lock_Impl( SfxObjectShell* pDoc, bool bLock )
{
    SfxViewFrame *pFrame= SfxViewFrame::GetFirst( pDoc );
    while ( pFrame )
    {
        pFrame->GetDispatcher()->Lock( bLock );
        pFrame->Enable( !bLock );
        pFrame = SfxViewFrame::GetNext( *pFrame, pDoc );
    }

}


bool SfxObjectShell::SaveTo_Impl
(
     SfxMedium &rMedium, // Medium, in which it will be stored
     const SfxItemSet* pSet
)

/*  [Description]

    Writes the current contents to the medium rMedium. If the target medium is
    no storage, then saving to a temporary storage, or directly if the medium
    is transacted, if we ourselves have opened it, and if we are a server
    either the container a transacted storage provides or created a
    temporary storage by one self.
*/

{
    SAL_INFO( "sfx.doc", "saving \"" << rMedium.GetName() << "\"" );

    UpdateDocInfoForSave();

    AddLog( OSL_LOG_PREFIX "Begin" );

    ModifyBlocker_Impl aMod(this);

    std::shared_ptr<const SfxFilter> pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        // if no filter was set, use the default filter
        // this should be changed in the feature, it should be an error!
        SAL_WARN( "sfx.doc","No filter set!");
        pFilter = GetFactory().GetFilterContainer()->GetAnyFilter( SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT );
        rMedium.SetFilter(pFilter);
    }

    bool bStorageBasedSource = IsPackageStorageFormat_Impl( *pMedium );
    bool bStorageBasedTarget = IsPackageStorageFormat_Impl( rMedium );
    bool bOwnSource = IsOwnStorageFormat( *pMedium );
    bool bOwnTarget = IsOwnStorageFormat( rMedium );

    // Examine target format to determine whether to query if any password
    // protected libraries exceed the size we can handler
    if ( bOwnTarget && !QuerySaveSizeExceededModules_Impl( rMedium.GetInteractionHandler() ) )
    {
        SetError( ERRCODE_IO_ABORT, OSL_LOG_PREFIX );
        return false;
    }

    bool bNeedsDisconnectionOnFail = false;

    bool bStoreToSameLocation = false;

    // the detection whether the script is changed should be done before saving
    bool bTryToPreserveScriptSignature = false;
    // no way to detect whether a filter is oasis format, have to wait for saving process
    bool bNoPreserveForOasis = false;
    if ( bOwnSource && bOwnTarget
      && ( pImpl->nScriptingSignatureState == SignatureState::OK
        || pImpl->nScriptingSignatureState == SignatureState::NOTVALIDATED
        || pImpl->nScriptingSignatureState == SignatureState::INVALID ) )
    {
        AddLog( OSL_LOG_PREFIX "MacroSignaturePreserving" );

        // the checking of the library modified state iterates over the libraries, should be done only when required
        // currently the check is commented out since it is broken, we have to check the signature every time we save
        // TODO/LATER: let isAnyContainerModified() work!
        bTryToPreserveScriptSignature = true; // !pImpl->pBasicManager->isAnyContainerModified();
        if ( bTryToPreserveScriptSignature )
        {
            // check that the storage format stays the same
            SvtSaveOptions aSaveOpt;
            SvtSaveOptions::ODFDefaultVersion nVersion = aSaveOpt.GetODFDefaultVersion();

            OUString aODFVersion;
            try
            {
                uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
                xPropSet->getPropertyValue("Version") >>= aODFVersion;
            }
            catch( uno::Exception& )
            {}

            // preserve only if the same filter has been used
            bTryToPreserveScriptSignature = pMedium->GetFilter() && pFilter && pMedium->GetFilter()->GetFilterName() == pFilter->GetFilterName();

            bNoPreserveForOasis = (
                                   (aODFVersion == ODFVER_012_TEXT && nVersion == SvtSaveOptions::ODFVER_011) ||
                                   (aODFVersion.isEmpty() && nVersion >= SvtSaveOptions::ODFVER_012)
                                  );
        }
    }

    bool bCopyTo = false;
    SfxItemSet *pMedSet = rMedium.GetItemSet();
    if( pMedSet )
    {
        const SfxBoolItem* pSaveToItem = SfxItemSet::GetItem<SfxBoolItem>(pMedSet, SID_SAVETO, false);
        bCopyTo =   GetCreateMode() == SfxObjectCreateMode::EMBEDDED ||
                    (pSaveToItem && pSaveToItem->GetValue());
    }

    // use UCB for case sensitive/insensitive file name comparison
    if ( !pMedium->GetName().equalsIgnoreAsciiCase("private:stream")
      && !rMedium.GetName().equalsIgnoreAsciiCase("private:stream")
      && ::utl::UCBContentHelper::EqualURLs( pMedium->GetName(), rMedium.GetName() ) )
    {
        bStoreToSameLocation = true;
        AddLog( OSL_LOG_PREFIX "Save" );

        if ( pMedium->DocNeedsFileDateCheck() )
            rMedium.CheckFileDate( pMedium->GetInitFileDate( false ) );

        if ( bCopyTo && GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
        {
            // export to the same location is forbidden
            SetError( ERRCODE_IO_CANTWRITE, OSL_LOG_PREFIX );
        }
        else
        {
            // before we overwrite the original file, we will make a backup if there is a demand for that
            // if the backup is not created here it will be created internally and will be removed in case of successful saving
            const bool bDoBackup = SvtSaveOptions().IsBackup();
            if ( bDoBackup )
            {
                AddLog( OSL_LOG_PREFIX "DoBackup" );
                rMedium.DoBackup_Impl();
                if ( rMedium.GetError() )
                {
                    SetError( rMedium.GetErrorCode(), OSL_LOG_PREFIX );
                    rMedium.ResetError();
                }
            }

            if ( bStorageBasedSource && bStorageBasedTarget )
            {
                // The active storage must be switched. The simple saving is not enough.
                // The problem is that the target medium contains target MediaDescriptor.

                    // In future the switch of the persistence could be done on stream level:
                    // a new wrapper service will be implemented that allows to exchange
                    // persistence on the fly. So the real persistence will be set
                    // to that stream only after successful commit of the storage.
                    // TODO/LATER:
                    // create wrapper stream based on the URL
                    // create a new storage based on this stream
                    // store to this new storage
                    // commit the new storage
                    // call saveCompleted based with this new storage ( get rid of old storage and "frees" URL )
                    // commit the wrapper stream ( the stream will connect the URL only on commit, after that it will hold it )
                    // if the last step is failed the stream should stay to be transacted and should be committed on any flush
                    // so we can forget the stream in any way and the next storage commit will flush it

                AddLog( OSL_LOG_PREFIX "Save: Own to Own" );

                bNeedsDisconnectionOnFail = DisconnectStorage_Impl(
                    *pMedium, rMedium );
                if ( bNeedsDisconnectionOnFail
                  || ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                {
                    pMedium->CloseAndRelease();

                    // TODO/LATER: for now the medium must be closed since it can already contain streams from old medium
                    //             in future those streams should not be copied in case a valid target url is provided,
                    //             if the url is not provided ( means the document is based on a stream ) this code is not
                    //             reachable.
                    rMedium.CloseAndRelease();
                    rMedium.GetOutputStorage();
                }
            }
            else if ( !bStorageBasedSource && !bStorageBasedTarget )
            {
                // the source and the target formats are alien
                // just disconnect the stream from the source format
                // so that the target medium can use it

                AddLog( OSL_LOG_PREFIX "Save: Alien to Alien" );

                pMedium->CloseAndRelease();
                rMedium.CloseAndRelease();
                rMedium.CreateTempFileNoCopy();
                rMedium.GetOutStream();
            }
            else if ( !bStorageBasedSource && bStorageBasedTarget )
            {
                // the source format is an alien one but the target
                // format is an own one so just disconnect the source
                // medium

                AddLog( OSL_LOG_PREFIX "Save: Alien to Own" );

                pMedium->CloseAndRelease();
                rMedium.CloseAndRelease();
                rMedium.GetOutputStorage();
            }
            else // means if ( bStorageBasedSource && !bStorageBasedTarget )
            {
                // the source format is an own one but the target is
                // an alien format, just connect the source to temporary
                // storage

                AddLog( OSL_LOG_PREFIX "Save: Own to Alien" );

                bNeedsDisconnectionOnFail = DisconnectStorage_Impl(
                    *pMedium, rMedium );
                if ( bNeedsDisconnectionOnFail
                  || ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                {
                    pMedium->CloseAndRelease();
                    rMedium.CloseAndRelease();
                    rMedium.CreateTempFileNoCopy();
                    rMedium.GetOutStream();
                }
            }
        }
    }
    else
    {
        // This is SaveAs or export action, prepare the target medium
        // the alien filters still might write directly to the file, that is of course a bug,
        // but for now the framework has to be ready for it
        // TODO/LATER: let the medium be prepared for alien formats as well

        AddLog( OSL_LOG_PREFIX "SaveAs/Export" );

        rMedium.CloseAndRelease();
        if ( bStorageBasedTarget )
        {
            rMedium.GetOutputStorage();
        }
    }

    // TODO/LATER: error handling
    if( rMedium.GetErrorCode() || pMedium->GetErrorCode() || GetErrorCode() )
    {
        SAL_WARN("sfx.doc", "SfxObjectShell::SaveTo_Impl: very early error return");
        return false;
    }

    AddLog( OSL_LOG_PREFIX "Locking" );

    rMedium.LockOrigFileOnDemand( false, false );

    if ( bStorageBasedTarget )
    {
        if ( rMedium.GetErrorCode() )
            return false;

        // If the filter is a "cross export" filter ( f.e. a filter for exporting an impress document from
        // a draw document ), the ClassId of the destination storage is different from the ClassId of this
        // document. It can be retrieved from the default filter for the desired target format
        SotClipboardFormatId nFormat = rMedium.GetFilter()->GetFormat();
        SfxFilterMatcher& rMatcher = SfxGetpApp()->GetFilterMatcher();
        std::shared_ptr<const SfxFilter> pFilt = rMatcher.GetFilter4ClipBoardId( nFormat );
        if ( pFilt )
        {
            if ( pFilt->GetServiceName() != rMedium.GetFilter()->GetServiceName() )
            {
                datatransfer::DataFlavor aDataFlavor;
                SotExchange::GetFormatDataFlavor( nFormat, aDataFlavor );

                try
                {
                    uno::Reference< beans::XPropertySet > xProps( rMedium.GetStorage(), uno::UNO_QUERY );
                    DBG_ASSERT( xProps.is(), "The storage implementation must implement XPropertySet!" );
                    if ( !xProps.is() )
                        throw uno::RuntimeException();

                    xProps->setPropertyValue("MediaType",
                                            uno::makeAny( aDataFlavor.MimeType ) );
                }
                catch( uno::Exception& )
                {
                }
            }
        }
    }

    // TODO/LATER: error handling
    if( rMedium.GetErrorCode() || pMedium->GetErrorCode() || GetErrorCode() )
        return false;

    bool bOldStat = pImpl->bForbidReload;
    pImpl->bForbidReload = true;

    // lock user interface while saving the document
    Lock_Impl( this, true );

    bool bOk = false;
    // TODO/LATER: get rid of bOk
    if (bOwnTarget && pFilter && !(pFilter->GetFilterFlags() & SfxFilterFlags::STARONEFILTER))
    {
        AddLog( OSL_LOG_PREFIX "Storing in own format." );
        uno::Reference< embed::XStorage > xMedStorage = rMedium.GetStorage();
        if ( !xMedStorage.is() )
        {
            // no saving without storage, unlock UI and return
            Lock_Impl( this, false );
            pImpl->bForbidReload = bOldStat;
            AddLog( OSL_LOG_PREFIX "Storing failed, still no error set." );
            return false;
        }

        // transfer password from the parameters to the storage
        uno::Sequence< beans::NamedValue > aEncryptionData;
        bool bPasswdProvided = false;
        if ( GetEncryptionData_Impl( rMedium.GetItemSet(), aEncryptionData ) )
        {
            bPasswdProvided = true;
            try {
                ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( xMedStorage, aEncryptionData );
                bOk = true;
            }
            catch( uno::Exception& )
            {
                SAL_WARN( "sfx.doc", "Setting of common encryption key failed!" );
                SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
            }
        }
        else
            bOk = true;

        pFilter = rMedium.GetFilter();

        const SfxStringItem *pVersionItem = !rMedium.IsInCheckIn()? SfxItemSet::GetItem<SfxStringItem>(pSet, SID_DOCINFO_COMMENTS, false): nullptr;
        OUString aTmpVersionURL;

        if ( bOk )
        {
            bOk = false;
            // currently the case that the storage is the same should be impossible
            if ( xMedStorage == GetStorage() )
            {
                OSL_ENSURE( !pVersionItem, "This scenario is impossible currently!\n" );
                AddLog( OSL_LOG_PREFIX "Should be impossible." );
                // usual save procedure
                bOk = Save();
            }
            else
            {
                // save to target
                AddLog( OSL_LOG_PREFIX "Save as own format." );
                bOk = SaveAsOwnFormat( rMedium );
                if ( bOk && pVersionItem )
                {
                    AddLog( OSL_LOG_PREFIX "pVersionItem != NULL" );
                    aTmpVersionURL = CreateTempCopyOfStorage_Impl( xMedStorage );
                    bOk =  !aTmpVersionURL.isEmpty();
                }
            }
        }

        //fdo#61320: only store thumbnail image if the corresponding option is enabled in the configuration
        if ( bOk && officecfg::Office::Common::Save::Document::GenerateThumbnail::get()
                && GetCreateMode() != SfxObjectCreateMode::EMBEDDED && !bPasswdProvided && IsUseThumbnailSave() )
        {
            // store the thumbnail representation image
            // the thumbnail is not stored in case of encrypted document
            AddLog( OSL_LOG_PREFIX "Thumbnail creation." );
            if ( !GenerateAndStoreThumbnail( bPasswdProvided,
                                            pFilter->IsOwnTemplateFormat(),
                                            xMedStorage ) )
            {
                // TODO: error handling
                SAL_WARN( "sfx.doc", "Couldn't store thumbnail representation!" );
            }
        }

        if ( bOk )
        {
            if ( pImpl->bIsSaving || pImpl->bPreserveVersions )
            {
                AddLog( OSL_LOG_PREFIX "Preserve versions." );
                try
                {
                    Sequence < util::RevisionTag > aVersions = rMedium.GetVersionList();
                    if ( aVersions.getLength() )
                    {
                        // copy the version streams
                        OUString aVersionsName( "Versions"  );
                        uno::Reference< embed::XStorage > xNewVerStor = xMedStorage->openStorageElement(
                                                        aVersionsName,
                                                        embed::ElementModes::READWRITE );
                        uno::Reference< embed::XStorage > xOldVerStor = GetStorage()->openStorageElement(
                                                        aVersionsName,
                                                        embed::ElementModes::READ );
                        if ( !xNewVerStor.is() || !xOldVerStor.is() )
                            throw uno::RuntimeException();

                        for ( sal_Int32 n=0; n<aVersions.getLength(); n++ )
                        {
                            if ( xOldVerStor->hasByName( aVersions[n].Identifier ) )
                                xOldVerStor->copyElementTo( aVersions[n].Identifier, xNewVerStor, aVersions[n].Identifier );
                        }

                        uno::Reference< embed::XTransactedObject > xTransact( xNewVerStor, uno::UNO_QUERY );
                        if ( xTransact.is() )
                            xTransact->commit();
                    }
                }
                catch( uno::Exception& )
                {
                    AddLog( OSL_LOG_PREFIX "Preserve versions has failed." );
                    SAL_WARN( "sfx.doc", "Couldn't copy versions!" );
                    bOk = false;
                    // TODO/LATER: a specific error could be set
                }
            }

            if ( bOk && pVersionItem && !rMedium.IsInCheckIn() )
            {
                // store a version also
                const SfxStringItem *pAuthorItem = SfxItemSet::GetItem<SfxStringItem>(pSet, SID_DOCINFO_AUTHOR, false);

                // version comment
                util::RevisionTag aInfo;
                aInfo.Comment = pVersionItem->GetValue();

                // version author
                if ( pAuthorItem )
                    aInfo.Author = pAuthorItem->GetValue();
                else
                    // if not transferred as a parameter, get it from user settings
                    aInfo.Author = SvtUserOptions().GetFullName();

                DateTime aTime( DateTime::SYSTEM );
                aInfo.TimeStamp.Day = aTime.GetDay();
                aInfo.TimeStamp.Month = aTime.GetMonth();
                aInfo.TimeStamp.Year = aTime.GetYear();
                aInfo.TimeStamp.Hours = aTime.GetHour();
                aInfo.TimeStamp.Minutes = aTime.GetMin();
                aInfo.TimeStamp.Seconds = aTime.GetSec();

                if ( bOk )
                {
                    // add new version information into the versionlist and save the versionlist
                    // the version list must have been transferred from the "old" medium before
                    rMedium.AddVersion_Impl( aInfo );
                    rMedium.SaveVersionList_Impl();
                    bOk = PutURLContentsToVersionStream_Impl( aTmpVersionURL, xMedStorage, aInfo.Identifier );
                }
            }
            else if ( bOk && ( pImpl->bIsSaving || pImpl->bPreserveVersions ) )
            {
                rMedium.SaveVersionList_Impl();
            }
        }

        if ( !aTmpVersionURL.isEmpty() )
            ::utl::UCBContentHelper::Kill( aTmpVersionURL );
    }
    else
    {
        AddLog( OSL_LOG_PREFIX "Storing in alien format." );
        // it's a "SaveAs" in an alien format
        if ( rMedium.GetFilter() && ( rMedium.GetFilter()->GetFilterFlags() & SfxFilterFlags::STARONEFILTER ) )
            bOk = ExportTo( rMedium );
        else
            bOk = ConvertTo( rMedium );

        // after saving the document, the temporary object storage must be updated
        // if the old object storage was not a temporary one, it will be updated also, because it will be used
        // as a source for copying the objects into the new temporary storage that will be created below
        // updating means: all child objects must be stored into it
        // ( same as on loading, where these objects are copied to the temporary storage )
        // but don't commit these changes, because in the case when the old object storage is not a temporary one,
        // all changes will be written into the original file !

        if( bOk && !bCopyTo )
            // we also don't touch any graphical replacements here
            bOk = SaveChildren( true );
    }

    if ( bOk )
    {
        // if ODF version of oasis format changes on saving the signature should not be preserved
        if ( bOk && bTryToPreserveScriptSignature && bNoPreserveForOasis )
            bTryToPreserveScriptSignature = ( SotStorage::GetVersion( rMedium.GetStorage() ) == SOFFICE_FILEFORMAT_60 );

        uno::Reference< security::XDocumentDigitalSignatures > xDDSigns;
        if ( bOk && bTryToPreserveScriptSignature )
        {
            AddLog( OSL_LOG_PREFIX "Copying scripting signature." );

            // if the scripting code was not changed and it is signed the signature should be preserved
            // unfortunately at this point we have only information whether the basic code has changed or not
            // so the only way is to check the signature if the basic was not changed
            try
            {
                // get the ODF version of the new medium
                OUString aVersion;
                try
                {
                    uno::Reference < beans::XPropertySet > xPropSet( rMedium.GetStorage(), uno::UNO_QUERY_THROW );
                    xPropSet->getPropertyValue("Version") >>= aVersion;
                }
                catch( uno::Exception& )
                {
                }

                xDDSigns = security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), aVersion);

                OUString aScriptSignName = xDDSigns->getScriptingContentSignatureDefaultStreamName();

                if ( !aScriptSignName.isEmpty() )
                {
                    // target medium is still not committed, it should not be closed
                    // commit the package storage and close it, but leave the streams open
                    rMedium.StorageCommit_Impl();
                    rMedium.CloseStorage();

                    uno::Reference< embed::XStorage > xReadOrig = pMedium->GetZipStorageToSign_Impl();
                    if ( !xReadOrig.is() )
                        throw uno::RuntimeException();
                    uno::Reference< embed::XStorage > xMetaInf = xReadOrig->openStorageElement(
                                "META-INF",
                                embed::ElementModes::READ );

                    uno::Reference< embed::XStorage > xTarget = rMedium.GetZipStorageToSign_Impl( false );
                    if ( !xTarget.is() )
                        throw uno::RuntimeException();
                    uno::Reference< embed::XStorage > xTargetMetaInf = xTarget->openStorageElement(
                                "META-INF",
                                embed::ElementModes::READWRITE );

                    if ( xMetaInf.is() && xTargetMetaInf.is() )
                    {
                        xMetaInf->copyElementTo( aScriptSignName, xTargetMetaInf, aScriptSignName );

                        uno::Reference< embed::XTransactedObject > xTransact( xTargetMetaInf, uno::UNO_QUERY );
                        if ( xTransact.is() )
                            xTransact->commit();

                        xTargetMetaInf->dispose();

                        // now check the copied signature
                        uno::Sequence< security::DocumentSignatureInformation > aInfos =
                            xDDSigns->verifyScriptingContentSignatures( xTarget,
                                                                        uno::Reference< io::XInputStream >() );
                        SignatureState nState = ImplCheckSignaturesInformation( aInfos );
                        if ( nState == SignatureState::OK || nState == SignatureState::NOTVALIDATED
                            || nState == SignatureState::PARTIAL_OK)
                        {
                            rMedium.SetCachedSignatureState_Impl( nState );

                            // commit the ZipStorage from target medium
                            xTransact.set( xTarget, uno::UNO_QUERY );
                            if ( xTransact.is() )
                                xTransact->commit();
                        }
                        else
                        {
                            // it should not happen, the copies signature is invalid!
                            // throw the changes away
                            SAL_WARN( "sfx.doc", "An invalid signature was copied!" );
                        }
                    }
                }
            }
            catch( uno::Exception& )
            {
            }

            rMedium.CloseZipStorage_Impl();
        }

        AddLog( OSL_LOG_PREFIX "Medium commit." );

        OUString sName( rMedium.GetName( ) );
        bOk = rMedium.Commit();
        OUString sNewName( rMedium.GetName( ) );

        if ( sName != sNewName )
            GetMedium( )->SwitchDocumentToFile( sNewName );

        if ( bOk )
        {
            AddLog( OSL_LOG_PREFIX "Storing is successful." );

            // if the target medium is an alien format and the "old" medium was an own format and the "old" medium
            // has a name, the object storage must be exchanged, because now we need a new temporary storage
            // as object storage
            if ( !bCopyTo && bStorageBasedSource && !bStorageBasedTarget )
            {
                if ( bStoreToSameLocation )
                {
                    // if the old medium already disconnected from document storage, the storage still must
                    // be switched if backup file is used
                    if ( bNeedsDisconnectionOnFail )
                        ConnectTmpStorage_Impl( pImpl->m_xDocStorage, nullptr );
                }
                else if (!pMedium->GetName().isEmpty()
                  || ( pMedium->HasStorage_Impl() && pMedium->WillDisposeStorageOnClose_Impl() ) )
                {
                    OSL_ENSURE(!pMedium->GetName().isEmpty(), "Fallback is used, the medium without name should not dispose the storage!\n");
                    // copy storage of old medium to new temporary storage and take this over
                    if( !ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                    {
                        AddLog( OSL_LOG_PREFIX "Process after storing has failed." );
                        bOk = false;
                    }
                }
            }
        }
        else
        {
            AddLog( OSL_LOG_PREFIX "Storing has failed." );

            // in case the document storage was connected to backup temporarely it must be disconnected now
            if ( bNeedsDisconnectionOnFail )
                ConnectTmpStorage_Impl( pImpl->m_xDocStorage, nullptr );
        }
    }

    // unlock user interface
    Lock_Impl( this, false );
    pImpl->bForbidReload = bOldStat;

    if ( bOk )
    {
        try
        {
            ::ucbhelper::Content aContent( rMedium.GetName(), utl::UCBContentHelper::getDefaultCommandEnvironment(), comphelper::getProcessComponentContext() );
            css::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                OUString aAuthor( "Author" );
                OUString aKeywords( "Keywords" );
                OUString aSubject( "Subject" );

                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();

                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aContent.setPropertyValue( aAuthor, Any(xDocProps->getAuthor()) );
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    Any aAny;
                    aAny <<= ::comphelper::string::convertCommaSeparated(
                                xDocProps->getKeywords());
                    aContent.setPropertyValue( aKeywords, aAny );
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aContent.setPropertyValue( aSubject, Any(xDocProps->getSubject()) );
                }
            }
        }
        catch( Exception& )
        {
        }
    }

    return bOk;
}


// This method contains a call to disable the UNLOCK of a WebDAV resource, that work while saving a file.
// If the method is called from another process (e.g. not when saving a file),
// that disabling needs tweaking
bool SfxObjectShell::DisconnectStorage_Impl( SfxMedium& rSrcMedium, SfxMedium& rTargetMedium )
{
    // this method disconnects the storage from source medium, and attaches it to the backup created by the target medium

    uno::Reference< embed::XStorage > xStorage = rSrcMedium.GetStorage();

    bool bResult = false;
    if ( xStorage == pImpl->m_xDocStorage )
    {
        try
        {
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY_THROW );
            OUString aBackupURL = rTargetMedium.GetBackup_Impl();
            if ( aBackupURL.isEmpty() )
            {
                // the backup could not be created, try to disconnect the storage and close the source SfxMedium
                // in this case the optimization is not possible, connect storage to a temporary file
                rTargetMedium.ResetError();
                xOptStorage->writeAndAttachToStream( uno::Reference< io::XStream >() );
                rSrcMedium.CanDisposeStorage_Impl( false );
                // need to modify this for WebDAV if this method is called outside
                // the process of saving a file
                rSrcMedium.DisableUnlockWebDAV();
                rSrcMedium.Close();
                // see comment on the previous third row
                rSrcMedium.DisableUnlockWebDAV( false );

                // now try to create the backup
                rTargetMedium.GetBackup_Impl();
            }
            else
            {
                // the following call will only compare stream sizes
                // TODO/LATER: this is a very risky part, since if the URL contents are different from the storage
                // contents, the storage will be broken
                xOptStorage->attachToURL( aBackupURL, true );

                // the storage is successfully attached to backup, thus it is owned by the document not by the medium
                rSrcMedium.CanDisposeStorage_Impl( false );
                bResult = true;
            }
        }
        catch ( uno::Exception& )
        {}
    }

    return bResult;
}


bool SfxObjectShell::ConnectTmpStorage_Impl(
    const uno::Reference< embed::XStorage >& xStorage,
    SfxMedium* pMediumArg )

/*   [Description]

     If the application operates on a temporary storage, then it may not take
     the temporary storage from the SaveCompleted. Therefore the new storage
     is connected already here in this case and SaveCompleted then does nothing.
*/

{
    bool bResult = false;

    if ( xStorage.is() )
    {
        try
        {
            // the empty argument means that the storage will create temporary stream itself
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY_THROW );
            xOptStorage->writeAndAttachToStream( uno::Reference< io::XStream >() );

            // the storage is successfully disconnected from the original sources, thus the medium must not dispose it
            if ( pMediumArg )
                pMediumArg->CanDisposeStorage_Impl( false );

            bResult = true;
        }
        catch( uno::Exception& )
        {
        }

        // if switching of the storage does not work for any reason ( nonroot storage for example ) use the old method
        if ( !bResult ) try
        {
            uno::Reference< embed::XStorage > xTmpStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();

            DBG_ASSERT( xTmpStorage.is(), "If a storage can not be created an exception must be thrown!\n" );
            if ( !xTmpStorage.is() )
                throw uno::RuntimeException();

            // TODO/LATER: may be it should be done in SwitchPersistence also
            // TODO/LATER: find faster way to copy storage; perhaps sharing with backup?!
            xStorage->copyToStorage( xTmpStorage );
            bResult = SaveCompleted( xTmpStorage );

            if ( bResult )
            {
                pImpl->aBasicManager.setStorage( xTmpStorage );

                // Get rid of this workaround after issue i113914 is fixed
                try
                {
                    uno::Reference< script::XStorageBasedLibraryContainer > xBasicLibraries( pImpl->xBasicLibraries, uno::UNO_QUERY_THROW );
                    xBasicLibraries->setRootStorage( xTmpStorage );
                }
                catch( uno::Exception& )
                {}
                try
                {
                    uno::Reference< script::XStorageBasedLibraryContainer > xDialogLibraries( pImpl->xDialogLibraries, uno::UNO_QUERY_THROW );
                    xDialogLibraries->setRootStorage( xTmpStorage );
                }
                catch( uno::Exception& )
                {}
            }
        }
        catch( uno::Exception& )
        {}

        if ( !bResult )
        {
            // TODO/LATER: may need error code setting based on exception
            SetError( ERRCODE_IO_GENERAL, OSL_LOG_PREFIX );
        }
    }
    else if (!GetMedium()->GetFilter()->IsOwnFormat())
        bResult = true;

    return bResult;
}


bool SfxObjectShell::DoSaveObjectAs( SfxMedium& rMedium, bool bCommit )
{
    bool bOk = false;

    ModifyBlocker_Impl aBlock( this );

    uno::Reference < embed::XStorage > xNewStor = rMedium.GetStorage();
    if ( !xNewStor.is() )
        return false;

    uno::Reference < beans::XPropertySet > xPropSet( xNewStor, uno::UNO_QUERY );
    if ( !xPropSet.is() )
        return false;

    Any a = xPropSet->getPropertyValue("MediaType");
    OUString aMediaType;
    if ( !(a>>=aMediaType) || aMediaType.isEmpty() )
    {
        SAL_WARN( "sfx.doc", "The mediatype must be set already!" );
        SetupStorage( xNewStor, SOFFICE_FILEFORMAT_CURRENT, false );
    }

    pImpl->bIsSaving = false;
    bOk = SaveAsOwnFormat( rMedium );

    if ( bCommit )
    {
        try {
            uno::Reference< embed::XTransactedObject > xTransact( xNewStor, uno::UNO_QUERY_THROW );
            xTransact->commit();
        }
        catch( uno::Exception& )
        {
            SAL_WARN( "sfx.doc", "The storage was not committed on DoSaveAs!" );
        }
    }

    return bOk;
}


// TODO/LATER: may be the call must be removed completely
bool SfxObjectShell::DoSaveAs( SfxMedium& rMedium )
{
    // here only root storages are included, which are stored via temp file
    rMedium.CreateTempFileNoCopy();
    SetError(rMedium.GetErrorCode(), OSL_LOG_PREFIX );
    if ( GetError() )
        return false;

    // copy version list from "old" medium to target medium, so it can be used on saving
    if ( pImpl->bPreserveVersions )
        rMedium.TransferVersionList_Impl( *pMedium );

    bool bRet = SaveTo_Impl( rMedium, nullptr );
    if ( !bRet )
        SetError(rMedium.GetErrorCode(), OSL_LOG_PREFIX );
    return bRet;
}


bool SfxObjectShell::DoSaveCompleted( SfxMedium* pNewMed, bool bRegisterRecent )
{
    bool bOk = true;
    bool bMedChanged = pNewMed && pNewMed!=pMedium;

    DBG_ASSERT( !pNewMed || pNewMed->GetError() == ERRCODE_NONE, "DoSaveCompleted: Medium has error!" );

    // delete Medium (and Storage!) after all notifications
    SfxMedium* pOld = pMedium;
    if ( bMedChanged )
    {
        pMedium = pNewMed;
        pMedium->CanDisposeStorage_Impl( true );
    }

    std::shared_ptr<const SfxFilter> pFilter = pMedium ? pMedium->GetFilter() : nullptr;
    if ( pNewMed )
    {
        if( bMedChanged )
        {
            if (!pNewMed->GetName().isEmpty())
                bHasName = true;
            Broadcast( SfxHint(SfxHintId::NameChanged) );
            EnableSetModified(false);
            getDocProperties()->setGenerator(
               ::utl::DocInfoHelper::GetGeneratorString() );
            EnableSetModified();
        }

        uno::Reference< embed::XStorage > xStorage;
        if ( !pFilter || IsPackageStorageFormat_Impl( *pMedium ) )
        {
            uno::Reference < embed::XStorage > xOld = GetStorage();

            // when the package based medium is broken and has no storage or if the storage
            // is the same as the document storage the current document storage should be preserved
            xStorage = pMedium->GetStorage();
            bOk = SaveCompleted( xStorage );
            if ( bOk && xStorage.is() && xOld != xStorage
              && (!pOld || !pOld->HasStorage_Impl() || xOld != pOld->GetStorage() ) )
            {
                // old own storage was not controlled by old Medium -> dispose it
                try {
                    xOld->dispose();
                } catch( uno::Exception& )
                {
                    // the storage is disposed already
                    // can happen during reload scenario when the medium has
                    // disposed it during the closing
                    // will be fixed in one of the next milestones
                }
            }
        }
        else
        {
            if (pImpl->m_bSavingForSigning && pFilter && pFilter->GetSupportsSigning())
                // So that pMedium->pImpl->xStream becomes a non-empty
                // reference, and at the end we attempt locking again in
                // SfxMedium::LockOrigFileOnDemand().
                pMedium->GetMedium_Impl();

            if( pMedium->GetOpenMode() & StreamMode::WRITE )
                pMedium->GetInStream();
            xStorage = GetStorage();
        }

        // TODO/LATER: may be this code will be replaced, but not sure
        // Set storage in document library containers
        pImpl->aBasicManager.setStorage( xStorage );

        // Get rid of this workaround after issue i113914 is fixed
        try
        {
            uno::Reference< script::XStorageBasedLibraryContainer > xBasicLibraries( pImpl->xBasicLibraries, uno::UNO_QUERY_THROW );
            xBasicLibraries->setRootStorage( xStorage );
        }
        catch( uno::Exception& )
        {}
        try
        {
            uno::Reference< script::XStorageBasedLibraryContainer > xDialogLibraries( pImpl->xDialogLibraries, uno::UNO_QUERY_THROW );
            xDialogLibraries->setRootStorage( xStorage );
        }
        catch( uno::Exception& )
        {}
    }
    else
    {
        if( pMedium )
        {
            if( pFilter && !IsPackageStorageFormat_Impl( *pMedium ) && (pMedium->GetOpenMode() & StreamMode::WRITE ))
            {
                pMedium->ReOpen();
                bOk = SaveCompletedChildren();
            }
            else
                bOk = SaveCompleted( nullptr );
        }
        // either Save or ConvertTo
        else
            bOk = SaveCompleted( nullptr );
    }

    if ( bOk && pNewMed )
    {
        if( bMedChanged )
        {
            delete pOld;

            uno::Reference< frame::XModel > xModel = GetModel();
            if ( xModel.is() )
            {
                OUString aURL = pNewMed->GetOrigURL();
                uno::Sequence< beans::PropertyValue > aMediaDescr;
                TransformItems( SID_OPENDOC, *pNewMed->GetItemSet(), aMediaDescr );
                try
                {
                    xModel->attachResource( aURL, aMediaDescr );
                }
                catch( uno::Exception& )
                {}
            }

            // before the title regenerated the document must lose the signatures
            pImpl->nDocumentSignatureState = SignatureState::NOSIGNATURES;
            pImpl->nScriptingSignatureState = pNewMed->GetCachedSignatureState_Impl();
            OSL_ENSURE( pImpl->nScriptingSignatureState != SignatureState::BROKEN, "The signature must not be broken at this place" );
            pImpl->bSignatureErrorIsShown = false;

            // TODO/LATER: in future the medium must control own signature state, not the document
            pNewMed->SetCachedSignatureState_Impl( SignatureState::NOSIGNATURES ); // set the default value back

            // Set new title
            if (!pNewMed->GetName().isEmpty() && SfxObjectCreateMode::EMBEDDED != eCreateMode)
                InvalidateName();
            SetModified(false); // reset only by set medium
            Broadcast( SfxHint(SfxHintId::ModeChanged) );

            // this is the end of the saving process, it is possible that
            // the file was changed
            // between medium commit and this step (attributes change and so on)
            // so get the file date again
            if ( pNewMed->DocNeedsFileDateCheck() )
                pNewMed->GetInitFileDate( true );
        }
    }

    pMedium->ClearBackup_Impl();
    pMedium->LockOrigFileOnDemand( true, false );

    if (bRegisterRecent)
        AddToRecentlyUsedList();

    return bOk;
}

void SfxObjectShell::AddToRecentlyUsedList()
{
    INetURLObject aUrl( pMedium->GetOrigURL() );

    if ( aUrl.GetProtocol() == INetProtocol::File )
    {
        std::shared_ptr<const SfxFilter> pOrgFilter = pMedium->GetOrigFilter();
        Application::AddToRecentDocumentList( aUrl.GetURLNoPass( INetURLObject::DecodeMechanism::NONE ),
                                              (pOrgFilter) ? pOrgFilter->GetMimeType() : OUString(),
                                              (pOrgFilter) ? pOrgFilter->GetServiceName() : OUString() );
    }
}


bool SfxObjectShell::ConvertFrom
(
    SfxMedium&  /*rMedium*/     /*  <SfxMedium>, which describes the source file
                                    (for example file name, <SfxFilter>,
                                    Open-Modi and so on) */
)

/*  [Description]

    This method is called for loading of documents over all filters which are
    not SfxFilterFlags::OWN or for which no clipboard format has been registered
    (thus no storage format that is used). In other words, with this method
    it is imported.

    Files which are to be opened here should be opened through 'rMedium'
    to guarantee the right open modes. Especially if the format is retained
    (only possible with SfxFilterFlags::SIMULATE or SfxFilterFlags::ONW) file which must
    be opened STREAM_SHARE_DENYWRITE.

    [Return value]

    bool                true
                        The document could be loaded.

                        false
                        The document could not be loaded, an error code
                        received through  <SvMedium::GetError()const>

    [Example]

    bool DocSh::ConvertFrom( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetInStream();
        if( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream >> ...;

            // Do not call 'rMedium.CloseInStream()'! Keep File locked!
            return SVSTREAM_OK == rMedium.GetError();
        }

        return false;
    }

    [Cross-references]

    <SfxObjectShell::ConvertTo(SfxMedium&)>
    <SfxFilterFlags::REGISTRATION>
*/
{
    return false;
}

bool SfxObjectShell::ImportFrom(SfxMedium& rMedium,
        css::uno::Reference<css::text::XTextRange> const& xInsertPosition)
{
    OUString aFilterName( rMedium.GetFilter()->GetFilterName() );

    uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
    uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );

    uno::Sequence < beans::PropertyValue > aProps;
    uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
    if ( xFilters->hasByName( aFilterName ) )
    {
        xFilters->getByName( aFilterName ) >>= aProps;
        rMedium.GetItemSet()->Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
    }

    OUString aFilterImplName;
    sal_Int32 nFilterProps = aProps.getLength();
    for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
    {
        const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
        if (rFilterProp.Name == "FilterService")
        {
            rFilterProp.Value >>= aFilterImplName;
            break;
        }
    }

    uno::Reference< document::XFilter > xLoader;
    if ( !aFilterImplName.isEmpty() )
    {
        try
        {
            xLoader.set( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
        }
        catch(const uno::Exception&)
        {
            xLoader.clear();
        }
    }
    if ( xLoader.is() )
    {
        // it happens that xLoader does not support xImporter!
        try{
        uno::Reference< lang::XComponent >  xComp( GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< document::XImporter > xImporter( xLoader, uno::UNO_QUERY_THROW );
        xImporter->setTargetDocument( xComp );

        uno::Sequence < beans::PropertyValue > lDescriptor;
        rMedium.GetItemSet()->Put( SfxStringItem( SID_FILE_NAME, rMedium.GetName() ) );
        TransformItems( SID_OPENDOC, *rMedium.GetItemSet(), lDescriptor );

        css::uno::Sequence < css::beans::PropertyValue > aArgs ( lDescriptor.getLength() );
        css::beans::PropertyValue * pNewValue = aArgs.getArray();
        const css::beans::PropertyValue * pOldValue = lDescriptor.getConstArray();
        const OUString sInputStream ( "InputStream"  );

        bool bHasInputStream = false;
        bool bHasBaseURL = false;
        sal_Int32 i;
        sal_Int32 nEnd = lDescriptor.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue [i].Name == sInputStream )
                bHasInputStream = true;
            else if ( pOldValue[i].Name == "DocumentBaseURL" )
                bHasBaseURL = true;
        }

        if ( !bHasInputStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sInputStream;
            aArgs[nEnd-1].Value <<= css::uno::Reference < css::io::XInputStream > ( new utl::OSeekableInputStreamWrapper ( *rMedium.GetInStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = "DocumentBaseURL";
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL();
        }

        if (xInsertPosition.is()) {
            aArgs.realloc( ++nEnd );
            aArgs[nEnd-1].Name = "InsertMode";
            aArgs[nEnd-1].Value <<= true;
            aArgs.realloc( ++nEnd );
            aArgs[nEnd-1].Name = "TextInsertModeRange";
            aArgs[nEnd-1].Value <<= xInsertPosition;
        }

        // #i119492# During loading, some OLE objects like chart will be set
        // modified flag, so needs to reset the flag to false after loading
        bool bRtn = xLoader->filter( aArgs );
        uno::Sequence < OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
        for ( sal_Int32 n = 0; n < aNames.getLength(); ++n )
        {
            OUString aName = aNames[n];
            uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObjectContainer().GetEmbeddedObject( aName );
            OSL_ENSURE( xObj.is(), "An empty entry in the embedded objects list!\n" );
            if ( xObj.is() )
            {
                sal_Int32 nState = xObj->getCurrentState();
                if ( nState == embed::EmbedStates::LOADED || nState == embed::EmbedStates::RUNNING )    // means that the object is not active
                {
                    uno::Reference< util::XModifiable > xModifiable( xObj->getComponent(), uno::UNO_QUERY );
                    if (xModifiable.is() && xModifiable->isModified())
                    {
                        uno::Reference<embed::XEmbedPersist> const xPers(xObj, uno::UNO_QUERY);
                        assert(xPers.is() && "Modified object without persistence!");
                        // store it before resetting modified!
                        xPers->storeOwn();
                        xModifiable->setModified(false);
                    }
                }
            }
        }
        return bRtn;
        }
        catch (const packages::zip::ZipIOException&)
        {
            SetError( ERRCODE_IO_BROKENPACKAGE, "Badness in the underlying package format." );
        }
        catch (const lang::WrappedTargetRuntimeException& rWrapped)
        {
            io::WrongFormatException e;
            if (rWrapped.TargetException >>= e)
            {
                SetError(*new StringErrorInfo(ERRCODE_SFX_FORMAT_ROWCOL,
                    e.Message, ErrorHandlerFlags::ButtonsOk | ErrorHandlerFlags::MessageError ), "");
            }
        }
        catch (...)
        {
            std::abort(); // cannot happen
        }
    }

    return false;
}

bool SfxObjectShell::ExportTo( SfxMedium& rMedium )
{
    OUString aFilterName( rMedium.GetFilter()->GetFilterName() );
    uno::Reference< document::XExporter > xExporter;

    {
        uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( "com.sun.star.document.FilterFactory" ), uno::UNO_QUERY );

        uno::Sequence < beans::PropertyValue > aProps;
        uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
        if ( xFilters->hasByName( aFilterName ) )
            xFilters->getByName( aFilterName ) >>= aProps;

        OUString aFilterImplName;
        sal_Int32 nFilterProps = aProps.getLength();
        for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
        {
            const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
            if (rFilterProp.Name == "FilterService")
            {
                rFilterProp.Value >>= aFilterImplName;
                break;
            }
        }

        if ( !aFilterImplName.isEmpty() )
        {
            try
            {
                xExporter.set( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
            }
            catch(const uno::Exception&)
            {
                xExporter.clear();
            }
        }
    }

    if ( xExporter.is() )
    {
        try{
        uno::Reference< lang::XComponent >  xComp( GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );

        css::uno::Sequence < css::beans::PropertyValue > aOldArgs;
        SfxItemSet* pItems = rMedium.GetItemSet();
        TransformItems( SID_SAVEASDOC, *pItems, aOldArgs );

        const css::beans::PropertyValue * pOldValue = aOldArgs.getConstArray();
        css::uno::Sequence < css::beans::PropertyValue > aArgs ( aOldArgs.getLength() );
        css::beans::PropertyValue * pNewValue = aArgs.getArray();

        // put in the REAL file name, and copy all PropertyValues
        const OUString sOutputStream ( "OutputStream"  );
        const OUString sStream ( "StreamForOutput"  );
        bool bHasOutputStream = false;
        bool bHasStream = false;
        bool bHasBaseURL = false;
        bool bHasFilterName = false;
        sal_Int32 i;
        sal_Int32 nEnd = aOldArgs.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue[i].Name == "FileName" )
                pNewValue[i].Value <<= OUString ( rMedium.GetName() );
            else if ( pOldValue[i].Name == sOutputStream )
                bHasOutputStream = true;
            else if ( pOldValue[i].Name == sStream )
                bHasStream = true;
            else if ( pOldValue[i].Name == "DocumentBaseURL" )
                bHasBaseURL = true;
            else if( pOldValue[i].Name == "FilterName" )
                bHasFilterName = true;
        }

        if ( !bHasOutputStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sOutputStream;
            aArgs[nEnd-1].Value <<= css::uno::Reference < css::io::XOutputStream > ( new utl::OOutputStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        // add stream as well, for OOX export and maybe others
        if ( !bHasStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sStream;
            aArgs[nEnd-1].Value <<= css::uno::Reference < css::io::XStream > ( new utl::OStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = "DocumentBaseURL";
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL( true );
        }

        if( !bHasFilterName )
        {
            aArgs.realloc( ++nEnd );
            aArgs[nEnd-1].Name = "FilterName";
            aArgs[nEnd-1].Value <<= aFilterName;
        }

        return xFilter->filter( aArgs );
        }catch(const uno::Exception&)
        {}
    }

    return false;
}


bool SfxObjectShell::ConvertTo
(
    SfxMedium&  /*rMedium*/   /*  <SfxMedium>, which describes the target file
                                    (for example file name, <SfxFilter>,
                                    Open-Modi and so on) */
)

/*  [Description]

    This method is called for saving of documents over all filters which are
    not SfxFilterFlags::OWN or for which no clipboard format has been registered
    (thus no storage format that is used). In other words, with this method
    it is exported.

    Files which are to be opened here should be opened through 'rMedium'
    to guarantee the right open modes. Especially if the format is retained
    (only possible with SfxFilterFlags::SIMULATE or SfxFilterFlags::ONW) file which must
    be opened STREAM_SHARE_DENYWRITE.

    [Return value]

    bool                true
                        The document could be saved.

                        false
                        The document could not be saved, an error code is
                        received by <SvMedium::GetError()const>


    [Example]

    bool DocSh::ConvertTo( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetOutStream();
        if ( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream << ...;

            rMedium.CloseOutStream(); // opens the InStream automatically
            return SVSTREAM_OK == rMedium.GetError();
        }
        return false ;
    }

    [Cross-references]

    <SfxObjectShell::ConvertFrom(SfxMedium&)>
    <SfxFilterFlags::REGISTRATION>
*/

{
    return false;
}


bool SfxObjectShell::DoSave_Impl( const SfxItemSet* pArgs )
{
    SfxMedium* pRetrMedium = GetMedium();
    std::shared_ptr<const SfxFilter> pFilter = pRetrMedium->GetFilter();

    // copy the original itemset, but remove the "version" item, because pMediumTmp
    // is a new medium "from scratch", so no version should be stored into it
    SfxItemSet* pSet = new SfxAllItemSet(*pRetrMedium->GetItemSet());
    pSet->ClearItem( SID_VERSION );
    pSet->ClearItem( SID_DOC_BASEURL );

    // copy the version comment and major items for the checkin only
    if ( pRetrMedium->IsInCheckIn( ) )
    {
        const SfxPoolItem* pMajor = pArgs->GetItem( SID_DOCINFO_MAJOR );
        if ( pMajor )
            pSet->Put( *pMajor );

        const SfxPoolItem* pComments = pArgs->GetItem( SID_DOCINFO_COMMENTS );
        if ( pComments )
            pSet->Put( *pComments );
    }

    // create a medium as a copy; this medium is only for writing, because it
    // uses the same name as the original one writing is done through a copy,
    // that will be transferred to the target (of course after calling HandsOff)
    SfxMedium* pMediumTmp = new SfxMedium( pRetrMedium->GetName(), pRetrMedium->GetOpenMode(), pFilter, pSet );
    pMediumTmp->SetInCheckIn( pRetrMedium->IsInCheckIn( ) );
    pMediumTmp->SetLongName( pRetrMedium->GetLongName() );
    if ( pMediumTmp->GetErrorCode() != ERRCODE_NONE )
    {
        SetError( pMediumTmp->GetError(), OSL_LOG_PREFIX );
        delete pMediumTmp;
        return false;
    }

    // copy version list from "old" medium to target medium, so it can be used on saving
    pMediumTmp->TransferVersionList_Impl( *pRetrMedium );

    // an interaction handler here can acquire only in case of GUI Saving
    // and should be removed after the saving is done
    css::uno::Reference< XInteractionHandler > xInteract;
    const SfxUnoAnyItem* pxInteractionItem = SfxItemSet::GetItem<SfxUnoAnyItem>(pArgs, SID_INTERACTIONHANDLER, false);
    if ( pxInteractionItem && ( pxInteractionItem->GetValue() >>= xInteract ) && xInteract.is() )
        pMediumTmp->GetItemSet()->Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, makeAny( xInteract ) ) );

    bool bSaved = false;
    if( !GetError() && SaveTo_Impl( *pMediumTmp, pArgs ) )
    {
        bSaved = true;

        if( pMediumTmp->GetItemSet() )
        {
            pMediumTmp->GetItemSet()->ClearItem( SID_INTERACTIONHANDLER );
            pMediumTmp->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        }

        SetError(pMediumTmp->GetErrorCode(), OSL_LOG_PREFIX );

        bool bOpen( false );
        bOpen = DoSaveCompleted( pMediumTmp );

        DBG_ASSERT(bOpen,"Error handling for DoSaveCompleted not implemented");
        (void)bOpen;
    }
    else
    {
        // transfer error code from medium to objectshell
        SetError( pMediumTmp->GetError(), OSL_LOG_PREFIX );

        // reconnect to object storage
        DoSaveCompleted();

        if( pRetrMedium->GetItemSet() )
        {
            pRetrMedium->GetItemSet()->ClearItem( SID_INTERACTIONHANDLER );
            pRetrMedium->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        }

        delete pMediumTmp;
    }

    SetModified( !bSaved );
    return bSaved;
}


bool SfxObjectShell::Save_Impl( const SfxItemSet* pSet )
{
    if ( IsReadOnly() )
    {
        SetError( ERRCODE_SFX_DOCUMENTREADONLY, OSL_LOG_PREFIX );
        return false;
    }


    pImpl->bIsSaving = true;
    bool bSaved = false;
    const SfxStringItem* pSalvageItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_DOC_SALVAGE, false);
    if ( pSalvageItem )
    {
        const SfxStringItem* pFilterItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_FILTER_NAME, false);
        OUString aFilterName;
        std::shared_ptr<const SfxFilter> pFilter;
        if ( pFilterItem )
            pFilter = SfxFilterMatcher( OUString::createFromAscii( GetFactory().GetShortName()) ).GetFilter4FilterName( aFilterName );

        SfxMedium *pMed = new SfxMedium(
            pSalvageItem->GetValue(), StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC, pFilter );

        const SfxStringItem* pPasswordItem = SfxItemSet::GetItem<SfxStringItem>(GetMedium()->GetItemSet(), SID_PASSWORD, false);
        if ( pPasswordItem )
            pMed->GetItemSet()->Put( *pPasswordItem );

        bSaved = DoSaveAs( *pMed );
        if ( bSaved )
            bSaved = DoSaveCompleted( pMed );
        else
            delete pMed;
    }
    else
        bSaved = DoSave_Impl( pSet );
    return bSaved;
}

bool SfxObjectShell::CommonSaveAs_Impl(const INetURLObject& aURL, const OUString& aFilterName, SfxItemSet& rItemSet)
{
    if( aURL.HasError() )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER, OSL_LOG_PREFIX );
        return false;
    }

    if ( aURL != INetURLObject( OUString( "private:stream"  ) ) )
    {
        // Is there already a Document with this name?
        SfxObjectShell* pDoc = nullptr;
        for ( SfxObjectShell* pTmp = SfxObjectShell::GetFirst();
                pTmp && !pDoc;
                pTmp = SfxObjectShell::GetNext(*pTmp) )
        {
            if( ( pTmp != this ) && pTmp->GetMedium() )
            {
                INetURLObject aCompare( pTmp->GetMedium()->GetName() );
                if ( aCompare == aURL )
                    pDoc = pTmp;
            }
        }
        if ( pDoc )
        {
            // Then error message: "already opened"
            SetError(ERRCODE_SFX_ALREADYOPEN, OSL_LOG_PREFIX);
            return false;
        }
    }

    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "Illegal URL!" );
    DBG_ASSERT( rItemSet.Count() != 0, "Incorrect Parameter");

    const SfxBoolItem* pSaveToItem = rItemSet.GetItem<SfxBoolItem>(SID_SAVETO, false);
    bool bSaveTo = pSaveToItem && pSaveToItem->GetValue();

    std::shared_ptr<const SfxFilter> pFilter = GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName );
    if ( !pFilter
        || !pFilter->CanExport()
        || (!bSaveTo && !pFilter->CanImport()) )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER, OSL_LOG_PREFIX );
        return false;
    }

    const SfxBoolItem* pCopyStreamItem = rItemSet.GetItem<SfxBoolItem>(SID_COPY_STREAM_IF_POSSIBLE, false);
    if ( bSaveTo && pCopyStreamItem && pCopyStreamItem->GetValue() && !IsModified() )
    {
        if (pMedium->TryDirectTransfer(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), rItemSet))
            return true;
    }
    rItemSet.ClearItem( SID_COPY_STREAM_IF_POSSIBLE );

    pImpl->bPasswd = SfxItemState::SET == rItemSet.GetItemState(SID_PASSWORD);

    SfxMedium *pActMed = GetMedium();
    const INetURLObject aActName(pActMed->GetName());

    bool bWasReadonly = IsReadOnly();

    if ( aURL == aActName && aURL != INetURLObject( OUString("private:stream") )
        && IsReadOnly() )
    {
        SetError(ERRCODE_SFX_DOCUMENTREADONLY, OSL_LOG_PREFIX);
        return false;
    }

    if (SfxItemState::SET != rItemSet.GetItemState(SID_UNPACK) && SvtSaveOptions().IsSaveUnpacked())
        rItemSet.Put(SfxBoolItem(SID_UNPACK, false));

    OUString aTempFileURL;
    if ( IsDocShared() )
        aTempFileURL = pMedium->GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE );

    if (PreDoSaveAs_Impl(aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), aFilterName, rItemSet))
    {
        // Update Data on media
        SfxItemSet *pSet = GetMedium()->GetItemSet();
        pSet->ClearItem( SID_INTERACTIONHANDLER );
        pSet->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        pSet->ClearItem( SID_STANDARD_DIR );
        pSet->ClearItem( SID_PATH );

        if ( !bSaveTo )
        {
            pSet->ClearItem( SID_REFERER );
            pSet->ClearItem( SID_POSTDATA );
            pSet->ClearItem( SID_TEMPLATE );
            pSet->ClearItem( SID_DOC_READONLY );
            pSet->ClearItem( SID_CONTENTTYPE );
            pSet->ClearItem( SID_CHARSET );
            pSet->ClearItem( SID_FILTER_NAME );
            pSet->ClearItem( SID_OPTIONS );
            pSet->ClearItem( SID_VERSION );
            pSet->ClearItem( SID_EDITDOC );
            pSet->ClearItem( SID_OVERWRITE );
            pSet->ClearItem( SID_DEFAULTFILEPATH );
            pSet->ClearItem( SID_DEFAULTFILENAME );

            const SfxStringItem* pFilterItem = rItemSet.GetItem<SfxStringItem>(SID_FILTER_NAME, false);
            if ( pFilterItem )
                pSet->Put( *pFilterItem );

            const SfxStringItem* pOptionsItem = rItemSet.GetItem<SfxStringItem>(SID_OPTIONS, false);
            if ( pOptionsItem )
                pSet->Put( *pOptionsItem );

            const SfxStringItem* pFilterOptItem = rItemSet.GetItem<SfxStringItem>(SID_FILE_FILTEROPTIONS, false);
            if ( pFilterOptItem )
                pSet->Put( *pFilterOptItem );

#if HAVE_FEATURE_MULTIUSER_ENVIRONMENT
            if ( IsDocShared() && !aTempFileURL.isEmpty() )
            {
                // this is a shared document that has to be disconnected from the old location
                FreeSharedFile( aTempFileURL );

                if ( pFilter->IsOwnFormat()
                  && pFilter->UsesStorage()
                  && pFilter->GetVersion() >= SOFFICE_FILEFORMAT_60 )
                {
                    // the target format is the own format
                    // the target document must be shared
                    SwitchToShared( true, false );
                }
            }
#endif
        }

        if ( bWasReadonly && !bSaveTo )
            Broadcast( SfxHint(SfxHintId::ModeChanged) );

        return true;
    }
    else
        return false;
}

bool SfxObjectShell::PreDoSaveAs_Impl(const OUString& rFileName, const OUString& aFilterName, SfxItemSet& rItemSet)
{
    // copy all items stored in the itemset of the current medium
    SfxAllItemSet* pMergedParams = new SfxAllItemSet( *pMedium->GetItemSet() );

    // in "SaveAs" title and password will be cleared ( maybe the new itemset contains new values, otherwise they will be empty )
    pMergedParams->ClearItem( SID_ENCRYPTIONDATA );
    pMergedParams->ClearItem( SID_PASSWORD );
    // #i119366# - As the SID_ENCRYPTIONDATA and SID_PASSWORD are using for setting password together, we need to clear them both.
    // Also, ( maybe the new itemset contains new values, otherwise they will be empty )
    pMergedParams->ClearItem( SID_ENCRYPTIONDATA );
    pMergedParams->ClearItem( SID_DOCINFO_TITLE );

    pMergedParams->ClearItem( SID_INPUTSTREAM );
    pMergedParams->ClearItem( SID_STREAM );
    pMergedParams->ClearItem( SID_CONTENT );
    pMergedParams->ClearItem( SID_DOC_READONLY );
    pMergedParams->ClearItem( SID_DOC_BASEURL );

    pMergedParams->ClearItem( SID_REPAIRPACKAGE );

    // "SaveAs" will never store any version information - it's a complete new file !
    pMergedParams->ClearItem( SID_VERSION );

    // merge the new parameters into the copy
    // all values present in both itemsets will be overwritten by the new parameters
    pMergedParams->Put(rItemSet);

    SAL_WARN_IF( pMergedParams->GetItemState( SID_DOC_SALVAGE) >= SfxItemState::SET,
        "sfx.doc","Salvage item present in Itemset, check the parameters!");

    // should be unnecessary - too hot to handle!
    pMergedParams->ClearItem( SID_DOC_SALVAGE );

    // create a medium for the target URL
    SfxMedium *pNewFile = new SfxMedium( rFileName, StreamMode::READWRITE | StreamMode::SHARE_DENYWRITE | StreamMode::TRUNC, nullptr, pMergedParams );

    // set filter; if no filter is given, take the default filter of the factory
    if ( !aFilterName.isEmpty() )
        pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName ) );
    else
        pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetAnyFilter( SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT ) );

    if ( pNewFile->GetErrorCode() != ERRCODE_NONE )
    {
        // creating temporary file failed ( f.e. floppy disk not inserted! )
        SetError( pNewFile->GetError(), OSL_LOG_PREFIX );
        delete pNewFile;
        return false;
    }

    // check if a "SaveTo" is wanted, no "SaveAs"
    const SfxBoolItem* pSaveToItem = pMergedParams->GetItem<SfxBoolItem>(SID_SAVETO, false);
    bool bCopyTo = GetCreateMode() == SfxObjectCreateMode::EMBEDDED || (pSaveToItem && pSaveToItem->GetValue());

    // distinguish between "Save" and "SaveAs"
    pImpl->bIsSaving = false;

    // copy version list from "old" medium to target medium, so it can be used on saving
    if ( pImpl->bPreserveVersions )
        pNewFile->TransferVersionList_Impl( *pMedium );

    // Save the document ( first as temporary file, then transfer to the target URL by committing the medium )
    bool bOk = false;
    if ( !pNewFile->GetErrorCode() && SaveTo_Impl( *pNewFile, nullptr ) )
    {
        // transfer a possible error from the medium to the document
        SetError( pNewFile->GetErrorCode(), OSL_LOG_PREFIX );

        // notify the document that saving was done successfully
        if ( !bCopyTo )
        {
            bOk = DoSaveCompleted( pNewFile );
        }
        else
            bOk = DoSaveCompleted();

        if( bOk )
        {
            if( !bCopyTo )
                SetModified( false );
        }
        else
        {
            // TODO/LATER: the code below must be dead since the storage commit makes all the stuff
            //       and the DoSaveCompleted call should not be able to fail in general

            DBG_ASSERT( !bCopyTo, "Error while reconnecting to medium, can't be handled!");
            SetError( pNewFile->GetErrorCode(), OSL_LOG_PREFIX );

            if ( !bCopyTo )
            {
                // reconnect to the old medium
                bool bRet( false );
                bRet = DoSaveCompleted( pMedium );
                DBG_ASSERT( bRet, "Error in DoSaveCompleted, can't be handled!");
                (void)bRet;
            }

            // TODO/LATER: disconnect the new file from the storage for the case when pure saving is done
            //       if storing has corrupted the file, probably it must be restored either here or
            //       by the storage
            DELETEZ( pNewFile );
        }
    }
    else
    {
        SetError( pNewFile->GetErrorCode(), OSL_LOG_PREFIX );

        // reconnect to the old storage
        DoSaveCompleted();

        DELETEZ( pNewFile );
    }

    if ( bCopyTo )
        DELETEZ( pNewFile );
    else if( !bOk )
        SetModified();

    return bOk;
}


bool SfxObjectShell::LoadFrom( SfxMedium& /*rMedium*/ )
{
    SAL_WARN( "sfx.doc", "Base implementation, must not be called in general!" );
    return true;
}


bool SfxObjectShell::CanReload_Impl()

/*  [Description]

    Internal method for determining whether a reload of the document
    (as RevertToSaved or last known version) is possible.
*/

{
    return pMedium && HasName() && !IsInModalMode() && !pImpl->bForbidReload;
}


HiddenInformation SfxObjectShell::GetHiddenInformationState( HiddenInformation nStates )
{
    HiddenInformation nState = HiddenInformation::NONE;
    if ( nStates & HiddenInformation::DOCUMENTVERSIONS )
    {
        if ( GetMedium()->GetVersionList().getLength() )
            nState |= HiddenInformation::DOCUMENTVERSIONS;
    }

    return nState;
}

sal_Int16 SfxObjectShell::QueryHiddenInformation( HiddenWarningFact eFact, vcl::Window* pParent )
{
    sal_Int16 nRet = RET_YES;
    sal_uInt16 nResId = sal_uInt16();
    SvtSecurityOptions::EOption eOption = SvtSecurityOptions::EOption();

    switch ( eFact )
    {
        case HiddenWarningFact::WhenSaving :
        {
            nResId = STR_HIDDENINFO_CONTINUE_SAVING;
            eOption = SvtSecurityOptions::E_DOCWARN_SAVEORSEND;
            break;
        }
        case HiddenWarningFact::WhenPrinting :
        {
            nResId = STR_HIDDENINFO_CONTINUE_PRINTING;
            eOption = SvtSecurityOptions::E_DOCWARN_PRINT;
            break;
        }
        case HiddenWarningFact::WhenSigning :
        {
            nResId = STR_HIDDENINFO_CONTINUE_SIGNING;
            eOption = SvtSecurityOptions::E_DOCWARN_SIGNING;
            break;
        }
        case HiddenWarningFact::WhenCreatingPDF :
        {
            nResId = STR_HIDDENINFO_CONTINUE_CREATEPDF;
            eOption = SvtSecurityOptions::E_DOCWARN_CREATEPDF;
            break;
        }
        default:
            assert(false); // this cannot happen
    }

    if ( SvtSecurityOptions().IsOptionSet( eOption ) )
    {
        OUString sMessage( SfxResId(STR_HIDDENINFO_CONTAINS).toString() );
        HiddenInformation nWantedStates = HiddenInformation::RECORDEDCHANGES | HiddenInformation::NOTES;
        if ( eFact != HiddenWarningFact::WhenPrinting )
            nWantedStates |= HiddenInformation::DOCUMENTVERSIONS;
        HiddenInformation nStates = GetHiddenInformationState( nWantedStates );
        bool bWarning = false;

        if ( nStates & HiddenInformation::RECORDEDCHANGES )
        {
            sMessage += SfxResId(STR_HIDDENINFO_RECORDCHANGES).toString();
            sMessage += "\n";
            bWarning = true;
        }
        if ( nStates & HiddenInformation::NOTES )
        {
            sMessage += SfxResId(STR_HIDDENINFO_NOTES).toString();
            sMessage += "\n";
            bWarning = true;
        }
        if ( nStates & HiddenInformation::DOCUMENTVERSIONS )
        {
            sMessage += SfxResId(STR_HIDDENINFO_DOCVERSIONS).toString();
            sMessage += "\n";
            bWarning = true;
        }

        if ( bWarning )
        {
            sMessage += "\n";
            sMessage += SfxResId(nResId).toString();
            ScopedVclPtrInstance< WarningBox > aWBox(pParent, WB_YES_NO | WB_DEF_NO, sMessage);
            nRet = aWBox->Execute();
        }
    }

    return nRet;
}

bool SfxObjectShell::IsSecurityOptOpenReadOnly() const
{
    return IsLoadReadonly();
}

void SfxObjectShell::SetSecurityOptOpenReadOnly( bool _b )
{
    SetLoadReadonly( _b );
}

bool SfxObjectShell::LoadOwnFormat( SfxMedium& rMedium )
{
    SAL_INFO( "sfx.doc", "loading \" " << rMedium.GetName() << "\"" );

    uno::Reference< embed::XStorage > xStorage = rMedium.GetStorage();
    if ( xStorage.is() )
    {
        // Password
        const SfxStringItem* pPasswdItem = SfxItemSet::GetItem<SfxStringItem>(rMedium.GetItemSet(), SID_PASSWORD, false);
        if ( pPasswdItem || ERRCODE_IO_ABORT != CheckPasswd_Impl( this, SfxGetpApp()->GetPool(), pMedium ) )
        {
            uno::Sequence< beans::NamedValue > aEncryptionData;
            if ( GetEncryptionData_Impl(pMedium->GetItemSet(), aEncryptionData) )
            {
                try
                {
                    // the following code must throw an exception in case of failure
                    ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( xStorage, aEncryptionData );
                }
                catch( uno::Exception& )
                {
                    // TODO/LATER: handle the error code
                }
            }

            // load document
            return Load( rMedium );
        }
        return false;
    }
    else
        return false;
}

bool SfxObjectShell::SaveAsOwnFormat( SfxMedium& rMedium )
{
    uno::Reference< embed::XStorage > xStorage = rMedium.GetStorage();
    if( xStorage.is() )
    {
        sal_Int32 nVersion = rMedium.GetFilter()->GetVersion();

        // OASIS templates have own mediatypes ( SO7 also actually, but it is to late to use them here )
        const bool bTemplate = rMedium.GetFilter()->IsOwnTemplateFormat()
            && nVersion > SOFFICE_FILEFORMAT_60;

        std::shared_ptr<const SfxFilter> pFilter = rMedium.GetFilter();
        bool bChart = false;
        if(pFilter->GetName() == "chart8")
            bChart = true;

        SetupStorage( xStorage, nVersion, bTemplate, bChart );
#if HAVE_FEATURE_SCRIPTING
        if ( HasBasic() )
        {
            // Initialize Basic
            GetBasicManager();

            // Save dialog/script container
            pImpl->aBasicManager.storeLibrariesToStorage( xStorage );
        }
#endif
        return SaveAs( rMedium );
    }
    else return false;
}

uno::Reference< embed::XStorage > SfxObjectShell::GetStorage()
{
    if ( !pImpl->m_xDocStorage.is() )
    {
        OSL_ENSURE( pImpl->m_bCreateTempStor, "The storage must exist already!\n" );
        try {
            // no notification is required the storage is set the first time
            pImpl->m_xDocStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
            OSL_ENSURE( pImpl->m_xDocStorage.is(), "The method must either return storage or throw an exception!" );

            SetupStorage( pImpl->m_xDocStorage, SOFFICE_FILEFORMAT_CURRENT, false );
            pImpl->m_bCreateTempStor = false;
            if (!utl::ConfigManager::IsAvoidConfig())
                SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::StorageChanged, GlobalEventConfig::GetEventName(GlobalEventId::STORAGECHANGED), this ) );
        }
        catch( uno::Exception& )
        {
            // TODO/LATER: error handling?
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    OSL_ENSURE( pImpl->m_xDocStorage.is(), "The document storage must be created!" );
    return pImpl->m_xDocStorage;
}


bool SfxObjectShell::SaveChildren( bool bObjectsOnly )
{
    bool bResult = true;
    if ( pImpl->mpObjectContainer )
    {
        bool bOasis = ( SotStorage::GetVersion( GetStorage() ) > SOFFICE_FILEFORMAT_60 );
        GetEmbeddedObjectContainer().StoreChildren(bOasis,bObjectsOnly);
    }

    return bResult;
}

bool SfxObjectShell::SaveAsChildren( SfxMedium& rMedium )
{
    bool bResult = true;

    uno::Reference < embed::XStorage > xStorage = rMedium.GetStorage();
    if ( !xStorage.is() )
        return false;

    if ( xStorage == GetStorage() )
        return SaveChildren();

    if ( pImpl->mpObjectContainer )
    {
        bool bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
        GetEmbeddedObjectContainer().StoreAsChildren(bOasis,SfxObjectCreateMode::EMBEDDED == eCreateMode,xStorage);
    }

    if ( bResult )
        bResult = CopyStoragesOfUnknownMediaType( GetStorage(), xStorage );

    return bResult;
}

bool SfxObjectShell::SaveCompletedChildren()
{
    bool bResult = true;

    if ( pImpl->mpObjectContainer )
    {
        uno::Sequence < OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
        for ( sal_Int32 n=0; n<aNames.getLength(); n++ )
        {
            uno::Reference < embed::XEmbeddedObject > xObj = GetEmbeddedObjectContainer().GetEmbeddedObject( aNames[n] );
            OSL_ENSURE( xObj.is(), "An empty entry in the embedded objects list!\n" );
            if ( xObj.is() )
            {
                uno::Reference< embed::XEmbedPersist > xPersist( xObj, uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    try
                    {
                        xPersist->saveCompleted( false/*bSuccess*/ );
                    }
                    catch( uno::Exception& )
                    {
                        // TODO/LATER: error handling
                        bResult = false;
                        break;
                    }
                }
            }
        }
    }

    return bResult;
}

bool SfxObjectShell::SwitchChildrenPersistance( const uno::Reference< embed::XStorage >& xStorage,
                                                    bool bForceNonModified )
{
    if ( !xStorage.is() )
    {
        // TODO/LATER: error handling
        return false;
    }

    bool bResult = true;

    if ( pImpl->mpObjectContainer )
        pImpl->mpObjectContainer->SetPersistentEntries(xStorage,bForceNonModified);

    return bResult;
}

// Never call this method directly, always use the DoSaveCompleted call
bool SfxObjectShell::SaveCompleted( const uno::Reference< embed::XStorage >& xStorage )
{
    bool bResult = false;
    bool bSendNotification = false;
    uno::Reference< embed::XStorage > xOldStorageHolder;

    // check for wrong creation of object container
    bool bHasContainer = ( pImpl->mpObjectContainer != nullptr );

    if ( !xStorage.is() || xStorage == GetStorage() )
    {
        // no persistence change
        bResult = SaveCompletedChildren();
    }
    else
    {
        if ( pImpl->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( xStorage );

        bResult = SwitchChildrenPersistance( xStorage, true );
    }

    if ( bResult )
    {
        if ( xStorage.is() && pImpl->m_xDocStorage != xStorage )
        {
            // make sure that until the storage is assigned the object
            // container is not created by accident!
            DBG_ASSERT( bHasContainer == (pImpl->mpObjectContainer != nullptr), "Wrong storage in object container!" );
            xOldStorageHolder = pImpl->m_xDocStorage;
            pImpl->m_xDocStorage = xStorage;
            bSendNotification = true;

            if ( IsEnableSetModified() )
                SetModified( false );
        }
    }
    else
    {
        if ( pImpl->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( pImpl->m_xDocStorage );

        // let already successfully connected objects be switched back
        SwitchChildrenPersistance( pImpl->m_xDocStorage, true );
    }

    if ( bSendNotification )
    {
        SfxGetpApp()->NotifyEvent( SfxEventHint( SfxEventHintId::StorageChanged, GlobalEventConfig::GetEventName(GlobalEventId::STORAGECHANGED), this ) );
    }

    return bResult;
}

bool StoragesOfUnknownMediaTypeAreCopied_Impl( const uno::Reference< embed::XStorage >& xSource,
                                                   const uno::Reference< embed::XStorage >& xTarget )
{
    OSL_ENSURE( xSource.is() && xTarget.is(), "Source and/or target storages are not available!\n" );
    if ( !xSource.is() || !xTarget.is() || xSource == xTarget )
        return true;

    try
    {
        uno::Sequence< OUString > aSubElements = xSource->getElementNames();
        for ( sal_Int32 nInd = 0; nInd < aSubElements.getLength(); nInd++ )
        {
            if ( xSource->isStorageElement( aSubElements[nInd] ) )
            {
                OUString aMediaType;
                OUString aMediaTypePropName( "MediaType"  );
                bool bGotMediaType = false;

                try
                {
                    uno::Reference< embed::XOptimizedStorage > xOptStorage( xSource, uno::UNO_QUERY_THROW );
                    bGotMediaType =
                        ( xOptStorage->getElementPropertyValue( aSubElements[nInd], aMediaTypePropName ) >>= aMediaType );
                }
                catch( uno::Exception& )
                {}

                if ( !bGotMediaType )
                {
                    uno::Reference< embed::XStorage > xSubStorage;
                    try {
                        xSubStorage = xSource->openStorageElement( aSubElements[nInd], embed::ElementModes::READ );
                    } catch( uno::Exception& )
                    {}

                    if ( !xSubStorage.is() )
                    {
                        xSubStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        xSource->copyStorageElementLastCommitTo( aSubElements[nInd], xSubStorage );
                    }

                    uno::Reference< beans::XPropertySet > xProps( xSubStorage, uno::UNO_QUERY_THROW );
                    xProps->getPropertyValue( aMediaTypePropName ) >>= aMediaType;
                }

                // TODO/LATER: there should be a way to detect whether an object with such a MediaType can exist
                //             probably it should be placed in the MimeType-ClassID table or in standalone table
                if ( !aMediaType.isEmpty()
                  && aMediaType != "application/vnd.sun.star.oleobject" )
                {
                    css::datatransfer::DataFlavor aDataFlavor;
                    aDataFlavor.MimeType = aMediaType;
                    SotClipboardFormatId nFormat = SotExchange::GetFormat( aDataFlavor );

                    switch ( nFormat )
                    {
                        case SotClipboardFormatId::STARWRITER_60 :
                        case SotClipboardFormatId::STARWRITERWEB_60 :
                        case SotClipboardFormatId::STARWRITERGLOB_60 :
                        case SotClipboardFormatId::STARDRAW_60 :
                        case SotClipboardFormatId::STARIMPRESS_60 :
                        case SotClipboardFormatId::STARCALC_60 :
                        case SotClipboardFormatId::STARCHART_60 :
                        case SotClipboardFormatId::STARMATH_60 :
                        case SotClipboardFormatId::STARWRITER_8:
                        case SotClipboardFormatId::STARWRITERWEB_8:
                        case SotClipboardFormatId::STARWRITERGLOB_8:
                        case SotClipboardFormatId::STARDRAW_8:
                        case SotClipboardFormatId::STARIMPRESS_8:
                        case SotClipboardFormatId::STARCALC_8:
                        case SotClipboardFormatId::STARCHART_8:
                        case SotClipboardFormatId::STARMATH_8:
                            break;

                        default:
                        {
                            if ( !xTarget->hasByName( aSubElements[nInd] ) )
                                return false;
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {
        SAL_WARN( "sfx.doc", "Can not check storage consistency!" );
    }

    return true;
}

bool SfxObjectShell::SwitchPersistance( const uno::Reference< embed::XStorage >& xStorage )
{
    bool bResult = false;
    // check for wrong creation of object container
    bool bHasContainer = ( pImpl->mpObjectContainer != nullptr );
    if ( xStorage.is() )
    {
        if ( pImpl->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( xStorage );
        bResult = SwitchChildrenPersistance( xStorage );

        // TODO/LATER: substorages that have unknown mimetypes probably should be copied to the target storage here
        OSL_ENSURE( StoragesOfUnknownMediaTypeAreCopied_Impl( pImpl->m_xDocStorage, xStorage ),
                    "Some of substorages with unknown mimetypes is lost!" );
    }

    if ( bResult )
    {
        // make sure that until the storage is assigned the object container is not created by accident!
        DBG_ASSERT( bHasContainer == (pImpl->mpObjectContainer != nullptr), "Wrong storage in object container!" );
        if ( pImpl->m_xDocStorage != xStorage )
            DoSaveCompleted( new SfxMedium( xStorage, GetMedium()->GetBaseURL() ) );

        if ( IsEnableSetModified() )
            SetModified(); // ???
    }

    return bResult;
}

bool SfxObjectShell::CopyStoragesOfUnknownMediaType( const uno::Reference< embed::XStorage >& xSource,
                                                         const uno::Reference< embed::XStorage >& xTarget )
{
    // This method does not commit the target storage and should not do it
    bool bResult = true;

    try
    {
        uno::Sequence< OUString > aSubElements = xSource->getElementNames();
        for ( sal_Int32 nInd = 0; nInd < aSubElements.getLength(); nInd++ )
        {
            if ( aSubElements[nInd] == "Configurations" )
            {
                // The workaround for compatibility with SO7, "Configurations" substorage must be preserved
                if ( xSource->isStorageElement( aSubElements[nInd] ) )
                {
                    OSL_ENSURE( !xTarget->hasByName( aSubElements[nInd] ),
                                "The target storage is an output storage, the element should not exist in the target!\n" );

                    xSource->copyElementTo( aSubElements[nInd], xTarget, aSubElements[nInd] );
                }
            }
            else if ( xSource->isStorageElement( aSubElements[nInd] ) )
            {
                OUString aMediaType;
                OUString aMediaTypePropName( "MediaType"  );
                bool bGotMediaType = false;

                try
                {
                    uno::Reference< embed::XOptimizedStorage > xOptStorage( xSource, uno::UNO_QUERY_THROW );
                    bGotMediaType =
                        ( xOptStorage->getElementPropertyValue( aSubElements[nInd], aMediaTypePropName ) >>= aMediaType );
                }
                catch( uno::Exception& )
                {}

                if ( !bGotMediaType )
                {
                    uno::Reference< embed::XStorage > xSubStorage;
                    try {
                        xSubStorage = xSource->openStorageElement( aSubElements[nInd], embed::ElementModes::READ );
                    } catch( uno::Exception& )
                    {}

                    if ( !xSubStorage.is() )
                    {
                        // TODO/LATER: as optimization in future a substorage of target storage could be used
                        //             instead of the temporary storage; this substorage should be removed later
                        //             if the MimeType is wrong
                        xSubStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        xSource->copyStorageElementLastCommitTo( aSubElements[nInd], xSubStorage );
                    }

                    uno::Reference< beans::XPropertySet > xProps( xSubStorage, uno::UNO_QUERY_THROW );
                    xProps->getPropertyValue( aMediaTypePropName ) >>= aMediaType;
                }

                // TODO/LATER: there should be a way to detect whether an object with such a MediaType can exist
                //             probably it should be placed in the MimeType-ClassID table or in standalone table
                if ( !aMediaType.isEmpty()
                  && aMediaType != "application/vnd.sun.star.oleobject" )
                {
                    css::datatransfer::DataFlavor aDataFlavor;
                    aDataFlavor.MimeType = aMediaType;
                    SotClipboardFormatId nFormat = SotExchange::GetFormat( aDataFlavor );

                    switch ( nFormat )
                    {
                        case SotClipboardFormatId::STARWRITER_60 :
                        case SotClipboardFormatId::STARWRITERWEB_60 :
                        case SotClipboardFormatId::STARWRITERGLOB_60 :
                        case SotClipboardFormatId::STARDRAW_60 :
                        case SotClipboardFormatId::STARIMPRESS_60 :
                        case SotClipboardFormatId::STARCALC_60 :
                        case SotClipboardFormatId::STARCHART_60 :
                        case SotClipboardFormatId::STARMATH_60 :
                        case SotClipboardFormatId::STARWRITER_8:
                        case SotClipboardFormatId::STARWRITERWEB_8:
                        case SotClipboardFormatId::STARWRITERGLOB_8:
                        case SotClipboardFormatId::STARDRAW_8:
                        case SotClipboardFormatId::STARIMPRESS_8:
                        case SotClipboardFormatId::STARCALC_8:
                        case SotClipboardFormatId::STARCHART_8:
                        case SotClipboardFormatId::STARMATH_8:
                            break;

                        default:
                        {
                            OSL_ENSURE( aSubElements[nInd] == "Configurations2" || nFormat == SotClipboardFormatId::STARBASE_8 || !xTarget->hasByName( aSubElements[nInd] ),
                                        "The target storage is an output storage, the element should not exist in the target!\n" );

                            if ( !xTarget->hasByName( aSubElements[nInd] ) )
                            {
                                xSource->copyElementTo( aSubElements[nInd], xTarget, aSubElements[nInd] );
                            }
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {
        bResult = false;
        // TODO/LATER: a specific error could be provided
    }

    return bResult;
}

bool SfxObjectShell::GenerateAndStoreThumbnail(bool bEncrypted, bool bIsTemplate, const uno::Reference<embed::XStorage>& xStorage)
{
    //optimize thumbnail generate and store procedure to improve odt saving performance, i120030
    bIsInGenerateThumbnail = true;

    bool bResult = false;

    try
    {
        uno::Reference<embed::XStorage> xThumbnailStorage = xStorage->openStorageElement("Thumbnails", embed::ElementModes::READWRITE);

        if (xThumbnailStorage.is())
        {
            uno::Reference<io::XStream> xStream = xThumbnailStorage->openStreamElement("thumbnail.png", embed::ElementModes::READWRITE);

            if (xStream.is() && WriteThumbnail(bEncrypted, bIsTemplate, xStream))
            {
                uno::Reference<embed::XTransactedObject> xTransactedObject(xThumbnailStorage, uno::UNO_QUERY_THROW);
                xTransactedObject->commit();
                bResult = true;
            }
        }
    }
    catch( uno::Exception& )
    {
    }

    //optimize thumbnail generate and store procedure to improve odt saving performance, i120030
    bIsInGenerateThumbnail = false;

    return bResult;
}

bool SfxObjectShell::WriteThumbnail(bool bEncrypted, bool bIsTemplate, const uno::Reference<io::XStream>& xStream)
{
    bool bResult = false;

    if (!xStream.is())
        return false;

    try
    {
        uno::Reference<io::XTruncate> xTruncate(xStream->getOutputStream(), uno::UNO_QUERY_THROW);
        xTruncate->truncate();

        uno::Reference <beans::XPropertySet> xSet(xStream, uno::UNO_QUERY);
        if (xSet.is())
            xSet->setPropertyValue("MediaType", uno::makeAny(OUString("image/png")));
        if (bEncrypted)
        {
            OUString sFactoryName = OUString::createFromAscii(GetFactory().GetShortName());
            sal_uInt16 nResID = GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl(sFactoryName, bIsTemplate);
            if (nResID)
                bResult = GraphicHelper::getThumbnailReplacement_Impl(nResID, xStream);
        }
        else
        {
            std::shared_ptr<GDIMetaFile> xMetaFile = GetPreviewMetaFile();
            if (xMetaFile)
            {
                bResult = GraphicHelper::getThumbnailFormatFromGDI_Impl(xMetaFile.get(), xStream);
            }
        }
    }
    catch(uno::Exception&)
    {}

    return bResult;
}

void SfxObjectShell::UpdateLinks()
{
}

bool SfxObjectShell::LoadExternal( SfxMedium& )
{
    // Not implemented. It's an error if the code path ever comes here.
    assert(false);
    return false;
}

bool SfxObjectShell::InsertGeneratedStream(SfxMedium&,
        uno::Reference<text::XTextRange> const&)
{
    // Not implemented. It's an error if the code path ever comes here.
    assert(false);
    return false;
}

void SfxObjectShell::CheckConfigOptions()
{
    // not handled.  Each app's shell needs to overwrite this method to add handler.
    SetConfigOptionsChecked(true);
}

bool SfxObjectShell::IsConfigOptionsChecked() const
{
    return pImpl->m_bConfigOptionsChecked;
}

void SfxObjectShell::SetConfigOptionsChecked( bool bChecked )
{
    pImpl->m_bConfigOptionsChecked = bChecked;
}

bool SfxObjectShell::QuerySaveSizeExceededModules_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
#if !HAVE_FEATURE_SCRIPTING
    (void) xHandler;
#else
    if ( !HasBasic() )
        return true;

    if ( !pImpl->aBasicManager.isValid() )
        GetBasicManager();
    std::vector< OUString > sModules;
    if ( xHandler.is() )
    {
        if( pImpl->aBasicManager.LegacyPsswdBinaryLimitExceeded( sModules ) )
        {
            ModuleSizeExceeded* pReq =  new ModuleSizeExceeded( sModules );
            uno::Reference< task::XInteractionRequest > xReq( pReq );
            xHandler->handle( xReq );
            return pReq->isApprove();
        }
    }
#endif
    // No interaction handler, default is to continue to save
    return true;
}

// comphelper::IEmbeddedHelper
uno::Reference< task::XInteractionHandler > SfxObjectShell::getInteractionHandler() const
{
    uno::Reference< task::XInteractionHandler > xRet;
    if ( GetMedium() )
        xRet = GetMedium()->GetInteractionHandler();
    return xRet;
}

OUString SfxObjectShell::getDocumentBaseURL() const
{
    return GetMedium()->GetBaseURL();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
