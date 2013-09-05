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
#include <com/sun/star/ui/dialogs/XFilePicker2.hpp>
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

#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/stillreadwriteinteraction.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <tools/urlobj.hxx>
#include <vcl/help.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/localfilehelper.hxx>
#include <osl/file.hxx>
#include <osl/mutex.hxx>
#include <osl/security.hxx>
#include <osl/thread.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/mnemonic.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/securityoptions.hxx>
#include <svl/itemset.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/graphicfilter.hxx>
#include <unotools/viewoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <svtools/helpid.hrc>
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
#include "openflag.hxx"
#include <sfx2/passwd.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/sfxsids.hrc>
#include "filedlghelper.hrc"
#include "filtergrouping.hxx"
#include <sfx2/request.hxx>
#include "filedlgimpl.hxx"
#include <helpid.hrc>
#include <sfxlocal.hrc>
#include <rtl/strbuf.hxx>

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
using namespace ::rtl;
using namespace ::cppu;

#define IODLG_CONFIGNAME        OUString("FilePicker_Save")
#define IMPGRF_CONFIGNAME       OUString("FilePicker_Graph")
#define USERITEM_NAME           OUString("UserItem")

namespace sfx2
{

namespace
{
    bool lclSupportsOOXMLEncryption(OUString aFilterName)
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

const OUString* GetLastFilterConfigId( FileDialogHelper::Context _eContext )
{
    static const OUString aSD_EXPORT_IDENTIFIER( "SdExportLastFilter"  );
    static const OUString aSI_EXPORT_IDENTIFIER( "SiExportLastFilter"  );
    static const OUString aSW_EXPORT_IDENTIFIER( "SwExportLastFilter"  );

    const OUString* pRet = NULL;

    switch( _eContext )
    {
        case FileDialogHelper::SD_EXPORT: pRet = &aSD_EXPORT_IDENTIFIER; break;
        case FileDialogHelper::SI_EXPORT: pRet = &aSI_EXPORT_IDENTIFIER; break;
        case FileDialogHelper::SW_EXPORT: pRet = &aSW_EXPORT_IDENTIFIER; break;
        default: break;
    }

    return pRet;
}

OUString EncodeSpaces_Impl( const OUString& rSource );
OUString DecodeSpaces_Impl( const OUString& rSource );

// FileDialogHelper_Impl

// XFilePickerListener Methods
void SAL_CALL FileDialogHelper_Impl::fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->FileSelectionChanged( aEvent );
}

void SAL_CALL FileDialogHelper_Impl::directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->DirectoryChanged( aEvent );
}

OUString SAL_CALL FileDialogHelper_Impl::helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    return mpAntiImpl->HelpRequested( aEvent );
}

void SAL_CALL FileDialogHelper_Impl::controlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->ControlStateChanged( aEvent );
}

void SAL_CALL FileDialogHelper_Impl::dialogSizeChanged() throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->DialogSizeChanged();
}

// XDialogClosedListener Methods
void SAL_CALL FileDialogHelper_Impl::dialogClosed( const DialogClosedEvent& _rEvent ) throw ( RuntimeException )
{
    SolarMutexGuard aGuard;
    mpAntiImpl->DialogClosed( _rEvent );
    postExecute( _rEvent.DialogResult );
}

// handle XFilePickerListener events
void FileDialogHelper_Impl::handleFileSelectionChanged( const FilePickerEvent& )
{
    if ( mbHasVersions )
        updateVersions();

    if ( mbShowPreview )
        maPreViewTimer.Start();
}

void FileDialogHelper_Impl::handleDirectoryChanged( const FilePickerEvent& )
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( NULL );
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

        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            sHelpId = HID_FILESAVE_SELECTION;
            break;

        default:
            SAL_WARN( "sfx.dialog", "invalid element id" );
    }

    OUString aHelpText;
    Help* pHelp = Application::GetHelp();
    if ( pHelp )
        aHelpText = pHelp->GetHelpText( OStringToOUString(sHelpId, RTL_TEXTENCODING_UTF8), NULL );
    return aHelpText;
}

void FileDialogHelper_Impl::handleControlStateChanged( const FilePickerEvent& aEvent )
{
    switch ( aEvent.ElementId )
    {
        case CommonFilePickerElementIds::LISTBOX_FILTER:
            updateFilterOptionsBox();
            enablePasswordBox( sal_False );
            updateSelectionBox();
            // only use it for export and with our own dialog
            if ( mbExport && !mbSystemPicker )
                updateExportButton();
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW:
            updatePreviewState();
            break;
    }
}

void FileDialogHelper_Impl::handleDialogSizeChanged()
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( NULL );
}

// XEventListener Methods
void SAL_CALL FileDialogHelper_Impl::disposing( const EventObject& ) throw ( RuntimeException )
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
    SvtViewOptions aDlgOpt( E_DIALOG, IODLG_CONFIGNAME );

    if( aDlgOpt.Exists() )
    {
        OUString    aLastFilter;
        if( aDlgOpt.GetUserItem( _rContextIdentifier ) >>= aLastFilter )
            setFilter( aLastFilter );
    }
}

void FileDialogHelper_Impl::SaveLastUsedFilter( const OUString& _rContextIdentifier )
{
    SvtViewOptions( E_DIALOG, IODLG_CONFIGNAME ).SetUserItem( _rContextIdentifier,
                        makeAny( getFilterWithExtension( getFilter() ) ) );
}

void FileDialogHelper_Impl::SaveLastUsedFilter( void )
{
    const OUString* pConfigId = GetLastFilterConfigId( meContext );
    if( pConfigId )
        SaveLastUsedFilter( *pConfigId );
}

const SfxFilter* FileDialogHelper_Impl::getCurentSfxFilter()
{
    OUString aFilterName = getCurrentFilterUIName();

    const SfxFilter* pFilter = NULL;
    if ( mpMatcher && !aFilterName.isEmpty() )
        pFilter = mpMatcher->GetFilter4UIName( aFilterName, m_nMustFlags, m_nDontFlags );

    return pFilter;
}

