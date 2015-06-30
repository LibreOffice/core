/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/RemoteFilesDialog.hxx>
#include "../contnr/contentenumeration.hxx"

class FolderTree : public SvTreeListBox
{
private:
    Reference< XCommandEnvironment > m_xEnv;
    ::osl::Mutex m_aMutex;

public:
    FolderTree( vcl::Window* pParent, WinBits nBits )
        : SvTreeListBox( pParent, nBits )
    {
        Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        Reference< XInteractionHandler > xInteractionHandler(
                    InteractionHandler::createWithParent( xContext, 0 ), UNO_QUERY_THROW );
        m_xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
    }

    virtual void RequestingChildren( SvTreeListEntry* pEntry )
    {
        FillTreeEntry( pEntry );
    }

    void FillTreeEntry( SvTreeListEntry* pEntry )
    {
        // fill only empty entries
        if( pEntry && GetChildCount( pEntry ) == 0 )
        {
            ::std::vector< SortingData_Impl* > aContent;

            FileViewContentEnumerator* pContentEnumerator = new FileViewContentEnumerator(
                m_xEnv, aContent, m_aMutex, NULL );

            OUString* pURL = static_cast< OUString* >( pEntry->GetUserData() );

            if( pURL )
            {
                FolderDescriptor aFolder( *pURL );
                Sequence< OUString > aBlackList;

                EnumerationResult eResult =
                    pContentEnumerator->enumerateFolderContentSync( aFolder, aBlackList );

                if ( SUCCESS == eResult )
                {
                    for( unsigned int i = 0; i < aContent.size(); i++ )
                    {
                        if( aContent[i]->mbIsFolder )
                        {
                            SvTreeListEntry* pNewEntry = InsertEntry( aContent[i]->GetTitle(), pEntry, true );

                            OUString* sData = new OUString( aContent[i]->maTargetURL );
                            pNewEntry->SetUserData( static_cast< void* >( sData ) );
                        }
                    }
                }
            }
        }
    }

    void SetTreePath( OUString sUrl )
    {
        INetURLObject aUrl( sUrl );
        aUrl.setFinalSlash();

        OUString sPath = aUrl.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );

        SvTreeListEntry* pEntry = First();
        bool end = false;

        while( pEntry && !end )
        {
            if( pEntry->GetUserData() )
            {
                OUString sNodeUrl = *static_cast< OUString* >( pEntry->GetUserData() );

                INetURLObject aUrlObj( sNodeUrl );
                aUrlObj.setFinalSlash();

                sNodeUrl = aUrlObj.GetURLPath( INetURLObject::DECODE_WITH_CHARSET );

                if( sPath == sNodeUrl )
                {
                    Select( pEntry );

                    if( !IsExpanded( pEntry ) )
                        Expand( pEntry );

                    end = true;
                }
                else if( sPath.startsWith( sNodeUrl ) )
                {
                    if( !IsExpanded( pEntry ) )
                        Expand( pEntry );

                    pEntry = FirstChild( pEntry );
                }
                else
                {
                    pEntry = NextSibling( pEntry );
                }
            }
            else
                break;
        }
    }
};

class FileViewContainer : public vcl::Window
{
    private:
    VclPtr< SvtFileView > m_pFileView;
    VclPtr< FolderTree > m_pTreeView;
    VclPtr< Splitter > m_pSplitter;

    int m_nCurrentFocus;
    vcl::Window* m_pFocusWidgets[4];

    public:
    FileViewContainer( vcl::Window *pParent )
        : Window( pParent, WB_TABSTOP )
        , m_pFileView( NULL )
        , m_pTreeView( NULL )
        , m_pSplitter( NULL )
    {
    }

    virtual ~FileViewContainer()
    {
        disposeOnce();
    }

