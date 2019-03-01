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
#include <sfx2/filedlghelper.hxx>
#include <sal/types.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XControlInformation.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
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
#include <osl/thread.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/mnemonic.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/saveopt.hxx>
#include <unotools/securityoptions.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/helpids.h>
#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <comphelper/storagehelper.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <sfx2/app.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/docfac.hxx>
#include <openflag.hxx>
#include <sfx2/passwd.hxx>
#include <sfx2/sfxresid.hxx>
#include <sfx2/sfxsids.hrc>
#include "filtergrouping.hxx"
#include <sfx2/request.hxx>
#include "filedlgimpl.hxx"
#include <helpids.h>
#include <sfx2/strings.hrc>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>

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

#define IODLG_CONFIGNAME        "FilePicker_Save"
#define IMPGRF_CONFIGNAME       "FilePicker_Graph"
#define USERITEM_NAME           "UserItem"

namespace sfx2
{

namespace
{
    bool lclSupportsOOXMLEncryption(const OUString& aFilterName)
    {
        return  aFilterName == "Calc MS Excel 2007 XML"
                ||  aFilterName == "MS Word 2007 XML"
                ||  aFilterName == "Impress MS PowerPoint 2007 XML"
                ||  aFilterName == "Impress MS PowerPoint 2007 XML AutoPlay"
                ||  aFilterName == "Calc Office Open XML"
                ||  aFilterName == "Impress Office Open XML"
                ||  aFilterName == "Impress Office Open XML AutoPlay"
                ||  aFilterName == "Office Open XML Text";
    }
}

static const OUString* GetLastFilterConfigId( FileDialogHelper::Context _eContext )
{
    static const OUString aSD_EXPORT_IDENTIFIER("SdExportLastFilter");
    static const OUString aSI_EXPORT_IDENTIFIER("SiExportLastFilter");
    static const OUString aSW_EXPORT_IDENTIFIER("SwExportLastFilter");

    const OUString* pRet = nullptr;

    switch( _eContext )
    {
        case FileDialogHelper::SD_EXPORT: pRet = &aSD_EXPORT_IDENTIFIER; break;
        case FileDialogHelper::SI_EXPORT: pRet = &aSI_EXPORT_IDENTIFIER; break;
        case FileDialogHelper::SW_EXPORT: pRet = &aSW_EXPORT_IDENTIFIER; break;
        default: break;
    }

    return pRet;
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
    //!!! todo: cache the help strings (here or TRA)

    OString sHelpId;
    // mapping from element id -> help id
    switch ( aEvent.ElementId )
    {
        case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION :
            sHelpId = HID_FILESAVE_AUTOEXTENSION;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
            sHelpId = HID_FILESAVE_SAVEWITHPASSWORD;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
            sHelpId = HID_FILESAVE_CUSTOMIZEFILTER;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_READONLY :
            sHelpId = HID_FILEOPEN_READONLY;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_LINK :
            sHelpId = HID_FILEDLG_LINK_CB;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW :
            sHelpId = HID_FILEDLG_PREVIEW_CB;
            break;

        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY :
            sHelpId = HID_FILESAVE_DOPLAY;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_VERSION :
            sHelpId = HID_FILEOPEN_VERSION;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE :
            sHelpId = HID_FILESAVE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE :
            sHelpId = HID_FILEOPEN_IMAGE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_ANCHOR :
            sHelpId = HID_FILEOPEN_IMAGE_ANCHOR;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            sHelpId = HID_FILESAVE_SELECTION;
            break;

        default:
            SAL_WARN( "sfx.dialog", "invalid element id" );
    }

    OUString aHelpText;
    Help* pHelp = Application::GetHelp();
    if ( pHelp )
        aHelpText = pHelp->GetHelpText(OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), static_cast<weld::Widget*>(nullptr));
    return aHelpText;
}

void FileDialogHelper_Impl::handleControlStateChanged( const FilePickerEvent& aEvent )
{
    switch ( aEvent.ElementId )
    {
        case CommonFilePickerElementIds::LISTBOX_FILTER:
            updateFilterOptionsBox();
            enablePasswordBox( false );
            updateSelectionBox();
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
        uno::Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
        if ( xNotifier.is() )
            xNotifier->removeFilePickerListener( this );

        ::comphelper::disposeComponent( mxFileDlg );
        mxFileDlg.clear();
    }
}

