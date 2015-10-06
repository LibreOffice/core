/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "RemoteFilesDialog.hxx"

class FileViewContainer : public vcl::Window
{
    enum FocusState
    {
        Prev = 0,
        TreeView,
        FileView,
        Next,
        FocusCount
    };

    private:
    VclPtr< SvtFileView > m_pFileView;
    VclPtr< FolderTree > m_pTreeView;
    VclPtr< Splitter > m_pSplitter;

    int m_nCurrentFocus;
    VclPtr<vcl::Window> m_pFocusWidgets[FocusState::FocusCount];

    public:
    FileViewContainer( vcl::Window *pParent )
        : Window( pParent, WB_TABSTOP )
        , m_pFileView( NULL )
        , m_pTreeView( NULL )
        , m_pSplitter( NULL )
        , m_nCurrentFocus( 0 )
    {
    }

    virtual ~FileViewContainer()
    {
        disposeOnce();
    }

    virtual void dispose() SAL_OVERRIDE
    {
        m_pFileView.clear();
        m_pTreeView.clear();
        m_pSplitter.clear();
        vcl::Window::dispose();
    }

    void init( SvtFileView* pFileView,
              Splitter* pSplitter,
              FolderTree* pTreeView,
              vcl::Window* pPrevSibling,
              vcl::Window* pNextSibling )
    {
        m_pFileView = pFileView;
        m_pTreeView = pTreeView;
        m_pSplitter = pSplitter;
        m_pFocusWidgets[FocusState::Prev] = pPrevSibling;
        m_pFocusWidgets[FocusState::TreeView] = pTreeView;
        m_pFocusWidgets[FocusState::FileView] = pFileView;
        m_pFocusWidgets[FocusState::Next] = pNextSibling;
    }

    virtual void Resize() SAL_OVERRIDE
    {
        Window::Resize();

        if( !m_pFileView || !m_pTreeView )
            return;

        Size aSize = GetSizePixel();
        Point aPos( m_pFileView->GetPosPixel() );
        Size aNewSize( aSize.Width() - aPos.X(), aSize.Height() );

        m_pFileView->SetSizePixel( aNewSize );

        // Resize the Splitter to fit the height
        Size splitterNewSize = m_pSplitter->GetSizePixel();
        splitterNewSize.Height() = aSize.Height();
        m_pSplitter->SetSizePixel( splitterNewSize );
        sal_Int32 nMinX = m_pTreeView->GetPosPixel().X();
        sal_Int32 nMaxX = m_pFileView->GetPosPixel().X() + m_pFileView->GetSizePixel().Width() - nMinX;
        m_pSplitter->SetDragRectPixel( Rectangle( Point( nMinX, 0 ), Size( nMaxX, aSize.Width() ) ) );

        // Resize the tree list box to fit the height of the FileView
        Size placesNewSize( m_pTreeView->GetSizePixel() );
        placesNewSize.Height() = aSize.Height();
        m_pTreeView->SetSizePixel( placesNewSize );
    }

