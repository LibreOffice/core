/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svtools/PlaceEditDialog.hxx>
#include <svtools/ServerDetailsControls.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <officecfg/Office/Common.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/svtools.hrc>
#include <vcl/msgbox.hxx>

using namespace com::sun::star::uno;

PlaceEditDialog::PlaceEditDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "PlaceEditDialog", "svt/ui/placeedit.ui")
    , m_xCurrentDetails()
    , m_nCurrentType( 0 )
    , bLabelChanged( false )
    , m_bShowPassword( true )
{
    get( m_pEDServerName, "name" );
    get( m_pLBServerType, "type" );
    get( m_pEDUsername, "login" );
    get( m_pFTUsernameLabel, "loginLabel" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pBTDelete, "delete" );
    get( m_pBTRepoRefresh, "repositoriesRefresh" );
    get( m_pCBPassword, "rememberPassword" );
    get( m_pEDPassword, "password" );
    get( m_pFTPasswordLabel, "passwordLabel" );

    m_pBTOk->SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_pBTOk->Enable( false );

    m_pEDServerName->SetModifyHdl( LINK( this, PlaceEditDialog, EditLabelHdl) );

    // This constructor is called when user request a place creation, so
    // delete button is hidden.
    m_pBTDelete->Hide();

    m_pLBServerType->SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );
    m_pEDUsername->SetModifyHdl( LINK( this, PlaceEditDialog, EditUsernameHdl ) );
    m_pEDPassword->SetModifyHdl( LINK( this, PlaceEditDialog, EditUsernameHdl ) );

    InitDetails( );
}

PlaceEditDialog::PlaceEditDialog(vcl::Window* pParent, const std::shared_ptr<Place>& rPlace)
    : ModalDialog(pParent, "PlaceEditDialog", "svt/ui/placeedit.ui")
    , m_xCurrentDetails( )
    , bLabelChanged( true )
    , m_bShowPassword( false )
{
    get( m_pEDServerName, "name" );
    get( m_pLBServerType, "type" );
    get( m_pEDUsername, "login" );
    get( m_pFTUsernameLabel, "loginLabel" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pBTDelete, "delete" );
    get( m_pBTRepoRefresh, "repositoriesRefresh" );
    get( m_pTypeGrid, "TypeGrid" );
    get( m_pCBPassword, "rememberPassword" );
    get( m_pEDPassword, "password" );
    get( m_pFTPasswordLabel, "passwordLabel" );

    m_pEDPassword->Hide();
    m_pFTPasswordLabel->Hide();
    m_pCBPassword->Hide();

    m_pBTOk->SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_pBTDelete->SetClickHdl ( LINK( this, PlaceEditDialog, DelHdl) );

    m_pEDServerName->SetModifyHdl( LINK( this, PlaceEditDialog, ModifyHdl) );
    m_pLBServerType->SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );

    InitDetails( );

    m_pEDServerName->SetText(rPlace->GetName());

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
                m_pEDUsername->SetText( INetURLObject::decode( rUrl.GetUser( ),
                                                              INetURLObject::DecodeMechanism::WithCharset ) );
                m_aDetailsContainers[i]->setUsername( INetURLObject::decode( rUrl.GetUser( ),
                                                              INetURLObject::DecodeMechanism::WithCharset ) );
            }

            m_pLBServerType->SelectEntryPos( i );
            SelectTypeHdl( *m_pLBServerType );
        }
    }

    // In edit mode user can't change connection type
    m_pTypeGrid->Hide();
}

PlaceEditDialog::~PlaceEditDialog()
{
    disposeOnce();
}

void PlaceEditDialog::dispose()
{
    m_pEDServerName.clear();
    m_pLBServerType.clear();
    m_pEDUsername.clear();
    m_pFTUsernameLabel.clear();
    m_pBTOk.clear();
    m_pBTCancel.clear();
    m_pBTDelete.clear();
    m_pEDPassword.clear();
    m_pFTPasswordLabel.clear();
    m_pCBPassword.clear();
    m_pBTRepoRefresh.clear();
    m_pTypeGrid.clear();
    ModalDialog::dispose();
}

OUString PlaceEditDialog::GetServerUrl()
{
    OUString sUrl;
    if (m_xCurrentDetails.get())
    {
        INetURLObject aUrl = m_xCurrentDetails->getUrl();
        OUString sUsername = m_pEDUsername->GetText( ).trim( );
        if ( !sUsername.isEmpty( ) )
            aUrl.SetUser( sUsername );
        if ( !aUrl.HasError( ) )
            sUrl = aUrl.GetMainURL( INetURLObject::DecodeMechanism::NONE );
    }

    return sUrl;
}

