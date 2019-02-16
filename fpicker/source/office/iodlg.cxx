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

#include <sal/macros.h>
#include <sal/log.hxx>
#include "iodlg.hxx"
#include <svtools/PlaceEditDialog.hxx>
#include "PlacesListBox.hxx"
#include <fpsofficeResMgr.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <unotools/ucbhelper.hxx>
#include <svtools/ehdl.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/viewoptions.hxx>
#include <svtools/fileview.hxx>
#include <svtools/sfxecode.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/treelistentry.hxx>

#include <fpicker/strings.hrc>
#include <svtools/helpids.h>
#include <strings.hrc>
#include <bitmaps.hlst>
#include "asyncfilepicker.hxx"
#include "iodlgimp.hxx"
#include <svtools/inettbc.hxx>
#include <unotools/syslocale.hxx>
#include "QueryFolderName.hxx"
#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ControlActions.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <comphelper/interaction.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#include <osl/file.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/waitobj.hxx>
#include <vcl/settings.hxx>

#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/InteractiveAugmentedIOException.hpp>
#include "fpinteraction.hxx"
#include <osl/process.h>

#include <officecfg/Office/Common.hxx>

#include <algorithm>
#include <functional>
#include <vector>
#include <memory>

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::sdbc;
using namespace ::utl;
using namespace ::svt;

using namespace ExtendedFilePickerElementIds;
using namespace CommonFilePickerElementIds;
using namespace InternalFilePickerElementIds;

// Time to wait while traveling in the filterbox until
// the browsebox gets filtered ( in ms).
#define TRAVELFILTER_TIMEOUT    750

// functions -------------------------------------------------------------

namespace
{


    OUString getMostCurrentFilter( std::unique_ptr<SvtExpFileDlg_Impl> const & pImpl )
    {
        assert( pImpl && "invalid impl pointer" );
        const SvtFileDialogFilter_Impl* pFilter = pImpl->_pUserFilter.get();

        if ( !pFilter )
            pFilter = pImpl->GetCurFilter();

        if ( !pFilter )
            return OUString();

        return pFilter->GetType();
    }


    bool restoreCurrentFilter( std::unique_ptr<SvtExpFileDlg_Impl> const & pImpl )
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
        return pImpl->m_bNeedDelayedFilterExecute;
    }


    OUString GetFsysExtension_Impl( const OUString& rFile, const OUString& rLastFilterExt )
    {
        sal_Int32 nDotPos = rFile.lastIndexOf( '.' );
        if ( nDotPos != -1 )
        {
            if ( !rLastFilterExt.isEmpty() )
            {
                if ( rFile.copy( nDotPos + 1 ).equalsIgnoreAsciiCase( rLastFilterExt ) )
                    return rLastFilterExt;
            }
            else
                return rFile.copy( nDotPos );
        }
        return OUString();
    }


    void SetFsysExtension_Impl( OUString& rFile, const OUString& rExtension )
    {
        const sal_Unicode* p0 = rFile.getStr();
        const sal_Unicode* p1 = p0 + rFile.getLength() - 1;
        while ( p1 >= p0 && *p1 != '.' )
            p1--;
        if ( p1 >= p0 )
            // remove old extension
            rFile = rFile.copy( 0, p1 - p0 + 1 - ( rExtension.getLength() > 0 ? 0 : 1 ) );
        else if ( !rExtension.isEmpty() )
            // no old extension
            rFile += ".";
        rFile += rExtension;
    }

    void lcl_autoUpdateFileExtension( SvtFileDialog* _pDialog, const OUString& _rLastFilterExt )
    {
        // if auto extension is enabled ....
        if ( _pDialog->isAutoExtensionEnabled() )
        {
            // automatically switch to the extension of the (maybe just newly selected) extension
            OUString aNewFile = _pDialog->getCurrentFileText( );
            OUString aExt = GetFsysExtension_Impl( aNewFile, _rLastFilterExt );

            // but only if there already is an extension
            if ( !aExt.isEmpty() )
            {
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
                const OUString sHomeDirPropertyName( "HomeDirectory" );
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
            OSL_FAIL( "lcl_getHomeDirectory: caught an exception!" );
        }
        return !_rHomeDir.isEmpty();
    }
#endif

    OUString lcl_ensureFinalSlash( const OUString& _rDir )
    {
        INetURLObject aWorkPathObj( _rDir, INetProtocol::File );
        aWorkPathObj.setFinalSlash();
        return  aWorkPathObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }


    /** retrieves the value of an environment variable
        @return <TRUE/> if and only if the retrieved string value is not empty
    */
    bool getEnvironmentValue( const sal_Char* _pAsciiEnvName, OUString& _rValue )
    {
        _rValue.clear();
        OUString sEnvName = OUString::createFromAscii( _pAsciiEnvName );
        osl_getEnvironment( sEnvName.pData, &_rValue.pData );
        return !_rValue.isEmpty();
    }
}

// SvtFileDialog

SvtFileDialog::SvtFileDialog
(
    vcl::Window* _pParent,
    PickerFlags nBits
) :
    SvtFileDialog_Base( _pParent, "ExplorerFileDialog", "fps/ui/explorerfiledialog.ui" )

    ,_pCbReadOnly( nullptr )
    ,_pCbLinkBox( nullptr)
    ,_pCbPreviewBox( nullptr )
    ,_pCbSelection( nullptr )
    ,_pPbPlay( nullptr )
    ,_pPrevWin( nullptr )
    ,_pPrevBmp( nullptr )
    ,_pFileView( nullptr )
    ,_pFileNotifier( nullptr )
    ,pImpl( new SvtExpFileDlg_Impl )
    ,_nPickerFlags( nBits )
    ,_bIsInExecute( false )
    ,m_bInExecuteAsync( false )
    ,m_bHasFilename( false )
    ,m_context(comphelper::getProcessComponentContext())
{
    Init_Impl( nBits );
}


class CustomContainer : public vcl::Window
{
    enum FocusState
    {
        Prev = 0,
        Places,
        Add,
        Delete,
        FileView,
        Next,
        FocusCount
    };

    SvtExpFileDlg_Impl* pImpl;
    VclPtr<SvtFileView> _pFileView;
    VclPtr<Splitter>    _pSplitter;

    int m_nCurrentFocus;
    VclPtr<vcl::Window> m_pFocusWidgets[FocusState::FocusCount];

public:
    explicit CustomContainer(vcl::Window *pParent)
        : Window(pParent)
        , pImpl(nullptr)
        , _pFileView(nullptr)
        , _pSplitter(nullptr)
        , m_nCurrentFocus(FocusState::Prev)
    {
    }
    virtual ~CustomContainer() override { disposeOnce(); }
    virtual void dispose() override
    {
        _pFileView.clear();
        _pSplitter.clear();
        vcl::Window::dispose();
    }

    void init(SvtExpFileDlg_Impl* pImp,
              SvtFileView* pFileView,
              Splitter* pSplitter,
              vcl::Window* pPrev,
              vcl::Window* pNext)
    {
        pImpl      = pImp;
        _pFileView = pFileView;
        _pSplitter = pSplitter;

        m_pFocusWidgets[FocusState::Prev] = pPrev;
        m_pFocusWidgets[FocusState::Places] = pImpl->_pPlaces->GetPlacesListBox();
        m_pFocusWidgets[FocusState::Add] = pImpl->_pPlaces->GetAddButton();
        m_pFocusWidgets[FocusState::Delete] = pImpl->_pPlaces->GetDeleteButton();
        m_pFocusWidgets[FocusState::FileView] = pFileView;
        m_pFocusWidgets[FocusState::Next] = pNext;
    }

    virtual void Resize() override
    {
        Window::Resize();

        if (!pImpl || !pImpl->_pPlaces)
            return;

        Size aSize = GetSizePixel();

        Point aBoxPos(_pFileView->GetPosPixel());
        Size aNewSize(aSize.Width() - aBoxPos.X(), aSize.Height());
        _pFileView->SetSizePixel( aNewSize );

        // Resize the Splitter to fit the height
        Size splitterNewSize = _pSplitter->GetSizePixel( );
        splitterNewSize.setHeight( aSize.Height() );
        _pSplitter->SetSizePixel( splitterNewSize );
        sal_Int32 nMinX = pImpl->_pPlaces->GetPosPixel( ).X( );
        sal_Int32 nMaxX = _pFileView->GetPosPixel( ).X( ) + _pFileView->GetSizePixel( ).Width() - nMinX;
        _pSplitter->SetDragRectPixel( tools::Rectangle( Point( nMinX, 0 ), Size( nMaxX, aSize.Width() ) ) );

        // Resize the places list box to fit the height of the FileView
        Size placesNewSize(pImpl->_pPlaces->GetSizePixel());
        placesNewSize.setHeight( aSize.Height() );
        pImpl->_pPlaces->SetSizePixel( placesNewSize );
    }

