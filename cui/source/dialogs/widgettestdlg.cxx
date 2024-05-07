/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <widgettestdlg.hxx>
#include <bitmaps.hlst>

WidgetTestDialog::WidgetTestDialog(weld::Window* pParent)
    : GenericDialogController(pParent, u"cui/ui/widgettestdialog.ui"_ustr, u"WidgetTestDialog"_ustr)
{
    m_xOKButton = m_xBuilder->weld_button(u"ok_btn"_ustr);
    m_xCancelButton = m_xBuilder->weld_button(u"cancel_btn"_ustr);
    m_xTreeView = m_xBuilder->weld_tree_view(u"contenttree"_ustr);
    m_xTreeView2 = m_xBuilder->weld_tree_view(u"contenttree2"_ustr);

    m_xOKButton->connect_clicked(LINK(this, WidgetTestDialog, OkHdl));
    m_xCancelButton->connect_clicked(LINK(this, WidgetTestDialog, CancelHdl));

    FillTreeView();
}

WidgetTestDialog::~WidgetTestDialog() {}

IMPL_LINK_NOARG(WidgetTestDialog, OkHdl, weld::Button&, void) { m_xDialog->response(RET_OK); }

IMPL_LINK_NOARG(WidgetTestDialog, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

void WidgetTestDialog::FillTreeView()
{
    OUString aImage1(RID_SVXBMP_CELL_LR);
    OUString aImage2(RID_SVXBMP_SHADOW_BOT_LEFT);

    for (size_t nCount = 0; nCount < 4; nCount++)
    {
        OUString sText = OUString::Concat("Test ") + OUString::Concat(OUString::number(nCount));
        std::unique_ptr<weld::TreeIter> xEntry = m_xTreeView->make_iterator();
        m_xTreeView->insert(nullptr, -1, &sText, &sText, nullptr, nullptr, false, xEntry.get());
        m_xTreeView->set_image(*xEntry, (nCount % 2 == 0) ? aImage1 : aImage2);

        m_xTreeView2->append();
        m_xTreeView2->set_image(nCount, (nCount % 2 == 0) ? aImage1 : aImage2);
        m_xTreeView2->set_text(nCount, u"First Column"_ustr, 0);
        m_xTreeView2->set_text(
            nCount, OUString::Concat("Row ") + OUString::Concat(OUString::number(nCount)), 1);
        m_xTreeView2->set_id(nCount, OUString::number(nCount));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
