/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_oauth2.h>

#include <svtools/PlaceEditDialog.hxx>
#include <svtools/ServerDetailsControls.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <officecfg/Office/Common.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <svtools/place.hxx>

using namespace com::sun::star::uno;

PlaceEditDialog::PlaceEditDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "svt/ui/placeedit.ui", "PlaceEditDialog")
    , m_xCurrentDetails()
    , m_nCurrentType( 0 )
    , m_bLabelChanged( false )
    , m_bShowPassword( true )
    , m_xEDServerName(m_xBuilder->weld_entry("name"))
    , m_xLBServerType(m_xBuilder->weld_combo_box("type"))
    , m_xEDUsername(m_xBuilder->weld_entry("login"))
    , m_xFTUsernameLabel(m_xBuilder->weld_label("loginLabel"))
    , m_xBTOk(m_xBuilder->weld_button("ok"))
    , m_xBTCancel(m_xBuilder->weld_button("cancel"))
    , m_xBTDelete(m_xBuilder->weld_button("delete"))
    , m_xBTRepoRefresh(m_xBuilder->weld_button("repositoriesRefresh"))
    , m_xCBPassword(m_xBuilder->weld_check_button("rememberPassword"))
    , m_xEDPassword(m_xBuilder->weld_entry("password"))
    , m_xFTPasswordLabel(m_xBuilder->weld_label("passwordLabel"))
    , m_xTypeGrid(m_xBuilder->weld_widget("TypeGrid"))

    , m_xRepositoryBox(m_xBuilder->weld_widget("RepositoryDetails"))
    , m_xFTRepository(m_xBuilder->weld_label("repositoryLabel"))
    , m_xLBRepository(m_xBuilder->weld_combo_box("repositories"))

    , m_xEDShare(m_xBuilder->weld_entry("share"))
    , m_xFTShare(m_xBuilder->weld_label("shareLabel"))

    , m_xDetailsGrid(m_xBuilder->weld_widget("Details"))
    , m_xHostBox(m_xBuilder->weld_widget("HostDetails"))
    , m_xEDHost(m_xBuilder->weld_entry("host"))
    , m_xFTHost(m_xBuilder->weld_label("hostLabel"))
    , m_xEDPort(m_xBuilder->weld_spin_button("port"))
    , m_xFTPort(m_xBuilder->weld_label("portLabel"))
    , m_xEDRoot(m_xBuilder->weld_entry("path"))
    , m_xFTRoot(m_xBuilder->weld_label("pathLabel"))

    , m_xCBDavs(m_xBuilder->weld_check_button("webdavs"))
{
    m_xBTOk->connect_clicked( LINK( this, PlaceEditDialog, OKHdl) );
    m_xBTOk->set_sensitive( false );

    m_xEDServerName->connect_changed( LINK( this, PlaceEditDialog, EditLabelHdl) );

    // This constructor is called when user request a place creation, so
    // delete button is hidden.
    m_xBTDelete->hide();

    m_xLBServerType->connect_changed( LINK( this, PlaceEditDialog, SelectTypeHdl ) );
    m_xEDUsername->connect_changed( LINK( this, PlaceEditDialog, EditUsernameHdl ) );
    m_xEDPassword->connect_changed( LINK( this, PlaceEditDialog, EditUsernameHdl ) );

    InitDetails( );
}

