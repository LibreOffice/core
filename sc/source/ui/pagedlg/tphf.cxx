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
#include <svx/pageitem.hxx>

#include <attrib.hxx>
#include <tphf.hxx>
#include <scres.hrc>
#include <scabstdlg.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <styledlg.hxx>
#include <scuitphfedit.hxx>
#include <memory>
#include <helpids.h>


ScHFPage::ScHFPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet, sal_uInt16 nSetId)
    : SvxHFPage(pPage, pController, rSet, nSetId)
    , aDataSet(*rSet.GetPool(), svl::Items<ATTR_PAGE, ATTR_PAGE, ATTR_PAGE_HEADERLEFT, ATTR_PAGE_FOOTERFIRST>)
    , nPageUsage(SvxPageUsage::All)
    , pStyleDlg(nullptr)
    , m_xBtnEdit(m_xBuilder->weld_button(u"buttonEdit"_ustr))
{
    SetExchangeSupport();

    SfxViewShell*   pSh = SfxViewShell::Current();
    ScTabViewShell* pViewSh = dynamic_cast< ScTabViewShell *>( pSh );
    m_xBtnEdit->show();

    aDataSet.Put( rSet );

    if ( pViewSh )
    {
        ScViewData& rViewData = pViewSh->GetViewData();
        ScDocument& rDoc      = rViewData.GetDocument();

        aStrPageStyle = rDoc.GetPageStyle( rViewData.GetTabNo() );
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
    pStyleDlg = nullptr;
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
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_HEADERFIRST ) );
    }
    else
    {
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_FOOTERLEFT ) );
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_FOOTERRIGHT ) );
        rOutSet->Put( aDataSet.Get( ATTR_PAGE_FOOTERFIRST ) );
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

// Handler:

IMPL_LINK_NOARG(ScHFPage, TurnOnHdl, weld::Toggleable&, void)
{
    SvxHFPage::TurnOnHdl(*m_xTurnOnBox);

    if (m_xTurnOnBox->get_active())
        m_xBtnEdit->set_sensitive(true);
    else
        m_xBtnEdit->set_sensitive(false);
}

IMPL_LINK_NOARG(ScHFPage, BtnHdl, weld::Button&, void)
{
    SfxViewShell*   pViewSh = SfxViewShell::Current();

    if ( !pViewSh )
    {
        OSL_FAIL( "Current ViewShell not found." );
        return;
    }

    if ( (m_xCntSharedBox->get_sensitive() && !m_xCntSharedBox->get_active()) ||
         (m_xCntSharedFirstBox->get_sensitive() && !m_xCntSharedFirstBox->get_active()) )
    {
        sal_uInt16 nResId;

        if ( m_xCntSharedBox->get_sensitive() && !m_xCntSharedBox->get_active() &&
             m_xCntSharedFirstBox->get_sensitive() && !m_xCntSharedFirstBox->get_active() )
        {
            nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                     ? RID_SCDLG_HFED_HEADER
                     : RID_SCDLG_HFED_FOOTER;
        }
        else if (m_xCntSharedBox->get_sensitive() && !m_xCntSharedBox->get_active())
        {
            nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                     ? RID_SCDLG_HFEDIT_SHAREDFIRSTHEADER
                     : RID_SCDLG_HFEDIT_SHAREDFIRSTFOOTER;
        }
        else
        {
            OSL_ENSURE( m_xCntSharedFirstBox->get_sensitive() && !m_xCntSharedFirstBox->get_active(), "This should be logically impossible." );
            nResId = ( nId == SID_ATTR_PAGE_HEADERSET )
                     ? RID_SCDLG_HFEDIT_SHAREDLEFTHEADER
                     : RID_SCDLG_HFEDIT_SHAREDLEFTFOOTER;
        }

        ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();

        VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateScHFEditDlg(
            GetFrameWeld(), aDataSet, aStrPageStyle, nResId));
        pDlg->StartExecuteAsync([this, pDlg](sal_Int32 nResult){
            if ( nResult == RET_OK )
            {
                aDataSet.Put( *pDlg->GetOutputItemSet() );
            }

            pDlg->disposeOnce();
        });
    }
    else
    {
        OUString  aText;
        SfxSingleTabDialogController aDlg(GetFrameWeld(), &aDataSet);
        bool bRightPage = m_xCntSharedBox->get_active() || (SvxPageUsage::Left != nPageUsage);

        if ( nId == SID_ATTR_PAGE_HEADERSET )
        {
            aText = ScResId( STR_PAGEHEADER );
            if ( bRightPage )
                aDlg.SetTabPage(ScRightHeaderEditPage::Create(aDlg.get_content_area(), &aDlg, &aDataSet));
            else
                aDlg.SetTabPage(ScLeftHeaderEditPage::Create(aDlg.get_content_area(), &aDlg, &aDataSet));
        }
        else
        {
            aText = ScResId( STR_PAGEFOOTER );
            if ( bRightPage )
                aDlg.SetTabPage(ScRightFooterEditPage::Create(aDlg.get_content_area(), &aDlg, &aDataSet));
            else
                aDlg.SetTabPage(ScLeftFooterEditPage::Create(aDlg.get_content_area(), &aDlg, &aDataSet));
        }

        SvxNumType eNumType = aDataSet.Get(ATTR_PAGE).GetNumType();
        static_cast<ScHFEditPage*>(aDlg.GetTabPage())->SetNumType(eNumType);

        aText += " (" + ScResId( STR_PAGESTYLE ) +
            ": " + aStrPageStyle + ")";

        aDlg.set_title(aText);

        if (aDlg.run() == RET_OK)
        {
            aDataSet.Put(*aDlg.GetOutputItemSet());
        }
    }
}


ScHeaderPage::ScHeaderPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : ScHFPage(pPage, pController, rSet, SID_ATTR_PAGE_HEADERSET)
{
}

std::unique_ptr<SfxTabPage> ScHeaderPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet)
{
    return std::make_unique<ScHeaderPage>(pPage, pController, *rCoreSet);
}

WhichRangesContainer ScHeaderPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}


ScFooterPage::ScFooterPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : ScHFPage( pPage, pController, rSet, SID_ATTR_PAGE_FOOTERSET )
{
}

std::unique_ptr<SfxTabPage> ScFooterPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rCoreSet)
{
    return std::make_unique<ScFooterPage>(pPage, pController, *rCoreSet);
}

WhichRangesContainer ScFooterPage::GetRanges()
{
    return SvxHeaderPage::GetRanges();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
