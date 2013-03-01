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



//------------------------------------------------------------------

#define _TPHF_CXX
#include "scitems.hxx"
#include <sfx2/basedlgs.hxx>
#include <svl/style.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

#include "tphf.hxx"
#include "sc.hrc"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "hfedtdlg.hxx"
#include "styledlg.hxx"
#include "scresid.hxx"
#include "scuitphfedit.hxx"
#undef _TPHF_CXX



//==================================================================
// class ScHFPage
//==================================================================

ScHFPage::ScHFPage( Window* pParent, const SfxItemSet& rSet, sal_uInt16 nSetId )

    :   SvxHFPage   ( pParent, rSet, nSetId ),
        aDataSet    ( *rSet.GetPool(),
                       ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERRIGHT,
                       ATTR_PAGE, ATTR_PAGE, 0 ),
        nPageUsage  ( (sal_uInt16)SVX_PAGE_ALL ),
        pStyleDlg   ( NULL )
{
    get(m_pBtnEdit,"buttonEdit");

    SetExchangeSupport();

    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = PTR_CAST(ScTabViewShell,pSh);
    m_pBtnEdit->Show();

    aDataSet.Put( rSet );

    if ( pViewSh )
    {
        ScViewData* pViewData = pViewSh->GetViewData();
        ScDocument* pDoc      = pViewData->GetDocument();

        aStrPageStyle = pDoc->GetPageStyle( pViewData->GetTabNo() );
    }

    m_pBtnEdit->SetClickHdl    ( LINK( this, ScHFPage, BtnHdl ) );
    m_pTurnOnBox->SetClickHdl  ( LINK( this, ScHFPage, TurnOnHdl ) );

    if ( nId == SID_ATTR_PAGE_HEADERSET )
        m_pBtnEdit->SetHelpId( HID_SC_HEADER_EDIT );
    else
        m_pBtnEdit->SetHelpId( HID_SC_FOOTER_EDIT );

    m_pBtnEdit->SetAccessibleRelationMemberOf(m_pFrm);
}

//------------------------------------------------------------------

ScHFPage::~ScHFPage()
{
}

//------------------------------------------------------------------

void ScHFPage::Reset( const SfxItemSet& rSet )
{
    SvxHFPage::Reset( rSet );
    TurnOnHdl( 0 );
}

//------------------------------------------------------------------

sal_Bool ScHFPage::FillItemSet( SfxItemSet& rOutSet )
{
    sal_Bool bResult = SvxHFPage::FillItemSet( rOutSet );

    if ( nId == SID_ATTR_PAGE_HEADERSET )
    {
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_HEADERLEFT ) );
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_HEADERRIGHT ) );
    }
    else
    {
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_FOOTERLEFT ) );
        rOutSet.Put( aDataSet.Get( ATTR_PAGE_FOOTERRIGHT ) );
    }

    return bResult;
}

//------------------------------------------------------------------

void ScHFPage::ActivatePage( const SfxItemSet& rSet )
{
    sal_uInt16          nPageWhich = GetWhich( SID_ATTR_PAGE );
    const SvxPageItem&  rPageItem  = (const SvxPageItem&)
                                     rSet.Get(nPageWhich);

    nPageUsage = rPageItem.GetPageUsage();

    if ( pStyleDlg )
        aStrPageStyle = pStyleDlg->GetStyleSheet().GetName();

    aDataSet.Put( rSet.Get(ATTR_PAGE) );

    SvxHFPage::ActivatePage( rSet );
}

//------------------------------------------------------------------

int ScHFPage::DeactivatePage( SfxItemSet* pSetP )
{
    if ( LEAVE_PAGE == SvxHFPage::DeactivatePage( pSetP ) )
        if ( pSetP )
            FillItemSet( *pSetP );

    return LEAVE_PAGE;
}

//------------------------------------------------------------------

void ScHFPage::ActivatePage()
{
}

void ScHFPage::DeactivatePage()
{
}

//------------------------------------------------------------------
// Handler:
//------------------------------------------------------------------

