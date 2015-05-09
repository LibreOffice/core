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

#include <officecfg/Office/Common.hxx>
#include <svtools/svtresid.hxx>
#include <vcl/msgbox.hxx>

PlaceEditDialog::PlaceEditDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "PlaceEditDialog", "svt/ui/placeedit.ui")
    , m_xCurrentDetails()
{
    get( m_pEDServerName, "name" );
    get( m_pLBServerType, "type" );
    get( m_pEDUsername, "login" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pBTDelete, "delete" );

    m_pBTOk->SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_pBTOk->Enable( false );

    m_pEDServerName->SetModifyHdl( LINK( this, PlaceEditDialog, EditHdl) );

    // This constructor is called when user request a place creation, so
    // delete button is hidden.
    m_pBTDelete->Hide();

    m_pLBServerType->SetSelectHdl( LINK( this, PlaceEditDialog, SelectTypeHdl ) );
    m_pEDUsername->SetModifyHdl( LINK( this, PlaceEditDialog, EditUsernameHdl ) );

    InitDetails( );
}

PlaceEditDialog::PlaceEditDialog(vcl::Window* pParent, const std::shared_ptr<Place>& rPlace)
    : ModalDialog(pParent, "PlaceEditDialog", "svt/ui/placeedit.ui")
    , m_xCurrentDetails( )
{
    get( m_pEDServerName, "name" );
    get( m_pLBServerType, "type" );
    get( m_pEDUsername, "login" );
    get( m_pBTOk, "ok" );
    get( m_pBTCancel, "cancel" );
    get( m_pBTDelete, "delete" );

    m_pBTOk->SetClickHdl( LINK( this, PlaceEditDialog, OKHdl) );
    m_pBTDelete->SetClickHdl ( LINK( this, PlaceEditDialog, DelHdl) );

    m_pEDServerName->SetModifyHdl( LINK( this, PlaceEditDialog, EditHdl) );
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
            m_pLBServerType->SelectEntryPos( i );
            SelectTypeHdl( m_pLBServerType );

            // Fill the Username field
            if ( rUrl.HasUserData( ) )
                m_pEDUsername->SetText( rUrl.GetUser( ) );
        }
    }
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
    m_pBTOk.clear();
    m_pBTCancel.clear();
    m_pBTDelete.clear();
    ModalDialog::dispose();
}

OUString PlaceEditDialog::GetServerUrl()
{
    OUString sUrl;
    if (m_xCurrentDetails.get())
    {
        INetURLObject aUrl = m_xCurrentDetails->getUrl();
        OUString sUsername = OUString( m_pEDUsername->GetText( ) ).trim( );
        if ( !sUsername.isEmpty( ) )
            aUrl.SetUser( sUsername );
        if ( !aUrl.HasError( ) )
            sUrl = aUrl.GetMainURL( INetURLObject::NO_DECODE );
    }

    return sUrl;
}

std::shared_ptr<Place> PlaceEditDialog::GetPlace()
{
    return std::make_shared<Place>(m_pEDServerName->GetText(), GetServerUrl(), true);
}

void PlaceEditDialog::InitDetails( )
{
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

    // Create CMIS control
    std::shared_ptr<DetailsContainer> xCmisDetails(std::make_shared<CmisDetailsContainer>(this));
    xCmisDetails->setChangeHdl( LINK( this, PlaceEditDialog, EditHdl ) );
    m_aDetailsContainers.push_back(xCmisDetails);

    // Set default to first value
    m_pLBServerType->SelectEntryPos( 0 );
    SelectTypeHdl( m_pLBServerType );
}

IMPL_LINK ( PlaceEditDialog,  OKHdl, Button *, )
{
    EndDialog( RET_OK );
    return 1;
}

IMPL_LINK ( PlaceEditDialog, DelHdl, Button *, )
{
    // ReUsing existing symbols...
    EndDialog( RET_NO );
    return 1;
}

IMPL_LINK_NOARG( PlaceEditDialog, EditHdl )
{
    OUString sUrl = GetServerUrl( );
    OUString sName = OUString( m_pEDServerName->GetText() ).trim( );
    m_pBTOk->Enable( !sName.isEmpty( ) && !sUrl.isEmpty( ) );
    return 1;
}

IMPL_LINK_NOARG( PlaceEditDialog, EditUsernameHdl )
{
    for ( std::vector< std::shared_ptr< DetailsContainer > >::iterator it = m_aDetailsContainers.begin( );
            it != m_aDetailsContainers.end( ); ++it )
    {
        ( *it )->setUsername( OUString( m_pEDUsername->GetText() ) );
    }
    return 1;
}

IMPL_LINK_NOARG( PlaceEditDialog, SelectTypeHdl )
{
    if (m_xCurrentDetails.get())
        m_xCurrentDetails->show(false);

    sal_uInt16 nPos = m_pLBServerType->GetSelectEntryPos( );
    m_xCurrentDetails = m_aDetailsContainers[nPos];

    m_xCurrentDetails->show(true);

    SetSizePixel(GetOptimalSize());
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