PlaceEditDialog::PlaceEditDialog(weld::Window* pParent, const std::shared_ptr<Place>& rPlace)
    : GenericDialogController(pParent, "svt/ui/placeedit.ui", "PlaceEditDialog")
    , m_xCurrentDetails( )
    , m_bLabelChanged( true )
    , m_bShowPassword( false )
    , m_xEDServerName(m_xBuilder->weld_entry("name"))
    , m_xLBServerType(m_xBuilder->weld_combo_box("type"))
    , m_xEDUsername(m_xBuilder->weld_entry("login"))
    , m_xFTUsernameLabel(m_xBuilder->weld_label("loginLabel"))
    , m_xBTOk(m_xBuilder->weld_button("ok"))
    , m_xBTCancel(m_xBuilder->weld_button("cancel"))
    , m_xBTDelete(m_xBuilder->weld_button("delete"))
    , m_xBTRepoRefresh(m_xBuilder->weld_button("repositoriesRefresh"))
    , m_xCBPassword(m_xBuilder->weld_check_button("rememberPassword"))
    , m_xEDPassword(m_xBuilder->weld_entry("password"))
    , m_xFTPasswordLabel(m_xBuilder->weld_label("passwordLabel"))
    , m_xTypeGrid(m_xBuilder->weld_widget("TypeGrid"))

    , m_xRepositoryBox(m_xBuilder->weld_widget("RepositoryDetails"))
    , m_xFTRepository(m_xBuilder->weld_label("repositoryLabel"))
    , m_xLBRepository(m_xBuilder->weld_combo_box("repositories"))

    , m_xEDShare(m_xBuilder->weld_entry("share"))
    , m_xFTShare(m_xBuilder->weld_label("shareLabel"))

    , m_xDetailsGrid(m_xBuilder->weld_widget("Details"))
    , m_xHostBox(m_xBuilder->weld_widget("HostDetails"))
    , m_xEDHost(m_xBuilder->weld_entry("host"))
    , m_xFTHost(m_xBuilder->weld_label("hostLabel"))
    , m_xEDPort(m_xBuilder->weld_spin_button("port"))
    , m_xFTPort(m_xBuilder->weld_label("portLabel"))
    , m_xEDRoot(m_xBuilder->weld_entry("path"))
    , m_xFTRoot(m_xBuilder->weld_label("pathLabel"))

    , m_xCBDavs(m_xBuilder->weld_check_button("webdavs"))
{
    m_xEDPassword->hide();
    m_xFTPasswordLabel->hide();
    m_xCBPassword->hide();

    m_xBTOk->connect_clicked( LINK( this, PlaceEditDialog, OKHdl) );
    m_xBTDelete->connect_clicked( LINK( this, PlaceEditDialog, DelHdl) );

    m_xEDServerName->connect_changed( LINK( this, PlaceEditDialog, ModifyHdl) );
    m_xLBServerType->connect_changed( LINK( this, PlaceEditDialog, SelectTypeHdl ) );

    InitDetails( );

    m_xEDServerName->set_text(rPlace->GetName());

    // Fill the boxes with the URL parts
    bool bSuccess = false;
    for (size_t i = 0 ; i < m_aDetailsContainers.size( ) && !bSuccess; ++i)
    {
        INetURLObject& rUrl = rPlace->GetUrlObject();
        bSuccess = m_aDetailsContainers[i]->setUrl( rUrl );
        if ( bSuccess )
        {
            // Fill the Username field
            if ( rUrl.HasUserData( ) )
            {
                m_xEDUsername->set_text( INetURLObject::decode( rUrl.GetUser( ),
                                                              INetURLObject::DecodeMechanism::WithCharset ) );
                m_aDetailsContainers[i]->setUsername( INetURLObject::decode( rUrl.GetUser( ),
                                                              INetURLObject::DecodeMechanism::WithCharset ) );
            }

            m_xLBServerType->set_active(i);
            SelectType(true);
        }
    }

    // In edit mode user can't change connection type
    m_xTypeGrid->hide();
}

PlaceEditDialog::~PlaceEditDialog()
{
}

OUString PlaceEditDialog::GetServerUrl()
{
    OUString sUrl;
    if (m_xCurrentDetails.get())
    {
        INetURLObject aUrl = m_xCurrentDetails->getUrl();
        OUString sUsername = m_xEDUsername->get_text().trim();
        if ( !sUsername.isEmpty( ) )
            aUrl.SetUser( sUsername );
        if ( !aUrl.HasError( ) )
            sUrl = aUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }

    return sUrl;
}

std::shared_ptr<Place> PlaceEditDialog::GetPlace()
{
    return std::make_shared<Place>(m_xEDServerName->get_text(), GetServerUrl(), true);
}

