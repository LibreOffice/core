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

#include <sal/config.h>

#include <sal/log.hxx>
#include "fileview.hxx"
#include "iodlg.hxx"
#include <svtools/PlaceEditDialog.hxx>
#include "OfficeControlAccess.hxx"
#include "PlacesListBox.hxx"
#include <fpicker/fpsofficeResMgr.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/errinf.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <unotools/ucbhelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/sfxecode.hxx>

#include <fpicker/strings.hrc>
#include <svtools/helpids.h>
#include <strings.hrc>
#include "asyncfilepicker.hxx"
#include "iodlgimp.hxx"
#include <svtools/inettbc.hxx>
#include "QueryFolderName.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <osl/file.hxx>
#include <vcl/dibtools.hxx>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include "fpinteraction.hxx"
#include <osl/process.h>
#include <o3tl/string_view.hxx>

#include <officecfg/Office/Common.hxx>

#include <algorithm>
#include <memory>
#include <string_view>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace ::utl;
using namespace ::svt;

using namespace ExtendedFilePickerElementIds;
using namespace CommonFilePickerElementIds;
using namespace InternalFilePickerElementIds;

// functions -------------------------------------------------------------

namespace
{
    OUString getMostCurrentFilter( std::unique_ptr<SvtExpFileDlg_Impl> const & pImpl )
    {
        assert( pImpl && "invalid impl pointer" );
        const SvtFileDialogFilter_Impl* pFilter = pImpl->m_xUserFilter.get();

        if ( !pFilter )
            pFilter = pImpl->GetCurFilter();

        if ( !pFilter )
            return OUString();

        return pFilter->GetType();
    }

    void restoreCurrentFilter( std::unique_ptr<SvtExpFileDlg_Impl> const & pImpl )
    {
        SAL_WARN_IF( !pImpl->GetCurFilter(), "fpicker.office", "restoreCurrentFilter: no current filter!" );
        SAL_WARN_IF( pImpl->GetCurFilterDisplayName().isEmpty(), "fpicker.office", "restoreCurrentFilter: no current filter (no display name)!" );

        pImpl->SelectFilterListEntry( pImpl->GetCurFilterDisplayName() );

#ifdef DBG_UTIL
        OUString sSelectedDisplayName;
        DBG_ASSERT( ( pImpl->GetSelectedFilterEntry( sSelectedDisplayName ) == pImpl->GetCurFilter() )
                &&  ( sSelectedDisplayName == pImpl->GetCurFilterDisplayName() ),
            "restoreCurrentFilter: inconsistence!" );
#endif
    }


    OUString GetFsysExtension_Impl( std::u16string_view rFile, const OUString& rLastFilterExt )
    {
        size_t nDotPos = rFile.rfind( '.' );
        if ( nDotPos != std::u16string_view::npos )
        {
            if ( !rLastFilterExt.isEmpty() )
            {
                if ( o3tl::equalsIgnoreAsciiCase(rFile.substr( nDotPos + 1 ), rLastFilterExt ) )
                    return rLastFilterExt;
            }
            else
                return OUString(rFile.substr( nDotPos ));
        }
        return OUString();
    }


    void SetFsysExtension_Impl( OUString& rFile, std::u16string_view rExtension )
    {
        const sal_Int32 nDotPos{ rFile.lastIndexOf('.') };
        if (nDotPos>=0)
        {
            if (!rExtension.empty())
                rFile = OUString::Concat(rFile.subView(0, nDotPos)) + rExtension; // replace old extension with new (not empty) one
            else if (nDotPos)
                rFile = rFile.copy(0, nDotPos-1); // truncate extension (new one is empty)
            else
                rFile.clear(); // Filename was just an extension
        }
        else if (!rExtension.empty())
            rFile += OUString::Concat(".") + rExtension;
                // no extension was present, append new one if not empty
    }

