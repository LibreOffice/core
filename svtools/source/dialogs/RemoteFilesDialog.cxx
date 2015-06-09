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

class FileViewContainer : public vcl::Window
{
    private:
    VclPtr<SvtFileView> m_pFileView;
    VclPtr<SvTreeListBox> m_pTreeView;
    VclPtr<Splitter> m_pSplitter;

    public:
    FileViewContainer(vcl::Window *pParent)
        : Window(pParent)
        , m_pFileView(NULL)
        , m_pTreeView(NULL)
        , m_pSplitter(NULL)
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

    void init(SvtFileView* pFileView,
              Splitter* pSplitter,
              SvTreeListBox* pTreeView)
    {
        m_pFileView = pFileView;
        m_pTreeView = pTreeView;
        m_pSplitter = pSplitter;
    }

    virtual void Resize() SAL_OVERRIDE
    {
        Window::Resize();

        if(!m_pFileView || !m_pTreeView)
            return;

        Size aSize = GetSizePixel();
        Point aPos( m_pFileView->GetPosPixel() );
        Size aNewSize(aSize.Width() - aPos.X(), aSize.Height());

        m_pFileView->SetSizePixel( aNewSize );

        // Resize the Splitter to fit the height
        Size splitterNewSize = m_pSplitter->GetSizePixel( );
        splitterNewSize.Height() = aSize.Height();
        m_pSplitter->SetSizePixel( splitterNewSize );
        sal_Int32 nMinX = m_pTreeView->GetPosPixel( ).X( );
        sal_Int32 nMaxX = m_pFileView->GetPosPixel( ).X( ) + m_pFileView->GetSizePixel( ).Width() - nMinX;
        m_pSplitter->SetDragRectPixel( Rectangle( Point( nMinX, 0 ), Size( nMaxX, aSize.Width() ) ) );

        // Resize the tree list box to fit the height of the FileView
        Size placesNewSize(m_pTreeView->GetSizePixel());
        placesNewSize.Height() = aSize.Height();
        m_pTreeView->SetSizePixel( placesNewSize );
    }
};

class Breadcrumb : public VclHBox
{
    private:
    const unsigned int m_cCount = 4; // how many labels we have - temporary

    std::vector< VclPtr< FixedHyperlink > > m_aLinks;
    std::vector< VclPtr< FixedText > > m_aSeparators;

    OUString m_sPath;
    OUString m_sClickedURL;

    Link<> m_aClickHdl;

    DECL_LINK ( ClickLinkHdl, FixedHyperlink* );

    public:
    Breadcrumb( vcl::Window* pParent ) : VclHBox( pParent )
    {
        set_spacing( 6 );

        for(unsigned int i = 0; i < m_cCount; i++)
        {
            m_aLinks.push_back( VclPtr< FixedHyperlink >::Create( this ) );
            m_aLinks[i]->Hide();
            m_aLinks[i]->SetClickHdl( LINK( this, Breadcrumb, ClickLinkHdl ) );
            m_aSeparators.push_back( VclPtr< FixedText >::Create( this ) );
            m_aSeparators[i]->SetText( ">" );
            m_aSeparators[i]->Hide();
        }
    }

    ~Breadcrumb()
    {
        disposeOnce();
    }

    void dispose()
    {
        for(unsigned int i = 0; i < m_cCount; i++)
        {
            m_aSeparators[i].disposeAndClear();
            m_aLinks[i].disposeAndClear();
        }

        VclHBox::dispose();
    }

    void SetClickHdl( const Link<>& rLink )
    {
        m_aClickHdl = rLink;
    }

    OUString GetHdlURL()
    {
        return m_sClickedURL;
    }

    void SetURL( const OUString& rURL )
    {
        m_sPath = rURL;
        INetURLObject aURL( rURL );
        aURL.setFinalSlash();
        OUString sPath = aURL.GetURLPath(INetURLObject::DECODE_WITH_CHARSET);

        unsigned int nSegments = aURL.getSegmentCount();
        unsigned int nPos = 0;
        unsigned int i;

        m_aLinks[0]->SetText( "Root" );
        m_aLinks[0]->Show();
        m_aLinks[0]->SetURL( INetURLObject::GetScheme( aURL.GetProtocol() )
                                + aURL.GetHost() );
        m_aSeparators[0]->Show();

        for(i = 1; i < m_cCount && i < nSegments + 1; i++)
        {
            unsigned int nEnd = sPath.indexOf( '/', nPos + 1 );
            OUString sLabel = OUString( sPath.getStr() + nPos + 1, nEnd - nPos - 1 );

            m_aLinks[i]->SetText( sLabel );
            m_aLinks[i]->SetURL( INetURLObject::GetScheme( aURL.GetProtocol() )
                                + aURL.GetHost()
                                + OUString( sPath.getStr(), nEnd ) );
            m_aLinks[i]->Show();
            m_aSeparators[i]->Show();

            nPos = nEnd;
        }
        for(; i < m_cCount; i++)
        {
            m_aLinks[i]->Hide();
            m_aSeparators[i]->Hide();
        }
    }
};

