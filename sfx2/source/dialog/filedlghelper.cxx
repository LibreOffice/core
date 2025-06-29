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

#include <memory>
#include <optional>
#include <string_view>
#include <o3tl/test_info.hxx>

#include <sfx2/filedlghelper.hxx>
#include <sal/types.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/FolderPicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XControlInformation.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/util/RevisionTag.hpp>
#include <comphelper/fileurl.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <vcl/weld.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/securityoptions.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/filteroptions_settings.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/helpids.h>
#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxsids.hrc>
#include <guisaveas.hxx>
#include "filtergrouping.hxx"
#include "filedlgimpl.hxx"
#include <fpicker/fpsofficeResMgr.hxx>
#include <fpicker/strings.hrc>
#include <sfx2/strings.hrc>
#include <sal/log.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>
#include <officecfg/Office/Common.hxx>

#include <config_gpgme.h>
#if HAVE_FEATURE_GPGME
# include <com/sun/star/xml/crypto/SEInitializer.hpp>
# include <com/sun/star/xml/crypto/GPGSEInitializer.hpp>
# include <com/sun/star/xml/crypto/XXMLSecurityContext.hpp>
#endif
#include <comphelper/xmlsechelper.hxx>
#include <unotools/useroptions.hxx>

#ifdef UNX
#include <errno.h>
#include <sys/stat.h>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::ui::dialogs::TemplateDescription;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::cppu;

constexpr OUString IODLG_CONFIGNAME = u"FilePicker_Save"_ustr;
constexpr OUString IMPGRF_CONFIGNAME = u"FilePicker_Graph"_ustr;
constexpr OUString USERITEM_NAME = u"UserItem"_ustr;

namespace sfx2
{

namespace
{
    bool lclSupportsOOXMLEncryption(std::u16string_view aFilterName)
    {
        return  aFilterName == u"Calc MS Excel 2007 XML"
                ||  aFilterName == u"MS Word 2007 XML"
                ||  aFilterName == u"Impress MS PowerPoint 2007 XML"
                ||  aFilterName == u"Impress MS PowerPoint 2007 XML AutoPlay"
                ||  aFilterName == u"Calc Office Open XML"
                ||  aFilterName == u"Impress Office Open XML"
                ||  aFilterName == u"Impress Office Open XML AutoPlay"
                ||  aFilterName == u"Office Open XML Text";
    }
}

static std::optional<OUString> GetLastFilterConfigId( FileDialogHelper::Context _eContext )
{
    static constexpr OUStringLiteral aSD_EXPORT_IDENTIFIER(u"SdExportLastFilter");
    static constexpr OUStringLiteral aSI_EXPORT_IDENTIFIER(u"SiExportLastFilter");
    static constexpr OUStringLiteral aSW_EXPORT_IDENTIFIER(u"SwExportLastFilter");

    switch( _eContext )
    {
        case FileDialogHelper::DrawExport: return aSD_EXPORT_IDENTIFIER;
        case FileDialogHelper::ImpressExport: return aSI_EXPORT_IDENTIFIER;
        case FileDialogHelper::WriterExport: return aSW_EXPORT_IDENTIFIER;
        default: break;
    }

    return {};
}

static OUString EncodeSpaces_Impl( const OUString& rSource );
static OUString DecodeSpaces_Impl( const OUString& rSource );

// FileDialogHelper_Impl

// XFilePickerListener Methods
void SAL_CALL FileDialogHelper_Impl::fileSelectionChanged( const FilePickerEvent& )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->FileSelectionChanged();
}

void SAL_CALL FileDialogHelper_Impl::directoryChanged( const FilePickerEvent& )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->DirectoryChanged();
}

OUString SAL_CALL FileDialogHelper_Impl::helpRequested( const FilePickerEvent& aEvent )
{
    SolarMutexGuard aGuard;
    return sfx2::FileDialogHelper::HelpRequested( aEvent );
}

void SAL_CALL FileDialogHelper_Impl::controlStateChanged( const FilePickerEvent& aEvent )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->ControlStateChanged( aEvent );
}

void SAL_CALL FileDialogHelper_Impl::dialogSizeChanged()
{
    SolarMutexGuard aGuard;
    mpAntiImpl->DialogSizeChanged();
}

// XDialogClosedListener Methods
void SAL_CALL FileDialogHelper_Impl::dialogClosed( const DialogClosedEvent& _rEvent )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->DialogClosed( _rEvent );
    postExecute( _rEvent.DialogResult );
}

// handle XFilePickerListener events
void FileDialogHelper_Impl::handleFileSelectionChanged()
{
    if ( mbHasVersions )
        updateVersions();

    if ( mbShowPreview )
        maPreviewIdle.Start();
}

void FileDialogHelper_Impl::handleDirectoryChanged()
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( nullptr );
}

OUString FileDialogHelper_Impl::handleHelpRequested( const FilePickerEvent& aEvent )
{
    TranslateId aHelpId;

    // mapping from element id -> help id
    switch ( aEvent.ElementId )
    {
        case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION :
            aHelpId = STR_FILESAVE_AUTOEXTENSION;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
            aHelpId = STR_FILESAVE_SAVEWITHPASSWORD;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
            aHelpId = STR_FILESAVE_CUSTOMIZEFILTER;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_READONLY :
            aHelpId = STR_FILEOPEN_READONLY;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_LINK :
            aHelpId = STR_FILEDLG_LINK_CB;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW :
            aHelpId = STR_FILEDLG_PREVIEW_CB;
            break;

        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY :
            aHelpId = STR_FILESAVE_DOPLAY;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_VERSION :
            aHelpId = STR_FILEOPEN_VERSION;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE :
            aHelpId = STR_FILESAVE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE :
            aHelpId = STR_FILEOPEN_IMAGE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR :
            aHelpId = STR_FILEOPEN_IMAGE_ANCHOR;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            aHelpId = STR_FILESAVE_SELECTION;
            break;

        default:
            SAL_WARN( "sfx.dialog", "invalid element id" );
    }

    return aHelpId ? FpsResId(aHelpId) : OUString();
}

void FileDialogHelper_Impl::handleControlStateChanged( const FilePickerEvent& aEvent )
{
    switch ( aEvent.ElementId )
    {
        case CommonFilePickerElementIds::LISTBOX_FILTER:
            updateFilterOptionsBox();
            enablePasswordBox( false );
            enableGpgEncrBox( false );
            updateSelectionBox();
            updateSignByDefault();
            // only use it for export and with our own dialog
            if ( mbExport && !mbSystemPicker )
                updateExportButton();
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            updatePreviewState(true);
            break;
    }
}

void FileDialogHelper_Impl::handleDialogSizeChanged()
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( nullptr );
}

// XEventListener Methods
void SAL_CALL FileDialogHelper_Impl::disposing( const EventObject& )
{
    SolarMutexGuard aGuard;
    dispose();
}

void FileDialogHelper_Impl::dispose()
{
    if ( mxFileDlg.is() )
    {
        // remove the event listener
        mxFileDlg->removeFilePickerListener( this );

        ::comphelper::disposeComponent( mxFileDlg );
        mxFileDlg.clear();
    }
}

OUString FileDialogHelper_Impl::getCurrentFilterUIName() const
{
    OUString aFilterName;

    if( mxFileDlg.is() )
    {
        aFilterName = mxFileDlg->getCurrentFilter();

        if ( !aFilterName.isEmpty() && isShowFilterExtensionEnabled() )
            aFilterName = getFilterName( aFilterName );
    }

    return aFilterName;
}

void FileDialogHelper_Impl::LoadLastUsedFilter( const OUString& _rContextIdentifier )
{
    SvtViewOptions aDlgOpt( EViewType::Dialog, IODLG_CONFIGNAME );

    if( aDlgOpt.Exists() )
    {
        OUString    aLastFilter;
        if( aDlgOpt.GetUserItem( _rContextIdentifier ) >>= aLastFilter )
            setFilter( aLastFilter );
    }
}

void FileDialogHelper_Impl::SaveLastUsedFilter()
{
    std::optional<OUString> pConfigId = GetLastFilterConfigId( meContext );
    if( pConfigId )
        SvtViewOptions( EViewType::Dialog, IODLG_CONFIGNAME ).SetUserItem( *pConfigId,
                            Any( getFilterWithExtension( getFilter() ) ) );
}

std::shared_ptr<const SfxFilter> FileDialogHelper_Impl::getCurrentSfxFilter()
{
    OUString aFilterName = getCurrentFilterUIName();

    if ( mpMatcher && !aFilterName.isEmpty() )
        return mpMatcher->GetFilter4UIName( aFilterName, m_nMustFlags, m_nDontFlags );

    return nullptr;
}

bool FileDialogHelper_Impl::updateExtendedControl( sal_Int16 _nExtendedControlId, bool _bEnable )
{
    bool bIsEnabled = false;

    uno::Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
    if ( xCtrlAccess.is() )
    {
        try
        {
            xCtrlAccess->enableControl( _nExtendedControlId, _bEnable );
            bIsEnabled = _bEnable;
        }
        catch( const IllegalArgumentException& )
        {
            TOOLS_WARN_EXCEPTION( "sfx", "FileDialogHelper_Impl::updateExtendedControl" );
        }
    }
    return bIsEnabled;
}

bool FileDialogHelper_Impl::CheckFilterOptionsCapability( const std::shared_ptr<const SfxFilter>& _pFilter )
{
    bool bResult = false;

    if( mxFilterCFG.is() && _pFilter )
    {
        try
        {
            Sequence < PropertyValue > aProps;
            Any aAny = mxFilterCFG->getByName( _pFilter->GetName() );
            if ( aAny >>= aProps )
            {
                OUString aServiceName;
                for (const auto& rProp : aProps)
                {
                    if( rProp.Name == "UIComponent" )
                    {
                        rProp.Value >>= aServiceName;
                        if( !aServiceName.isEmpty() )
                            bResult = true;
                    }
                }
            }
        }
        catch( const Exception& )
        {
        }
    }

    return bResult;
}

bool FileDialogHelper_Impl::isInOpenMode() const
{
    bool bRet = false;

    switch ( m_nDialogType )
    {
        case FILEOPEN_SIMPLE:
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
        case FILEOPEN_PLAY:
        case FILEOPEN_LINK_PLAY:
        case FILEOPEN_READONLY_VERSION:
        case FILEOPEN_READONLY_VERSION_FILTEROPTIONS:
        case FILEOPEN_LINK_PREVIEW:
        case FILEOPEN_PREVIEW:
            bRet = true;
    }

    return bRet;
}

void FileDialogHelper_Impl::updateFilterOptionsBox()
{
    if ( !m_bHaveFilterOptions )
        return;

    bool bFilterOptionsEnabled = CheckFilterOptionsCapability(getCurrentSfxFilter());

    updateExtendedControl(ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS,
                          bFilterOptionsEnabled);

    if (isInOpenMode())
    {
        if (auto xCtrlAccess = mxFileDlg.query<XFilePickerControlAccess>())
        {
            OUString filter;
            getRealFilter(filter);
            bool bChecked = bFilterOptionsEnabled && utl::isShowFilterOptionsDialog(filter);
            xCtrlAccess->setValue(ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS, 0,
                                  Any(bChecked));
        }
    }
}

void FileDialogHelper_Impl::updateExportButton()
{
    uno::Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
    if ( !xCtrlAccess.is() )
        return;

    OUString sOldLabel( xCtrlAccess->getLabel( CommonFilePickerElementIds::PUSHBUTTON_OK ) );

    // initialize button label; we need the label with the mnemonic char
    if ( maButtonLabel.isEmpty() || maButtonLabel.indexOf( MNEMONIC_CHAR ) == -1 )
    {
        // cut the ellipses, if necessary
        sal_Int32 nIndex = sOldLabel.indexOf( "..." );
        if ( -1 == nIndex )
            nIndex = sOldLabel.getLength();
        maButtonLabel = sOldLabel.copy( 0, nIndex );
    }

    OUString sLabel = maButtonLabel;
    // filter with options -> append ellipses on export button label
    if ( CheckFilterOptionsCapability( getCurrentSfxFilter() ) )
        sLabel += "...";

    if ( sOldLabel != sLabel )
    {
        try
        {
            xCtrlAccess->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK, sLabel );
        }
        catch( const IllegalArgumentException& )
        {
            TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::updateExportButton" );
        }
    }
}

