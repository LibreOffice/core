/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sfx2.hxx"

#include "sfx2/securitypage.hxx"

#include "securitypage.hrc"
#include "sfxresid.hxx"

#include <sfx2/sfx.hrc>
#include <sfx2/sfxsids.hrc>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/passwd.hxx>

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
#include <svl/eitem.hxx>
#include <svl/poolitem.hxx>
#include <svl/intitem.hxx>
#include <svl/PasswordHelper.hxx>
#include <svtools/xwindowitem.hxx>


using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////


namespace
{
    enum RedliningMode  { RL_NONE, RL_WRITER, RL_CALC };
    enum RedlineFunc    { RF_ON, RF_PROTECT };

    bool QueryState( sal_uInt16 _nSlot, bool& _rValue )
    {
        bool bRet = false;
        SfxViewShell* pViewSh = SfxViewShell::Current();
        if (pViewSh)
        {
            const SfxPoolItem* pItem;
            SfxDispatcher* pDisp = pViewSh->GetDispatcher();
            SfxItemState nState = pDisp->QueryState( _nSlot, pItem );
            bRet = SFX_ITEM_AVAILABLE <= nState;
            if (bRet)
                _rValue = ( static_cast< const SfxBoolItem* >( pItem ) )->GetValue();
        }
        return bRet;
    }


    bool QueryRecordChangesProtectionState( RedliningMode _eMode, bool& _rValue )
    {
        bool bRet = false;
        if (_eMode != RL_NONE)
        {
            sal_uInt16 nSlot = _eMode == RL_WRITER ? FN_REDLINE_PROTECT : SID_CHG_PROTECT;
            bRet = QueryState( nSlot, _rValue );
        }
        return bRet;
    }


    bool QueryRecordChangesState( RedliningMode _eMode, bool& _rValue )
    {
        bool bRet = false;
        if (_eMode != RL_NONE)
        {
            sal_uInt16 nSlot = _eMode == RL_WRITER ? FN_REDLINE_ON : FID_CHG_RECORD;
            bRet = QueryState( nSlot, _rValue );
        }
        return bRet;
    }
}

//////////////////////////////////////////////////////////////////////

static short lcl_GetPassword(
    Window *pParent,
    bool bProtect,
    /*out*/String &rPassword )
{
    bool bRes = false;
    SfxPasswordDialog aPasswdDlg( pParent );
    aPasswdDlg.SetMinLen( 1 );
    if (bProtect)
        aPasswdDlg.ShowExtras( SHOWEXTRAS_CONFIRM );
    if (RET_OK == aPasswdDlg.Execute() && aPasswdDlg.GetPassword().Len() > 0)
    {
        rPassword = aPasswdDlg.GetPassword();
        bRes = true;
    }
    return bRes;
}


static bool lcl_IsPasswordCorrect( const String &rPassword )
{
    bool bRes = false;

    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    uno::Sequence< sal_Int8 >   aPasswordHash;
    pCurDocShell->GetProtectionHash( aPasswordHash );

    // check if supplied password was correct
    uno::Sequence< sal_Int8 > aNewPasswd( aPasswordHash );
    SvPasswordHelper::GetHashPassword( aNewPasswd, rPassword );
    if (SvPasswordHelper::CompareHashPassword( aPasswordHash, rPassword ))
        bRes = true;    // password was correct
    else
        InfoBox( NULL, String( SfxResId( RID_SFX_INCORRECT_PASSWORD ) ) ).Execute();

    return bRes;
}

//////////////////////////////////////////////////////////////////////

struct SfxSecurityPage_Impl
{
    SfxSecurityPage &   m_rMyTabPage;

    FixedLine           m_aNewPasswordToOpenFL;
    FixedText           m_aNewPasswordToOpenFT;
    Edit                m_aNewPasswordToOpenED;
    FixedText           m_aConfirmPasswordToOpenFT;
    Edit                m_aConfirmPasswordToOpenED;
    FixedText           m_aNewPasswordInfoFT;

    FixedLine           m_aNewPasswordToModifyFL;
    FixedText           m_aNewPasswordToModifyFT;
    Edit                m_aNewPasswordToModifyED;
    FixedText           m_aConfirmPasswordToModifyFT;
    Edit                m_aConfirmPasswordToModifyED;

