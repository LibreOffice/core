/*************************************************************************
 *
 *  $RCSfile: filedlghelper.cxx,v $
 *
 *  $Revision: 1.105 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 14:52:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FILEDLGHELPER_HXX
#include "filedlghelper.hxx"
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef  _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef  _COM_SUN_STAR_UI_DIALOGS_COMMONFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXECUTABLEDIALOGRESULTS_HPP_
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_EXTENDEDFILEPICKERELEMENTIDS_HPP_
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_FILEPREVIEWIMAGEFORMATS_HPP_
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_CONTROLACTIONS_HPP_
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_TEMPLATEDESCRIPTION_HPP_
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERCONTROLACCESS_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILEPREVIEW_HPP_
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#endif
#ifndef  _COM_SUN_STAR_UI_DIALOGS_XFILTERMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILTERGROUPMANAGER_HPP_
#include <com/sun/star/ui/dialogs/XFilterGroupManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HDL_
#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif

#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif
#ifndef _UNOTOOLS_UCBHELPER_HXX
#include <unotools/ucbhelper.hxx>
#endif
#ifndef _UNOTOOLS_LOCALFILEHELPER_HXX
#include <unotools/localfilehelper.hxx>
#endif
#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_CVTGRF_HXX
#include <vcl/cvtgrf.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _FILTER_HXX
#include <svtools/filter.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#include <svtools/viewoptions.hxx>
#endif
#ifndef _SVT_HELPID_HRC
#include <svtools/helpid.hrc>
#endif
#ifndef _PICKERHELPER_HXX
#include <svtools/pickerhelper.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _SFXAPP_HXX
#include "app.hxx"
#endif
#ifndef _SFXDOCFILE_HXX
#include "docfile.hxx"
#endif
#ifndef _SFX_OBJFAC_HXX
#include "docfac.hxx"
#endif
#ifndef _SFX_OPENFLAG_HXX
#include "openflag.hxx"
#endif
#ifndef _SFX_PASSWD_HXX
#include "passwd.hxx"
#endif
#ifndef _SFX_SFXRESID_HXX
#include "sfxresid.hxx"
#endif
#ifndef _SFXSIDS_HRC
#include "sfxsids.hrc"
#endif
#ifndef _SFX_EXPLORER_HRC
#include "explorer.hrc"
#endif
#ifndef _SFX_FILEDLGHELPER_HRC
#include "filedlghelper.hrc"
#endif
#ifndef SFX2_FILTERGROUPING_HXX
#include "filtergrouping.hxx"
#endif
#ifndef SFX2_REQUEST_HXX
#include "request.hxx"
#endif
#ifndef _SFX_FILEDLGIMPL_HXX
#include "filedlgimpl.hxx"
#endif

//-----------------------------------------------------------------------------

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::rtl;
using namespace ::cppu;

//-----------------------------------------------------------------------------

const short FILEOPEN_SIMPLE = TemplateDescription::FILEOPEN_SIMPLE;
const short FILESAVE_SIMPLE = TemplateDescription::FILESAVE_SIMPLE;
const short FILESAVE_AUTOEXTENSION_PASSWORD = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD;
const short FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS = TemplateDescription::FILESAVE_AUTOEXTENSION_PASSWORD_FILTEROPTIONS;
const short FILESAVE_AUTOEXTENSION_SELECTION = TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
const short FILESAVE_AUTOEXTENSION_TEMPLATE = TemplateDescription::FILESAVE_AUTOEXTENSION_TEMPLATE;
const short FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE = TemplateDescription::FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE;
const short FILEOPEN_PLAY = TemplateDescription::FILEOPEN_PLAY;
const short FILEOPEN_READONLY_VERSION = TemplateDescription::FILEOPEN_READONLY_VERSION;
const short FILEOPEN_LINK_PREVIEW = TemplateDescription::FILEOPEN_LINK_PREVIEW;
const short FILESAVE_AUTOEXTENSION = TemplateDescription::FILESAVE_AUTOEXTENSION;

//-----------------------------------------------------------------------------

#define IODLG_CONFIGNAME        String(DEFINE_CONST_UNICODE("FilePicker_Save"))
#define IMPGRF_CONFIGNAME       String(DEFINE_CONST_UNICODE("FilePicker_Graph"))
#define USERITEM_NAME           ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "UserItem" ))

//-----------------------------------------------------------------------------

namespace sfx2
{

const OUString* GetLastFilterConfigId( FileDialogHelper::Context _eContext )
{
    static const OUString   aSD_EXPORT_IDENTIFIER( RTL_CONSTASCII_USTRINGPARAM( "SdExportLastFilter" ) );
    static const OUString   aSI_EXPORT_IDENTIFIER( RTL_CONSTASCII_USTRINGPARAM( "SiExportLastFilter" ) );

    const OUString* pRet = NULL;

    switch( _eContext )
    {
        case FileDialogHelper::SD_EXPORT:   pRet = &aSD_EXPORT_IDENTIFIER;  break;
        case FileDialogHelper::SI_EXPORT:   pRet = &aSI_EXPORT_IDENTIFIER;  break;
    }

    return pRet;
}

String EncodeSpaces_Impl( const String& rSource );
String DecodeSpaces_Impl( const String& rSource );

// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// XFilePickerListener Methods
// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::fileSelectionChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    mpAntiImpl->FileSelectionChanged( aEvent );
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::directoryChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    mpAntiImpl->DirectoryChanged( aEvent );
}

// ------------------------------------------------------------------------
OUString SAL_CALL FileDialogHelper_Impl::helpRequested( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return mpAntiImpl->HelpRequested( aEvent );
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::controlStateChanged( const FilePickerEvent& aEvent ) throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    mpAntiImpl->ControlStateChanged( aEvent );
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::dialogSizeChanged() throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    mpAntiImpl->DialogSizeChanged();
}

// ------------------------------------------------------------------------
// handle XFilePickerListener events
// ------------------------------------------------------------------------
void FileDialogHelper_Impl::handleFileSelectionChanged( const FilePickerEvent& aEvent )
{
    if ( mbHasVersions )
        updateVersions();

    if ( mbShowPreview )
        maPreViewTimer.Start();
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::handleDirectoryChanged( const FilePickerEvent& aEvent )
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( NULL );
}

// ------------------------------------------------------------------------
OUString FileDialogHelper_Impl::handleHelpRequested( const FilePickerEvent& aEvent )
{
    //!!! todo: cache the help strings (here or TRA)

    ULONG nHelpId = 0;
    // mapping from element id -> help id
    switch ( aEvent.ElementId )
    {
        case ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION :
            nHelpId = HID_FILESAVE_AUTOEXTENSION;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PASSWORD :
            nHelpId = HID_FILESAVE_SAVEWITHPASSWORD;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS :
            nHelpId = HID_FILESAVE_CUSTOMIZEFILTER;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_READONLY :
            nHelpId = HID_FILEOPEN_READONLY;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_LINK :
            nHelpId = HID_FILEDLG_LINK_CB;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_PREVIEW :
            nHelpId = HID_FILEDLG_PREVIEW_CB;
            break;

        case ExtendedFilePickerElementIds::PUSHBUTTON_PLAY :
            nHelpId = HID_FILESAVE_DOPLAY;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_VERSION_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_VERSION :
            nHelpId = HID_FILEOPEN_VERSION;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_TEMPLATE :
            nHelpId = HID_FILESAVE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE_LABEL :
        case ExtendedFilePickerElementIds::LISTBOX_IMAGE_TEMPLATE :
            nHelpId = HID_FILEOPEN_IMAGE_TEMPLATE;
            break;

        case ExtendedFilePickerElementIds::CHECKBOX_SELECTION :
            nHelpId = HID_FILESAVE_SELECTION;
            break;

        default:
            DBG_ERRORFILE( "invalid element id" );
    }

    OUString aHelpText;
    Help* pHelp = Application::GetHelp();
    if ( pHelp )
        aHelpText = String( pHelp->GetHelpText( nHelpId, NULL ) );
    return aHelpText;
}

// ------------------------------------------------------------------------
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

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::handleDialogSizeChanged()
{
    if ( mbShowPreview )
        TimeOutHdl_Impl( NULL );
}

// ------------------------------------------------------------------------
// XEventListener Methods
// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper_Impl::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    dispose();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void FileDialogHelper_Impl::dispose()
{
    if ( mxFileDlg.is() )
    {
        // remove the event listener
        Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
        if ( xNotifier.is() )
            xNotifier->removeFilePickerListener( this );

        ::comphelper::disposeComponent( mxFileDlg );
        mxFileDlg.clear();
    }
}

// ------------------------------------------------------------------------
String FileDialogHelper_Impl::getCurrentFilterUIName() const
{
    String aFilterName;
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if( xFltMgr.is() )
    {
        aFilterName = xFltMgr->getCurrentFilter();

        if ( aFilterName.Len() && isShowFilterExtensionEnabled() )
            aFilterName = getFilterName( aFilterName );
    }

    return aFilterName;
}

// ------------------------------------------------------------------------
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

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::SaveLastUsedFilter( const OUString& _rContextIdentifier )
{
    SvtViewOptions( E_DIALOG, IODLG_CONFIGNAME ).SetUserItem( _rContextIdentifier,
                        makeAny( getFilterWithExtension( getFilter() ) ) );
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::SaveLastUsedFilter( void )
{
    const OUString* pConfigId = GetLastFilterConfigId( meContext );
    if( pConfigId )
        SaveLastUsedFilter( *pConfigId );
}

// ------------------------------------------------------------------------
const SfxFilter* FileDialogHelper_Impl::getCurentSfxFilter()
{
    String aFilterName = getCurrentFilterUIName();

    const SfxFilter* pFilter = NULL;
    if ( mpMatcher && aFilterName.Len() )
        pFilter = mpMatcher->GetFilter4UIName( aFilterName, m_nMustFlags, m_nDontFlags );

    return pFilter;
}

// ------------------------------------------------------------------------
sal_Bool FileDialogHelper_Impl::updateExtendedControl( sal_Int16 _nExtendedControlId, sal_Bool _bEnable )
{
    sal_Bool bIsEnabled = sal_False;

    Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
    if ( xCtrlAccess.is() )
    {
        try
        {
            xCtrlAccess->enableControl( _nExtendedControlId, _bEnable );
            bIsEnabled = _bEnable;
        }
        catch( const IllegalArgumentException& )
        {
            DBG_ERROR( "FileDialogHelper_Impl::updateExtendedControl: caught an exception!" );
        }
    }
    return bIsEnabled;
}

// ------------------------------------------------------------------------
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
                   ::rtl::OUString aServiceName;
                   sal_Int32 nPropertyCount = aProps.getLength();
                   for( sal_Int32 nProperty=0; nProperty < nPropertyCount; ++nProperty )
                {
                       if( aProps[nProperty].Name.equals( DEFINE_CONST_OUSTRING( "UIComponent") ) )
                       {
                        ::rtl::OUString aServiceName;
                           aProps[nProperty].Value >>= aServiceName;
                        if( aServiceName.getLength() )
                            bResult = sal_True;
                    }
                }
            }
        }
        catch( Exception& )
        {
        }
    }

    return bResult;
}

// ------------------------------------------------------------------------
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

// ------------------------------------------------------------------------

void FileDialogHelper_Impl::updateFilterOptionsBox()
{
    if ( !m_bHaveFilterOptions )
        return;

    updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_FILTEROPTIONS,
        CheckFilterOptionsCapability( getCurentSfxFilter() )
    );
}

// ------------------------------------------------------------------------

void FileDialogHelper_Impl::updateExportButton()
{
    Reference < XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
    if ( xCtrlAccess.is() )
    {
        OUString sEllipses( RTL_CONSTASCII_USTRINGPARAM( "..." ) );
        OUString sOldLabel( xCtrlAccess->getLabel( CommonFilePickerElementIds::PUSHBUTTON_OK ) );

        // initialize button label; we need the label with the mnemonic char
        if ( !maButtonLabel.getLength() || maButtonLabel.indexOf( MNEMONIC_CHAR ) == -1 )
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
            sLabel += OUString( RTL_CONSTASCII_USTRINGPARAM( "..." ) );

        if ( sOldLabel != sLabel )
        {
            try
            {
                xCtrlAccess->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK, sLabel );
            }
            catch( const IllegalArgumentException& )
            {
                DBG_ERRORFILE( "FileDialogHelper_Impl::updateExportButton: caught an exception!" );
            }
        }
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::updateSelectionBox()
{
    if ( !mbExport )
        return;

    const SfxFilter* pFilter = getCurentSfxFilter();

    updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_SELECTION,
        ( mbSelectionEnabled && pFilter && ( pFilter->GetFilterFlags() & SFX_FILTER_SUPPORTSSELECTION ) != 0 ) );

    Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
    xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0, makeAny( (sal_Bool)mbSelection ) );
}

// ------------------------------------------------------------------------
struct CheckPasswordCapability
{
    sal_Bool operator() ( const SfxFilter* _pFilter )
    {
        return  _pFilter && _pFilter->IsOwnFormat()
            &&  _pFilter->UsesStorage()
            &&  ( SOFFICE_FILEFORMAT_60 <= _pFilter->GetVersion() );
    }
};

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::enablePasswordBox( sal_Bool bInit )
{
    if ( ! mbHasPassword )
        return;

    sal_Bool bWasEnabled = mbIsPwdEnabled;

    mbIsPwdEnabled = updateExtendedControl(
        ExtendedFilePickerElementIds::CHECKBOX_PASSWORD,
        CheckPasswordCapability()( getCurentSfxFilter() )
    );

    if( bInit )
    {
        // in case of inintialization previous state is not interesting
        if( mbIsPwdEnabled )
        {
            Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
            if( mbPwdCheckBoxState )
                xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( sal_True ) );
        }
    }
    else if( !bWasEnabled && mbIsPwdEnabled )
    {
        Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        if( mbPwdCheckBoxState )
            xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( sal_True ) );
    }
    else if( bWasEnabled && !mbIsPwdEnabled )
    {
        // remember user settings until checkbox is enabled
        Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );
        Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
        sal_Bool bPassWord = sal_False;
        mbPwdCheckBoxState = ( aValue >>= bPassWord ) && bPassWord;
        xCtrlAccess->setValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0, makeAny( sal_False ) );
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::updatePreviewState( sal_Bool _bUpdatePreviewWindow )
{
    if ( mbHasPreview )
    {
        Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

        // check, wether or not we have to display a preview
        if ( xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
                sal_Bool bShowPreview = sal_False;

                if ( aValue >>= bShowPreview )
                {
                    mbShowPreview = bShowPreview;

                    // #97633
                    // setShowState has currently no effect for the
                    // OpenOffice FilePicker (see svtools/source/filepicker/iodlg.cxx)
                    Reference< XFilePreview > xFilePreview( mxFileDlg, UNO_QUERY );
                    if ( xFilePreview.is() )
                        xFilePreview->setShowState( mbShowPreview );

                    if ( _bUpdatePreviewWindow )
                        TimeOutHdl_Impl( NULL );
                }
            }
            catch( Exception )
            {
                DBG_ERRORFILE( "FileDialogHelper_Impl::updatePreviewState: caught an exception!" );
            }
        }
    }
}

// ------------------------------------------------------------------------
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
            SvStorageRef pStor = new SvStorage( FALSE, aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, 0 );
            if( pStor->GetError() == SVSTREAM_OK )
            {
                SfxVersionTableDtor* pVerTable = SfxMedium::GetVersionList( pStor ); // aMed.GetVersionList();

                if ( pVerTable )
                {
                    SvStringsDtor* pVersions = pVerTable->GetVersions();

                    aEntries.realloc( pVersions->Count() + 1 );
                    aEntries[0] = OUString( String ( SfxResId( STR_SFX_FILEDLG_ACTUALVERSION ) ) );

                    for ( USHORT i = 0; i < pVersions->Count(); i++ )
                        aEntries[ i + 1 ] = OUString( *(pVersions->GetObject(i)) );

                    delete pVersions;
                    delete pVerTable;
                }
                else
                {
                    SfxFilterFlags nMust = SFX_FILTER_IMPORT | SFX_FILTER_OWN;
                    SfxFilterFlags nDont = SFX_FILTER_NOTINSTALLED | SFX_FILTER_STARONEFILTER;
                    if ( SFX_APP()->GetFilterMatcher().GetFilter4ClipBoardId( pStor->GetFormat(), nMust, nDont ) )
                    {
                        aEntries.realloc( 1 );
                        aEntries[0] = OUString( String ( SfxResId( STR_SFX_FILEDLG_ACTUALVERSION ) ) );
                    }
                }
            }
        }
    }

    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    try
    {
        xDlg->setValue( ExtendedFilePickerElementIds::LISTBOX_VERSION,
                        ControlActions::DELETE_ITEMS, aValue );
    }
    catch( IllegalArgumentException ){}

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
        catch( IllegalArgumentException ){}
    }
}

// -----------------------------------------------------------------------
class OReleaseSolarMutex
{
private:
    const sal_Int32 m_nAquireCount;
public:
    OReleaseSolarMutex( )
        :m_nAquireCount( Application::ReleaseSolarMutex() )
    {
    }
    ~OReleaseSolarMutex( )
    {
        Application::AcquireSolarMutex( m_nAquireCount );
    }
};

// -----------------------------------------------------------------------
IMPL_LINK( FileDialogHelper_Impl, TimeOutHdl_Impl, Timer*, EMPTYARG )
{
    if ( !mbHasPreview )
        return 0;

    maGraphic.Clear();

    Any aAny;
    Reference < XFilePreview > xFilePicker( mxFileDlg, UNO_QUERY );

    if ( ! xFilePicker.is() )
        return 0;

    Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

    if ( mbShowPreview && ( aPathSeq.getLength() == 1 ) )
    {
        OUString    aURL = aPathSeq[0];

        if ( ERRCODE_NONE == getGraphic( aURL, maGraphic ) )
        {
            // #89491
            // changed the code slightly;
            // before: the bitmap was scaled and
            // surrounded a white frame
            // now: the bitmap will only be scaled
            // and the filepicker implementation
            // is responsible for placing it at its
            // proper position and painting a frame

            Bitmap aBmp = maGraphic.GetBitmap();

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

            // #94505# Convert to true color, to allow CopyPixel
            aBmp.Convert( BMP_CONVERSION_24BIT );

            // and copy it into the Any
            SvMemoryStream aData;

            aData << aBmp;

            Sequence < sal_Int8 > aBuffer( (sal_Int8*) aData.GetData(), aData.GetSize() );

            aAny <<= aBuffer;
        }
    }

    try
    {
        OReleaseSolarMutex aReleaseForCallback;
        // clear the preview window
        xFilePicker->setImage( FilePreviewImageFormats::BITMAP, aAny );
    }
    catch( IllegalArgumentException )
    {
    }

    return 0;
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::getGraphic( const OUString& rURL,
                                           Graphic& rGraphic ) const
{
    if ( utl::UCBContentHelper::IsFolder( rURL ) )
        return ERRCODE_IO_NOTAFILE;

    if ( !mpGraphicFilter )
        return ERRCODE_IO_NOTSUPPORTED;

    // select graphic filter from dialog filter selection
    OUString aCurFilter( getFilter() );

    sal_uInt16 nFilter = aCurFilter.getLength() && mpGraphicFilter->GetImportFormatCount()
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

// ------------------------------------------------------------------------
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

        if ( aPath.getLength() )
            nRet = getGraphic( aPath, rGraphic );
        else
            nRet = ERRCODE_IO_GENERAL;
    }
    else
        rGraphic = maGraphic;

    return nRet;
}

// ------------------------------------------------------------------------
sal_Bool lcl_isSystemFilePicker( const Reference< XFilePicker >& _rxFP )
{
    try
    {
        Reference< XServiceInfo > xSI( _rxFP, UNO_QUERY );
        if ( xSI.is() && xSI->supportsService( DEFINE_CONST_OUSTRING( "com.sun.star.ui.dialogs.SystemFilePicker" ) ) )
            return sal_True;
    }
    catch( const Exception& )
    {
    }
    return sal_False;
}


// ------------------------------------------------------------------------
// -----------      FileDialogHelper_Impl       ---------------------------
// ------------------------------------------------------------------------

FileDialogHelper_Impl::FileDialogHelper_Impl( FileDialogHelper* _pAntiImpl, const short nDialogType, sal_uInt32 nFlags, Window* _pPreferredParentWindow )
    :m_nDialogType          ( nDialogType )
    ,meContext              ( FileDialogHelper::UNKNOWN_CONTEXT )
{
    OUString aService( RTL_CONSTASCII_USTRINGPARAM( FILE_OPEN_SERVICE_NAME ) );

    Reference< XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );

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
    mbHasLink               = sal_False;
    mbDeleteMatcher         = sal_False;
    mbInsert                = SFXWB_INSERT == ( nFlags & SFXWB_INSERT );
    mbExport                = SFXWB_EXPORT == ( nFlags & SFXWB_EXPORT );
    mbIsSaveDlg             = sal_False;
    mbPwdCheckBoxState      = sal_False;
    mbSelection             = sal_False;
    mbSelectionEnabled      = sal_True;

    // default settings
    m_nDontFlags = SFX_FILTER_INTERNAL | SFX_FILTER_NOTINFILEDLG | SFX_FILTER_NOTINSTALLED;
    if( WB_OPEN == ( nFlags & WB_OPEN ) )
        m_nMustFlags = SFX_FILTER_IMPORT;
    else
        m_nMustFlags = SFX_FILTER_EXPORT;


    mpMatcher = NULL;
    mpGraphicFilter = NULL;
    mnPostUserEventId = 0;

    // create the picker component
    mxFileDlg = mxFileDlg.query( xFactory->createInstance( aService ) );
    mbSystemPicker = lcl_isSystemFilePicker( mxFileDlg );

    Reference< XFilePickerNotifier > xNotifier( mxFileDlg, UNO_QUERY );
    Reference< XInitialization > xInit( mxFileDlg, UNO_QUERY );

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
                    mxFilterCFG = Reference< XNameAccess >(
                        xFactory->createInstance( DEFINE_CONST_OUSTRING( "com.sun.star.document.FilterFactory" ) ),
                        UNO_QUERY );
                }

                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                break;

            case FILESAVE_AUTOEXTENSION_SELECTION:
                nTemplateDescription = TemplateDescription::FILESAVE_AUTOEXTENSION_SELECTION;
                mbHasAutoExt = sal_True;
                mbIsSaveDlg = sal_True;
                if ( mbExport && !mxFilterCFG.is() && xFactory.is() )
                {
                    mxFilterCFG = Reference< XNameAccess >(
                        xFactory->createInstance( DEFINE_CONST_OUSTRING( "com.sun.star.document.FilterFactory" ) ),
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
                mbHasLink = sal_True;

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
                mbHasLink = sal_True;
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
                DBG_ERRORFILE( "FileDialogHelper::ctor with unknown type" );
                break;
        }

        Sequence < Any > aInitArguments( mbSystemPicker || !mpPreferredParentWindow ? 1 : 2 );
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
                                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TemplateDescription" ) ),
                                    makeAny( nTemplateDescription )
                                );

            if ( mpPreferredParentWindow )
                aInitArguments[1] <<= NamedValue(
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" ) ),
                                        makeAny( VCLUnoHelper::GetInterface( mpPreferredParentWindow ) )
                                    );
        }

        try
        {
            xInit->initialize( aInitArguments );
        }
        catch( const Exception& )
        {
            DBG_ERROR( "FileDialogHelper_Impl::FileDialogHelper_Impl: could not initialize the picker!" );
        }
    }


    // set multiselection mode
    if ( nFlags & SFXWB_MULTISELECTION )
        mxFileDlg->setMultiSelectionMode( sal_True );

    if ( mbHasLink )        // generate graphic filter only on demand
        addGraphicFilter();

    // Export dialog
    if ( mbExport )
        mxFileDlg->setTitle( OUString( String( SfxResId( STR_SFX_EXPLORERFILE_EXPORT ) ) ) );

    // the "insert file" dialog needs another title
    if ( mbInsert )
    {
        mxFileDlg->setTitle( OUString( String( SfxResId( STR_SFX_EXPLORERFILE_INSERT ) ) ) );
        Reference < XFilePickerControlAccess > xExtDlg( mxFileDlg, UNO_QUERY );
        if ( xExtDlg.is() )
        {
            try
            {
                xExtDlg->setLabel( CommonFilePickerElementIds::PUSHBUTTON_OK,
                                   OUString( String( SfxResId( STR_SFX_EXPLORERFILE_BUTTONINSERT ) ) ) );
            }
            catch( IllegalArgumentException ){}
        }
    }

    // add the event listener
    xNotifier->addFilePickerListener( this );
}

// ------------------------------------------------------------------------
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

#define nMagic (sal_Int16) 0xFFFF

class PickerThread_Impl : public ::vos::OThread
{
    Reference < XFilePicker > mxPicker;
    ::vos::OMutex           maMutex;
    virtual void SAL_CALL   run();
    sal_Int16               mnRet;
public:
                            PickerThread_Impl( const Reference < XFilePicker >& rPicker )
                            : mxPicker( rPicker ), mnRet(nMagic) {}

    sal_Int16               GetReturnValue()
                            { ::vos::OGuard aGuard( maMutex ); return mnRet; }

    void                    SetReturnValue( sal_Int16 aRetValue )
                            { ::vos::OGuard aGuard( maMutex ); mnRet = aRetValue; }
};

void SAL_CALL PickerThread_Impl::run()
{
    try
    {
        sal_Int16 n = mxPicker->execute();
        SetReturnValue( n );
    }
    catch( RuntimeException& )
    {
        SetReturnValue( ExecutableDialogResults::CANCEL );
        DBG_ERRORFILE( "RuntimeException caught" );
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setControlHelpIds( const sal_Int16* _pControlId, const sal_Int32* _pHelpId )
{
    DBG_ASSERT( _pControlId && _pHelpId, "FileDialogHelper_Impl::setControlHelpIds: invalid array pointers!" );
    if ( !_pControlId || !_pHelpId )
        return;

    // forward these ids to the file picker
    try
    {
        const ::rtl::OUString sHelpIdPrefix( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
        // the ids for the single controls
        Reference< XFilePickerControlAccess > xControlAccess( mxFileDlg, UNO_QUERY );
        if ( xControlAccess.is() )
        {
            while ( *_pControlId )
            {
                // calc the help id of the element
                ::rtl::OUString sId( sHelpIdPrefix );
                sId += ::rtl::OUString::valueOf( *_pHelpId );
                // set the help id
                xControlAccess->setValue( *_pControlId, ControlActions::SET_HELP_URL, makeAny( sId ) );

                ++_pControlId; ++_pHelpId;
            }
        }
    }
    catch( const Exception& )
    {
        DBG_ERROR( "FileDialogHelper_Impl::setControlHelpIds: caught an exception while setting the help ids!" );
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setDialogHelpId( const sal_Int32 _nHelpId )
{
    svt::SetDialogHelpId( mxFileDlg, _nHelpId );
}

// ------------------------------------------------------------------------
IMPL_LINK( FileDialogHelper_Impl, InitControls, void*, NOTINTERESTEDIN )
{
    mnPostUserEventId = 0;
    enablePasswordBox( sal_True );
    updateFilterOptionsBox( );
    updateSelectionBox( );

    return 0L;
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::preExecute()
{
    loadConfig( );
    setDefaultValues( );
    updatePreviewState( sal_False );

    implInitializeFileName( );
    // #106079# / 2002-12-09 / fs@openoffice.org

    // allow for dialog implementations which need to be executed before they return valid values for
    // current filter and such
    mnPostUserEventId = Application::PostUserEvent( LINK( this, FileDialogHelper_Impl, InitControls ) );
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::postExecute( sal_Int16 _nResult )
{
    if ( ExecutableDialogResults::CANCEL != _nResult )
        saveConfig();
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::implInitializeFileName( )
{
    if ( maFileName.getLength() )
    {
        INetURLObject aObj( maPath );
        aObj.Append( maFileName );

        // in case we're operating as save dialog, and "auto extension" is checked,
        // cut the extension from the name
        // #106079# / 2002-12-09 / fs@openoffice.org
        if ( mbIsSaveDlg && mbHasAutoExt )
        {
            try
            {
                sal_Bool bAutoExtChecked = sal_False;

                Reference < XFilePickerControlAccess > xControlAccess( mxFileDlg, UNO_QUERY );
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
            catch( const Exception& e )
            {
                e;  // make compiler happy
                DBG_ERROR( "FileDialogHelper_Impl::implInitializeFileName: could not ask for the auto-extension current-value!" );
            }
        }
    }
}

// ------------------------------------------------------------------------
sal_Int16 FileDialogHelper_Impl::implDoExecute()
{
    preExecute();

    sal_Int16 nRet = ExecutableDialogResults::CANCEL;

    if ( mbSystemPicker )
    {
        PickerThread_Impl* pThread = new PickerThread_Impl( mxFileDlg );
        pThread->create();
        while ( pThread->GetReturnValue() == nMagic )
            Application::Yield();
        pThread->join();
        nRet = pThread->GetReturnValue();
        delete pThread;
    }
    else
    {
        try
        {
            nRet = mxFileDlg->execute();
        }
        catch( const Exception& )
        {
            DBG_ERRORFILE( "FileDialogHelper_Impl::implDoExecute: caught an exception!" );
        }
    }

    postExecute( nRet );

    return nRet;
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper_Impl::execute( SvStringsDtor*& rpURLList,
                                        SfxItemSet *&   rpSet,
                                        String&         rFilter )
{
    // rpSet is in/out parameter, usually just a media-descriptor that
    // can be changed by dialog

    Reference< XFilePickerControlAccess > xCtrlAccess( mxFileDlg, UNO_QUERY );

    // retrieves parameters from rpSet
    // for now only Password is used
    if ( rpSet )
    {
        // check password checkbox if the document had password before
        if( mbHasPassword )
        {
            SFX_ITEMSET_ARG( rpSet, pPassItem, SfxStringItem, SID_PASSWORD, FALSE );
            mbPwdCheckBoxState = ( pPassItem != NULL );
        }

        SFX_ITEMSET_ARG( rpSet, pSelectItem, SfxBoolItem, SID_SELECTION, FALSE );
        if ( pSelectItem )
            mbSelection = pSelectItem->GetValue();
        else
            mbSelectionEnabled = sal_False;

        // the password will be set in case user decide so
        rpSet->ClearItem( SID_PASSWORD );
    }

    rpURLList = NULL;

    if ( ! mxFileDlg.is() )
        return ERRCODE_ABORT;

    if ( ExecutableDialogResults::CANCEL != implDoExecute() )
    {
        // create an itemset if there is no
        if( !rpSet )
            rpSet = new SfxAllItemSet( SFX_APP()->GetPool() );

        // check, wether or not we have to display a password box
        if ( mbHasPassword && mbIsPwdEnabled && xCtrlAccess.is() )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_PASSWORD, 0 );
                sal_Bool bPassWord = sal_False;
                if ( ( aValue >>= bPassWord ) && bPassWord )
                {
                    // ask for the password
                    SfxPasswordDialog aPasswordDlg( NULL );
                    aPasswordDlg.ShowExtras( SHOWEXTRAS_CONFIRM );
                    BOOL bOK = FALSE;
                    short nRet = aPasswordDlg.Execute();
                    if ( RET_OK == nRet )
                    {
                        String aPasswd = aPasswordDlg.GetPassword();
                        rpSet->Put( SfxStringItem( SID_PASSWORD, aPasswd ) );
                    }
                    else
                        return ERRCODE_ABORT;
                }
            }
            catch( IllegalArgumentException ){}
        }

        if( mbExport )
        {
            try
            {
                Any aValue = xCtrlAccess->getValue( ExtendedFilePickerElementIds::CHECKBOX_SELECTION, 0 );
                sal_Bool bSelection = sal_False;
                if ( ( aValue >>= bSelection ) && bSelection )
                    rpSet->Put( SfxBoolItem( SID_SELECTION, bSelection ) );
            }
            catch( IllegalArgumentException )
            {
                DBG_ERROR( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
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
                catch( IllegalArgumentException )
                {
                    DBG_ERROR( "FileDialogHelper_Impl::execute: caught an IllegalArgumentException!" );
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
            catch( IllegalArgumentException ){}
        }

        // set the filter
        getRealFilter( rFilter );

        // fill the rpURLList
        Sequence < OUString > aPathSeq = mxFileDlg->getFiles();

        if ( aPathSeq.getLength() )
        {
            rpURLList = new SvStringsDtor;

            if ( aPathSeq.getLength() == 1 )
            {
                OUString aFileURL( aPathSeq[0] );

                String* pURL = new String( aFileURL );
                rpURLList->Insert( pURL, 0 );
            }
            else
            {
                INetURLObject aPath( aPathSeq[0] );
                aPath.setFinalSlash();

                for ( USHORT i = 1; i < aPathSeq.getLength(); ++i )
                {
                    if ( i == 1 )
                        aPath.Append( aPathSeq[i] );
                    else
                        aPath.setName( aPathSeq[i] );

                    String* pURL = new String( aPath.GetMainURL( INetURLObject::NO_DECODE ) );
                    rpURLList->Insert( pURL, rpURLList->Count() );
                }
            }
            SaveLastUsedFilter();
            return ERRCODE_NONE;
        }
        else
            return ERRCODE_ABORT;
    }
    else
        return ERRCODE_ABORT;
}

// ------------------------------------------------------------------------
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

// ------------------------------------------------------------------------
OUString FileDialogHelper_Impl::getPath() const
{
    OUString aPath;

    if ( mxFileDlg.is() )
        aPath = mxFileDlg->getDisplayDirectory();

    if ( !aPath.getLength() )
        aPath = maPath;

    return aPath;
}

// ------------------------------------------------------------------------
OUString FileDialogHelper_Impl::getFilter() const
{
    String aFilter = getCurrentFilterUIName();

    if( !aFilter.Len() )
        aFilter = maCurFilter;

    return aFilter;
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::getRealFilter( String& _rFilter ) const
{
    _rFilter = getCurrentFilterUIName();

    if ( !_rFilter.Len() )
        _rFilter = maCurFilter;

    if ( _rFilter.Len() && mpMatcher )
    {
        const SfxFilter* pFilter =
            mpMatcher->GetFilter4UIName( _rFilter, m_nMustFlags, m_nDontFlags );
        _rFilter = pFilter ? pFilter->GetFilterName() : _rFilter.Erase();
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::displayFolder( const ::rtl::OUString& _rPath )
{
    if ( ! _rPath.getLength() )
        // nothing to do
        return;

    if ( !::utl::UCBContentHelper::IsFolder( _rPath ) )
        // only valid folders accepted here
        return;

    maPath = _rPath;
    if ( mxFileDlg.is() )
    {
        try
        {
            mxFileDlg->setDisplayDirectory( maPath );
        }
        catch( const IllegalArgumentException& e )
        {
            e; // make compiler happy
            DBG_ERROR( "FileDialogHelper_Impl::displayFolder: caught an exception!" );
        }
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setFileName( const ::rtl::OUString& _rFile )
{
    maFileName = _rFile;
    if ( mxFileDlg.is() )
    {
        try
        {
            mxFileDlg->setDefaultName( maFileName );
        }
        catch( const IllegalArgumentException& e )
        {
            e; // make compiler happy
            DBG_ERROR( "FileDialogHelper_Impl::setFileName: caught an exception!" );
        }
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setFilter( const OUString& rFilter )
{
    DBG_ASSERT( rFilter.indexOf(':') == -1, "Old filter name used!");

    maCurFilter = rFilter;

    if ( rFilter.getLength() && mpMatcher )
    {
        const SfxFilter* pFilter = mpMatcher->GetFilter4FilterName(
                                        rFilter, m_nMustFlags, m_nDontFlags );
        if ( pFilter )
            maCurFilter = pFilter->GetUIName();
    }

    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( maCurFilter.getLength() && xFltMgr.is() )
    {
        try
        {
            xFltMgr->setCurrentFilter( maCurFilter );
        }
        catch( IllegalArgumentException ){}
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::createMatcher( const String& rFactory )
{
    mpMatcher = new SfxFilterMatcher( rFactory );
    mbDeleteMatcher = sal_True;
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addFilters( sal_uInt32 nFlags,
                                        const String& rFactory,
                                        SfxFilterFlags nMust,
                                        SfxFilterFlags nDont )
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    // create the list of filters

    if ( !rFactory.Len() )
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

    m_nMustFlags |= nMust;
    m_nDontFlags |= nDont;

    SfxFilterMatcherIter aIter( mpMatcher, m_nMustFlags, m_nDontFlags );

    // append the filters
    ::rtl::OUString sFirstFilter;
    if ( WB_OPEN == ( nFlags & WB_OPEN ) )
        ::sfx2::appendFiltersForOpen( aIter, xFltMgr, sFirstFilter, *this );
    else if ( mbExport )
        ::sfx2::appendExportFilters( aIter, xFltMgr, sFirstFilter, *this );
    else
        ::sfx2::appendFiltersForSave( aIter, xFltMgr, sFirstFilter, *this );

    // set our initial selected filter (if we do not already have one)
    if ( maSelectFilter.getLength() )
        maSelectFilter = sFirstFilter;
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addFilter( const OUString& rFilterName,
                                       const OUString& rExtension )
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    try
    {
        xFltMgr->appendFilter( rFilterName, rExtension );

        if ( !maSelectFilter.getLength() )
            maSelectFilter = rFilterName;
    }
    catch( IllegalArgumentException )
    {
#ifdef DBG_UTIL
        ByteString aMsg( "Could not append Filter" );
        aMsg += ByteString( String( rFilterName ), RTL_TEXTENCODING_UTF8 );
        DBG_ERRORFILE( aMsg.GetBuffer() );
#endif
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::addGraphicFilter()
{
    Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );

    if ( ! xFltMgr.is() )
        return;

    // create the list of filters
    mpGraphicFilter = new GraphicFilter;
    USHORT i, j, nCount = mpGraphicFilter->GetImportFormatCount();

    // compute the extension string for all known import filters
    String aExtensions;

    for ( i = 0; i < nCount; i++ )
    {
        j = 0;
        String sWildcard;
        while( TRUE )
        {
            sWildcard = mpGraphicFilter->GetImportWildcard( i, j++ );
            if ( !sWildcard.Len() )
                break;
            if ( aExtensions.Search( sWildcard ) == STRING_NOTFOUND )
            {
                if ( aExtensions.Len() )
                    aExtensions += sal_Unicode(';');
                aExtensions += sWildcard;
            }
        }
    }

#if defined(WIN) || defined(WNT)
    if ( aExtensions.Len() > 240 )
        aExtensions = DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL );
#endif
    sal_Bool bIsInOpenMode = isInOpenMode();

    try
    {
        OUString aAllFilterName = String( SfxResId( STR_SFX_IMPORT_ALL ) );
        aAllFilterName = ::sfx2::addExtension( aAllFilterName, aExtensions, bIsInOpenMode, *this );

        xFltMgr->appendFilter( aAllFilterName, aExtensions );
        maSelectFilter = aAllFilterName;
    }
    catch( IllegalArgumentException )
    {
        DBG_ERRORFILE( "Could not append Filter" );
    }

    // Now add the filter
    for ( i = 0; i < nCount; i++ )
    {
        String aName = mpGraphicFilter->GetImportFormatName( i );
        String aExtensions;
        j = 0;
        String sWildcard;
        while( TRUE )
        {
            sWildcard = mpGraphicFilter->GetImportWildcard( i, j++ );
            if ( !sWildcard.Len() )
                break;
            if ( aExtensions.Search( sWildcard ) == STRING_NOTFOUND )
            {
                if ( aExtensions.Len() )
                    aExtensions += sal_Unicode(';');
                aExtensions += sWildcard;
            }
        }
        aName = ::sfx2::addExtension( aName, aExtensions, bIsInOpenMode, *this );
        try
        {
            xFltMgr->appendFilter( aName, aExtensions );
        }
        catch( IllegalArgumentException )
        {
            DBG_ERRORFILE( "Could not append Filter" );
        }
    }
}

// ------------------------------------------------------------------------
#define GRF_CONFIG_STR      "   "
#define STD_CONFIG_STR      "1 "

void FileDialogHelper_Impl::saveConfig()
{
    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aDlgOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData = DEFINE_CONST_UNICODE( GRF_CONFIG_STR );

        try
        {
            aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0 );
            sal_Bool bValue = sal_False;
            aValue >>= bValue;
            aUserData.SetToken( 0, ' ', String::CreateFromInt32( (sal_Int32) bValue ) );

            aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0 );
            bValue = sal_False;
            aValue >>= bValue;
            aUserData.SetToken( 1, ' ', String::CreateFromInt32( (sal_Int32) bValue ) );

            INetURLObject aObj( getPath() );

            if ( aObj.GetProtocol() == INET_PROT_FILE )
                aUserData.SetToken( 2, ' ', aObj.GetMainURL( INetURLObject::NO_DECODE ) );

            String aFilter = getFilter();
            aFilter = EncodeSpaces_Impl( aFilter );
            aUserData.SetToken( 3, ' ', aFilter );

            aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aUserData ) ) );
        }
        catch( IllegalArgumentException ){}
    }
    else
    {
        sal_Bool bWriteConfig = sal_False;
        SvtViewOptions aDlgOpt( E_DIALOG, IODLG_CONFIGNAME );
        String aUserData = DEFINE_CONST_UNICODE( STD_CONFIG_STR );

        if ( aDlgOpt.Exists() )
        {
            Any aUserItem = aDlgOpt.GetUserItem( USERITEM_NAME );
            OUString aTemp;
            if ( aUserItem >>= aTemp )
                aUserData = String( aTemp );
        }

        if ( mbHasAutoExt )
        {
            try
            {
                aValue = xDlg->getValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0 );
                sal_Bool bAutoExt = sal_True;
                aValue >>= bAutoExt;
                aUserData.SetToken( 0, ' ', String::CreateFromInt32( (sal_Int32) bAutoExt ) );
                bWriteConfig = sal_True;
            }
            catch( IllegalArgumentException ){}
        }

        if ( ! mbIsSaveDlg )
        {
            OUString aPath = getPath();
            if ( aPath.getLength() &&
                 utl::LocalFileHelper::IsLocalFile( aPath ) )
            {
                aUserData.SetToken( 1, ' ', aPath );
                bWriteConfig = sal_True;
            }
        }

        if ( bWriteConfig )
            aDlgOpt.SetUserItem( USERITEM_NAME, makeAny( OUString( aUserData ) ) );
    }

    SfxApplication *pSfxApp = SFX_APP();
    pSfxApp->SetLastDir_Impl( getPath() );
}

// ------------------------------------------------------------------------
namespace
{
    static ::rtl::OUString getInitPath( const String& _rFallback, const xub_StrLen _nFallbackToken )
    {
        SfxApplication *pSfxApp = SFX_APP();
        String sPath = pSfxApp->GetLastDir_Impl();

        if ( !sPath.Len() )
            sPath = _rFallback.GetToken( _nFallbackToken, ' ' );

        // check if the path points to a valid (accessible) directory
        sal_Bool bValid = sal_False;
        if ( sPath.Len() )
        {
            String sPathCheck( sPath );
            if ( sPathCheck.GetBuffer()[ sPathCheck.Len() - 1 ] != '/' )
                sPathCheck += '/';
            sPathCheck += '.';
            try
            {
                ::ucb::Content aContent( sPathCheck, Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
                bValid = aContent.isFolder();
            }
            catch( const Exception& e )
            {
                e;  // make compiler happy
            }
        }

        if ( !bValid )
            sPath.Erase();

        return sPath;
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::loadConfig()
{
    Reference < XFilePickerControlAccess > xDlg( mxFileDlg, UNO_QUERY );
    Any aValue;

    if ( ! xDlg.is() )
        return;

    if ( mbHasPreview )
    {
        SvtViewOptions aViewOpt( E_DIALOG, IMPGRF_CONFIGNAME );
        String aUserData;

        if ( aViewOpt.Exists() )
        {
            Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
            OUString aTemp;
            if ( aUserItem >>= aTemp )
                aUserData = String( aTemp );
        }

        if ( aUserData.Len() > 0 )
        {
            try
            {
                // respect the last "insert as link" state
                sal_Bool bLink = (sal_Bool) aUserData.GetToken( 0, ' ' ).ToInt32();
                aValue <<= bLink;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aValue );

                // respect the last "show preview" state
                sal_Bool bShowPreview = (sal_Bool) aUserData.GetToken( 1, ' ' ).ToInt32();
                aValue <<= bShowPreview;
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_PREVIEW, 0, aValue );

                if ( !maPath.getLength() )
                    displayFolder( getInitPath( aUserData, 2 ) );

                if ( ! maCurFilter.getLength() )
                {
                    String aFilter = aUserData.GetToken( 3, ' ' );
                    aFilter = DecodeSpaces_Impl( aFilter );
                    setFilter( aFilter );
                }

                // set the member so we know that we have to show the preview
                mbShowPreview = bShowPreview;
            }
            catch( IllegalArgumentException ){}
        }

        if ( !maPath.getLength() )
            displayFolder( SvtPathOptions().GetGraphicPath() );
    }
    else
    {
        SvtViewOptions aViewOpt( E_DIALOG, IODLG_CONFIGNAME );
        String aUserData;

        if ( aViewOpt.Exists() )
        {
            Any aUserItem = aViewOpt.GetUserItem( USERITEM_NAME );
            OUString aTemp;
            if ( aUserItem >>= aTemp )
                aUserData = String( aTemp );
        }

        if ( ! aUserData.Len() )
            aUserData = DEFINE_CONST_UNICODE( STD_CONFIG_STR );

        if ( ! maPath.getLength() )
            displayFolder( getInitPath( aUserData, 1 ) );

        if ( mbHasAutoExt )
        {
            sal_Int32 nFlag = aUserData.GetToken( 0, ' ' ).ToInt32();
            aValue <<= (sal_Bool) nFlag;
            try
            {
                xDlg->setValue( ExtendedFilePickerElementIds::CHECKBOX_AUTOEXTENSION, 0, aValue );
            }
            catch( IllegalArgumentException ){}
        }

        if ( !maPath.getLength() )
            displayFolder( SvtPathOptions().GetWorkPath() );
    }
}

// ------------------------------------------------------------------------
void FileDialogHelper_Impl::setDefaultValues()
{
    // when no filter is set, we set the curentFilter to <all>
    if ( !maCurFilter.getLength() && maSelectFilter.getLength() )
    {
        Reference< XFilterManager > xFltMgr( mxFileDlg, UNO_QUERY );
        try
        {
            xFltMgr->setCurrentFilter( maSelectFilter );
        }
        catch( IllegalArgumentException )
        {}
    }

    // when no path is set, we use the standard 'work' folder
    if ( ! maPath.getLength() )
    {
        OUString aWorkFolder = SvtPathOptions().GetWorkPath();
        try
        {
            mxFileDlg->setDisplayDirectory( aWorkFolder );
        }
        catch( const Exception& )
        {
            DBG_ERROR( "FileDialogHelper_Impl::setDefaultValues: caught an exception while setting the display directory!" );
        }

        // INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
        //SetStandardDir( aStdDirObj.GetMainURL( INetURLObject::NO_DECODE ) );
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

    sal_Int32       nNewHelpId = 0;
    OUString        aConfigId;

    switch( _eNewContext )
    {
// #104952# dependency to SVX not allowed! When used again, another solution has to be found
//      case FileDialogHelper::SW_INSERT_GRAPHIC:
//      case FileDialogHelper::SC_INSERT_GRAPHIC:
//      case FileDialogHelper::SD_INSERT_GRAPHIC:       nNewHelpId = SID_INSERT_GRAPHIC;        break;
        case FileDialogHelper::SW_INSERT_SOUND:
        case FileDialogHelper::SC_INSERT_SOUND:
        case FileDialogHelper::SD_INSERT_SOUND:         nNewHelpId = SID_INSERT_SOUND;          break;
        case FileDialogHelper::SW_INSERT_VIDEO:
        case FileDialogHelper::SC_INSERT_VIDEO:
        case FileDialogHelper::SD_INSERT_VIDEO:         nNewHelpId = SID_INSERT_VIDEO;          break;
    }

    const OUString* pConfigId = GetLastFilterConfigId( _eNewContext );
    if( pConfigId )
        LoadLastUsedFilter( *pConfigId );

//  if( nNewHelpId )
//      this->setDialogHelpId( nNewHelpId );
}

// ------------------------------------------------------------------------
// -----------          FileDialogHelper        ---------------------------
// ------------------------------------------------------------------------

FileDialogHelper::FileDialogHelper( sal_uInt32 nFlags,
                                    const String& rFact,
                                    SfxFilterFlags nMust,
                                    SfxFilterFlags nDont )
{
    mpImp = new FileDialogHelper_Impl( this, getDialogType( nFlags ), nFlags );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters( nFlags, rFact, nMust, nDont );
}

// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( sal_uInt32 nFlags )
{
    const short nDialogType = getDialogType( nFlags );

    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags );
    mxImp = mpImp;
}

// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( const short nDialogType,
                                    sal_uInt32 nFlags,
                                    const String& rFact,
                                    SfxFilterFlags nMust,
                                    SfxFilterFlags nDont )
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags );
    mxImp = mpImp;

    // create the list of filters
    mpImp->addFilters( nFlags, rFact, nMust, nDont );
}

#ifdef FS_PRIV_DEBUG
// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( const short nDialogType,
                                    sal_uInt32 nFlags )
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags );
    mxImp = mpImp;
}
#endif

// ------------------------------------------------------------------------
FileDialogHelper::FileDialogHelper( const short nDialogType,
                                    sal_uInt32 nFlags, Window* _pPreferredParent )
{
    mpImp = new FileDialogHelper_Impl( this, nDialogType, nFlags, _pPreferredParent );
    mxImp = mpImp;
}

// ------------------------------------------------------------------------
FileDialogHelper::~FileDialogHelper()
{
    mpImp->dispose();
    mxImp.clear();
}

// ------------------------------------------------------------------------
void FileDialogHelper::CreateMatcher( const String& rFactory )
{
    mpImp->createMatcher( rFactory );
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetControlHelpIds( const sal_Int16* _pControlId, const sal_Int32* _pHelpId )
{
    mpImp->setControlHelpIds( _pControlId, _pHelpId );
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetDialogHelpId( const sal_Int32 _nHelpId )
{
    mpImp->setDialogHelpId( _nHelpId );
}

void FileDialogHelper::SetContext( Context _eNewContext )
{
    mpImp->SetContext( _eNewContext );
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute( const String&   rPath,
                                   SvStringsDtor*& rpURLList,
                                   SfxItemSet *&   rpSet,
                                   String&         rFilter )
{
    SetDisplayDirectory( rPath );

    return mpImp->execute( rpURLList, rpSet, rFilter );
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute()
{
    return mpImp->execute();
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::Execute( SfxItemSet *&   rpSet,
                                   String&         rFilter )
{
    ErrCode nRet;
    SvStringsDtor* pURLList;

    nRet = mpImp->execute( pURLList, rpSet, rFilter );

    delete pURLList;

    return nRet;
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetTitle( const String& rNewTitle )
{
    if ( mpImp->mxFileDlg.is() )
        mpImp->mxFileDlg->setTitle( rNewTitle );
}

// ------------------------------------------------------------------------
String FileDialogHelper::GetPath() const
{
    OUString aPath;

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

// ------------------------------------------------------------------------
Sequence < OUString > FileDialogHelper::GetMPath() const
{
    if ( mpImp->mxFileDlg.is() )
        return mpImp->mxFileDlg->getFiles();
    else
    {
        Sequence < OUString > aEmpty;
        return aEmpty;
    }
}

// ------------------------------------------------------------------------
String FileDialogHelper::GetDisplayDirectory() const
{
    return mpImp->getPath();
}

// ------------------------------------------------------------------------
String FileDialogHelper::GetCurrentFilter() const
{
    return mpImp->getFilter();
}

// ------------------------------------------------------------------------
ErrCode FileDialogHelper::GetGraphic( Graphic& rGraphic ) const
{
    return mpImp->getGraphic( rGraphic );
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetDisplayDirectory( const String& _rPath )
{
    if ( !_rPath.Len() )
        return;

    // if the given path isn't a folder, we cut off the last part
    // and take it as filename and the rest of the path should be
    // the folder

    ::rtl::OUString sPath;
    ::rtl::OUString sFileName;

    INetURLObject aObj( _rPath );
    if ( !::utl::UCBContentHelper::IsFolder( _rPath ) )
    {
        sFileName = aObj.GetName( INetURLObject::DECODE_WITH_CHARSET );
        aObj.removeSegment();
    }

    sPath = aObj.GetMainURL( INetURLObject::NO_DECODE );

    mpImp->displayFolder( sPath );
    mpImp->setFileName( sFileName );
}

// ------------------------------------------------------------------------
void FileDialogHelper::AddFilter( const String& rFilterName,
                                  const String& rExtension )
{
    mpImp->addFilter( rFilterName, rExtension );
}

// ------------------------------------------------------------------------
void FileDialogHelper::SetCurrentFilter( const String& rFilter )
{
    String sFilter( rFilter );
    if ( mpImp->isShowFilterExtensionEnabled() )
        sFilter = mpImp->getFilterWithExtension( rFilter );
    mpImp->setFilter( sFilter );
}

// ------------------------------------------------------------------------
Reference < XFilePicker > FileDialogHelper::GetFilePicker() const
{
    return mpImp->mxFileDlg;
}

// ------------------------------------------------------------------------
const short FileDialogHelper::getDialogType( sal_uInt32 nFlags ) const
{
    short nDialogType = FILEOPEN_SIMPLE;

    if ( nFlags & WB_SAVEAS )
    {
        if ( nFlags & SFXWB_PASSWORD )
            nDialogType = FILESAVE_AUTOEXTENSION_PASSWORD;
        else
            nDialogType = FILESAVE_SIMPLE;
    }
    else if ( nFlags & SFXWB_GRAPHIC )
    {
        if ( nFlags & SFXWB_SHOWSTYLES )
            nDialogType = FILEOPEN_LINK_PREVIEW_IMAGE_TEMPLATE;
        else
            nDialogType = FILEOPEN_LINK_PREVIEW;
    }
    else if ( SFXWB_INSERT != ( nFlags & SFXWB_INSERT ) )
        nDialogType = FILEOPEN_READONLY_VERSION;

    return nDialogType;
}

// ------------------------------------------------------------------------
// XFilePickerListener Methods
// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper::FileSelectionChanged( const FilePickerEvent& aEvent )
{
    mpImp->handleFileSelectionChanged( aEvent );
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper::DirectoryChanged( const FilePickerEvent& aEvent )
{
    mpImp->handleDirectoryChanged( aEvent );
}

// ------------------------------------------------------------------------
OUString SAL_CALL FileDialogHelper::HelpRequested( const FilePickerEvent& aEvent )
{
    return mpImp->handleHelpRequested( aEvent );
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper::ControlStateChanged( const FilePickerEvent& aEvent )
{
    mpImp->handleControlStateChanged( aEvent );
}

// ------------------------------------------------------------------------
void SAL_CALL FileDialogHelper::DialogSizeChanged()
{
    mpImp->handleDialogSizeChanged();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

ErrCode FileOpenDialog_Impl( sal_uInt32 nFlags,
                             const String& rFact,
                             SvStringsDtor *& rpURLList,
                             String& rFilter,
                             SfxItemSet *& rpSet,
                             String aPath,
                             ULONG nHelpId )
{
    ErrCode nRet;
    FileDialogHelper aDialog( nFlags, rFact );

//  if( nHelpId )
//      aDialog.SetDialogHelpId( nHelpId );

    nRet = aDialog.Execute( aPath, rpURLList, rpSet, rFilter );
    DBG_ASSERT( rFilter.SearchAscii(": ") == STRING_NOTFOUND, "Old filter name used!");

    aPath = aDialog.GetDisplayDirectory();

    return nRet;
}


// ------------------------------------------------------------------------
String EncodeSpaces_Impl( const String& rSource )
{
    String sRet( rSource );
    sRet.SearchAndReplaceAll( DEFINE_CONST_UNICODE( " " ), DEFINE_CONST_UNICODE( "%20" ) );
    return sRet;
}

// ------------------------------------------------------------------------
String DecodeSpaces_Impl( const String& rSource )
{
    String sRet( rSource );
    sRet.SearchAndReplaceAll( DEFINE_CONST_UNICODE( "%20" ), DEFINE_CONST_UNICODE( " " ) );
    return sRet;
}

// ------------------------------------------------------------------------

}   // end of namespace sfx2