    void changeFocus( bool bReverse )
    {
        if( !m_pFileView || !m_pTreeView )
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

    virtual void GetFocus() SAL_OVERRIDE
    {
        if( !m_pFileView || !m_pTreeView )
            return;

        GetFocusFlags aFlags = GetGetFocusFlags();

        if( aFlags & GetFocusFlags::Forward )
            m_nCurrentFocus = FocusState::TreeView;
        else if( aFlags & GetFocusFlags::Backward )
            m_nCurrentFocus = FocusState::FileView;

        if( m_nCurrentFocus >= FocusState::Prev && m_nCurrentFocus <= FocusState::Next )
        {
            m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
    }

    virtual bool Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE
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
        return Window::Notify( rNEvt );
    }
};

RemoteFilesDialog::RemoteFilesDialog( vcl::Window* pParent, WinBits nBits )
    : SvtFileDialog_Base( pParent, "RemoteFilesDialog", "fps/ui/remotefilesdialog.ui" )
    , m_xContext( comphelper::getProcessComponentContext() )
    , m_xMasterPasswd( PasswordContainer::create( m_xContext ) )
    , m_nWidth( 0 )
    , m_nHeight( 0 )
    , m_pCurrentAsyncAction( NULL )
    , m_pFileNotifier( NULL )
    , m_pSplitter( NULL )
    , m_pFileView( NULL )
    , m_pContainer( NULL )
    , m_pAddMenu( NULL )
{
    get( m_pCancel_btn, "cancel" );
    get( m_pAddService_btn, "add_service_btn" );
    get( m_pServices_lb, "services_lb" );
    get( m_pFilter_lb, "filter_lb" );
    get( m_pNewFolder, "new_folder" );

    m_eMode = ( nBits & WB_SAVEAS ) ? REMOTEDLG_MODE_SAVE : REMOTEDLG_MODE_OPEN;
    m_eType = ( nBits & WB_PATH ) ? REMOTEDLG_TYPE_PATHDLG : REMOTEDLG_TYPE_FILEDLG;
    m_bMultiselection = ( nBits & SFXWB_MULTISELECTION ) != 0;
    m_bIsUpdated = false;
    m_bIsConnected = false;
    m_bServiceChanged = false;
    m_nCurrentFilter = LISTBOX_ENTRY_NOTFOUND;

    m_pName_ed = VclPtr< AutocompleteEdit >::Create( get< vcl::Window >( "filename_container" ) );
    m_pName_ed->Show();

    m_pFilter_lb->Enable( false );
    m_pName_ed->Enable( false );

    if( m_eMode == REMOTEDLG_MODE_OPEN )
    {
        get( m_pOk_btn, "open" );

        m_pNewFolder->Hide();
    }
    else
    {
        get( m_pOk_btn, "save" );

        m_aImages = ImageList( fpicker::SvtResId( RID_FILEPICKER_IMAGES ) );
        m_pNewFolder->SetModeImage( m_aImages.GetImage( IMG_FILEDLG_CREATEFOLDER ) );
        m_pNewFolder->SetClickHdl( LINK( this, RemoteFilesDialog, NewFolderHdl ) );
    }

    m_pOk_btn->Show();
    m_pOk_btn->Enable( false );

    m_pOk_btn->SetClickHdl( LINK( this, RemoteFilesDialog, OkHdl ) );
    m_pCancel_btn->SetClickHdl( LINK( this, RemoteFilesDialog, CancelHdl ) );

    m_sRootLabel = fpicker::SvtResId( STR_SVT_ROOTLABEL );
    m_pPath = VclPtr<Breadcrumb>::Create( get< vcl::Window >( "breadcrumb_container" ) );
    m_pPath->set_hexpand( true );
    m_pPath->SetClickHdl( LINK( this, RemoteFilesDialog, SelectBreadcrumbHdl ) );
    m_pPath->SetMode( SvtBreadcrumbMode::ALL_VISITED );
    m_pPath->Show();

    m_pContainer = VclPtr< FileViewContainer >::Create( get< vcl::Window >("container") );

    m_pContainer->set_hexpand( true );
    m_pContainer->set_vexpand( true );

    m_pFileView = VclPtr< SvtFileView >::Create( m_pContainer, WB_BORDER | WB_TABSTOP,
                                       REMOTEDLG_TYPE_PATHDLG == m_eType,
                                       m_bMultiselection, false );

    m_pFileView->Show();
    m_pFileView->EnableAutoResize();
    m_pFileView->SetDoubleClickHdl( LINK( this, RemoteFilesDialog, DoubleClickHdl ) );
    m_pFileView->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectHdl ) );
    m_pFileView->EnableDelete( true );

    m_pSplitter = VclPtr< Splitter >::Create( m_pContainer, WB_HSCROLL );
    m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ) );
    m_pSplitter->SetSplitHdl( LINK( this, RemoteFilesDialog, SplitHdl ) );
    m_pSplitter->Show();

    m_pTreeView = VclPtr< FolderTree >::Create( m_pContainer, WB_BORDER );
    Size aSize( 150, 200 );
    m_pTreeView->set_height_request( aSize.Height() );
    m_pTreeView->set_width_request( aSize.Width() );
    m_pTreeView->SetSizePixel( aSize );
    m_pTreeView->Show();

    m_pTreeView->SetSelectHdl( LINK( this, RemoteFilesDialog, TreeSelectHdl ) );

    sal_Int32 nPosX = m_pTreeView->GetSizePixel().Width();
    m_pSplitter->SetPosPixel( Point( nPosX, 0 ) );
    nPosX += m_pSplitter->GetSizePixel().Width();
    m_pFileView->SetPosPixel( Point( nPosX, 0 ) );

    m_pContainer->init( m_pFileView, m_pSplitter, m_pTreeView, m_pAddService_btn, m_pFilter_lb );
    m_pContainer->Show();
    m_pContainer->Enable( false );

    m_sIniKey = "RemoteFilesDialog";
    InitSize();

    m_pName_ed->SetGetFocusHdl( LINK( this, RemoteFilesDialog, FileNameGetFocusHdl ) );
    m_pName_ed->SetModifyHdl( LINK( this, RemoteFilesDialog, FileNameModifyHdl ) );

    m_pAddService_btn->SetMenuMode( MENUBUTTON_MENUMODE_TIMED );
    m_pAddMenu = m_pAddService_btn->GetPopupMenu();
    m_pAddService_btn->SetClickHdl( LINK( this, RemoteFilesDialog, AddServiceHdl ) );
    m_pAddService_btn->SetSelectHdl( LINK( this, RemoteFilesDialog, EditServiceMenuHdl ) );

    FillServicesListbox();

    m_pServices_lb->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectServiceHdl ) );

    m_pFilter_lb->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectFilterHdl ) );
}

RemoteFilesDialog::~RemoteFilesDialog()
{
    disposeOnce();
}

void RemoteFilesDialog::dispose()
{
    m_pFileView->SetSelectHdl( Link<SvTreeListBox*,void>() );

    // save window state
    if( !m_sIniKey.isEmpty() )
    {
        SvtViewOptions aDlgOpt( E_DIALOG, m_sIniKey );
        aDlgOpt.SetWindowState( OStringToOUString( GetWindowState(), osl_getThreadTextEncoding() ) );

        Size aSize( GetSizePixel() );

        OUString sSize = OUString::number( aSize.Width() ) + "|";
        sSize = sSize + OUString::number( aSize.Height() ) + "|";

        OUString sUserData = m_pFileView->GetConfigString();
        aDlgOpt.SetUserItem( OUString( "UserData" ),
                             makeAny( sSize + sUserData ) );
    }

    // save services
    std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create( m_xContext ) );

    officecfg::Office::Common::Misc::FilePickerLastService::set( m_sLastServiceUrl, batch );

    if( m_bIsUpdated )
    {
        Sequence< OUString > placesUrlsList( m_aServices.size() );
        Sequence< OUString > placesNamesList( m_aServices.size() );

        int i = 0;
        for( std::vector< ServicePtr >::const_iterator it = m_aServices.begin(); it != m_aServices.end(); ++it )
        {
            placesUrlsList[i] = ( *it )->GetUrl();
            placesNamesList[i] = ( *it )->GetName();
            ++i;
        }

        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set( placesUrlsList, batch );
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set( placesNamesList, batch );
    }

    batch->commit();

    m_pTreeView.disposeAndClear();
    m_pFileView.disposeAndClear();
    m_pSplitter.disposeAndClear();
    m_pContainer.disposeAndClear();
    m_pPath.disposeAndClear();

    m_pOk_btn.clear();
    m_pCancel_btn.clear();
    m_pAddService_btn.clear();
    m_pServices_lb.clear();
    m_pFilter_lb.clear();
    m_pName_ed.disposeAndClear();
    m_pNewFolder.clear();

    ModalDialog::dispose();
}