    void lcl_autoUpdateFileExtension( SvtFileDialog* _pDialog, const OUString& _rLastFilterExt )
    {
        // if auto extension is enabled...
        if ( !_pDialog->isAutoExtensionEnabled() )
            return;

        // automatically switch to the extension of the (maybe just newly selected) extension
        OUString aNewFile = _pDialog->getCurrentFileText( );
        OUString aExt = GetFsysExtension_Impl( aNewFile, _rLastFilterExt );

        // but only if there already is an extension
        if ( aExt.isEmpty() )
            return;

        // check if it is a real file extension, and not only the "post-dot" part in
        // a directory name
        bool bRealExtensions = true;
        if ( -1 != aExt.indexOf( '/' ) )
            bRealExtensions = false;
        else if ( -1 != aExt.indexOf( '\\' ) )
            bRealExtensions = false;
        else
        {
            // no easy way to tell, because the part containing the dot already is the last
            // segment of the complete file name
            // So we have to check if the file name denotes a folder or a file.
            // For performance reasons, we do this for file urls only
            INetURLObject aURL( aNewFile );
            if ( INetProtocol::NotValid == aURL.GetProtocol() )
            {
                OUString sURL;
                if ( osl::FileBase::getFileURLFromSystemPath( aNewFile, sURL )
                     == osl::FileBase::E_None )
                    aURL = INetURLObject( sURL );
            }
            if ( INetProtocol::File == aURL.GetProtocol() )
            {
                try
                {
                    bRealExtensions = !_pDialog->ContentIsFolder( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                }
                catch( const css::uno::Exception& )
                {
                    SAL_INFO( "fpicker.office", "Exception in lcl_autoUpdateFileExtension" );
                }
            }
        }

        if ( bRealExtensions )
        {
            SetFsysExtension_Impl( aNewFile, _pDialog->GetDefaultExt() );
            _pDialog->setCurrentFileText( aNewFile );
        }
    }

#if defined( UNX )
    bool lcl_getHomeDirectory( const OUString& _rForURL, OUString& /* [out] */ _rHomeDir )
    {
        _rHomeDir.clear();

        // now ask the content broker for a provider for this scheme

        try
        {
            // get the provider for the current scheme
            Reference< XContentProvider > xProvider(
                UniversalContentBroker::create(
                    comphelper::getProcessComponentContext() )->
                queryContentProvider( _rForURL ) );

            SAL_WARN_IF( !xProvider.is(), "fpicker.office", "lcl_getHomeDirectory: could not find a (valid) content provider for the current URL!" );
            Reference< XPropertySet > xProviderProps( xProvider, UNO_QUERY );
            if ( xProviderProps.is() )
            {
                Reference< XPropertySetInfo > xPropInfo = xProviderProps->getPropertySetInfo();
                static constexpr OUString sHomeDirPropertyName( u"HomeDirectory"_ustr );
                if ( !xPropInfo.is() || xPropInfo->hasPropertyByName( sHomeDirPropertyName ) )
                {
                    OUString sHomeDirectory;
                    xProviderProps->getPropertyValue( sHomeDirPropertyName ) >>= sHomeDirectory;
                    _rHomeDir = sHomeDirectory;
                }
            }
        }
        catch( const Exception& )
        {
            TOOLS_WARN_EXCEPTION( "fpicker", "lcl_getHomeDirectory" );
        }
        return !_rHomeDir.isEmpty();
    }
#endif

    OUString lcl_ensureFinalSlash( std::u16string_view _rDir )
    {
        INetURLObject aWorkPathObj( _rDir, INetProtocol::File );
        aWorkPathObj.setFinalSlash();
        return  aWorkPathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }


    /** retrieves the value of an environment variable
        @return <TRUE/> if and only if the retrieved string value is not empty
    */
    bool getEnvironmentValue( const char* _pAsciiEnvName, OUString& _rValue )
    {
        _rValue.clear();
        OUString sEnvName = OUString::createFromAscii( _pAsciiEnvName );
        osl_getEnvironment( sEnvName.pData, &_rValue.pData );
        return !_rValue.isEmpty();
    }
}

// SvtFileDialog
SvtFileDialog::SvtFileDialog(weld::Window* pParent, PickerFlags nStyle)
    : SvtFileDialog_Base(pParent, u"fps/ui/explorerfiledialog.ui"_ustr, u"ExplorerFileDialog"_ustr)
    , m_xCbReadOnly(m_xBuilder->weld_check_button(u"readonly"_ustr))
    , m_xCbLinkBox(m_xBuilder->weld_check_button(u"link"_ustr))
    , m_xCbPreviewBox(m_xBuilder->weld_check_button(u"cb_preview"_ustr))
    , m_xCbSelection(m_xBuilder->weld_check_button(u"selection"_ustr))
    , m_xPbPlay(m_xBuilder->weld_button(u"play"_ustr))
    , m_xPreviewFrame(m_xBuilder->weld_widget(u"previewframe"_ustr))
    , m_xPrevBmp(m_xBuilder->weld_image(u"preview"_ustr))
    , m_pFileNotifier(nullptr)
    , m_xImpl(new SvtExpFileDlg_Impl)
    , m_nPickerFlags(nStyle)
    , m_bIsInExecute(false)
    , m_bInExecuteAsync(false)
    , m_bHasFilename(false)
{
    m_xImpl->m_xCbOptions = m_xBuilder->weld_check_button(u"options"_ustr);
    m_xImpl->m_xFtFileName = m_xBuilder->weld_label(u"file_name_label"_ustr);
    m_xImpl->m_xEdFileName.reset(new SvtURLBox(m_xBuilder->weld_combo_box(u"file_name"_ustr)));
    m_xImpl->m_xFtFileType = m_xBuilder->weld_label(u"file_type_label"_ustr);
    m_xImpl->m_xLbFilter = m_xBuilder->weld_combo_box(u"file_type"_ustr);
    m_xImpl->m_xEdCurrentPath.reset(new SvtURLBox(m_xBuilder->weld_combo_box(u"current_path"_ustr)));
    m_xImpl->m_xBtnFileOpen = m_xBuilder->weld_button(u"open"_ustr);
    m_xImpl->m_xBtnCancel = m_xBuilder->weld_button(u"cancel"_ustr);
    m_xImpl->m_xBtnHelp = m_xBuilder->weld_button(u"help"_ustr);
    m_xImpl->m_xBtnConnectToServer = m_xBuilder->weld_button(u"connect_to_server"_ustr);
    m_xImpl->m_xBtnNewFolder = m_xBuilder->weld_button(u"new_folder"_ustr);
    m_xImpl->m_xCbPassword = m_xBuilder->weld_check_button(u"password"_ustr);
    m_xImpl->m_xCbGPGEncrypt = m_xBuilder->weld_check_button(u"gpgencrypt"_ustr);
    m_xImpl->m_xCbAutoExtension = m_xBuilder->weld_check_button(u"extension"_ustr);
    m_xImpl->m_xSharedLabel = m_xBuilder->weld_label(u"shared_label"_ustr);
    m_xImpl->m_xSharedListBox = m_xBuilder->weld_combo_box(u"shared"_ustr);

    // because the "<All Formats> (*.bmp,*...)" entry is too wide,
    // we need to disable the auto width feature of the filter box
    int nWidth = m_xImpl->m_xLbFilter->get_approximate_digit_width() * 60;
    m_xImpl->m_xSharedListBox->set_size_request(nWidth, -1);
    m_xImpl->m_xLbFilter->set_size_request(nWidth, -1);

    m_xImpl->m_xBtnUp.reset(new SvtUpButton_Impl(m_xBuilder->weld_toolbar(u"up_bar"_ustr),
                                                 m_xBuilder->weld_menu(u"up_menu"_ustr),
                                                 this));
    m_xImpl->m_xBtnUp->set_help_id(HID_FILEOPEN_LEVELUP);
    m_xImpl->m_xBtnUp->show();

    m_xImpl->m_nStyle = nStyle;
    m_xImpl->m_eMode = ( nStyle & PickerFlags::SaveAs ) ? FILEDLG_MODE_SAVE : FILEDLG_MODE_OPEN;
    m_xImpl->m_eDlgType = FILEDLG_TYPE_FILEDLG;

    if (nStyle & PickerFlags::PathDialog)
        m_xImpl->m_eDlgType = FILEDLG_TYPE_PATHDLG;

    // Set the directory for the "back to the default dir" button
    INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
    SetStandardDir( aStdDirObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    // Create control element, the order defines the tab control.
    m_xImpl->m_xEdFileName->connect_changed( LINK( this, SvtFileDialog, EntrySelectHdl_Impl ) );
    m_xImpl->m_xEdFileName->connect_entry_activate( LINK( this, SvtFileDialog, OpenUrlHdl_Impl ) );

    // in folder picker mode, only auto-complete directories (no files)
    bool bIsFolderPicker = m_xImpl->m_eDlgType == FILEDLG_TYPE_PATHDLG;
    m_xImpl->m_xEdFileName->SetOnlyDirectories( bIsFolderPicker );

    // in save mode, don't use the autocompletion as selection in the edit part
    bool bSaveMode = FILEDLG_MODE_SAVE == m_xImpl->m_eMode;
    m_xImpl->m_xEdFileName->SetNoURLSelection( bSaveMode );

    if (nStyle & PickerFlags::MultiSelection)
        m_xImpl->m_bMultiSelection = true;

    m_xContainer = m_xBuilder->weld_container(u"container"_ustr);
    m_xContainer->set_size_request(m_xContainer->get_approximate_digit_width() * 95, -1);

    m_xFileView.reset(new SvtFileView(m_xDialog.get(),
                                      m_xBuilder->weld_tree_view(u"fileview"_ustr),
                                      m_xBuilder->weld_icon_view(u"iconview"_ustr),
                                      FILEDLG_TYPE_PATHDLG == m_xImpl->m_eDlgType,
                                      m_xImpl->m_bMultiSelection));
    m_xFileView->set_help_id( HID_FILEDLG_STANDARD );

    if ( nStyle & PickerFlags::ReadOnly )
    {
        m_xCbReadOnly->set_help_id( HID_FILEOPEN_READONLY );
        m_xCbReadOnly->set_label( FpsResId( STR_SVT_FILEPICKER_READONLY ) );
        m_xCbReadOnly->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xCbReadOnly->show();
    }

    if ( nStyle & PickerFlags::Password )
    {
        m_xImpl->m_xCbPassword->set_label( FpsResId( STR_SVT_FILEPICKER_PASSWORD ) );
        m_xImpl->m_xCbPassword->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xImpl->m_xCbPassword->show();
        m_xImpl->m_xCbGPGEncrypt->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xImpl->m_xCbGPGEncrypt->show();
    }

    // set the ini file for extracting the size
    m_xImpl->m_aIniKey = "FileDialog";

    AddControls_Impl( );

    // adjust the labels to the mode
    TranslateId pResId = STR_EXPLORERFILE_OPEN;
    TranslateId pButtonResId;

    if ( nStyle & PickerFlags::SaveAs )
    {
        pResId = STR_EXPLORERFILE_SAVE;
        pButtonResId = STR_EXPLORERFILE_BUTTONSAVE;
    }

    if ( nStyle & PickerFlags::PathDialog )
    {
        m_xImpl->m_xFtFileName->set_label( FpsResId( STR_PATHNAME ) );
        pResId = STR_PATHSELECT;
        pButtonResId = STR_BUTTONSELECT;
    }

    m_xDialog->set_title(FpsResId(pResId));

    if ( pButtonResId )
        m_xImpl->m_xBtnFileOpen->set_label( FpsResId( pButtonResId ) );

    if ( FILEDLG_TYPE_FILEDLG != m_xImpl->m_eDlgType )
    {
        m_xImpl->m_xFtFileType->hide();
        m_xImpl->GetFilterListControl()->hide();
    }

    // Setting preferences of the control elements.
    m_xImpl->m_xBtnNewFolder->connect_clicked( LINK( this, SvtFileDialog, NewFolderHdl_Impl ) );
    m_xImpl->m_xBtnFileOpen->connect_clicked( LINK( this, SvtFileDialog, OpenClickHdl_Impl ) );
    m_xImpl->m_xBtnCancel->connect_clicked( LINK( this, SvtFileDialog, CancelHdl_Impl ) );
    m_xImpl->SetFilterListSelectHdl( LINK( this, SvtFileDialog, FilterSelectHdl_Impl ) );
    m_xImpl->m_xEdFileName->connect_focus_in( LINK( this, SvtFileDialog, FileNameGetFocusHdl_Impl ) );
    m_xImpl->m_xEdFileName->connect_changed( LINK( this, SvtFileDialog, FileNameModifiedHdl_Impl ) );
    m_xImpl->m_xEdCurrentPath->connect_entry_activate( LINK( this, SvtFileDialog, URLBoxModifiedHdl_Impl ) );
    m_xImpl->m_xBtnConnectToServer->connect_clicked( LINK ( this, SvtFileDialog, ConnectToServerPressed_Hdl ) );

    m_xFileView->SetSelectHdl( LINK( this, SvtFileDialog, SelectHdl_Impl ) );
    m_xFileView->SetDoubleClickHdl( LINK( this, SvtFileDialog, DblClickHdl_Impl ) );
    m_xFileView->SetOpenDoneHdl( LINK( this, SvtFileDialog, OpenDoneHdl_Impl ) );

    // set timer for the filterbox travel
    m_xImpl->m_aFilterIdle.SetPriority(TaskPriority::LOWEST);
    m_xImpl->m_aFilterIdle.SetInvokeHandler( LINK( this, SvtFileDialog, FilterSelectTimerHdl_Impl ) );

    if ( PickerFlags::SaveAs & nStyle )
    {
        // different help ids if in save-as mode
        m_xDialog->set_help_id( HID_FILESAVE_DIALOG );

        m_xImpl->m_xEdFileName->set_help_id( HID_FILESAVE_FILEURL );
        m_xImpl->m_xBtnFileOpen->set_help_id( HID_FILESAVE_DOSAVE );
        m_xImpl->m_xBtnNewFolder->set_help_id( HID_FILESAVE_CREATEDIRECTORY );
        m_xImpl->m_xBtnUp->set_help_id( HID_FILESAVE_LEVELUP );
        m_xImpl->GetFilterListControl()->set_help_id( HID_FILESAVE_FILETYPE );
        m_xFileView->set_help_id( HID_FILESAVE_FILEVIEW );

        // formerly, there was only _pLbFileVersion, which was used for 3 different
        // use cases. For reasons of maintainability, I introduced extra members (_pLbTemplates, _pLbImageTemplates)
        // for the extra use cases, and separated _pLbFileVersion
        // I did not find out in which cases the help ID is really needed HID_FILESAVE_TEMPLATE - all
        // tests I made lead to a dialog where _no_ of the three list boxes was present.
        if (m_xImpl->m_xSharedListBox)
            m_xImpl->m_xSharedListBox->set_help_id( HID_FILESAVE_TEMPLATE );

        if ( m_xImpl->m_xCbPassword ) m_xImpl->m_xCbPassword->set_help_id( HID_FILESAVE_SAVEWITHPASSWORD );
        if ( m_xImpl->m_xCbAutoExtension ) m_xImpl->m_xCbAutoExtension->set_help_id( HID_FILESAVE_AUTOEXTENSION );
        if ( m_xImpl->m_xCbOptions ) m_xImpl->m_xCbOptions->set_help_id( HID_FILESAVE_CUSTOMIZEFILTER );
        if ( m_xCbSelection ) m_xCbSelection->set_help_id( HID_FILESAVE_SELECTION );
    }

    /// read our settings from the configuration
    m_aConfiguration = OConfigurationTreeRoot::createWithComponentContext(
        ::comphelper::getProcessComponentContext(),
        u"/org.openoffice.Office.UI/FilePicker"_ustr
    );

    m_xDialog->connect_size_allocate(LINK(this, SvtFileDialog, SizeAllocHdl));
    SizeAllocHdl(Size());

    m_xImpl->m_xEdFileName->grab_focus();
}

SvtFileDialog::~SvtFileDialog()
{
    if (!m_xImpl->m_aIniKey.isEmpty())
    {
        // save window state
        SvtViewOptions aDlgOpt( EViewType::Dialog, m_xImpl->m_aIniKey );
        aDlgOpt.SetWindowState(m_xDialog->get_window_state(vcl::WindowDataMask::All));
        OUString sUserData = m_xFileView->GetConfigString();
        aDlgOpt.SetUserItem( u"UserData"_ustr,
                             Any( sUserData ) );
    }

    m_xFileView->SetSelectHdl(Link<SvtFileView*,void>());

    // Save bookmarked places
    if (!m_xImpl->m_xPlaces->IsUpdated())
        return;

    const std::vector<PlacePtr> aPlaces = m_xImpl->m_xPlaces->GetPlaces();
    Sequence< OUString > placesUrlsList(m_xImpl->m_xPlaces->GetNbEditablePlaces());
    auto placesUrlsListRange = asNonConstRange(placesUrlsList);
    Sequence< OUString > placesNamesList(m_xImpl->m_xPlaces->GetNbEditablePlaces());
    auto placesNamesListRange = asNonConstRange(placesNamesList);
    int i(0);
    for (auto const& place : aPlaces)
    {
        if(place->IsEditable()) {
            placesUrlsListRange[i] = place->GetUrl();
            placesNamesListRange[i] = place->GetName();
            ++i;
        }
    }

    std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch);
    officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch);
    batch->commit();
}

