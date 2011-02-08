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
#include <vcl/msgbox.hxx>


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
// currently the spec does not want to display this text anymore...
//    m_aDefaultTxt = String( CUI_RES( STR_PASSWD_MUST_BE_CONFIRMED ) );
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
    PasswordToOpenModifyDialog *    m_pParent;

    FixedLine                   m_aFileEncryptionFL;
    FixedText                   m_aPasswdToOpenFT;
    Edit                        m_aPasswdToOpenED;
    FixedText                   m_aReenterPasswdToOpenFT;
    PasswordReenterEdit_Impl    m_aReenterPasswdToOpenED;
//    FixedImage                  m_aPasswdToOpenMatchFI;
    FixedText                   m_aPasswdNoteFT;
    FixedLine                   m_aButtonsFL;
    MoreButton                  m_aMoreFewerOptionsBTN;
    OKButton                    m_aOk;
    CancelButton                m_aCancel;
    FixedLine                   m_aFileSharingOptionsFL;
    CheckBox                    m_aOpenReadonlyCB;
    FixedText                   m_aPasswdToModifyFT;
    Edit                        m_aPasswdToModifyED;
    FixedText                   m_aReenterPasswdToModifyFT;
    PasswordReenterEdit_Impl    m_aReenterPasswdToModifyED;
//    FixedImage                  m_aPasswdToModifyMatchFI;

    String                      m_aOneMismatch;
    String                      m_aTwoMismatch;
    String                      m_aInvalidStateForOkButton;
    String                      m_aInvalidStateForOkButton_v2;

    bool                        m_bIsPasswordToModify;


//    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( OkBtnClickHdl, OKButton * );

    PasswordToOpenModifyDialog_Impl( PasswordToOpenModifyDialog * pParent,
            sal_uInt16 nMinPasswdLen, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify );
    ~PasswordToOpenModifyDialog_Impl();
};


PasswordToOpenModifyDialog_Impl::PasswordToOpenModifyDialog_Impl(
        PasswordToOpenModifyDialog * pParent,
        sal_uInt16 nMinPasswdLen,
        sal_uInt16 nMaxPasswdLen,
        bool bIsPasswordToModify ) :
    m_pParent( pParent ),
    m_aFileEncryptionFL         ( pParent, CUI_RES( FL_FILE_ENCRYPTION ) ),
    m_aPasswdToOpenFT           ( pParent, CUI_RES( FT_PASSWD_TO_OPEN ) ),
    m_aPasswdToOpenED           ( pParent, CUI_RES( ED_PASSWD_TO_OPEN ) ),
    m_aReenterPasswdToOpenFT    ( pParent, CUI_RES( FT_REENTER_PASSWD_TO_OPEN ) ),
    m_aReenterPasswdToOpenED    ( pParent, CUI_RES( ED_REENTER_PASSWD_TO_OPEN ) ),
//    m_aPasswdToOpenMatchFI      ( pParent, CUI_RES( FI_PASSWD_TO_OPEN_MATCH ) ),
    m_aPasswdNoteFT             ( pParent, CUI_RES( FT_PASSWD_NOTE ) ),
    m_aButtonsFL                ( pParent, CUI_RES( FL_BUTTONS ) ),
    m_aMoreFewerOptionsBTN      ( pParent, CUI_RES( BTN_MORE_FEWER_OPTIONS ) ),
    m_aOk                       ( pParent, CUI_RES( BTN_OK ) ),
    m_aCancel                   ( pParent, CUI_RES( BTN_CANCEL ) ),
    m_aFileSharingOptionsFL     ( pParent, CUI_RES( FL_FILE_SHARING_OPTIONS ) ),
    m_aOpenReadonlyCB           ( pParent, CUI_RES( CB_OPEN_READONLY ) ),
    m_aPasswdToModifyFT         ( pParent, CUI_RES( FT_PASSWD_TO_MODIFY ) ),
    m_aPasswdToModifyED         ( pParent, CUI_RES( ED_PASSWD_TO_MODIFY ) ),
    m_aReenterPasswdToModifyFT  ( pParent, CUI_RES( FT_REENTER_PASSWD_TO_MODIFY ) ),
    m_aReenterPasswdToModifyED  ( pParent, CUI_RES( ED_REENTER_PASSWD_TO_MODIFY ) ),
