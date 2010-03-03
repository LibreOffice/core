/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include "passwdomdlg.hrc"
#include "passwdomdlg.hxx"

#include "cuires.hrc"
#include "dialmgr.hxx"

#include <sfx2/tabdlg.hxx>
#include <tools/debug.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/morebtn.hxx>
#include <vcl/settings.hxx>


//////////////////////////////////////////////////////////////////////

class PasswordReenterEdit_Impl : public Edit
{
    String  m_aDefaultTxt;
    bool    m_bForceNonEmptyPasswd;

    // disallow use of copy c-tor and assignment operator
    PasswordReenterEdit_Impl( const PasswordReenterEdit_Impl & );
    PasswordReenterEdit_Impl & operator = ( const PasswordReenterEdit_Impl & );

public:
    PasswordReenterEdit_Impl( Window * pParent, const ResId &rResId, bool bForceNonEmptyPasswd );
    virtual ~PasswordReenterEdit_Impl();

    // Edit
//    virtual void        Modify();
    virtual void        Paint( const Rectangle& rRect );
};


PasswordReenterEdit_Impl::PasswordReenterEdit_Impl( Window * pParent, const ResId &rResId, bool bForceNonEmptyPasswd ) :
    Edit( pParent, rResId ),
    m_bForceNonEmptyPasswd( bForceNonEmptyPasswd )
{
    m_aDefaultTxt = String( CUI_RES( STR_PASSWD_MUST_BE_CONFIRMED ) );
}


PasswordReenterEdit_Impl::~PasswordReenterEdit_Impl()
{
}

/*
void PasswordReenterEdit_Impl::Modify()
{
    Edit::Modify();
    SetModifyFlag();
    if (m_bForceNonEmptyPasswd && GetText().Len() == 0)
        Invalidate();   // get the m_aDefaultTxt displayed again
}
*/

void PasswordReenterEdit_Impl::Paint( const Rectangle& rRect )
{
    if (!IsEnabled() || !IsModified() || (m_bForceNonEmptyPasswd && GetText().Len() == 0))
    {
        Push( PUSH_FILLCOLOR | PUSH_TEXTFILLCOLOR );

        Color aFillColor( GetParent()->GetBackground().GetColor() );
        SetLineColor();
        SetFillColor( aFillColor );
        SetTextFillColor( aFillColor );

        DrawRect( Rectangle( Point(), GetOutputSizePixel() ) );
        DrawText( Point(), m_aDefaultTxt );

        Pop();
    }
    else
        Edit::Paint( rRect );
}


//////////////////////////////////////////////////////////////////////

struct PasswordToOpenModifyDialog_Impl
{
    Window *                    m_pParent;

    FixedText                   m_aPasswdToOpenFT;
    Edit                        m_aPasswdToOpenED;
    FixedText                   m_aReenterPasswdToOpenFT;
    PasswordReenterEdit_Impl    m_aReenterPasswdToOpenED;
    FixedImage                  m_aPasswdToOpenMatchFI;
    FixedText                   m_aPasswdNoteFT;
    FixedLine                   m_aButtonsFL;
    MoreButton                  m_aMoreFewerOptionsBTN;
    OKButton                    m_aOk;
    CancelButton                m_aCancel;
    FixedLine                   m_aFileSharingOptionsFL;
    FixedText                   m_aPasswdToModifyFT;
    Edit                        m_aPasswdToModifyED;
    FixedText                   m_aReenterPasswdToModifyFT;
    PasswordReenterEdit_Impl    m_aReenterPasswdToModifyED;
    FixedImage                  m_aPasswdToModifyMatchFI;

    bool                        m_bReenterPasswdToOpenEdited;
    bool                        m_bReenterPasswdToModifyEdited;


    DECL_LINK( ModifyHdl, Edit * );

    PasswordToOpenModifyDialog_Impl( Window * pParent );
    ~PasswordToOpenModifyDialog_Impl();
};