void RemoteFilesDialog::Resize()
{
    ModalDialog::Resize();

    if( m_pFileView && m_pContainer )
    {
        Size aSize = m_pContainer->GetSizePixel();
        m_pFileView->SetSizePixel( aSize );
    }
    Invalidate(InvalidateFlags::Update);
}

short RemoteFilesDialog::Execute()
{
    if( m_pServices_lb->GetEntryCount() > 0 )
    {
        Show();
        SelectServiceHdl( NULL );
    }
    if( !m_bIsConnected )
    {
        m_pServices_lb->SetNoSelection();
        m_pAddService_btn->SetPopupMenu( NULL );
    }

    short nRet = SvtFileDialog_Base::Execute();

    return nRet;
}

void RemoteFilesDialog::Show()
{
    SvtFileDialog_Base::Show();

    if( m_nWidth > 0 && m_nHeight > 0 )
    {
        Size aSize( m_nWidth, m_nHeight );
        SetSizePixel( aSize );
    }
}

OUString lcl_GetServiceType( ServicePtr pService )
{
    INetProtocol aProtocol = pService->GetUrlObject().GetProtocol();
    switch( aProtocol )
    {
        case INetProtocol::Ftp:
            return OUString( "FTP" );
        case INetProtocol::Cmis:
        {
            OUString sHost = pService->GetUrlObject().GetHost( INetURLObject::DECODE_WITH_CHARSET );

            if( sHost.startsWith( GDRIVE_BASE_URL ) )
                return OUString( "Google Drive" );
            else if( sHost.startsWith( ALFRESCO_CLOUD_BASE_URL ) )
                return OUString( "Alfresco Cloud" );
            else if( sHost.startsWith( ONEDRIVE_BASE_URL ) )
                return OUString( "OneDrive" );

            return OUString( "CMIS" );
        }
        case INetProtocol::Smb:
            return OUString( "Windows Share" );
        case INetProtocol::File:
            return OUString( "SSH" );
        case INetProtocol::Http:
            return OUString( "WebDAV" );
        case INetProtocol::Https:
            return OUString( "WebDAV" );
        case INetProtocol::Generic:
            return OUString( "SSH" );
        default:
            return OUString( "" );
    }
}

void RemoteFilesDialog::InitSize()
{
    if( m_sIniKey.isEmpty() )
        return;

    // initialize from config
    SvtViewOptions aDlgOpt( E_DIALOG, m_sIniKey );

    if( aDlgOpt.Exists() )
    {
        SetWindowState( OUStringToOString( aDlgOpt.GetWindowState(), osl_getThreadTextEncoding() ) );

        Any aUserData = aDlgOpt.GetUserItem( OUString( "UserData" ) );
        OUString sCfgStr;
        if( aUserData >>= sCfgStr )
        {
            int nPos = sCfgStr.indexOf( "|" );
            if( nPos != -1 )
            {
                nPos = sCfgStr.indexOf( "|", nPos + 1 );
                if( nPos != -1 )
                {
                    sal_Int32 nIdx = 0;
                    m_nWidth = sCfgStr.getToken( 0, '|', nIdx ).toInt32();
                    m_nHeight = sCfgStr.getToken( 0, '|', nIdx ).toInt32();

                    m_pFileView->SetConfigString( sCfgStr.copy( nPos + 1) );
                }
            }
        }
    }
}

void RemoteFilesDialog::FillServicesListbox()
{
    m_pServices_lb->Clear();
    m_aServices.clear();

    // Load from user settings
    Sequence< OUString > placesUrlsList( officecfg::Office::Common::Misc::FilePickerPlacesUrls::get( m_xContext ) );
    Sequence< OUString > placesNamesList( officecfg::Office::Common::Misc::FilePickerPlacesNames::get( m_xContext ) );

    unsigned int nPos = 0;
    unsigned int i = 0;

    m_sLastServiceUrl = officecfg::Office::Common::Misc::FilePickerLastService::get( m_xContext );

    for( sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace )
    {
        ServicePtr pService( new Place( placesNamesList[nPlace], placesUrlsList[nPlace], true ) );
        m_aServices.push_back( pService );

        // Add to the listbox only remote services, not local bookmarks
        if( !pService->IsLocal() )
        {
            OUString sPrefix = lcl_GetServiceType( pService );

            if( !sPrefix.isEmpty() )
                 sPrefix += ": ";

            if( placesUrlsList[nPlace] == m_sLastServiceUrl )
                nPos = i;

            m_pServices_lb->InsertEntry( sPrefix + placesNamesList[nPlace] );

            i++;
        }
    }

    if( m_pServices_lb->GetEntryCount() > 0 )
    {
        m_pServices_lb->SelectEntryPos( nPos );
        m_pAddService_btn->SetPopupMenu( m_pAddMenu );
    }
    else
        m_pAddService_btn->SetPopupMenu( NULL );

    EnableControls();
}

int RemoteFilesDialog::GetSelectedServicePos()
{
    int nSelected = m_pServices_lb->GetSelectEntryPos();
    int nPos = 0;
    int i = -1;

    if( m_aServices.size() == 0 )
        return -1;

    while( nPos < ( int )m_aServices.size() )
    {
        while( m_aServices[nPos]->IsLocal() )
            nPos++;
        i++;
        if( i == nSelected )
            break;
        nPos++;
    }

    return nPos;
}