    FixedLine           m_aOptionsFL;
    CheckBox            m_aOpenReadonlyCB;
    CheckBox            m_aRecordChangesCB;         // for record changes
    PushButton          m_aChangeProtectionPB;      // for record changes
    String              m_aProtectSTR;              // for record changes
    String              m_aUnProtectSTR;            // for record changes
    RedliningMode       m_eRedlingMode;             // for record changes

    bool                m_bOrigPasswordIsConfirmed;
    bool                m_bNewPasswordIsValid;
    String              m_aNewPassword;

    String              m_aEndRedliningWarning;
    bool                m_bEndRedliningWarningDone;

    DECL_LINK( RecordChangesCBToggleHdl, void* );
    DECL_LINK( ChangeProtectionPBHdl, void* );

    SfxSecurityPage_Impl( SfxSecurityPage &rDlg, const SfxItemSet &rItemSet );
    ~SfxSecurityPage_Impl();

    sal_Bool    FillItemSet_Impl( SfxItemSet & );
    void    Reset_Impl( const SfxItemSet & );
};


SfxSecurityPage_Impl::SfxSecurityPage_Impl( SfxSecurityPage &rTabPage, const SfxItemSet & ) :
    m_rMyTabPage                    (rTabPage),
    m_aNewPasswordToOpenFL          (&rTabPage, SfxResId( PASSWORD_TO_OPEN_FL ) ),
    m_aNewPasswordToOpenFT          (&rTabPage, SfxResId( PASSWORD_TO_OPEN_FT ) ),
    m_aNewPasswordToOpenED          (&rTabPage, SfxResId( PASSWORD_TO_OPEN_ED ) ),
    m_aConfirmPasswordToOpenFT      (&rTabPage, SfxResId( CONFIRM_PASSWORD_TO_OPEN_FT ) ),
    m_aConfirmPasswordToOpenED      (&rTabPage, SfxResId( CONFIRM_PASSWORD_TO_OPEN_ED ) ),
    m_aNewPasswordInfoFT            (&rTabPage, SfxResId( PASSWORD_INFO_FT ) ),
    m_aNewPasswordToModifyFL        (&rTabPage, SfxResId( PASSWORD_TO_MODIFY_FL ) ),
    m_aNewPasswordToModifyFT        (&rTabPage, SfxResId( PASSWORD_TO_MODIFY_FT ) ),
    m_aNewPasswordToModifyED        (&rTabPage, SfxResId( PASSWORD_TO_MODIFY_ED ) ),
    m_aConfirmPasswordToModifyFT    (&rTabPage, SfxResId( CONFIRM_PASSWORD_TO_MODIFY_FT ) ),
    m_aConfirmPasswordToModifyED    (&rTabPage, SfxResId( CONFIRM_PASSWORD_TO_MODIFY_ED ) ),
    m_aOptionsFL                    (&rTabPage, SfxResId( OPTIONS_FL ) ),
    m_aOpenReadonlyCB               (&rTabPage, SfxResId( OPEN_READONLY_CB ) ),
    m_aRecordChangesCB              (&rTabPage, SfxResId( RECORD_CHANGES_CB ) ),
    m_aChangeProtectionPB           (&rTabPage, SfxResId( CHANGE_PROTECTION_PB ) ),
    m_aProtectSTR                   ( SfxResId( STR_PROTECT ) ),
    m_aUnProtectSTR                 ( SfxResId( STR_UNPROTECT ) ),
    m_eRedlingMode                  ( RL_NONE ),
    m_bOrigPasswordIsConfirmed      ( false ),
    m_bNewPasswordIsValid           ( false ),
    m_aEndRedliningWarning          ( SfxResId( STR_END_REDLINING_WARNING ) ),
    m_bEndRedliningWarningDone      ( false )
{
    m_aChangeProtectionPB.SetText( m_aProtectSTR );
    // adjust button width if necessary
    long nBtnTextWidth = 0;
    long nTemp = m_aChangeProtectionPB.GetCtrlTextWidth( m_aChangeProtectionPB.GetText() );
    if (nTemp > nBtnTextWidth)
        nBtnTextWidth = nTemp;

    // force toggle hdl called before visual change of checkbox
    m_aRecordChangesCB.SetStyle( m_aRecordChangesCB.GetStyle() | WB_EARLYTOGGLE );
    m_aRecordChangesCB.SetToggleHdl( LINK( this, SfxSecurityPage_Impl, RecordChangesCBToggleHdl ) );
    m_aChangeProtectionPB.SetClickHdl( LINK( this, SfxSecurityPage_Impl, ChangeProtectionPBHdl ) );


    // #i112277: for the time being (OOO 3.3) the following options should not
    // be available. In the long run however it is planned to implement the yet
    // missing functionality. Thus now we hide them and move the remaining ones up.
    m_aNewPasswordToOpenFL.Hide();
    m_aNewPasswordToOpenFT.Hide();
    m_aNewPasswordToOpenED.Hide();
    m_aConfirmPasswordToOpenFT.Hide();
    m_aConfirmPasswordToOpenED.Hide();
    m_aNewPasswordInfoFT.Hide();
    m_aNewPasswordToModifyFL.Hide();
    m_aNewPasswordToModifyFT.Hide();
    m_aNewPasswordToModifyED.Hide();
    m_aConfirmPasswordToModifyFT.Hide();
    m_aConfirmPasswordToModifyED.Hide();
    const long nDelta = m_aOptionsFL.GetPosPixel().Y() - m_aNewPasswordToOpenFL.GetPosPixel().Y();
    Point aPos;
    aPos = m_aOptionsFL.GetPosPixel();
    aPos.Y() -= nDelta;
    m_aOptionsFL.SetPosPixel( aPos );
    aPos = m_aOpenReadonlyCB.GetPosPixel();
    aPos.Y() -= nDelta;
    m_aOpenReadonlyCB.SetPosPixel( aPos );
    aPos = m_aRecordChangesCB.GetPosPixel();
    aPos.Y() -= nDelta;
    m_aRecordChangesCB.SetPosPixel( aPos );
    aPos = m_aChangeProtectionPB.GetPosPixel();
    aPos.Y() -= nDelta;
    m_aChangeProtectionPB.SetPosPixel( aPos );
}