sal_Bool FileDialogHelper_Impl::updateExtendedControl( sal_Int16 _nExtendedControlId, sal_Bool _bEnable )
{
    sal_Bool bIsEnabled = sal_False;

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

sal_Bool FileDialogHelper_Impl::CheckFilterOptionsCapability( const SfxFilter* _pFilter )
{
    sal_Bool bResult = sal_False;

    if( mxFilterCFG.is() && _pFilter )
    {
        try {
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
                            bResult = sal_True;
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

sal_Bool FileDialogHelper_Impl::isInOpenMode() const
{
    sal_Bool bRet = sal_False;

    switch ( m_nDialogType )
    {
        case FILEOPEN_SIMPLE:
        case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
        case FILEOPEN_PLAY:
        case FILEOPEN_READONLY_VERSION:
        case FILEOPEN_LINK_PREVIEW:
            bRet = sal_True;
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
    if ( xCtrlAccess.is() )
    {
        OUString sEllipses( "..."  );
        OUString sOldLabel( xCtrlAccess->getLabel( CommonFilePickerElementIds::PUSHBUTTON_OK ) );

        // initialize button label; we need the label with the mnemonic char
        if ( maButtonLabel.isEmpty() || maButtonLabel.indexOf( MNEMONIC_CHAR ) == -1 )
        {
            // cut the ellipses, if necessary
            sal_Int32 nIndex = sOldLabel.indexOf( sEllipses );
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
                SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::updateExportButton: caught an exception!" );
            }
        }
    }
}

void FileDialogHelper_Impl::updateSelectionBox()
{
    if ( !mbHasSelectionBox )
        return;

    // Does the selection box exist?
    sal_Bool bSelectionBoxFound = sal_False;
    uno::Reference< XControlInformation > xCtrlInfo( mxFileDlg, UNO_QUERY );
    if ( xCtrlInfo.is() )
    {
        Sequence< OUString > aCtrlList = xCtrlInfo->getSupportedControls();
        sal_uInt32 nCount = aCtrlList.getLength();
        for ( sal_uInt32 nCtrl = 0; nCtrl < nCount; ++nCtrl )
            if ( aCtrlList[ nCtrl ] == "SelectionBox" )
            {
                bSelectionBoxFound = sal_True;
                break;
            }
    }

    if ( bSelectionBoxFound )
    {
        const SfxFilter* pFilter = getCurentSfxFilter();
        mbSelectionFltrEnabled = updateExtendedControl(
            ExtendedFilePickerElementIds::CHECKBOX_SELECTION,
            ( mbSelectionEnabled && pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_SUPPORTSSELECTION ) != 0 ) );
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0, makeAny( (sal_Bool)mbSelection ) );
    }
}

void FileDialogHelper_Impl::enablePasswordBox( sal_Bool bInit )
{
    if ( ! mbHasPassword )
        return;

    sal_Bool bWasEnabled = mbIsPwdEnabled;

    const SfxFilter* pCurrentFilter = getCurentSfxFilter();
    mbIsPwdEnabled = updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_PASSWORD,
        pCurrentFilter && ( pCurrentFilter->GetFilterFlags() & SFX_FILTER_ENCRYPTION )
    );

    if( bInit )
    {
        // in case of inintialization previous state is not interesting
        if( mbIsPwdEnabled )
        {
            uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
            if( mbPwdCheckBoxState )
                xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( sal_True ) );
        }
    }
    else if( !bWasEnabled && mbIsPwdEnabled )
    {
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        if( mbPwdCheckBoxState )
            xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( sal_True ) );
    }
    else if( bWasEnabled && !mbIsPwdEnabled )
    {
        // remember user settings until checkbox is enabled
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
        sal_Bool bPassWord = sal_False;
        mbPwdCheckBoxState = ( aValue >>= bPassWord ) && bPassWord;
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( sal_False ) );
    }
}

void FileDialogHelper_Impl::updatePreviewState( sal_Bool _bUpdatePreviewWindow )
{
    if ( mbHasPreview )
    {
        uno::Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

        // check, whether or not we have to display a preview
        if ( xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
                sal_Bool bShowPreview = sal_False;

                if ( aValue >>= bShowPreview )
                {
                    mbShowPreview = bShowPreview;

                    // setShowState has currently no effect for the
                    // OpenOffice FilePicker (see svtools/source/filepicker/iodlg.cxx)
                    uno::Reference< XFilePreview > xFilePreview( mxFileDlg, UNO_QUERY );
                    if ( xFilePreview.is() )
                        xFilePreview->setShowState( mbShowPreview );

                    if ( _bUpdatePreviewWindow )
                        TimeOutHdl_Impl( NULL );
                }
            }
            catch( const Exception& )
            {
                SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::updatePreviewState: caught an exception!" );
            }
        }
    }
}

void FileDialogHelper_Impl::updateVersions()
{
    Sequence < OUString > aEntries;
    Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

    if ( aPathSeq.getLength() == 1 )
    {
        INetURLObject aObj( aPathSeq[0] );

        if ( ( aObj.GetProtocol() == INET_PROT_FILE ) &&
            ( utl::UCBContentHelper::IsDocument( aObj.GetMainURL( INetURLObject::NO_DECODE ) ) ) )
        {
            try
            {
                uno::Reference< embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                                                                aObj.GetMainURL( INetURLObject::NO_DECODE ),
                                                                embed::ElementModes::READ );

                DBG_ASSERT( xStorage.is(), "The method must return the storage or throw an exception!" );
                if ( !xStorage.is() )
                    throw uno::RuntimeException();

                uno::Sequence < util::RevisionTag > xVersions = SfxMedium::GetVersionList( xStorage );

                aEntries.realloc( xVersions.getLength() + 1 );
                aEntries[0] = SfxResId( STR_SFX_FILEDLG_ACTUALVERSION ).toString();

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

    if ( nCount )
    {
        try
        {
            aValue <<= aEntries;
            xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                            ControlActions::ADD_ITEMS, aValue );

            Any aPos;
            aPos <<= (sal_Int32) 0;
            xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                            ControlActions::SET_SELECT_ITEM, aPos );
        }
        catch( const IllegalArgumentException& ){}
    }
}

