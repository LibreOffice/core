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

#include <actctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>

NumEditAction::NumEditAction(vcl::Window* pParent)
    : InterimItemWindow(pParent, "modules/swriter/ui/spinbox.ui", "SpinBox")
    , m_xWidget(m_xBuilder->weld_spin_button("spin"))
{
    m_xWidget->connect_key_press(LINK(this, NumEditAction, KeyInputHdl));
    limitWidth();
    SetSizePixel(m_xContainer->get_preferred_size());
}

void NumEditAction::limitWidth()
{
    m_xWidget->set_width_chars(3);
}

void NumEditAction::set_max(int nMax)
{
    m_xWidget->set_max(nMax);
    limitWidth();
}

IMPL_LINK(NumEditAction, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