IMPL_LINK_NOARG(SvtFileDialog, NewFolderHdl_Impl, weld::Button&, void)
{
    m_xFileView->EndInplaceEditing();

    SmartContent aContent( m_xFileView->GetViewURL( ) );
    OUString aTitle;
    aContent.getTitle( aTitle );
    QueryFolderNameDialog aDlg(m_xDialog.get(), aTitle, FpsResId(STR_SVT_NEW_FOLDER));
    bool bHandled = false;

    while ( !bHandled )
    {
        if (aDlg.run() == RET_OK)
        {
            OUString aUrl = aContent.createFolder(aDlg.GetName());
            if ( !aUrl.isEmpty( ) )
            {
                m_xFileView->CreatedFolder(aUrl, aDlg.GetName());
                bHandled = true;
            }
        }
        else
            bHandled = true;
    }
}

void SvtFileDialog::createNewUserFilter( const OUString& _rNewFilter )
{
    // delete the old user filter and create a new one
    m_xImpl->m_xUserFilter.reset( new SvtFileDialogFilter_Impl( _rNewFilter, _rNewFilter ) );

    // remember the extension
    bool bIsAllFiles = _rNewFilter == FILEDIALOG_FILTER_ALL;
    if ( bIsAllFiles )
        EraseDefaultExt();
    else
        SetDefaultExt( _rNewFilter.copy( 2 ) );
        // TODO: this is nonsense. In the whole file there are a lot of places where we assume that a user filter
        // is always "*.<something>". But changing this would take some more time than I have now...

    // now, the default extension is set to the one of the user filter (or empty)
    if ( m_xImpl->GetCurFilter( ) )
        SetDefaultExt( m_xImpl->GetCurFilter( )->GetExtension() );
    else
        EraseDefaultExt();
}


AdjustFilterFlags SvtFileDialog::adjustFilter( const OUString& rFilter )
{
    AdjustFilterFlags nReturn = AdjustFilterFlags::NONE;

    const bool bNonEmpty = !rFilter.isEmpty();
    if ( bNonEmpty )
    {
        nReturn |= AdjustFilterFlags::NonEmpty;

        bool bFilterChanged = true;

        // search for a corresponding filter
        SvtFileDialogFilter_Impl* pFilter = FindFilter_Impl( rFilter, false, bFilterChanged );

        // look for multi-ext filters if necessary
        if ( !pFilter )
            pFilter = FindFilter_Impl( rFilter, true, bFilterChanged );

        if ( bFilterChanged )
            nReturn |= AdjustFilterFlags::Changed;

        if ( !pFilter )
        {
            nReturn |= AdjustFilterFlags::UserFilter;
            // no filter found : use it as user defined filter
            createNewUserFilter( rFilter );
        }
    }

    return nReturn;
}

IMPL_LINK_NOARG(SvtFileDialog, CancelHdl_Impl, weld::Button&, void)
{
    if ( m_pCurrentAsyncAction.is() )
    {
        m_pCurrentAsyncAction->cancel();
        onAsyncOperationFinished();
    }
    else
    {
        m_xDialog->response(RET_CANCEL);
    }
}

IMPL_LINK( SvtFileDialog, OpenClickHdl_Impl, weld::Button&, rVoid, void )
{
    OpenHdl_Impl(&rVoid);
}

IMPL_LINK( SvtFileDialog, OpenUrlHdl_Impl, weld::ComboBox&, rVoid, bool )
{
    OpenHdl_Impl(&rVoid);
    return true;
}

void SvtFileDialog::OpenHdl_Impl(void const * pVoid)
{
    if ( m_xImpl->m_bMultiSelection && m_xFileView->GetSelectionCount() > 1 )
    {
        // special open in case of multiselection
        OpenMultiSelection_Impl();
        return;
    }

    OUString aFileName;
    OUString aOldPath(m_xFileView->GetViewURL());
    if ( m_xImpl->m_bDoubleClick || m_xFileView->has_focus() )
    {
        // Selection done by doubleclicking in the view, get filename from the view
        aFileName = m_xFileView->GetCurrentURL();
    }

    if ( aFileName.isEmpty() )
    {
        // if an entry is selected in the view...
        if ( m_xFileView->GetSelectionCount() )
        {   // -> use this one. This will allow us to step down this folder
            aFileName = m_xFileView->GetCurrentURL();
        }
    }

    if ( aFileName.isEmpty() )
    {
        // get the URL from the edit field ( if not empty )
        if ( !m_xImpl->m_xEdFileName->get_active_text().isEmpty() )
        {
            OUString aText = m_xImpl->m_xEdFileName->get_active_text();

            // did we reach the root?
            if ( !INetURLObject( aOldPath ).getSegmentCount() )
            {
                if ( ( aText.getLength() == 2 && aText == ".." ) ||
                     ( aText.getLength() == 3 && ( aText == "..\\" || aText == "../" ) ) )
                    // don't go higher than the root
                    return;
            }

#if defined( UNX )
            if ( ( 1 == aText.getLength() ) && ( '~' == aText[0] ) )
            {
                // go to the home directory
                if ( lcl_getHomeDirectory( m_xFileView->GetViewURL(), aFileName ) )
                    // in case we got a home dir, reset the text of the edit
                    m_xImpl->m_xEdFileName->set_entry_text( OUString() );
            }
            if ( aFileName.isEmpty() )
#endif
            {
                // get url from autocomplete edit
                aFileName = m_xImpl->m_xEdFileName->GetURL();
            }
        }
        else if ( pVoid == m_xImpl->m_xBtnFileOpen.get() )
            // OpenHdl was called for the "Open" Button; if edit field is empty, use selected element in the view
            aFileName = m_xFileView->GetCurrentURL();
    }

    // MBA->PB: ?!
    if ( aFileName.isEmpty() && pVoid == m_xImpl->m_xEdFileName.get() && m_xImpl->m_xUserFilter )
    {
        m_xImpl->m_xUserFilter.reset();
        return;
    }

    sal_Int32 nLen = aFileName.getLength();
    if ( !nLen )
    {
        // if the dialog was opened to select a folder, the last selected folder should be selected
        if( m_xImpl->m_eDlgType == FILEDLG_TYPE_PATHDLG )
        {
            aFileName = m_xImpl->m_xEdCurrentPath->get_active_text();
            nLen = aFileName.getLength();
        }
        else
            // no file selected !
            return;
    }

    // mark input as selected
    m_xImpl->m_xEdFileName->select_entry_region(0, nLen);

    // if a path with wildcards is given, divide the string into path and wildcards
    OUString aFilter;
    if ( !SvtFileDialog::IsolateFilterFromPath_Impl( aFileName, aFilter ) )
        return;

    // if a filter was retrieved, there were wildcards !
    AdjustFilterFlags nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & AdjustFilterFlags::Changed )
    {
        // cut off all text before wildcard in edit and select wildcard
        m_xImpl->m_xEdFileName->set_entry_text( aFilter );
        m_xImpl->m_xEdFileName->select_entry_region(0, -1);
    }

    {
        INetURLObject aFileObject( aFileName );
        if ( ( aFileObject.GetProtocol() == INetProtocol::NotValid ) && !aFileName.isEmpty() )
        {
            OUString sCompleted = SvtURLBox::ParseSmart( aFileName, m_xFileView->GetViewURL() );
            if ( !sCompleted.isEmpty() )
                aFileName = sCompleted;
        }
    }

    // check if it is a folder
    bool bIsFolder = false;

    // first thing before doing anything with the content: Reset it. When the user presses "open" (or "save" or "export",
    // for that matter), s/he wants the complete handling, including all possible error messages, even if s/he
    // does the same thing for the same content twice, s/he wants both fails to be displayed.
    // Without the reset, it could be that the content cached all relevant information, and will not display any
    // error messages for the same content a second time...
    m_aContent.bindTo( OUString( ) );

    if ( !aFileName.isEmpty() )
    {
        // Make sure we have own Interaction Handler in place. We do not need
        // to intercept interactions here, but to record the fact that there
        // was an interaction.
        SmartContent::InteractionHandlerType eInterActionHandlerType
            = m_aContent.queryCurrentInteractionHandler();
        if ( ( eInterActionHandlerType == SmartContent::IHT_NONE ) ||
             ( eInterActionHandlerType == SmartContent::IHT_DEFAULT ) )
            m_aContent.enableOwnInteractionHandler(
                OFilePickerInteractionHandler::E_NOINTERCEPTION );

        bIsFolder = m_aContent.isFolder( aFileName );

        // access denied to the given resource - and interaction was already
        // used => break following operations
        OFilePickerInteractionHandler* pHandler
            = m_aContent.getOwnInteractionHandler();

        OSL_ENSURE( pHandler, "Got no Interaction Handler!!!" );

        if ( pHandler->wasAccessDenied() )
            return;

        if ( m_aContent.isInvalid() &&
             ( m_xImpl->m_eMode == FILEDLG_MODE_OPEN ) )
        {
            if ( !pHandler->wasUsed() )
                ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTS );

            return;
        }

        // restore previous Interaction Handler
        if ( eInterActionHandlerType == SmartContent::IHT_NONE )
            m_aContent.disableInteractionHandler();
        else if ( eInterActionHandlerType == SmartContent::IHT_DEFAULT )
            m_aContent.enableDefaultInteractionHandler();
    }

    if  (   !bIsFolder                              // no existent folder
        &&  m_xImpl->m_xCbAutoExtension             // auto extension is enabled in general
        &&  m_xImpl->m_xCbAutoExtension->get_active()// auto extension is really to be used
        &&  !GetDefaultExt().isEmpty()              // there is a default extension
        &&  GetDefaultExt() != "*" // the default extension is not "all"
        && !(   FILEDLG_MODE_SAVE == m_xImpl->m_eMode    // we're saving a file
            &&  m_xFileView->GetSelectionCount()          // there is a selected file in the file view -> it will later on
            )                                                   //    (in SvtFileDialog::GetPathList) be taken as file to save to

        && FILEDLG_MODE_OPEN != m_xImpl->m_eMode // #i83408# don't append extension on open
        )
    {
        // check extension and append the default extension if necessary
        appendDefaultExtension(aFileName,
                               GetDefaultExt(),
                               m_xImpl->GetCurFilter()->GetType());
    }

    bool bOpenFolder = ( FILEDLG_TYPE_PATHDLG == m_xImpl->m_eDlgType ) &&
                       !m_xImpl->m_bDoubleClick && pVoid != m_xImpl->m_xEdFileName.get();
    if ( bIsFolder )
    {
        if ( bOpenFolder )
        {
            m_aPath = aFileName;
        }
        else
        {
            if ( aFileName != m_xFileView->GetViewURL() )
            {
                OpenURL_Impl( aFileName );
            }
            else
            {
                if ( nNewFilterFlags & AdjustFilterFlags::Changed )
                    ExecuteFilter();
            }

            return;
        }
    }
    else if ( !( nNewFilterFlags & AdjustFilterFlags::NonEmpty ) )
    {
        // if applicable save URL
        m_aPath = aFileName;
    }
    else
    {
        // if applicable filter again
        if ( nNewFilterFlags & AdjustFilterFlags::Changed )
            ExecuteFilter();
        return;
    }

    INetURLObject aFileObj( aFileName );
    if ( aFileObj.HasError() )
    {
        ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        return;
    }

    switch (m_xImpl->m_eMode)
    {
        case FILEDLG_MODE_SAVE:
        {
            if ( ::utl::UCBContentHelper::Exists( aFileObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) ) )
            {
                OUString aMsg = FpsResId(STR_SVT_ALREADYEXISTOVERWRITE);
                aMsg = aMsg.replaceFirst(
                    "$filename$",
                    aFileObj.getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset)
                );
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Question, VclButtonsType::YesNo, aMsg));
                if (xBox->run() != RET_YES)
                    return;
            }
            else
            {
                OUString aCurPath;
                if (osl::FileBase::getSystemPathFromFileURL(aFileName, aCurPath) == osl::FileBase::E_None)
                {
                    // if content does not exist: at least its path must exist
                    INetURLObject aPathObj = aFileObj;
                    aPathObj.removeSegment();
                    bool bFolder = m_aContent.isFolder( aPathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                    if ( !bFolder )
                    {
                        ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTSPATH );
                        return;
                    }
                }
            }
        }
        break;

        case FILEDLG_MODE_OPEN:
        {
            // do an existence check herein, again

            if ( INetProtocol::File == aFileObj.GetProtocol( ) )
            {
                bool bExists = m_aContent.is( aFileObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

                if ( !bExists )
                {
                    OUString sError(FpsResId(RID_FILEOPEN_NOTEXISTENTFILE));

                    OUString sInvalidFile( aFileObj.GetMainURL( INetURLObject::DecodeMechanism::ToIUri ) );
                    if ( INetProtocol::File == aFileObj.GetProtocol() )
                    {   // if it's a file URL, transform the URL into system notation
                        OUString sURL( sInvalidFile );
                        OUString sSystem;
                        osl_getSystemPathFromFileURL( sURL.pData, &sSystem.pData );
                        sInvalidFile = sSystem;
                    }
                    sError = sError.replaceFirst( "$name$", sInvalidFile );

                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                              VclMessageType::Warning, VclButtonsType::Ok, sError));
                    xBox->run();
                    return;
                }
            }
        }
        break;

        default:
            OSL_FAIL("SvtFileDialog, OpenHdl_Impl: invalid mode!");
    }

    m_xDialog->response(RET_OK);
}

