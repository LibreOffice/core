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

#include <pagenumberdlg.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/virdev.hxx>

SwPageNumberDlg::SwPageNumberDlg(weld::Window* pParent)
    : SfxDialogController(pParent, "modules/swriter/ui/pagenumberdlg.ui", "PageNumberDialog")
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xCancel(m_xBuilder->weld_button("cancel"))
    , m_xPageNumberPosition(m_xBuilder->weld_combo_box("positionCombo"))
    , m_xPageNumberAlignment(m_xBuilder->weld_combo_box("alignmentCombo"))
    , m_xPreviewImage(m_xBuilder->weld_image("previewImage"))
    , m_aPageNumberPosition(1)
    , m_aPageNumberAlignment(0)
{
    m_xOk->connect_clicked(LINK(this, SwPageNumberDlg, OkHdl));
    m_xPageNumberPosition->connect_changed(LINK(this, SwPageNumberDlg, PositionSelectHdl));
    m_xPageNumberAlignment->connect_changed(LINK(this, SwPageNumberDlg, AlignmentSelectHdl));
    m_xPageNumberPosition->set_active(m_aPageNumberPosition);
    m_xPageNumberAlignment->set_active(m_aPageNumberAlignment);
    updateImage();
}

IMPL_LINK_NOARG(SwPageNumberDlg, OkHdl, weld::Button&, void) { m_xDialog->response(RET_OK); }

IMPL_LINK_NOARG(SwPageNumberDlg, CancelHdl, weld::Button&, void)
{
    m_xDialog->response(RET_CANCEL);
}

IMPL_LINK_NOARG(SwPageNumberDlg, PositionSelectHdl, weld::ComboBox&, void)
{
    m_aPageNumberPosition = m_xPageNumberPosition->get_active();
    updateImage();
}

IMPL_LINK_NOARG(SwPageNumberDlg, AlignmentSelectHdl, weld::ComboBox&, void)
{
    m_aPageNumberAlignment = m_xPageNumberAlignment->get_active();
    updateImage();
}

void SwPageNumberDlg::updateImage()
{
    int nBackgroundWidth = 75;
    int nBackgroundHeight = 105;

    int nSpriteWidth = 10;
    int nSpriteHeight = 14;

    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    Size aVDSize(nBackgroundWidth, nBackgroundHeight);
    pVirtualDev->SetOutputSizePixel(aVDSize);
    pVirtualDev->SetBackground(Color(0xF0, 0xF0, 0xF0));
    pVirtualDev->Erase();

    int y = m_aPageNumberPosition ? (nBackgroundHeight - nSpriteHeight - 5) : 5;
    int x = 5;
    if (m_aPageNumberAlignment == 1)
    {
        x = (nBackgroundWidth - nSpriteWidth) / 2;
    }
    else if (m_aPageNumberAlignment == 2)
    {
        x = nBackgroundWidth - nSpriteWidth - 5;
    }
    pVirtualDev->DrawText(Point(x, y), "#");

    m_xPreviewImage->set_image(pVirtualDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