//    m_aPasswdToModifyMatchFI    ( pParent, CUI_RES( FI_PASSWD_TO_MODIFY_MATCH ) )
    m_aOneMismatch( CUI_RES( STR_ONE_PASSWORD_MISMATCH ) ),
    m_aTwoMismatch( CUI_RES( STR_TWO_PASSWORDS_MISMATCH ) ),
    m_aInvalidStateForOkButton( CUI_RES( STR_INVALID_STATE_FOR_OK_BUTTON ) ),
    m_aInvalidStateForOkButton_v2( CUI_RES( STR_INVALID_STATE_FOR_OK_BUTTON_V2 ) ),
    m_bIsPasswordToModify( bIsPasswordToModify )
{
/*
    const sal_Bool bHighContrast = pParent->GetSettings().GetStyleSettings().GetHighContrastMode();
    const Image aImage( CUI_RES( bHighContrast ? IMG_PASSWD_MATCH_HC : IMG_PASSWD_MATCH ) );
    m_aPasswdToOpenMatchFI.SetImage( aImage );
    m_aPasswdToModifyMatchFI.SetImage( aImage );
*/

    m_aMoreFewerOptionsBTN.SetMoreText( String( CUI_RES( STR_MORE_OPTIONS ) ) );
    m_aMoreFewerOptionsBTN.SetLessText( String( CUI_RES( STR_FEWER_OPTIONS ) ) );

#if 0
    Link aModifyLink = LINK( this, PasswordToOpenModifyDialog_Impl, ModifyHdl );
    m_aPasswdToOpenED.SetModifyHdl( aModifyLink );
    m_aReenterPasswdToOpenED.SetModifyHdl( aModifyLink );
    m_aPasswdToModifyED.SetModifyHdl( aModifyLink );
    m_aReenterPasswdToModifyED.SetModifyHdl( aModifyLink );
#endif

    m_aOk.SetClickHdl( LINK( this, PasswordToOpenModifyDialog_Impl, OkBtnClickHdl ) );

//    m_aOk.Enable( sal_False );

    if (nMaxPasswdLen)
    {
        m_aPasswdToOpenED.SetMaxTextLen( nMaxPasswdLen );
        m_aReenterPasswdToOpenED.SetMaxTextLen( nMaxPasswdLen );
        m_aPasswdToModifyED.SetMaxTextLen( nMaxPasswdLen );
        m_aReenterPasswdToModifyED.SetMaxTextLen( nMaxPasswdLen );
    }

    (void) nMinPasswdLen;   // currently not supported

    m_aPasswdToOpenED.GrabFocus();

//    ModifyHdl( NULL );

    m_aMoreFewerOptionsBTN.Enable( bIsPasswordToModify );
    if (!bIsPasswordToModify)
        m_aMoreFewerOptionsBTN.Hide( sal_True );
}


PasswordToOpenModifyDialog_Impl::~PasswordToOpenModifyDialog_Impl()
{
}

#if 0
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

//    m_aPasswdToOpenMatchFI.Enable( bToOpenMatch && !bBothEmpty );
//    m_aPasswdToModifyMatchFI.Enable( bToModifyMatch && !bBothEmpty );

    return 0;
}
#endif


IMPL_LINK( PasswordToOpenModifyDialog_Impl, OkBtnClickHdl, OKButton *, EMPTYARG /*pBtn*/ )
{
    bool bInvalidState = !m_aOpenReadonlyCB.IsChecked() &&
            m_aPasswdToOpenED.GetText().Len() == 0 &&
            m_aPasswdToModifyED.GetText().Len() == 0;
    if (bInvalidState)
    {
        ErrorBox aErrorBox( m_pParent, WB_OK,
            m_bIsPasswordToModify? m_aInvalidStateForOkButton : m_aInvalidStateForOkButton_v2 );
        aErrorBox.Execute();
    }
    else // check for mismatched passwords...
    {
        const bool bToOpenMatch     = m_aPasswdToOpenED.GetText()   == m_aReenterPasswdToOpenED.GetText();
        const bool bToModifyMatch   = m_aPasswdToModifyED.GetText() == m_aReenterPasswdToModifyED.GetText();
        const int nMismatch = (bToOpenMatch? 0 : 1) + (bToModifyMatch? 0 : 1);
        if (nMismatch > 0)
        {
            ErrorBox aErrorBox( m_pParent, WB_OK, nMismatch == 1 ? m_aOneMismatch : m_aTwoMismatch );
            aErrorBox.Execute();

            Edit &rEdit = !bToOpenMatch? m_aPasswdToOpenED : m_aPasswdToModifyED;
            PasswordReenterEdit_Impl &rRepeatEdit = !bToOpenMatch? m_aReenterPasswdToOpenED : m_aReenterPasswdToModifyED;
            String aEmpty;
            if (nMismatch == 1)
            {
                rEdit.SetText( aEmpty );
                rRepeatEdit.SetText( aEmpty );
            }
            else if (nMismatch == 2)
            {
                m_aPasswdToOpenED.SetText( aEmpty );
                m_aReenterPasswdToOpenED.SetText( aEmpty );
                m_aPasswdToModifyED.SetText( aEmpty );
                m_aReenterPasswdToModifyED.SetText( aEmpty );
            }
            rEdit.GrabFocus();
        }
        else
        {
            m_pParent->EndDialog( RET_OK );
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////


PasswordToOpenModifyDialog::PasswordToOpenModifyDialog(
        Window * pParent,
        sal_uInt16 nMinPasswdLen,
        sal_uInt16 nMaxPasswdLen,
        bool bIsPasswordToModify ) :
    SfxModalDialog( pParent, CUI_RES( RID_DLG_PASSWORD_TO_OPEN_MODIFY ) )
{
    m_pImpl = std::auto_ptr< PasswordToOpenModifyDialog_Impl >(
            new PasswordToOpenModifyDialog_Impl( this, nMinPasswdLen, nMaxPasswdLen, bIsPasswordToModify ) );

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