void FileDialogHelper_Impl::updateSelectionBox()
{
    if ( !mbHasSelectionBox )
        return;

    // Does the selection box exist?
    bool bSelectionBoxFound = false;
    uno::Reference< XControlInformation > xCtrlInfo( mxFileDlg, UNO_QUERY );
    if ( xCtrlInfo.is() )
    {
        Sequence< OUString > aCtrlList = xCtrlInfo->getSupportedControls();
        bSelectionBoxFound = comphelper::findValue(aCtrlList, "SelectionBox") != -1;
    }

    if ( bSelectionBoxFound )
    {
        std::shared_ptr<const SfxFilter> pFilter = getCurrentSfxFilter();
        mbSelectionFltrEnabled = updateExtendedControl(
            ExtendedFilePickerElementIds::CHECKBOX_SELECTION,
            ( mbSelectionEnabled && pFilter && ( pFilter->GetFilterFlags() & SfxFilterFlags::SUPPORTSSELECTION ) ) );
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0, Any( mbSelection ) );
    }
}

void FileDialogHelper_Impl::updateSignByDefault()
{
#if HAVE_FEATURE_GPGME
    if (!mbHasSignByDefault)
        return;

    OUString aSigningKey = SvtUserOptions{}.GetSigningKey();
    updateExtendedControl(ExtendedFilePickerElementIds::CHECKBOX_GPGSIGN, !aSigningKey.isEmpty());
#endif
}

void FileDialogHelper_Impl::enablePasswordBox( bool bInit )
{
    if ( ! mbHasPassword )
        return;

    // in case of initialization assume previous state to be false
    bool bWasEnabled = !bInit && mbIsPwdEnabled;

    std::shared_ptr<const SfxFilter> pCurrentFilter = getCurrentSfxFilter();
    mbIsPwdEnabled = updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_PASSWORD,
        pCurrentFilter && ( pCurrentFilter->GetFilterFlags() & SfxFilterFlags::ENCRYPTION )
    );

    if( !bWasEnabled && mbIsPwdEnabled )
    {
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        if( mbPwdCheckBoxState )
            xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, Any( true ) );
    }
    else if( bWasEnabled && !mbIsPwdEnabled )
    {
        // remember user settings until checkbox is enabled
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
        bool bPassWord = false;
        mbPwdCheckBoxState = ( aValue >>= bPassWord ) && bPassWord;
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, Any( false ) );
    }
}

void FileDialogHelper_Impl::enableGpgEncrBox( bool bInit )
{
    if ( ! mbHasPassword )  // CHECKBOX_GPGENCRYPTION is visible if CHECKBOX_PASSWORD is visible
        return;

    // in case of initialization assume previous state to be false
    bool bWasEnabled = !bInit && mbIsGpgEncrEnabled;

    std::shared_ptr<const SfxFilter> pCurrentFilter = getCurrentSfxFilter();
    mbIsGpgEncrEnabled = updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION,
        pCurrentFilter && ( pCurrentFilter->GetFilterFlags() & SfxFilterFlags::GPGENCRYPTION )
    );

    if( !bWasEnabled && mbIsGpgEncrEnabled )
    {
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        if( mbGpgCheckBoxState )
            xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION, 0, Any( true ) );
    }
    else if( bWasEnabled && !mbIsGpgEncrEnabled )
    {
        // remember user settings until checkbox is enabled
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION, 0 );
        bool bGpgEncryption = false;
        mbGpgCheckBoxState = ( aValue >>= bGpgEncryption ) && bGpgEncryption;
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION, 0, Any( false ) );
    }
}

void FileDialogHelper_Impl::updatePreviewState( bool _bUpdatePreviewWindow )
{
    if ( !mbHasPreview )
        return;

    uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

    // check, whether or not we have to display a preview
    if ( !xCtrlAccess.is() )
        return;

    try
    {
        Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
        bool bShowPreview = false;

        if ( aValue >>= bShowPreview )
        {
            mbShowPreview = bShowPreview;

            // setShowState has currently no effect for the
            // OpenOffice FilePicker (see svtools/source/filepicker/iodlg.cxx)
            uno::Reference< XFilePreview > xFilePreview( mxFileDlg, UNO_QUERY );
            if ( xFilePreview.is() )
                xFilePreview->setShowState( mbShowPreview );

            if ( _bUpdatePreviewWindow )
                TimeOutHdl_Impl( nullptr );
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::updatePreviewState" );
    }
}

void FileDialogHelper_Impl::updateVersions()
{
    Sequence < OUString > aEntries;
    Sequence < OUString > aPathSeq = mxFileDlg->getSelectedFiles();

    if ( aPathSeq.getLength() == 1 )
    {
        INetURLObject aObj( aPathSeq[0] );

        if ( ( aObj.GetProtocol() == INetProtocol::File ) &&
            ( utl::UCBContentHelper::IsDocument( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) ) )
        {
            try
            {
                uno::Reference< embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                                                                aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                                                embed::ElementModes::READ );

                DBG_ASSERT( xStorage.is(), "The method must return the storage or throw exception!" );
                if ( !xStorage.is() )
                    throw uno::RuntimeException();

                const uno::Sequence < util::RevisionTag > xVersions = SfxMedium::GetVersionList( xStorage );

                aEntries.realloc( xVersions.getLength() + 1 );
                aEntries.getArray()[0] = SfxResId( STR_SFX_FILEDLG_ACTUALVERSION );

                std::transform(xVersions.begin(), xVersions.end(), std::next(aEntries.getArray()),
                    [](const util::RevisionTag& rVersion) -> OUString { return rVersion.Identifier; });
            }
            catch( const uno::Exception& )
            {
            }
        }
    }

    uno::Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    try
    {
        xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                        ControlActions::DELETE_ITEMS, aValue );
    }
    catch( const IllegalArgumentException& ){}

    if ( !aEntries.hasElements() )
        return;

    try
    {
        aValue <<= aEntries;
        xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                        ControlActions::ADD_ITEMS, aValue );

        Any aPos;
        aPos <<= sal_Int32(0);
        xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                        ControlActions::SET_SELECT_ITEM, aPos );
    }
    catch( const IllegalArgumentException& ){}
}

IMPL_LINK_NOARG(FileDialogHelper_Impl, TimeOutHdl_Impl, Timer *, void)
{
    if ( !mbHasPreview )
        return;

    maGraphic.Clear();

    Any aAny;
    uno::Reference < XFilePreview > xFilePicker( mxFileDlg, UNO_QUERY );

    if ( ! xFilePicker.is() )
        return;

    Sequence < OUString > aPathSeq = mxFileDlg->getSelectedFiles();

    if ( mbShowPreview && ( aPathSeq.getLength() == 1 ) )
    {
        const OUString&    aURL = aPathSeq[0];

        if ( ERRCODE_NONE == getGraphic( aURL, maGraphic ) )
        {
            // changed the code slightly;
            // before: the bitmap was scaled and
            // surrounded a white frame
            // now: the bitmap will only be scaled
            // and the filepicker implementation
            // is responsible for placing it at its
            // proper position and painting a frame

            BitmapEx aBmp = maGraphic.GetBitmapEx();
            if ( !aBmp.IsEmpty() )
            {
                // scale the bitmap to the correct size
                sal_Int32 nOutWidth  = xFilePicker->getAvailableWidth();
                sal_Int32 nOutHeight = xFilePicker->getAvailableHeight();
                sal_Int32 nBmpWidth  = aBmp.GetSizePixel().Width();
                sal_Int32 nBmpHeight = aBmp.GetSizePixel().Height();

                double nXRatio = static_cast<double>(nOutWidth) / nBmpWidth;
                double nYRatio = static_cast<double>(nOutHeight) / nBmpHeight;

                if ( nXRatio < nYRatio )
                    aBmp.Scale( nXRatio, nXRatio );
                else
                    aBmp.Scale( nYRatio, nYRatio );

                // Convert to true color, to allow CopyPixel
                aBmp.Convert( BmpConversion::N24Bit );

                // and copy it into the Any
                SvMemoryStream aData;

                WriteDIB(aBmp, aData, false);

                const Sequence < sal_Int8 > aBuffer(
                    static_cast< const sal_Int8* >(aData.GetData()),
                    aData.GetEndOfData() );

                aAny <<= aBuffer;
            }
        }
    }

    try
    {
        SolarMutexReleaser aReleaseForCallback;
        // clear the preview window
        xFilePicker->setImage( FilePreviewImageFormats::BITMAP, aAny );
    }
    catch( const IllegalArgumentException& )
    {
    }
}

ErrCode FileDialogHelper_Impl::getGraphic( const OUString& rURL,
                                           Graphic& rGraphic )
{
    if ( utl::UCBContentHelper::IsFolder( rURL ) )
        return ERRCODE_IO_NOTAFILE;

    if ( !mpGraphicFilter )
        return ERRCODE_IO_NOTSUPPORTED;

    // select graphic filter from dialog filter selection
    OUString aCurFilter( getFilter() );

    sal_uInt16 nFilter = !aCurFilter.isEmpty() && mpGraphicFilter->GetImportFormatCount()
                    ? mpGraphicFilter->GetImportFormatNumber( aCurFilter )
                    : GRFILTER_FORMAT_DONTKNOW;

    INetURLObject aURLObj( rURL );

    if ( aURLObj.HasError() || INetProtocol::NotValid == aURLObj.GetProtocol() )
    {
        aURLObj.SetSmartProtocol( INetProtocol::File );
        aURLObj.SetSmartURL( rURL );
    }

    uno::Reference<task::XInteractionHandler2> xInteractionHandler =
        task::InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(),  GetFrameInterface());

    ErrCode nRet = ERRCODE_NONE;

    GraphicFilterImportFlags nFilterImportFlags = GraphicFilterImportFlags::SetLogsizeForJpeg;
    // non-local?
    if ( INetProtocol::File != aURLObj.GetProtocol() )
    {
        std::unique_ptr<SvStream> pStream = ::utl::UcbStreamHelper::CreateStream( rURL, StreamMode::READ );

        if( pStream )
            nRet = mpGraphicFilter->ImportGraphic(rGraphic, rURL, *pStream, nFilter, nullptr, nFilterImportFlags, -1, xInteractionHandler);
        else
            nRet = mpGraphicFilter->ImportGraphic(rGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags, xInteractionHandler);
    }
    else
    {
        nRet = mpGraphicFilter->ImportGraphic(rGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags, xInteractionHandler);
    }

    return nRet;
}

ErrCode FileDialogHelper_Impl::getGraphic( Graphic& rGraphic )
{
    ErrCode nRet = ERRCODE_NONE;

    // rhbz#1079672 do not return maGraphic, it needs not to be the selected file

    OUString aPath;
    Sequence<OUString> aPathSeq = mxFileDlg->getSelectedFiles();

    if (aPathSeq.getLength() == 1)
    {
        aPath = aPathSeq[0];
    }

    if (!aPath.isEmpty())
        nRet = getGraphic(aPath, rGraphic);
    else
        nRet = ERRCODE_IO_GENERAL;

    return nRet;
}

static bool lcl_isSystemFilePicker( const uno::Reference< XExecutableDialog >& _rxFP )
{
    try
    {
        uno::Reference< XServiceInfo > xSI( _rxFP, UNO_QUERY );
        if ( !xSI.is() )
            return true;
        return xSI->supportsService( u"com.sun.star.ui.dialogs.SystemFilePicker"_ustr );
    }
    catch( const Exception& )
    {
    }
    return false;
}