OUString FileDialogHelper_Impl::getCurrentFilterUIName() const
{
    OUString aFilterName;
    uno::Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if( xFltMgr.is() )
    {
        aFilterName = xFltMgr->getCurrentFilter();

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
    const OUString* pConfigId = GetLastFilterConfigId( meContext );
    if( pConfigId )
        SvtViewOptions( EViewType::Dialog, IODLG_CONFIGNAME ).SetUserItem( *pConfigId,
                            makeAny( getFilterWithExtension( getFilter() ) ) );
}

std::shared_ptr<const SfxFilter> FileDialogHelper_Impl::getCurentSfxFilter()
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
            OSL_FAIL( "FileDialogHelper_Impl::updateExtendedControl: caught an exception!" );
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
                sal_Int32 nPropertyCount = aProps.getLength();
                for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                {
                    if( aProps[nProperty].Name == "UIComponent" )
                    {
                        aProps[nProperty].Value >>= aServiceName;
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

    updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS,
        CheckFilterOptionsCapability( getCurentSfxFilter() )
    );
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
    if ( CheckFilterOptionsCapability( getCurentSfxFilter() ) )
        sLabel += "...";

    if ( sOldLabel != sLabel )
    {
        try
        {
            xCtrlAccess->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK, sLabel );
        }
        catch( const IllegalArgumentException& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::updateExportButton: caught an exception " << exceptionToString(ex) );
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
        sal_uInt32 nCount = aCtrlList.getLength();
        for ( sal_uInt32 nCtrl = 0; nCtrl < nCount; ++nCtrl )
            if ( aCtrlList[ nCtrl ] == "SelectionBox" )
            {
                bSelectionBoxFound = true;
                break;
            }
    }

    if ( bSelectionBoxFound )
    {
        std::shared_ptr<const SfxFilter> pFilter = getCurentSfxFilter();
        mbSelectionFltrEnabled = updateExtendedControl(
            ExtendedFilePickerElementIds::CHECKBOX_SELECTION,
            ( mbSelectionEnabled && pFilter && ( pFilter->GetFilterFlags() & SfxFilterFlags::SUPPORTSSELECTION ) ) );
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0, makeAny( mbSelection ) );
    }
}

void FileDialogHelper_Impl::enablePasswordBox( bool bInit )
{
    if ( ! mbHasPassword )
        return;

    bool bWasEnabled = mbIsPwdEnabled;

    std::shared_ptr<const SfxFilter> pCurrentFilter = getCurentSfxFilter();
    mbIsPwdEnabled = updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_PASSWORD,
        pCurrentFilter && ( pCurrentFilter->GetFilterFlags() & SfxFilterFlags::ENCRYPTION )
    );

    if( bInit )
    {
        // in case of initialization previous state is not interesting
        if( mbIsPwdEnabled )
        {
            uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
            if( mbPwdCheckBoxState )
                xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( true ) );
        }
    }
    else if( !bWasEnabled && mbIsPwdEnabled )
    {
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        if( mbPwdCheckBoxState )
            xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( true ) );
    }
    else if( bWasEnabled && !mbIsPwdEnabled )
    {
        // remember user settings until checkbox is enabled
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
        bool bPassWord = false;
        mbPwdCheckBoxState = ( aValue >>= bPassWord ) && bPassWord;
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( false ) );
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
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::updatePreviewState: caught an exception " << exceptionToString(ex) );
    }
}

void FileDialogHelper_Impl::updateVersions()
{
    Sequence < OUString > aEntries;
    Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

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

                DBG_ASSERT( xStorage.is(), "The method must return the storage or throw an exception!" );
                if ( !xStorage.is() )
                    throw uno::RuntimeException();

                uno::Sequence < util::RevisionTag > xVersions = SfxMedium::GetVersionList( xStorage );

                aEntries.realloc( xVersions.getLength() + 1 );
                aEntries[0] = SfxResId( STR_SFX_FILEDLG_ACTUALVERSION );

                for ( sal_Int32 i=0; i<xVersions.getLength(); i++ )
                    aEntries[ i + 1 ] = xVersions[i].Identifier;
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

    sal_Int32 nCount = aEntries.getLength();

    if ( !nCount )
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

    Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

    if ( mbShowPreview && ( aPathSeq.getLength() == 1 ) )
    {
        OUString    aURL = aPathSeq[0];

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
                                           Graphic& rGraphic ) const
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

    ErrCode nRet = ERRCODE_NONE;

    GraphicFilterImportFlags nFilterImportFlags = GraphicFilterImportFlags::SetLogsizeForJpeg;
    // non-local?
    if ( INetProtocol::File != aURLObj.GetProtocol() )
    {
        std::unique_ptr<SvStream> pStream = ::utl::UcbStreamHelper::CreateStream( rURL, StreamMode::READ );

        if( pStream )
            nRet = mpGraphicFilter->ImportGraphic( rGraphic, rURL, *pStream, nFilter, nullptr, nFilterImportFlags );
        else
            nRet = mpGraphicFilter->ImportGraphic( rGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags );
    }
    else
    {
        nRet = mpGraphicFilter->ImportGraphic( rGraphic, aURLObj, nFilter, nullptr, nFilterImportFlags );
    }

    return nRet;
}