SfxSecurityPage_Impl::~SfxSecurityPage_Impl()
{
}


sal_Bool SfxSecurityPage_Impl::FillItemSet_Impl( SfxItemSet & )
{
    bool bModified = false;

    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    if (pCurDocShell&& !pCurDocShell->IsReadOnly())
    {
        if (m_eRedlingMode != RL_NONE )
        {
            const bool bDoRecordChanges     = m_aRecordChangesCB.IsChecked();
            const bool bDoChangeProtection  = m_aChangeProtectionPB.GetText() != m_aProtectSTR;

            // sanity checks
            DBG_ASSERT( bDoRecordChanges || !bDoChangeProtection, "no change recording should imply no change protection" );
            DBG_ASSERT( bDoChangeProtection || !bDoRecordChanges, "no change protection should imply no change recording" );
            DBG_ASSERT( !bDoChangeProtection || m_aNewPassword.Len() > 0, "change protection should imply password length is > 0" );
            DBG_ASSERT( bDoChangeProtection || m_aNewPassword.Len() == 0, "no change protection should imply password length is 0" );

            // change recording
            if (bDoRecordChanges != pCurDocShell->IsChangeRecording())
            {
                pCurDocShell->SetChangeRecording( bDoRecordChanges );
                bModified = true;
            }

            // change record protection
            if (m_bNewPasswordIsValid &&
                bDoChangeProtection != pCurDocShell->HasChangeRecordProtection())
            {
                DBG_ASSERT( !bDoChangeProtection || bDoRecordChanges,
                        "change protection requires record changes to be active!" );
                pCurDocShell->SetProtectionPassword( m_aNewPassword );
                bModified = true;
            }
        }

        // open read-only?
        const sal_Bool bDoOpenReadonly = m_aOpenReadonlyCB.IsChecked();
        if (pCurDocShell->HasSecurityOptOpenReadOnly() &&
            bDoOpenReadonly != pCurDocShell->IsSecurityOptOpenReadOnly())
        {
            pCurDocShell->SetSecurityOptOpenReadOnly( bDoOpenReadonly );
            bModified = true;
        }
    }

    return bModified;
}