    void changeFocus( bool bReverse )
    {
        if( !_pFileView || !pImpl || !pImpl->_pPlaces )
            return;

        if( bReverse && m_nCurrentFocus > FocusState::Prev && m_nCurrentFocus <= FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus(false);
            m_pFocusWidgets[m_nCurrentFocus]->LoseFocus();

            m_pFocusWidgets[--m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
        else if( !bReverse && m_nCurrentFocus >= FocusState::Prev && m_nCurrentFocus < FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus(false);
            m_pFocusWidgets[m_nCurrentFocus]->LoseFocus();

            m_pFocusWidgets[++m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
    }

    virtual void GetFocus() override
    {
        if( !_pFileView || !pImpl || !pImpl->_pPlaces )
            return;

        GetFocusFlags aFlags = GetGetFocusFlags();

        if( aFlags & GetFocusFlags::Forward )
            m_nCurrentFocus = FocusState::Places;
        else if( aFlags & GetFocusFlags::Backward )
            m_nCurrentFocus = FocusState::FileView;

        if( m_nCurrentFocus >= FocusState::Prev && m_nCurrentFocus <= FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
    }

    virtual bool EventNotify( NotifyEvent& rNEvt ) override
    {
        if( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            // we must also update counter when user change focus using mouse
            for(int i = FocusState::Prev; i <= FocusState::Next; i++)
            {
                if( rNEvt.GetWindow() == m_pFocusWidgets[i] )
                {
                    m_nCurrentFocus = i;
                    return true;
                }
            }

            // GETFOCUS for one of FileView's subcontrols
            m_nCurrentFocus = FocusState::FileView;
            return true;
        }
        if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();
            bool bShift = rCode.IsShift();
            if( rCode.GetCode() == KEY_TAB )
            {
                changeFocus( bShift );
                return true;
            }
        }
        return Window::EventNotify(rNEvt);
    }
};

SvtFileDialog::~SvtFileDialog()
{
    disposeOnce();
}

void SvtFileDialog::dispose()
{
    if ( !pImpl->_aIniKey.isEmpty() )
    {
        // save window state
        SvtViewOptions aDlgOpt( EViewType::Dialog, pImpl->_aIniKey );
        aDlgOpt.SetWindowState(OStringToOUString(GetWindowState(), osl_getThreadTextEncoding()));
        OUString sUserData = _pFileView->GetConfigString();
        aDlgOpt.SetUserItem( "UserData",
                             makeAny( sUserData ) );
    }

    _pFileView->SetSelectHdl( Link<SvTreeListBox*,void>() );

    // Save bookmarked places
    if(pImpl->_pPlaces->IsUpdated()) {
        const std::vector<PlacePtr> aPlaces = pImpl->_pPlaces->GetPlaces();
        Sequence< OUString > placesUrlsList(pImpl->_pPlaces->GetNbEditablePlaces());
        Sequence< OUString > placesNamesList(pImpl->_pPlaces->GetNbEditablePlaces());
        int i(0);
        for (auto const& place : aPlaces)
        {
            if(place->IsEditable()) {
                placesUrlsList[i] = place->GetUrl();
                placesNamesList[i] = place->GetName();
                ++i;
            }
        }

        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch);
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch);
        batch->commit();
    }

    pImpl.reset();
    _pFileView.disposeAndClear();
    _pSplitter.disposeAndClear();
    _pContainer.disposeAndClear();
    _pPrevBmp.disposeAndClear();
    _pCbReadOnly.clear();
    _pCbLinkBox.clear();
    _pCbPreviewBox.clear();
    _pCbSelection.clear();
    _pPbPlay.clear();
    _pPrevWin.clear();
    m_aDisabledControls.clear();
    ModalDialog::dispose();
}

void SvtFileDialog::Init_Impl
(
    PickerFlags nStyle
)
{
    get(_pCbReadOnly, "readonly");
    get(_pCbLinkBox, "link");
    get(_pCbPreviewBox, "cb_preview");
    get(_pCbSelection, "selection");
    get(_pPrevWin, "preview");
    get(_pPbPlay, "play");
    get(pImpl->_pCbOptions, "options");
    get(pImpl->_pFtFileName, "file_name_label");
    get(pImpl->_pEdFileName, "file_name");
    pImpl->_pEdFileName->GetFocus();
    get(pImpl->_pFtFileType, "file_type_label");
    get(pImpl->_pLbFilter, "file_type");
    get(pImpl->_pEdCurrentPath, "current_path");
    get(pImpl->_pBtnFileOpen, "open");
    get(pImpl->_pBtnCancel, "cancel");
    get(pImpl->_pBtnHelp, "help");
    get(pImpl->_pBtnConnectToServer, "connect_to_server");
    get(pImpl->_pBtnNewFolder, "new_folder");
    get(pImpl->_pCbPassword, "password");
    get(pImpl->_pCbGPGEncrypt, "gpgencrypt");
    get(pImpl->_pCbAutoExtension, "extension");
    get(pImpl->_pFtFileVersion, "shared_label");
    get(pImpl->_pLbFileVersion, "shared");
    get(pImpl->_pFtTemplates, "shared_label");
    get(pImpl->_pLbTemplates, "shared");
    get(pImpl->_pFtImageTemplates, "shared_label");
    get(pImpl->_pLbImageTemplates, "shared");
    get(pImpl->_pFtImageAnchor, "shared_label");
    get(pImpl->_pLbImageAnchor, "shared");

    pImpl->_pLbImageTemplates->setMaxWidthChars(40);
    pImpl->_pLbFilter->setMaxWidthChars(40);

    vcl::Window *pUpContainer = get<vcl::Window>("up");
    pImpl->_pBtnUp = VclPtr<SvtUpButton_Impl>::Create(pUpContainer, this, 0);
    pImpl->_pBtnUp->SetHelpId( HID_FILEOPEN_LEVELUP );
    pImpl->_pBtnUp->set_vexpand(true);
    pImpl->_pBtnUp->Show();

    pImpl->_nStyle = nStyle;
    pImpl->_eMode = ( nStyle & PickerFlags::SaveAs ) ? FILEDLG_MODE_SAVE : FILEDLG_MODE_OPEN;
    pImpl->_eDlgType = FILEDLG_TYPE_FILEDLG;

    if ( nStyle & PickerFlags::PathDialog )
        pImpl->_eDlgType = FILEDLG_TYPE_PATHDLG;

    // Set the directory for the "back to the default dir" button
    INetURLObject aStdDirObj( SvtPathOptions().GetWorkPath() );
    SetStandardDir( aStdDirObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

    // Create control element, the order defines the tab control.
    pImpl->_pEdFileName->SetSelectHdl( LINK( this, SvtFileDialog, EntrySelectHdl_Impl ) );
    pImpl->_pEdFileName->SetOpenHdl( LINK( this, SvtFileDialog, OpenUrlHdl_Impl ) );

    // in folder picker mode, only auto-complete directories (no files)
    bool bIsFolderPicker = ( pImpl->_eDlgType == FILEDLG_TYPE_PATHDLG );
    pImpl->_pEdFileName->SetOnlyDirectories( bIsFolderPicker );

    // in save mode, don't use the autocompletion as selection in the edit part
    bool bSaveMode = ( FILEDLG_MODE_SAVE == pImpl->_eMode );
    pImpl->_pEdFileName->SetNoURLSelection( bSaveMode );

    pImpl->_pBtnUp->SetAccessibleName( pImpl->_pBtnUp->GetQuickHelpText() );

    if ( nStyle & PickerFlags::MultiSelection )
        pImpl->_bMultiSelection = true;

    _pContainer.reset(VclPtr<CustomContainer>::Create(get<vcl::Window>("container")));
    Size aSize(LogicToPixel(Size(270, 85), MapMode(MapUnit::MapAppFont)));
    _pContainer->set_height_request(aSize.Height());
    _pContainer->set_width_request(aSize.Width());
    _pContainer->set_hexpand(true);
    _pContainer->set_vexpand(true);
    _pContainer->SetStyle( _pContainer->GetStyle() | WB_TABSTOP );

    _pFileView = VclPtr<SvtFileView>::Create( _pContainer, WB_BORDER,
                                       FILEDLG_TYPE_PATHDLG == pImpl->_eDlgType,
                                       pImpl->_bMultiSelection );
    _pFileView->Show();
    _pFileView->EnableAutoResize();
    _pFileView->SetHelpId( HID_FILEDLG_STANDARD );
    _pFileView->SetStyle( _pFileView->GetStyle() | WB_TABSTOP );

    _pSplitter = VclPtr<Splitter>::Create( _pContainer, WB_HSCROLL );
    _pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    _pSplitter->SetSplitHdl( LINK( this, SvtFileDialog, Split_Hdl ) );

    Image aNewFolderImg( GetButtonImage( BMP_FILEDLG_CREATEFOLDER ) );
    pImpl->_pBtnNewFolder->SetModeImage( aNewFolderImg );

    if ( nStyle & PickerFlags::ReadOnly )
    {
        _pCbReadOnly->SetHelpId( HID_FILEOPEN_READONLY );
        _pCbReadOnly->SetText( FpsResId( STR_SVT_FILEPICKER_READONLY ) );
        _pCbReadOnly->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbReadOnly->Show();
    }

    if ( nStyle & PickerFlags::Password )
    {
        pImpl->_pCbPassword->SetText( FpsResId( STR_SVT_FILEPICKER_PASSWORD ) );
        pImpl->_pCbPassword->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        pImpl->_pCbPassword->Show();
        pImpl->_pCbGPGEncrypt->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        pImpl->_pCbGPGEncrypt->Show();
    }

    // set the ini file for extracting the size
    pImpl->_aIniKey = "FileDialog";

    AddControls_Impl( );

    // adjust the labels to the mode
    const char* pResId = STR_EXPLORERFILE_OPEN;
    const char* pButtonResId = nullptr;

    if ( nStyle & PickerFlags::SaveAs )
    {
        pResId = STR_EXPLORERFILE_SAVE;
        pButtonResId = STR_EXPLORERFILE_BUTTONSAVE;
    }

    if ( nStyle & PickerFlags::PathDialog )
    {
        pImpl->_pFtFileName->SetText( FpsResId( STR_PATHNAME ) );
        pResId = STR_PATHSELECT;
        pButtonResId = STR_BUTTONSELECT;
    }

    SetText( FpsResId( pResId ) );

    if ( pButtonResId )
        pImpl->_pBtnFileOpen->SetText( FpsResId( pButtonResId ) );

    if ( FILEDLG_TYPE_FILEDLG != pImpl->_eDlgType )
    {
        pImpl->_pFtFileType->Hide();
        pImpl->GetFilterListControl()->Hide();
    }

    // Setting preferences of the control elements.
    pImpl->_pBtnNewFolder->SetClickHdl( LINK( this, SvtFileDialog, NewFolderHdl_Impl ) );
    pImpl->_pBtnFileOpen->SetClickHdl( LINK( this, SvtFileDialog, OpenClickHdl_Impl ) );
    pImpl->_pBtnCancel->SetClickHdl( LINK( this, SvtFileDialog, CancelHdl_Impl ) );
    pImpl->SetFilterListSelectHdl( LINK( this, SvtFileDialog, FilterSelectHdl_Impl ) );
    pImpl->_pEdFileName->SetGetFocusHdl( LINK( this, SvtFileDialog, FileNameGetFocusHdl_Impl ) );
    pImpl->_pEdFileName->SetModifyHdl( LINK( this, SvtFileDialog, FileNameModifiedHdl_Impl ) );
    pImpl->_pEdCurrentPath->SetOpenHdl ( LINK( this, SvtFileDialog, URLBoxModifiedHdl_Impl ) );
    pImpl->_pBtnConnectToServer->SetClickHdl( LINK ( this, SvtFileDialog, ConnectToServerPressed_Hdl ) );


    _pFileView->SetSelectHdl( LINK( this, SvtFileDialog, SelectHdl_Impl ) );
    _pFileView->SetDoubleClickHdl( LINK( this, SvtFileDialog, DblClickHdl_Impl ) );
    _pFileView->SetOpenDoneHdl( LINK( this, SvtFileDialog, OpenDoneHdl_Impl ) );


    // set timer for the filterbox travel
    pImpl->_aFilterTimer.SetTimeout( TRAVELFILTER_TIMEOUT );
    pImpl->_aFilterTimer.SetInvokeHandler( LINK( this, SvtFileDialog, FilterSelectTimerHdl_Impl ) );

    if ( PickerFlags::SaveAs & nStyle )
    {
        // different help ids if in save-as mode
        SetHelpId( HID_FILESAVE_DIALOG );

        pImpl->_pEdFileName->SetHelpId( HID_FILESAVE_FILEURL );
        pImpl->_pBtnFileOpen->SetHelpId( HID_FILESAVE_DOSAVE );
        pImpl->_pBtnNewFolder->SetHelpId( HID_FILESAVE_CREATEDIRECTORY );
        pImpl->_pBtnUp->SetHelpId( HID_FILESAVE_LEVELUP );
        pImpl->GetFilterListControl()->SetHelpId( HID_FILESAVE_FILETYPE );
        _pFileView->SetHelpId( HID_FILESAVE_FILEVIEW );

        // formerly, there was only _pLbFileVersion, which was used for 3 different
        // use cases. For reasons of maintainability, I introduced extra members (_pLbTemplates, _pLbImageTemplates)
        // for the extra use cases, and separated _pLbFileVersion
        // I did not find out in which cases the help ID is really needed HID_FILESAVE_TEMPLATE - all
        // tests I made lead to a dialog where _no_ of the three list boxes was present.
        if ( pImpl->_pLbFileVersion )
            pImpl->_pLbFileVersion->SetHelpId( HID_FILESAVE_TEMPLATE );
        if ( pImpl->_pLbTemplates )
            pImpl->_pLbTemplates->SetHelpId( HID_FILESAVE_TEMPLATE );
        if ( pImpl->_pLbImageTemplates )
            pImpl->_pLbImageTemplates->SetHelpId( HID_FILESAVE_TEMPLATE );

        if ( pImpl->_pCbPassword ) pImpl->_pCbPassword->SetHelpId( HID_FILESAVE_SAVEWITHPASSWORD );
        if ( pImpl->_pCbAutoExtension ) pImpl->_pCbAutoExtension->SetHelpId( HID_FILESAVE_AUTOEXTENSION );
        if ( pImpl->_pCbOptions ) pImpl->_pCbOptions->SetHelpId( HID_FILESAVE_CUSTOMIZEFILTER );
        if ( _pCbSelection ) _pCbSelection->SetHelpId( HID_FILESAVE_SELECTION );
    }

    /// read our settings from the configuration
    m_aConfiguration = OConfigurationTreeRoot::createWithComponentContext(
        ::comphelper::getProcessComponentContext(),
        "/org.openoffice.Office.UI/FilePicker"
    );

    _pContainer->init(pImpl.get(), _pFileView, _pSplitter, pImpl->_pBtnNewFolder, pImpl->_pEdFileName);
    _pContainer->Show();

    Resize();
}

IMPL_LINK_NOARG( SvtFileDialog, NewFolderHdl_Impl, Button*, void)
{
    _pFileView->EndInplaceEditing();

    SmartContent aContent( _pFileView->GetViewURL( ) );
    OUString aTitle;
    aContent.getTitle( aTitle );
    QueryFolderNameDialog aDlg(GetFrameWeld(), aTitle, FpsResId(STR_SVT_NEW_FOLDER));
    bool bHandled = false;

    while ( !bHandled )
    {
        if (aDlg.run() == RET_OK)
        {
            OUString aUrl = aContent.createFolder(aDlg.GetName());
            if ( !aUrl.isEmpty( ) )
            {
                _pFileView->CreatedFolder(aUrl, aDlg.GetName());
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
    pImpl->_pUserFilter.reset( new SvtFileDialogFilter_Impl( _rNewFilter, _rNewFilter ) );

    // remember the extension
    bool bIsAllFiles = _rNewFilter == FILEDIALOG_FILTER_ALL;
    if ( bIsAllFiles )
        EraseDefaultExt();
    else
        SetDefaultExt( _rNewFilter.copy( 2 ) );
        // TODO: this is nonsense. In the whole file there are a lot of places where we assume that a user filter
        // is always "*.<something>". But changing this would take some more time than I have now...

    // now, the default extension is set to the one of the user filter (or empty)
    if ( pImpl->GetCurFilter( ) )
        SetDefaultExt( pImpl->GetCurFilter( )->GetExtension() );
    else
        EraseDefaultExt();
}


AdjustFilterFlags SvtFileDialog::adjustFilter( const OUString& _rFilter )
{
    AdjustFilterFlags nReturn = AdjustFilterFlags::NONE;

    const bool bNonEmpty = !_rFilter.isEmpty();
    if ( bNonEmpty )
    {
        nReturn |= AdjustFilterFlags::NonEmpty;

        bool bFilterChanged = true;

        // search for a corresponding filter
        SvtFileDialogFilter_Impl* pFilter = FindFilter_Impl( _rFilter, false, bFilterChanged );

        // look for multi-ext filters if necessary
        if ( !pFilter )
            pFilter = FindFilter_Impl( _rFilter, true, bFilterChanged );

        if ( bFilterChanged )
            nReturn |= AdjustFilterFlags::Changed;

        if ( !pFilter )
        {
            nReturn |= AdjustFilterFlags::UserFilter;
            // no filter found : use it as user defined filter
            createNewUserFilter( _rFilter );
        }
    }

    return nReturn;
}


IMPL_LINK_NOARG(SvtFileDialog, CancelHdl_Impl, Button*, void)
{
    if ( m_pCurrentAsyncAction.is() )
    {
        m_pCurrentAsyncAction->cancel();
        onAsyncOperationFinished();
    }
    else
    {
        EndDialog();
    }
}


IMPL_LINK( SvtFileDialog, OpenClickHdl_Impl, Button*, pVoid, void )
{
    OpenHdl_Impl(pVoid);
}
IMPL_LINK( SvtFileDialog, OpenUrlHdl_Impl, SvtURLBox*, pVoid, void )
{
    OpenHdl_Impl(pVoid);
}
void SvtFileDialog::OpenHdl_Impl(void const * pVoid)
{
    if ( pImpl->_bMultiSelection && _pFileView->GetSelectionCount() > 1 )
    {
        // special open in case of multiselection
        OpenMultiSelection_Impl();
        return;
    }

    OUString aFileName;
    OUString aOldPath( _pFileView->GetViewURL() );
    if ( pImpl->_bDoubleClick || _pFileView->HasChildPathFocus() )
        // Selection done by doubleclicking in the view, get filename from the view
        aFileName = _pFileView->GetCurrentURL();

    if ( aFileName.isEmpty() )
    {
        // if an entry is selected in the view ....
        if ( _pFileView->GetSelectionCount() )
        {   // -> use this one. This will allow us to step down this folder
            aFileName = _pFileView->GetCurrentURL();
        }
    }

    if ( aFileName.isEmpty() )
    {
        if ( pImpl->_eMode == FILEDLG_MODE_OPEN && pImpl->_pEdFileName->IsTravelSelect() )
            // OpenHdl called from URLBox; travelling through the list of URLs should not cause an opening
            return;                   // MBA->PB: seems to be called never ?!

        // get the URL from the edit field ( if not empty )
        if ( !pImpl->_pEdFileName->GetText().isEmpty() )
        {
            OUString aText = pImpl->_pEdFileName->GetText();

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
                if ( lcl_getHomeDirectory( _pFileView->GetViewURL(), aFileName ) )
                    // in case we got a home dir, reset the text of the edit
                    pImpl->_pEdFileName->SetText( OUString() );
            }
            if ( aFileName.isEmpty() )
#endif
            {
                // get url from autocomplete edit
                aFileName = pImpl->_pEdFileName->GetURL();
            }
        }
        else if ( pVoid == pImpl->_pBtnFileOpen.get() )
            // OpenHdl was called for the "Open" Button; if edit field is empty, use selected element in the view
            aFileName = _pFileView->GetCurrentURL();
    }

    // MBA->PB: ?!
    if ( aFileName.isEmpty() && pVoid == pImpl->_pEdFileName && pImpl->_pUserFilter )
    {
        pImpl->_pUserFilter.reset();
        return;
    }

    sal_Int32 nLen = aFileName.getLength();
    if ( !nLen )
    {
        // if the dialog was opened to select a folder, the last selected folder should be selected
        if( pImpl->_eDlgType == FILEDLG_TYPE_PATHDLG )
        {
            aFileName = pImpl->_pEdCurrentPath->GetText();
            nLen = aFileName.getLength();
        }
        else
            // no file selected !
            return;
    }

    // mark input as selected
    pImpl->_pEdFileName->SetSelection( Selection( 0, nLen ) );

    // if a path with wildcards is given, divide the string into path and wildcards
    OUString aFilter;
    if ( !SvtFileDialog::IsolateFilterFromPath_Impl( aFileName, aFilter ) )
        return;

    // if a filter was retrieved, there were wildcards !
    AdjustFilterFlags nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & AdjustFilterFlags::Changed )
    {
        // cut off all text before wildcard in edit and select wildcard
        pImpl->_pEdFileName->SetText( aFilter );
        pImpl->_pEdFileName->SetSelection( Selection( 0, aFilter.getLength() ) );
    }

    {
        INetURLObject aFileObject( aFileName );
        if ( ( aFileObject.GetProtocol() == INetProtocol::NotValid ) && !aFileName.isEmpty() )
        {
            OUString sCompleted = SvtURLBox::ParseSmart( aFileName, _pFileView->GetViewURL() );
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
             ( pImpl->_eMode == FILEDLG_MODE_OPEN ) )
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

    if  (   !bIsFolder                                      // no existent folder
        &&  pImpl->_pCbAutoExtension                 // auto extension is enabled in general
        &&  pImpl->_pCbAutoExtension->IsChecked()    // auto extension is really to be used
        &&  !GetDefaultExt().isEmpty()               // there is a default extension
        &&  GetDefaultExt() != "*" // the default extension is not "all"
        && !(   FILEDLG_MODE_SAVE == pImpl->_eMode       // we're saving a file
            &&  _pFileView->GetSelectionCount()          // there is a selected file in the file view -> it will later on
            )                                                   //    (in SvtFileDialog::GetPathList) be taken as file to save to

        && FILEDLG_MODE_OPEN != pImpl->_eMode // #i83408# don't append extension on open
        )
    {
        // check extension and append the default extension if necessary
        appendDefaultExtension(aFileName,
                               GetDefaultExt(),
                               pImpl->GetCurFilter()->GetType());
    }

    bool bOpenFolder = ( FILEDLG_TYPE_PATHDLG == pImpl->_eDlgType ) &&
                       !pImpl->_bDoubleClick && pVoid != pImpl->_pEdFileName;
    if ( bIsFolder )
    {
        if ( bOpenFolder )
        {
            _aPath = aFileName;
        }
        else
        {
            if ( aFileName != _pFileView->GetViewURL() )
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
        _aPath = aFileName;
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

    switch ( pImpl->_eMode )
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
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
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

                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
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

    EndDialog( RET_OK );
}


void SvtFileDialog::EnableAutocompletion( bool _bEnable )
{
    pImpl->_pEdFileName->EnableAutocompletion( _bEnable );
}


IMPL_LINK_NOARG( SvtFileDialog, FilterSelectHdl_Impl, ListBox&, void )
{
    OUString sSelectedFilterDisplayName;
    SvtFileDialogFilter_Impl* pSelectedFilter = pImpl->GetSelectedFilterEntry( sSelectedFilterDisplayName );
    if ( !pSelectedFilter )
    {   // there is no current selection. This happens if for instance the user selects a group separator using
        // the keyboard, and then presses enter: When the selection happens, we immediately deselect the entry,
        // so in this situation there is no current selection.
        if ( restoreCurrentFilter( pImpl ) )
            ExecuteFilter();
    }
    else
    {
        if ( pSelectedFilter->isGroupSeparator() )
        {   // group separators can't be selected
            // return to the previously selected entry
            if ( pImpl->IsFilterListTravelSelect() )
            {
                pImpl->SetNoFilterListSelection( );

                // stop the timer for executing the filter
                if ( pImpl->_aFilterTimer.IsActive() )
                    pImpl->m_bNeedDelayedFilterExecute = true;
                pImpl->_aFilterTimer.Stop();
            }
            else
            {
                if ( restoreCurrentFilter( pImpl ) )
                    ExecuteFilter();
            }
        }
        else if (   ( pSelectedFilter != pImpl->GetCurFilter() )
                ||  pImpl->_pUserFilter
                )
        {
            // Store the old filter for the auto extension handling
            OUString sLastFilterExt = pImpl->GetCurFilter()->GetExtension();
            pImpl->_pUserFilter.reset();

            // if applicable remove filter of the user
            pImpl->SetCurFilter( pSelectedFilter, sSelectedFilterDisplayName );

            // if applicable show extension
            SetDefaultExt( pSelectedFilter->GetExtension() );
            sal_Int32 nSepPos = GetDefaultExt().indexOf( FILEDIALOG_DEF_EXTSEP );

            if ( nSepPos != -1 )
                EraseDefaultExt( nSepPos );

            // update the extension of the current file if necessary
            lcl_autoUpdateFileExtension( this, sLastFilterExt );

            // if the user is traveling fast through the filterbox
            // do not filter instantly
            if ( pImpl->IsFilterListTravelSelect() )
            {
                // FilterSelectHdl_Impl should be started again in
                // TRAVELFILTER_TIMEOUT ms
                pImpl->_aFilterTimer.Start();
            }
            else
            {
                // stop previously started timer
                pImpl->_aFilterTimer.Stop();

                // filter the view again
                ExecuteFilter();
            }
        }
    }
}

IMPL_LINK_NOARG(SvtFileDialog, FilterSelectTimerHdl_Impl, Timer*, void)
{
    // filter the view again
    ExecuteFilter();
}

IMPL_LINK_NOARG( SvtFileDialog, FileNameGetFocusHdl_Impl, Control&, void )
{
    _pFileView->SetNoSelection();
    _pFileView->Update();
}


IMPL_LINK_NOARG( SvtFileDialog, FileNameModifiedHdl_Impl, Edit&, void )
{
    FileNameGetFocusHdl_Impl( *pImpl->_pEdFileName );
}


IMPL_LINK_NOARG( SvtFileDialog, URLBoxModifiedHdl_Impl, SvtURLBox*, void )
{
    OUString aPath = pImpl->_pEdCurrentPath->GetURL();
    OpenURL_Impl(aPath);
}


IMPL_LINK_NOARG( SvtFileDialog, ConnectToServerPressed_Hdl, Button*, void )
{
    _pFileView->EndInplaceEditing();

    PlaceEditDialog aDlg(GetFrameWeld());
    short aRetCode = aDlg.run();

    switch (aRetCode) {
        case RET_OK :
        {
            PlacePtr newPlace = aDlg.GetPlace();
            pImpl->_pPlaces->AppendPlace(newPlace);

            break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };
}


IMPL_LINK_NOARG ( SvtFileDialog, AddPlacePressed_Hdl, Button*, void )
{
    // Maybe open the PlacesDialog would have been a better idea
    // there is an ux choice to make we did not make...
    INetURLObject aURLObj( _pFileView->GetViewURL() );
    PlacePtr newPlace(
        new Place( aURLObj.GetLastName(INetURLObject::DecodeMechanism::WithCharset),
                _pFileView->GetViewURL(), true));
    pImpl->_pPlaces->AppendPlace(newPlace);
}


IMPL_LINK_NOARG ( SvtFileDialog, RemovePlacePressed_Hdl, Button*, void )
{
    pImpl->_pPlaces->RemoveSelectedPlace();
}


SvtFileDialogFilter_Impl* SvtFileDialog::FindFilter_Impl
(
    const OUString& _rFilter,
    bool _bMultiExt,/*  TRUE - regard filter with several extensions
                            FALSE - do not ...
                        */
    bool& _rFilterChanged
)

/*  [Description]

    This method looks for the specified extension in the included filters.
*/

{
    SvtFileDialogFilter_Impl* pFoundFilter = nullptr;
    SvtFileDialogFilterList_Impl& rList = pImpl->m_aFilter;
    sal_uInt16 nFilter = rList.size();

    while ( nFilter-- )
    {
        SvtFileDialogFilter_Impl* pFilter = rList[ nFilter ].get();
        const OUString& rType = pFilter->GetType();
        OUString aSingleType = rType;

        if ( _bMultiExt )
        {
            sal_Int32 nIdx = 0;
            while ( !pFoundFilter && nIdx != -1 )
            {
                aSingleType = rType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nIdx );
#ifdef UNX
                if ( aSingleType == _rFilter )
#else
                if ( aSingleType.equalsIgnoreAsciiCase( _rFilter ) )
#endif
                    pFoundFilter = pFilter;
            }
        }
#ifdef UNX
        else if ( rType == _rFilter )
#else
        else if ( rType.equalsIgnoreAsciiCase( _rFilter ) )
#endif
            pFoundFilter = pFilter;

        if ( pFoundFilter )
        {
            // activate filter
            _rFilterChanged = pImpl->_pUserFilter || ( pImpl->GetCurFilter() != pFilter );

            createNewUserFilter( _rFilter );

            break;
        }
    }
    return pFoundFilter;
}


void SvtFileDialog::ExecuteFilter()
{
    pImpl->m_bNeedDelayedFilterExecute = false;
    executeAsync( AsyncPickerAction::eExecuteFilter, OUString(), getMostCurrentFilter( pImpl ) );
}


void SvtFileDialog::OpenMultiSelection_Impl()

/*  [Description]

    OpenHandler for MultiSelection
*/

{
    sal_uLong nCount = _pFileView->GetSelectionCount();
    SvTreeListEntry* pEntry = nCount ? _pFileView->FirstSelected() : nullptr;

    if ( nCount && pEntry )
        _aPath = SvtFileView::GetURL( pEntry );

    EndDialog( RET_OK );
}


void SvtFileDialog::UpdateControls( const OUString& rURL )
{
    pImpl->_pEdFileName->SetBaseURL( rURL );

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
        if ( FILEDLG_TYPE_PATHDLG == pImpl->_eDlgType )
            // -> set new path in the edit field
            pImpl->_pEdFileName->SetText( sText );

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
        pImpl->_pEdCurrentPath->SetText( sText );
    }


    _aPath = rURL;
    if ( _pFileNotifier )
        _pFileNotifier->notify( DIRECTORY_CHANGED, 0 );
}


IMPL_LINK( SvtFileDialog, SelectHdl_Impl, SvTreeListBox*, pBox, void )
{
    SvTreeListEntry* pEntry = pBox->FirstSelected();
    assert( pEntry && "SelectHandler without selected entry" );
    SvtContentEntry* pUserData = static_cast<SvtContentEntry*>(pEntry->GetUserData());

    if ( pUserData )
    {
        INetURLObject aObj( pUserData->maURL );
        if ( FILEDLG_TYPE_PATHDLG == pImpl->_eDlgType )
        {
            if ( aObj.GetProtocol() == INetProtocol::File )
            {
                if ( !pUserData->mbIsFolder )
                    aObj.removeSegment();
                OUString aName = aObj.getFSysPath( static_cast<FSysStyle>(FSysStyle::Detect & ~FSysStyle::Vos) );
                pImpl->_pEdFileName->SetText( aName );
                pImpl->_pEdFileName->SetSelection( Selection( 0, aName.getLength() ) );
                _aPath = pUserData->maURL;
            }
            else if ( !pUserData->mbIsFolder )
            {
                pImpl->_pEdFileName->SetText( pUserData->maURL );
                pImpl->_pEdFileName->SetSelection( Selection( 0, pUserData->maURL.getLength() ) );
                _aPath = pUserData->maURL;
            }
            else
                pImpl->_pEdFileName->SetText( OUString() );
        }
        else
        {
            if ( !pUserData->mbIsFolder )
            {
                OUString aName = SvTabListBox::GetEntryText( pEntry, 0 );
                pImpl->_pEdFileName->SetText( aName );
                pImpl->_pEdFileName->SetSelection( Selection( 0, aName.getLength() ) );
                _aPath = pUserData->maURL;
            }
        }
    }

    if ( pImpl->_bMultiSelection && _pFileView->GetSelectionCount() > 1 )
    {
        // clear the file edit for multiselection
        pImpl->_pEdFileName->SetText( OUString() );
    }

    FileSelect();
}


IMPL_LINK_NOARG(SvtFileDialog, DblClickHdl_Impl, SvTreeListBox*, bool)
{
    pImpl->_bDoubleClick = true;
    OpenHdl_Impl( nullptr );
    pImpl->_bDoubleClick = false;

    return false;
}


IMPL_LINK_NOARG(SvtFileDialog, EntrySelectHdl_Impl, ComboBox&, void)
{
    FileSelect();
}


IMPL_LINK( SvtFileDialog, OpenDoneHdl_Impl, SvtFileView*, pView, void )
{
    const OUString& sCurrentFolder( pView->GetViewURL() );
    // check if we can create new folders
    EnableControl( pImpl->_pBtnNewFolder, ContentCanMakeFolder( sCurrentFolder ) );

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
    EnableControl( pImpl->_pBtnUp, bCanTravelUp );
}


IMPL_LINK_NOARG(SvtFileDialog, AutoExtensionHdl_Impl, Button*, void)
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                CHECKBOX_AUTOEXTENSION );

    // update the extension of the current file if necessary
    lcl_autoUpdateFileExtension( this, pImpl->GetCurFilter()->GetExtension() );
}


IMPL_LINK( SvtFileDialog, ClickHdl_Impl, Button*, pCheckBox, void )
{
    if ( ! _pFileNotifier )
        return;

    sal_Int16 nId = -1;

    if ( pCheckBox == pImpl->_pCbOptions )
        nId = CHECKBOX_FILTEROPTIONS;
    else if ( pCheckBox == _pCbSelection )
        nId = CHECKBOX_SELECTION;
    else if ( pCheckBox == _pCbReadOnly )
        nId = CHECKBOX_READONLY;
    else if ( pCheckBox == pImpl->_pCbPassword )
        nId = CHECKBOX_PASSWORD;
    else if ( pCheckBox == pImpl->_pCbGPGEncrypt )
        nId = CHECKBOX_GPGENCRYPTION;
    else if ( pCheckBox == _pCbLinkBox )
        nId = CHECKBOX_LINK;
    else if ( pCheckBox == _pCbPreviewBox )
        nId = CHECKBOX_PREVIEW;

    if ( nId != -1 )
        _pFileNotifier->notify( CTRL_STATE_CHANGED, nId );
}


IMPL_LINK_NOARG(SvtFileDialog, PlayButtonHdl_Impl, Button*, void)
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                PUSHBUTTON_PLAY );
}


bool SvtFileDialog::EventNotify( NotifyEvent& rNEvt )

/*  [Description]

    This method gets called to catch <BACKSPACE>.
*/

{
    MouseNotifyEvent nType = rNEvt.GetType();
    bool bRet = false;

    if ( MouseNotifyEvent::KEYINPUT == nType && rNEvt.GetKeyEvent() )
    {
        const vcl::KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        sal_uInt16 nCode = rKeyCode.GetCode();

        if ( !rKeyCode.GetModifier() &&
             KEY_BACKSPACE == nCode && !pImpl->_pEdFileName->HasChildPathFocus() )
        {
            bRet = false;

            if ( !bRet && pImpl->_pBtnUp->IsEnabled() )
            {
                PrevLevel_Impl();
                bRet = true;
            }
        }
    }
    return bRet || ModalDialog::EventNotify(rNEvt);
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


OUString SvtFileDialog::implGetInitialURL( const OUString& _rPath, const OUString& _rFallback )
{
    // an URL parser for the fallback
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


short SvtFileDialog::Execute()
{
    if ( !PrepareExecute() )
        return 0;

    // start the dialog
    _bIsInExecute = true;
    short nResult = ModalDialog::Execute();
    _bIsInExecute = false;

    SAL_WARN_IF( m_pCurrentAsyncAction.is(), "fpicker.office", "SvtFilePicker::Execute: still running an async action!" );
        // the dialog should not be cancellable while an async action is running - first, the action
        // needs to be cancelled

    // remember last directory
    if ( RET_OK == nResult )
    {
        INetURLObject aURL( _aPath );
        if ( aURL.GetProtocol() == INetProtocol::File )
        {
            // remember the selected directory only for file URLs not for virtual folders
            sal_Int32 nLevel = aURL.getSegmentCount();
            bool bDir = m_aContent.isFolder( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
            if ( nLevel > 1 && ( FILEDLG_TYPE_FILEDLG == pImpl->_eDlgType || !bDir ) )
                aURL.removeSegment();
        }
    }

    return nResult;
}

bool SvtFileDialog::StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx)
{
    if (!PrepareExecute())
        return false;

    // start of the dialog
    return ModalDialog::StartExecuteAsync(rCtx);
}

void SvtFileDialog::onAsyncOperationStarted()
{
    EnableUI( false );
    // the cancel button must be always enabled
    pImpl->_pBtnCancel->Enable();
    pImpl->_pBtnCancel->GrabFocus();
}

void SvtFileDialog::onAsyncOperationFinished()
{
    EnableUI( true );
    m_pCurrentAsyncAction = nullptr;
    if ( !m_bInExecuteAsync )
        pImpl->_pEdFileName->GrabFocus();
        // (if m_bInExecuteAsync is true, then the operation was finished within the minimum wait time,
        // and to the user, the operation appears to be synchronous)
}


void SvtFileDialog::RemovablePlaceSelected(bool enable)
{
    pImpl->_pPlaces->SetDelEnabled( enable );
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
        aException.Arguments.realloc( 2 );
        aException.Arguments[ 0 ] <<= sDisplayPath;
        aException.Arguments[ 1 ] <<= PropertyValue(
            "Uri",
            -1, aException.Arguments[ 0 ], PropertyState_DIRECT_VALUE
        );
            // (formerly, it was sufficient to put the URL first parameter. Nowadays,
            // the services expects the URL in a PropertyValue named "Uri" ...)
        aException.Code = _eCode;
        aException.Classification = InteractionClassification_ERROR;

        // let and interaction handler handle this exception
        ::comphelper::OInteractionRequest* pRequest = nullptr;
        Reference< css::task::XInteractionRequest > xRequest = pRequest =
            new ::comphelper::OInteractionRequest( makeAny( aException ) );
        pRequest->addContinuation( new ::comphelper::OInteractionAbort( ) );

        Reference< XInteractionHandler2 > xHandler(
            InteractionHandler::createWithParent( ::comphelper::getProcessComponentContext(), nullptr ) );
        xHandler->handle( xRequest );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "iodlg::displayIOException: caught an exception!" );
    }
}


void SvtFileDialog::EnableUI( bool _bEnable )
{
    Enable( _bEnable );

    if ( _bEnable )
    {
        for ( auto& rxControl : m_aDisabledControls )
        {
            rxControl->Enable( false );
        }
    }
}


void SvtFileDialog::EnableControl( Control* _pControl, bool _bEnable )
{
    if ( !_pControl )
    {
        SAL_WARN( "fpicker.office", "SvtFileDialog::EnableControl: invalid control!" );
        return;
    }

    _pControl->Enable( _bEnable );

    if ( _bEnable )
    {
        auto aPos = m_aDisabledControls.find( _pControl );
        if ( m_aDisabledControls.end() != aPos )
            m_aDisabledControls.erase( aPos );
    }
    else
        m_aDisabledControls.insert( _pControl );
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
            Sequence< OUString > aProps(2);
            aProps[0] = "IsVolume";
            aProps[1] = "IsRemoveable";

            Reference< XResultSet > xResultSet
                = aCnt.createCursor( aProps, ::ucbhelper::INCLUDE_FOLDERS_ONLY );
            if ( xResultSet.is() && !xResultSet->next() )
            {
                std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetFrameWeld(),
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

    if ( ( pImpl->_nStyle & PickerFlags::SaveAs ) && m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);
    else
        m_aContent.enableDefaultInteractionHandler();

    // possibly just a filename without a path
    OUString aFileNameOnly;
    if( !_aPath.isEmpty() && (pImpl->_eMode == FILEDLG_MODE_SAVE)
                     && (_aPath.indexOf(':') == -1)
                     && (_aPath.indexOf('\\') == -1)
                     && (_aPath.indexOf('/') == -1))
    {
        aFileNameOnly = _aPath;
        _aPath.clear();
    }

    // no starting path specified?
    if ( _aPath.isEmpty() )
    {
        // then use the standard directory
        _aPath = lcl_ensureFinalSlash( pImpl->GetStandardDir() );

        // attach given filename to path
        if ( !aFileNameOnly.isEmpty() )
            _aPath += aFileNameOnly;
    }


    _aPath = implGetInitialURL( _aPath, GetStandardDir() );

    if ( pImpl->_nStyle & PickerFlags::SaveAs && !m_bHasFilename )
        // when doing a save-as, we do not want the handler to handle "this file does not exist" messages
        // - finally we're going to save that file, aren't we?
        m_aContent.enableOwnInteractionHandler(::svt::OFilePickerInteractionHandler::E_DOESNOTEXIST);

    // if applicable show filter
    pImpl->InitFilterList();

    // set up initial filter
    sal_uInt16 nFilterCount = GetFilterCount();
    OUString aAll = FpsResId( STR_FILTERNAME_ALL );
    bool bHasAll = pImpl->HasFilterListEntry( aAll );
    if ( pImpl->GetCurFilter() || nFilterCount == 1 || ( nFilterCount == 2 && bHasAll ) )
    {
        // if applicable set the only filter or the only filter that
        // does not refer to all files, as the current one
        if ( !pImpl->GetCurFilter() )
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
            SvtFileDialogFilter_Impl* pNewCurFilter = pImpl->m_aFilter[ nPos ].get();
            assert( pNewCurFilter && "SvtFileDialog::Execute: invalid filter pos!" );
            pImpl->SetCurFilter( pNewCurFilter, pNewCurFilter->GetName() );
        }

        // adjust view
        pImpl->SelectFilterListEntry( pImpl->GetCurFilter()->GetName() );
        SetDefaultExt( pImpl->GetCurFilter()->GetExtension() );
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
            pImpl->InsertFilterListEntry( pAllFilter );
            pImpl->SetCurFilter( pAllFilter, aAll );
        }
        pImpl->SelectFilterListEntry( aAll );
    }

    // if applicable isolate filter
    OUString aFilter;

    if ( !IsolateFilterFromPath_Impl( _aPath, aFilter ) )
        return false;

    AdjustFilterFlags nNewFilterFlags = adjustFilter( aFilter );
    if ( nNewFilterFlags & ( AdjustFilterFlags::NonEmpty | AdjustFilterFlags::UserFilter ) )
    {
        pImpl->_pEdFileName->SetText( aFilter );
    }

    // create and show instance for set path
    INetURLObject aFolderURL( _aPath );
    OUString aFileName( aFolderURL.getName( INetURLObject::LAST_SEGMENT, false ) );
    sal_Int32 nFileNameLen = aFileName.getLength();
    bool bFileToSelect = nFileNameLen != 0;
    if ( bFileToSelect && aFileName[ nFileNameLen - 1 ] != '/' )
    {
        OUString aDecodedName = aFolderURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
        pImpl->_pEdFileName->SetText( aDecodedName );
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

    _pFileView->Show();
    _pSplitter->Show();

    // if applicable read and set size from ini
    InitSize();

    return true;
}


void SvtFileDialog::executeAsync( ::svt::AsyncPickerAction::Action _eAction,
                                    const OUString& _rURL, const OUString& _rFilter )
{
    SAL_WARN_IF( m_pCurrentAsyncAction.is(), "fpicker.office", "SvtFileDialog::executeAsync: previous async action not yet finished!" );

    m_pCurrentAsyncAction = new AsyncPickerAction( this, _pFileView, _eAction );

    bool bReallyAsync = true;
    m_aConfiguration.getNodeValue( OUString( "FillAsynchronously" ) ) >>= bReallyAsync;

    sal_Int32 nMinTimeout = 0;
    m_aConfiguration.getNodeValue( OUString( "Timeout/Min" ) ) >>= nMinTimeout;
    sal_Int32 nMaxTimeout = 0;
    m_aConfiguration.getNodeValue( OUString( "Timeout/Max" ) ) >>= nMaxTimeout;

    m_bInExecuteAsync = true;
    m_pCurrentAsyncAction->execute( _rURL, _rFilter, bReallyAsync ? nMinTimeout : -1, nMaxTimeout, GetBlackList() );
    m_bInExecuteAsync = false;
}


void SvtFileDialog::FileSelect()
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( FILE_SELECTION_CHANGED, 0 );
}


void SvtFileDialog::FilterSelect()
{
    if ( _pFileNotifier )
        _pFileNotifier->notify( CTRL_STATE_CHANGED,
                                LISTBOX_FILTER );
}


void SvtFileDialog::SetStandardDir( const OUString& rStdDir )

/*  [Description]

   This method sets the path for the default button.
*/

{
    INetURLObject aObj( rStdDir );
    SAL_WARN_IF( aObj.GetProtocol() == INetProtocol::NotValid, "fpicker.office", "Invalid protocol!" );
    aObj.setFinalSlash();
    pImpl->SetStandardDir( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
}

void SvtFileDialog::SetBlackList( const css::uno::Sequence< OUString >& rBlackList )
{
    pImpl->SetBlackList( rBlackList );
}


const css::uno::Sequence< OUString >& SvtFileDialog::GetBlackList() const
{
    return pImpl->GetBlackList();
}


const OUString& SvtFileDialog::GetStandardDir() const

/*  [Description]

    This method returns the standard path.
*/

{
    return pImpl->GetStandardDir();
}


void SvtFileDialog::PrevLevel_Impl()
{
    _pFileView->EndInplaceEditing();

    OUString sDummy;
    executeAsync( AsyncPickerAction::ePrevLevel, sDummy, sDummy );
}


void SvtFileDialog::OpenURL_Impl( const OUString& _rURL )
{
    _pFileView->EndInplaceEditing();

    executeAsync( AsyncPickerAction::eOpenURL, _rURL, getMostCurrentFilter( pImpl ) );
}


SvtFileDialogFilter_Impl* SvtFileDialog::implAddFilter( const OUString& _rFilter, const OUString& _rType )
{
    SvtFileDialogFilter_Impl* pNewFilter = new SvtFileDialogFilter_Impl( _rFilter, _rType );
    pImpl->m_aFilter.push_front( std::unique_ptr<SvtFileDialogFilter_Impl>( pNewFilter ) );

    if ( !pImpl->GetCurFilter() )
        pImpl->SetCurFilter( pNewFilter, _rFilter );

    return pNewFilter;
}


void SvtFileDialog::AddFilter( const OUString& _rFilter, const OUString& _rType )
{
    SAL_WARN_IF( IsInExecute(), "fpicker.office", "SvtFileDialog::AddFilter: currently executing!" );
    implAddFilter ( _rFilter, _rType );
}


void SvtFileDialog::AddFilterGroup( const OUString& _rFilter, const Sequence< StringPair >& _rFilters )
{
    SAL_WARN_IF( IsInExecute(), "fpicker.office", "SvtFileDialog::AddFilter: currently executing!" );

    implAddFilter( _rFilter, OUString() );
    const StringPair* pSubFilters       =               _rFilters.getConstArray();
    const StringPair* pSubFiltersEnd    = pSubFilters + _rFilters.getLength();
    for ( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        implAddFilter( pSubFilters->First, pSubFilters->Second );
}


void SvtFileDialog::SetCurFilter( const OUString& rFilter )
{
    SAL_WARN_IF( IsInExecute(), "fpicker.office", "SvtFileDialog::SetCurFilter: currently executing!" );

    // look for corresponding filter
    sal_uInt16 nPos = pImpl->m_aFilter.size();

    while ( nPos-- )
    {
        SvtFileDialogFilter_Impl* pFilter = pImpl->m_aFilter[ nPos ].get();
        if ( pFilter->GetName() == rFilter )
        {
            pImpl->SetCurFilter( pFilter, rFilter );
            break;
        }
    }
}


OUString SvtFileDialog::GetCurFilter() const
{
    OUString aFilter;

    const SvtFileDialogFilter_Impl* pCurrentFilter = pImpl->GetCurFilter();
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
    return pImpl->m_aFilter.size();
}


const OUString& SvtFileDialog::GetFilterName( sal_uInt16 nPos ) const
{
    assert( nPos < GetFilterCount() && "invalid index" );
    return pImpl->m_aFilter[ nPos ]->GetName();
}


void SvtFileDialog::InitSize()
{
    if ( pImpl->_aIniKey.isEmpty() )
        return;

    // initialize from config
    SvtViewOptions aDlgOpt( EViewType::Dialog, pImpl->_aIniKey );

    if ( aDlgOpt.Exists() )
    {
        SetWindowState(OUStringToOString(aDlgOpt.GetWindowState(), osl_getThreadTextEncoding()));

        Any aUserData = aDlgOpt.GetUserItem( "UserData");
        OUString sCfgStr;
        if ( aUserData >>= sCfgStr )
            _pFileView->SetConfigString( sCfgStr );
    }
}


std::vector<OUString> SvtFileDialog::GetPathList() const
{
    std::vector<OUString> aList;
    sal_uLong           nCount = _pFileView->GetSelectionCount();
    SvTreeListEntry*    pEntry = nCount ? _pFileView->FirstSelected() : nullptr;

    if ( ! pEntry )
    {
        if ( !pImpl->_pEdFileName->GetText().isEmpty() && _bIsInExecute )
            aList.push_back(pImpl->_pEdFileName->GetURL());
        else
            aList.push_back(_aPath);
    }
    else
    {
        while ( pEntry )
        {
            aList.push_back(SvtFileView::GetURL(pEntry));
            pEntry = _pFileView->NextSelected( pEntry );
        }
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

    if ( nWildCardPos != -1 )
    {
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
            rFilter = aReversePath;
            rFilter = rFilter.copy( 0, nPathTokenPos );
            rFilter = comphelper::string::reverseString(rFilter);

            // determine folder
            rPath = aReversePath;
            rPath = rPath.copy( nPathTokenPos );
            rPath = comphelper::string::reverseString(rPath);
        }
        else
        {
            rFilter = rPath;
            rPath.clear();
        }
    }

    return true;
}


void SvtFileDialog::implUpdateImages( )
{
    // set the appropriate images on the buttons
    if ( pImpl->_pBtnUp )
        pImpl->_pBtnUp->SetModeImage( GetButtonImage( BMP_FILEDLG_BTN_UP ) );

    if ( pImpl->_pBtnNewFolder )
        pImpl->_pBtnNewFolder->SetModeImage( GetButtonImage( BMP_FILEDLG_CREATEFOLDER ) );
}


void SvtFileDialog::DataChanged( const DataChangedEvent& _rDCEvt )
{
    if ( DataChangedEventType::SETTINGS == _rDCEvt.GetType() )
        implUpdateImages( );

    ModalDialog::DataChanged( _rDCEvt );
}


void SvtFileDialog::Resize()
{
    Dialog::Resize();

    if ( IsRollUp() )
        return;

    if ( _pFileNotifier )
        _pFileNotifier->notify( DIALOG_SIZE_CHANGED, 0 );
}

Control* SvtFileDialog::getControl( sal_Int16 _nControlId, bool _bLabelControl ) const
{
    Control* pReturn = nullptr;

    switch ( _nControlId )
    {
        case CONTROL_FILEVIEW:
            pReturn = _bLabelControl ? nullptr : static_cast< Control* >( _pFileView );
            break;

        case EDIT_FILEURL:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( pImpl->_pFtFileName )
                    :   static_cast< Control* >( pImpl->_pEdFileName );
            break;

        case EDIT_FILEURL_LABEL:
            pReturn = static_cast< Control* >( pImpl->_pFtFileName );
            break;

        case CHECKBOX_AUTOEXTENSION:
            pReturn = pImpl->_pCbAutoExtension;
            break;

        case CHECKBOX_PASSWORD:
            pReturn = pImpl->_pCbPassword;
            break;

        case CHECKBOX_GPGENCRYPTION:
            pReturn = pImpl->_pCbGPGEncrypt;
            break;

        case CHECKBOX_FILTEROPTIONS:
            pReturn = pImpl->_pCbOptions;
            break;

        case CHECKBOX_READONLY:
            pReturn = _pCbReadOnly;
            break;

        case CHECKBOX_LINK:
            pReturn = _pCbLinkBox;
            break;

        case CHECKBOX_PREVIEW:
            pReturn = _pCbPreviewBox;
            break;

        case CHECKBOX_SELECTION:
            pReturn = _pCbSelection;
            break;

        case LISTBOX_FILTER:
            pReturn = _bLabelControl ? pImpl->_pFtFileType : pImpl->GetFilterListControl();
            break;

        case LISTBOX_FILTER_LABEL:
            pReturn = pImpl->_pFtFileType;
            break;

        case FIXEDTEXT_CURRENTFOLDER:
            pReturn = pImpl->_pEdCurrentPath;
            break;

        case LISTBOX_VERSION:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( pImpl->_pFtFileVersion )
                    :   static_cast< Control* >( pImpl->_pLbFileVersion );
            break;

        case LISTBOX_TEMPLATE:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( pImpl->_pFtTemplates )
                    :   static_cast< Control* >( pImpl->_pLbTemplates );
            break;

        case LISTBOX_IMAGE_TEMPLATE:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( pImpl->_pFtImageTemplates )
                    :   static_cast< Control* >( pImpl->_pLbImageTemplates );
            break;

        case LISTBOX_IMAGE_ANCHOR:
            pReturn =   _bLabelControl
                    ?   static_cast< Control* >( pImpl->_pFtImageAnchor )
                    :   static_cast< Control* >( pImpl->_pLbImageAnchor );
            break;

        case LISTBOX_VERSION_LABEL:
            pReturn = pImpl->_pFtFileVersion;
            break;

        case LISTBOX_TEMPLATE_LABEL:
            pReturn = pImpl->_pFtTemplates;
            break;

        case LISTBOX_IMAGE_TEMPLATE_LABEL:
            pReturn = pImpl->_pFtImageTemplates;
            break;

        case LISTBOX_IMAGE_ANCHOR_LABEL:
            pReturn = pImpl->_pFtImageAnchor;
            break;

        case PUSHBUTTON_OK:
            pReturn = pImpl->_pBtnFileOpen;
            break;

        case PUSHBUTTON_CANCEL:
            pReturn = pImpl->_pBtnCancel;
            break;

        case PUSHBUTTON_PLAY:
            pReturn = _pPbPlay;
            break;

        case PUSHBUTTON_HELP:
            pReturn = pImpl->_pBtnHelp;
            break;

        case TOOLBOXBUTOON_LEVEL_UP:
            pReturn = pImpl->_pBtnUp;
            break;

        case TOOLBOXBUTOON_NEW_FOLDER:
            pReturn = pImpl->_pBtnNewFolder;
            break;

        case LISTBOX_FILTER_SELECTOR:
            // only exists on SalGtkFilePicker
            break;

        default:
            SAL_WARN( "fpicker.office", "SvtFileDialog::getControl: invalid id!" );
    }
    return pReturn;
}


void SvtFileDialog::enableControl( sal_Int16 _nControlId, bool _bEnable )
{
    Control* pControl = getControl( _nControlId );
    if ( pControl )
        EnableControl( pControl, _bEnable );
    Control* pLabel = getControl( _nControlId, true );
    if ( pLabel )
        EnableControl( pLabel, _bEnable );
}


void SvtFileDialog::AddControls_Impl( )
{
    // create the "insert as link" checkbox, if needed
    if ( _nPickerFlags & PickerFlags::InsertAsLink )
    {
        _pCbLinkBox ->SetText( FpsResId( STR_SVT_FILEPICKER_INSERT_AS_LINK ) );
        _pCbLinkBox ->SetHelpId( HID_FILEDLG_LINK_CB );
        _pCbLinkBox->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbLinkBox->Show();
    }

    // create the "show preview" checkbox ( and the preview window, too ), if needed
    if ( _nPickerFlags & PickerFlags::ShowPreview  )
    {
        pImpl->_aIniKey = "ImportGraphicDialog";
        // because the "<All Formats> (*.bmp,*...)" entry is to wide,
        // we need to disable the auto width feature of the filter box
        pImpl->DisableFilterBoxAutoWidth();

        // "preview"
        _pCbPreviewBox->SetText( FpsResId( STR_SVT_FILEPICKER_SHOW_PREVIEW ) );
        _pCbPreviewBox->SetHelpId( HID_FILEDLG_PREVIEW_CB );
        _pCbPreviewBox->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbPreviewBox->Show();

        // generate preview window just here
        _pPrevWin->SetOutputSizePixel(Size(200, 300));
        _pPrevWin->Show();

        _pPrevBmp = VclPtr<FixedBitmap>::Create( _pPrevWin, WB_BORDER );
        _pPrevBmp->SetBackground( Wallpaper( COL_WHITE ) );
        _pPrevBmp->SetSizePixel(_pPrevWin->GetSizePixel());
        _pPrevBmp->Show();
        _pPrevBmp->SetAccessibleName(FpsResId(STR_PREVIEW));
    }

    if ( _nPickerFlags & PickerFlags::AutoExtension )
    {
        pImpl->_pCbAutoExtension->SetText( FpsResId( STR_SVT_FILEPICKER_AUTO_EXTENSION ) );
        pImpl->_pCbAutoExtension->Check();
        pImpl->_pCbAutoExtension->SetClickHdl( LINK( this, SvtFileDialog, AutoExtensionHdl_Impl ) );
        pImpl->_pCbAutoExtension->Show();
    }

    if ( _nPickerFlags & PickerFlags::FilterOptions )
    {
        pImpl->_pCbOptions->SetText( FpsResId( STR_SVT_FILEPICKER_FILTER_OPTIONS ) );
        pImpl->_pCbOptions->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        pImpl->_pCbOptions->Show();
    }

    if ( _nPickerFlags & PickerFlags::Selection )
    {
        _pCbSelection->SetText( FpsResId( STR_SVT_FILEPICKER_SELECTION ) );
        _pCbSelection->SetClickHdl( LINK( this, SvtFileDialog, ClickHdl_Impl ) );
        _pCbSelection->Show();
    }

    if ( _nPickerFlags & PickerFlags::PlayButton )
    {
        _pPbPlay->SetText( FpsResId( STR_SVT_FILEPICKER_PLAY ) );
        _pPbPlay->SetHelpId( HID_FILESAVE_DOPLAY );
        _pPbPlay->SetClickHdl( LINK( this, SvtFileDialog, PlayButtonHdl_Impl ) );
        _pPbPlay->Show();
    }

    if ( _nPickerFlags & PickerFlags::ShowVersions )
    {
        pImpl->_pFtFileVersion->SetText( FpsResId( STR_SVT_FILEPICKER_VERSION ) );
        pImpl->_pFtFileVersion->Show();

        pImpl->_pLbFileVersion->SetHelpId( HID_FILEOPEN_VERSION );
        pImpl->_pLbFileVersion->Show();
    }
    else if ( _nPickerFlags & PickerFlags::Templates )
    {
        pImpl->_pFtTemplates->SetText( FpsResId( STR_SVT_FILEPICKER_TEMPLATES ) );
        pImpl->_pFtTemplates->Show();

        pImpl->_pLbTemplates->SetHelpId( HID_FILEOPEN_VERSION );
        pImpl->_pLbTemplates->Show();
            // This is strange. During the re-factoring during 96930, I discovered that this help id
            // is set in the "Templates mode". This was hidden in the previous implementation.
            // Shouldn't this be a more meaningful help id.
    }
    else if ( _nPickerFlags & PickerFlags::ImageTemplate )
    {
        pImpl->_pFtImageTemplates->SetText( FpsResId( STR_SVT_FILEPICKER_IMAGE_TEMPLATE ) );
        pImpl->_pFtImageTemplates->Show();

        pImpl->_pLbImageTemplates->SetHelpId( HID_FILEOPEN_IMAGE_TEMPLATE );
        pImpl->_pLbImageTemplates->Show();
    }
    else if ( _nPickerFlags & PickerFlags::ImageAnchor )
    {
        pImpl->_pFtImageAnchor->SetText( FpsResId( STR_SVT_FILEPICKER_IMAGE_ANCHOR ) );
        pImpl->_pFtImageAnchor->Show();

        pImpl->_pLbImageAnchor->SetHelpId( HID_FILEOPEN_IMAGE_ANCHOR );
        pImpl->_pLbImageAnchor->Show();
    }

    pImpl->_pPlaces = VclPtr<PlacesListBox>::Create(_pContainer, this, FpsResId(STR_PLACES_TITLE), WB_BORDER);
    pImpl->_pPlaces->SetHelpId("SVT_HID_FILESAVE_PLACES_LISTBOX");
    Size aSize(LogicToPixel(Size(50, 85), MapMode(MapUnit::MapAppFont)));
    pImpl->_pPlaces->set_height_request(aSize.Height());
    pImpl->_pPlaces->set_width_request(aSize.Width());
    pImpl->_pPlaces->SetSizePixel(aSize);
    pImpl->_pPlaces->Show();

    sal_Int32 nPosX = pImpl->_pPlaces->GetSizePixel().Width();
    _pSplitter->SetPosPixel(Point(nPosX, 0));
    nPosX += _pSplitter->GetSizePixel().Width();
    _pFileView->SetPosPixel(Point(nPosX, 0));

    pImpl->_pPlaces->SetAddHdl( LINK ( this, SvtFileDialog, AddPlacePressed_Hdl ) );
    pImpl->_pPlaces->SetDelHdl( LINK ( this, SvtFileDialog, RemovePlacePressed_Hdl ) );

    initDefaultPlaces();
}


sal_Int32 SvtFileDialog::getTargetColorDepth()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetBitCount();
    else
        return 0;
}


sal_Int32 SvtFileDialog::getAvailableWidth()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetOutputSizePixel().Width();
    else
        return 0;
}


sal_Int32 SvtFileDialog::getAvailableHeight()
{
    if ( _pPrevBmp )
        return _pPrevBmp->GetOutputSizePixel().Height();
    else
        return 0;
}


void SvtFileDialog::setImage( sal_Int16 /*aImageFormat*/, const Any& rImage )
{
    if ( ! _pPrevBmp || ! _pPrevBmp->IsVisible() )
        return;

    Sequence < sal_Int8 > aBmpSequence;

    if ( rImage >>= aBmpSequence )
    {
        BitmapEx        aBmp;
        SvMemoryStream  aData( aBmpSequence.getArray(),
                               aBmpSequence.getLength(),
                               StreamMode::READ );
        ReadDIBBitmapEx(aBmp, aData);

        _pPrevBmp->SetBitmap( aBmp );
    }
    else
    {
        BitmapEx aEmpty;
        _pPrevBmp->SetBitmap( aEmpty );
    }
}


OUString SvtFileDialog::getCurrentFileText( ) const
{
    OUString sReturn;
    if ( pImpl && pImpl->_pEdFileName )
        sReturn = pImpl->_pEdFileName->GetText();
    return sReturn;
}


void SvtFileDialog::setCurrentFileText( const OUString& _rText, bool _bSelectAll )
{
    if ( pImpl && pImpl->_pEdFileName )
    {
        pImpl->_pEdFileName->SetText( _rText );
        if ( _bSelectAll )
            pImpl->_pEdFileName->SetSelection( Selection( 0, _rText.getLength() ) );
    }
}


bool SvtFileDialog::isAutoExtensionEnabled()
{
    return pImpl->_pCbAutoExtension && pImpl->_pCbAutoExtension->IsChecked();
}


bool SvtFileDialog::getShowState()
{
    if ( _pPrevBmp )
        return _pPrevBmp->IsVisible();
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

void SvtFileDialog::appendDefaultExtension(OUString& _rFileName,
                                           const OUString& _rFilterDefaultExtension,
                                           const OUString& _rFilterExtensions)
{
    const OUString aType(_rFilterExtensions.toAsciiLowerCase());

    if ( aType != FILEDIALOG_FILTER_ALL )
    {
        const OUString aTemp(_rFileName.toAsciiLowerCase());
        sal_Int32 nPos = 0;

        do
        {
            if (nPos+1<aType.getLength() && aType[nPos]=='*') // take care of a leading *
                ++nPos;
            const OUString aExt(aType.getToken( 0, FILEDIALOG_DEF_EXTSEP, nPos ));
            if (aExt.isEmpty())
                continue;
            if (aTemp.endsWith(aExt))
                return;
        }
        while (nPos>=0);

        _rFileName += "." + _rFilterDefaultExtension;
    }
}

void SvtFileDialog::initDefaultPlaces( )
{
    PlacePtr pRootPlace( new Place( FpsResId(STR_DEFAULT_DIRECTORY), GetStandardDir() ) );
    pImpl->_pPlaces->AppendPlace( pRootPlace );

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context));
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context));

    for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
    {
        PlacePtr pPlace(new Place(placesNamesList[nPlace], placesUrlsList[nPlace], true));
        pImpl->_pPlaces->AppendPlace(pPlace);
    }

    // Reset the placesList "updated" state
    pImpl->_pPlaces->IsUpdated();
}