    virtual void dispose() SAL_OVERRIDE
    {
        m_pFileView.clear();
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
        m_pFocusWidgets[0] = pPrevSibling;
        m_pFocusWidgets[1] = pTreeView;
        m_pFocusWidgets[2] = pFileView;
        m_pFocusWidgets[3] = pNextSibling;
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
        if( !bReverse && m_nCurrentFocus < 4 )
        {
            m_pFocusWidgets[++m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
        else if( m_nCurrentFocus > 0 )
        {
            m_pFocusWidgets[--m_nCurrentFocus]->SetFakeFocus( true );
            m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
        }
    }

    virtual void GetFocus() SAL_OVERRIDE
    {
        m_nCurrentFocus = 1;
        m_pFocusWidgets[m_nCurrentFocus]->SetFakeFocus( true );
        m_pFocusWidgets[m_nCurrentFocus]->GrabFocus();
    }

    virtual bool Notify( NotifyEvent& rNEvt )
    {
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
    : SvtFileDialog_Base( pParent, "RemoteFilesDialog", "svt/ui/remotefilesdialog.ui" )
    , m_context( comphelper::getProcessComponentContext() )
    , m_aFolderImage( SvtResId( IMG_SVT_FOLDER ) )
    , m_pSplitter( NULL )
    , m_pFileView( NULL )
    , m_pContainer( NULL )
{
    get( m_pOpen_btn, "open" );
    get( m_pSave_btn, "save" );
    get( m_pCancel_btn, "cancel" );
    get( m_pAddService_btn, "add_service_btn" );
    get( m_pServices_lb, "services_lb" );
    get( m_pFilter_lb, "filter_lb" );
    get( m_pName_ed, "name_ed" );

    m_eMode = ( nBits & WB_SAVEAS ) ? REMOTEDLG_MODE_SAVE : REMOTEDLG_MODE_OPEN;
    m_eType = ( nBits & WB_PATH ) ? REMOTEDLG_TYPE_PATHDLG : REMOTEDLG_TYPE_FILEDLG;
    m_bMultiselection = ( nBits & WB_MULTISELECTION ) ? true : false;
    m_bIsUpdated = false;

    m_pOpen_btn->Enable( false );
    m_pSave_btn->Enable( false );
    m_pFilter_lb->Enable( false );
    m_pName_ed->Enable( false );

    if( m_eMode == REMOTEDLG_MODE_OPEN )
    {
        m_pSave_btn->Hide();
        m_pOpen_btn->Show();
    }
    else
    {
        m_pSave_btn->Show();
        m_pOpen_btn->Hide();
    }

    m_pOpen_btn->SetClickHdl( LINK( this, RemoteFilesDialog, OkHdl ) );

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
                                       m_bMultiselection );

    m_pFileView->Show();
    m_pFileView->EnableAutoResize();
    m_pFileView->SetDoubleClickHdl( LINK( this, RemoteFilesDialog, DoubleClickHdl ) );
    m_pFileView->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectHdl ) );

    m_pSplitter = VclPtr< Splitter >::Create( m_pContainer, WB_HSCROLL );
    m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ) );
    m_pSplitter->SetSplitHdl( LINK( this, RemoteFilesDialog, SplitHdl ) );
    m_pSplitter->Show();

    m_pTreeView = VclPtr< FolderTree >::Create( m_pContainer, WB_BORDER | WB_SORT | WB_TABSTOP );
    Size aSize( 100, 200 );
    m_pTreeView->set_height_request( aSize.Height() );
    m_pTreeView->set_width_request( aSize.Width() );
    m_pTreeView->SetSizePixel( aSize );
    m_pTreeView->Show();
    m_pTreeView->SetDefaultCollapsedEntryBmp( m_aFolderImage );
    m_pTreeView->SetDefaultExpandedEntryBmp( m_aFolderImage );

    m_pTreeView->SetSelectHdl( LINK( this, RemoteFilesDialog, TreeSelectHdl ) );

    sal_Int32 nPosX = m_pTreeView->GetSizePixel().Width();
    m_pSplitter->SetPosPixel( Point( nPosX, 0 ) );
    nPosX += m_pSplitter->GetSizePixel().Width();
    m_pFileView->SetPosPixel( Point( nPosX, 0 ) );

    m_pContainer->init( m_pFileView, m_pSplitter, m_pTreeView, m_pAddService_btn, m_pFilter_lb );
    m_pContainer->Show();
    m_pContainer->Enable( false );

    m_pAddService_btn->SetMenuMode( MENUBUTTON_MENUMODE_TIMED );
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
    m_pFileView->SetSelectHdl( Link<>() );

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

        std::shared_ptr< comphelper::ConfigurationChanges > batch( comphelper::ConfigurationChanges::create( m_context ) );
        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set( placesUrlsList, batch );
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set( placesNamesList, batch );
        batch->commit();
    }

    m_pTreeView.disposeAndClear();
    m_pFileView.disposeAndClear();
    m_pSplitter.disposeAndClear();
    m_pContainer.disposeAndClear();
    m_pPath.disposeAndClear();

    m_pOpen_btn.clear();
    m_pSave_btn.clear();
    m_pCancel_btn.clear();
    m_pAddService_btn.clear();
    m_pServices_lb.clear();
    m_pFilter_lb.clear();
    m_pName_ed.clear();

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