void RemoteFilesDialog::AddFilter( const OUString& rFilter, const OUString& rType )
{
    OUString sName = rFilter;

    if ( rType.isEmpty() )
        sName = "------------------------------------------";

    m_aFilters.push_back( std::pair< OUString, OUString >( rFilter, rType ) );
    m_pFilter_lb->InsertEntry( sName );

    if( m_pFilter_lb->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
        m_pFilter_lb->SelectEntryPos( 0 );
}


FileViewResult RemoteFilesDialog::OpenURL( OUString const & sURL )
{
    FileViewResult eResult = eFailure;

    if( m_pFileView )
    {
        m_pTreeView->EndSelection();
        DisableControls();

        EnableChildPointerOverwrite( true );
        SetPointer( PointerStyle::Wait );
        Invalidate(InvalidateFlags::Update);

        if( !sURL.isEmpty() )
        {
            OUString sFilter = FILEDIALOG_FILTER_ALL;

            if( m_nCurrentFilter != LISTBOX_ENTRY_NOTFOUND )
            {
                sFilter = m_aFilters[m_nCurrentFilter].second;
            }

            m_pFileView->EndInplaceEditing( false );

            DBG_ASSERT( !m_pCurrentAsyncAction.is(), "SvtFileDialog::executeAsync: previous async action not yet finished!" );

            m_pCurrentAsyncAction = new AsyncPickerAction( this, m_pFileView, AsyncPickerAction::Action::eOpenURL );

            // -1 timeout - sync
            m_pCurrentAsyncAction->execute( sURL, sFilter, -1, -1, GetBlackList() );

            if( m_eMode != REMOTEDLG_MODE_SAVE )
                m_pName_ed->SetText( "" );

            m_pFileView->GrabFocus();
        }
        else
        {
            SetPointer( PointerStyle::Arrow );
            EnableChildPointerOverwrite( false );

            // content doesn't exist
            ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTS );

            EnableControls();
            return eFailure;
        }

        SetPointer( PointerStyle::Arrow );
        EnableChildPointerOverwrite( false );
    }

    return eResult;
}

void RemoteFilesDialog::AddFileExtension()
{
    if( m_nCurrentFilter != LISTBOX_ENTRY_NOTFOUND )
    {
        OUString sExt = m_aFilters[m_nCurrentFilter].second;
        OUString sFileName = m_pName_ed->GetText();

        sal_Int32 nDotPos = sFileName.lastIndexOf( '.' );

        if ( nDotPos == -1 )
        {
            sFileName += sExt.copy( 1 ); // without '*'
            m_pName_ed->SetText( sFileName );
        }
    }
}

void RemoteFilesDialog::EnableControls()
{
    if( m_pServices_lb->GetEntryCount() > 0 )
    {
        m_pServices_lb->Enable( true );

        if( m_pServices_lb->GetSelectEntryCount() )
        {
            m_pAddMenu->EnableItem( "change_password", false );

            try
            {
                if( m_xMasterPasswd->isPersistentStoringAllowed() )
                {
                    int nPos = GetSelectedServicePos();

                    if( nPos >= 0 )
                    {
                        OUString sUrl( m_aServices[nPos]->GetUrl() );

                        UrlRecord aURLEntries = m_xMasterPasswd->find( sUrl, Reference< XInteractionHandler>() );

                        if( aURLEntries.UserList.getLength() )
                        {
                            m_pAddMenu->EnableItem( "change_password" );
                        }
                    }
                }
            }
            catch( const Exception& )
            {}
        }
    }
    else
        m_pServices_lb->Enable( false );

    if( m_bIsConnected )
    {
        m_pFilter_lb->Enable( true );
        m_pName_ed->Enable( true );
        m_pContainer->Enable( true );

        if( !m_pName_ed->GetText().isEmpty() )
            m_pOk_btn->Enable( true );
        else
            m_pOk_btn->Enable( false );
    }
    else
    {
        m_pFilter_lb->Enable( false );
        m_pName_ed->Enable( false );
        m_pContainer->Enable( false );
        m_pOk_btn->Enable( false );
    }

    m_pPath->EnableFields( true );
    m_pAddService_btn->Enable( true );

    Invalidate(InvalidateFlags::Update);
}

void RemoteFilesDialog::DisableControls()
{
    m_pServices_lb->Enable( false );
    m_pFilter_lb->Enable( false );
    m_pAddService_btn->Enable( false );
    m_pName_ed->Enable( false );
    m_pContainer->Enable( false );
    m_pOk_btn->Enable( false );
    m_pPath->EnableFields( false );

    m_pCancel_btn->Enable( true );
}

void RemoteFilesDialog::SavePassword( const OUString& rURL, const OUString& rUser
                                    , const OUString& rPassword, bool bPersistent )
{
    if( rURL.isEmpty() || rUser.isEmpty() || rPassword.isEmpty() )
        return;

    try
    {
        if( !bPersistent ||
            ( m_xMasterPasswd->isPersistentStoringAllowed()
            && m_xMasterPasswd->authorizateWithMasterPassword( Reference< XInteractionHandler>() ) )
        )
        {
            Reference< XInteractionHandler > xInteractionHandler(
                InteractionHandler::createWithParent( m_xContext, 0 ),
                UNO_QUERY );

            Sequence< OUString > aPasswd( 1 );
            aPasswd[0] = rPassword;

            if( bPersistent )
                m_xMasterPasswd->addPersistent(
                    rURL, rUser, aPasswd, xInteractionHandler );
            else
                m_xMasterPasswd->add( rURL, rUser, aPasswd, xInteractionHandler );
        }
    }
    catch( const Exception& )
    {}
}