IMPL_LINK_NOARG(FileDialogHelper_Impl, TimeOutHdl_Impl)
{
    if ( !mbHasPreview )
        return 0;

    maGraphic.Clear();

    Any aAny;
    uno::Reference < XFilePreview > xFilePicker( mxFileDlg, UNO_QUERY );

    if ( ! xFilePicker.is() )
        return 0;

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

            Bitmap aBmp = maGraphic.GetBitmap();
            if ( !aBmp.IsEmpty() )
            {
                // scale the bitmap to the correct size
                sal_Int32 nOutWidth  = xFilePicker->getAvailableWidth();
                sal_Int32 nOutHeight = xFilePicker->getAvailableHeight();
                sal_Int32 nBmpWidth  = aBmp.GetSizePixel().Width();
                sal_Int32 nBmpHeight = aBmp.GetSizePixel().Height();

                double nXRatio = (double) nOutWidth / nBmpWidth;
                double nYRatio = (double) nOutHeight / nBmpHeight;

                if ( nXRatio < nYRatio )
                    aBmp.Scale( nXRatio, nXRatio );
                else
                    aBmp.Scale( nYRatio, nYRatio );

                // Convert to true color, to allow CopyPixel
                aBmp.Convert( BMP_CONVERSION_24BIT );

                // and copy it into the Any
                SvMemoryStream aData;

                WriteDIB(aBmp, aData, false, true);

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

    return 0;
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

    if ( aURLObj.HasError() || INET_PROT_NOT_VALID == aURLObj.GetProtocol() )
    {
        aURLObj.SetSmartProtocol( INET_PROT_FILE );
        aURLObj.SetSmartURL( rURL );
    }

    ErrCode nRet = ERRCODE_NONE;

    sal_uInt32 nFilterImportFlags = GRFILTER_I_FLAGS_SET_LOGSIZE_FOR_JPEG;
    // non-local?
    if ( INET_PROT_FILE != aURLObj.GetProtocol() )
    {
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( rURL, STREAM_READ );

        if( pStream )
            nRet = mpGraphicFilter->ImportGraphic( rGraphic, rURL, *pStream, nFilter, NULL, nFilterImportFlags );
        else
            nRet = mpGraphicFilter->ImportGraphic( rGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
        delete pStream;
    }
    else
    {
        nRet = mpGraphicFilter->ImportGraphic( rGraphic, aURLObj, nFilter, NULL, nFilterImportFlags );
    }

    return nRet;
}

ErrCode FileDialogHelper_Impl::getGraphic( Graphic& rGraphic ) const
{
    ErrCode nRet = ERRCODE_NONE;

    if ( ! maGraphic )
    {
        OUString aPath;;
        Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
        {
            aPath = aPathSeq[0];
        }

        if ( !aPath.isEmpty() )
            nRet = getGraphic( aPath, rGraphic );
        else
            nRet = ERRCODE_IO_GENERAL;
    }
    else
        rGraphic = maGraphic;

    return nRet;
}

static bool lcl_isSystemFilePicker( const uno::Reference< XFilePicker >& _rxFP )
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
        case FILEOPEN_PLAY:
        case FILEOPEN_READONLY_VERSION:
        case FILEOPEN_LINK_PREVIEW:
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

FileDialogHelper_Impl::FileDialogHelper_Impl(
    FileDialogHelper* _pAntiImpl,
    sal_Int16 nDialogType,
    sal_Int64 nFlags,
    sal_Int16 nDialog,
    Window* _pPreferredParentWindow,
    const OUString& sStandardDir,
    const ::com::sun::star::uno::Sequence< OUString >& rBlackList
    )
    :m_nDialogType          ( nDialogType )
    ,meContext              ( FileDialogHelper::UNKNOWN_CONTEXT )
{
    const char* pServiceName=0;
    if ( nDialog == SFX2_IMPL_DIALOG_SYSTEM )
        pServiceName = FILE_OPEN_SERVICE_NAME_OOO;
    else if ( nDialog == SFX2_IMPL_DIALOG_OOO )
        pServiceName = FILE_OPEN_SERVICE_NAME_OOO;
    else
        pServiceName = "com.sun.star.ui.dialogs.FilePicker";
    OUString aService = OUString::createFromAscii( pServiceName );

    uno::Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

    // create the file open dialog
    // the flags can be SFXWB_INSERT or SFXWB_MULTISELECTION

    mpPreferredParentWindow = _pPreferredParentWindow;
    mpAntiImpl              = _pAntiImpl;
    mnError                 = ERRCODE_NONE;
    mbHasAutoExt            = sal_False;
    mbHasPassword           = sal_False;
    m_bHaveFilterOptions    = sal_False;
    mbIsPwdEnabled          = sal_True;
    mbHasVersions           = sal_False;
    mbHasPreview            = sal_False;
    mbShowPreview           = sal_False;
    mbAddGraphicFilter      = SFXWB_GRAPHIC == (nFlags & SFXWB_GRAPHIC);
    mbDeleteMatcher         = sal_False;
    mbInsert                = SFXWB_INSERT == ( nFlags & SFXWB_INSERT );
    mbExport                = SFXWB_EXPORT == ( nFlags & SFXWB_EXPORT );
    mbIsSaveDlg             = sal_False;
    mbIsSaveACopyDlg        = SFXWB_SAVEACOPY == ( nFlags & SFXWB_SAVEACOPY );
    mbPwdCheckBoxState      = sal_False;
    mbSelection             = sal_False;
    mbSelectionEnabled      = sal_True;
    mbHasSelectionBox       = sal_False;
    mbSelectionFltrEnabled  = sal_False;

    // default settings
    m_nDontFlags = SFX_FILTER_INTERNAL | SFX_FILTER_NOTINFILEDLG | SFX_FILTER_NOTINSTALLED;
    if (OPEN == lcl_OpenOrSave(m_nDialogType))
        m_nMustFlags = SFX_FILTER_IMPORT;
    else
        m_nMustFlags = SFX_FILTER_EXPORT;


    mpMatcher = NULL;
    mpGraphicFilter = NULL;
    mnPostUserEventId = 0;

    // create the picker component
    mxFileDlg = mxFileDlg.query( xFactory->createInstance( aService ) );
    mbSystemPicker = lcl_isSystemFilePicker( mxFileDlg );

    uno::Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
    uno::Reference< XInitialization > xInit( mxFileDlg, UNO_QUERY );

    if ( ! mxFileDlg.is() || ! xNotifier.is() )
    {
        mnError = ERRCODE_ABORT;
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
                mbIsSaveDlg = sal_True;
                break;

            case FILESAVE_AUTOEXTENSION_PASSWORD:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD;
                mbHasPassword = sal_True;
                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                break;

            case FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS;
                mbHasPassword = sal_True;

                m_bHaveFilterOptions = sal_True;
                if( xFactory.is() )
                {
                    mxFilterCFG = uno::Reference< XNameAccess >(
                        xFactory->createInstance( "com.sun.star.document.FilterFactory" ),
                        UNO_QUERY );
                }

                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                break;

            case FILESAVE_AUTOEXTENSION_SELECTION:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                mbHasSelectionBox = sal_True;
                if ( mbExport && !mxFilterCFG.is() && xFactory.is() )
                {
                    mxFilterCFG = uno::Reference< XNameAccess >(
                        xFactory->createInstance( "com.sun.star.document.FilterFactory" ),
                        UNO_QUERY );
                }
                break;

            case FILESAVE_AUTOEXTENSION_TEMPLATE:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE;
                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                break;

            case FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE:
                nTemplateDescription = TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE;
                mbHasPreview = sal_True;

                // aPreviewTimer
                maPreViewTimer.SetTimeout( 500 );
                maPreViewTimer.SetTimeoutHdl( LINK( this, FileDialogHelper_Impl, TimeOutHdl_Impl ) );
                break;

            case FILEOPEN_PLAY:
                nTemplateDescription = TemplateDescription::FILEOPEN_PLAY;
                break;

            case FILEOPEN_READONLY_VERSION:
                nTemplateDescription = TemplateDescription::FILEOPEN_READONLY_VERSION;
                mbHasVersions = sal_True;
                break;

            case FILEOPEN_LINK_PREVIEW:
                nTemplateDescription = TemplateDescription::FILEOPEN_LINK_PREVIEW;
                mbHasPreview = sal_True;
                // aPreviewTimer
                maPreViewTimer.SetTimeout( 500 );
                maPreViewTimer.SetTimeoutHdl( LINK( this, FileDialogHelper_Impl, TimeOutHdl_Impl ) );
                break;

            case FILESAVE_AUTOEXTENSION:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION;
                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                break;

            default:
                SAL_WARN( "sfx.dialog", "FileDialogHelper::ctor with unknown type" );
                break;
        }

        Sequence < Any > aInitArguments( !mpPreferredParentWindow ? 3 : 4 );

        // This is a hack. We currently know that the internal file picker implementation
        // supports the extended arguments as specified below.
        // TODO:
        // a) adjust the service description so that it includes the TemplateDescription and ParentWindow args
        // b) adjust the implementation of the system file picker to that it recognizes it
        if ( mbSystemPicker )
        {
            aInitArguments[0] <<= nTemplateDescription;
        }
        else
        {
            aInitArguments[0] <<= NamedValue(
                                    OUString( "TemplateDescription"  ),
                                    makeAny( nTemplateDescription )
                                );

            OUString sStandardDirTemp = OUString( sStandardDir );

            aInitArguments[1] <<= NamedValue(
                                    OUString( "StandardDir"  ),
                                    makeAny( sStandardDirTemp )
                                );

            aInitArguments[2] <<= NamedValue(
                                    OUString( "BlackList"  ),
                                    makeAny( rBlackList )
                                );


            if ( mpPreferredParentWindow )
                aInitArguments[3] <<= NamedValue(
                                        OUString( "ParentWindow"  ),
                                        makeAny( VCLUnoHelper::GetInterface( mpPreferredParentWindow ) )
                                    );
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
    if ( nFlags & SFXWB_MULTISELECTION )
        mxFileDlg->setMultiSelectionMode( sal_True );

    if (mbAddGraphicFilter) // generate graphic filter only on demand
    {
        addGraphicFilter();
    }

    // Export dialog
    if ( mbExport )
    {
        mxFileDlg->setTitle( SfxResId( STR_SFX_EXPLORERFILE_EXPORT ).toString() );
        try {
                com::sun::star::uno::Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY_THROW );
                xCtrlAccess->enableControl( ExtendedFilePickerElementIds::LISTBOX_FILTER_SELECTOR, sal_True );
        }
        catch( const Exception & ) { }
    }

    // Save a copy dialog
    if ( mbIsSaveACopyDlg )
    {
        mxFileDlg->setTitle( SfxResId( STR_PB_SAVEACOPY ).toString() );
    }

    // the "insert file" dialog needs another title
    if ( mbInsert )
    {
        mxFileDlg->setTitle( SfxResId( STR_SFX_EXPLORERFILE_INSERT ).toString() );
        uno::Reference < XFilePickerControlAccess > xExtDlg( mxFileDlg, UNO_QUERY );
        if ( xExtDlg.is() )
        {
            try
            {
                xExtDlg->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK,
                                   SfxResId( STR_SFX_EXPLORERFILE_BUTTONINSERT ).toString() );
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
    mnPostUserEventId = 0;

    delete mpGraphicFilter;

    if ( mbDeleteMatcher )
        delete mpMatcher;

    maPreViewTimer.SetTimeoutHdl( Link() );

    ::comphelper::disposeComponent( mxFileDlg );
}

#define nMagic -1

class PickerThread_Impl : public ::osl::Thread
{
    uno::Reference < XFilePicker > mxPicker;
    ::osl::Mutex            maMutex;
    virtual void SAL_CALL   run();
    sal_Int16               mnRet;
public:
                            PickerThread_Impl( const uno::Reference < XFilePicker >& rPicker )
                            : mxPicker( rPicker ), mnRet(nMagic) {}

    sal_Int16               GetReturnValue()
                            { ::osl::MutexGuard aGuard( maMutex ); return mnRet; }

    void                    SetReturnValue( sal_Int16 aRetValue )
                            { ::osl::MutexGuard aGuard( maMutex ); mnRet = aRetValue; }
};

void SAL_CALL PickerThread_Impl::run()
{
    try
    {
        sal_Int16 n = mxPicker->execute();
        SetReturnValue( n );
    }
    catch( const RuntimeException& )
    {
        SetReturnValue( ExecutableDialogResults::CANCEL );
        SAL_WARN( "sfx.dialog", "RuntimeException caught" );
    }
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
                DBG_ASSERT( INetURLObject( OStringToOUString( *_pHelpId, RTL_TEXTENCODING_UTF8 ) ).GetProtocol() == INET_PROT_NOT_VALID, "Wrong HelpId!" );
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

IMPL_LINK( FileDialogHelper_Impl, InitControls, void*, NOTINTERESTEDIN )
{
    (void)NOTINTERESTEDIN;
    mnPostUserEventId = 0;
    enablePasswordBox( sal_True );
    updateFilterOptionsBox( );
    updateSelectionBox( );

    return 0L;
}

void FileDialogHelper_Impl::preExecute()
{
    loadConfig( );
    setDefaultValues( );
    updatePreviewState( sal_False );

    implInitializeFileName( );

#if !(defined(MACOSX) && defined(MACOSX)) && !defined(WNT)
    // allow for dialog implementations which need to be executed before they return valid values for
    // current filter and such

    // On Vista (at least SP1) it's the same as on MacOSX, the modal dialog won't let message pass
    // through before it returns from execution
    mnPostUserEventId = Application::PostUserEvent( LINK( this, FileDialogHelper_Impl, InitControls ) );
#else
    // However, the Mac OS X implementation's pickers run modally in execute and so the event doesn't
    // get through in time... so we call the methods directly
    enablePasswordBox( sal_True );
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
    if ( !maFileName.isEmpty() )
    {
        INetURLObject aObj( maPath );
        aObj.Append( maFileName );

        // in case we're operating as save dialog, and "auto extension" is checked,
        // cut the extension from the name
        if ( mbIsSaveDlg && mbHasAutoExt )
        {
            try
            {
                sal_Bool bAutoExtChecked = sal_False;

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
                        mxFileDlg->setDefaultName( aObj.GetName( INetURLObject::DECODE_WITH_CHARSET ) );
                    }
                }
            }
            catch( const Exception& )
            {
                OSL_FAIL( "FileDialogHelper_Impl::implInitializeFileName: could not ask for the auto-extension current-value!" );
            }
        }
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
#ifdef WNT
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
            SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::implDoExecute: caught an exception!" );
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
            SAL_WARN( "sfx.dialog", "FileDialogHelper_Impl::implDoExecute: caught an exception!" );
        }
    }
}

void lcl_saveLastURLs(std::vector<OUString>& rpURLList,
                      ::comphelper::SequenceAsVector< OUString >& lLastURLs )
{
    lLastURLs.clear();
    for(std::vector<OUString>::iterator i = rpURLList.begin(); i != rpURLList.end(); ++i)
        lLastURLs.push_back(*i);
}

void FileDialogHelper_Impl::implGetAndCacheFiles(const uno::Reference< XInterface >& xPicker, std::vector<OUString>& rpURLList, const SfxFilter* pFilter)
{
    rpURLList.clear();

    OUString sExtension;
    if (pFilter)
    {
        sExtension = pFilter->GetDefaultExtension ();
        sExtension = comphelper::string::remove(sExtension, '*');
        sExtension = comphelper::string::remove(sExtension, '.');
    }

    // a) the new way (optional!)
    uno::Reference< XFilePicker2 > xPickNew(xPicker, UNO_QUERY);
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
        uno::Reference< XFilePicker > xPickOld(xPicker, UNO_QUERY_THROW);
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

                rpURLList.push_back(aPath.GetMainURL(INetURLObject::NO_DECODE));
            }
        }
    }

    lcl_saveLastURLs(rpURLList, mlLastURLs);
}

