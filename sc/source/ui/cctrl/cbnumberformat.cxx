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

#include <cbnumberformat.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <sc.hrc>

ScNumberFormat::ScNumberFormat(vcl::Window* pParent)
    : InterimItemWindow(pParent, u"modules/scalc/ui/numberbox.ui"_ustr, u"NumberBox"_ustr, true,
                        reinterpret_cast<sal_uInt64>(SfxViewShell::Current()))
    , m_xWidget(m_xBuilder->weld_combo_box(u"numbertype"_ustr))
{
    m_xWidget->append_text(ScResId(STR_GENERAL));
    m_xWidget->append_text(ScResId(STR_NUMBER));
    m_xWidget->append_text(ScResId(STR_PERCENT));
    m_xWidget->append_text(ScResId(STR_CURRENCY));
    m_xWidget->append_text(ScResId(STR_DATE));
    m_xWidget->append_text(ScResId(STR_TIME));
    m_xWidget->append_text(ScResId(STR_SCIENTIFIC));
    m_xWidget->append_text(ScResId(STR_FRACTION));
    m_xWidget->append_text(ScResId(STR_BOOLEAN_VALUE));
    m_xWidget->append_text(ScResId(STR_TEXT));

    m_xWidget->connect_changed(LINK(this, ScNumberFormat, NumFormatSelectHdl));
    m_xWidget->connect_key_press(LINK(this, ScNumberFormat, KeyInputHdl));

    SetSizePixel(m_xWidget->get_preferred_size());
}

void ScNumberFormat::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

ScNumberFormat::~ScNumberFormat() { disposeOnce(); }

void ScNumberFormat::GetFocus()
{
    if (m_xWidget)
        m_xWidget->grab_focus();
    InterimItemWindow::GetFocus();
}

IMPL_STATIC_LINK(ScNumberFormat, NumFormatSelectHdl, weld::ComboBox&, rBox, void)
{
    auto* pCurSh = SfxViewFrame::Current();
    if (!pCurSh)
        return;

    SfxDispatcher* pDisp = pCurSh->GetBindings().GetDispatcher();
    if (pDisp)
    {
        const sal_Int32 nVal = rBox.get_active();
        SfxUInt16Item aItem(SID_NUMBER_TYPE_FORMAT, nVal);
        pDisp->ExecuteList(SID_NUMBER_TYPE_FORMAT, SfxCallMode::RECORD, { &aItem });

        pCurSh->GetWindow().GrabFocus();
    }
}

IMPL_LINK(ScNumberFormat, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