IMPL_LINK_NOARG_TYPED ( RemoteFilesDialog, AddServiceHdl, Button*, void )
{
    ScopedVclPtrInstance< PlaceEditDialog > aDlg( this );
    aDlg->ShowPasswordControl();
    short aRetCode = aDlg->Execute();

    switch( aRetCode )
    {
        case RET_OK :
        {
            ServicePtr newService = aDlg->GetPlace();
            m_aServices.push_back( newService );

            OUString sPassword = aDlg->GetPassword();
            OUString sUser = aDlg->GetUser();
            if( !sUser.isEmpty() && !sPassword.isEmpty() )
            {
                bool bPersistent = aDlg->IsRememberChecked();
                SavePassword( newService->GetUrl(), sUser, sPassword, bPersistent );
            }

            OUString sPrefix = lcl_GetServiceType( newService );

            if(!sPrefix.isEmpty())
                 sPrefix += ": ";

            m_pServices_lb->InsertEntry( sPrefix + newService->GetName() );
            m_pServices_lb->SelectEntryPos( m_pServices_lb->GetEntryCount() - 1 );
            m_pAddService_btn->SetPopupMenu( m_pAddMenu );
            SelectServiceHdl( NULL );

            m_bIsUpdated = true;

            EnableControls();
      break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectServiceHdl )
{
    int nPos = GetSelectedServicePos();

    if( nPos >= 0 )
    {
        OUString sURL = m_aServices[nPos]->GetUrl();
        m_pAddService_btn->SetPopupMenu( m_pAddMenu );

        m_bServiceChanged = true;
        OpenURL( sURL );
    }

    return 1;
}

IMPL_LINK_TYPED ( RemoteFilesDialog, EditServiceMenuHdl, MenuButton *, pButton, void )
{
    OString sIdent( pButton->GetCurItemIdent() );
    if( sIdent == "edit_service"  && m_pServices_lb->GetEntryCount() > 0 )
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if( nPos >= 0 )
        {
            ScopedVclPtrInstance< PlaceEditDialog > aDlg( this, m_aServices[nPos] );
            short aRetCode = aDlg->Execute();

            switch( aRetCode )
            {
                case RET_OK :
                {
                    ServicePtr pEditedService = aDlg->GetPlace();

                    m_aServices[nPos] = pEditedService;
                    m_pServices_lb->RemoveEntry( nSelected );

                    OUString sPrefix = lcl_GetServiceType( pEditedService );

                    if(!sPrefix.isEmpty())
                        sPrefix += ": ";

                    m_pServices_lb->InsertEntry( sPrefix + pEditedService->GetName(), nSelected );
                    m_pServices_lb->SelectEntryPos( nSelected );

                    m_bIsUpdated = true;
                    break;
                }
                case RET_NO:
                    sIdent = "delete_service";
                    break;
                case RET_CANCEL :
                default :
                    // Do Nothing
                    break;
            };
        }
    }
    if( sIdent == "delete_service"  && m_pServices_lb->GetEntryCount() > 0 )
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if( nPos >= 0 )
        {
            OUString sMsg = fpicker::SvtResId( STR_SVT_DELETESERVICE );
            sMsg = sMsg.replaceFirst( "$servicename$", m_pServices_lb->GetSelectEntry() );
            ScopedVclPtrInstance< MessageDialog > aBox( this, sMsg, VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO );

            if( aBox->Execute() == RET_YES )
            {
                // remove password
                try
                {
                    if( m_xMasterPasswd->isPersistentStoringAllowed() )
                    {
                        OUString sUrl( m_aServices[nPos]->GetUrl() );

                        Reference< XInteractionHandler > xInteractionHandler(
                            InteractionHandler::createWithParent( m_xContext, 0 ),
                            UNO_QUERY );

                        UrlRecord aURLEntries = m_xMasterPasswd->find( sUrl, xInteractionHandler );

                        if( aURLEntries.Url == sUrl && aURLEntries.UserList.getLength() )
                        {
                            OUString sUserName = aURLEntries.UserList[0].UserName;

                            m_xMasterPasswd->removePersistent( sUrl, sUserName );
                        }
                    }
                }
                catch( const Exception& )
                {}

                m_aServices.erase( m_aServices.begin() + nPos );
                m_pServices_lb->RemoveEntry( nSelected );

                m_pServices_lb->SetNoSelection();
                m_pAddService_btn->SetPopupMenu( NULL );

                m_bIsUpdated = true;

                m_bIsConnected = false;
                EnableControls();
            }
        }
    }
    else if( sIdent == "change_password" )
    {
        try
        {
            if( m_xMasterPasswd->isPersistentStoringAllowed() && m_xMasterPasswd->authorizateWithMasterPassword( Reference< XInteractionHandler>() ) )
            {
                int nPos = GetSelectedServicePos();

                if( nPos >= 0 )
                {
                    OUString sUrl( m_aServices[nPos]->GetUrl() );

                    Reference< XInteractionHandler > xInteractionHandler(
                        InteractionHandler::createWithParent( m_xContext, 0 ),
                        UNO_QUERY );

                    UrlRecord aURLEntries = m_xMasterPasswd->find( sUrl, xInteractionHandler );

                    if( aURLEntries.Url == sUrl && aURLEntries.UserList.getLength() )
                    {
                        OUString sUserName = aURLEntries.UserList[0].UserName;

                        ::comphelper::SimplePasswordRequest* pPasswordRequest
                            = new ::comphelper::SimplePasswordRequest( PasswordRequestMode_PASSWORD_CREATE );
                        Reference< XInteractionRequest > rRequest( pPasswordRequest );

                        xInteractionHandler->handle( rRequest );

                        if ( pPasswordRequest->isPassword() )
                        {
                            OUString aNewPass = pPasswordRequest->getPassword();
                            Sequence< OUString > aPasswd( 1 );
                            aPasswd[0] = aNewPass;

                            m_xMasterPasswd->addPersistent(
                                sUrl, sUserName, aPasswd, xInteractionHandler );
                        }
                    }
                }
            }
        }
        catch( const Exception& )
        {}
    }

    EnableControls();
}