PasswordToOpenModifyDialog_Impl::PasswordToOpenModifyDialog_Impl( Window * pParent ) :
    m_pParent( pParent ),
    m_aPasswdToOpenFT           ( pParent, CUI_RES( FT_PASSWD_TO_OPEN ) ),
    m_aPasswdToOpenED           ( pParent, CUI_RES( ED_PASSWD_TO_OPEN ) ),
    m_aReenterPasswdToOpenFT    ( pParent, CUI_RES( FT_REENTER_PASSWD_TO_OPEN ) ),
    m_aReenterPasswdToOpenED    ( pParent, CUI_RES( ED_REENTER_PASSWD_TO_OPEN ), true ),
    m_aPasswdToOpenMatchFI      ( pParent, CUI_RES( FI_PASSWD_TO_OPEN_MATCH ) ),
    m_aPasswdNoteFT             ( pParent, CUI_RES( FT_PASSWD_NOTE ) ),
    m_aButtonsFL                ( pParent, CUI_RES( FL_BUTTONS ) ),
    m_aMoreFewerOptionsBTN      ( pParent, CUI_RES( BTN_MORE_FEWER_OPTIONS ) ),
    m_aOk                       ( pParent, CUI_RES( BTN_OK ) ),
    m_aCancel                   ( pParent, CUI_RES( BTN_CANCEL ) ),
    m_aFileSharingOptionsFL     ( pParent, CUI_RES( FL_FILE_SHARING_OPTIONS ) ),
    m_aPasswdToModifyFT         ( pParent, CUI_RES( FT_PASSWD_TO_MODIFY ) ),
    m_aPasswdToModifyED         ( pParent, CUI_RES( ED_PASSWD_TO_MODIFY ) ),
    m_aReenterPasswdToModifyFT  ( pParent, CUI_RES( FT_REENTER_PASSWD_TO_MODIFY ) ),
    m_aReenterPasswdToModifyED  ( pParent, CUI_RES( ED_REENTER_PASSWD_TO_MODIFY ), false ),
    m_aPasswdToModifyMatchFI    ( pParent, CUI_RES( FI_PASSWD_TO_MODIFY_MATCH ) ),
    m_bReenterPasswdToOpenEdited    ( false ),
    m_bReenterPasswdToModifyEdited  ( false )
{
    const sal_Bool bHighContrast = pParent->GetSettings().GetStyleSettings().GetHighContrastMode();
    const Image aImage( CUI_RES( bHighContrast ? IMG_PASSWD_MATCH_HC : IMG_PASSWD_MATCH ) );
    m_aPasswdToOpenMatchFI.SetImage( aImage );
    m_aPasswdToModifyMatchFI.SetImage( aImage );

    m_aMoreFewerOptionsBTN.SetMoreText( String( CUI_RES( STR_MORE_OPTIONS ) ) );
    m_aMoreFewerOptionsBTN.SetLessText( String( CUI_RES( STR_FEWER_OPTIONS ) ) );

    Link aModifyLink = LINK( this, PasswordToOpenModifyDialog_Impl, ModifyHdl );
    m_aPasswdToOpenED.SetModifyHdl( aModifyLink );
    m_aReenterPasswdToOpenED.SetModifyHdl( aModifyLink );
    m_aPasswdToModifyED.SetModifyHdl( aModifyLink );
    m_aReenterPasswdToModifyED.SetModifyHdl( aModifyLink );

    m_aReenterPasswdToOpenED.Enable( FALSE );
    m_aReenterPasswdToModifyED.Enable( FALSE );
    m_aOk.Enable( FALSE );

    ModifyHdl( NULL );
}


PasswordToOpenModifyDialog_Impl::~PasswordToOpenModifyDialog_Impl()
{
}


IMPL_LINK( PasswordToOpenModifyDialog_Impl, ModifyHdl, Edit *, pEdit )
{
    if (pEdit == &m_aReenterPasswdToOpenED)
        m_bReenterPasswdToOpenEdited = true;
    if (pEdit == &m_aReenterPasswdToModifyED)
        m_bReenterPasswdToModifyEdited = true;

    m_aReenterPasswdToOpenED.Enable( m_aPasswdToOpenED.GetText().Len() > 0 );
    m_aReenterPasswdToModifyED.Enable( m_aPasswdToModifyED.GetText().Len() > 0 );

    const bool bToOpenEqual     = m_aPasswdToOpenED.GetText() == m_aReenterPasswdToOpenED.GetText();
    const bool bToModifyEqual   = m_aPasswdToModifyED.GetText() == m_aReenterPasswdToModifyED.GetText();
    const BOOL bEnableOk =
            m_bReenterPasswdToOpenEdited && m_aPasswdToOpenED.GetText().Len() > 0 &&
            bToOpenEqual && (!m_bReenterPasswdToModifyEdited || bToModifyEqual);
    m_aOk.Enable( bEnableOk );

    m_aPasswdToOpenMatchFI.Enable( bToOpenEqual && m_aPasswdToOpenED.GetText().Len() > 0 );
    m_aPasswdToModifyMatchFI.Enable( bToModifyEqual );

    if (m_aReenterPasswdToOpenED.GetText().Len() == 0)
        m_aReenterPasswdToOpenED.Invalidate();   // get the m_aDefaultTxt displayed again

    return 0;
}


//////////////////////////////////////////////////////////////////////


PasswordToOpenModifyDialog::PasswordToOpenModifyDialog( Window * pParent ) :
    SfxModalDialog( pParent, CUI_RES( RID_DLG_PASSWORD_TO_OPEN_MODIFY ) )
{
    m_pImpl = boost::shared_ptr< PasswordToOpenModifyDialog_Impl >( new PasswordToOpenModifyDialog_Impl( this ) );

    FreeResource();
}


PasswordToOpenModifyDialog::~PasswordToOpenModifyDialog()
{
}


String PasswordToOpenModifyDialog::GetPasswordToOpen() const
{
    const bool bPasswdEditedAndOk =
            m_pImpl->m_bReenterPasswdToOpenEdited && m_pImpl->m_aPasswdToOpenED.GetText().Len() > 0 &&
            m_pImpl->m_aPasswdToOpenED.GetText() == m_pImpl->m_aReenterPasswdToOpenED.GetText();
    return bPasswdEditedAndOk ? m_pImpl->m_aPasswdToOpenED.GetText() : String();
}


String PasswordToOpenModifyDialog::GetPasswordToModify() const
{
    const bool bPasswdEditedAndOk =
            m_pImpl->m_bReenterPasswdToModifyEdited && m_pImpl->m_aPasswdToModifyED.GetText().Len() > 0 &&
            m_pImpl->m_aPasswdToModifyED.GetText() == m_pImpl->m_aReenterPasswdToModifyED.GetText();
    return bPasswdEditedAndOk ? m_pImpl->m_aPasswdToModifyED.GetText() : String();
}

//////////////////////////////////////////////////////////////////////

