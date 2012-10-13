/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "passwdomdlg.hrc"
#include "passwdomdlg.hxx"

#include "cuires.hrc"
#include "dialmgr.hxx"

#include <sfx2/tabdlg.hxx>
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
}


PasswordReenterEdit_Impl::~PasswordReenterEdit_Impl()
{
}


void PasswordReenterEdit_Impl::Paint( const Rectangle& rRect )
{
    if (GetText().Len() == 0)
    {
        Push( PUSH_TEXTCOLOR );
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

    String                      m_aOneMismatch;
    String                      m_aTwoMismatch;
    String                      m_aInvalidStateForOkButton;
    String                      m_aInvalidStateForOkButton_v2;

    bool                        m_bIsPasswordToModify;


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
    m_aOneMismatch( CUI_RES( STR_ONE_PASSWORD_MISMATCH ) ),
    m_aTwoMismatch( CUI_RES( STR_TWO_PASSWORDS_MISMATCH ) ),
    m_aInvalidStateForOkButton( CUI_RES( STR_INVALID_STATE_FOR_OK_BUTTON ) ),
    m_aInvalidStateForOkButton_v2( CUI_RES( STR_INVALID_STATE_FOR_OK_BUTTON_V2 ) ),
    m_bIsPasswordToModify( bIsPasswordToModify )
{
    m_aMoreFewerOptionsBTN.SetMoreText( String( CUI_RES( STR_MORE_OPTIONS ) ) );
    m_aMoreFewerOptionsBTN.SetLessText( String( CUI_RES( STR_FEWER_OPTIONS ) ) );

    m_aOk.SetClickHdl( LINK( this, PasswordToOpenModifyDialog_Impl, OkBtnClickHdl ) );

    if (nMaxPasswdLen)
    {
        m_aPasswdToOpenED.SetMaxTextLen( nMaxPasswdLen );
        m_aReenterPasswdToOpenED.SetMaxTextLen( nMaxPasswdLen );
        m_aPasswdToModifyED.SetMaxTextLen( nMaxPasswdLen );
        m_aReenterPasswdToModifyED.SetMaxTextLen( nMaxPasswdLen );
    }

    (void) nMinPasswdLen;   // currently not supported

    m_aPasswdToOpenED.GrabFocus();

    m_aMoreFewerOptionsBTN.Enable( bIsPasswordToModify );
    if (!bIsPasswordToModify)
        m_aMoreFewerOptionsBTN.Hide( sal_True );
}


PasswordToOpenModifyDialog_Impl::~PasswordToOpenModifyDialog_Impl()
{
}

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