IMPL_LINK_NOARG_TYPED( RemoteFilesDialog, DoubleClickHdl, SvTreeListBox*, bool )
{
    if( m_pFileView->GetSelectionCount() )
    {
        SvTreeListEntry* pEntry = m_pFileView->FirstSelected();

        if( pEntry )
        {
            SvtContentEntry* pData = static_cast< SvtContentEntry* >( pEntry->GetUserData() );

            if( pData )
            {
                if( !pData->mbIsFolder )
                {
                    EndDialog( RET_OK );
                }
                else
                {
                    OpenURL( pData->maURL );
                }
            }
        }
    }

    return true;
}

IMPL_LINK_NOARG_TYPED( RemoteFilesDialog, SelectHdl, SvTreeListBox*, void )
{
    SvTreeListEntry* pEntry = m_pFileView->FirstSelected();

    if( pEntry )
    {
        SvtContentEntry* pData = static_cast< SvtContentEntry* >( pEntry->GetUserData() );

        if( pData )
        {
            if( ( pData->mbIsFolder && ( m_eType == REMOTEDLG_TYPE_PATHDLG ) )
                || ( !pData->mbIsFolder && ( m_eType == REMOTEDLG_TYPE_FILEDLG ) ) )
            {
                // url must contain user info, because we need this info in recent files entry
                // (to fill user field in login box by default)
                INetURLObject aURL( pData->maURL );
                INetURLObject aCurrentURL( m_sLastServiceUrl );
                aURL.SetUser( aCurrentURL.GetUser() );

                m_sPath = aURL.GetMainURL( INetURLObject::NO_DECODE );

                m_pName_ed->SetText( INetURLObject::decode( aURL.GetLastName(), INetURLObject::DECODE_WITH_CHARSET ) );
            }
            else
            {
                if( m_eMode == REMOTEDLG_MODE_OPEN )
                {
                    m_sPath.clear();
                    m_pName_ed->SetText( "" );
                }
            }

            EnableControls();
        }
    }
}

IMPL_LINK_NOARG_TYPED( RemoteFilesDialog, FileNameGetFocusHdl, Control&, void )
{
    m_pFileView->SetNoSelection();
}

IMPL_LINK_NOARG( RemoteFilesDialog, FileNameModifyHdl )
{
    m_pFileView->SetNoSelection();
    if( !m_pOk_btn->IsEnabled() )
        EnableControls();

    return 1;
}

IMPL_LINK_NOARG_TYPED( RemoteFilesDialog, SplitHdl, Splitter*, void )
{
    sal_Int32 nSplitPos = m_pSplitter->GetSplitPosPixel();

    // Resize the tree list box
    sal_Int32 nPlaceX = m_pTreeView->GetPosPixel().X();
    Size placeSize = m_pTreeView->GetSizePixel();
    placeSize.Width() = nSplitPos - nPlaceX;
    m_pTreeView->SetSizePixel( placeSize );

    // Change Pos and size of the fileview
    Point fileViewPos = m_pFileView->GetPosPixel();
    sal_Int32 nOldX = fileViewPos.X();
    sal_Int32 nNewX = nSplitPos + m_pSplitter->GetSizePixel().Width();
    fileViewPos.X() = nNewX;
    Size fileViewSize = m_pFileView->GetSizePixel();
    fileViewSize.Width() -= ( nNewX - nOldX );
    m_pFileView->SetPosSizePixel( fileViewPos, fileViewSize );

    m_pSplitter->SetPosPixel( Point( placeSize.Width(), m_pSplitter->GetPosPixel().Y() ) );
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectFilterHdl )
{
    unsigned int nPos = m_pFilter_lb->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND && !m_aFilters[nPos].second.isEmpty() )
    {
        m_nCurrentFilter = nPos;

        OUString sCurrentURL = m_pFileView->GetViewURL();

        if( !sCurrentURL.isEmpty() && m_bIsConnected )
            OpenURL( sCurrentURL );
    }

    return 1;
}

IMPL_LINK_TYPED( RemoteFilesDialog, TreeSelectHdl, SvTreeListBox *, pBox, void )
{
    OUString* sURL = static_cast< OUString* >( pBox->GetHdlEntry()->GetUserData() );

    if( sURL )
    {
        OpenURL( *sURL );
        m_pFileView->GrabFocus();
    }
}

IMPL_LINK_TYPED ( RemoteFilesDialog, SelectBreadcrumbHdl, Breadcrumb*, pPtr, void )
{
    OpenURL( pPtr->GetHdlURL() );
}

IMPL_LINK_NOARG_TYPED ( RemoteFilesDialog, NewFolderHdl, Button*, void )
{
    m_pFileView->EndInplaceEditing( false );

    SmartContent aContent( m_pFileView->GetViewURL() );
    OUString aTitle;
    aContent.getTitle( aTitle );
    ScopedVclPtrInstance< QueryFolderNameDialog > aDlg( this, aTitle, fpicker::SVT_RESSTR( STR_SVT_NEW_FOLDER ) );
    bool bHandled = false;

    while( !bHandled )
    {
        if( aDlg->Execute() == RET_OK )
        {
            OUString aUrl = aContent.createFolder( aDlg->GetName() );
            if( !aUrl.isEmpty() )
            {
                m_pFileView->CreatedFolder( aUrl, aDlg->GetName() );
                bHandled = true;
            }
        }
        else
            bHandled = true;
    }
}