ErrCode FileDialogHelper_Impl::execute( std::vector<OUString>& rpURLList,
                                        SfxItemSet *&   rpSet,
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
            SFX_ITEMSET_ARG( rpSet, pPassItem, SfxBoolItem, SID_PASSWORDINTERACTION, sal_False );
            mbPwdCheckBoxState = ( pPassItem != NULL && pPassItem->GetValue() );

            // in case the document has password to modify, the dialog should be shown
            SFX_ITEMSET_ARG( rpSet, pPassToModifyItem, SfxUnoAnyItem, SID_MODIFYPASSWORDINFO, sal_False );
            mbPwdCheckBoxState |= ( pPassToModifyItem && pPassToModifyItem->GetValue().hasValue() );
        }

        SFX_ITEMSET_ARG( rpSet, pSelectItem, SfxBoolItem, SID_SELECTION, sal_False );
        if ( pSelectItem )
            mbSelection = pSelectItem->GetValue();
        else
            mbSelectionEnabled = sal_False;

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
            aSecOpt.IsOptionSet( SvtSecurityOptions::E_DOCWARN_RECOMMENDPASSWORD ) );
    }

    rpURLList.clear();

    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    if ( ExecutableDialogResults::CANCEL != implDoExecute() )
    {
        // create an itemset if there is no
        if( !rpSet )
            rpSet = new SfxAllItemSet( SFX_APP()->GetPool() );

        // the item should remain only if it was set by the dialog
        rpSet->ClearItem( SID_SELECTION );

        if( mbExport && mbHasSelectionBox )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
                sal_Bool bSelection = sal_False;
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
            rpSet->Put( SfxBoolItem( SID_DOC_READONLY, sal_True ) );
        else
        {
            if ( ( FILEOPEN_READONLY_VERSION == m_nDialogType ) && xCtrlAccess.is() )
            {
                try
                {
                    Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_READONLY, 0 );
                    sal_Bool bReadOnly = sal_False;
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
                    rpSet->Put( SfxInt16Item( SID_VERSION, (short)nVersion ) );
            }
            catch( const IllegalArgumentException& ){}
        }

        // set the filter
        getRealFilter( rFilter );

        const SfxFilter* pCurrentFilter = getCurentSfxFilter();

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
                sal_Bool bPassWord = sal_False;
                if ( ( aValue >>= bPassWord ) && bPassWord )
                {
                    // ask for a password
                    OUString aDocName(rpURLList[0]);
                    ErrCode errCode = RequestPassword(pCurrentFilter, aDocName, rpSet);
                    if (errCode != ERRCODE_NONE)
                        return errCode;
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
        const SfxFilter* pFilter =
            mpMatcher->GetFilter4UIName( _rFilter, m_nMustFlags, m_nDontFlags );
        _rFilter = pFilter ? pFilter->GetFilterName() : OUString("");
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
    if (url.GetProtocol() != INET_PROT_FILE
        || url.getName(0, true, INetURLObject::DECODE_WITH_CHARSET) != "tmp")
    {
        return;
    }
    if (maFileName.indexOf('/') != -1) {
        SAL_WARN("sfx.dialog", maFileName << " contains /");
        return;
    }
    url.insertName(
        maFileName, false, INetURLObject::LAST_SEGMENT, true,
        INetURLObject::ENCODE_ALL);
    OUString sysPathU;
    osl::FileBase::RC e = osl::FileBase::getSystemPathFromFileURL(
        url.GetMainURL(INetURLObject::NO_DECODE), sysPathU);
    if (e != osl::FileBase::E_None) {
        SAL_WARN(
            "sfx.dialog",
            "getSystemPathFromFileURL("
                << url.GetMainURL(INetURLObject::NO_DECODE) << ") failed with "
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
        SAL_WARN(
            "sfx.dialog",
            "stat(" << sysPathC.getStr() << ") failed with errno " << errno);
        return;
    }
    if ((aFileStat.st_mode & (S_IRWXO | S_IRWXG | S_IRWXU)) == S_IRUSR) {
        maPath = SvtPathOptions().GetWorkPath();
        mxFileDlg->setDisplayDirectory( maPath );
    }
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
        const SfxFilter* pFilter = mpMatcher->GetFilter4FilterName(
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
    mbDeleteMatcher = sal_True;
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
        SfxApplication *pSfxApp = SFX_APP();
        mpMatcher = &pSfxApp->GetFilterMatcher();
        mbDeleteMatcher = sal_False;
    }
    else
    {
        mpMatcher = new SfxFilterMatcher( rFactory );
        mbDeleteMatcher = sal_True;
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
    sQuery.appendAscii(RTL_CONSTASCII_STRINGPARAM("getSortedFilterList()"));
    sQuery.appendAscii(RTL_CONSTASCII_STRINGPARAM(":module="));
    sQuery.append(rFactory); // use long name here !
    sQuery.appendAscii(RTL_CONSTASCII_STRINGPARAM(":iflags="));
    sQuery.append(OUString::number(m_nMustFlags));
    sQuery.appendAscii(RTL_CONSTASCII_STRINGPARAM(":eflags="));
    sQuery.append(OUString::number(m_nDontFlags));

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
    mpGraphicFilter = new GraphicFilter;
    sal_uInt16 i, j, nCount = mpGraphicFilter->GetImportFormatCount();

    // compute the extension string for all known import filters
    OUString aExtensions;

    for ( i = 0; i < nCount; i++ )
    {
        j = 0;
        OUString sWildcard;
        while( true )
        {
            sWildcard = mpGraphicFilter->GetImportWildcard( i, j++ );
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

#if defined(WNT)
    if ( aExtensions.getLength() > 240 )
        aExtensions = FILEDIALOG_FILTER_ALL;
#endif
    sal_Bool bIsInOpenMode = isInOpenMode();

    try
    {
        OUString aAllFilterName = SfxResId( STR_SFX_IMPORT_ALL ).toString();
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
        OUString sWildcard;
        while( true )
        {
            sWildcard = mpGraphicFilter->GetImportWildcard( i, j++ );
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

void FileDialogHelper_Impl::saveConfig()
{
    uno::Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aDlgOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData(GRF_CONFIG_STR);

        try
        {
            aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
            sal_Bool bValue = sal_False;
            aValue >>= bValue;
            aUserData.SetToken( 1, ' ', OUString::number( (sal_Int32) bValue ) );

            INetURLObject aObj( getPath() );

            if ( aObj.GetProtocol() == INET_PROT_FILE )
                aUserData.SetToken( 2, ' ', aObj.GetMainURL( INetURLObject::NO_DECODE ) );

            OUString aFilter = getFilter();
            aFilter = EncodeSpaces_Impl( aFilter );
            aUserData.SetToken( 3, ' ', aFilter );

            aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aUserData ) ) );
        }
        catch( const IllegalArgumentException& ){}
    }
    else
    {
        sal_Bool bWriteConfig = sal_False;
        SvtViewOptions aDlgOpt( E_DIALOG, IODLG_CONFIGNAME );
        String aUserData(STD_CONFIG_STR);

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
                sal_Bool bAutoExt = sal_True;
                aValue >>= bAutoExt;
                aUserData.SetToken( 0, ' ', OUString::number( (sal_Int32) bAutoExt ) );
                bWriteConfig = sal_True;
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( ! mbIsSaveDlg )
        {
            OUString aPath = getPath();
            if ( !aPath.isEmpty() &&
                 utl::LocalFileHelper::IsLocalFile( aPath ) )
            {
                aUserData.SetToken( 1, ' ', aPath );
                bWriteConfig = sal_True;
            }
        }

        if( mbHasSelectionBox && mbSelectionFltrEnabled )
        {
            try
            {
                aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
                sal_Bool bSelection = sal_True;
                aValue >>= bSelection;
                if ( comphelper::string::getTokenCount(aUserData, ' ') < 3 )
                    aUserData += " ";
                aUserData.SetToken( 2, ' ', OUString::number( (sal_Int32) bSelection ) );
                bWriteConfig = sal_True;
            }
            catch( const IllegalArgumentException& ){}
        }

        if ( bWriteConfig )
            aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aUserData ) ) );
    }

    SfxApplication *pSfxApp = SFX_APP();
    pSfxApp->SetLastDir_Impl( getPath() );
}

namespace
{
    static OUString getInitPath( const OUString& _rFallback, const xub_StrLen _nFallbackToken )
    {
        SfxApplication *pSfxApp = SFX_APP();
        OUString sPath = pSfxApp->GetLastDir_Impl();

        if ( sPath.isEmpty() )
            sPath = _rFallback.getToken( _nFallbackToken, ' ' );

        // check if the path points to a valid (accessible) directory
        sal_Bool bValid = sal_False;
        if ( !sPath.isEmpty() )
        {
            OUString sPathCheck( sPath );
            if ( sPathCheck[ sPathCheck.getLength() - 1 ] != '/' )
                sPathCheck += "/";
            sPathCheck += ".";
            try
            {
                ::ucbhelper::Content aContent( sPathCheck, uno::Reference< ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
                bValid = aContent.isFolder();
            }
            catch( const Exception& ) {}
        }

        if ( !bValid )
            sPath = "";

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
        SvtViewOptions aViewOpt( E_DIALOG, IMPGRF_CONFIGNAME );
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
                sal_Bool bLink = (sal_Bool) aUserData.getToken( 0, ' ' ).toInt32();
                aValue <<= bLink;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aValue );

                // respect the last "show preview" state
                sal_Bool bShowPreview = (sal_Bool) aUserData.getToken( 1, ' ' ).toInt32();
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
        SvtViewOptions aViewOpt( E_DIALOG, IODLG_CONFIGNAME );
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
            aValue <<= (sal_Bool) nFlag;
            try
            {
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue );
            }
            catch( const IllegalArgumentException& ){}
        }

        if( mbHasSelectionBox )
        {
            sal_Int32 nFlag = aUserData.getToken( 2, ' ' ).toInt32();
            aValue <<= (sal_Bool) nFlag;
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
    // when no filter is set, we set the curentFilter to <all>
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

sal_Bool FileDialogHelper_Impl::isShowFilterExtensionEnabled() const
{
    return !maFilters.empty();
}

void FileDialogHelper_Impl::addFilterPair( const OUString& rFilter,
                                           const OUString& rFilterWithExtension )
{
    maFilters.push_back( FilterPair( rFilter, rFilterWithExtension ) );

}

OUString FileDialogHelper_Impl::getFilterName( const OUString& rFilterWithExtension ) const
{
    OUString sRet;
    for( ::std::vector< FilterPair >::const_iterator pIter = maFilters.begin(); pIter != maFilters.end(); ++pIter )
    {
        if ( (*pIter).Second == rFilterWithExtension )
        {
            sRet = (*pIter).First;
            break;
        }
    }
    return sRet;
}

OUString FileDialogHelper_Impl::getFilterWithExtension( const OUString& rFilter ) const
{
    OUString sRet;
    for( ::std::vector< FilterPair >::const_iterator pIter = maFilters.begin(); pIter != maFilters.end(); ++pIter )
    {
        if ( (*pIter).First == rFilter )
        {
            sRet = (*pIter).Second;
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
    sal_Int64 nFlags,
    const OUString& rFact,
    SfxFilterFlags nMust,
    SfxFilterFlags nDont )
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters(
            SfxObjectShell::GetServiceNameFromFactory(rFact), nMust, nDont );
}

FileDialogHelper::FileDialogHelper(
    sal_Int16 nDialogType,
    sal_Int64 nFlags,
    const OUString& rFact,
    sal_Int16 nDialog,
    SfxFilterFlags nMust,
    SfxFilterFlags nDont,
    const OUString& rStandardDir,
    const ::com::sun::star::uno::Sequence< OUString >& rBlackList)
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags, nDialog, NULL, rStandardDir, rBlackList );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters(
            SfxObjectShell::GetServiceNameFromFactory(rFact), nMust, nDont );
}

FileDialogHelper::FileDialogHelper(
    sal_Int16 nDialogType,
    sal_Int64 nFlags,
    Window* _pPreferredParent )
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags, SFX2_IMPL_DIALOG_CONFIG, _pPreferredParent );
    mxImp = mpImp;
}