void SvtFileDialog::EnableAutocompletion(bool bEnable)
{
    m_xImpl->m_xEdFileName->EnableAutocomplete(bEnable);
}

IMPL_LINK_NOARG( SvtFileDialog, FilterSelectHdl_Impl, weld::ComboBox&, void )
{
    OUString sSelectedFilterDisplayName;
    SvtFileDialogFilter_Impl* pSelectedFilter = m_xImpl->GetSelectedFilterEntry( sSelectedFilterDisplayName );
    if ( !pSelectedFilter )
    {   // there is no current selection. This happens if for instance the user selects a group separator using
        // the keyboard, and then presses enter: When the selection happens, we immediately deselect the entry,
        // so in this situation there is no current selection.
        restoreCurrentFilter( m_xImpl );
    }
    else
    {
        if (   ( pSelectedFilter != m_xImpl->GetCurFilter() )
           ||  m_xImpl->m_xUserFilter
           )
        {
            // Store the old filter for the auto extension handling
            OUString sLastFilterExt = m_xImpl->GetCurFilter()->GetExtension();
            m_xImpl->m_xUserFilter.reset();

            // if applicable remove filter of the user
            m_xImpl->SetCurFilter( pSelectedFilter, sSelectedFilterDisplayName );

            // if applicable show extension
            SetDefaultExt( pSelectedFilter->GetExtension() );
            sal_Int32 nSepPos = GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );

            if ( nSepPos != -1 )
                EraseDefaultExt( nSepPos );

            // update the extension of the current file if necessary
            lcl_autoUpdateFileExtension( this, sLastFilterExt );

            // if the user is traveling fast through the filterbox
            // do not filter instantly
            // FilterSelectHdl_Impl should be started again at idle
            m_xImpl->m_aFilterIdle.Start();
        }
    }
}

IMPL_LINK_NOARG(SvtFileDialog, FilterSelectTimerHdl_Impl, Timer*, void)
{
    // filter the view again
    ExecuteFilter();
}

IMPL_LINK_NOARG( SvtFileDialog, FileNameGetFocusHdl_Impl, weld::Widget&, void )
{
    m_xFileView->SetNoSelection();
}

IMPL_LINK( SvtFileDialog, FileNameModifiedHdl_Impl, weld::ComboBox&, rComboBox, void )
{
    FileNameGetFocusHdl_Impl(rComboBox);
}

IMPL_LINK_NOARG(SvtFileDialog, URLBoxModifiedHdl_Impl, weld::ComboBox&, bool)
{
    OUString aPath = m_xImpl->m_xEdCurrentPath->GetURL();
    OpenURL_Impl(aPath);
    return true;
}

IMPL_LINK_NOARG( SvtFileDialog, ConnectToServerPressed_Hdl, weld::Button&, void )
{
    m_xFileView->EndInplaceEditing();

    PlaceEditDialog aDlg(m_xDialog.get());
    short aRetCode = aDlg.run();

    switch (aRetCode) {
        case RET_OK :
        {
            PlacePtr newPlace = aDlg.GetPlace();
            m_xImpl->m_xPlaces->AppendPlace(newPlace);

            break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    }
}

IMPL_LINK_NOARG ( SvtFileDialog, AddPlacePressed_Hdl, weld::Button&, void )
{
    // Maybe open the PlacesDialog would have been a better idea
    // there is an ux choice to make we did not make...
    INetURLObject aURLObj( m_xFileView->GetViewURL() );
    PlacePtr newPlace =
        std::make_shared<Place>( aURLObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset),
                m_xFileView->GetViewURL(), true);
    m_xImpl->m_xPlaces->AppendPlace(newPlace);
}

IMPL_LINK_NOARG ( SvtFileDialog, RemovePlacePressed_Hdl, weld::Button&, void )
{
    m_xImpl->m_xPlaces->RemoveSelectedPlace();
}

SvtFileDialogFilter_Impl* SvtFileDialog::FindFilter_Impl
(
    const OUString& rFilter,
    bool bMultiExt,/*  TRUE - regard filter with several extensions
                            FALSE - do not ...
                        */
    bool& rFilterChanged
)

/*  [Description]

    This method looks for the specified extension in the included filters.
*/

{
    SvtFileDialogFilter_Impl* pFoundFilter = nullptr;
    SvtFileDialogFilterList_Impl& rList = m_xImpl->m_aFilter;
    sal_uInt16 nFilter = rList.size();

    while ( nFilter-- )
    {
        SvtFileDialogFilter_Impl* pFilter = rList[ nFilter ].get();
        const OUString& rType = pFilter->GetType();

        if ( bMultiExt )
        {
            sal_Int32 nIdx = 0;
            while ( !pFoundFilter && nIdx != -1 )
            {
                const OUString aSingleType = rType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nIdx );
#ifdef UNX
                if ( aSingleType == rFilter )
#else
                if ( aSingleType.equalsIgnoreAsciiCase( rFilter ) )
#endif
                    pFoundFilter = pFilter;
            }
        }
#ifdef UNX
        else if ( rType == rFilter )
#else
        else if ( rType.equalsIgnoreAsciiCase( rFilter ) )
#endif
            pFoundFilter = pFilter;

        if ( pFoundFilter )
        {
            // activate filter
            rFilterChanged = m_xImpl->m_xUserFilter || ( m_xImpl->GetCurFilter() != pFilter );

            createNewUserFilter( rFilter );

            break;
        }
    }
    return pFoundFilter;
}


void SvtFileDialog::ExecuteFilter()
{
    executeAsync( AsyncPickerAction::eExecuteFilter, OUString(), getMostCurrentFilter(m_xImpl) );
}

/*  [Description]

    OpenHandler for MultiSelection
*/
void SvtFileDialog::OpenMultiSelection_Impl()
{
    SvtContentEntry* pEntry = m_xFileView->FirstSelected();

    if (pEntry)
        m_aPath = pEntry->maURL;

    m_xDialog->response(RET_OK);
}

