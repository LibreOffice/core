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

#include <comphelper/string.hxx>
#include <svx/colorbox.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/weld.hxx>
#include <unotools/localedatawrapper.hxx>

#include <global.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <tabvwsh.hxx>
#include <viewdata.hxx>
#include <document.hxx>
#include <scendlg.hxx>

ScNewScenarioDlg::ScNewScenarioDlg(weld::Window* pParent, const OUString& rName, bool bEdit, bool bSheetProtected)
    : GenericDialogController(pParent, "modules/scalc/ui/scenariodialog.ui", "ScenarioDialog")
    , aDefScenarioName(rName)
    , bIsEdit(bEdit)
    , m_xEdName(m_xBuilder->weld_entry("name"))
    , m_xEdComment(m_xBuilder->weld_text_view("comment"))
    , m_xCbShowFrame(m_xBuilder->weld_check_button("showframe"))
    , m_xLbColor(new ColorListBox(m_xBuilder->weld_menu_button("bordercolor"), pParent))
    , m_xCbTwoWay(m_xBuilder->weld_check_button("copyback"))
    , m_xCbCopyAll(m_xBuilder->weld_check_button("copysheet"))
    , m_xCbProtect(m_xBuilder->weld_check_button("preventchanges"))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
    , m_xAltTitle(m_xBuilder->weld_label("alttitle"))
    , m_xCreatedFt(m_xBuilder->weld_label("createdft"))
    , m_xOnFt(m_xBuilder->weld_label("onft"))
{
    m_xEdComment->set_size_request(m_xEdComment->get_approximate_digit_width() * 60,
                                   m_xEdComment->get_height_rows(6));

    if (bIsEdit)
        m_xDialog->set_title(m_xAltTitle->get_label());

    SvtUserOptions aUserOpt;

    OUString sCreatedBy(m_xCreatedFt->get_label());
    OUString sOn(m_xOnFt->get_label());

    OUString aComment(sCreatedBy + " " + aUserOpt.GetFirstName() + " " +aUserOpt.GetLastName()
              + ", " + sOn + " " + ScGlobal::GetpLocaleData()->getDate(Date(Date::SYSTEM))
              + ", " + ScGlobal::GetpLocaleData()->getTime(tools::Time(tools::Time::SYSTEM)));

    m_xEdComment->set_text(aComment);
    m_xEdName->set_text(rName);
    m_xBtnOk->connect_clicked(LINK(this, ScNewScenarioDlg, OkHdl));
    m_xCbShowFrame->connect_toggled(LINK(this, ScNewScenarioDlg, EnableHdl));

    m_xLbColor->SelectEntry( COL_LIGHTGRAY );
    m_xCbShowFrame->set_active(true);
    m_xCbTwoWay->set_active(true);
    m_xCbCopyAll->set_active(false);
    m_xCbProtect->set_active(true);

    if (bIsEdit)
        m_xCbCopyAll->set_active(false);
    // If the Sheet is protected then we disable the Scenario Protect input
    // and default it to true above. Note we are in 'Add' mode here as: if
    // Sheet && scenario protection are true, then we cannot edit this dialog.
    if (bSheetProtected)
        m_xCbProtect->set_active(false);
}

ScNewScenarioDlg::~ScNewScenarioDlg()
{
}

void ScNewScenarioDlg::GetScenarioData( OUString& rName, OUString& rComment,
                                        Color& rColor, ScScenarioFlags& rFlags ) const
{
    rComment = m_xEdComment->get_text();
    rName    = m_xEdName->get_text();

    if (rName.isEmpty())
        rName = aDefScenarioName;

    rColor = m_xLbColor->GetSelectEntryColor();
    ScScenarioFlags nBits = ScScenarioFlags::NONE;
    if (m_xCbShowFrame->get_active())
        nBits |= ScScenarioFlags::ShowFrame;
    if (m_xCbTwoWay->get_active())
        nBits |= ScScenarioFlags::TwoWay;
    if (m_xCbCopyAll->get_active())
        nBits |= ScScenarioFlags::CopyAll;
    if (m_xCbProtect->get_active())
        nBits |= ScScenarioFlags::Protected;
    rFlags = nBits;
}

void ScNewScenarioDlg::SetScenarioData(const OUString& rName, const OUString& rComment,
                                        const Color& rColor, ScScenarioFlags nFlags)
{
    m_xEdComment->set_text(rComment);
    m_xEdName->set_text(rName);
    m_xLbColor->SelectEntry(rColor);

    m_xCbShowFrame->set_active( (nFlags & ScScenarioFlags::ShowFrame) != ScScenarioFlags::NONE );
    EnableHdl(*m_xCbShowFrame);
    m_xCbTwoWay->set_active( (nFlags & ScScenarioFlags::TwoWay)    != ScScenarioFlags::NONE );
    //  not CopyAll
    m_xCbProtect->set_active( (nFlags & ScScenarioFlags::Protected) != ScScenarioFlags::NONE );
}

IMPL_LINK_NOARG(ScNewScenarioDlg, OkHdl, weld::Button&, void)
{
    OUString      aName = comphelper::string::strip(m_xEdName->get_text(), ' ');
    ScDocument* pDoc    = static_cast<ScTabViewShell*>(SfxViewShell::Current())->GetViewData().GetDocument();

    m_xEdName->set_text(aName);

    if ( !ScDocument::ValidTabName( aName ) )
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(STR_INVALIDTABNAME)));
        xInfoBox->run();
        m_xEdName->grab_focus();
    }
    else if ( !bIsEdit && !pDoc->ValidNewTabName( aName ) )
    {
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      ScResId(STR_NEWTABNAMENOTUNIQUE)));
        xInfoBox->run();
        m_xEdName->grab_focus();
    }
    else
        m_xDialog->response(RET_OK);

    //! when editing, test whether another table has the name!
}

IMPL_LINK(ScNewScenarioDlg, EnableHdl, weld::ToggleButton&, rBox, void)
{
    if (&rBox == m_xCbShowFrame.get())
        m_xLbColor->set_sensitive(m_xCbShowFrame->get_active());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
