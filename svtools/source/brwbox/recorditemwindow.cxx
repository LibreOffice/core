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

#include <svtools/recorditemwindow.hxx>
#include <vcl/event.hxx>

RecordItemWindowBase::RecordItemWindowBase(std::unique_ptr<weld::Entry> xEntry)
    : m_xWidget(std::move(xEntry))
{
    m_xWidget->connect_key_press(LINK(this, RecordItemWindowBase, KeyInputHdl));
    m_xWidget->connect_activate(LINK(this, RecordItemWindowBase, ActivatedHdl));
    m_xWidget->connect_focus_out(LINK(this, RecordItemWindowBase, FocusOutHdl));

    m_xWidget->show();
}

RecordItemWindowBase::~RecordItemWindowBase() {}

RecordItemWindow::RecordItemWindow(vcl::Window* pParent)
    : InterimItemWindow(pParent, "svx/ui/absrecbox.ui", "AbsRecBox")
    , RecordItemWindowBase(m_xBuilder->weld_entry("entry-frame"))
{
    InitControlBase(m_xWidget.get());

    auto aPrefSize(m_xWidget->get_preferred_size());

    m_xWidget->set_width_chars(1); // so a smaller than default width can be used later

    SetSizePixel(aPrefSize);
}

void RecordItemWindow::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

RecordItemWindow::~RecordItemWindow() { disposeOnce(); }

void RecordItemWindowBase::FirePosition(bool _bForce)
{
    if (!_bForce && !m_xWidget->get_value_changed_from_saved())
        return;

    sal_Int64 nRecord = m_xWidget->get_text().toInt64();
    if (nRecord < 1)
        nRecord = 1;

    PositionFired(nRecord);

    m_xWidget->save_value();
}

IMPL_LINK_NOARG(RecordItemWindowBase, FocusOutHdl, weld::Widget&, void) { FirePosition(false); }

bool RecordItemWindowBase::DoKeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    bool bUp = (aCode.GetCode() == KEY_UP);
    bool bDown = (aCode.GetCode() == KEY_DOWN);

    if (!aCode.IsShift() && !aCode.IsMod1() && !aCode.IsMod2() && (bUp || bDown))
    {
        sal_Int64 nRecord = m_xWidget->get_text().toInt64();
        if (bUp)
            ++nRecord;
        else
            --nRecord;
        if (nRecord < 1)
            nRecord = 1;
        m_xWidget->set_text(OUString::number(nRecord));
        return true;
    }

    return false;
}

bool RecordItemWindow::DoKeyInput(const KeyEvent& rKEvt)
{
    return RecordItemWindowBase::DoKeyInput(rKEvt) || ChildKeyInput(rKEvt);
}

void RecordItemWindowBase::PositionFired(sal_Int64 /*nRecord*/) {}

IMPL_LINK(RecordItemWindowBase, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return DoKeyInput(rKEvt);
}

IMPL_LINK_NOARG(RecordItemWindowBase, ActivatedHdl, weld::Entry&, bool)
{
    if (!m_xWidget->get_text().isEmpty())
        FirePosition(true);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
