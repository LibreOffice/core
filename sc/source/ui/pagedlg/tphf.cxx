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

#include <scitems.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svl/style.hxx>
#include <vcl/svapp.hxx>

#include <attrib.hxx>
#include <tphf.hxx>
#include <sc.hrc>
#include <scres.hrc>
#include <scabstdlg.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <hfedtdlg.hxx>
#include <styledlg.hxx>
#include <scuitphfedit.hxx>
#include <memory>
#include <helpids.h>

// class ScHFPage

ScHFPage::ScHFPage(TabPageParent pParent, const SfxItemSet& rSet, sal_uInt16 nSetId)
    : SvxHFPage(pParent, rSet, nSetId)
    , aDataSet(*rSet.GetPool(), svl::Items<ATTR_PAGE, ATTR_PAGE, ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERRIGHT>{})
    , nPageUsage(SvxPageUsage::All)
    , pStyleDlg(nullptr)
    , m_xBtnEdit(m_xBuilder->weld_button("buttonEdit"))
{
    SetExchangeSupport();

    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( pSh );
    m_xBtnEdit->show();

    aDataSet.Put( rSet );

    if ( pViewSh )
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocument* pDoc      = rViewData.GetDocument();

        aStrPageStyle = pDoc->GetPageStyle( rViewData.GetTabNo() );
    }

    m_xBtnEdit->connect_clicked(LINK(this, ScHFPage, BtnHdl));
    m_xTurnOnBox->connect_toggled(LINK(this, ScHFPage, TurnOnHdl));

    if ( nId == SID_ATTR_PAGE_HEADERSET )
        m_xBtnEdit->set_help_id(HID_SC_HEADER_EDIT);
    else
        m_xBtnEdit->set_help_id(HID_SC_FOOTER_EDIT);
}

ScHFPage::~ScHFPage()
{
    disposeOnce();
}

void ScHFPage::dispose()
{
    pStyleDlg = nullptr;
    SvxHFPage::dispose();
}

void ScHFPage::Reset( const SfxItemSet* rSet )
{
    SvxHFPage::Reset( rSet );
    TurnOnHdl(*m_xTurnOnBox);
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

DeactivateRC ScHFPage::DeactivatePage( SfxItemSet* pSetP )
{
    if ( DeactivateRC::LeavePage == SvxHFPage::DeactivatePage( pSetP ) )
        if ( pSetP )
            FillItemSet( pSetP );

    return DeactivateRC::LeavePage;
}

void ScHFPage::ActivatePage()
{
}

void ScHFPage::DeactivatePage()
{
}

// Handler:

IMPL_LINK_NOARG(ScHFPage, TurnOnHdl, weld::ToggleButton&, void)
{
    SvxHFPage::TurnOnHdl(*m_xTurnOnBox);

    if (m_xTurnOnBox->get_active())
        m_xBtnEdit->set_sensitive(true);
    else
        m_xBtnEdit->set_sensitive(false);
}

IMPL_LINK_NOARG(ScHFPage, BtnHdl, weld::Button&, void)
{
    // When the Edit-Dialog is directly called from the Button's Click-Handler,
    // the GrabFocus from the Edit-Dialog under OS/2 doesn't work.(Bug #41805#).
    // With the new StarView, this workaround should be again considered!

    Application::PostUserEvent( LINK( this, ScHFPage, HFEditHdl ), nullptr, true );
}

IMPL_LINK_NOARG(ScHFPage, HFEditHdl, void*, void)
{
    SfxViewShell*   pViewSh = SfxViewShell::Current();

    if ( !pViewSh )
    {
        OSL_FAIL( "Current ViewShell not found." );
        return;
    }

    if (m_xCntSharedBox->get_sensitive() && !m_xCntSharedBox->get_active())
    {
        sal_uInt16 nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                            ? RID_SCDLG_HFED_HEADER
                            : RID_SCDLG_HFED_FOOTER;

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

        VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScHFEditDlg(
            this, aDataSet, aStrPageStyle, nResId));
        pDlg->StartExecuteAsync([this, pDlg](sal_Int32 nResult){
            if ( nResult == RET_OK )
            {
                aDataSet.Put( *pDlg->GetOutputItemSet() );
            }
        });
    }
    else
    {
        OUString  aText;
        VclPtrInstance< SfxSingleTabDialog > pDlg(nullptr, aDataSet);
        bool bRightPage = m_xCntSharedBox->get_active() || (SvxPageUsage::Left != nPageUsage);

        if ( nId == SID_ATTR_PAGE_HEADERSET )
        {
            aText = ScResId( STR_PAGEHEADER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightHeaderEditPage::Create( pDlg->get_content_area(), &aDataSet ) );
            else
                pDlg->SetTabPage( ScLeftHeaderEditPage::Create( pDlg->get_content_area(), &aDataSet ) );
        }
        else
        {
            aText = ScResId( STR_PAGEFOOTER );
            if ( bRightPage )
                pDlg->SetTabPage( ScRightFooterEditPage::Create( pDlg->get_content_area(), &aDataSet ) );
            else
                pDlg->SetTabPage( ScLeftFooterEditPage::Create( pDlg->get_content_area(), &aDataSet ) );
        }

        SvxNumType eNumType = aDataSet.Get(ATTR_PAGE).GetNumType();
        static_cast<ScHFEditPage*>(pDlg->GetTabPage())->SetNumType(eNumType);

        aText += " (" + ScResId( STR_PAGESTYLE );
        aText += ": " + aStrPageStyle + ")";

        pDlg->SetText( aText );

        if ( pDlg->Execute() == RET_OK )
        {
            aDataSet.Put( *pDlg->GetOutputItemSet() );
        }
    }
}

// class ScHeaderPage

ScHeaderPage::ScHeaderPage(TabPageParent pParent, const SfxItemSet& rSet)
    : ScHFPage(pParent, rSet, SID_ATTR_PAGE_HEADERSET)
{
}

VclPtr<SfxTabPage> ScHeaderPage::Create(TabPageParent pParent, const SfxItemSet* rCoreSet)
{
    return VclPtr<ScHeaderPage>::Create(pParent, *rCoreSet);
}

const sal_uInt16* ScHeaderPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

// class ScFooterPage

ScFooterPage::ScFooterPage(TabPageParent pParent, const SfxItemSet& rSet)
    : ScHFPage( pParent, rSet, SID_ATTR_PAGE_FOOTERSET )
{
}

VclPtr<SfxTabPage> ScFooterPage::Create(TabPageParent pParent, const SfxItemSet* rCoreSet)
{
    return VclPtr<ScFooterPage>::Create(pParent, *rCoreSet);
}

const sal_uInt16* ScFooterPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
