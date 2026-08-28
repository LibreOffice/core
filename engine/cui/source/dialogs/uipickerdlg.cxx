/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <uipickerdlg.hxx>
#include <toolbartabpage.hxx>

#include <dialmgr.hxx>
#include <strings.hrc>

#include <vcl/tabs.hrc>

UIPickerDialog::UIPickerDialog(weld::Window* pParent)
    : SfxTabDialogController(pParent, u"cui/ui/uipickerdialog.ui"_ustr, u"UIPickerDialog"_ustr)
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xApplyBtn(m_xBuilder->weld_button(u"apply"_ustr))
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr)) // Close
    , m_xResetBtn(m_xBuilder->weld_button(u"reset"_ustr))
{
    AddTabPage(u"toolbars"_ustr, TabResId(RID_TAB_TOOLBARS.aLabel), ToolbarTabPage::Create,
               RID_L + RID_TAB_TOOLBARS.sIconName);

    m_xOKBtn->set_visible(false);
    m_xApplyBtn->set_visible(false);
    m_xResetBtn->set_visible(false);
    m_xCancelBtn->set_label(CuiResId(RID_CUISTR_HYPDLG_CLOSEBUT)); // "close"
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