void PlaceEditDialog::InitDetails( )
{
    // Create CMIS controls for each server type

    Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();

    // Load the ServerType entries
    bool bSkipGDrive = OUString( GDRIVE_CLIENT_ID ).isEmpty() ||
                       OUString( GDRIVE_CLIENT_SECRET ).isEmpty();
    bool bSkipAlfresco = OUString( ALFRESCO_CLOUD_CLIENT_ID ).isEmpty() ||
                       OUString( ALFRESCO_CLOUD_CLIENT_SECRET ).isEmpty();
    bool bSkipOneDrive= OUString( ONEDRIVE_CLIENT_ID ).isEmpty() ||
                       OUString( ONEDRIVE_CLIENT_SECRET ).isEmpty();

    Sequence< OUString > aTypesUrlsList( officecfg::Office::Common::Misc::CmisServersUrls::get( xContext ) );
    Sequence< OUString > aTypesNamesList( officecfg::Office::Common::Misc::CmisServersNames::get( xContext ) );

    int nPos = 0;
    for ( sal_Int32 i = 0; i < aTypesUrlsList.getLength( ) && aTypesNamesList.getLength( ); ++i )
    {
        OUString sUrl = aTypesUrlsList[i].replaceFirst("<host", "<" + SvtResId(STR_SVT_HOST)).replaceFirst("port>",  SvtResId(STR_SVT_PORT) + ">");

        if ((sUrl == GDRIVE_BASE_URL && bSkipGDrive) ||
            (sUrl.startsWith( ALFRESCO_CLOUD_BASE_URL) && bSkipAlfresco) ||
            (sUrl == ONEDRIVE_BASE_URL && bSkipOneDrive))
        {
            // this service is not supported
            continue;
        }

        m_xLBServerType->insert_text(nPos, aTypesNamesList[i].replaceFirst("Other CMIS", SvtResId(STR_SVT_OTHER_CMIS)));

        std::shared_ptr<DetailsContainer> xCmisDetails(std::make_shared<CmisDetailsContainer>(this, sUrl));
        xCmisDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
        m_aDetailsContainers.push_back(xCmisDetails);

        ++nPos;
    }

    // Create WebDAV / FTP / SSH details control
    std::shared_ptr<DetailsContainer> xDavDetails(std::make_shared<DavDetailsContainer>(this));
    xDavDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(xDavDetails);

    std::shared_ptr<DetailsContainer> xFtpDetails(std::make_shared<HostDetailsContainer>(this, 21, "ftp"));
    xFtpDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(xFtpDetails);

    std::shared_ptr<DetailsContainer> xSshDetails(std::make_shared<HostDetailsContainer>(this, 22, "ssh"));
    xSshDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(xSshDetails);

    // Remove Windows Share entry from dialog on Windows OS, where it's non-functional
#if defined(_WIN32)
    // nPos is the position of first item that is pre-defined in svtools/uiconfig/ui/placeedit.ui,
    // after other CMIS types were inserted
    m_xLBServerType->remove(nPos + 3);
#else
    // Create Windows Share control
    std::shared_ptr<DetailsContainer> xSmbDetails(std::make_shared<SmbDetailsContainer>(this));
    xSmbDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(xSmbDetails);
#endif

    // Set default to first value
    m_xLBServerType->set_active(0);

    if (m_xLBServerType->get_active_text() == "--------------------" )
        m_xLBServerType->set_active(1);

    SelectType(true);
}

IMPL_LINK( PlaceEditDialog, OKHdl, weld::Button&, /*rBtn*/, void)
{
    if ( !m_xCurrentDetails.get() )
        return;

    OUString sUrl = m_xCurrentDetails->getUrl().GetHost( INetURLObject::DecodeMechanism::WithCharset );

    if ( sUrl.startsWith( GDRIVE_BASE_URL )
       || sUrl.startsWith( ALFRESCO_CLOUD_BASE_URL )
       || sUrl.startsWith( ONEDRIVE_BASE_URL ) )
    {
        m_xBTRepoRefresh->clicked();

        sUrl = m_xCurrentDetails->getUrl().GetHost( INetURLObject::DecodeMechanism::WithCharset );
        INetURLObject aHostUrl( sUrl );
        OUString sRepoId = aHostUrl.GetMark();

        if ( !sRepoId.isEmpty() )
        {
            m_xDialog->response(RET_OK);
        }
        else
        {
            // TODO: repository id missing. Auth error?
        }
    }
    else
    {
        m_xDialog->response(RET_OK);
    }
}