FileDialogHelper::FileDialogHelper(
    sal_Int16 nDialogType,
    sal_Int64 nFlags,
    const OUString& aFilterUIName,
    const OUString& aExtName,
    const OUString& rStandardDir,
    const ::com::sun::star::uno::Sequence< OUString >& rBlackList,
    Window* _pPreferredParent )
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags, SFX2_IMPL_DIALOG_CONFIG, _pPreferredParent,rStandardDir, rBlackList );
    mxImp = mpImp;

    // the wildcard here is expected in form "*.extension"
    OUString aWildcard;
    if ( aExtName.indexOf( (sal_Unicode)'*' ) != 0 )
    {
        if ( !aExtName.isEmpty() && aExtName.indexOf( (sal_Unicode)'.' ) != 0 )
            aWildcard = OUString( "*."  );
        else
            aWildcard = OUString( "*"  );
    }

    aWildcard += aExtName;

    OUString const aUIString = ::sfx2::addExtension( aFilterUIName,
            aWildcard, (OPEN == lcl_OpenOrSave(mpImp->m_nDialogType)), *mpImp);
    AddFilter( aUIString, aWildcard );
}

FileDialogHelper::~FileDialogHelper()
{
    mpImp->dispose();
    mxImp.clear();
}

void FileDialogHelper::CreateMatcher( const OUString& rFactory )
{
    mpImp->createMatcher( SfxObjectShell::GetServiceNameFromFactory(rFactory) );
}