IMPL_LINK ( Breadcrumb, ClickLinkHdl, FixedHyperlink*, pLink )
{
    m_sClickedURL = pLink->GetURL();
    m_aClickHdl.Call( this );

    return 1;
}

RemoteFilesDialog::RemoteFilesDialog(vcl::Window* pParent, WinBits nBits)
    : ModalDialog(pParent, "RemoteFilesDialog", "svt/ui/remotefilesdialog.ui")
    , m_context(comphelper::getProcessComponentContext())
    , m_aFolderImage(SvtResId(IMG_SVT_FOLDER))
    , m_pSplitter(NULL)
    , m_pFileView(NULL)
    , m_pContainer(NULL)
{
    get(m_pOpen_btn, "open");
    get(m_pSave_btn, "save");
    get(m_pCancel_btn, "cancel");
    get(m_pAddService_btn, "add_service_btn");
    get(m_pServices_lb, "services_lb");
    get(m_pFilter_lb, "filter_lb");
    get(m_pName_ed, "name_ed");

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    Reference< XInteractionHandler > xInteractionHandler(
                    InteractionHandler::createWithParent(xContext, 0), UNO_QUERY_THROW );
    m_xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );

    m_eMode = (nBits & WB_SAVEAS) ? REMOTEDLG_MODE_SAVE : REMOTEDLG_MODE_OPEN;
    m_eType = (nBits & WB_PATH) ? REMOTEDLG_TYPE_PATHDLG : REMOTEDLG_TYPE_FILEDLG;
    m_bMultiselection = (nBits & WB_MULTISELECTION) ? true : false;
    m_bIsUpdated = false;

    m_pOpen_btn->Enable( false );
    m_pSave_btn->Enable( false );
    m_pFilter_lb->Enable( false );
    m_pName_ed->Enable( false );

    if(m_eMode == REMOTEDLG_MODE_OPEN)
    {
        m_pSave_btn->Hide();
        m_pOpen_btn->Show();
    }
    else
    {
        m_pSave_btn->Show();
        m_pOpen_btn->Hide();
    }

    m_pPath = VclPtr<Breadcrumb>::Create( get<vcl::Window>("breadcrumb_container") );
    m_pPath->set_hexpand(true);
    m_pPath->SetClickHdl( LINK( this, RemoteFilesDialog, SelectBreadcrumbHdl ) );
    m_pPath->Show();

    m_pContainer = VclPtr<FileViewContainer>::Create( get<vcl::Window>("container") );

    m_pContainer->set_hexpand(true);
    m_pContainer->set_vexpand(true);

    m_pFileView = VclPtr<SvtFileView>::Create( m_pContainer, WB_BORDER,
                                       REMOTEDLG_TYPE_PATHDLG == m_eType,
                                       m_bMultiselection );

    m_pFileView->Show();
    m_pFileView->EnableAutoResize();
    m_pFileView->SetDoubleClickHdl( LINK( this, RemoteFilesDialog, DoubleClickHdl ) );
    m_pFileView->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectHdl ) );

    m_pSplitter = VclPtr<Splitter>::Create( m_pContainer, WB_HSCROLL );
    m_pSplitter->SetBackground( Wallpaper( Application::GetSettings().GetStyleSettings().GetFaceColor() ));
    m_pSplitter->SetSplitHdl( LINK( this, RemoteFilesDialog, SplitHdl ) );
    m_pSplitter->Show();

    m_pTreeView = VclPtr<SvTreeListBox>::Create( m_pContainer, WB_BORDER );
    Size aSize(100, 200);
    m_pTreeView->set_height_request(aSize.Height());
    m_pTreeView->set_width_request(aSize.Width());
    m_pTreeView->SetSizePixel(aSize);
    m_pTreeView->Show();
    m_pTreeView->SetDefaultCollapsedEntryBmp(m_aFolderImage);
    m_pTreeView->SetDefaultExpandedEntryBmp(m_aFolderImage);

    m_pTreeView->SetSelectHdl( LINK( this, RemoteFilesDialog, TreeSelectHdl ) );
    m_pTreeView->SetExpandedHdl( LINK( this, RemoteFilesDialog, TreeExpandHdl ) );

    sal_Int32 nPosX = m_pTreeView->GetSizePixel().Width();
    m_pSplitter->SetPosPixel(Point(nPosX, 0));
    nPosX += m_pSplitter->GetSizePixel().Width();
    m_pFileView->SetPosPixel(Point(nPosX, 0));

    m_pContainer->init(m_pFileView, m_pSplitter, m_pTreeView);
    m_pContainer->Show();
    m_pContainer->Enable( false );

    m_pAddService_btn->SetMenuMode(MENUBUTTON_MENUMODE_TIMED);
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

    if(m_bIsUpdated)
    {
        Sequence< OUString > placesUrlsList(m_aServices.size());
        Sequence< OUString > placesNamesList(m_aServices.size());

        int i = 0;
        for(std::vector<ServicePtr>::const_iterator it = m_aServices.begin(); it != m_aServices.end(); ++it)
        {
            placesUrlsList[i] = (*it)->GetUrl();
            placesNamesList[i] = (*it)->GetName();
            ++i;
        }

        std::shared_ptr<comphelper::ConfigurationChanges> batch(comphelper::ConfigurationChanges::create(m_context));
        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set(placesUrlsList, batch);
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set(placesNamesList, batch);
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

    if(m_pFileView && m_pContainer)
    {
        Size aSize = m_pContainer->GetSizePixel();
        m_pFileView->SetSizePixel(aSize);
    }
}

