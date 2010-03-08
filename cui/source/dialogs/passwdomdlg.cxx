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

    // disallow use of copy c-tor and assignment operator
    PasswordReenterEdit_Impl( const PasswordReenterEdit_Impl & );
    PasswordReenterEdit_Impl & operator = ( const PasswordReenterEdit_Impl & );

public:
    PasswordReenterEdit_Impl( Window * pParent, const ResId &rResId );
    virtual ~PasswordReenterEdit_Impl();

    // Edit
    virtual void        Paint( const Rectangle& rRect );
};


PasswordReenterEdit_Impl::PasswordReenterEdit_Impl( Window * pParent, const ResId &rResId ) :
    Edit( pParent, rResId )
{
    m_aDefaultTxt = String( CUI_RES( STR_PASSWD_MUST_BE_CONFIRMED ) );
}


PasswordReenterEdit_Impl::~PasswordReenterEdit_Impl()
{
}


void PasswordReenterEdit_Impl::Paint( const Rectangle& rRect )
{
    if (GetText().Len() == 0)
    {
        Push( /*PUSH_FILLCOLOR | PUSH_TEXTFILLCOLOR |*/ PUSH_TEXTCOLOR );
/*
        Color aFillColor( GetParent()->GetBackground().GetColor() );
        SetLineColor(); // don't draw a border when painting the Edit field rectangle with the new background color
        SetFillColor( aFillColor );
        SetTextFillColor( aFillColor );
        SetTextColor( GetParent()->GetTextColor() );    // use plain text color even if the Edit field is disabled (it is hard to read the text otherwise)

        DrawRect( Rectangle( Point(), GetOutputSizePixel() ) );
*/
        SetTextColor( Color( COL_GRAY ) );
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
    CheckBox                    m_aOpenReadonlyCB;


    DECL_LINK( ModifyHdl, Edit * );

    PasswordToOpenModifyDialog_Impl( Window * pParent, sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen );
    ~PasswordToOpenModifyDialog_Impl();
};


PasswordToOpenModifyDialog_Impl::PasswordToOpenModifyDialog_Impl(
        Window * pParent,
        sal_uInt16 nMinPasswdLen,
        sal_uInt16 nMaxPasswdLen ) :
    m_pParent( pParent ),
    m_aPasswdToOpenFT           ( pParent, CUI_RES( FT_PASSWD_TO_OPEN ) ),
    m_aPasswdToOpenED           ( pParent, CUI_RES( ED_PASSWD_TO_OPEN ) ),
    m_aReenterPasswdToOpenFT    ( pParent, CUI_RES( FT_REENTER_PASSWD_TO_OPEN ) ),
    m_aReenterPasswdToOpenED    ( pParent, CUI_RES( ED_REENTER_PASSWD_TO_OPEN ) ),
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
    m_aReenterPasswdToModifyED  ( pParent, CUI_RES( ED_REENTER_PASSWD_TO_MODIFY ) ),
    m_aPasswdToModifyMatchFI    ( pParent, CUI_RES( FI_PASSWD_TO_MODIFY_MATCH ) ),
    m_aOpenReadonlyCB           ( pParent, CUI_RES( CB_OPEN_READONLY ) )
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

    m_aOk.Enable( FALSE );

    if (nMaxPasswdLen)
    {
        m_aPasswdToOpenED.SetMaxTextLen( nMaxPasswdLen );
        m_aReenterPasswdToOpenED.SetMaxTextLen( nMaxPasswdLen );
        m_aPasswdToModifyED.SetMaxTextLen( nMaxPasswdLen );
        m_aReenterPasswdToModifyED.SetMaxTextLen( nMaxPasswdLen );
    }

    (void) nMinPasswdLen;   // currently not supported

    m_aPasswdToOpenED.GrabFocus();

    ModifyHdl( NULL );
}


PasswordToOpenModifyDialog_Impl::~PasswordToOpenModifyDialog_Impl()
{
}


IMPL_LINK( PasswordToOpenModifyDialog_Impl, ModifyHdl, Edit *, EMPTYARG /*pEdit*/ )
{
    // force repaints to get the m_aDefaultTxt displayed again
    if (m_aReenterPasswdToOpenED.GetText().Len() == 0)
        m_aReenterPasswdToOpenED.Invalidate();
    if (m_aReenterPasswdToModifyED.GetText().Len() == 0)
        m_aReenterPasswdToModifyED.Invalidate();

    const sal_Int32 nPasswdToOpenLen    = m_aPasswdToOpenED.GetText().Len();
    const sal_Int32 nPasswdToModifyLen  = m_aPasswdToModifyED.GetText().Len();

    const bool bBothEmpty       = nPasswdToOpenLen == 0 && nPasswdToModifyLen == 0;
    const bool bToOpenMatch     = m_aPasswdToOpenED.GetText()   == m_aReenterPasswdToOpenED.GetText();
    const bool bToModifyMatch   = m_aPasswdToModifyED.GetText() == m_aReenterPasswdToModifyED.GetText();

    m_aOk.Enable( bToOpenMatch && bToModifyMatch && !bBothEmpty );

    m_aPasswdToOpenMatchFI.Enable( bToOpenMatch && !bBothEmpty );
    m_aPasswdToModifyMatchFI.Enable( bToModifyMatch && !bBothEmpty );

    return 0;
}


//////////////////////////////////////////////////////////////////////


PasswordToOpenModifyDialog::PasswordToOpenModifyDialog(
        Window * pParent,
        sal_uInt16 nMinPasswdLen,
        sal_uInt16 nMaxPasswdLen ) :
    SfxModalDialog( pParent, CUI_RES( RID_DLG_PASSWORD_TO_OPEN_MODIFY ) )
{
    m_pImpl = std::auto_ptr< PasswordToOpenModifyDialog_Impl >(
            new PasswordToOpenModifyDialog_Impl( this, nMinPasswdLen, nMaxPasswdLen ) );

    FreeResource();
}


PasswordToOpenModifyDialog::~PasswordToOpenModifyDialog()
{
}


String PasswordToOpenModifyDialog::GetPasswordToOpen() const
{
    const bool bPasswdOk =
            m_pImpl->m_aPasswdToOpenED.GetText().Len() > 0 &&
            m_pImpl->m_aPasswdToOpenED.GetText() == m_pImpl->m_aReenterPasswdToOpenED.GetText();
    return bPasswdOk ? m_pImpl->m_aPasswdToOpenED.GetText() : String();
}


String PasswordToOpenModifyDialog::GetPasswordToModify() const
{
    const bool bPasswdOk =
            m_pImpl->m_aPasswdToModifyED.GetText().Len() > 0 &&
            m_pImpl->m_aPasswdToModifyED.GetText() == m_pImpl->m_aReenterPasswdToModifyED.GetText();
    return bPasswdOk ? m_pImpl->m_aPasswdToModifyED.GetText() : String();
}


bool PasswordToOpenModifyDialog::IsRecommendToOpenReadonly() const
{
    return m_pImpl->m_aOpenReadonlyCB.IsChecked();
}


//////////////////////////////////////////////////////////////////////