IMPL_LINK_NOARG_TYPED ( RemoteFilesDialog, OkHdl, Button*, void )
{
    OUString sNameNoExt = m_pName_ed->GetText();
    OUString sPathNoExt;

    // auto extension
    if( m_eMode == REMOTEDLG_MODE_SAVE )
        AddFileExtension();

    // check if file/path exists

    OUString sCurrentPath = m_pFileView->GetViewURL();
    OUString sSelectedItem = m_pFileView->GetCurrentURL();
    OUString sName = m_pName_ed->GetText();

    bool bFileDlg = ( m_eType == REMOTEDLG_TYPE_FILEDLG );
    bool bSelected = ( m_pFileView->GetSelectionCount() > 0 );

    if( !sCurrentPath.endsWith("/") )
        sCurrentPath += "/";

    if( !bSelected )
    {
        m_sPath = sCurrentPath + INetURLObject::encode( sName, INetURLObject::PART_FPATH, INetURLObject::ENCODE_ALL );
        sPathNoExt = sCurrentPath + INetURLObject::encode( sNameNoExt, INetURLObject::PART_FPATH, INetURLObject::ENCODE_ALL );
    }
    else
    {
        if( m_eType == REMOTEDLG_TYPE_PATHDLG )
            m_sPath = sCurrentPath;
        else
            m_sPath = sSelectedItem;

        // url must contain user info, because we need this info in recent files entry
        // (to fill user field in login box by default)
        INetURLObject aURL( m_sPath );
        INetURLObject aCurrentURL( m_sLastServiceUrl );
        aURL.SetUser( aCurrentURL.GetUser() );

        m_sPath = aURL.GetMainURL( INetURLObject::NO_DECODE );
    }

    bool bExists = false;

    if( bFileDlg )
        bExists = ContentIsDocument( m_sPath );
    else
        bExists = ContentIsFolder( m_sPath );

    if( bExists )
    {
        if( m_eMode == REMOTEDLG_MODE_SAVE )
        {
            OUString sMsg = fpicker::SvtResId( STR_SVT_ALREADYEXISTOVERWRITE );
            sMsg = sMsg.replaceFirst( "$filename$", sName );
            ScopedVclPtrInstance< MessageDialog > aBox( this, sMsg, VCL_MESSAGE_QUESTION, VCL_BUTTONS_YES_NO );
            if( aBox->Execute() != RET_YES )
                return;
        }
    }
    else
    {
        if( ContentIsFolder( sPathNoExt ) )
        {
            OpenURL( sPathNoExt );
            m_pName_ed->SetText( "" );

            if( !bSelected )
                m_pName_ed->GrabFocus();

            return;
        }

        if( m_eMode == REMOTEDLG_MODE_OPEN )
            return;
    }

    EndDialog( RET_OK );
}

IMPL_LINK_NOARG_TYPED ( RemoteFilesDialog, CancelHdl, Button*, void )
{
    if( m_pCurrentAsyncAction.is() )
    {
        m_pCurrentAsyncAction->cancel();
        onAsyncOperationFinished();
    }
    else
    {
        EndDialog();
    }
}

// SvtFileDialog_Base

SvtFileView* RemoteFilesDialog::GetView()
{
    return m_pFileView;
}

void RemoteFilesDialog::SetHasFilename( bool )
{
}

void RemoteFilesDialog::SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList )
{
    m_aBlackList = rBlackList;
    m_pTreeView->SetBlackList( rBlackList );
}

const ::com::sun::star::uno::Sequence< OUString >& RemoteFilesDialog::GetBlackList() const
{
    return m_aBlackList;
}

void RemoteFilesDialog::SetStandardDir( const OUString& rStdDir )
{
    m_sStdDir = rStdDir;
}

const OUString& RemoteFilesDialog::GetStandardDir() const
{
    return m_sStdDir;
}

void RemoteFilesDialog::SetPath( const OUString& rNewURL )
{
    m_sPath = rNewURL;

    if( m_eMode == REMOTEDLG_MODE_SAVE )
    {
        INetURLObject aUrl( m_sPath );
        OUString sFileName = aUrl.GetLastName( INetURLObject::DECODE_WITH_CHARSET );

        m_pName_ed->SetText( sFileName );
    }
}

OUString RemoteFilesDialog::getCurrentFileText() const
{
    OUString sReturn;
    if( m_pName_ed )
        sReturn = m_pName_ed->GetText();
    return sReturn;
}

void RemoteFilesDialog::setCurrentFileText( const OUString& rText, bool bSelectAll )
{
    if( m_pName_ed )
    {
        m_pName_ed->SetText( rText );
        if( bSelectAll )
            m_pName_ed->SetSelection( Selection( 0, rText.getLength() ) );
    }
}

void RemoteFilesDialog::AddFilterGroup(
                                  const OUString& rFilter,
                                  const com::sun::star::uno::Sequence< com::sun::star::beans::StringPair >& rFilters )
{
    AddFilter( rFilter, OUString() );
    const StringPair* pSubFilters       =               rFilters.getConstArray();
    const StringPair* pSubFiltersEnd    = pSubFilters + rFilters.getLength();
    for ( ; pSubFilters != pSubFiltersEnd; ++pSubFilters )
        AddFilter( pSubFilters->First, pSubFilters->Second );
}

OUString RemoteFilesDialog::GetCurFilter() const
{
    OUString sFilter;

    if( m_nCurrentFilter != LISTBOX_ENTRY_NOTFOUND )
    {
        sFilter = m_aFilters[m_nCurrentFilter].first;
    }

    return sFilter;
}

OUString RemoteFilesDialog::getCurFilter( ) const
{
    return GetCurFilter();
}

void RemoteFilesDialog::SetCurFilter( const OUString& rFilter )
{
    DBG_ASSERT( !IsInExecute(), "SvtFileDialog::SetCurFilter: currently executing!" );

    // look for corresponding filter
    sal_uInt16 nPos = m_aFilters.size();

    while ( nPos-- )
    {
        if ( m_aFilters[nPos].first == rFilter )
        {
            m_nCurrentFilter = nPos;
            m_pFilter_lb->SelectEntryPos( m_nCurrentFilter );
            break;
        }
    }
}

