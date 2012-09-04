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

#include "sal/config.h"

#include <cassert>

#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/FilterOptionsRequest.hpp>
#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionAskLater.hpp>
#include <com/sun/star/task/FutureDocumentVersionProductUpdateRequest.hpp>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
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
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/interaction.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/securityoptions.hxx>
#include <cppuhelper/weak.hxx>
#include <unotools/streamwrap.hxx>

#include <unotools/saveopt.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/docinfohelper.hxx>
#include <ucbhelper/content.hxx>
#include <sot/storinfo.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/string.hxx>
#include <vcl/bitmapex.hxx>
#include <svtools/embedhlp.hxx>
#include <rtl/logfile.hxx>
#include <basic/modsizeexceeded.hxx>
#include <osl/file.hxx>

#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/request.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/docfile.hxx>
#include "fltfnc.hxx"
#include <sfx2/docfilt.hxx>
#include <sfx2/docfac.hxx>
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

#include "../appl/app.hrc"

extern sal_uInt32 CheckPasswd_Impl( SfxObjectShell*, SfxItemPool&, SfxMedium* );

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::document;
using namespace ::rtl;
using namespace ::cppu;

namespace css = ::com::sun::star;

//=========================================================================
void impl_addToModelCollection(const css::uno::Reference< css::frame::XModel >& xModel)
{
    if (!xModel.is())
        return;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    css::uno::Reference< css::container::XSet > xModelCollection(
        xSMGR->createInstance(::rtl::OUString("com.sun.star.frame.GlobalEventBroadcaster")),
        css::uno::UNO_QUERY);
    if (xModelCollection.is())
    {
        try
        {
            xModelCollection->insert(css::uno::makeAny(xModel));
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "The document seems to be in the collection already!\n" );
        }
    }
}

//=========================================================================

sal_Bool SfxObjectShell::Save()
{
    return SaveChildren();
}

//--------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveAs( SfxMedium& rMedium )
{
    return SaveAsChildren( rMedium );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::QuerySlotExecutable( sal_uInt16 /*nSlotId*/ )
{
    return sal_True;
}

//-------------------------------------------------------------------------

bool GetEncryptionData_Impl( const SfxItemSet* pSet, uno::Sequence< beans::NamedValue >& o_rEncryptionData )
{
    bool bResult = false;
    if ( pSet )
    {
        SFX_ITEMSET_ARG( pSet, pEncryptionDataItem, SfxUnoAnyItem, SID_ENCRYPTIONDATA, sal_False);
        if ( pEncryptionDataItem )
        {
            pEncryptionDataItem->GetValue() >>= o_rEncryptionData;
            bResult = true;
        }
        else
        {
            SFX_ITEMSET_ARG( pSet, pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False);
            if ( pPasswordItem )
            {
                ::rtl::OUString aPassword = pPasswordItem->GetValue();
                o_rEncryptionData = ::comphelper::OStorageHelper::CreatePackageEncryptionData( aPassword );
                bResult = true;
            }
        }
    }

    return bResult;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::PutURLContentsToVersionStream_Impl(
                                            ::rtl::OUString aURL,
                                            const uno::Reference< embed::XStorage >& xDocStorage,
                                            ::rtl::OUString aStreamName )
{
    sal_Bool bResult = sal_False;
    try
    {
        uno::Reference< embed::XStorage > xVersion = xDocStorage->openStorageElement(
                                                        ::rtl::OUString("Versions"),
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

        bResult = sal_True;
    }
    catch( uno::Exception& )
    {
        // TODO/LATER: handle the error depending on exception
        SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
    }

    return bResult;
}

//-------------------------------------------------------------------------
::rtl::OUString SfxObjectShell::CreateTempCopyOfStorage_Impl( const uno::Reference< embed::XStorage >& xStorage )
{
    ::rtl::OUString aTempURL = ::utl::TempFile().GetURL();

    DBG_ASSERT( !aTempURL.isEmpty(), "Can't create a temporary file!\n" );
    if ( !aTempURL.isEmpty() )
    {
        try
        {
            uno::Reference< embed::XStorage > xTempStorage =
                ::comphelper::OStorageHelper::GetStorageFromURL( aTempURL, embed::ElementModes::READWRITE );

            // the password will be transfered from the xStorage to xTempStorage by storage implemetation
            xStorage->copyToStorage( xTempStorage );

            // the temporary storage was commited by the previous method and it will die by refcount
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "Creation of a storage copy is failed!" );
            ::utl::UCBContentHelper::Kill( aTempURL );

            aTempURL = ::rtl::OUString();

            // TODO/LATER: may need error code setting based on exception
            SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        }
    }

    return aTempURL;
}

//-------------------------------------------------------------------------
SvGlobalName SfxObjectShell::GetClassName() const
{
    return GetFactory().GetClassId();
}

//-------------------------------------------------------------------------
void SfxObjectShell::SetupStorage( const uno::Reference< embed::XStorage >& xStorage,
                                   sal_Int32 nVersion,
                                   sal_Bool bTemplate ) const
{
    uno::Reference< beans::XPropertySet > xProps( xStorage, uno::UNO_QUERY );

    if ( xProps.is() )
    {
        SvGlobalName aName;
        String aFullTypeName, aShortTypeName, aAppName;
        sal_uInt32 nClipFormat=0;

        FillClass( &aName, &nClipFormat, &aAppName, &aFullTypeName, &aShortTypeName, nVersion, bTemplate );
        if ( nClipFormat )
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
                    xProps->setPropertyValue( ::rtl::OUString("MediaType"), uno::makeAny( aDataFlavor.MimeType ) );
                }
                catch( uno::Exception& )
                {
                    const_cast<SfxObjectShell*>( this )->SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
                }

                SvtSaveOptions aSaveOpt;
                SvtSaveOptions::ODFDefaultVersion nDefVersion = aSaveOpt.GetODFDefaultVersion();

                uno::Sequence< beans::NamedValue > aEncryptionAlgs( 3 );
                aEncryptionAlgs[0].Name = ::rtl::OUString( "StartKeyGenerationAlgorithm"  );
                aEncryptionAlgs[1].Name = ::rtl::OUString( "EncryptionAlgorithm"  );
                aEncryptionAlgs[2].Name = ::rtl::OUString( "ChecksumAlgorithm"  );
                // the default values, that should be used for ODF1.1 and older formats
                aEncryptionAlgs[0].Value <<= xml::crypto::DigestID::SHA1;
                aEncryptionAlgs[1].Value <<= xml::crypto::CipherID::BLOWFISH_CFB_8;
                aEncryptionAlgs[2].Value <<= xml::crypto::DigestID::SHA1_1K;

                if ( nDefVersion >= SvtSaveOptions::ODFVER_012 )
                {
                    try
                    {
                        // older versions can not have this property set, it exists only starting from ODF1.2
                        xProps->setPropertyValue( ::rtl::OUString("Version" ), uno::makeAny( ODFVER_012_TEXT ) );
                    }
                    catch( uno::Exception& )
                    {
                    }

                    if ( !aSaveOpt.IsUseSHA1InODF12() && nDefVersion != SvtSaveOptions::ODFVER_012_EXT_COMPAT )
                    {
                        aEncryptionAlgs[0].Value <<= xml::crypto::DigestID::SHA256;
                        aEncryptionAlgs[2].Value <<= xml::crypto::DigestID::SHA256_1K;
                    }
                    if ( !aSaveOpt.IsUseBlowfishInODF12() && nDefVersion != SvtSaveOptions::ODFVER_012_EXT_COMPAT )
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
                    const_cast<SfxObjectShell*>( this )->SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
                }

            }
        }
    }
}