std::shared_ptr<Place> PlaceEditDialog::GetPlace()
{
    return std::make_shared<Place>(m_pEDServerName->GetText(), GetServerUrl(), true);
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

    unsigned int nPos = 0;
    for ( sal_Int32 i = 0; i < aTypesUrlsList.getLength( ) && aTypesNamesList.getLength( ); ++i )
    {
        OUString sUrl = aTypesUrlsList[i].replaceFirst("<host", "<" + SVT_RESSTR(STR_SVT_HOST)).replaceFirst("port>",  SVT_RESSTR(STR_SVT_PORT) + ">");

        if ((sUrl == GDRIVE_BASE_URL && bSkipGDrive) ||
            (sUrl.startsWith( ALFRESCO_CLOUD_BASE_URL) && bSkipAlfresco) ||
            (sUrl == ONEDRIVE_BASE_URL && bSkipOneDrive))
        {
            // this service is not supported
            continue;
        }

        nPos = m_pLBServerType->InsertEntry( aTypesNamesList[i], nPos );

        std::shared_ptr<DetailsContainer> xCmisDetails(std::make_shared<CmisDetailsContainer>(this, this, sUrl));
        xCmisDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
        m_aDetailsContainers.push_back(xCmisDetails);

        nPos++;
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

    // Create Windows Share control
    std::shared_ptr<DetailsContainer> xSmbDetails(std::make_shared<SmbDetailsContainer>(this));
    xSmbDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(xSmbDetails);

    // Set default to first value
    m_pLBServerType->SelectEntryPos( 0 );

    if ( m_pLBServerType->GetSelectEntry() == "--------------------" )
        m_pLBServerType->SelectEntryPos( 1 );

    SelectTypeHdl( *m_pLBServerType );
}

IMPL_LINK( PlaceEditDialog, OKHdl, Button*, /*pBtn*/, void)
{
    if ( m_xCurrentDetails.get() )
    {
        OUString sUrl = m_xCurrentDetails->getUrl().GetHost( INetURLObject::DecodeMechanism::WithCharset );
        OUString sGDriveHost( GDRIVE_BASE_URL );
        OUString sAlfrescoHost( ALFRESCO_CLOUD_BASE_URL );
        OUString sOneDriveHost( ONEDRIVE_BASE_URL );

        if ( sUrl.compareTo( sGDriveHost, sGDriveHost.getLength() ) == 0
           || sUrl.compareTo( sAlfrescoHost, sAlfrescoHost.getLength() ) == 0
           || sUrl.compareTo( sOneDriveHost, sOneDriveHost.getLength() ) == 0 )
        {
            m_pBTRepoRefresh->Click();

            sUrl = m_xCurrentDetails->getUrl().GetHost( INetURLObject::DecodeMechanism::WithCharset );
            INetURLObject aHostUrl( sUrl );
            OUString sRepoId = aHostUrl.GetMark();

            if ( !sRepoId.isEmpty() )
            {
                EndDialog( RET_OK );
            }
            else
            {
                // TODO: repository id missing. Auth error?
            }
        }
        else
        {
            EndDialog( RET_OK );
        }
    }
}

IMPL_LINK( PlaceEditDialog, DelHdl, Button*, /*pButton*/, void)
{
    // ReUsing existing symbols...
    EndDialog( RET_NO );
}

IMPL_LINK_NOARG( PlaceEditDialog, EditHdl, DetailsContainer*, void )
{
    if( !bLabelChanged )
    {
        if( !m_pEDUsername->GetText().isEmpty( ) )
        {
            OUString sLabel = SvtResId( STR_SVT_DEFAULT_SERVICE_LABEL );
            OUString sUser = m_pEDUsername->GetText();

            int nLength = sUser.indexOf( '@' );
            if( nLength < 0 )
                nLength = sUser.getLength();

            sLabel = sLabel.replaceFirst( "$user$", sUser.copy( 0, nLength ) );
            sLabel = sLabel.replaceFirst( "$service$", m_pLBServerType->GetSelectEntry() );

            m_pEDServerName->SetText( sLabel );
            bLabelChanged = false;
        }
        else
        {
            m_pEDServerName->SetText( m_pLBServerType->GetSelectEntry( ) );
        }
    }

    OUString sUrl = GetServerUrl( );
    OUString sName = m_pEDServerName->GetText().trim( );
    m_pBTOk->Enable( !sName.isEmpty( ) && !sUrl.isEmpty( ) );
}

IMPL_LINK_NOARG( PlaceEditDialog, ModifyHdl, Edit&, void )
{
    EditHdl(nullptr);
}

IMPL_LINK_NOARG( PlaceEditDialog, EditLabelHdl, Edit&, void )
{
    bLabelChanged = true;
    EditHdl(nullptr);
}

IMPL_LINK_NOARG( PlaceEditDialog, EditUsernameHdl, Edit&, void )
{
    for ( std::vector< std::shared_ptr< DetailsContainer > >::iterator it = m_aDetailsContainers.begin( );
            it != m_aDetailsContainers.end( ); ++it )
    {
        ( *it )->setUsername( OUString( m_pEDUsername->GetText() ) );
        ( *it )->setPassword( m_pEDPassword->GetText() );
    }

    EditHdl(nullptr);
}

IMPL_LINK_NOARG( PlaceEditDialog, SelectTypeHdl, ListBox&, void )
{
    if ( m_pLBServerType->GetSelectEntry() == "--------------------" )
    {
        if( !m_pLBServerType->IsTravelSelect() )
            m_pLBServerType->SelectEntryPos( m_nCurrentType );
        else
            m_pLBServerType->SetNoSelection();

        return;
    }

    if (m_xCurrentDetails.get())
        m_xCurrentDetails->show(false);

    const sal_Int32 nPos = m_pLBServerType->GetSelectEntryPos( );
    m_xCurrentDetails = m_aDetailsContainers[nPos];
    m_nCurrentType = nPos;

    m_xCurrentDetails->show();

    m_pCBPassword->Show( m_bShowPassword && m_xCurrentDetails->enableUserCredentials() );
    m_pEDPassword->Show( m_bShowPassword && m_xCurrentDetails->enableUserCredentials() );
    m_pFTPasswordLabel->Show( m_bShowPassword && m_xCurrentDetails->enableUserCredentials() );
    m_pEDUsername->Show( m_xCurrentDetails->enableUserCredentials() );
    m_pFTUsernameLabel->Show( m_xCurrentDetails->enableUserCredentials() );

    SetSizePixel(GetOptimalSize());

    EditHdl(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