void SfxSecurityPage_Impl::Reset_Impl( const SfxItemSet & )
{
    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();

    String sNewText = m_aProtectSTR;
    if (!pCurDocShell)
    {
        // no doc -> hide document settings
        m_aOpenReadonlyCB.Disable();
        m_aRecordChangesCB.Disable();
        m_aChangeProtectionPB.Disable();
    }
    else
    {
        bool bIsHTMLDoc = false;
        SfxViewShell* pViewSh = SfxViewShell::Current();
        if (pViewSh)
        {
            const SfxPoolItem* pItem;
            SfxDispatcher* pDisp = pViewSh->GetDispatcher();
            if (SFX_ITEM_AVAILABLE <= pDisp->QueryState( SID_HTML_MODE, pItem ))
            {
                sal_uInt16 nMode = static_cast< const SfxUInt16Item* >( pItem )->GetValue();
                bIsHTMLDoc = ( ( nMode & HTMLMODE_ON ) != 0 );
            }
        }

        sal_Bool bIsReadonly = pCurDocShell->IsReadOnly();
        if (pCurDocShell->HasSecurityOptOpenReadOnly() && !bIsHTMLDoc)
        {
            m_aOpenReadonlyCB.Check( pCurDocShell->IsSecurityOptOpenReadOnly() );
            m_aOpenReadonlyCB.Enable( !bIsReadonly );
        }
        else
            m_aOpenReadonlyCB.Disable();

        bool bRecordChanges;
        if (QueryRecordChangesState( RL_WRITER, bRecordChanges ) && !bIsHTMLDoc)
            m_eRedlingMode = RL_WRITER;
        else if (QueryRecordChangesState( RL_CALC, bRecordChanges ))
            m_eRedlingMode = RL_CALC;
        else
            m_eRedlingMode = RL_NONE;

        if (m_eRedlingMode != RL_NONE)
        {
            bool bProtection;
            QueryRecordChangesProtectionState( m_eRedlingMode, bProtection );

            m_aChangeProtectionPB.Enable( !bIsReadonly );
            // set the right text
            if (bProtection)
                sNewText = m_aUnProtectSTR;

            m_aRecordChangesCB.Check( bRecordChanges );
            m_aRecordChangesCB.Enable( /*!bProtection && */!bIsReadonly );

            m_bOrigPasswordIsConfirmed = true;   // default case if no password is set
            uno::Sequence< sal_Int8 > aPasswordHash;
            // check if password is available
            if (pCurDocShell->GetProtectionHash( aPasswordHash ) &&
                aPasswordHash.getLength() > 0)
                m_bOrigPasswordIsConfirmed = false;  // password found, needs to be confirmed later on
        }
        else
        {
            // A Calc document that is shared will have 'm_eRedlingMode == RL_NONE'
            // In shared documents change recording and protection must be disabled,
            // similar to documents that do not support change recording at all.
            m_aRecordChangesCB.Check( sal_False );
            m_aRecordChangesCB.Disable();
            m_aChangeProtectionPB.Check( sal_False );
            m_aChangeProtectionPB.Disable();
        }
    }

    m_aChangeProtectionPB.SetText( sNewText );
}