namespace {

bool lcl_isAsyncFilePicker( const uno::Reference< XExecutableDialog >& _rxFP )
{
    try
    {
        uno::Reference<XAsynchronousExecutableDialog> xSI(_rxFP, UNO_QUERY);
        return xSI.is();
    }
    catch( const Exception& )
    {
    }
    return false;
}

enum open_or_save_t {OPEN, SAVE, UNDEFINED};

}

static open_or_save_t lcl_OpenOrSave(sal_Int16 const nDialogType)
{
    switch (nDialogType)
    {
        case FILEOPEN_SIMPLE:
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
        case FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR:
        case FILEOPEN_PLAY:
        case FILEOPEN_LINK_PLAY:
        case FILEOPEN_READONLY_VERSION:
        case FILEOPEN_READONLY_VERSION_FILTEROPTIONS:
        case FILEOPEN_LINK_PREVIEW:
        case FILEOPEN_PREVIEW:
            return OPEN;
        case FILESAVE_SIMPLE:
        case FILESAVE_AUTOEXTENSION_PASSWORD:
        case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
        case FILESAVE_AUTOEXTENSION_SELECTION:
        case FILESAVE_AUTOEXTENSION_TEMPLATE:
        case FILESAVE_AUTOEXTENSION:
            return SAVE;
        default:
            assert(false); // invalid dialog type
    }
    return UNDEFINED;
}

// FileDialogHelper_Impl

css::uno::Reference<css::awt::XWindow> FileDialogHelper_Impl::GetFrameInterface()
{
    if (mpFrameWeld)
        return mpFrameWeld->GetXWindow();
    return css::uno::Reference<css::awt::XWindow>();
}

FileDialogHelper_Impl::FileDialogHelper_Impl(
    FileDialogHelper* _pAntiImpl,
    sal_Int16 nDialogType,
    FileDialogFlags nFlags,
    sal_Int16 nDialog,
    weld::Window* pFrameWeld,
    const OUString& sPreselectedDir,
    const css::uno::Sequence< OUString >& rDenyList
    )
    :msPreselectedDir( sPreselectedDir )
    ,maPreviewIdle("sfx2 FileDialogHelper_Impl maPreviewIdle")
    ,m_nDialogType          ( nDialogType )
    ,meContext              ( FileDialogHelper::UnknownContext )
{
    const char* pServiceName=nullptr;
    switch (nDialog)
    {
        case SFX2_IMPL_DIALOG_SYSTEM:
        case SFX2_IMPL_DIALOG_OOO:
            pServiceName = "com.sun.star.ui.dialogs.OfficeFilePicker";
            break;
        case SFX2_IMPL_DIALOG_REMOTE:
            pServiceName = "com.sun.star.ui.dialogs.RemoteFilePicker";
            break;
        default:
            pServiceName = "com.sun.star.ui.dialogs.FilePicker";
            break;
    }

    OUString aService = OUString::createFromAscii( pServiceName );

    uno::Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    // create the file open dialog
    // the flags can be SFXWB_INSERT or SFXWB_MULTISELECTION

    mpFrameWeld             = pFrameWeld;
    mpAntiImpl              = _pAntiImpl;
    mbHasAutoExt            = false;
    mbHasPassword           = false;
    mbHasSignByDefault      = false;
    m_bHaveFilterOptions    = false;
    mbIsPwdEnabled          = true;
    mbIsGpgEncrEnabled      = true;
    mbHasVersions           = false;
    mbHasPreview            = false;
    mbShowPreview           = false;
    mbDeleteMatcher         = false;
    mbInsert                = bool(nFlags & (FileDialogFlags::Insert|
                                             FileDialogFlags::InsertCompare|
                                             FileDialogFlags::InsertMerge));
    mbExport                = bool(nFlags & FileDialogFlags::Export);
    mbIsSaveDlg             = false;
    mbPwdCheckBoxState      = false;
    mbGpgCheckBoxState      = false;
    mbSelection             = false;
    mbSelectionEnabled      = true;
    mbHasSelectionBox       = false;
    mbSelectionFltrEnabled  = false;

    // default settings
    m_nDontFlags = SFX_FILTER_NOTINSTALLED | SfxFilterFlags::INTERNAL | SfxFilterFlags::NOTINFILEDLG;
    if (OPEN == lcl_OpenOrSave(m_nDialogType))
        m_nMustFlags = SfxFilterFlags::IMPORT;
    else
        m_nMustFlags = SfxFilterFlags::EXPORT;


    mpMatcher = nullptr;
    mpGraphicFilter = nullptr;

    // create the picker component
    mxFileDlg.set(xFactory->createInstance( aService ), css::uno::UNO_QUERY);
    mbSystemPicker = lcl_isSystemFilePicker( mxFileDlg );
    mbAsyncPicker = lcl_isAsyncFilePicker(mxFileDlg);

    if ( ! mxFileDlg.is() )
    {
        return;
    }

    uno::Reference< XInitialization > xInit( mxFileDlg, UNO_QUERY );
    if ( xInit.is() )
    {
        sal_Int16 nTemplateDescription = TemplateDescription::FILEOPEN_SIMPLE;

        switch ( m_nDialogType )
        {
            case FILEOPEN_SIMPLE:
                nTemplateDescription = TemplateDescription::FILEOPEN_SIMPLE;
                break;

            case FILESAVE_SIMPLE:
                nTemplateDescription = TemplateDescription::FILESAVE_SIMPLE;
                mbIsSaveDlg = true;
                break;

            case FILESAVE_AUTOEXTENSION_PASSWORD:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD;
                mbHasPassword = true;
                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                mbHasSignByDefault = true;
                break;

            case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS;
                mbHasPassword = true;

                m_bHaveFilterOptions = true;
                if( xFactory.is() )
                {
                    mxFilterCFG.set(
                        xFactory->createInstance( u"com.sun.star.document.FilterFactory"_ustr ),
                        UNO_QUERY );
                }

                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                mbHasSignByDefault = true;
                break;

            case FILESAVE_AUTOEXTENSION_SELECTION:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                mbHasSelectionBox = true;
                if ( mbExport && !mxFilterCFG.is() && xFactory.is() )
                {
                    mxFilterCFG.set(
                        xFactory->createInstance( u"com.sun.star.document.FilterFactory"_ustr ),
                        UNO_QUERY );
                }
                break;

            case FILESAVE_AUTOEXTENSION_TEMPLATE:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE;
                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                break;

            case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
                nTemplateDescription = TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE;
                mbHasPreview = true;
                break;

            case FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR:
                nTemplateDescription = TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_ANCHOR;
                mbHasPreview = true;
                break;

            case FILEOPEN_PLAY:
                nTemplateDescription = TemplateDescription::FILEOPEN_PLAY;
                break;

            case FILEOPEN_LINK_PLAY:
                nTemplateDescription = TemplateDescription::FILEOPEN_LINK_PLAY;
                break;

            case FILEOPEN_READONLY_VERSION:
                nTemplateDescription = TemplateDescription::FILEOPEN_READONLY_VERSION;
                mbHasVersions = true;
                break;

            case FILEOPEN_READONLY_VERSION_FILTEROPTIONS:
                nTemplateDescription = TemplateDescription::FILEOPEN_READONLY_VERSION_FILTEROPTIONS;
                m_bHaveFilterOptions = true;
                if( xFactory.is() )
                {
                    mxFilterCFG.set(
                        xFactory->createInstance( u"com.sun.star.document.FilterFactory"_ustr ),
                        UNO_QUERY );
                }
                mbHasVersions = true;
                break;

            case FILEOPEN_LINK_PREVIEW:
                nTemplateDescription = TemplateDescription::FILEOPEN_LINK_PREVIEW;
                mbHasPreview = true;
                break;

            case FILESAVE_AUTOEXTENSION:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION;
                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                break;

            case FILEOPEN_PREVIEW:
                nTemplateDescription = TemplateDescription::FILEOPEN_PREVIEW;
                mbHasPreview = true;
                break;

            default:
                SAL_WARN( "sfx.dialog", "FileDialogHelper::ctor with unknown type" );
                break;
        }

        if (mbHasPreview)
        {
            maPreviewIdle.SetPriority( TaskPriority::LOWEST );
            maPreviewIdle.SetInvokeHandler( LINK( this, FileDialogHelper_Impl, TimeOutHdl_Impl ) );
        }

        auto xWindow = GetFrameInterface();

        Sequence < Any > aInitArguments(!xWindow.is() ? 2 : 3);
        auto pInitArguments = aInitArguments.getArray();

        // This is a hack. We currently know that the internal file picker implementation
        // supports the extended arguments as specified below.
        // TODO:
        // a) adjust the service description so that it includes the TemplateDescription and ParentWindow args
        // b) adjust the implementation of the system file picker to that it recognizes it
        if ( mbSystemPicker )
        {
            pInitArguments[0] <<= nTemplateDescription;
            if (xWindow.is())
                pInitArguments[1] <<= xWindow;
        }
        else
        {
            pInitArguments[0] <<= NamedValue(
                                    u"TemplateDescription"_ustr,
                                    Any( nTemplateDescription )
                                );

            pInitArguments[1] <<= NamedValue(
                                    u"DenyList"_ustr,
                                    Any( rDenyList )
                                );


            if (xWindow.is())
                pInitArguments[2] <<= NamedValue(u"ParentWindow"_ustr, Any(xWindow));
        }

        try
        {
            xInit->initialize( aInitArguments );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "FileDialogHelper_Impl::FileDialogHelper_Impl: could not initialize the picker!" );
        }
    }


    // set multiselection mode
    if ( nFlags & FileDialogFlags::MultiSelection )
        mxFileDlg->setMultiSelectionMode( true );

    if ( nFlags & FileDialogFlags::Graphic ) // generate graphic filter only on demand
    {
        addGraphicFilter();
    }

    // Export dialog
    if ( mbExport )
    {
        mxFileDlg->setTitle( SfxResId( STR_SFX_EXPLORERFILE_EXPORT ) );
        try {
                css::uno::Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY_THROW );
                xCtrlAccess->enableControl( ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR, true );
        }
        catch( const Exception & ) { }
    }

    // Save a copy dialog
    if ( nFlags & FileDialogFlags::SaveACopy )
    {
        mxFileDlg->setTitle( SfxResId( STR_PB_SAVEACOPY ) );
    }

    // the "insert file" dialog needs another title
    if ( mbInsert )
    {
        if ( nFlags & FileDialogFlags::InsertCompare )
        {
            mxFileDlg->setTitle( SfxResId( STR_PB_COMPAREDOC ) );
        }
        else if ( nFlags & FileDialogFlags::InsertMerge )
        {
            mxFileDlg->setTitle( SfxResId( STR_PB_MERGEDOC ) );
        }
        else
        {
            mxFileDlg->setTitle( SfxResId( STR_SFX_EXPLORERFILE_INSERT ) );
        }
        uno::Reference < XFilePickerControlAccess > xExtDlg( mxFileDlg, UNO_QUERY );
        if ( xExtDlg.is() )
        {
            try
            {
                xExtDlg->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK,
                                   SfxResId( STR_SFX_EXPLORERFILE_BUTTONINSERT ) );
            }
            catch( const IllegalArgumentException& ){}
        }
    }

    // add the event listener
    mxFileDlg->addFilePickerListener( this );
}