ErrCode FileDialogHelper_Impl::getGraphic( Graphic& rGraphic ) const
{
    ErrCode nRet = ERRCODE_NONE;

    // rhbz#1079672 do not return maGraphic, it need not be the selected file

    OUString aPath;
    Sequence<OUString> aPathSeq = mxFileDlg->getFiles();

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

static bool lcl_isSystemFilePicker( const uno::Reference< XFilePicker3 >& _rxFP )
{
    try
    {
        uno::Reference< XServiceInfo > xSI( _rxFP, UNO_QUERY );
        if ( !xSI.is() )
            return true;
        return xSI->supportsService( "com.sun.star.ui.dialogs.SystemFilePicker" );
    }
    catch( const Exception& )
    {
    }
    return false;
}

enum open_or_save_t {OPEN, SAVE, UNDEFINED};
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
    const OUString& sStandardDir,
    const css::uno::Sequence< OUString >& rBlackList
    )
    :m_nDialogType          ( nDialogType )
    ,meContext              ( FileDialogHelper::UNKNOWN_CONTEXT )
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
    m_bHaveFilterOptions    = false;
    mbIsPwdEnabled          = true;
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
    mnPostUserEventId = nullptr;

    // create the picker component
    mxFileDlg.set(xFactory->createInstance( aService ), css::uno::UNO_QUERY);
    mbSystemPicker = lcl_isSystemFilePicker( mxFileDlg );

    uno::Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
    uno::Reference< XInitialization > xInit( mxFileDlg, UNO_QUERY );

    if ( ! mxFileDlg.is() || ! xNotifier.is() )
    {
        return;
    }


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
                break;

            case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS;
                mbHasPassword = true;

                m_bHaveFilterOptions = true;
                if( xFactory.is() )
                {
                    mxFilterCFG.set(
                        xFactory->createInstance( "com.sun.star.document.FilterFactory" ),
                        UNO_QUERY );
                }

                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                break;

            case FILESAVE_AUTOEXTENSION_SELECTION:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
                mbHasAutoExt = true;
                mbIsSaveDlg = true;
                mbHasSelectionBox = true;
                if ( mbExport && !mxFilterCFG.is() && xFactory.is() )
                {
                    mxFilterCFG.set(
                        xFactory->createInstance( "com.sun.star.document.FilterFactory" ),
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

        Sequence < Any > aInitArguments(!xWindow.is() ? 3 : 4);

        // This is a hack. We currently know that the internal file picker implementation
        // supports the extended arguments as specified below.
        // TODO:
        // a) adjust the service description so that it includes the TemplateDescription and ParentWindow args
        // b) adjust the implementation of the system file picker to that it recognizes it
        if ( mbSystemPicker )
        {
            aInitArguments[0] <<= nTemplateDescription;
            if (xWindow.is())
                aInitArguments[1] <<= xWindow;
        }
        else
        {
            aInitArguments[0] <<= NamedValue(
                                    "TemplateDescription",
                                    makeAny( nTemplateDescription )
                                );

            aInitArguments[1] <<= NamedValue(
                                    "StandardDir",
                                    makeAny( sStandardDir )
                                );

            aInitArguments[2] <<= NamedValue(
                                    "BlackList",
                                    makeAny( rBlackList )
                                );


            if (xWindow.is())
                aInitArguments[3] <<= NamedValue("ParentWindow", makeAny(xWindow));
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
    xNotifier->addFilePickerListener( this );
}

FileDialogHelper_Impl::~FileDialogHelper_Impl()
{
    // Remove user event if we haven't received it yet
    if ( mnPostUserEventId )
        Application::RemoveUserEvent( mnPostUserEventId );
    mnPostUserEventId = nullptr;

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
                OUString sId( sHelpIdPrefix );
                sId += OUString( *_pHelpId, strlen( *_pHelpId ), RTL_TEXTENCODING_UTF8 );
                xControlAccess->setValue( *_pControlId, ControlActions::SET_HELP_URL, makeAny( sId ) );

                ++_pControlId; ++_pHelpId;
            }
        }
    }
    catch( const Exception& )
    {
        OSL_FAIL( "FileDialogHelper_Impl::setControlHelpIds: caught an exception while setting the help ids!" );
    }
}

IMPL_LINK_NOARG( FileDialogHelper_Impl, InitControls, void*, void )
{
    mnPostUserEventId = nullptr;
    enablePasswordBox( true );
    updateFilterOptionsBox( );
    updateSelectionBox( );
}

void FileDialogHelper_Impl::preExecute()
{
    loadConfig( );
    setDefaultValues( );
    updatePreviewState( false );

    implInitializeFileName( );

#if !(defined(MACOSX) && defined(MACOSX)) && !defined(_WIN32)
    // allow for dialog implementations which need to be executed before they return valid values for
    // current filter and such

    // On Vista (at least SP1) it's the same as on MacOSX, the modal dialog won't let message pass
    // through before it returns from execution
    mnPostUserEventId = Application::PostUserEvent( LINK( this, FileDialogHelper_Impl, InitControls ) );
#else
    // However, the macOS implementation's pickers run modally in execute and so the event doesn't
    // get through in time... so we call the methods directly
    enablePasswordBox( true );
    updateFilterOptionsBox( );
    updateSelectionBox( );
#endif
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
                mxFileDlg->setDefaultName( aObj.GetName( INetURLObject::DecodeMechanism::WithCharset ) );
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
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::implDoExecute: caught an exception " << exceptionToString(ex) );
        }
    }

    postExecute( nRet );

    return nRet;
}

