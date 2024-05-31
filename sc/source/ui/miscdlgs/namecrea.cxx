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

#include <namecrea.hxx>

ScNameCreateDlg::ScNameCreateDlg(weld::Window* pParent, CreateNameFlags nFlags)
    : GenericDialogController(pParent, u"modules/scalc/ui/createnamesdialog.ui"_ustr,
                              u"CreateNamesDialog"_ustr)
    , m_xTopBox(m_xBuilder->weld_check_button(u"top"_ustr))
    , m_xLeftBox(m_xBuilder->weld_check_button(u"left"_ustr))
    , m_xBottomBox(m_xBuilder->weld_check_button(u"bottom"_ustr))
    , m_xRightBox(m_xBuilder->weld_check_button(u"right"_ustr))
{
    m_xTopBox->set_active(bool(nFlags & CreateNameFlags::Top));
    m_xLeftBox->set_active(bool(nFlags & CreateNameFlags::Left));
    m_xBottomBox->set_active(bool(nFlags & CreateNameFlags::Bottom));
    m_xRightBox->set_active(bool(nFlags & CreateNameFlags::Right));
}

ScNameCreateDlg::~ScNameCreateDlg() {}

CreateNameFlags ScNameCreateDlg::GetFlags() const
{
    CreateNameFlags nResult = CreateNameFlags::NONE;

    if (m_xTopBox->get_active())
        nResult |= CreateNameFlags::Top;
    if (m_xLeftBox->get_active())
        nResult |= CreateNameFlags::Left;
    if (m_xBottomBox->get_active())
        nResult |= CreateNameFlags::Bottom;
    if (m_xRightBox->get_active())
        nResult |= CreateNameFlags::Right;

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
