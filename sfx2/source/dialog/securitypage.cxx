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
#include "sfx2/sfx.hrc"
#include "securitypage.hrc"
#include "sfxresid.hxx"

#include <vcl/button.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <svl/eitem.hxx>
#include <svx/svxids.hrc>
#include <sfx2/viewsh.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/poolitem.hxx>
#include <svtools/xwindowitem.hxx>
#include <sfx2/objsh.hxx>
#include <svl/intitem.hxx>
#include <svx/htmlmode.hxx>



//////////////////////////////////////////////////////////////////////

namespace
{
    enum RedliningMode  { RL_NONE, RL_WRITER, RL_CALC };

    enum RedlineFunc    { RF_ON, RF_PROTECT };

    const SfxBoolItem* ExecuteRecordChangesFunc( RedliningMode _eMode, RedlineFunc _eFunc, BOOL _bVal, Window* _pParent = NULL )
    {
        const SfxBoolItem* pRet = NULL;

        if (_eMode != RL_NONE)
        {
            USHORT nSlot;
            if (_eMode == RL_WRITER)
                nSlot = _eFunc == RF_ON ? FN_REDLINE_ON : FN_REDLINE_PROTECT;
            else
                nSlot = _eFunc == RF_ON ? FID_CHG_RECORD : SID_CHG_PROTECT;

            // execute
            SfxViewShell* pViewSh = SfxViewShell::Current();
            if (pViewSh)
            {
                bool bNeedItem = _eMode == RL_WRITER || _eFunc != RF_ON;
                SfxBoolItem* pItem = bNeedItem ? new SfxBoolItem( nSlot, _bVal ) : NULL;
                SfxDispatcher* pDisp = pViewSh->GetDispatcher();
                if (_pParent)
                {
                    XWindowItem aParentItem( SID_ATTR_XWINDOW, _pParent );
                    pRet = static_cast< const SfxBoolItem* >(
                        pDisp->Execute( nSlot, SFX_CALLMODE_SYNCHRON, &aParentItem, pItem, 0L ) );
                }
                else
                    pRet = static_cast< const SfxBoolItem* >(
                        pDisp->Execute( nSlot, SFX_CALLMODE_SYNCHRON, pItem, 0L ) );
                delete pItem;
            }
        }

        return pRet;
    }

    bool QueryState( USHORT _nSlot, bool& _rValue )
    {
        bool bRet = false;

        SfxViewShell* pViewSh = SfxViewShell::Current();
        if (pViewSh)
        {
            const SfxPoolItem* pItem;
            SfxDispatcher* pDisp = pViewSh->GetDispatcher();
            bRet = SFX_ITEM_AVAILABLE <= pDisp->QueryState( _nSlot, pItem );
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
            USHORT nSlot = _eMode == RL_WRITER ? FN_REDLINE_PROTECT : SID_CHG_PROTECT;
            bRet = QueryState( nSlot, _rValue );
        }

        return bRet;
    }

    bool QueryRecordChangesState( RedliningMode _eMode, bool& _rValue )
    {
        bool bRet = false;

        if (_eMode != RL_NONE)
        {
            USHORT nSlot = _eMode == RL_WRITER ? FN_REDLINE_ON : FID_CHG_RECORD;
            bRet = QueryState( nSlot, _rValue );
        }

        return bRet;
    }
}


//////////////////////////////////////////////////////////////////////


struct SfxSecurityPage_Impl
{
    SfxSecurityPage &   m_rDialog;

    FixedLine           m_aPasswordToOpenFL;
    FixedText           m_aPasswordToOpenFT;
    Edit                m_aPasswordToOpenED;
    FixedText           m_aConfirmPasswordToOpenFT;
    Edit                m_aConfirmPasswordToOpenED;
    FixedText           m_aPasswordInfoFT;

    FixedLine           m_aPasswordToModifyFL;
    FixedText           m_aPasswordToModifyFT;
    Edit                m_aPasswordToModifyED;
    FixedText           m_aConfirmPasswordToModifyFT;
    Edit                m_aConfirmPasswordToModifyED;

    FixedLine           m_aOptionsFL;
    CheckBox            m_aOpenReadonlyCB;
    CheckBox            m_aRemoveInfoOnSavingCB;
    CheckBox            m_aRecordChangesCB;         // for record changes
    PushButton          m_aChangeProtectionPB;      // for record changes
    String              m_aProtectSTR;              // for record changes
    String              m_aUnProtectSTR;            // for record changes
    RedliningMode       m_eRedlingMode;             // for record changes

    DECL_LINK( RecordChangesCBHdl, void* );
    DECL_LINK( ChangeProtectionPBHdl, void* );

    SfxSecurityPage_Impl( SfxSecurityPage &rDlg, const SfxItemSet &rItemSet );
    ~SfxSecurityPage_Impl();

    void    CheckRecordChangesState( void );

    BOOL    FillItemSet_Impl( SfxItemSet & );
    void    Reset_Impl( const SfxItemSet & );
};