void RemoteFilesDialog::FillServicesListbox()
{
    m_pServices_lb->Clear();
    m_aServices.clear();

    // Load from user settings
    Sequence< OUString > placesUrlsList(officecfg::Office::Common::Misc::FilePickerPlacesUrls::get(m_context));
    Sequence< OUString > placesNamesList(officecfg::Office::Common::Misc::FilePickerPlacesNames::get(m_context));

    for(sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace)
    {
        ServicePtr pService(new Place(placesNamesList[nPlace], placesUrlsList[nPlace], true));
        m_aServices.push_back(pService);

        // Add to the listbox only remote services, not local bookmarks
        if(!pService->IsLocal())
        {
            m_pServices_lb->InsertEntry(placesNamesList[nPlace]);
        }
    }

    if(m_pServices_lb->GetEntryCount() > 0)
        m_pServices_lb->SelectEntryPos(0);
    else
        m_pServices_lb->Enable(false);
}

int RemoteFilesDialog::GetSelectedServicePos()
{
    int nSelected = m_pServices_lb->GetSelectEntryPos();
    int nPos = 0;
    int i = -1;

    if(m_aServices.size() == 0)
        return -1;

    while(nPos < (int)m_aServices.size())
    {
        while(m_aServices[nPos]->IsLocal())
            nPos++;
        i++;
        if(i == nSelected)
            break;
        nPos++;
    }

    return nPos;
}

void RemoteFilesDialog::AddFilter( OUString sName, OUString sType )
{
    m_aFilters.push_back( sType );
    m_pFilter_lb->InsertEntry( sName );

    if(m_pFilter_lb->GetSelectEntryPos() == LISTBOX_ENTRY_NOTFOUND)
        m_pFilter_lb->SelectEntryPos( 0 );
}

OUString RemoteFilesDialog::getCurrentFilter()
{
    OUString sFilter;

    int nPos = m_pFilter_lb->GetSelectEntryPos();

    if(nPos != LISTBOX_ENTRY_NOTFOUND)
        sFilter = m_aFilters[nPos];
    else
        sFilter = FILTER_ALL;

    return sFilter;
}

FileViewResult RemoteFilesDialog::OpenURL( OUString sURL )
{
    FileViewResult eResult = eFailure;

    if(m_pFileView)
    {
        OUStringList BlackList;
        OUString sFilter = getCurrentFilter();

        m_pFileView->EndInplaceEditing( false );
        eResult = m_pFileView->Initialize( sURL, sFilter, NULL, BlackList );

        if( eResult == eSuccess )
        {
            m_pPath->SetURL( sURL );
            m_pFilter_lb->Enable( true );
            m_pName_ed->Enable( true );
            m_pContainer->Enable( true );
        }
    }

    return eResult;
}

void RemoteFilesDialog::fillTreeEntry( SvTreeListEntry* pParent )
{
    if( pParent && m_pTreeView->IsExpanded( pParent ) )
    {
        // remove childs

        if ( pParent )
        {
            SvTreeList* pModel = m_pTreeView->GetModel();

            if( pModel->HasChildren( pParent ) )
            {
                SvTreeListEntries& rEntries = pModel->GetChildList( pParent );
                rEntries.clear();
            }
        }

        // fill with new ones

        ::std::vector< SortingData_Impl* > aContent;

        FileViewContentEnumerator* pContentEnumerator = new FileViewContentEnumerator(
            m_xEnv, aContent, m_aMutex, NULL );

        OUString* pURL = static_cast< OUString* >( pParent->GetUserData() );

        if( pURL )
        {
            FolderDescriptor aFolder( *pURL );
            Sequence< OUString > aBlackList;

            EnumerationResult eResult =
                pContentEnumerator->enumerateFolderContentSync( aFolder, aBlackList );

            if ( SUCCESS == eResult )
            {
                unsigned int nChilds = 0;

                for( unsigned int i = 0; i < aContent.size(); i++ )
                {
                    if( aContent[i]->mbIsFolder )
                    {
                        SvTreeListEntry* pEntry = m_pTreeView->InsertEntry( aContent[i]->GetTitle(), pParent, true );

                        OUString* sData = new OUString( aContent[i]->maTargetURL );
                        pEntry->SetUserData( static_cast< void* >( sData ) );

                        nChilds++;
                    }
                }

                if( nChilds == 0 )
                {
                    m_pTreeView->Collapse( pParent );
                }
            }
        }
    }
}

