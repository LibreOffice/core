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

#undef SC_DLLIMPLEMENTATION

#include "scitems.hxx"
#include <sfx2/basedlgs.hxx>
#include <svl/style.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include "tphf.hxx"
#include "sc.hrc"
#include "scabstdlg.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "hfedtdlg.hxx"
#include "styledlg.hxx"
#include "scresid.hxx"
#include "scuitphfedit.hxx"
#include <memory>

// class ScHFPage

ScHFPage::ScHFPage( vcl::Window* pParent, const SfxItemSet& rSet, sal_uInt16 nSetId )

    :   SvxHFPage   ( pParent, rSet, nSetId ),
        aDataSet    ( *rSet.GetPool(),
                       ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERRIGHT,
                       ATTR_PAGE, ATTR_PAGE, 0 ),
        nPageUsage  ( (sal_uInt16)SVX_PAGE_ALL ),
        pStyleDlg   ( nullptr )
{
    get(m_pBtnEdit, "buttonEdit");

    SetExchangeSupport();

    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( pSh );
    m_pBtnEdit->Show();

    aDataSet.Put( rSet );

    if ( pViewSh )
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocument* pDoc      = rViewData.GetDocument();

        aStrPageStyle = pDoc->GetPageStyle( rViewData.GetTabNo() );
    }

    m_pBtnEdit->SetClickHdl    ( LINK( this, ScHFPage, BtnHdl ) );
    m_pTurnOnBox->SetClickHdl  ( LINK( this, ScHFPage, TurnOnHdl ) );

    if ( nId == SID_ATTR_PAGE_HEADERSET )
        m_pBtnEdit->SetHelpId( HID_SC_HEADER_EDIT );
    else
        m_pBtnEdit->SetHelpId( HID_SC_FOOTER_EDIT );
}

ScHFPage::~ScHFPage()
{
    disposeOnce();
}

void ScHFPage::dispose()
{
    m_pBtnEdit.clear();
    pStyleDlg.clear();
    SvxHFPage::dispose();
}

void ScHFPage::Reset( const SfxItemSet* rSet )
{
    SvxHFPage::Reset( rSet );
    TurnOnHdl( nullptr );
}

bool ScHFPage::FillItemSet( SfxItemSet* rOutSet )
{
    bool bResult = SvxHFPage::FillItemSet( rOutSet );

    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_HEADERLEFT ) );
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_HEADERRIGHT ) );
    }
    else
    {
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_FOOTERLEFT ) );
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_FOOTERRIGHT ) );
    }

    return bResult;
}

void ScHFPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_uInt16          nPageWhich = GetWhich( SID_ATTR_PAGE );
    const SvxPageItem&  rPageItem  = static_cast<const SvxPageItem&>(
                                     rSet.Get(nPageWhich));

    nPageUsage = rPageItem.GetPageUsage();

    if ( pStyleDlg )
        aStrPageStyle = pStyleDlg->GetStyleSheet().GetName();

    aDataSet.Put( rSet.Get(ATTR_PAGE) );

    SvxHFPage::ActivatePage( rSet );
}

SfxTabPage::sfxpg ScHFPage::DeactivatePage( SfxItemSet* pSetP )
{
    if ( LEAVE_PAGE == SvxHFPage::DeactivatePage( pSetP ) )
        if ( pSetP )
            FillItemSet( pSetP );

    return LEAVE_PAGE;
}

void ScHFPage::ActivatePage()
{
}

void ScHFPage::DeactivatePage()
{
}

// Handler:

IMPL_LINK_NOARG_TYPED(ScHFPage, TurnOnHdl, Button*, void)
{
    SvxHFPage::TurnOnHdl( m_pTurnOnBox );

    if ( m_pTurnOnBox->IsChecked() )
        m_pBtnEdit->Enable();
    else
        m_pBtnEdit->Disable();
}