void RemoteFilesDialog::FilterSelect()
{
}

void RemoteFilesDialog::SetFileCallback( ::svt::IFilePickerListener *pNotifier )
{
    m_pFileNotifier = pNotifier;
}

void RemoteFilesDialog::onAsyncOperationStarted()
{
    DisableControls();
}

void RemoteFilesDialog::onAsyncOperationFinished()
{
    m_pCurrentAsyncAction = NULL;
    EnableControls();
}

void RemoteFilesDialog::UpdateControls( const OUString& rURL )
{
    int nPos = GetSelectedServicePos();

    if( nPos >= 0 && m_bServiceChanged && rURL == m_aServices[nPos]->GetUrl() )
    {
        OUString sURL = m_aServices[nPos]->GetUrl();

        m_pPath->SetRootName( m_sRootLabel );
        m_pTreeView->Clear();

        SvTreeListEntry* pRoot = m_pTreeView->InsertEntry( m_sRootLabel, NULL, true );
        OUString* sData = new OUString( rURL );
        pRoot->SetUserData( static_cast< void* >( sData ) );

        m_pName_ed->GrabFocus();

        m_sLastServiceUrl = sURL;

        m_bServiceChanged = false;
    }

    m_pPath->SetURL( rURL );

    m_pTreeView->SetSelectHdl( Link<SvTreeListBox*,void>() );

    // read cached data for this url and fill the tree
    const ::std::vector< SvtContentEntry >& rFolders = m_pFileView->GetContent();
    ::std::vector< std::pair< OUString, OUString > > aFolders;

    m_pName_ed->ClearEntries();

    for( ::std::vector< SvtContentEntry >::size_type i = 0; i < rFolders.size(); i++ )
    {
        int nTitleStart = rFolders[i].maURL.lastIndexOf( '/' );
        if( nTitleStart != -1 )
        {
            OUString sTitle( INetURLObject::decode(
                                rFolders[i].maURL.copy( nTitleStart + 1 ),
                                INetURLObject::DECODE_WITH_CHARSET ) );

            if( rFolders[i].mbIsFolder )
            {
                aFolders.push_back( std::pair< OUString, OUString > ( sTitle, rFolders[i].maURL ) );
            }

            // add entries to the autocompletion mechanism
            m_pName_ed->AddEntry( sTitle );
        }
    }

    m_pTreeView->FillTreeEntry( rURL, aFolders );

    m_pTreeView->SetSelectHdl( LINK( this, RemoteFilesDialog, TreeSelectHdl ) );

    m_bIsConnected = true;
    EnableControls();
}

void RemoteFilesDialog::EnableAutocompletion( bool )
{
    // This dialog contains Breadcrumb, not Edit
}

const OUString& RemoteFilesDialog::GetPath()
{
    return m_sPath;
}

std::vector<OUString> RemoteFilesDialog::GetPathList() const
{
    std::vector<OUString> aList;
    sal_uLong nCount = m_pFileView->GetSelectionCount();
    SvTreeListEntry* pEntry = nCount ? m_pFileView->FirstSelected() : NULL;

    while( pEntry )
    {
        // url must contain user info, because we need this info in recent files entry
        // (to fill user field in login box by default)
        INetURLObject aURL( SvtFileView::GetURL( pEntry ) );
        INetURLObject aCurrentURL( m_sLastServiceUrl );
        aURL.SetUser( aCurrentURL.GetUser() );

        aList.push_back( aURL.GetMainURL( INetURLObject::NO_DECODE ) );
        pEntry = m_pFileView->NextSelected( pEntry );
    }

    if( aList.size() == 0 && !m_sPath.isEmpty() )
        aList.push_back( m_sPath );

    return aList;
}

bool RemoteFilesDialog::ContentIsFolder( const OUString& rURL )
{
    try
    {
        Reference< XInteractionHandler > xInteractionHandler(
                        InteractionHandler::createWithParent( m_xContext, 0 ), UNO_QUERY_THROW );
        Reference< XCommandEnvironment > xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        ::ucbhelper::Content aContent( rURL, xEnv, m_xContext );

        return aContent.isFolder();
    }
    catch( const Exception& )
    {
        // a content doesn't exist
    }

    return false;
}

bool RemoteFilesDialog::ContentIsDocument( const OUString& rURL )
{
    try
    {
        Reference< XInteractionHandler > xInteractionHandler(
                        InteractionHandler::createWithParent( m_xContext, 0 ), UNO_QUERY_THROW );
        Reference< XCommandEnvironment > xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
        ::ucbhelper::Content aContent( rURL, xEnv, m_xContext );

        return aContent.isDocument();
    }
    catch( const Exception& )
    {
        // a content doesn't exist
    }

    return false;
}

sal_Int32 RemoteFilesDialog::getTargetColorDepth()
{
    // This dialog doesn't contain preview
    return 0;
}

sal_Int32 RemoteFilesDialog::getAvailableWidth()
{
    // This dialog doesn't contain preview
    return 0;
}

sal_Int32 RemoteFilesDialog::getAvailableHeight()
{
    // This dialog doesn't contain preview
    return 0;
}

void RemoteFilesDialog::setImage( sal_Int16, const ::com::sun::star::uno::Any& )
{
    // This dialog doesn't contain preview
}

bool RemoteFilesDialog::getShowState()
{
    // This dialog doesn't contain preview
    return false;
}

Control* RemoteFilesDialog::getControl( sal_Int16, bool) const
{
    return NULL;
}
void RemoteFilesDialog::enableControl( sal_Int16, bool )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
