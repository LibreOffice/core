/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2012 Julien Levesy <jlevesy@gmail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include "iodlg.hrc"
#include <svtools/svtools.hrc>
#include <vcl/msgbox.hxx>

#include "fpsofficeResMgr.hxx"
#include "PlacesListBox.hxx"
#include "SvtPlaceDialog.hxx"


SvtPlaceDialog::SvtPlaceDialog(	Window* pParent ) :
	ModalDialog( pParent, SvtResId( DLG_SVT_ADDPLACE ) ),
	m_aFTServerUrl( this, SvtResId( FT_ADDPLACE_SERVERURL ) ),
    m_aFTServerName( this, SvtResId( FT_ADDPLACE_SERVERNAME ) ),
    m_aFTServerType( this, SvtResId( FT_ADDPLACE_SERVERTYPE ) ),
    m_aFTServerLogin( this, SvtResId( FT_ADDPLACE_SERVERLOGIN ) ),
    m_aFTServerPassword( this, SvtResId( FT_ADDPLACE_SERVERPASSWORD) ),
	m_aEDServerUrl ( this, SvtResId( ED_ADDPLACE_SERVERURL ) ),
	m_aEDServerName ( this, SvtResId( ED_ADDPLACE_SERVERNAME ) ),
	m_aEDServerType ( this, SvtResId( ED_ADDPLACE_SERVERTYPE ) ),
	m_aEDServerLogin ( this, SvtResId( ED_ADDPLACE_SERVERLOGIN ) ),
	m_aEDServerPassword ( this, SvtResId( ED_ADDPLACE_SERVERPASSWORD ) ),
    m_aBTOk( this, SvtResId( BT_ADDPLACE_OK ) ),
    m_aBTCancel ( this, SvtResId ( BT_ADDPLACE_CANCEL ) ),
    m_aBTDelete ( this, SvtResId (BT_ADDPLACE_DELETE ) )
{
	// This constructor is called when user request a place creation, so
	// delete button is hidden.
	m_aBTOk.SetClickHdl( LINK( this, SvtPlaceDialog, OKHdl) );
	m_aBTOk.Enable( sal_False );

	m_aEDServerName.SetModifyHdl( LINK( this, SvtPlaceDialog, EditHdl) );
	m_aEDServerUrl.SetModifyHdl( LINK( this, SvtPlaceDialog, EditHdl) );

	m_aEDServerUrl.SetUrlFilter( &m_UrlFilter );
	Edit aDummyEdit ( this, SvtResId( ED_ADDPLACE_SERVERURL ) );
	m_aEDServerUrl.SetPosSizePixel( aDummyEdit.GetPosPixel(), aDummyEdit.GetSizePixel() );
	m_aEDServerUrl.Show();
	m_aBTDelete.Hide();
}

SvtPlaceDialog::SvtPlaceDialog( Window* pParent, PlacePtr pPlace ) :
	ModalDialog( pParent, SvtResId( DLG_SVT_ADDPLACE ) ),
	m_aFTServerUrl( this, SvtResId( FT_ADDPLACE_SERVERURL ) ),
    m_aFTServerName( this, SvtResId( FT_ADDPLACE_SERVERNAME ) ),
    m_aFTServerType( this, SvtResId( FT_ADDPLACE_SERVERTYPE ) ),
    m_aFTServerLogin( this, SvtResId( FT_ADDPLACE_SERVERLOGIN ) ),
    m_aFTServerPassword( this, SvtResId( FT_ADDPLACE_SERVERPASSWORD) ),
	m_aEDServerUrl ( this, SvtResId( ED_ADDPLACE_SERVERURL ) ),
	m_aEDServerName ( this, SvtResId( ED_ADDPLACE_SERVERNAME ) ),
	m_aEDServerType ( this, SvtResId( ED_ADDPLACE_SERVERTYPE ) ),
	m_aEDServerLogin ( this, SvtResId( ED_ADDPLACE_SERVERLOGIN ) ),
	m_aEDServerPassword ( this, SvtResId( ED_ADDPLACE_SERVERPASSWORD ) ),
    m_aBTOk( this, SvtResId( BT_ADDPLACE_OK ) ),
    m_aBTCancel ( this, SvtResId ( BT_ADDPLACE_CANCEL ) ),
    m_aBTDelete ( this, SvtResId (BT_ADDPLACE_DELETE ) )
{
	m_aBTOk.SetClickHdl( LINK( this, SvtPlaceDialog, OKHdl) );
	m_aBTDelete.SetClickHdl ( LINK( this, SvtPlaceDialog, DelHdl) );

	m_aEDServerName.SetModifyHdl( LINK( this, SvtPlaceDialog, EditHdl) );
	m_aEDServerUrl.SetModifyHdl( LINK( this, SvtPlaceDialog, EditHdl) );

	m_aEDServerUrl.SetUrlFilter( &m_UrlFilter );
	Edit aDummyEdit ( this, SvtResId( ED_ADDPLACE_SERVERURL ) );
	m_aEDServerUrl.SetPosSizePixel( aDummyEdit.GetPosPixel(), aDummyEdit.GetSizePixel() );
	m_aEDServerUrl.Show();

	m_aEDServerName.SetText( pPlace->GetName() );
	m_aEDServerUrl.SetText( pPlace->GetUrl() );
}

SvtPlaceDialog::~SvtPlaceDialog()
{
}

PlacePtr SvtPlaceDialog::GetPlace()
{
	PlacePtr newPlace( new Place( m_aEDServerName.GetText(), m_aEDServerUrl.GetURL(), Place::e_PlaceLocal, true) );
	return newPlace;
}

IMPL_LINK ( SvtPlaceDialog,  OKHdl, Button *, EMPTYARG )
{
	EndDialog( RET_OK );
	return 1;
}

IMPL_LINK ( SvtPlaceDialog, DelHdl, Button *, EMPTYARG )
{
	// ReUsing existing symbols...
	EndDialog( RET_NO );
	return 1;
}

IMPL_LINK ( SvtPlaceDialog, EditHdl, Edit *, EMPTYARG )
{
	String anUrl = m_aEDServerUrl.GetText();
	anUrl.EraseLeadingChars().EraseTrailingChars();
	String aName = m_aEDServerName.GetText();
	aName.EraseLeadingChars().EraseTrailingChars();
	if ( ( anUrl.Len() ) && ( aName.Len() ) )
	{
		if ( !m_aBTOk.IsEnabled() )
			m_aBTOk.Enable( sal_True );
	}
	else
	{
		if ( m_aBTOk.IsEnabled() )
			m_aBTOk.Enable( sal_False );
	}
	return 1;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