void SvtFileDialog::UpdateControls( const OUString& rURL )
{
    m_xImpl->m_xEdFileName->SetBaseURL( rURL );

    INetURLObject aObj( rURL );

    {
        OUString sText;
        SAL_WARN_IF( INetProtocol::NotValid == aObj.GetProtocol(), "fpicker.office", "SvtFileDialog::UpdateControls: Invalid URL!" );

        if ( aObj.getSegmentCount() )
        {
            osl::FileBase::getSystemPathFromFileURL(rURL, sText);
            if ( !sText.isEmpty() )
            {
                // no Fsys path for server file system ( only UCB has mountpoints! )
                if ( INetProtocol::File != aObj.GetProtocol() )
                    sText = rURL.copy( INetURLObject::GetScheme( aObj.GetProtocol() ).getLength() );
            }

            if ( sText.isEmpty() && aObj.getSegmentCount() )
                sText = rURL;
        }

        // path mode ?
        if ( FILEDLG_TYPE_PATHDLG == m_xImpl->m_eDlgType )
            // -> set new path in the edit field
            m_xImpl->m_xEdFileName->set_entry_text( sText );

        // in the "current path" field, truncate the trailing slash
        if ( aObj.hasFinalSlash() )
        {
            aObj.removeFinalSlash();
            OUString sURL( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            if (osl::FileBase::getSystemPathFromFileURL(sURL, sText) != osl::FileBase::E_None)
                sText = sURL;
        }

        if ( sText.isEmpty() && !rURL.isEmpty() )
            // happens, for instance, for URLs which the INetURLObject does not know to belong to a hierarchical scheme
            sText = rURL;
        m_xImpl->m_xEdCurrentPath->set_entry_text(sText);
    }

    m_aPath = rURL;

    m_xImpl->m_xBtnUp->FillURLMenu();

    if (m_pFileNotifier)
        m_pFileNotifier->notify( DIRECTORY_CHANGED, 0 );
}

IMPL_LINK( SvtFileDialog, SelectHdl_Impl, SvtFileView*, pBox, void )
{
    SvtContentEntry* pUserData = pBox->FirstSelected();
    if (pUserData)
    {
        INetURLObject aObj( pUserData->maURL );
        if ( FILEDLG_TYPE_PATHDLG == m_xImpl->m_eDlgType )
        {
            if ( aObj.GetProtocol() == INetProtocol::File )
            {
                if ( !pUserData->mbIsFolder )
                    aObj.removeSegment();
                OUString aName = aObj.getFSysPath( static_cast<FSysStyle>(FSysStyle::Detect & ~FSysStyle::Vos) );
                m_xImpl->m_xEdFileName->set_entry_text( aName );
                m_xImpl->m_xEdFileName->select_entry_region(0, -1);
                m_aPath = pUserData->maURL;
            }
            else if ( !pUserData->mbIsFolder )
            {
                m_xImpl->m_xEdFileName->set_entry_text( pUserData->maURL );
                m_xImpl->m_xEdFileName->select_entry_region(0, -1);
                m_aPath = pUserData->maURL;
            }
            else
               m_xImpl->m_xEdFileName->set_entry_text( OUString() );
        }
        else
        {
            if ( !pUserData->mbIsFolder )
            {
                OUString aName = pBox->get_selected_text();
                m_xImpl->m_xEdFileName->set_entry_text( aName );
                m_xImpl->m_xEdFileName->select_entry_region(0, -1);
                m_aPath = pUserData->maURL;
            }
        }
    }

    if ( m_xImpl->m_bMultiSelection && m_xFileView->GetSelectionCount() > 1 )
    {
        // clear the file edit for multiselection
        m_xImpl->m_xEdFileName->set_entry_text( OUString() );
    }

    FileSelect();
}

IMPL_LINK_NOARG(SvtFileDialog, DblClickHdl_Impl, SvtFileView*, bool)
{
    m_xImpl->m_bDoubleClick = true;
    OpenHdl_Impl( nullptr );
    m_xImpl->m_bDoubleClick = false;
    return true;
}

IMPL_LINK_NOARG(SvtFileDialog, EntrySelectHdl_Impl, weld::ComboBox&, void)
{
    FileSelect();
}

IMPL_LINK( SvtFileDialog, OpenDoneHdl_Impl, SvtFileView*, pView, void )
{
    const OUString& sCurrentFolder( pView->GetViewURL() );
    // check if we can create new folders
    EnableControl( m_xImpl->m_xBtnNewFolder.get(), ContentCanMakeFolder( sCurrentFolder ) );

    // check if we can travel one level up
    bool bCanTravelUp = ContentHasParentFolder( pView->GetViewURL() );
    if ( bCanTravelUp )
    {
        // additional check: the parent folder should not be prohibited
        INetURLObject aCurrentFolder( sCurrentFolder );
        SAL_WARN_IF( INetProtocol::NotValid == aCurrentFolder.GetProtocol(),
            "fpicker.office", "SvtFileDialog::OpenDoneHdl_Impl: invalid current URL!" );

        aCurrentFolder.removeSegment();
    }
    EnableControl( m_xImpl->m_xBtnUp->getWidget(), bCanTravelUp );
}

IMPL_LINK_NOARG(SvtFileDialog, AutoExtensionHdl_Impl, weld::Toggleable&, void)
{
    if (m_pFileNotifier)
        m_pFileNotifier->notify(CTRL_STATE_CHANGED, CHECKBOX_AUTOEXTENSION);

    // update the extension of the current file if necessary
    lcl_autoUpdateFileExtension( this, m_xImpl->GetCurFilter()->GetExtension() );
}

IMPL_LINK( SvtFileDialog, ClickHdl_Impl, weld::Toggleable&, rCheckBox, void )
{
    if (!m_pFileNotifier)
        return;

    sal_Int16 nId = -1;

    if ( &rCheckBox == m_xImpl->m_xCbOptions.get() )
        nId = CHECKBOX_FILTEROPTIONS;
    else if ( &rCheckBox == m_xCbSelection.get() )
        nId = CHECKBOX_SELECTION;
    else if ( &rCheckBox == m_xCbReadOnly.get() )
        nId = CHECKBOX_READONLY;
    else if ( &rCheckBox == m_xImpl->m_xCbPassword.get() )
        nId = CHECKBOX_PASSWORD;
    else if ( &rCheckBox == m_xImpl->m_xCbGPGEncrypt.get() )
        nId = CHECKBOX_GPGENCRYPTION;
    else if ( &rCheckBox == m_xCbLinkBox.get() )
        nId = CHECKBOX_LINK;
    else if ( &rCheckBox == m_xCbPreviewBox.get() )
        nId = CHECKBOX_PREVIEW;

    if ( nId != -1 )
        m_pFileNotifier->notify( CTRL_STATE_CHANGED, nId );
}

IMPL_LINK_NOARG(SvtFileDialog, PlayButtonHdl_Impl, weld::Button&, void)
{
    if (m_pFileNotifier)
        m_pFileNotifier->notify(CTRL_STATE_CHANGED, PUSHBUTTON_PLAY);
}

namespace
{

bool implIsInvalid( const OUString & rURL )
{
    SmartContent aContent( rURL );
    aContent.enableOwnInteractionHandler( ::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST );
    aContent.isFolder();    // do this _before_ asking isInvalid! Otherwise result might be wrong.
    return aContent.isInvalid();
}

}


OUString SvtFileDialog::implGetInitialURL( const OUString& _rPath, std::u16string_view _rFallback )
{
    // a URL parser for the fallback
    INetURLObject aURLParser;

    // set the path
    bool bWasAbsolute = false;
    aURLParser = aURLParser.smartRel2Abs( _rPath, bWasAbsolute );

    // is it a valid folder?
    m_aContent.bindTo( aURLParser.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    bool bIsFolder = m_aContent.isFolder( );    // do this _before_ asking isInvalid!
    bool bIsInvalid = m_aContent.isInvalid();

    if ( bIsInvalid && m_bHasFilename && !aURLParser.hasFinalSlash() )
    {   // check if the parent folder exists
        INetURLObject aParent( aURLParser );
        aParent.removeSegment( );
        aParent.setFinalSlash( );
        bIsInvalid = implIsInvalid( aParent.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
    }

    if ( bIsInvalid )
    {
        INetURLObject aFallback( _rFallback );
        bIsInvalid = implIsInvalid( aFallback.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        if ( !bIsInvalid )
            aURLParser = aFallback;
    }

    if ( bIsInvalid )
    {
        INetURLObject aParent( aURLParser );
        while ( bIsInvalid && aParent.removeSegment() )
        {
            aParent.setFinalSlash( );
            bIsInvalid = implIsInvalid( aParent.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        }

        if ( !bIsInvalid )
            aURLParser = aParent;
    }

    if ( !bIsInvalid && bIsFolder )
    {
        aURLParser.setFinalSlash();
    }
    return aURLParser.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}


short SvtFileDialog::run()
{
    if ( !PrepareExecute() )
        return 0;

    // start the dialog
    m_bIsInExecute = true;
    short nResult = GenericDialogController::run();
    m_bIsInExecute = false;

    SAL_WARN_IF( m_pCurrentAsyncAction.is(), "fpicker.office", "SvtFilePicker::run: still running an async action!" );
        // the dialog should not be cancellable while an async action is running - first, the action
        // needs to be cancelled

    // remember last directory
    if ( RET_OK == nResult )
    {
        INetURLObject aURL( m_aPath );
        if ( aURL.GetProtocol() == INetProtocol::File )
        {
            // remember the selected directory only for file URLs not for virtual folders
            sal_Int32 nLevel = aURL.getSegmentCount();
            bool bDir = m_aContent.isFolder( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            if ( nLevel > 1 && ( FILEDLG_TYPE_FILEDLG == m_xImpl->m_eDlgType || !bDir ) )
                aURL.removeSegment();
        }
    }

    return nResult;
}

void SvtFileDialog::onAsyncOperationStarted()
{
    EnableUI( false );
    // the cancel button must be always enabled
    m_xImpl->m_xBtnCancel->set_sensitive(true);
    m_xImpl->m_xBtnCancel->grab_focus();
}

void SvtFileDialog::onAsyncOperationFinished()
{
    EnableUI( true );
    m_pCurrentAsyncAction = nullptr;
    if ( !m_bInExecuteAsync )
        m_xImpl->m_xEdFileName->grab_focus();
    // (if m_bInExecuteAsync is true, then the operation was finished within the minimum wait time,
    // and to the user, the operation appears to be synchronous)
}

void SvtFileDialog::RemovablePlaceSelected(bool enable)
{
    m_xImpl->m_xPlaces->SetDelEnabled( enable );
}

void SvtFileDialog::displayIOException( const OUString& _rURL, IOErrorCode _eCode )
{
    try
    {
        // create make a human-readable string from the URL
        OUString sDisplayPath;
        if (osl::FileBase::getSystemPathFromFileURL(_rURL, sDisplayPath)
            == osl::FileBase::E_None)
        {
            sDisplayPath = _rURL;
        }

        // build an own exception which tells "access denied"
        InteractiveAugmentedIOException aException;
        aException.Arguments =
        { css::uno::Any(sDisplayPath),
          css::uno::Any(PropertyValue(
            u"Uri"_ustr,
            -1, aException.Arguments[ 0 ], PropertyState_DIRECT_VALUE
          )) };
            // (formerly, it was sufficient to put the URL first parameter. Nowadays,
            // the services expects the URL in a PropertyValue named "Uri" ...)
        aException.Code = _eCode;
        aException.Classification = InteractionClassification_ERROR;

        // let and interaction handler handle this exception
        rtl::Reference<::comphelper::OInteractionRequest> pRequest =
            new ::comphelper::OInteractionRequest( Any( aException ) );
        pRequest->addContinuation( new ::comphelper::OInteractionAbort( ) );

        Reference< XInteractionHandler2 > xHandler(
            InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr ) );
        xHandler->handle( pRequest );
    }
    catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "fpicker", "iodlg::displayIOException" );
    }
}

void SvtFileDialog::EnableUI(bool bEnable)
{
    m_xDialog->set_sensitive(bEnable);

    if (bEnable)
    {
        for (auto& rxControl : m_aDisabledControls)
        {
            rxControl->set_sensitive(false);
        }
    }
}

void SvtFileDialog::EnableControl(weld::Widget* pControl, bool bEnable)
{
    if (!pControl)
    {
        SAL_WARN( "fpicker.office", "SvtFileDialog::EnableControl: invalid control!" );
        return;
    }

    pControl->set_sensitive(bEnable);

    if (bEnable)
    {
        auto aPos = m_aDisabledControls.find( pControl );
        if ( m_aDisabledControls.end() != aPos )
            m_aDisabledControls.erase( aPos );
    }
    else
        m_aDisabledControls.insert( pControl );
}

bool SvtFileDialog::PrepareExecute()
{
    if (comphelper::LibreOfficeKit::isActive())
        return false;

    OUString aEnvValue;
    if ( getEnvironmentValue( "WorkDirMustContainRemovableMedia", aEnvValue ) && aEnvValue == "1" )
    {
        try
        {
            INetURLObject aStdDir( GetStandardDir() );
            ::ucbhelper::Content aCnt( aStdDir.GetMainURL(
                                                    INetURLObject::DecodeMechanism::NONE ),
                                 Reference< XCommandEnvironment >(),
                                 comphelper::getProcessComponentContext() );
            Sequence< OUString > aProps { u"IsVolume"_ustr, u"IsRemoveable"_ustr };

            Reference< XResultSet > xResultSet
                = aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_ONLY );
            if ( xResultSet.is() && !xResultSet->next() )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Warning, VclButtonsType::Ok,
                                                          FpsResId(STR_SVT_NOREMOVABLEDEVICE)));
                xBox->run();
                return false;
            }
        }
        catch ( ContentCreationException const & )
        {
        }
        catch ( CommandAbortedException const & )
        {
        }
    }

    if ( ( m_xImpl->m_nStyle & PickerFlags::SaveAs ) && m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);
    else
        m_aContent.enableDefaultInteractionHandler();

    // possibly just a filename without a path
    OUString aFileNameOnly;
    if( !m_aPath.isEmpty() && (m_xImpl->m_eMode == FILEDLG_MODE_SAVE)
                     && (m_aPath.indexOf(':') == -1)
                     && (m_aPath.indexOf('\\') == -1)
                     && (m_aPath.indexOf('/') == -1))
    {
        aFileNameOnly = m_aPath;
        m_aPath.clear();
    }

    // no starting path specified?
    if ( m_aPath.isEmpty() )
    {
        // then use the standard directory
        m_aPath = lcl_ensureFinalSlash( m_xImpl->GetStandardDir() );

        // attach given filename to path
        if ( !aFileNameOnly.isEmpty() )
            m_aPath += aFileNameOnly;
    }


    m_aPath = implGetInitialURL( m_aPath, GetStandardDir() );

    if ( m_xImpl->m_nStyle & PickerFlags::SaveAs && !m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);

    // if applicable show filter
    m_xImpl->InitFilterList();

    // set up initial filter
    sal_uInt16 nFilterCount = GetFilterCount();
    OUString aAll = FpsResId( STR_FILTERNAME_ALL );
    bool bHasAll = m_xImpl->HasFilterListEntry( aAll );
    if ( m_xImpl->GetCurFilter() || nFilterCount == 1 || ( nFilterCount == 2 && bHasAll ) )
    {
        // if applicable set the only filter or the only filter that
        // does not refer to all files, as the current one
        if ( !m_xImpl->GetCurFilter() )
        {
            sal_uInt16 nPos = 0;
            if ( 2 == nFilterCount && bHasAll )
            {
                nPos = nFilterCount;
                while ( nPos-- )
                {
                    if ( aAll != GetFilterName( nPos ) )
                        break;
                }
            }
            SvtFileDialogFilter_Impl* pNewCurFilter = m_xImpl->m_aFilter[ nPos ].get();
            assert( pNewCurFilter && "SvtFileDialog::run: invalid filter pos!" );
            m_xImpl->SetCurFilter( pNewCurFilter, pNewCurFilter->GetName() );
        }

        // adjust view
        m_xImpl->SelectFilterListEntry( m_xImpl->GetCurFilter()->GetName() );
        SetDefaultExt( m_xImpl->GetCurFilter()->GetExtension() );
        sal_Int32 nSepPos = GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );
        if ( nSepPos != -1 )
            EraseDefaultExt( nSepPos );
    }
    else
    {
        // if applicable set respectively create filter for all files
        if ( !bHasAll )
        {
            SvtFileDialogFilter_Impl* pAllFilter = implAddFilter( aAll, FILEDIALOG_FILTER_ALL );
            m_xImpl->InsertFilterListEntry( pAllFilter );
            m_xImpl->SetCurFilter( pAllFilter, aAll );
        }
        m_xImpl->SelectFilterListEntry( aAll );
    }

    // if applicable isolate filter
    OUString aFilter;

    if ( !IsolateFilterFromPath_Impl( m_aPath, aFilter ) )
        return false;

    AdjustFilterFlags nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & ( AdjustFilterFlags::NonEmpty | AdjustFilterFlags::UserFilter ) )
    {
        m_xImpl->m_xEdFileName->set_entry_text( aFilter );
    }

    // create and show instance for set path
    INetURLObject aFolderURL( m_aPath );
    OUString aFileName( aFolderURL.getName( INetURLObject::LAST_SEGMENT, false ) );
    sal_Int32 nFileNameLen = aFileName.getLength();
    bool bFileToSelect = nFileNameLen != 0;
    if ( bFileToSelect && aFileName[ nFileNameLen - 1 ] != '/' )
    {
        OUString aDecodedName = aFolderURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
        m_xImpl->m_xEdFileName->set_entry_text( aDecodedName );
        aFolderURL.removeSegment();
    }

    INetURLObject aObj = aFolderURL;
    if ( aObj.GetProtocol() == INetProtocol::File )
    {
        // set folder as current directory
        aObj.setFinalSlash();
    }

    UpdateControls( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    // Somebody might want to enable some controls according to the current filter
    FilterSelect();

    OpenURL_Impl( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    // if applicable read and set size from ini
    InitSize();

    return true;
}

void SvtFileDialog::executeAsync( ::svt::AsyncPickerAction::Action eAction,
                                    const OUString& rURL, const OUString& rFilter )
{
    SAL_WARN_IF( m_pCurrentAsyncAction.is(), "fpicker.office", "SvtFileDialog::executeAsync: previous async action not yet finished!" );

    m_pCurrentAsyncAction = new AsyncPickerAction( this, m_xFileView.get(), eAction );

    bool bReallyAsync = true;
    m_aConfiguration.getNodeValue( u"FillAsynchronously"_ustr ) >>= bReallyAsync;

    sal_Int32 nMinTimeout = 0;
    m_aConfiguration.getNodeValue( u"Timeout/Min"_ustr ) >>= nMinTimeout;
    sal_Int32 nMaxTimeout = 0;
    m_aConfiguration.getNodeValue( u"Timeout/Max"_ustr ) >>= nMaxTimeout;

    m_bInExecuteAsync = true;
    m_pCurrentAsyncAction->execute(rURL, rFilter, bReallyAsync ? nMinTimeout : -1, nMaxTimeout, GetDenyList());
    m_bInExecuteAsync = false;
}


void SvtFileDialog::FileSelect()
{
    if (m_pFileNotifier)
        m_pFileNotifier->notify( FILE_SELECTION_CHANGED, 0 );
}


void SvtFileDialog::FilterSelect()
{
    if (m_pFileNotifier)
        m_pFileNotifier->notify( CTRL_STATE_CHANGED,
                                LISTBOX_FILTER );
}


/*  [Description]

   This method sets the path for the default button.
*/
void SvtFileDialog::SetStandardDir( const OUString& rStdDir )
{
    INetURLObject aObj( rStdDir );
    SAL_WARN_IF( aObj.GetProtocol() == INetProtocol::NotValid, "fpicker.office", "Invalid protocol!" );
    aObj.setFinalSlash();
    m_xImpl->SetStandardDir( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
}

void SvtFileDialog::SetDenyList( const css::uno::Sequence< OUString >& rDenyList )
{
    m_xImpl->SetDenyList( rDenyList );
}


const css::uno::Sequence< OUString >& SvtFileDialog::GetDenyList() const
{
    return m_xImpl->GetDenyList();
}


/*  [Description]

    This method returns the standard path.
*/
const OUString& SvtFileDialog::GetStandardDir() const
{
    return m_xImpl->GetStandardDir();
}


void SvtFileDialog::PrevLevel_Impl()
{
    m_xFileView->EndInplaceEditing();

    OUString sDummy;
    executeAsync( AsyncPickerAction::ePrevLevel, sDummy, sDummy );
}

void SvtFileDialog::OpenURL_Impl( const OUString& _rURL )
{
    m_xFileView->EndInplaceEditing();

    executeAsync( AsyncPickerAction::eOpenURL, _rURL, getMostCurrentFilter( m_xImpl ) );
}

SvtFileDialogFilter_Impl* SvtFileDialog::implAddFilter( const OUString& rFilter, const OUString& _rType )
{
    SvtFileDialogFilter_Impl* pNewFilter = new SvtFileDialogFilter_Impl( rFilter, _rType );
    m_xImpl->m_aFilter.push_front( std::unique_ptr<SvtFileDialogFilter_Impl>( pNewFilter ) );

    if ( !m_xImpl->GetCurFilter() )
        m_xImpl->SetCurFilter( pNewFilter, rFilter );

    return pNewFilter;
}

void SvtFileDialog::AddFilter( const OUString& rFilter, const OUString& _rType )
{
    SAL_WARN_IF( m_bIsInExecute, "fpicker.office", "SvtFileDialog::AddFilter: currently executing!" );
    implAddFilter ( rFilter, _rType );
}


void SvtFileDialog::AddFilterGroup( const OUString& rFilter, const Sequence< StringPair >& rFilters )
{
    SAL_WARN_IF( m_bIsInExecute, "fpicker.office", "SvtFileDialog::AddFilter: currently executing!" );

    implAddFilter( rFilter, OUString() );
    const StringPair* pSubFilters       =               rFilters.getConstArray();
    const StringPair* pSubFiltersEnd    = pSubFilters + rFilters.getLength();
    for ( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        implAddFilter( pSubFilters->First, pSubFilters->Second );
}


void SvtFileDialog::SetCurFilter( const OUString& rFilter )
{
    SAL_WARN_IF( m_bIsInExecute, "fpicker.office", "SvtFileDialog::SetCurFilter: currently executing!" );

    // look for corresponding filter
    sal_uInt16 nPos = m_xImpl->m_aFilter.size();

    while ( nPos-- )
    {
        SvtFileDialogFilter_Impl* pFilter = m_xImpl->m_aFilter[ nPos ].get();
        if ( pFilter->GetName() == rFilter )
        {
            m_xImpl->SetCurFilter( pFilter, rFilter );
            break;
        }
    }
}

OUString SvtFileDialog::GetCurFilter() const
{
    OUString aFilter;

    const SvtFileDialogFilter_Impl* pCurrentFilter = m_xImpl->GetCurFilter();
    if ( pCurrentFilter )
        aFilter = pCurrentFilter->GetName();

    return aFilter;
}

OUString SvtFileDialog::getCurFilter( ) const
{
    return GetCurFilter();
}

sal_uInt16 SvtFileDialog::GetFilterCount() const
{
    return m_xImpl->m_aFilter.size();
}

const OUString& SvtFileDialog::GetFilterName( sal_uInt16 nPos ) const
{
    assert( nPos < GetFilterCount() && "invalid index" );
    return m_xImpl->m_aFilter[ nPos ]->GetName();
}

void SvtFileDialog::InitSize()
{
    if (m_xImpl->m_aIniKey.isEmpty())
        return;

    // initialize from config
    SvtViewOptions aDlgOpt( EViewType::Dialog, m_xImpl->m_aIniKey );

    if ( aDlgOpt.Exists() )
    {
        m_xDialog->set_window_state(aDlgOpt.GetWindowState());

        Any aUserData = aDlgOpt.GetUserItem( u"UserData"_ustr);
        OUString sCfgStr;
        if ( aUserData >>= sCfgStr )
            m_xFileView->SetConfigString( sCfgStr );
    }
}

std::vector<OUString> SvtFileDialog::GetPathList() const
{
    std::vector<OUString> aList;

    m_xFileView->selected_foreach([this, &aList](weld::TreeIter& rCurEntry){
        aList.push_back(m_xFileView->GetURL(rCurEntry));
        return false;
    });

    if (aList.empty())
    {
        if ( !m_xImpl->m_xEdFileName->get_active_text().isEmpty() && m_bIsInExecute )
            aList.push_back(m_xImpl->m_xEdFileName->GetURL());
        else
            aList.push_back(m_aPath);
    }

    return aList;
}

bool SvtFileDialog::IsolateFilterFromPath_Impl( OUString& rPath, OUString& rFilter )
{
    OUString aReversePath = comphelper::string::reverseString(rPath);
    sal_Int32 nQuestionMarkPos = rPath.indexOf( '?' );
    sal_Int32 nWildCardPos = rPath.indexOf( FILEDIALOG_DEF_WILDCARD );

    if ( nQuestionMarkPos != -1 )
    {
        // use question mark as wildcard only for files
        INetProtocol eProt = INetURLObject::CompareProtocolScheme( rPath );

        if ( INetProtocol::NotValid != eProt && INetProtocol::File != eProt )
            nQuestionMarkPos = -1;

        nWildCardPos = std::min( nWildCardPos, nQuestionMarkPos );
    }

    rFilter.clear();

    if ( nWildCardPos == -1 )
        return true;

    sal_Int32 nPathTokenPos = aReversePath.indexOf( '/' );

    if ( nPathTokenPos == -1 )
    {
        OUString aDelim(
#if defined(_WIN32)
                '\\'
#else
                '/'
#endif
        );

        nPathTokenPos = aReversePath.indexOf( aDelim );
#if !defined( UNX )
        if ( nPathTokenPos == -1 )
        {
            nPathTokenPos = aReversePath.indexOf( ':' );
        }
#endif
    }

    // check syntax
    if ( nPathTokenPos != -1 )
    {
        if ( nPathTokenPos < (rPath.getLength() - nWildCardPos - 1) )
        {
            ErrorHandler::HandleError( ERRCODE_SFX_INVALIDSYNTAX );
            return false;
        }

        // cut off filter
        rFilter = aReversePath.copy( 0, nPathTokenPos );
        rFilter = comphelper::string::reverseString(rFilter);

        // determine folder
        rPath = aReversePath.copy( nPathTokenPos );
        rPath = comphelper::string::reverseString(rPath);
    }
    else
    {
        rFilter = rPath;
        rPath.clear();
    }

    return true;
}

IMPL_LINK_NOARG(SvtFileDialog, SizeAllocHdl, const Size&, void)
{
    if (m_pFileNotifier)
        m_pFileNotifier->notify(DIALOG_SIZE_CHANGED, 0);
}

weld::Widget* SvtFileDialog::getControl( sal_Int16 nControlId, bool bLabelControl ) const
{
    weld::Widget* pReturn = nullptr;

    switch ( nControlId )
    {
        case CONTROL_FILEVIEW:
            pReturn = bLabelControl ? nullptr : m_xFileView->identifier();
            break;

        case EDIT_FILEURL:
            pReturn =   bLabelControl
                    ? static_cast<weld::Widget*>(m_xImpl->m_xFtFileName.get())
                    : static_cast<weld::Widget*>(m_xImpl->m_xEdFileName->getWidget());
            break;

        case EDIT_FILEURL_LABEL:
            pReturn = m_xImpl->m_xFtFileName.get();
            break;

        case CHECKBOX_AUTOEXTENSION:
            pReturn = m_xImpl->m_xCbAutoExtension.get();
            break;

        case CHECKBOX_PASSWORD:
            pReturn = m_xImpl->m_xCbPassword.get();
            break;

        case CHECKBOX_GPGENCRYPTION:
            pReturn = m_xImpl->m_xCbGPGEncrypt.get();
            break;

        case CHECKBOX_FILTEROPTIONS:
            pReturn = m_xImpl->m_xCbOptions.get();
            break;

        case CHECKBOX_READONLY:
            pReturn = m_xCbReadOnly.get();
            break;

        case CHECKBOX_LINK:
            pReturn = m_xCbLinkBox.get();
            break;

        case CHECKBOX_PREVIEW:
            pReturn = m_xCbPreviewBox.get();
            break;

        case CHECKBOX_SELECTION:
            pReturn = m_xCbSelection.get();
            break;

        case LISTBOX_FILTER:
            pReturn = bLabelControl ? m_xImpl->m_xFtFileType.get() : m_xImpl->GetFilterListControl();
            break;

        case LISTBOX_FILTER_LABEL:
            pReturn = m_xImpl->m_xFtFileType.get();
            break;

        case FIXEDTEXT_CURRENTFOLDER:
            pReturn = m_xImpl->m_xEdCurrentPath->getWidget();
            break;

        case LISTBOX_VERSION:
            pReturn =   bLabelControl
                    ? static_cast<weld::Widget*>(m_xImpl->m_xSharedLabel.get())
                    : static_cast<weld::Widget*>(m_xImpl->m_xSharedListBox.get());
            break;

        case LISTBOX_TEMPLATE:
            pReturn =   bLabelControl
                    ? static_cast<weld::Widget*>(m_xImpl->m_xSharedLabel.get())
                    : static_cast<weld::Widget*>(m_xImpl->m_xSharedListBox.get());
            break;

        case LISTBOX_IMAGE_TEMPLATE:
            pReturn =   bLabelControl
                    ? static_cast<weld::Widget*>(m_xImpl->m_xSharedLabel.get())
                    : static_cast<weld::Widget*>(m_xImpl->m_xSharedListBox.get());
            break;

        case LISTBOX_IMAGE_ANCHOR:
            pReturn =   bLabelControl
                    ? static_cast<weld::Widget*>(m_xImpl->m_xSharedLabel.get())
                    : static_cast<weld::Widget*>(m_xImpl->m_xSharedListBox.get());
            break;

        case LISTBOX_VERSION_LABEL:
            pReturn = m_xImpl->m_xSharedLabel.get();
            break;

        case LISTBOX_TEMPLATE_LABEL:
            pReturn = m_xImpl->m_xSharedLabel.get();
            break;

        case LISTBOX_IMAGE_TEMPLATE_LABEL:
            pReturn = m_xImpl->m_xSharedLabel.get();
            break;

        case LISTBOX_IMAGE_ANCHOR_LABEL:
            pReturn = m_xImpl->m_xSharedLabel.get();
            break;

        case PUSHBUTTON_OK:
            pReturn = m_xImpl->m_xBtnFileOpen.get();
            break;

        case PUSHBUTTON_CANCEL:
            pReturn = m_xImpl->m_xBtnCancel.get();
            break;

        case PUSHBUTTON_PLAY:
            pReturn = m_xPbPlay.get();
            break;

        case PUSHBUTTON_HELP:
            pReturn = m_xImpl->m_xBtnHelp.get();
            break;

        case TOOLBOXBUTTON_LEVEL_UP:
            pReturn = m_xImpl->m_xBtnUp->getWidget();
            break;

        case TOOLBOXBUTTON_NEW_FOLDER:
            pReturn = m_xImpl->m_xBtnNewFolder.get();
            break;

        case LISTBOX_FILTER_SELECTOR:
            // only exists on SalGtkFilePicker
            break;

        default:
            SAL_WARN( "fpicker.office", "SvtFileDialog::getControl: invalid id!" );
    }
    return pReturn;
}

void SvtFileDialog::enableControl(sal_Int16 nControlId, bool bEnable)
{
    weld::Widget* pControl = getControl(nControlId);
    if (pControl)
        EnableControl(pControl, bEnable);
    weld::Widget* pLabel = getControl(nControlId, true);
    if (pLabel)
        EnableControl(pLabel, bEnable);
}

void SvtFileDialog::AddControls_Impl( )
{
    // create the "insert as link" checkbox, if needed
    if ( m_nPickerFlags & PickerFlags::InsertAsLink )
    {
        m_xCbLinkBox->set_label( FpsResId( STR_SVT_FILEPICKER_INSERT_AS_LINK ) );
        m_xCbLinkBox->set_help_id( HID_FILEDLG_LINK_CB );
        m_xCbLinkBox->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xCbLinkBox->show();
    }

    // create the "show preview" checkbox ( and the preview window, too ), if needed
    if ( m_nPickerFlags & PickerFlags::ShowPreview  )
    {
        m_xImpl->m_aIniKey = "ImportGraphicDialog";

        // "preview"
        m_xCbPreviewBox->set_label( FpsResId( STR_SVT_FILEPICKER_SHOW_PREVIEW ) );
        m_xCbPreviewBox->set_help_id( HID_FILEDLG_PREVIEW_CB );
        m_xCbPreviewBox->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xCbPreviewBox->show();

        // generate preview window just here
        m_aPreviewSize = Size(200, 300);
        m_xPrevBmp->set_size_request(m_aPreviewSize.Width(), m_aPreviewSize.Height());
        m_xPrevBmp->connect_size_allocate(LINK(this, SvtFileDialog, PreviewSizeAllocHdl));
        m_xPreviewFrame->show();
        m_xPrevBmp->set_accessible_name(FpsResId(STR_PREVIEW));
    }

    if ( m_nPickerFlags & PickerFlags::AutoExtension )
    {
        m_xImpl->m_xCbAutoExtension->set_label( FpsResId( STR_SVT_FILEPICKER_AUTO_EXTENSION ) );
        m_xImpl->m_xCbAutoExtension->set_active(true);
        m_xImpl->m_xCbAutoExtension->connect_toggled( LINK( this, SvtFileDialog, AutoExtensionHdl_Impl ) );
        m_xImpl->m_xCbAutoExtension->show();
    }

    if ( m_nPickerFlags & PickerFlags::FilterOptions )
    {
        m_xImpl->m_xCbOptions->set_label( FpsResId( STR_SVT_FILEPICKER_FILTER_OPTIONS ) );
        m_xImpl->m_xCbOptions->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xImpl->m_xCbOptions->show();
    }

    if ( m_nPickerFlags & PickerFlags::Selection )
    {
        m_xCbSelection->set_label( FpsResId( STR_SVT_FILEPICKER_SELECTION ) );
        m_xCbSelection->connect_toggled( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        m_xCbSelection->show();
    }

    if ( m_nPickerFlags & PickerFlags::PlayButton )
    {
        m_xPbPlay->set_label( FpsResId( STR_SVT_FILEPICKER_PLAY ) );
        m_xPbPlay->set_help_id( HID_FILESAVE_DOPLAY );
        m_xPbPlay->connect_clicked( LINK( this, SvtFileDialog, PlayButtonHdl_Impl ) );
        m_xPbPlay->show();
    }

    if ( m_nPickerFlags & PickerFlags::ShowVersions )
    {
        m_xImpl->m_xSharedLabel->set_label( FpsResId( STR_SVT_FILEPICKER_VERSION ) );
        m_xImpl->m_xSharedLabel->show();

        m_xImpl->m_xSharedListBox->set_help_id( HID_FILEOPEN_VERSION );
        m_xImpl->m_xSharedListBox->show();
    }
    else if ( m_nPickerFlags & PickerFlags::Templates )
    {
        m_xImpl->m_xSharedLabel->set_label( FpsResId( STR_SVT_FILEPICKER_TEMPLATES ) );
        m_xImpl->m_xSharedLabel->show();

        m_xImpl->m_xSharedListBox->set_help_id( HID_FILEOPEN_VERSION );
        m_xImpl->m_xSharedListBox->show();
        // This is strange. During the re-factoring during 96930, I discovered that this help id
        // is set in the "Templates mode". This was hidden in the previous implementation.
        // Shouldn't this be a more meaningful help id.
    }
    else if ( m_nPickerFlags & PickerFlags::ImageTemplate )
    {
        m_xImpl->m_xSharedLabel->set_label( FpsResId( STR_SVT_FILEPICKER_IMAGE_TEMPLATE ) );
        m_xImpl->m_xSharedLabel->show();

        m_xImpl->m_xSharedListBox->set_help_id( HID_FILEOPEN_IMAGE_TEMPLATE );
        m_xImpl->m_xSharedListBox->show();
    }
    else if ( m_nPickerFlags & PickerFlags::ImageAnchor )
    {
        m_xImpl->m_xSharedLabel->set_label( FpsResId( STR_SVT_FILEPICKER_IMAGE_ANCHOR ) );
        m_xImpl->m_xSharedLabel->show();

        m_xImpl->m_xSharedListBox->set_help_id( HID_FILEOPEN_IMAGE_ANCHOR );
        m_xImpl->m_xSharedListBox->show();
    }

    m_xImpl->m_xPlaces.reset(new PlacesListBox(m_xBuilder->weld_tree_view(u"places"_ustr),
                                               m_xBuilder->weld_button(u"add"_ustr),
                                               m_xBuilder->weld_button(u"del"_ustr),
                                               this));
    m_xImpl->m_xPlaces->set_help_id(u"SVT_HID_FILESAVE_PLACES_LISTBOX"_ustr);
    m_xImpl->m_xPlaces->SetAddHdl( LINK ( this, SvtFileDialog, AddPlacePressed_Hdl ) );
    m_xImpl->m_xPlaces->SetDelHdl( LINK ( this, SvtFileDialog, RemovePlacePressed_Hdl ) );

    initDefaultPlaces();
}

IMPL_LINK(SvtFileDialog, PreviewSizeAllocHdl, const Size&, rSize, void)
{
    m_aPreviewSize = rSize;
}

sal_Int32 SvtFileDialog::getAvailableWidth()
{
    if (m_xPrevBmp)
        return m_aPreviewSize.Width();
    else
        return 0;
}

sal_Int32 SvtFileDialog::getAvailableHeight()
{
    if (m_xPrevBmp)
        return m_aPreviewSize.Height();
    else
        return 0;
}

void SvtFileDialog::setImage(const Any& rImage)
{
    if (!m_xPrevBmp || !m_xPreviewFrame->get_visible())
        return;

    Sequence < sal_Int8 > aBmpSequence;

    if ( rImage >>= aBmpSequence )
    {
        BitmapEx        aBmp;
        SvMemoryStream  aData( aBmpSequence.getArray(),
                               aBmpSequence.getLength(),
                               StreamMode::READ );
        ReadDIBBitmapEx(aBmp, aData);

        m_xPrevBmp->set_image(Graphic(aBmp).GetXGraphic());
    }
    else
    {
        m_xPrevBmp->set_image(nullptr);
    }
}

OUString SvtFileDialog::getCurrentFileText( ) const
{
    OUString sReturn;
    if (m_xImpl && m_xImpl->m_xEdFileName)
        sReturn = m_xImpl->m_xEdFileName->get_active_text();
    return sReturn;
}

void SvtFileDialog::setCurrentFileText( const OUString& _rText, bool m_bSelectAll )
{
    if (m_xImpl && m_xImpl->m_xEdFileName)
    {
        m_xImpl->m_xEdFileName->set_entry_text( _rText );
        if ( m_bSelectAll )
            m_xImpl->m_xEdFileName->select_entry_region(0, -1);
    }
}

bool SvtFileDialog::isAutoExtensionEnabled() const
{
    return m_xImpl->m_xCbAutoExtension && m_xImpl->m_xCbAutoExtension->get_active();
}

bool SvtFileDialog::getShowState()
{
    if (m_xPreviewFrame)
        return m_xPreviewFrame->get_visible();
    else
        return false;
}

bool SvtFileDialog::ContentHasParentFolder( const OUString& rURL )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    return m_aContent.hasParentFolder( ) && m_aContent.isValid();
}

bool SvtFileDialog::ContentCanMakeFolder( const OUString& rURL )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    return m_aContent.canCreateFolder( ) && m_aContent.isValid();
}

bool SvtFileDialog::ContentGetTitle( const OUString& rURL, OUString& rTitle )
{
    m_aContent.bindTo( rURL );

    if ( m_aContent.isInvalid() )
        return false;

    OUString sTitle;
    m_aContent.getTitle( sTitle );
    rTitle = sTitle;

    return m_aContent.isValid();
}

void SvtFileDialog::appendDefaultExtension(OUString& rFileName,
                                           std::u16string_view rFilterDefaultExtension,
                                           const OUString& rFilterExtensions)
{
    const OUString aType(rFilterExtensions.toAsciiLowerCase());

    if ( aType == FILEDIALOG_FILTER_ALL )
        return;

    const OUString aTemp(rFileName.toAsciiLowerCase());
    sal_Int32 nPos = 0;

    do
    {
        if (nPos+1<aType.getLength() && aType[nPos]=='*') // take care of a leading *
            ++nPos;
        const std::u16string_view aExt(o3tl::getToken(aType, 0, FILEDIALOG_DEF_EXTSEP, nPos ));
        if (aExt.empty())
            continue;
        if (o3tl::ends_with(aTemp, aExt))
            return;
    }
    while (nPos>=0);

    rFileName += OUString::Concat(".") + rFilterDefaultExtension;
}

void SvtFileDialog::initDefaultPlaces( )
{
    PlacePtr pRootPlace = std::make_shared<Place>( FpsResId(STR_DEFAULT_DIRECTORY), GetStandardDir() );
    m_xImpl->m_xPlaces->AppendPlace( pRootPlace );

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get());
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get());

    for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
    {
        PlacePtr pPlace = std::make_shared<Place>(placesNamesList[nPlace], placesUrlsList[nPlace], true);
        m_xImpl->m_xPlaces->AppendPlace(pPlace);
    }

    // Reset the placesList "updated" state
    m_xImpl->m_xPlaces->IsUpdated();
}

