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

#include <tabbgcolordlg.hxx>

#include <svx/colorwindow.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/Dialog.hxx>

ScTabBgColorDlg::ScTabBgColorDlg(weld::Window* pParent, const OUString& rTitle,
                                 const Color& rDefaultColor)
    : GenericDialogController(pParent, u"modules/scalc/ui/tabcolordialog.ui"_ustr,
                              u"TabColorDialog"_ustr)
    , m_aTabBgColor(rDefaultColor)
    , m_xColorListBox(new ColorListBox(m_xBuilder->weld_menu_button(u"colorlistbox"_ustr),
                                       [pParent] { return pParent; }))
{
    m_xDialog->set_title(rTitle);
    m_xColorListBox->SetSlotId(0, /*bShowNoneButton=*/true);

    // tdf#163838 - select the current tab color when reopening the dialog
    if (rDefaultColor != COL_AUTO)
        m_xColorListBox->SelectEntry(rDefaultColor);

    m_xColorListBox->SetSelectHdl(LINK(this, ScTabBgColorDlg, ColorSelectedHdl));
    Application::PostUserEvent(LINK(this, ScTabBgColorDlg, AutoOpenPickerHdl));
}

ScTabBgColorDlg::~ScTabBgColorDlg()
{
}

Color ScTabBgColorDlg::GetSelectedColor() const
{
    return m_aTabBgColor;
}

IMPL_LINK_NOARG(ScTabBgColorDlg, ColorSelectedHdl, ColorListBox&, void)
{
    Color aColor = m_xColorListBox->GetSelectEntryColor();
    m_aTabBgColor = aColor == COL_NONE_COLOR ? COL_AUTO : aColor;
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ScTabBgColorDlg, AutoOpenPickerHdl, void*, void)
{
    m_xColorListBox->get_widget().set_active(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