void FileDialogHelper_Impl::implStartExecute()
{
    DBG_ASSERT( mxFileDlg.is(), "invalid file dialog" );

    preExecute();

    if ( mbSystemPicker )
    {
    }
    else
    {
        try
        {
            uno::Reference< XAsynchronousExecutableDialog > xAsyncDlg( mxFileDlg, UNO_QUERY );
            if ( xAsyncDlg.is() )
                xAsyncDlg->startExecuteModal( this );
        }
        catch( const Exception& )
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::implDoExecute: caught an exception " << exceptionToString(ex) );
        }
    }
}

static void lcl_saveLastURLs(std::vector<OUString>& rpURLList,
                      ::std::vector< OUString >& lLastURLs )
{
    lLastURLs.clear();
    for (auto const& url : rpURLList)
        lLastURLs.push_back(url);
}

void FileDialogHelper_Impl::implGetAndCacheFiles(const uno::Reference< XInterface >& xPicker, std::vector<OUString>& rpURLList, const std::shared_ptr<const SfxFilter>& pFilter)
{
    rpURLList.clear();

    OUString sExtension;
    if (pFilter)
    {
        sExtension = pFilter->GetDefaultExtension ();
        sExtension = sExtension.replaceAll("*", "").replaceAll(".", "");
    }

    // a) the new way (optional!)
    uno::Reference< XFilePicker3 > xPickNew(xPicker, UNO_QUERY);
    if (xPickNew.is())
    {
        Sequence< OUString > lFiles    = xPickNew->getSelectedFiles();
        ::sal_Int32          nFiles    = lFiles.getLength();
        for(sal_Int32 i = 0; i < nFiles; ++i)
            rpURLList.push_back(lFiles[i]);
    }

    // b) the olde way ... non optional.
    else
    {
        uno::Reference< XFilePicker3 > xPickOld(xPicker, UNO_QUERY_THROW);
        Sequence< OUString > lFiles = xPickOld->getFiles();
        ::sal_Int32          nFiles = lFiles.getLength();
        if ( nFiles == 1 )
        {
            rpURLList.push_back(lFiles[0]);
        }
        else if ( nFiles > 1 )
        {
            INetURLObject aPath( lFiles[0] );
            aPath.setFinalSlash();

            for (::sal_Int32 i = 1; i < nFiles; i++)
            {
                if (i == 1)
                    aPath.Append( lFiles[i] );
                else
                    aPath.setName( lFiles[i] );

                rpURLList.push_back(aPath.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            }
        }
    }

    lcl_saveLastURLs(rpURLList, mlLastURLs);
}

ErrCode FileDialogHelper_Impl::execute( std::vector<OUString>& rpURLList,
                                        std::unique_ptr<SfxItemSet>& rpSet,
                                        OUString&       rFilter )
{
    // rFilter is a pure output parameter, it shouldn't be used for anything else
    // changing this would surely break code
    // rpSet is in/out parameter, usually just a media-descriptor that can be changed by dialog

    uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

    // retrieves parameters from rpSet
    // for now only Password is used
    if ( rpSet )
    {
        // check password checkbox if the document had password before
        if( mbHasPassword )
        {
            const SfxBoolItem* pPassItem = SfxItemSet::GetItem<SfxBoolItem>(rpSet.get(), SID_PASSWORDINTERACTION, false);
            mbPwdCheckBoxState = ( pPassItem != nullptr && pPassItem->GetValue() );

            // in case the document has password to modify, the dialog should be shown
            const SfxUnoAnyItem* pPassToModifyItem = SfxItemSet::GetItem<SfxUnoAnyItem>(rpSet.get(), SID_MODIFYPASSWORDINFO, false);
            mbPwdCheckBoxState |= ( pPassToModifyItem && pPassToModifyItem->GetValue().hasValue() );
        }

        const SfxBoolItem* pSelectItem = SfxItemSet::GetItem<SfxBoolItem>(rpSet.get(), SID_SELECTION, false);
        if ( pSelectItem )
            mbSelection = pSelectItem->GetValue();
        else
            mbSelectionEnabled = false;

        // the password will be set in case user decide so
        rpSet->ClearItem( SID_PASSWORDINTERACTION );
        rpSet->ClearItem( SID_PASSWORD );
        rpSet->ClearItem( SID_ENCRYPTIONDATA );
        rpSet->ClearItem( SID_RECOMMENDREADONLY );
        rpSet->ClearItem( SID_MODIFYPASSWORDINFO );

    }

    if ( mbHasPassword && !mbPwdCheckBoxState )
    {
        SvtSecurityOptions aSecOpt;
        mbPwdCheckBoxState = (
            aSecOpt.IsOptionSet( SvtSecurityOptions::EOption::DocWarnRecommendPassword ) );
    }

    rpURLList.clear();

    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    if ( ExecutableDialogResults::CANCEL != implDoExecute() )
    {
        // create an itemset if there is no
        if( !rpSet )
            rpSet.reset(new SfxAllItemSet( SfxGetpApp()->GetPool() ));

        // the item should remain only if it was set by the dialog
        rpSet->ClearItem( SID_SELECTION );

        if( mbExport && mbHasSelectionBox )
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
                OSL_FAIL( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
            }
        }


        // set the read-only flag. When inserting a file, this flag is always set
        if ( mbInsert )
            rpSet->Put( SfxBoolItem( SID_DOC_READONLY, true ) );
        else
        {
            if ( ( FILEOPEN_READONLY_VERSION == m_nDialogType ) && xCtrlAccess.is() )
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
                    OSL_FAIL( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
                }
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

        std::shared_ptr<const SfxFilter> pCurrentFilter = getCurentSfxFilter();

        // fill the rpURLList
        implGetAndCacheFiles( mxFileDlg, rpURLList, pCurrentFilter );
        if ( rpURLList.empty() )
            return ERRCODE_ABORT;

        // check, whether or not we have to display a password box
        if ( pCurrentFilter && mbHasPassword && mbIsPwdEnabled && xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
                bool bPassWord = false;
                if ( ( aValue >>= bPassWord ) && bPassWord )
                {
                    // ask for a password
                    OUString aDocName(rpURLList[0]);
                    ErrCode errCode = RequestPassword(pCurrentFilter, aDocName, rpSet.get(), GetFrameInterface());
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
                            aEncryptionData = ::comphelper::OStorageHelper::CreateGpgPackageEncryptionData();
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
                        rpSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aEncryptionData) ) );
                }
            }
            catch( const IllegalArgumentException& ){}
        }

        SaveLastUsedFilter();
        return ERRCODE_NONE;
    }
    else
        return ERRCODE_ABORT;
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
            OSL_FAIL( "FileDialogHelper_Impl::displayFolder: caught an exception!" );
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
            OSL_FAIL( "FileDialogHelper_Impl::setFileName: caught an exception!" );
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

    uno::Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( !maCurFilter.isEmpty() && xFltMgr.is() )
    {
        try
        {
            xFltMgr->setCurrentFilter( maCurFilter );
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
    uno::Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
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
        xSMGR->createInstance("com.sun.star.document.FilterFactory"),
        UNO_QUERY);
    if ( ! xFilterCont.is() )
        return;

    m_nMustFlags |= nMust;
    m_nDontFlags |= nDont;

    // create the list of filters
    OUStringBuffer sQuery(256);
    sQuery.append("getSortedFilterList()");
    sQuery.append(":module=");
    sQuery.append(rFactory); // use long name here !
    sQuery.append(":iflags=");
    sQuery.append(OUString::number(static_cast<sal_Int32>(m_nMustFlags)));
    sQuery.append(":eflags=");
    sQuery.append(OUString::number(static_cast<sal_Int32>(m_nDontFlags)));

    uno::Reference< XEnumeration > xResult;
    try
    {
        xResult = xFilterCont->createSubSetEnumerationByQuery(sQuery.makeStringAndClear());
    }
    catch( const uno::Exception& )
    {
        SAL_WARN( "sfx.dialog", "Could not get filters from the configuration!" );
    }

    TSortedFilterList         aIter   (xResult);

    // append the filters
    OUString sFirstFilter;
    if (OPEN == lcl_OpenOrSave(m_nDialogType))
        ::sfx2::appendFiltersForOpen( aIter, xFltMgr, sFirstFilter, *this );
    else if ( mbExport )
        ::sfx2::appendExportFilters( aIter, xFltMgr, sFirstFilter, *this );
    else
        ::sfx2::appendFiltersForSave( aIter, xFltMgr, sFirstFilter, *this, rFactory );

    // set our initial selected filter (if we do not already have one)
    if ( maSelectFilter.isEmpty() )
        maSelectFilter = sFirstFilter;
}

