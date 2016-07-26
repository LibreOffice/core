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


#include <sfx2/securitypage.hxx>

#include <sfx2/sfxresid.hxx>

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

#include "../appl/app.hrc"


using namespace ::com::sun::star;


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
            bRet = SfxItemState::DEFAULT <= nState;
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


static bool lcl_GetPassword(
    vcl::Window *pParent,
    bool bProtect,
    /*out*/OUString &rPassword )
{
    bool bRes = false;
    ScopedVclPtrInstance< SfxPasswordDialog > aPasswdDlg(pParent);
    aPasswdDlg->SetMinLen( 1 );
    if (bProtect)
        aPasswdDlg->ShowExtras( SfxShowExtras::CONFIRM );
    if (RET_OK == aPasswdDlg->Execute() && !aPasswdDlg->GetPassword().isEmpty())
    {
        rPassword = aPasswdDlg->GetPassword();
        bRes = true;
    }
    return bRes;
}


static bool lcl_IsPasswordCorrect( const OUString &rPassword )
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
        ScopedVclPtrInstance<InfoBox>(nullptr, SFX2_RESSTR(RID_SVXSTR_INCORRECT_PASSWORD))->Execute();

    return bRes;
}


struct SfxSecurityPage_Impl
{
    SfxSecurityPage &   m_rMyTabPage;

    VclPtr<CheckBox>    m_pOpenReadonlyCB;
    VclPtr<CheckBox>    m_pRecordChangesCB;         // for record changes
    VclPtr<PushButton>  m_pProtectPB;               // for record changes
    VclPtr<PushButton>  m_pUnProtectPB;             // for record changes
    RedliningMode       m_eRedlingMode;             // for record changes

    bool                m_bOrigPasswordIsConfirmed;
    bool                m_bNewPasswordIsValid;
    OUString            m_aNewPassword;

    OUString            m_aEndRedliningWarning;
    bool                m_bEndRedliningWarningDone;

    DECL_LINK_TYPED( RecordChangesCBToggleHdl, CheckBox&, void );
    DECL_LINK_TYPED( ChangeProtectionPBHdl, Button*, void );

    SfxSecurityPage_Impl( SfxSecurityPage &rDlg, const SfxItemSet &rItemSet );
    ~SfxSecurityPage_Impl();

    bool    FillItemSet_Impl( SfxItemSet & );
    void    Reset_Impl( const SfxItemSet & );
};


SfxSecurityPage_Impl::SfxSecurityPage_Impl( SfxSecurityPage &rTabPage, const SfxItemSet & ) :
    m_rMyTabPage                    (rTabPage),
    m_eRedlingMode                  ( RL_NONE ),
    m_bOrigPasswordIsConfirmed      ( false ),
    m_bNewPasswordIsValid           ( false ),
    m_aEndRedliningWarning          ( SFX2_RESSTR(RID_SVXSTR_END_REDLINING_WARNING) ),
    m_bEndRedliningWarningDone      ( false )
{
    rTabPage.get(m_pOpenReadonlyCB, "readonly");
    rTabPage.get(m_pRecordChangesCB, "recordchanges");
    rTabPage.get(m_pProtectPB, "protect");
    rTabPage.get(m_pUnProtectPB, "unprotect");
    m_pProtectPB->Show();
    m_pUnProtectPB->Hide();

    // force toggle hdl called before visual change of checkbox
    m_pRecordChangesCB->SetStyle( m_pRecordChangesCB->GetStyle() | WB_EARLYTOGGLE );
    m_pRecordChangesCB->SetToggleHdl( LINK( this, SfxSecurityPage_Impl, RecordChangesCBToggleHdl ) );
    m_pProtectPB->SetClickHdl( LINK( this, SfxSecurityPage_Impl, ChangeProtectionPBHdl ) );
    m_pUnProtectPB->SetClickHdl( LINK( this, SfxSecurityPage_Impl, ChangeProtectionPBHdl ) );
}


SfxSecurityPage_Impl::~SfxSecurityPage_Impl()
{
}