css::uno::Reference<css::ui::dialogs::XFolderPicker2> createFolderPicker(const css::uno::Reference<css::uno::XComponentContext>& rContext, weld::Window* pPreferredParent)
{
    auto xRet = css::ui::dialogs::FolderPicker::create(rContext);

    // see FileDialogHelper_Impl::FileDialogHelper_Impl (above) for args to FilePicker
    // reuse the same arguments for FolderPicker
    if (pPreferredParent && lcl_isSystemFilePicker(xRet))
    {
        uno::Reference< XInitialization > xInit(xRet, UNO_QUERY);
        if (xInit.is())
        {
            Sequence<Any> aInitArguments{ Any(sal_Int32(0)), Any(pPreferredParent->GetXWindow()) };

            try
            {
                xInit->initialize(aInitArguments);
            }
            catch (const Exception&)
            {
                OSL_FAIL( "createFolderPicker: could not initialize the picker!" );
            }
        }
    }

    return xRet;
}

FileDialogHelper_Impl::~FileDialogHelper_Impl()
{
    mpGraphicFilter.reset();

    if ( mbDeleteMatcher )
        delete mpMatcher;

    maPreviewIdle.ClearInvokeHandler();

    ::comphelper::disposeComponent( mxFileDlg );
}

void FileDialogHelper_Impl::setControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId )
{
    DBG_ASSERT( _pControlId && _pHelpId, "FileDialogHelper_Impl::setControlHelpIds: invalid array pointers!" );
    if ( !_pControlId || !_pHelpId )
        return;

    // forward these ids to the file picker
    try
    {
        const OUString sHelpIdPrefix( INET_HID_SCHEME  );
        // the ids for the single controls
        uno::Reference< XFilePickerControlAccess > xControlAccess( mxFileDlg, UNO_QUERY );
        if ( xControlAccess.is() )
        {
            while ( *_pControlId )
            {
                DBG_ASSERT( INetURLObject( OStringToOUString( *_pHelpId, RTL_TEXTENCODING_UTF8 ) ).GetProtocol() == INetProtocol::NotValid, "Wrong HelpId!" );
                OUString sId = sHelpIdPrefix +
                    OUString( *_pHelpId, strlen( *_pHelpId ), RTL_TEXTENCODING_UTF8 );
                xControlAccess->setValue( *_pControlId, ControlActions::SET_HELP_URL, Any( sId ) );

                ++_pControlId; ++_pHelpId;
            }
        }
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::setControlHelpIds: caught an exception while setting the help ids!" );
    }
}

void FileDialogHelper_Impl::preExecute()
{
    loadConfig( );
    setDefaultValues( );
    updatePreviewState( false );

    implInitializeFileName( );

    enablePasswordBox( true );
    enableGpgEncrBox( true );
    updateFilterOptionsBox( );
    updateSelectionBox( );
    updateSignByDefault();
}

void FileDialogHelper_Impl::postExecute( sal_Int16 _nResult )
{
    if ( ExecutableDialogResults::CANCEL != _nResult )
        saveConfig();
}

void FileDialogHelper_Impl::implInitializeFileName( )
{
    if ( maFileName.isEmpty() )
        return;

    INetURLObject aObj( maPath );
    aObj.Append( maFileName );

    // in case we're operating as save dialog, and "auto extension" is checked,
    // cut the extension from the name
    if ( !(mbIsSaveDlg && mbHasAutoExt) )
        return;

    try
    {
        bool bAutoExtChecked = false;

        uno::Reference < XFilePickerControlAccess > xControlAccess( mxFileDlg, UNO_QUERY );
        if  (   xControlAccess.is()
            &&  (   xControlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0 )
                >>= bAutoExtChecked
                )
            )
        {
            if ( bAutoExtChecked )
            {   // cut the extension
                aObj.removeExtension( );
                mxFileDlg->setDefaultName(
                    aObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset));
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FileDialogHelper_Impl::implInitializeFileName: could not ask for the auto-extension current-value!" );
    }
}

sal_Int16 FileDialogHelper_Impl::implDoExecute()
{
    preExecute();

    sal_Int16 nRet = ExecutableDialogResults::CANCEL;

//On MacOSX the native file picker has to run in the primordial thread because of drawing issues
//On Linux the native gtk file picker, when backed by gnome-vfs2, needs to be run in the same
//primordial thread as the ucb gnome-vfs2 provider was initialized in.

    {
        try
        {
#ifdef _WIN32
            if ( mbSystemPicker )
            {
                SolarMutexReleaser aSolarMutex;
                nRet = mxFileDlg->execute();
            }
            else
#endif
                nRet = mxFileDlg->execute();
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::implDoExecute" );
        }
    }

    postExecute( nRet );

    return nRet;
}

void FileDialogHelper_Impl::implStartExecute()
{
    DBG_ASSERT( mxFileDlg.is(), "invalid file dialog" );

    assert(mbAsyncPicker);
    preExecute();

    try
    {
        uno::Reference< XAsynchronousExecutableDialog > xAsyncDlg( mxFileDlg, UNO_QUERY );
        if ( xAsyncDlg.is() )
            xAsyncDlg->startExecuteModal( this );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::implDoExecute" );
    }
}

ErrCode FileDialogHelper_Impl::execute( css::uno::Sequence<OUString>& rpURLList,
                                        std::optional<SfxAllItemSet>& rpSet,
                                        OUString&       rFilter,
                                        SignatureState const nScriptingSignatureState)
{
    // rFilter is a pure output parameter, it shouldn't be used for anything else
    // changing this would surely break code
    // rpSet is in/out parameter, usually just a media-descriptor that can be changed by dialog

    // retrieves parameters from rpSet
    // for now only Password is used
    if ( rpSet )
    {
        // check password checkbox if the document had password before
        if( mbHasPassword )
        {
            const SfxBoolItem* pPassItem = rpSet->GetItem(SID_PASSWORDINTERACTION, false);
            // TODO: tdf#158839 problem: Is also true if the file is GPG encrypted. (not with a password)
            mbPwdCheckBoxState = ( pPassItem != nullptr && pPassItem->GetValue() );

            // in case the document has password to modify, the dialog should be shown
            const SfxUnoAnyItem* pPassToModifyItem = rpSet->GetItem(SID_MODIFYPASSWORDINFO, false);
            mbPwdCheckBoxState |= ( pPassToModifyItem && pPassToModifyItem->GetValue().hasValue() );
        }

        const SfxBoolItem* pSelectItem = rpSet->GetItem(SID_SELECTION, false);
        if ( pSelectItem )
            mbSelection = pSelectItem->GetValue();
        else
            mbSelectionEnabled = false;

        // the password will be set in case user decide so
        rpSet->ClearItem( SID_PASSWORDINTERACTION );
        if (rpSet->HasItem( SID_PASSWORD ))
        {
            // As the SID_ENCRYPTIONDATA and SID_PASSWORD are using for setting password together, we need to clear them both.
            // Note: Do not remove SID_ENCRYPTIONDATA without SID_PASSWORD
            rpSet->ClearItem( SID_PASSWORD );
            rpSet->ClearItem( SID_ENCRYPTIONDATA );
        }
        rpSet->ClearItem( SID_RECOMMENDREADONLY );
        rpSet->ClearItem( SID_MODIFYPASSWORDINFO );

    }

    if ( mbHasPassword && !mbPwdCheckBoxState )
    {
        mbPwdCheckBoxState = (
            SvtSecurityOptions::IsOptionSet( SvtSecurityOptions::EOption::DocWarnRecommendPassword ) );
    }

    rpURLList = {};
    maPath.clear(); // tdf#165228 This should not survive between calls to execute

    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    if (ExecutableDialogResults::CANCEL == implDoExecute())
        return ERRCODE_ABORT;

    // fill the rpURLList
    rpURLList = mxFileDlg->getSelectedFiles();
    if (!rpURLList.hasElements())
        return ERRCODE_ABORT;

    uno::Reference<XFilePickerControlAccess> xCtrlAccess(mxFileDlg, UNO_QUERY);

    // create an itemset if there is no
    if( !rpSet )
        rpSet.emplace( SfxGetpApp()->GetPool() );

    // the item should remain only if it was set by the dialog
    rpSet->ClearItem( SID_SELECTION );

    if (mbExport && mbHasSelectionBox && xCtrlAccess)
    {
        try
        {
            Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
            bool bSelection = false;
            if ( aValue >>= bSelection )
                rpSet->Put( SfxBoolItem( SID_SELECTION, bSelection ) );
        }
        catch( const IllegalArgumentException& )
        {
            TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
        }
    }


    // set the read-only flag. When inserting a file, this flag is always set
    if ( mbInsert )
        rpSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
    else if ( ( FILEOPEN_READONLY_VERSION == m_nDialogType ) && xCtrlAccess.is() )
    {
        try
        {
            Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 );
            bool bReadOnly = false;
            if ( ( aValue >>= bReadOnly ) && bReadOnly )
                rpSet->Put( SfxBoolItem( SID_DOC_READONLY, bReadOnly ) );
        }
        catch( const IllegalArgumentException& )
        {
            TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
        }
    }
    if ( mbHasVersions && xCtrlAccess.is() )
    {
        try
        {
            Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                                                ControlActions::GET_SELECTED_ITEM_INDEX );
            sal_Int32 nVersion = 0;
            if ( ( aValue >>= nVersion ) && nVersion > 0 )
                // open a special version; 0 == current version
                rpSet->Put( SfxInt16Item( SID_VERSION, static_cast<short>(nVersion) ) );
        }
        catch( const IllegalArgumentException& ){}
    }

    // set the filter
    getRealFilter( rFilter );

    std::shared_ptr<const SfxFilter> pCurrentFilter = getCurrentSfxFilter();

    // check, whether or not we have to display a password box
    if ( pCurrentFilter && mbHasPassword && mbIsPwdEnabled && xCtrlAccess.is() )
    {
        try
        {
            Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
            bool bPassWord = false;
            if ( ( aValue >>= bPassWord ) && bPassWord )
            {
                SvtSaveOptions::ODFSaneDefaultVersion nVersion{
                    SvtSaveOptions::ODFSVER_LATEST_EXTENDED};
                if (!comphelper::IsFuzzing())
                {
                    nVersion = GetODFSaneDefaultVersion();
                }
                // old per-zip-entry ODF encryption destroys macro signatures
                if (!::sfx2::UseODFWholesomeEncryption(nVersion)
                    && (   SignatureState::OK == nScriptingSignatureState
                        || SignatureState::INVALID == nScriptingSignatureState
                        || SignatureState::NOTVALIDATED == nScriptingSignatureState
                        || SignatureState::PARTIAL_OK == nScriptingSignatureState))
                {
                    std::unique_ptr<weld::MessageDialog> xBox(
                        Application::CreateMessageDialog(mpFrameWeld,
                            VclMessageType::Question, VclButtonsType::YesNo,
                            SfxResId(RID_SVXSTR_XMLSEC_QUERY_LOSINGSCRIPTINGSIGNATURE)));
                    if (xBox->run() == RET_NO)
                    {
                        bPassWord = false;
                    }
                }
            }

            if (bPassWord)
            {
                // ask for a password
                const OUString& aDocName(rpURLList[0]);
                // TODO: tdf#158839 problem: Also asks for a password if CHECKBOX_GPGENCRYPTION && CHECKBOX_PASSWORD
                //       are checked. But only encrypts using GPG and discards the password.
                ErrCode errCode = RequestPassword(pCurrentFilter, aDocName, &*rpSet, GetFrameInterface());
                if (errCode != ERRCODE_NONE)
                    return errCode;
            }
        }
        catch( const IllegalArgumentException& ){}
    }
    // check, whether or not we have to display a key selection box
    if ( pCurrentFilter && mbHasPassword && xCtrlAccess.is() )
    {
        try
        {
            Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_GPGENCRYPTION, 0 );
            bool bGpg = false;
            if ( ( aValue >>= bGpg ) && bGpg )
            {
                uno::Sequence< beans::NamedValue > aEncryptionData;
                while(true)
                {
                    try
                    {
                        // ask for keys
                        aEncryptionData
                            = ::comphelper::OStorageHelper::CreateGpgPackageEncryptionData(
                                GetFrameInterface());
                        break; // user cancelled or we've some keys now
                    }
                    catch( const IllegalArgumentException& )
                    {
                        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(mpFrameWeld,
                                                                                 VclMessageType::Warning, VclButtonsType::Ok,
                                                                                 SfxResId(RID_SVXSTR_GPG_ENCRYPT_FAILURE)));
                        xBox->run();
                    }
                }

                if ( aEncryptionData.hasElements() )
                    rpSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::Any( aEncryptionData) ) );
                else
                    return ERRCODE_ABORT;
            }
        }
        catch( const IllegalArgumentException& ){}
    }
    if ( pCurrentFilter && xCtrlAccess.is() )
    {
        try
        {
            Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_GPGSIGN, 0 );
            bool bSign = false;
            if ((aValue >>= bSign) && bSign)
            {
                rpSet->Put(SfxBoolItem(SID_GPGSIGN, bSign));
            }
        }
        catch( const IllegalArgumentException& ){}
    }

    SaveLastUsedFilter();
    return ERRCODE_NONE;
}