IMPL_LINK_NOARG_TYPED(ScHFPage, BtnHdl, Button*, void)
{
    // When the Edit-Dialog is directly called from the Button's Click-Handler,
    // the GrabFocus from the Edit-Dialog under OS/2 doesn't work.(Bug #41805#).
    // With the new StarView, this workaround should be again considered!

    Application::PostUserEvent( LINK( this, ScHFPage, HFEditHdl ), nullptr, true );
}

IMPL_LINK_NOARG_TYPED(ScHFPage, HFEditHdl, void*, void)
{
    SfxViewShell*   pViewSh = SfxViewShell::Current();

    if ( !pViewSh )
    {
        OSL_FAIL( "Current ViewShell not found." );
        return;
    }

    if (   m_pCntSharedBox->IsEnabled()
        && !m_pCntSharedBox->IsChecked() )
    {
        sal_uInt16 nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                            ? RID_SCDLG_HFED_HEADER
                            : RID_SCDLG_HFED_FOOTER;

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "ScAbstractFactory create fail!");

        std::unique_ptr<SfxAbstractTabDialog> pDlg(pFact->CreateScHFEditDlg(
            pViewSh->GetViewFrame(), this, aDataSet, aStrPageStyle, nResId));

        OSL_ENSURE(pDlg, "Dialog create fail!");
        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }
    }
    else
    {
        OUString  aText;
        VclPtrInstance< SfxSingleTabDialog > pDlg(this, aDataSet);
        const int nSettingsId = 42;
        bool bRightPage =   m_pCntSharedBox->IsChecked()
                         || ( SVX_PAGE_LEFT != SvxPageUsage(nPageUsage) );

        if ( nId == SID_ATTR_PAGE_HEADERSET )
        {
            aText = ScGlobal::GetRscString( STR_PAGEHEADER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightHeaderEditPage::Create( pDlg->get_content_area(), &aDataSet ), nullptr, nSettingsId );
            else
                pDlg->SetTabPage( ScLeftHeaderEditPage::Create( pDlg->get_content_area(), &aDataSet ), nullptr, nSettingsId );
        }
        else
        {
            aText = ScGlobal::GetRscString( STR_PAGEFOOTER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightFooterEditPage::Create( pDlg->get_content_area(), &aDataSet ), nullptr, nSettingsId );
            else
                pDlg->SetTabPage( ScLeftFooterEditPage::Create( pDlg->get_content_area(), &aDataSet ), nullptr, nSettingsId );
        }

        SvxNumType eNumType = static_cast<const SvxPageItem&>(aDataSet.Get(ATTR_PAGE)).GetNumType();
        static_cast<ScHFEditPage*>(pDlg->GetTabPage())->SetNumType(eNumType);

        aText += " (" + ScGlobal::GetRscString( STR_PAGESTYLE );
        aText += ": " + aStrPageStyle + ")";

        pDlg->SetText( aText );

        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }
    }
}

// class ScHeaderPage

ScHeaderPage::ScHeaderPage( vcl::Window* pParent, const SfxItemSet& rSet )
    : ScHFPage( pParent, rSet, SID_ATTR_PAGE_HEADERSET )
{
}

VclPtr<SfxTabPage> ScHeaderPage::Create( vcl::Window* pParent, const SfxItemSet* rCoreSet )
{
    return VclPtr<ScHeaderPage>::Create( pParent, *rCoreSet );
}

const sal_uInt16* ScHeaderPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

// class ScFooterPage

ScFooterPage::ScFooterPage( vcl::Window* pParent, const SfxItemSet& rSet )
    : ScHFPage( pParent, rSet, SID_ATTR_PAGE_FOOTERSET )
{
}

VclPtr<SfxTabPage> ScFooterPage::Create( vcl::Window* pParent, const SfxItemSet* rCoreSet )
{
    return VclPtr<ScFooterPage>::Create( pParent, *rCoreSet );
}

const sal_uInt16* ScFooterPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