void FileDialogHelper_Impl::addFilter( const OUString& rFilterName,
                                       const OUString& rExtension )
{
    uno::Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    try
    {
        xFltMgr->appendFilter( rFilterName, rExtension );

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
    uno::Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
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
        OUString aAllFilterName = SfxResId( STR_SFX_IMPORT_ALL );
        aAllFilterName = ::sfx2::addExtension( aAllFilterName, aExtensions, bIsInOpenMode, *this );

        xFltMgr->appendFilter( aAllFilterName, aExtensions );
        maSelectFilter = aAllFilterName;
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
            xFltMgr->appendFilter( aName, aExt );
        }
        catch( const IllegalArgumentException& )
        {
            SAL_WARN( "sfx.dialog", "Could not append Filter" );
        }
    }
}

#define GRF_CONFIG_STR      "   "
#define STD_CONFIG_STR      "1 "

static void SetToken( OUString& rOrigStr, sal_Int32 nToken, sal_Unicode cTok, const OUString& rStr)
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


void FileDialogHelper_Impl::saveConfig()
{
    uno::Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aDlgOpt( EViewType::Dialog, IMPGRF_CONFIGNAME );
        OUString aUserData(GRF_CONFIG_STR);

        try
        {
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

            aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( aUserData ) );
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
            aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( aUserData ) );
    }

    SfxApplication *pSfxApp = SfxGetpApp();
    pSfxApp->SetLastDir_Impl( getPath() );
}