bool SfxSecurityPage_Impl::FillItemSet_Impl( SfxItemSet & )
{
    bool bModified = false;

    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    if (pCurDocShell&& !pCurDocShell->IsReadOnly())
    {
        if (m_eRedlingMode != RL_NONE )
        {
            const bool bDoRecordChanges     = m_pRecordChangesCB->IsChecked();
            const bool bDoChangeProtection  = m_pUnProtectPB->IsVisible();

            // sanity checks
            DBG_ASSERT( bDoRecordChanges || !bDoChangeProtection, "no change recording should imply no change protection" );
            DBG_ASSERT( bDoChangeProtection || !bDoRecordChanges, "no change protection should imply no change recording" );
            DBG_ASSERT( !bDoChangeProtection || !m_aNewPassword.isEmpty(), "change protection should imply password length is > 0" );
            DBG_ASSERT( bDoChangeProtection || m_aNewPassword.isEmpty(), "no change protection should imply password length is 0" );

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
        const bool bDoOpenReadonly = m_pOpenReadonlyCB->IsChecked();
        if (bDoOpenReadonly != pCurDocShell->IsSecurityOptOpenReadOnly())
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

    if (!pCurDocShell)
    {
        // no doc -> hide document settings
        m_pOpenReadonlyCB->Disable();
        m_pRecordChangesCB->Disable();
        m_pProtectPB->Show();
        m_pProtectPB->Disable();
        m_pUnProtectPB->Hide();
        m_pUnProtectPB->Disable();
    }
    else
    {
        bool bIsHTMLDoc = false;
        bool bProtect = true, bUnProtect = false;
        SfxViewShell* pViewSh = SfxViewShell::Current();
        if (pViewSh)
        {
            const SfxPoolItem* pItem;
            SfxDispatcher* pDisp = pViewSh->GetDispatcher();
            if (SfxItemState::DEFAULT <= pDisp->QueryState( SID_HTML_MODE, pItem ))
            {
                sal_uInt16 nMode = static_cast< const SfxUInt16Item* >( pItem )->GetValue();
                bIsHTMLDoc = ( ( nMode & HTMLMODE_ON ) != 0 );
            }
        }

        bool bIsReadonly = pCurDocShell->IsReadOnly();
        if (!bIsHTMLDoc)
        {
            m_pOpenReadonlyCB->Check( pCurDocShell->IsSecurityOptOpenReadOnly() );
            m_pOpenReadonlyCB->Enable( !bIsReadonly );
        }
        else
            m_pOpenReadonlyCB->Disable();

        bool bRecordChanges;
        if (QueryRecordChangesState( RL_WRITER, bRecordChanges ) && !bIsHTMLDoc)
            m_eRedlingMode = RL_WRITER;
        else if (QueryRecordChangesState( RL_CALC, bRecordChanges ))
            m_eRedlingMode = RL_CALC;
        else
            m_eRedlingMode = RL_NONE;

        if (m_eRedlingMode != RL_NONE)
        {
            bool bProtection(false);
            QueryRecordChangesProtectionState( m_eRedlingMode, bProtection );

            m_pProtectPB->Enable( !bIsReadonly );
            m_pUnProtectPB->Enable( !bIsReadonly );
            // set the right text
            if (bProtection)
            {
                bProtect = false;
                bUnProtect = true;
            }

            m_pRecordChangesCB->Check( bRecordChanges );
            m_pRecordChangesCB->Enable( /*!bProtection && */!bIsReadonly );

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
            m_pRecordChangesCB->Check( false );
            m_pRecordChangesCB->Disable();
            m_pProtectPB->Check( false );
            m_pUnProtectPB->Check( false );
            m_pProtectPB->Disable();
            m_pUnProtectPB->Disable();
        }

        m_pProtectPB->Show(bProtect);
        m_pUnProtectPB->Show(bUnProtect);
    }
}


IMPL_LINK_NOARG_TYPED(SfxSecurityPage_Impl, RecordChangesCBToggleHdl, CheckBox&, void)
{
    // when change recording gets disabled protection must be disabled as well
    if (!m_pRecordChangesCB->IsChecked())    // the new check state is already present, thus the '!'
    {
        bool bAlreadyDone = false;
        if (!m_bEndRedliningWarningDone)
        {
            ScopedVclPtrInstance<WarningBox> aBox(m_rMyTabPage.GetParent(), WinBits(WB_YES_NO | WB_DEF_NO),
                    m_aEndRedliningWarning );
            if (aBox->Execute() != RET_YES)
                bAlreadyDone = true;
            else
                m_bEndRedliningWarningDone = true;
        }

        const bool bNeedPasssword = !m_bOrigPasswordIsConfirmed
                && m_pProtectPB->IsVisible();
        if (!bAlreadyDone && bNeedPasssword)
        {
            OUString aPasswordText;

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
            m_pRecordChangesCB->Check();     // restore original state
        else
        {
            // remember required values to change protection and change recording in
            // FillItemSet_Impl later on if password was correct.
            m_bNewPasswordIsValid = true;
            m_aNewPassword.clear();
            m_pProtectPB->Show();
            m_pUnProtectPB->Hide();
        }
    }
}


IMPL_LINK_NOARG_TYPED(SfxSecurityPage_Impl, ChangeProtectionPBHdl, Button*, void)
{
    if (m_eRedlingMode == RL_NONE)
        return;

    // the push button text is always the opposite of the current state. Thus:
    const bool bCurrentProtection = m_pUnProtectPB->IsVisible();

    // ask user for password (if still necessary)
    OUString aPasswordText;
    bool bNewProtection = !bCurrentProtection;
    const bool bNeedPassword = bNewProtection || !m_bOrigPasswordIsConfirmed;
    if (bNeedPassword)
    {
        // ask for password and if dialog is canceled or no password provided return
        if (!lcl_GetPassword( m_rMyTabPage.GetParent(), bNewProtection, aPasswordText ))
            return;

        // provided password still needs to be checked?
        if (!bNewProtection && !m_bOrigPasswordIsConfirmed)
        {
            if (lcl_IsPasswordCorrect( aPasswordText ))
                m_bOrigPasswordIsConfirmed = true;
            else
                return;
        }
    }
    DBG_ASSERT( m_bOrigPasswordIsConfirmed, "ooops... this should not have happened!" );

    // remember required values to change protection and change recording in
    // FillItemSet_Impl later on if password was correct.
    m_bNewPasswordIsValid = true;
    m_aNewPassword = bNewProtection? aPasswordText : OUString();

    m_pRecordChangesCB->Check( bNewProtection );

    m_pUnProtectPB->Show(bNewProtection);
    m_pProtectPB->Show(!bNewProtection);
}


VclPtr<SfxTabPage> SfxSecurityPage::Create( vcl::Window * pParent, const SfxItemSet * rItemSet )
{
    return VclPtr<SfxSecurityPage>::Create( pParent, *rItemSet );
}


SfxSecurityPage::SfxSecurityPage( vcl::Window* pParent, const SfxItemSet& rItemSet )
    : SfxTabPage(pParent, "SecurityInfoPage", "sfx/ui/securityinfopage.ui", &rItemSet)
{
    m_pImpl.reset(new SfxSecurityPage_Impl( *this, rItemSet ));
}


bool SfxSecurityPage::FillItemSet( SfxItemSet * rItemSet )
{
    bool bModified = false;
    DBG_ASSERT( m_pImpl.get(), "implementation pointer is 0. Still in c-tor?" );
    if (m_pImpl.get() != nullptr)
        bModified =  m_pImpl->FillItemSet_Impl( *rItemSet );
    return bModified;
}


void SfxSecurityPage::Reset( const SfxItemSet * rItemSet )
{
    DBG_ASSERT( m_pImpl.get(), "implementation pointer is 0. Still in c-tor?" );
    if (m_pImpl.get() != nullptr)
        m_pImpl->Reset_Impl( *rItemSet );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
