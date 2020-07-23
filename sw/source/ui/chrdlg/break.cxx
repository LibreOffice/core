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

#include <vcl/svapp.hxx>

#include <uitool.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <viewopt.hxx>
#include <break.hxx>
#include <pagedesc.hxx>
#include <poolfmt.hxx>

#include <strings.hrc>
#include <SwStyleNameMapper.hxx>

void SwBreakDlg::rememberResult()
{
    nKind = 0;
    if (m_xLineBtn->get_active())
        nKind = 1;
    else if(m_xColumnBtn->get_active())
        nKind = 2;
    else if(m_xPageBtn->get_active())
    {
        nKind = 3;
        const int nPos = m_xPageCollBox->get_active();
        if (nPos != 0 && nPos != -1)
        {
            m_aTemplate = m_xPageCollBox->get_active_text();
            oPgNum.reset();
            if (m_xPageNumBox->get_active())
            {
                oPgNum = static_cast<sal_uInt16>(m_xPageNumEdit->get_value());
            }
        }
    }
}

IMPL_LINK_NOARG(SwBreakDlg, ToggleHdl, weld::ToggleButton&, void)
{
    CheckEnable();
}

IMPL_LINK_NOARG(SwBreakDlg, ChangeHdl, weld::ComboBox&, void)
{
    CheckEnable();
}

// Handler for Change Page Number
IMPL_LINK(SwBreakDlg, PageNumHdl, weld::ToggleButton&, rBox, void)
{
    if (rBox.get_active())
        m_xPageNumEdit->set_value(1);
    else
        m_xPageNumEdit->set_text(OUString());
}

// By changing the Page number the checkbox is checked.
IMPL_LINK_NOARG(SwBreakDlg, PageNumModifyHdl, weld::SpinButton&, void)
{
    m_xPageNumBox->set_active(true);
}

/*
 * Ok-Handler;
 * checks whether pagenumber nPage is a legal pagenumber (left pages with even
 * numbers etc. for a page template with alternating pages)
 */
IMPL_LINK_NOARG(SwBreakDlg, OkHdl, weld::Button&, void)
{
    if (m_xPageNumBox->get_active())
    {
        // In case of differing page descriptions, test validity
        const int nPos = m_xPageCollBox->get_active();
        // position 0 says 'Without'.
        const SwPageDesc *pPageDesc;
        if (nPos != 0 && nPos != -1)
            pPageDesc = rSh.FindPageDescByName(m_xPageCollBox->get_active_text(), true);
        else
            pPageDesc = &rSh.GetPageDesc(rSh.GetCurPageDesc());

        OSL_ENSURE(pPageDesc, "Page description not found.");
        const sal_uInt16 nUserPage = sal_uInt16(m_xPageNumEdit->get_value());
        bool bOk = true;
        switch(pPageDesc->GetUseOn())
        {
            case UseOnPage::Mirror:
            case UseOnPage::All: break;
            case UseOnPage::Left: bOk = 0 == nUserPage % 2; break;
            case UseOnPage::Right: bOk = 1 == nUserPage % 2; break;
            default:; //prevent warning
        }
        if(!bOk)
        {
            std::unique_ptr<weld::Dialog> xDialog(Application::CreateMessageDialog(m_xPageNumEdit.get(), VclMessageType::Info,
                                                     VclButtonsType::Ok, SwResId(STR_ILLEGAL_PAGENUM)));
            xDialog->run();
            m_xPageNumEdit->grab_focus();
            return;
        }
    }
    rememberResult();
    m_xDialog->response(RET_OK);
}

SwBreakDlg::SwBreakDlg(weld::Window *pParent, SwWrtShell &rS)
    : GenericDialogController(pParent, "modules/swriter/ui/insertbreak.ui", "BreakDialog")
    , m_xLineBtn(m_xBuilder->weld_radio_button("linerb"))
    , m_xColumnBtn(m_xBuilder->weld_radio_button("columnrb"))
    , m_xPageBtn(m_xBuilder->weld_radio_button("pagerb"))
    , m_xPageCollText(m_xBuilder->weld_label("styleft"))
    , m_xPageCollBox(m_xBuilder->weld_combo_box("stylelb"))
    , m_xPageNumBox(m_xBuilder->weld_check_button("pagenumcb"))
    , m_xPageNumEdit(m_xBuilder->weld_spin_button("pagenumsb"))
    , m_xOkBtn(m_xBuilder->weld_button("ok"))
    , rSh(rS)
    , nKind(0)
    , bHtmlMode(0 != ::GetHtmlMode(rS.GetView().GetDocShell()))
{
    Link<weld::ToggleButton&,void> aLk = LINK(this, SwBreakDlg, ToggleHdl);
    m_xPageBtn->connect_toggled(aLk);
    m_xLineBtn->connect_toggled(aLk);
    m_xColumnBtn->connect_toggled(aLk);
    m_xPageCollBox->connect_changed(LINK(this, SwBreakDlg, ChangeHdl));

    m_xOkBtn->connect_clicked(LINK(this, SwBreakDlg, OkHdl));
    m_xPageNumBox->connect_toggled(LINK(this, SwBreakDlg, PageNumHdl));
    m_xPageNumEdit->connect_value_changed(LINK(this, SwBreakDlg, PageNumModifyHdl));

    // Insert page description to Listbox
    const size_t nCount = rSh.GetPageDescCnt();
    for (size_t i = 0; i < nCount; ++i)
    {
        const SwPageDesc &rPageDesc = rSh.GetPageDesc(i);
        ::InsertStringSorted("", rPageDesc.GetName(), *m_xPageCollBox, 1 );
    }

    OUString aFormatName;
    for (sal_uInt16 i = RES_POOLPAGE_BEGIN; i < RES_POOLPAGE_END; ++i)
    {
        aFormatName = SwStyleNameMapper::GetUIName( i, aFormatName );
        if (m_xPageCollBox->find_text(aFormatName) == -1)
            ::InsertStringSorted("", aFormatName, *m_xPageCollBox, 1 );
    }
    //add landscape page
    aFormatName = SwStyleNameMapper::GetUIName( RES_POOLPAGE_LANDSCAPE, aFormatName );
    if (m_xPageCollBox->find_text(aFormatName) == -1)
        ::InsertStringSorted("", aFormatName, *m_xPageCollBox, 1);
    CheckEnable();
    m_xPageNumEdit->set_text(OUString());
}

void SwBreakDlg::CheckEnable()
{
    bool bEnable = true;
    if ( bHtmlMode )
    {
        m_xColumnBtn->set_sensitive(false);
        m_xPageCollBox->set_sensitive(false);
        bEnable = false;
    }
    else if(rSh.GetFrameType(nullptr,true)
        & (FrameTypeFlags::FLY_ANY | FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER  | FrameTypeFlags::FOOTNOTE))
    {
        m_xPageBtn->set_sensitive(false);
        if (m_xPageBtn->get_active())
            m_xLineBtn->set_active(true);
        bEnable = false;
    }
    const bool bPage = m_xPageBtn->get_active();
    m_xPageCollText->set_sensitive(bPage);
    m_xPageCollBox->set_sensitive(bPage);

    bEnable &= bPage;
    if ( bEnable )
    {
        // position 0 says 'Without' page template.
        const int nPos = m_xPageCollBox->get_active();
        if (nPos == 0 || nPos == -1)
            bEnable = false;
    }
    m_xPageNumBox->set_sensitive(bEnable);
    m_xPageNumEdit->set_sensitive(bEnable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