SfxSecurityPage_Impl::SfxSecurityPage_Impl( SfxSecurityPage &rDlg, const SfxItemSet & ) :
    m_rDialog                       (rDlg),
    m_aPasswordToOpenFL             (&rDlg, SfxResId( PASSWORD_TO_OPEN_FL ) ),
    m_aPasswordToOpenFT             (&rDlg, SfxResId( PASSWORD_TO_OPEN_FT ) ),
    m_aPasswordToOpenED             (&rDlg, SfxResId( PASSWORD_TO_OPEN_ED ) ),
    m_aConfirmPasswordToOpenFT      (&rDlg, SfxResId( CONFIRM_PASSWORD_TO_OPEN_FT ) ),
    m_aConfirmPasswordToOpenED      (&rDlg, SfxResId( CONFIRM_PASSWORD_TO_OPEN_ED ) ),
    m_aPasswordInfoFT               (&rDlg, SfxResId( PASSWORD_INFO_FT ) ),
    m_aPasswordToModifyFL           (&rDlg, SfxResId( PASSWORD_TO_MODIFY_FL ) ),
    m_aPasswordToModifyFT           (&rDlg, SfxResId( PASSWORD_TO_MODIFY_FT ) ),
    m_aPasswordToModifyED           (&rDlg, SfxResId( PASSWORD_TO_MODIFY_ED ) ),
    m_aConfirmPasswordToModifyFT    (&rDlg, SfxResId( CONFIRM_PASSWORD_TO_MODIFY_FT ) ),
    m_aConfirmPasswordToModifyED    (&rDlg, SfxResId( CONFIRM_PASSWORD_TO_MODIFY_ED ) ),
    m_aOptionsFL                    (&rDlg, SfxResId( OPTIONS_FL ) ),
    m_aOpenReadonlyCB               (&rDlg, SfxResId( OPEN_READONLY_CB ) ),
    m_aRemoveInfoOnSavingCB         (&rDlg, SfxResId( REMOVE_INFO_ON_SAVING_CB ) ),
    m_aRecordChangesCB              (&rDlg, SfxResId( RECORD_CHANGES_CB ) ),
    m_aChangeProtectionPB           (&rDlg, SfxResId( CHANGE_PROTECTION_PB ) ),
    m_aProtectSTR                   ( SfxResId( STR_PROTECT ) ),
    m_aUnProtectSTR                 ( SfxResId( STR_UNPROTECT ) ),
    m_eRedlingMode                  ( RL_NONE )
{
    m_aChangeProtectionPB.SetText( m_aProtectSTR );
    // adjust button width if necessary
    long nBtnTextWidth = 0;
    long nTemp = m_aChangeProtectionPB.GetCtrlTextWidth( m_aChangeProtectionPB.GetText() );
    if (nTemp > nBtnTextWidth)
        nBtnTextWidth = nTemp;

    m_aRecordChangesCB.SetClickHdl( LINK( this, SfxSecurityPage_Impl, RecordChangesCBHdl ) );
    m_aChangeProtectionPB.SetClickHdl( LINK( this, SfxSecurityPage_Impl, ChangeProtectionPBHdl ) );
}


SfxSecurityPage_Impl::~SfxSecurityPage_Impl()
{
}


void SfxSecurityPage_Impl::CheckRecordChangesState( void )
{
    bool bVal;
    if (QueryRecordChangesState( m_eRedlingMode, bVal ))
    {
        m_aRecordChangesCB.Enable();
        m_aRecordChangesCB.Check( bVal );
    }
    else
        m_aRecordChangesCB.Disable();        // because now we don't know the state!

    m_aChangeProtectionPB.Enable( QueryRecordChangesProtectionState( m_eRedlingMode, bVal ) );
}


BOOL SfxSecurityPage_Impl::FillItemSet_Impl( SfxItemSet & )
{
    BOOL bModified = FALSE;

    SfxObjectShell* pCurDocShell = SfxObjectShell::Current();
    if (pCurDocShell)
    {
        if (pCurDocShell->HasSecurityOptOpenReadOnly())
            pCurDocShell->SetSecurityOptOpenReadOnly( m_aOpenReadonlyCB.IsChecked() );
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
                USHORT nMode = static_cast< const SfxUInt16Item* >( pItem )->GetValue();
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

        bool bVal;
        if (QueryRecordChangesState( RL_WRITER, bVal ) && !bIsHTMLDoc)
            m_eRedlingMode = RL_WRITER;
        else if (QueryRecordChangesState( RL_CALC, bVal ))
            m_eRedlingMode = RL_CALC;
        else
            m_eRedlingMode = RL_NONE;

        if (m_eRedlingMode != RL_NONE)
        {
            m_aRecordChangesCB.Check( bVal );
            m_aRecordChangesCB.Enable( !bVal && !bIsReadonly );
            m_aChangeProtectionPB.Enable(
                QueryRecordChangesProtectionState( m_eRedlingMode, bVal ) && !bIsReadonly );
            // set the right text
            if (bVal)
                sNewText = m_aUnProtectSTR;
        }
    }

    m_aChangeProtectionPB.SetText( sNewText );
}


IMPL_LINK( SfxSecurityPage_Impl, RecordChangesCBHdl, void*, EMPTYARG )
{
    ExecuteRecordChangesFunc( m_eRedlingMode, RF_ON, m_aRecordChangesCB.IsChecked(), &m_rDialog );
    CheckRecordChangesState();
    return 0;
}


IMPL_LINK( SfxSecurityPage_Impl, ChangeProtectionPBHdl, void*, EMPTYARG )
{
    bool bProt;
    QueryRecordChangesProtectionState( m_eRedlingMode, bProt );
    ExecuteRecordChangesFunc( m_eRedlingMode, RF_PROTECT, !bProt, &m_rDialog );
    CheckRecordChangesState();

    if (QueryRecordChangesProtectionState( m_eRedlingMode, bProt ))
    {
        // RecordChangesCB is enabled if protection is off
        m_aRecordChangesCB.Enable( !bProt );
        // toggle text of button "Protect" <-> "Unprotect"
        String sNewText = bProt ? m_aUnProtectSTR : m_aProtectSTR;
        m_aChangeProtectionPB.SetText( sNewText );
    }
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


BOOL SfxSecurityPage::FillItemSet( SfxItemSet & rItemSet )
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