void FileDialogHelper::SetControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId )
{
    mpImp->setControlHelpIds( _pControlId, _pHelpId );
}

void FileDialogHelper::SetContext( Context _eNewContext )
{
    mpImp->SetContext( _eNewContext );
}

IMPL_LINK_NOARG(FileDialogHelper, ExecuteSystemFilePicker)
{
    m_nError = mpImp->execute();
    if ( m_aDialogClosedLink.IsSet() )
        m_aDialogClosedLink.Call( this );

    return 0L;
}

// rDirPath has to be a directory
ErrCode FileDialogHelper::Execute( std::vector<OUString>& rpURLList,
                                   SfxItemSet *&   rpSet,
                                   OUString&       rFilter,
                                   const OUString& rDirPath )
{
    SetDisplayFolder( rDirPath );
    return mpImp->execute( rpURLList, rpSet, rFilter );
}


ErrCode FileDialogHelper::Execute()
{
    return mpImp->execute();
}

ErrCode FileDialogHelper::Execute( SfxItemSet *&   rpSet,
                                   OUString&       rFilter )
{
    ErrCode nRet;
    std::vector<OUString> rURLList;
    nRet = mpImp->execute(rURLList, rpSet, rFilter);
    return nRet;
}

void FileDialogHelper::StartExecuteModal( const Link& rEndDialogHdl )
{
    m_aDialogClosedLink = rEndDialogHdl;
    m_nError = ERRCODE_NONE;
    if ( mpImp->isSystemFilePicker() )
        Application::PostUserEvent( LINK( this, FileDialogHelper, ExecuteSystemFilePicker ) );
    else
        mpImp->implStartExecute();
}