void RemoteFilesDialog::FillServicesListbox()
{
    m_pServices_lb->Clear();
    m_aServices.clear();

    // Load from user settings
    Sequence< OUString > placesUrlsList( officecfg::Office::Common::Misc::FilePickerPlacesUrls::get( m_context ) );
    Sequence< OUString > placesNamesList( officecfg::Office::Common::Misc::FilePickerPlacesNames::get( m_context ) );

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

            m_pServices_lb->InsertEntry( sPrefix + placesNamesList[nPlace] );
        }
    }

    if( m_pServices_lb->GetEntryCount() > 0 )
        m_pServices_lb->SelectEntryPos( 0 );
    else
        m_pServices_lb->Enable( false );
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
    m_aFilters.push_back( rType );
    m_pFilter_lb->InsertEntry( rFilter );

    if( m_pFilter_lb->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND )
        m_pFilter_lb->SelectEntryPos( 0 );
}

OUString RemoteFilesDialog::GetPath() const
{
    return m_sPath;
}

OUString RemoteFilesDialog::GetCurrentFilter()
{
    OUString sFilter;

    int nPos = m_pFilter_lb->GetSelectEntryPos();

    if( nPos != LISTBOX_ENTRY_NOTFOUND )
        sFilter = m_aFilters[nPos];
    else
        sFilter = FILTER_ALL;

    return sFilter;
}

FileViewResult RemoteFilesDialog::OpenURL( OUString sURL )
{
    FileViewResult eResult = eFailure;

    if( m_pFileView )
    {
        OUStringList BlackList;
        OUString sFilter = GetCurrentFilter();

        m_pFileView->EndInplaceEditing( false );
        eResult = m_pFileView->Initialize( sURL, sFilter, NULL, BlackList );

        if( eResult == eSuccess )
        {
            m_pPath->SetURL( sURL );
            m_pTreeView->SetTreePath( sURL );
            m_pFilter_lb->Enable( true );
            m_pName_ed->Enable( true );
            m_pContainer->Enable( true );
        }
    }

    return eResult;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, AddServiceHdl )
{
    ScopedVclPtrInstance< PlaceEditDialog > aDlg( this );
    short aRetCode = aDlg->Execute();

    switch( aRetCode )
    {
        case RET_OK :
        {
            ServicePtr newService = aDlg->GetPlace();
            m_aServices.push_back( newService );
            m_pServices_lb->Enable( true );

            OUString sPrefix = lcl_GetServiceType( newService );

            if(!sPrefix.isEmpty())
                 sPrefix += ": ";

            m_pServices_lb->InsertEntry( sPrefix + newService->GetName() );
            m_pServices_lb->SelectEntryPos( m_pServices_lb->GetEntryCount() - 1 );

            m_bIsUpdated = true;
      break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    };

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectServiceHdl )
{
    int nPos = GetSelectedServicePos();

    if( nPos > 0 )
    {
        OUString sURL = m_aServices[nPos]->GetUrl();
        OUString sName = m_aServices[nPos]->GetName();

        if( OpenURL( sURL ) == eSuccess )
        {
            m_pPath->SetRootName( sName );
            m_pTreeView->Clear();

            SvTreeListEntry* pRoot = m_pTreeView->InsertEntry( sName, NULL, true );
            OUString* sData = new OUString( sURL );
            pRoot->SetUserData( static_cast< void* >( sData ) );

            m_pTreeView->Expand( pRoot );
        }
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

        if( nPos > 0 )
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
                    m_pServices_lb->InsertEntry( pEditedService->GetName(), nSelected );
                    m_pServices_lb->SelectEntryPos( nSelected );

                    m_bIsUpdated = true;
                    break;
                }
                case RET_CANCEL :
                default :
                    // Do Nothing
                    break;
            };
        }
    }
    else if( sIdent == "delete_service"  && m_pServices_lb->GetEntryCount() > 0 )
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if( nPos > 0 )
        {
            // TODO: Confirm dialog

            m_aServices.erase( m_aServices.begin() + nPos );
            m_pServices_lb->RemoveEntry( nSelected );

            if( m_pServices_lb->GetEntryCount() > 0 )
            {
                m_pServices_lb->SelectEntryPos( 0 );
            }
            else
            {
                m_pServices_lb->SetNoSelection();
                m_pServices_lb->Enable( false );
            }

            m_bIsUpdated = true;
        }
    }
}