//-------------------------------------------------------------------------
void SfxObjectShell::PrepareSecondTryLoad_Impl()
{
    // only for internal use
    pImp->m_xDocStorage = uno::Reference< embed::XStorage >();
    pImp->m_bIsInit = sal_False;
    ResetError();
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::GeneralInit_Impl( const uno::Reference< embed::XStorage >& xStorage,
                                            sal_Bool bTypeMustBeSetAlready )
{
    if ( pImp->m_bIsInit )
        return sal_False;

    pImp->m_bIsInit = sal_True;
    if ( xStorage.is() )
    {
        // no notification is required the storage is set the first time
        pImp->m_xDocStorage = xStorage;

        try {
            uno::Reference < beans::XPropertySet > xPropSet( xStorage, uno::UNO_QUERY_THROW );
            Any a = xPropSet->getPropertyValue( ::rtl::OUString("MediaType"  ) );
            ::rtl::OUString aMediaType;
            if ( !(a>>=aMediaType) || aMediaType.isEmpty() )
            {
                if ( bTypeMustBeSetAlready )
                {
                    SetError( ERRCODE_IO_BROKENPACKAGE, ::rtl::OUString( OSL_LOG_PREFIX  ) );
                    return sal_False;
                }

                SetupStorage( xStorage, SOFFICE_FILEFORMAT_CURRENT, sal_False );
            }
        }
        catch ( uno::Exception& )
        {
            OSL_FAIL( "Can't check storage's mediatype!\n" );
        }
    }
    else
        pImp->m_bCreateTempStor = sal_True;

    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::InitNew( const uno::Reference< embed::XStorage >& xStorage )
{
    return GeneralInit_Impl( xStorage, sal_False );
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::Load( SfxMedium& rMedium )
{
    return GeneralInit_Impl( rMedium.GetStorage(), sal_True );
}

sal_Bool SfxObjectShell::DoInitNew( SfxMedium* pMed )
/*  [Description]

    This from SvPersist inherited virtual method is called to initialize
    the SfxObjectShell instance from a storage (PStore! = 0) or (PStore == 0)

    Like with all Do...-methods there is a from a control, the actual
    implementation is done by the virtual method in which also the
    InitNew(SvStorate *) from the SfxObjectShell-Subclass is implemented.

    For pStore == 0 the SfxObjectShell-instance is connected to an empty
    SfxMedium, otherwise a SfxMedium, which refers to the SvStorage
    passed as a parameter.

    The object is only initialized correctly after InitNew() or Load().

    [Return value]
    sal_True            The object has been initialized.
    sal_False           The object could not be initialized
*/

{
    ModifyBlocker_Impl aBlock( this );
    pMedium = pMed;
    if ( !pMedium )
    {
        bIsTmp = sal_True;
        pMedium = new SfxMedium;
    }

    pMedium->CanDisposeStorage_Impl( sal_True );

    if ( InitNew( pMed ? pMed->GetStorage() : uno::Reference < embed::XStorage >() ) )
    {
        // empty documents always get their macros from the user, so there is no reason to restrict access
        pImp->aMacroMode.allowMacroExecution();
        if ( SFX_CREATE_MODE_EMBEDDED == eCreateMode )
            SetTitle(SfxResId(STR_NONAME).toString());

        uno::Reference< frame::XModel >  xModel ( GetModel(), uno::UNO_QUERY );
        if ( xModel.is() )
        {
            SfxItemSet *pSet = GetMedium()->GetItemSet();
            uno::Sequence< beans::PropertyValue > aArgs;
            TransformItems( SID_OPENDOC, *pSet, aArgs );
            sal_Int32 nLength = aArgs.getLength();
            aArgs.realloc( nLength + 1 );
            aArgs[nLength].Name = DEFINE_CONST_UNICODE("Title");
            aArgs[nLength].Value <<= ::rtl::OUString( GetTitle( SFX_TITLE_DETECT ) );
            xModel->attachResource( ::rtl::OUString(), aArgs );
            impl_addToModelCollection(xModel);
        }

        SetInitialized_Impl( true );
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ImportFromGeneratedStream_Impl(
                    const uno::Reference< io::XStream >& xStream,
                    const uno::Sequence< beans::PropertyValue >& aMediaDescr )
{
    if ( !xStream.is() )
        return sal_False;

    if ( pMedium && pMedium->HasStorage_Impl() )
        pMedium->CloseStorage();

    sal_Bool bResult = sal_False;

    try
    {
        uno::Reference< embed::XStorage > xStorage =
            ::comphelper::OStorageHelper::GetStorageFromStream( xStream, embed::ElementModes::READWRITE );

        if ( !xStorage.is() )
            throw uno::RuntimeException();

        if ( !pMedium )
            pMedium = new SfxMedium( xStorage, String() );
        else
            pMedium->SetStorage_Impl( xStorage );

        SfxAllItemSet aSet( SFX_APP()->GetPool() );
        TransformParameters( SID_OPENDOC, aMediaDescr, aSet );
        pMedium->GetItemSet()->Put( aSet );
        pMedium->CanDisposeStorage_Impl( sal_False );

        // allow the subfilter to reinit the model
        if ( pImp->m_bIsInit )
            pImp->m_bIsInit = sal_False;

        if ( LoadOwnFormat( *pMedium ) )
        {
            bHasName = sal_True;
            if ( !IsReadOnly() && IsLoadReadonly() )
                SetReadOnlyUI();

            bResult = sal_True;
            OSL_ENSURE( pImp->m_xDocStorage == xStorage, "Wrong storage is used!\n" );
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

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoLoad( SfxMedium *pMed )
{
    ModifyBlocker_Impl aBlock( this );

    pMedium = pMed;
    pMedium->CanDisposeStorage_Impl( sal_True );

    sal_Bool bOk = sal_False;
    const SfxFilter* pFilter = pMed->GetFilter();
    SfxItemSet* pSet = pMedium->GetItemSet();
    if( !pImp->nEventId )
    {
        SFX_ITEMSET_ARG(
            pSet, pTemplateItem, SfxBoolItem,
            SID_TEMPLATE, sal_False);
        SetActivateEvent_Impl(
            ( pTemplateItem && pTemplateItem->GetValue() )
            ? SFX_EVENT_CREATEDOC : SFX_EVENT_OPENDOC );
    }


    SFX_ITEMSET_ARG( pSet, pBaseItem, SfxStringItem,
                     SID_BASEURL, sal_False);
    rtl::OUString aBaseURL;
    SFX_ITEMSET_ARG( pMedium->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if( pBaseItem )
        aBaseURL = pBaseItem->GetValue();
    else
    {
        if ( pSalvageItem )
        {
            rtl::OUString aName( pMed->GetPhysicalName() );
            ::utl::LocalFileHelper::ConvertPhysicalNameToURL( aName, aBaseURL );
        }
        else
            aBaseURL = pMed->GetBaseURL();
    }
    pMed->GetItemSet()->Put( SfxStringItem( SID_DOC_BASEURL, aBaseURL ) );

    pImp->nLoadedFlags = 0;
    pImp->bModelInitialized = sal_False;

    //TODO/LATER: make a clear strategy how to handle "UsesStorage" etc.
    sal_Bool bOwnStorageFormat = IsOwnStorageFormat_Impl( *pMedium );
    sal_Bool bHasStorage = IsPackageStorageFormat_Impl( *pMedium );
    if ( pMedium->GetFilter() )
    {
        sal_uInt32 nError = HandleFilter( pMedium, this );
        if ( nError != ERRCODE_NONE )
            SetError( nError, ::rtl::OUString( OSL_LOG_PREFIX  ) );

        if (pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_STARTPRESENTATION)
            pSet->Put( SfxBoolItem( SID_DOC_STARTPRESENTATION, sal_True) );
    }

    EnableSetModified( sal_False );

    pMedium->LockOrigFileOnDemand( sal_True, sal_False );
    if ( GetError() == ERRCODE_NONE && bOwnStorageFormat && ( !pFilter || !( pFilter->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) ) )
    {
        uno::Reference< embed::XStorage > xStorage;
        if ( pMedium->GetError() == ERRCODE_NONE )
            xStorage = pMedium->GetStorage();

        if( xStorage.is() && pMedium->GetLastStorageCreationState() == ERRCODE_NONE )
        {
            DBG_ASSERT( pFilter, "No filter for storage found!" );

            try
            {
                sal_Bool bWarnMediaTypeFallback = sal_False;
                SFX_ITEMSET_ARG( pMedium->GetItemSet(), pRepairPackageItem, SfxBoolItem, SID_REPAIRPACKAGE, sal_False);

                // treat the package as broken if the mediatype was retrieved as a fallback
                uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );
                xStorProps->getPropertyValue( ::rtl::OUString( "MediaTypeFallbackUsed"  ) )
                                                                    >>= bWarnMediaTypeFallback;

                if ( pRepairPackageItem && pRepairPackageItem->GetValue() )
                {
                    // the macros in repaired documents should be disabled
                    pMedium->GetItemSet()->Put( SfxUInt16Item( SID_MACROEXECMODE, document::MacroExecMode::NEVER_EXECUTE ) );

                    // the mediatype was retrieved by using fallback solution but this is a repairing mode
                    // so it is acceptable to open the document if there is no contents that required manifest.xml
                    bWarnMediaTypeFallback = sal_False;
                }

                if ( bWarnMediaTypeFallback || !xStorage->getElementNames().getLength() )
                    SetError( ERRCODE_IO_BROKENPACKAGE, ::rtl::OUString( OSL_LOG_PREFIX  ) );
            }
            catch( uno::Exception& )
            {
                // TODO/LATER: may need error code setting based on exception
                SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
            }

            // Load
            if ( !GetError() )
            {
                pImp->nLoadedFlags = 0;
                pImp->bModelInitialized = sal_False;
                bOk = xStorage.is() && LoadOwnFormat( *pMed );
                if ( bOk )
                {
                    // the document loaded from template has no name
                    SFX_ITEMSET_ARG( pMedium->GetItemSet(), pTemplateItem, SfxBoolItem, SID_TEMPLATE, sal_False);
                    if ( !pTemplateItem || !pTemplateItem->GetValue() )
                        bHasName = sal_True;
                }
                else
                    SetError( ERRCODE_ABORT, ::rtl::OUString( OSL_LOG_PREFIX  ) );
            }
        }
        else
            SetError( pMed->GetLastStorageCreationState(), ::rtl::OUString( OSL_LOG_PREFIX  ) );
    }
    else if ( GetError() == ERRCODE_NONE && InitNew(0) )
    {
        // Name vor ConvertFrom setzen, damit GetSbxObject() schon funktioniert
        bHasName = sal_True;
        SetName( SfxResId(STR_NONAME).toString() );

        if( !bHasStorage )
            pMedium->GetInStream();
        else
            pMedium->GetStorage();

        if ( GetError() == ERRCODE_NONE )
        {
            pImp->nLoadedFlags = 0;
            pImp->bModelInitialized = sal_False;
            if ( pMedium->GetFilter() && ( pMedium->GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
            {
                uno::Reference < beans::XPropertySet > xSet( GetModel(), uno::UNO_QUERY );
                ::rtl::OUString sLockUpdates("LockUpdates");
                bool bSetProperty = true;
                try
                {
                    xSet->setPropertyValue( sLockUpdates, makeAny( (sal_Bool) sal_True ) );
                }
                catch(const beans::UnknownPropertyException& )
                {
                    bSetProperty = false;
                }
                bOk = ImportFrom( *pMedium, false );
                if(bSetProperty)
                {
                    try
                    {
                        xSet->setPropertyValue( sLockUpdates, makeAny( (sal_Bool) sal_False ) );
                    }
                    catch(const beans::UnknownPropertyException& )
                    {}
                }
                UpdateLinks();
                FinishedLoading( SFX_LOADED_ALL );
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
            ::ucbhelper::Content aContent( pMedium->GetName(), com::sun::star::uno::Reference < XCommandEnvironment >() );
            com::sun::star::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                ::rtl::OUString aAuthor( "Author" );
                ::rtl::OUString aKeywords( "Keywords" );
                ::rtl::OUString aSubject( "Subject" );
                Any aAny;
                ::rtl::OUString aValue;
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
        if ( !( pImp->nLoadedFlags & SFX_LOADED_MAINDOCUMENT ) &&
            ( !pMedium->GetFilter() || pMedium->GetFilter()->UsesStorage() )
            )
            FinishedLoading( SFX_LOADED_MAINDOCUMENT );

        if( IsOwnStorageFormat_Impl(*pMed) && pMed->GetFilter() )
        {
        }
        Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );

        if ( SFX_CREATE_MODE_EMBEDDED != eCreateMode )
        {
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pAsTempItem, SfxBoolItem, SID_TEMPLATE, sal_False);
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pPreviewItem, SfxBoolItem, SID_PREVIEW, sal_False);
            SFX_ITEMSET_ARG( pMedium->GetItemSet(), pHiddenItem, SfxBoolItem, SID_HIDDEN, sal_False);
            if( bOk && !pMedium->GetOrigURL().isEmpty()
            && !( pAsTempItem && pAsTempItem->GetValue() )
            && !( pPreviewItem && pPreviewItem->GetValue() )
            && !( pHiddenItem && pHiddenItem->GetValue() ) )
            {
                AddToRecentlyUsedList();
            }
        }

        const SfxBoolItem* pDdeReconnectItem = static_cast<const SfxBoolItem*>(
            SfxRequest::GetItem(pMedium->GetItemSet(), SID_DDE_RECONNECT_ONLOAD, false, TYPE(SfxBoolItem)));

        bool bReconnectDde = true; // by default, we try to auto-connect DDE connections.
        if (pDdeReconnectItem)
            bReconnectDde = pDdeReconnectItem->GetValue();

        if (bReconnectDde)
            ReconnectDdeLinks(*this);
    }

    return bOk;
}

sal_uInt32 SfxObjectShell::HandleFilter( SfxMedium* pMedium, SfxObjectShell* pDoc )
{
    sal_uInt32 nError = ERRCODE_NONE;
    SfxItemSet* pSet = pMedium->GetItemSet();
    SFX_ITEMSET_ARG( pSet, pOptions, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
    SFX_ITEMSET_ARG( pSet, pData, SfxUnoAnyItem, SID_FILTER_DATA, sal_False );
    if ( !pData && !pOptions )
    {
        com::sun::star::uno::Reference< XMultiServiceFactory > xServiceManager = ::comphelper::getProcessServiceFactory();
        com::sun::star::uno::Reference< XNameAccess > xFilterCFG;
        if( xServiceManager.is() )
        {
            xFilterCFG = com::sun::star::uno::Reference< XNameAccess >(
                xServiceManager->createInstance( ::rtl::OUString("com.sun.star.document.FilterFactory") ),
                UNO_QUERY );
        }

        if( xFilterCFG.is() )
        {
            sal_Bool bAbort = sal_False;
            try {
                const SfxFilter* pFilter = pMedium->GetFilter();
                Sequence < PropertyValue > aProps;
                Any aAny = xFilterCFG->getByName( pFilter->GetName() );
                if ( aAny >>= aProps )
                {
                    sal_Int32 nPropertyCount = aProps.getLength();
                    for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                        if( aProps[nProperty].Name == "UIComponent" )
                        {
                            ::rtl::OUString aServiceName;
                            aProps[nProperty].Value >>= aServiceName;
                            if( !aServiceName.isEmpty() )
                            {
                                com::sun::star::uno::Reference< XInteractionHandler > rHandler = pMedium->GetInteractionHandler();
                                if( rHandler.is() )
                                {
                                    // we need some properties in the media descriptor, so we have to make sure that they are in
                                    Any aStreamAny;
                                    aStreamAny <<= pMedium->GetInputStream();
                                    if ( pSet->GetItemState( SID_INPUTSTREAM ) < SFX_ITEM_SET )
                                        pSet->Put( SfxUnoAnyItem( SID_INPUTSTREAM, aStreamAny ) );
                                    if ( pSet->GetItemState( SID_FILE_NAME ) < SFX_ITEM_SET )
                                        pSet->Put( SfxStringItem( SID_FILE_NAME, pMedium->GetName() ) );
                                    if ( pSet->GetItemState( SID_FILTER_NAME ) < SFX_ITEM_SET )
                                        pSet->Put( SfxStringItem( SID_FILTER_NAME, pFilter->GetName() ) );

                                    Sequence< PropertyValue > rProperties;
                                    TransformItems( SID_OPENDOC, *pSet, rProperties, NULL );
                                    RequestFilterOptions* pFORequest = new RequestFilterOptions( pDoc->GetModel(), rProperties );

                                    com::sun::star::uno::Reference< XInteractionRequest > rRequest( pFORequest );
                                    rHandler->handle( rRequest );

                                    if ( !pFORequest->isAbort() )
                                    {
                                            SfxAllItemSet aNewParams( pDoc->GetPool() );
                                            TransformParameters( SID_OPENDOC,
                                                            pFORequest->getFilterOptions(),
                                                            aNewParams,
                                                            NULL );

                                            SFX_ITEMSET_ARG( &aNewParams,
                                                        pFilterOptions,
                                                        SfxStringItem,
                                                        SID_FILE_FILTEROPTIONS,
                                                        sal_False );
                                            if ( pFilterOptions )
                                                pSet->Put( *pFilterOptions );

                                            SFX_ITEMSET_ARG( &aNewParams,
                                                        pFilterData,
                                                        SfxUnoAnyItem,
                                                        SID_FILTER_DATA,
                                                        sal_False );
                                            if ( pFilterData )
                                                pSet->Put( *pFilterData );
                                    }
                                    else
                                        bAbort = sal_True;
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

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsOwnStorageFormat_Impl(const SfxMedium &rMedium) const
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->IsOwnFormat() &&
             rMedium.GetFilter()->UsesStorage() &&
             rMedium.GetFilter()->GetVersion() >= SOFFICE_FILEFORMAT_60 );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::IsPackageStorageFormat_Impl(const SfxMedium &rMedium) const
{
    return !rMedium.GetFilter() || // Embedded
           ( rMedium.GetFilter()->UsesStorage() &&
             rMedium.GetFilter()->GetVersion() >= SOFFICE_FILEFORMAT_60 );
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSave()
// DoSave is only invoked for OLE. Save your own documents in the SFX through
// DoSave_Impl order to allow for the creation of backups.
// Save in your own format again.
{
    sal_Bool bOk = sal_False ;
    {
        ModifyBlocker_Impl aBlock( this );

        pImp->bIsSaving = sal_True;

        uno::Sequence< beans::NamedValue > aEncryptionData;
        if ( IsPackageStorageFormat_Impl( *GetMedium() ) )
        {
            if ( GetEncryptionData_Impl( GetMedium()->GetItemSet(), aEncryptionData ) )
            {
                try
                {
                    //TODO/MBA: GetOutputStorage?! Special mode, because it's "Save"?!
                    ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( GetMedium()->GetStorage(), aEncryptionData );
                    bOk = sal_True;
                }
                catch( uno::Exception& )
                {
                    SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
                }

                DBG_ASSERT( bOk, "The root storage must allow to set common password!\n" );
            }
            else
                bOk = sal_True;
#ifndef DISABLE_SCRIPTING
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

                    ::rtl::OUString aBasicStorageName( "Basic"  );
                    ::rtl::OUString aDialogsStorageName( "Dialogs"  );
                    if ( GetMedium()->GetStorage()->hasByName( aBasicStorageName ) )
                        GetMedium()->GetStorage()->copyElementTo( aBasicStorageName, xTmpStorage, aBasicStorageName );
                    if ( GetMedium()->GetStorage()->hasByName( aDialogsStorageName ) )
                        GetMedium()->GetStorage()->copyElementTo( aDialogsStorageName, xTmpStorage, aDialogsStorageName );

                    GetBasicManager();

                    // disconnect from the current storage
                    pImp->pBasicManager->setStorage( xTmpStorage );

                    // store to the current storage
                    pImp->pBasicManager->storeLibrariesToStorage( GetMedium()->GetStorage() );

                    // connect to the current storage back
                    pImp->pBasicManager->setStorage( GetMedium()->GetStorage() );
                }
                catch( uno::Exception& )
                {
                    SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
                    bOk = sal_False;
                }
            }
#endif
        }

        if ( bOk )
            bOk = Save();

        bOk = pMedium->Commit();
    }

    return bOk;
}

void Lock_Impl( SfxObjectShell* pDoc, sal_Bool bLock )
{
    SfxViewFrame *pFrame= SfxViewFrame::GetFirst( pDoc );
    while ( pFrame )
    {
        pFrame->GetDispatcher()->Lock( bLock );
        pFrame->Enable( !bLock );
        pFrame = SfxViewFrame::GetNext( *pFrame, pDoc );
    }

}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::SaveTo_Impl
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
    RTL_LOGFILE_PRODUCT_CONTEXT( aLog, "PERFORMANCE SfxObjectShell::SaveTo_Impl" );
    if( RTL_LOGFILE_HASLOGFILE() )
    {
        rtl::OString aString(
            rtl::OUStringToOString(rMedium.GetName(), RTL_TEXTENCODING_ASCII_US));
        RTL_LOGFILE_PRODUCT_CONTEXT_TRACE1(aLog, "saving \"%s\"", aString.getStr());
    }

    AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Begin"  ) );

    ModifyBlocker_Impl aMod(this);

    const SfxFilter *pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        // if no filter was set, use the default filter
        // this should be changed in the feature, it should be an error!
        OSL_FAIL("No filter set!");
        pFilter = GetFactory().GetFilterContainer()->GetAnyFilter( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT );
        rMedium.SetFilter(pFilter);
    }

    sal_Bool bStorageBasedSource = IsPackageStorageFormat_Impl( *pMedium );
    sal_Bool bStorageBasedTarget = IsPackageStorageFormat_Impl( rMedium );
    sal_Bool bOwnSource = IsOwnStorageFormat_Impl( *pMedium );
    sal_Bool bOwnTarget = IsOwnStorageFormat_Impl( rMedium );

    // Examine target format to determine whether to query if any password
    // protected libraries exceed the size we can handler
    if ( bOwnTarget && !QuerySaveSizeExceededModules_Impl( rMedium.GetInteractionHandler() ) )
    {
        SetError( ERRCODE_IO_ABORT, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        return sal_False;
    }

    sal_Bool bNeedsDisconnectionOnFail = sal_False;

    sal_Bool bStoreToSameLocation = sal_False;

    // the detection whether the script is changed should be done before saving
    sal_Bool bTryToPreserveScriptSignature = sal_False;
    // no way to detect whether a filter is oasis format, have to wait for saving process
    sal_Bool bNoPreserveForOasis = sal_False;
    if ( bOwnSource && bOwnTarget
      && ( pImp->nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_OK
        || pImp->nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED
        || pImp->nScriptingSignatureState == SIGNATURESTATE_SIGNATURES_INVALID ) )
    {
        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "MacroSignaturePreserving"  ) );

        // the checking of the library modified state iterates over the libraries, should be done only when required
        // currently the check is commented out since it is broken, we have to check the signature every time we save
        // TODO/LATER: let isAnyContainerModified() work!
        bTryToPreserveScriptSignature = sal_True; // !pImp->pBasicManager->isAnyContainerModified();
        if ( bTryToPreserveScriptSignature )
        {
            // check that the storage format stays the same
            SvtSaveOptions aSaveOpt;
            SvtSaveOptions::ODFDefaultVersion nVersion = aSaveOpt.GetODFDefaultVersion();

            ::rtl::OUString aODFVersion;
            try
            {
                uno::Reference < beans::XPropertySet > xPropSet( GetStorage(), uno::UNO_QUERY_THROW );
                xPropSet->getPropertyValue( ::rtl::OUString( "Version"  ) ) >>= aODFVersion;
            }
            catch( uno::Exception& )
            {}

            // preserve only if the same filter has been used
            bTryToPreserveScriptSignature = pMedium->GetFilter() && pFilter && pMedium->GetFilter()->GetFilterName() == pFilter->GetFilterName();

            bNoPreserveForOasis = (
                                   (aODFVersion.equals( ODFVER_012_TEXT ) && nVersion == SvtSaveOptions::ODFVER_011) ||
                                   (aODFVersion.isEmpty() && nVersion >= SvtSaveOptions::ODFVER_012)
                                  );
        }
    }

    sal_Bool bCopyTo = sal_False;
    SfxItemSet *pMedSet = rMedium.GetItemSet();
    if( pMedSet )
    {
        SFX_ITEMSET_ARG( pMedSet, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
        bCopyTo =   GetCreateMode() == SFX_CREATE_MODE_EMBEDDED ||
                    (pSaveToItem && pSaveToItem->GetValue());
    }

    // use UCB for case sensitive/insensitive file name comparison
    if ( pMedium
      && !pMedium->GetName().equalsIgnoreAsciiCaseAscii("private:stream")
      && !rMedium.GetName().equalsIgnoreAsciiCaseAscii("private:stream")
      && ::utl::UCBContentHelper::EqualURLs( pMedium->GetName(), rMedium.GetName() ) )
    {
        bStoreToSameLocation = sal_True;
        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Save"  ) );

        if ( pMedium->DocNeedsFileDateCheck() )
            rMedium.CheckFileDate( pMedium->GetInitFileDate( sal_False ) );

        if ( bCopyTo && GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
        {
            // export to the same location is forbidden
            SetError( ERRCODE_IO_CANTWRITE, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        }
        else
        {
            // before we overwrite the original file, we will make a backup if there is a demand for that
            // if the backup is not created here it will be created internally and will be removed in case of successful saving
            const sal_Bool bDoBackup = SvtSaveOptions().IsBackup();
            if ( bDoBackup )
            {
                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "DoBackup"  ) );
                rMedium.DoBackup_Impl();
                if ( rMedium.GetError() )
                {
                    SetError( rMedium.GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );
                    rMedium.ResetError();
                }
            }

            if ( bStorageBasedSource && bStorageBasedTarget )
            {
                // The active storage must be switched. The simple saving is not enough.
                // The problem is that the target medium contains target MediaDescriptor.

                    // In future the switch of the persistance could be done on stream level:
                    // a new wrapper service will be implemented that allows to exchange
                    // persistance on the fly. So the real persistance will be set
                    // to that stream only after successful commit of the storage.
                    // TODO/LATER:
                    // create wrapper stream based on the URL
                    // create a new storage based on this stream
                    // store to this new storage
                    // commit the new storage
                    // call saveCompleted based with this new storage ( get rid of old storage and "frees" URL )
                    // commit the wrapper stream ( the stream will connect the URL only on commit, after that it will hold it )
                    // if the last step is failed the stream should stay to be transacted and should be commited on any flush
                    // so we can forget the stream in any way and the next storage commit will flush it

                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Save: Own to Own"  ) );

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

                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Save: Alien to Alien"  ) );

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

                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Save: Alien to Own"  ) );

                pMedium->CloseAndRelease();
                rMedium.CloseAndRelease();
                rMedium.GetOutputStorage();
            }
            else // means if ( bStorageBasedSource && !bStorageBasedTarget )
            {
                // the source format is an own one but the target is
                // an alien format, just connect the source to temporary
                // storage

                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Save: Own to Alien"  ) );

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

        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "SaveAs/Export"  ) );

        rMedium.CloseAndRelease();
        if ( bStorageBasedTarget )
        {
            rMedium.GetOutputStorage();
        }
    }

    // TODO/LATER: error handling
    if( rMedium.GetErrorCode() || pMedium->GetErrorCode() || GetErrorCode() )
        return sal_False;

    AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Locking"  ) );

    rMedium.LockOrigFileOnDemand( sal_False, sal_False );

    if ( bStorageBasedTarget )
    {
        if ( rMedium.GetErrorCode() )
            return sal_False;

        // If the filter is a "cross export" filter ( f.e. a filter for exporting an impress document from
        // a draw document ), the ClassId of the destination storage is different from the ClassId of this
        // document. It can be retrieved from the default filter for the desired target format
        long nFormat = rMedium.GetFilter()->GetFormat();
        SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
        const SfxFilter *pFilt = rMatcher.GetFilter4ClipBoardId( nFormat );
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

                    xProps->setPropertyValue( ::rtl::OUString("MediaType"),
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
        return sal_False;

    sal_Bool bOldStat = pImp->bForbidReload;
    pImp->bForbidReload = sal_True;

    // lock user interface while saving the document
    Lock_Impl( this, sal_True );

    sal_Bool bOk = sal_False;
    // TODO/LATER: get rid of bOk

    if( bOwnTarget && !( pFilter->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
    {
        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing in own format."  ) );
        uno::Reference< embed::XStorage > xMedStorage = rMedium.GetStorage();
        if ( !xMedStorage.is() )
        {
            // no saving without storage, unlock UI and return
            Lock_Impl( this, sal_False );
            pImp->bForbidReload = bOldStat;
            AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing failed, still no error set."  ) );
            return sal_False;
        }

        // transfer password from the parameters to the storage
        uno::Sequence< beans::NamedValue > aEncryptionData;
        sal_Bool bPasswdProvided = sal_False;
        if ( GetEncryptionData_Impl( rMedium.GetItemSet(), aEncryptionData ) )
        {
            bPasswdProvided = sal_True;
            try {
                ::comphelper::OStorageHelper::SetCommonStorageEncryptionData( xMedStorage, aEncryptionData );
                bOk = sal_True;
            }
            catch( uno::Exception& )
            {
                OSL_FAIL( "Setting of common encryption key failed!" );
                SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
            }
        }
        else
            bOk = sal_True;

        pFilter = rMedium.GetFilter();

        const SfxStringItem *pVersionItem = pSet ? (const SfxStringItem*)
                SfxRequest::GetItem( pSet, SID_DOCINFO_COMMENTS, sal_False, TYPE(SfxStringItem) ) : NULL;
        ::rtl::OUString aTmpVersionURL;

        if ( bOk )
        {
            bOk = sal_False;
            // currently the case that the storage is the same should be impossible
            if ( xMedStorage == GetStorage() )
            {
                OSL_ENSURE( !pVersionItem, "This scenario is impossible currently!\n" );
                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Should be impossible."  ) );
                // usual save procedure
                bOk = Save();
            }
            else
            {
                // save to target
                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Save as own format."  ) );
                bOk = SaveAsOwnFormat( rMedium );
                if ( bOk && pVersionItem )
                {
                    AddLog( ::rtl::OUString( OSL_LOG_PREFIX "pVersionItem != NULL"  ) );
                    aTmpVersionURL = CreateTempCopyOfStorage_Impl( xMedStorage );
                    bOk =  !aTmpVersionURL.isEmpty();
                }
            }
        }


        if ( bOk && GetCreateMode() != SFX_CREATE_MODE_EMBEDDED && !bPasswdProvided )
        {
            // store the thumbnail representation image
            // the thumbnail is not stored in case of encrypted document
            AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Thumbnail creation."  ) );
            if ( !GenerateAndStoreThumbnail( bPasswdProvided,
                                            sal_False,
                                            pFilter->IsOwnTemplateFormat(),
                                            xMedStorage ) )
            {
                // TODO: error handling
                OSL_FAIL( "Couldn't store thumbnail representation!" );
            }
        }

        if ( bOk )
        {
            if ( pImp->bIsSaving || pImp->bPreserveVersions )
            {
                AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Preserve versions."  ) );
                try
                {
                    Sequence < util::RevisionTag > aVersions = rMedium.GetVersionList();
                    if ( aVersions.getLength() )
                    {
                        // copy the version streams
                        ::rtl::OUString aVersionsName( "Versions"  );
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
                    AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Preserve versions has failed."  ) );
                    OSL_FAIL( "Couldn't copy versions!\n" );
                    bOk = sal_False;
                    // TODO/LATER: a specific error could be set
                }
            }

            if ( bOk && pVersionItem )
            {
                // store a version also
                const SfxStringItem *pAuthorItem = pSet ? (const SfxStringItem*)
                    SfxRequest::GetItem( pSet, SID_DOCINFO_AUTHOR, sal_False, TYPE(SfxStringItem) ) : NULL;

                // version comment
                util::RevisionTag aInfo;
                aInfo.Comment = pVersionItem->GetValue();

                // version author
                String aAuthor;
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
                    rMedium.SaveVersionList_Impl( sal_True );
                    bOk = PutURLContentsToVersionStream_Impl( aTmpVersionURL, xMedStorage, aInfo.Identifier );
                }
            }
            else if ( bOk && ( pImp->bIsSaving || pImp->bPreserveVersions ) )
            {
                rMedium.SaveVersionList_Impl( sal_True );
            }
        }

        if ( !aTmpVersionURL.isEmpty() )
            ::utl::UCBContentHelper::Kill( aTmpVersionURL );
    }
    else
    {
        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing in alien format."  ) );
        // it's a "SaveAs" in an alien format
        if ( rMedium.GetFilter() && ( rMedium.GetFilter()->GetFilterFlags() & SFX_FILTER_STARONEFILTER ) )
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
            bOk = SaveChildren( sal_True );
    }

    if ( bOk )
    {
        // if ODF version of oasis format changes on saving the signature should not be preserved
        if ( bOk && bTryToPreserveScriptSignature && bNoPreserveForOasis )
            bTryToPreserveScriptSignature = ( SotStorage::GetVersion( rMedium.GetStorage() ) == SOFFICE_FILEFORMAT_60 );

        uno::Reference< security::XDocumentDigitalSignatures > xDDSigns;
        if ( bOk && bTryToPreserveScriptSignature )
        {
            AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Copying scripting signature."  ) );

            // if the scripting code was not changed and it is signed the signature should be preserved
            // unfortunately at this point we have only information whether the basic code has changed or not
            // so the only way is to check the signature if the basic was not changed
            try
            {
                // get the ODF version of the new medium
                ::rtl::OUString aVersion;
                try
                {
                    uno::Reference < beans::XPropertySet > xPropSet( rMedium.GetStorage(), uno::UNO_QUERY_THROW );
                    xPropSet->getPropertyValue( ::rtl::OUString( "Version"  ) ) >>= aVersion;
                }
                catch( uno::Exception& )
                {
                }

                xDDSigns = security::DocumentDigitalSignatures::createWithVersion(comphelper::getProcessComponentContext(), aVersion);

                ::rtl::OUString aScriptSignName = xDDSigns->getScriptingContentSignatureDefaultStreamName();

                if ( !aScriptSignName.isEmpty() )
                {
                    pMedium->Close();

                    // target medium is still not commited, it should not be closed
                    // commit the package storage and close it, but leave the streams open
                    rMedium.StorageCommit_Impl();
                    rMedium.CloseStorage();

                    uno::Reference< embed::XStorage > xReadOrig = pMedium->GetZipStorageToSign_Impl();
                    if ( !xReadOrig.is() )
                        throw uno::RuntimeException();
                    uno::Reference< embed::XStorage > xMetaInf = xReadOrig->openStorageElement(
                                ::rtl::OUString( "META-INF"  ),
                                embed::ElementModes::READ );

                    uno::Reference< embed::XStorage > xTarget = rMedium.GetZipStorageToSign_Impl( sal_False );
                    if ( !xTarget.is() )
                        throw uno::RuntimeException();
                    uno::Reference< embed::XStorage > xTargetMetaInf = xTarget->openStorageElement(
                                ::rtl::OUString( "META-INF"  ),
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
                        sal_uInt16 nState = ImplCheckSignaturesInformation( aInfos );
                        if ( nState == SIGNATURESTATE_SIGNATURES_OK || nState == SIGNATURESTATE_SIGNATURES_NOTVALIDATED
                            || nState == SIGNATURESTATE_SIGNATURES_PARTIAL_OK)
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
                            OSL_FAIL( "An invalid signature was copied!" );
                        }
                    }
                }
            }
            catch( uno::Exception& )
            {
            }

            pMedium->Close();
            rMedium.CloseZipStorage_Impl();
        }

        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Medium commit."  ) );

        bOk = rMedium.Commit();

        if ( bOk )
        {
            AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing is successful."  ) );

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
                        ConnectTmpStorage_Impl( pImp->m_xDocStorage, NULL );
                }
                else if (!pMedium->GetName().isEmpty()
                  || ( pMedium->HasStorage_Impl() && pMedium->WillDisposeStorageOnClose_Impl() ) )
                {
                    OSL_ENSURE(!pMedium->GetName().isEmpty(), "Fallback is used, the medium without name should not dispose the storage!\n");
                    // copy storage of old medium to new temporary storage and take this over
                    if( !ConnectTmpStorage_Impl( pMedium->GetStorage(), pMedium ) )
                    {
                        AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Process after storing has failed."  ) );
                        bOk = sal_False;
                    }
                }
            }
        }
        else
        {
            AddLog( ::rtl::OUString( OSL_LOG_PREFIX "Storing has failed."  ) );

            // in case the document storage was connected to backup temporarely it must be disconnected now
            if ( bNeedsDisconnectionOnFail )
                ConnectTmpStorage_Impl( pImp->m_xDocStorage, NULL );
        }
    }

    // unlock user interface
    Lock_Impl( this, sal_False );
    pImp->bForbidReload = bOldStat;

    if ( bOk )
    {
        try
        {
            ::ucbhelper::Content aContent( rMedium.GetName(), com::sun::star::uno::Reference < XCommandEnvironment >() );
            com::sun::star::uno::Reference < XPropertySetInfo > xProps = aContent.getProperties();
            if ( xProps.is() )
            {
                ::rtl::OUString aAuthor( "Author" );
                ::rtl::OUString aKeywords( "Keywords" );
                ::rtl::OUString aSubject( "Subject" );
                Any aAny;

                uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                    GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference<document::XDocumentProperties> xDocProps
                    = xDPS->getDocumentProperties();

                if ( xProps->hasPropertyByName( aAuthor ) )
                {
                    aAny <<= xDocProps->getAuthor();
                    aContent.setPropertyValue( aAuthor, aAny );
                }
                if ( xProps->hasPropertyByName( aKeywords ) )
                {
                    aAny <<= ::comphelper::string::convertCommaSeparated(
                                xDocProps->getKeywords());
                    aContent.setPropertyValue( aKeywords, aAny );
                }
                if ( xProps->hasPropertyByName( aSubject ) )
                {
                    aAny <<= xDocProps->getSubject();
                    aContent.setPropertyValue( aSubject, aAny );
                }
            }
        }
        catch( Exception& )
        {
        }
    }

    return bOk;
}

//------------------------------------------------------------------------
sal_Bool SfxObjectShell::DisconnectStorage_Impl( SfxMedium& rSrcMedium, SfxMedium& rTargetMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::DisconnectStorage_Impl" );

    // this method disconnects the storage from source medium, and attaches it to the backup created by the target medium

    uno::Reference< embed::XStorage > xStorage = rSrcMedium.GetStorage();

    sal_Bool bResult = sal_False;
    if ( xStorage == pImp->m_xDocStorage )
    {
        try
        {
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY_THROW );
            ::rtl::OUString aBackupURL = rTargetMedium.GetBackup_Impl();
            if ( aBackupURL.isEmpty() )
            {
                // the backup could not be created, try to disconnect the storage and close the source SfxMedium
                // in this case the optimization is not possible, connect storage to a temporary file
                rTargetMedium.ResetError();
                xOptStorage->writeAndAttachToStream( uno::Reference< io::XStream >() );
                rSrcMedium.CanDisposeStorage_Impl( sal_False );
                rSrcMedium.Close();

                // now try to create the backup
                rTargetMedium.GetBackup_Impl();
            }
            else
            {
                // the following call will only compare stream sizes
                // TODO/LATER: this is a very risky part, since if the URL contents are different from the storage
                // contents, the storag will be broken
                xOptStorage->attachToURL( aBackupURL, sal_True );

                // the storage is successfuly attached to backup, thus it it owned by the document not by the medium
                rSrcMedium.CanDisposeStorage_Impl( sal_False );
                bResult = sal_True;
            }
        }
        catch ( uno::Exception& )
        {}
    }

    OSL_ENSURE( bResult, "Storage disconnecting has failed - affects performance!" );

    return bResult;
}

//------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConnectTmpStorage_Impl(
    const uno::Reference< embed::XStorage >& xStorage,
    SfxMedium* pMediumArg )

/*   [Description]

     If the application operates on a temporary storage, then it may not take
     the temporary storage from the SaveCompleted. Therefore the new storage
     is connected already here in this case and SaveCompleted then does nothing.
*/

{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::ConnectTmpStorage_Impl" );

    sal_Bool bResult = sal_False;

    if ( xStorage.is() )
    {
        try
        {
            // the empty argument means that the storage will create temporary stream itself
            uno::Reference< embed::XOptimizedStorage > xOptStorage( xStorage, uno::UNO_QUERY_THROW );
            xOptStorage->writeAndAttachToStream( uno::Reference< io::XStream >() );

            // the storage is successfuly disconnected from the original sources, thus the medium must not dispose it
            if ( pMediumArg )
                pMediumArg->CanDisposeStorage_Impl( sal_False );

            bResult = sal_True;
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
                pImp->pBasicManager->setStorage( xTmpStorage );

                // Get rid of this workaround after issue i113914 is fixed
                try
                {
                    uno::Reference< script::XStorageBasedLibraryContainer > xBasicLibraries( pImp->xBasicLibraries, uno::UNO_QUERY_THROW );
                    xBasicLibraries->setRootStorage( xTmpStorage );
                }
                catch( uno::Exception& )
                {}
                try
                {
                    uno::Reference< script::XStorageBasedLibraryContainer > xDialogLibraries( pImp->xDialogLibraries, uno::UNO_QUERY_THROW );
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
            SetError( ERRCODE_IO_GENERAL, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        }
    }

    return bResult;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveObjectAs( SfxMedium& rMedium, sal_Bool bCommit )
{
    sal_Bool bOk = sal_False;
    {
        ModifyBlocker_Impl aBlock( this );

        uno::Reference < embed::XStorage > xNewStor = rMedium.GetStorage();
        if ( !xNewStor.is() )
            return sal_False;

        uno::Reference < beans::XPropertySet > xPropSet( xNewStor, uno::UNO_QUERY );
        if ( xPropSet.is() )
        {
            Any a = xPropSet->getPropertyValue( ::rtl::OUString("MediaType"  ) );
            ::rtl::OUString aMediaType;
            if ( !(a>>=aMediaType) || aMediaType.isEmpty() )
            {
                OSL_FAIL( "The mediatype must be set already!\n" );
                SetupStorage( xNewStor, SOFFICE_FILEFORMAT_CURRENT, sal_False );
            }

            pImp->bIsSaving = sal_False;
            bOk = SaveAsOwnFormat( rMedium );

            if ( bCommit )
            {
                try {
                    uno::Reference< embed::XTransactedObject > xTransact( xNewStor, uno::UNO_QUERY_THROW );
                    xTransact->commit();
                }
                catch( uno::Exception& )
                {
                    OSL_FAIL( "The strotage was not commited on DoSaveAs!\n" );
                }
            }
        }
    }

    return bOk;
}

//-------------------------------------------------------------------------
// TODO/LATER: may be the call must be removed completelly
sal_Bool SfxObjectShell::DoSaveAs( SfxMedium& rMedium )
{
    // here only root storages are included, which are stored via temp file
    rMedium.CreateTempFileNoCopy();
    SetError(rMedium.GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );
    if ( GetError() )
        return sal_False;

    // copy version list from "old" medium to target medium, so it can be used on saving
    if ( pImp->bPreserveVersions )
        rMedium.TransferVersionList_Impl( *pMedium );

    sal_Bool bRet = SaveTo_Impl( rMedium, NULL );
    if ( !bRet )
        SetError(rMedium.GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );
    return bRet;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSaveCompleted( SfxMedium* pNewMed )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::DoSaveCompleted" );

    sal_Bool bOk = sal_True;
    sal_Bool bMedChanged = pNewMed && pNewMed!=pMedium;

    DBG_ASSERT( !pNewMed || pNewMed->GetError() == ERRCODE_NONE, "DoSaveCompleted: Medium has error!" );

    // delete Medium (and Storage!) after all notifications
    SfxMedium* pOld = pMedium;
    if ( bMedChanged )
    {
        pMedium = pNewMed;
        pMedium->CanDisposeStorage_Impl( sal_True );
    }

    const SfxFilter *pFilter = pMedium ? pMedium->GetFilter() : 0;
    if ( pNewMed )
    {
        if( bMedChanged )
        {
            if (!pNewMed->GetName().isEmpty())
                bHasName = sal_True;
            Broadcast( SfxSimpleHint(SFX_HINT_NAMECHANGED) );
            getDocProperties()->setGenerator(
               ::utl::DocInfoHelper::GetGeneratorString() );
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
            if( pMedium->GetOpenMode() & STREAM_WRITE )
                pMedium->GetInStream();
            xStorage = GetStorage();
        }

        // TODO/LATER: may be this code will be replaced, but not sure
        // Set storage in document library containers
        pImp->pBasicManager->setStorage( xStorage );

        // Get rid of this workaround after issue i113914 is fixed
        try
        {
            uno::Reference< script::XStorageBasedLibraryContainer > xBasicLibraries( pImp->xBasicLibraries, uno::UNO_QUERY_THROW );
            xBasicLibraries->setRootStorage( xStorage );
        }
        catch( uno::Exception& )
        {}
        try
        {
            uno::Reference< script::XStorageBasedLibraryContainer > xDialogLibraries( pImp->xDialogLibraries, uno::UNO_QUERY_THROW );
            xDialogLibraries->setRootStorage( xStorage );
        }
        catch( uno::Exception& )
        {}
    }
    else
    {
        if( pMedium )
        {
            if( pFilter && !IsPackageStorageFormat_Impl( *pMedium ) && (pMedium->GetOpenMode() & STREAM_WRITE ))
            {
                pMedium->ReOpen();
                bOk = SaveCompletedChildren( sal_False );
            }
            else
                bOk = SaveCompleted( NULL );
        }
        // either Save or ConvertTo
        else
            bOk = SaveCompleted( NULL );
    }

    if ( bOk && pNewMed )
    {
        if( bMedChanged )
        {
            delete pOld;

            uno::Reference< frame::XModel > xModel = GetModel();
            if ( xModel.is() )
            {
                ::rtl::OUString aURL = pNewMed->GetOrigURL();
                uno::Sequence< beans::PropertyValue > aMediaDescr;
                TransformItems( SID_OPENDOC, *pNewMed->GetItemSet(), aMediaDescr );
                try
                {
                    xModel->attachResource( aURL, aMediaDescr );
                }
                catch( uno::Exception& )
                {}
            }

            // before the title regenerated the document must loose the signatures
            pImp->nDocumentSignatureState = SIGNATURESTATE_NOSIGNATURES;
            pImp->nScriptingSignatureState = pNewMed->GetCachedSignatureState_Impl();
            OSL_ENSURE( pImp->nScriptingSignatureState != SIGNATURESTATE_SIGNATURES_BROKEN, "The signature must not be broken at this place" );
            pImp->bSignatureErrorIsShown = sal_False;

            // TODO/LATER: in future the medium must control own signature state, not the document
            pNewMed->SetCachedSignatureState_Impl( SIGNATURESTATE_NOSIGNATURES ); // set the default value back

            // Set new title
            if (!pNewMed->GetName().isEmpty() && SFX_CREATE_MODE_EMBEDDED != eCreateMode)
                InvalidateName();
            SetModified(sal_False); // reset only by set medium
            Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );

            // this is the end of the saving process, it is possible that
            // the file was changed
            // between medium commit and this step (attributes change and so on)
            // so get the file date again
            if ( pNewMed->DocNeedsFileDateCheck() )
                pNewMed->GetInitFileDate( sal_True );
        }
    }

    pMedium->ClearBackup_Impl();
    pMedium->LockOrigFileOnDemand( sal_True, sal_False );

    AddToRecentlyUsedList();

    return bOk;
}

void SfxObjectShell::AddToRecentlyUsedList()
{
    INetURLObject aUrl( pMedium->GetOrigURL() );

    if ( aUrl.GetProtocol() == INET_PROT_FILE )
    {
        const SfxFilter* pOrgFilter = pMedium->GetOrigFilter();
        Application::AddToRecentDocumentList( aUrl.GetURLNoPass( INetURLObject::NO_DECODE ),
                                              (pOrgFilter) ? pOrgFilter->GetMimeType() : ::rtl::OUString() );
    }
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConvertFrom
(
    SfxMedium&  /*rMedium*/     /*  <SfxMedium>, which describes the source file
                                    (for example file name, <SfxFilter>,
                                    Open-Modi and so on) */
)

/*  [Description]

    This method is called for loading of documents over all filters which are
    not SFX_FILTER_OWN or for which no clipboard format has been registered
    (thus no storage format that is used). In other words, whith this method
    it is imported.

    Files which are to be opened here should be opened through 'rMedium'
    to guarantee the right open modes. Especially if the format is retained
    (only possible with SFX_FILTER_SIMULATE or SFX_FILTER_ONW) file which must
    be opened STREAM_SHARE_DENYWRITE.

    [Return value]

    sal_Bool                sal_True
                        The document could be loaded.

                        sal_False
                        The document could not be loaded, an error code
                        received through  <SvMedium::GetError()const>

    [Example]

    sal_Bool DocSh::ConvertFrom( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetInStream();
        if( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream >> ...;

            // Do not call 'rMedium.CloseInStream()'! Keep File locked!
            return SVSTREAM_OK == rMedium.GetError();
        }

        return sal_False;
    }

    [Cross-references]

    <SfxObjectShell::ConvertTo(SfxMedium&)>
    <SFX_FILTER_REGISTRATION>
*/
{
    return sal_False;
}

sal_Bool SfxObjectShell::ImportFrom( SfxMedium& rMedium, bool bInsert )
{
    ::rtl::OUString aTypeName( rMedium.GetFilter()->GetTypeName() );
    ::rtl::OUString aFilterName( rMedium.GetFilter()->GetFilterName() );

    uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
    uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), uno::UNO_QUERY );

    uno::Sequence < beans::PropertyValue > aProps;
    uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
    if ( xFilters->hasByName( aFilterName ) )
    {
        xFilters->getByName( aFilterName ) >>= aProps;
        rMedium.GetItemSet()->Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
    }

    ::rtl::OUString aFilterImplName;
    sal_Int32 nFilterProps = aProps.getLength();
    for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
    {
        const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
        if ( rFilterProp.Name.compareToAscii("FilterService") == COMPARE_EQUAL )
        {
            rFilterProp.Value >>= aFilterImplName;
            break;
        }
    }

    uno::Reference< document::XFilter > xLoader;
    if ( !aFilterImplName.isEmpty() )
    {
        try{
        xLoader = uno::Reference< document::XFilter >
            ( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
        }catch(const uno::Exception&)
            { xLoader.clear(); }
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

        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aArgs ( lDescriptor.getLength() );
        com::sun::star::beans::PropertyValue * pNewValue = aArgs.getArray();
        const com::sun::star::beans::PropertyValue * pOldValue = lDescriptor.getConstArray();
        const OUString sInputStream ( "InputStream"  );

        sal_Bool bHasInputStream = sal_False;
        sal_Bool bHasBaseURL = sal_False;
        sal_Int32 i;
        sal_Int32 nEnd = lDescriptor.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue [i].Name == sInputStream )
                bHasInputStream = sal_True;
            else if ( pOldValue[i].Name == "DocumentBaseURL" )
                bHasBaseURL = sal_True;
        }

        if ( !bHasInputStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sInputStream;
            aArgs[nEnd-1].Value <<= com::sun::star::uno::Reference < com::sun::star::io::XInputStream > ( new utl::OSeekableInputStreamWrapper ( *rMedium.GetInStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = ::rtl::OUString( "DocumentBaseURL"  );
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL();
        }

        if ( bInsert ) {
            aArgs.realloc( ++nEnd );
            aArgs[nEnd-1].Name = ::rtl::OUString( "InsertMode"  );
            aArgs[nEnd-1].Value <<= (sal_Bool) sal_True;
        }

        return xLoader->filter( aArgs );
        }catch(...)
        {}
    }

    return sal_False;
}

sal_Bool SfxObjectShell::ExportTo( SfxMedium& rMedium )
{
    ::rtl::OUString aTypeName( rMedium.GetFilter()->GetTypeName() );
    ::rtl::OUString aFilterName( rMedium.GetFilter()->GetFilterName() );
    uno::Reference< document::XExporter > xExporter;

    {
        uno::Reference< lang::XMultiServiceFactory >  xMan = ::comphelper::getProcessServiceFactory();
        uno::Reference < lang::XMultiServiceFactory > xFilterFact (
                xMan->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), uno::UNO_QUERY );

        uno::Sequence < beans::PropertyValue > aProps;
        uno::Reference < container::XNameAccess > xFilters ( xFilterFact, uno::UNO_QUERY );
        if ( xFilters->hasByName( aFilterName ) )
            xFilters->getByName( aFilterName ) >>= aProps;

        ::rtl::OUString aFilterImplName;
        sal_Int32 nFilterProps = aProps.getLength();
        for ( sal_Int32 nFilterProp = 0; nFilterProp<nFilterProps; nFilterProp++ )
        {
            const beans::PropertyValue& rFilterProp = aProps[nFilterProp];
            if ( rFilterProp.Name.compareToAscii("FilterService") == COMPARE_EQUAL )
            {
                rFilterProp.Value >>= aFilterImplName;
                break;
            }
        }

        if ( !aFilterImplName.isEmpty() )
        {
            try{
            xExporter = uno::Reference< document::XExporter >
                ( xFilterFact->createInstanceWithArguments( aFilterName, uno::Sequence < uno::Any >() ), uno::UNO_QUERY );
            }catch(const uno::Exception&)
                { xExporter.clear(); }
        }
    }

    if ( xExporter.is() )
    {
        try{
        uno::Reference< lang::XComponent >  xComp( GetModel(), uno::UNO_QUERY_THROW );
        uno::Reference< document::XFilter > xFilter( xExporter, uno::UNO_QUERY_THROW );
        xExporter->setSourceDocument( xComp );

        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aOldArgs;
        SfxItemSet* pItems = rMedium.GetItemSet();
        TransformItems( SID_SAVEASDOC, *pItems, aOldArgs );

        const com::sun::star::beans::PropertyValue * pOldValue = aOldArgs.getConstArray();
        com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aArgs ( aOldArgs.getLength() );
        com::sun::star::beans::PropertyValue * pNewValue = aArgs.getArray();

        // put in the REAL file name, and copy all PropertyValues
        const OUString sOutputStream ( "OutputStream"  );
        const OUString sStream ( "StreamForOutput"  );
        sal_Bool bHasOutputStream = sal_False;
        sal_Bool bHasStream = sal_False;
        sal_Bool bHasBaseURL = sal_False;
        sal_Int32 i;
        sal_Int32 nEnd = aOldArgs.getLength();

        for ( i = 0; i < nEnd; i++ )
        {
            pNewValue[i] = pOldValue[i];
            if ( pOldValue[i].Name == "FileName" )
                pNewValue[i].Value <<= OUString ( rMedium.GetName() );
            else if ( pOldValue[i].Name == sOutputStream )
                bHasOutputStream = sal_True;
            else if ( pOldValue[i].Name == sStream )
                bHasStream = sal_True;
            else if ( pOldValue[i].Name == "DocumentBaseURL" )
                bHasBaseURL = sal_True;
        }

        if ( !bHasOutputStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sOutputStream;
            aArgs[nEnd-1].Value <<= com::sun::star::uno::Reference < com::sun::star::io::XOutputStream > ( new utl::OOutputStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        // add stream as well, for OOX export and maybe others
        if ( !bHasStream )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = sStream;
            aArgs[nEnd-1].Value <<= com::sun::star::uno::Reference < com::sun::star::io::XStream > ( new utl::OStreamWrapper ( *rMedium.GetOutStream() ) );
        }

        if ( !bHasBaseURL )
        {
            aArgs.realloc ( ++nEnd );
            aArgs[nEnd-1].Name = ::rtl::OUString( "DocumentBaseURL"  );
            aArgs[nEnd-1].Value <<= rMedium.GetBaseURL( sal_True );
        }

        return xFilter->filter( aArgs );
        }catch(const uno::Exception&)
        {}
    }

    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::ConvertTo
(
    SfxMedium&  /*rMedium*/   /*  <SfxMedium>, which describes the target file
                                    (for example file name, <SfxFilter>,
                                    Open-Modi and so on) */
)

/*  [Description]

    This method is called for saving of documents over all filters which are
    not SFX_FILTER_OWN or for which no clipboard format has been registered
    (thus no storage format that is used). In other words, with this method
    it is exported.

    Files which are to be opened here should be opened through 'rMedium'
    to guarantee the right open modes. Especially if the format is retained
    (only possible with SFX_FILTER_SIMULATE or SFX_FILTER_ONW) file which must
    be opened STREAM_SHARE_DENYWRITE.

    [Return value]

    sal_Bool                sal_True
                        The document could be saved.

                        sal_False
                        The document could not be saved, an error code is
                        received by <SvMedium::GetError()const>


    [Example]

    sal_Bool DocSh::ConvertTo( SfxMedium &rMedium )
    {
        SvStreamRef xStream = rMedium.GetOutStream();
        if ( xStream.is() )
        {
            xStream->SetBufferSize(4096);
            *xStream << ...;

            rMedium.CloseOutStream(); // opens the InStream automatically
            return SVSTREAM_OK == rMedium.GetError();
        }
        return sal_False ;
    }

    [Cross-references]

    <SfxObjectShell::ConvertFrom(SfxMedium&)>
    <SFX_FILTER_REGISTRATION>
*/

{
    return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::DoSave_Impl( const SfxItemSet* pArgs )
{
    SfxMedium* pRetrMedium = GetMedium();
    const SfxFilter* pFilter = pRetrMedium->GetFilter();

    // copy the original itemset, but remove the "version" item, because pMediumTmp
    // is a new medium "from scratch", so no version should be stored into it
    SfxItemSet* pSet = new SfxAllItemSet(*pRetrMedium->GetItemSet());
    pSet->ClearItem( SID_VERSION );
    pSet->ClearItem( SID_DOC_BASEURL );

    // create a medium as a copy; this medium is only for writingm, because it
    // uses the same name as the original one writing is done through a copy,
    // that will be transferred to the target (of course after calling HandsOff)
    SfxMedium* pMediumTmp = new SfxMedium( pRetrMedium->GetName(), pRetrMedium->GetOpenMode(), pFilter, pSet );
    pMediumTmp->SetLongName( pRetrMedium->GetLongName() );
    if ( pMediumTmp->GetErrorCode() != ERRCODE_NONE )
    {
        SetError( pMediumTmp->GetError(), ::rtl::OUString( OSL_LOG_PREFIX  ) );
        delete pMediumTmp;
        return sal_False;
    }

    // copy version list from "old" medium to target medium, so it can be used on saving
    pMediumTmp->TransferVersionList_Impl( *pRetrMedium );

    // an interaction handler here can aquire only in case of GUI Saving
    // and should be removed after the saving is done
    com::sun::star::uno::Reference< XInteractionHandler > xInteract;
    SFX_ITEMSET_ARG( pArgs, pxInteractionItem, SfxUnoAnyItem, SID_INTERACTIONHANDLER, sal_False );
    if ( pxInteractionItem && ( pxInteractionItem->GetValue() >>= xInteract ) && xInteract.is() )
        pMediumTmp->GetItemSet()->Put( SfxUnoAnyItem( SID_INTERACTIONHANDLER, makeAny( xInteract ) ) );

    sal_Bool bSaved = sal_False;
    if( !GetError() && SaveTo_Impl( *pMediumTmp, pArgs ) )
    {
        bSaved = sal_True;

        if( pMediumTmp->GetItemSet() )
        {
            pMediumTmp->GetItemSet()->ClearItem( SID_INTERACTIONHANDLER );
            pMediumTmp->GetItemSet()->ClearItem( SID_PROGRESS_STATUSBAR_CONTROL );
        }

        SetError(pMediumTmp->GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );

        sal_Bool bOpen( sal_False );
        bOpen = DoSaveCompleted( pMediumTmp );

        DBG_ASSERT(bOpen,"Error handling for DoSaveCompleted not implemented");
        (void)bOpen;
    }
    else
    {
        // transfer error code from medium to objectshell
        SetError( pMediumTmp->GetError(), ::rtl::OUString( OSL_LOG_PREFIX  ) );

        // reconnect to object storage
        DoSaveCompleted( 0 );

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

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::Save_Impl( const SfxItemSet* pSet )
{
    if ( IsReadOnly() )
    {
        SetError( ERRCODE_SFX_DOCUMENTREADONLY, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        return sal_False;
    }

    DBG_CHKTHIS(SfxObjectShell, 0);

    pImp->bIsSaving = sal_True;
    sal_Bool bSaved = sal_False;
    SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pSalvageItem, SfxStringItem, SID_DOC_SALVAGE, sal_False);
    if ( pSalvageItem )
    {
        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False);
        String aFilterName;
        const SfxFilter *pFilter = NULL;
        if ( pFilterItem )
            pFilter = SfxFilterMatcher( rtl::OUString::createFromAscii( GetFactory().GetShortName()) ).GetFilter4FilterName( aFilterName );

        SfxMedium *pMed = new SfxMedium(
            pSalvageItem->GetValue(), STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC, pFilter );

        SFX_ITEMSET_ARG( GetMedium()->GetItemSet(), pPasswordItem, SfxStringItem, SID_PASSWORD, sal_False );
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

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::CommonSaveAs_Impl
(
    const INetURLObject&   aURL,
    const String&   aFilterName,
    SfxItemSet*     aParams
)
{
    if( aURL.HasError() )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        return sal_False;
    }

    if ( aURL != INetURLObject( ::rtl::OUString( "private:stream"  ) ) )
    {
        // Is there already a Document with this name?
        SfxObjectShell* pDoc = 0;
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
            SetError(ERRCODE_SFX_ALREADYOPEN, ::rtl::OUString( OSL_LOG_PREFIX  ));
            return sal_False;
        }
    }

    DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "Illegal URL!" );
    DBG_ASSERT( aParams->Count() != 0, "Incorrect Parameter");

    SFX_ITEMSET_ARG( aParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
    sal_Bool bSaveTo = pSaveToItem ? pSaveToItem->GetValue() : sal_False;

    const SfxFilter* pFilter = GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName );
    if ( !pFilter
        || !pFilter->CanExport()
        || (!bSaveTo && !pFilter->CanImport()) )
    {
        SetError( ERRCODE_IO_INVALIDPARAMETER, ::rtl::OUString( OSL_LOG_PREFIX  ) );
        return sal_False;
    }

    SFX_ITEMSET_ARG( aParams, pCopyStreamItem, SfxBoolItem, SID_COPY_STREAM_IF_POSSIBLE, sal_False );
    if ( bSaveTo && pCopyStreamItem && pCopyStreamItem->GetValue() && !IsModified() )
    {
        if ( pMedium->TryDirectTransfer( aURL.GetMainURL( INetURLObject::NO_DECODE ), *aParams ) )
            return sal_True;
    }
    aParams->ClearItem( SID_COPY_STREAM_IF_POSSIBLE );

    pImp->bPasswd = aParams && SFX_ITEM_SET == aParams->GetItemState(SID_PASSWORD);

    SfxMedium *pActMed = GetMedium();
    const INetURLObject aActName(pActMed->GetName());

    sal_Bool bWasReadonly = IsReadOnly();

    if ( aURL == aActName && aURL != INetURLObject( OUString("private:stream") )
        && IsReadOnly() )
    {
        SetError(ERRCODE_SFX_DOCUMENTREADONLY, ::rtl::OUString( OSL_LOG_PREFIX  ));
        return sal_False;
    }

    if( SFX_ITEM_SET != aParams->GetItemState(SID_UNPACK) && SvtSaveOptions().IsSaveUnpacked() )
        aParams->Put( SfxBoolItem( SID_UNPACK, sal_False ) );

    ::rtl::OUString aTempFileURL;
    if ( IsDocShared() )
        aTempFileURL = pMedium->GetURLObject().GetMainURL( INetURLObject::NO_DECODE );

    if ( PreDoSaveAs_Impl(aURL.GetMainURL( INetURLObject::NO_DECODE ),aFilterName,aParams))
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

            SFX_ITEMSET_GET( (*aParams), pFilterItem, SfxStringItem, SID_FILTER_NAME, sal_False );
            if ( pFilterItem )
                pSet->Put( *pFilterItem );

            SFX_ITEMSET_GET( (*aParams), pOptionsItem, SfxStringItem, SID_OPTIONS, sal_False );
            if ( pOptionsItem )
                pSet->Put( *pOptionsItem );

            SFX_ITEMSET_GET( (*aParams), pFilterOptItem, SfxStringItem, SID_FILE_FILTEROPTIONS, sal_False );
            if ( pFilterOptItem )
                pSet->Put( *pFilterOptItem );

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
                    SwitchToShared( sal_True, sal_False );
                }
            }
        }

        if ( bWasReadonly && !bSaveTo )
            Broadcast( SfxSimpleHint(SFX_HINT_MODECHANGED) );

        return sal_True;
    }
    else
        return sal_False;
}

//-------------------------------------------------------------------------

sal_Bool SfxObjectShell::PreDoSaveAs_Impl
(
    const String&   rFileName,
    const String&   aFilterName,
    SfxItemSet*     pParams
)
{
    // copy all items stored in the itemset of the current medium
    SfxAllItemSet* pMergedParams = new SfxAllItemSet( *pMedium->GetItemSet() );

    // in "SaveAs" title and password will be cleared ( maybe the new itemset contains new values, otherwise they will be empty )
    pMergedParams->ClearItem( SID_ENCRYPTIONDATA );
    pMergedParams->ClearItem( SID_PASSWORD );
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
    if( pParams )
        pMergedParams->Put( *pParams );

#ifdef DBG_UTIL
    if ( pMergedParams->GetItemState( SID_DOC_SALVAGE) >= SFX_ITEM_SET )
        OSL_FAIL("Salvage item present in Itemset, check the parameters!");
#endif

    // should be unneccessary - too hot to handle!
    pMergedParams->ClearItem( SID_DOC_SALVAGE );

    // take over the new merged itemset
    pParams = pMergedParams;

    // create a medium for the target URL
    SfxMedium *pNewFile = new SfxMedium( rFileName, STREAM_READWRITE | STREAM_SHARE_DENYWRITE | STREAM_TRUNC, 0, pParams );

    // set filter; if no filter is given, take the default filter of the factory
    if ( aFilterName.Len() )
        pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetFilter4FilterName( aFilterName ) );
    else
        pNewFile->SetFilter( GetFactory().GetFilterContainer()->GetAnyFilter( SFX_FILTER_IMPORT | SFX_FILTER_EXPORT ) );

    if ( pNewFile->GetErrorCode() != ERRCODE_NONE )
    {
        // creating temporary file failed ( f.e. floppy disk not inserted! )
        SetError( pNewFile->GetError(), ::rtl::OUString( OSL_LOG_PREFIX  ) );
        delete pNewFile;
        return sal_False;
    }

    // check if a "SaveTo" is wanted, no "SaveAs"
    SFX_ITEMSET_ARG( pParams, pSaveToItem, SfxBoolItem, SID_SAVETO, sal_False );
    sal_Bool bCopyTo = GetCreateMode() == SFX_CREATE_MODE_EMBEDDED || (pSaveToItem && pSaveToItem->GetValue());

    // distinguish between "Save" and "SaveAs"
    pImp->bIsSaving = sal_False;

    // copy version list from "old" medium to target medium, so it can be used on saving
    if ( pImp->bPreserveVersions )
        pNewFile->TransferVersionList_Impl( *pMedium );

    // Save the document ( first as temporary file, then transfer to the target URL by committing the medium )
    sal_Bool bOk = sal_False;
    if ( !pNewFile->GetErrorCode() && SaveTo_Impl( *pNewFile, NULL ) )
    {
        bOk = sal_True;

        // transfer a possible error from the medium to the document
        SetError( pNewFile->GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );

        // notify the document that saving was done successfully
        if ( !bCopyTo )
        {
            bOk = DoSaveCompleted( pNewFile );
        }
        else
            bOk = DoSaveCompleted(0);

        if( bOk )
        {
            if( !bCopyTo )
                SetModified( sal_False );
        }
        else
        {
            // TODO/LATER: the code below must be dead since the storage commit makes all the stuff
            //       and the DoSaveCompleted call should not be able to fail in general

            DBG_ASSERT( !bCopyTo, "Error while reconnecting to medium, can't be handled!");
            SetError( pNewFile->GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );

            if ( !bCopyTo )
            {
                // reconnect to the old medium
                sal_Bool bRet( sal_False );
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
        SetError( pNewFile->GetErrorCode(), ::rtl::OUString( OSL_LOG_PREFIX  ) );

        // reconnect to the old storage
        DoSaveCompleted( 0 );

        DELETEZ( pNewFile );
    }

    if ( bCopyTo )
        DELETEZ( pNewFile );
    else if( !bOk )
        SetModified( sal_True );

    return bOk;
}

//------------------------------------------------------------------------

sal_Bool SfxObjectShell::LoadFrom( SfxMedium& /*rMedium*/ )
{
    OSL_FAIL( "Base implementation, must not be called in general!" );
    return sal_True;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::IsInformationLost()
{
    Sequence< PropertyValue > aProps = GetModel()->getArgs();
    ::rtl::OUString aFilterName;
    ::rtl::OUString aPreusedFilterName;
    for ( sal_Int32 nInd = 0; nInd < aProps.getLength(); nInd++ )
    {
        if ( aProps[nInd].Name == "FilterName" )
            aProps[nInd].Value >>= aFilterName;
        else if ( aProps[nInd].Name == "PreusedFilterName" )
            aProps[nInd].Value >>= aPreusedFilterName;
    }

    // if current filter can lead to information loss and it was used
    // for the latest store then the user should be asked to store in own format
    if ( !aFilterName.isEmpty() && aFilterName.equals( aPreusedFilterName ) )
    {
        const SfxFilter *pFilt = GetMedium()->GetFilter();
        DBG_ASSERT( pFilt && aFilterName.equals( pFilt->GetName() ), "MediaDescriptor contains wrong filter!\n" );
        return ( pFilt && pFilt->IsAlienFormat() );
    }

    return sal_False;
}

//-------------------------------------------------------------------------
sal_Bool SfxObjectShell::CanReload_Impl()

/*  [Description]

    Internal method for determining whether a reload of the document
    (as RevertToSaved or last known version) is possible.
*/

{
    return pMedium && HasName() && !IsInModalMode() && !pImp->bForbidReload;
}

//-------------------------------------------------------------------------

sal_uInt16 SfxObjectShell::GetHiddenInformationState( sal_uInt16 nStates )
{
    sal_uInt16 nState = 0;
    if ( nStates & HIDDENINFORMATION_DOCUMENTVERSIONS )
    {
        if ( GetMedium()->GetVersionList().getLength() )
            nState |= HIDDENINFORMATION_DOCUMENTVERSIONS;
    }

    return nState;
}

sal_Int16 SfxObjectShell::QueryHiddenInformation( HiddenWarningFact eFact, Window* pParent )
{
    sal_Int16 nRet = RET_YES;
    sal_uInt16 nResId = 0;
    SvtSecurityOptions::EOption eOption = static_cast< SvtSecurityOptions::EOption >( -1 );

    switch ( eFact )
    {
        case WhenSaving :
        {
            nResId = STR_HIDDENINFO_CONTINUE_SAVING;
            eOption = SvtSecurityOptions::E_DOCWARN_SAVEORSEND;
            break;
        }
        case WhenPrinting :
        {
            nResId = STR_HIDDENINFO_CONTINUE_PRINTING;
            eOption = SvtSecurityOptions::E_DOCWARN_PRINT;
            break;
        }
        case WhenSigning :
        {
            nResId = STR_HIDDENINFO_CONTINUE_SIGNING;
            eOption = SvtSecurityOptions::E_DOCWARN_SIGNING;
            break;
        }
        case WhenCreatingPDF :
        {
            nResId = STR_HIDDENINFO_CONTINUE_CREATEPDF;
            eOption = SvtSecurityOptions::E_DOCWARN_CREATEPDF;
            break;
        }
        default:
        {
            SAL_WARN( "sfx2.doc", "SfxObjectShell::DetectHiddenInformation(): what fact?" );
        }
    }

    if ( eOption != -1 && SvtSecurityOptions().IsOptionSet( eOption ) )
    {
        String sMessage( SfxResId(STR_HIDDENINFO_CONTAINS).toString() );
        sal_uInt16 nWantedStates = HIDDENINFORMATION_RECORDEDCHANGES | HIDDENINFORMATION_NOTES;
        if ( eFact != WhenPrinting )
            nWantedStates |= HIDDENINFORMATION_DOCUMENTVERSIONS;
        sal_uInt16 nStates = GetHiddenInformationState( nWantedStates );
        bool bWarning = false;

        if ( ( nStates & HIDDENINFORMATION_RECORDEDCHANGES ) == HIDDENINFORMATION_RECORDEDCHANGES )
        {
            sMessage += SfxResId(STR_HIDDENINFO_RECORDCHANGES).toString();
            sMessage += '\n';
            bWarning = true;
        }
        if ( ( nStates & HIDDENINFORMATION_NOTES ) == HIDDENINFORMATION_NOTES )
        {
            sMessage += SfxResId(STR_HIDDENINFO_NOTES).toString();
            sMessage += '\n';
            bWarning = true;
        }
        if ( ( nStates & HIDDENINFORMATION_DOCUMENTVERSIONS ) == HIDDENINFORMATION_DOCUMENTVERSIONS )
        {
            sMessage += SfxResId(STR_HIDDENINFO_DOCVERSIONS).toString();
            sMessage += '\n';
            bWarning = true;
        }

        if ( bWarning )
        {
            sMessage += '\n';
            sMessage += SfxResId(nResId).toString();
            WarningBox aWBox( pParent, WB_YES_NO | WB_DEF_NO, sMessage );
            nRet = aWBox.Execute();
        }
    }

    return nRet;
}

sal_Bool SfxObjectShell::HasSecurityOptOpenReadOnly() const
{
    return sal_True;
}

sal_Bool SfxObjectShell::IsSecurityOptOpenReadOnly() const
{
    return IsLoadReadonly();
}

void SfxObjectShell::SetSecurityOptOpenReadOnly( sal_Bool _b )
{
    SetLoadReadonly( _b );
}

sal_Bool SfxObjectShell::LoadOwnFormat( SfxMedium& rMedium )
{
    RTL_LOGFILE_PRODUCT_CONTEXT( aLog, "PERFORMANCE SfxObjectShell::LoadOwnFormat" );
    if( RTL_LOGFILE_HASLOGFILE() )
    {
        rtl::OString aString(
            rtl::OUStringToOString(rMedium.GetName(), RTL_TEXTENCODING_ASCII_US));
        RTL_LOGFILE_PRODUCT_CONTEXT_TRACE1(aLog, "loading \"%s\"", aString.getStr());
    }

    uno::Reference< embed::XStorage > xStorage = rMedium.GetStorage();
    if ( xStorage.is() )
    {
        // Password
        SFX_ITEMSET_ARG( rMedium.GetItemSet(), pPasswdItem, SfxStringItem, SID_PASSWORD, sal_False );
        if ( pPasswdItem || ERRCODE_IO_ABORT != CheckPasswd_Impl( this, SFX_APP()->GetPool(), pMedium ) )
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
        return sal_False;
    }
    else
        return sal_False;
}

sal_Bool SfxObjectShell::SaveAsOwnFormat( SfxMedium& rMedium )
{
    uno::Reference< embed::XStorage > xStorage = rMedium.GetStorage();
    if( xStorage.is() )
    {
        sal_Int32 nVersion = rMedium.GetFilter()->GetVersion();

        // OASIS templates have own mediatypes ( SO7 also actually, but it is to late to use them here )
        sal_Bool bTemplate = ( rMedium.GetFilter()->IsOwnTemplateFormat() && nVersion > SOFFICE_FILEFORMAT_60 );

        SetupStorage( xStorage, nVersion, bTemplate );
#ifndef DISABLE_SCRIPTING
        if ( HasBasic() )
        {
            // Initialize Basic
            GetBasicManager();

            // Save dialog/script container
            pImp->pBasicManager->storeLibrariesToStorage( xStorage );
        }
#endif
        return SaveAs( rMedium );
    }
    else return sal_False;
}

uno::Reference< embed::XStorage > SfxObjectShell::GetStorage()
{
    if ( !pImp->m_xDocStorage.is() )
    {
        OSL_ENSURE( pImp->m_bCreateTempStor, "The storage must exist already!\n" );
        try {
            // no notification is required the storage is set the first time
            pImp->m_xDocStorage = ::comphelper::OStorageHelper::GetTemporaryStorage();
            OSL_ENSURE( pImp->m_xDocStorage.is(), "The method must either return storage or throw an exception!" );

            SetupStorage( pImp->m_xDocStorage, SOFFICE_FILEFORMAT_CURRENT, sal_False );
            pImp->m_bCreateTempStor = sal_False;
            SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_STORAGECHANGED, GlobalEventConfig::GetEventName(STR_EVENT_STORAGECHANGED), this ) );
        }
        catch( uno::Exception& )
        {
            // TODO/LATER: error handling?
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    OSL_ENSURE( pImp->m_xDocStorage.is(), "The document storage must be created!" );
    return pImp->m_xDocStorage;
}


sal_Bool SfxObjectShell::SaveChildren( sal_Bool bObjectsOnly )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveChildren" );

    sal_Bool bResult = sal_True;
    if ( pImp->mpObjectContainer )
    {
        sal_Bool bOasis = ( SotStorage::GetVersion( GetStorage() ) > SOFFICE_FILEFORMAT_60 );
        GetEmbeddedObjectContainer().StoreChildren(bOasis,bObjectsOnly);
    }

    return bResult;
}

sal_Bool SfxObjectShell::SaveAsChildren( SfxMedium& rMedium )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveAsChildren" );

    sal_Bool bResult = sal_True;

    uno::Reference < embed::XStorage > xStorage = rMedium.GetStorage();
    if ( !xStorage.is() )
        return sal_False;

    if ( xStorage == GetStorage() )
        return SaveChildren();

    sal_Bool bOasis = sal_True;
    if ( pImp->mpObjectContainer )
    {
        bOasis = ( SotStorage::GetVersion( xStorage ) > SOFFICE_FILEFORMAT_60 );
        GetEmbeddedObjectContainer().StoreAsChildren(bOasis,SFX_CREATE_MODE_EMBEDDED == eCreateMode,xStorage);
    }

    if ( bResult )
        bResult = CopyStoragesOfUnknownMediaType( GetStorage(), xStorage );

    return bResult;
}

sal_Bool SfxObjectShell::SaveCompletedChildren( sal_Bool bSuccess )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveCompletedChildren" );

    sal_Bool bResult = sal_True;

    if ( pImp->mpObjectContainer )
    {
        uno::Sequence < ::rtl::OUString > aNames = GetEmbeddedObjectContainer().GetObjectNames();
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
                        xPersist->saveCompleted( bSuccess );
                    }
                    catch( uno::Exception& )
                    {
                        // TODO/LATER: error handling
                        bResult = sal_False;
                        break;
                    }
                }
            }
        }
    }

    return bResult;
}

sal_Bool SfxObjectShell::SwitchChildrenPersistance( const uno::Reference< embed::XStorage >& xStorage,
                                                    sal_Bool bForceNonModified )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SwitchChildrenPersistence" );

    if ( !xStorage.is() )
    {
        // TODO/LATER: error handling
        return sal_False;
    }

    sal_Bool bResult = sal_True;

    if ( pImp->mpObjectContainer )
        pImp->mpObjectContainer->SetPersistentEntries(xStorage,bForceNonModified);

    return bResult;
}

// Never call this method directly, always use the DoSaveCompleted call
sal_Bool SfxObjectShell::SaveCompleted( const uno::Reference< embed::XStorage >& xStorage )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SaveCompleted" );

    sal_Bool bResult = sal_False;
    sal_Bool bSendNotification = sal_False;
    uno::Reference< embed::XStorage > xOldStorageHolder;

#ifdef DBG_UTIL
    // check for wrong creation of object container
    sal_Bool bHasContainer = ( pImp->mpObjectContainer != 0 );
#endif

    if ( !xStorage.is() || xStorage == GetStorage() )
    {
        // no persistence change
        bResult = SaveCompletedChildren( sal_False );
    }
    else
    {
        if ( pImp->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( xStorage );

        bResult = SwitchChildrenPersistance( xStorage, sal_True );
    }

    if ( bResult )
    {
        if ( xStorage.is() && pImp->m_xDocStorage != xStorage )
        {
            // make sure that until the storage is assigned the object
            // container is not created by accident!
            DBG_ASSERT( bHasContainer == (pImp->mpObjectContainer != 0), "Wrong storage in object container!" );
            xOldStorageHolder = pImp->m_xDocStorage;
            pImp->m_xDocStorage = xStorage;
            bSendNotification = sal_True;

            if ( IsEnableSetModified() )
                SetModified( sal_False );
        }
    }
    else
    {
        if ( pImp->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( pImp->m_xDocStorage );

        // let already successfully connected objects be switched back
        SwitchChildrenPersistance( pImp->m_xDocStorage, sal_True );
    }

    if ( bSendNotification )
    {
        SFX_APP()->NotifyEvent( SfxEventHint( SFX_EVENT_STORAGECHANGED, GlobalEventConfig::GetEventName(STR_EVENT_STORAGECHANGED), this ) );
    }

    return bResult;
}

#if OSL_DEBUG_LEVEL > 0
sal_Bool StoragesOfUnknownMediaTypeAreCopied_Impl( const uno::Reference< embed::XStorage >& xSource,
                                                   const uno::Reference< embed::XStorage >& xTarget )
{
    OSL_ENSURE( xSource.is() && xTarget.is(), "Source and/or target storages are not available!\n" );
    if ( !xSource.is() || !xTarget.is() || xSource == xTarget )
        return sal_True;

    try
    {
        uno::Sequence< ::rtl::OUString > aSubElements = xSource->getElementNames();
        for ( sal_Int32 nInd = 0; nInd < aSubElements.getLength(); nInd++ )
        {
            if ( xSource->isStorageElement( aSubElements[nInd] ) )
            {
                ::rtl::OUString aMediaType;
                ::rtl::OUString aMediaTypePropName( "MediaType"  );
                sal_Bool bGotMediaType = sal_False;

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
                    bGotMediaType = ( xProps->getPropertyValue( aMediaTypePropName ) >>= aMediaType );
                }

                // TODO/LATER: there should be a way to detect whether an object with such a MediaType can exist
                //             probably it should be placed in the MimeType-ClassID table or in standalone table
                if ( !aMediaType.isEmpty()
                  && aMediaType.compareToAscii( "application/vnd.sun.star.oleobject" ) != COMPARE_EQUAL )
                {
                    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
                    aDataFlavor.MimeType = aMediaType;
                    sal_uInt32 nFormat = SotExchange::GetFormat( aDataFlavor );

                    switch ( nFormat )
                    {
                        case SOT_FORMATSTR_ID_STARWRITER_60 :
                        case SOT_FORMATSTR_ID_STARWRITERWEB_60 :
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_60 :
                        case SOT_FORMATSTR_ID_STARDRAW_60 :
                        case SOT_FORMATSTR_ID_STARIMPRESS_60 :
                        case SOT_FORMATSTR_ID_STARCALC_60 :
                        case SOT_FORMATSTR_ID_STARCHART_60 :
                        case SOT_FORMATSTR_ID_STARMATH_60 :
                        case SOT_FORMATSTR_ID_STARWRITER_8:
                        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
                        case SOT_FORMATSTR_ID_STARDRAW_8:
                        case SOT_FORMATSTR_ID_STARIMPRESS_8:
                        case SOT_FORMATSTR_ID_STARCALC_8:
                        case SOT_FORMATSTR_ID_STARCHART_8:
                        case SOT_FORMATSTR_ID_STARMATH_8:
                            break;

                        default:
                        {
                            if ( !xTarget->hasByName( aSubElements[nInd] ) )
                                return sal_False;
                        }
                    }
                }
            }
        }
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( "Cant check storage consistency!\n" );
    }

    return sal_True;
}
#endif

sal_Bool SfxObjectShell::SwitchPersistance( const uno::Reference< embed::XStorage >& xStorage )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::SwitchPersistance" );

    sal_Bool bResult = sal_False;
#ifdef DBG_UTIL
    // check for wrong creation of object container
    sal_Bool bHasContainer = ( pImp->mpObjectContainer != 0 );
#endif
    if ( xStorage.is() )
    {
        if ( pImp->mpObjectContainer )
            GetEmbeddedObjectContainer().SwitchPersistence( xStorage );
        bResult = SwitchChildrenPersistance( xStorage );

        // TODO/LATER: substorages that have unknown mimetypes probably should be copied to the target storage here
        OSL_ENSURE( StoragesOfUnknownMediaTypeAreCopied_Impl( pImp->m_xDocStorage, xStorage ),
                    "Some of substorages with unknown mimetypes is lost!" );
    }

    if ( bResult )
    {
        // make sure that until the storage is assigned the object container is not created by accident!
        DBG_ASSERT( bHasContainer == (pImp->mpObjectContainer != 0), "Wrong storage in object container!" );
        if ( pImp->m_xDocStorage != xStorage )
            DoSaveCompleted( new SfxMedium( xStorage, GetMedium()->GetBaseURL() ) );

        if ( IsEnableSetModified() )
            SetModified( sal_True ); // ???
    }

    return bResult;
}

sal_Bool SfxObjectShell::CopyStoragesOfUnknownMediaType( const uno::Reference< embed::XStorage >& xSource,
                                                         const uno::Reference< embed::XStorage >& xTarget )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::CopyStoragesOfUnknownMediaType" );

    // This method does not commit the target storage and should not do it
    sal_Bool bResult = sal_True;

    try
    {
        uno::Sequence< ::rtl::OUString > aSubElements = xSource->getElementNames();
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
                ::rtl::OUString aMediaType;
                ::rtl::OUString aMediaTypePropName( "MediaType"  );
                sal_Bool bGotMediaType = sal_False;

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
                    bGotMediaType = ( xProps->getPropertyValue( aMediaTypePropName ) >>= aMediaType );
                }

                // TODO/LATER: there should be a way to detect whether an object with such a MediaType can exist
                //             probably it should be placed in the MimeType-ClassID table or in standalone table
                if ( !aMediaType.isEmpty()
                  && aMediaType.compareToAscii( "application/vnd.sun.star.oleobject" ) != COMPARE_EQUAL )
                {
                    ::com::sun::star::datatransfer::DataFlavor aDataFlavor;
                    aDataFlavor.MimeType = aMediaType;
                    sal_uInt32 nFormat = SotExchange::GetFormat( aDataFlavor );

                    switch ( nFormat )
                    {
                        case SOT_FORMATSTR_ID_STARWRITER_60 :
                        case SOT_FORMATSTR_ID_STARWRITERWEB_60 :
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_60 :
                        case SOT_FORMATSTR_ID_STARDRAW_60 :
                        case SOT_FORMATSTR_ID_STARIMPRESS_60 :
                        case SOT_FORMATSTR_ID_STARCALC_60 :
                        case SOT_FORMATSTR_ID_STARCHART_60 :
                        case SOT_FORMATSTR_ID_STARMATH_60 :
                        case SOT_FORMATSTR_ID_STARWRITER_8:
                        case SOT_FORMATSTR_ID_STARWRITERWEB_8:
                        case SOT_FORMATSTR_ID_STARWRITERGLOB_8:
                        case SOT_FORMATSTR_ID_STARDRAW_8:
                        case SOT_FORMATSTR_ID_STARIMPRESS_8:
                        case SOT_FORMATSTR_ID_STARCALC_8:
                        case SOT_FORMATSTR_ID_STARCHART_8:
                        case SOT_FORMATSTR_ID_STARMATH_8:
                            break;

                        default:
                        {
                            OSL_ENSURE( aSubElements[nInd] == "Configurations2" || !xTarget->hasByName( aSubElements[nInd] ),
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
        bResult = sal_False;
        // TODO/LATER: a specific error could be provided
    }

    return bResult;
}

sal_Bool SfxObjectShell::GenerateAndStoreThumbnail( sal_Bool bEncrypted,
                                                    sal_Bool bSigned,
                                                    sal_Bool bIsTemplate,
                                                    const uno::Reference< embed::XStorage >& xStor )
{
    RTL_LOGFILE_CONTEXT( aLog, "sfx2 (mv76033) SfxObjectShell::GenerateAndStoreThumbnail" );

    sal_Bool bResult = sal_False;

    try {
        uno::Reference< embed::XStorage > xThumbnailStor =
                                        xStor->openStorageElement( ::rtl::OUString("Thumbnails"),
                                                                    embed::ElementModes::READWRITE );
        if ( xThumbnailStor.is() )
        {
            uno::Reference< io::XStream > xStream = xThumbnailStor->openStreamElement(
                                                        ::rtl::OUString("thumbnail.png"),
                                                        embed::ElementModes::READWRITE );

            if ( xStream.is() && WriteThumbnail( bEncrypted, bSigned, bIsTemplate, xStream ) )
            {
                uno::Reference< embed::XTransactedObject > xTransact( xThumbnailStor, uno::UNO_QUERY_THROW );
                xTransact->commit();
                bResult = sal_True;
            }
        }
    }
    catch( uno::Exception& )
    {
    }

    return bResult;
}

sal_Bool SfxObjectShell::WriteThumbnail( sal_Bool bEncrypted,
                                         sal_Bool bSigned,
                                         sal_Bool bIsTemplate,
                                         const uno::Reference< io::XStream >& xStream )
{
    sal_Bool bResult = sal_False;

    if ( xStream.is() )
    {
        try {
            uno::Reference< io::XTruncate > xTruncate( xStream->getOutputStream(), uno::UNO_QUERY_THROW );
            xTruncate->truncate();

            uno::Reference < beans::XPropertySet > xSet( xStream, uno::UNO_QUERY );
            if ( xSet.is() )
                xSet->setPropertyValue( ::rtl::OUString("MediaType"),
                                        uno::makeAny( ::rtl::OUString("image/png") ) );
            if ( bEncrypted )
            {
                sal_uInt16 nResID = GraphicHelper::getThumbnailReplacementIDByFactoryName_Impl(
                                        ::rtl::OUString::createFromAscii( GetFactory().GetShortName() ),
                                        bIsTemplate );
                if ( nResID )
                {
                    if ( !bSigned )
                    {
                        bResult = GraphicHelper::getThumbnailReplacement_Impl( nResID, xStream );
                    }
                    else
                    {
                        // retrieve the bitmap and write a signature bitmap over it
                        SfxResId aResId( nResID );
                        BitmapEx aThumbBitmap( aResId );
                        bResult = GraphicHelper::getSignedThumbnailFormatFromBitmap_Impl( aThumbBitmap, xStream );
                    }
                }
            }
            else
            {
                ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                    GetPreviewMetaFile( sal_False );
                if ( pMetaFile )
                {
                    bResult = GraphicHelper::getThumbnailFormatFromGDI_Impl(
                                pMetaFile.get(), bSigned, xStream );
                }
            }
        }
        catch( uno::Exception& )
        {}
    }

    return bResult;
}

void SfxObjectShell::UpdateLinks()
{
}

void SfxObjectShell::CheckConfigOptions()
{
    // not handled.  Each app's shell needs to overwrite this method to add handler.
    SetConfigOptionsChecked(true);
}

sal_Bool SfxObjectShell::IsConfigOptionsChecked() const
{
    return pImp->m_bConfigOptionsChecked;
}

void SfxObjectShell::SetConfigOptionsChecked( sal_Bool bChecked )
{
    pImp->m_bConfigOptionsChecked = bChecked;
}

sal_Bool SfxObjectShell::QuerySaveSizeExceededModules_Impl( const uno::Reference< task::XInteractionHandler >& xHandler )
{
#ifdef DISABLE_SCRIPTING
    (void) xHandler;
#else
    if ( !HasBasic() )
        return sal_True;

    if ( !pImp->pBasicManager->isValid() )
        GetBasicManager();
    uno::Sequence< rtl::OUString > sModules;
    if ( xHandler.is() )
    {
        if( pImp->pBasicManager->LegacyPsswdBinaryLimitExceeded( sModules ) )
        {
            ModuleSizeExceeded* pReq =  new ModuleSizeExceeded( sModules );
            uno::Reference< task::XInteractionRequest > xReq( pReq );
            xHandler->handle( xReq );
            return pReq->isApprove();
        }
    }
#endif
    // No interaction handler, default is to continue to save
    return sal_True;
}
// -----------------------------------------------------------------------------
uno::Reference< task::XInteractionHandler > SfxObjectShell::getInteractionHandler() const
{
    uno::Reference< task::XInteractionHandler > xRet;
    if ( GetMedium() )
        xRet = GetMedium()->GetInteractionHandler();
    return xRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