short FileDialogHelper::GetDialogType() const
{
    return mpImp ? mpImp->m_nDialogType : 0;
}

sal_Bool FileDialogHelper::IsPasswordEnabled() const
{
    return mpImp ? mpImp->isPasswordEnabled() : sal_False;
}

OUString FileDialogHelper::GetRealFilter() const
{
    OUString sFilter;
    if ( mpImp )
        mpImp->getRealFilter( sFilter );
    return sFilter;
}

void FileDialogHelper::SetTitle( const OUString& rNewTitle )
{
    if ( mpImp->mxFileDlg.is() )
        mpImp->mxFileDlg->setTitle( rNewTitle );
}

OUString FileDialogHelper::GetPath() const
{
    OUString aPath;

    if ( mpImp->mlLastURLs.size() > 0)
        return mpImp->mlLastURLs[0];

    if ( mpImp->mxFileDlg.is() )
    {
        Sequence < OUString > aPathSeq = mpImp->mxFileDlg->getFiles();

        if ( aPathSeq.getLength() == 1 )
        {
            aPath = aPathSeq[0];
        }
    }

    return aPath;
}

Sequence < OUString > FileDialogHelper::GetMPath() const
{
    if ( mpImp->mlLastURLs.size() > 0)
        return mpImp->mlLastURLs.getAsConstList();

    if ( mpImp->mxFileDlg.is() )
        return mpImp->mxFileDlg->getFiles();
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
    uno::Reference< XFilePicker2 > xPickNew(mpImp->mxFileDlg, UNO_QUERY);
    if (xPickNew.is())
    {
        aResultSeq = xPickNew->getSelectedFiles();
    }
    // b) the olde way ... non optional.
    else
    {
        uno::Reference< XFilePicker > xPickOld(mpImp->mxFileDlg, UNO_QUERY_THROW);
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

                aResultSeq[i-1] = OUString(aPath.GetMainURL( INetURLObject::NO_DECODE ));
            }
        }
        else
            aResultSeq = lFiles;
    }

    return aResultSeq;
}