IMPL_LINK_NOARG ( RemoteFilesDialog, AddServiceHdl )
{
    ScopedVclPtrInstance< PlaceEditDialog > aDlg(this);
    short aRetCode = aDlg->Execute();

    switch(aRetCode)
    {
        case RET_OK :
        {
            ServicePtr newService = aDlg->GetPlace();
            m_aServices.push_back(newService);
            m_pServices_lb->Enable(true);
            m_pServices_lb->InsertEntry(newService->GetName());
            m_pServices_lb->SelectEntryPos(m_pServices_lb->GetEntryCount() - 1);

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

    if(nPos > 0)
    {
        OUString sURL = m_aServices[nPos]->GetUrl();

        if( OpenURL( sURL ) == eSuccess )
        {
            m_pTreeView->Clear();

            SvTreeListEntry* pRoot = m_pTreeView->InsertEntry( m_pServices_lb->GetSelectEntry(), NULL, true );
            OUString* sData = new OUString( sURL );
            pRoot->SetUserData( static_cast< void* >( sData ) );

            m_pTreeView->Expand( pRoot );
        }
    }

    return 1;
}

IMPL_LINK_TYPED ( RemoteFilesDialog, EditServiceMenuHdl, MenuButton *, pButton, void )
{
    OString sIdent(pButton->GetCurItemIdent());
    if(sIdent == "edit_service"  && m_pServices_lb->GetEntryCount() > 0)
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if(nPos > 0)
        {
            ScopedVclPtrInstance< PlaceEditDialog > aDlg(this, m_aServices[nPos]);
            short aRetCode = aDlg->Execute();

            switch(aRetCode)
            {
                case RET_OK :
                {
                    ServicePtr pEditedService = aDlg->GetPlace();

                    m_aServices[nPos] = pEditedService;
                    m_pServices_lb->RemoveEntry(nSelected);
                    m_pServices_lb->InsertEntry(pEditedService->GetName(), nSelected);
                    m_pServices_lb->SelectEntryPos(nSelected);

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
    else if(sIdent == "delete_service"  && m_pServices_lb->GetEntryCount() > 0)
    {
        unsigned int nSelected = m_pServices_lb->GetSelectEntryPos();
        int nPos = GetSelectedServicePos();

        if(nPos > 0)
        {
            // TODO: Confirm dialog

            m_aServices.erase(m_aServices.begin() + nPos);
            m_pServices_lb->RemoveEntry(nSelected);

            if(m_pServices_lb->GetEntryCount() > 0)
            {
                m_pServices_lb->SelectEntryPos(0);
            }
            else
            {
                m_pServices_lb->SetNoSelection();
                m_pServices_lb->Enable(false);
            }

            m_bIsUpdated = true;
        }
    }
}

IMPL_LINK_NOARG ( RemoteFilesDialog, DoubleClickHdl )
{
    OUString sURL = m_pFileView->GetCurrentURL();

    OpenURL( sURL );

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SelectHdl )
{
    SvTreeListEntry* pEntry = m_pFileView->FirstSelected();
    SvtContentEntry* pData = static_cast<SvtContentEntry*>(pEntry->GetUserData());

    INetURLObject aURL(pData->maURL);
    m_pName_ed->SetText(aURL.GetLastName());

    return 1;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, SplitHdl )
{
    sal_Int32 nSplitPos = m_pSplitter->GetSplitPosPixel();

    // Resize the tree list box
    sal_Int32 nPlaceX = m_pTreeView->GetPosPixel( ).X();
    Size placeSize = m_pTreeView->GetSizePixel( );
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

IMPL_LINK ( RemoteFilesDialog, TreeSelectHdl, SvTreeListBox *, pBox )
{
    OUString* sURL = static_cast< OUString* >( pBox->GetHdlEntry()->GetUserData() );

    if( sURL )
        OpenURL( *sURL );

    return 1;
}

IMPL_LINK ( RemoteFilesDialog, TreeExpandHdl, SvTreeListBox *, pBox )
{
    fillTreeEntry( pBox->GetHdlEntry() );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