QueryFolderNameDialog::QueryFolderNameDialog(weld::Window* _pParent,
    const OUString& rTitle, const OUString& rDefaultText)
    : GenericDialogController(_pParent, u"fps/ui/foldernamedialog.ui"_ustr, u"FolderNameDialog"_ustr)
    , m_xNameEdit(m_xBuilder->weld_entry(u"entry"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xDialog->set_title(rTitle);
    m_xNameEdit->set_text(rDefaultText);
    m_xNameEdit->select_region(0, -1);
    m_xOKBtn->connect_clicked(LINK(this, QueryFolderNameDialog, OKHdl));
    m_xNameEdit->connect_changed(LINK(this, QueryFolderNameDialog, NameHdl));
};

QueryFolderNameDialog::~QueryFolderNameDialog()
{
}

IMPL_LINK_NOARG(QueryFolderNameDialog, OKHdl, weld::Button&, void)
{
    // trim the strings
    m_xNameEdit->set_text(comphelper::string::strip(m_xNameEdit->get_text(), ' '));
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(QueryFolderNameDialog, NameHdl, weld::Entry&, void)
{
    // trim the strings
    OUString aName = comphelper::string::strip(m_xNameEdit->get_text(), ' ');
    m_xOKBtn->set_sensitive(!aName.isEmpty());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
