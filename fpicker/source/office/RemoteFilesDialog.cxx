/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_oauth2.h>

#include "fpsmartcontent.hxx"
#include "QueryFolderName.hxx"
#include "RemoteFilesDialog.hxx"
#include <fpsofficeResMgr.hxx>
#include <fpicker/strings.hrc>
#include <strings.hrc>
#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/stillreadwriteinteraction.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <svtools/PlaceEditDialog.hxx>
#include <tools/debug.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <vcl/errinf.hxx>
#include <officecfg/Office/Common.hxx>

using namespace ::svt;

RemoteFilesDialog::RemoteFilesDialog( weld::Window* pParent, PickerFlags nBits )
    : SvtFileDialog_Base( pParent, "fps/ui/remotefilesdialog.ui", "RemoteFilesDialog" )
    , m_xContext( comphelper::getProcessComponentContext() )
    , m_xMasterPasswd( PasswordContainer::create( m_xContext ) )
    , m_bIsInExecute( false )
    , m_xCancel_btn(m_xBuilder->weld_button("cancel"))
    , m_xManageServices(m_xBuilder->weld_menu_button("add_service_btn"))
    , m_xServices_lb(m_xBuilder->weld_combo_box("services_lb"))
    , m_xPathContainer(m_xBuilder->weld_container("breadcrumb_container"))
    , m_xNewFolder(m_xBuilder->weld_button("new_folder"))
    , m_xListView_btn(m_xBuilder->weld_toggle_button("list_view"))
    , m_xIconView_btn(m_xBuilder->weld_toggle_button("icon_view"))
    , m_xFilter_lb(m_xBuilder->weld_combo_box("filter_lb"))
    , m_xName_ed(new AutocompleteEdit(m_xBuilder->weld_entry("filename")))
{
    m_eMode = ( nBits & PickerFlags::SaveAs ) ? REMOTEDLG_MODE_SAVE : REMOTEDLG_MODE_OPEN;
    m_eType = ( nBits & PickerFlags::PathDialog ) ? REMOTEDLG_TYPE_PATHDLG : REMOTEDLG_TYPE_FILEDLG;
    bool bMultiselection = bool( nBits & PickerFlags::MultiSelection );
    m_bIsUpdated = false;
    m_bIsConnected = false;
    m_bServiceChanged = false;
    m_nCurrentFilter = -1;

    m_xName_ed->show();

    // limit width due to super wide strings that may end up here
    m_xFilter_lb->set_size_request(m_xFilter_lb->get_approximate_digit_width() * 60, -1);

    m_xFilter_lb->set_sensitive(false);
    m_xName_ed->set_sensitive(false);
    m_xNewFolder->set_sensitive(false);

    if( m_eMode == REMOTEDLG_MODE_OPEN )
    {
        m_xOk_btn = m_xBuilder->weld_button("open");

        m_xNewFolder->hide();
    }
    else
    {
        m_xOk_btn = m_xBuilder->weld_button("save");
        m_xNewFolder->connect_clicked( LINK( this, RemoteFilesDialog, NewFolderHdl ) );
    }

    m_xListView_btn->set_active(true);
    m_xIconView_btn->connect_clicked( LINK( this, RemoteFilesDialog, IconViewHdl ) );
    m_xListView_btn->connect_clicked( LINK( this, RemoteFilesDialog, ListViewHdl ) );

    m_xOk_btn->show();
    m_xOk_btn->set_sensitive(false);

    m_xOk_btn->connect_clicked( LINK( this, RemoteFilesDialog, OkHdl ) );
    m_xCancel_btn->connect_clicked( LINK( this, RemoteFilesDialog, CancelHdl ) );

    m_sRootLabel = FpsResId( STR_SVT_ROOTLABEL );
    m_xPath.reset(new Breadcrumb(m_xPathContainer.get()));
    m_xPath->connect_clicked( LINK( this, RemoteFilesDialog, SelectBreadcrumbHdl ) );
    m_xPath->SetMode( SvtBreadcrumbMode::ALL_VISITED );

    m_xContainer = m_xBuilder->weld_container("container");
    m_xContainer->set_size_request(m_xContainer->get_approximate_digit_width() * 82, -1);

    m_xFileView.reset(new SvtFileView(m_xDialog.get(),
                                      m_xBuilder->weld_tree_view("fileview"),
                                      m_xBuilder->weld_icon_view("iconview"),
                                      REMOTEDLG_TYPE_PATHDLG == m_eType,
                                      bMultiselection, false));

    m_xFileView->SetDoubleClickHdl( LINK( this, RemoteFilesDialog, DoubleClickHdl ) );
    m_xFileView->SetSelectHdl( LINK( this, RemoteFilesDialog, SelectHdl ) );
    m_xFileView->EnableDelete( true );

    m_xTreeView.reset(new FolderTree(m_xBuilder->weld_tree_view("foldertree"), m_xDialog.get()));
    m_xTreeView->connect_changed(LINK(this, RemoteFilesDialog, TreeSelectHdl));

    m_xContainer->set_sensitive(false);

    m_sIniKey = "RemoteFilesDialog";
    InitSize();

    m_xName_ed->connect_focus_in(LINK(this, RemoteFilesDialog, FileNameGetFocusHdl));
    m_xName_ed->connect_changed(LINK(this, RemoteFilesDialog, FileNameModifyHdl));

    m_xManageServices->connect_selected(LINK(this, RemoteFilesDialog, EditServiceMenuHdl));

    FillServicesListbox();

    m_xServices_lb->connect_changed( LINK( this, RemoteFilesDialog, SelectServiceHdl ) );

    m_xFilter_lb->connect_changed( LINK( this, RemoteFilesDialog, SelectFilterHdl ) );
}