ErrCode FileDialogHelper_Impl::execute()
{
    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    sal_Int16 nRet = implDoExecute();

    maPath = mxFileDlg->getDisplayDirectory();

    if ( ExecutableDialogResults::CANCEL == nRet )
        return ERRCODE_ABORT;
    else
    {
        return ERRCODE_NONE;
    }
}

OUString FileDialogHelper_Impl::getPath() const
{
    OUString aPath;

    if ( mxFileDlg.is() )
        aPath = mxFileDlg->getDisplayDirectory();

    if ( aPath.isEmpty() )
        aPath = maPath;

    return aPath;
}

OUString FileDialogHelper_Impl::getFilter() const
{
    OUString aFilter = getCurrentFilterUIName();

    if( aFilter.isEmpty() )
        aFilter = maCurFilter;

    return aFilter;
}

void FileDialogHelper_Impl::getRealFilter( OUString& _rFilter ) const
{
    _rFilter = getCurrentFilterUIName();

    if ( _rFilter.isEmpty() )
        _rFilter = maCurFilter;

    if ( !_rFilter.isEmpty() && mpMatcher )
    {
        std::shared_ptr<const SfxFilter> pFilter =
            mpMatcher->GetFilter4UIName( _rFilter, m_nMustFlags, m_nDontFlags );
        _rFilter = pFilter ? pFilter->GetFilterName() : OUString();
    }
}

void FileDialogHelper_Impl::verifyPath()
{
#ifdef UNX
    // lp#905355, fdo#43895
    // Check that the file has read only permission and is in /tmp -- this is
    //  the case if we have opened the file from the web with firefox only.
    if (maFileName.isEmpty()) {
        return;
    }
    INetURLObject url(maPath);
    if (url.GetProtocol() != INetProtocol::File
        || url.getName(0, true, INetURLObject::DecodeMechanism::WithCharset) != "tmp")
    {
        return;
    }
    if (maFileName.indexOf('/') != -1) {
        SAL_WARN("sfx.dialog", maFileName << " contains /");
        return;
    }
    url.insertName(
        maFileName, false, INetURLObject::LAST_SEGMENT,
        INetURLObject::EncodeMechanism::All);
    OUString sysPathU;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        url.GetMainURL(INetURLObject::DecodeMechanism::NONE), sysPathU);
    if (e != osl::FileBase::E_None) {
        SAL_WARN(
            "sfx.dialog",
            "getSystemPathFromFileURL("
                << url.GetMainURL(INetURLObject::DecodeMechanism::NONE) << ") failed with "
                << +e);
        return;
    }
    OString sysPathC;
    if (!sysPathU.convertToString(
            &sysPathC, osl_getThreadTextEncoding(),
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        SAL_WARN(
            "sfx.dialog",
            "convertToString(" << sysPathU << ") failed for encoding "
                << +osl_getThreadTextEncoding());
        return;
    }
    struct stat aFileStat;
    if (stat(sysPathC.getStr(), &aFileStat) == -1) {
        SAL_WARN( "sfx.dialog", "stat(" << sysPathC << ") failed with errno " << errno);
        return;
    }
    if ((aFileStat.st_mode & (S_IRWXO | S_IRWXG | S_IRWXU)) == S_IRUSR) {
        maPath = SvtPathOptions().GetWorkPath();
        mxFileDlg->setDisplayDirectory( maPath );
    }
#else
    (void) this;
#endif
}

void FileDialogHelper_Impl::displayFolder( const OUString& _rPath )
{
    if ( _rPath.isEmpty() )
        // nothing to do
        return;

    maPath = _rPath;
    if ( mxFileDlg.is() )
    {
        try
        {
            mxFileDlg->setDisplayDirectory( maPath );
            verifyPath();
        }
        catch( const IllegalArgumentException& )
        {
            TOOLS_WARN_EXCEPTION( "sfx", "FileDialogHelper_Impl::displayFolder" );
        }
    }
}

void FileDialogHelper_Impl::setFileName( const OUString& _rFile )
{
    maFileName = _rFile;
    if ( mxFileDlg.is() )
    {
        try
        {
            mxFileDlg->setDefaultName( maFileName );
            verifyPath();
        }
        catch( const IllegalArgumentException& )
        {
            TOOLS_WARN_EXCEPTION( "sfx", "FileDialogHelper_Impl::setFileName" );
        }
    }
}

void FileDialogHelper_Impl::setFilter( const OUString& rFilter )
{
    DBG_ASSERT( rFilter.indexOf(':') == -1, "Old filter name used!");

    maCurFilter = rFilter;

    if ( !rFilter.isEmpty() && mpMatcher )
    {
        std::shared_ptr<const SfxFilter> pFilter = mpMatcher->GetFilter4FilterName(
                                        rFilter, m_nMustFlags, m_nDontFlags );
        if ( pFilter )
            maCurFilter = pFilter->GetUIName();
    }

    if ( !maCurFilter.isEmpty() && mxFileDlg.is() )
    {
        try
        {
            mxFileDlg->setCurrentFilter( maCurFilter );
        }
        catch( const IllegalArgumentException& ){}
    }
}

void FileDialogHelper_Impl::createMatcher( const OUString& rFactory )
{
    if (mbDeleteMatcher)
        delete mpMatcher;

    mpMatcher = new SfxFilterMatcher( SfxObjectShell::GetServiceNameFromFactory(rFactory) );
    mbDeleteMatcher = true;
}

void FileDialogHelper_Impl::addFilters( const OUString& rFactory,
                                        SfxFilterFlags nMust,
                                        SfxFilterFlags nDont )
{
    if ( ! mxFileDlg.is() )
        return;

    if (mbDeleteMatcher)
        delete mpMatcher;

    // we still need a matcher to convert UI names to filter names
    if ( rFactory.isEmpty() )
    {
        SfxApplication *pSfxApp = SfxGetpApp();
        mpMatcher = &pSfxApp->GetFilterMatcher();
        mbDeleteMatcher = false;
    }
    else
    {
        mpMatcher = new SfxFilterMatcher( rFactory );
        mbDeleteMatcher = true;
    }

    uno::Reference< XMultiServiceFactory > xSMGR = ::comphelper::getProcessServiceFactory();
    uno::Reference< XContainerQuery > xFilterCont(
        xSMGR->createInstance(u"com.sun.star.document.FilterFactory"_ustr),
        UNO_QUERY);
    if ( ! xFilterCont.is() )
        return;

    m_nMustFlags |= nMust;
    m_nDontFlags |= nDont;

    // create the list of filters
    OUString sQuery =
        "getSortedFilterList()"
        ":module=" +
        rFactory + // use long name here !
        ":iflags=" +
        OUString::number(static_cast<sal_Int32>(m_nMustFlags)) +
        ":eflags=" +
        OUString::number(static_cast<sal_Int32>(m_nDontFlags));

    uno::Reference< XEnumeration > xResult;
    try
    {
        xResult = xFilterCont->createSubSetEnumerationByQuery(sQuery);
    }
    catch( const uno::Exception& )
    {
        SAL_WARN( "sfx.dialog", "Could not get filters from the configuration!" );
    }

    TSortedFilterList         aIter   (xResult);

    // append the filters
    OUString sFirstFilter;
    if (OPEN == lcl_OpenOrSave(m_nDialogType))
        ::sfx2::appendFiltersForOpen( aIter, mxFileDlg, sFirstFilter, *this );
    else if ( mbExport )
        ::sfx2::appendExportFilters( aIter, mxFileDlg, sFirstFilter, *this );
    else
        ::sfx2::appendFiltersForSave( aIter, mxFileDlg, sFirstFilter, *this, rFactory );

    // set our initial selected filter (if we do not already have one)
    if ( maSelectFilter.isEmpty() )
        maSelectFilter = sFirstFilter;
}

void FileDialogHelper_Impl::addFilter( const OUString& rFilterName,
                                       const OUString& rExtension )
{
    if ( ! mxFileDlg.is() )
        return;

    try
    {
        mxFileDlg->appendFilter( rFilterName, rExtension );

        if ( maSelectFilter.isEmpty() )
            maSelectFilter = rFilterName;
    }
    catch( const IllegalArgumentException& )
    {
        SAL_WARN( "sfx.dialog", "Could not append Filter" << rFilterName );
    }
}

void FileDialogHelper_Impl::addGraphicFilter()
{
    if ( ! mxFileDlg.is() )
        return;

    // create the list of filters
    mpGraphicFilter.reset( new GraphicFilter );
    sal_uInt16 i, j, nCount = mpGraphicFilter->GetImportFormatCount();

    // compute the extension string for all known import filters
    OUString aExtensions;

    for ( i = 0; i < nCount; i++ )
    {
        j = 0;
        while( true )
        {
            OUString sWildcard = mpGraphicFilter->GetImportWildcard( i, j++ );
            if ( sWildcard.isEmpty() )
                break;
            if ( aExtensions.indexOf( sWildcard ) == -1 )
            {
                if ( !aExtensions.isEmpty() )
                    aExtensions += ";";
                aExtensions += sWildcard;
            }
        }
    }

#if defined(_WIN32)
    if ( aExtensions.getLength() > 240 )
        aExtensions = FILEDIALOG_FILTER_ALL;
#endif
    bool bIsInOpenMode = isInOpenMode();

    try
    {
        // if the extension is not "All files", insert "All images"
        if (aExtensions != FILEDIALOG_FILTER_ALL)
        {
            OUString aAllFilterName = SfxResId(STR_SFX_IMPORT_ALL_IMAGES);
            aAllFilterName = ::sfx2::addExtension( aAllFilterName, aExtensions, bIsInOpenMode, *this );
            mxFileDlg->appendFilter( aAllFilterName, aExtensions );
            maSelectFilter = aAllFilterName; // and make it the default
        }

        // rhbz#1715109 always include All files *.* or *
        OUString aAllFilesName = SfxResId( STR_SFX_FILTERNAME_ALL );
        aAllFilesName = ::sfx2::addExtension( aAllFilesName, FILEDIALOG_FILTER_ALL, bIsInOpenMode, *this );
        mxFileDlg->appendFilter( aAllFilesName, FILEDIALOG_FILTER_ALL );

        // if the extension is "All files", make that the default
        if (aExtensions == FILEDIALOG_FILTER_ALL)
            maSelectFilter = aAllFilesName;
    }
    catch( const IllegalArgumentException& )
    {
        SAL_WARN( "sfx.dialog", "Could not append Filter" );
    }

    // Now add the filter
    for ( i = 0; i < nCount; i++ )
    {
        OUString aName = mpGraphicFilter->GetImportFormatName( i );
        OUString aExt;
        j = 0;
        while( true )
        {
            OUString sWildcard = mpGraphicFilter->GetImportWildcard( i, j++ );
            if ( sWildcard.isEmpty() )
                break;
            if ( aExt.indexOf( sWildcard ) == -1 )
            {
                if ( !aExt.isEmpty() )
                    aExt += ";";
                aExt += sWildcard;
            }
        }
        aName = ::sfx2::addExtension( aName, aExt, bIsInOpenMode, *this );
        try
        {
            mxFileDlg->appendFilter( aName, aExt );
        }
        catch( const IllegalArgumentException& )
        {
            SAL_WARN( "sfx.dialog", "Could not append Filter" );
        }
    }
}