IMPL_LINK( SfxSecurityPage_Impl, RecordChangesCBToggleHdl, void*, EMPTYARG )
{
    // when change recording gets disabled protection must be disabled as well
    if (!m_aRecordChangesCB.IsChecked())    // the new check state is already present, thus the '!'
    {
        bool bAlreadyDone = false;
        if (!m_bEndRedliningWarningDone)
        {
            WarningBox aBox( m_rMyTabPage.GetParent(), WinBits(WB_YES_NO | WB_DEF_NO),
                    m_aEndRedliningWarning );
            if (aBox.Execute() != RET_YES)
                bAlreadyDone = true;
            else
                m_bEndRedliningWarningDone = true;
        }

        const bool bNeedPasssword = !m_bOrigPasswordIsConfirmed
                && m_aChangeProtectionPB.GetText() != m_aProtectSTR;
        if (!bAlreadyDone && bNeedPasssword)
        {
            String aPasswordText;

            // dialog canceled or no password provided
            if (!lcl_GetPassword( m_rMyTabPage.GetParent(), false, aPasswordText ))
                bAlreadyDone = true;

            // ask for password and if dialog is canceled or no password provided return
            if (lcl_IsPasswordCorrect( aPasswordText ))
                m_bOrigPasswordIsConfirmed = true;
            else
                bAlreadyDone = true;
        }

        if (bAlreadyDone)
            m_aRecordChangesCB.Check( true );     // restore original state
        else
        {
            // remember required values to change protection and change recording in
            // FillItemSet_Impl later on if password was correct.
            m_bNewPasswordIsValid = true;
            m_aNewPassword = String();

            m_aChangeProtectionPB.SetText( m_aProtectSTR );
        }
    }

    return 0;
}


IMPL_LINK( SfxSecurityPage_Impl, ChangeProtectionPBHdl, void*, EMPTYARG )
{
    if (m_eRedlingMode == RL_NONE)
        return 0;

    // the push button text is always the opposite of the current state. Thus:
    const bool bCurrentProtection = m_aChangeProtectionPB.GetText() != m_aProtectSTR;

    // ask user for password (if still necessary)
    String aPasswordText;
    bool bNewProtection = !bCurrentProtection;
    const bool bNeedPassword = bNewProtection || !m_bOrigPasswordIsConfirmed;
    if (bNeedPassword)
    {
        // ask for password and if dialog is canceled or no password provided return
        if (!lcl_GetPassword( m_rMyTabPage.GetParent(), bNewProtection, aPasswordText ))
            return 0;

        // provided password still needs to be checked?
        if (!bNewProtection && !m_bOrigPasswordIsConfirmed)
        {
            if (lcl_IsPasswordCorrect( aPasswordText ))
                m_bOrigPasswordIsConfirmed = true;
            else
                return 0;
        }
    }
    DBG_ASSERT( m_bOrigPasswordIsConfirmed, "ooops... this should not have happened!" );

    // remember required values to change protection and change recording in
    // FillItemSet_Impl later on if password was correct.
    m_bNewPasswordIsValid = true;
    m_aNewPassword = bNewProtection? aPasswordText : String();

    m_aRecordChangesCB.Check( bNewProtection );
    // toggle text of button "Protect" <-> "Unprotect"
    m_aChangeProtectionPB.SetText( bNewProtection ? m_aUnProtectSTR : m_aProtectSTR );

    return 0;
}

//////////////////////////////////////////////////////////////////////

SfxTabPage* SfxSecurityPage::Create( Window * pParent, const SfxItemSet & rItemSet )
{
    return new SfxSecurityPage( pParent, rItemSet );
}


SfxSecurityPage::SfxSecurityPage( Window* pParent, const SfxItemSet& rItemSet ) :
    SfxTabPage( pParent, SfxResId( TP_DOCINFOSECURITY ), rItemSet )
{
    m_pImpl = std::auto_ptr< SfxSecurityPage_Impl >(new SfxSecurityPage_Impl( *this, rItemSet ));

    FreeResource();
}


SfxSecurityPage::~SfxSecurityPage()
{
}


sal_Bool SfxSecurityPage::FillItemSet( SfxItemSet & rItemSet )
{
    bool bModified = false;
    DBG_ASSERT( m_pImpl.get(), "implementation pointer is 0. Still in c-tor?" );
    if (m_pImpl.get() != 0)
        bModified =  m_pImpl->FillItemSet_Impl( rItemSet );
    return bModified;
}


void SfxSecurityPage::Reset( const SfxItemSet & rItemSet )
{
    DBG_ASSERT( m_pImpl.get(), "implementation pointer is 0. Still in c-tor?" );
    if (m_pImpl.get() != 0)
        m_pImpl->Reset_Impl( rItemSet );
}

//////////////////////////////////////////////////////////////////////

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
