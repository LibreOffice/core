/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <checkin.hxx>

SfxCheckinDialog::SfxCheckinDialog(weld::Window* pParent)
    : GenericDialogController( pParent, u"sfx/ui/checkin.ui"_ustr, u"CheckinDialog"_ustr)
    , m_xCommentED(m_xBuilder->weld_text_view(u"VersionComment"_ustr))
    , m_xMajorCB(m_xBuilder->weld_check_button(u"MajorVersion"_ustr))
    , m_xOKBtn(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xOKBtn->connect_clicked(LINK(this, SfxCheckinDialog, OKHdl));
}

SfxCheckinDialog::~SfxCheckinDialog()
{
}

OUString SfxCheckinDialog::GetComment( ) const
{
    return m_xCommentED->get_text();
}

bool SfxCheckinDialog::IsMajor( ) const
{
    return m_xMajorCB->get_active();
}

IMPL_LINK_NOARG(SfxCheckinDialog, OKHdl, weld::Button&, void )
{
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