constexpr OUStringLiteral GRF_CONFIG_STR = u"   ";
constexpr OUString STD_CONFIG_STR = u"1 "_ustr;

static void SetToken( OUString& rOrigStr, sal_Int32 nToken, sal_Unicode cTok, std::u16string_view rStr)
{
    const sal_Unicode*  pStr        = rOrigStr.getStr();
    sal_Int32     nLen              = rOrigStr.getLength();
    sal_Int32     nTok              = 0;
    sal_Int32     nFirstChar        = 0;
    sal_Int32     i                 = nFirstChar;

    // Determine token position and length
    pStr += i;
    while ( i < nLen )
    {
        // Increase token count if match
        if ( *pStr == cTok )
        {
            ++nTok;

            if ( nTok == nToken )
                nFirstChar = i+1;
            else
            {
                if ( nTok > nToken )
                    break;
            }
        }

        ++pStr;
        ++i;
    }

    if ( nTok >= nToken )
        rOrigStr = rOrigStr.replaceAt( nFirstChar, i-nFirstChar, rStr );
}

namespace
{
void SaveLastDirectory(OUString const& sContext, OUString const& sDirectory)
{
    if (sContext.isEmpty())
        return;

    std::shared_ptr<comphelper::ConfigurationChanges> batch(
        comphelper::ConfigurationChanges::create());
    Reference<container::XNameContainer> set(
        officecfg::Office::Common::Misc::FilePickerLastDirectory::get(batch));

    bool found;
    Any v;
    try
    {
        v = set->getByName(sContext);
        found = true;
    }
    catch (container::NoSuchElementException&)
    {
        found = false;
    }
    if (found)
    {
        Reference<XPropertySet> el(v.get<Reference<XPropertySet>>(), UNO_SET_THROW);
        el->setPropertyValue(u"LastPath"_ustr, Any(sDirectory));
    }
    else
    {
        Reference<XPropertySet> el(
            (Reference<lang::XSingleServiceFactory>(set, UNO_QUERY_THROW)->createInstance()),
            UNO_QUERY_THROW);
        el->setPropertyValue(u"LastPath"_ustr, Any(sDirectory));
        Any v2(el);
        set->insertByName(sContext, v2);
    }
    batch->commit();
}
}

void FileDialogHelper_Impl::saveConfig()
{
    uno::Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aDlgOpt( EViewType::Dialog, IMPGRF_CONFIGNAME );

        try
        {
            // tdf#61358 - remember the last "insert as link" state
            if (o3tl::IsRunningUITest())
                aValue <<= false;
            else
                aValue = xDlg->getValue(ExtendedFilePickerElementIds::CHECKBOX_LINK, 0);
            bool bLink = false;
            aValue >>= bLink;
            OUString aUserData(GRF_CONFIG_STR);
            SetToken(aUserData, 0, ' ', OUString::number(static_cast<sal_Int32>(bLink)));

            aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
            bool bValue = false;
            aValue >>= bValue;
            SetToken( aUserData, 1, ' ', OUString::number( static_cast<sal_Int32>(bValue) ) );

            INetURLObject aObj( getPath() );

            if ( aObj.GetProtocol() == INetProtocol::File )
                SetToken( aUserData, 2, ' ', aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

            OUString aFilter = getFilter();
            aFilter = EncodeSpaces_Impl( aFilter );
            SetToken( aUserData, 3, ' ', aFilter );

            aDlgOpt.SetUserItem( USERITEM_NAME, Any( aUserData ) );
        }
        catch( const IllegalArgumentException& ){}
    }
    else
    {
        bool bWriteConfig = false;
        SvtViewOptions aDlgOpt( EViewType::Dialog, IODLG_CONFIGNAME );
        OUString aUserData(STD_CONFIG_STR);

        if ( aDlgOpt.Exists() )
        {
            Any aUserItem = aDlgOpt.GetUserItem( USERITEM_NAME );
            OUString aTemp;
            if ( aUserItem >>= aTemp )
                aUserData = aTemp;
        }

        if ( mbHasAutoExt )
        {
            try
            {
                aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0 );
                bool bAutoExt = true;
                aValue >>= bAutoExt;
                SetToken( aUserData, 0, ' ', OUString::number( static_cast<sal_Int32>(bAutoExt) ) );
                bWriteConfig = true;
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( ! mbIsSaveDlg )
        {
            OUString aPath = getPath();
            if ( comphelper::isFileUrl( aPath ) )
            {
                SetToken( aUserData, 1, ' ', aPath );
                bWriteConfig = true;
            }
        }

        if( mbHasSelectionBox && mbSelectionFltrEnabled )
        {
            try
            {
                aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
                bool bSelection = true;
                aValue >>= bSelection;
                if ( comphelper::string::getTokenCount(aUserData, ' ') < 3 )
                    aUserData += " ";
                SetToken( aUserData, 2, ' ', OUString::number( static_cast<sal_Int32>(bSelection) ) );
                bWriteConfig = true;
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( bWriteConfig )
            aDlgOpt.SetUserItem( USERITEM_NAME, Any( aUserData ) );
    }

    // Store to config, if explicit context is set (and default directory is not given)
    if (meContext != FileDialogHelper::UnknownContext && msPreselectedDir.isEmpty())
    {
        SaveLastDirectory(FileDialogHelper::contextToString(meContext), getPath());
    }
}

OUString FileDialogHelper_Impl::getInitPath(std::u16string_view _rFallback,
                                            const sal_Int32 _nFallbackToken)
{
    OUString sPath;
    // Load from config, if explicit context is set. Otherwise load from (global) runtime var.
    if (meContext == FileDialogHelper::UnknownContext || !msPreselectedDir.isEmpty())
    {
        // For export, the default directory is passed on
        sPath = msPreselectedDir;
    }
    else
    {
        OUString sContext = FileDialogHelper::contextToString(meContext);
        Reference<XNameAccess> set(officecfg::Office::Common::Misc::FilePickerLastDirectory::get());
        Any v;
        try
        {
            v = set->getByName(sContext);
            Reference<XPropertySet> el(v.get<Reference<XPropertySet>>(), UNO_SET_THROW);
            sPath = el->getPropertyValue(u"LastPath"_ustr).get<OUString>();
        }
        catch (NoSuchElementException&)
        {
        }
    }

    if ( sPath.isEmpty() )
        sPath = o3tl::getToken(_rFallback, _nFallbackToken, ' ' );

    // check if the path points to a valid (accessible) directory
    bool bValid = false;
    if ( !sPath.isEmpty() )
    {
        OUString sPathCheck( sPath );
        if ( sPathCheck[ sPathCheck.getLength() - 1 ] != '/' )
            sPathCheck += "/";
        sPathCheck += ".";
        try
        {
            ::ucbhelper::Content aContent( sPathCheck,
                                            utl::UCBContentHelper::getDefaultCommandEnvironment(),
                                            comphelper::getProcessComponentContext() );
            bValid = aContent.isFolder();
        }
        catch( const Exception& ) {}
    }
    if ( !bValid )
        sPath.clear();
    return sPath;
}

void FileDialogHelper_Impl::loadConfig()
{
    uno::Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aViewOpt( EViewType::Dialog, IMPGRF_CONFIGNAME );
        OUString aUserData;

        if ( aViewOpt.Exists() )
        {
            Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
            OUString aTemp;
            if ( aUserItem >>= aTemp )
                aUserData = aTemp;
        }

        if ( !aUserData.isEmpty() )
        {
            try
            {
                // respect the last "insert as link" state
                bool bLink = o3tl::toInt32(o3tl::getToken(aUserData, 0, ' ' ));
                aValue <<= bLink;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aValue );

                // respect the last "show preview" state
                bool bShowPreview = o3tl::toInt32(o3tl::getToken(aUserData, 1, ' ' ));
                aValue <<= bShowPreview;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, aValue );

                if ( maPath.isEmpty() )
                    displayFolder( getInitPath( aUserData, 2 ) );

                if ( maCurFilter.isEmpty() )
                {
                    OUString aFilter = aUserData.getToken( 3, ' ' );
                    aFilter = DecodeSpaces_Impl( aFilter );
                    setFilter( aFilter );
                }

                // set the member so we know that we have to show the preview
                mbShowPreview = bShowPreview;
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( maPath.isEmpty() )
            displayFolder( SvtPathOptions().GetWorkPath() );
    }
    else
    {
        SvtViewOptions aViewOpt( EViewType::Dialog, IODLG_CONFIGNAME );
        OUString aUserData;

        if ( aViewOpt.Exists() )
        {
            Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
            OUString aTemp;
            if ( aUserItem >>= aTemp )
                aUserData = aTemp;
        }

        if ( aUserData.isEmpty() )
            aUserData = STD_CONFIG_STR;

        if ( maPath.isEmpty() )
            displayFolder( getInitPath( aUserData, 1 ) );

        if ( mbHasAutoExt )
        {
            sal_Int32 nFlag = o3tl::toInt32(o3tl::getToken(aUserData, 0, ' ' ));
            aValue <<= static_cast<bool>(nFlag);
            try
            {
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue );
            }
            catch( const IllegalArgumentException& ){}
        }

        if( mbHasSelectionBox )
        {
            sal_Int32 nFlag = o3tl::toInt32(o3tl::getToken(aUserData, 2, ' ' ));
            aValue <<= static_cast<bool>(nFlag);
            try
            {
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0, aValue );
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( maPath.isEmpty() )
            displayFolder( SvtPathOptions().GetWorkPath() );
    }
}

void FileDialogHelper_Impl::setDefaultValues()
{
    // when no filter is set, we set the currentFilter to <all>
    if ( maCurFilter.isEmpty() && !maSelectFilter.isEmpty() )
    {
        try
        {
            mxFileDlg->setCurrentFilter( maSelectFilter );
        }
        catch( const IllegalArgumentException& )
        {}
    }

    // when no path is set, we use the standard 'work' folder
    if ( maPath.isEmpty() )
    {
        OUString aWorkFolder = SvtPathOptions().GetWorkPath();
        try
        {
            mxFileDlg->setDisplayDirectory( aWorkFolder );
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "sfx.dialog", "FileDialogHelper_Impl::setDefaultValues: caught an exception while setting the display directory!" );
        }
    }
}

bool FileDialogHelper_Impl::isShowFilterExtensionEnabled() const
{
    return !maFilters.empty();
}

void FileDialogHelper_Impl::addFilterPair( const OUString& rFilter,
                                           const OUString& rFilterWithExtension )
{
    maFilters.emplace_back( rFilter, rFilterWithExtension );

}

OUString FileDialogHelper_Impl::getFilterName( std::u16string_view rFilterWithExtension ) const
{
    OUString sRet;
    for (auto const& filter : maFilters)
    {
        if (filter.Second == rFilterWithExtension)
        {
            sRet = filter.First;
            break;
        }
    }
    return sRet;
}

OUString FileDialogHelper_Impl::getFilterWithExtension( std::u16string_view rFilter ) const
{
    OUString sRet;
    for (auto const& filter : maFilters)
    {
        if ( filter.First == rFilter )
        {
            sRet = filter.Second;
            break;
        }
    }
    return sRet;
}

void FileDialogHelper_Impl::SetContext( FileDialogHelper::Context _eNewContext )
{
    meContext = _eNewContext;

    std::optional<OUString> pConfigId = GetLastFilterConfigId( _eNewContext );
    if( pConfigId )
        LoadLastUsedFilter( *pConfigId );
}

// FileDialogHelper

FileDialogHelper::FileDialogHelper(
    sal_Int16 nDialogType,
    FileDialogFlags nFlags,
    const OUString& rFact,
    SfxFilterFlags nMust,
    SfxFilterFlags nDont,
    weld::Window* pPreferredParent)
    :   m_nError(0),
        mpImpl(new FileDialogHelper_Impl(this, nDialogType, nFlags, SFX2_IMPL_DIALOG_CONFIG, pPreferredParent))
{

    // create the list of filters
    mpImpl->addFilters(
            SfxObjectShell::GetServiceNameFromFactory(rFact), nMust, nDont );
}

FileDialogHelper::FileDialogHelper(
    sal_Int16 nDialogType,
    FileDialogFlags nFlags,
    const OUString& rFact,
    sal_Int16 nDialog,
    SfxFilterFlags nMust,
    SfxFilterFlags nDont,
    const OUString& rPreselectedDir,
    const css::uno::Sequence< OUString >& rDenyList,
    weld::Window* pPreferredParent)
    :   m_nError(0),
        mpImpl( new FileDialogHelper_Impl( this, nDialogType, nFlags, nDialog, pPreferredParent, rPreselectedDir, rDenyList ) )
{
    // create the list of filters
    mpImpl->addFilters(
            SfxObjectShell::GetServiceNameFromFactory(rFact), nMust, nDont );
}

FileDialogHelper::FileDialogHelper(sal_Int16 nDialogType, FileDialogFlags nFlags, weld::Window* pPreferredParent)
    :   m_nError(0),
        mpImpl( new FileDialogHelper_Impl( this, nDialogType, nFlags, SFX2_IMPL_DIALOG_CONFIG, pPreferredParent ) )
{
}

FileDialogHelper::FileDialogHelper(
    sal_Int16 nDialogType,
    FileDialogFlags nFlags,
    const OUString& aFilterUIName,
    std::u16string_view aExtName,
    const OUString& rPreselectedDir,
    const css::uno::Sequence< OUString >& rDenyList,
    weld::Window* pPreferredParent )
    :   m_nError(0),
        mpImpl( new FileDialogHelper_Impl( this, nDialogType, nFlags, SFX2_IMPL_DIALOG_CONFIG, pPreferredParent, rPreselectedDir, rDenyList ) )
{
    // the wildcard here is expected in form "*.extension"
    OUString aWildcard;
    if ( aExtName.find( '*' ) != 0 )
    {
        if ( !aExtName.empty() && aExtName.find( '.' ) != 0 )
            aWildcard = "*.";
        else
            aWildcard = "*";
    }

    aWildcard += aExtName;

    OUString const aUIString = ::sfx2::addExtension(
        aFilterUIName, aWildcard, (OPEN == lcl_OpenOrSave(mpImpl->m_nDialogType)), *mpImpl);
    AddFilter( aUIString, aWildcard );
}

FileDialogHelper::~FileDialogHelper()
{
    mpImpl->dispose();
}

void FileDialogHelper::CreateMatcher( const OUString& rFactory )
{
    mpImpl->createMatcher( SfxObjectShell::GetServiceNameFromFactory(rFactory) );
}

void FileDialogHelper::SetControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId )
{
    mpImpl->setControlHelpIds( _pControlId, _pHelpId );
}