namespace
{
    OUString getInitPath( const OUString& _rFallback, const sal_Int32 _nFallbackToken )
    {
        SfxApplication *pSfxApp = SfxGetpApp();
        OUString sPath = pSfxApp->GetLastDir_Impl();

        if ( sPath.isEmpty() )
            sPath = _rFallback.getToken( _nFallbackToken, ' ' );

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
                bool bLink = aUserData.getToken( 0, ' ' ).toInt32();
                aValue <<= bLink;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aValue );

                // respect the last "show preview" state
                bool bShowPreview = aUserData.getToken( 1, ' ' ).toInt32();
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
            displayFolder( SvtPathOptions().GetGraphicPath() );
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
            sal_Int32 nFlag = aUserData.getToken( 0, ' ' ).toInt32();
            aValue <<= static_cast<bool>(nFlag);
            try
            {
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue );
            }
            catch( const IllegalArgumentException& ){}
        }

        if( mbHasSelectionBox )
        {
            sal_Int32 nFlag = aUserData.getToken( 2, ' ' ).toInt32();
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
        uno::Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );
        try
        {
            xFltMgr->setCurrentFilter( maSelectFilter );
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
            OSL_FAIL( "FileDialogHelper_Impl::setDefaultValues: caught an exception while setting the display directory!" );
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

OUString FileDialogHelper_Impl::getFilterName( const OUString& rFilterWithExtension ) const
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

OUString FileDialogHelper_Impl::getFilterWithExtension( const OUString& rFilter ) const
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

    const OUString* pConfigId = GetLastFilterConfigId( _eNewContext );
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
    const OUString& rStandardDir,
    const css::uno::Sequence< OUString >& rBlackList,
    weld::Window* pPreferredParent)
    :   m_nError(0),
        mpImpl( new FileDialogHelper_Impl( this, nDialogType, nFlags, nDialog, pPreferredParent, rStandardDir, rBlackList ) )
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
    const OUString& aExtName,
    const OUString& rStandardDir,
    const css::uno::Sequence< OUString >& rBlackList,
    weld::Window* pPreferredParent )
    :   m_nError(0),
        mpImpl( new FileDialogHelper_Impl( this, nDialogType, nFlags, SFX2_IMPL_DIALOG_CONFIG, pPreferredParent, rStandardDir, rBlackList ) )
{
    // the wildcard here is expected in form "*.extension"
    OUString aWildcard;
    if ( aExtName.indexOf( '*' ) != 0 )
    {
        if ( !aExtName.isEmpty() && aExtName.indexOf( '.' ) != 0 )
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

IMPL_LINK_NOARG(FileDialogHelper, ExecuteSystemFilePicker, void*, void)
{
    m_nError = mpImpl->execute();
    m_aDialogClosedLink.Call( this );
}

// rDirPath has to be a directory
ErrCode FileDialogHelper::Execute( std::vector<OUString>& rpURLList,
                                   std::unique_ptr<SfxItemSet>& rpSet,
                                   OUString&       rFilter,
                                   const OUString& rDirPath )
{
    SetDisplayFolder( rDirPath );
    return mpImpl->execute( rpURLList, rpSet, rFilter );
}


ErrCode FileDialogHelper::Execute()
{
    return mpImpl->execute();
}

ErrCode FileDialogHelper::Execute( std::unique_ptr<SfxItemSet>& rpSet,
                                   OUString&       rFilter )
{
    ErrCode nRet;
    std::vector<OUString> rURLList;
    nRet = mpImpl->execute(rURLList, rpSet, rFilter);
    return nRet;
}

void FileDialogHelper::StartExecuteModal( const Link<FileDialogHelper*,void>& rEndDialogHdl )
{
    m_aDialogClosedLink = rEndDialogHdl;
    m_nError = ERRCODE_NONE;
    if ( mpImpl->isSystemFilePicker() )
        Application::PostUserEvent( LINK( this, FileDialogHelper, ExecuteSystemFilePicker ) );
    else
        mpImpl->implStartExecute();
}

sal_Int16 FileDialogHelper::GetDialogType() const { return mpImpl ? mpImpl->m_nDialogType : 0; }

bool FileDialogHelper::IsPasswordEnabled() const
{
    return mpImpl.get() && mpImpl->isPasswordEnabled();
}

OUString FileDialogHelper::GetRealFilter() const
{
    OUString sFilter;
    if (mpImpl)
        mpImpl->getRealFilter( sFilter );
    return sFilter;
}

void FileDialogHelper::SetTitle( const OUString& rNewTitle )
{
    if ( mpImpl->mxFileDlg.is() )
        mpImpl->mxFileDlg->setTitle( rNewTitle );
}

OUString FileDialogHelper::GetPath() const
{
    OUString aPath;

    if ( !mpImpl->mlLastURLs.empty())
        return mpImpl->mlLastURLs[0];

    if ( mpImpl->mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mpImpl->mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
        {
            aPath = aPathSeq[0];
        }
    }

    return aPath;
}

Sequence < OUString > FileDialogHelper::GetMPath() const
{
    if ( !mpImpl->mlLastURLs.empty())
        return comphelper::containerToSequence(mpImpl->mlLastURLs);

    if ( mpImpl->mxFileDlg.is() )
        return mpImpl->mxFileDlg->getFiles();
    else
    {
        Sequence < OUString > aEmpty;
        return aEmpty;
    }
}

Sequence< OUString > FileDialogHelper::GetSelectedFiles() const
{
    // a) the new way (optional!)
    uno::Sequence< OUString > aResultSeq;
    uno::Reference< XFilePicker3 > xPickNew(mpImpl->mxFileDlg, UNO_QUERY);
    if (xPickNew.is())
    {
        aResultSeq = xPickNew->getSelectedFiles();
    }
    // b) the olde way ... non optional.
    else
    {
        uno::Reference< XFilePicker > xPickOld(mpImpl->mxFileDlg, UNO_QUERY_THROW);
        Sequence< OUString > lFiles = xPickOld->getFiles();
        ::sal_Int32          nFiles = lFiles.getLength();
        if ( nFiles > 1 )
        {
            aResultSeq = Sequence< OUString >( nFiles-1 );

            INetURLObject aPath( lFiles[0] );
            aPath.setFinalSlash();

            for (::sal_Int32 i = 1; i < nFiles; i++)
            {
                if (i == 1)
                    aPath.Append( lFiles[i] );
                else
                    aPath.setName( lFiles[i] );

                aResultSeq[i-1] = aPath.GetMainURL( INetURLObject::DecodeMechanism::NONE );
            }
        }
        else
            aResultSeq = lFiles;
    }

    return aResultSeq;
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

    OUString sFileName = aObj.GetName( INetURLObject::DecodeMechanism::WithCharset );
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
                             std::vector<OUString>& rpURLList,
                             OUString& rFilter,
                             std::unique_ptr<SfxItemSet>& rpSet,
                             const OUString* pPath,
                             sal_Int16 nDialog,
                             const OUString& rStandardDir,
                             const css::uno::Sequence< OUString >& rBlackList )
{
    ErrCode nRet;
    std::unique_ptr<FileDialogHelper> pDialog;
    // Sign existing PDF: only works with PDF files and they are opened
    // read-only to discourage editing (which would invalidate existing
    // signatures).
    if (nFlags & FileDialogFlags::SignPDF)
        pDialog.reset(new FileDialogHelper(nDialogType, nFlags, SfxResId(STR_SFX_FILTERNAME_PDF), "pdf", rStandardDir, rBlackList, pParent));
    else
        pDialog.reset(new FileDialogHelper(nDialogType, nFlags, OUString(), nDialog, SfxFilterFlags::NONE, SfxFilterFlags::NONE, rStandardDir, rBlackList, pParent));

    OUString aPath;
    if ( pPath )
        aPath = *pPath;

    nRet = pDialog->Execute(rpURLList, rpSet, rFilter, aPath);
    DBG_ASSERT( rFilter.indexOf(": ") == -1, "Old filter name used!");

    if (rpSet && nFlags & FileDialogFlags::SignPDF)
        rpSet->Put(SfxBoolItem(SID_DOC_READONLY, true));
    return nRet;
}

ErrCode RequestPassword(const std::shared_ptr<const SfxFilter>& pCurrentFilter, OUString const & aURL, SfxItemSet* pSet, const css::uno::Reference<css::awt::XWindow>& rParent)
{
    uno::Reference<task::XInteractionHandler2> xInteractionHandler = task::InteractionHandler::createWithParent(::comphelper::getProcessComponentContext(), rParent);
    // TODO: need a save way to distinguish MS filters from other filters
    // for now MS-filters are the only alien filters that support encryption
    bool bMSType = !pCurrentFilter->IsOwnFormat();
    ::comphelper::DocPasswordRequestType eType = bMSType ?
        ::comphelper::DocPasswordRequestType::MS :
        ::comphelper::DocPasswordRequestType::Standard;

    ::rtl::Reference< ::comphelper::DocPasswordRequest > pPasswordRequest( new ::comphelper::DocPasswordRequest( eType, css::task::PasswordRequestMode_PASSWORD_CREATE, aURL, bool( pCurrentFilter->GetFilterFlags() & SfxFilterFlags::PASSWORDTOMODIFY ) ) );

    uno::Reference< css::task::XInteractionRequest > rRequest( pPasswordRequest.get() );
    do
    {
        xInteractionHandler->handle( rRequest );
        if (!pPasswordRequest->isPassword() || bMSType)
        {
            break;
        }
        OString const utf8Pwd(OUStringToOString(pPasswordRequest->getPassword(), RTL_TEXTENCODING_UTF8));
        OString const utf8Ptm(OUStringToOString(pPasswordRequest->getPasswordToModify(), RTL_TEXTENCODING_UTF8));
        if (!(52 <= utf8Pwd.getLength() && utf8Pwd.getLength() <= 55
                && SvtSaveOptions().GetODFDefaultVersion() < SvtSaveOptions::ODFVER_012)
            && !(52 <= utf8Ptm.getLength() && utf8Ptm.getLength() <= 55))
        {
            break;
        }
        vcl::Window* pWin = Application::GetDefDialogParent();
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr, VclMessageType::Warning,
            VclButtonsType::Ok, SfxResId(STR_PASSWORD_LEN)));
        xBox->set_secondary_text(SfxResId(STR_PASSWORD_WARNING));
        xBox->run();
    }
    while (true);
    if ( pPasswordRequest->isPassword() )
    {
        if ( pPasswordRequest->getPassword().getLength() )
        {
            // TODO/LATER: The filters should show the password dialog themself in future
            if ( bMSType )
            {
                // Check if filter supports OOXML encryption
                if ( lclSupportsOOXMLEncryption( pCurrentFilter->GetFilterName() ) )
                {
                    ::comphelper::SequenceAsHashMap aHashData;
                    aHashData[ OUString( "OOXPassword"  ) ] <<= pPasswordRequest->getPassword();
                    pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aHashData.getAsConstNamedValueList() ) ) );
                }
                else
                {
                    uno::Sequence< sal_Int8 > aUniqueID = ::comphelper::DocPasswordHelper::GenerateRandomByteSequence( 16 );
                    uno::Sequence< sal_Int8 > aEncryptionKey = ::comphelper::DocPasswordHelper::GenerateStd97Key( pPasswordRequest->getPassword(), aUniqueID );

                    if ( aEncryptionKey.getLength() )
                    {
                        ::comphelper::SequenceAsHashMap aHashData;
                        aHashData[ OUString( "STD97EncryptionKey"  ) ] <<= aEncryptionKey;
                        aHashData[ OUString( "STD97UniqueID"  ) ] <<= aUniqueID;

                        pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( aHashData.getAsConstNamedValueList() ) ) );
                    }
                    else
                    {
                        return ERRCODE_IO_NOTSUPPORTED;
                    }
                }
            }
            else
            {
                pSet->Put( SfxUnoAnyItem( SID_ENCRYPTIONDATA, uno::makeAny( ::comphelper::OStorageHelper::CreatePackageEncryptionData( pPasswordRequest->getPassword() ) ) ) );
            }
        }

        if ( pPasswordRequest->getRecommendReadOnly() )
            pSet->Put( SfxBoolItem( SID_RECOMMENDREADONLY, true ) );

        if ( bMSType )
        {
            // the empty password has 0 as Hash
            sal_Int32 nHash = SfxMedium::CreatePasswordToModifyHash( pPasswordRequest->getPasswordToModify(),
                                                                     pCurrentFilter->GetServiceName() == "com.sun.star.text.TextDocument" );
            if ( nHash )
                pSet->Put( SfxUnoAnyItem( SID_MODIFYPASSWORDINFO, uno::makeAny( nHash ) ) );
        }
        else
        {
            uno::Sequence< beans::PropertyValue > aModifyPasswordInfo = ::comphelper::DocPasswordHelper::GenerateNewModifyPasswordInfo( pPasswordRequest->getPasswordToModify() );
            if ( aModifyPasswordInfo.getLength() )
                pSet->Put( SfxUnoAnyItem( SID_MODIFYPASSWORDINFO, uno::makeAny( aModifyPasswordInfo ) ) );
        }
    }
    else
        return ERRCODE_ABORT;
    return ERRCODE_NONE;
}

OUString EncodeSpaces_Impl( const OUString& rSource )
{
    OUString sRet( rSource );
    sRet = sRet.replaceAll( " ", "%20" );
    return sRet;
}

OUString DecodeSpaces_Impl( const OUString& rSource )
{
    OUString sRet( rSource );
    sRet = sRet.replaceAll( "%20", " " );
    return sRet;
}

}   // end of namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