OUString FileDialogHelper::GetDisplayDirectory() const
{
    return mpImp->getPath();
}

OUString FileDialogHelper::GetCurrentFilter() const
{
    return mpImp->getFilter();
}

ErrCode FileDialogHelper::GetGraphic( Graphic& rGraphic ) const
{
    return mpImp->getGraphic( rGraphic );
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

    OUString sFileName = aObj.GetName( INetURLObject::DECODE_WITH_CHARSET );
    aObj.removeSegment();
    OUString sPath = aObj.GetMainURL( INetURLObject::NO_DECODE );

    int nIsFolder = impl_isFolder( _rPath );
    if ( nIsFolder == 0 ||
         ( nIsFolder == -1 && impl_isFolder( sPath ) == 1 ) )
    {
        mpImp->setFileName( sFileName );
        mpImp->displayFolder( sPath );
    }
    else
    {
        INetURLObject aObjPathName( _rPath );
        OUString sFolder( aObjPathName.GetMainURL( INetURLObject::NO_DECODE ) );
        if ( sFolder.isEmpty() )
        {
            // _rPath is not a valid path -> fallback to home directory
            osl::Security aSecurity;
            aSecurity.getHomeDir( sFolder );
        }
        mpImp->displayFolder( sFolder );
    }
}

void FileDialogHelper::SetDisplayFolder( const OUString& _rURL )
{
    mpImp->displayFolder( _rURL );
}

void FileDialogHelper::SetFileName( const OUString& _rFileName )
{
    mpImp->setFileName( _rFileName );
}

void FileDialogHelper::AddFilter( const OUString& rFilterName,
                                  const OUString& rExtension )
{
    mpImp->addFilter( rFilterName, rExtension );
}

void FileDialogHelper::SetCurrentFilter( const OUString& rFilter )
{
    OUString sFilter( rFilter );
    if ( mpImp->isShowFilterExtensionEnabled() )
        sFilter = mpImp->getFilterWithExtension( rFilter );
    mpImp->setFilter( sFilter );
}

uno::Reference < XFilePicker > FileDialogHelper::GetFilePicker() const
{
    return mpImp->mxFileDlg;
}

// XFilePickerListener Methods
void SAL_CALL FileDialogHelper::FileSelectionChanged( const FilePickerEvent& aEvent )
{
    mpImp->handleFileSelectionChanged( aEvent );
}

void SAL_CALL FileDialogHelper::DirectoryChanged( const FilePickerEvent& aEvent )
{
    mpImp->handleDirectoryChanged( aEvent );
}

OUString SAL_CALL FileDialogHelper::HelpRequested( const FilePickerEvent& aEvent )
{
    return mpImp->handleHelpRequested( aEvent );
}

void SAL_CALL FileDialogHelper::ControlStateChanged( const FilePickerEvent& aEvent )
{
    mpImp->handleControlStateChanged( aEvent );
}

void SAL_CALL FileDialogHelper::DialogSizeChanged()
{
    mpImp->handleDialogSizeChanged();
}

void SAL_CALL FileDialogHelper::DialogClosed( const DialogClosedEvent& _rEvent )
{
    m_nError = ( RET_OK == _rEvent.DialogResult ) ? ERRCODE_NONE : ERRCODE_ABORT;
    if ( m_aDialogClosedLink.IsSet() )
        m_aDialogClosedLink.Call( this );
}

ErrCode FileOpenDialog_Impl( sal_Int16 nDialogType,
                             sal_Int64 nFlags,
                             const OUString& rFact,
                             std::vector<OUString>& rpURLList,
                             OUString& rFilter,
                             SfxItemSet *& rpSet,
                             const OUString* pPath,
                             sal_Int16 nDialog,
                             const OUString& rStandardDir,
                             const ::com::sun::star::uno::Sequence< OUString >& rBlackList )
{
    ErrCode nRet;
    FileDialogHelper aDialog( nDialogType, nFlags,
            rFact, nDialog, 0, 0, rStandardDir, rBlackList );

    OUString aPath;
    if ( pPath )
        aPath = *pPath;

    nRet = aDialog.Execute( rpURLList, rpSet, rFilter, aPath );
    DBG_ASSERT( rFilter.indexOf(": ") == -1, "Old filter name used!");

    return nRet;
}

ErrCode RequestPassword(const SfxFilter* pCurrentFilter, OUString& aURL, SfxItemSet* pSet)
{
    uno::Reference < task::XInteractionHandler2 > xInteractionHandler = task::InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), 0 );
    // TODO: need a save way to distinguish MS filters from other filters
    // for now MS-filters are the only alien filters that support encryption
    sal_Bool bMSType = !pCurrentFilter->IsOwnFormat();
    ::comphelper::DocPasswordRequestType eType = bMSType ?
        ::comphelper::DocPasswordRequestType_MS :
        ::comphelper::DocPasswordRequestType_STANDARD;

    ::rtl::Reference< ::comphelper::DocPasswordRequest > pPasswordRequest( new ::comphelper::DocPasswordRequest( eType, ::com::sun::star::task::PasswordRequestMode_PASSWORD_CREATE, aURL, ( pCurrentFilter->GetFilterFlags() & SFX_FILTER_PASSWORDTOMODIFY ) != 0 ) );

    uno::Reference< com::sun::star::task::XInteractionRequest > rRequest( pPasswordRequest.get() );
    xInteractionHandler->handle( rRequest );
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
                    aHashData[ OUString( "Password"  ) ] <<= pPasswordRequest->getPassword();
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
            pSet->Put( SfxBoolItem( SID_RECOMMENDREADONLY, sal_True ) );

        if ( bMSType )
        {
            // the empty password has 0 as Hash
            sal_Int32 nHash = SfxMedium::CreatePasswordToModifyHash( pPasswordRequest->getPasswordToModify(), OUString( "com.sun.star.text.TextDocument"  ).equals( pCurrentFilter->GetServiceName() ) );
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
