/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/checkin.hxx>
#include <vcl/msgbox.hxx>

SfxCheckinDialog::SfxCheckinDialog( vcl::Window* pParent ) :
    ModalDialog( pParent, "CheckinDialog", "sfx/ui/checkin.ui" )
{
    get( m_pCommentED, "VersionComment" );
    get( m_pMajorCB, "MajorVersion" );

    get( m_pOKBtn, "ok" );
    m_pOKBtn->SetClickHdl( LINK( this, SfxCheckinDialog, OKHdl ) );
}

SfxCheckinDialog::~SfxCheckinDialog()
{
    disposeOnce();
}

void SfxCheckinDialog::dispose()
{
    m_pCommentED.clear();
    m_pMajorCB.clear();
    m_pOKBtn.clear();
    ModalDialog::dispose();
}

OUString SfxCheckinDialog::GetComment( )
{
    return m_pCommentED->GetText( );
}

bool SfxCheckinDialog::IsMajor( )
{
    return m_pMajorCB->IsChecked( );
}

IMPL_LINK_NOARG( SfxCheckinDialog, OKHdl, Button*, void )
{
    EndDialog( RET_OK );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