IMPL_LINK_NOARG(ScHFPage, TurnOnHdl)
{
    SvxHFPage::TurnOnHdl( m_pTurnOnBox );

    if ( m_pTurnOnBox->IsChecked() )
        m_pBtnEdit->Enable();
    else
        m_pBtnEdit->Disable();

    return 0;
}


//------------------------------------------------------------------

IMPL_LINK_NOARG(ScHFPage, BtnHdl)
{
    // When the Edit-Dialog is directly called from the Button's Click-Handler,
    // the GrabFocus from the Edit-Dialog under OS/2 doesn't work.(Bug #41805#).
    // With the new StarView, this workaround should be again considered!

    Application::PostUserEvent( LINK( this, ScHFPage, HFEditHdl ) );
    return 0;
}

IMPL_LINK_NOARG(ScHFPage, HFEditHdl)
{
    SfxViewShell*   pViewSh = SfxViewShell::Current();

    if ( !pViewSh )
    {
        OSL_FAIL( "Current ViewShell not found." );
        return 0;
    }

    if (   m_pCntSharedBox->IsEnabled()
        && !m_pCntSharedBox->IsChecked() )
    {
        sal_uInt16 nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                            ? RID_SCDLG_HFED_HEADER
                            : RID_SCDLG_HFED_FOOTER;

        ScHFEditDlg* pDlg
             = new ScHFEditDlg( pViewSh->GetViewFrame(), this,
                                aDataSet, aStrPageStyle, nResId );

        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }

        delete pDlg;
    }
    else
    {
        OUString  aText;
        SfxNoLayoutSingleTabDialog* pDlg = new SfxNoLayoutSingleTabDialog( this, aDataSet, 42 );
        sal_Bool bRightPage =   m_pCntSharedBox->IsChecked()
                         || ( SVX_PAGE_LEFT != SvxPageUsage(nPageUsage) );

        if ( nId == SID_ATTR_PAGE_HEADERSET )
        {
            aText = ScGlobal::GetRscString( STR_PAGEHEADER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightHeaderEditPage::Create( pDlg, aDataSet ) );
            else
                pDlg->SetTabPage( ScLeftHeaderEditPage::Create( pDlg, aDataSet ) );
        }
        else
        {
            aText = ScGlobal::GetRscString( STR_PAGEFOOTER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightFooterEditPage::Create( pDlg, aDataSet ) );
            else
                pDlg->SetTabPage( ScLeftFooterEditPage::Create( pDlg, aDataSet ) );
        }

        SvxNumType eNumType = ((const SvxPageItem&)aDataSet.Get(ATTR_PAGE)).GetNumType();
        ((ScHFEditPage*)pDlg->GetTabPage())->SetNumType(eNumType);

        aText += " (" + ScGlobal::GetRscString( STR_PAGESTYLE );
        aText += ": " + aStrPageStyle + ")";

        pDlg->SetText( aText );

        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }

        delete pDlg;
    }

    return 0;
}

//==================================================================
// class ScHeaderPage
//==================================================================

ScHeaderPage::ScHeaderPage( Window* pParent, const SfxItemSet& rSet )
    : ScHFPage( pParent, rSet, SID_ATTR_PAGE_HEADERSET )
{
}

//------------------------------------------------------------------

SfxTabPage* ScHeaderPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
{
    return ( new ScHeaderPage( pParent, rCoreSet ) );
}

//------------------------------------------------------------------

sal_uInt16* ScHeaderPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

//==================================================================
// class ScFooterPage
//==================================================================

ScFooterPage::ScFooterPage( Window* pParent, const SfxItemSet& rSet )
    : ScHFPage( pParent, rSet, SID_ATTR_PAGE_FOOTERSET )
{
}

//------------------------------------------------------------------

SfxTabPage* ScFooterPage::Create( Window* pParent, const SfxItemSet& rCoreSet )
{
    return ( new ScFooterPage( pParent, rCoreSet ) );
}

//------------------------------------------------------------------

sal_uInt16* ScFooterPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