void FileDialogHelper::SetContext( Context _eNewContext )
{
    mpImpl->SetContext( _eNewContext );
}

OUString FileDialogHelper::contextToString(Context context)
{
    // These strings are used in the configuration, to store the last used directory for each context.
    // Please don't change them.
    switch(context) {
        case AcceleratorConfig:
            return u"AcceleratorConfig"_ustr;
        case AutoRedact:
            return u"AutoRedact"_ustr;
        case BaseDataSource:
            return u"BaseDataSource"_ustr;
        case BaseSaveAs:
            return u"BaseSaveAs"_ustr;
        case BasicExportDialog:
            return u"BasicExportDialog"_ustr;
        case BasicExportPackage:
            return u"BasicExportPackage"_ustr;
        case BasicExportSource:
            return u"BasicExportSource"_ustr;
        case BasicImportDialog:
            return u"BasicImportDialog"_ustr;
        case BasicImportSource:
            return u"BasicImportSource"_ustr;
        case BasicInsertLib:
            return u"BasicInsertLib"_ustr;
        case BulletsAddImage:
            return u"BulletsAddImage"_ustr;
        case CalcDataProvider:
            return u"CalcDataProvider"_ustr;
        case CalcDataStream:
            return u"CalcDataStream"_ustr;
        case CalcExport:
            return u"CalcExport"_ustr;
        case CalcSaveAs:
            return u"CalcSaveAs"_ustr;
        case CalcXMLSource:
            return u"CalcXMLSource"_ustr;
        case ExportImage:
            return u"ExportImage"_ustr;
        case ExtensionManager:
            return u"ExtensionManager"_ustr;
        case FormsAddInstance:
            return u"FormsAddInstance"_ustr;
        case FormsInsertImage:
            return u"FormsInsertImage"_ustr;
        case LinkClientOLE:
            return u"LinkClientOLE"_ustr;
        case LinkClientFile:
            return u"LinkClientFile"_ustr;
        case DrawImpressInsertFile:
            return u"DrawImpressInsertFile"_ustr;
        case DrawImpressOpenSound:
            return u"DrawImpressOpenSound"_ustr;
        case DrawExport:
            return u"DrawExport"_ustr;
        case DrawSaveAs:
            return u"DrawSaveAs"_ustr;
        case IconImport:
            return u"IconImport"_ustr;
        case ImpressClickAction:
            return u"ImpressClickAction"_ustr;
        case ImpressExport:
            return u"ImpressExport"_ustr;
        case ImpressPhotoDialog:
            return u"ImpressPhotoDialog"_ustr;
        case ImpressSaveAs:
            return u"ImpressSaveAs"_ustr;
        case ImageMap:
            return u"ImageMap"_ustr;
        case InsertDoc:
            return u"InsertDoc"_ustr;
        case InsertImage:
            return u"InsertImage"_ustr;
        case InsertOLE:
            return u"InsertOLE"_ustr;
        case InsertMedia:
            return u"InsertMedia"_ustr;
        case JavaClassPath:
            return u"JavaClassPath"_ustr;
        case ReportInsertImage:
            return u"ReportInsertImage"_ustr;
        case ScreenshotAnnotation:
            return u"ScreenshotAnnotation"_ustr;
        case SignatureLine:
            return u"SignatureLine"_ustr;
        case TemplateImport:
            return u"TemplateImport"_ustr;
        case WriterCreateAddressList:
            return u"WriterCreateAddressList"_ustr;
        case WriterExport:
            return u"WriterExport"_ustr;
        case WriterImportAutotext:
            return u"WriterImportAutotext"_ustr;
        case WriterInsertImage:
            return u"WriterInsertImage"_ustr;
        case WriterInsertScript:
            return u"WriterInsertScript"_ustr;
        case WriterLoadTemplate:
            return u"WriterLoadTemplate"_ustr;
        case WriterMailMerge:
            return u"WriterMailMerge"_ustr;
        case WriterMailMergeSaveAs:
            return u"WriterMailMergeSaveAs"_ustr;
        case WriterNewHTMLGlobalDoc:
            return u"WriterNewHTMLGlobalDoc"_ustr;
        case WriterRegisterDataSource:
            return u"WriterRegisterDataSource"_ustr;
        case WriterSaveAs:
            return u"WriterSaveAs"_ustr;
        case WriterSaveHTML:
            return u"WriterSaveHTML"_ustr;
        case XMLFilterSettings:
            return u"XMLFilterSettings"_ustr;
        case UnknownContext:
        default:
            return u""_ustr;
    }
}

IMPL_LINK_NOARG(FileDialogHelper, ExecuteSystemFilePicker, void*, void)
{
    m_nError = mpImpl->execute();
    m_aDialogClosedLink.Call( this );
}

// rDirPath has to be a directory
ErrCode FileDialogHelper::Execute( css::uno::Sequence<OUString>& rpURLList,
                                   std::optional<SfxAllItemSet>& rpSet,
                                   OUString&       rFilter,
                                   const OUString& rDirPath )
{
    SetDisplayFolder( rDirPath );
    return mpImpl->execute(rpURLList, rpSet, rFilter, SignatureState::UNKNOWN);
}


ErrCode FileDialogHelper::Execute()
{
    return mpImpl->execute();
}

ErrCode FileDialogHelper::Execute( std::optional<SfxAllItemSet>& rpSet,
                                   OUString&       rFilter,
                                   SignatureState const nScriptingSignatureState)
{
    ErrCode nRet;
    css::uno::Sequence<OUString> rURLList;
    nRet = mpImpl->execute(rURLList, rpSet, rFilter, nScriptingSignatureState);
    return nRet;
}

void FileDialogHelper::StartExecuteModal( const Link<FileDialogHelper*,void>& rEndDialogHdl )
{
    m_aDialogClosedLink = rEndDialogHdl;
    m_nError = ERRCODE_NONE;
    if (!mpImpl->isAsyncFilePicker())
        Application::PostUserEvent( LINK( this, FileDialogHelper, ExecuteSystemFilePicker ) );
    else
        mpImpl->implStartExecute();
}

sal_Int16 FileDialogHelper::GetDialogType() const { return mpImpl ? mpImpl->m_nDialogType : 0; }

bool FileDialogHelper::IsPasswordEnabled() const
{
    return mpImpl && mpImpl->isPasswordEnabled();
}

OUString FileDialogHelper::GetRealFilter() const
{
    OUString sFilter;
    if (mpImpl)
        mpImpl->getRealFilter( sFilter );
    return sFilter;
}

bool FileDialogHelper::CheckCurrentFilterOptionsCapability() const
{
    return mpImpl->CheckFilterOptionsCapability(mpImpl->getCurrentSfxFilter());
}

void FileDialogHelper::SetTitle( const OUString& rNewTitle )
{
    if ( mpImpl->mxFileDlg.is() )
        mpImpl->mxFileDlg->setTitle( rNewTitle );
}

OUString FileDialogHelper::GetPath() const
{
    if (mpImpl->mxFileDlg)
        if (auto aPathSeq = mpImpl->mxFileDlg->getSelectedFiles(); aPathSeq.hasElements())
            return aPathSeq[0];

    return {};
}

Sequence< OUString > FileDialogHelper::GetSelectedFiles() const
{
    uno::Reference<XFilePicker3> xFileDlg(mpImpl->mxFileDlg, uno::UNO_SET_THROW);
    return xFileDlg->getSelectedFiles();
}

OUString FileDialogHelper::GetDisplayDirectory() const
{
    return mpImpl->getPath();
}

OUString FileDialogHelper::GetCurrentFilter() const
{
    return mpImpl->getFilter();
}

ErrCode FileDialogHelper::GetGraphic( Graphic& rGraphic ) const
{
    return mpImpl->getGraphic( rGraphic );
}

static int impl_isFolder( const OUString& rPath )
{
    try
    {
        ::ucbhelper::Content aContent(
            rPath, uno::Reference< ucb::XCommandEnvironment > (),
            comphelper::getProcessComponentContext() );
        if ( aContent.isFolder() )
            return 1;

        return 0;
    }
    catch ( const Exception & )
    {
    }

    return -1;
}