RemoteFilesDialog::~RemoteFilesDialog()
{
    m_xFileView->SetSelectHdl(Link<SvtFileView*,void>());

    // save window state
    if( !m_sIniKey.isEmpty() )
    {
        SvtViewOptions aDlgOpt( EViewType::Dialog, m_sIniKey );
        aDlgOpt.SetWindowState(OStringToOUString(m_xDialog->get_window_state(WindowStateMask::All), RTL_TEXTENCODING_UTF8));

        Size aSize(m_xDialog->get_size());

        OUString sSize = OUString::number( aSize.Width() ) + "|";
        sSize = sSize + OUString::number( aSize.Height() ) + "|";

        OUString sUserData = m_xFileView->GetConfigString();
        aDlgOpt.SetUserItem( "UserData",
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
        for (auto const& service : m_aServices)
        {
            placesUrlsList[i] = service->GetUrl();
            placesNamesList[i] = service->GetName();
            ++i;
        }

        officecfg::Office::Common::Misc::FilePickerPlacesUrls::set( placesUrlsList, batch );
        officecfg::Office::Common::Misc::FilePickerPlacesNames::set( placesNamesList, batch );
    }

    batch->commit();
}

void RemoteFilesDialog::EnableExtraMenuItems(bool bEnable)
{
    m_xManageServices->set_item_visible("change_password", bEnable);
    m_xManageServices->set_item_visible("edit_service", bEnable);
    m_xManageServices->set_item_visible("delete_service", bEnable);
    m_xManageServices->set_item_visible("change_password", bEnable);
}

short RemoteFilesDialog::run()
{
    if (m_xServices_lb->get_count() > 0)
    {
        m_xDialog->show();
        SelectServiceHdl(*m_xServices_lb);
    }
    if (!m_bIsConnected)
    {
        m_xServices_lb->set_active(-1);
        EnableExtraMenuItems(false);
    }

    m_bIsInExecute = true;
    short nRet = SvtFileDialog_Base::run();
    m_bIsInExecute = false;
    return nRet;
}

static OUString lcl_GetServiceType( const ServicePtr& pService )
{
    INetProtocol aProtocol = pService->GetUrlObject().GetProtocol();
    switch( aProtocol )
    {
        case INetProtocol::Ftp:
            return "FTP";
        case INetProtocol::Cmis:
        {
            OUString sHost = pService->GetUrlObject().GetHost( INetURLObject::DecodeMechanism::WithCharset );

            if( sHost.startsWith( GDRIVE_BASE_URL ) )
                return "Google Drive";
            else if( sHost.startsWith( ALFRESCO_CLOUD_BASE_URL ) )
                return "Alfresco Cloud";
            else if( sHost.startsWith( ONEDRIVE_BASE_URL ) )
                return "OneDrive";

            return "CMIS";
        }
        case INetProtocol::Smb:
            return "Windows Share";
        case INetProtocol::File:
            return "SSH";
        case INetProtocol::Http:
            return "WebDAV";
        case INetProtocol::Https:
            return "WebDAV";
        case INetProtocol::Generic:
            return "SSH";
        default:
            return OUString();
    }
}

void RemoteFilesDialog::InitSize()
{
    if( m_sIniKey.isEmpty() )
        return;

    // initialize from config
    SvtViewOptions aDlgOpt( EViewType::Dialog, m_sIniKey );

    if( !aDlgOpt.Exists() )
        return;

    m_xDialog->set_window_state(OUStringToOString(aDlgOpt.GetWindowState(), RTL_TEXTENCODING_UTF8));

    Any aUserData = aDlgOpt.GetUserItem( "UserData" );
    OUString sCfgStr;
    if( aUserData >>= sCfgStr )
    {
        sal_Int32 nPos1{ sCfgStr.indexOf('|') };
        if (nPos1<0)
            return;
        sal_Int32 nPos2{ sCfgStr.indexOf('|', nPos1+1 ) };
        if (nPos2<0)
            return;
        m_xFileView->SetConfigString( sCfgStr.copy(nPos2+1) );
    }
}

void RemoteFilesDialog::FillServicesListbox()
{
    m_xServices_lb->clear();
    m_aServices.clear();

    // Load from user settings
    Sequence< OUString > placesUrlsList( officecfg::Office::Common::Misc::FilePickerPlacesUrls::get( m_xContext ) );
    Sequence< OUString > placesNamesList( officecfg::Office::Common::Misc::FilePickerPlacesNames::get( m_xContext ) );

    unsigned int nPos = 0;
    unsigned int i = 0;

    m_sLastServiceUrl = officecfg::Office::Common::Misc::FilePickerLastService::get( m_xContext );

    for( sal_Int32 nPlace = 0; nPlace < placesUrlsList.getLength() && nPlace < placesNamesList.getLength(); ++nPlace )
    {
        ServicePtr pService = std::make_shared<Place>( placesNamesList[nPlace], placesUrlsList[nPlace], true );
        m_aServices.push_back( pService );

        // Add to the listbox only remote services, not local bookmarks
        if( !pService->IsLocal() )
        {
            OUString sPrefix = lcl_GetServiceType( pService );

            if( !sPrefix.isEmpty() )
                 sPrefix += ": ";

            if( placesUrlsList[nPlace] == m_sLastServiceUrl )
                nPos = i;

            m_xServices_lb->append_text(sPrefix + placesNamesList[nPlace]);

            i++;
        }
    }

    if (m_xServices_lb->get_count() > 0)
    {
        m_xServices_lb->set_active(nPos);
        EnableExtraMenuItems(true);
    }
    else
        EnableExtraMenuItems(false);

    EnableControls();
}

int RemoteFilesDialog::GetSelectedServicePos()
{
    if( m_aServices.empty() )
        return -1;

    int nPos = 0;
    int i = -1;

    int nSelected = m_xServices_lb->get_active();

    int nServices = static_cast<int>(m_aServices.size());
    while( nPos < nServices )
    {
        while( (nPos < nServices) && m_aServices[nPos]->IsLocal() )
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

    m_aFilters.emplace_back( rFilter, rType );
    if (rType.isEmpty())
        m_xFilter_lb->append_separator("");
    else
        m_xFilter_lb->append_text(sName);

    if (m_xFilter_lb->get_active() == -1)
        m_xFilter_lb->set_active(0);
}

void RemoteFilesDialog::OpenURL( OUString const & sURL )
{
    if( !m_xFileView )
        return;

    DisableControls();

    auto xWait = std::make_unique<weld::WaitObject>(m_xDialog.get());

    if( !sURL.isEmpty() )
    {
        OUString sFilter = FILEDIALOG_FILTER_ALL;

        if( m_nCurrentFilter != -1)
        {
            sFilter = m_aFilters[m_nCurrentFilter].second;
        }

        m_xFileView->EndInplaceEditing();

        DBG_ASSERT( !m_pCurrentAsyncAction.is(), "SvtFileDialog::executeAsync: previous async action not yet finished!" );

        m_pCurrentAsyncAction = new AsyncPickerAction( this, m_xFileView.get(), AsyncPickerAction::Action::eOpenURL );

        // -1 timeout - sync
        m_pCurrentAsyncAction->execute( sURL, sFilter, -1, -1, GetDenyList() );

        if( m_eMode != REMOTEDLG_MODE_SAVE )
            m_xName_ed->set_text( "" );

        m_xFileView->grab_focus();
    }
    else
    {
        xWait.reset();

        // content doesn't exist
        ErrorHandler::HandleError( ERRCODE_IO_NOTEXISTS );

        EnableControls();
    }
}

void RemoteFilesDialog::AddFileExtension()
{
    if (m_nCurrentFilter == -1)
        return;

    OUString sExt = m_aFilters[m_nCurrentFilter].second;
    OUString sFileName = m_xName_ed->get_text();

    sal_Int32 nDotPos = sFileName.lastIndexOf( '.' );

    if ( nDotPos == -1 )
    {
        sFileName += sExt.subView( 1 ); // without '*'
        m_xName_ed->set_text( sFileName );
    }
}

void RemoteFilesDialog::EnableControls()
{
    if (m_xServices_lb->get_count() > 0)
    {
        m_xServices_lb->set_sensitive(true);

        if (m_xServices_lb->get_active() != -1)
        {
            m_xManageServices->set_item_sensitive("change_password", false);

            try
            {
                if( m_xMasterPasswd->isPersistentStoringAllowed() )
                {
                    int nPos = GetSelectedServicePos();

                    if( nPos >= 0 )
                    {
                        OUString sUrl( m_aServices[nPos]->GetUrl() );

                        UrlRecord aURLEntries = m_xMasterPasswd->find( sUrl, Reference< XInteractionHandler>() );

                        if( aURLEntries.UserList.hasElements() )
                        {
                            m_xManageServices->set_item_sensitive("change_password", true);
                        }
                    }
                }
            }
            catch( const Exception& )
            {}
        }
    }
    else
        m_xServices_lb->set_sensitive(false);

    if( m_bIsConnected )
    {
        m_xFilter_lb->set_sensitive(true);
        m_xName_ed->set_sensitive(true);
        m_xContainer->set_sensitive(true);
        m_xNewFolder->set_sensitive(true);

        if (!m_xName_ed->get_text().isEmpty())
            m_xOk_btn->set_sensitive(true);
        else
            m_xOk_btn->set_sensitive(false);
    }
    else
    {
        m_xFilter_lb->set_sensitive(false);
        m_xName_ed->set_sensitive(false);
        m_xContainer->set_sensitive(false);
        m_xNewFolder->set_sensitive(false);
        m_xOk_btn->set_sensitive(false);
    }

    m_xPath->EnableFields( true );
    m_xManageServices->set_sensitive(true);
}

void RemoteFilesDialog::DisableControls()
{
    m_xServices_lb->set_sensitive(false);
    m_xFilter_lb->set_sensitive(false);
    m_xManageServices->set_sensitive(false);
    m_xName_ed->set_sensitive(false);
    m_xContainer->set_sensitive(false);
    m_xOk_btn->set_sensitive(false);
    m_xPath->EnableFields( false );

    m_xCancel_btn->set_sensitive(true);
}

void RemoteFilesDialog::SavePassword(const OUString& rURL, const OUString& rUser,
                                     const OUString& rPassword, bool bPersistent)
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
            Reference< XInteractionHandler > xInteractionHandler =
                InteractionHandler::createWithParent( m_xContext, nullptr );

            Sequence<OUString> aPasswd { rPassword };

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

IMPL_LINK_NOARG ( RemoteFilesDialog, IconViewHdl, weld::Button&, void )
{
    m_xListView_btn->set_active(false);
    m_xFileView->SetViewMode( eIcon );
}

IMPL_LINK_NOARG ( RemoteFilesDialog, ListViewHdl, weld::Button&, void )
{
    m_xIconView_btn->set_active(false);
    m_xFileView->SetViewMode( eDetailedList );
}

void RemoteFilesDialog::AddService()
{
    PlaceEditDialog aDlg(m_xDialog.get());
    aDlg.ShowPasswordControl();
    short aRetCode = aDlg.run();

    switch( aRetCode )
    {
        case RET_OK :
        {
            ServicePtr newService = aDlg.GetPlace();
            m_aServices.push_back( newService );

            OUString sPassword = aDlg.GetPassword();
            OUString sUser = aDlg.GetUser();
            if( !sUser.isEmpty() && !sPassword.isEmpty() )
            {
                bool bPersistent = aDlg.IsRememberChecked();
                SavePassword( newService->GetUrl(), sUser, sPassword, bPersistent );
            }

            OUString sPrefix = lcl_GetServiceType( newService );

            if(!sPrefix.isEmpty())
                 sPrefix += ": ";

            m_xServices_lb->append_text( sPrefix + newService->GetName() );
            m_xServices_lb->set_active( m_xServices_lb->get_count() - 1 );
            EnableExtraMenuItems(true);
            SelectServiceHdl( *m_xServices_lb );

            m_bIsUpdated = true;

            EnableControls();
            break;
        }
        case RET_CANCEL :
        default :
            // Do Nothing
            break;
    }
}

IMPL_LINK_NOARG( RemoteFilesDialog, SelectServiceHdl, weld::ComboBox&, void )
{
    int nPos = GetSelectedServicePos();

    if( nPos >= 0 )
    {
        OUString sURL = m_aServices[nPos]->GetUrl();
        EnableExtraMenuItems(true);

        m_bServiceChanged = true;
        OpenURL( sURL );
    }
}

IMPL_LINK ( RemoteFilesDialog, EditServiceMenuHdl, const OString&, rIdent, void )
{
    OString sIdent(rIdent);
    if( sIdent == "edit_service"  && m_xServices_lb->get_count() > 0 )
    {
        int nSelected = m_xServices_lb->get_active();
        int nPos = GetSelectedServicePos();

        if( nPos >= 0 )
        {
            PlaceEditDialog aDlg(m_xDialog.get(), m_aServices[nPos]);
            short aRetCode = aDlg.run();

            switch( aRetCode )
            {
                case RET_OK :
                {
                    ServicePtr pEditedService = aDlg.GetPlace();

                    m_aServices[nPos] = pEditedService;
                    m_xServices_lb->remove( nSelected );

                    OUString sPrefix = lcl_GetServiceType( pEditedService );

                    if(!sPrefix.isEmpty())
                        sPrefix += ": ";

                    m_xServices_lb->insert_text(nSelected, sPrefix + pEditedService->GetName());
                    m_xServices_lb->set_active( nSelected );

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
            }
        }
    }
    if( sIdent == "delete_service"  && m_xServices_lb->get_count() > 0 )
    {
        int nSelected = m_xServices_lb->get_active();
        int nPos = GetSelectedServicePos();

        if( nPos >= 0 )
        {
            OUString sMsg = FpsResId( STR_SVT_DELETESERVICE );
            sMsg = sMsg.replaceFirst( "$servicename$", m_xServices_lb->get_active_text() );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Question, VclButtonsType::YesNo, sMsg));
            if (xBox->run() == RET_YES)
            {
                // remove password
                try
                {
                    if( m_xMasterPasswd->isPersistentStoringAllowed() )
                    {
                        OUString sUrl( m_aServices[nPos]->GetUrl() );

                        Reference< XInteractionHandler > xInteractionHandler =
                            InteractionHandler::createWithParent( m_xContext, nullptr );

                        UrlRecord aURLEntries = m_xMasterPasswd->find( sUrl, xInteractionHandler );

                        if( aURLEntries.Url == sUrl && aURLEntries.UserList.hasElements() )
                        {
                            OUString sUserName = aURLEntries.UserList[0].UserName;

                            m_xMasterPasswd->removePersistent( sUrl, sUserName );
                        }
                    }
                }
                catch( const Exception& )
                {}

                m_aServices.erase( m_aServices.begin() + nPos );
                m_xServices_lb->remove( nSelected );

                m_xServices_lb->set_active(-1);
                EnableExtraMenuItems(false);

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

                    Reference< XInteractionHandler > xInteractionHandler =
                        InteractionHandler::createWithParent( m_xContext, nullptr );

                    UrlRecord aURLEntries = m_xMasterPasswd->find( sUrl, xInteractionHandler );

                    if( aURLEntries.Url == sUrl && aURLEntries.UserList.hasElements() )
                    {
                        OUString sUserName = aURLEntries.UserList[0].UserName;

                        rtl::Reference<::comphelper::SimplePasswordRequest> pPasswordRequest
                            = new ::comphelper::SimplePasswordRequest;

                        xInteractionHandler->handle( pPasswordRequest );

                        if ( pPasswordRequest->isPassword() )
                        {
                            OUString aNewPass = pPasswordRequest->getPassword();
                            Sequence<OUString> aPasswd { aNewPass };

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
    else if( sIdent == "add_service" )
        AddService();

    EnableControls();
}

IMPL_LINK_NOARG( RemoteFilesDialog, DoubleClickHdl, SvtFileView*, bool )
{
    SvtContentEntry* pData = m_xFileView->FirstSelected();
    if (pData)
    {
        if (!pData->mbIsFolder)
            m_xDialog->response(RET_OK);
        else
            OpenURL(pData->maURL);
    }
    return true;
}

IMPL_LINK_NOARG( RemoteFilesDialog, SelectHdl, SvtFileView*, void )
{
    SvtContentEntry* pData = m_xFileView->FirstSelected();
    if (!pData)
        return;

    if( ( pData->mbIsFolder && ( m_eType == REMOTEDLG_TYPE_PATHDLG ) )
        || ( !pData->mbIsFolder && ( m_eType == REMOTEDLG_TYPE_FILEDLG ) ) )
    {
        // url must contain user info, because we need this info in recent files entry
        // (to fill user field in login box by default)
        INetURLObject aURL( pData->maURL );
        INetURLObject aCurrentURL( m_sLastServiceUrl );
        aURL.SetUser( aCurrentURL.GetUser() );

        m_sPath = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        m_xName_ed->set_text( aURL.GetLastName(INetURLObject::DecodeMechanism::WithCharset) );
    }
    else
    {
        if( m_eMode == REMOTEDLG_MODE_OPEN )
        {
            m_sPath.clear();
            m_xName_ed->set_text( "" );
        }
    }

    EnableControls();
}

IMPL_LINK_NOARG(RemoteFilesDialog, FileNameGetFocusHdl, weld::Widget&, void)
{
    m_xFileView->SetNoSelection();
}

IMPL_LINK_NOARG(RemoteFilesDialog, FileNameModifyHdl, weld::Entry&, void)
{
    m_xFileView->SetNoSelection();
    if (!m_xOk_btn->get_sensitive())
        EnableControls();
}

IMPL_LINK_NOARG( RemoteFilesDialog, SelectFilterHdl, weld::ComboBox&, void )
{
    int nPos = m_xFilter_lb->get_active();

    if( nPos != -1 && !m_aFilters[nPos].second.isEmpty() )
    {
        m_nCurrentFilter = nPos;

        OUString sCurrentURL = m_xFileView->GetViewURL();

        if( !sCurrentURL.isEmpty() && m_bIsConnected )
            OpenURL( sCurrentURL );
    }
}

IMPL_LINK(RemoteFilesDialog, TreeSelectHdl, weld::TreeView&, rBox, void)
{
    OpenURL(rBox.get_selected_id());
    m_xFileView->grab_focus();
}

IMPL_LINK(RemoteFilesDialog, SelectBreadcrumbHdl, Breadcrumb*, pPtr, bool)
{
    OpenURL( pPtr->GetHdlURL() );
    return true;
}

IMPL_LINK_NOARG ( RemoteFilesDialog, NewFolderHdl, weld::Button&, void )
{
    m_xFileView->EndInplaceEditing();

    // will be bound after InteractionHandler is enabled
    SmartContent aContent;
    aContent.enableDefaultInteractionHandler();
    // now it can be bound
    aContent.bindTo( m_xFileView->GetViewURL() );
    if( !aContent.canCreateFolder() )
        return;

    OUString aTitle;
    aContent.getTitle( aTitle );
    QueryFolderNameDialog aDlg(m_xDialog.get(), aTitle, FpsResId(STR_SVT_NEW_FOLDER));
    bool bHandled = false;

    while( !bHandled )
    {
        if (aDlg.run() == RET_OK)
        {
            OUString aUrl = aContent.createFolder(aDlg.GetName());
            if( !aUrl.isEmpty() )
            {
                m_xFileView->CreatedFolder(aUrl, aDlg.GetName());
                bHandled = true;
            }
        }
        else
            bHandled = true;
    }
}

IMPL_LINK_NOARG ( RemoteFilesDialog, OkHdl, weld::Button&, void )
{
    OUString sNameNoExt = m_xName_ed->get_text();
    OUString sPathNoExt;

    // auto extension
    if( m_eMode == REMOTEDLG_MODE_SAVE )
        AddFileExtension();

    // check if file/path exists

    OUString sCurrentPath = m_xFileView->GetViewURL();
    OUString sSelectedItem = m_xFileView->GetCurrentURL();
    OUString sName = m_xName_ed->get_text();

    bool bFileDlg = ( m_eType == REMOTEDLG_TYPE_FILEDLG );
    bool bSelected = ( m_xFileView->GetSelectionCount() > 0 );

    if( !sCurrentPath.endsWith("/") )
        sCurrentPath += "/";

    if( !bSelected )
    {
        m_sPath = sCurrentPath + INetURLObject::encode( sName, INetURLObject::PART_FPATH, INetURLObject::EncodeMechanism::All );
        sPathNoExt = sCurrentPath + INetURLObject::encode( sNameNoExt, INetURLObject::PART_FPATH, INetURLObject::EncodeMechanism::All );
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

        m_sPath = aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE );
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
            OUString sMsg = FpsResId( STR_SVT_ALREADYEXISTOVERWRITE );
            sMsg = sMsg.replaceFirst( "$filename$", sName );
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Question, VclButtonsType::YesNo, sMsg));
            if (xBox->run() != RET_YES)
                return;
        }
    }
    else
    {
        if( ContentIsFolder( sPathNoExt ) )
        {
            OpenURL( sPathNoExt );
            m_xName_ed->set_text( "" );

            if (!bSelected)
                m_xName_ed->grab_focus();

            return;
        }

        if( m_eMode == REMOTEDLG_MODE_OPEN )
            return;
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG ( RemoteFilesDialog, CancelHdl, weld::Button&, void )
{
    if( m_pCurrentAsyncAction.is() )
    {
        m_pCurrentAsyncAction->cancel();
        onAsyncOperationFinished();
    }
    else
    {
        m_xDialog->response(RET_CANCEL);
    }
}

// SvtFileDialog_Base
SvtFileView* RemoteFilesDialog::GetView()
{
    return m_xFileView.get();
}

void RemoteFilesDialog::SetHasFilename( bool )
{
}

void RemoteFilesDialog::SetDenyList( const css::uno::Sequence< OUString >& rDenyList )
{
    m_aDenyList = rDenyList;
    m_xTreeView->SetDenyList( rDenyList );
}

const css::uno::Sequence< OUString >& RemoteFilesDialog::GetDenyList() const
{
    return m_aDenyList;
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
        OUString sFileName = aUrl.GetLastName( INetURLObject::DecodeMechanism::WithCharset );

        m_xName_ed->set_text( sFileName );
    }
}

OUString RemoteFilesDialog::getCurrentFileText() const
{
    OUString sReturn;
    if( m_xName_ed )
        sReturn = m_xName_ed->get_text();
    return sReturn;
}

void RemoteFilesDialog::setCurrentFileText( const OUString& rText, bool bSelectAll )
{
    if (m_xName_ed)
    {
        m_xName_ed->set_text(rText);
        if( bSelectAll )
            m_xName_ed->select_region(0, -1);
    }
}

void RemoteFilesDialog::AddFilterGroup(
                                  const OUString& rFilter,
                                  const css::uno::Sequence< css::beans::StringPair >& rFilters )
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

    if (m_nCurrentFilter != -1)
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
    DBG_ASSERT( !m_bIsInExecute, "SvtFileDialog::SetCurFilter: currently executing!" );

    // look for corresponding filter
    sal_uInt16 nPos = m_aFilters.size();

    while ( nPos-- )
    {
        if ( m_aFilters[nPos].first == rFilter )
        {
            m_nCurrentFilter = nPos;
            m_xFilter_lb->set_active( m_nCurrentFilter );
            break;
        }
    }
}

void RemoteFilesDialog::FilterSelect()
{
}

void RemoteFilesDialog::SetFileCallback( ::svt::IFilePickerListener * )
{
}

void RemoteFilesDialog::onAsyncOperationStarted()
{
    DisableControls();
}

void RemoteFilesDialog::onAsyncOperationFinished()
{
    m_pCurrentAsyncAction = nullptr;
    EnableControls();
}

void RemoteFilesDialog::UpdateControls( const OUString& rURL )
{
    int nPos = GetSelectedServicePos();

    if( nPos >= 0 && m_bServiceChanged && rURL == m_aServices[nPos]->GetUrl() )
    {
        OUString sURL = m_aServices[nPos]->GetUrl();

        m_xPath->SetRootName( m_sRootLabel );
        m_xTreeView->clear();

        m_xTreeView->InsertRootEntry(rURL, m_sRootLabel);

        m_xName_ed->grab_focus();

        m_sLastServiceUrl = sURL;

        m_bServiceChanged = false;
    }

    m_xPath->SetURL( rURL );

    m_xTreeView->connect_changed(Link<weld::TreeView&,void>());

    // read cached data for this url and fill the tree
    const ::std::vector< SvtContentEntry >& rFolders = m_xFileView->GetContent();
    ::std::vector< std::pair< OUString, OUString > > aFolders;

    m_xName_ed->ClearEntries();

    for(const auto & rFolder : rFolders)
    {
        //WebDAV folders path ends in '/', so strip it
        OUString aFolderName = rFolder.maURL;
        if( rFolder.mbIsFolder && ( ( aFolderName.lastIndexOf( '/' ) + 1 ) == aFolderName.getLength() ) )
            aFolderName = aFolderName.copy( 0, aFolderName.getLength() - 1 );

        int nTitleStart = aFolderName.lastIndexOf( '/' );
        if( nTitleStart != -1 )
        {
            OUString sTitle( INetURLObject::decode(
                                aFolderName.subView( nTitleStart + 1 ),
                                INetURLObject::DecodeMechanism::WithCharset ) );

            if( rFolder.mbIsFolder )
            {
                aFolders.emplace_back( sTitle, aFolderName );
            }

            // add entries to the autocompletion mechanism
            m_xName_ed->AddEntry( sTitle );
        }
    }

    m_xTreeView->FillTreeEntry( rURL, aFolders );

    m_xTreeView->connect_changed( LINK( this, RemoteFilesDialog, TreeSelectHdl ) );

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

    m_xFileView->selected_foreach([this, &aList](weld::TreeIter& rCurEntry){
        // url must contain user info, because we need this info in recent files entry
        // (to fill user field in login box by default)
        INetURLObject aURL(m_xFileView->GetURL(rCurEntry));
        INetURLObject aCurrentURL( m_sLastServiceUrl );
        aURL.SetUser( aCurrentURL.GetUser() );

        aList.push_back( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );

        return false;
    });

    if( aList.empty() && !m_sPath.isEmpty() )
        aList.push_back( m_sPath );

    return aList;
}

bool RemoteFilesDialog::ContentIsFolder( const OUString& rURL )
{
    try
    {
        Reference< XInteractionHandler > xInteractionHandler(
                        InteractionHandler::createWithParent( m_xContext, nullptr ), UNO_QUERY_THROW );
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
                        InteractionHandler::createWithParent( m_xContext, nullptr ), UNO_QUERY_THROW );
        //check if WebDAV or not
        if ( !INetURLObject( rURL ).isAnyKnownWebDAVScheme() )
        {
                // no webdav, use the interaction handler as is
                Reference< XCommandEnvironment > xEnv = new ::ucbhelper::CommandEnvironment( xInteractionHandler, Reference< XProgressHandler >() );
                ::ucbhelper::Content aContent( rURL, xEnv, m_xContext );

                return aContent.isDocument();
        }
        else
        {
            // It's a webdav URL, so use the same open sequence as in normal open process.
            // Let's use a comphelper::StillReadWriteInteraction to trap errors here without showing the user.
            // This sequence will result in an exception if the target URL resource is not present
            rtl::Reference<comphelper::StillReadWriteInteraction> pInteraction = new comphelper::StillReadWriteInteraction(xInteractionHandler,xInteractionHandler);

            Reference< XCommandEnvironment > xEnv = new ::ucbhelper::CommandEnvironment( pInteraction, Reference< XProgressHandler >() );
            ::ucbhelper::Content aContent( rURL, xEnv, m_xContext );

            aContent.openStream();
            return aContent.isDocument();
        }
    }
    catch( const Exception& )
    {
        // a content doesn't exist
    }

    return false;
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

void RemoteFilesDialog::setImage( const css::uno::Any& )
{
    // This dialog doesn't contain preview
}

bool RemoteFilesDialog::getShowState()
{
    // This dialog doesn't contain preview
    return false;
}

weld::Widget* RemoteFilesDialog::getControl( sal_Int16, bool) const
{
    return nullptr;
}

void RemoteFilesDialog::enableControl( sal_Int16, bool )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
