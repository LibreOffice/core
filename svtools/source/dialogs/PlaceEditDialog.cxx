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

#include <com/sun/star/uno/Sequence.hxx>
#include <officecfg/Office/Common.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <svtools/place.hxx>

#include "ServerDetailsControls.hxx"

using namespace com::sun::star::uno;

PlaceEditDialog::PlaceEditDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"svt/ui/placeedit.ui"_ustr, u"PlaceEditDialog"_ustr)
    , m_xCurrentDetails()
    , m_nCurrentType( 0 )
    , m_bLabelChanged( false )
    , m_bShowPassword( true )
    , m_xEDServerName(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xLBServerType(m_xBuilder->weld_combo_box(u"type"_ustr))
    , m_xEDUsername(m_xBuilder->weld_entry(u"login"_ustr))
    , m_xFTUsernameLabel(m_xBuilder->weld_label(u"loginLabel"_ustr))
    , m_xBTOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBTDelete(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xBTRepoRefresh(m_xBuilder->weld_button(u"repositoriesRefresh"_ustr))
    , m_xCBPassword(m_xBuilder->weld_check_button(u"rememberPassword"_ustr))
    , m_xEDPassword(m_xBuilder->weld_entry(u"password"_ustr))
    , m_xFTPasswordLabel(m_xBuilder->weld_label(u"passwordLabel"_ustr))
    , m_xTypeGrid(m_xBuilder->weld_widget(u"TypeGrid"_ustr))

    , m_xRepositoryBox(m_xBuilder->weld_widget(u"RepositoryDetails"_ustr))
    , m_xFTRepository(m_xBuilder->weld_label(u"repositoryLabel"_ustr))
    , m_xLBRepository(m_xBuilder->weld_combo_box(u"repositories"_ustr))

    , m_xEDShare(m_xBuilder->weld_entry(u"share"_ustr))
    , m_xFTShare(m_xBuilder->weld_label(u"shareLabel"_ustr))

    , m_xDetailsGrid(m_xBuilder->weld_widget(u"Details"_ustr))
    , m_xHostBox(m_xBuilder->weld_widget(u"HostDetails"_ustr))
    , m_xEDHost(m_xBuilder->weld_entry(u"host"_ustr))
    , m_xFTHost(m_xBuilder->weld_label(u"hostLabel"_ustr))
    , m_xEDPort(m_xBuilder->weld_spin_button(u"port"_ustr))
    , m_xFTPort(m_xBuilder->weld_label(u"portLabel"_ustr))
    , m_xEDRoot(m_xBuilder->weld_entry(u"path"_ustr))
    , m_xFTRoot(m_xBuilder->weld_label(u"pathLabel"_ustr))

    , m_xCBDavs(m_xBuilder->weld_check_button(u"webdavs"_ustr))
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
    : GenericDialogController(pParent, u"svt/ui/placeedit.ui"_ustr, u"PlaceEditDialog"_ustr)
    , m_xCurrentDetails( )
    , m_bLabelChanged( true )
    , m_bShowPassword( false )
    , m_xEDServerName(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xLBServerType(m_xBuilder->weld_combo_box(u"type"_ustr))
    , m_xEDUsername(m_xBuilder->weld_entry(u"login"_ustr))
    , m_xFTUsernameLabel(m_xBuilder->weld_label(u"loginLabel"_ustr))
    , m_xBTOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xBTDelete(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xBTRepoRefresh(m_xBuilder->weld_button(u"repositoriesRefresh"_ustr))
    , m_xCBPassword(m_xBuilder->weld_check_button(u"rememberPassword"_ustr))
    , m_xEDPassword(m_xBuilder->weld_entry(u"password"_ustr))
    , m_xFTPasswordLabel(m_xBuilder->weld_label(u"passwordLabel"_ustr))
    , m_xTypeGrid(m_xBuilder->weld_widget(u"TypeGrid"_ustr))

    , m_xRepositoryBox(m_xBuilder->weld_widget(u"RepositoryDetails"_ustr))
    , m_xFTRepository(m_xBuilder->weld_label(u"repositoryLabel"_ustr))
    , m_xLBRepository(m_xBuilder->weld_combo_box(u"repositories"_ustr))

    , m_xEDShare(m_xBuilder->weld_entry(u"share"_ustr))
    , m_xFTShare(m_xBuilder->weld_label(u"shareLabel"_ustr))

    , m_xDetailsGrid(m_xBuilder->weld_widget(u"Details"_ustr))
    , m_xHostBox(m_xBuilder->weld_widget(u"HostDetails"_ustr))
    , m_xEDHost(m_xBuilder->weld_entry(u"host"_ustr))
    , m_xFTHost(m_xBuilder->weld_label(u"hostLabel"_ustr))
    , m_xEDPort(m_xBuilder->weld_spin_button(u"port"_ustr))
    , m_xFTPort(m_xBuilder->weld_label(u"portLabel"_ustr))
    , m_xEDRoot(m_xBuilder->weld_entry(u"path"_ustr))
    , m_xFTRoot(m_xBuilder->weld_label(u"pathLabel"_ustr))

    , m_xCBDavs(m_xBuilder->weld_check_button(u"webdavs"_ustr))
{
    m_xEDPassword->hide();
    m_xFTPasswordLabel->hide();
    m_xCBPassword->hide();

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
                m_xEDUsername->set_text( rUrl.GetUser(INetURLObject::DecodeMechanism::WithCharset) );
                m_aDetailsContainers[i]->setUsername( rUrl.GetUser(INetURLObject::DecodeMechanism::WithCharset) );
            }

            m_xLBServerType->set_active(i);
            SelectType(true);
        }
    }

    // In edit mode user can't change connection type
    m_xTypeGrid->hide();

    m_xBTOk->connect_clicked(LINK(this, PlaceEditDialog, OKHdl));
    m_xBTDelete->connect_clicked(LINK(this, PlaceEditDialog, DelHdl));

    m_xEDServerName->connect_changed(LINK(this, PlaceEditDialog, ModifyHdl));
    m_xLBServerType->connect_changed(LINK(this, PlaceEditDialog, SelectTypeHdl));
}

PlaceEditDialog::~PlaceEditDialog()
{
}

OUString PlaceEditDialog::GetServerUrl()
{
    OUString sUrl;
    if (m_xCurrentDetails)
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

    // Load the ServerType entries
    bool bSkipGDrive = std::string_view( GDRIVE_CLIENT_ID ).empty() ||
                       std::string_view( GDRIVE_CLIENT_SECRET ).empty();
    bool bSkipAlfresco = std::string_view( ALFRESCO_CLOUD_CLIENT_ID ).empty() ||
                       std::string_view( ALFRESCO_CLOUD_CLIENT_SECRET ).empty();
    bool bSkipOneDrive= std::string_view( ONEDRIVE_CLIENT_ID ).empty() ||
                       std::string_view( ONEDRIVE_CLIENT_SECRET ).empty();

    Sequence< OUString > aTypesUrlsList( officecfg::Office::Common::Misc::CmisServersUrls::get() );
    Sequence< OUString > aTypesNamesList( officecfg::Office::Common::Misc::CmisServersNames::get() );

    int nPos = 0;
    auto nSize = std::min(aTypesUrlsList.getLength(), aTypesNamesList.getLength());
    for ( sal_Int32 i = 0; i < nSize; ++i )
    {
        OUString sUrl = aTypesUrlsList[i].replaceFirst("<host", Concat2View("<" + SvtResId(STR_SVT_HOST))).replaceFirst("port>",  Concat2View(SvtResId(STR_SVT_PORT) + ">"));

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

    // Create WebDAV / SSH details control
    std::shared_ptr<DetailsContainer> xDavDetails(std::make_shared<DavDetailsContainer>(this));
    xDavDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(std::move(xDavDetails));

    std::shared_ptr<DetailsContainer> xSshDetails(std::make_shared<HostDetailsContainer>(this, 22, "sftp"));
    xSshDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(std::move(xSshDetails));

    // Remove Windows Share entry from dialog on Windows OS, where it's non-functional
#if defined(_WIN32)
    // nPos is the position of first item that is pre-defined in svtools/uiconfig/ui/placeedit.ui,
    // after other CMIS types were inserted
    m_xLBServerType->remove(nPos + 2);
#else
    // Create Windows Share control
    std::shared_ptr<DetailsContainer> xSmbDetails(std::make_shared<SmbDetailsContainer>(this));
    xSmbDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(std::move(xSmbDetails));
#endif

    // Set default to first value
    m_xLBServerType->set_active(0);

    if (m_xLBServerType->get_active_text() == "--------------------" )
        m_xLBServerType->set_active(1);

    SelectType(true);
}

IMPL_LINK( PlaceEditDialog, OKHdl, weld::Button&, /*rBtn*/, void)
{
    if ( !m_xCurrentDetails )
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

            sLabel = sLabel.replaceFirst( "$user$", sUser.subView( 0, nLength ) );
            sLabel = sLabel.replaceFirst( "$service$", m_xLBServerType->get_active_text() );

            m_xEDServerName->set_text( sLabel );
        }
        else
        {
            m_xEDServerName->set_text( m_xLBServerType->get_active_text( ) );
        }
        m_bLabelChanged = false;
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

    if (m_xCurrentDetails)
        m_xCurrentDetails->set_visible(false);

    const int nPos = m_xLBServerType->get_active( );
    if (nPos < 0)
        return;
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