void FileDialogHelper::SetDisplayDirectory( const OUString& _rPath )
{
    if ( _rPath.isEmpty() )
        return;

    // if the given path isn't a folder, we cut off the last part
    // and take it as filename and the rest of the path should be
    // the folder

    INetURLObject aObj( _rPath );

    OUString sFileName = aObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset);
    aObj.removeSegment();
    OUString sPath = aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

    int nIsFolder = impl_isFolder( _rPath );
    if ( nIsFolder == 0 ||
         ( nIsFolder == -1 && impl_isFolder( sPath ) == 1 ) )
    {
        mpImpl->setFileName( sFileName );
        mpImpl->displayFolder( sPath );
    }
    else
    {
        INetURLObject aObjPathName( _rPath );
        OUString sFolder( aObjPathName.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        if ( sFolder.isEmpty() )
        {
            // _rPath is not a valid path -> fallback to home directory
            osl::Security aSecurity;
            aSecurity.getHomeDir( sFolder );
        }
        mpImpl->displayFolder( sFolder );
    }
}

void FileDialogHelper::SetDisplayFolder( const OUString& _rURL )
{
    mpImpl->displayFolder( _rURL );
}

void FileDialogHelper::SetFileName( const OUString& _rFileName )
{
    mpImpl->setFileName( _rFileName );
}

void FileDialogHelper::AddFilter( const OUString& rFilterName,
                                  const OUString& rExtension )
{
    mpImpl->addFilter( rFilterName, rExtension );
}

void FileDialogHelper::SetCurrentFilter( const OUString& rFilter )
{
    OUString sFilter( rFilter );
    if ( mpImpl->isShowFilterExtensionEnabled() )
        sFilter = mpImpl->getFilterWithExtension( rFilter );
    mpImpl->setFilter( sFilter );
}

const uno::Reference < XFilePicker3 >& FileDialogHelper::GetFilePicker() const
{
    return mpImpl->mxFileDlg;
}

// XFilePickerListener Methods
void FileDialogHelper::FileSelectionChanged()
{
    mpImpl->handleFileSelectionChanged();
}

void FileDialogHelper::DirectoryChanged()
{
    mpImpl->handleDirectoryChanged();
}

OUString FileDialogHelper::HelpRequested( const FilePickerEvent& aEvent )
{
    return sfx2::FileDialogHelper_Impl::handleHelpRequested( aEvent );
}

void FileDialogHelper::ControlStateChanged( const FilePickerEvent& aEvent )
{
    mpImpl->handleControlStateChanged( aEvent );
}

void FileDialogHelper::DialogSizeChanged()
{
    mpImpl->handleDialogSizeChanged();
}

void FileDialogHelper::DialogClosed( const DialogClosedEvent& _rEvent )
{
    m_nError = ( RET_OK == _rEvent.DialogResult ) ? ERRCODE_NONE : ERRCODE_ABORT;
    m_aDialogClosedLink.Call( this );
}

ErrCode FileOpenDialog_Impl( weld::Window* pParent,
                             sal_Int16 nDialogType,
                             FileDialogFlags nFlags,
                             css::uno::Sequence<OUString>& rpURLList,
                             OUString& rFilter,
                             std::optional<SfxAllItemSet>& rpSet,
                             const OUString* pPath,
                             sal_Int16 nDialog,
                             const css::uno::Sequence< OUString >& rDenyList,
                             std::optional<bool>& rShowFilterDialog )
{
    ErrCode nRet;
    std::unique_ptr<FileDialogHelper> pDialog;
    // Sign existing PDF: only works with PDF files and they are opened
    // read-only to discourage editing (which would invalidate existing
    // signatures).
    if (nFlags & FileDialogFlags::SignPDF)
        pDialog.reset(new FileDialogHelper(nDialogType, nFlags, SfxResId(STR_SFX_FILTERNAME_PDF), u"pdf", u""_ustr, rDenyList, pParent));
    else
        pDialog.reset(new FileDialogHelper(nDialogType, nFlags, OUString(), nDialog, SfxFilterFlags::NONE, SfxFilterFlags::NONE, u""_ustr, rDenyList, pParent));

    OUString aPath;
    if ( pPath )
        aPath = *pPath;

    nRet = pDialog->Execute(rpURLList, rpSet, rFilter, aPath);
    DBG_ASSERT( rFilter.indexOf(": ") == -1, "Old filter name used!");

    if (rpSet && nFlags & FileDialogFlags::SignPDF)
        rpSet->Put(SfxBoolItem(SID_DOC_READONLY, true));

    uno::Reference< ui::dialogs::XFilePickerControlAccess > xExtFileDlg( pDialog->GetFilePicker(), uno::UNO_QUERY );
    uno::Any aVal = xExtFileDlg->getValue( ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS, 0 );
    if (aVal.has<bool>() && pDialog->CheckCurrentFilterOptionsCapability())
        rShowFilterDialog = aVal.get<bool>();

    return nRet;
}

bool IsMSType(const std::shared_ptr<const SfxFilter>& pCurrentFilter)
{
    // TODO: need a save way to distinguish MS filters from other filters
    // for now MS-filters are the only alien filters that support encryption
    return !pCurrentFilter->IsOwnFormat();
}

bool IsOOXML(const std::shared_ptr<const SfxFilter>& pCurrentFilter)
{
    // For OOXML we can use the standard password ("unlimited" characters)
    return IsMSType(pCurrentFilter) && lclSupportsOOXMLEncryption( pCurrentFilter->GetFilterName());
}

ErrCode SetPassword(const std::shared_ptr<const SfxFilter>& pCurrentFilter, SfxItemSet* pSet,
                    const OUString& rPasswordToOpen, std::u16string_view rPasswordToModify,
                    bool bAllowPasswordReset)
{
    const bool bMSType = IsMSType(pCurrentFilter);
    const bool bOOXML = IsOOXML(pCurrentFilter);

    if ( rPasswordToOpen.getLength() )
    {
        css::uno::Sequence< css::beans::NamedValue > aEncryptionData;

        if ( bMSType )
        {
            if (bOOXML)
            {
                ::comphelper::SequenceAsHashMap aHashData;
                aHashData[ u"OOXPassword"_ustr ] <<= rPasswordToOpen;
                aHashData[ u"CryptoType"_ustr ] <<= u"Standard"_ustr;
                aEncryptionData = aHashData.getAsConstNamedValueList();
            }
            else
            {
                uno::Sequence< sal_Int8 > aUniqueID = ::comphelper::DocPasswordHelper::GenerateRandomByteSequence( 16 );
                uno::Sequence< sal_Int8 > aEncryptionKey = ::comphelper::DocPasswordHelper::GenerateStd97Key( rPasswordToOpen, aUniqueID );

                if ( aEncryptionKey.hasElements() )
                {
                    ::comphelper::SequenceAsHashMap aHashData;
                    aHashData[ u"STD97EncryptionKey"_ustr ] <<= aEncryptionKey;
                    aHashData[ u"STD97UniqueID"_ustr ] <<= aUniqueID;

                    aEncryptionData = aHashData.getAsConstNamedValueList();
                }
                else
                {
                    return ERRCODE_IO_NOTSUPPORTED;
                }
            }
        }

        // tdf#118639: We need ODF encryption data for autorecovery where password will already
        // be unavailable, even for non-ODF documents, so append it here unconditionally
        pSet->Put(SfxUnoAnyItem(
            SID_ENCRYPTIONDATA,
            uno::Any(comphelper::concatSequences(
                aEncryptionData, comphelper::OStorageHelper::CreatePackageEncryptionData(
                                    rPasswordToOpen)))));
    }
    else if (bAllowPasswordReset)
    {
        // Remove password

        if (pSet->HasItem(SID_ENCRYPTIONDATA))
            pSet->ClearItem(SID_MODIFYPASSWORDINFO);
        if (pSet->HasItem(SID_ENCRYPTIONDATA))
            pSet->ClearItem(SID_ENCRYPTIONDATA);

        return ERRCODE_NONE;
    }

    if ( bMSType )
    {
        if (bOOXML)
        {
            uno::Sequence<beans::PropertyValue> aModifyPasswordInfo
                = ::comphelper::DocPasswordHelper::GenerateNewModifyPasswordInfoOOXML(
                    rPasswordToModify);
            if (aModifyPasswordInfo.hasElements() && pSet)
                pSet->Put(
                    SfxUnoAnyItem(SID_MODIFYPASSWORDINFO, uno::Any(aModifyPasswordInfo)));
        }
        else
        {
            // the empty password has 0 as Hash
            sal_Int32 nHash = SfxMedium::CreatePasswordToModifyHash(
                rPasswordToModify,
                pCurrentFilter->GetServiceName() == "com.sun.star.text.TextDocument");
            if (nHash && pSet)
                pSet->Put(SfxUnoAnyItem(SID_MODIFYPASSWORDINFO, uno::Any(nHash)));
        }
    }
    else
    {
        uno::Sequence< beans::PropertyValue > aModifyPasswordInfo = ::comphelper::DocPasswordHelper::GenerateNewModifyPasswordInfo( rPasswordToModify );
        if ( aModifyPasswordInfo.hasElements() && pSet)
            pSet->Put( SfxUnoAnyItem( SID_MODIFYPASSWORDINFO, uno::Any( aModifyPasswordInfo ) ) );
    }
    return ERRCODE_NONE;
}



ErrCode RequestPassword(const std::shared_ptr<const SfxFilter>& pCurrentFilter, OUString const & aURL, SfxItemSet* pSet, const css::uno::Reference<css::awt::XWindow>& rParent)
{
    uno::Reference<task::XInteractionHandler2> xInteractionHandler = task::InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), rParent);
    const auto eType = IsMSType(pCurrentFilter) && !IsOOXML(pCurrentFilter) ?
        ::comphelper::DocPasswordRequestType::MS :
        ::comphelper::DocPasswordRequestType::Standard;

    ::rtl::Reference< ::comphelper::DocPasswordRequest > pPasswordRequest( new ::comphelper::DocPasswordRequest( eType, css::task::PasswordRequestMode_PASSWORD_CREATE, aURL, bool( pCurrentFilter->GetFilterFlags() & SfxFilterFlags::PASSWORDTOMODIFY ) ) );

    const bool bMSType = IsMSType(pCurrentFilter);

    do
    {
        xInteractionHandler->handle( uno::Reference< css::task::XInteractionRequest >( pPasswordRequest ) );
        if (!pPasswordRequest->isPassword() || bMSType)
        {
            break;
        }
        OString const utf8Pwd(OUStringToOString(pPasswordRequest->getPassword(), RTL_TEXTENCODING_UTF8));
        OString const utf8Ptm(OUStringToOString(pPasswordRequest->getPasswordToModify(), RTL_TEXTENCODING_UTF8));
        if (!(52 <= utf8Pwd.getLength() && utf8Pwd.getLength() <= 55
                && GetODFSaneDefaultVersion() < SvtSaveOptions::ODFSVER_012)
            && (52 > utf8Ptm.getLength() || utf8Ptm.getLength() > 55))
        {
            break;
        }
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(Application::GetFrameWeld(rParent), VclMessageType::Warning,
            VclButtonsType::Ok, SfxResId(STR_PASSWORD_LEN)));
        xBox->set_secondary_text(SfxResId(STR_PASSWORD_WARNING));
        xBox->run();
    }
    while (true);
    if ( !pPasswordRequest->isPassword() )
        return ERRCODE_ABORT;

    const auto result = SetPassword(pCurrentFilter, pSet, pPasswordRequest->getPassword(), pPasswordRequest->getPasswordToModify());

    if ( result != ERRCODE_IO_NOTSUPPORTED && pPasswordRequest->getRecommendReadOnly() )
        pSet->Put( SfxBoolItem( SID_RECOMMENDREADONLY, true ) );

    return result;
}

OUString EncodeSpaces_Impl( const OUString& rSource )
{
    OUString sRet = rSource.replaceAll( " ", "%20" );
    return sRet;
}

OUString DecodeSpaces_Impl( const OUString& rSource )
{
    OUString sRet = rSource.replaceAll( "%20", " " );
    return sRet;
}

}   // end of namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
