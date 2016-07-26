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

#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>

#include <cmdid.h>
#include <uitool.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <basesh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <break.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>

#include <chrdlg.hrc>
#include <SwStyleNameMapper.hxx>

void SwBreakDlg::Apply()
{
    nKind = 0;
    if(m_pLineBtn->IsChecked())
        nKind = 1;
    else if(m_pColumnBtn->IsChecked())
        nKind = 2;
    else if(m_pPageBtn->IsChecked())
    {
        nKind = 3;
        const sal_Int32 nPos = m_pPageCollBox->GetSelectEntryPos();
        if(0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos)
        {
            aTemplate = m_pPageCollBox->GetSelectEntry();
            oPgNum = boost::none;
            if (m_pPageNumBox->IsChecked())
            {
                oPgNum = (sal_uInt16)m_pPageNumEdit->GetValue();
            }
        }
    }
}

IMPL_LINK_NOARG_TYPED(SwBreakDlg, ClickHdl, Button*, void)
{
    CheckEnable();
}

IMPL_LINK_NOARG_TYPED(SwBreakDlg, SelectHdl, ListBox&, void)
{
    CheckEnable();
}

// Handler for Change Page Number
IMPL_LINK_TYPED( SwBreakDlg, PageNumHdl, Button*, pBox, void )
{
    if(static_cast<CheckBox*>(pBox)->IsChecked())
        m_pPageNumEdit->SetValue(1);
    else
        m_pPageNumEdit->SetText(OUString());
}

// By changing the Page number the checkbox is checked.
IMPL_LINK_NOARG_TYPED(SwBreakDlg, PageNumModifyHdl, Edit&, void)
{
    m_pPageNumBox->Check();
}

/*
 * Ok-Handler;
 * checks whether pagenumber nPage is a legal pagenumber (left pages with even
 * numbers etc. for a page template with alternating pages)
 */
IMPL_LINK_NOARG_TYPED(SwBreakDlg, OkHdl, Button*, void)
{
    if(m_pPageNumBox->IsChecked()) {
        // In case of differing page descriptions, test validity
        const sal_Int32 nPos = m_pPageCollBox->GetSelectEntryPos();
        // position 0 says 'Without'.
        const SwPageDesc *pPageDesc;
        if ( 0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos )
            pPageDesc = rSh.FindPageDescByName( m_pPageCollBox->GetSelectEntry(),
                                                true );
        else
            pPageDesc = &rSh.GetPageDesc(rSh.GetCurPageDesc());

        OSL_ENSURE(pPageDesc, "Page description not found.");
        const sal_uInt16 nUserPage = sal_uInt16(m_pPageNumEdit->GetValue());
        bool bOk = true;
        switch(pPageDesc->GetUseOn())
        {
            case nsUseOnPage::PD_MIRROR:
            case nsUseOnPage::PD_ALL: break;
            case nsUseOnPage::PD_LEFT: bOk = 0 == nUserPage % 2; break;
            case nsUseOnPage::PD_RIGHT: bOk = 1 == nUserPage % 2; break;
            default:; //prevent warning
        }
        if(!bOk) {
            ScopedVclPtrInstance<MessageDialog>(this, SW_RES(STR_ILLEGAL_PAGENUM), VCL_MESSAGE_INFO)->Execute();
            m_pPageNumEdit->GrabFocus();
            return;
        }
    }
    EndDialog(RET_OK);
}

SwBreakDlg::SwBreakDlg( vcl::Window *pParent, SwWrtShell &rS )
    : SvxStandardDialog(pParent, "BreakDialog", "modules/swriter/ui/insertbreak.ui")
    , rSh(rS)
    , nKind(0)
    , bHtmlMode(0 != ::GetHtmlMode(rS.GetView().GetDocShell()))
{
    get(m_pLineBtn, "linerb");
    get(m_pColumnBtn, "columnrb");
    get(m_pPageBtn, "pagerb");
    get(m_pPageCollText, "styleft");
    get(m_pPageCollBox, "stylelb");
    get(m_pPageNumBox, "pagenumcb");
    get(m_pPageNumEdit, "pagenumsb");

    m_pPageNumEdit->SetAccessibleName(m_pPageNumBox->GetText());

    Link<Button*,void> aLk = LINK(this,SwBreakDlg,ClickHdl);
    m_pPageBtn->SetClickHdl( aLk );
    m_pLineBtn->SetClickHdl( aLk );
    m_pColumnBtn->SetClickHdl( aLk );
    m_pPageCollBox->SetSelectHdl( LINK(this,SwBreakDlg,SelectHdl) );

    get<OKButton>("ok")->SetClickHdl(LINK(this,SwBreakDlg,OkHdl));
    m_pPageNumBox->SetClickHdl(LINK(this,SwBreakDlg,PageNumHdl));
    m_pPageNumEdit->SetModifyHdl(LINK(this,SwBreakDlg,PageNumModifyHdl));

    // Insert page description to Listbox
    const size_t nCount = rSh.GetPageDescCnt();
    for( size_t i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = rSh.GetPageDesc(i);
        ::InsertStringSorted(rPageDesc.GetName(), *m_pPageCollBox, 1 );
    }

    OUString aFormatName;
    for(sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
    {
        aFormatName = SwStyleNameMapper::GetUIName( i, aFormatName );
        if(LISTBOX_ENTRY_NOTFOUND == m_pPageCollBox->GetEntryPos(aFormatName))
            ::InsertStringSorted(aFormatName, *m_pPageCollBox, 1 );
    }
    //add landscape page
    aFormatName = SwStyleNameMapper::GetUIName( RES_POOLPAGE_LANDSCAPE, aFormatName );
    if(LISTBOX_ENTRY_NOTFOUND == m_pPageCollBox->GetEntryPos(aFormatName))
            ::InsertStringSorted(aFormatName, *m_pPageCollBox, 1 );
    CheckEnable();
    m_pPageNumEdit->SetText(OUString());
}

SwBreakDlg::~SwBreakDlg()
{
    disposeOnce();
}

void SwBreakDlg::dispose()
{
    m_pLineBtn.clear();
    m_pColumnBtn.clear();
    m_pPageBtn.clear();
    m_pPageCollText.clear();
    m_pPageCollBox.clear();
    m_pPageNumBox.clear();
    m_pPageNumEdit.clear();
    SvxStandardDialog::dispose();
}

void SwBreakDlg::CheckEnable()
{
    bool bEnable = true;
    if ( bHtmlMode )
    {
        m_pColumnBtn->Enable(false);
        m_pPageCollBox->Enable(false);
        bEnable = false;
    }
    else if(rSh.GetFrameType(nullptr,true)
        & (FrameTypeFlags::FLY_ANY | FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER  | FrameTypeFlags::FOOTNOTE))
    {
        m_pPageBtn->Enable(false);
        if(m_pPageBtn->IsChecked())
            m_pLineBtn->Check();
        bEnable = false;
    }
    const bool bPage = m_pPageBtn->IsChecked();
    m_pPageCollText->Enable( bPage );
    m_pPageCollBox->Enable ( bPage );

    bEnable &= bPage;
    if ( bEnable )
    {
        // position 0 says 'Without' page template.
        const sal_Int32 nPos = m_pPageCollBox->GetSelectEntryPos();
        if ( 0 == nPos || LISTBOX_ENTRY_NOTFOUND == nPos )
            bEnable = false;
    }
    m_pPageNumBox->Enable(bEnable);
    m_pPageNumEdit->Enable(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
