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
#include <iostream>
#include <vector>
#include <sal/config.h>
#include <sal/log.hxx>

#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/weld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <helpids.h>
#include <strings.hrc>

#include <acccfg.hxx>
#include <cfg.hxx>
#include <SvxNotebookbarConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include <dialmgr.hxx>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

SvxNotebookbarConfigPage::SvxNotebookbarConfigPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SvxConfigPage(pParent, rSet)
{
    m_xDescriptionFieldLb->set_visible(false);
    m_xSearchEdit->set_visible(false);
    m_xDescriptionField->set_visible(false);
    m_xMoveUpButton->set_visible(false);
    m_xMoveDownButton->set_visible(false);
    m_xAddCommandButton->set_visible(false);
    m_xRemoveCommandButton->set_visible(false);
    m_xLeftFunctionLabel->set_visible(false);
    m_xSearchLabel->set_visible(false);
    m_xCategoryLabel->set_visible(false);
    m_xCategoryListBox->set_visible(false);
    m_xInsertBtn->set_visible(false);
    m_xModifyBtn->set_visible(false);
    m_xResetBtn->set_visible(false);
    m_xCustomizeLabel->set_visible(false);
    weld::TreeView& rCommandCategoryBox = m_xFunctions->get_widget();
    rCommandCategoryBox.hide();

    m_xContentsListBox.reset(new SvxNotebookbarEntriesListBox(m_xBuilder->weld_tree_view("toolcontents"), this));
    std::vector<int> aWidths;
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    Size aSize(m_xFunctions->get_size_request());
    rTreeView.set_size_request(aSize.Width(), aSize.Height());

    int nExpectedSize = 16;

    int nStandardImageColWidth = rTreeView.get_checkbox_column_width();
    int nMargin = nStandardImageColWidth - nExpectedSize;
    if (nMargin < 16)
        nMargin = 16;

    if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_LARGE)
        nExpectedSize = 24;
    else if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_32)
        nExpectedSize = 32;

    int nImageColWidth = nExpectedSize + nMargin;

    aWidths.push_back(nStandardImageColWidth);
    aWidths.push_back(nImageColWidth);
    rTreeView.set_column_fixed_widths(aWidths);

    rTreeView.set_hexpand(true);
    rTreeView.set_vexpand(true);
    rTreeView.set_help_id( HID_SVX_CONFIG_TOOLBAR_CONTENTS );
    rTreeView.show();
    rTreeView.connect_changed(
        LINK( this, SvxNotebookbarConfigPage, SelectToolbarEntry ) );

    rTreeView.connect_model_changed(LINK(this, SvxNotebookbarConfigPage, ListModifiedHdl));

    m_xCommandCategoryListBox->connect_changed(
        LINK( this, SvxNotebookbarConfigPage, SelectCategory ) );

    m_xResetBtn->connect_clicked(
        LINK( this, SvxNotebookbarConfigPage, ResetToolbarHdl ) );

}

IMPL_LINK_NOARG(SvxNotebookbarConfigPage, ListModifiedHdl, weld::TreeView&, void) {}

SvxNotebookbarConfigPage::~SvxNotebookbarConfigPage() {}

void SvxNotebookbarConfigPage::dispose(){}

void SvxNotebookbarConfigPage::DeleteSelectedTopLevel() {}

void SvxNotebookbarConfigPage::DeleteSelectedContent() {}

void SvxNotebookbarConfigPage::Init() {}

SaveInData* SvxNotebookbarConfigPage::CreateSaveInData(
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    return static_cast< SaveInData* >(
        new ToolbarSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ));
}

IMPL_LINK_NOARG(SvxNotebookbarConfigPage, SelectToolbarEntry, weld::TreeView&, void) {}

IMPL_LINK_NOARG( SvxNotebookbarConfigPage, SelectCategory, weld::ComboBox&, void ) {}

IMPL_LINK_NOARG(SvxNotebookbarConfigPage, ResetToolbarHdl, weld::Button&, void) {}

void SvxNotebookbarConfigPage::UpdateButtonStates() {}

short SvxNotebookbarConfigPage::QueryReset() {}

void SvxNotebookbarConfigPage::SelectElement() {}

SvxNotebookbarEntriesListBox::SvxNotebookbarEntriesListBox(std::unique_ptr<weld::TreeView> xParent, SvxConfigPage* pPg)
    : SvxMenuEntriesListBox(std::move(xParent), pPg)
{
    m_xControl->connect_toggled(LINK(this, SvxNotebookbarEntriesListBox, CheckButtonHdl));
    m_xControl->connect_key_press(Link<const KeyEvent&, bool>());
    m_xControl->connect_key_press(LINK(this, SvxNotebookbarEntriesListBox, KeyInputHdl));
}

SvxNotebookbarEntriesListBox::~SvxNotebookbarEntriesListBox() {}

void SvxNotebookbarEntriesListBox::ChangedVisibility(int nRow) {}

IMPL_LINK(SvxNotebookbarEntriesListBox, CheckButtonHdl, const row_col&, rRowCol, void) {}

IMPL_LINK(SvxNotebookbarEntriesListBox, KeyInputHdl, const KeyEvent&, rKeyEvent, bool) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
