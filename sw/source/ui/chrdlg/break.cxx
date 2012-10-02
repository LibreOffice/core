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

#include <sfx2/request.hxx>
#include <svl/stritem.hxx>
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
        const sal_uInt16 nPos = m_pPageCollBox->GetSelectEntryPos();
        if(0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos)
        {
            aTemplate = m_pPageCollBox->GetSelectEntry();
            nPgNum = m_pPageNumBox->IsChecked() ? (sal_uInt16)m_pPageNumEdit->GetValue() : 0;
        }
    }
}

IMPL_LINK_NOARG_INLINE_START(SwBreakDlg, ClickHdl)
{
    CheckEnable();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwBreakDlg, ClickHdl)

/*------------------------------------------------------------------------
 Description:   Handler for Change Page Number
------------------------------------------------------------------------*/

IMPL_LINK_INLINE_START( SwBreakDlg, PageNumHdl, CheckBox *, pBox )
{
    if(pBox->IsChecked()) m_pPageNumEdit->SetValue(1);
    else m_pPageNumEdit->SetText(OUString());
    return 0;
}
IMPL_LINK_INLINE_END( SwBreakDlg, PageNumHdl, CheckBox *, pBox )

/*------------------------------------------------------------------------
 Description:   By changing the Page number the checkbox is checked.
------------------------------------------------------------------------*/

IMPL_LINK_NOARG_INLINE_START(SwBreakDlg, PageNumModifyHdl)
{
    m_pPageNumBox->Check();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SwBreakDlg, PageNumModifyHdl)

/*------------------------------------------------------------------------
 Description:   Ok-Handler;
                checks whether pagenumber nPage is a legal pagenumber
                (left pages with even numbers etc. for a page template
                with alternating pages)
------------------------------------------------------------------------*/

IMPL_LINK_NOARG(SwBreakDlg, OkHdl)
{
    if(m_pPageNumBox->IsChecked()) {
        // In case of differing page descriptions, test validity
        const sal_uInt16 nPos = m_pPageCollBox->GetSelectEntryPos();
        // position 0 says 'Without'.
        const SwPageDesc *pPageDesc;
        if ( 0 != nPos && LISTBOX_ENTRY_NOTFOUND != nPos )
            pPageDesc = rSh.FindPageDescByName( m_pPageCollBox->GetSelectEntry(),
                                                sal_True );
        else
            pPageDesc = &rSh.GetPageDesc(rSh.GetCurPageDesc());

        OSL_ENSURE(pPageDesc, "Page description not found.");
        const sal_uInt16 nUserPage = sal_uInt16(m_pPageNumEdit->GetValue());
        sal_Bool bOk = sal_True;
        switch(pPageDesc->GetUseOn())
        {
            case nsUseOnPage::PD_MIRROR:
            case nsUseOnPage::PD_ALL: break;
            case nsUseOnPage::PD_LEFT: bOk = 0 == nUserPage % 2; break;
            case nsUseOnPage::PD_RIGHT: bOk = static_cast< sal_Bool >(nUserPage % 2); break;
            default:; //prevent warning
        }
        if(!bOk) {
            InfoBox(this, SW_RES(MSG_ILLEGAL_PAGENUM)).Execute();
            m_pPageNumEdit->GrabFocus();
            return 0;
        }
    }
    EndDialog(RET_OK);
    return 0;
}

SwBreakDlg::SwBreakDlg( Window *pParent, SwWrtShell &rS )
    : SvxStandardDialog(pParent, "BreakDialog", "modules/swriter/ui/insertbreak.ui")
    , rSh(rS)
    , nKind(0)
    , nPgNum(0)
    , bHtmlMode(0 != ::GetHtmlMode(rS.GetView().GetDocShell()))
{
    get(m_pLineBtn, "linerb");
    get(m_pColumnBtn, "columnrb");
    get(m_pPageBtn, "pagerb");
    get(m_pPageCollText, "styleft");
    get(m_pPageCollBox, "stylelb");
    get(m_pPageNumBox, "pagenumcb");
    get(m_pPageNumEdit, "pagenumsb");

    m_pPageNumEdit->SetAccessibleRelationLabeledBy(m_pPageNumBox);
    m_pPageNumEdit->SetAccessibleName(m_pPageNumBox->GetText());

    Link aLk = LINK(this,SwBreakDlg,ClickHdl);
    m_pPageBtn->SetClickHdl( aLk );
    m_pLineBtn->SetClickHdl( aLk );
    m_pColumnBtn->SetClickHdl( aLk );
    m_pPageCollBox->SetSelectHdl( aLk );

    get<OKButton>("ok")->SetClickHdl(LINK(this,SwBreakDlg,OkHdl));;
    m_pPageNumBox->SetClickHdl(LINK(this,SwBreakDlg,PageNumHdl));
    m_pPageNumEdit->SetModifyHdl(LINK(this,SwBreakDlg,PageNumModifyHdl));


    // Insert page description to Listbox
    const sal_uInt16 nCount = rSh.GetPageDescCnt();
    sal_uInt16 i;

    for( i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = rSh.GetPageDesc(i);
        ::InsertStringSorted(rPageDesc.GetName(), *m_pPageCollBox, 1 );
    }

    String aFmtName;
    for(i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
        if(LISTBOX_ENTRY_NOTFOUND == m_pPageCollBox->GetEntryPos( aFmtName =
                                    SwStyleNameMapper::GetUIName( i, aFmtName )))
            ::InsertStringSorted(aFmtName, *m_pPageCollBox, 1 );
    //add landscape page
    if(LISTBOX_ENTRY_NOTFOUND == m_pPageCollBox->GetEntryPos( aFmtName =
                                    SwStyleNameMapper::GetUIName( RES_POOLPAGE_LANDSCAPE, aFmtName )))
            ::InsertStringSorted(aFmtName, *m_pPageCollBox, 1 );
    CheckEnable();
    m_pPageNumEdit->SetText(OUString());
}

void SwBreakDlg::CheckEnable()
{
    sal_Bool bEnable = sal_True;
    if ( bHtmlMode )
    {
        m_pColumnBtn->Enable(sal_False);
        m_pPageCollBox->Enable(sal_False);
        bEnable = sal_False;
    }
    else if(rSh.GetFrmType(0,sal_True)
        & (FRMTYPE_FLY_ANY | FRMTYPE_HEADER | FRMTYPE_FOOTER  | FRMTYPE_FOOTNOTE))
    {
        m_pPageBtn->Enable(sal_False);
        if(m_pPageBtn->IsChecked())
            m_pLineBtn->Check(sal_True);
        bEnable = sal_False;
    }
    const sal_Bool bPage = m_pPageBtn->IsChecked();
    m_pPageCollText->Enable( bPage );
    m_pPageCollBox->Enable ( bPage );

    bEnable &= bPage;
    if ( bEnable )
    {
        // position 0 says 'Without' page template.
        const sal_uInt16 nPos = m_pPageCollBox->GetSelectEntryPos();
        if ( 0 == nPos || LISTBOX_ENTRY_NOTFOUND == nPos )
            bEnable = sal_False;
    }
    m_pPageNumBox->Enable(bEnable);
    m_pPageNumEdit->Enable(bEnable);
}

SwBreakDlg::~SwBreakDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