IMPL_LINK( PlaceEditDialog, DelHdl, weld::Button&, /*rButton*/, void)
{
    // ReUsing existing symbols...
    m_xDialog->response(RET_NO);
}

IMPL_LINK_NOARG( PlaceEditDialog, EditHdl, DetailsContainer*, void )
{
    if( !m_bLabelChanged )
    {
        if( !m_xEDUsername->get_text().isEmpty( ) )
        {
            OUString sLabel = SvtResId( STR_SVT_DEFAULT_SERVICE_LABEL );
            OUString sUser = m_xEDUsername->get_text();

            int nLength = sUser.indexOf( '@' );
            if( nLength < 0 )
                nLength = sUser.getLength();

            sLabel = sLabel.replaceFirst( "$user$", sUser.copy( 0, nLength ) );
            sLabel = sLabel.replaceFirst( "$service$", m_xLBServerType->get_active_text() );

            m_xEDServerName->set_text( sLabel );
            m_bLabelChanged = false;
        }
        else
        {
            m_xEDServerName->set_text( m_xLBServerType->get_active_text( ) );
        }
    }

    OUString sUrl = GetServerUrl( );
    OUString sName = m_xEDServerName->get_text().trim();
    m_xBTOk->set_sensitive( !sName.isEmpty( ) && !sUrl.isEmpty( ) );
}

IMPL_LINK_NOARG( PlaceEditDialog, ModifyHdl, weld::Entry&, void )
{
    EditHdl(nullptr);
}

IMPL_LINK_NOARG( PlaceEditDialog, EditLabelHdl, weld::Entry&, void )
{
    m_bLabelChanged = true;
    EditHdl(nullptr);
}

IMPL_LINK_NOARG( PlaceEditDialog, EditUsernameHdl, weld::Entry&, void )
{
    for ( auto& rxDetailsContainer : m_aDetailsContainers )
    {
        rxDetailsContainer->setUsername( m_xEDUsername->get_text() );
        rxDetailsContainer->setPassword( m_xEDPassword->get_text() );
    }

    EditHdl(nullptr);
}

IMPL_LINK_NOARG( PlaceEditDialog, SelectTypeHdl, weld::ComboBox&, void )
{
    SelectType(false);
}

void PlaceEditDialog::SelectType(bool bSkipSeparator)
{
    if ( m_xLBServerType->get_active_text() == "--------------------" )
    {
        if (bSkipSeparator)
            m_xLBServerType->set_active(m_nCurrentType);
        else
            m_xLBServerType->set_active(-1);
        return;
    }

    if (m_xCurrentDetails.get())
        m_xCurrentDetails->set_visible(false);

    const int nPos = m_xLBServerType->get_active( );
    m_xCurrentDetails = m_aDetailsContainers[nPos];
    m_nCurrentType = nPos;

    m_xCurrentDetails->set_visible(true);

    m_xCBPassword->set_visible( m_bShowPassword && m_xCurrentDetails->enableUserCredentials() );
    m_xEDPassword->set_visible( m_bShowPassword && m_xCurrentDetails->enableUserCredentials() );
    m_xFTPasswordLabel->set_visible( m_bShowPassword && m_xCurrentDetails->enableUserCredentials() );
    m_xEDUsername->set_visible( m_xCurrentDetails->enableUserCredentials() );
    m_xFTUsernameLabel->set_visible( m_xCurrentDetails->enableUserCredentials() );

    m_xDialog->resize_to_request();

    EditHdl(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