IMPL_LINK_NOARG( SvtFileDialog, Split_Hdl, Splitter*, void )
{
    sal_Int32 nSplitPos = _pSplitter->GetSplitPosPixel();

    // Resize the places list
    sal_Int32 nPlaceX = pImpl->_pPlaces->GetPosPixel( ).X();
    Size placeSize = pImpl->_pPlaces->GetSizePixel( );
    placeSize.setWidth( nSplitPos - nPlaceX );
    pImpl->_pPlaces->SetSizePixel( placeSize );

    // Change Pos and size of the fileview
    Point fileViewPos = _pFileView->GetPosPixel();
    sal_Int32 nOldX = fileViewPos.X();
    sal_Int32 nNewX = nSplitPos + _pSplitter->GetSizePixel().Width();
    fileViewPos.setX( nNewX );
    Size fileViewSize = _pFileView->GetSizePixel();
    fileViewSize.AdjustWidth( -( nNewX - nOldX ) );
    _pFileView->SetPosSizePixel( fileViewPos, fileViewSize );

    _pSplitter->SetPosPixel( Point( placeSize.Width(), _pSplitter->GetPosPixel().Y() ) );
}

Image SvtFileDialog::GetButtonImage(const OUString& rButtonId)
{
    return Image(StockImage::Yes, rButtonId);
}

QueryFolderNameDialog::QueryFolderNameDialog(weld::Window* _pParent,
    const OUString& rTitle, const OUString& rDefaultText)
    : GenericDialogController(_pParent, "fps/ui/foldernamedialog.ui", "FolderNameDialog")
    , m_xNameEdit(m_xBuilder->weld_entry("entry"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
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