IMPL_LINK_NOARG ( RemoteFilesDialog, DoubleClickHdl )
{
    SvTreeListEntry* pEntry = m_pFileView->FirstSelected();
    SvtContentEntry* pData = static_cast< SvtContentEntry* >( pEntry->GetUserData() );

    if( pData->mbIsFolder )
    {
        OUString sURL = m_pFileView->GetCurrentURL();

        OpenURL( sURL );
    }
    else
    {
        EndDialog( RET_OK );
    }

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectHdl )
{
    SvTreeListEntry* pEntry = m_pFileView->FirstSelected();
    SvtContentEntry* pData = static_cast< SvtContentEntry* >( pEntry->GetUserData() );

    if( ( pData->mbIsFolder && ( m_eType == REMOTEDLG_TYPE_PATHDLG ) )
       || ( !pData->mbIsFolder && ( m_eType == REMOTEDLG_TYPE_FILEDLG ) ) )
    {
        INetURLObject aURL( pData->maURL );
        m_sPath = pData->maURL;

        m_pName_ed->SetText( INetURLObject::decode( aURL.GetLastName(), INetURLObject::DECODE_WITH_CHARSET ) );

        m_pOpen_btn->Enable( true );
    }
    else
    {
        m_pOpen_btn->Enable( false );
        m_sPath = "";
        m_pName_ed->SetText( "" );
    }

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SplitHdl )
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

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectFilterHdl )
{
    OUString sCurrentURL = m_pFileView->GetViewURL();

    if( !sCurrentURL.isEmpty() )
        OpenURL( sCurrentURL );

    return 1;
}

IMPL_LINK ( RemoteFilesDialog, TreeSelectHdl, FolderTree *, pBox )
{
    OUString* sURL = static_cast< OUString* >( pBox->GetHdlEntry()->GetUserData() );

    if( sURL )
        OpenURL( *sURL );

    return 1;
}

IMPL_LINK ( RemoteFilesDialog, SelectBreadcrumbHdl, Breadcrumb*, pPtr )
{
    if( pPtr )
    {
        OpenURL( pPtr->GetHdlURL() );
    }

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, OkHdl )
{
    EndDialog( RET_OK );
    return 1;
}

// SvtFileDialog_Base

SvtFileView* RemoteFilesDialog::GetView()
{
    return m_pFileView;
}

void RemoteFilesDialog::SetHasFilename( bool bHasFilename )
{
    // TODO
}

void RemoteFilesDialog::SetBlackList( const ::com::sun::star::uno::Sequence< OUString >& rBlackList )
{
    // TODO
}

const ::com::sun::star::uno::Sequence< OUString >& RemoteFilesDialog::GetBlackList() const
{
    // TODO
    ::com::sun::star::uno::Sequence< OUString > aSequence( 0 );
    return aSequence;
}

void RemoteFilesDialog::SetStandardDir( const OUString& rStdDir )
{
    // TODO
}

const OUString& RemoteFilesDialog::GetStandardDir() const
{
    // TODO
    return OUString( "" );
}

void RemoteFilesDialog::SetPath( const OUString& rNewURL )
{
    m_sPath = rNewURL;
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

void RemoteFilesDialog::SetCurFilter( const OUString& rFilter )
{
    // TODO
}

void RemoteFilesDialog::SetFileCallback( ::svt::IFilePickerListener *pNotifier )
{
    // TODO
}

void RemoteFilesDialog::EnableAutocompletion( bool _bEnable )
{
    // TODO
}

const OUString& RemoteFilesDialog::GetPath()
{
    return m_sPath;
}

std::vector<OUString> RemoteFilesDialog::GetPathList() const
{
    std::vector<OUString> aPaths;
    aPaths.push_back(m_sPath);
    return aPaths;
}

bool RemoteFilesDialog::ContentIsFolder( const OUString& rURL )
{
    // TODO
    return false;
}

sal_Int32 RemoteFilesDialog::getTargetColorDepth()
{
    // TODO
    return 0;
}

sal_Int32 RemoteFilesDialog::getAvailableWidth()
{
    // TODO
    return 0;
}

sal_Int32 RemoteFilesDialog::getAvailableHeight()
{
    // TODO
    return 0;
}

void RemoteFilesDialog::setImage( sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& rImage )
{
    // TODO
}

bool RemoteFilesDialog::getShowState()
{
    // TODO
    return false;
}

OUString RemoteFilesDialog::GetCurFilter() const
{
    // TODO
    return OUString( "" );
}

Control* RemoteFilesDialog::getControl( sal_Int16 _nControlId, bool _bLabelControl) const
{
    // TODO
    return NULL;
}
void RemoteFilesDialog::enableControl( sal_Int16 _nControlId, bool _bEnable )
{
    // TODO
}

OUString RemoteFilesDialog::getCurFilter( ) const
{
    // TODO
    return OUString("");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
